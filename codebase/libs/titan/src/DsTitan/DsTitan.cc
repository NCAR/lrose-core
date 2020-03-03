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
// DsTitan.cc
//
// DsTitan object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2001
//
///////////////////////////////////////////////////////////////
//
// The DsTitan object reads Titan data from the disk or 
// remotely from the DsTitanServer.
//
///////////////////////////////////////////////////////////////


#include <titan/DsTitan.hh>
#include <titan/DsTitanMsg.hh>
#include <dsserver/DsLocator.hh>
#include <dsserver/DsClient.hh>
#include <didss/RapDataDir.hh>
#include <toolsa/TaStr.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/Socket.hh>
#include <toolsa/compress.h>
#include <cstdlib>
#include <iostream>
using namespace std;

////////////////////////////////////////////////////////////
// Constructor

DsTitan::DsTitan() : TitanServer()
  
{
  _debug = false;
  _readCompressed = false;
}

////////////////////////////////////////////////////////////
// destructor

DsTitan::~DsTitan()

{

}

////////////////////////////////////////////////////////////
// Override print read request

void DsTitan::printReadRequest(ostream &out)

{

  TitanServer::printReadRequest(out);

  if (_readCompressed) {
    out << "  Compress data returned from read: true" << endl;
  } else {
    out << "  Compress data returned from read: false" << endl;
  }
  

}

/////////////////////////////////////////////////////
// Override read
//
// Returns 0 on success, -1 on failure.
//
// On failure, use getErrStr() to get error string.

int DsTitan::read(const string &urlStr)

{

  _clearErrStr();
  clearArrays();
  _errStr = "ERROR - DsTitan::read\n";
  TaStr::AddStr(_errStr, "Url: ", urlStr);
  
  // resolve server details
  
  DsURL url(urlStr);
  bool contactServer;
  if (DsLocator.resolve(url, &contactServer, false)) {
    _errStr += "  Cannot resolve URL: ";
    return -1;
  }
  
  // is it a local access? If so, use base class

  if (!contactServer) {
    string dirPath;
    RapDataDir.fillPath(url.getFile(), dirPath);
    return (TitanServer::read(dirPath));
  }

  // create message

  DsTitanMsg msg;
  void *buf = msg.assembleReadRequest(urlStr, *this);
  if (buf == NULL) {
    _errStr += "  Cannot assemble outgoing message\n";
    return -1;
  }
  
  // communicate with server
  
  if (_communicate(url, msg, buf, msg.lengthAssembled())) {
    _errStr += "  Communicating with server\n";
    return -1;
  }
  
  if (msg.getError()) {
    return -1;
  }
  
  if (msg.getSubType() != DsTitanMsg::DS_TITAN_READ_REPLY) {
    TaStr::AddInt(_errStr, "  Incorrect return subType: ", msg.getSubType());
    return -1;
  }
  
  return 0;

}

////////////////////////////////////////////
// communicate with server
//
// This involves opening the socket, writing the
// request message, receiving the reply and 
// disassembling the reply.
//
// Returns 0 on success, -1 on error

int DsTitan::_communicate(DsURL &url,
			  DsTitanMsg &msg,
			  const void *msgBuf,
			  int msgLen)
  
{
  
  DsClient client;
  client.setDebug(_debug);
  client.setErrStr("ERROR - DsTitan::_communicate\n");
  
  if (client.communicateAutoFwd(url, DsTitanMsg::DS_MESSAGE_TYPE_TITAN,
				msgBuf, msgLen)) {
    _errStr += client.getErrStr();
    return -1;
  }
  
  // disassemble the reply
  
  if (_debug) {
    cerr << "----> DsTitan::_communicate() dissasembling reply" << endl;
  }

  // disassemble the reply, checking for compression
  
  int iret = 0;
  if (ta_is_compressed(client.getReplyBuf(), client.getReplyLen())) {
    unsigned int bufLen;
    void *bufPtr = ta_decompress(client.getReplyBuf(), &bufLen);
    iret = msg.disassemble(bufPtr, bufLen, *this);
    ta_compress_free(bufPtr);
  } else {
    iret = msg.disassemble(client.getReplyBuf(), client.getReplyLen(), *this);
  }
  
  if (iret) {
    _errStr += "ERROR - DsTitan::_communicate msg.disassemble\n";
    _errStr += "Invalid reply - cannot disassemble\n";
    _errStr += msg.getErrStr();
    _errStr += "\n";
    return -1;
  }

  return 0;

}

