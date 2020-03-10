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
// DsThreadedClient.cc
//
// Object for clients to use to contact servers
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2002
//
///////////////////////////////////////////////////////////////

#include <dsserver/DsThreadedClient.hh>
#include <dsserver/DsLocator.hh>
#include <dsserver/DsServerMsg.hh>
#include <toolsa/TaStr.hh>
#include <pthread.h>
#include <cstdlib>
using namespace std;

// constructor

DsThreadedClient::DsThreadedClient() :
  DsClient()

{

}

// destructor

DsThreadedClient::~DsThreadedClient()

{

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

int DsThreadedClient::communicateAutoFwd(DsURL &url,
					 int msgType,
					 const void *msgBuf,
					 ssize_t msgLen,
					 ssize_t &nbytesWriteExpected,
					 ssize_t &nbytesWriteDone,
					 ssize_t &nbytesReadExpected,
					 ssize_t &nbytesReadDone)
  
{

  if (_debug) {
    cerr << "--> DsThreadedClient::communicate()" << endl;
    cerr << "----> Url: " << url.getURLStr() << endl;
    url.print(cerr);
  }
  
  // check for forwarding
  
  if (url.prepareForwarding("DsThreadedClient::communicate", msgLen)) {
    _errStr += "ERROR - DsThreadedClient::communicate\n";
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
    cerr << "----> commTimeoutMsecs: " << commTimeoutMsecs << endl;
  }

  if (url.useForwarding()) {
    
    if (_debug) {
      cerr << "----> using forwarding" << endl;
    }
    
    _errStr += "ERROR - DsThreadedClient::communicate, with fwd\n";
    TaStr::AddStr(_errStr, "  url: ", url.getURLStr());
    
    // forwarding is on - communicate through tunnel and/or proxy
    
    bool tunnelFailed;
    if (_communicateFwd(url, msgType, msgBuf, msgLen, commTimeoutMsecs,
			nbytesWriteExpected, nbytesWriteDone,
			nbytesReadExpected, nbytesReadDone,
			tunnelFailed)) {
      
      if (tunnelFailed) {
	if (_debug) {
	  cerr << "----------> tunnel failed" << endl;
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
	cerr << "--------> first comm failed" << endl;
	cerr << "--------> Requesting DsServerMgr to start it" << endl;
      }
      
      // tunnel/proxy OK, something else happened. ping server.
      // The ping will use the Mgr to start the server if possible
      
      if (requestMgrStartServer(url)) {
	_errStr += "  Cannot access or start server\n";
	TaStr::AddStr(_errStr, "  url: ", url.getURLStr());
	return -1;
      }

      if (_debug) {
	cerr << "--------> Server started, retry" << endl;
      }
      
      if (_communicateFwd(url, msgType, msgBuf, msgLen, commTimeoutMsecs,
			  nbytesWriteExpected, nbytesWriteDone,
			  nbytesReadExpected, nbytesReadDone,
			  tunnelFailed)) {
	// 2nd server access failed
	_errStr += "  Cannot access server\n";
	return -1;
      } // if (communicateFwd(url, ...
      
    } // if (communicateFwd(url, ...

  } else {
    
    // no forwarding - communicate directly
    
    if (_debug) {
      cerr << "----> no forwarding" << endl;
    }

    if (_communicateNoFwd(url, msgType, msgBuf, msgLen,
			  nbytesWriteExpected, nbytesWriteDone,
			  nbytesReadExpected, nbytesReadDone)) {
      
      if (_debug) {
	cerr << "--------> first comm failed" << endl;
	cerr << "--------> Requesting DsServerMgr to start it" << endl;
      }
      
      if (requestMgrStartServer(url)) {
	_errStr += "  Cannot access or start server\n";
	TaStr::AddStr(_errStr, "  url: ", url.getURLStr());
	return -1;
      }
      if (_debug) {
	cerr << "--------> Server started, retry" << endl;
      }
      
      if (_communicateNoFwd(url, msgType, msgBuf, msgLen,
			    nbytesWriteExpected, nbytesWriteDone,
			    nbytesReadExpected, nbytesReadDone)) {
	
	// 2nd server access failed
	
	_errStr += "ERROR - DsThreadedClient::communicate, no fwd\n";
	_errStr += "  Cannot access server\n";
	TaStr::AddStr(_errStr, "  url: ", url.getURLStr());
	return -1;

      } // if (communicateNoFwd(url, ...
      
    } // if (communicateNoFwd(url, ...

  } // if (url.useForwarding())

  return 0;
      
}

////////////////////////////////////////////
// Communicate with server - no forwarding
//
// Open the server's socket, write the
// request message, receive and disassemble the reply.
//
// Returns 0 on success, -1 on error

int DsThreadedClient::_communicateNoFwd(DsURL &url,
					int msgType,
					const void *msgBuf,
					ssize_t msgLen,
					ssize_t &nbytesWriteExpected,
					ssize_t &nbytesWriteDone,
					ssize_t &nbytesReadExpected,
					ssize_t &nbytesReadDone)
  
{
  
  if (_debug) {
    cerr << "------> _communicateNoFwd() opening socket" << endl;
  }
  
  // open socket
  if (_sock.open(url.getHost().c_str(), url.getPort())) {
    _errStr +=
      "ERROR - COMM - DsThreadedClient::_communicateNoFwd _sock.open\n";
    _errStr += "  Cannot connect to server\n";
    TaStr::AddStr(_errStr, "  host: ", url.getHost());
    TaStr::AddInt(_errStr, "  port: ", url.getPort());
    _errStr += _sock.getErrStr();
    return -1;
  }

  // write the message
  
  if (_debug) {
    cerr << "------> _communicateNoFwd() writing message" << endl;
  }
  
  nbytesWriteExpected = msgLen;
  if (_sock.writeMessageIncr(msgType,
			     msgBuf, msgLen, nbytesWriteDone)) {
    _errStr +=
      "ERROR - COMM - _communicateNoFwd _sock.writeMessage\n";
    _errStr += "  Errors writing message to server.\n";
    TaStr::AddStr(_errStr, "  host: ", url.getHost());
    TaStr::AddInt(_errStr, "  port: ", url.getPort());
    _errStr += _sock.getErrStr();
    _errStr += "\n";
    _closeSocket();
    freeData();
    return -1;
  }
  
  // read the reply
  
  if (_debug) {
    cerr << "------> _communicateNoFwd() reading reply" << endl;
  }

  if (_sock.readMessageIncr(nbytesReadExpected, nbytesReadDone)) {
    _errStr +=
      "ERROR - COMM - _communicateNoFwd _sock.readMessage\n";
    _errStr += "  Cannot read reply from server.\n";
    TaStr::AddStr(_errStr, "  host: ", url.getHost());
    TaStr::AddInt(_errStr, "  port: ", url.getPort());
    _errStr += _sock.getErrStr();
    _closeSocket();
    freeData();
    return -1;
  }
  
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

int DsThreadedClient::_communicateFwd(DsURL &url,
				      int msgType,
				      const void *msgBuf,
				      ssize_t msgLen,
				      ssize_t commTimeoutMsecs,
				      ssize_t &nbytesWriteExpected,
				      ssize_t &nbytesWriteDone,
				      ssize_t &nbytesReadExpected,
				      ssize_t &nbytesReadDone,
				      bool &tunnelFailed)
  
{

  tunnelFailed = false;
  
  // open socket
  
  if (_debug) {
    cerr << "------> _communicateFwd() opening socket" << endl;
  }
  
  if (_sock.open(url.getForwardingHost().c_str(),
		 url.getForwardingPort())) {
    
    // Abort immediately as Http service is not available
    _errStr += "ERROR - COMM - _communicateFwd _sock.open\n";
    _errStr += "  Error opening socket.\n";
    _errStr += _sock.getErrStr();
    _errStr += "\n";
    tunnelFailed = true;
    return -1;
    
  }
  
  // send the http header
  
  if (_debug) {
    cerr << "------> _communicateFwd() writing http header" << endl;
    cerr << url.getHttpHeader() << endl;
  }
  
  if (_sock.writeBuffer((void *) url.getHttpHeader().c_str(),
			url.getHttpHeader().size(),
			commTimeoutMsecs)) {
    
    // Give up - no way to contact the servers
    _errStr += "ERROR - COMM - _communicateFwd _sock.writeBuffer\n";
    _errStr += "  Error writing http header.\n";
    _errStr += _sock.getErrStr();
    tunnelFailed = true;
    _closeSocket();
    freeData();
    return -1;

  }
  
  // write the message
  
  if (_debug) {
    cerr << "------> _communicateFwd() writing main message" << endl;
  }

  nbytesWriteExpected = msgLen;
  if (_sock.writeMessageIncr(msgType,
			     msgBuf, msgLen, nbytesWriteDone)) {

    // Give up - no way to contact the servers
    _errStr += "ERROR - COMM - _communicateFwd _sock.writeMessageIncr\n";
    _errStr += "  Errors writing message to server.\n";
    _errStr += _sock.getErrStr();
    tunnelFailed = true;
    _closeSocket();
    freeData();
    return -1;

  }
  
  // strip the http header from reply
  
  if (_debug) {
    cerr << "------> _communicateFwd() reading http header" << endl;
  }

  string httpHeader;
  if (_sock.stripHttpHeader(httpHeader, commTimeoutMsecs)) {
    _errStr +=
      "ERROR - COMM - _communicateFwd - stripHttpHeader\n";
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
    cerr << "------> http header: " << httpHeader << endl;
  }

  // if not found (error 404) return error here

  if (httpHeader.find("404", 0) != string::npos) {
    if (_debug) {
      cerr << "--------> Server not found, will try ping" << endl;
    }
    _closeSocket();
    freeData();
    return -1;
  }

  // read the main body of the reply
  
  if (_debug) {
    cerr << "------> _communicateFwd() reading main body" << endl;
    cerr << "nbytesReadExpected: " << nbytesReadExpected << endl;
  }

  
  if (_sock.readMessageIncr(nbytesReadExpected, nbytesReadDone)) {
    _errStr +=
      "ERROR - COMM - _communicateFwd - _sock.readMessageIncr\n";
    TaStr::AddStr(_errStr, "  Reading main reply message");
    TaStr::AddStr(_errStr, "======= http Header on reply ==========");
    TaStr::AddStr(_errStr, httpHeader);
    TaStr::AddStr(_errStr, "=======================================");
    TaStr::AddStr(_errStr, _sock.getErrStr());
    _closeSocket();
    freeData();
    return -1;
  }
  
  if (_debug) {
    cerr << "nbytesReadDone: " << nbytesReadDone << endl;
  }
  
  _closeSocket();
  return 0;

}

