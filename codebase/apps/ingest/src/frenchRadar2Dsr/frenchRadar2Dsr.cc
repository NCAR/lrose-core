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

#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <cstdlib> 

#include "frenchRadar2Dsr.hh"
using namespace std;


frenchRadar2Dsr::frenchRadar2Dsr( Params *TDRP_params ){

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
  _msgLog.setApplication( "frenchRadar2Dsr" );

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
			"frenchRadar2Dsr",
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


  _fieldParams[0] = new DsFieldParams("DBZ", "dBZ", _scale, _bias, sizeof(fl32), _badVal);

 
  return;

}

////////////////////////////////////////////////////
//
// Destructor - does nothing.
//

frenchRadar2Dsr::~frenchRadar2Dsr( ){
  return;
}


////////////////////////////////////////////////////
//
// Main routine - processes a file.
//

void  frenchRadar2Dsr::frenchRadar2DsrFile( char *filename )
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


  FILE *fp = fopen(filename, "r");
  if (fp == NULL){
    cerr << filename << " not found." << endl;
    return;
  }

  DsRadarParams& radarParams = _radarMsg.getRadarParams();
  DsRadarFlags& radarFlags = _radarMsg.getRadarFlags();

  radarParams.radarId = _params->radarDef.radarID;
  radarParams.numFields = _nFields;

  radarParams.radarName = _params->radarDef.radarName;
  radarParams.latitude = _params->radarDef.latDeg;
  radarParams.longitude = _params->radarDef.lonDeg;
  radarParams.altitude = _params->radarDef.altKm;
  
  int content;

  double elevation = _params->radarDef.elevationDeg;

  //
  // Set some ray parameters.
  //
  double gateSpacing = _params->radarDef.gateSpacingKm;
  double gateStart = _params->radarDef.gateStartKm;
  double hBeamWidth = _params->radarDef.beamWidth;
  double vBeamWidth = _params->radarDef.beamWidth;

  date_time_t beamTime;
  memset(&beamTime, 0, sizeof(beamTime));
  
  double refAz;
  int sentCount = 0;
  char Line[4096];

  while (NULL != fgets(Line, 4096, fp)){

    if (strlen(Line) < 3) continue;

    // Is this a line with the UTC time? If so, get the time and ref azimuth and move on.
    if (0==strncmp("WRDR", Line, 4)){
      if (6 == sscanf(Line+5,"%2d%2d%2d%2d%2d%2d",
		      &beamTime.year, &beamTime.month, &beamTime.day,
		      &beamTime.hour, &beamTime.min, &beamTime.sec)){
	
	beamTime.year += 2000;
	uconvert_to_utime( &beamTime );

	if (_params->debug)
	  cerr << "Beam time is " << utimstr(beamTime.unix_time) << endl;
      }

      char *p = strstr(Line, "azi");

      if (p == NULL){
	cerr << "ERROR : Header line without reference azimuth found." << endl;
	exit(-1);
      }
      sscanf(p, "azi = %lf", &refAz);

      if (_params->debug)
	cerr << "Reference azimuth is " << refAz << endl;


      //
      // The first tilt - this is not the end of the last tilt.
      // It is the start of the volume.
      //
      radarFlags.startOfVolume = false;
      radarFlags.startOfTilt   = false;
      radarFlags.endOfTilt     = true;
      radarFlags.endOfVolume   = true;
      //
      if (_params->debug){
	fprintf(stderr,"End of last volume, if any.\n");
      }
      
 
      content = DsRadarMsg::RADAR_FLAGS;
      if( _radarQueue.putDsMsg( _radarMsg, content ) != 0 ) {  
	fprintf(stderr," Failed to send radar flags.\n");
	exit(-1);
      }
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
      
 
      content = DsRadarMsg::RADAR_FLAGS;
      if( _radarQueue.putDsMsg( _radarMsg, content ) != 0 ) {  
	fprintf(stderr," Failed to send radar flags.\n");
	exit(-1);
      }


      sleep(_params->sleepPostVolume);


      //
      // Set the count to 0 at the start of a sweep so that a new header is
      // sent at the start of each tilt.
      //
      sentCount = 0;
    
      continue; // Move on to the next line.

    }

    if (0!=strncmp("ppw", Line, 3)){
      cerr << "WARNING : unrecognised line :" << endl;
      cerr << Line;
      continue;
    }

    double azimuth = 0.0;

    // Character index in line. Skip past leading
    //'ppw' characters by setting to 3.
    int lineIndex = 3; 

    char number[24];
    int numIndex = 0;

    int first = 1;

    vector <double> beamValues;
    beamValues.clear();

    while (1) {
      if ((Line[lineIndex] == '.') ||
	  (Line[lineIndex] == '-') ||
	  isdigit(Line[lineIndex])
	  ){
	//
	// It's part of a number, record it and move on
	//
	number[numIndex] = Line[lineIndex];
	numIndex++;
	number[numIndex] = char(0);
      } else {

	double value = atof(number);
	
	// cerr << "VALUE " << value << endl;
	if (first){
	  azimuth = value; // Used to be value + refAz but this was erronneous
	  // if (azimuth > 360.0) azimuth -= 360.0;
	  first = 0;
	} else {
	  beamValues.push_back(value);
	}

	//
	// There may be multiple tab characters, representing bad data.
	//
	do {
	  if ((int)Line[lineIndex+1] == 9){
	    lineIndex++;
	    beamValues.push_back( _badVal );
	  }
	} while ((int)Line[lineIndex+1] == 9);

	if (((int)Line[lineIndex] == 10) || ((int)Line[lineIndex] == 0)) break; // End of the line, have all values.
	
	numIndex = 0;
	number[numIndex]=char(0);
      }
      lineIndex++;
    }

    if (_params->debug){

      int bFirst = 1;
      double min=0.0;
      double max=0.0;
      double total = 0.0;
      int numGood = 0;
      for (unsigned ib=0; ib < beamValues.size(); ib++){
	if (beamValues[ib] > -500) { // Non-missing value
	  numGood++;
	  total += beamValues[ib];
	  if (bFirst){
	    min = beamValues[ib];
	    max = beamValues[ib];
	    bFirst = 0;
	  } else {
	    if (min > beamValues[ib])  min = beamValues[ib];
	    if (max < beamValues[ib])  max = beamValues[ib];
	  }
	}
      }
      if (numGood){
	double mean = total / double( beamValues.size() );
	cerr << "Beam values range from " << min << " to " << max << " with mean " << mean;
	cerr << " at azimuth " << azimuth << " " << beamValues.size() << " gates." << endl;
      } else {
	cerr << "Beam at azimuth " << azimuth << "had no non-missing values." << endl;
      }
    }

    int nBins =  beamValues.size();

    if (nBins == 0) continue;

    if (sentCount == 0){

	radarParams.numGates = _params->nGatesToSend;

	radarParams.gateSpacing = gateSpacing;
	radarParams.startRange = gateStart;
	radarParams.horizBeamWidth = hBeamWidth;
	radarParams.vertBeamWidth = vBeamWidth;
	
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
      radarBeam.tiltNum    = 0;
      radarBeam.azimuth    = azimuth;
      radarBeam.elevation  = elevation;
      radarBeam.targetElev = radarBeam.elevation; // Control system for antenna not known.

      fl32 *beamData = (fl32 *)malloc(_nFields*_params->nGatesToSend*sizeof(fl32));
      if (beamData == NULL){
	cerr << "Malloc failed." << endl;
	exit(-1);
      }

      for (int ib=0; ib < _params->nGatesToSend; ib++){
	beamData[ib] = _badVal;
      }

      for (int ib=0; ((ib < nBins) && (ib < _params->nGatesToSend)); ib++){
	beamData[ib] = beamValues[ib];
      }
      beamValues.clear();
      
      if (nBins > _params->nGatesToSend)
	cerr << "WARNING : Beam was truncatedfrom " << nBins << " to " << _params->nGatesToSend << " gates!" << endl;


      radarBeam.loadData( beamData, _nFields*_params->nGatesToSend*sizeof(fl32), sizeof(fl32) ); 
 
      content = DsRadarMsg::RADAR_BEAM;  
      if( _radarQueue.putDsMsg(_radarMsg, content ) != 0 ) {  
	fprintf(stderr," Failed to send radar flags.\n");
	exit(-1);
      }
      
      free(beamData);
      
      sentCount = (sentCount + 1) % _params->beamsPerMessage;
      
  } // End of loop through rays
  
  fclose(fp);

  //
  // End of volume
  //
  radarFlags.startOfVolume = false;
  radarFlags.startOfTilt   = false;
  radarFlags.endOfTilt     = true;
  radarFlags.endOfVolume   = true;
  //
  if (_params->debug){
    fprintf(stderr,"End of volume.\n");
  }
  

  content = DsRadarMsg::RADAR_FLAGS;
  if( _radarQueue.putDsMsg(_radarMsg, content ) != 0 ) {  
    fprintf(stderr," Failed to send end of volume flag.\n");
    return;
  }

  _volumeNum = (_volumeNum+1) % 100; // Arbitrary

  return;

}

