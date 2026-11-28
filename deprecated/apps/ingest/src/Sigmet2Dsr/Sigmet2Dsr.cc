// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 

#include <cstdio>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <rapformats/DsRadarParams.hh>
#include <rapformats/DsFieldParams.hh>
#include <rapformats/DsRadarElev.hh>
#include <rapformats/DsRadarBeam.hh>

#include <trmm_rsl/rsl.h>

#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>
#include <cstdlib> 

#include "Sigmet2Dsr.hh"
using namespace std;

const int n_field_types = 32;
const char *output_field_name[n_field_types] =
  {"DZ", "VR", "SW", "CZ", "ZT", "DR", "LR",
   "ZD", "DM", "RH", "PH", "XZ", "CD", "MZ",
   "MD", "ZE", "VE", "KD", "TI", "DX", "CH",
   "AH", "CV", "AV", "SQ", "VS", "VL", "VG",
   "VT", "NP", "HC", "VC" };

Sigmet2Dsr::Sigmet2Dsr( Params *TDRP_params ){

  _beamData = NULL;

  //
  // Reset the volume number.
  //
  _volumeNum = 0;
  //
  // Point to the TDRP parameters.
  //
  _params = TDRP_params;


  //
  // Set up the message log.
  //
  _msgLog.setApplication( "Sigmet2Dsr" );

  //
  // Enable debug level messaging
  //
  if ( _params->debug == TRUE )
    _msgLog.enableMsg( DEBUG, true );
  else 
    _msgLog.enableMsg( DEBUG, false );
  
  //
  // Enable info level messaging
  //
  if ( _params->info == TRUE )
    _msgLog.enableMsg( INFO, true );
  else
    _msgLog.enableMsg( INFO, false );


  //
  //
  //
  if ( _msgLog.setOutputDir( _params->msgLog_dir ) != 0 ) {   
    cerr << "Failed to set up message log to directory ";
    cerr << _params->msgLog_dir;
    cerr << endl;
    exit(-1);
  }

  //
  // Initialize the radar queue.
  //
  if( _radarQueue.init( _params->output_fmq_url,
			"Sigmet2Dsr",
			 _params->debug,           
			 DsFmq::READ_WRITE, DsFmq::END,
			_params->output_fmq_compress,
			_params->output_fmq_nslots,
			_params->output_fmq_size, 1000,
			&_msgLog )) {
    cerr << "Could not initialize fmq " << _params->output_fmq_url << endl; 
    exit(-1);
  }
  if (_params->mode == Params::ARCHIVE) {
    _radarQueue.setBlockingWrite();
  }

  //
  // Set an int to the number of fields we expect, which
  // we take from the size of the lookup table.
  //
  _numExpectedFields = _params->scales_n;

  if ( _numExpectedFields > _maxFields){
    fprintf(stderr,"Too many fields desired - I cannot cope.\n");
    exit(-1);
  }

  //
  // Set up the field headers.
  //
  
  //
  // The byte width and the missing value are forced on us.
  //
  int missingVal = 0;
  int byteWidth = 1;
  
  for(int i=0; i <  _numExpectedFields; i++){
    _fieldParams[i] = new DsFieldParams(_params->_scales[i].fieldName,
					_params->_scales[i].units,
					_params->_scales[i].scale,
					_params->_scales[i].bias,
					byteWidth, missingVal);
    
  }

  // allocate beam data buffer

  _beamData = new unsigned char[_maxNumBins * _maxFields];

}

////////////////////////////////////////////////////
//
// Destructor
//

Sigmet2Dsr::~Sigmet2Dsr( ){

  if (_beamData) {
    delete[] _beamData;
  }
  _radarQueue.closeMsgQueue();
}


////////////////////////////////////////////////////
//
// Main routine - processes a file.
//

void  Sigmet2Dsr::Sigmet2DsrFile( char *filename )
{
  if (_params->fileQuiescentSecs > 0)
  {
    struct stat prev_file_stat;
    
    if (stat(filename, &prev_file_stat) != 0)
    {
      fprintf(stderr, "Error stating file: %s\n", filename);
      return;
    }
    
    while (true)
    {
      PMU_auto_register("Waiting for file quiescence");
      sleep(_params->fileQuiescentSecs);
      
      struct stat curr_file_stat;
      
      if (stat(filename, &curr_file_stat) != 0)
      {
	fprintf(stderr, "Error stating file: %s\n", filename);
	return;
      }
      
      if (prev_file_stat.st_size == curr_file_stat.st_size)
	break;
      
      prev_file_stat = curr_file_stat;
    }
    
  }
  
  //
  // If the input file is gzipped, apply gunzip with a system() call.
  //
  char *p = filename + strlen(filename) - 3;
  if (!(strcmp(p, ".gz"))){
    int maxTries = 5;
    int tries = 0;
    int retVal;
    do {
      char com[1024];
      sprintf(com,"gunzip -f %s", filename);
      if (_params->debug){
        fprintf(stderr,"Try number %d : applying command %s\n",
		tries+1, com);
      }
      retVal = system(com);
      if (_params->debug){
	fprintf(stderr,"Command returned %d on try %d\n",
		retVal, tries+1); 
      }
      if (retVal) sleep(1);
      tries ++;
    } while ((tries < maxTries) && (retVal != 0));
    *p = char(0); // truncate .gz from end of filename.
  }

  if (_params->debug){
    fprintf(stderr,"Processing file %s into DSR\n", filename);
  }


  Radar *radar;

  if (_params->debug >= Params::DEBUG_VERBOSE){
    RSL_radar_verbose_on(); 
  }

  radar = RSL_anyformat_to_radar( filename, NULL );

  if (radar == NULL){
    fprintf(stderr,"\n\nProblems with %s\n\n", filename);
    return;
  }
  //
  // OK - have the radar structure.
  //

  double lat = radar->h.latd + radar->h.latm/60.0 + radar->h.lats/3600.0;
  double lon = radar->h.lond + radar->h.lonm/60.0 + radar->h.lons/3600.0;


  //
  // Count the number of "true" volumes.
  //
  Volume **volume;
  Sweep  **sweep;
  Ray     *ray;

  int true_nvolumes = 0; 
  int maxsweeps = 0; 
  int nrays = 0;
  int lastNumBins = -1;
  int sweep_num = 0;
  int ray_num = 0;
  int nfield = 0;

  int nvolumes = radar->h.nvolumes;
  volume   = (Volume **) calloc(nvolumes, sizeof(Volume *));
  sweep    = (Sweep  **) calloc(nvolumes, sizeof(Sweep  *));
  int *nsweeps  = (int *)     calloc(nvolumes, sizeof(int));


  // Get the the number of sweeps in the radar structure.  This will be
  // the main controlling loop variable.

  for (int i=0; i<nvolumes; i++) {
    volume[i] = radar->v[i];
    if(volume[i]) {
      nsweeps[i] = volume[i]->h.nsweeps;
      if (nsweeps[i] > maxsweeps) maxsweeps = nsweeps[i];
      true_nvolumes++;
    }
  }

  if (_params->debug){
    //
    // Print some things from the header out.
    //
    

    fprintf(stderr,"Data time : %d/%02d/%02d %02d:%02d:%02d\n", 
	    radar->h.year,
	    radar->h.month,
	    radar->h.day,
	    radar->h.hour,
	    radar->h.minute,
	    (int)radar->h.sec );

    fprintf(stderr,"Radar type : %s\n",radar->h.radar_type);
    fprintf(stderr,"Name : %s\n",radar->h.name);
    fprintf(stderr,"City : %s\n",radar->h.city);
    fprintf(stderr,"State : %s\n",radar->h.state);
    fprintf(stderr,"Site name : %s\n",radar->h.name);
    
    fprintf(stderr,"Lat : %d|%d|%d (%g)\n", radar->h.latd, radar->h.latm,
	    radar->h.lats, lat);

    fprintf(stderr,"Lon : %d|%d|%d (%g)\n", radar->h.lond, radar->h.lonm,
	    radar->h.lons, lon);

    fprintf(stderr,"Number of fields found : %d\n",
	    true_nvolumes );
  }


  DsRadarParams& radarParams = _radarMsg.getRadarParams();
  DsRadarFlags& radarFlags = _radarMsg.getRadarFlags();

  radarParams.radarId = radar->h.number;
  radarParams.numFields = _numExpectedFields;
  radarParams.radarName = _params->radarName;
  radarParams.latitude = lat;
  radarParams.longitude = lon;
  radarParams.altitude = radar->h.height/1000.0;
  
  int content;
  //  LOOP for all sweeps (typically 11 or 16 for wsr88d data).

  for (int i=0; i<maxsweeps; i++) {
    PMU_auto_register("Processing sweep");
    
    //
    // Send the flags appropriate to this tilt.
    //  

    if (i == 0){
      //
      // The first tilt - this is not the end of the last tilt.
      // It is the start of the volume. Send both end and start
      // of volume flags.
      //
      radarFlags.startOfVolume = true;
      radarFlags.startOfTilt   = true;
      radarFlags.endOfTilt     = false;
      radarFlags.endOfVolume   = false;
      //
      if (_params->debug >= Params::DEBUG_VERBOSE){
	fprintf(stderr,"Start of volume.\n");
      }
      
    } else {
      //
      // Otherwise, we are between tilts.
      //
      radarFlags.startOfVolume = false;
      radarFlags.startOfTilt   = true;
      radarFlags.endOfTilt     = true;
      radarFlags.endOfVolume   = false;
      if (_params->debug >= Params::DEBUG_VERBOSE){
	fprintf(stderr,"End of tilt.\n");
	fprintf(stderr,"Start of tilt.\n");
      }
    }
    
    if (_params->sendStartEndFlags){
      if (_params->debug >= Params::DEBUG_VERBOSE){
	fprintf(stderr,"Sending start/end flags.\n");
      }
      content = DsRadarMsg::RADAR_FLAGS;
      if( _radarQueue.putDsMsg( _radarMsg, content ) != 0 ) {  
	fprintf(stderr," Failed to send radar flags.\n");
	exit(-1);
      }
    }
    
    //
    // Set the count to 0 at the start of a sweep so that a new header is
    // sent at the start of each tilt.
    //
    int sentCount = 0;

    // Get the array of volume and sweep pointers; one for each field type.

    nrays = 0;
    for (int k=0; k<nvolumes; k++) {
      if (volume[k]) sweep[k] = volume[k]->sweep[i];
      //  
      // Check if we really can access this sweep.  Paul discovered that
      // if the actual number of sweeps is less than the maximum that we
      // could be chasing a bad pointer (a NON-NULL garbage pointer).
      //
      if (i >= nsweeps[k]) sweep[k] = NULL;

      if (sweep[k]) if (sweep[k]->h.nrays > nrays) nrays = sweep[k]->h.nrays;
    }

    sweep_num++;  // I guess it will be ok to count NULL sweeps. 
    ray_num = 0;

    if (_params->debug >= Params::DEBUG_VERBOSE){
      
      fprintf(stderr,"Processing sweep %d for %d rays.", i, nrays);
      if (little_endian()) 
	fprintf(stderr," ... On Little endian.\n");
      else 
	fprintf(stderr,"\n");
    }

    // In some data sets (e.g Sigmet raw volumes) the rays in the sweep
    // are stored from North, rather than from the earliest time.
    // So search through the ray times, locating the start index for
    // processing the rays

    time_t prevTime = -1;
    int startIndex = 0;
    for (int j = 0; j < nrays; j++) {
      ray = NULL;
      for (int k=0; k<nvolumes; k++) {
	if (sweep[k])
	  if (j < sweep[k]->h.nrays)
	    if (sweep[k]->ray)
	      if ((ray = sweep[k]->ray[j])) break;
      }
      if (!ray) { // null ray
        continue;
      }
      int raysec = (int) rint(ray->h.sec);
      DateTime rayTime(ray->h.year, ray->h.month, ray->h.day,
                       ray->h.hour, ray->h.minute, raysec);
      if (prevTime >= 0) {
        if (rayTime.utime() < prevTime) {
          // jump back in time indicates the start of the sweep
          startIndex = j;
          break;
        }
      }
      prevTime = rayTime.utime();
    } // j
      
    //
    // Now LOOP for all rays within this particular sweep (i).
    //    Get all the field types together for the ray, see ray[k], and
    //    fill the data buffer appropriately.
    //
    memset(_beamData, 0, _maxNumBins * _maxFields);
    for (int jj=0; jj<nrays; jj++) {
      int j = (jj + startIndex) % nrays;
      nfield = 0;
      ray_num++;  // And counting, possibly, NULL rays.
      //
      // Find any ray for header information.
      //
      ray = NULL;
      for (int k=0; k<nvolumes; k++) {
	if (sweep[k])
	  if (j < sweep[k]->h.nrays)
	    if (sweep[k]->ray)
	      if ((ray = sweep[k]->ray[j])) break;
      }

      // If there is no such ray, then continue on to the next ray.
      if (ray) {
	//
	if (_params->debug >= Params::DEBUG_VERBOSE){
      
	  fprintf(stderr,
		  "Ray: %.4d, Time: %2.2d:%2.2d:%f  %.2d/%.2d/%.4d Az: %f Elev: %f\n", 
		  ray_num, ray->h.hour, ray->h.minute, ray->h.sec, 
		  ray->h.month, ray->h.day, ray->h.year,
		  ray->h.azimuth, ray->h.elev);
	}

	// Here is where we loop on each field type. 
	// Field types expected are: Reflectivity,
	// Velocity, and Spectrum width; this is a typicial list but it
	// is not restricted to it.
	//

	//
	// Allocate space for the beam data if the number of bins has changed.
	//
	int numBins = ray->h.nbins;
	
	if (lastNumBins != numBins){
	
	  if (numBins > _maxNumBins){
	    fprintf(stderr,"Too many bins...%d\n",numBins);
	    exit(-1);
	  }
	  
	  lastNumBins = numBins;
	}
	
	int fieldNum = 0;
	
	for (int k=0; k<nvolumes; k++) {
	  if (sweep[k]){

	    if (sweep[k]->ray){
	      ray = sweep[k]->ray[j];
	    } else {
	      ray = NULL;
	    }
	  } else {
	    ray = NULL;
	  }

	  if (k >= n_field_types) {
	    ray = NULL;
	    fprintf(stderr,"ERROR - unknown field type encountered.\n");
	    fprintf(stderr,"  The field type index exceeds the number of known field types.\n");
            fprintf(stderr, " field index: %d\n", k);
            fprintf(stderr, " max index: %d\n", n_field_types - 1);
	  }

	  //
	  // Get the scale and bias based on the field name.
	  // If we can't do this, set ray to NULL.
	  //
	  double scale, bias;
	  char units[16];
	  if (ray != NULL){


	    if (_getScale(output_field_name[k], &scale, &bias, units)){
	      if (_params->debug >= Params::DEBUG_DATA){
		fprintf(stderr,
			"No scale or bias information specified for field %s - skipping ...\n",
			output_field_name[k]);
	      }
	      ray = NULL;
	    } else {
	      if (_params->debug >= Params::DEBUG_DATA){
		fprintf(stderr,"Field %s : scale=%g bias=%g units %s\n", 
			output_field_name[k], scale, bias, units);
	      }
	    }
	  }

	  if (ray != NULL) {
		
	    float dataVal;
	    unsigned char dataByte;

	    int len_data = ray->h.nbins;



	    for (int m=0; m<len_data; m++) {
	      float x = ray->h.f(ray->range[m]);
	      if (x == BADVAL || x == RFVAL || x == APFLAG || x == NOECHO){
		dataVal = (signed short) -999.0; // The bad value.
		dataByte = 0;
	      } else {
		dataVal = x;
		double d = rint((dataVal - bias)/scale);
		if (d < 0) d = 0;
		if (d > 255.0) d = 255.0;
		dataByte = (unsigned char) d;
	      }
	      _beamData[_numExpectedFields*m + fieldNum] = dataByte;

	      if (_params->debug >= Params::DEBUG_DATA){
		fprintf(stderr,"%g\t", dataVal);
	      }
	    }

	    if (_params->debug >= Params::DEBUG_DATA){
	      fprintf(stderr,"\n");
	    }

	    //
	    // See if it is time to send another radarParams and fieldParams header.
	    //
	    
	    if ((sentCount == 0) && (fieldNum == 0)){

	      if (_params->debug >= Params::DEBUG_VERBOSE){
		fprintf(stderr,"Sending headers...\n");
	      }
	      int msec = int (1000.0*ray->h.sec - 1000.0*floor(ray->h.sec));

	      if (_params->debug){
		fprintf(stderr,"  Sweep %d Ray %d at %d/%02d/%02d %02d:%02d:%02d:%03d az %g ",
			i, ray->h.ray_num, ray->h.year, ray->h.month, ray->h.day,
			ray->h.hour, ray->h.minute, (int)floor(ray->h.sec), msec,
			ray->h.azimuth );
		fprintf(stderr,"nbins %d\n",ray->h.nbins);
	      }

	      //
	      // Fill in the remaining radar parameters.
	      //
	      radarParams.numGates = ray->h.nbins;
	  
	      double degPerSec = (360.0 * ray->h.sweep_rate) / 60.0;
	      double timePerBeam = ray->h.beam_width / degPerSec;
	      radarParams.samplesPerBeam = (int) (ray->h.prf * timePerBeam);

	      radarParams.gateSpacing = (double) ray->h.gate_size / 1000.0; // Meters to Km.
	      radarParams.startRange = (double) ray->h.range_bin1 / 1000.0; // Meters to Km.
	      radarParams.horizBeamWidth = (double) ray->h.beam_width;
	      radarParams.vertBeamWidth = (double)  ray->h.beam_width;
	      radarParams.pulseWidth = (double) ray->h.pulse_width;
	      radarParams.pulseRepFreq = (double) ray->h.prf;
	      radarParams.wavelength = (double) ray->h.wavelength * 100.0; // Meters to cm.

	      //
	      // Send the radar and field params.
	      //
	      if (_params->debug >= Params::DEBUG_VERBOSE)
		fprintf(stderr, "Writing radar params to FMQ\n");
	      
	      content =  DsRadarMsg::RADAR_PARAMS;
	      if( _radarQueue.putDsMsg(_radarMsg, content ) != 0 ) {  
		fprintf(stderr," Failed to send radar flags.\n");
		exit(-1);
	      }
	
	      vector< DsFieldParams* >& fieldParams = _radarMsg.getFieldParams();
	      fieldParams.clear();

	      for(int ifld=0; ifld <  _numExpectedFields; ifld++){
		fieldParams.push_back( _fieldParams[ifld] );
	      }

	      if (_params->debug >= Params::DEBUG_VERBOSE)
	      {
		fprintf(stderr, "Writing field params to FMQ:\n");
		for (size_t i = 0; i < fieldParams.size(); ++i)
		  fprintf(stderr, "    Field name <%s>\n",
			  fieldParams[i]->name.c_str());
	      }

	      content =  DsRadarMsg::FIELD_PARAMS;
	      
	      if( _radarQueue.putDsMsg(_radarMsg, content ) != 0 ) {  
		fprintf(stderr," Failed to send radar flags.\n");
		exit(-1);
	      }
	    }

	    fieldNum = (fieldNum +1) % _numExpectedFields;
	    
	    //
	    // Regardless of if we have sent headers, send
	    // the beam.
	    //

	    if (fieldNum == _numExpectedFields-1){
	      date_time_t beamTime;
	      beamTime.year = ray->h.year;    beamTime.month = ray->h.month;
	      beamTime.day = ray->h.day;      beamTime.hour  = ray->h.hour;
	      beamTime.min = ray->h.minute;   beamTime.sec  = (int)rint(ray->h.sec);
	      uconvert_to_utime( &beamTime );
	      
	      beamTime.unix_time = beamTime.unix_time + _params->time_offset;
	      uconvert_from_utime( &beamTime );

	      DsRadarBeam& radarBeam = _radarMsg.getRadarBeam();
	      
	      radarBeam.dataTime   = beamTime.unix_time;
	      radarBeam.volumeNum  = _volumeNum;
	      radarBeam.tiltNum    = i;
	      radarBeam.azimuth    = ray->h.azimuth;
	      radarBeam.elevation  = ray->h.elev;
	      radarBeam.targetElev = radarBeam.elevation; // Control system for antenna not known.
	      

	      //
	      // Mask the beam, if requested.
	      //
	      if (_params->maskOnField){
		_maskBeam(_beamData, numBins);
	      }
	      //
	      // Send out this beam.
	      //
	      
	      radarBeam.loadData( _beamData, _numExpectedFields*numBins ); 
	   
	      content = DsRadarMsg::RADAR_BEAM;

	      if (_params->debug >= Params::DEBUG_VERBOSE)
		fprintf(stderr, "Writing beam to radar FMQ. Az = %f, Elev = %f, Num fields = %d\n",
			radarBeam.azimuth, radarBeam.elevation, _numExpectedFields);
	      
	      if( _radarQueue.putDsMsg(_radarMsg, content ) != 0 ) {  
		fprintf(stderr," Failed to send radar flags.\n");
		exit(-1);
	      }
	    }
	  }
	}
	  
	sentCount = (sentCount + 1) % _params->beamsPerMessage;
  
      } // if (ray)
    } 

    if (_params->mode != Params::REALTIME){
      for (int i=0; i < _params->sleepBetweenTilts; i++){
	PMU_auto_register("Sleeping between tilts");
	sleep(1);
      }
    }
  }


  if (_params->mode != Params::REALTIME){
    for (int i=0; i < _params->sleepBetweenVolumes; i++){
      PMU_auto_register("Sleeping between volumes");
      sleep(1);
    }
  }

  _volumeNum = (_volumeNum + 1) % 100; // Arbitrary wrap point.

  //
  // Send the final end-of-volume flag.
  //
  if (_params->sendStartEndFlags){
    radarFlags.startOfVolume = false;
    radarFlags.startOfTilt   = false;
    radarFlags.endOfTilt     = true;
    radarFlags.endOfVolume   = true;
    
    content = DsRadarMsg::RADAR_FLAGS;
    if( _radarQueue.putDsMsg(_radarMsg, content ) != 0 ) {  
      fprintf(stderr," Failed to send end of volume flag.\n");
      return;
    }
    
    if (_params->debug >= Params::DEBUG_VERBOSE){
      fprintf(stderr,"End of volume flag sent.\n");
    }
  }

  if (_params->mode != Params::REALTIME){
    for (int i=0; i < _params->sleepBetweenVolumes; i++){
      PMU_auto_register("Sleeping between volumes");
      sleep(1);
    }
  }

  RSL_free_radar(radar);
  free(volume); free(sweep); free(nsweeps);

  return;


}


int Sigmet2Dsr::_getScale(const char *key,
                          double *scale, double *bias, char *units){


  for (int i=0; i < _params->scales_n; i++){
    if (!(strcmp(_params->_scales[i].fieldName, key))){
      *scale = _params->_scales[i].scale;
      *bias = _params->_scales[i].bias;
      sprintf(units,"%s", _params->_scales[i].units);
      return 0; // Got it.
    }
  }

  return 1; // Didn't get it.

}
//
// Mask other beams based on one beam's data.
//
void Sigmet2Dsr::_maskBeam(unsigned char *_beamData, int numBins){

  for (int ifld=0; ifld < _numExpectedFields; ifld++){

    if (ifld == _params->maskOnFieldNum) continue;

    for (int ig=0; ig < numBins; ig++){
      //
      // If the refrence field is missing, set the other fields to missing too.
      //
      if (_beamData[_numExpectedFields*ig + _params->maskOnFieldNum] == 0){
	_beamData[_numExpectedFields*ig + ifld] = 0;
      }
    }

  }

  return;

}
