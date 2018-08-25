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
// MultMsgFmq2Fmq.cc
//
// MultMsgFmq2Fmq object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 1999
//
///////////////////////////////////////////////////////////////////////
//
// MultMsgFmq2Fmq reads an FMQ into which multiple messages have been 
// put by Fmq2MultMsgFmq, unpacks the messages and writes the individual 
// messages to an output FMQ. See Fmq2MultMsgFmq for details on writing 
// the multiple-message FMQ.
//
///////////////////////////////////////////////////////////////////////

#include <iomanip>
#include <dataport/bigend.h>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/compress.h>
#include <toolsa/MsgLog.hh>
#include <didss/DsMessage.hh>
#include <didss/DsMsgPart.hh>
#include <Fmq/DsFmq.hh>
#include "MultMsgFmq2Fmq.hh"
using namespace std;

// Constructor

MultMsgFmq2Fmq::MultMsgFmq2Fmq(int argc, char **argv)

{

  isOK = true;

  // set programe name

  _progName = "MultMsgFmq2Fmq";
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

MultMsgFmq2Fmq::~MultMsgFmq2Fmq()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int MultMsgFmq2Fmq::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  while (true) {
    _run();
    sleep(10);
    cerr << "MultMsgFmq2Fmq::Run:" << endl;
    cerr << "  Trying to contact input server at url: "
	 << _params.input_url << endl;
    cerr << "  Trying to contact output server at url: "
	 << _params.output_url << endl;
  }

  return (0);

}

//////////////////////////////////////////////////
// _run

int MultMsgFmq2Fmq::_run ()
{

  // register with procmap
  
  PMU_auto_register("_run");

  // open input and output FMQ's

  DsFmq input, output;
  MsgLog msgLog(_progName);

  if (input.initReadBlocking(_params.input_url,
			     _progName.c_str(),
			     _params.debug >= Params::DEBUG_VERBOSE,
			     DsFmq::END,
			     _params.msecs_sleep_blocking,
			     &msgLog)) {
    cerr << "ERROR - MultMsgFmq2Fmq::_run" << endl;
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
    cerr << "ERROR - MultMsgFmq2Fmq::_run" << endl;
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

  // create DsMessage object for reading mult messages

  DsMessage inMsg;
  
  while (true) {
    
    PMU_auto_register("_run: read loop");
    
    if (input.readMsgBlocking(DsFmq::multMessageType)) {
      cerr << "ERROR - MultMsgFmq2Fmq::_run" << endl;
      cerr << "  Cannot read from FMQ at url: " << _params.input_url << endl;
      return -1;
    }
    
    if (_params.debug) {
      cerr << "    Read message, len: " << input.getMsgLen() << endl;
    }
    
    // disassemble the message
    
    if (inMsg.disassemble(input.getMsg(), input.getMsgLen())) {
      cerr << "ERROR - MultMsgFmq2Fmq::_run" << endl;
      cerr << "  Cannot disassemble message from FMQ at url: "
	   << _params.input_url << endl;
      continue;
    }
    
    if (inMsg.getType() != DsFmq::multMessageType) {
      cerr << "ERROR - MultMsgFmq2Fmq::_run" << endl;
      cerr << "  Message is incorrect type: " << inMsg.getType() << endl;
      cerr << "  Should be type DsFmq::multMessageType: "
	   << DsFmq::multMessageType << endl;
      cerr << "  Ignoring this message." << endl;
      continue;
    }

    int nParts = inMsg.partExists(DsFmq::multMessagePart);
    if (_params.debug) {
      cerr << "    Message has nParts: " << nParts << endl;
    }
    
    for (int i = 0; i < nParts; i++) {
      DsMsgPart *part = inMsg.getPartByType(DsFmq::multMessagePart, i);
      if (part != NULL) {

	// recover the type and subtype from the start of the message

	si32 msgType, msgSubtype;
	memcpy(&msgType, part->getBuf(), sizeof(si32));
	BE_to_array_32(&msgType, sizeof(si32));
	memcpy(&msgSubtype, (ui08 *) part->getBuf() + sizeof(si32),
	       sizeof(si32));
	BE_to_array_32(&msgSubtype, sizeof(si32));

	// the actual message is 8 bytes from the start

	void *fmqMsg = (void *) ((ui08 *) part->getBuf() + 2 * sizeof(si32));
	int fmqMsgSize = part->getLength() - 2 * sizeof(si32);

	if (_params.debug >= Params::DEBUG_VERBOSE) {
	  cerr << "    Read message, len: " << setw(8) << fmqMsgSize
	       << ", type: " << setw(8) << msgType
	       << ", subtype: " << setw(8) << msgSubtype
	       << endl;
	}

	if (output.writeMsg(msgType, msgSubtype,
			    fmqMsg, fmqMsgSize)) {
	  cerr << "ERROR - MultMsgFmq2Fmq::_run" << endl;
	  cerr << "  Cannot write to FMQ at url: "
	       << _params.output_url << endl;
	  return -1;
	}
	if (_params.debug >= Params::DEBUG_VERBOSE) {
	  cerr << "  Writing message, nbytes: " << fmqMsgSize << endl;
	}
      }
    } // i
    
  } // while
  
  return (0);

}

