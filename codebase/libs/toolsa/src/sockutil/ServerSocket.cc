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
////////////////////////////////////////////////////////////////////
// ServerSocket.cc
//
// ServerSocket class. This is a wrapper for the sockutil server-type calls.
//
// Paddy McCarthy, RAP, NCAR, POBox 3000, Boulder, CO, 80307, USA
//
// Dec 1998
//
////////////////////////////////////////////////////////////////////


#include <toolsa/Socket.hh>
#include <toolsa/ThreadSocket.hh>
#include <toolsa/HttpSocket.hh>
#include <toolsa/ServerSocket.hh>
#include <toolsa/mem.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <iostream>
#include <unistd.h>
#include <cerrno>
using namespace std;

void _close_server_socket(int sd);

///////////////
// constructor
//

ServerSocket::ServerSocket() : SockUtil()
{
  _objectType |= SERVER_SOCKET;
  _protoSd = -1;
}

///////////////
// destructor
//
// Closes socket if open.

ServerSocket::~ServerSocket()
{
  ServerSocket::close();
}

//////////////////
// openServer()
//
// Open connection as server for listening.
//
// Returns 0 on success, -1 on  failure
//
// On failure, getErrNum() will return the following:
//   SERVER_OPEN_FAILED   = Could not open socket (max file descriptors ?)
//   SERVER_BIND_FAILED   = Could not bind to specified port
//   SERVER_LISTEN_FAILED = Could not listen
//   UNEXPECTED           = Unknown error returned
//

int ServerSocket::openServer(const int port)
{

  // get a file descriptor for the connection to the remote port
  
  if((_protoSd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    addState(STATE_ERROR);
    _errNum = SERVER_OPEN_FAILED;
    _errString = "ERROR - COMM - Could not open socket: ";
    char tmpStr[128];
    sprintf(tmpStr, "port %d", port);
    _errString += tmpStr;
    return (-1);
  }

  // cerr << "ServerSocket::openServer: opened: " << _protoSd << endl;
  
  //  set socket options

  _setSocketOptions(_protoSd);
  
  // local sock info

  struct sockaddr_in loc_soc;
  memset((void*) &loc_soc, 0, sizeof(loc_soc));

#ifdef AIX
  // AIX has different sockaddr_in structure
  // see /usr/include/netinet/in.h
  loc_soc.sin_len = sizeof(loc_soc);
#endif
  loc_soc.sin_port = htons(port);
  loc_soc.sin_family = AF_INET;
  loc_soc.sin_addr.s_addr = htonl(INADDR_ANY);
  
  // bind to a local port

  errno = 0;
  if(::bind(_protoSd,
	  (struct sockaddr *) &loc_soc,
	  sizeof(loc_soc)) < 0) {
    addState(STATE_ERROR);
    _errNum = SERVER_BIND_FAILED;
    _errString = "ERROR - COMM - Could not bind: ";
    char tmpStr[128];
    sprintf(tmpStr, "port %d", port);
    _errString += tmpStr;
    _close_server_socket(_protoSd);
    return (-1);
  }
  
  // Wait for remote connection request
  // 
  //  Note: Pass zero as second arg to listen() so that the kernel
  //          does NOT queue up requests from clients. If the kernel
  //          is allowed to do this, clients can connect without
  //          blocking, then begin writing to the buffer. They die
  //          when the buffer fills up.

  errno = 0;
  if (listen(_protoSd, 5) < 0) {
    addState(STATE_ERROR);
    _errNum = SERVER_LISTEN_FAILED;
    _errString = "ERROR - COMM - Could not listen: ";
    char tmpStr[128];
    sprintf(tmpStr, "port %d", port);
    _errString += tmpStr;
    _close_server_socket(_protoSd);
    return (-1);
  }

  addState(STATE_OPENED);
  return(0);

}

///////////////////////////////////////////////////////////////
// getClient()
//
// Try to get a client.
// If wait_msecs is set, times out after wait_msecs (millisecs).
// If wait_msecs is not set, blocks until a client connects.
//
// Returns valid open Socket pointer on success, NULL on failure.
//   Caller is responsible for deleting the returned socket.
//
// On failure, getErrNum() will return the following:
//   TIMED_OUT:     timeout        -- timed version only.
//   SELECT_FAILED: select failure -- timed version only.
//   ACCEPT_FAILED: accept failure -- both timed and untimed versions.
//   UNEXPECTED           = Unknown error returned
//

Socket * ServerSocket::getClient(const ssize_t wait_msecs /* = -1*/ )
{

  addState(STATE_WAITING_FOR_CLIENT);

  // if wait_msecs is set, check for client

  if (wait_msecs >= 0) {
    if (_readSelect(_protoSd, wait_msecs)) {
      addState(STATE_ERROR);
      _errNum = SELECT_FAILED;
      _errString = "ERROR - COMM - Select failed waiting for client.";
      return (NULL);
    }
    if (_timedOut) {
      addState(STATE_ERROR);
      _errNum = TIMED_OUT;
      _errString = "  Normal time-out waiting for client.";
      return (NULL);
    }
  }

  // something ready to accept

#if (defined(__linux)) || defined __APPLE__
  socklen_t name_len = sizeof(struct sockaddr_in);
#else
  int name_len = sizeof(struct sockaddr_in);
#endif
  errno = 0;
  union sunion {
    struct sockaddr_in sin;
    struct sockaddr_un sund;
  } sadd;

  int sd = -1;
  if((sd = accept(_protoSd,
		  (struct sockaddr *) &sadd,
		  &name_len)) < 0) {
    addState(STATE_ERROR);
    _errNum = ACCEPT_FAILED;
    _errString = "ERROR - COMM - Accept failure when waiting for client.";
    return (NULL);
  }

  // cerr << "ServerSocket::getClient: accept: " << sd << endl;

  // set the socket options

  _setSocketOptions(sd);

  // create a socket and return it

  Socket *client = new Socket(sd);
  return client;

}

///////////////////////////////////////////////////////////////
// getThreadClient()
//
// Try to get a client, using a ThreadSocket.
// If wait_msecs is set, times out after wait_msecs (millisecs).
// If wait_msecs is not set, blocks until a client connects.
//
// Returns valid open Socket pointer on success, NULL on failure.
//   Caller is responsible for deleting the returned socket.
//
// On failure, getErrNum() will return the following:
//   TIMED_OUT:     timeout        -- timed version only.
//   SELECT_FAILED: select failure -- timed version only.
//   ACCEPT_FAILED: accept failure -- both timed and untimed versions.
//   UNEXPECTED           = Unknown error returned
//

ThreadSocket * ServerSocket::getThreadClient(const ssize_t wait_msecs /* = -1*/ )
{

  addState(STATE_WAITING_FOR_CLIENT);

  // if wait_msecs is set, check for client

  if (wait_msecs >= 0) {
    if (_readSelect(_protoSd, wait_msecs)) {
      addState(STATE_ERROR);
      _errNum = SELECT_FAILED;
      _errString = "ERROR - COMM - Select failed waiting for client.";
      return (NULL);
    }
    if (_timedOut) {
      addState(STATE_ERROR);
      _errNum = TIMED_OUT;
      _errString = "  Normal time-out waiting for client.";
      return (NULL);
    }
  }

  // something ready to accept

#if (defined(__linux)) || defined __APPLE__
  socklen_t name_len = sizeof(struct sockaddr_in);
#else
  int name_len = sizeof(struct sockaddr_in);
#endif
  errno = 0;
  union sunion {
    struct sockaddr_in sin;
    struct sockaddr_un sund;
  } sadd;

  int sd = -1;
  if((sd = accept(_protoSd,
		  (struct sockaddr *) &sadd,
		  &name_len)) < 0) {
    addState(STATE_ERROR);
    _errNum = ACCEPT_FAILED;
    _errString = "ERROR - COMM - Accept failure when waiting for client.";
    return (NULL);
  }

  // set the socket options

  _setSocketOptions(sd);

  // create a socket and return it

  ThreadSocket *client = new ThreadSocket(sd);
  return client;

}

///////////////////////////////////////////////////////////////
// getHttpClient()
//
// Try to get a client that will strip off http headers.
// If wait_msecs is set, times out after wait_msecs (millisecs).
// If wait_msecs is not set, blocks until a client connects.
//
// Returns valid open HttpSocket pointer on success, NULL on failure.
//   Caller is responsible for deleting the returned socket.
//
// On failure, getErrNum() will return the following:
//   TIMED_OUT:     timeout        -- timed version only.
//   SELECT_FAILED: select failure -- timed version only.
//   ACCEPT_FAILED: accept failure -- both timed and untimed versions.
//   UNEXPECTED           = Unknown error returned
//

HttpSocket * ServerSocket::getHttpClient(const ssize_t wait_msecs /* = -1*/ )
{

  addState(STATE_WAITING_FOR_CLIENT);

  // if wait_msecs is set, check for client

  if (wait_msecs >= 0) {
    if (_readSelect(_protoSd, wait_msecs)) {
      addState(STATE_ERROR);
      _errNum = SELECT_FAILED;
      _errString = "ERROR - COMM - Select failed waiting for client.";
      return (NULL);
    }
    if (_timedOut) {
      addState(STATE_ERROR);
      _errNum = TIMED_OUT;
      _errString = "  Normal time-out waiting for client.";
      return (NULL);
    }
  }

  // something ready to accept

#if (defined(__linux)) || defined (__APPLE__)
  socklen_t name_len = sizeof(struct sockaddr_in);
#else
  int name_len = sizeof(struct sockaddr_in);
#endif
  errno = 0;
  union sunion {
    struct sockaddr_in sin;
    struct sockaddr_un sund;
  } sadd;

  int sd = -1;
  if((sd = accept(_protoSd,
		  (struct sockaddr *) &sadd,
		  &name_len)) < 0) {
    addState(STATE_ERROR);
    _errNum = ACCEPT_FAILED;
    _errString = "ERROR - COMM - Accept failure when waiting for client.";
    return (NULL);
  }

  // cerr << "ServerSocket::getHttpClient: accept: " << sd << endl;

  // set the socket options

  _setSocketOptions(sd);

  // create a socket and return it

  HttpSocket *client = new HttpSocket(sd);
  return client;

}

////////////////////////
// close()
//
// Close socket
//

void ServerSocket::close()
{
  removeState(STATE_OPENED);
  addState(STATE_CLOSED);
  if (_protoSd >= 0) {
    // cerr << "ServerSocket::close(): " << _protoSd << endl;
    _close_server_socket(_protoSd);
    _protoSd = 0;
  }
}

///////////////////////////////////////////////////////////////////
// function to wrap close so that the compiler is not confused with
// the Socket::close()

void _close_server_socket(int sd)

{
  close(sd);
}
