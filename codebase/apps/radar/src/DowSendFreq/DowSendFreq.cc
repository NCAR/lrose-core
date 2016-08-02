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
// DowSendFreq.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2014
//
///////////////////////////////////////////////////////////////
//
// DowSendFreq provides the RF frequency for both the high and low
// frequency radars, via a socket, to a device designed to control the
// magnetron frequencies. The control device will act as a server, to
// which this application will connect
//
///////////////////////////////////////////////////////////////////////

#include <iostream>
#include <iomanip>
#include <toolsa/uusleep.h>
#include <toolsa/pmu.h>
#include "DowSendFreq.hh"
#include "StatusReader.hh"

using namespace std;

// Constructor

DowSendFreq::DowSendFreq(int argc, char **argv)
  
{

  isOK = true;

  _highReader = NULL;
  _lowReader = NULL;
  _sock = NULL;
  
  // set programe name
  
  _progName = "DowSendFreq";
  
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

  // set up readers

  _highReader = new StatusReader(_params,
                                 _params.high_freq_input_fmq_name,
                                 _params.high_frequency_active,
                                 _params.high_freq_default_mhz);
  
  _lowReader = new StatusReader(_params,
                                _params.low_freq_input_fmq_name,
                                _params.low_frequency_active,
                                _params.low_freq_default_mhz);

  // init process mapper registration
  
  if (_params.register_with_procmap) {
    PMU_auto_init((char *) _progName.c_str(),
                  _params.instance,
                  PROCMAP_REGISTER_INTERVAL);
  }
  
}

// destructor

DowSendFreq::~DowSendFreq()

{

  if (_highReader) {
    delete _highReader;
  }
  
  if (_lowReader) {
    delete _lowReader;
  }

  if (_sock) {
    _sock->close();
    delete _sock;
  }

  if (_params.tcp_mode == Params::TCP_MODE_SERVER) {
    _server.close();
  }

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Main run method

int DowSendFreq::Run()
{
  
  PMU_auto_register("Run");
  
  if (_params.debug) {
    cerr << "Running DowSendFreq - extra verbose debug mode" << endl;
  } else if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Running DowSendFreq - verbose debug mode" << endl;
  } else if (_params.debug) {
    cerr << "Running DowSendFreq - debug mode" << endl;
  }
  
  // set up server if needed
  
  if (_params.tcp_mode == Params::TCP_MODE_SERVER) {
    if (_server.openServer(_params.local_listening_port)) {
      cerr << "ERROR - DowSendFreq" << endl;
      cerr << "  Cannot open server port for listening: "
           << _params.local_listening_port << endl;
      cerr << "  " << _server.getErrStr() << endl;
      return -1;
    }
  }
  
  int iret = 0;
  
  while (true) {
    
    PMU_auto_register("Reading ...");
    
    if (_highReader->readStatus() == 0) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "========== High Freq Status ========" << endl;
        cerr << "Time: " << _highReader->getLatestStatusTime().asString() << endl;
        cerr << "FreqMhz: "
             << setprecision(9)
             << _highReader->getLatestFreqMhz() << endl;
        cerr << _highReader->getLatestStatusXml();
      } else if (_params.debug) {
        cerr << "==>> HighFreqMhz: "
             << setprecision(9)
             << _highReader->getLatestFreqMhz() << endl;
      }
    }

    if (_lowReader->readStatus() == 0) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "========== Low Freq Status ========" << endl;
        cerr << "Time: " << _lowReader->getLatestStatusTime().asString() << endl;
        cerr << "FreqMhz: "
             << setprecision(9)
             << _lowReader->getLatestFreqMhz() << endl;
        cerr << _lowReader->getLatestStatusXml();
      } else if (_params.debug) {
        cerr << "==>>  LowFreqMhz: "
             << setprecision(9)
             << _lowReader->getLatestFreqMhz() << endl;
      }
    }

    // format up the data

    char messageOut[1024];
    sprintf(messageOut, "%.7f,%.7f\n",
            _highReader->getLatestFreqMhz(),
            _lowReader->getLatestFreqMhz());
    
    // send data

    _sendData(messageOut);

    // sleep a bit
    
    int sleepUsecs = (int) (_params.time_between_reads_secs * 1.0e6 + 0.5);
    uusleep(sleepUsecs);

  } // while

  return iret;

}

//////////////////////////////////////////////////
// Send the data to the contoller

int DowSendFreq::_sendData(const char *messageOut)
{

  if (_params.tcp_mode == Params::TCP_MODE_SERVER) {
    return _sendDataAsServer(messageOut);
  } else {
    return _sendDataAsClient(messageOut);
  }

}

//////////////////////////////////////////////////
// Send the data, acting as a server

int DowSendFreq::_sendDataAsServer(const char *messageOut)
{
  
  // check we have a connection

  if (_sock == NULL) {
    _sock = _server.getClient(100);
    if (_sock == NULL) {
      if (_params.debug) {
        cerr << "ERROR - _sendDataAsServer()" << endl;
        cerr << "  No client yet" << endl;
      }
      return -1;
    }
  }

  // write message
  
  if (_sock->writeBuffer(messageOut, strlen(messageOut) + 1)) {
    cerr << "ERROR - _sendDataAsServer()" << endl;
    cerr << "  Error writing response to client" << endl;
    cerr << "  " << _sock->getErrStr() << endl;
    _sock->close();
    delete _sock;
    _sock = NULL;
    return -1;
  }
  
  return 0;

}

  
//////////////////////////////////////////////////
// Send the data, acting as a client

int DowSendFreq::_sendDataAsClient(const char *messageOut)
{

  // check we are connected

  if (_sock == NULL) {
    _sock = new Socket;
    if (_sock->open(_params.remote_server_host,
                    _params.remote_server_port)) {
      if (_params.debug) {
        cerr << "ERROR - _sendDataAsClient()" << endl;
        cerr << "  No server available yet" << endl;
      }
      delete _sock;
      _sock = NULL;
      return -1;
    }
  }

  // write message
  
  if (_sock->writeBuffer(messageOut, strlen(messageOut) + 1)) {
    cerr << "ERROR - _sendDataAsClient()" << endl;
    cerr << "  Error writing data to server" << endl;
    cerr << "  " << _sock->getErrStr() << endl;
    _sock->close();
    delete _sock;
    _sock = NULL;
    return -1;
  }
  
  return 0;

}

  
