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
// MasterReader.cc
//
// MasterReader object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2010
//
///////////////////////////////////////////////////////////////
//
// MasterReader reads radar time series data
//
////////////////////////////////////////////////////////////////

#include <cerrno>
#include <iostream>
#include <dataport/swap.h>
#include "MasterReader.hh"
using namespace std;

////////////////////////////////////////////////////
// constructor

MasterReader::MasterReader(const Params &params) :
        _params(params)
        
{

  if (_params.debug >= Params::DEBUG_EXTRA) {
    _opsInfo.setDebug(IWRF_DEBUG_VERBOSE);
  } else if (_params.debug >= Params::DEBUG_VERBOSE) {
    _opsInfo.setDebug(IWRF_DEBUG_NORM);
  } 

  _fmqIsOpen = false;
  _part = NULL;
  _nParts = 0;
  _pos = 0;
  _pulseCount = -1;
}

//////////////////////////////////////////////////////////////////
// destructor

MasterReader::~MasterReader()

{
}

///////////////////////////////////////////////////////////////////
// get next pulse
// returns IwrfTsPulse pointer on success, NULL on failure

IwrfTsPulse *MasterReader::getNextPulse()
  
{

  // make sure, before we look for pulses, that we have
  // scan segment and ts processing info
  
  while (!_opsInfo.isRadarInfoActive() ||
         // !_opsInfo.isScanSegmentActive() ||
         !_opsInfo.isTsProcessingActive()) {
    
    // get next message part
    
    if (_getNextPart()) {
      return NULL;
    }
    
    // save master structs in static memory, for use later by slave
    // the copy preserves the byte order of the incoming data

    int msgType = _part->getType();

    if (msgType == IWRF_RADAR_INFO_ID) {
      memcpy(&_latestRadarInfo, _part->getBuf(), sizeof(_latestRadarInfo));
    } 
    if (msgType == IWRF_SCAN_SEGMENT_ID) {
      memcpy(&_latestScanSeg, _part->getBuf(), sizeof(_latestScanSeg));
    } 
    if (msgType == IWRF_TS_PROCESSING_ID) {
      memcpy(&_latestTsProc, _part->getBuf(), sizeof(_latestTsProc));
    } 

    // load up info if available
    
    if (_opsInfo.isInfo(msgType)) {
      _opsInfo.setFromBuffer((void *)  _part->getBuf(), _part->getLength());
    }

  } // while

  // read in pulse
  
  while (true) {

    // get next message part
    
    if (_getNextPart()) {
      return NULL;
    }
    
    int msgType = _part->getType();

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Master reader - read packet type: "
           << iwrf_packet_id_to_str(msgType) << endl;
    }
    if (_params.debug >= Params::DEBUG_EXTRA) {
      iwrf_packet_print(stderr, _part->getBuf(), _part->getLength());
    }
    
    // save master structs in static memory, for use later by slave
    // the copy preserves the byte order of the incoming data
    
    if (msgType == IWRF_RADAR_INFO_ID) {
      memcpy(&_latestRadarInfo, _part->getBuf(), sizeof(_latestRadarInfo));
    } 
    if (msgType == IWRF_SCAN_SEGMENT_ID) {
      memcpy(&_latestScanSeg, _part->getBuf(), sizeof(_latestScanSeg));
    } 
    if (msgType == IWRF_TS_PROCESSING_ID) {
      memcpy(&_latestTsProc, _part->getBuf(), sizeof(_latestTsProc));
    } 

    if (_opsInfo.isInfo(msgType)) {
      
      // if this is an info part, load up info
      
      _opsInfo.setFromBuffer((void *) _part->getBuf(), _part->getLength());
      
    }

#ifdef NOTNOW
    if (msgType == IWRF_PULSE_HEADER_ID && _opsInfo.isRadarInfoActive() &&
        _opsInfo.isScanSegmentActive() && _opsInfo.isTsProcessingActive()) {
#endif

    if (msgType == IWRF_PULSE_HEADER_ID) {
      
      // for pulse, save header in original byte order

      if (_pulseCount < 0) {
        memcpy(&_firstPulseHdr, _part->getBuf(), sizeof(_latestPulseHdr));
        _prevPulseHdr = _firstPulseHdr;
      } else {
        _prevPulseHdr = _latestPulseHdr;
      }
      memcpy(&_latestPulseHdr, _part->getBuf(), sizeof(_latestPulseHdr));

      _pulseCount++;

      IwrfTsPulse *pulse = new IwrfTsPulse(_opsInfo);
      if (pulse->setFromBuffer((void *) _part->getBuf(),
                               _part->getLength(), true) == 0) {
	return pulse;
      } else {
        delete pulse;
      }
      
    }
    
  } // while
  
  return NULL;

}

////////////////////////////
// get next message part
//
// Returns 0 on success, -1 on failure

int MasterReader::_getNextPart()
  
{
  
  PMU_auto_register("Get next part");
  
  // check we have an open FMQ
  
  if (!_fmqIsOpen) {
    
    // initialize FMQ

    Fmq::openPosition initPos = Fmq::END;
    if (_params.start_reading_at_fmq_start) {
      initPos = Fmq::START;
    }
    _fmq.setHeartbeat(PMU_auto_register);
    
    if (_fmq.initReadBlocking(_params.master_input_fmq_name,
			      "MasterReader",
			      _params.debug >= Params::DEBUG_EXTRA,
			      initPos)) {
      cerr << "ERROR - MasterReader::_getNextPart" << endl;
      cerr << "  Cannot init FMQ for reading" << endl;
      cerr << "  Fmq: " << _params.master_input_fmq_name << endl;
      cerr << _fmq.getErrStr() << endl;
      return -1;
    }

    _fmqIsOpen = true;

  } // if
  
  while (_pos >= _nParts) {
    
    // we need a new message
    // blocking read registers with Procmap while waiting
    
    if (_fmq.readMsgBlocking()) {
      cerr << "ERROR - MasterReader::_getNextPart" << endl;
      cerr << "  Cannot read message from FMQ" << endl;
      cerr << "  Fmq: " << _params.master_input_fmq_name << endl;
      cerr << _fmq.getErrStr() << endl;
      _fmq.closeMsgQueue();
      _fmqIsOpen = false;
      return -1;
    }

    if (_params.debug >= Params::DEBUG_EXTRA) {
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

///////////////////////////////////////
// get time of pulse header since start

double MasterReader::getTimeSinceStart(const iwrf_pulse_header_t &phdr)
  
{

  if (_pulseCount < 0) {
    return -1.0;
  }

  double dsecs = (double) (phdr.packet.time_secs_utc - 
                           _firstPulseHdr.packet.time_secs_utc);

  double dnano = (double) (phdr.packet.time_nano_secs -
                           _firstPulseHdr.packet.time_nano_secs);

  return (dsecs + dnano * 1.0e-9);

}


