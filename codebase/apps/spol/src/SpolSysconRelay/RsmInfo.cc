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
// RsmInfo.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2010
//
///////////////////////////////////////////////////////////////
//
// RsmInfo reads RSM - status packets, from a SysCon server
// and loads up a shared memory segment with selected parts
// of that data
//
////////////////////////////////////////////////////////////////

#include "RsmInfo.hh"
#include <iostream>
#include <toolsa/pmu.h>
#include <toolsa/ushmem.h>
#include <toolsa/DateTime.hh>

using namespace std;

// Constructor

RsmInfo::RsmInfo(const string &progName, 
                 const Params &params) :
        _progName(progName),
        _params(params),
        _seqNum(0)
    
{

}

// destructor

RsmInfo::~RsmInfo()

{
  _closeSocket();
}

/////////////////////////////
// check for data
// waiting specified time

int RsmInfo::checkForData(int waitMsecs)
{

  if (!_sock.isOpen()) {
    if (_openSocket(_params.syscon_server_host,
                    _params.syscon_rsm_info_port)) {
      cerr << "ERROR -  RsmInfo::checkForData" << endl;
      cerr <<  "  Cannot open socket, host, port: "
           << _params.syscon_server_host << ", "
           << _params.syscon_rsm_info_port << endl;
      cerr << _sock.getErrStr() << endl;
      return -1;
    }
  }
  
  if (_sock.readSelect(waitMsecs)) {
    return -1;
  }

  return 0;
  
}


///////////////////////////////////////////////
// read data from the server
// returns 0 on success, -1 on failure

int RsmInfo::read()
  
{

  if (!_sock.isOpen()) {
    if (_openSocket(_params.syscon_server_host,
                    _params.syscon_rsm_info_port)) {
      cerr << "ERROR -  RsmInfo::read" << endl;
      cerr <<  "  Cannot open socket, host, port: "
           << _params.syscon_server_host << ", "
           << _params.syscon_rsm_info_port << endl;
      return -1;
    }
  }

  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "Reading RSM data from syscon" << endl;
  }

  int packetId, packetLen;
  MemBuf buf;
  if (_readPacket(packetId, packetLen, buf)) {
    cerr << "ERROR -  RsmInfo::read" << endl;
    cerr <<  "  Reading packet" << endl;
    return -1;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Read RSM data from syscon, id, len: "
         << packetId << ", " << packetLen << endl;

  }

  if (packetId == IWRF_UI_OPERATIONS_ID || packetId == 0) {
    
    iwrf_ui_task_operations_t ops;
    memcpy(&ops, buf.getPtr(), sizeof(ops));
    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      iwrf_ui_task_operations_print(stderr, ops);
    }
    
    switch (ops.op_code) {
      case IWRF_UI_UPD_RSM_PACKET: {
        _handleRsmPacket(ops.un.rsm_pkt);
        break;
      }
      case IWRF_UI_UPD_SCAN_SEGMENT: {
        _handleScanSegment(ops.un.scan_segment);
        break;
      }
      case IWRF_UI_UPD_TS_PROCESSING: {
        _handleTsProcessing(ops.un.ts_processing);
        break;
      }
      default: {}
    }
    
  }

  return 0;

}

//////////////////////////////////////////////
// open socket
// returns 0 on success, -1 on failure

int RsmInfo::_openSocket(const char *host, int port)

{

  if (_sock.open(host, port, 10000)) {
    
    if (_params.debug) {
      if (_sock.getErrNum() == Socket::TIMED_OUT) {
        cerr << "  Waiting for server to come up ..." << endl;
      } else {
        cerr << "ERROR - SpolSysconRelay::Run" << endl;
        cerr << "  Connecting to server" << endl;
        cerr << "  " << _sock.getErrStr() << endl;
      }
    }

    return -1;

  }

  return 0;

}

//////////////////////////////////////////////
// close socket

void RsmInfo::_closeSocket()
  
{
  _sock.close();
}

///////////////////////////////////////////////////////////////////
// Peek at buffer from socket
// Returns 0 on success, -1 on failure

int RsmInfo::_peekAtBuffer(void *buf, int nbytes)

{
  
  while (true) {
    PMU_auto_register("peekAtBuffer");
    if (_sock.peek(buf, nbytes, 1000) == 0) {
      return 0;
    } else {
      if (_sock.getErrNum() == Socket::TIMED_OUT) {
        PMU_auto_register("Timed out ...");
        continue;
      }
      cerr << "ERROR - RsmInfo::_peekAtBuffer" << endl;
      cerr << "  " << _sock.getErrStr() << endl;
      return -1;
    }
  }
  
  return -1;

}

///////////////////////////////////////////////////////////////////
// Read in next packet, set id and load buffer.
// Returns 0 on success, -1 on failure

int RsmInfo::_readPacket(int &id, int &len, MemBuf &buf)

{

  // peek at the first 8 bytes to get length
  
  si32 packetTop[2];
  if (_peekAtBuffer(packetTop, sizeof(packetTop))) {
    cerr << "ERROR - RsmInfo::_readPacket" << endl;
    cerr << "  " << _sock.getErrStr() << endl;
    return -1;
  }
  
  // check ID for packet, and get its length

  id = packetTop[0];
  len = packetTop[1];

  // read it in
  
  buf.reserve(len);
  if (_sock.readBuffer(buf.getPtr(), len)) {
    cerr << "ERROR - RsmInfo::_readPacket" << endl;
    cerr << "  " << _sock.getErrStr() << endl;
    return -1;
  }

  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "RsmInfo:: read in packet, id, len: "
         << id << ", " << len << endl;
  }

  return 0;

}

/////////////////////////////////////////////
// handle a scan_segment packet

void RsmInfo::_handleScanSegment(iwrf_scan_segment_t &scanSegment)
  
{
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "=========== RMS SCAN SEGMENT ============" << endl;
    iwrf_scan_segment_print(stderr, scanSegment);
    cerr << "=========================================" << endl;
  }

  scanSegment.packet.seq_num = _seqNum++;

}

/////////////////////////////////////////////
// handle a ts_processing packet

void RsmInfo::_handleTsProcessing(iwrf_ts_processing_t &tsProcessing)
  
{
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "=========== RMS TS PROCESSING ============" << endl;
    iwrf_ts_processing_print(stderr, tsProcessing);
    cerr << "==========================================" << endl;
  }

  tsProcessing.packet.seq_num = _seqNum++;

}

/////////////////////////////////////////////
// handle an RSM packet

void RsmInfo::_handleRsmPacket(const rsm_pkt_t &rsm_pkt)
  
{

  rsm_msghdr_t hdr = rsm_pkt.header;

  if (_params.debug >= Params::DEBUG_EXTRA) {
    rsm_msghdr_print(stderr, hdr);
  }

  string moduleId = iwrf_safe_str(hdr.module_id, 11);
  if (moduleId == "INS_PM") {
    _handleRsmInsPm(rsm_pkt.payload.pm);
  } else if (moduleId == "SYSCON") {
    _handleRsmSyscon(rsm_pkt.payload.syscon);
  } else if (moduleId == "ANTCON") {
    _handleRsmAntcon(rsm_pkt.payload.antcon);
  }

}

/////////////////////////////////////////////
// handle RSM instrument power meter data

void RsmInfo::_handleRsmInsPm(const rsm_ins_pm_t &pm)
  
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    rsm_ins_pm_print(stderr, pm);
  }

}

/////////////////////////////////////////////
// handle RSM syscon data

void RsmInfo::_handleRsmSyscon(const rsm_syscon_t &syscon)
  
{

  if (_params.debug >= Params::DEBUG_EXTRA) {
    rsm_syscon_print(stderr, syscon);
  }

}

/////////////////////////////////////////////
// handle RSM antcon data

void RsmInfo::_handleRsmAntcon(const rsm_antcon_t &antcon)
  
{

  if (_params.debug >= Params::DEBUG_EXTRA) {
    rsm_antcon_print(stderr, antcon);
  }

}

