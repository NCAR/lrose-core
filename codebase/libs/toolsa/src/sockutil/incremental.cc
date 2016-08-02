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
// incremental.cc
//
// Incremental read/write routines in ThreadSocket class.
// Kept in separate file for linkage reasons, so that not
// all clients of ThreadSocket need to link in pthreads.
//
// Mike Dixon, RAP, NCAR, POBox 3000, Boulder, CO, 80307, USA
//
// Nov 1999
//
////////////////////////////////////////////////////////////////////


#include <dataport/bigend.h>
#include <toolsa/umisc.h>
#include <toolsa/ThreadSocket.hh>
#include <pthread.h>
#include <limits.h>
#include <iostream>
using namespace std;

///////////////////////////////////////////////////////////////////
// readMessageIncr()
//
// Read a message incrementally into a buffer local to the object.
//
// A message comprises an 8-byte magic cookie and a 12-byte header
// struct followed by a data buffer. The header contains the
// size of the data buffer to be read.
// 
// This method is thread-safe if a single thread has access to this object.
//
// The value of nbytesExpected is initialized to 1024, and then set
// after reading the header. nbytesRead will be updated
// as the message is read in. In multi-threaded applications this
// allows a different thread to keep track of the progress of the read.
//
// returns 0 on success, -1 on failure.
//
// data() will return a pointer to the data buffer
// len() will return the length of the data buffer
// id() will return the id of the header
// seqNo() will return the sequence number of the header
// nBytes() will return the number of bytes actually read.
//
// On failure, getErrNum() will return one the following:
//  BAD_BYTE_COUNT:      not enough bytes for messag header, or incorrect
//                       number of bytes in message.
//                       getErrString() identifies which.
//  BAD_MAGIC_COOKIE:    incorrect magic cookie in header
//  READ_HEADER_FAILED:  error reading header

int ThreadSocket::readMessageIncr(ssize_t &nbytesExpected, ssize_t &nbytesRead)

{

  nbytesExpected = 1024;
  nbytesRead = 0;

  removeState(STATE_WROTELAST);
  addState(STATE_READLAST);
  _nBytes = 0;
  _errString = "";
  
  // Read the header into the _header data member.
  if (_readHeaderIncr()) {
    return (-1);                         
  }
  nbytesExpected = _msgLen;

  // alloc the data buffer
  if (_msgLen > _nDataAlloc) {
    if (_data) {
      delete[] _data;
    }
    _data = new ui08[_msgLen];
    _nDataAlloc = _msgLen;
  }

  // Read the data into the _data data member.
  
  if (_readIncr(_data, nbytesExpected, nbytesRead)) {
    _nBytes = nbytesRead;
    addState(STATE_ERROR);
    _errNum = BAD_BYTE_COUNT;
    _errString += "ERROR - ThreadSocket::readMessageIncr()\n";
    _errString += "  Too few bytes read.\n";
    char tmpStr[128];
    sprintf(tmpStr, "%lld bytes requested, %lld read.",
	    (long long) nbytesExpected, (long long) nbytesRead);
    _errString += tmpStr;
    return (-1);
  }

  _nBytes = nbytesRead;
  return (0);

}

////////////////////////////////////////////////////////
// writeMessageIncr()
//
// Writes a message incrementally.
// The message comprises an 8-byte magic cookie and a 12-byte header
// struct followed by a data buffer. The header contains the
// size of the data buffer to be written.
//
// This method is thread-safe if a single thread has access to this object.
// 
// The value of nbytesRead will be updated as the message is written.
// In multi-threaded applications this allows a different thread to
// keep track of the progress of the write.
//
// Increments this object's sequence number for the header to be sent.
//
// Returns 0 on success, -1 on failure.
//
// On failure, getErrNum() will return the following:
//  SELECT_FAILED:       select failed
//  WRITE_HEADER_FAILED: SKU_write_header failed for one of various reasons.
//  BAD_BYTE_COUNT:      write incomplete.
//  UNEXPECTED           Unknown error returned from the SKU_ functions.
//
int ThreadSocket::writeMessageIncr(const int product_id,
				   const void *data,
				   const ssize_t len,
				   ssize_t &nbytesWritten)

{

  nbytesWritten = 0;

  removeState(STATE_READLAST);
  addState(STATE_WROTELAST);
  
  _seqNo++;
  _nBytes = 0;
  _errString = "";

  if (_writeHeaderIncr(len, product_id, _seqNo)) {
    _errString += "ERROR - Socket::writeMessage\n";
    return (-1);
  }

  if (_writeIncr(data, len, nbytesWritten)) {
    _nBytes = nbytesWritten;
    addState(STATE_ERROR);
    _errNum = BAD_BYTE_COUNT;
    _errString += "ERROR - ThreadSocket::writeMessageIncr()\n";
    _errString += "  Too few bytes written.\n";
    char tmpStr[128];
    sprintf(tmpStr, "  %lld bytes in message, %lld written.",
	    (long long) len, (long long) nbytesWritten);
    _errString += tmpStr;
    return (-1);
  }

  _nBytes = nbytesWritten;
  return 0;

}

////////////////////////////////////////////////////////
// _readHeaderIncr()
//
// Reads a msg_header.
//
// len() will return the length of the data buffer
// id() will return the id of the header
// seqNo() will return the sequence number of the header
//
// Returns 0 on success, -1 on failure.
// 
// On failure, getErrNum() will return:
//   BAD_BYTE_COUNT
//   BAD_MAGIC_COOKIE
//   READ_HEADER_FAILED

#define SOCKET_MAGIC 0xf0f0f0f0

int ThreadSocket::_readHeaderIncr()
{

  removeState(STATE_WROTELAST);
  addState(STATE_READLAST);

  // read in the first 8 bytes - the magic cookie

  ui32 magic[2];
  ssize_t nbytesRead;
  
  if (_readIncr(magic, sizeof(magic), nbytesRead)) {
    addState(STATE_ERROR);
    _errNum = BAD_BYTE_COUNT;
    _errString +=
      "ThreadSocket::_readHeaderIncr(): Cannot read magic cookie.\n";
    return (-1);
  }
  
  // check the cookie - 32-bit header
  
  if (magic[0] == SOCKET_MAGIC && magic[1] == SOCKET_MAGIC) {

    msg_header_t hdr;
    if (_readIncr(&hdr, sizeof(msg_header_t), nbytesRead)) {
      addState(STATE_ERROR);
      _errNum = READ_HEADER_FAILED;
      _errString += "ThreadSocket::_readHeaderIncr(): Cannot read header.\n";
      return (-1);
    }

    _msgId = BE_to_si32(hdr.id);
    _msgLen = BE_to_si32(hdr.len);
    _msgSeqNo = BE_to_si32(hdr.seq_no);
    
    return 0;

  }

  // check the cookie - 64-bit header
  
  if (magic[0] == SOCKET_MAGIC_64 && magic[1] == SOCKET_MAGIC_64) {

    msg_header_64_t hdr;
    if (_readIncr(&hdr, sizeof(hdr), nbytesRead)) {
      addState(STATE_ERROR);
      _errNum = READ_HEADER_FAILED;
      _errString += "ThreadSocket::_readHeaderIncr(): Cannot read header.\n";
      return (-1);
    }
    
    _msgId = BE_to_si64(hdr.id);
    _msgLen = BE_to_si64(hdr.len);
    _msgSeqNo = BE_to_si64(hdr.seq_no);
    
    return 0;

  }

  addState(STATE_ERROR);
  _errNum = BAD_MAGIC_COOKIE;
  _errString += "ThreadSocket::_readHeaderIncr(): bad magic cookie.";
  return (-1);

}

////////////////////////////////////////////////////////
// _writeHeaderIncr()
//
// Writes an msg_header, based on the len, id and seq_no.
//
// Returns 0 on success, -1 on failure.
//
// On failure, getErrNum() will return the following:
//  SELECT_FAILED:       select failed
//  WRITE_HEADER_FAILED: SKU_write_header failed for one of various reasons.
//  UNEXPECTED           Unknown error returned from the SKU_ functions.
//

int ThreadSocket::_writeHeaderIncr(const ssize_t len,
				   const int product_id,
				   const ssize_t seq_no)
{

  removeState(STATE_READLAST);
  addState(STATE_WROTELAST);
  
  if (len < INT_MAX) {

    // use 32-bit header
    // write the first 8 bytes - the magic cookie
    
    ui32 magic[2] = {SOCKET_MAGIC, SOCKET_MAGIC};
    
    ssize_t nbytesWritten;
    if (_writeIncr(magic, sizeof(magic), nbytesWritten)) {
      addState(STATE_ERROR);
      _errNum = BAD_BYTE_COUNT;
      _errString += "ERROR - ThreadSocket::_writeHeaderIncr()\n";
      _errString += "Cannot write magic cookie.\n";
      return (-1);
    }

    // write the header

    msg_header_t hdr;
    hdr.id = BE_from_si32(product_id);
    hdr.len = BE_from_si32(len);
    hdr.seq_no = BE_from_si32(seq_no);
    
    if (_writeIncr(&hdr, sizeof(hdr), nbytesWritten)) {
      addState(STATE_ERROR);
      _errNum = WRITE_HEADER_FAILED;
      _errString += "ERROR - ThreadSocket::_writeHeaderIncr()\n";
      _errString += "Cannot write header.\n";
      return (-1);
    }

  } else {

    // use 64-bit header
    // write the first 8 bytes - the magic cookie
    
    ui32 magic[2] = {SOCKET_MAGIC_64, SOCKET_MAGIC_64};
    
    ssize_t nbytesWritten;
    if (_writeIncr(magic, sizeof(magic), nbytesWritten)) {
      addState(STATE_ERROR);
      _errNum = BAD_BYTE_COUNT;
      _errString += "ERROR - ThreadSocket::_writeHeaderIncr()\n";
      _errString += "Cannot write magic cookie.\n";
      return (-1);
    }

    // write the header

    msg_header_64_t hdr;
    hdr.id = BE_from_si64(product_id);
    hdr.len = BE_from_si64(len);
    hdr.seq_no = BE_from_si64(seq_no);
    
    if (_writeIncr(&hdr, sizeof(hdr), nbytesWritten)) {
      addState(STATE_ERROR);
      _errNum = WRITE_HEADER_FAILED;
      _errString += "ERROR - ThreadSocket::_writeHeaderIncr()\n";
      _errString += "Cannot write header.\n";
      return (-1);
    }

  }

  return (0);

}

/////////////////////////////////////////////////////////////
// _readIncr
//
// Incremental read of a buffer of given length.
//
// Memory for mess allocated by calling routine.
//
// Initializes nbytesRead to 0 at start of routine.
// Updates nbytesRead as the message is read incrementally.
// This enables a different thread to monitor the progress
// of the read if required.
//
// Returns 0 on success, -1 on error

int ThreadSocket::_readIncr(void *mess,
			    const ssize_t len,
			    ssize_t &nbytesRead)
  
{

  nbytesRead = 0;

  int errCount = 0;
  ssize_t nTarget = len;
  char *ptr = (char *) mess;
  
  while(nTarget) {

    // test if thread is cancelled while waiting for read
    while (_readSelect(_sd, 10)) {
      pthread_testcancel();
    }
    
    errno = 0;
    ssize_t nRead = read(_sd, ptr, nTarget);
    
    if (nRead <= 0) {
      if (errno != EINTR) { // system call was not interrupted
	errCount++;
      }
      if (errCount >= 100) {
        return (-1);
      }
    } else {
      errCount = 0;
      ptr += nRead;
      nbytesRead += nRead;
      nTarget -= nRead;
    }
  }

  return (0);

}

/////////////////////////////////////////////////////////////
// _writeIncr
//
// Incremental write of a buffer of given length.
//
// Initializes nbytesWritten to 0 at start of routine.
// Updates nbytesWritten as the message is written incrementally.
// This enables a different thread to monitor the progress
// of the write if required.
//
// Returns 0 on success, -1 on error

int ThreadSocket::_writeIncr(const void *mess,
			     const ssize_t len,
			     ssize_t &nbytesWritten)
  
{

  char *ptr = (char *) mess;
  nbytesWritten = 0;
  ssize_t nTarget = len;
  int errCount = 0;
  
  while(nTarget > 0) {

    // test if thread is cancelled while waiting for write
    while (_writeSelect(_sd, 10)) {
      pthread_testcancel();
    }
    
    errno = 0;
    ssize_t nWritten = write(_sd, ptr, nTarget);

    if(nWritten <= 0) {
      if (errno != EINTR) { // system call was not interrupted
	errCount++;
      }
      if(errCount >= 100) {
	return (-1);
      }
    } else {
      errCount = 0;
      ptr += nWritten;
      nbytesWritten += nWritten;
      nTarget -= nWritten;
    }

  }
  
  return (0);

}

