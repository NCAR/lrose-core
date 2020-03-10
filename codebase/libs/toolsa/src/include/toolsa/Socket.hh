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
// Socket.hh
//
// Socket class. This is a wrapper for the sockutil read/write calls.
//
// After construction, open() must be used to
// open the socket. After that, reads and writes may be used as usual.
// The destructor closes the socket.
//
// Test with $CVSROOT/libs/toolsa/test/SocketTest
//
// Mike Dixon, RAP, NCAR, POBox 3000, Boulder, CO, 80307, USA
//
// Dec 1998
//
////////////////////////////////////////////////////////////////////

#ifndef SOCKET_HH
#define SOCKET_HH


#include <toolsa/SockUtil.hh>
#include <dataport/port_types.h>
using namespace std;

class Socket : public SockUtil {

public:

  // heartbeat function typedef

  typedef void (*heartbeat_t)(const char *label);
  
  // message header struct
  // the message header is an 8 byte magic cookie
  // (f0f0f0f0) followed by the msg_header_t

  typedef struct {
    si32 id;
    si32 len; /* message size in bytes, not including header */
    si32 seq_no;
  } msg_header_t;

  // 64-bit message header struct - for messages longer than INT_MAX
  // the message header is an 8 byte magic cookie
  // (f6f6f6f6) followed by the msg_header_64_t

  typedef struct {
    si64 id;
    si64 len; /* message size in bytes, not including header */
    si64 seq_no;
  } msg_header_64_t;

  ///////////////////////
  // default constructor
  // 
  Socket();

  //////////////////////////////////////////////////////////////
  // constructor
  //   Takes a socket descriptor.
  //   Used by the ServerSocket class when a client is obtained.
  // 
  Socket(int sd);

  //////////////
  // destructor
  //   Closes socket if open
  //   Frees any data.
  // 
  virtual ~Socket();

  //////////////////////
  // free up data buffer
  //
  // Frees any data.
  //
  void freeData();

  ////////////////////////////////////////
  // open()
  //
  // Open connection to indicated host and port
  //
  // Note: this open is NOT THREAD-SAFE, because of the 
  // gethostbyname() call. Use the ThreadSocket class in the dsserver
  // library for thread-safe socketing.
  //
  // Returns 0 on success, -1 on  failure
  //
  // On failure, errNum() will return the following:
  //   UNKNOWN_HOST   = Could not find host name
  //   SOCKET_FAILED  = Could not setup socket (max file descriptors?)
  //   CONNECT_FAILED = Could not connect to specified host and port
  //   TIMED_OUT      = Timed out trying to connect
  //                      (only applicable if wait_msecs is set)
  //   UNEXPECTED     = Unknown error returned from the functions.
  //
  virtual int open(const char *hostname,
		   const int port,
		   const ssize_t wait_msecs = -1);

  ///////////////
  // close()
  //
  // Close socket
  // 
  //   Note that sockets cannot be reopened.
  //
  virtual void close();

  //////////////////////
  // is the socket open?
  //
  bool isOpen() const { return (_sd >= 0); }

  /////////////////////////////////////////////
  // readSelect()
  //
  // Checks if something is waiting to be read.
  // If wait_msecs is set, returns after wait_msecs (millisecs) if
  // there is nothing yet. Blocks if wait_msecs is not set.
  //
  // Returns 0 on success, -1 on failure.
  //
  // On failure, errNum() will return the following:
  //  TIMED_OUT:     timed out
  //  SOCKET_FAILED: select failed
  //  UNEXPECTED:    unknown error returned from the functions.
  //
  int readSelect(const ssize_t wait_msecs = -1);

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
  int readSelectPmu(ssize_t const wait_msecs = -1);

  /////////////////////////////////////////////////
  // writeSelect()
  //
  // Checks for write access.
  // If wait_msecs is set, returns after wait_msecs (millisecs) if
  // there is nothing yet. Blocks if wait_msecs is not set.
  //
  // Returns 0 on success, -1 on failure.
  //
  // On failure, errNum() will return the following:
  //  TIMED_OUT:     timed out
  //  SELECT_FAILED: select failed
  //  UNEXPECTED     Unknown error returned from the functions.
  //
  int writeSelect(const ssize_t wait_msecs);

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
  int readBuffer(void * const buf,
                 const ssize_t len,
                 const ssize_t wait_msecs = -1);

  ////////////////////////////////////////////////////////
  // peek()
  //
  // Reads a buffer of a given length, without removing the
  // data from the read queue.
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
  int peek(void * const buf,
	   const ssize_t len,
	   const ssize_t wait_msecs = -1);

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
  // the select() before a chunk times out. Both len and chunk_size
  // are in units of bytes.
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
  int readBufferHb(void * const buf,
                   const ssize_t len,
                   const ssize_t chunk_size,
                   const heartbeat_t heartbeat_func,
                   const ssize_t wait_msecs = -1);

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
  int writeBuffer(const void *buf,
                  const ssize_t len,
                  const ssize_t wait_msecs = -1);
  
  ////////////////////////////////////////////////////////
  // readHeader()
  //
  // Reads a message header.
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
  // On failure, getErrNum() will return BAD_BYTE_COUNT.
  //
  virtual int readHeader(const ssize_t wait_msecs = -1);

  ////////////////////////////////////////////////////////
  // writeHeader()
  //
  // Writes a message header, based on the len, id and seq_no provided.
  //
  // If wait_msecs is set, returns after wait_msecs (millisecs)
  // even if write is not complete. Blocks if wait_msecs is not set.
  //
  // Returns 0 on success, -1 on failure.
  // 
  //  TIMED_OUT:           timed out
  //  SELECT_FAILED:       select failed
  //  WRITE_HEADER_FAILED: _writeHeader failed for one of various reasons.
  //  UNEXPECTED           Unknown error returned from the functions.
  //
  virtual int writeHeader(const ssize_t len,
                          const ssize_t product_id,
                          const ssize_t seq_no,
                          const ssize_t wait_msecs = -1);
  
  ///////////////////////////////////////////////////////////////////
  // readMessage()
  //
  // Read a message into a buffer local to the object. A message is
  //   comprised of an msg_header_t struct, followed by a data buffer.
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
  int readMessage(const ssize_t wait_msecs = -1);

  ////////////////////////////////////////////////////////
  // writeMessage()
  //
  // Writes a message as a header followed by a data buffer.
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
  //  WRITE_HEADER_FAILED: write_header failed for one of various reasons.
  //  BAD_BYTE_COUNT:      write incomplete.
  //  UNEXPECTED           Unknown error returned from the functions.
  //
  int writeMessage(const int product_id,
                   const void *data,
                   const ssize_t len,
                   const ssize_t wait_msecs = -1);

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
  
  int stripHttpHeader(string &header,
		      const ssize_t wait_msecs = -1);
  
  //////////////////////////////////////
  // number of bytes read or written, not including the header (if a message).
  // 
  ssize_t getNumBytes() { return (_nBytes); }

  /////////////////////////////////////
  // header info after reading messages

  ssize_t getHeaderId() const { return _msgId; }
  ssize_t getHeaderLength() const { return _msgLen; }
  ssize_t getHeaderSeqNo() const { return _msgSeqNo; }
  
  ////////////////////////////////////
  // data pointer after reading messages
  // 
  const void *getData() const { return (_data); }

protected:

  static ui32 SOCKET_MAGIC;
  static ui32 SOCKET_MAGIC_64;

  int _sd;             // socket descriptor
  ssize_t _nBytes;     // number of bytes read or written

  ssize_t _msgId;      // message ID
  ssize_t _msgLen;     // message size in bytes, not including header
  ssize_t _msgSeqNo;   // message sequence number

  ui08 *_data;         // message data
  ssize_t _nDataAlloc; // size of _data buffer
  ssize_t _seqNo;      // sequence number for outgoing messages

  int _openClient(const char *hostname,
		  const int port,
		  const ssize_t wait_msecs = -1);
  ssize_t _read(void *mess, const ssize_t len, const ssize_t wait_msecs = -1);
  ssize_t _peek(void *mess, const ssize_t len, const ssize_t wait_msecs = -1);
  ssize_t _readHb(void *mess, const ssize_t len,
                  const ssize_t chunk_size,
                  const heartbeat_t heartbeat_func = NULL,
                  const ssize_t wait_msecs = -1);
  ssize_t _write(const void *mess, const ssize_t len);
  
private:
  
};

#endif

