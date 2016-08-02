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
// Udp2Tcp.cc
//
// Udp2Tcp object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// August 1999
//
///////////////////////////////////////////////////////////////

#include <dataport/bigend.h>
#include <toolsa/umisc.h>
#include <toolsa/ServerSocket.hh>
#include <toolsa/Socket.hh>
#include <toolsa/pmu.h>
#include <toolsa/compress.h>
#include <didss/DsMessage.hh>
#include "Udp2Tcp.hh"
#include "Input.hh"
using namespace std;

// Constructor

Udp2Tcp::Udp2Tcp(int argc, char **argv)

{

  isOK = true;

  // set programe name

  _progName = "Udp2Tcp";
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

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  return;

}

// destructor

Udp2Tcp::~Udp2Tcp()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int Udp2Tcp::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  // open socket as server
  
  ServerSocket serv;
  if (serv.openServer(_params.output_tcp_port)) {
    cerr << "Error - opening TCP socket for listening, port: "
	 << _params.output_tcp_port << endl;
    return (-1);
  }
  
  while (true) {
    
    PMU_auto_register("Run: waiting to get client");
    if (_params.debug) {
      cerr << "  ... waiting to get client, port "
	   << _params.output_tcp_port << " ... " << endl;
    }
    
    Socket *outSock = serv.getClient(1000);
    if (outSock != NULL) {
      if (_params.debug) {
	cerr << "  ... got client ... " << endl;
      }
      _serviceClient(outSock);
      delete outSock;
    }

  }
  
  serv.close();
  return (0);

}

int Udp2Tcp::_serviceClient(Socket *out_sock)

{

  // create input object

  Input input(_progName, _params);
  if (!input.isOK()) {
    cerr << "ERROR - " << _progName << endl;
    cerr << "  Cannot create input UDP object." << endl;
    return (-1);
  }
  
  int nMsgs = 0;
  int nBytes = 0;
  time_t prevTime = time(NULL);

  DsMessage msg;
    
  while (true) {

    // clear out message

    msg.clearAll();
    
    // read input, assemble into single message

    for (int i = 0; i < _params.npackets_per_message; i++) {
      while (input.readMsg()) {
	cerr << "ERROR - Udp2Tcp::_serviceClient." << endl;
	cerr << "  Error reading input packet." << endl;
	sleep (1);
      }
      msg.addPart(1,  input.msgLen(), input.msgPtr());
    }
    msg.assemble();

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Message is ready for xmit, size: "
	   << msg.lengthAssembled() << endl;
    }
      
    void *compressed_buffer;
    unsigned int nbytes_compressed;
    
    switch(_params.compression) {
      
    case Params::RLE_COMPRESSION:
      compressed_buffer =
	rle_compress(msg.assembledMsg(),
		     msg.lengthAssembled(), &nbytes_compressed);
      break;
      
    case Params::LZO_COMPRESSION:
      compressed_buffer =
	lzo_compress(msg.assembledMsg(),
		     msg.lengthAssembled(), &nbytes_compressed);
      break;
      
    case Params::ZLIB_COMPRESSION:
      compressed_buffer =
	zlib_compress(msg.assembledMsg(),
		      msg.lengthAssembled(), &nbytes_compressed);
      break;
      
    case Params::BZIP_COMPRESSION:
      compressed_buffer =
	bzip_compress(msg.assembledMsg(),
		      msg.lengthAssembled(), &nbytes_compressed);
      break;
      
    case Params::NO_COMPRESSION:
      compressed_buffer = msg.assembledMsg();
      nbytes_compressed = msg.lengthAssembled();
      break;
      
    }
      
    if (out_sock->writeMessage(0, compressed_buffer,
			       nbytes_compressed, -1)) {
      cerr << "Error writing to output socket." << endl;
      return (-1);
    } else {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << "  Wrote " << nbytes_compressed << " nbytes." << endl;
      }
    }
    
    if (compressed_buffer != msg.assembledMsg()) {
      ta_compress_free(compressed_buffer);
    }
    
    if (_params.debug) {
      nMsgs++;
      nBytes += nbytes_compressed;
      if (nMsgs == _params.nmessages_for_data_rate) {
	time_t now = time(NULL);
	double dtime = now - prevTime;
	double rate = (nBytes / dtime);
	cerr << utimstr(now) << ": data rate: " << rate
	     << " bytes/s" << endl;
	nMsgs = 0;
	nBytes = 0;
	prevTime = now;
      }
    }
    
  } // while (true)
  
  return (0);
  
}
