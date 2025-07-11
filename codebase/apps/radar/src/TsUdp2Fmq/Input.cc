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
// Input.cc
//
// UDP input
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// August 1999
//
//////////////////////////////////////////////////////////

#include "Input.hh"

#include <rapformats/swap.h>
#include <toolsa/sockutil.h>
#include <toolsa/pmu.h>
#include <dataport/bigend.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cerrno>
#include <cstdio>
#include <unistd.h>
#include <cstdlib>

#if defined(__linux)
#include <asm/ioctls.h>
#include <sys/ioctl.h>
#elif defined (__APPLE__)
#include <sys/ioctl.h>
#else
#include <sys/filio.h>
using namespace std;
#endif

/////////////
//constructor

Input::Input(string &prog_name, Params  &params) :
    _progName(prog_name), _params(params)
{

  _isOK = true;
  _udpFd = -1;

  // open a socket

  if  ((_udpFd = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
    fprintf(stderr, "Could not open UDP socket, port %d\n",
          _params.udp_port);
    perror ("socket error:");
    _isOK = false;
    return;
  }

  // make the socket non-blocking

  int blocking_flag = 1;          // argument for ioctl call
  if (ioctl(_udpFd, FIONBIO, &blocking_flag) != 0) {
    fprintf(stderr, "Could not make socket non-blocking, port %d\n",
          _params.udp_port);
    perror ("ioctl error:");
    close (_udpFd);
    _udpFd = -1;
    _isOK = false;
    return;
  }

  // set the socket for reuse

  int val = 1;
  int valen = sizeof(val);
  setsockopt(_udpFd, SOL_SOCKET, SO_REUSEADDR, (char *) &val, valen);

  // bind local address to the socket

  struct sockaddr_in addr;        // address structure for socket
  memset((void *) &addr, 0, sizeof (addr));
  addr.sin_port = htons (_params.udp_port);
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl (INADDR_ANY);

  if (bind (_udpFd, (struct sockaddr *) &addr, sizeof (addr)) < 0) {
    fprintf(stderr, "Could not bind UDP socket, port %d\n",
          _params.udp_port);
    perror ("bind error:");
    close (_udpFd);
    _udpFd = -1;
    _isOK = false;
    return;
  }

  if (_params.debug) {
    fprintf(stderr, "Opened UDP socket, port %d\n", _params.udp_port);
  }

  _done = false;
  std::thread r( [&]{
    while(!_done) {
      receiveMsg();
    }
    Input::Message nullmsg;
    _pktQueue.push(nullmsg);
  } );
  _reader = std::move(r);

  return;
}

//////////////
// destructor

Input::~Input()
{
  halt();

  if (_udpFd >= 0) {
    close(_udpFd);
    _udpFd = -1;
  }

  if (_params.debug) {
    fprintf(stderr, "Reader done\n");
  }
}

///////////////////////////////////////////////////////////
// receiveMsg()
//
// Reads a udp packet
//
// returns 0 on success, -1 on failure.
//
int Input::receiveMsg()
{

  socklen_t addrlen = sizeof (struct sockaddr_in);

  int iret;
  struct sockaddr_in from;   // address from which packet came

  // wait on socket for up to 1 sec at a time

  while ((iret = SKU_read_select(_udpFd, 1000)) == -1) {
    if (_done) {
      return 0;
    }
    // timeout
    PMU_auto_register("Waiting for udp data");
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      fprintf(stderr, "Waiting for udp data\n");
    }
  }

  if (iret == 1) {

    // data is available for reading

    PMU_auto_register("Reading udp data");

    errno = EINTR;
    ssize_t pktLen = 0;
    Message msg(16384);
    while (errno == EINTR || errno == EWOULDBLOCK) {
      errno = 0;
      pktLen = recvfrom (_udpFd, msg.data(), msg.size(), 0,
                         (struct sockaddr *)&from, &addrlen);
    }

    if (pktLen > 0) {
      if (_params.debug >= Params::DEBUG_EXTRA) {
        fprintf(stderr, "Read packet, %d bytes\n", pktLen);
      }
      msg.resize(pktLen);
      _pktQueue.push(msg);
      return (0);
    }
    else {
        fprintf(stderr, "ERROR - %s:_getUdpPkt\n", _progName.c_str());
        fprintf(stderr, "Reading ethernet - _pktLen = %d\n", pktLen);
        perror("");
        _done = true;
        return (-1);
    }

  } // if (iret == 1)

  return (-1);

}

//////////////
// halt

void Input::halt()
{
  if (_params.debug) {
    fprintf(stderr, "Halting reader\n");
  }
  _done = true;
  if (_reader.joinable()) {
    _reader.join();
  }
}
