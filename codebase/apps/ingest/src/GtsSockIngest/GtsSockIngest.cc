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
// GtsSockIngest.cc
//
// GtsSockIngest object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 1999
//
///////////////////////////////////////////////////////////////

#include <dataport/bigend.h>
#include <toolsa/umisc.h>
#include <toolsa/file_io.h>
#include <toolsa/pmu.h>
#include <toolsa/ServerSocket.hh>
#include <toolsa/MemBuf.hh>
#include <didss/LdataInfo.hh>
#include "GtsSockIngest.hh"
#include "Output.hh"
using namespace std;

// Constructor

GtsSockIngest::GtsSockIngest(int argc, char **argv)

{

  isOK = true;

  // set programe name

  _progName = "GtsSockIngest";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = FALSE;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = FALSE;
  }

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  return;

}

// destructor

GtsSockIngest::~GtsSockIngest()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int GtsSockIngest::Run ()
{

  while (true) {

    PMU_auto_register("Trying to run");

    if (_tryConnect()) {
      cerr << "ERROR - GtsSockIngest" << endl;
      cerr << "  Run failed - retrying" << endl;
    }

    for (int i = 0; i < 60; i++) {
      PMU_auto_register("Waiting to retry _run()");
      umsleep (1000);
    }

  }

}

//////////////////////////////////////////////////
// try to connect

int GtsSockIngest::_tryConnect()
{

  ServerSocket serverSock;

  if (serverSock.openServer(_params.input_port)) {
    cerr << "ERROR - cannot open server" << endl;
    cerr << "  " << serverSock.getErrStr() << endl;
    return -1;
  }

  Socket *input;
  while ((input = serverSock.getClient(1000)) == NULL) {
    if (serverSock.getErrNum() != SockUtil::TIMED_OUT) {
      cerr << "ERROR - cannot get client" << endl;
      cerr << "  " << serverSock.getErrStr() << endl;
      return -1;
    }
    PMU_auto_register("Waiting for connect");  
    if (_params.debug) {
      cerr << "Waiting for connection, port: " << _params.input_port << endl;
    }
  }

  if (_params.debug) {
    cerr << "Got connection, port: " << _params.input_port << endl;
  }

  int iret = _getData(input);
  delete input;
  return iret;

}

//////////////////////////////////////////////////
// get data

int GtsSockIngest::_getData(Socket *input)
{

  // register with procmap
  
  PMU_auto_register("_getData");

  // output object

  Output out(_progName, _params);

  if (out.open()) {
    return (-1);
  }

  // memory buffer for input line

  MemBuf lineBuf;

  // start time
  time_t startTime = time(NULL);
  
  // ending time
  time_t endTime = (((startTime / _params.output_interval) + 1)
		    * _params.output_interval);

  if (_params.debug) {
    cerr << "End of current period: " << utimstr(endTime) << endl;
  }
  
  char eol = '\n';
  char ctrlc = 0x03;
  char ctrlk = 0x0b;
  char cc = 0;
  char cc1 = 0, cc2 = 0, cc3 = 0, cc4 = 0;
  bool eom = false;

  bool forever = true;
  while (forever) {
    
    bool timedOut = true;
    while (timedOut) {
      
      int iret = input->readSelect(1000);
      
      if (iret == 0) {
        timedOut = false;
        continue; // break out
      } else {
        if (input->getErrNum() != Socket::TIMED_OUT) {
          cerr << "ERROR - reading socket" << endl;
          cerr << input->getErrStr() << endl;
          return -1;
        }
      }
      
      // timed out

      PMU_auto_register("Waiting for data");
      
      if (_params.force_output_if_stalled) {
	
	// check if we should close out file and start new one
	
	time_t now = time(NULL);
	if (now >= endTime) {
	  out.close();
	  if (out.save(endTime)) {
	    return (-1);
	  }
	  endTime += _params.output_interval;
	  if (out.open()) {
	    return (-1);
	  }
	  if (_params.debug) {
	    cerr << "End of current period: " << utimstr(endTime) << endl;
	  }
	} // if (now >= endTime)
	
      } // if (_params.force_output_if_stalled) 

    } // while (timedOut)

    if (input->readBuffer(&cc, 1, 1000)) {
      cerr << "ERROR - GtsSockIngest::Run" << endl;
      cerr << "Cannot read socket, port: " << _params.input_port << endl;
      return (-1);
    }

    PMU_auto_register("Reading data");

    if (_params.debug) {
      fprintf(stderr, "%c", cc);
    }
    
    // add char to line buffer
    if (cc != '\r') {
      lineBuf.add(&cc, 1);
    }

    //  check for end of a message

    if (_params.end_of_message_check == Params::EOM_CHECK_NNNN) {
      cc4 = cc3;
      cc3 = cc2;
      cc2 = cc1;
      cc1 = cc;
      if (cc1 == 'N' && cc2 == 'N' && cc3 == 'N' && cc4 == 'N') {
	eom = true;
	lineBuf.add(&eol, 1);
      } else {
	eom = false;
      }
    } else if (_params.end_of_message_check == Params::EOM_CHECK_CTRLC) {
      if (cc == ctrlc) {
	eom = true;
      } else {
	eom = false;
      }
    } else if (_params.end_of_message_check == Params::EOM_CHECK_CTRLKC) {
      cc2 = cc1;
      cc1 = cc;
      if (cc2 == ctrlk && cc1 == ctrlc) {
	eom = true;
      } else {
	eom = false;
      }
    } else if (_params.end_of_message_check == Params::EOM_CHECK_EQUALS) {
      cc2 = cc1;
      cc1 = cc;
      if (cc2 == '=' && cc1 == '\n') {
	eom = true;
      } else {
	eom = false;
      }
    } else {
      if (cc == '\n') {
	eom = true;
      } else {
	eom = false;
      }
    }
    
    // respond to end of line
    
    if (cc == '\n' || eom) {
      
      // write line to output
      if (out.putBuf(lineBuf)) {
	return (-1);
      }
      
      // clear line buffer
      lineBuf.reset();

    }

    // respond to end of message

    if (eom) {
      
      // check if we should close out file and start new one
      
      time_t now = time(NULL);
      if (now >= endTime) {
	out.close();
	if (out.save(endTime)) {
	  return (-1);
	}
	endTime += _params.output_interval;
	if (out.open()) {
	  return (-1);
	}
	// start file with ctrol chars if required
	if (_params.end_of_message_check == Params::EOM_CHECK_CTRLC) {
	  lineBuf.add(&ctrlc, 1);
	} else if (_params.end_of_message_check == Params::EOM_CHECK_CTRLKC) {
	  lineBuf.add(&ctrlk, 1);
	  lineBuf.add(&ctrlc, 1);
	}
	if (_params.debug) {
	  cerr << "End of current period: " << utimstr(endTime) << endl;
	}
      } // if (now >= endTime)
      
    } // if (eom)
    
  } // while (forever)
  
  out.close();
  out.save(endTime);
  return (0);

}

