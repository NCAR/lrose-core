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
///////////////////////////////////////////////////////////////
// IpsTsReader.cc
//
// IpsTsReader object
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Aug 2019
//
///////////////////////////////////////////////////////////////
//
// Support for Independent Pulse Sampling.
//
// IpsTsReader reads radar time series data
//
////////////////////////////////////////////////////////////////

#include <cerrno>
#include <iostream>
#include <dataport/swap.h>
#include <radar/IpsTsReader.hh>
using namespace std;

////////////////////////////////////////////////////
// Base class

IpsTsReader::IpsTsReader(IpsTsDebug_t debug) :
        _debug(debug)
  
{
  _opsInfo.setDebug(_debug);
  _pulseSeqNumLatestPulse = 0;
  _pktSeqNumLatestPulse = 0;
  _pktSeqNumPrevPulse = 0;
  _radarId = 0;
  _nonBlocking = false;
  _msecsWait = 0;
  _copyPulseWidthFromTsProc = false;
  _timedOut = false;
  _endOfFile = false;
  _georefTimeMarginSecs = 1.0;
  _georefUseSecondary = false;
}

//////////////////////////////////////////////////////////////////
// destructor

IpsTsReader::~IpsTsReader()

{

}

//////////////////////////////////////////////////////////////////
// reset - used for sim mode

void IpsTsReader::reset()

{
  _pulseSeqNumLatestPulse = 0;
  _pktSeqNumLatestPulse = 0;
  _pktSeqNumPrevPulse = 0;
}

// check to see if the ops info has changed since the previous pulse

bool IpsTsReader::isOpsInfoNew() const

{

  if (_opsInfo.getRadarInfoPktSeqNum() > _pktSeqNumPrevPulse ||
      _opsInfo.getScanSegmentPktSeqNum() > _pktSeqNumPrevPulse ||      
      _opsInfo.getTsProcessingPktSeqNum() > _pktSeqNumPrevPulse ||      
      _opsInfo.getStatusXmlPktSeqNum() > _pktSeqNumPrevPulse ||      
      _opsInfo.getCalibrationPktSeqNum() > _pktSeqNumPrevPulse ||      
      _opsInfo.getPlatformGeorefPktSeqNum() > _pktSeqNumPrevPulse ||      
      _opsInfo.getPlatformGeoref1PktSeqNum() > _pktSeqNumPrevPulse) {
    return true;
  }
  return false;

}
  
//////////////////////////////////////////////////////////////////
// set event flags on the pulse

void IpsTsReader::_setEventFlags(IpsTsPulse &pulse)

{

  if (_opsInfo.isStartOfSweep()) {
    pulse.setStartOfSweep();
  }
  if (_opsInfo.isStartOfVolume()) {
    pulse.setStartOfVolume();
  }
  if (_opsInfo.isEndOfSweep()) {
    pulse.setEndOfSweep();
  }
  if (_opsInfo.isEndOfVolume()) {
    pulse.setEndOfVolume();
  }

  _opsInfo.clearEventFlags();

}

//////////////////////////////////////////////////////////////////
// set platform georef on pulse

void IpsTsReader::_setPlatformGeoref(IpsTsPulse &pulse)

{

  // check whether we should try the secondary

  if (_georefUseSecondary &&
      _opsInfo.isPlatformGeoref1Active()) {
    const ips_ts_platform_georef_t &georef1 = _opsInfo.getPlatformGeoref1();
    double gtime = ips_ts_get_packet_time_as_double(georef1.packet);
    double ptime = pulse.getFTime();
    double dtime = fabs(gtime - ptime);
    if (dtime <= _georefTimeMarginSecs) {
      pulse.setPlatformGeoref(georef1);
      return;
    }
  }

  // use primary if active

  if (_opsInfo.isPlatformGeorefActive()) {
    const ips_ts_platform_georef_t &georef = _opsInfo.getPlatformGeoref();
    double gtime = ips_ts_get_packet_time_as_double(georef.packet);
    double ptime = pulse.getFTime();
    double dtime = fabs(gtime - ptime);
    if (dtime <= _georefTimeMarginSecs) {
      pulse.setPlatformGeoref(georef);
    }
  }

}

//////////////////////////////////////////////////////////////////
// update the pulse data and metadata as appropriate

void IpsTsReader::_updatePulse(IpsTsPulse &pulse)

{
  
  if (_copyPulseWidthFromTsProc) {
    pulse.copyPulseWidthFromTsProc();
  }

}

////////////////////////////////////////////////////
////////////////////////////////////////////////////
// Read pulses from FILE
// Derived class

// REALTIME mode, read files as they arrive
// Specify input directory to watch.
//
// Blocks on read.
// Calls heartbeat_func when blocked, if non-null.

IpsTsReaderFile::IpsTsReaderFile(const char *input_dir,
                                 int max_realtime_age_secs,
                                 DsInput_heartbeat_t heartbeat_func,
                                 bool use_ldata_info,
                                 IpsTsDebug_t debug) :
        IpsTsReader(debug)

{
  
  _input = new DsInputPath("IpsTsReaderFile",
                           _debug >= IpsTsDebug_t::NORM,
                           input_dir,
                           max_realtime_age_secs,
                           heartbeat_func,
                           use_ldata_info);
  
  _in = NULL;
  
}

// ARCHIVE mode - specify list of files to be read

IpsTsReaderFile::IpsTsReaderFile(const vector<string> &fileList,
                                 IpsTsDebug_t debug) :
        IpsTsReader(debug),
        _fileList(fileList)
  
{
  
  _input = new DsInputPath("IpsTsReaderFile", (debug != IpsTsDebug_t::OFF),
                           _fileList);
  _in = NULL;
  if (_debug != IpsTsDebug_t::OFF) {
    cerr << "INFO - IpsTsReaderFile" << endl;
    const vector<string> &pathList = _input->getPathList();
    for (size_t ii = 0; ii < pathList.size(); ii++) {
      cerr << "  using file: " << pathList[ii] << endl;
    }
  }

}

//////////////////////////////////////////////////////////////////
// destructor

IpsTsReaderFile::~IpsTsReaderFile()

{

  if (_input) {
    delete _input;
  }

  if (_in) {
    fclose(_in);
    _in = NULL;
  } 

}

///////////////////////////////////////////
// Get next pulse from file.
//
// Converts IQ data to floats if requested.
//
// If pulse arg is non-NULL, it will be filled out and returned.
// If pulse arg is NULL, a new pulse object is allocated.
//
// Caller must free non-NULL pulses returned by this method.
//
// Returns:
//   pointer to pulse object.
//   NULL at end of data, or error.

IpsTsPulse*
  IpsTsReaderFile::getNextPulse(bool convertToFloats /* = false */,
                                IpsTsPulse *inPulse /* = NULL*/)
  
{
  
  // Create a new pulse object if required
  
  IpsTsPulse *pulse = inPulse;
  if (pulse == NULL) {
    pulse = new IpsTsPulse(_opsInfo, _debug);
  } else {
    pulse->setOpsInfo(_opsInfo);
    pulse->setDebug(_debug);
  }

  _endOfFile = false;

  if (_in == NULL || feof(_in)) {
    if (_openNextFile()) {
      delete pulse;
      _endOfFile = true;
      return NULL;
    }
  }

  // read in pulse headers and data, opening new files as needed

  while (_in != NULL) {

    int iret = _readPulse(*pulse);

    if (iret == 0) {
      // success
      _pktSeqNumPrevPulse = _pktSeqNumLatestPulse;
      _pktSeqNumLatestPulse = pulse->getPktSeqNum();
      _pulseSeqNumLatestPulse = pulse->getPulseSeqNum();
      if (convertToFloats) {
	pulse->convertToFL32();
      }
      _updatePulse(*pulse);
      if (_debug >= IpsTsDebug_t::VERBOSE) {
        pulse->printHeader(stderr);
      }
      _setEventFlags(*pulse);
      _setPlatformGeoref(*pulse);
      return pulse;
    }
    
    // failure with this file
    
    if ((_debug != IpsTsDebug_t::OFF) && !feof(_in)) {
      cerr << "ERROR - IpsTsReader::_processFile" << endl;
      cerr << "  Cannot read in pulse headers and data" << endl;
      cerr << "  File: " << _inputPath << endl;
    }

    // set end of file flag
    
    _endOfFile = true;

    // try new file
    if (_openNextFile()) {
      // no good
      delete pulse;
      return NULL;
    }
    
  } // while

  // should not get here

  delete pulse;
  return NULL;

}

////////////////////////////
// open next available file
//
// Returns 0 on success, -1 on failure

int IpsTsReaderFile::_openNextFile()

{

  PMU_auto_register("Opening next file");

  if (_in) {
    _prevInputPath = _inputPath;
    fclose(_in);
    _in = NULL;
  }
  
  _inputPath.clear();
  const char *inputPath = _input->next();
  if (inputPath == NULL) {
    // no more files
    return -1;
  }
  _inputPath = inputPath;

  if (_debug != IpsTsDebug_t::OFF) {
    cerr << "Opening input file: " << _inputPath << endl;
  }

  // open file
  
  if ((_in = fopen(_inputPath.c_str(), "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - IpsTsReaderFile::_openNextFile" << endl;
    cerr << "  Cannot open file: " << _inputPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  return 0;

}

///////////////////////////////////////////
// read next pulse from file
// returns 0 on success, -1 on error

int IpsTsReaderFile::_readPulse(IpsTsPulse &pulse)
  
{
  
  while (true) {

    // read in the next 8 bytes

    si32 packetTop[2];
    if (fread(packetTop, sizeof(si32), 2, _in) != 2) {
      return -1;
    }
    // seek back 8 bytes, so we are back to the top of packet
    if (fseek(_in, -8L, SEEK_CUR)) {
      return -1;
    }

    // is this an IPS TS packet?
    
    si32 packetId = packetTop[0];
    si32 packetLen = packetTop[1];

    if (_debug >= IpsTsDebug_t::VERBOSE) {
      fprintf(stderr, "Found packet, id, len: 0x%x, %d\n",
              packetId, packetLen);
    }
    
    if (ips_ts_check_packet_id(packetId, packetLen)) {
      if (_resync()) {
	return -1;
      }
      continue;
    }
    
    if (packetLen > 10000000) {
      cerr << "ERROR - IpsTsReaderFile::_readPulse" << endl;
      cerr << "  Packet too long, len: " << packetLen << endl;
      cerr << "  Packet id: " << packetId << endl;
      return -1;
    }
    
    // resize the read buffer
    
    _pktBuf.reserve(packetLen);
    
    // read it in
    
    if (fread(_pktBuf.getPtr(), _pktBuf.getLen(), 1, _in) != 1) {
      return -1;
    }

    if (_debug >= IpsTsDebug_t::EXTRAVERBOSE) {
      cerr << "======================================================" << endl;
      ips_ts_packet_print(stderr, _pktBuf.getPtr(), _pktBuf.getLen());
      cerr << "======================================================" << endl;
    }

    // check radar id
    
    if (!ips_ts_check_radar_id(_pktBuf.getPtr(), _pktBuf.getLen(), _radarId)) {
      continue;
    }

    // is this an opsInfo packet?

    if (_opsInfo.isInfo(packetId)) {

      if (_opsInfo.setFromBuffer(_pktBuf.getPtr(), _pktBuf.getLen())) {
	return -1;
      }

      if (_debug >= IpsTsDebug_t::VERBOSE) {
        _opsInfo.print(stderr);
      }

    } else if (packetId == IPS_TS_PULSE_HEADER_ID) {

      if (pulse.setFromBuffer(_pktBuf.getPtr(), _pktBuf.getLen(), false)) {
	return -1;
      }
      
      // success
      _pktSeqNumPrevPulse = _pktSeqNumLatestPulse;
      _pktSeqNumLatestPulse = pulse.getPktSeqNum();
      _pulseSeqNumLatestPulse = pulse.getPulseSeqNum();

      return 0;
    }

  } // while

  return -1;

}

///////////////////////////////////////////
// re-sync the data stream
// returns 0 on success, -1 on error

int IpsTsReaderFile::_resync()
  
{

  if (_debug != IpsTsDebug_t::OFF) {
    cerr << "Trying to resync ....." << endl;
  }
  
  si32 check[2];

  while (!feof(_in)) {

    // read in the next 8 bytes
    
    if (fread(check, sizeof(si32), 2, _in) != 2) {
      return -1;
    }

    if (check[0] == IPS_TS_SYNC_VAL_00 &&
	check[1] == IPS_TS_SYNC_VAL_01) {
      // back in sync
      if (_debug != IpsTsDebug_t::OFF) {
	cerr << "Found sync packet, back in sync" << endl;
      }
      return 0;
    }

    si32 swapped = SWAP_si32(check[0]);
    if (_opsInfo.isInfo(check[0]) ||
	_opsInfo.isInfo(swapped)) {
      // found start of a packet
      // seek back 8 bytes, so we are back to the top of packet
      if (_debug != IpsTsDebug_t::OFF) {
	cerr << "Found top of packet, back in sync" << endl;
      }
      if (fseek(_in, -8L, SEEK_CUR)) {
	return -1;
      }
      return 0;
    }
    
    // no sync yet, move back by 7 bytes and try again
    
    if (fseek(_in, -7L, SEEK_CUR)) {
      return -1;
    }

  } // while

  return -1;

}

//////////////////////////////////////////////////////////////////
// reset - used for sim mode

void IpsTsReaderFile::reset()

{
  if (_input) {
    _input->reset();
  }
  if (_in) {
    fclose(_in);
    _in = NULL;
  } 
  IpsTsReader::reset();
}

//////////////////////////////////////////////////////////////////
// seek to end - no op in file mode

void IpsTsReaderFile::seekToEnd()

{
}

////////////////////////////////////////////////////
////////////////////////////////////////////////////
// Read pulses from FMQ
// Derived class

IpsTsReaderFmq::IpsTsReaderFmq(const char *input_fmq,
                               IpsTsDebug_t debug,
                               bool position_fmq_at_start) :
        IpsTsReader(debug),
        _inputFmq(input_fmq),
        _positionFmqAtStart(position_fmq_at_start)
  
{
  _nParts = 0;
  _pos = 0;
  _fmqIsOpen = false;
}

//////////////////////////////////////////////////////////////////
// destructor

IpsTsReaderFmq::~IpsTsReaderFmq()

{

}

///////////////////////////////////////////
// Get next pulse from FMQ.
//
// Converts IQ data to floats if requested.
//
// If pulse arg is non-NULL, it will be filled out and returned.
// If pulse arg is NULL, a new pulse object is allocated.
//
// Caller must free non-NULL pulses returned by this method.
//
// Returns:
//   pointer to pulse object.
//   NULL at end of data, or error.

IpsTsPulse*
  IpsTsReaderFmq::getNextPulse(bool convertToFloats /* = false */,
                               IpsTsPulse *inPulse /* = NULL*/)
  
{
  
  // Create pulse object as needed
  
  IpsTsPulse *pulse = inPulse;
  if (pulse == NULL) {
    pulse = new IpsTsPulse(_opsInfo, _debug);
  } else {
    pulse->setOpsInfo(_opsInfo);
    pulse->setDebug(_debug);
  }

  while (!_opsInfo.isEssentialInfoReady()) {
    
    // get next message part
    
    if (_getNextPart()) {
      delete pulse;
      return NULL;
    }
    
    if (_debug >= IpsTsDebug_t::EXTRAVERBOSE) {
      ips_ts_packet_print(stderr, _part->getBuf(), _part->getLength());
    }

    // check radar id

    if (!ips_ts_check_radar_id(_part->getBuf(), _part->getLength(), _radarId)) {
      continue;
    }

    // load up info if available
    
    if (_opsInfo.isInfo(_part->getType())) {
      _opsInfo.setFromBuffer((void *) _part->getBuf(), _part->getLength());
    }
    
  } // while
  
  while (true) {
    
    // get next message part
    
    if (_getNextPart()) {
      delete pulse;
      return NULL;
    }
    
    // check radar id
    
    if (!ips_ts_check_radar_id(_part->getBuf(), _part->getLength(), _radarId)) {
      continue;
    }

    // if this is an info part, load up info
    
    int partType = _part->getType();
    
    if (_opsInfo.isInfo(partType)) {
      
      _opsInfo.setFromBuffer((void *) _part->getBuf(), _part->getLength());
      
      if (_debug >= IpsTsDebug_t::VERBOSE) {
        _opsInfo.print(stderr);
      }

    } else if (partType == IPS_TS_PULSE_HEADER_ID) {
      
      if (pulse->setFromBuffer((void *) _part->getBuf(),
                               _part->getLength(), convertToFloats) == 0) {
        _updatePulse(*pulse);
	_pktSeqNumPrevPulse = _pktSeqNumLatestPulse;
	_pktSeqNumLatestPulse = pulse->getPktSeqNum();
	_pulseSeqNumLatestPulse = pulse->getPulseSeqNum();
        _setEventFlags(*pulse);
        _setPlatformGeoref(*pulse);
	return pulse;
      }
      
    } // if (_opsInfo.isInfo ...
    
  } // while

  // should not reach here

  delete pulse;
  return NULL;

}

////////////////////////////
// get next message part
//
// Returns 0 on success, -1 on failure

int IpsTsReaderFmq::_getNextPart()
  
{

  PMU_auto_register("Get next part");
  
  // check we have an open FMQ
  
  if (!_fmqIsOpen) {

    // initialize FMQ

    Fmq::openPosition initPos = Fmq::END;
    if (_positionFmqAtStart) {
      initPos = Fmq::START;
    }
    _fmq.setHeartbeat(PMU_auto_register);
    
    int iret = 0;
    if (_nonBlocking) {
      iret = _fmq.initReadOnly(_inputFmq.c_str(),
                               "IpsTsReader",
                               _debug >= IpsTsDebug_t::NORM,
                               initPos, 
                               _msecsWait);
    } else {
      iret = _fmq.initReadBlocking(_inputFmq.c_str(),
                                   "IpsTsReader",
                                   _debug >= IpsTsDebug_t::NORM,
                                   initPos);
    }
    
    if (iret) {
      cerr << "ERROR - IpsTsReaderFmq::_getNextPart" << endl;
      cerr << "  Cannot init FMQ for reading" << endl;
      cerr << "  Fmq: " << _inputFmq << endl;
      cerr << _fmq.getErrStr() << endl;
      return -1;
    }

    _fmqIsOpen = true;
    
  } // if

  while (_pos >= _nParts) {
    
    // we need a new message
    // blocking read registers with Procmap while waiting

    if (_nonBlocking) {
      bool gotOne = false;
      _timedOut = false;
      if (_fmq.readMsg(&gotOne, -1, _msecsWait)) {
        _handleReadError();
        return -1;
      }
      if (!gotOne) {
        _timedOut = true;
        return -1;
      }
    } else {
      if (_fmq.readMsgBlocking()) {
        _handleReadError();
        return -1;
      }
    }

    if (_debug >= IpsTsDebug_t::EXTRAVERBOSE) {
      cerr << "--->> Got FMQ message" << endl;
    }

    // disassemble the message

    const void *msg = _fmq.getMsg();
    int len = _fmq.getMsgLen();
    if (_msg.disassemble(msg, len) == 0) {
      _pos = 0;
      _nParts = _msg.getNParts();
    }
    
  } // while

  _part = _msg.getPart(_pos);
  _pos++;

  return 0;

}

//////////////////////////////////////////////////////////////////
// handle FMQ read error

void IpsTsReaderFmq::_handleReadError()
  
{
  cerr << "ERROR - IpsTsReaderFmq::_getNextPart" << endl;
  cerr << "  Cannot read message from FMQ" << endl;
  cerr << "  Fmq: " << _inputFmq << endl;
  cerr << _fmq.getErrStr() << endl;
  _fmq.closeMsgQueue();
  _fmqIsOpen = false;
}

//////////////////////////////////////////////////////////////////
// reset - used for sim mode

void IpsTsReaderFmq::reset()

{
  _fmq.seek(Fmq::FMQ_SEEK_START);
  IpsTsReader::reset();
}

//////////////////////////////////////////////////////////////////
// seek to end

void IpsTsReaderFmq::seekToEnd()

{
  _fmq.seek(Fmq::FMQ_SEEK_END);
}

////////////////////////////////////////////////////
////////////////////////////////////////////////////
// Read pulses from TCP socket
// Derived class

IpsTsReaderTcp::IpsTsReaderTcp(const char *server_host,
                               int server_port,
                               IpsTsDebug_t debug) :
        IpsTsReader(debug),
        _serverHost(server_host),
        _serverPort(server_port)
        
{

  char serverDetails[1024];
  sprintf(serverDetails, "%s:%d", server_host, server_port);
  _serverDetails = serverDetails;

}

//////////////////////////////////////////////////////////////////
// destructor

IpsTsReaderTcp::~IpsTsReaderTcp()

{
  _sock.close();
}

///////////////////////////////////////////
// Get next pulse from TCP.
//
// Converts IQ data to floats if requested.
//
// If pulse arg is non-NULL, it will be filled out and returned.
// If pulse arg is NULL, a new pulse object is allocated.
//
// Caller must free non-NULL pulses returned by this method.
//
// Returns:
//   pointer to pulse object.
//   NULL at end of data, or error.

IpsTsPulse*
  IpsTsReaderTcp::getNextPulse(bool convertToFloats /* = false */,
                               IpsTsPulse *inPulse /* = NULL*/)
  
{
  
  // Create pulse object as needed
  
  IpsTsPulse *pulse = inPulse;
  if (pulse == NULL) {
    pulse = new IpsTsPulse(_opsInfo, _debug);
  } else {
    pulse->setOpsInfo(_opsInfo);
    pulse->setDebug(_debug);
  }

  MemBuf buf;
  int packetId, packetLen;

  while (!_opsInfo.isEssentialInfoReady()) {

    // read packet from time series server server
    
    if (_readTcpPacket(packetId, packetLen, buf)) {
      delete pulse;
      return NULL;
    }
    
    if (_debug >= IpsTsDebug_t::EXTRAVERBOSE) {
      ips_ts_packet_print(stderr, buf.getPtr(), buf.getLen());
    }

    // check radar id
    
    if (!ips_ts_check_radar_id(buf.getPtr(), buf.getLen(), _radarId)) {
      continue;
    }

    // load up info if available
    
    if (_opsInfo.isInfo(packetId)) {
      _opsInfo.setFromBuffer(buf.getPtr(), buf.getLen());
    }
    
  } // while
  
  while (true) {
    
    // read packet from time series server server
    
    if (_readTcpPacket(packetId, packetLen, buf)) {
      delete pulse;
      return NULL;
    }
    
    // check radar id
    
    if (!ips_ts_check_radar_id(buf.getPtr(), buf.getLen(), _radarId)) {
      continue;
    }

    // if this is an info part, load up info
    
    if (_opsInfo.isInfo(packetId)) {
      
      _opsInfo.setFromBuffer(buf.getPtr(), buf.getLen());
      
    } else if (packetId == IPS_TS_PULSE_HEADER_ID) {
      
      if (pulse->setFromBuffer(buf.getPtr(), buf.getLen(),
                               convertToFloats) == 0) {
        _updatePulse(*pulse);
	_pktSeqNumPrevPulse = _pktSeqNumLatestPulse;
	_pktSeqNumLatestPulse = pulse->getPktSeqNum();
	_pulseSeqNumLatestPulse = pulse->getPulseSeqNum();
        _setEventFlags(*pulse);
        _setPlatformGeoref(*pulse);
	return pulse;
      }
      
    } // if (_opsInfo.isInfo ...
    
  } // while

  // should not reach here

  delete pulse;
  return NULL;

}

//////////////////////////////////////////
// open the socket to the server
// Returns 0 on success, -1 on failure

int IpsTsReaderTcp::_openSocket()
  
{

  if (_sock.isOpen()) {
    return 0;
  }

  while (true) {

    PMU_auto_register("Connecting to socket");

    if (_sock.open(_serverHost.c_str(), _serverPort, 5000) == 0) {
      cerr << "====>>>> opening socket, host , port: "
           << _serverHost.c_str() << ", "
           << _serverPort << endl;
      return 0;
    }

    if (_sock.getErrNum() == Socket::TIMED_OUT) {
      cerr << "ERROR - IpsTsReaderTcp::_openSocket()" << endl;
      cerr << "     host: " << _serverHost << endl;
      cerr << "     port: " << _serverPort << endl;
    } else {
      cerr << "ERROR - IpsTsReaderTcp::_openSocket()" << endl;
      cerr << "  Connecting to server" << endl;
      cerr << "  " << _sock.getErrStr() << endl;
    }
    cerr << "  Waiting for server to come up ..." << endl;
    umsleep(2000);

  } // while

  return 0;

}

///////////////////////////////////////////////////////////////////
// Read in next packet, set id and load buffer.
// Returns 0 on success, -1 on failure

int IpsTsReaderTcp::_readTcpPacket(int &id, int &len, MemBuf &buf)

{

  if (_openSocket()) {
    return -1;
  }

  bool have_good_header = false;
  si32 packetId;
  si32 packetLen;
  si32 packetTop[2];

  do {

    PMU_auto_register("Reading data");
    
    // read the first 8 bytes (id, len)

    if (_nonBlocking) {
      _timedOut = false;
      if (_sock.readBuffer(packetTop, sizeof(packetTop), _msecsWait)) {
        if (_sock.getErrNum() == SockUtil::TIMED_OUT) {
          _timedOut = true;
          return -1;
        }
        cerr << "ERROR - IpsTsReader::_readTcpPacket" << endl;
        cerr << "  " << _sock.getErrStr() << endl;
        return -1;
      }
    } else {
      if (_sock.readBufferHb(packetTop, sizeof(packetTop),
                             sizeof(packetTop), PMU_auto_register, -1)) {
        cerr << "ERROR - IpsTsReader::_readTcpPacket" << endl;
        cerr << "  " << _sock.getErrStr() << endl;
        return -1;
      }
    }

    // check ID for packet, and get its length
    packetId = packetTop[0];
    packetLen = packetTop[1];

    if (ips_ts_check_packet_id(packetId, packetLen)) {
      // read bytes to re-synchronize data stream
      if (_reSync()) {
        cerr << "ERROR - IpsTsReader::_readPacket" << endl;
        cerr << " Cannot re-sync incoming data stream from socket";
        cerr << endl;
        return -1;
      }
    } else {
      have_good_header = true;
      id = packetId;
      len = packetLen;
    }
  } while (!have_good_header);
    
  // make the bufferlarge enough

  buf.reserve(packetLen);

  // copy the packet top into the start of the buffer

  memcpy(buf.getPtr(), packetTop, sizeof(packetTop));
  
  // read in the remainder of the buffer

  char *startPtr = (char *) buf.getPtr() + sizeof(packetTop);
  int nBytesLeft = packetLen - sizeof(packetTop);

  if (_sock.readBufferHb(startPtr, nBytesLeft, 1024, 
                         PMU_auto_register, 10000)) {
    cerr << "ERROR - IpsTsReader::_readTcpPacket" << endl;
    cerr << "  " << _sock.getErrStr() << endl;
    return -1;
  }

  return 0;

}

///////////////////////////////////////////
// re-sync the data stream
// returns 0 on success, -1 on error

int IpsTsReaderTcp::_reSync()
  
{
  int sync_count = 0;

  if (_debug != IpsTsDebug_t::OFF) {
    cerr << "Trying to resync ....." << endl;
  }
  
  unsigned int check[2];

  while (true) {
    
    // peek at the next 8 bytes
    
    if (_peekAtBuffer(check, sizeof(check))) {
      cerr << "ERROR - IpsTsReader::_reSync" << endl;
      cerr << "  " << _sock.getErrStr() << endl;
      return -1;
    }

    if(check[0] == IPS_TS_RADAR_INFO_ID &&
       check[1] == sizeof(ips_ts_radar_info_t)) {
      return 0; // We've found a legitimate IPS packet header
    } 

    // Search for the sync packet 
    if (check[0] == IPS_TS_SYNC_VAL_00 && check[1] == IPS_TS_SYNC_VAL_01) {
      // These are "sync packet" bytes read the 8 sync bytes and move on
      if (_debug != IpsTsDebug_t::OFF) {
	cerr << "Found sync packet, back in sync" << endl;
      }
      if (_sock.readBufferHb(check, sizeof(check), sizeof(check),
                             PMU_auto_register, 10000)) {
	cerr << "ERROR - IpsTsReader::_reSync" << endl;
	cerr << "  " << _sock.getErrStr() << endl;
	return -1;
      }
      return 0;
    }
    
    // no sync yet, read 1 byte and start again

    char byteVal;
    if (_sock.readBufferHb(&byteVal, 1, 1, PMU_auto_register, 10000)) {
      cerr << "ERROR - IpsTsReader::_reSync" << endl;
      cerr << "  " << _sock.getErrStr() << endl;
      return -1;
    }
    sync_count++;

  } // while

  return -1;

}

///////////////////////////////////////////////////////////////////
// Peek at buffer from socket
// Returns 0 on success, -1 on failure

int IpsTsReaderTcp::_peekAtBuffer(void *buf, int nbytes)

{

  int count = 0;

  while (true) {
    PMU_auto_register("peekAtBuffer");
    if (_sock.peek(buf, nbytes, 1000) == 0) {
      return 0;
    } else {
      if (_sock.getErrNum() == Socket::TIMED_OUT) {
        PMU_auto_register("Timed out ...");
	count++;
        continue;
      }
      cerr << "ERROR - IpsTsReader::_peekAtBuffer" << endl;
      cerr << "  " << _sock.getErrStr() << endl;
      return -1;
    }
  }
  
  return -1;

}

//////////////////////////////////////////////////////////////////
// reset - used for sim mode

void IpsTsReaderTcp::reset()

{
  IpsTsReader::reset();
}

//////////////////////////////////////////////////////////////////
// seek to end

void IpsTsReaderTcp::seekToEnd()

{
}
