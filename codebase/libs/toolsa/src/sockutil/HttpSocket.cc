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
// HttpSocket.cc
//
// HttpSocket class has ability to remove an http header
//
// Paddy McCarthy
// RAP, NCAR, POBox 3000, Boulder, CO, 80307, USA
//
// Nov 2009
//
////////////////////////////////////////////////////////////////////


#include <toolsa/umisc.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/HttpSocket.hh>
#include <toolsa/TaStr.hh>
#include <toolsa/mem.h>
#include <dataport/bigend.h>
#include <cstdio>
#include <unistd.h>
#include <cerrno>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
using namespace std;

#define SOCKET_MAGIC 0xf0f0f0f0


///////////////
// default constructor
//
HttpSocket::HttpSocket() : Socket()
{
  _objectType |= HTTP_SOCKET;
}

///////////////
// constructor
//   Takes a socket descriptor. Used by the ServerSocket class when a 
//     client is obtained.
// 
HttpSocket::HttpSocket(int sd) : Socket(sd)
{
  _objectType |= HTTP_SOCKET;
}

///////////////
// destructor
//
// Closes socket if open.
// Frees any data.
//
HttpSocket::~HttpSocket()
{
}

////////////////////////////////////////////////////////
// readHeader()
//
// Reads a msg_header.
//
// len() will return the length of the data buffer
// id() will return the id of the header
// seqNo() will return the sequence number of the header
//
// If wait_msecs is set, returns after wait_msecs (millisecs)
// even if read is not complete. Blocks if wait_msecs is not set.
//
// Returns 0 on success, -1 on failure.
// 
// On failure, getErrNum() will return:
//   TIMED_OUT
//   BAD_BYTE_COUNT
//   BAD_MAGIC_COOKIE
//   READ_HEADER_FAILED
//
int HttpSocket::readHeader(const ssize_t wait_msecs /* = -1*/ )
{

  removeState(STATE_WROTELAST);
  addState(STATE_READLAST);

  // read in the first 8 bytes - the magic cookie

  ui32 magic[2];

  if (_peek(magic, 8, wait_msecs) != 8) {
    addState(STATE_ERROR);
    if (_timedOut) {
      _errNum = TIMED_OUT;
      _errString = "ERROR - COMM - HttpSocket::readHeader(): Timed out.";
      TaStr::AddInt(_errString, "  wait_msecs: ", wait_msecs);
    } else {
      _errNum = BAD_BYTE_COUNT;
      _errString =
	"ERROR - COMM - HttpSocket::readHeader():\n"
	"  Cannot read magic cookie -- first 8 bytes of the reply message.\n"
	"  If app is server, client probably exited\n"
	"  If app is client, server child probably exited (maybe SEGV?)\n";
    }
    
    return (-1);
  }
  
  _requestHadHttpHeader = FALSE;

  // 
  // Check the first bytes to see if they match the magic cookie
  // 
  if (magic[0] == SOCKET_MAGIC && magic[1] == SOCKET_MAGIC) {
    // cerr << "HttpSocket::readHeader() found no HTTP Header. Forwarding to Socket::readHeader()" << endl;

    // Message starts with magic cookie -- no HTTP Header present.
    return Socket::readHeader(wait_msecs);
  }
  else {

    // No Magic Cookie -- check for HTTP header followed by a cookie.
    char c;
    char buf[4] = {'\0', '\0', '\0', '\0'};
    int checkLen = 2000; // The HTTP Header may be this long.
    for (int i = 0; i < checkLen; i++) {
      if (_read(&c, 1, wait_msecs) != 1) {
        addState(STATE_ERROR);
        if (_timedOut) {
          _errNum = TIMED_OUT;
          _errString = "ERROR - COMM - HttpSocket::readHeader(): Timed out while reading char of potential HTTP Header.";
          TaStr::AddInt(_errString, "  wait_msecs: ", wait_msecs);
        } else {
          _errNum = BAD_BYTE_COUNT;
          _errString =
                "ERROR - COMM - HttpSocket::readHeader():\n"
                "  Cannot read char to test for HTTP Header.\n"
                "  If app is server, client probably exited\n"
                "  If app is client, server child probably exited (maybe SEGV?)\n";
        }
    
        return (-1);
      }

      buf[0] = buf[1];
      buf[1] = buf[2];
      buf[2] = buf[3];
      buf[3] = c;

      if (buf[0] == '\r' && buf[1] == '\n' && buf[2] == '\r' && buf[3] == '\n'){

        // Found end of header. Peek to check for magic cookie.
        if (_peek(magic, 8, wait_msecs) != 8) {
          addState(STATE_ERROR);
          if (_timedOut) {
            _errNum = TIMED_OUT;
            _errString = "ERROR - COMM - HttpSocket::readHeader(): Timed out.";
            TaStr::AddInt(_errString, "  wait_msecs: ", wait_msecs);
          } else {
            _errNum = BAD_BYTE_COUNT;
            _errString =
                  "ERROR - COMM - HttpSocket::readHeader():\n"
                  "  Cannot peek magic cookie -- first 8 bytes of the reply message.\n"
                  "  If app is server, client probably exited\n"
                  "  If app is client, server child probably exited (maybe SEGV?)\n";
          }
      
          return (-1);
        }

        if (magic[0] == SOCKET_MAGIC && magic[1] == SOCKET_MAGIC) {
          // cerr << "HttpSocket::readHeader() found magic cookie after HTTP Header. Forwarding to Socket::readHeader()" << endl;
      
          _requestHadHttpHeader = TRUE;

          // The rest of the message starts with magic cookie...
          return Socket::readHeader(wait_msecs);
        }

        // 
        // No magic cookie after the http header.
        // 
        addState(STATE_ERROR);
        _errNum = BAD_MAGIC_COOKIE;
        _errString = "ERROR - COMM - HTTPSocket::readHeader(): Found end of HTTP Header, but it was not followed by magic cookie.";
       
        return (-1);
      }
    }

    // 
    // Found no end of HTTP Header -- bail.
    // 
    addState(STATE_ERROR);
    _errNum = BAD_MAGIC_COOKIE;
    _errString = "ERROR - COMM - HTTPSocket::readHeader(): no magic cookie, and couldn't find end of HTTP Header.";
    return (-1);
  }
}

////////////////////////////////////////////////////////
// writeHeader()
//
// Writes an Http Header, then calls superclass method.
//
// If wait_msecs is set, returns after wait_msecs (millisecs)
// even if write is not complete. Blocks if wait_msecs is not set.
//
// Returns 0 on success, -1 on failure.
//
// On failure, getErrNum() will return the following:
//  TIMED_OUT:           timed out
//  SELECT_FAILED:       select failed
//  WRITE_HEADER_FAILED: SKU_write_header failed for one of various reasons.
//  UNEXPECTED           Unknown error returned from the SKU_ functions.
//
int HttpSocket::writeHeader(const ssize_t len,
                            const ssize_t product_id,
                            const ssize_t seq_no,
                            const ssize_t wait_msecs /* = -1*/ )
{
  removeState(STATE_READLAST);
  addState(STATE_WROTELAST);
 
  // Check if the write is possible in the requested time.
  if (_writeSelect(_sd, wait_msecs)) {
    addState(STATE_ERROR);
    if (_timedOut) {
      _errNum = TIMED_OUT;
      _errString = "ERROR - COMM - Socket::writeHeader(): timed out.";
    } else {
      _errNum = SELECT_FAILED;
      _errString = "ERROR - COMM - Socket::writeHeader(): write select failed.";
    }
    return (-1);
  }
 
  if (_requestHadHttpHeader) {
    // Write the Http Header -- nn bytes;
    //                   123456789012345678901234567890
    string httpHeader = "HTTP/1.1 200 OK\r\n\r\n";
  
    int httplen = httpHeader.size();
    if (_write(httpHeader.c_str(), httplen) != httplen) {
      addState(STATE_ERROR);
      _errNum = BAD_BYTE_COUNT;
      _errString = "ERROR - COMM - HttpSocket::writeHeader(): Cannot write http header.";
      return (-1);
    }
  }
  
  return Socket::writeHeader(len, product_id, seq_no, wait_msecs);
 
}

