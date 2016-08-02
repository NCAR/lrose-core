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
#include <unistd.h>

#include "Windtracer2Dsr.hh"

const float Windtracer2Dsr::MISSING_FLOAT = -9999.0;
const float Windtracer2Dsr::SCALE = 1.0;
const float Windtracer2Dsr::BIAS = 0.0;  

//
// Constructor. Copies parameters.
//
Windtracer2Dsr::Windtracer2Dsr(Params *TDRP_params){


  //
  // Reset the volume number and other things.
  //
  _beamsSentSoFar = 0;
  _volumeNum = 0; _numFieldsSoFar = 0;
  _year=0;  _month=0;  _day=0;  
  _hour=0;  _min=0;    _sec=0;
  _el=0.0; _az=0.0; _beamData=NULL;
  _firstAz = true;
  _onLastScan = false;
  _reachedFirstElevation = false;
  _reachedFirstAzimuth = false;
  _numOnLastScan = 0;
  _timeToCloseFile = false;
  // 
  _firstRange=0.0; _deltaRange=0.0; _numGates = 0;
  //
  _volumeStartTime = 0.0;
  _hasVolumeStartedYet = FALSE;
  _beamTime = 0.0;
  _beamDayTime = 0.0;
  //
  //
  // Point to the TDRP parameters.
  //
  _params = TDRP_params;
  //
  // Set up the message log.
  //
  _msgLog.setApplication( "Windtracer2Dsr" );

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
			"Windtracer22Dsr",
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
  // Set an int to the number of fields we expect, which
  // we take from the size of the lookup table.
  //
  _numExpectedFields = _params->fields_n;

  if ( _numExpectedFields > _maxFields){
    fprintf(stderr,"Too many fields desired - I cannot cope.\n");
    exit(-1);
  }



  //
  // Set up the field headers.
  //
  

  
  for(int i=0; i <  _numExpectedFields; i++){
    _fieldParams[i] = new DsFieldParams(_params->_fields[i].outputFieldName,
					_params->_fields[i].units,
					SCALE,
					BIAS,
					DATA_BYTE_WIDTH, MISSING_FLOAT);
    
  }




}
//
// Destructor. Does little.
//
Windtracer2Dsr::~Windtracer2Dsr(){

  if (_beamData != NULL){
    free (_beamData);
  }

  /* This seems to cause a core thump - Niles.
  for(int i=0; i <  _numExpectedFields; i++){
    delete _fieldParams[i];
  }
  */
}
//
// Main routine.
//
void Windtracer2Dsr::Windtracer2DsrFile( char *filename ){
  //
  // Initialize variables.
  //
  _firstTime = true;
  _fileTimeOffset = 0;
  _numFieldsSoFar = 0;
  _year=0;  _month=0;  _day=0;  
  _hour=0;  _min=0;    _sec=0;
  _el=0.0; _az=0.0; _beamData=NULL;
  _firstAz = true;
  _onLastScan = false;
  _numOnLastScan = 0;
  _timeToCloseFile = false;
  // 
  _firstRange=0.0; _deltaRange=0.0; _numGates = 0;
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
  _fileTime=0;
  if (_params->timeFromFilename){
    //
    // Back up to either the start of the string or the last '/'.
    //
    int startPos = strlen(filename);
    do {
      if (
	  (startPos == 0) || 
	  (filename[startPos] == '/')
	  ){
	break;
      }
      startPos--;
    } while(1);

    if (filename[startPos] == '/') startPos++;

    date_time_t nameTime;
    
    if (6 != sscanf(filename + startPos, "%4d%2d%2d_%2d%2d%2d",
		    &nameTime.year, &nameTime.month, &nameTime.day,
		    &nameTime.hour, &nameTime.min, &nameTime.sec)){
      fprintf(stderr,"Failed to decode time from %s\n",
	      filename + startPos);
      return;
    }

    uconvert_to_utime( &nameTime );

    _fileTime = nameTime.unix_time;

    if (_params->debug){
      fprintf(stderr,"Decoded time from filename %s is %s\n",
	      filename + startPos, utimstr(_fileTime));
    }
  }


  int content;
  //
  // Set up the params part of the message.
  //
  DsRadarParams& radarParams = _radarMsg.getRadarParams();
  DsRadarFlags& radarFlags = _radarMsg.getRadarFlags();

  radarParams.radarId = _params->lidarID.lidarNumber;
  if (_params->addTimeField){
    radarParams.numFields = _numExpectedFields + 2;
  } else {
    radarParams.numFields = _numExpectedFields;
  }

  radarParams.radarName = _params->lidarID.Name;
  radarParams.latitude = _params->lidarID.lat;
  radarParams.longitude = _params->lidarID.lon;
  radarParams.altitude = _params->lidarID.alt;
  
  if (_params->oneFilePerVolume){
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



  int irec = 0;

  do {

    int id, ver;
    int len;

    id = _readShort( &fp );
    ver = _readShort( &fp );
    len = _readLong( &fp );


    irec++;
    
    if (_params->debug){
      fprintf(stderr,"RECORD : %d ID : %x (%d) VER : %d LEN : %d ", 
	      irec, (int)id, (int)id, (int)ver, len);
      
      _printID( id ); fprintf(stderr,"\n");
    }
    
    if (
	(len < 1) ||
	(id == 0)
	){
      //
      // Reached the end of the file.
      //
      _timeToCloseFile = true;
    }

    if (_timeToCloseFile){
      fclose(fp);
      if (_params->oneFilePerVolume){
	//
	// Send an end of volume flag.
	//
	radarFlags.startOfVolume = false;
	radarFlags.startOfTilt   = false;
	radarFlags.endOfTilt     = true;
	radarFlags.endOfVolume   = true;
	
	content = DsRadarMsg::RADAR_FLAGS;
	if( _radarQueue.putDsMsg( _radarMsg, content ) != 0 ) {  
	  fprintf(stderr," Failed to send end of volume flag.\n");
	  exit(-1);
	}
      }
      //
      // Delay before next file.
      //
      for (int k=0; k < _params->fileDelaySecs; k++){
	sleep(1);
	if ( (k % 15) == 0){
	  PMU_auto_register("Delaying after beam parameters...");
	}
      }

      _volumeNum = (_volumeNum + 1) % 100;

      return;
    }

    switch (id) {
      
    case 0x10 :
      _processSpecial(fp);
      break;

    case 0x6 :
      _processText(fp, len-2);
      break;

    case 0xf :
      _processHeader(fp);
      break;

    default :

      //
      // If the field is desired, do it, otherwise skip it.
      //
      bool wantField = false;
   
      for (int i=0; i < _numExpectedFields; i++){
	if (_params->_fields[i].windtracerCode == id){
	  wantField = true;
	}
      }
      //
      if ((!wantField) || (_delAz > _params->maxDelAz)){
	//
	// Skip it.
	//
	int byteLen = 4*len-8;
	fseek(fp, byteLen, SEEK_CUR);
	break;
      } else {
	//
	// We want these data, process.
	// Check that we have read the text header first.
	//
	if (_numGates == 0){
	  fprintf(stderr,"ERROR - data encountered before text header.\n");
	  fprintf(stderr,"        Attempting to use existing file %s\n", 
		  _params->temp_file);
	  _processExistingText();
	}
	_processData(fp);
      }
    }


  } while (!(feof(fp)));

  fclose(fp);

  return;

}
//
// Read a short int, byte swapped.
//
int Windtracer2Dsr::_readShort(FILE **fp){

  unsigned char a,b;
  fread(&a, sizeof(unsigned char), 1, *fp);
  fread(&b, sizeof(unsigned char), 1, *fp);
  
  return a*256+b;

}
//
// Read a long int, byte swapped.
//
int Windtracer2Dsr::_readLong(FILE **fp){

  unsigned char a,b,c,d;

  fread(&a, sizeof(unsigned char), 1, *fp);
  fread(&b, sizeof(unsigned char), 1, *fp);
  fread(&c, sizeof(unsigned char), 1, *fp);
  fread(&d, sizeof(unsigned char), 1, *fp);

  return a*256*256*256 + b*256*256 + c*256 + d;

}

//
// Print a string to stderr describing the field.
//
void Windtracer2Dsr::_printID( int ID ){

  switch (ID) {
 
  case 0x0 :
    fprintf(stderr,"END OF FILE");
    break;

  case 0xf :
    fprintf(stderr,"CREC HDR FOR PRODUCT RECORD");
    break;

  case 0x10 :
    fprintf(stderr,"CREC SPECIAL FOR PRODUCT RECORD");
    break;

  case 0x11 :
    fprintf(stderr,"CREC RESULTS FOR PRODUCT RECORD");
    break;

  case 0x6 :
    fprintf(stderr,"ACTUAL START OF TEXT");
    break;

  case 0x15 :
    fprintf(stderr,"TEXT");
    break;

  case 0x1e :
    fprintf(stderr,"VELOCITY");
    break;

  case 0x1f :
    fprintf(stderr,"SNR");
    break;

  case 0x20 :
    fprintf(stderr,"CFAR STATUS");
    break;

  case 0x21 :
    fprintf(stderr,"SPECTRAL WIDTH");
    break;
   
  case 0x55 :
    fprintf(stderr,"MEDIAN FILTERED VELOCITY");
    break;

  case 0x56 :
    fprintf(stderr,"MEDIAN FILTERED SNR");
    break;

  case 0x58 :
    fprintf(stderr,"MODIFIED CFAR");
    break;

  case 0x59 :
    fprintf(stderr,"DATA QUALITY BITS");
    break;

  case 0x5a :
    fprintf(stderr,"AEROSOL BACKSCATTER FOR CLUTTER");
    break;

  default :
    fprintf(stderr,"UNKNOWN CODE : %x", ID);
    if ((ID > 1000) || (ID < -1000)){
      exit(-1); // Experience has shown this to be needed - Niles.
    }
    break;


  }


}
//
// Process the special header - for us, this holds the azimuth
// and the elevation.
//
void Windtracer2Dsr::_processSpecial(FILE *fp){
  //
  // Read the 18 entry header and decode the entries we need.
  //
  for (int i=0; i < 18; i++){
    float h;
    fread(&h, sizeof(float), 1, fp); 
    _byteSwap4(&h);
    if (i == 11) _az = h;
    if (i == 12) _el = h;
  }
  //
  if (_params->roundElevations){
    int iEl = (int)rint(_el/_params->elevationRounding);
    _el = iEl * _params->elevationRounding;
  }
  //
  // If this is the first azimuth we have encountered, and
  // we are trying to override the parameter location with
  // a file-based entry, then do the override.
  //
  if ((_firstAz) && (_params->readLocationFromFile)){

    date_time_t T;
    T.year = _year; T.month = _month; T.day = _day;
    T.hour = _hour; T.min = _min; T.sec = _sec;

    uconvert_to_utime( &T );
    T.unix_time = T.unix_time + _params->time_offset;

    double lat, lon, alt;
    if (_readLocationFile(T.unix_time,  &lat, &lon, &alt)){
      cerr << "Read from location file failed." << endl;
      exit(-1);
    }
    _params->lidarID.lat = lat;
    _params->lidarID.lon = lon;
    _params->lidarID.alt = alt/1000.0; // Convert to Km.

    if (_params->debug){
      cerr << "File location : " << lat << ", " << lon << endl;
    }

  }
  //
  // If this is the first az, make a note of it and
  // set the DelAz to 0.0.
  //

  if (_firstAz){
    _lastAz = _az;
    _signedDelAz = 0.0;
    _firstAz = false;
  }

  _delAz = fabs(_lastAz - _az);
  if (_delAz >= 360.0) _delAz = _delAz - 360.0;
 
  _lastSignedDelAz = _signedDelAz;
  _signedDelAz = _az - _lastAz;
  //
  // Cope with the breakpoint at 360.0.
  //
  if (_signedDelAz > 360.0)  _signedDelAz = _signedDelAz - 360.0;
  if (_signedDelAz < -360.0) _signedDelAz = _signedDelAz + 360.0;
  //
  // Update the _lastAz variable.
  //
  _lastAz = _az;
  //
  // Set the _azReversal variable.
  //
  if ( _signedDelAz*_lastSignedDelAz < 0.0){
    _azReversal = true;
  } else {
    _azReversal = false;
  }
  //
  // If this is on the last elevation, incement the
  // counter, otherwise reset it.
  //
  if (!(_onLastScan)){
    if (fabs(_el - _params->lastElevation) <= _params->elevationTolerance){
      _numOnLastScan++;
    } else {
      _numOnLastScan = 0;
    }
  }
  //
  // Use the counter to see if we are on the last scan.
  //
  if (_numOnLastScan > 4){
    _onLastScan = true;
  }
  //
  // If we are on the last scan and the elvation changes, set the
  // flag to close this file.
  //
  if (_onLastScan){
    if (fabs(_el - _params->lastElevation) > _params->elevationTolerance){
      _timeToCloseFile = true;
    }
  }

  //
  // If we have reached the first elevation, set the flag.
  //
  if (fabs(_el - _params->firstElevation) < _params->elevationTolerance){
    _reachedFirstElevation = true;
  }
  //
  // Similarly for the first azimuth.
  //
  if ( fabs(_az - _params->firstAzimuth) <= _params->firstAzimuthTolerance){
    _reachedFirstAzimuth = true;
  }
  //
  // The time should be set by now as well, so
  // print time and pointing vector if requested.
  //
  if (_params->debug){
    if ( _azReversal ){
      fprintf(stderr,"TIME FROM FILE : %d/%02d/%02d %0d:%02d:%02d AZ : %g EL : %g DEL AZ : %g REVERSAL\n",
	      _year, _month, _day, _hour, _min, _sec, _az, _el, _signedDelAz);
    } else {
      fprintf(stderr,"TIME FROM FILE : %d/%02d/%02d %0d:%02d:%02d AZ : %g EL : %g DEL AZ : %g\n",
	      _year, _month, _day, _hour, _min, _sec, _az, _el, _signedDelAz);
    }
  }
  //
  // Add elevation, azimuth offset.
  //
  _az = _az + _params->azimuth_offset;
  _el = _el + _params->elevation_offset;

  // Make sure the angles are still on the desired range.

  do {
    if (_az > 360.0) _az = _az - 360.0;
    if (_az < 0.0) _az = _az + 360.0;
    if (_el > 90.0){
      if (_el > 180.0){
	fprintf(stderr,"Silly elevation of %g, I cannot cope.\n",_el);
	exit(-1);
      }
      _el = 180.0 - _el;
      _az = _az + 180.0;
    }
  } while ((_el > 90.0) || (_az > 360.0) || (_az < 0.0));

  //
  return;

}
//
// Process the header - for us, holds the hour, minute and second.
//
void Windtracer2Dsr::_processHeader(FILE *fp){

  int hour=0, min=0, sec=0, msec=0;

  for (int i=0; i < 6; i++){
    int k;
    fread(&k, sizeof(int), 1, fp); 
    _byteSwap4(&k);
   
    if (i == 1) hour = k;
    if (i == 2) min = k;
    if (i == 3) sec = k;
    if (i == 4) msec = k;

  }

  //
  // Store these in the global section.
  //
  _hour = hour; _min = min; 
  _sec = sec;

  //
  // Set the _beamTime variable, which is used in outputting the
  // time field.
  //
  date_time_t TempTime;

  TempTime.year = _year; TempTime.month = _month; TempTime.day = _day;
  TempTime.hour = _hour; TempTime.min = _min; TempTime.sec = _sec;

  uconvert_to_utime ( &TempTime );
  TempTime.unix_time += _params->time_offset;
  uconvert_from_utime( &TempTime );

  _beamDayTime = double(TempTime.unix_time % SECONDS_PER_DAY) + 
    double(msec) / 1000.0;

  _beamTime = double(TempTime.unix_time) +  double(msec) / 1000.0;

  return;

}
//
// Routine to process data part of the file.
//
void Windtracer2Dsr::_processData(FILE *fp ){

  int count=0;
  for (int i=0; i < _numGates; i++){

    float d;
   
    fread(&d, sizeof(float), 1, fp); 
    _byteSwap4(&d);

    _beamData[_numExpectedFields*i + _numFieldsSoFar] = d;

    if (_params->debug >= Params::DEBUG_DATA){
      fprintf(stderr,"(%d, %g)", i+1, d);
      count ++;
      if (count > 4){
	fprintf(stderr,"\n");
	count = 0;
      } else {
	fprintf(stderr,"    ");
      }
    }
  }

  //
  // OK - having read the data, we discard it if we have specified ascending az
  // only and it is descending (or vice-versa).
  //
  if ((_params->azAscendingOnly) && (_signedDelAz <= 0.0)){
    if ( _params->debug >= Params::DEBUG_NORM ){
      cerr << "Scan rejected - not ascending in azimuth" << endl;
    }
    return;
  }

  if ((_params->azDescendingOnly) && (_signedDelAz >= 0.0)){
    if ( _params->debug >= Params::DEBUG_NORM ){
      cerr << "Scan rejected - not descending in azimuth" << endl;
    }
    return;
  }
  //
  // Also return if there is a first elevation specified and we have not
  // reached that first elevation (ie. discard leading junk).
  //
  if ((_params->firstElevationSpecified) && (!(_reachedFirstElevation))){
    if ( _params->debug >= Params::DEBUG_NORM ){
      cerr << "Scan rejected - not at first elevation yet" << endl;
    }
    return;
  }
  //
  // Do the same with the first azimuth.
  //
  if ((_params->firstAzimuthSpecified) && (!(_reachedFirstAzimuth))){
    if ( _params->debug >= Params::DEBUG_NORM ){
      cerr << "Scan rejected - not at first azimuth yet" << endl;
    }
    return;
  }

  //
  // Reject if we are out of angular bounds.
  //
  if (
     (_az < _params->minAzimuth) ||
     (_az > _params->maxAzimuth) ||
     (_el < _params->minElevation) ||
     (_el > _params->maxElevation)
     ){
    if ( _params->debug >= Params::DEBUG_NORM ){
      cerr << "Scan rejected - outside angular bounds" << endl;
    }
    return;
  }


  if (_params->debug >= Params::DEBUG_DATA) fprintf(stderr,"\n");

  _numFieldsSoFar++;

  if (_numFieldsSoFar == _numExpectedFields){

    //
    // Threshold all the fields on one field, if desired.
    //
    if (_params->thresholdOnField){

      for (int ig=0; ig < _numGates; ig++){
	//
	// Get the physical value at this gate for the threshold variable.
	//
	int threshFieldNum = _params->thresholdFieldNum;
	double physVal = _beamData[_numExpectedFields*ig + threshFieldNum];

	if (
	    (physVal < _params->threshMin) ||
	    (physVal > _params->threshMax)
	    ){

	  for (int ifn=0; ifn < _numExpectedFields; ifn++){
	    //
	    // Do not threshold the threhold variable itself.
	    // Or the time variable.
	    //
	    if ((ifn != -1) && (ifn != threshFieldNum)){
	      _beamData[_numExpectedFields*ig + ifn] = MISSING_FLOAT;
	    }
	  }
	}
      }
    }

    //
    // Do a shear test on the data, if desired.
    // This is done before the run-length test so that
    // gaps from data removed by the shear test are present for the run 
    // length test.
    //
    if (_params->doShearTest){
      _shearTest(_beamData);
    }


    //
    // Make a run-length check on the data.
    //
    if (_params->minRunLen > 0){
      //
      // Loop through the fields.
      //
      for (int ifn=0; ifn < _numExpectedFields; ifn++){
	//
	// get run lengths and set to bad if not long enough.
	//
	int igate = 0;
	do {

	if ( _beamData[_numExpectedFields*igate + ifn] != MISSING_FLOAT){
	  igate++;
	} else {
	  //
	  // We have a bad value - look for the start of the run,
	  // ie. the next non-bad value.
	  //
	  int irunStart = igate;
	  do{
	    irunStart++;
	    if (irunStart > _numGates-1) break;
	    if (_beamData[_numExpectedFields*irunStart + ifn] != MISSING_FLOAT) break;
	  } while(1); // loop exited by break statements.
	  //
	  // If we were bad right to the end of the run,
	  // forget it - advance the pointer to past the end of the beam.
	  //
	  if (irunStart > _numGates-1){
	    igate = _numGates + 1;
	    continue;
	  }
	  //
	  // Now find the end of the run - the next bad value,
	  // or the end of the beam.
	  //
	  int irunEnd = irunStart;
	  do{
	    irunEnd++;
	    if (irunEnd > _numGates-1) {
	      irunEnd = _numGates-1;
	      break;
	    }
	    if (_beamData[_numExpectedFields*irunEnd + ifn] == MISSING_FLOAT){
	      //
	      // Back off by 1 so we point to the good value.
	      //
	      irunEnd--;
	      break;
	    }
	  } while(1); // loop exited by break statements.
	  //
	  // Get the run length and set the run to be
	  // bad values if the run length is too short.
	  //
	  int irunLen = irunEnd - irunStart + 1;
	  if (irunLen < _params->minRunLen){
	    //
	    // Set the run to be missing data - it is too short.
	    //
	    for (int ir=irunStart; ir <= irunEnd; ir++){
	      _beamData[_numExpectedFields*ir + ifn] = MISSING_FLOAT;
	    }
	  }
	  //
	  // Advance the pointer to past the end of the run.
	  //
	  igate = irunEnd+1;
	}	
       } while (igate < _numGates);
      }
    } // End of run-length check on the data.

    //
    // See if it is time to send the radar params.
    //
    if (_beamsSentSoFar == 0) {
      //
      // It is - send them.
      //

      DsRadarParams& radarParams = _radarMsg.getRadarParams();

      radarParams.numGates = _numGates;
      
      radarParams.gateSpacing = _deltaRange / 1000.0; // Meters to Km.
      radarParams.startRange = _firstRange / 1000.0; // Meters to Km.


      radarParams.horizBeamWidth = _params->horizBeamWidth;
      radarParams.vertBeamWidth = _params->vertBeamWidth;
      /*
      radarParams.pulseWidth = (double) ray->h.pulse_width;
      radarParams.pulseRepFreq = (double) ray->h.prf;
      radarParams.wavelength = (double) ray->h.wavelength * 100.0; // Meters to cm.
      */

      //
      // Send the radar and field params.
      //
      int content =  DsRadarMsg::RADAR_PARAMS;
      if( _radarQueue.putDsMsg(_radarMsg, content ) != 0 ) {  
	fprintf(stderr," Failed to send radar params.\n");
	exit(-1);
      }
	
      vector< DsFieldParams* >& fieldParams = _radarMsg.getFieldParams();
      fieldParams.clear();
      
      for(int ifld=0; ifld <  _numExpectedFields; ifld++){
	fieldParams.push_back( _fieldParams[ifld] );
      }


      //
      // Push back another one if we are adding time.
      //
      if (_params->addTimeField){
	DsFieldParams *timeFieldParams = new DsFieldParams("TIM",
							   "sec",
							   SCALE, BIAS,
							   DATA_BYTE_WIDTH,
							   MISSING_FLOAT);
	fieldParams.push_back( timeFieldParams );

	DsFieldParams *timeFieldParamsAbs = new DsFieldParams("TIMABS",
							   "sec",
							   SCALE, BIAS,
							   DATA_BYTE_WIDTH,
							   MISSING_FLOAT);
	fieldParams.push_back( timeFieldParamsAbs );

      }

      content =  DsRadarMsg::FIELD_PARAMS;
      if( _radarQueue.putDsMsg(_radarMsg, content ) != 0 ) {  
	fprintf(stderr," Failed to send radar flags.\n");
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

   date_time_t beamTime;

   beamTime.year = _year;    beamTime.month = _month;
   beamTime.day = _day;      beamTime.hour  = _hour;
   beamTime.min = _min;   beamTime.sec  = _sec;
   
   uconvert_to_utime( &beamTime );
   
   if  (_params->timeFromFilename){
     if (_firstTime){
       //
       // This is the first beam in the file. Calculate the offset
       // between the file name time and the actual time in the file.
       //
       _firstTime = false;
       
       _fileTimeOffset = _fileTime - beamTime.unix_time;
       
     }
     //
     // If it is the first one or not, add the offset.
     //
     beamTime.unix_time = beamTime.unix_time + _fileTimeOffset;
     uconvert_from_utime( & beamTime);
   }
   

   DsRadarBeam& radarBeam = _radarMsg.getRadarBeam();
   
   if (_params->useRealTime){
     radarBeam.dataTime   = time(NULL);
   } else {
     radarBeam.dataTime   = beamTime.unix_time + _params->time_offset;
   }
   radarBeam.volumeNum  = _volumeNum;
   //   radarBeam.tiltNum    = i;
   radarBeam.azimuth    = _az;
   radarBeam.elevation  = _el;
   radarBeam.targetElev = radarBeam.elevation; // Control system for antenna not known.
   
   //
   // Send out this beam. If there has been a reversal in azimuth, and we
   // are sending volume end/start flags in that event, then do that first.
   //

   if ((_params->endVolumeOnAzReversal) &&  (_azReversal)){
     //
     if (_params->debug){
       fprintf(stderr,"Sending a volume start/end\n");
     }
     //
     // reset the beams sent count.
     //
     _beamsSentSoFar = 0;
     //
     // Send an end of volume flag.
     //
     DsRadarFlags& radarFlags = _radarMsg.getRadarFlags();
     radarFlags.startOfVolume = false;
     radarFlags.startOfTilt   = false;
     radarFlags.endOfTilt     = true;
     radarFlags.endOfVolume   = true;
     
     int content = DsRadarMsg::RADAR_FLAGS;
     if( _radarQueue.putDsMsg( _radarMsg, content ) != 0 ) {  
       fprintf(stderr," Failed to send end of volume flag.\n");
       exit(-1);
     }
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

   if (_params->debug) fprintf(stderr, "Sending beam...\n");

   //
   // If we want to output the time of the beam relative to the first
   // beam time, do that.
   //

   // What I will have to do here is to allocate another
   // beamData array with space for two more fields and then
   // plonk the time field at the end of that. Anything else
   // messes with the  reading of the data too much.

   if ( _params->addTimeField ){

     float *beamDataPlusTime = (float *) 
       malloc(sizeof(float)*(_numExpectedFields +2)*_numGates);

     if (NULL == beamDataPlusTime){
       cerr << " Malloc failed." << endl;
       exit(-1);
     }

     for (int ig = 0; ig < _numGates; ig++){
       for (int ifld = 0; ifld < _numExpectedFields; ifld++){

	 beamDataPlusTime[(_numExpectedFields + 2)*ig + ifld] = 
	   _beamData[_numExpectedFields*ig + ifld];

       }
       
       if (!_hasVolumeStartedYet){
	 //
	 // This is the first beam - send it with
	 // time offset of 0.0, and record the start time.
	 //
	 beamDataPlusTime[(_numExpectedFields+2)*ig + _numExpectedFields] = 0.0;
	 beamDataPlusTime[(_numExpectedFields+2)*ig + _numExpectedFields + 1] = _beamDayTime;
	 _volumeStartTime = _beamTime;
	 _hasVolumeStartedYet = true;

	 if (_params->debug >= Params::DEBUG_DATA)
	   cerr << "VOLUME START TIME : " << _volumeStartTime << endl;

       } else {
	 beamDataPlusTime[(_numExpectedFields+2)*ig + _numExpectedFields] = _beamTime - _volumeStartTime; 
	 beamDataPlusTime[(_numExpectedFields+2)*ig + _numExpectedFields + 1] = _beamDayTime;
       }
       if (_params->debug >= Params::DEBUG_DATA){
	 cerr << "SENDING BEAM TIMES " << beamDataPlusTime[(_numExpectedFields+2)*ig + _numExpectedFields];
	 cerr << " and " << beamDataPlusTime[(_numExpectedFields+2)*ig + _numExpectedFields + 1];
	 cerr << " FOR " << _hour << " " << _min << " " << _sec << endl;
       }
     }

     if (_params->debug >= Params::DEBUG_DATA)
       cerr << "SUBSEQUENT TIME : " << _beamTime - _volumeStartTime << endl;

     radarBeam.loadData( beamDataPlusTime,
			 (_numExpectedFields+2)*_numGates*DATA_BYTE_WIDTH, 
			 DATA_BYTE_WIDTH ); 

     free( beamDataPlusTime );
   } else {
     //
     // Load it up without the time.
     //

     radarBeam.loadData( _beamData,
			 _numExpectedFields*_numGates*DATA_BYTE_WIDTH, 
			 DATA_BYTE_WIDTH ); 
   }

   int content = DsRadarMsg::RADAR_BEAM;  
   if( _radarQueue.putDsMsg(_radarMsg, content ) != 0 ) {  
     fprintf(stderr," Failed to send beam data.\n");
     exit(-1);
   }
   //
   //
   // Reset the count.
   //
   _numFieldsSoFar = 0;
   _beamsSentSoFar = ( _beamsSentSoFar + 1 ) % _params->beamsPerMessage;
  }

}
//
// Routine to process the text part of the file. The text is pulled
// out and written to a temporary file, and then that temporary file is processed.
//
void Windtracer2Dsr::_processText(FILE *fp, int len){

  //
  // Pull the text section of the input file out and
  // write it to a file.
  //
  FILE *ofp = fopen(_params->temp_file,"w");
  if (ofp == NULL){
    fprintf(stderr,"Could not create %s\n", _params->temp_file);
    exit(-1);
  }
  
  int byteLen = len * 4;
  for(int i=0; i < byteLen; i++){
    char c;
    fread(&c, sizeof(c), 1, fp);
    //
    // Leave out control code 13 - windows file.
    //
    if ((int)c != 13)    fprintf(ofp,"%c", c);
  }
  
  fclose(ofp);
  //
  //  Process the file we have just written.
  //
  _processExistingText();

}

void Windtracer2Dsr::_processExistingText(){
  //
  // Process the file to read the year, month, date and the
  // parameters we will need to calulate range gate spacing.
  //
  FILE *ifp = fopen(_params->temp_file,"r");
  if (ifp == NULL){
    fprintf(stderr,"Could not read file %s\n", _params->temp_file);
    exit(-1);
  }

  double year, month, day;
  double sample_frequency;
  double raw_data_offset_meters;
  double raw_data_first_sample;
  double raw_data_sample_count;
  double range_gates;
  double samples_per_gate;
  double gates_to_merge;

  //
  // Read the year, month, day and sample frequency.
  // Must be done in this order (the order in which they appear
  // in the file).

  _readKeyword(ifp, "YEAR", &year, 1);
  _readKeyword(ifp, "MONTH", &month, 1);
  _readKeyword(ifp, "DAY", &day, 1);

  _year = (int)rint(year);
  _month = (int)rint(month);
  _day = (int)rint(day);


  _readKeyword(ifp, "SAMPLE_FREQUENCY", &sample_frequency, 1);

  //
  // Seek ahead to the tag line prior to reading the other parameters.
  //
  // 
  _readKeyword(ifp, _params->tag_line, NULL, 0);

  //
  // From this section, read the rest of the parameters we need and close the file.
  //
  _readKeyword(ifp, "RAW_DATA_OFFSET_METERS", &raw_data_offset_meters, 1);

  _readKeyword(ifp, "RAW_DATA_FIRST_SAMPLE", &raw_data_first_sample, 1);
  _readKeyword(ifp, "RAW_DATA_SAMPLE_COUNT", &raw_data_sample_count, 1);
  _readKeyword(ifp, "RANGE_GATES", &range_gates, 1);
  _readKeyword(ifp, "SAMPLES_PER_GATE", &samples_per_gate, 1);
  _readKeyword(ifp, "GATES_TO_MERGE", &gates_to_merge, 1);

  fclose(ifp);

  //
  // Print these out.
  //

  if (_params->debug){
    fprintf(stderr,"\nTEXT DATA FOR %d/%02d/%02d :\n", (int)year, (int)month, (int)day);

    fprintf(stderr,"Sample frequency : %g\n", sample_frequency);
    fprintf(stderr,"Raw data offset (meters) : %g\n", raw_data_offset_meters);
    fprintf(stderr,"Raw data first sample : %g\n", raw_data_first_sample);
    fprintf(stderr,"Raw data sample count : %g\n", raw_data_sample_count);
    fprintf(stderr,"Range gates : %g\n", range_gates);
    fprintf(stderr,"Samples per gate : %g\n", samples_per_gate);
    fprintf(stderr,"Gates to merge : %g\n", gates_to_merge);
  }
  //
  // Now, do the calculation of range gate spacing. This is
  // right out of the text I have from CLR.
  //
  double RangePerSample = 1.5e+08/sample_frequency;

  int SamplesBetweenRangeGateCenters = (int)rint((raw_data_sample_count-samples_per_gate)/(range_gates-1.0));

  double RangeBetweenGateCenters = SamplesBetweenRangeGateCenters * RangePerSample;

  double RangePerGate = samples_per_gate * RangePerSample;

  double FirstRange = raw_data_offset_meters + (raw_data_first_sample + samples_per_gate/2.0)*RangePerSample;

  double CorrectedFirstRange = FirstRange + ((gates_to_merge-1.0)/2.0) * RangePerGate;
  //
  // Print out the gate spacing parameters.
  //
  if (_params->debug){
    fprintf(stderr,"First range : %gm\n", CorrectedFirstRange);
    fprintf(stderr,"Range step : %gm\n", RangeBetweenGateCenters);
    fprintf(stderr,"Last range : %gm\n\n", 
	    CorrectedFirstRange+range_gates*RangeBetweenGateCenters);
  }
  //
  // Store them in some class parameters.
  //
  _firstRange = CorrectedFirstRange;
  _deltaRange = RangeBetweenGateCenters;
  //  _numGates = (int)rint(range_gates);
  _numGates = (int)rint(range_gates) - (int)rint(gates_to_merge) + 1;


  //
  // Allocate space for the data. Free space allocated prior, if any.
  //
  if (_beamData != NULL) free(_beamData);
  _beamData = (float *)malloc(_numGates * _numExpectedFields * sizeof(float));


  return;

}

//
// Small routine to read values from the text file. If tryRead is
// set, then a value is read into the double pointed at by val - otherwise the
// stream is positioned just after the keyword.
//
void Windtracer2Dsr::_readKeyword(FILE *fp, char *key, double *val, int tryRead){

  const int lineLen = 1024;
  char Line[lineLen];

  do {
    if (NULL == fgets(Line, lineLen, fp)){
      //
      // Must have hit the end of the file - should not have happened.
      //
      fprintf(stderr,"Could not locate keyword %s\n", key);
      exit(-1);
    }
    if (!(strncmp(Line, key, strlen(key)))){
      //
      // Found the key word.
      // If we don't have to read a value, return.
      //
      if (!(tryRead)) return;
      //
      // Otherwise, read the value.
      //
      char *p = Line + strlen(key);
      if (1 != sscanf(p, "%lf", val)){
	fprintf(stderr,"Could not decode keyword %s\n", key);
	exit(-1);
      }
      return;
    }
  } while(1);


}



void Windtracer2Dsr::_byteSwap4(void *p){

  unsigned char *b = (unsigned char *)p;

  unsigned char b1 = *b;
  unsigned char b2 = *(b+1);
  unsigned char b3 = *(b+2);
  unsigned char b4 = *(b+3);


  *(b+3) = b1;
  *(b+2) = b2;
  *(b+1) = b3;
  *b = b4;

  return;

}
//
// Shear test routine.
//
void Windtracer2Dsr::_shearTest(float *_beamData){

  //
  // Set up a bad value and unfold the velocity data into
  // physical values.
  //
  double *velData = (double *) malloc(sizeof(double)*_numGates);
  
  for (int ig=0; ig < _numGates; ig++){
    velData[ig] = _beamData[_numExpectedFields*ig + _params->velFieldNum];
  }
  //
  // Do the shear test.
  //
  double maxShear=0.0;
  int numRemoved = 0;
  for (int ii=0; ii < _numGates-1; ii++){
    if (velData[ii] != MISSING_FLOAT){
      for (int ij = ii+1; ij < _numGates; ij++){
	if (velData[ij] != MISSING_FLOAT){
	  double dist =  _deltaRange * (ij-ii);
	  double shear = (velData[ij] - velData[ii])/dist;
	  if (shear < 0.0) shear = -shear;
	  if (shear > maxShear){
	    maxShear = shear;
	  }
	  if (shear > _params->maxShear){
	    velData[ij] = MISSING_FLOAT;
	    numRemoved++;
	  }
	}
      }
    }
  }
  if (_params->debug){
    fprintf(stderr,"Maximum shear is %g\n", maxShear);
    fprintf(stderr,"%d points removed by shear test.\n",
	    numRemoved);
  }
  
  //
  // Fold the data back into the _beamData array of bytes.
  //
  for (int ig=0; ig < _numGates; ig++){
    _beamData[_numExpectedFields*ig + _params->velFieldNum] = velData[ig];
  }

  free(velData);


}

int Windtracer2Dsr::_readLocationFile(time_t DataTime, double *lat,
                                      double *lon, double *alt){

  //
  // Routine to read the location from a file
  // to keep up with the lidar moves.
  //
  FILE *fp = fopen(_params->locationFile,"r");
  if (fp == NULL){
    cerr << "Could not open location file " << _params->locationFile << endl;
    return -1;
  }

  //
  // Take two hits at the file - first, find out how many
  // lines are in it.
  //
  int numLines = 0;

  char Line[1024];
  while (NULL != fgets(Line, 1024, fp)){
    int year, month, day, hour, min, sec;
    char timeZone[256];
    double lat, lon, alt;

    if (10 == sscanf(Line,
		     "%d-%d-%d %d:%d:%d %s %lf %lf %lf",
		     &year, &month, &day,
		     &hour, &min, &sec,
		     timeZone,
		     &lat, &lon, &alt)){
      numLines++;
    }
  }
  rewind(fp);
  if (numLines == 0){
    cerr << "No data found in " << _params->locationFile << endl;
    fclose(fp);
    return -1;
  }
  //
  // OK - now know the number of entries in this file - allocate
  // space for the entries and read them in.
  //
  time_t *entryTimes = (time_t *) malloc(sizeof(time_t) * numLines);
  double *lats = (double *) malloc(sizeof( double ) * numLines);
  double *lons = (double *) malloc(sizeof( double ) * numLines);
  double *alts = (double *) malloc(sizeof( double ) * numLines);

  if ((entryTimes == NULL) || (lats == NULL) ||
      (lons == NULL) || (alts == NULL)){
    cerr << "Malloc failed!" << endl;
    exit(-1); // Unlikely.
  }

  int index = 0;
  while (NULL != fgets(Line, 1024, fp)){

    char timeZone[256];
    date_time_t T;
    if (10 == sscanf(Line,
		     "%d-%d-%d %d:%d:%d %s %lf %lf %lf",
		     &T.year, &T.month, &T.day,
		     &T.hour, &T.min, &T.sec,
		     timeZone,
		     &lats[index], &lons[index], &alts[index])){
      uconvert_to_utime( &T );
      entryTimes[index] = T.unix_time + _params->time_offset;

      index++;
      if (index > numLines){
	//
	// Should not happen.
	//
	cerr << "Fatal error!" << endl;
	exit(-1);
      }
    }
  }

  fclose(fp);

  //
  // Now we have what we need to make the decision 
  // as to what location to return. Return the last entry time
  // that is before the data time - or the very first entry time
  // if none of the entry times is before the data time.
  //

  for (int ii=numLines-1; ii >= 0; ii--){

    if ((ii == 0) || (entryTimes[ii] <= DataTime)){
      *lon = lons[ii];
      *lat = lats[ii];
      *alt = alts[ii];

      free(entryTimes); free(lats); free(lons); free(alts);
      return 0; // All went well.
    }
  }
  return -1; // Should never get here.
}
