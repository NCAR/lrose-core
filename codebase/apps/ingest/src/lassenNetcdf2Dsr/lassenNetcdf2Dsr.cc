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

/////////////////////////////////////////////////////////////
//
// Inits FMQ in constructor, sends data in method.
//
// Niles Oien, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
//
/////////////////////////////////////////////////////////////

#include <netcdf.h>
#include "lassenNetcdf2Dsr.hh"

// Constructor. Inits output FMQ.
lassenNetcdf2Dsr::lassenNetcdf2Dsr ( Params *TDRP_params ){

  _lastTiltEl = -999.0;
  _firstTilt = true;
  
  _params = TDRP_params;
 //
  // Enable debug level messaging
  //
  if ( _params->debug ){
    _msgLog.enableMsg( DEBUG, true );
    _msgLog.enableMsg( INFO, true );
  } else {
    _msgLog.enableMsg( DEBUG, false );
    _msgLog.enableMsg( INFO, false );
  }
  //
  //
  //
  if ( _msgLog.setOutputDir( _params->outputFmq.fmqMsgLogDir ) != 0 ) {
    cerr << "Failed to set up message log to directory ";
    cerr << _params->outputFmq.fmqMsgLogDir;
    cerr << endl;
    exit(-1);
  }

  //
  // Initialize the radar queue.
  //
  if( _radarQueue.init( _params->outputFmq.fmqUrl,
                        "lassenNetcdf2Dsr",
                        _params->debug,
                        DsFmq::READ_WRITE, DsFmq::END,
                        _params->outputFmq.fmqCompress,
                        _params->outputFmq.fmqNumSlots,
                        _params->outputFmq.fmqSizeBytes, 1000,
                        &_msgLog )) {
    cerr << "Could not initialize fmq " << _params->outputFmq.fmqUrl << endl;
    exit(-1);
  }

  if ( _radarQueue.setBlockingWrite()){
    cerr << "Failed to set blocking write!" << endl;
    exit(-1);
  }

  _fieldParams[0] = new DsFieldParams("DBZ", "dbz",
				      SCALE,
				      BIAS,
				      DATA_BYTE_WIDTH, (int)MISSING_FLOAT);
  
  
  _fieldParams[1] = new DsFieldParams("VEL", "m/s",
				      SCALE,
				      BIAS,
				      DATA_BYTE_WIDTH, (int)MISSING_FLOAT);

  _volNum = 0;

  return;
}

// Process volumes.
void lassenNetcdf2Dsr::processFilePair( char *zFile, char *uFile, time_t dataTime, int tiltNum ){


  bool haveU = true;
  if (uFile == NULL) haveU = false;

  //
  // Pull both files into memory.
  //
  int ZnetID;
  int status = nc_open(zFile, NC_NOWRITE, &ZnetID);
  if ( _checkStatus(status, "Failed to open input Z file.")){
    cerr << "Problem file was " << zFile << endl;
    return;
  }

  int UnetID;
  if (haveU){
    status = nc_open(uFile, NC_NOWRITE, &UnetID);
    if ( _checkStatus(status, "Failed to open input U file.")){
      cerr << "Problem file was " << uFile << endl;
      haveU = false;
    }
  }
  //
  // Get the dimensions of the array so we can allocate space and read it in.
  // I'm assuming that U and Z have the same dimensions.
  //
  int rangeDimID;
  status = nc_inq_dimid(ZnetID, "Range", &rangeDimID);
  if ( _checkStatus(status, "Failed to get range dimension ID.")){
    cerr << "Z file was " << zFile << endl;
    nc_close( ZnetID );
    if (haveU) nc_close( UnetID );
    return;
  }

  size_t ZrangeNum;
  status = nc_inq_dimlen(ZnetID, rangeDimID, &ZrangeNum);
  if ( _checkStatus(status, "Failed to get range dimension.")){
    cerr << "Z file was " << zFile << endl;
    nc_close( ZnetID );
    if (haveU) nc_close( UnetID );
    return;
  }

  unsigned maxRangeNum = ZrangeNum;

  //
  // U and Z can have different numbers of gates.
  // Get the number from U.
  //
  size_t UrangeNum = ZrangeNum;
  if (haveU){
    status = nc_inq_dimid(UnetID, "Range", &rangeDimID);
    if ( _checkStatus(status, "Failed to get U range dimension ID.")){
      cerr << "Z file was " << zFile << endl;
      nc_close( ZnetID );
      if (haveU) nc_close( UnetID );
      return;
    }

    status = nc_inq_dimlen(UnetID, rangeDimID, &UrangeNum);
    if ( _checkStatus(status, "Failed to get U range dimension.")){
      cerr << "Z file was " << zFile << endl;
      nc_close( ZnetID );
      if (haveU) nc_close( UnetID );
      return;
    }

    if (UrangeNum > ZrangeNum) maxRangeNum = UrangeNum;
  }

  int azDimID;
  status = nc_inq_dimid(ZnetID, "Azimuth", &azDimID);
  if ( _checkStatus(status, "Failed to get az dimension ID.")){
    cerr << "Z file was " << zFile << endl;
    nc_close( ZnetID );
    if (haveU) nc_close( UnetID );
    return;
  }

  size_t azNum;
  status = nc_inq_dimlen(ZnetID, azDimID, &azNum);
  if ( _checkStatus(status, "Failed to get az dimension.")){
    cerr << "Z file was " << zFile << endl;
    nc_close( ZnetID );
    if (haveU) nc_close( UnetID );
    return;
  }

  if (azNum != 360){
    cerr << "WARNING : Expected 360 azimuths, got " << azNum << endl;
  }


  if (haveU){
    //
    // Check that U is the same in azimuth (it has to be)
    //
    status = nc_inq_dimid(UnetID, "Azimuth", &azDimID);
    if ( _checkStatus(status, "Failed to get U az dimension ID.")){
      cerr << "Z file was " << zFile << endl;
      nc_close( ZnetID );
      if (haveU) nc_close( UnetID );
      return;
    }
    
    size_t UazNum;
    status = nc_inq_dimlen(UnetID, azDimID, &UazNum);
    if ( _checkStatus(status, "Failed to get U az dimension.")){
      cerr << "Z file was " << zFile << endl;
      nc_close( ZnetID );
      if (haveU) nc_close( UnetID );
      return;
    }

    if (azNum != UazNum){
      cerr << "ERROR : azimuth number mismatch " << UazNum << " vs " << azNum << endl;
      cerr << "Z file was " << zFile << endl;
      nc_close( ZnetID );
      if (haveU) nc_close( UnetID );
      return;
    }
  }


  //
  // Get some other information
  //
  float lat, lon, elev;
  elev = 0.0; lat=0.0; lon = 0.0; // Just to avoid compiler warnings
  int varID;
  char radarName[32];
  sprintf(radarName, "Swiss");

  if (_params->useLocationChars){
   
    bool gotLoc = false;
    char locChar = zFile[strlen(zFile)-strlen("AP_070010000_I.nc")];
   
    if (locChar == 'A'){
      lat=47.28540; lon=8.51301; elev = 930; gotLoc = true;
      sprintf(radarName, "Albis");
    }
   
    if (locChar == 'D'){
      lat=46.42616; lon=6.10016; elev = 1680; gotLoc = true;
      sprintf(radarName, "Dole");
    }
   
    if (locChar == 'L'){
      lat=46.04179; lon=8.83436; elev = 1630; gotLoc = true;
      sprintf(radarName, "Lema");
    }


    if (!(gotLoc)){
      cerr << "Did not recognize location character in filename : ";
      cerr << locChar << endl;
      return;
    }

    elev = elev / 1000.0; // Change units to Km

  } else {

    status = nc_inq_varid(ZnetID, "Latitude", &varID);
    if ( _checkStatus(status, "Failed to get lat variable ID")){
      cerr << "Z file was " << zFile << endl;
      nc_close( ZnetID );
      if (haveU) nc_close( UnetID );
      return;
    }
   
    status = nc_get_var_float(ZnetID, varID, &lat);
    if ( _checkStatus(status, "Failed to read lat")){
      cerr << "Z file was " << zFile << endl;
      nc_close( ZnetID );
      if (haveU) nc_close( UnetID );
      return;
    }
   

    status = nc_inq_varid(ZnetID, "Longitude", &varID);
    if ( _checkStatus(status, "Failed to get lon variable ID")){
      cerr << "Z file was " << zFile << endl;
      nc_close( ZnetID );
      if (haveU) nc_close( UnetID );
      return;
    }
    
    status = nc_get_var_float(ZnetID, varID, &lon);
    if ( _checkStatus(status, "Failed to read lon")){
      cerr << "Z file was " << zFile << endl;
      nc_close( ZnetID );
      if (haveU) nc_close( UnetID );
      return;
    }

  }


  float el; // Antenna elevation in degrees, not height above ground.
  status = nc_inq_varid(ZnetID, "Elevation", &varID);
  if ( _checkStatus(status, "Failed to get elevation variable ID")){
    cerr << "Z file was " << zFile << endl;
    nc_close( ZnetID );
    if (haveU) nc_close( UnetID );
    return;
  }

  status = nc_get_var_float(ZnetID, varID, &el);
  if ( _checkStatus(status, "Failed to read el")){
    cerr << "Z file was " << zFile << endl;
    nc_close( ZnetID );
    if (haveU) nc_close( UnetID );
    return;
  }

  //
  // Send an EOV if the elevation has decreased, if desired.
  //
  if (el < _lastTiltEl){
    if (_params->sendEOVonTiltDecrease){
      sendEOV();
      if (_params->debug) cerr << "Tilt angle decreased, EOV sent" << endl;
    }
  }

  _lastTiltEl = el;


  if (_params->debug){
    cerr << "Data are " << maxRangeNum << " in range, " << azNum << " in azimuth" << endl;
    cerr << "Radar location is " << lat << ", " << lon << " elevation angle " << el;
    cerr << " radar is " << radarName << endl;
  }

  float *zData = (float *) malloc(sizeof(float)*ZrangeNum*azNum);
  float *uData = (float *) malloc(sizeof(float)*UrangeNum*azNum);

  if ((zData == NULL) || (uData == NULL)){
    cerr << "Malloc failed!" << endl;
    exit(-1);
  }

  status = nc_inq_varid(ZnetID, "Datafield", &varID);
  if ( _checkStatus(status, "Failed to get Z datafield variable ID")){
    cerr << "Z file was " << zFile << endl;
    nc_close( ZnetID );
    if (haveU) nc_close( UnetID );
    return;
  }

  status = nc_get_var_float(ZnetID, varID, zData);
  if ( _checkStatus(status, "Failed to read zData")){
    cerr << "Z file was " << zFile << endl;
    nc_close( ZnetID );
    if (haveU) nc_close( UnetID );
    return;
  }


  if (haveU){
    status = nc_inq_varid(UnetID, "Datafield", &varID);
    if ( _checkStatus(status, "Failed to get U datafield variable ID")){
      cerr << "Z file was " << zFile << endl;
      nc_close( ZnetID );
      if (haveU) nc_close( UnetID );
      return;
    }

    status = nc_get_var_float(UnetID, varID, uData);
    if ( _checkStatus(status, "Failed to read uData")){
      cerr << "Z file was " << zFile << endl;
      haveU = false;
      for (unsigned iu=0; iu < UrangeNum*azNum; iu++){
	uData[iu] = MISSING_FLOAT;
      }
    }
  } else {
    for (unsigned iu=0; iu < UrangeNum*azNum; iu++){
      uData[iu] = MISSING_FLOAT;
    }
  }

  //
  // Close the netCDF files
  //
  nc_close(ZnetID);  nc_close(UnetID);

  //
  // Threshold the dbz, if required.
  //
  if (_params->lowerLimit.applyLowerLimit){
    for (unsigned idbz=0; idbz < ZrangeNum*azNum; idbz++){
      if (zData[idbz] < _params->lowerLimit.lowerLimit){
	zData[idbz] = MISSING_FLOAT;
      }
    }
  }


  //
  // Record this time as the time to save data, if appropriate.
  //
  if (_firstTilt){
    _firstTilt = false;
    _saveTime = dataTime;
  }

  if (tiltNum == 0) _saveTime = dataTime;

  //
  // Now that we know we have data, start writing to the FMQ.
  // Send start of tilt flag.
  //
  DsRadarFlags& radarFlags = _radarMsg.getRadarFlags();
  radarFlags.startOfVolume = false;
  radarFlags.startOfTilt   = true;
  radarFlags.endOfTilt     = false;
  radarFlags.endOfVolume   = false;

  int content = DsRadarMsg::RADAR_FLAGS;
  if( _radarQueue.putDsMsg( _radarMsg, content ) != 0 ) {
    cerr << " Failed to send radar flags." << endl;
    exit(-1);
  }


  //
  // Loop through azimuths.
  //
  for (unsigned iaz=0; iaz < azNum; iaz++){
    //
    // Is it time to send the radar params?
    //
    if (iaz % _params->outputFmq.fmqBeamsPerRadarParams == 0){

      DsRadarParams& radarParams = _radarMsg.getRadarParams();

      radarParams.radarId = 42;
      radarParams.numFields = 2;

      radarParams.radarName = radarName;
      radarParams.latitude = lat;
      radarParams.longitude = lon;
      radarParams.altitude = elev;
       
      radarParams.numGates = maxRangeNum;
      radarParams.gateSpacing = 1; // 1 Km
      radarParams.startRange = 1;
      radarParams.horizBeamWidth = 1;
      radarParams.vertBeamWidth = 1;
      radarParams.unambigVelocity = 0.0;
      //
      // Send the radar and field params.
      //
      content =  DsRadarMsg::RADAR_PARAMS;
      if( _radarQueue.putDsMsg(_radarMsg, content ) != 0 ) {
	cerr << "Failed to send radar param structure" << endl;
	exit(-1);
      }

      vector< DsFieldParams* >& fieldParams = _radarMsg.getFieldParams();
      fieldParams.clear();

      for(int ifld=0; ifld < 2; ifld++){
	fieldParams.push_back( _fieldParams[ifld] );
      }

      content =  DsRadarMsg::FIELD_PARAMS;
      if( _radarQueue.putDsMsg(_radarMsg, content ) != 0 ) {
	cerr << "Failed to send field params" << endl;
	exit(-1);
      }
    } // End of if we are sending radar params.

    //
    // Sens the beam.
    //
    DsRadarBeam& radarBeam = _radarMsg.getRadarBeam();

    radarBeam.dataTime   = _saveTime;

    radarBeam.volumeNum  = _volNum;
    radarBeam.azimuth    = double(iaz);
    radarBeam.elevation  = el;
    radarBeam.targetElev = radarBeam.elevation;

    float *beamData = (float *)malloc(2*maxRangeNum*sizeof(float));
    if (beamData == NULL){
      cerr << "malloc failed!" << endl;
      exit(-1);
    }


    unsigned int numMissing = 0;
    for (unsigned igate=0; igate < maxRangeNum; igate++){

      if (igate < ZrangeNum){
	beamData[2*igate] = zData[iaz*ZrangeNum + igate];
      } else {
	beamData[2*igate] = MISSING_FLOAT;
      }

      if (igate < UrangeNum){
	beamData[2*igate+1] = uData[iaz*UrangeNum + igate];
      } else {
	beamData[2*igate+1] = MISSING_FLOAT;
      }

      if ( beamData[2*igate] < -500.0) beamData[2*igate] = MISSING_FLOAT;
      if ( beamData[2*igate+1] < -500.0) beamData[2*igate+1] = MISSING_FLOAT;

      if (( beamData[2*igate] == MISSING_FLOAT) && (beamData[2*igate+1] == MISSING_FLOAT)) numMissing++;

    }

    radarBeam.loadData( beamData, 2*maxRangeNum*DATA_BYTE_WIDTH, DATA_BYTE_WIDTH );

    free(beamData);

    content = DsRadarMsg::RADAR_BEAM;
    if( _radarQueue.putDsMsg(_radarMsg, content ) != 0 ) {
      cerr << "Failed to send beam data." << endl;
      exit(-1);
    }
  }

  //
  // Done writing, send end of tilt flag.
  //
  radarFlags.startOfVolume = false;
  radarFlags.startOfTilt   = false;
  radarFlags.endOfTilt     = true;
  radarFlags.endOfVolume   = false;

  content = DsRadarMsg::RADAR_FLAGS;
  if( _radarQueue.putDsMsg( _radarMsg, content ) != 0 ) {
    cerr << " Failed to send radar flags." << endl;
    exit(-1);
  }

  free(uData); free(zData);

  //
  // Call a script after the tilt, if desired.
  //
  if (_params->postTilt.callScriptPostTilt){
    int retVal = system(_params->postTilt.postTiltScript);
    if (retVal){
      cerr << "WARNING : " << _params->postTilt.postTiltScript << " returned " << retVal << endl;
    }
  }


  return;
}

// Send an end of volume to the FMQ.
void lassenNetcdf2Dsr::sendEOV(){


  _volNum++;
  if (_volNum == 100) _volNum = 0;

  //
  // Send end, and then start, of volume.
  // Start and end of tilt sent elsewhere, in other method.
  //
  DsRadarFlags& radarFlags = _radarMsg.getRadarFlags();
  radarFlags.startOfVolume = false;
  radarFlags.startOfTilt   = false;
  radarFlags.endOfTilt     = false;
  radarFlags.endOfVolume   = true;

  int content = DsRadarMsg::RADAR_FLAGS;
  if( _radarQueue.putDsMsg( _radarMsg, content ) != 0 ) {
    cerr << " Failed to send radar flags." << endl;
    exit(-1);
  }

  radarFlags.startOfVolume = true;
  radarFlags.startOfTilt   = false;
  radarFlags.endOfTilt     = false;
  radarFlags.endOfVolume   = false;

  content = DsRadarMsg::RADAR_FLAGS;
  if( _radarQueue.putDsMsg( _radarMsg, content ) != 0 ) {
    cerr << " Failed to send radar flags." << endl;
    exit(-1);
  }
  
  if (_params->postVol.callScriptPostVol){
    int retVal = system(_params->postVol.postVolScript);
    if (retVal){
      cerr << "WARNING : " << _params->postVol.postVolScript << " returned " << retVal << endl;
    }
  }

  sleep (_params->postVol.postVolSleepSecs);

  return;
}

// Destructor.
lassenNetcdf2Dsr::~lassenNetcdf2Dsr (){
  return;
}


//
// The following is a small method that exits if things
// have gone wrong with a netCDF read. An error string is printed first.
//
int lassenNetcdf2Dsr::_checkStatus(int status, char *exitStr){

  if (status != NC_NOERR){
    cerr << exitStr << endl;
    return -1;
  }

  return 0;

}



