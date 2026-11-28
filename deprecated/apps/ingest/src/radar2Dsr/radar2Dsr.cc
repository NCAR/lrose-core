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
#include <rapformats/DsRadarParams.hh>
#include <rapformats/DsFieldParams.hh>
#include <rapformats/DsRadarElev.hh>
#include <rapformats/DsRadarBeam.hh>

#include <trmm_rsl/rsl.h>

#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <cstdlib> 

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


#include "radar2Dsr.hh"
using namespace std;

const double radar2Dsr::_scale = 1.0;
const double radar2Dsr::_bias = 0.0;


radar2Dsr::radar2Dsr( Params *TDRP_params ){

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
  _msgLog.setApplication( "radar2Dsr" );

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
			"radar2Dsr",
			 _params->debug,           
			 DsFmq::READ_WRITE, DsFmq::END,
			_params->output_fmq_compress,
			_params->output_fmq_nslots,
			_params->output_fmq_size, 1000,
			&_msgLog )) {
    cerr << "Could not initialize fmq " << _params->output_fmq_url << endl; 
    exit(-1);
  }
  if ((_params->mode == Params::ARCHIVE) || (_params->realtimeBlockingFmq)) {
    _radarQueue.setBlockingWrite();
  }


  _fieldParams[0] = new DsFieldParams("DBZ", "dBZ", _scale, _bias, sizeof(fl32), _badVal);
  _fieldParams[1] = new DsFieldParams("VEL", "m/s", _scale, _bias, sizeof(fl32), _badVal);
  _fieldParams[2] = new DsFieldParams("SW",  "m/s", _scale, _bias, sizeof(fl32), _badVal);
 
  _useCurrentTime = false;

  return;

}

////////////////////////////////////////////////////
//
// Destructor - does nothing.
//

radar2Dsr::~radar2Dsr( ){
  return;
}


////////////////////////////////////////////////////
//
// Main routine - processes a file.
//

void  radar2Dsr::radar2DsrFile( char *filename )
{
  //
  // If the input file is gzipped, apply gunzip with a system() call.
  //
  char *p = filename + strlen(filename) - 3;
  if (!(strcmp(p, ".gz"))){
    //
    // The file is gzipped, sleep if requested. This
    // allows the input file writing to complete in some
    // cases. This has proven to be a problem, so there
    // are some debugging prints around this.
    //
    for(int i=0; i < _params->sleepBeforeUnzip; i++){
      PMU_auto_register("Sleeping before unzip.");
      sleep(1);
    }

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

  if (_params->debug){
    RSL_radar_verbose_on(); 
  }

  radar = RSL_anyformat_to_radar( filename, NULL );

  if (radar == NULL){
    fprintf(stderr,"\n\nProblems with %s\n\n", filename);
    return;
  }

  //
  // See if we should be using wallclock time.
  //
  if ( _params->useWallclockTime.useCurrentTime ){
    if ( _params->useWallclockTime.minFileAge < 0){
      //
      // We don't care how old file is, just use current time
      //
      _useCurrentTime = true;
    } else {
      //
      // It depends how old the file is - do a stat to see
      //
      struct stat buf;
      if (stat(filename, &buf)){
	fprintf(stderr,"\n\nCould not stat %s\n\n", filename);
	return;
      }
      time_t fileAge = time(NULL) - buf.st_ctime;

      if (fileAge > _params->useWallclockTime.minFileAge) // It's old enough
	_useCurrentTime = true;
    }
  }

  //
  // OK - have the radar structure.
  //

  double lat = radar->h.latd + radar->h.latm/60.0 + radar->h.lats/3600.0;
  double lon = radar->h.lond + radar->h.lonm/60.0 + radar->h.lons/3600.0;


  if (_params->debug){
    //
    // This is somewhat long winded but worth it.
    // It helps if we get a file where the indices are set oddly.
    // Print which fields are present.
    //
    if (radar->v[DZ_INDEX] != NULL) cerr << "DZ field is present." << endl;
    if (radar->v[VR_INDEX] != NULL) cerr << "VR field is present." << endl;
    if (radar->v[SW_INDEX] != NULL) cerr << "SW field is present." << endl;
    if (radar->v[CZ_INDEX] != NULL) cerr << "CZ field is present." << endl;
    if (radar->v[ZT_INDEX] != NULL) cerr << "ZT field is present." << endl;
    if (radar->v[DR_INDEX] != NULL) cerr << "DR field is present." << endl;
    if (radar->v[LR_INDEX] != NULL) cerr << "LR field is present." << endl;
    if (radar->v[ZD_INDEX] != NULL) cerr << "ZD field is present." << endl;
    if (radar->v[DM_INDEX] != NULL) cerr << "DM field is present." << endl;
    if (radar->v[RH_INDEX] != NULL) cerr << "RH field is present." << endl;
    if (radar->v[PH_INDEX] != NULL) cerr << "PH field is present." << endl;
    if (radar->v[XZ_INDEX] != NULL) cerr << "XZ field is present." << endl;
    if (radar->v[CD_INDEX] != NULL) cerr << "CD field is present." << endl;
    if (radar->v[MZ_INDEX] != NULL) cerr << "MZ field is present." << endl;
    if (radar->v[MD_INDEX] != NULL) cerr << "MD field is present." << endl;
    if (radar->v[ZE_INDEX] != NULL) cerr << "ZE field is present." << endl;
    if (radar->v[VE_INDEX] != NULL) cerr << "VE field is present." << endl;
    if (radar->v[KD_INDEX] != NULL) cerr << "KD field is present." << endl;
    if (radar->v[TI_INDEX] != NULL) cerr << "TI field is present." << endl;
    if (radar->v[DX_INDEX] != NULL) cerr << "DX field is present." << endl;
    if (radar->v[CH_INDEX] != NULL) cerr << "CH field is present." << endl;
    if (radar->v[AH_INDEX] != NULL) cerr << "AH field is present." << endl;
    if (radar->v[CV_INDEX] != NULL) cerr << "CV field is present." << endl;
    if (radar->v[AV_INDEX] != NULL) cerr << "AV field is present." << endl;
  }


  Volume *dbzVolume;
  //
  // Get the DBZ field we want.
  //
  switch (_params->dbzChoice){

  case Params::INSIST_ON_DZ :
    dbzVolume = radar->v[DZ_INDEX];
    break;

  case Params::INSIST_ON_CZ :
    dbzVolume = radar->v[CZ_INDEX];
    break;

  case Params::PREFER_CZ :
    dbzVolume = radar->v[CZ_INDEX];
    if (dbzVolume == NULL){
      dbzVolume = radar->v[DZ_INDEX];
    }
	break;

  case Params::PREFER_DZ :
    dbzVolume = radar->v[DZ_INDEX];
    if (dbzVolume == NULL){
      dbzVolume = radar->v[CZ_INDEX];
    }
    break;

 case Params::USE_ZT :
    dbzVolume = radar->v[ZT_INDEX];
    break;

  default :
    cerr << "Failed to interpret DZ field code " << _params->dbzChoice << endl;
    exit(-1);
    break;

  }

  Volume *velVolume = radar->v[VR_INDEX];
  Volume *swVolume  = radar->v[SW_INDEX];
  
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

    fprintf(stderr,"Radar type : %8s\n",radar->h.radar_type);
    fprintf(stderr,"Name : %s\n",radar->h.name);
    fprintf(stderr,"City : %s\n",radar->h.city);
    fprintf(stderr,"State : %s\n",radar->h.state);
    fprintf(stderr,"Site name : %s\n",radar->h.name);
    
    fprintf(stderr,"Lat : %d|%d|%d (%g)\n", radar->h.latd, radar->h.latm,
	    radar->h.lats, lat);

    fprintf(stderr,"Lon : %d|%d|%d (%g)\n", radar->h.lond, radar->h.lonm,
	    radar->h.lons, lon);

  }


  DsRadarParams& radarParams = _radarMsg.getRadarParams();
  DsRadarFlags& radarFlags = _radarMsg.getRadarFlags();

  radarParams.radarId = radar->h.number;
  radarParams.numFields = _nFields;

  char radarName[9];
  for (int ik=0; ik < 8; ik++){
    radarName[ik]=radar->h.radar_name[ik]; radarName[ik+1]=(char)0;
  }

  radarParams.radarName = radarName;
  radarParams.latitude = lat;
  radarParams.longitude = lon;
  radarParams.altitude = radar->h.height/1000.0;
  
  int content;

  // get the number of sweeps. It may be that not all volumes
  // are present, so check them all, making sure that if they
  // are present they are consistent.

  int nSweeps = 0;

  if (dbzVolume != NULL) nSweeps = dbzVolume->h.nsweeps;

  if (velVolume != NULL) {
    if (nSweeps != 0){
      if (nSweeps != velVolume->h.nsweeps){
	fprintf(stderr, "ERROR - vel/dbz nsweep mismatch\n");
	return;
      }
    } else {
      nSweeps = velVolume->h.nsweeps;
    }
  }

  if (swVolume != NULL) {
    if (nSweeps != 0){
      if (nSweeps != swVolume->h.nsweeps){
	fprintf(stderr, "ERROR - SW nsweep mismatch\n");
	return;
      }
    } else {
      nSweeps = swVolume->h.nsweeps;
    }
  }


  for (int isweep=0; isweep < nSweeps; isweep++) {

    PMU_auto_register("Processing sweep");


    Sweep *dbzSweep = NULL;
    if (dbzVolume != NULL) dbzSweep = dbzVolume->sweep[isweep];

    Sweep *velSweep = NULL;
    if (velVolume != NULL) velSweep = velVolume->sweep[isweep];

    Sweep *swSweep = NULL;
    if (swVolume != NULL) swSweep = swVolume->sweep[isweep];

    //
    // Get the elevation.
    //
    double elevation = _badVal;

    if (dbzSweep != NULL) elevation = dbzSweep->h.elev;

    if ((elevation == _badVal) &&
	(velSweep != NULL)) elevation = dbzSweep->h.elev;

    if ((elevation == _badVal) &&
	(swSweep != NULL)) elevation = swSweep->h.elev;

    if (elevation == _badVal) continue;

    //
    // Get the number of rays in this sweep. Similar
    // to what we did before to get the number of
    // sweeps in the volumes.

    int nRays = 0;

    if (dbzSweep != NULL) nRays = dbzSweep->h.nrays;

    if (velSweep != NULL) {
      if (nRays != 0){
	if (nRays != velSweep->h.nrays){
	  fprintf(stderr, "ERROR - vel/dbz nray mismatch\n");
	  return;
	}
      } else {
	nRays = velSweep->h.nrays;
      }
    }

    if (swSweep != NULL) {
      if (nRays != 0){
	if (nRays != swSweep->h.nrays){
	  fprintf(stderr, "ERROR - SW nray mismatch\n");
	  return;
	}
      } else {
	nRays = swSweep->h.nrays;
      }
    }

    //
    // Send the flags appropriate to this tilt.
    //  
    if (isweep == 0){
      //
      // The first tilt - this is not the end of the last tilt.
      // It is the start of the volume.
      //
      radarFlags.startOfVolume = true;
      radarFlags.startOfTilt   = true;
      radarFlags.endOfTilt     = false;
      radarFlags.endOfVolume   = false;
      //
      if (_params->debug){
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
      if (_params->debug){
	fprintf(stderr,"End of tilt.\n");
	fprintf(stderr,"Start of tilt.\n");
      }
    }
    
    if (_params->sendStartEndFlags){
      if (_params->debug){
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

    //
    // Loop through the rays.
    //
    for (int iray=0; iray < nRays; iray++){

      Ray *dbzRay = NULL;
      if (dbzSweep != NULL) dbzRay = dbzSweep->ray[iray];

      Ray *velRay = NULL;
      if (velSweep != NULL) velRay = velSweep->ray[iray];

      Ray *swRay = NULL;
      if (swSweep != NULL) swRay = swSweep->ray[iray];

      //
      // Get the number of bins, gate spacing etc. by considering all
      // fields present.
      //

      int nBins = 0;

      if (dbzRay != NULL) nBins = dbzRay->h.nbins;

      if (velRay != NULL) {
	if (nBins != 0){
	  if (nBins != velRay->h.nbins){
	    fprintf(stderr, "ERROR - vel/dbz nbin mismatch\n");
	    return;
	  }
	} else {
	  nBins = velRay->h.nbins;
	}
      }

      if (swRay != NULL) {
	if (nBins != 0){
	  if (nBins != swRay->h.nbins){
	    fprintf(stderr, "ERROR - SW bin mismatch\n");
	    return;
	  }
	} else {
	  nBins = swRay->h.nbins;
	}
      }

      //
      // Get some ray parameters. Try velocity first just because
      // I trust the nyquist velocity more if it come from there.
      //
      double gateSpacing = _badVal;
      double gateStart = _badVal;
      double azimuth = _badVal;
      double hBeamWidth = _badVal;
      double vBeamWidth = _badVal;
      double pw = _badVal;
      double prf = _badVal;
      double wavelen = _badVal;
      double nyquistVel = _badVal;
      int samplesPerBeam = _badVal;
      date_time_t beamTime;

      if (velRay != NULL){
	gateSpacing = velRay->h.gate_size/1000.0; // Meters to Km.
	gateStart = velRay->h.range_bin1/1000.0;  // Meters to Km.
	azimuth = velRay->h.azimuth;
	hBeamWidth = velRay->h.beam_width;
	vBeamWidth = velRay->h.beam_width;
	pw = velRay->h.pulse_width;
	prf = velRay->h.prf;
	wavelen = velRay->h.wavelength * 100.0; // Meters to cm.
	nyquistVel = velRay->h.nyq_vel;
	double degPerSec = (360.0 * velRay->h.sweep_rate) / 60.0;
	double timePerBeam = velRay->h.beam_width / degPerSec;
	samplesPerBeam = (int) (velRay->h.prf * timePerBeam);
	beamTime.year = velRay->h.year;
	beamTime.month = velRay->h.month;
	beamTime.day = velRay->h.day;
	beamTime.hour = velRay->h.hour;
	beamTime.min = velRay->h.minute;
	beamTime.sec = (int)rint( velRay->h.sec );
      }

	  
      if ((gateSpacing == _badVal) && (dbzRay != NULL)){
	gateSpacing = dbzRay->h.gate_size/1000.0; // Meters to Km.
	gateStart = dbzRay->h.range_bin1/1000.0;  // Meters to Km.
	azimuth = dbzRay->h.azimuth;
	hBeamWidth = dbzRay->h.beam_width;
	vBeamWidth = dbzRay->h.beam_width;
	pw = dbzRay->h.pulse_width;
	prf = dbzRay->h.prf;
	wavelen = dbzRay->h.wavelength * 100.0; // Meters to cm.
	nyquistVel = dbzRay->h.nyq_vel;
	double degPerSec = (360.0 * dbzRay->h.sweep_rate) / 60.0;
	double timePerBeam = dbzRay->h.beam_width / degPerSec;
	samplesPerBeam = (int) (dbzRay->h.prf * timePerBeam);
	beamTime.year = dbzRay->h.year;
	beamTime.month = dbzRay->h.month;
	beamTime.day = dbzRay->h.day;
	beamTime.hour = dbzRay->h.hour;
	beamTime.min = dbzRay->h.minute;
	beamTime.sec = (int)rint( dbzRay->h.sec );
      }

	  
      if ((gateSpacing == _badVal) && (swRay != NULL)){
	gateSpacing = swRay->h.gate_size/1000.0; // Meters to Km.
	gateStart = swRay->h.range_bin1/1000.0;  // Meters to Km.
	azimuth = swRay->h.azimuth;
	hBeamWidth = swRay->h.beam_width;
	vBeamWidth = swRay->h.beam_width;
	pw = swRay->h.pulse_width;
	prf = swRay->h.prf;
	wavelen = swRay->h.wavelength * 100.0; // Meters to cm.
	nyquistVel = swRay->h.nyq_vel;
	double degPerSec = (360.0 * swRay->h.sweep_rate) / 60.0;
	double timePerBeam = swRay->h.beam_width / degPerSec;
	samplesPerBeam = (int) (swRay->h.prf * timePerBeam);
	beamTime.year = swRay->h.year;
	beamTime.month = swRay->h.month;
	beamTime.day = swRay->h.day;
	beamTime.hour = swRay->h.hour;
	beamTime.min = swRay->h.minute;
	beamTime.sec = (int)rint( swRay->h.sec );
      }

      if (gateSpacing == _badVal) continue;
      uconvert_to_utime( &beamTime );


      //      cerr << "gateSpacing, gateStart, elevation, nBins : " << gateSpacing << ", " << gateStart << ", " << elevation << ", " << nBins << endl;



      // Loop through the bins, assemble beam, send.

      fl32 *beamData = (fl32 *)malloc(nBins*_nFields*sizeof(fl32));
      if (beamData == NULL){
	fprintf(stderr, "Malloc failed!\n");
	exit(-1);
      }

      for (int ibin=0; ibin < nBins; ibin++){

	double dbzVal = _badVal;
	if ((dbzRay != NULL)) dbzVal = dbzRay->h.f(dbzRay->range[ibin]);
	if (dbzVal == BADVAL || dbzVal == RFVAL || dbzVal == APFLAG || dbzVal == NOECHO){
	  dbzVal = _badVal;
	}

	double velVal = _badVal;
	if ((velRay != NULL)) velVal = velRay->h.f(velRay->range[ibin]);
	if (velVal == BADVAL || velVal == RFVAL || velVal == APFLAG || velVal == NOECHO){
	  velVal = _badVal;
	}

	double swVal = _badVal;
	if ((swRay != NULL)) swVal = swRay->h.f(swRay->range[ibin]);
	if (swVal == BADVAL || swVal == RFVAL || swVal == APFLAG || swVal == NOECHO){
	  swVal = _badVal;
	}

	beamData[_nFields*ibin] = dbzVal;
	beamData[_nFields*ibin+1] = velVal;
	beamData[_nFields*ibin+2] = swVal;

      } // End of loop through bins

      if (sentCount == 0){

	radarParams.numGates = nBins;
	radarParams.samplesPerBeam = samplesPerBeam;
	
	radarParams.gateSpacing = gateSpacing;
	radarParams.startRange = gateStart;
	radarParams.horizBeamWidth = hBeamWidth;
	radarParams.vertBeamWidth = vBeamWidth;
	radarParams.pulseWidth = pw;
	radarParams.pulseRepFreq = prf;
	radarParams.wavelength = wavelen;

	if (_params->remapBeams.remapBeamGeometry){
	  //
	  // In this case overwrite what we send at the last nanosecond.
	  //
	  radarParams.gateSpacing = _params->remapBeams.gateSpacingKm;
	  radarParams.startRange =  _params->remapBeams.distFirstGateKm;
	  radarParams.numGates = _params->remapBeams.nGates;
	}

	
	//
	// Send the radar and field params.
	//
	content =  DsRadarMsg::RADAR_PARAMS;
	if( _radarQueue.putDsMsg(_radarMsg, content ) != 0 ) {  
	  fprintf(stderr," Failed to send radar flags.\n");
	  exit(-1);
	}
	
	vector< DsFieldParams* >& fieldParams = _radarMsg.getFieldParams();
	fieldParams.clear();
	
	for(int ifld=0; ifld < _nFields; ifld++){
	  fieldParams.push_back( _fieldParams[ifld] );
	}

	content =  DsRadarMsg::FIELD_PARAMS;
	if( _radarQueue.putDsMsg(_radarMsg, content ) != 0 ) {  
	  fprintf(stderr," Failed to send radar flags.\n");
	  exit(-1);
	}
      }

      // Send the beam.

      DsRadarBeam& radarBeam = _radarMsg.getRadarBeam();
	      
      radarBeam.dataTime   = beamTime.unix_time + _params->time_offset;
      radarBeam.volumeNum  = _volumeNum;
      radarBeam.tiltNum    = isweep;
      radarBeam.azimuth    = azimuth;
      radarBeam.elevation  = elevation;
      radarBeam.targetElev = radarBeam.elevation; // Control system for antenna not known.


      if (_params->remapBeams.remapBeamGeometry){
	//
	// In this case change what we send at the last nanosecond.
	//
	fl32 *remappedBeam = (fl32 *)malloc(sizeof(fl32)*_nFields*_params->remapBeams.nGates);
	if (remappedBeam == NULL){
	  cerr << "Malloc of remapped beam failed." << endl;
	  exit(-1);
	}

	//
	// Set all values in the remapped beam to missing.
	//       
	for (int ig=0; ig < _params->remapBeams.nGates; ig++){
	  for (int ifld = 0; ifld < _nFields; ifld++){
	    remappedBeam[ig*_nFields + ifld] = _badVal;
	  }
	}

	//
	// Fill up the remapped beam with values where we can.
	//
	for (int ig=0; ig < _params->remapBeams.nGates; ig++){
	  double distKm = _params->remapBeams.distFirstGateKm + ig*_params->remapBeams.gateSpacingKm;
	  int oldIndex = (int)rint((distKm-gateStart)/gateSpacing);
	  if ((oldIndex > -1) && (oldIndex < nBins)){
	    for (int ifld = 0; ifld < _nFields; ifld++){
	      remappedBeam[ig*_nFields + ifld] = beamData[oldIndex*_nFields + ifld];
	    }
	  }
	}

	if (_useCurrentTime)
	  radarBeam.dataTime = time(NULL);

	radarBeam.loadData( remappedBeam, _nFields*_params->remapBeams.nGates*sizeof(fl32), sizeof(fl32) ); 

	free(remappedBeam);

      } else {
	//
	// Not doing remapping, just send the original data.
	//
	if (_useCurrentTime)
	  radarBeam.dataTime = time(NULL);

	radarBeam.loadData( beamData, _nFields*nBins*sizeof(fl32), sizeof(fl32) ); 
      }

      content = DsRadarMsg::RADAR_BEAM;  
      if( _radarQueue.putDsMsg(_radarMsg, content ) != 0 ) {  
	fprintf(stderr," Failed to send radar flags.\n");
	exit(-1);
      }

      free(beamData);

      if (_params->delayBeam.delayPostBeam){
	umsleep(_params->delayBeam.msDelayPostBeam);
      }

      sentCount = (sentCount + 1) % _params->beamsPerMessage;

    } // End of loop through rays

    //
    // Send an end-of-tilt flag.
    //
    if (_params->sendStartEndFlags){
      radarFlags.startOfVolume = false;
      radarFlags.startOfTilt   = false;
      radarFlags.endOfTilt     = true;
      radarFlags.endOfVolume   = false;
    
      content = DsRadarMsg::RADAR_FLAGS;
      if( _radarQueue.putDsMsg(_radarMsg, content ) != 0 ) {  
	fprintf(stderr," Failed to send end of volume flag.\n");
	return;
      }
      
      if (_params->debug){
	fprintf(stderr,"End of volume flag sent.\n");
      }
    }

  } // End of loop through sweeps
      
  _volumeNum = (_volumeNum+1) % 100; // Arbitrary
      
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
    
    if (_params->debug){
      fprintf(stderr,"End of volume flag sent.\n");
    }
  }

  for (int i=0; i < _params->sleepBetweenVolumes; i++){
    PMU_auto_register("Sleeping between volumes");
    sleep(1);
  }

  RSL_free_radar(radar);

  return;

}

