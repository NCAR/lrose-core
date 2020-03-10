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
// ThreadSocket.hh
//
// ThreadSocket class.
// This is a thread-safe version of the Socket class.
//
// Mike Dixon, RAP, NCAR, POBox 3000, Boulder, CO, 80307, USA
//
// Feb 1999
//
////////////////////////////////////////////////////////////////////

#ifndef ThreadSocket_HH
#define ThreadSocket_HH


#include <toolsa/Socket.hh>
using namespace std;

class ThreadSocket : public Socket {

public:

  //////////////////////
  // default constructor
  // 
  ThreadSocket();

  ////////////////////////////////////////////////////////////////////
  // constructor
  //   Takes a socket descriptor.
  //   Used by the ServerSocket class when a client is obtained.
  // 
  ThreadSocket(int sd);

  ////////////////////////////////////////
  // open()
  //
  // Open connection to indicated host and port
  //
  // Returns 0 on success, -1 on  failure
  //
  // On failure, errNum() will return the following:
  //   UNKNOWN_HOST   = Could not find host name
  //   ThreadSocket_FAILED  = Could not setup ThreadSocket
  //   CONNECT_FAILED = Could not connect to specified host and port
  //   TIMED_OUT      = Timed out trying to connect
  //                      (only applicable if wait_msecs is set)
  //   UNEXPECTED     = Unknown error returned from the functions.
  //
  virtual int open(const char *hostname,
		   const int port,
		   const ssize_t wait_msecs = -1);

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
  
  int readMessageIncr(ssize_t &nbytesExpected, ssize_t &nbytesRead);

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

  int writeMessageIncr(const int product_id, const void *data,
		       const ssize_t len, ssize_t &nbytesWritten);

  
protected:
  
  int _openClient(const char *hostname,
                  const int port,
                  const ssize_t wait_msecs = -1);

  int _readHeaderIncr();
  
  int _readIncr(void *mess, const ssize_t len, ssize_t &nbytesRead);
  
  int _writeHeaderIncr(const ssize_t len,
		       const int product_id,
		       const ssize_t seq_no);

  int _writeIncr(const void *mess, const ssize_t len, ssize_t &nbytesWritten);
  
private:
  
};

#endif
