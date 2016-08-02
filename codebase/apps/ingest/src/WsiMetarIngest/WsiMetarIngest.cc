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
// WsiMetarIngest.cc
//
// WsiMetarIngest object
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
#include <didss/LdataInfo.hh>
#include "WsiMetarIngest.hh"
#include "Output.hh"
using namespace std;

// Constructor

WsiMetarIngest::WsiMetarIngest(int argc, char **argv) 

{

  isOK = true;
  _input = NULL;

  // set programe name

  _progName = "WsiMetarIngest";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = FALSE;
    return;
  }

  // get TDRP params
  
  _paramsPath = "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = FALSE;
  }

  // set up input object

  if (_params.read_stdin) {
    _input = new StdInput(_params.input_buffer_size);
  } else {
    _input = new SockInput(_params.wsi_host, _params.wsi_port,
			   _params.input_buffer_size);
  }

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  return;

}

// destructor

WsiMetarIngest::~WsiMetarIngest()

{

  // unregister process

  PMU_auto_unregister();

  // free up

  if (_input) {
    delete _input;
  }

}

//////////////////////////////////////////////////
// Run

int WsiMetarIngest::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  // output object

  Output out(_progName, _params);
  if (!out.isOK()) {
    cerr << "ERROR - WsiMetarIngest::Run" << endl;
    cerr << "  Invalid output object." << endl;
    cerr << "  Check dir '" << _params.output_dir_path << "' exists" << endl;
    return (-1);
  }

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
  
  char cc;
  bool metarsInProgress = false;
  
  while (_input->read(&cc, 1) == 0) {
    
    if (metarsInProgress) {
      if (cc == 0x03) {
	metarsInProgress = false; // metar blocks end with ^C
      }
    }
    
    // add char to line buffer
    lineBuf.add(&cc, 1);

    // check for end of line

    if (cc == '\n') {

      char *line = (char *) lineBuf.getBufPtr();

      // metar blocks start with "METAR\r" or "METAR "
      // speci blocks start with "SPECI\r" or "SPECI "
      if (!metarsInProgress) {
	if (!memcmp(line, "METAR", 5) || !memcmp(line, "SPECI", 5)) {
	  if (line[5] == ' ' || line[5] == '\r') {
	    metarsInProgress = true;
	  }
	}
      }

      // write line to output
      if (metarsInProgress) {
	if (out.putBuf(lineBuf)) {
	  return (-1);
	}
      }

      // clear line buffer
      lineBuf.reset();

      // check if we should close out file and start new one

      if (!metarsInProgress) {
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
      }
	
    } // if (cc ...
    
  } // while

  out.close();
  out.save(endTime);
  return (0);

}

