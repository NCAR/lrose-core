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

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <toolsa/pmu.h>
#include <toolsa/umisc.h>
#include <unistd.h>

#include "WTXlidar2Dsr.hh"


double WTXlidar2Dsr::_parseConfigElement(char *config, char *keyStr, bool &ok){

  ok = false;

  // Find key in config file
  char *p = strstr(config, keyStr);
  if (p == NULL) return -999.0;

  // Jump over key to numerical value
  char *s = p + strlen(keyStr);

  // Decode numerical value
  double val = atof(s);

  ok = true;
  return val;

}



//
// Constructor. Copies parameters.
//
WTXlidar2Dsr::WTXlidar2Dsr(Params *TDRP_params){
  //
  _volumeNum = 0;  _volumeStartTime=0L;
  //
  // Point to the TDRP parameters.
  //
  _params = TDRP_params;
  //
  // Set up the message log.
  //
  _msgLog.setApplication( "WTXlidar2Dsr" );

  //
  // Enable debug level messaging
  //
  if ( _params->debug == TRUE ){
    _msgLog.enableMsg( DEBUG, true );
    _msgLog.enableMsg( INFO, true );
  } else { 
    _msgLog.enableMsg( DEBUG, false );
    _msgLog.enableMsg( INFO, false );
  }
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
			"WTXlidar2Dsr",
			 _params->debug,           
			 DsFmq::READ_WRITE, DsFmq::END,
			_params->output_fmq_compress,
			_params->output_fmq_nslots,
			_params->output_fmq_size, 1000,
			&_msgLog )) {
    cerr << "Could not initialize fmq " << _params->output_fmq_url << endl; 
    exit(-1);
  }

  //
  // If archive mode is our game, use blocking writes.
  //
  if (_params->mode == Params::ARCHIVE){
    if (_params->debug){
      cerr << "Using blocking writes." << endl;
    }
    if ( _radarQueue.setBlockingWrite()){
      cerr << "Failed to set blocking write!" << endl;
      exit(-1);
    }
  }


  //
  // Set up the field headers. Boring but effective.
  //
  _fieldParams[0] = new DsFieldParams("VEL", "m/s", SCALE, BIAS, DATA_BYTE_WIDTH, MISSING_FLOAT);
  _fieldParams[1] = new DsFieldParams("SNR", "none", SCALE, BIAS, DATA_BYTE_WIDTH, MISSING_FLOAT);
  _fieldParams[2] = new DsFieldParams("SW", "m/s", SCALE, BIAS, DATA_BYTE_WIDTH, MISSING_FLOAT);
  _fieldParams[3] = new DsFieldParams("BACK", "none", SCALE, BIAS, DATA_BYTE_WIDTH, MISSING_FLOAT);
  _fieldParams[4] = new DsFieldParams("TIM", "sec", SCALE, BIAS, DATA_BYTE_WIDTH, MISSING_FLOAT);
  _fieldParams[5] = new DsFieldParams("TIMABS", "sec", SCALE, BIAS, DATA_BYTE_WIDTH, MISSING_FLOAT);

  _timingID = 0x0420;
  if (_params->filtered)  _timingID = 0x0421;

  return;

}

// Given field ID, get field index
int WTXlidar2Dsr::_getFieldIndex(int fieldID){


  if (_params->filtered){

    switch (fieldID) {

    case 0x04b0 :
      return 0;
      
    case 0x04b1 :
      return 1;
      
    case 0x04b2 :
      return 3;
      
    case 0x04b3 :
      return 2;


    default :
      return -1;

    }

  } else {

    switch (fieldID) {

    case 0x04a1 :
      return 0;

    case 0x04a2 :
      return 1;

    case 0x04a5 :
      return 2;

    case 0x04a8 :
      return 3;

    default :
      return -1;

    }
  }
}

void WTXlidar2Dsr::_clearData(){

  _az = MISSING_FLOAT;
  _el = MISSING_FLOAT;
  if (_beamData != NULL){
    for (int i=0; i < _numGates*_numFields; i++){
      _beamData[i]=MISSING_FLOAT;
    }
  }
  return;
}

//
// Destructor. Does little.
//
WTXlidar2Dsr::~WTXlidar2Dsr(){

  if (_beamData != NULL){
    free (_beamData);
  }

  return;

}
//
// Main routine.
//
void WTXlidar2Dsr::WTXlidar2DsrFile( char *filename ){
  //
  // Initialize variables.
  //
  _beamData = NULL;
  _numGates = -1;
  _clearData();
  //

  //
  // Open the input file.
  //

  if (_params->debug){
    fprintf(stderr,"Processing file %s\n", filename);
  }

  FILE *fp = fopen (filename,"r");
  if (fp == NULL){
    fprintf(stderr,"%s not found.\n",filename);
    return;
  }


  //
  // If we are using the file name time, decode it.
  //
  time_t fileTime=0L;
  if (_params->timeFromFilename){
    //
    char *p = filename + strlen(filename) - strlen("20101011_020653_base.prd");

    date_time_t nameTime;
    
    if (6 == sscanf(p, "%4d%2d%2d_%2d%2d%2d",
		    &nameTime.year, &nameTime.month, &nameTime.day,
		    &nameTime.hour, &nameTime.min, &nameTime.sec)){

      uconvert_to_utime( &nameTime );

      fileTime = nameTime.unix_time;

      if (_params->debug){
	fprintf(stderr,"Decoded time from filename %s is %s\n",
		p, utimstr(fileTime));
      }
    } else {
      fprintf(stderr,"Failed to decode time from filename %s, using data times.\n", filename);
    }
  }

  //
  // Set up the params part of the message.
  //
  DsRadarParams& radarParams = _radarMsg.getRadarParams();
  DsRadarFlags& radarFlags = _radarMsg.getRadarFlags();

  radarParams.radarId = _params->lidarID.lidarNumber;
  radarParams.numFields = _numFields;
  radarParams.radarName = _params->lidarID.Name;
  radarParams.latitude = _params->lidarID.lat;
  radarParams.longitude = _params->lidarID.lon;
  radarParams.altitude = _params->lidarID.alt;
  


  int lastID =0;
  ///////////////////////// Config data ///////////////////////////
  bool gotConfig = false;
  while (1) { 
    BlockDescriptor_t B;

    long startPos = ftell(fp);
    if (1 != fread(&B, sizeof(BlockDescriptor_t), 1, fp)) break;

    if (B.nId == 0x04f0){ // Config data - read in this long string with room for last zero terminator byte
      int lenOfString = B.nBlockLength - sizeof(BlockDescriptor_t) + 1;
      char *config  = (char *)malloc(lenOfString);
      if (config == NULL){
	fprintf(stderr,"Malloc failed!\n");
	exit(-1);
      }

      if (lenOfString-1 != (int)fread(config, sizeof(char), lenOfString-1, fp)) break;
      config[lenOfString-1]=char(0);

      bool ok, totalOK;
      _numGates = (int)rint( _parseConfigElement(config, (char *)"P_RANGE_GATES", ok)); totalOK = ok;
      double fSampleFrequency = _parseConfigElement(config, (char *)"Q_SAMPLING_RATE", ok); ok = ok && totalOK;
      double fRawDataOffsetMeters =  _parseConfigElement(config, (char *)"Q_RAW_DATA_OFFSET_METERS", ok); ok = ok && totalOK;
      double dwRawDataSampleCount = _parseConfigElement(config, (char *)"Q_RAW_SIGNAL_BLOCK_SAMPLE_COUNT", ok); ok = ok && totalOK;
      double dwRangeGates = _parseConfigElement(config, (char *)"P_RANGE_GATES", ok); ok = ok && totalOK;
      double dwSamplesPerGate = _parseConfigElement(config, (char *)"P_SAMPLES_PER_GATE", ok); ok = ok && totalOK;
      double dwGatesToMerge = _parseConfigElement(config, (char *)"P_GATES_TO_MERGE", ok); ok = ok && totalOK;

      if (!(totalOK)){
	fprintf(stderr,"Error processing config block\n");
	exit(-1);
      }

      double RangePerSample = 1.5e+8/fSampleFrequency;
      double SamplesBetweenRangeGateCenters = floor((dwRawDataSampleCount-dwSamplesPerGate)/(dwRangeGates-1.0));
      double RangeBetweenGateCenters=SamplesBetweenRangeGateCenters*RangePerSample;
      double RangePerGate=dwSamplesPerGate*RangePerSample;
      double FirstRange = fRawDataOffsetMeters+(dwSamplesPerGate/2.0)*RangePerSample;
      double CorrectedFirstRange = FirstRange+((dwGatesToMerge-1.0)/2.0)*RangePerGate;

      _firstRange = FirstRange/1000.0; _deltaRange = RangeBetweenGateCenters/1000.0; // Convert m to Km

      if (_params->debug){
	fprintf(stderr,"Config information - N GATES : %d ", _numGates);
	fprintf(stderr,"RANGE TO FIRST GATE : %g ", CorrectedFirstRange);
	fprintf(stderr,"GATE SPACING : %g\n", RangeBetweenGateCenters);
      }

      if (_beamData == NULL){
	_beamData = (fl32 *)malloc( _numFields * _numGates * sizeof(fl32));
	if (_beamData == NULL){
	  fprintf(stderr, "Malloc failed!\n");
	  exit(-1);
	}
      }
      free(config);
      fseek(fp, startPos + B.nBlockLength, SEEK_SET); lastID = B.nId;
      gotConfig = true;
      break;
    }

    fseek(fp, startPos + B.nBlockLength, SEEK_SET); lastID = B.nId;

  }

  if (!(gotConfig)){
    fprintf(stderr,"Error - no config block found.\n");
    exit(-1);
  }


  int numFieldsFound=0;
  int numBeamsSent=0;
  bool havePointing = false;
  int content;
  ////////////////// Main loop. Timing, pointing and beam information. /////////////////////////
  while (1) {

    PMU_auto_register("Processing...");

    BlockDescriptor_t B;
    long startPos = ftell(fp);
    if (1 != fread(&B, sizeof(BlockDescriptor_t), 1, fp)) break;

    // Timing information? If we have read fields, send a beam, and if it is time, send radar and field params.
    if (B.nId == _timingID){
      RecordHeader_t R;
      if (1 != fread(&R, sizeof(RecordHeader_t), 1, fp)) break;


      date_time_t T;
      T.year = 	R.nYear;      T.month = R.nMonth;      T.day = R.nDayOfMonth;
      T.hour = R.nHour;       T.min = R.nMinute;       T.sec = R.nSecond;

      uconvert_to_utime( &T ); _dataTime = T.unix_time;

      if (_params->debug){
	fprintf(stderr,"\nTiming information : %d/%02d/%02d %02d:%02d:%02d from ID %04x (%s)\n",
		R.nYear, R.nMonth, R.nDayOfMonth, R.nHour, R.nMinute, R.nSecond, B.nId, utimstr(_dataTime));
      }

      if ((_dataTime != 0L) && (numFieldsFound)){ // We have data to send, send it.
	numFieldsFound = 0;

	//
	// See if it is time to send the radar params.
	//
	if (numBeamsSent == 0) {
	  //
	  // It is - send them.
	  //
	  DsRadarParams& radarParams = _radarMsg.getRadarParams();
	  
	  radarParams.numGates = _numGates;
	  radarParams.gateSpacing = _deltaRange;
	  radarParams.startRange = _firstRange;
	  radarParams.horizBeamWidth = _params->horizBeamWidth;
	  radarParams.vertBeamWidth = _params->vertBeamWidth;
	  content =  DsRadarMsg::RADAR_PARAMS;
	  if( _radarQueue.putDsMsg(_radarMsg, content ) != 0 ) {
	    fprintf(stderr," Failed to send radar params.\n");
	    exit(-1);
	  }
	  
	  vector< DsFieldParams* >& fieldParams = _radarMsg.getFieldParams();
	  fieldParams.clear();
	  for(int ifld=0; ifld <  _numFields; ifld++){
	    fieldParams.push_back( _fieldParams[ifld] );
	  }
	  content =  DsRadarMsg::FIELD_PARAMS;
	  if( _radarQueue.putDsMsg(_radarMsg, content ) != 0 ) {
	    fprintf(stderr," Failed to send field params.\n");
	    exit(-1);
	  }
	  
	  //
	  // Delay.
	  //
	  for (int k=0; k < _params->delaySecs; k++){
	    sleep(1);
	    if ( (k % 15) == 0){
	      PMU_auto_register("Delaying after beam parameters...");
	    }
	  }
	}

	// Send beam, if ready.
	if ((_dataTime != 0L) && (havePointing)){
	  DsRadarBeam& radarBeam = _radarMsg.getRadarBeam();

	  radarBeam.dataTime = _dataTime;
	  if ((_params->timeFromFilename) && (fileTime != 0L)) radarBeam.dataTime   = fileTime;
	  if (_params->useRealTime) radarBeam.dataTime   = time(NULL);
	  radarBeam.dataTime += _params->time_offset;

	  if ( _volumeStartTime == 0L)  _volumeStartTime =  _dataTime;

	  //
	  // Add the time and time absolute fields -the last two fields.
	  //
	  for(int ig=0; ig < _numGates; ig++){
	    _beamData[(_numFields-2) + _numFields*ig] = _dataTime - _volumeStartTime; 
	    _beamData[(_numFields-1) + _numFields*ig] = _dataTime;
	  }

	  radarBeam.volumeNum  = _volumeNum;
	  radarBeam.azimuth    = _az;
	  radarBeam.elevation  = _el;
	  radarBeam.targetElev = radarBeam.elevation;
	  radarBeam.loadData( _beamData, _numFields*_numGates*DATA_BYTE_WIDTH, DATA_BYTE_WIDTH );

	  int content = DsRadarMsg::RADAR_BEAM;
	  if( _radarQueue.putDsMsg(_radarMsg, content ) != 0 ) {
	    fprintf(stderr," Failed to send beam data.\n");
	    exit(-1);
	  }
	  umsleep(_params->msDelayPostBeam);
	  // Get ready for next beam
	  _clearData();
	  numBeamsSent = (numBeamsSent+1) % _params->beamsPerMessage;
	  numFieldsFound=0;
	}
      }

    }

    // Pointing information? Also has volume start/end info ------------------------------------------------
    if ((lastID == _timingID) && (B.nId == 0x04ef)){
      ScanInfo_t S;
      if (1 != fread(&S, sizeof(ScanInfo_t), 1, fp)) break;
      if (_params->debug){
	fprintf(stderr, "Pointing information after ID %04x : AZ EL STATE ENABLED : %g %g %d %d\n", 
		lastID, S.fScanAzimuth_deg, S.fScanElevation_deg, (int)S.nAcqScanState, (int)S.nScanEnabled);
      }

      if (S.nAcqScanState == 1){
	if (_params->debug) fprintf(stderr, "\n\nSTART OF VOLUME\n");
	//
	// Send a start of volume flag.
	//
	radarFlags.startOfVolume = true;
	radarFlags.startOfTilt   = true;
	radarFlags.endOfTilt     = false;
	radarFlags.endOfVolume   = false;

	content = DsRadarMsg::RADAR_FLAGS;
	if( _radarQueue.putDsMsg( _radarMsg, content ) != 0 ) {  
	  fprintf(stderr," Failed to send start of volume flag.\n");
	  exit(-1);
	}
      }

      if (S.nAcqScanState == 3){
	_volumeNum = (_volumeNum + 1) % 100;
	if (_params->debug) fprintf(stderr, "\n\nEND OF VOLUME\n");
	//
	// Send an end of volume flag.
	//
	radarFlags.startOfVolume = false;
	radarFlags.startOfTilt   = false;
	radarFlags.endOfTilt     = true;
	radarFlags.endOfVolume   = true;

	content = DsRadarMsg::RADAR_FLAGS;
	if( _radarQueue.putDsMsg( _radarMsg, content ) != 0 ) {  
	  fprintf(stderr," Failed to send start of volume flag.\n");
	  exit(-1);
	}
	_volumeStartTime=0L;
      }

      _az =  S.fScanAzimuth_deg; _el = S.fScanElevation_deg;

      havePointing=true;

    }

    /*
    // Product pulse? ---------------------------------------------------------------------------
    if (B.nId == 0x04a0){
      ProductPulseInfo_t P;
      if (1 != fread(&P, sizeof(ProductPulseInfo_t), 1, fp)) break;
      fprintf(stderr,"Pulse information : %g %g %g\n",
	      P.fAzimuthMin_deg, P.fAzimuthMean_deg,P.fAzimuthMax_deg );
      break;	      
    }
    */
    
    
    // Beam data? --------------------------------------------------------------------------------
    if ((_numGates > 0) && (B.nBlockLength == sizeof(BlockDescriptor_t) + sizeof(fl32)*_numGates)){
      int fieldIndex = _getFieldIndex(B.nId);
 
      // Only proceed if ready for beam data
      if ((fieldIndex > -1) && (_beamData != NULL)){
 
	// Allocate memory and read data in.
	fl32 *data = (fl32 *)malloc(sizeof(fl32)*_numGates);
	if (data==NULL){
	  fprintf(stderr,"Malloc error\n");
	  exit(-1);
	}
	if (_numGates != (int)fread(data, sizeof(fl32), _numGates, fp)){
	  free(data);
	  break;
	}

	if (_params->debug){ // Print debug info.
	  double min=0.0;
	  double max=0.0;
	  double mean=0.0;
	  int nGood=0;
	  for(int i=0; i < _numGates; i++){
	    if (fabs(data[i]-MISSING_FLOAT) > 0.1){
	      mean += data[i];
	      if (nGood == 0){
		min=data[i]; max=data[i];
	      } else {
		if (data[i] < min) min = data[i];
		if (data[i] > max) max = data[i];
	      }
	      nGood++;
	    }
	  }
	  fprintf(stderr,"Beam data - ID : %04x LEN %d index %d ", B.nId, (int)B.nBlockLength, fieldIndex);
	  if (nGood){
	    mean=mean/double(nGood);
	    fprintf(stderr,"data run from %g to %g mean %g, %d samples\n", min, max, mean, nGood);
	  } else {
	    fprintf(stderr, "No non-missing data found.\n");
	  }

	} // End of debugging prints.

	// Copy data into right place in _beamData array.
	for(int i=0; i < _numGates; i++){
	  _beamData[fieldIndex + _numFields*i]=data[i];
	}
	numFieldsFound++;
	free(data);
      }
    }
 
    fseek(fp, startPos + B.nBlockLength, SEEK_SET); lastID = B.nId;

  }

  fclose(fp);

  //
  // If we have a beam still to send, send it
  //
  if (numFieldsFound){
    DsRadarBeam& radarBeam = _radarMsg.getRadarBeam();
    radarBeam.dataTime = _dataTime;
    if ((_params->timeFromFilename) && (fileTime != 0L)) radarBeam.dataTime   = fileTime;
    if (_params->useRealTime) radarBeam.dataTime   = time(NULL);
    radarBeam.dataTime += _params->time_offset;
    radarBeam.volumeNum  = _volumeNum;
    radarBeam.azimuth    = _az;
    radarBeam.elevation  = _el;
    radarBeam.targetElev = radarBeam.elevation;
    radarBeam.loadData( _beamData, _numFields*_numGates*DATA_BYTE_WIDTH, DATA_BYTE_WIDTH );
    
    int content = DsRadarMsg::RADAR_BEAM;
    if( _radarQueue.putDsMsg(_radarMsg, content ) != 0 ) {
      fprintf(stderr," Failed to send beam data.\n");
      exit(-1);
    }
  }

  //
  // Send an end of volume flag at end of file.
  //
  radarFlags.startOfVolume = false;
  radarFlags.startOfTilt   = false;
  radarFlags.endOfTilt     = true;
  radarFlags.endOfVolume   = true;
  
  content = DsRadarMsg::RADAR_FLAGS;
  if( _radarQueue.putDsMsg( _radarMsg, content ) != 0 ) {  
    fprintf(stderr," Failed to send start of volume flag.\n");
    exit(-1);
  }
  _volumeStartTime=0L;


  if (_beamData != NULL){
    free(_beamData);
    _beamData = NULL;
  }

  for (int k=0; k < _params->fileDelaySecs; k++){
    sleep(1);
    if ( (k % 15) == 0){
      PMU_auto_register("Delaying after file...");
    }
  }

  return;

}
