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
// DsProxyServer.cc
//
// Proxy Server Object
//
// Paddy McCarthy, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 1999
//
///////////////////////////////////////////////////////////////

#include "Params.hh"
#include "DsProxyServer.hh"
#include <toolsa/ThreadSocket.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/TaStr.hh>
#include <dsserver/DsLocator.hh>
#include <dsserver/DsLdataServerMsg.hh>
#include <dsserver/DsFileCopyMsg.hh>
#include <didss/DsMsgPart.hh>
#include <Spdb/DsSpdbMsg.hh>
using namespace std;

DsProxyServer::DsProxyServer(string executableName,
                             const Params &params) :
        DsProcessServer(executableName,
                        params.instance,
                        params.port,
                        params.qmax,
                        params.max_clients,
                        params.debug >= Params::DEBUG_NORM,
                        params.debug >= Params::DEBUG_VERBOSE,
                        params.run_secure,
                        params.run_read_only,
                        params.allow_http),
        _params(params)

{
  setNoThreadDebug(_params.no_threads);
}


DsProxyServer::~DsProxyServer()
{
}

// Handle data commands from the client.
//   Returning failure (-1) from this method makes the server die.
//   So don't ever do that.
// 
// virtual
int DsProxyServer::handleDataCommand(Socket *clientSock,
                                     const void *data, 
                                     ssize_t dataSize)
{
  if (_isVerbose) {
    cerr << "Handling data command in DsProxyServer." << endl;
  }
  string errMsg;
  TaStr::AddStr(errMsg, "ERROR - DsProxyServer::handleDataCommand()");

  // disassemble incoming message

  DsServerMsg msg;
  int status = msg.disassemble(data, dataSize);
  if (status < 0) {
    TaStr::AddStr(errMsg, "Could not disassemble DsMessage.");
    _sendErrorReply(clientSock, errMsg, DsServerMsg::BAD_MESSAGE);
    return 0;
  }
  
  // Verify we have parts in the message.
  
  int numParts = msg.getNParts();
  if (_isVerbose) {
    cerr << "Data command has " << numParts << " parts." << endl;
    msg.print(cerr, "  ");
  }
  if (numParts <= 0) {
    TaStr::AddStr(errMsg, "Got data message with no parts.");
    _sendErrorReply(clientSock, errMsg, DsServerMsg::BAD_MESSAGE);
    return 0;
  }

  // Get the URL out of the message.
  // first check the standard
  
  string urlStr = _getUrlStr(msg);
  if (urlStr.size() == 0) {
    TaStr::AddStr(errMsg, "Data command has no URL.");
    _sendErrorReply(clientSock, errMsg, DsServerMsg::BAD_MESSAGE);
    return 0;                                            
  }
  if (_isVerbose) {
    cerr << "URL String: " << urlStr << endl;
  }
  const DsURL url(urlStr);
  
  // Check that this is a request/response protocol
  // fmq connections are not supported

  if (url.getProtocol().find("fmqp") != string::npos) {
    TaStr::AddStr(errMsg, "Cannot handle FMQ protocol.");
    TaStr::AddStr(errMsg, "  Must be a single connect/reply protocol.");
    _sendErrorReply(clientSock, errMsg, DsServerMsg::SERVICE_DENIED);
    return 0;                                            
  }

  // get port from URL
  
  int port = DsLocator.getDefaultPort(url);
  if (_isDebug) {
    cerr << "Forwarding on to host, port: "
         << _params.target_host << ", " << port << endl;
  }

  // ensure the server is running by contacting the DsServerMgr

  // Open a socket to the target server.
  ThreadSocket serverSock;
  if (serverSock.open(_params.target_host, port, -1) != 0) {

    // server not running
    // ping manager to start it up
    

    DsURL mgrUrl;
    mgrUrl.setHost(_params.target_host);
    mgrUrl.setProtocol(url.getProtocol());
    mgrUrl.setTranslator(url.getTranslator());
    mgrUrl.setParamFile(url.getParamFile());
    mgrUrl.setFile(url.getFile());
    string mgrUrlStr = mgrUrl.getURLStr();
    if (_isDebug) {
      cerr << "Contacting mgr, URL: " << mgrUrlStr << endl;
      mgrUrl.print(cerr);
    }
    string mgrErrStr;
    if (DsLocator.resolve(mgrUrl, NULL, true, &mgrErrStr)) {
      TaStr::AddStr(errMsg, "Cannot contact server manager to start server");
      TaStr::AddStr(errMsg, "  host: ", _params.target_host);
      TaStr::AddInt(errMsg, "  port: ", port);
      TaStr::AddStr(errMsg, mgrErrStr);
      _sendErrorReply(clientSock, errMsg, DsServerMsg::BAD_MESSAGE);
      return 0;
    }
    if (_isDebug) {
      cerr << "Mgr started server, URL: " << endl;
      mgrUrl.print(cerr);
    }
    
    // try opening again

    if (serverSock.open(_params.target_host, port, -1) != 0) {
      TaStr::AddStr(errMsg, "Trying to connect to true server, host: ",
                    _params.target_host);
      TaStr::AddInt(errMsg, "    port: ", port);
      _sendErrorReply(clientSock, errMsg, DsServerMsg::BAD_MESSAGE);
      return 0;
    }

  }

  // Send the message.
  if (serverSock.writeMessage(0, data, dataSize, -1) != 0) {
    TaStr::AddStr(errMsg, "Could not forward message to server on target host: ",
                  _params.target_host);
    TaStr::AddStr(errMsg, serverSock.getErrString());
    _sendErrorReply(clientSock, errMsg, DsServerMsg::BAD_MESSAGE);
    return 0;
  }
    
  // Read the reply.
  if (serverSock.readMessage() != 0) {
    TaStr::AddStr(errMsg,
                  "Could not read reply message from target server on host: ",
                  _params.target_host);
    TaStr::AddStr(errMsg, serverSock.getErrString());
    _sendErrorReply(clientSock, errMsg, DsServerMsg::SERVER_ERROR);
    return 0;
  }

  int replyLen     = serverSock.getNumBytes();
  const void * replyData = serverSock.getData();
    
  // Send the reply back to the client.
  if (clientSock->writeMessage(0, replyData, replyLen, -1) != 0) {
    TaStr::AddStr(errMsg, "Could not send reply message back to client");
    TaStr::AddStr(errMsg, serverSock.getErrString());
    _sendErrorReply(clientSock, errMsg, DsServerMsg::SERVER_ERROR);
    return 0;
  }

  return 0;

}

////////////////////////////////////////
// Send error reply to client

void DsProxyServer::_sendErrorReply(Socket *clientSock,
                                    const string &errMsg,
                                    DsServerMsg::msgErr errType)
  
{
  if (_isDebug) {
    cerr << errMsg << endl;
  }
  // Send error reply to client.
  string statusString;
  DsProcessServer::sendReply(clientSock, errType, errMsg,
                             statusString, 10000);
}

////////////////////////////////////////
// Get the URL out of the message.

string DsProxyServer::_getUrlStr(const DsServerMsg &msg)

{

  // first check the standard
  
  string urlStr = msg.getFirstURLStr();
  if (urlStr.size() > 0) {
    return urlStr;
  }

  // then check SPDB URL
  if (msg.partExists(DsSpdbMsg::DS_SPDB_URL_PART)) {
    DsMsgPart *part =
      msg.getPartByType(DsSpdbMsg::DS_SPDB_URL_PART);
    urlStr = (char *) part->getBuf();
  }
  if (urlStr.size() > 0) {
    return urlStr;
  }

  // then check LDATA_SERVER URL
  if (msg.partExists(DsLdataServerMsg::DS_LDATA_SERVER_URL_PART)) {
    DsMsgPart *part =
      msg.getPartByType(DsLdataServerMsg::DS_LDATA_SERVER_URL_PART);
    urlStr = (char *) part->getBuf();
  }
  if (urlStr.size() > 0) {
    return urlStr;
  }

  // then check FILECOPY URL
  if (msg.partExists(DsFileCopyMsg::DS_FILECOPY_URL_PART)) {
    DsMsgPart *part =
      msg.getPartByType(DsFileCopyMsg::DS_FILECOPY_URL_PART);
    urlStr = (char *) part->getBuf();
  }
  if (urlStr.size() > 0) {
    return urlStr;
  }

  return urlStr; // error - will be empty

}
  

