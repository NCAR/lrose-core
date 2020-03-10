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
// DsSocket.hh
//
// Socket class that allows servers to remove and http header.
//
// Paddy McCarthy, RAP, NCAR, POBox 3000, Boulder, CO, 80307, USA
//
// Nov 2009
//
////////////////////////////////////////////////////////////////////

#ifndef HTTP_SOCKET_HH
#define HTTP_SOCKET_HH


#include <toolsa/Socket.hh>
#include <dataport/port_types.h>
using namespace std;

class HttpSocket : public Socket {

public:

  //////////////
  // default constructor
  // 
  HttpSocket();

  ///////////////
  // constructor
  //   Takes a socket descriptor. Used by the ServerSocket class when a 
  //     client is obtained.
  // 
  HttpSocket(int sd);

  //////////////
  // destructor
  //   Closes socket if open
  //   Frees any data.
  // 
  virtual ~HttpSocket();

  ////////////////////////////////////////////////////////
  // readHeader()
  //
  // Reads a message header.
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
  // On failure, getErrNum() will return BAD_BYTE_COUNT.
  //
  virtual int readHeader(const ssize_t wait_msecs = -1);

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
  //  TIMED_OUT:           timed out
  //  SELECT_FAILED:       select failed
  //  WRITE_HEADER_FAILED: _writeHeader failed for one of various reasons.
  //  UNEXPECTED           Unknown error returned from the functions.
  //
  virtual int writeHeader(const ssize_t len,
                          const ssize_t product_id,
                          const ssize_t seq_no,
                          const ssize_t wait_msecs = -1);
  
private:
  bool _requestHadHttpHeader;

};

#endif

