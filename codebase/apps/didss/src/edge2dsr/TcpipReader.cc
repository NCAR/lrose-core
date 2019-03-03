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
/*********************************************************************
 * TcpipReader : Class for retrieving realtime data from the EDGE system
 *             via TCP/IP sockets.
 *
 * RAP, NCAR, Boulder CO
 *
 * November 2001
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <math.h>
#include <cerrno>
#include <fcntl.h>
#include <cstdio>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <toolsa/pmu.h>
#include <toolsa/mem.h>
#include <toolsa/sockutil.h>
#include <toolsa/str.h>

#include <toolsa/MsgLog.hh>

#if defined(__linux)
#include <asm/ioctls.h>
#include <sys/ioctl.h>
#else
#include <sys/filio.h>
#endif

#include "TcpipReader.hh"
#include "Edge2Dsr.hh"
#include "EdgeMsg.hh"
using namespace std;


// Define constants

const int TcpipReader::HEADER_SIZE = 40;
const int TcpipReader::SOCKET_HUNG_SECS = 10;


/*********************************************************************
 * Constructor
 */

TcpipReader::TcpipReader(const string &socket_name,
			 const string &host_name,
			 const int port_num,
			 const bool check_missing_beams) :
  _socketName(socket_name),
  _hostName(host_name),
  _portNum(port_num),
  _socketFd(-1),
  _currMsgAlloc(0),
  _currMsgSize(0),
  _lastMsgTime(time(0)),
  _prevAzimuth(-1),
  _checkMissingBeams(check_missing_beams)
{
  
  // Allocate space for the current message being read.  We begin with
  // 40 bytes because that is the size of the header.

  _currMsg = (char *)umalloc(HEADER_SIZE);
  _currMsgAlloc = HEADER_SIZE;
}


/*********************************************************************
 * Destructor
 */

TcpipReader::~TcpipReader() 
{
  if (_socketFd >= 0)
    SKU_close(_socketFd);

  ufree(_currMsg);
}


/*********************************************************************
 * init() - Initialize the information in this object.
 *
 * Returns true if successful, false otherwise.
 */

bool TcpipReader::init() 
{
   // Try to open the socket

   POSTMSG(INFO, "Opening TCP/IP socket on port %d", _portNum);
   PMU_auto_register("Opening a TCP/IP socket");

   _socketFd = _openSocket(_hostName, _portNum);

   return true;
}


/*********************************************************************
 * messageReady() - Check to see if there is a message ready on this
 *                  reader.
 *
 * Returns true if there is a message ready, false otherwise.
 */

bool TcpipReader::messageReady()
{
  // First see if this socket seems to be hung

  if (time(0) > _lastMsgTime + SOCKET_HUNG_SECS)
  {
    POSTMSG(ERROR,
	    "%s socket seems to be hung -- closing socket",
	    _socketName.c_str());

    closeSocket();
  }
    
  // Get the file descriptor for the reader.  If there is an error
  // on the socket, there won't be a descriptor available so it will
  // be negative.

  if (_socketFd < 0)
  {
    // First see if we can currently open the socket

    _socketFd = _openSocket(_hostName, _portNum);
    
    if (_socketFd < 0)
    {
      POSTMSG(WARNING,
	      "%s socket not open", _socketName.c_str());
    
      return false;
    }
  }
  
  // See if there is any data to read on the file descriptor

  if (SKU_read_select(_socketFd, 0) < 0)
  {
    POSTMSG(WARNING,
	    "%s socket has not data to read", _socketName.c_str());

    return false;
  }
  
  return true;
}


/*********************************************************************
 * readMsg() - Try to read a message from the socket.  Returns true if
 *             a full message has been read, false otherwise.
 */

bool TcpipReader::readMsg()
{
  // First read the header from the socket

  int sku_return;
  
  if ((sku_return = SKU_read(_socketFd, _currMsg, HEADER_SIZE, -1)) < 0)
  {
    POSTMSG(ERROR,
	    "Error reading %s message header from TCP/IP socket",
	    _socketName.c_str());
    closeSocket();
    
    return false;
  }
  else if (sku_return == 0)
  {
    POSTMSG(ERROR,
	    "0 bytes of %s message header read from TCP/IP socket",
	    _socketName.c_str());
    closeSocket();
    
    return false;
  }
  else if (sku_return != HEADER_SIZE)
  {
    POSTMSG(ERROR,
	    "Not enough bytes read for %s header\n"
	    "          Expected %d bytes, got %d bytes",
	    _socketName.c_str(), HEADER_SIZE, sku_return);
    closeSocket();
    
    return false;
  }
  
  // Then make sure we have a big enough buffer for the entire message

  int binaryAz, binaryEl, checkSum;
  int nParts;
  unsigned int rolling_count;
  int moment;
  int uncompressedLen;
  int compressedLen;
  int compression;
  
  if ((nParts = sscanf(_currMsg,
		       "%04x %04x %08x %4d %4d %04x %1d",
		       &binaryAz, &binaryEl, &checkSum, &uncompressedLen,
		       &compressedLen, &moment, &compression)) == 7)
  {
    // Do nothing -- continue on with processing old style header
  }
  else if ((nParts = sscanf(_currMsg,
			    "%04x %04x %08x %4d %4d %04x %d %02x",
			    &binaryAz, &binaryEl, &checkSum, &uncompressedLen,
			    &compressedLen, &moment, &compression,
			    &rolling_count)) == 8)
  {
    // Do nothing -- continue on with processing new header
  }
  else
  {
    POSTMSG( ERROR,
	     "Couldn't read the header from _readNextMessage: %s\n"
	     "Expected: binaryAz binaryEl checkSum uncompressedLen compressedLen moment compression   -- OR --\n"
	     "          binaryAz binaryEl checkSum uncompressedLen compressedLen moment compression rolling_count",
	     _currMsg);
//      for (int i = 0; i < 40; ++i)
//	fprintf(stderr, "    byte %d: %d\n", i, _currMsg[i]);
     
    return false;
  }
  
  double azimuth = binaryAz/65536.0 * 360.0;

  if (_prevAzimuth > 0 && _checkMissingBeams)
  {
    double az_diff = fabs(azimuth - _prevAzimuth);
    
    if (az_diff > 1.25 && az_diff < 358.0)
      POSTMSG(ERROR,
	      "*** Missing beam in %s data:\n"
	      "    prev azimuth: %f\n"
	      "    curr azimuth: %f",
	      _socketName.c_str(), _prevAzimuth, azimuth);
  }
  
  _prevAzimuth = azimuth;
  
  _currMsgSize = HEADER_SIZE + compressedLen;
  
  if (_currMsgAlloc < _currMsgSize)
  {
    _currMsgAlloc = _currMsgSize;
    _currMsg = (char *)urealloc((void *)_currMsg, _currMsgAlloc);
  }
  
  // Now read the data from the socket

  if ((sku_return = SKU_read(_socketFd, _currMsg + HEADER_SIZE,
			     compressedLen, -1)) < 0)
  {
    POSTMSG(ERROR,
	    "Error reading %s message data from TCP/IP socket",
	    _socketName.c_str());
    closeSocket();
    
    return false;
  }
  else if (sku_return == 0)
  {
    POSTMSG(ERROR,
	    "0 bytes of %s message data read from TCP/IP socket",
	    _socketName.c_str());
    closeSocket();
    
    return false;
  }
  else if (sku_return != compressedLen)
  {
    POSTMSG(ERROR,
	    "Not enough bytes read for %s data\n"
	    "          Expected %d bytes, got %d bytes",
	    _socketName.c_str(), compressedLen, sku_return);
    closeSocket();
    
    return false;
  }
  
  _lastMsgTime = time(0);

  return true;
  
}


/*********************************************************************
 * waitForMsg() - Block waiting for the next message from this socket.
 */

void TcpipReader::waitForMsg()
{
  // First see if this socket seems to be hung

  if (time(0) > _lastMsgTime + SOCKET_HUNG_SECS)
  {
    POSTMSG(ERROR,
	    "%s socket seems to be hung -- closing socket",
	    _socketName.c_str());

    closeSocket();
  }
    
  // Get the file descriptor for the reader.  If there is an error
  // on the socket, there won't be a descriptor available so it will
  // be negative.

  while (_socketFd < 0)
  {
    PMU_auto_register("Waiting for server socket to be ready");
    
    POSTMSG(WARNING,
	    "%s socket not open -- trying to open it", _socketName.c_str());

    _socketFd = _openSocket(_hostName, _portNum);
    
    if (_socketFd < 0)
      sleep(1);
  }
  
  // See if there is any data to read on the file descriptor

  while (SKU_read_select(_socketFd, 1000) < 0)
    PMU_auto_register("Waiting for message from server");
}


/*********************************************************************
 * closeSocket() - Close the socket associated with this reader.  This
 *                 is generally done if there was an error of some kind
 *                 detected on the socket.
 */

void TcpipReader::closeSocket()
{
  SKU_close(_socketFd);
  _socketFd = -1;
}
  
   
/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _openSocket() - Open a client socket on the EDGE host using the
 *                 specified port.
 *
 * On success, returns the file descriptor for the opened socket.
 * Otherwise, returns -1.
 */

int TcpipReader::_openSocket(const string &host_name, const int port_number) const
{
  POSTMSG(DEBUG,
	  "Reopening %s socket", _socketName.c_str());
  
  // Open the client socket

  int sock_fd = SKU_open_client_timed(host_name.c_str(), port_number, 5000);
  
  if (sock_fd < 0)
  {
    switch (sock_fd)
    {
    case -1 :
      POSTMSG(ERROR,
	      "Error getting the remote host information for EDGE host: %s",
	      host_name.c_str());
      break;
      
    case -2 :
      POSTMSG(ERROR,
	      "Error getting a file descriptor for the connection to the remote port: %s::%d\n"
	      "Could the maximum number of file descriptors have been exceeded??",
	      host_name.c_str(), port_number);
      break;
      
    case -3 :
      POSTMSG(ERROR,
	      "Error connecting local socket to remote port: %s::%d",
	      host_name.c_str(), port_number);
      break;
      
    case -4 :
      POSTMSG(ERROR,
	      "Timed out trying to connect to EDGE socket: %s::%d",
	      host_name.c_str(), port_number);
      break;
      
    default:
      POSTMSG(ERROR,
	      "Unknown error connecting socket: %s::%d",
	      host_name.c_str(), port_number);
      break;
    } /* endswitch - sock_fd */
    
    return -1;
  }
  
  // Increase the socket buffer size

  int bufsize = 81920;
  setsockopt(sock_fd, SOL_SOCKET, SO_RCVBUF, &bufsize, sizeof(bufsize));
  
  // Make the socket non-blocking since we are having a problem with
  // the socket blocking periodically for 1.5 minutes and we can't figure
  // out what the problem is.

  int val;
  
  if ((val = fcntl(sock_fd, F_GETFL, 0)) < 0)
    POSTMSG(ERROR, "Error getting current control flags for socket");
  
  val |= O_NONBLOCK;
  
  if (fcntl(sock_fd, F_SETFL, val) < 0)
    POSTMSG(ERROR, "Error setting non-blocking control flag for socket");
  
  _lastMsgTime = time(0);

  return sock_fd;
}
