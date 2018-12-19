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
// Fmq2MultMsgFmq.cc
//
// Fmq2MultMsgFmq object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 1999
//
///////////////////////////////////////////////////////////////////////
//
// Fmq2MultMsgFmq packs a number of input messages from an FMQ into a 
// single output message, which is then written to an output FMQ. The 
// reasons for doing this are (a) to improve compression by allowing the 
// compression algorithm to work on a larger buffer and (b) to reduce 
// the number of remote writes made across a network link.
//
///////////////////////////////////////////////////////////////////////

#include <iomanip>
#include <dataport/bigend.h>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/MemBuf.hh>
#include <toolsa/compress.h>
#include <toolsa/MsgLog.hh>
#include <didss/DsMessage.hh>
#include <Fmq/DsFmq.hh>
#include "Fmq2MultMsgFmq.hh"
using namespace std;

// Constructor

Fmq2MultMsgFmq::Fmq2MultMsgFmq(int argc, char **argv)

{

  isOK = true;

  // set programe name

  _progName = "Fmq2MultMsgFmq";
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

Fmq2MultMsgFmq::~Fmq2MultMsgFmq()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int Fmq2MultMsgFmq::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  while (true) {
    _run();
    sleep(10);
    cerr << "Fmq2MultMsgFmq::Run:" << endl;
    cerr << "  Trying to contact input server at url: "
	 << _params.input_url << endl;
    cerr << "  Trying to contact output server at url: "
	 << _params.output_url << endl;
  }

  return (0);

}

//////////////////////////////////////////////////
// _run

int Fmq2MultMsgFmq::_run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  // open input and output FMQ's

  DsFmq input, output;
  MsgLog msgLog(_progName);

  if (input.initReadBlocking(_params.input_url,
			     _progName.c_str(),
			     _params.debug >= Params::DEBUG_VERBOSE,
			     DsFmq::END,
			     _params.msecs_sleep_blocking,
			     &msgLog)) {
    cerr << "ERROR - Fmq2MultMsgFmq::Run" << endl;
    cerr << "  Cannot open input FMQ at url: " << _params.input_url << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "Opened input from URL: " << _params.input_url << endl;
  }
  
  if (output.initReadWrite(_params.output_url,
			   _progName.c_str(),
			   (_params.debug >= Params::DEBUG_VERBOSE),
			   DsFmq::END,
			   (_params.output_compression !=
			    Params::NO_COMPRESSION),
			   _params.output_n_slots,
			   _params.output_buf_size,
			   _params.msecs_sleep_blocking,
			   &msgLog)) {
    cerr << "ERROR - Fmq2MultMsgFmq::Run" << endl;
    cerr << "  Cannot open output FMQ at url: " << _params.output_url << endl;
    return -1;
  }
  
  if (_params.debug) {
    cerr << "Opened output to URL: " << _params.output_url << endl;
  }
  
  if (_params.output_compression != Params::NO_COMPRESSION) {
    switch(_params.output_compression) {
    case Params::RLE_COMPRESSION:
      output.setCompressionMethod(TA_COMPRESSION_RLE);
      break;
    case Params::LZO_COMPRESSION:
      output.setCompressionMethod(TA_COMPRESSION_LZO);
      break;
    case Params::ZLIB_COMPRESSION:
      output.setCompressionMethod(TA_COMPRESSION_ZLIB);
      break;
    case Params::BZIP_COMPRESSION:
      output.setCompressionMethod(TA_COMPRESSION_BZIP);
      break;
    default:
      break;
    }
  }

  // create DsMessage object for combining messages

  MemBuf outBuf;
  DsMessage outMsg;
  int nMessages = 0;
  int nBytes = 0;
  
  while (true) {
    
    PMU_auto_register("Run: read loop");

    if (input.readMsgBlocking()) {
      cerr << "ERROR - Fmq2MultMsgFmq::Run" << endl;
      cerr << "  Cannot read from FMQ at url: " << _params.input_url << endl;
      return -1;
    }
    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "    Read message, len: " << setw(8) << input.getMsgLen()
	   << ", type: " << setw(8) << input.getMsgType()
	   << ", subtype: " << setw(8) << input.getMsgSubtype()
	   << endl;
    }

    // use a message buffer to prepend the fmq type and subtype
    // to the message we just read
    
    outBuf.free();
    si32 fmqMsgType = input.getMsgType();
    si32 fmqMsgSubtype = input.getMsgSubtype();
    outBuf.add(&fmqMsgType, sizeof(si32));
    outBuf.add(&fmqMsgSubtype, sizeof(si32));
    BE_from_array_32(outBuf.getPtr(), 2 * sizeof(si32));
    outBuf.add(input.getMsg(), input.getMsgLen());

    // add the part

    outMsg.addPart(DsFmq::multMessagePart, outBuf.getLen(), outBuf.getPtr());
    nMessages++;
    nBytes += outBuf.getLen();

    bool doWrite = false;
    if (_params.pack_decision == Params::PACK_NMESSAGES) {
      if (nMessages >= _params.n_messages_packed) {
	doWrite = true;
      }
    } else if (_params.pack_decision == Params::PACK_NBYTES) {
      if (nBytes >= _params.min_bytes_packed) {
	doWrite = true;
      }
    } else {
      if (nMessages >= _params.n_messages_packed ||
	  nBytes >= _params.min_bytes_packed) {
	doWrite = true;
      }
    }

    if (doWrite) {

      outMsg.setType(DsFmq::multMessageType);
      outMsg.assemble();
    
      if (output.writeMsg(DsFmq::multMessageType, DsFmq::multMessagePart,
			  outMsg.assembledMsg(),
			  outMsg.lengthAssembled())) {
	cerr << "ERROR - Fmq2MultMsgFmq::Run" << endl;
	cerr << "  Cannot write to FMQ at url: " << _params.output_url << endl;
	return -1;
      }

      if (_params.debug) {
	cerr << "  Writing mult message, nmess: " << nMessages
	     << ", nbytes: " << nBytes << endl;
      }

      outMsg.clearAll();
      nMessages = 0;
      nBytes = 0;

    }

  }
  
  return (0);

}

