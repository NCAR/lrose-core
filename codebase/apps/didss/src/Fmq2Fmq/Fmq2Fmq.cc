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
////////////////////////////////////////////////////////////////////////
// Fmq2Fmq.cc
//
// Fmq2Fmq object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 1999
//
///////////////////////////////////////////////////////////////////////
//
// Fmq2Fmq reads an input FMQ and copies the contents unchanged to an 
// output FMQ. It is useful for reading data from a remote queue and 
// copying it to a local queue. The clients can then read the local 
// queue rather than all access the remote queue.
//
///////////////////////////////////////////////////////////////////////

#include <iomanip>
#include <dataport/bigend.h>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/compress.h>
#include <toolsa/MsgLog.hh>
#include <toolsa/TaArray.hh>
#include <Fmq/DsFmq.hh>
#include "Fmq2Fmq.hh"
using namespace std;

// Constructor

Fmq2Fmq::Fmq2Fmq(int argc, char **argv)

{

  isOK = true;

  // set programe name

  _progName = "Fmq2Fmq";
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
  
  PMU_auto_init(_progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  return;

}

// destructor

Fmq2Fmq::~Fmq2Fmq()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int Fmq2Fmq::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  while (true) {
    _run();
    sleep(10);
    cerr << "Fmq2Fmq::Run:" << endl;
    cerr << "  Trying to contact input server at url: "
	 << _params.input_url << endl;
    for (int i = 0; i < _params.output_urls_n; i++) {
      cerr << "  Trying to contact output server at url: "
	   << _params._output_urls[i] << endl;
    }
  }

  return (0);

}

//////////////////////////////////////////////////
// _run

int Fmq2Fmq::_run ()
{

  // register with procmap
  
  PMU_auto_register("_run");

  // open input and output FMQ's

  DsFmq input;
  TaArray<DsFmq> outputs_;
  DsFmq *outputs = outputs_.alloc(_params.output_urls_n);
  MsgLog msgLog(_progName);

  if (input.initReadBlocking(_params.input_url,
			     _progName.c_str(),
			     _params.debug >= Params::DEBUG_VERBOSE,
			     DsFmq::END,
			     _params.msecs_sleep_blocking,
			     &msgLog)) {
    cerr << "ERROR - Fmq2Fmq::Run" << endl;
    cerr << "  Cannot open input FMQ at url: " << _params.input_url << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "Opened input from URL: " << _params.input_url << endl;
  }
  PMU_auto_register("opened input");

  for (int i = 0; i < _params.output_urls_n; i++) {

    if (outputs[i].initReadWrite(_params._output_urls[i],
				 _progName.c_str(),
				 (_params.debug >= Params::DEBUG_VERBOSE),
				 DsFmq::END,
				 (_params.output_compression !=
				  Params::NO_COMPRESSION),
				 _params.output_n_slots,
				 _params.output_buf_size,
				 _params.msecs_sleep_blocking,
				 &msgLog)) {
      cerr << "ERROR - Fmq2Fmq::Run" << endl;
      cerr << "  Cannot open output FMQ at url: "
	   << _params._output_urls[i] << endl;
      return -1;
    }
    if (_params.data_mapper_report_interval > 0) {
      outputs[i].setRegisterWithDmap(true, _params.data_mapper_report_interval);
    }
    
    if (_params.debug) {
      cerr << "Opened output to URL: " << _params._output_urls[i] << endl;
    }
    
    if (_params.output_compression != Params::NO_COMPRESSION) {
      switch(_params.output_compression) {
      case Params::RLE_COMPRESSION:
	outputs[i].setCompressionMethod(TA_COMPRESSION_RLE);
	break;
      case Params::LZO_COMPRESSION:
	outputs[i].setCompressionMethod(TA_COMPRESSION_LZO);
	break;
      case Params::ZLIB_COMPRESSION:
	outputs[i].setCompressionMethod(TA_COMPRESSION_ZLIB);
	break;
      case Params::BZIP_COMPRESSION:
	outputs[i].setCompressionMethod(TA_COMPRESSION_BZIP);
	break;
      default:
	break;
      }
    } // if (_params.output_compression ...

    if (_params.write_blocking) {
      outputs[i].setBlockingWrite();
    }

  } // i

  PMU_auto_register("opened outputs");

  // read / write

  while (true) {
    
    PMU_auto_register("Run: read loop");

    // read 

    bool gotOne;
    if (input.readMsg(&gotOne)) {
      cerr << "ERROR - Fmq2Fmq::Run" << endl;
      cerr << "  Cannot read from FMQ at url: " << _params.input_url << endl;
      return -1;
    }
    
    if (gotOne) {

      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << "    Read message, len: " << setw(8) << input.getMsgLen()
	     << ", type: " << setw(8) << input.getMsgType()
	     << ", subtype: " << setw(8) << input.getMsgSubtype()
	     << endl;
      }

      // add message to write cache
      
      for (int ii = 0; ii < _params.output_urls_n; ii++) {
        PMU_auto_register("write to cache");
	outputs[ii].addToWriteCache(input.getMsgType(), input.getMsgSubtype(),
				    input.getMsg(), input.getMsgLen());
      }
      
    }
    
    // write

    for (int ii = 0; ii < _params.output_urls_n; ii++) {

      PMU_auto_register("write to output");

      int cacheSize = outputs[ii].getWriteCacheSize();
      if (cacheSize > 0 && (!gotOne || cacheSize > _params.max_cache_size)) {
	if (_params.debug >= Params::DEBUG_VERBOSE) {
	  cerr << "Fmq2Fmq - writing cache, size: " << cacheSize << endl;
	}
	if (outputs[ii].writeTheCache()) {
	  cerr << "ERROR - Fmq2Fmq::Run" << endl;
	  cerr << "  Cannot write to FMQ at url: "
	       << _params._output_urls[ii] << endl;
	  return -1;
	}
      }

    } // ii

    // sleep if no data available

    if (!gotOne) {
      umsleep(20);
    }
    
  }
  
  return (0);

}

