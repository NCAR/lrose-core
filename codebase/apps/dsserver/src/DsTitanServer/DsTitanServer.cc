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
// DsTitanServer.cc
//
// Titan Server Object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2001
//
///////////////////////////////////////////////////////////////

#include "Params.hh"
#include "DsTitanServer.hh"

#include <dsserver/DsLocator.hh>
#include <titan/DsTitanMsg.hh>
#include <titan/DsTitan.hh>
#include <didss/RapDataDir.hh>
#include <toolsa/TaStr.hh>
#include <toolsa/Socket.hh>
#include <toolsa/compress.h>
using namespace std;

DsTitanServer::DsTitanServer(string executableName,
			     Params *params,
			     bool allowParamsOverride)
  : DsProcessServer(executableName,
		    params->instance,
		    params->port,
		    params->qmax,
		    params->max_clients,
		    params->debug >= Params::DEBUG_NORM,
		    params->debug >= Params::DEBUG_VERBOSE,
                    params->run_secure, false, true),
    _params(params),
    _allowParamsOverride(allowParamsOverride)

{

  setNoThreadDebug(_params->no_threads);

}

DsTitanServer::~DsTitanServer()
{

}

// Handle data commands from the client.
// Should always return 0, otherwise the server will die
// virtual

int DsTitanServer::handleDataCommand(Socket * socket,
				     const void * data,
				     ssize_t dataSize)

{

  if (_isVerbose) {
    cerr << "Handling data command in DsTitanServer." << endl;
  }

  // peek at the message
  
  DsTitanMsg msg;
  if (msg.decodeHeader(data, dataSize)) {
    string errMsg  = "Error in DsTitanServer::handleDataCommand(): ";
    errMsg += "Could not decode header.";
    if (_isDebug) {
      cerr << errMsg << endl;
    }
    // Send error reply to client.
    string statusString;
    if (DsProcessServer::sendReply(socket, DsServerMsg::BAD_MESSAGE,
				   errMsg, statusString, 10000)) {
      cerr << "ERROR - COMM - DsTitanServer - " << statusString << endl;
    }
    return 0;
  }
  
  // check subtype for validity

  if (msg.getSubType() != DsTitanMsg::DS_TITAN_READ_REQUEST) {
    string errMsg  = "Error in DsTitanServer::handleDataCommand(): ";
    TaStr::AddInt(errMsg, "Incorrect subtype: ", msg.getSubType());
    if (_isDebug) {
      cerr << errMsg << endl;
    }
    // Send error reply to client.
    string statusString;
    if (DsProcessServer::sendReply(socket, DsServerMsg::BAD_MESSAGE,
				   errMsg, statusString, 10000)) {
      cerr << "ERROR - COMM - DsTitanServer - " << statusString << endl;
    }
    return 0;
  }

  // disassemble the message, load up DsTitan object

  DsTitan titan;
  if (msg.disassemble(data, dataSize, titan)) {
    string errMsg  = "Error in DsTitanServer::handleDataCommand(): ";
    errMsg += "Could not disassemble message.";
    errMsg += msg.getErrStr();
    if (_isDebug) {
      cerr << errMsg << endl;
    }
    // Send error reply to client.
    string statusString;
    if (DsProcessServer::sendReply(socket, DsServerMsg::BAD_MESSAGE,
				   errMsg, statusString, 10000)) {
      cerr << "ERROR - COMM - DsTitanServer - " << statusString << endl;
    }
    return 0;
  }
  if (_isDebug) {
    titan.printReadRequest(cerr);
  }
  bool compressReply = titan.getReadCompressed();
  
  // decode URL

  DsURL url(titan.getUrlInUse());

  // perform the read

  string dirPath;
  RapDataDir.fillPath(url.getFile(), dirPath);

  DsTitanMsg retMsg;
  if (titan.read(dirPath)) {
    retMsg.assembleReadReplyError(titan);
  } else {
    retMsg.assembleReadReplySuccess(titan);
  }

  // send reply
  
  void *msgToSend = retMsg.assembledMsg();
  int msgLen = retMsg.lengthAssembled();
  int iret = 0;
  if (compressReply) {
    unsigned int nbytesCompressed;
    void *compressedBuf = ta_compress(TA_COMPRESSION_GZIP,
				      msgToSend, msgLen, &nbytesCompressed);
    iret = socket->writeMessage(0, compressedBuf, nbytesCompressed);
    ta_compress_free(compressedBuf);
    if (_isDebug) {
      cerr << "Returned message" << endl;
      cerr << "  Uncompressed len: " << msgLen << endl;
      cerr << "  Compressed len: " << nbytesCompressed << endl;
    }
  } else {
    iret = socket->writeMessage(0, msgToSend, msgLen);
    if (_isDebug) {
      cerr << "Returned message len: " << msgLen << endl;
    }
  }
  if (iret) {
    cerr << "ERROR - COMM - DsTitanServer::handleDataCommand." << endl;
    cerr << "  Sending message to server." << endl;
    cerr << socket->getErrStr() << endl;
  }

  return 0;

}

int
DsTitanServer::_checkForLocalParams( DsServerMsg &msg,
				     Params **paramsInUse,
				     bool &paramsAreLocal,
				     string errStr )
  
{
  
  //
  // NOTE: the DsLocator will resolve the parameter file name 
  //       and modify the url
  //
  
  bool paramsExist;
  string urlStr = msg.getFirstURLStr();
  if (urlStr == "") {
    errStr += "No URL in message.\n";
    return -1;
  }
  DsURL url(urlStr);
  if ( DsLocator.resolveParam( url, _executableName,
			       &paramsExist ) != 0 ) {
    errStr += "Cannot resolve parameter specification in url:\n";
    errStr += url.getURLStr();
    errStr += "\n";
    return -1;
  }
  
  //
  // The application-specfic code must load the override parameters
  //
  if ( paramsExist ) {
    if ( _loadLocalParams( url.getParamFile(), paramsInUse ) != 0 ) {
      errStr += "Cannot load parameter file:\n";
      errStr += url.getParamFile();
      errStr += "\n";
      return( -1 );
    }
    paramsAreLocal = true;
  } // if ( paramsExist )

  return 0;

}
  
int
DsTitanServer::_loadLocalParams( const string &paramFile, Params **params_p)
{

  char   **tdrpOverrideList = NULL;
  bool     expandEnvVars = true;
  
  if (_isDebug) {
    cerr << "Loading new params from file: " << paramFile << endl;
  }
  
  Params *localParams = new Params(*_params);
  if ( localParams->load( (char*)paramFile.c_str(),
			  tdrpOverrideList,
			  expandEnvVars,
			  false ) != 0 ) {
    delete localParams;
    return -1;
  }

  if (_isVerbose) {
    localParams->print(stderr, PRINT_SHORT);
  }
  
  *params_p = localParams;
  return 0;
}
