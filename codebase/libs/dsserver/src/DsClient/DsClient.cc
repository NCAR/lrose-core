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
// DsClient.cc
//
// Object for clients to use to contact servers
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2002
//
///////////////////////////////////////////////////////////////

#include <cstdlib>
#include <dsserver/DsClient.hh>
#include <dsserver/DsLocator.hh>
#include <dsserver/DsServerMsg.hh>
#include <toolsa/TaStr.hh>
using namespace std;

// constructor

DsClient::DsClient()

{
  _debug = false;
  _mergeDebugWithErrStr = false;
  _openTimeoutMsecs = -1;
}

// destructor

DsClient::~DsClient()

{

}

// free up data which the socket object manages

void DsClient::freeData()
{
  _sock.freeData();
}

////////////////////////////////////////////////////
// Communicate with server, automatically forwarding
// through proxy or tunnel as required
//
// This assumes a one-shot message sequence,
// i.e. a single request to the server followed
// by a single reply.
//
// Forwarding via a proxy and/or tunnel is handled.
//
// After success, retrieve the returned message
// using getReplyBuf() and getReplyLen().
//
// Returns 0 on success, -1 on failure.

int DsClient::communicateAutoFwd(const DsURL &url,
				 int msgType,
				 const void *msgBuf,
				 ssize_t msgLen)
  
{

  if (_debug) {
    _writeDebug("--> DsClient::communicateAutoFwd()");
    _writeDebug("----> Url: ", url.getURLStr());
    cerr << "------ URL DETAILS ------" << endl;
    url.print(cerr);
    cerr << "-------------------------" << endl;
  }

  // check for forwarding
  
  if (url.prepareForwarding("DsClient::communicateAutoFwd", msgLen)) {
    _errStr += "ERROR - DsClient::communicateAutoFwd\n";
    TaStr::AddStr(_errStr, "", url.getErrString());
    return -1;
  }
  
  // compute comm timeout
  
  ssize_t commTimeoutMsecs = DS_DEFAULT_COMM_TIMEOUT_MSECS;
  char *DS_COMM_TIMEOUT_MSECS = getenv("DS_COMM_TIMEOUT_MSECS");
  if (DS_COMM_TIMEOUT_MSECS != NULL) {
    ssize_t timeout;
    if (sscanf(DS_COMM_TIMEOUT_MSECS, "%ld", &timeout) == 1) {
      commTimeoutMsecs = timeout;
    }
  }
  // increase timeout by 3 secs if forwarding is in use
  // this helps to account for latency problems
  if (url.useForwarding()) {
    commTimeoutMsecs += 3000;
  }
  if (_debug) {
    _writeDebug("----> commTimeoutMsecs: ", commTimeoutMsecs);
  }

  if (url.useForwarding()) {
    
    if (_debug) {
      _writeDebug("----> using forwarding");
    }
    
    _errStr += "ERROR - DsClient::communicateAutoFwd, with fwd\n";
    TaStr::AddStr(_errStr, "  url: ", url.getURLStr());
    
    // forwarding is on - communicate through tunnel and/or proxy
    
    bool tunnelFailed = false;
    if (_communicateFwd(url, msgType, msgBuf, msgLen,
			commTimeoutMsecs, tunnelFailed)) {

      if (tunnelFailed) {
	if (_debug) {
	  _writeDebug("----------> tunnel failed");
	}
	if (url.useProxy()) {
	  _errStr +=
	    "  Cannot open http proxy. Is the http proxy running?\n";
	} else {
	  _errStr +=
	    "  Cannot open http tunnel. Is the web server running?.\n";
	}
	TaStr::AddStr(_errStr, "  host: ", url.getForwardingHost());
	TaStr::AddInt(_errStr, "  port: ", url.getForwardingPort());
	return -1;
      }
      
      if (_debug) {
	_writeDebug("--------> first comm failed");
	_writeDebug("--------> Requesting DsServerMgr to start it");
      }
      
      // tunnel/proxy OK, something else happened.
      // Request that Mgr starts server
      
      if (requestMgrStartServer(url)) {
	_errStr += "  Cannot access or start server\n";
	TaStr::AddStr(_errStr, "  url: ", url.getURLStr());
	return -1;
      }
      if (_debug) {
	_writeDebug("--------> Server started, retry");
      }

      if (_communicateFwd(url, msgType, msgBuf, msgLen,
			  commTimeoutMsecs, tunnelFailed)) {
	// 2nd server access failed
	_errStr += "  Cannot access server\n";
	return -1;
      } // if (communicateFwd(url, ...
      
    } // if (communicateFwd(url, ...

  } else {
    
    // no forwarding - communicate directly
    
    if (_debug) {
      _writeDebug("----> no forwarding");
    }

    if (_communicateNoFwd(url, msgType, msgBuf, msgLen, commTimeoutMsecs)) {
      
      if (_debug) {
	_writeDebug("--------> first comm failed");
	_writeDebug("--------> Requesting DsServerMgr to start it");
      }
      
      if (requestMgrStartServer(url)) {
	_errStr += "  Cannot access or start server\n";
	TaStr::AddStr(_errStr, "  url: ", url.getURLStr());
	return -1;
      }
      if (_debug) {
	_writeDebug("--------> Server started, retry");
      }
      
      if (_communicateNoFwd(url, msgType, msgBuf, msgLen, commTimeoutMsecs)) {
	
	// 2nd server access failed
	
	_errStr += "ERROR - DsClient::communicateAutoFwd, no fwd\n";
	_errStr += "  Cannot access server\n";
	TaStr::AddStr(_errStr, "  url: ", url.getURLStr());
	return -1;
	
      } // if (communicateNoFwd(url, ...
      
    } // if (communicateNoFwd(url, ...

  } // if (url.useForwarding())

  return 0;
      
}

// close the socket

void DsClient::_closeSocket()
{
  _sock.close();
}

////////////////////////////////////////////
// Communicate with server - no forwarding
//
// Open the server's socket, write the
// request message, receive and disassemble the reply.
//
// Returns 0 on success, -1 on error

int DsClient::_communicateNoFwd(const DsURL &url,
				int msgType,
				const void *msgBuf,
				ssize_t msgLen,
				int commTimeoutMsecs)
  
{
  
  if (_debug) {
    _writeDebug("------> _communicateNoFwd() opening socket");
  }
  
  // open socket
  if (_sock.open(url.getHost().c_str(),
                 url.getPort(),
                 _openTimeoutMsecs)) {
    _errStr += "ERROR - COMM - DsClient::_communicateNoFwd _sock.open\n";
    _errStr += "  Cannot connect to server\n";
    TaStr::AddStr(_errStr, "  host: ", url.getHost());
    TaStr::AddInt(_errStr, "  port: ", url.getPort());
    TaStr::AddStr(_errStr, "  url: ", url.getURLStr());
    _errStr += _sock.getErrStr();
    return -1;
  }

  // write the message
  
  if (_debug) {
    _writeDebug("------> _communicateNoFwd() writing message");
  }
  
  if (_sock.writeMessage(msgType,
			 msgBuf, msgLen, commTimeoutMsecs)) {
    _errStr +=
      "ERROR - COMM - DsClient::_communicateNoFwd _sock.writeMessage\n";
    _errStr += "  Errors writing message to server.\n";
    TaStr::AddStr(_errStr, "  host: ", url.getHost());
    TaStr::AddInt(_errStr, "  port: ", url.getPort());
    TaStr::AddStr(_errStr, "  url: ", url.getURLStr());
    _errStr += _sock.getErrStr();
    _closeSocket();
    freeData();
    return -1;
  }
  
  // read the reply
  
  if (_debug) {
    _writeDebug("------> _communicateNoFwd() reading reply");
  }

  if (_sock.readMessage(commTimeoutMsecs)) {
    _errStr +=
      "ERROR - COMM - DsClient::_communicateNoFwd _sock.readMessage\n";
    _errStr += "  Cannot read reply from server.\n";
    TaStr::AddStr(_errStr, "  host: ", url.getHost());
    TaStr::AddInt(_errStr, "  port: ", url.getPort());
    TaStr::AddStr(_errStr, "  url: ", url.getURLStr());
    _errStr += _sock.getErrStr();
    _closeSocket();
    freeData();
    return -1;
  }
  
  _closeSocket();
  return 0;

}

////////////////////////////////////////////
// Communicate with server - with forwarding
// Uses http tunnel and/or proxy.
//
// Open the server's  socket, write the
// request message, receive and disassemble the reply.
// Add and strip Http headers if required.
//
// Returns 0 on success, -1 on error

int DsClient::_communicateFwd(const DsURL &url,
			      int msgType,
			      const void *msgBuf,
			      ssize_t msgLen,
			      int commTimeoutMsecs,
			      bool &tunnelFailed)
  
{

  tunnelFailed = false;
  
  // open socket
  
  if (_debug) {
    _writeDebug("------> _communicateFwd() opening socket");
  }
  
  if (_sock.open(url.getForwardingHost().c_str(),
		 url.getForwardingPort(),
                 _openTimeoutMsecs)) {
    
    // Abort immediately as Http service is not available
    _errStr += "ERROR - COMM - DsClient::_communicateFwd _sock.open\n";
    _errStr += "  Error opening socket.\n";
    TaStr::AddStr(_errStr, "  url: ", url.getURLStr());
    _errStr += _sock.getErrStr();
    tunnelFailed = true;
    return -1;
    
  }
  
  // send the http header
  
  if (_debug) {
    _writeDebug("------> _communicateFwd() writing http header");
    _writeDebug("========= HTTP HEADER START ===============");
    _writeDebug(url.getHttpHeader());
    _writeDebug("========== HTTP HEADER END ================");
  }
  
  if (_sock.writeBuffer((void *) url.getHttpHeader().c_str(),
			url.getHttpHeader().size(),
			commTimeoutMsecs)) {
    
    // Give up - no way to contact the servers
    _errStr += "ERROR - COMM - DsClient::_communicateFwd _sock.writeBuffer\n";
    _errStr += "  Error writing http header.\n";
    TaStr::AddStr(_errStr, "  url: ", url.getURLStr());
    _errStr += _sock.getErrStr();
    tunnelFailed = true;
    _closeSocket();
    freeData();
    return -1;

  }
  
  // write the message
  
  if (_debug) {
    _writeDebug("------> _communicateFwd() writing main message");
  }

  if (_sock.writeMessage(msgType,
			 msgBuf, msgLen, commTimeoutMsecs)) {

    // Give up - no way to contact the servers
    _errStr += "ERROR - COMM - DsClient::_communicateFwd _sock.writeMessage\n";
    _errStr += "  Errors writing message to server.\n";
    TaStr::AddStr(_errStr, "  url: ", url.getURLStr());
    _errStr += _sock.getErrStr();
    tunnelFailed = true;
    _closeSocket();
    freeData();
    return -1;

  }
  
  // strip the http header from reply
  
  if (_debug) {
    _writeDebug("------> _communicateFwd() reading http header");
  }

  string httpHeader;
  if (_sock.stripHttpHeader(httpHeader, commTimeoutMsecs)) {
    _errStr += "ERROR - COMM - DsClient::_communicateFwd - stripHttpHeader\n";
    TaStr::AddStr(_errStr, "Stripping http header from return message");
    TaStr::AddStr(_errStr, "======= http Header on reply ==========");
    TaStr::AddStr(_errStr, httpHeader);
    TaStr::AddStr(_errStr, "=======================================");
    TaStr::AddStr(_errStr, _sock.getErrStr());
    _closeSocket();
    freeData();
    return -1;
  }

  if (_debug) {
    _writeDebug("======= start http header on reply ==========");
    _writeDebug("------> http header: ", httpHeader);
    _writeDebug("======== end http header on reply ===========");
  }

  // if not found (error 404) return error here

  if (httpHeader.find("404", 0) != string::npos) {
    if (_debug) {
      _writeDebug("--------> Server not found, will try ping");
    }
    _closeSocket();
    freeData();
    return -1;
  }

  // read the main body of the reply
  
  if (_debug) {
    _writeDebug("------> _communicateFwd() reading main body");
  }

  if (_sock.readMessage(commTimeoutMsecs)) {
    _errStr +=
      "ERROR - COMM - DsClient::_communicateFwd - _sock.readMessage\n";
    TaStr::AddStr(_errStr, "  Reading main reply message");
    TaStr::AddStr(_errStr, "======= http Header on reply ==========");
    TaStr::AddStr(_errStr, httpHeader);
    TaStr::AddStr(_errStr, "=======================================");
    TaStr::AddStr(_errStr, _sock.getErrStr());
    _closeSocket();
    freeData();
    return -1;
  }

  _closeSocket();
  return 0;

}

///////////////////////////////////////////////////////////
// Request the DsServerMgr to start the server for this URL
//
// Returns 0 on success, -1 on failure.

int DsClient::requestMgrStartServer(const DsURL &url)
  
{

  if (_debug) {
    _writeDebug("====>> _requestMgrStartServer()");
  }
  
  _errStr += "\n";
  _errStr += "DsClient::_requestMgrStartServer\n";
  _errStr += "  Requesting DsServerMgr to start server if needed\n";

  // copy the URL for contacting the Mgr
  
  DsURL mgrUrl = url;
  
  // compose message
  
  DsServerMsg msg;
  msg.setCategory(DsServerMsg::Generic);
  if (msg.addURLNoFwd(mgrUrl)) {
    TaStr::AddStr(_errStr, "  msg.addURLNoFwd: ",
		  "Could not add url to DsServerMsg.");
    return -1;
  }
  void *msgBuf = msg.assemble();
  ssize_t msgLen = msg.lengthAssembled();
  
  // check for forwarding
  
  int mgrPort = DsLocator.getDefaultPort("DsServerMgr");
  if (mgrUrl.prepareForwarding("DsClient::_requestMgrStartServer",
			       msgLen, mgrPort)) {
    TaStr::AddStr(_errStr, "  mgrUrl.prepareForwarding: ",
		  mgrUrl.getErrString());
    return -1;
  }
  
  // compute comm timeout
  
  int pingTimeoutMsecs = DS_DEFAULT_PING_TIMEOUT_MSECS;
  char *DS_PING_TIMEOUT_MSECS = getenv("DS_PING_TIMEOUT_MSECS");
  if (DS_PING_TIMEOUT_MSECS != NULL) {
    int timeout;
    if (sscanf(DS_PING_TIMEOUT_MSECS, "%d", &timeout) == 1) {
      pingTimeoutMsecs = timeout;
    }
  }
  // increase timeout by 3 secs if forwarding is in use
  // this helps to account for latency problems
  if (url.useForwarding()) {
    pingTimeoutMsecs += 3000;
  }
  if (_debug) {
    _writeDebug("======>> pingTimeoutMsecs: ", pingTimeoutMsecs);
  }

  // open socket
  
  if (mgrUrl.useForwarding()) {
    if (_debug) {
      _writeDebug("======>> opening socket, forwarding on");
    }
    if (_sock.open(mgrUrl.getForwardingHost().c_str(),
		   mgrUrl.getForwardingPort(),
                   _openTimeoutMsecs)) {
      TaStr::AddStr(_errStr, " _sock.open: ", _sock.getErrStr());
      return -1;
    }
  } else {
    if (_debug) {
      _writeDebug("======>> opening socket, forwarding off");
    }
    if (_sock.open(url.getHost().c_str(),
                   mgrPort,
                   _openTimeoutMsecs)) {
      TaStr::AddStr(_errStr, " _sock.open: ", _sock.getErrStr());
      TaStr::AddStr(_errStr, _sock.getErrStr());
      return -1;
    }
  }
  
  // if forwarding is active, send the http header
  
  if (mgrUrl.useForwarding()) {
    if (_debug) {
      _writeDebug("======>> writing header buffer for forwarding");
    }
    if (_sock.writeBuffer((void *) mgrUrl.getHttpHeader().c_str(),
			  mgrUrl.getHttpHeader().size(),
			  pingTimeoutMsecs)) {
      TaStr::AddStr(_errStr, " _sock.writeBuffer: ", _sock.getErrStr());
      _closeSocket();
      freeData();
      return -1;
    }
  }
  
  // send main message
  
  if (_debug) {
    _writeDebug("======>> writing main message");
  }
  if (_sock.writeMessage(0, msgBuf, msgLen, pingTimeoutMsecs)) {
    TaStr::AddStr(_errStr, " _sock.writeMessage: ", _sock.getErrStr());
    _closeSocket();
    freeData();
    return -1;
  }
  
  // if forwarding is active, strip the http header from reply
  
  string httpHeader;
  if (mgrUrl.useForwarding()) {
    if (_debug) {
      _writeDebug("======>> reading http header reply");
    }
    if (_sock.stripHttpHeader(httpHeader, pingTimeoutMsecs)) {
      TaStr::AddStr(_errStr, "Stripping http header from return message");
      TaStr::AddStr(_errStr, "======= http Header on reply ==========");
      TaStr::AddStr(_errStr, httpHeader);
      TaStr::AddStr(_errStr, "=======================================");
      _closeSocket();
      freeData();
      return -1;
    }
  }
  
  // read main reply, close socket
  
  if (_debug) {
    _writeDebug("======>> reading main reply");
  }
  if (_sock.readMessage(pingTimeoutMsecs)) {
    TaStr::AddStr(_errStr, " _sock.readMessage: ", _sock.getErrStr());
    _closeSocket();
    freeData();
    return -1;
  }
  _closeSocket();
  
  // disassemble reply
  
  DsServerMsg reply;
  if (reply.disassemble(_sock.getData())) {
    TaStr::AddStr(_errStr, "  reply.disassemble: ",
		  "Could not disassemble server reply.");
    freeData();
    return -1;
  }
  
  // Check for error return from the server.
  if (reply.getMessageErr() > 0) {
    // If it's not one of the two recoverable errors,
    // the returned url (if any) is not useable.
    if (reply.getMessageErr() != DsServerMsg::BAD_HOST_PROVIDED &&
	reply.getMessageErr() != DsServerMsg::BAD_PORT_PROVIDED) {
      TaStr::AddStr(_errStr, "  Server returned error: ",
		    reply.getFirstErrString());
      freeData();
      return -1;
    }
  }
  
  if (_debug) {
    _writeDebug("======>> Server running");
  }
  _errStr += "  Server running\n";

  freeData();
  return 0;

}

///////////////////////////////////////////////////////////
// write a debug message

void DsClient::_writeDebug(const string &label, const string &msg)
  
{

  if (_mergeDebugWithErrStr) {
    TaStr::AddStr(_errStr, label, msg);
  } else {
    cerr << label << msg << endl;
  }

}

void DsClient::_writeDebug(const string &label, int ival)
  
{

  if (_mergeDebugWithErrStr) {
    TaStr::AddInt(_errStr, label, ival);
  } else {
    cerr << label << ival << endl;
  }

}

void DsClient::_writeDebug(const string &label, long lval)
  
{

  if (_mergeDebugWithErrStr) {
    TaStr::AddLong(_errStr, label, lval);
  } else {
    cerr << label << lval << endl;
  }

}

