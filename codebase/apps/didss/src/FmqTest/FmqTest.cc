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
// FmqTest.cc
//
// FmqTest object
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2021
//
///////////////////////////////////////////////////////////////////////
//
// FmqTest reads an input FMQ and copies the contents unchanged to an 
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
#include "FmqTest.hh"
using namespace std;

// Constructor

FmqTest::FmqTest(int argc, char **argv)

{

  isOK = true;

  // set programe name

  _progName = "FmqTest";
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

  // allocate output FMQs

  _outputFmqs = _outputFmqs_.alloc(_params.output_urls_n);
  _prevTimeForOpen = 0;

  // logging

  _msgLog = new MsgLog(_progName);

  // init process mapper registration
  
  PMU_auto_init(_progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  return;

}

// destructor

FmqTest::~FmqTest()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int FmqTest::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  while (true) {
    _run();
    sleep(10);
    cerr << "FmqTest::Run:" << endl;
    cerr << "  Trying to contact input server at url: "
	 << _params.input_url << endl;
  }

  return (0);

}

//////////////////////////////////////////////////
// _run

int FmqTest::_run ()
{

  // register with procmap
  
  PMU_auto_register("_run");

  // open input FMQ

  if (_inputFmq.initReadBlocking(_params.input_url,
                                 _progName.c_str(),
                                 _params.debug >= Params::DEBUG_VERBOSE,
                                 DsFmq::END,
                                 _params.msecs_sleep_blocking,
                                 _msgLog)) {
    cerr << "ERROR - FmqTest::Run" << endl;
    cerr << "  Cannot open input FMQ at url: " << _params.input_url << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "Opened input from URL: " << _params.input_url << endl;
  }
  PMU_auto_register("opened input");

  // open output FMQ's

  _openOutputFmqs();
  PMU_auto_register("Initial open output FMQs");

  // read / write

  while (true) {
    
    PMU_auto_register("Run: read loop");
    _openOutputFmqs();

    // read 
    
    bool gotOne;
    if (_inputFmq.readMsg(&gotOne)) {
      cerr << "ERROR - FmqTest::Run" << endl;
      cerr << "  Cannot read from FMQ at url: " << _params.input_url << endl;
      _inputFmq.closeMsgQueue();
      return -1;
    }

    if (gotOne) {

      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << "    Read message, len: " << setw(8) << _inputFmq.getMsgLen()
	     << ", type: " << setw(8) << _inputFmq.getMsgType()
	     << ", subtype: " << setw(8) << _inputFmq.getMsgSubtype()
	     << endl;
      }

      // add message to write cache
      
      for (size_t ii = 0; ii < _outputFmqs_.size(); ii++) {
        DsFmq &fmq = _outputFmqs[ii];
        if (fmq.isOpen()) {
          PMU_auto_register("write to cache");
          fmq.addToWriteCache(_inputFmq.getMsgType(), _inputFmq.getMsgSubtype(),
                               _inputFmq.getMsg(), _inputFmq.getMsgLen());
        }
      }

    }
    
    // write

    for (size_t ii = 0; ii < _outputFmqs_.size(); ii++) {
      DsFmq &fmq = _outputFmqs[ii];
      PMU_auto_register("write to output");
      int cacheSize = fmq.getWriteCacheSize();
      if (cacheSize > 0 && (!gotOne || cacheSize > _params.max_cache_size)) {
	if (_params.debug >= Params::DEBUG_VERBOSE) {
	  cerr << "FmqTest - writing cache, size: " << cacheSize << endl;
	}
	if (fmq.writeTheCache()) {
	  cerr << "ERROR - FmqTest::Run" << endl;
	  cerr << "  Cannot write to FMQ at url: "
	       << _params._output_urls[ii] << endl;
          fmq.closeMsgQueue();
	}
      }
    } // ii
    
    // sleep if no data available
    
    if (!gotOne) {
      umsleep(20);
    }
    
  } // while
  
  return (0);

}

//////////////////////////////////////////////////
// open output FMQ's

void FmqTest::_openOutputFmqs()

{

  // check if 60 secs has elapsed since last open

  time_t now = time(NULL);
  double timeSinceOpen = (double) now - (double) _prevTimeForOpen;
  if (timeSinceOpen < 60) {
    return;
  }
  _prevTimeForOpen = now;

  for (size_t ii = 0; ii < _outputFmqs_.size(); ii++) {
    
    if (_params.debug) {
      cerr << "Opening FMQ to URL: " << _params._output_urls[ii] << endl;
    }
    
    DsFmq &fmq = _outputFmqs[ii];
    if (fmq.isOpen()) {
      continue;
    }
    
    if (fmq.initReadWrite(_params._output_urls[ii],
                           _progName.c_str(),
                           (_params.debug >= Params::DEBUG_VERBOSE),
                           DsFmq::END,
                           (_params.output_compression !=
                            Params::NO_COMPRESSION),
                           _params.output_n_slots,
                           _params.output_buf_size,
                           _params.msecs_sleep_blocking,
                           _msgLog)) {
      cerr << "WARNING - FmqTest::Run" << endl;
      cerr << "  Cannot open output FMQ at url: "
	   << _params._output_urls[ii] << endl;
      continue;
    }

    if (_params.data_mapper_report_interval > 0) {
      fmq.setRegisterWithDmap(true, _params.data_mapper_report_interval);
    }
    
    if (_params.debug) {
      cerr << "Opened output to URL: " << _params._output_urls[ii] << endl;
    }
    
    if (_params.output_compression != Params::NO_COMPRESSION) {
      switch(_params.output_compression) {
      case Params::RLE_COMPRESSION:
	fmq.setCompressionMethod(TA_COMPRESSION_RLE);
	break;
      case Params::LZO_COMPRESSION:
	fmq.setCompressionMethod(TA_COMPRESSION_LZO);
	break;
      case Params::ZLIB_COMPRESSION:
	fmq.setCompressionMethod(TA_COMPRESSION_ZLIB);
	break;
      case Params::BZIP_COMPRESSION:
	fmq.setCompressionMethod(TA_COMPRESSION_BZIP);
	break;
      default:
	break;
      }
    } // if (_params.output_compression ...

    if (_params.write_blocking) {
      fmq.setBlockingWrite();
    }

  } // ii

}
