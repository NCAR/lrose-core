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
// StatusReader.cc
//
// StatusReader object
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2010
//
///////////////////////////////////////////////////////////////
//
// StatusReader reads radar time series data
//
////////////////////////////////////////////////////////////////

#include <cerrno>
#include <iostream>
#include <dataport/swap.h>
#include <toolsa/pmu.h>
#include <toolsa/TaXml.hh>
#include "StatusReader.hh"
using namespace std;

////////////////////////////////////////////////////
// constructor

StatusReader::StatusReader(const Params &params,
                           const string &fmqPath,
                           bool isActive,
                           double defaultFreqMhz) :
        _params(params),
        _fmqPath(fmqPath),
        _isActive(isActive),
        _latestFreqMhz(defaultFreqMhz)
        
{

  _fmqIsOpen = false;
  _part = NULL;
  _nParts = 0;
  _pos = 0;

}

//////////////////////////////////////////////////////////////////
// destructor

StatusReader::~StatusReader()

{
  _fmq.closeMsgQueue();
}

///////////////////////////////////////////////////////////////////
// read in status xml
// returns 0 on success, -1 on failure

int StatusReader::readStatus()
  
{

  if (!_isActive) {
    return 0;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Reading FMQ: " << _fmqPath << endl;
  }

  while (true) {
    
    // get next message part
    
    bool gotMsg = false;
    if (_getNextPart(gotMsg)) {
      return -1;
    }

    if (!gotMsg) {
      // no more data for now
      return 0;
    }
    
    // get message type

    si32 msgType = _part->getType();

    // swap as needed
    
    bool isSwapped;
    if (iwrf_check_packet_id(msgType, &isSwapped)) {
      continue;
    }
    
    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "Ts reader - read packet type: "
           << iwrf_packet_id_to_str(msgType) << endl;
      iwrf_packet_print(stderr, _part->getBuf(), _part->getLength());
    }
    
    if (msgType == IWRF_STATUS_XML_ID) {

      // check we have enough space for the header plus at least a null
      
      if (_part->getLength() <= (int) sizeof(iwrf_status_xml_t)) {
        continue;
      }

      // get header
      
      iwrf_status_xml_t statusHdr;
      memcpy(&statusHdr, _part->getBuf(), sizeof(statusHdr));
      if (isSwapped) {
        iwrf_status_xml_swap(statusHdr);
      }

      // set time

      _latestStatusTime.set(statusHdr.packet.time_secs_utc);
      _latestStatusTime.setSubSec(statusHdr.packet.time_nano_secs / 1.0e9);
      
      // set status

      _latestStatusXml = (char *) _part->getBuf() + sizeof(statusHdr);

      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "================= Status =================" << endl;
        cerr << _latestStatusXml << endl;
        cerr << "==========================================" << endl;
      }

      // set frequency

      double freqHz;
      if (TaXml::readDouble(_latestStatusXml, "RfFreqHz", freqHz) == 0) {
        if (freqHz > 0) {
          _latestFreqMhz = freqHz * 1.0e-6;
        }
      }

      return 0;

    }

  } // while
  
  return -1;

}

/////////////////////////////////////////////////
// get next message part
//
// Sets gotMsg if a message was available.
//
// Returns 0 on success, -1 on failure

int StatusReader::_getNextPart(bool &gotMsg)
  
{
  
  PMU_auto_register("_getNextPart");
  
  // check we have an open FMQ
  
  if (!_fmqIsOpen) {
    
    // initialize FMQ

    Fmq::openPosition initPos = Fmq::END;
    if (_params.start_reading_at_fmq_start) {
      initPos = Fmq::START;
    }
    _fmq.setHeartbeat(PMU_auto_register);
    
    if (_fmq.initReadBlocking(_fmqPath.c_str(),
			      "StatusReader",
			      _params.debug >= Params::DEBUG_EXTRA,
			      initPos)) {
      cerr << "ERROR - StatusReader::_getNextPart" << endl;
      cerr << "  Cannot init FMQ for reading" << endl;
      cerr << "  Fmq: " << _fmqPath << endl;
      cerr << _fmq.getErrStr() << endl;
      return -1;
    }

    _fmqIsOpen = true;

    if (_params.debug) {
      cerr << "Opened FMQ for reading: " << _fmqPath << endl;
    }

  } // if

  while (_pos >= _nParts) {

    // we need a new message
    // non-blocking read, with 50 millisec timeout
    
    if (_fmq.readMsg(&gotMsg, -1, 50)) {
      cerr << "ERROR - StatusReader::_getNextPart" << endl;
      cerr << "  Cannot read message from FMQ" << endl;
      cerr << "  Fmq: " << _fmqPath << endl;
      cerr << _fmq.getErrStr() << endl;
      _fmq.closeMsgQueue();
      _fmqIsOpen = false;
      return -1;
    }

    if (!gotMsg) {
      // queue is empty
      return 0;
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

  gotMsg = true;
  _part = _msg.getPart(_pos);
  _pos++;

  return 0;

}

