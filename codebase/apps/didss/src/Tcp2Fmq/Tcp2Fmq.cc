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
// Tcp2Fmq.cc
//
// Tcp2Fmq object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// August 1999
//
///////////////////////////////////////////////////////////////

#include <dataport/bigend.h>
#include <toolsa/umisc.h>
#include <toolsa/Socket.hh>
#include <toolsa/pmu.h>
#include <toolsa/compress.h>
#include "Tcp2Fmq.hh"
using namespace std;

// Constructor

Tcp2Fmq::Tcp2Fmq(int argc, char **argv)

{

  isOK = true;

  // set programe name

  _progName = "Tcp2Fmq";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Problem with command line args" << endl;
    isOK = FALSE;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Problem with TDRP parameters" << endl;
    isOK = FALSE;
  }

  // initialize output FMQ

  if (_fmq.init(_params.output_fmq_url, _progName.c_str(),
	       _params.debug >= Params::DEBUG_VERBOSE,
		DsFmq::READ_WRITE, DsFmq::END,
	       _params.output_fmq_compressed,
	       _params.output_fmq_nslots,
	       _params.output_fmq_size)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Could not init FMQ for url: "
	 << _params.output_fmq_url << endl;
    isOK = false;
    return;
  }

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  return;

}

// destructor

Tcp2Fmq::~Tcp2Fmq()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int Tcp2Fmq::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  while (true) {
    
    PMU_auto_register("Run: waiting to connect to input data");
    
    // open socket for input
    
    Socket input;
    if (input.open(_params.input_tcp_host,
		   _params.input_tcp_port) == 0) {
      if (_handleInput(input)) {
	cerr << "Error - reading TCP socket, host: "
	     << _params.input_tcp_host << ", port: "
	     << _params.input_tcp_port << endl;
      }
      input.close();
    }

    sleep (1);

  }

  return (0);

}


//////////////////////////////////////////////////
// _handleInput

int Tcp2Fmq::_handleInput (Socket &input)
{

  // register with procmap
  
  PMU_auto_register("_handleInput");

  while (true) {
    
    PMU_auto_register("Run: waiting to read input data");
    
    if (input.readMessage(1000) == 0) {

      cerr << "nBytes read: " << input.getNumBytes() << endl;
      
      void *uncompressed_buffer;
      unsigned long nbytes_uncompressed;
      
      uncompressed_buffer = ta_decompress((void *) input.getData(),
					  &nbytes_uncompressed);
      
      if (_fmq.writeMsg(0, 0, uncompressed_buffer, nbytes_uncompressed)) {
	cerr << "ERROR - " << _progName << ": writing to FMQ." << endl;
      }

      if (uncompressed_buffer != NULL) {
	cerr << "nBytes uncompressed: " << nbytes_uncompressed << endl;
	ta_compress_free(uncompressed_buffer);
      }

    } else if (input.getErrNum() != SockUtil::TIMED_OUT) {

      cerr << "ERROR - reading socket: " << input.getErrString() << endl;
      cerr << "  errNum: " << input.getErrNum() << endl;
      return (-1);

    }

  }
  
  return (0);

}


