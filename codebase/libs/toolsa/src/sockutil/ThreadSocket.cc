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
// ThreadSocket.cc
//
// ThreadSocket class.
// This is a thread-safe version of the Socket class.
//
// Mike Dixon, RAP, NCAR, POBox 3000, Boulder, CO, 80307, USA
//
// Feb 1999
//
////////////////////////////////////////////////////////////////////


#include <cstdio>
#include <cerrno>
#include <netinet/in.h>
#include <pthread.h>
#include <dataport/bigend.h>
#include <toolsa/IpCache.hh>
#include <toolsa/ThreadSocket.hh>
#include <toolsa/GetHost.hh>
#include <toolsa/umisc.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/str.h>
using namespace std;

////////////////////////////////////////////////////
// default constructor
//
ThreadSocket::ThreadSocket() :
        Socket()
{
}

//////////////////////////////////////////////////////////////
// constructor
//   Takes a socket descriptor.
//   Used by the ServerSocket class when a client is obtained.
// 
ThreadSocket::ThreadSocket(int sd) :
        Socket(sd)
{

}

//////////////////////////////////////////////////////////////
// open()
//
// This open is thread-safe. It uses the GetHost class to wrap the
// gethostbyname() call.
//
// Open connection from client to server. If wait_msecs is specified,
// the attempt will time out after wait_msecs (milli-secs).
//
// Returns 0 on success, -1 on  failure
//
// On failure, getErrNum() will return the following:
//   UNKNOWN_HOST   = Could not find host name 
//   SOCKET_FAILED  = Could not setup socket (max file descriptors?)
//   CONNECT_FAILED = Could not connect to specified host and port 
//   SELECT_FAILED  = Could not do select on socket - non-blocking only
//   TIMED_OUT      = Timed out trying to connect
//                    (only applicable if wait_msecs is set)

int ThreadSocket::open(const char *hostname,
		       const int port,
		       const ssize_t wait_msecs /* = -1*/ )
{

  if (hasState(STATE_SERVER_OPENED)) {
    _errString = "Error: Trying to open a socket that was created ";
    _errString += "by a call to ServerSocket::getClient().";
    _errNum = UNEXPECTED;
    return -1;
  }

  if (hasState(STATE_OPENED)) {
    _errString = "Error: Trying to open a socket that is already opened.";
    _errNum = UNEXPECTED;
    return -1;
  }

  _errNum = _openClient(hostname, port, wait_msecs);
  
  if (_errNum) {

    addState(STATE_ERROR);

    switch (_errNum) {

    case UNKNOWN_HOST:
      _errString = "Could not find host \"";
      _errString += hostname;
      _errString += "\"";
      break;
      
    case SOCKET_FAILED:
      _errString = "Could not setup socket.";
      break;

    case CONNECT_FAILED: {
      _errString = "Could not connect to specified host: ";
      _errString += hostname;
      _errString += " port: ";
      char buf[10];
      sprintf(buf, "%d", port);
      _errString += buf;
      _errString += ".";
      break;
    }
    
    case SELECT_FAILED:
      _errString = "Could not do select on socket.";
      break;
      
    case TIMED_OUT:
      _errString = "Timed out trying to connect.";
      break;

    default:
      _errNum = UNEXPECTED;
      _errString = "Unexpected Error.";
      break;
    }

    return (-1);

  } else {

    removeState(STATE_ERROR); // Previous attempts to open may have failed.
    addState(STATE_OPENED);
    return (0);

  }

}

////////////////////////////////////////////////////////////////////
// _openClient()
//
// Open an Internet socket stream to server on hostname at given port.
//
// Blocks if wait_msecs is -1.
//
// If wait_msecs >= 0, times out if no client available after wait_msecs.
//
// Returns 0 on success,
//   else UNKNOWN_HOST, SOCKET_FAILED, CONNECT_FAILED,
//        SELECT_FAILED or TIMED_OUT
//

int ThreadSocket::_openClient(const char *hostname,
			      const int port,
			      const ssize_t wait_msecs /* = -1*/ )

{ 
  
  // use cache?

  bool useCache = false;
#ifdef __linux
  char *cache_str = getenv("USE_IP_CACHE");
  if (cache_str && STRequal(cache_str, "true")) {
    useCache = true;
  }
#endif
  
  // resolve host

  struct sockaddr_in rem_soc;
  memset((void*)&rem_soc, 0, sizeof(rem_soc));

  if (useCache) {
    
    in_addr addr;
    string ipStr;
    if (IpCache::getAddrByName(hostname, ipStr, &addr)) {
      return(UNKNOWN_HOST);
    }

    rem_soc.sin_family = AF_INET;

	// This variant removed due to failing on Darwin
	//   memcpy(&rem_soc.sin_addr, &addr.s_addr,
	//   MIN((int) sizeof(rem_soc.sin_addr), sizeof(addr.s_addr)));

	// Like Socket.cc
	memcpy(&rem_soc.sin_addr, &addr.s_addr,  sizeof(rem_soc.sin_addr));
    
  } else {

    GetHost getHost;
    const struct hostent *hh = getHost.getByName(hostname);
    if (hh == NULL) {
      return(UNKNOWN_HOST);
    }

    // copy the remote internet address to local struct
    
#ifdef AIX
    // AIX has different sockaddr_in structure ! see /usr/include/netinet/in.h */
    rem_soc.sin_len = sizeof(rem_soc);
#endif
    
    rem_soc.sin_family = AF_INET;

	// This variant removed due to failing on Darwin
    //memcpy(&rem_soc.sin_addr, hh->h_addr_list[0],
	//   MIN((int) sizeof(rem_soc.sin_addr), getHost.getAddrLen()));
	
	// Like Socket.cc
    memcpy(&rem_soc.sin_addr, hh->h_addr_list[0],sizeof(rem_soc.sin_addr));
    
  }

  unsigned short uport = port; // fill in port number */
  rem_soc.sin_port = htons(uport); 
  
  // get a file descriptor for the connection to the remote port
  
  if((_sd = socket(AF_INET,SOCK_STREAM,0)) == -1) {
    return(SOCKET_FAILED);
  }

  // cerr << "ThreadSocket::_openClient(), id: " << _sd << endl;

  // If required, check that the socket is ready.

  if (wait_msecs >= 0) {
    if (_writeSelect(_sd, wait_msecs)) {
      close();
      return (SELECT_FAILED);
    }
    if (_timedOut) {
      return (TIMED_OUT);
    }
  }

  // Connect

  errno = 0;
  if(connect(_sd, (struct sockaddr *) &rem_soc, sizeof(rem_soc)) < 0) {
    close();
    return(CONNECT_FAILED);
  }
  
  _setSocketOptions(_sd);
  
  return(0);
  
}

