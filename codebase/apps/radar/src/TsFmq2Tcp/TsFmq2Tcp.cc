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
////////////////////////////////////////////////////////////////////////////////
//
// TsFmq2Tcp.cc
// TsFmq2Tcp class
//
// Mike Dixon, RAL, NCAR, Boulder, CO, USA
// Feb 2009
//
// TsFmq2Tcp listens for clients. When a client connects, it spawns a child
// to handle the client. The child opens the time-series FMQ, reads a message
// at a time from the FMQ and writes this data, unchanged,
// to the client in a continuous stream.
// 
////////////////////////////////////////////////////////////////////////////////

#include <cassert>
#include <string>
#include <toolsa/pmu.h>
#include <toolsa/uusleep.h>
#include <toolsa/Socket.hh>
#include <toolsa/TaStr.hh>
#include <didss/DsMsgPart.hh>
#include <radar/iwrf_functions.hh>
#include "TsFmq2Tcp.hh"
using namespace std;

// constructor

TsFmq2Tcp::TsFmq2Tcp(const string& progName,
		     const Params& params) :
	ProcessServer(progName, 
		      params.instance,
		      params.port, 
		      params.max_clients,
		      params.debug >= Params::DEBUG_VERBOSE, 
		      params.debug >= Params::DEBUG_EXTRA),
	_progName(progName),
	_params(params)
  
{
  if (_params.no_threads) {
    setNoThreadDebug(true);
  }
}

// destructor

TsFmq2Tcp::~TsFmq2Tcp()

{

}

// handle clients which connect

int TsFmq2Tcp::handleClient(Socket* socket)
{

  PMU_auto_register("handleClient");
  
  while (true) {

    PMU_auto_register("Opening FMQ");
    if (_params.debug) {
      cerr << "  Opening Fmq: " << _params.fmq_path << endl;
    }
    
    Fmq fmq;
    fmq.setHeartbeat(PMU_auto_register);
    int msecsSleepBlocking = 100;
    
    if (fmq.initReadBlocking(_params.fmq_path,
			     "TsFmq2Tcp",
			     _params.debug >= Params::DEBUG_EXTRA,
			     Fmq::END, msecsSleepBlocking)) {
      cerr << "ERROR - TsFmq2Tcp::handleClient" << endl;
      cerr << "  Cannot init FMQ for reading" << endl;
      cerr << "  Fmq: " << _params.fmq_path << endl;
      cerr << fmq.getErrStr() << endl;
      umsleep(1000);
      continue;
    }

    // read data from the FMQ, send to client
    
    if (_processFmq(fmq, socket)) {
      fmq.closeMsgQueue();
      return -1;
    }

  } // while(true)

  return 0;

}

///////////////////////////////////////////
// read data from the FMQ, send to client

int TsFmq2Tcp::_processFmq(Fmq &fmq, Socket* socket)

{

  // initialize
  
  _msgNParts = 0;
  _msgPos = 0;

  // read data
  
  while (true) {
    
    PMU_auto_register("Reading data");
    
    const DsMsgPart *part = _getNextFromFmq(fmq);
    if (part == NULL) {
      cerr << "ERROR - TsFmq2Tcp::_readFromFmq" << endl;
      return -1;
    }
    
    si32 id = part->getType();

    if (iwrf_check_packet_id(id)) {
      cerr << "ERROR - TsFmq2Tcp::_readFromFmq" << endl;
      cerr << "  Incorrect packet type - ignoring id: " << id << endl;
      continue;
    }

    // load up buffer
    
    _pktBuf.reset();
    _pktBuf.add(part->getBuf(), part->getLength());

    // swap if needed

    iwrf_packet_swap(_pktBuf.getPtr(), _pktBuf.getLen());

    // if this is a radar_info packet, send a sync packet

    if (id == IWRF_RADAR_INFO_ID) {
      iwrf_sync_t sync;
      iwrf_sync_init(sync);
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << "Writing sync packet, id, len: "
             << iwrf_packet_id_to_str(IWRF_SYNC_ID) << ", "
             << sizeof(sync) << endl;
      }
      if (socket->writeBuffer(&sync, sizeof(sync))) {
	if (_params.debug) {
	  cerr << "ERROR - TsFmq2Tcp::_processFmq" << endl;
	  cerr << " Writing sync packet to client" << endl;
	  cerr << socket->getErrStr() << endl;
	}
	return -1;
      }
    }

    // send the packet

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Writing packet, id, len: " << ", "
           << iwrf_packet_id_to_str(id) << ", "
           << _pktBuf.getLen() << endl;
      if (_params.debug >= Params::DEBUG_EXTRA) {
        iwrf_packet_print(stderr, _pktBuf.getPtr(), _pktBuf.getLen());
      }
    }
    if (socket->writeBuffer(_pktBuf.getPtr(), _pktBuf.getLen())) {
      if (_params.debug) {
	cerr << "ERROR - TsFmq2Tcp::_processFmq" << endl;
	cerr << " Writing to client" << endl;
	cerr << " Packet type: " << iwrf_packet_id_to_str(id) << endl;
	cerr << socket->getErrStr() << endl;
      }
      return -1;
    }
    
  }

  return 0;

}

///////////////////////////////////////////
// get next message part from FMQ
// returns DsMsgPart object pointer on success, NULL on failure
// returns NULL at end of data, or error

const DsMsgPart *TsFmq2Tcp::_getNextFromFmq(Fmq &fmq)
  
{
  
  while (_msgPos >= _msgNParts) {
    
    // we need a new message
    // blocking read registers with Procmap while waiting
    
    if (fmq.readMsgBlocking()) {
      cerr << "ERROR - TsFmq2Tcp::_getNextPart" << endl;
      cerr << "  Cannot read message from FMQ" << endl;
      cerr << "  Fmq: " << _params.fmq_path << endl;
      return NULL;
    }

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Got message from FMQ, len: " << fmq.getMsgLen() << endl;
    }

    // disassemble the message
    
    if (_msg.disassemble(fmq.getMsg(), fmq.getMsgLen()) == 0) {
      _msgPos = 0;
      _msgNParts = _msg.getNParts();
    }
    
  } // while
  
  DsMsgPart *part = _msg.getPart(_msgPos);
  _msgPos++;

  return part;
  
}
