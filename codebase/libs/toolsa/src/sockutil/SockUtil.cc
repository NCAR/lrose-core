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
// SockUtil.cc
//
// SockUtil class. This is an abstract base class for
//   wrappers of the sockutil calls.
//
// Paddy McCarthy, RAP, NCAR, POBox 3000, Boulder, CO, 80307, USA
//
// Dec 1998
//
////////////////////////////////////////////////////////////////////


#include <toolsa/mem.h>
#include <toolsa/pmu.h>
#include <toolsa/SockUtil.hh>
#include <sys/time.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <cerrno>
#include <iostream>
using namespace std;

///////////////
// constructor
//

SockUtil::SockUtil()

{
  _objectType = SOCK_UTIL;
  _state      = STATE_NONE;
  _errNum     = 0;
  _errString  = "";
}

///////////////
// destructor
//

SockUtil::~SockUtil()
{
}

//////////////////////
// _setSocketOptions()

void SockUtil::_setSocketOptions(const int sd)
{

  int val;
  int valen = sizeof(val);
  
  // reuse the socket

  val = 1;
  errno = 0;
  if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *) &val, valen)) {
    cerr << strerror(errno) << endl;
    cerr << "WARNING - setsockopt(SO_REUSEADDR) failed" << endl;
  }

  // Set up keep-alive so that if the server side reboots. we will
  // get an error once it is back up and running

  // number of retries for keepalive
  val = 1;
  if (setsockopt(sd, SOL_SOCKET, SO_KEEPALIVE, (char *) &val, valen)) {
    cerr << strerror(errno) << endl;
    cerr << "WARNING - setsockopt(SO_KEEPALIVE) failed" << endl;
  }
  
#ifdef __APPLE__

  // count for keepalive
  val = 3;
  if (setsockopt(sd, SOL_SOCKET, TCP_KEEPCNT, (char *) &val, valen)) {
    cerr << strerror(errno) << endl;
    cerr << "WARNING - setsockopt(TCP_KEEPCNT) failed" << endl;
  }

  // interval for keepalive
  val = 10;
  if (setsockopt(sd, SOL_SOCKET, TCP_KEEPINTVL, (char *) &val, valen)) {
    cerr << strerror(errno) << endl;
    cerr << "WARNING - setsockopt(SO_KEEPINTVL) failed" << endl;
  }

#else

  // count for keepalive
  val = 3;
  if (setsockopt(sd, SOL_TCP, TCP_KEEPCNT, (char *) &val, valen)) {
    cerr << strerror(errno) << endl;
    cerr << "WARNING - setsockopt(SO_KEEPCNT) failed" << endl;
  }

  // interval for keepalive
  val = 10;
  if (setsockopt(sd, SOL_TCP, TCP_KEEPIDLE, (char *) &val, valen)) {
    cerr << strerror(errno) << endl;
    cerr << "WARNING - setsockopt(SO_KEEPIDLE) failed" << endl;
  }
  if (setsockopt(sd, SOL_TCP, TCP_KEEPINTVL, (char *) &val, valen)) {
    cerr << strerror(errno) << endl;
    cerr << "WARNING - setsockopt(SO_KEEPINTVL) failed" << endl;
  }

#endif
  
  // make sockets disappear quickly on close

  // errno = 0;
  // memset(&sl, 0, sizeof(sl));
  // sl.l_onoff = 0;
  // setsockopt(sd, SOL_SOCKET, SO_LINGER, (char *) &sl, sizeof(sl));

}

/////////////////////////////////////////////////////////
// _readSelectPmu()
//
// Waits for read access for up to wait_msecs millisecs.
// Registers with procmap while waiting
//
// Blocks if wait_msecs is -1
// Sets _timedOut set true if it times out.
//
// Returns 0 on success, -1 on failure.
//

int SockUtil::_readSelectPmu(const int sd,
			     const ssize_t wait_msecs /* = -1*/ )

{

  if (!PMU_init_done()) {
    return (_readSelect(sd, wait_msecs));
  }

  int msecs_left;
  if (wait_msecs < 0) {
    msecs_left = 1000;
  } else {
    msecs_left = wait_msecs;
  }
  
  while (msecs_left >= 0) {

    int msecs_sleep;
    if (msecs_left < 1000) {
      msecs_sleep = msecs_left;
    } else {
      msecs_sleep = 1000;
    }

    if (_readSelect(sd, msecs_sleep)) {
      return -1;
    }
    
    if (!_timedOut) {
      return 0;
    }

    if (wait_msecs >= 0) {
      msecs_left -= msecs_sleep;
    }

#ifndef NO_PROCMAPPER
    PMU_auto_register("In readSelect() ..... ");
#endif

  } // while

  _timedOut = true;
  return 0;

}
/////////////////////////////////////////////////////////
// _readSelect()
//
// Waits for read access for up to wait_msecs millisecs.
//
// Blocks is wait_msecs is -1
// Sets _timedOut set true if it times out.
//
// Returns 0 on success, -1 on failure.
//

int SockUtil::_readSelect(const int sd,
			  const ssize_t wait_msecs /* = -1*/ )

{

  _timedOut = false;
  
  // check only sd socket descriptor

  fd_set read_fd;
  FD_ZERO(&read_fd);
  FD_SET(sd, &read_fd);
  int maxfdp1 = sd + 1;

  int iret;
  errno = EINTR;
  while (errno == EINTR) {
    struct timeval wait;
    struct timeval *waitp;
    // set timeval structure
    if (wait_msecs < 0) {
      waitp = NULL;
    } else {
      ssize_t msecs = wait_msecs;
      wait.tv_sec = msecs / 1000;
      msecs -= wait.tv_sec * 1000;
      wait.tv_usec = msecs * 1000;
      waitp = &wait;
    }
    errno = 0;
    if (0 > (iret = select(maxfdp1, &read_fd, NULL, NULL, waitp))) {
      if (errno == EINTR) { // system call was interrupted
	continue;
      }
      return (-1);
    }
  } // while
  
  if (iret == 0) {
    _timedOut = true;
  }
  
  return (0);

}

/////////////////////////////////////////////////////////
// _writeSelect()
//
// Waits for write access for up to wait_msecs millisecs.
//
// Blocks is wait_msecs is -1
// Sets _timedOut set true if it times out.
//
// Returns 0 on success, -1 on failure.
//

int SockUtil::_writeSelect(const int sd,
			   const ssize_t wait_msecs /* = -1*/ )

{

  _timedOut = false;
  
  // check only sd socket descriptor

  fd_set write_fd;
  FD_ZERO(&write_fd);
  FD_SET(sd, &write_fd);
  int maxfdp1 = sd + 1;

  int iret;

  errno = EINTR;
  while (errno == EINTR) {
    struct timeval wait;
    struct timeval *waitp;
    // set timeval structure
    if (wait_msecs < 0) {
      waitp = NULL;
    } else {
      ssize_t msecs = wait_msecs;
      wait.tv_sec = msecs / 1000;
      msecs -= wait.tv_sec * 1000;
      wait.tv_usec = msecs * 1000;
      waitp = &wait;
    }
    errno = 0;
    if (0 > (iret = select(maxfdp1, NULL, &write_fd, NULL, waitp))) {
      if (errno == EINTR) { // system call was interrupted
	continue;
      }
      return (-1);
    }
  } // while
  
  if (iret == 0) {
    _timedOut = true;
  }
  
  return (0);

}


