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
// SlaveReader.cc
//
// SlaveReader object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2010
//
///////////////////////////////////////////////////////////////

#include <cerrno>
#include <iostream>
#include <dataport/swap.h>
#include "SlaveReader.hh"
using namespace std;

////////////////////////////////////////////////////
// constructor

SlaveReader::SlaveReader(const Params &params) :
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
  _latestPulsePacket = NULL;
  _latestPulsePacketLen = 0;
}

//////////////////////////////////////////////////////////////////
// destructor

SlaveReader::~SlaveReader()

{
  if (_latestPulsePacket) {
    delete[] _latestPulsePacket;
  }
}

///////////////////////////////////////////
// get next message part - slave queue
// returns ptr to msg part on success, NULL on failure
//
// no swapping occurs here, we pass on the parts as they are

const DsMsgPart *SlaveReader::getNextPart()
  
{
  
  // get next message part
  
  if (_getNextPart()) {
    return NULL;
  }
  
  int msgType = _part->getType();
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Slave reader - read packet type: "
         << iwrf_packet_id_to_str(msgType) << endl;
  }
  if (_params.debug >= Params::DEBUG_EXTRA) {
    iwrf_packet_print(stderr, _part->getBuf(), _part->getLength());
  }
    
  if (_opsInfo.isInfo(msgType)) {
    _opsInfo.setFromBuffer((void *) _part->getBuf(), _part->getLength());
  }

  // if this is a pulse packet, save
  
  if (msgType == IWRF_PULSE_HEADER_ID) {
    if (_latestPulsePacket) {
      delete[] _latestPulsePacket;
    }
    _latestPulsePacketLen = _part->getLength();
    _latestPulsePacket = new char[_latestPulsePacketLen];
    memcpy(_latestPulsePacket, _part->getBuf(), _latestPulsePacketLen);
    memcpy(&_latestPulseHdr, _part->getBuf(), sizeof(_latestPulseHdr));
  }

  return _part;

}

////////////////////////////
// get next message part
//
// Returns 0 on success, -1 on failure

int SlaveReader::_getNextPart()
  
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
    
    if (_fmq.initReadBlocking(_params.slave_input_fmq_name,
			      "SlaveReader",
			      _params.debug >= Params::DEBUG_EXTRA,
			      initPos)) {
      cerr << "ERROR - SlaveReader::_getNextPart" << endl;
      cerr << "  Cannot init FMQ for reading" << endl;
      cerr << "  Fmq: " << _params.slave_input_fmq_name << endl;
      cerr << _fmq.getErrStr() << endl;
      return -1;
    }

    _fmqIsOpen = true;

  } // if
  
  while (_pos >= _nParts) {
    
    // we need a new message
    // blocking read registers with Procmap while waiting
    
    if (_fmq.readMsgBlocking()) {
      cerr << "ERROR - SlaveReader::_getNextPart" << endl;
      cerr << "  Cannot read message from FMQ" << endl;
      cerr << "  Fmq: " << _params.slave_input_fmq_name << endl;
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

