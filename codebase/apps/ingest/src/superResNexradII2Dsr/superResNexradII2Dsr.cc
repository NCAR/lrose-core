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
// superResNexradII2Dsr.cc
//
// Inits FMQ in constructor. ProcVols() method processes volumes.
//
// Niles Oien, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
//
/////////////////////////////////////////////////////////////

#include <cmath>

#include <toolsa/pmu.h>
#include <toolsa/uusleep.h>

#include "superResNexradII2Dsr.hh"
#include "basicIO.hh"
#include "msg31beamData.hh"

using namespace std;

const fl32 superResNexradII2Dsr::MISSING_FLOAT = -9999.0;
const fl32 superResNexradII2Dsr::SCALE = 1.0;
const fl32 superResNexradII2Dsr::BIAS = 0.0;  

// Constructor. Inits output FMQ.
superResNexradII2Dsr::superResNexradII2Dsr ( Params *TDRP_params ){
  _params = TDRP_params;
  _vcpNum = -1;
  _volNum = -1;
  _resetInternalFlags();

  _elNum = 0;
  _outputTiltNum = 0;
  _lastBeamTimeSent = 0L;

  _lastMessageType = -1;

  _lat = _lon = _altKm = -999.0;
  _currentNyquistVel = _nyquistVel = 0.0;

  //
  // Enable debug level messaging
  //
  if ( _params->debug >= Params::DEBUG_DATA ){
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
			"superResNexradII2Dsr",
			_params->debug >= Params::DEBUG_DATA,           
			DsFmq::READ_WRITE, DsFmq::END,
			_params->outputFmq.fmqCompress,
			_params->outputFmq.fmqNumSlots,
			_params->outputFmq.fmqSizeBytes, 1000,
			&_msgLog )) {
    cerr << "Could not initialize fmq " << _params->outputFmq.fmqUrl << endl; 
    exit(-1);
  }

  //
  // Use blocking writes, if desired.
  //
  if ( _params->outputFmq.fmqBlockingWrites ){
    if (_params->debug >= Params::DEBUG_NORM){
      cerr << "Using blocking writes." << endl;
    }
    if ( _radarQueue.setBlockingWrite()){
      cerr << "Failed to set blocking write!" << endl;
      exit(-1);
    }
  }


  _numFields = 0;

  if (_params->fieldList.wantDBZ){
    _fieldParams[_numFields] = new DsFieldParams("DBZ", "dbz",
						 SCALE,
						 BIAS,
						 DATA_BYTE_WIDTH, (int)MISSING_FLOAT);
    _numFields++;
  }

  if (_params->fieldList.wantVEL){
    _fieldParams[_numFields] = new DsFieldParams("VEL", "m/s",
						 SCALE,
						 BIAS,
						 DATA_BYTE_WIDTH, (int)MISSING_FLOAT);
    _numFields++;
  }

  if (_params->fieldList.wantSW){
    _fieldParams[_numFields] = new DsFieldParams("SW", "m/s",
						 SCALE,
						 BIAS,
						 DATA_BYTE_WIDTH, (int)MISSING_FLOAT);
    _numFields++;
  }

  if (_params->fieldList.wantZDR){
    _fieldParams[_numFields] = new DsFieldParams("ZDR", "dbz",
						 SCALE,
						 BIAS,
						 DATA_BYTE_WIDTH, (int)MISSING_FLOAT);
    _numFields++;
  }

  if (_params->fieldList.wantPHI){
    _fieldParams[_numFields] = new DsFieldParams("PHI", "deg",
						 SCALE,
						 BIAS,
						 DATA_BYTE_WIDTH, (int)MISSING_FLOAT);
    _numFields++;
  }

  if (_params->fieldList.wantRHO){
    _fieldParams[_numFields] = new DsFieldParams("RHO", "none",
						 SCALE,
						 BIAS,
						 DATA_BYTE_WIDTH, (int)MISSING_FLOAT);
    _numFields++;
  }
  

  if (_params->debug >= Params::DEBUG_NORM){
    cerr << _numFields << " fields selected." << endl;
  }

  //
  // Get the number of gates we are resampling to.
  //

  _nGates = (int)rint((_params->resample.lastGateRangeKm - _params->resample.firstGateRangeKm)/_params->resample.gateSpacingKm);
  _nAz = (int)rint(360.0/_params->resample.azIncDeg) + 1;

  if (_params->debug >= Params::DEBUG_HEADERS){
    cerr << "Resampling beams to " << _nGates << " gates, first gate at " << _params->resample.firstGateRangeKm;
    cerr << "km, spacing " << _params->resample.gateSpacingKm << "km" << endl;
    cerr << _nAz << " azimuths will be buffered for an az increment of " << _params->resample.azIncDeg  << " degrees" << endl;
  }

  //
  // One great thing about knowing the geometry up front is that
  // we can allocate the space for beam data right now and free it in the destructor.
  //
  _beamData = (fl32 *) malloc(sizeof(fl32)*_nGates*_numFields*_nAz);
  if (_beamData == NULL){
    cerr << "Failed to allocate space for beam data" << endl;
    exit(-1); // Unlikely.
  }

  _beamMetaData = (beamMetaData_t *) malloc(sizeof(beamMetaData_t) * _nAz);
  if (_beamMetaData == NULL){
    cerr << "Failed to allocate space for beam meta data" << endl;
    exit(-1); // Unlikely.
  }

  _initTiltToMissing();

  return;
}


void superResNexradII2Dsr::_initTiltToMissing(){
  for (int k=0; k < _nGates*_numFields*_nAz; k++){
    _beamData[k] = MISSING_FLOAT;
  }

  beamMetaData_t bmd;
  bmd.beamTime = 0L; bmd.beamElev = MISSING_FLOAT;
  for (int k=0; k < _nAz; k++){
    _beamMetaData[k] = bmd;
  }

  return;
}


// Process volumes.
int superResNexradII2Dsr::procVols( char *Filename ){

  //
  // Init the basic IO object as appropriate for
  // where we are reading from.
  //
  basicIO *io=NULL;
  if (_params->mode == Params::READ_SOCKET){
    io = new basicIO(_params->socketInput.socketHostName,
		     _params->socketInput.socketPortNum,
		     _params);
  } else {
    io = new basicIO( Filename, _params );

    _filenameTime.unix_time = 0L;
    if (6 == sscanf( Filename + strlen(Filename) - strlen("20080208_203504.nexDat"),
		     "%4d%2d%2d_%2d%2d%2d", &_filenameTime.year, &_filenameTime.month, &_filenameTime.day, 
		     &_filenameTime.hour, &_filenameTime.min, &_filenameTime.sec)){
      uconvert_to_utime( &_filenameTime);

    } else {
       // Cope with the KTLX20080208_203504... file name convention.
  	   if (6 == sscanf( Filename + strlen(Filename) - strlen("20080624_211726_V03"),
			 "%4d%2d%2d_%2d%2d%2d", &_filenameTime.year, &_filenameTime.month, &_filenameTime.day,
			  &_filenameTime.hour, &_filenameTime.min, &_filenameTime.sec)){
			uconvert_to_utime( &_filenameTime);
      }
	}

  }

  do {

    //
    // It seems that at any time, 12 bytes set to 0 may be inserted into the stream between
    // messages. So read 12 bytes into the 16 byte header. If they are all 0, skip them,
    // and then read the next 16 bytes into the header. If not, read the remaining 4 bytes.
    //

    header_t msgHeader;
    io->readBytes(12, (unsigned char *) &msgHeader);
    if (!(io->isOk())) break;

    bool areAll0 = true;
    ui08 *bPtr = (ui08 *) &msgHeader;
    for (int ik=0; ik < 12; ik ++){
      if (bPtr[ik] != 0){
	areAll0 = false;
	break;
      }
    }


    if (areAll0){
      //
      // We just skipped 12 null bytes, read the next header.
      //
      io->readBytes(sizeof(superResNexradII2Dsr::header_t), (unsigned char *) &msgHeader);
      if (!(io->isOk())) break;
    } else {
      //
      // We just read 12 valid bytes, read the remaining 4.
      //
      io->readBytes(sizeof(superResNexradII2Dsr::header_t)-12, ((unsigned char *) &msgHeader) + 12);
      if (!(io->isOk())) break;
    }

    //
    // Special case - volume headers start with a string, typically "AR", 
    // indicating archive header - if that
    // is the case we have a 24 byte header. If this is the
    // case then since we have already read the 16 byte message header we
    // need to skip the next 8 bytes to make the total of 24 bytes.
    //
    if (0==strncmp((char *) &msgHeader, _params->archiveString, strlen(_params->archiveString))){
      
      if (_params->debug >= Params::DEBUG_HEADERS){
	cerr << _params->archiveString << " header detected." << endl;
      }
      _volumeHeader((unsigned char *) &msgHeader);
      //
      // Do an 8 byte read that will be disregarded
      //
      io->readBytes(8, (unsigned char *) &msgHeader);
      if (!(io->isOk())) break;

      //
      // Implied that we should send an end/start of volume with ARCHIVE header.
      //
      if (_params->debug >= Params::DEBUG_NORM){
	cerr << "Sending start of volume and tilt." << endl;
      }

      _outputTiltNum = 0;

      if(_radarQueue.putStartOfVolume(_volNum,_lastBeamTimeSent)) {  
        cerr << " Failed to send start of volume flag." << endl;
        return(-1);
      }
      if(_radarQueue.putStartOfTilt(_outputTiltNum,_lastBeamTimeSent)) {  
        cerr << " Failed to send start of tilt flag." << endl;
        return(-1);
      }

      continue;

    }

    //
    // If we got here we have a valid message header, byte swap the header elements
    // for use on a PC and print it.
    //
    _bSwap( &msgHeader.size );
    _bSwap( &msgHeader.msgSeqNum );
    _bSwap( &msgHeader.jDate );
    _bSwapLong(  &msgHeader.numSecMidnight);
    _bSwap( &msgHeader.totalMsgSegs );
    _bSwap( &msgHeader.msgSegNum );

    //
    // Get the number of bytes to read - the msgHeader.size entry is in
    // half word (two byte) increments. If it is not message type 31,
    // then the resulting size in bytes should be rounded up to the
    // next integer increment of 2416 bytes.
    //
    // If it is message type 31, then there is no need to round the
    // number of bytes to read up - it can be anything. It seems like
    // the number of bytes indicated in the message header is 4 bytes
    // more than it should be in this case. This is parameterized so that
    // if the RPG ever fixes it we can remove it without a recompile.
    //
    int bytesToRead = 2*msgHeader.size;

    if (msgHeader.msgType != 31)
      bytesToRead = 2416*(int)ceil(double(bytesToRead)/2416.0);
    else
      bytesToRead = bytesToRead - _params->message31SizeError;

    if (bytesToRead == 0) bytesToRead = 2416;

    //
    // Another oddity - if we get two type 2 messages in a row
    // then we find ourselves reading 12 bytes too many.
    //

    if ((msgHeader.msgType == 2) && (_lastMessageType == 2)){
      bytesToRead = bytesToRead - 12;
    }

    _lastMessageType = msgHeader.msgType;

    //
    // Get the message time.
    //
    date_time_t msgTime;
    _getTime(msgHeader.jDate, msgHeader.numSecMidnight, &msgTime);

    if (_params->debug >= Params::DEBUG_HEADERS){
      //
      // Print some elements in the message header.
      //
      cerr << "Message type " << (int) msgHeader.msgType << " size (bytes) " << bytesToRead << " at " << utimstr(msgTime.unix_time) << endl;
    }

    //
    // Allocate space for the actual message and read it.
    //
    unsigned char *buf = (unsigned char *) malloc(sizeof(unsigned char) * bytesToRead);
    if (buf == NULL){
      cerr << "Allocation failed!" << endl;
      return -1;
    }

    io->readBytes(bytesToRead, buf);
    if (!(io->isOk())) break;

    _resetInternalFlags();

    //
    // We got data - process the messages we care about - 2 and 31
    //
    if (msgHeader.msgType == 31){
      _message31(buf, bytesToRead);
    }
    

    if (io->atEnd()) {
      //
      // We're at the end of the data - maybe EOF? - set flags to flush buffers.
      //
      _sendElevEnd = true;
      _sendVolEnd = true;
    }

    free(buf);

    //
    // If we are at the end of an elevation, see if we are ready to
    // flush our buffer and send a tilt.
    //
    if (_sendElevEnd){

      bool itsGoTime = false;
      for (unsigned k=0; k < _writeAfterTheseElevs.size(); k++){
	if (_writeAfterTheseElevs[k] == _elNum){
	  itsGoTime = true; break;
	}
      }
      if (itsGoTime){

	if (_params->debug >= Params::DEBUG_NORM){
	  cerr << "Sending a tilt." << endl;
	}

	//
	// Time to send a tilt.
	//
	DsRadarFlags& radarFlags = _radarMsg.getRadarFlags();      
	radarFlags.startOfVolume = false;
	radarFlags.startOfTilt   = true;
	radarFlags.endOfTilt     = false;
	radarFlags.endOfVolume   = false;
	radarFlags.volumeNum     = _volNum;
	radarFlags.tiltNum       = _outputTiltNum;
	radarFlags.time = _lastBeamTimeSent;

	int content = DsRadarMsg::RADAR_FLAGS;
	if( _radarQueue.putDsMsg( _radarMsg, content ) != 0 ) {  
	  cerr << " Failed to send radar flags." << endl;
	  return(-1);
	}
	
	for (int iaz=0; iaz < _nAz; iaz++){

	  //
	  // Is it time to send the radar params?
	  //
	  if (iaz % _params->outputFmq.fmqBeamsPerRadarParams == 0){

	    DsRadarParams& radarParams = _radarMsg.getRadarParams();
	    
	    radarParams.radarId = _params->radarDesc.radarID;
	    radarParams.numFields = _numFields;
	    
	    radarParams.radarName = _params->radarDesc.radarName;
	    if (_params->locationFromInput){
	      radarParams.latitude = _lat;
	      radarParams.longitude = _lon;
	      radarParams.altitude = _altKm;
	    } else {
	      radarParams.latitude = _params->radarDesc.lat;
	      radarParams.longitude = _params->radarDesc.lon;
	      radarParams.altitude = _params->radarDesc.altKm;
	    }
	    radarParams.numGates = _nGates;
	    radarParams.gateSpacing = _params->resample.gateSpacingKm;
	    radarParams.startRange = _params->resample.firstGateRangeKm;
	    radarParams.horizBeamWidth = _params->radarDesc.horizBeamWidth;
	    radarParams.vertBeamWidth = _params->radarDesc.vertBeamWidth;
	    radarParams.unambigVelocity = _nyquistVel;
	    //
	    // Send the radar and field params. Optionally delay first.
	    //
	    if (_params->mode != Params::READ_SOCKET){
	      for (int id=0; id < _params->outputFmq.delayAfterSendingParamsSecs; id++){
		PMU_auto_register("Delaying");
		umsleep(1000);
	      }
	    }

            // add fields

	    vector< DsFieldParams* >& fieldParams = _radarMsg.getFieldParams();
	    fieldParams.clear();
	    for(int ifld=0; ifld <  _numFields; ifld++){
	      fieldParams.push_back( _fieldParams[ifld] );
	    }

            // put radar params and field params together
            // because nfields in radar params refers to number
            // of fields in field params
            
	    content =  DsRadarMsg::RADAR_PARAMS | DsRadarMsg::FIELD_PARAMS;
	    if( _radarQueue.putDsMsg(_radarMsg, content ) != 0 ) {  
	      cerr << "Failed to send radar and field params" << endl;
	      return(-1);
	    }
            
	    if (_params->debug >= Params::DEBUG_NORM){
	      cerr << "Sent radar params with Nyquist velocity "
                   << _nyquistVel << " m/s ";
	      cerr << "location " << radarParams.latitude << ", ";
	      cerr << radarParams.longitude << ", ";
	      cerr << radarParams.altitude << endl;
	    }

	  } // End of if we are sending radar params.

	  //
	  // Send an actual beam, if there are data at this azimuth.
	  //
	  if (_beamMetaData[iaz].beamElev != MISSING_FLOAT){

	    //
	    // See if we have data.
	    //
	    bool sendBeam = true;
	    bool anyDataThisBeam = false;
	    //
	    for (int iField = 0; iField < _numFields; iField++){
	      bool haveDataThisField = false;
	      for (int iGate=0; iGate < _nGates; iGate++){
		if (_beamData[iaz * _nGates * _numFields + 
			      iGate * _numFields + iField] != MISSING_FLOAT){
		  haveDataThisField = true;
		  break;
		}
	      }

	      if (haveDataThisField) anyDataThisBeam = true;

	      if ( haveDataThisField && (!(_params->requireAllFieldsInBeam))){
		sendBeam = true;
		break;
	      }

	      if ( !(haveDataThisField) && (_params->requireAllFieldsInBeam)){
		sendBeam = false;
		break;
	      }
	    }

	    if ( anyDataThisBeam && sendBeam ) {
	      //
	      // We have data in this beam, send it.
	      //
  	      DsRadarBeam& radarBeam = _radarMsg.getRadarBeam();
   
	      radarBeam.dataTime   = _beamMetaData[iaz].beamTime;
	      _lastBeamTimeSent = radarBeam.dataTime;

	      radarBeam.volumeNum  = _volNum;
	      radarBeam.tiltNum    = _outputTiltNum;
	      radarBeam.azimuth    = double(iaz) * _params->resample.azIncDeg;
	      radarBeam.elevation  = _beamMetaData[iaz].beamElev;
	      radarBeam.targetElev = radarBeam.elevation;
      
	      if (_params->reportAllMissing){
		bool allMissing = true;
		fl32 *thisBeamData = _beamData + iaz * _nGates * _numFields;

		for (int imb=0; imb < _numFields*_nGates; imb++){
		  if (thisBeamData[imb] != MISSING_FLOAT){
		    allMissing = false;
		    break;
		  }
		}
		if (allMissing){
		  cerr << "INFO - a beam with all missing data values was sent." << endl;
		}
	      }
	      //
	      // See if this azimuth is in a range we have said we don't want.
              bool azOk = true;
              for (int k=0; k < _params->excludeAzRanges_n; k++){ 
		if ((radarBeam.azimuth > _params->_excludeAzRanges[k].minAz) &&
		    (radarBeam.azimuth < _params->_excludeAzRanges[k].maxAz)){
		  azOk = false;
		  break;
		}
              }

              if (azOk){

		radarBeam.loadData( _beamData + iaz * _nGates * _numFields,
				    _numFields*_nGates*DATA_BYTE_WIDTH, 
				    DATA_BYTE_WIDTH ); 
     
		content = DsRadarMsg::RADAR_BEAM;  
		if( _radarQueue.putDsMsg(_radarMsg, content ) != 0 ) {  
		  cerr << "Failed to send beam data." << endl;
		  return(-1);
		}
		if (_params->delayBeam.delayPostBeam){
		  umsleep(_params->delayBeam.msDelayPostBeam);
		}
	      }
	    }
	  }

	} // End of loop through azimuths

	// Reset our buffer.
       	_initTiltToMissing();

	//
	// Send end of tilt
	//
	radarFlags.startOfVolume = false;
	radarFlags.startOfTilt   = false;
	radarFlags.endOfTilt     = true;
	radarFlags.endOfVolume   = false;
	radarFlags.volumeNum     = _volNum;
	radarFlags.tiltNum       = _outputTiltNum;
	radarFlags.time = _lastBeamTimeSent;

	content = DsRadarMsg::RADAR_FLAGS;
	if( _radarQueue.putDsMsg( _radarMsg, content ) != 0 ) {  
	  cerr << " Failed to send radar flags." << endl;
	  return(-1);
	}

	_outputTiltNum++;

	//
	// If this is the last tilt, send an end of volume.
	//
	if ((_writeAfterTheseElevs.size() > 0) && (_writeAfterTheseElevs[_writeAfterTheseElevs.size()-1] == _elNum)){

	  radarFlags.startOfVolume = false;
	  radarFlags.startOfTilt   = false;
	  radarFlags.endOfTilt     = false;
	  radarFlags.endOfVolume   = true;
	  radarFlags.volumeNum     = _volNum;
	  radarFlags.tiltNum       = _outputTiltNum;
	  radarFlags.time = _lastBeamTimeSent;
		
	  int content = DsRadarMsg::RADAR_FLAGS;
	  if( _radarQueue.putDsMsg( _radarMsg, content ) != 0 ) {  
	    cerr << " Failed to send radar flags." << endl;
	    return(-1);
	  }

	  _outputTiltNum=0;

	  if (_params->mode != Params::READ_SOCKET){
	    for (int is=0; is < _params->delay.delayPostVolSecs; is++){
	      PMU_auto_register("Delaying"); umsleep(1000);
	    }
	  }

	} else {
	  //
	  // End of tilt but not end of volume. Delay appropriately.
	  //
	  if (_params->mode != Params::READ_SOCKET){
	    for (int is=0; is < _params->delay.delayPostTiltSecs; is++){
	      PMU_auto_register("Delaying"); umsleep(1000);
	    }
	  }
	}
      }
    }

  } while (io->isOk());

  delete io;

  // Send end of vol flag

  if (_params->debug >= Params::DEBUG_NORM){
    cerr << "Sending end of volume." << endl;
  }

  if(_radarQueue.putEndOfVolume(_volNum,_lastBeamTimeSent)) {  
    cerr << " Failed to send radar flags." << endl;
    return(-1);
  }

  return 0;
}

// Destructor.
superResNexradII2Dsr::~superResNexradII2Dsr(){
  free(_beamData);
  free(_beamMetaData);
  return;
}

//
// Small routines to do byte swapping.
//
void superResNexradII2Dsr::_bSwap(void *i){

  if (!(_params->byteSwap)) return;

  unsigned char *bPtr = (unsigned char *) i;
  unsigned char tmp;
  tmp = *bPtr;
  *bPtr = *(bPtr+1);
  *(bPtr+1) = tmp;
  return;
}

void superResNexradII2Dsr::_bSwapLong(void *i){

  if (!(_params->byteSwap)) return;

  unsigned char *bPtr = (unsigned char *) i;
  unsigned char b0 = *bPtr;
  unsigned char b1 = *(bPtr+1);
  unsigned char b2 = *(bPtr+2);
  unsigned char b3 = *(bPtr+3);

  *bPtr = b3;
  *(bPtr+1) = b2;
  *(bPtr+2) = b1;
  *(bPtr+3) = b0;
 
  return;
}


void superResNexradII2Dsr::_message31(unsigned char *buf, int bufSize){

  if (_params->debug >= Params::DEBUG_NORM){
    cerr << "Message 31 ICAO : ";
    for (int k=0; k < 4; k++){
      cerr << (char)buf[k];
    }
    cerr << endl;
  }

  ui32 *msecPastMidnight = (ui32 *) &buf[4];
  _bSwapLong(msecPastMidnight);

  si16 *jDate = (si16 *) &buf[8];
  _bSwap(jDate);

  si16 *azNum = (si16 *) &buf[10];
  _bSwap(azNum);

  fl32 *az = (fl32 *) &buf[12];
  _bSwapLong((ui32 *) az);


  int compressionIndicator = (int)buf[16];

  if (_params->debug >= Params::DEBUG_NORM){
    switch ( compressionIndicator ) {
      
    case 0 :
      cerr << "Uncompressed" << endl;
      break;

    case 1 :
      cerr << "BZIP2 compression" << endl;
      break;

    case 2 :
      cerr << "ZLIB compression" << endl;
      break;

    default :
      cerr << "Compression indicator " << compressionIndicator << " not recognized!" << endl;
      break;
      
    }
  }


 si16 *radLen = (si16 *) &buf[18];
  _bSwap(radLen);

  unsigned char azSpacingFlag = buf[20];


  if (_params->debug >= Params::DEBUG_NORM){
    switch (azSpacingFlag) {

    case 1 :
      cerr << "0.5 deg az spacing" << endl;
      break;

    case 2 :
      cerr << "1.0 deg az spacing" << endl;
      break;

    default :
      cerr << "Unknown az spacing code " << azSpacingFlag << endl;
      break;

    }
  }

  unsigned char radStatus = buf[21];

  if (_params->debug >= Params::DEBUG_NORM){

    switch (radStatus) {

    case 0 :
      cerr << "Start of new elevation" << endl;
      break;

    case 1 :
      cerr << "Intermediate radial data" << endl;
      break;
    
    case 2 :
      cerr << "End of elevation" << endl;
      break;

    case 3 :
      cerr << "Start of volume" << endl;
      break;

    case 4 :
      cerr << "End of volume" << endl;
      break;

    default :
      cerr << "Bad radial status code " << radStatus << endl;
      break;

    }
  }


  int eleNum = (int)buf[22];

  switch (radStatus) {
    
  case 0 :
    _sendElevStart = true;
    break;
    
  case 2 :
    _sendElevEnd = true;
    break;

  case 3 :
    _sendElevStart = true;
    _sendVolStart = true;
    break;

  case 4 :
    _sendElevEnd = true;
    _sendVolEnd = true;
    break;
    
  default :
    break;

  }

  fl32 *el = (fl32 *) &buf[24];

  _bSwapLong((ui32 *) el);

  double elevToUse = 0.0;

  switch( _params->elevAction){

  case Params::ELEVATION_ASIS :
    elevToUse = *el;
    break;

  case Params::ELEVATION_ROUNDED :
    elevToUse = _params->roundElevDelta * (int)rint( *el / _params->roundElevDelta);
    break;

  case Params::ELEVATION_LOOKUP :
    elevToUse = _lookupElev( *el );
    break;

  default :
    cerr << "Unrecognized elevation action " << _params->elevAction << endl;
    exit(-1);
    break;

  }

  if (_params->debug >= Params::DEBUG_HEADERS){
    cerr << "Azimuth number " << *azNum << " at " << *az << " degrees, elevation number " << eleNum << " at " << *el << " degrees (using " << elevToUse << ")" << endl;
  }

  _az = *az; 

  //
  // If we are adding an offset to the azimuth, we need
  // to make sure we are in tyhe 0 -> 360 range and we
  // need to round the az to increments of the offset.
  //
  // I found out the hard way that this is necessary. Niles June 2009.
  //
  if (_params->azOffset != 0){
    _az = _params->azOffset * (int)rint( _az / _params->azOffset );
    _az +=  _params->azOffset;
    do {
      if (_az < 0.0) _az += 360.0;
      if (_az > 360.0) _az -= 360.0;
    } while ((_az < 0.0) || (_az > 360.0)); 
  }

  _el = elevToUse; _elNum = eleNum;

  unsigned char indexingMode = buf[29];


  if (_params->debug >= Params::DEBUG_NORM){

    switch ( indexingMode ) {

    case 0 :
      cerr << "No indexing" << endl;
      break;

    default :
      cerr << "Azimuth angles indexed to " << double(indexingMode)/100.0 << " degrees" << endl;
      break;

    }
  }

  si16 *dataBlockCount = (si16 *)&buf[30];
  _bSwap( dataBlockCount);

  if (_params->debug >= Params::DEBUG_NORM){
    cerr << *dataBlockCount << " data blocks found." << endl;
  }

  // Byte swap all the data block pointers. We need them
  // swapped so we can subtract them to work out the size.
  for (int idb=0; idb < *dataBlockCount; idb++){
    ui32 *dbPtr = (ui32 *) &buf[32 + 4 * idb];
    _bSwapLong( dbPtr );
  }


  for (int idb=0; idb < *dataBlockCount; idb++){

    //
    // Have to work out the size from the pointers.
    //
    int compressedSize = 0;
    if (idb < *dataBlockCount - 1){
      ui32 *thisDbPtr = (ui32 *) &buf[32 + 4 * idb];
      ui32 *nextDbPtr = (ui32 *) &buf[32 + 4 * (idb+1)];
      compressedSize = *nextDbPtr - *thisDbPtr;
    } else {
      ui32 *thisDbPtr = (ui32 *) &buf[32 + 4 * idb];
      compressedSize = bufSize - *thisDbPtr - 12; // 12 arrived at empirically
    }

    // Adjust for the 28 byte header on the data block.
    compressedSize -= 28;

    if (_params->debug >= Params::DEBUG_NORM){
      cerr << "Data block " << idb+1 << " of " << *dataBlockCount << " size " << compressedSize << " bytes" << endl;
    }

    //
    // Get the pointer to the data block and use it to init a message 31 type beam.
    //
    ui32 *dbPtr = (ui32 *) &buf[32 + 4 * idb];
    ui08 *dataBlock = NULL;
    if (*dbPtr != 0) dataBlock = &buf[ *dbPtr];
    
    msg31beamData msg31Data( _params, compressionIndicator, compressedSize, dataBlock);

    //
    // Use the VCP number to set up for the run
    //
    int newVCP = msg31Data.getVCP();
    if (_vcpNum != newVCP){
      _vcpNum = newVCP;

      DsRadarFlags& radarFlags = _radarMsg.getRadarFlags();      
      radarFlags.scanType = _vcpNum;
      int content = DsRadarMsg::RADAR_FLAGS;
      if( _radarQueue.putDsMsg( _radarMsg, content ) != 0 ) {  
	cerr << " Failed to send radar flags." << endl;
      }

      _readVCPinfoFile();
    }


    // Save off the nyquist velocity.

    if (msg31Data.getFieldName() == "RAD"){
      _currentNyquistVel = msg31Data.getNyquistVel();
      if (_currentNyquistVel < -900.0){
	_currentNyquistVel = 0.0;
      }
    }

    if (msg31Data.getFieldName() == "VOL"){
      if (
	  (_lat < -900.0) && 
	  (msg31Data.getLat() > -900.0)
	  ){
	_lat = msg31Data.getLat(); 
	_lon = msg31Data.getLon(); 
	_altKm = msg31Data.getAlt() / 1000.0;
      }    
    }

    if (!(msg31Data.isWanted())) continue;
    if (!(_isActive(eleNum, msg31Data.getFieldName()))) continue;

    //
    // If we're here and the field name is VEL then this is the velocity field we want.
    // Take the nyquist velocity from it.
    //
    if (msg31Data.getFieldName() == "VEL"){
      _nyquistVel = _currentNyquistVel;
    }


    msg31Data.resampleData(_params->resample.firstGateRangeKm, 
			   _params->resample.lastGateRangeKm, 
			   _params->resample.gateSpacingKm);

    int azIndex = (int)rint( _az / _params->resample.azIncDeg);
    if (azIndex > _nAz-1) azIndex = _nAz - 1;

    int fNum = msg31Data.getFieldNum();

    for (int ig=0; ig < _nGates; ig++){
      _beamData[azIndex * _nGates * _numFields + ig * _numFields + fNum ] = msg31Data.getGate(ig);
    }

    if (_beamMetaData[azIndex].beamElev == MISSING_FLOAT){
      //
      // No metadata, fill it in.
      //
      _beamMetaData[azIndex].beamElev = _el;

      date_time_t ralTime;

      switch ( _params->timeAction ){

      case Params::TIME_BEAM :
	_getTime(*jDate, *msecPastMidnight, &ralTime);
	_beamMetaData[azIndex].beamTime = ralTime.unix_time;
	break;

      case Params::TIME_FILENAME :
	if (_filenameTime.unix_time == 0){
	  cerr << "ERROR : Was unable to parse data time from input filename." << endl;
	  exit(-1);
	}
	_beamMetaData[azIndex].beamTime = _filenameTime.unix_time;
	break;

      case Params::TIME_WALLCLOCK :
	_beamMetaData[azIndex].beamTime = time(NULL);
	break;

      default :
	cerr << "Unrecognized timeAction " << _params->timeAction << endl;
	exit(-1);
	break;

      }
 
      _beamMetaData[azIndex].beamTime += _params->timeOffsetSecs;

    }

  }

  return;

}


void superResNexradII2Dsr::_readVCPinfoFile(){

  //
  // Read in the list of which fields to extract from which input
  // elevations for this VCP.
  //
  _elevSelections.clear();
  _writeAfterTheseElevs.clear();

  char vcpElevSelFilename[1024];
  sprintf(vcpElevSelFilename, _params->vcpElevSelectFilename, _vcpNum);

  FILE *vfp = fopen(vcpElevSelFilename, "r");
  if (vfp == NULL){
    cerr << "FATAL : Failed to open VCP field select file " << vcpElevSelFilename << endl;
    exit(-1);
  }

  char Line[1024];
  while (NULL != fgets(Line, 1024, vfp)){
    //
    // Skip blank lines, comments.
    //
    if (strlen(Line) < 5) continue;
    if (Line[0] == '#') continue;

    if (Line[strlen(Line)-1] == char(10))
      Line[strlen(Line)-1] = char(0);

    char *p = strtok(Line, " ");
    if (p == NULL) continue;
    char fieldName[32];
    int elevNum;
    int maxElevNum = -1;

    do {
      // fprintf(stderr, "Have token %s\n", p);
      int i=0;
      do {
	if (p[i]==char(0)) break;
	if ((p[i]==' ') || (p[i] == (char)(9))) continue;
	if (p[i]=='['){
	  sscanf(p+i,"[%d]", &elevNum);
	  break;
	}
	fieldName[i]=p[i];
	fieldName[i+1]=char(0);
	i++;
      } while(1);

	//  fprintf(stderr,"Field %s elevNum %d\n", fieldName, elevNum);

	elevSelect_t e;
	e.fieldName = fieldName;
	e.elevNum = elevNum;

	_elevSelections.push_back( e );

	if ((maxElevNum == -1) || (maxElevNum < elevNum)) maxElevNum = elevNum;

      p = strtok(NULL, " ");
    } while (p != NULL);

    _writeAfterTheseElevs.push_back( maxElevNum );

  }

  fclose(vfp);

  //
  // Read in the elevation range file for this VCP, if we
  // are using these files.
  //
  if (_params->elevAction == Params::ELEVATION_LOOKUP){

    _elevRanges.clear();

    char fileName[1024];
    sprintf(fileName, _params->vcpElevFilename, _vcpNum);

    FILE *fp = fopen(fileName,"r");
    if (fp == NULL){
      cerr << "Failed to open VCP elevation file " << fileName << endl;
      exit(-1); // Fatal, they need to set the file up.
    }

    superResNexradII2Dsr::elevRange_t e;

    char line[1024];
    while(NULL != fgets(line, 1024, fp)){
      if (3 == sscanf(line, "%lf %lf %lf", &e.min, &e.max, &e.targetElev)){
	_elevRanges.push_back( e );
      }
    }

    fclose(fp);

  }
  


  if (_params->debug >= Params::DEBUG_HEADERS){
    cerr << "VCP set to " << _vcpNum << endl;

    cerr << "Field makeup for this VCP : " << endl; 
    for (unsigned i=0; i < _elevSelections.size(); i++){
      cerr << "Field " << _elevSelections[i].fieldName;
      cerr << " will be taken from input elevation " << _elevSelections[i].elevNum << endl;
    }
    cerr << endl;

    cerr << "Output elevations will be sent after input elevations : ";
    for (unsigned k=0; k < _writeAfterTheseElevs.size(); k++){
      cerr << _writeAfterTheseElevs[k] << " ";
    }
    cerr << endl;

    if (_params->elevAction == Params::ELEVATION_LOOKUP){
      for (unsigned k=0; k < _elevRanges.size(); k++){
	cerr << "Elevations from " << _elevRanges[k].min << " to " << _elevRanges[k].max;
	cerr << " will be mapped to " << _elevRanges[k].targetElev << endl;
      }
    }

  }

  return;

}

void superResNexradII2Dsr::_volumeHeader(unsigned char *buf){

  //
  // All we want to decode here is the volume number and reset the vcp number.
  //
  char volNumStr[4];
  volNumStr[0] = (char)buf[9];
  volNumStr[1] = (char)buf[10];
  volNumStr[2] = (char)buf[11];
  volNumStr[3] = char(0);

  int volNum;
  if (sscanf(volNumStr, "%d", &volNum) == 1) {
    _volNum = volNum;
  } else {
    _volNum++;
  }

  _vcpNum = -1;

  if (_params->debug >= Params::DEBUG_HEADERS){
    cerr << "Volume number is " << _volNum << endl;
  }

  return;
}


//
// Determine if an elevation number, field is in our hit list for this VCP
//
bool superResNexradII2Dsr::_isActive( int eleNum, string fieldName ){

  for (unsigned k=0; k <  _elevSelections.size(); k++){
    if ((_elevSelections[k].elevNum == eleNum) && (_elevSelections[k].fieldName == fieldName)) return true;
  }
  return false;

}


//
// Turn the way they encode time into our way
//
void superResNexradII2Dsr::_getTime(int jDate, int mSecPastMidnight, date_time_t *ralTime){

  ralTime->unix_time = 86400*(jDate-1) + (int)rint(double(mSecPastMidnight)/1000.0);
  uconvert_from_utime( ralTime );

  return;

}

double superResNexradII2Dsr::_lookupElev( double el ){

  for (unsigned k=0; k < _elevRanges.size(); k++){
    if ((_elevRanges[k].min <= el) && (_elevRanges[k].max > el))
      return _elevRanges[k].targetElev;
  }

  return el; // Did not fit into any ranges.

}


void superResNexradII2Dsr::_resetInternalFlags(){

  _sendBeam=false;
   _sendElevStart=false;
   _sendElevEnd=false;
   _sendVolStart=false;
   _sendVolEnd=false;

  return;
}
