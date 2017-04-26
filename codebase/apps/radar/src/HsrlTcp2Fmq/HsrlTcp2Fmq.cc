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
// HsrlTcp2Fmq.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2017
//
///////////////////////////////////////////////////////////////
//
// HsrlTcp2Fmq reads raw HSRL fields from the instrument server via
// TCP/IP. It saves the data out to a file message queue (FMQ), which
// can be read by multiple clients.
//
////////////////////////////////////////////////////////////////

#include <iostream>
#include <cerrno>
#include <sys/stat.h>
#include <dataport/swap.h>
#include <toolsa/pmu.h>
#include <toolsa/uusleep.h>
#include <toolsa/DateTime.hh>
#include <radar/HsrlRawRay.hh>
#include "HsrlTcp2Fmq.hh"

using namespace std;

// Constructor

HsrlTcp2Fmq::HsrlTcp2Fmq(int argc, char **argv)
  
{

  isOK = true;

  _sockTimedOut = false;
  _timedOutCount = 0;
  _unknownMsgId = false;
  _unknownCount = 0;

  // set programe name
 
  _progName = "HsrlTcp2Fmq";

  // get command line args
  
  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = false;
    return;
  }

  // set up hearbeat func for reading from socket

  _heartBeatFunc = PMU_auto_register;
  if (_params.do_not_register_on_read) {
    _heartBeatFunc = NULL;
  }

  // initialize the output FMQ

  if (_openOutputFmq()) {
    isOK = false;
    return;
  }

  // init process mapper registration

  if (_params.register_with_procmap) {
    PMU_auto_init((char *) _progName.c_str(),
                  _params.instance,
                  PROCMAP_REGISTER_INTERVAL);
  }
  
  return;
  
}

// destructor

HsrlTcp2Fmq::~HsrlTcp2Fmq()

{

  // close socket

  _sock.close();

  // close FMQ

  _outputFmq.closeMsgQueue();

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int HsrlTcp2Fmq::Run ()
{

  PMU_auto_register("Run");
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Running HsrlTcp2Fmq - verbose debug mode" << endl;
  } else if (_params.debug) {
    cerr << "Running HsrlTcp2Fmq - debug mode" << endl;
  }
  if (_params.debug) {
    cerr << "  FMQ: " << _params.output_fmq_path << endl;
    cerr << "  nSlots: " << _params.output_fmq_nslots << endl;
    cerr << "  nBytes: " << _params.output_fmq_size << endl;
  }
  
  int iret = 0;

  while (true) {
    
    PMU_auto_register("Opening socket");
    
    // connect socket to server
    
    if (_sock.open(_params.hsrl_tcp_server_host,
                   _params.hsrl_tcp_server_port,
                   10000)) {
      if (_sock.getErrNum() == Socket::TIMED_OUT) {
	if (_params.debug) {
	  cerr << "  Waiting for time series server to come up ..." << endl;
          cerr << "    host: " << _params.hsrl_tcp_server_host << endl;
          cerr << "    port: " << _params.hsrl_tcp_server_port << endl;
	}
      } else {
	if (_params.debug) {
	  cerr << "ERROR - HsrlTcp2Fmq::Run" << endl;
	  cerr << "  Connecting to server" << endl;
	  cerr << "  " << _sock.getErrStr() << endl;
	}
        iret = -1;
      }
      umsleep(1000);
      continue;
    }

    // read from the server
    
    if (_readFromServer()) {
      iret = -1;
    }

    _sock.close();
    
  } // while(true)

  return iret;
  
}

/////////////////////////////
// read data from the server

int HsrlTcp2Fmq::_readFromServer()

{

  if (_params.debug) {
    cerr << "Reading from server ...." << endl;
  }

  // read data

  while (true) {
    
    if (!_params.do_not_register_on_read) {
      PMU_auto_register("Reading data");
    }

    // read packet from HSRL server
    
    if (_readMessage()) {
      if (!_sockTimedOut && !_unknownMsgId) {
        // error
        cerr << "ERROR - HsrlTcp2Fmq::_readFromServer" << endl;
        return -1;
      }
      // on timeout, skip to next message
      if (_sockTimedOut) {
        if (_params.debug) {
          cerr << " socket timed out" << endl;
        }
        continue;
      }
    }

    // write to the FMQ

    _writeToOutputFmq();
    
  } // while (true)

  return 0;

}

///////////////////////////////////////////////////////////////////
// Read in next message, set id and load buffer.
// Returns 0 on success, -1 on failure

int HsrlTcp2Fmq::_readMessage()
  
{

  while (true) {

    if (!_params.do_not_register_on_read) {
      PMU_auto_register("_readMessage");
    }

    if (_readTcpPacket() == 0) {
      return 0;
    }
    
    if (!_sockTimedOut && !_unknownMsgId) {
      // socket error
      cerr << "ERROR - HsrlTcp2Fmq::_readFromServer" << endl;
      return -1;
    }

  } // while

}

///////////////////////////////////////////////////////////////////
// Read in next packet, set id and load buffer.
// Returns 0 on success, -1 on failure

int HsrlTcp2Fmq::_readTcpPacket()

{
  
  si64 packetTop[2];
  
  // read the first 8 bytes (id, len), waiting for up to 1 second

  _unknownMsgId = false;
  _sockTimedOut = false;
    
  if (_sock.readBufferHb(packetTop, sizeof(packetTop),
                         sizeof(packetTop), _heartBeatFunc, 10000)) {
    
    // check for timeout
    if (_sock.getErrNum() == Socket::TIMED_OUT) {
      if (!_params.do_not_register_on_read) {
        PMU_auto_register("Timed out ...");
      }
      _sockTimedOut = true;
      _timedOutCount++;
      if (_params.debug) {
        cerr << "Timed out reading time series server, host, port: "
             << _params.hsrl_tcp_server_host << ", "
             << _params.hsrl_tcp_server_port << endl;
      }
    } else {
      _sockTimedOut = false;
      cerr << "ERROR - HsrlTcp2Fmq::_readTcpPacket" << endl;
      cerr << "  " << _sock.getErrStr() << endl;
    }
    return -1;
  }
  _timedOutCount = 0;
  
  // check ID for packet, and get its length
  
  _packetId = packetTop[0];
  _packetLen = packetTop[1];
  
  if (HsrlRawRay::idIsSwapped(_packetId)) {
    SWAP_array_64(packetTop, 16);
    _packetId = packetTop[0];
    _packetLen = packetTop[1];
    cerr << "INFO - HSRL packet data is swapped" << endl;
    cerr << "INFO - SHOULD NOT REACH HERE" << endl;
  }

  // make sure the size is reasonable
  
  if (_packetLen > 1000000 ||
      _packetLen < (int) sizeof(packetTop)) {
    cerr << "ERROR - HsrlTcp2Fmq::_readTcpPacket" << endl;
    cerr << "  Bad packet length: " << _packetLen << endl;
    fprintf(stderr, "  id: 0x%lx\n", _packetId);
    cerr << "  Need to reconnect to server to resync" << endl;
    return -1;
  }

  // check message is valid

  if (_packetId != HsrlRawRay::cookie) {
    cerr << "ERROR - HsrlTcp2Fmq::_readTcpPacket" << endl;
    cerr << "  Bad message id: " << _packetId << endl;
    cerr << "  Should be cookie: " << HsrlRawRay::cookie << endl;
    _unknownMsgId = true;
    return -1;
  }
  
  // compute nbytes still to read in
  
  int nBytesRemaining = _packetLen - sizeof(packetTop);

  // reserve space in the buffer
  
  _inputBuf.reserve(_packetLen);
  
  // copy the packet top into the start of the buffer
  
  memcpy(_inputBuf.getPtr(), packetTop, sizeof(packetTop));
  char *readPtr = (char *) _inputBuf.getPtr() + sizeof(packetTop);
  
  // read in the remainder of the buffer
  
  if (_sock.readBufferHb(readPtr, nBytesRemaining, 1024,
                         _heartBeatFunc, 10000)) {
    cerr << "ERROR - HsrlTcp2Fmq::_readTcpPacket" << endl;
    cerr << "  " << _sock.getErrStr() << endl;
    return -1;
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Read in HSRL raw ray packet" << endl;
    cerr << "  packetId: " << _packetId << endl;
    cerr << "  packetLen: " << _packetLen << endl;
  }
  
  return 0;

}

///////////////////////////////////////
// open the output FMQ
// returns 0 on success, -1 on failure

int HsrlTcp2Fmq::_openOutputFmq()

{

  // initialize the output FMQ
  
  if (_outputFmq.initReadWrite
      (_params.output_fmq_path,
       _progName.c_str(),
       _params.debug >= Params::DEBUG_EXTRA, // set debug?
       Fmq::END, // start position
       false,    // compression
       _params.output_fmq_nslots,
       _params.output_fmq_size)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Cannot initialize FMQ: " << _params.output_fmq_path << endl;
    cerr << "  nSlots: " << _params.output_fmq_nslots << endl;
    cerr << "  nBytes: " << _params.output_fmq_size << endl;
    cerr << _outputFmq.getErrStr() << endl;
    return -1;
  }
  _outputFmq.setSingleWriter();
  if (_params.data_mapper_report_interval > 0) {
    _outputFmq.setRegisterWithDmap(true, _params.data_mapper_report_interval);
  }

  return 0;

}

///////////////////////////////////////
// write to output FMQ if ready
// returns 0 on success, -1 on failure

int HsrlTcp2Fmq::_writeToOutputFmq()

{

  PMU_auto_register("writeToFmq");

  if (_params.debug >= Params::DEBUG_EXTRA) {
    HsrlRawRay rawRay;
    rawRay.dserialize((const char *) _inputBuf.getPtr(), _inputBuf.getLen());
    rawRay.printTcpHdr(cerr);
    rawRay.printMetaData(cerr);
  }
  
  if (_outputFmq.writeMsg(0, 0, _inputBuf.getPtr(), _inputBuf.getLen())) {
    cerr << "ERROR - HsrlTcp2Fmq" << endl;
    cerr << "  Cannot write FMQ: " << _params.output_fmq_path << endl;
    return -1;
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "==>> Wrote msg, len, path: "
         << _inputBuf.getLen() << ", "
         << _params.output_fmq_path << endl;
  }

  return 0;

}
    
