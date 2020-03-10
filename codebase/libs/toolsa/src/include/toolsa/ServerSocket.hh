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
// ServerSocket.hh
//
// ServerSocket class. This is a wrapper for the server-type sockutil calls.
//
// If the default constructor is used, openServer() must be called to
// open the socket. getClient() is then
// called to wait for a connecting client Socket.
//
// Test with $CVSROOT/libs/toolsa/test/SocketTest
//
// Paddy McCarthy, RAP, NCAR, POBox 3000, Boulder, CO, 80307, USA
//
// Dec 1998
//
////////////////////////////////////////////////////////////////////


// Todo:
//       o Convert returns to use enum within class scope.
//       o Add state info to class, so knows whether open, client/server, etc.
//       o   Other possible state info: Last operation. (Bad idea)
//       o Cache the hostname (client only), port, wait_msecs
//       o Error returns: Have all funcs return text that can be used in msg.
//       o Use string class for messages.
//       o Check for const correctness.

#ifndef SERVERSOCKET_HH
#define SERVERSOCKET_HH


#include <toolsa/sockutil.h>
#include <toolsa/SockUtil.hh>
#include <toolsa/Socket.hh>
#include <toolsa/ThreadSocket.hh>
#include <toolsa/HttpSocket.hh>
using namespace std;

class ServerSocket : public SockUtil {

public:

  //////////////
  // constructor  

  ServerSocket();

  //////////////
  // destructor
  // Closes socket if open

  virtual ~ServerSocket();

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
  
  int openServer(const int port);

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
  
  Socket * getClient(const ssize_t wait_msecs = -1);
    
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
  
  ThreadSocket * getThreadClient(const ssize_t wait_msecs = -1);

  ///////////////////////////////////////////////////////////////
  // getHttpClient()
  //
  // Try to get a client that strips off http header info.
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
  
  HttpSocket * getHttpClient(const ssize_t wait_msecs = -1);
    
  ///////////////
  // close()
  //
  // Close socket
  //
  
  virtual void close();

protected:

  int _protoSd;         // prototype socket descriptor

private:

    ///////////////////////////////////////
    // Copy constructor private with no body provided.
    //   Do not use.                                  

    ServerSocket(const ServerSocket & orig);

};


#endif
