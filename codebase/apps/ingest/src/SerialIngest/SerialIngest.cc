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
// SerialIngest.cc
//
// SerialIngest object
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
#include <toolsa/MemBuf.hh>
#include "SerialIngest.hh"
#include "Output.hh"
using namespace std;

// Constructor

SerialIngest::SerialIngest(int argc, char **argv)

{

  isOK = true;
  _input = NULL;

  // set programe name

  _progName = "SerialIngest";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = FALSE;
    return;
  }

  // get TDRP params
  
  _paramsPath = const_cast<char*>(string("unknown").c_str());
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = FALSE;
  }

  // create input object

  if (_params.connection == Params::SERIAL) {
    _input = new TtyInput(_progName, _params);
  } else {
    _input = new TcpInput(_progName, _params);
  }

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  return;

}

// destructor

SerialIngest::~SerialIngest()

{

  if (_input) {
    delete _input;
  }
  
  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int SerialIngest::Run ()
{

  int iret = 0;

  while (true) {
    
    if (_run()) {
      cerr << "ERROR - SerialIngest::Run ()" << endl;
      cerr << "  Retrying in 10 secs ..." << endl;
      iret = -1;
    }
    
    for (int i = 0; i < 10; i++) {
      PMU_auto_register("Waiting to retry connect");
      umsleep (1000);
    }

  }

  return iret;

}

//////////////////////////////////////////////////
// Run

int SerialIngest::_run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  // open input

  if (_input->open()) {
    cerr << "ERROR - SerialIngest::Run" << endl;
    cerr << "Cannot open input device" << endl;
    _input->close();
    return -1;
  }

  // output object

  Output out(_progName, _params);
  if (!out.isOK()) {
    cerr << "ERROR - SerialIngest::Run" << endl;
    cerr << "  Invalid output object." << endl;
    cerr << "  Check dir '" << _params.output_dir_path << "' exists" << endl;
    return -1;
  }

  if (out.open()) {
    return -1;
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
  char ctrlm = '\r';
  char _null_ = '\0';
  char ctrlc = 0x03;
  char ctrlk = 0x0b;
  char cc = 0;
  char cc1 = 0, cc2 = 0, cc3 = 0, cc4 = 0;
  bool eom = false;

  bool forever = true;

  //set this to the initial start time.
  //if we never get any data, then every _params.dataTimeout secs
  // SerialIngest will reset
  time_t lastDataRecvd = time(NULL);
  
  while (forever) {

    bool timedOut = true;
    while (timedOut) {

      if (_input->readSelect(1000, timedOut)) {
        cerr << "ERROR - SerialIngest::_run" << endl;
        cerr << "  readSelect failed" << endl;
        _input->close();
        return -1;
      }
      
      if (timedOut) {

        PMU_auto_register("Waiting for data");
	time_t now = time(NULL);

	// if we have not received data in a while, 
	// maybe we should reset?
	if (_params.dataTimeout != -1)
	  if ((now - lastDataRecvd)  > _params.dataTimeout)
	    {
	      cerr << "No data received since"  << utimstr(now) << endl;
	      cerr << "dataTimeout exceeded: resetting SerialIngest" << endl;
	      out.close();
	      return -1;
	    }
	
        if (_params.force_output_if_stalled) {
          
	  // check if we should close out file and start new one
          
          if (now >= endTime) {
            out.close();
            if (out.save(endTime)) {
              return -1;
            }
            endTime += _params.output_interval;
            if (out.open()) {
              return -1;
            }
            if (_params.debug) {
              cerr << "End of current period: " << utimstr(endTime) << endl;
            }
          } // if (now >= endTime)
          
        } // if (_params.force_output_if_stalled)
	
      } // if (timedOut) 

    } // while (timedOut)
  
    if (_input->readChar(cc)) {
      cerr << "ERROR - SerialIngest::Run" << endl;
      cerr << "Cannot read serial port: " << _params.input_device << endl;
      _input->close();
      return -1;
    }

    PMU_auto_register("Reading data");

    lastDataRecvd = time(NULL);

    if (_params.filter_ctrlm) {
      if (cc == ctrlm) {
	continue;
      }
    }

    if (_params.filter_nulls) {
      if (cc == _null_) {
	continue;
      }
    }

    if (_params.filter_non_printable) {
      if (cc < 32) {
	// cerr << " --" << (int) cc << "-- ";
	bool match = FALSE;
	for (int i = 0; i < _params.allowable_non_printable_n; i++) {
	  if (cc == (int) _params._allowable_non_printable[i]) {
	    match = TRUE;
	    break;
	  }
	}
	if (!match) {
	  continue;
	}
      }
    }

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << cc;
    }

    // add char to line buffer
    lineBuf.add(&cc, 1);

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
	return -1;
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
	  return -1;
	}
	endTime += _params.output_interval;
	if (out.open()) {
	  return -1;
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
  return 0;

}

