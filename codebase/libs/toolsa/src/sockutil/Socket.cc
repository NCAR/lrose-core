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
// Socket.cc
//
// Socket class. This is a wrapper for the sockutil read/write calls.
//
// Mike Dixon, Paddy McCarthy
// RAP, NCAR, POBox 3000, Boulder, CO, 80307, USA
//
// Dec 1998
//
////////////////////////////////////////////////////////////////////


#include <toolsa/umisc.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/Socket.hh>
#include <toolsa/TaStr.hh>
#include <dataport/bigend.h>
#include <cstdio>
#include <unistd.h>
#include <limits.h>
#include <cerrno>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
using namespace std;

ui32 Socket::SOCKET_MAGIC = 0xf0f0f0f0;
ui32 Socket::SOCKET_MAGIC_64 = 0xf6f6f6f6;

void _close_socket(int sd);

////////////////////////////////////////////////////
// default constructor
//
Socket::Socket() : SockUtil()
{
  _objectType |= SOCKET;
  _sd = -1;
  _nBytes = 0;
  _data = NULL;
  _nDataAlloc = 0;
  _seqNo = 0;

  _msgId = 0;
  _msgLen = 0;
  _msgSeqNo = 0;
}

/////////////////////////////////////////////////////////////
// constructor
//   Takes a socket descriptor.
//   Used by the ServerSocket class when a client is obtained.
// 
Socket::Socket(int sd) : SockUtil()
{
  _objectType |= SOCKET;

  _sd = sd;
  if (_sd >= 0) {
    addState(STATE_OPENED);
    addState(STATE_SERVER_OPENED);
  }

  _nBytes = 0;
  _data = NULL;
  _nDataAlloc = 0;
  _seqNo = 0;

  _msgId = 0;
  _msgLen = 0;
  _msgSeqNo = 0; 
}

///////////////
// destructor
//
// Closes socket if open.
// Frees any data.
//
Socket::~Socket()
{
  close();
  freeData();
}

//////////////////////
// free up data buffer
//
// Frees any data.
//
void Socket::freeData()
{
  if (_data) {
    delete[] _data;
    _data = NULL;
    _nDataAlloc = 0;
  }
}

//////////////////
// open()
//
// Note: this open is NOT THREAD-SAFE, because of the 
// gethostbyname() call. Use the ThreadSocket class in the dsserver
// library for thread-safe socketing.
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
//                      (only applicable if wait_msecs is set)
//   UNEXPECTED     = Unknown error returned from the SKU_ functions.
//

int Socket::open(const char *hostname,
                 const int port,
                 const ssize_t wait_msecs /* = -1*/ )
{

  if (hasState(STATE_SERVER_OPENED)) {
      _errString = "ERROR - COMM - Trying to open a socket that was created ";
      _errString += "by a call to ServerSocket::getClient().";
      _errNum = UNEXPECTED;
      return -1;
  }

  if (hasState(STATE_OPENED)) {
      _errString = "ERROR - COMM - Trying to open a socket that is already opened.";
      _errNum = UNEXPECTED;
      return -1;
  }

  _errNum = _openClient(hostname, port, wait_msecs);
  
  if (_errNum) {

    addState(STATE_ERROR);

    switch (_errNum) {

    case UNKNOWN_HOST:
      _errString = "ERROR - COMM - Could not find host \"";
      _errString += hostname;
      _errString += "\"";
      break;
      
    case SOCKET_FAILED:
      _errString = "ERROR - COMM - Could not setup socket.";
      break;

    case CONNECT_FAILED: {
      _errString = "ERROR - COMM - Could not connect to specified host: ";
      _errString += hostname;
      _errString += " port: ";
      char buf[10];
      sprintf(buf, "%d", port);
      _errString += buf;
      _errString += ".";
      break;
    }
    
    case SELECT_FAILED:
      _errString = "ERROR - COMM - Could not do select on socket.";
      break;
      
    case TIMED_OUT:
      _errString = "Timed out trying to connect.";
      break;

    default:
      _errNum = UNEXPECTED;
      _errString = "ERROR - Unexpected Error.";
      break;
    }

    return (-1);

  } else {

    removeState(STATE_ERROR); // Previous attempts to open may have failed.
    addState(STATE_OPENED);
    return (0);

  }

}

////////////////////////
// close()
//
// Close socket
//
//   Note that sockets cannot be reopened.
//
void Socket::close()
{
  removeState(STATE_OPENED);
  addState(STATE_CLOSED);
  if (_sd >= 0) {
    _close_socket(_sd);
    _sd = -1;
  }
}

////////////////////////////////////////////////////////
// readSelect()
//
// Checks if something is waiting to be read.
// If wait_msecs is set, returns after wait_msecs (millisecs) if
// there is nothing yet. Blocks if wait_msecs is not set.
//
// Returns 0 on success, -1 on failure.
//
// On failure, getErrNum() will return the following:
//  TIMED_OUT:     timed out
//  SELECT_FAILED: select failed
//  UNEXPECTED:    unknown error returned from the SKU_ functions.
//
int Socket::readSelect(const ssize_t wait_msecs /* = -1*/ )
{

  removeState(STATE_WROTELAST);
  addState(STATE_READLAST);
  
  if (_readSelect(_sd, wait_msecs)) {
    addState(STATE_ERROR);
    _errNum = SELECT_FAILED;
    _errString = "ERROR - COMM - readSelect() - failed.";
    return (-1);
  }

  if (_timedOut) {
    addState(STATE_ERROR);
    _errNum = TIMED_OUT;
    _errString = "ERROR - COMM - readSelect() - timed out.";
    return (-1);
  }
    
  return (0);

}

////////////////////////////////////////////////////////
// readSelectPmu()
//
// Checks if something is waiting to be read.
// If wait_msecs is set, returns after wait_msecs (millisecs) if
// there is nothing yet. Blocks if wait_msecs is not set.
// 
// Registers with procmap while waiting
//
// Returns 0 on success, -1 on failure.
//
// On failure, getErrNum() will return the following:
//  TIMED_OUT:     timed out
//  SELECT_FAILED: select failed
//  UNEXPECTED:    unknown error returned from the SKU_ functions.
//
int Socket::readSelectPmu(const ssize_t wait_msecs /* = -1*/ )
{

  removeState(STATE_WROTELAST);
  addState(STATE_READLAST);
  
  if (_readSelectPmu(_sd, wait_msecs)) {
    addState(STATE_ERROR);
    _errNum = SELECT_FAILED;
    _errString = "ERROR - COMM - readSelectPmu() failed.";
    return (-1);
  }

  if (_timedOut) {
    addState(STATE_ERROR);
    _errNum = TIMED_OUT;
    _errString = "ERROR - COMM - readSelectPmu() timed out.";
    return (-1);
  }
    
  return (0);

}

////////////////////////////////////////////////////////
// writeSelect()
//
// Checks for write access.
// If wait_msecs is set, returns after wait_msecs (millisecs) if
// there is nothing yet. Blocks if wait_msecs is not set.
//
// Returns 0 on success, -1 on failure.
//
// On failure, getErrNum() will return the following:
//  TIMED_OUT:     timed out
//  SELECT_FAILED: select failed
//  UNEXPECTED     Unknown error returned from the SKU_ functions.
//
int Socket::writeSelect(const ssize_t wait_msecs /* = -1*/ )
{

  removeState(STATE_WROTELAST);
  addState(STATE_READLAST);

  if (_writeSelect(_sd, wait_msecs)) {
    addState(STATE_ERROR);
    _errNum = SELECT_FAILED;
    _errString = "ERROR - COMM - writeSelect() failed.";
    return (-1);
  }

  if (_timedOut) {
    addState(STATE_ERROR);
    _errNum = TIMED_OUT;
    _errString = "ERROR - COMM - writeSelect timed out.";
    return (-1);
  }
    
  return (0);

}

////////////////////////////////////////////////////////
// readBuffer()
//
// Reads a buffer of a given length.
//
// If wait_msecs is set, returns after wait_msecs (millisecs)
// even if read is not complete. Blocks if wait_msecs is not set.
//
// Allocation of memory for the buffer is the
// responsibility of the calling routine.
//
// getNumBytes() will return the number of bytes actually read.
//
// Returns 0 on success, -1 on failure.
//
// On failure, getErrNum() will return BAD_BYTE_COUNT or TIMED_OUT.
//
int Socket::readBuffer(void * const buf,
                       const ssize_t len,
                       const ssize_t wait_msecs /* = -1*/ )
{

  removeState(STATE_WROTELAST);
  addState(STATE_READLAST);

  ssize_t nRead = _read(buf, len, wait_msecs);
  _nBytes = nRead;

  if (nRead != len) {
    addState(STATE_ERROR);
    if (wait_msecs >= 0 && _timedOut) {
      _errNum = TIMED_OUT;
      _errString  = "readBuffer() timed out.";
    } else {
      _errNum = BAD_BYTE_COUNT;
      _errString  = "ERROR - COMM - readBuffer()\n";
      _errString += "  Number of bytes read does not match read length.\n";
      char tmpStr[128];
      sprintf(tmpStr, "%lld bytes requested, %lld read.", 
              (long long) len, (long long) nRead);
      _errString += tmpStr;
    }
    return (-1);
  } else {
    return (0);
  }

}

////////////////////////////////////////////////////////
// peek()
//
// Reads a buffer of a given length, without removing
// the data from the queue.
//
// If wait_msecs is set, returns after wait_msecs (millisecs)
// even if read is not complete. Blocks if wait_msecs is not set.
//
// Allocation of memory for the buffer is the
// responsibility of the calling routine.
//
// getNumBytes() will return the number of bytes actually read.
//
// Returns 0 on success, -1 on failure.
//
// On failure, getErrNum() will return BAD_BYTE_COUNT or TIMED_OUT.
//
int Socket::peek(void * const buf,
		 const ssize_t len,
		 const ssize_t wait_msecs /* = -1*/ )
{
  
  removeState(STATE_WROTELAST);
  addState(STATE_READLAST);

  ssize_t nRead = _peek(buf, len, wait_msecs);
  _nBytes = nRead;
  
  if (nRead != len) {
    addState(STATE_ERROR);
    if (wait_msecs >= 0 && _timedOut) {
      _errNum = TIMED_OUT;
      _errString  = "peek() timed out.";
    } else {
      _errNum = BAD_BYTE_COUNT;
      _errString  = "ERROR - COMM - peek()\n";
      _errString += "  Number of bytes read does not match read length.\n";
      char tmpStr[128];
      sprintf(tmpStr, "%lld bytes requested, %lld read.", 
              (long long) len, (long long) nRead);
      _errString += tmpStr;
    }
    return (-1);
  } else {
    return (0);
  }

}

////////////////////////////////////////////////////////
// readBufferHb()
//
// Reads a buffer of a given length.
//
// If wait_msecs is set, returns after wait_msecs (millisecs)
// even if read is not complete. Blocks if wait_msecs is not set.
//
// Executes given heartbeat function periodically during read
// operation.  This is useful for registration requirements while
// reading large buffers of data off of slow connections.
//
// chunk_size indicates the number of bytes to read in each chunk.
// Chunks are read until the desired message length is reached or
// the select() before a chunk times out.
//
// Allocation of memory for the buffer is the
// responsibility of the calling routine.
//
// getNumBytes() will return the number of bytes actually read.
//
// Returns 0 on success, -1 on failure.
//
// On failure, getErrNum() will return BAD_BYTE_COUNT or TIMED_OUT.
//
int Socket::readBufferHb(void * const buf,
                         const ssize_t len,
                         const ssize_t chunk_size,
                         const heartbeat_t heartbeat_func,
                         const ssize_t wait_msecs /* = -1*/ )
{
  
  removeState(STATE_WROTELAST);
  addState(STATE_READLAST);
  
  ssize_t nRead = _readHb(buf, len, chunk_size, heartbeat_func, wait_msecs);
  _nBytes = nRead;
  
  if (nRead != len) {
    addState(STATE_ERROR);
    if (wait_msecs >= 0 && _timedOut) {
      _errNum = TIMED_OUT;
      _errString  = "readBufferHb() timed out.";
    } else {
      _errNum = BAD_BYTE_COUNT;
      _errString  = "ERROR - COMM - readBufferHb()\n";
      _errString += "  Number of bytes read does not match read length.\n";
      char tmpStr[128];
      sprintf(tmpStr, "%lld bytes requested, %lld read.", 
              (long long) len, (long long) nRead);
      _errString += tmpStr;
    }
    return (-1);
  } else {
    return (0);
  }

}

////////////////////////////////////////////////////////
// writeBuffer()
//
// Writes a buffer of a given length.
//
// If wait_msecs is set, returns after wait_msecs (millisecs)
// even if write device is not ready.
// Blocks if wait_msecs is not set.
//
// getNumBytes() will return the number of bytes actually written.
//
// Returns 0 on success, -1 on failure.
//
// On failure, getErrNum() will return the following:
//  TIMED_OUT:      The write timed out waiting for write access.
//  SELECT_FAILED:  Select failed when attempting to write.
//  BAD_BYTE_COUNT: The write was incomplete for some reason.
//
int Socket::writeBuffer(const void *buf,
                        const ssize_t len,
                        const ssize_t wait_msecs /* = -1*/ )
{

  removeState(STATE_READLAST);
  addState(STATE_WROTELAST);

  _nBytes = 0;

  if (wait_msecs >= 0) {
    if (_writeSelect(_sd, wait_msecs)) {
      addState(STATE_ERROR);
      _errNum = SELECT_FAILED;
      _errString = "ERROR - COMM - Socket::writeBuffer() - select error.";
      return (-1);
    }
    if (_timedOut) {
      addState(STATE_ERROR);
      _errNum = TIMED_OUT;
      _errString = "ERROR - COMM - Socket::writeBuffer() - timed out.";
      return (-1);
    }
  }

  _nBytes = _write(buf, len);

  if (_nBytes == len) {
    return (0);
  } else {
    addState(STATE_ERROR);
    _errNum = BAD_BYTE_COUNT;
    _errString = "ERROR - COMM - Socket::writeBuffer()\n";
    _errString += "  Number of bytes written is not expected length.\n";
    TaStr::AddInt(_errString, "  N expected: ", len, true);
    TaStr::AddInt(_errString, "  N written : ", _nBytes, true);
    return (-1);
  }

}

////////////////////////////////////////////////////////
// readHeader()
//
// Reads a msg_header.
//
// getHeaderLength() will return the length of the data buffer
// getHeaderId() will return the id of the header
// getHeaderSeqNo() will return the sequence number of the header
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
int Socket::readHeader(const ssize_t wait_msecs /* = -1*/ )
{

  removeState(STATE_WROTELAST);
  addState(STATE_READLAST);

  // read in the first 8 bytes - the magic cookie

  ui32 magic[2];

  if (_read(magic, 8, wait_msecs) != 8) {
    addState(STATE_ERROR);
    if (_timedOut) {
      _errNum = TIMED_OUT;
      _errString = "ERROR - COMM - Socket::readHeader(): Timed out.";
      TaStr::AddInt(_errString, "  wait_msecs: ", wait_msecs);
    } else {
      _errNum = BAD_BYTE_COUNT;
      _errString =
	"ERROR - COMM - Socket::readHeader():\n"
	"  Cannot read magic cookie -- first 8 bytes of the reply message.\n"
	"  If app is server, client probably exited\n"
	"  If app is client, server child probably exited (maybe SEGV?)\n";
    }
    
    return (-1);
  }
  
  // check the cookie - 32-bit header
  
  if (magic[0] == SOCKET_MAGIC && magic[1] == SOCKET_MAGIC) {

    msg_header_t hdr;
    if (_read(&hdr, sizeof(hdr), wait_msecs) != sizeof(hdr)) {
      addState(STATE_ERROR);
      if (_timedOut) {
        _errNum = TIMED_OUT;
        _errString = "ERROR - COMM - Socket::readHeader(): Timed out.";
      } else {
        _errNum = READ_HEADER_FAILED;
        _errString = "ERROR - COMM - Socket::readHeader(): Cannot read header.";
      }
      return -1;
    }
    
    _msgId = BE_to_si32(hdr.id);
    _msgLen = BE_to_si32(hdr.len);
    _msgSeqNo = BE_to_si32(hdr.seq_no);
    
    return 0;

  }

  // check the cookie - 64-bit header
  
  if (magic[0] == SOCKET_MAGIC_64 && magic[1] == SOCKET_MAGIC_64) {

    msg_header_64_t hdr;
    if (_read(&hdr, sizeof(hdr), wait_msecs) != sizeof(hdr)) {
      addState(STATE_ERROR);
      if (_timedOut) {
        _errNum = TIMED_OUT;
        _errString = "ERROR - COMM - Socket::readHeader(): Timed out.";
      } else {
        _errNum = READ_HEADER_FAILED;
        _errString = "ERROR - COMM - Socket::readHeader(): Cannot read header.";
      }
      return -1;
    }
    
    _msgId = BE_to_si64(hdr.id);
    _msgLen = BE_to_si64(hdr.len);
    _msgSeqNo = BE_to_si64(hdr.seq_no);
    
    return 0;

  }

  // error - no magic cookie found

  addState(STATE_ERROR);
  _errNum = BAD_MAGIC_COOKIE;
  _errString = "ERROR - COMM - Socket::readHeader(): bad magic cookie.";
  char errTxt[1024];
  sprintf(errTxt, "magic[0], magic[1]: %x, %x\n", magic[0], magic[1]);
  _errString += errTxt;
  return -1;

}

////////////////////////////////////////////////////////
// writeHeader()
//
// Writes an SKU header, based on the len, id and seq_no provided.
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
int Socket::writeHeader(const ssize_t len,
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

  if (len < INT_MAX) {

    // use 32-bit header
    // write the first 8 bytes - the magic cookie
    
    ui32 magic[2] = {SOCKET_MAGIC, SOCKET_MAGIC};
    
    if (_write(magic, 8) != 8) {
      addState(STATE_ERROR);
      _errNum = BAD_BYTE_COUNT;
      _errString = "ERROR - COMM - Socket::writeHeader(): Cannot write magic cookie.";
      return (-1);
    }
    
    msg_header_t hdr;
    hdr.id = BE_to_si32(product_id);
    hdr.len = BE_to_si32(len);
    hdr.seq_no = BE_to_si32(seq_no);

    if (_write(&hdr, sizeof(hdr)) != sizeof(hdr)) {
      addState(STATE_ERROR);
      _errNum = WRITE_HEADER_FAILED;
      _errString = "ERROR - COMM - Socket::writeHeader(): Cannot write header.";
      return (-1);
    }

  } else {

    // large message, use 64-bit header

    // write the first 8 bytes - the magic cookie
    
    ui32 magic[2] = {SOCKET_MAGIC_64, SOCKET_MAGIC_64};
    
    if (_write(magic, 8) != 8) {
      addState(STATE_ERROR);
      _errNum = BAD_BYTE_COUNT;
      _errString = "ERROR - COMM - Socket::writeHeader(): Cannot write magic cookie.";
      return (-1);
    }
    
    msg_header_64_t hdr;
    hdr.id = BE_to_si64(product_id);
    hdr.len = BE_to_si64(len);
    hdr.seq_no = BE_to_si64(seq_no);
    
    if (_write(&hdr, sizeof(hdr)) != sizeof(hdr)) {
      addState(STATE_ERROR);
      _errNum = WRITE_HEADER_FAILED;
      _errString = "ERROR - COMM - Socket::writeHeader(): Cannot write header.";
      return (-1);
    }

  }
    
  return (0);

}

///////////////////////////////////////////////////////////////////
// readMessage()
//
// Read a message into a buffer local to the object. A message is
//   comprised of an SKU_header_t struct, followed by a data buffer.
// 
// This method is thread-safe if a single thread has access to this object.
//
// If wait_msecs is set, returns after wait_msecs (millisecs)
//   even if read is not complete. Blocks if wait_msecs is not set.
//
// getData() will return a pointer to the data buffer
// getHeaderLength() will return the length of the data buffer
// getHeaderId() will return the id of the header
// getHeaderSeqNo() will return the sequence number of the header
// getNumBytes() will return the number of bytes actually read.
//
// returns 0 on success, -1 on failure.
//
// On failure, getErrNum() will return the following:
//  TIMED_OUT:           read timed out
//  BAD_BYTE_COUNT:      not enough bytes for messag header, or incorrect
//                       number of bytes in message.
//                       getErrString() identifies which.
//  BAD_MAGIC_COOKIE:    incorrect magic cookie in header
//  READ_HEADER_FAILED:  error reading header
// 
int Socket::readMessage(const ssize_t wait_msecs /* = -1*/ )
{
  removeState(STATE_WROTELAST);
  addState(STATE_READLAST);
  _nBytes = 0;

  // Read the header into the _header data member.
  if (readHeader(wait_msecs)) {
    // Note: readHeader() updates error num and error string.
    return (-1);                         
  }

  // Make sure the data buffer is large enough.
  if (_msgLen > _nDataAlloc) {
    if (_data) {
      delete[] _data;
    }
    _data = new ui08[_msgLen];
    _nDataAlloc = _msgLen;
  }

  // Read the data into the _data data member.
  if (readBuffer(_data, _msgLen, wait_msecs)) {
    // Note: readBuffer() updates error num and error string.
    return (-1);
  }

  return 0;
}

////////////////////////////////////////////////////////
// writeMessage()
//
// Writes a message as an SKU header followed by a data buffer.
//   Uses a sequence number for the message that is local to this
//   object. This may not make much sense, but it's thread safe.
// 
// This method is thread-safe if a single thread has access to this object.
// 
// Increments this object's sequence number for the header to be sent.
//
// If wait_msecs is set, returns after wait_msecs (millisecs)
//   even if write is not complete. Blocks if wait_msecs is not set.
//
// getNumBytes() will return the number of bytes actually written,
//   not including the header.
//
// Returns 0 on success, -1 on failure.
//
// On failure, getErrNum() will return the following:
//  TIMED_OUT:           timed out
//  SELECT_FAILED:       select failed
//  WRITE_HEADER_FAILED: SKU_write_header failed for one of various reasons.
//  BAD_BYTE_COUNT:      write incomplete.
//  UNEXPECTED           Unknown error returned from the SKU_ functions.
//
int Socket::writeMessage(const int product_id,
                         const void *data,
                         const ssize_t data_len,
                         const ssize_t wait_msecs /* = -1*/ )
{
  removeState(STATE_READLAST);
  addState(STATE_WROTELAST);

  if (_seqNo > INT_MAX - 1) {
    _seqNo = 0;
  }
  _seqNo++;
  _nBytes = 0;

  if (writeHeader(data_len, product_id, _seqNo, wait_msecs)) {
    // Note: writeHeader() updates error num and error string.
    return (-1);
  }

  if (writeBuffer((void *) data, data_len, wait_msecs)) {
    // Note: writebuffer() updates error num and error string.
    return (-1);
  }

  return (0);
}

/////////////////////////////////////////////////////////////
// strip an HTTP header off a socket message
//
// Reads socket until it reaches \r\n\r\n byte sequence.
//
// Loads up header string with bytes read.
//
// Blocks if wait_msecs == -1.
// Waits until something to read or times out.
// If times out _timedOut is set true.
//
// Returns 0 on success, -1 on failure

int Socket::stripHttpHeader(string &header,
			    const ssize_t wait_msecs /* = -1*/ )
  
{

  _errString = "Socket::stripHttpHeader\n";
  header = "";

  int errCount = 0;
  char c1 = 0, c2 = 0, c3 = 0, c4 = 0;
  
  if (wait_msecs > 0) {
    // set the timeout on the socket
    struct timeval tv;
    tv.tv_sec = wait_msecs / 1000;
    tv.tv_usec = (wait_msecs % 1000) * 1000;
    if (setsockopt (_sd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof tv)) {
      _errString += "  setsockopt() failed";
      return (-1);
    }
  }
  
  while(true) {
    
    if (wait_msecs >= 0) {
      if (_readSelect(_sd, wait_msecs)) {
  	_errString += "  readSelect() failed";
  	return (-1);
      }
      if (_timedOut) {
  	_errString += "  timed out\n";
  	_errString += strerror(errno);
  	return (-1);
      }
    }
    
    errno = 0;
    ssize_t nRead = read(_sd, &c4, 1);
    
    if (nRead <= 0) {
      if (errno != EINTR) { // system call was not interrupted
	errCount++;
      }
      if (errCount >= 100) {
	_errString += "  errCount exceeded 100";
        return (-1);
      }
    } else {
      header += c4;
      errCount = 0;
      if (c1 == '\r' && c2 == '\n' && c3 == '\r' && c4 == '\n') {
	// success
	return 0;
      } else {
	// shift
	c1 = c2;
	c2 = c3;
	c3 = c4;
      }
    } // if (nRead <= 0)

  } // while
  
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

int Socket::_openClient(const char *hostname,
			const int port,
			const ssize_t wait_msecs /* = -1*/ )

{ 
  
  struct sockaddr_in rem_soc;
  memset((void*)&rem_soc, 0, sizeof(rem_soc)); 

  struct hostent *hostport; // host port info

  hostport = gethostbyname(hostname); // get the remote host info
  if(!hostport) {
    return(UNKNOWN_HOST);
  }
  
  // copy the remote sockets internet address to local hostport struc
  
#ifdef AIX
  // AIX has different sockaddr_in structure ! see /usr/include/netinet/in.h */
  rem_soc.sin_len = sizeof(rem_soc);
#endif
  
 rem_soc.sin_family = AF_INET;
  memcpy(&rem_soc.sin_addr, hostport->h_addr, 
	 sizeof(rem_soc.sin_addr));
  
  rem_soc.sin_port = port; // fill in port number */
  rem_soc.sin_port = htons(rem_soc.sin_port); 
  
  // get a file descriptor for the connection to the remote port
  
  if((_sd = socket(AF_INET,SOCK_STREAM,0)) == -1) {
    return(SOCKET_FAILED);
  }

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

/////////////////////////////////////////////////////////////
// _read
//
// Reads a simple message of given length.
//
// Memory allocated by calling routine.
//
// Blocks if wait_msecs == -1.
// Waits until something to read or times out.
// If times out _timedOut is set true.
//
// Returns nbytes read. If successful, nbytes == len.
//

ssize_t Socket::_read(void *mess, const ssize_t len, 
                      const ssize_t wait_msecs /* = -1*/ )
  
{

  ssize_t totalRead = 0;
  int errCount = 0;
  ssize_t nTarget = len;
  char *ptr = (char *) mess;

  if (wait_msecs > 0) {
    // set the timeout on the socket
    struct timeval tv;
    tv.tv_sec = wait_msecs / 1000;
    tv.tv_usec = (wait_msecs % 1000) * 1000;
    if (setsockopt (_sd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof tv)) {
      return (0);
    }
  }
  
  while(nTarget) {

    if (wait_msecs >= 0) {
      if (_readSelect(_sd, wait_msecs)) {
	return (0);
      }
      if (_timedOut) {
	return (0);
      }
    }

    errno = 0;
    ssize_t nRead = read(_sd, ptr, nTarget);
    
    if (nRead <= 0) {
      if (errno != EINTR) { // system call was not interrupted
	errCount++;
      }
      if (errCount >= 100) {
        return (totalRead);
      }
    } else {
      errCount = 0;
      ptr += nRead;
      totalRead += nRead;
      nTarget -= nRead;
    }
  }

  return (totalRead);

}

/////////////////////////////////////////////////////////////
// _peek
//
// Reads a simple message of given length, without removing
// the data from the queue.
//
// Memory allocated by calling routine.
//
// Blocks if wait_msecs == -1.
// Waits until something to read or times out.
// If times out _timedOut is set true.
//
// Returns nbytes read. If successful, nbytes == len.

ssize_t Socket::_peek(void *mess, const ssize_t len, 
                      const ssize_t wait_msecs /* = -1*/ )
  
{
  
  ssize_t totalRead = 0;
  int errCount = 0;
  ssize_t nTarget = len;
  char *ptr = (char *) mess;

  if (wait_msecs > 0) {
    // set the timeout on the socket
    struct timeval tv;
    tv.tv_sec = wait_msecs / 1000;
    tv.tv_usec = (wait_msecs % 1000) * 1000;
    if (setsockopt (_sd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof tv)) {
       return (0);
    }
  }
  
  while(nTarget) {

    if (wait_msecs >= 0) {
      if (_readSelect(_sd, wait_msecs)) {
	return (0);
      }
      if (_timedOut) {
	return (0);
      }
    }

    errno = 0;
    ssize_t nRead = recv(_sd, ptr, nTarget, MSG_PEEK);
    
    if (nRead <= 0) {
      if (errno != EINTR) { // system call was not interrupted
	errCount++;
      }
      if (errCount >= 100) {
        return (totalRead);
      }
    } else {
      errCount = 0;
      ptr += nRead;
      totalRead += nRead;
      nTarget -= nRead;
    }
  }

  return (totalRead);

}

///////////////////////////////////////////////////////////////////////
// _readHb
//
// Reads a simple message of given length, breaking the message into
// chunks.
// This is useful for reading large buffers of data from slow connections.
// Executes given heartbeat function between reading each chunk.
//
// chunk_size indicates the number of bytes to read in each chunk.
// Chunks are read until the desired message length is reached or
// the select() before a chunk times out.
//
// Memory allocated by calling routine
//
// Blocks if wait_msecs == -1.
// Waits until something to read or times out.
// If times out _timedOut is set true.
//
// Returns nbytes read. If successful, nbytes == len.
//

ssize_t Socket::_readHb(void *mess, const ssize_t len,
                        const ssize_t chunk_size,
                        const heartbeat_t heartbeat_func /* = NULL*/,
                        const ssize_t wait_msecs /* = -1*/ )

{

  ssize_t totalRead = 0;
  ssize_t targetSize = len;
  char *ptr = (char *) mess;
  
  while (targetSize) {
    
    if (heartbeat_func != NULL) {
      heartbeat_func("In Socket::readBufferHb()");
    }
    
    ssize_t nRequest = MIN(targetSize, chunk_size);
    ssize_t nRead = _read(ptr, nRequest, wait_msecs);
    totalRead += nRead;
    ptr += nRead;
    targetSize -= nRead;
    if (nRead != nRequest) {
      return (totalRead);
    }

  } // while
  
  return (totalRead);

}


/////////////////////////////////////////////////////////////
// _write
//
// Writes a simple message of given length.
//
// Returns nbytes written. If successful, nbytes == len.
//

ssize_t Socket::_write(const void *mess, const ssize_t len)
  
{
  
  char *ptr = (char *) mess;
  ssize_t nTarget = len;
  ssize_t totalWritten = 0;
  int errCount = 0;
  
  while(nTarget > 0) {
    errno = 0;
    ssize_t bytesWritten = write(_sd, ptr, nTarget);
    if(bytesWritten < 0) {
      if (errno != EINTR) { // system call was not interrupted
	errCount++;
      }
      if(errCount >= 100) {
	return (totalWritten);
      }
      // sleep for 1 millisecond
      umsleep(10);
    } else {
      errCount = 0;
    }
    if (bytesWritten > 0) {
      ptr += bytesWritten;
      totalWritten += bytesWritten;
      nTarget -= bytesWritten;
    }
  }
  
  return (totalWritten);

}

///////////////////////////////////////////////////////////////////
// function to wrap close so that the compiler is not confused with
// the Socket::close()

void _close_socket(int sd)

{
  close(sd);
}

