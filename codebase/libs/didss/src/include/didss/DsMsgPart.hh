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

////////////////////////////////////////////////////////////////////////////////
//
// Terri Betancourt
// RAP, NCAR, POBox 3000, Boulder, CO, 80307-3000
// 
// May 1998
// 
////////////////////////////////////////////////////////////////////////////////

#ifndef _DS_MSG_PART_INC_
#define _DS_MSG_PART_INC_

#include <dataport/port_types.h>
#include <string>
#include <iostream>
using namespace std;

class DsMsgPart
{

public:

  ///////////////
  // constructor
  // You must choose the memory model in the constructor.
  //
  // if bufIsLocal, the buffer is allocated in the object
  // and the memory contents are copied in. If !bufIsLocal,
  // _buf just points to the incoming memory.
  
  DsMsgPart(int buf_is_local);

  /////////////////////////////
  // Copy constructor
  //
  
  DsMsgPart(const DsMsgPart &rhs);

  //////////////
  // destructor

  ~DsMsgPart();

  /////////////////////////////
  // Assignment
  //
  
  DsMsgPart &operator=(const DsMsgPart &rhs);

  ////////////////////////////////////////////////////////////
  // load a part from an incoming message which is assumed to
  // be in BE byte order
  //
  // If msg_len is provided, the part is checked to make
  // sure it does not run over the end of the message.
  //
  // Returns 0 on success, -1 on error
  // Error occurs if end of part is beyond end of message.
  
  int loadFromMsg(const ssize_t part_num,
		  const void *in_msg,
		  const ssize_t msg_len = -1);

  ////////////////////////////////////////////////////
  // load a part from memory which is assumed to be in
  // host byte order

  void loadFromMem(const int type,
		   const ssize_t len,
		   const void *in_mem);

  //////////////////////////////////////////////////
  // get the type, length, offset and buffer pointer

  inline int getType() const         { return (_type); }
  inline ssize_t getLength() const       { return (_length); }
  inline ssize_t getPaddedLength() const { return (_paddedLength); }
  inline ssize_t getOffset() const       { return (_offset); }
  inline const ui08 *getBuf() const  { return (_buf); }

  // set offset
  inline void setOffset(const ssize_t offset) { _offset = offset; }

  // print header
  // If num is not specified, it is not printed

  void printHeader(ostream &out, const char *spacer, ssize_t num = -1) const;

  // print header with label
  // If num is not specified, it is not printed

  void printHeader(ostream &out, const char *spacer,
		   const string &label, ssize_t num = -1) const;

protected:

private:
  
  int _type; // part type
  ssize_t _length; // length of part data
  ssize_t _paddedLength; // padded to even 8-bytes
  ssize_t _offset; // offset in assembled message

  ui08 *_buf;
  ssize_t _nBufAlloc;
  int _bufIsLocal;

  void _allocBuf();
  DsMsgPart &_copy(const DsMsgPart &rhs);
 
};

#endif
