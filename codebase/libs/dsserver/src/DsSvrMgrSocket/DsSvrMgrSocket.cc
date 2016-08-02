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

//////////////////////////////////////////////////////////
// DsSvrMgrSocket.cc
//
// Paddy McCarthy, RAP, NCAR, POBox 3000, Boulder, CO, 80307-3000
//
// Jan 1999
//
////////////////////////////////////////////////////////////

#include <dsserver/DsSvrMgrSocket.hh>
#include <dsserver/DsServerMsg.hh>
#include <dsserver/DsLocator.hh>
const char* DsSvrMgrSocket::EXEC_NAME = "DsServerMgr";

#include <toolsa/TaStr.hh>
#include <toolsa/DateTime.hh>
using namespace std;

DsSvrMgrSocket::DsSvrMgrSocket() : ThreadSocket()
{
}

DsSvrMgrSocket::~DsSvrMgrSocket()
{
}

// This version of findPortForURL() opens the connection, makes
// the enquiry and closes the connection

int DsSvrMgrSocket::findPortForURL(const char *hostname,
				   DsURL & url, int wait_msecs,
                                   string & errString)
{

  errString  = "Error in DsSvrMgrSocket::findPortForURL(): ";
  TaStr::AddStr(errString, "  ", DateTime::str());

  // compose message

  DsServerMsg msg;
  msg.setCategory(DsServerMsg::Generic);
  if (msg.addURLNoFwd(url)) {
    errString += "Could not add url to DsServerMsg.";
    close();
    return -1;
  }
  void * msgToSend = msg.assemble();
  int msgLen = msg.lengthAssembled();
  
  // check for forwarding

  int mgrPort = DsLocator.getDefaultPort( EXEC_NAME );
  if (url.prepareForwarding("DsLOCATOR::pingServer", msgLen, mgrPort)) {
    TaStr::AddStr(errString, url.getErrString());
    return -1;
  }

  // if forwarding, bump up time_out

  if (url.useForwarding() && wait_msecs > 0) {
    wait_msecs *= 5;
  }
  
  // open socket
  
  if (url.useForwarding()) {
    if (open(url.getForwardingHost().c_str(),
	     url.getForwardingPort())) {
      TaStr::AddStr(errString, _errString);
      return (-1);
    }
  } else {
    if (open(hostname, mgrPort)) {
      TaStr::AddStr(errString, _errString);
      return (-1);
    }
  }
  
  if ( !hasState(STATE_OPENED) ) {
    errString += "Socket to the DsServerMgr is not open.";
    close();
    return -1;
  }

  if ( hasState(STATE_ERROR) ) {
    errString += "Socket to the DsServerMgr has an error: ";
    errString += _errString;
    close();
    return -1;
  }

  url.getURLStr();
  if ( !url.isValid() ) {
    errString += "Invalid url was provided.";
    close();
    return -1;
  }
  
  // if forwarding is active, send the http header

  if (url.useForwarding()) {

    if (writeBuffer((void *) url.getHttpHeader().c_str(),
		    url.getHttpHeader().size(),
		    wait_msecs)) {
      TaStr::AddStr(errString, _errString);
      close();
      return -1;
    }
  }

  // send main message

  if (writeMessage(0, msgToSend, msgLen, wait_msecs)) {
    TaStr::AddStr(errString, _errString);
    close();
    return -1;
  }
  
  // if forwarding is active, strip the http header from reply
  
  string httpHeader;
  if (url.useForwarding()) {

    if (stripHttpHeader(httpHeader, wait_msecs)) {
      close();
      TaStr::AddStr(errString, "Stripping http header from return message");
      TaStr::AddStr(errString, "======= http Header on reply ==========");
      TaStr::AddStr(errString, httpHeader);
      TaStr::AddStr(errString, "=======================================");
      return -1;
    }
  }
  
  // read reply, close socket

  if (readMessage(wait_msecs)) {
    if (url.useForwarding()) {
      //TaStr::AddStr(errString, "======= http Header on reply ==========");
      //TaStr::AddStr(errString, httpHeader);
      //TaStr::AddStr(errString, "=======================================");
      close();
      return -1;
    }
    TaStr::AddStr(errString, _errString);
    close();
    return -1;
  }
  close();
  
  // disassemble reply

  DsServerMsg reply;
  const void * data = getData();
  if (reply.disassemble(data)) {
    errString += "Could not disassemble server reply.\n";
    return -1;
  }
  
  // Check for error return from the server.
  if (reply.getMessageErr() > 0) {
    // If it's not one of the two recoverable errors,
    //   the returned url (if any) is not useable.
    // 
    if (reply.getMessageErr() != DsServerMsg::BAD_HOST_PROVIDED &&
	reply.getMessageErr() != DsServerMsg::BAD_PORT_PROVIDED) {
      errString += "Server returned error: ";
      errString += reply.getFirstErrString();
      return -1;
    }
  }
  
  // Get the URL.
  DsURL * replyURL = reply.getFirstURL();
  if (replyURL == NULL) {
    errString += "Reply from server contained no URL.";
    return -1;
  }
  
  // Validate the URL.
  if ( !replyURL->isValid() ) {
    errString += "Server returned an invalid URL.";
    delete replyURL;
    return -1;
  }
  
  // Set the port.
  url.setPort(replyURL->getPort());
  url.getURLStr();
  delete replyURL;
  
  return (0);

}

int DsSvrMgrSocket::_doFind(DsURL & url, int wait_msecs,
			    string & errString)

{
  
  if ( !hasState(STATE_OPENED) ) {
    errString  = "Error in DsSvrMgrSocket::findPortForURL(): ";
    errString += "Socket to the DsServerMgr is not open.";
    return -1;
  }
  if ( hasState(STATE_ERROR) ) {
    errString  = "Error in DsSvrMgrSocket::findPortForURL(): ";
    errString += "Socket to the DsServerMgr has an error: ";
    errString += _errString;
    TaStr::AddStr(errString, "  ", DateTime::str());
    return -1;
  }
  url.getURLStr();
  if ( !url.isValid() ) {
    errString  = "Error in DsSvrMgrSocket::findPortForURL(): ";
    errString += "Invalid url was provided.";
    return -1;
  }
  
  DsServerMsg msg;
  msg.setCategory(DsServerMsg::Generic);
  if (msg.addURL(url)) {
    errString  = "Error in DsSvrMgrSocket::findPortForURL(): ";
    errString += "Could not add url to DsServerMsg.";
    return -1;
  }
  void * msgToSend = msg.assemble();
  int msgLen = msg.lengthAssembled();
  
  if (writeMessage(0, msgToSend, msgLen, wait_msecs)) {
    errString  = "Error in DsSvrMgrSocket::findPortForURL(): ";
    errString += "Could not write message: ";
    errString += _errString;
    TaStr::AddStr(errString, "  ", DateTime::str());
    return -1;
  }
  
  if (readMessage(wait_msecs)) {
    errString  = "Error in DsSvrMgrSocket::findPortForURL(): ";
    errString += "Could not read reply from server: ";
    errString += _errString;
    TaStr::AddStr(errString, "  ", DateTime::str());
    return -1;
  }
  
  DsServerMsg reply;
  const void * data = getData();
  if (reply.disassemble(data)) {
    errString  = "Error in DsSvrMgrSocket::findPortForURL(): ";
    errString += "Could not disassemble server reply.";
    return -1;
  }
  
  // Check for error return from the server.
  if (reply.getMessageErr() > 0) {
    // If it's not one of the two recoverable errors,
    //   the returned url (if any) is not useable.
    // 
    if (reply.getMessageErr() != DsServerMsg::BAD_HOST_PROVIDED &&
	reply.getMessageErr() != DsServerMsg::BAD_PORT_PROVIDED) {
      
      errString  = "Error in DsSvrMgrSocket::findPortForURL(): ";
      errString += "Server returned error: ";
      errString += reply.getFirstErrString();
      return -1;
    }
  }
  
  // Get the URL.
  DsURL * replyURL = reply.getFirstURL();
  if (replyURL == NULL) {
    errString  = "Error in DsSvrMgrSocket::findPortForURL(): ";
    errString += "Reply from server contained no URL.";
    return -1;
  }
  
  // Validate the URL.
  if ( !replyURL->isValid() ) {
    errString  = "Error in DsSvrMgrSocket::findPortForURL(): ";
    errString += "Server returned an invalid URL.";
    delete replyURL;
    return -1;
  }
  
  // Return the URL.
  url = *replyURL;
  delete replyURL;
  
  return 0;

}

