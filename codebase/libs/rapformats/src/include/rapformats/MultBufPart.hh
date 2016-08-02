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
// MultBufPart.hh
//
// From DsMsgPart.hh
//
// Mike Dixon, RAP, NCAR, POBox 3000, Boulder, CO, 80307-3000
//
// March 2000
//
////////////////////////////////////////////////////////////

#ifndef _MultBufPart_hh
#define _MultBufPart_hh


#include <string>
#include <rapformats/MultBuf.hh>
using namespace std;

class MultBufPart
{

public:

  ///////////////
  // constructor
  
  MultBufPart();

  //////////////
  // destructor

  ~MultBufPart();

  ////////////////////////////////////////////////////////////
  // load a part from an incoming buffer which is assumed to
  // be in BE byte order
  //
  // Returns 0 on success, -1 on error
  // Error occurs if end of part is beyond end of buffer.
  // Error string retrieved with getErrStr().
  
  int loadFromBuf(const int part_num,
		  const void *in_buf,
		  const int buf_len);

  ////////////////////////////////////////////////////
  // load a part from memory which is assumed to be in
  // host byte order

  void loadFromMem(const int part_type,
		   const int len,
		   const void *in_mem);

  //////////////////////////////////////////////////
  // get the type, length, offset and buffer pointer

  inline int getType() const         { return (_type); }
  inline int getLength() const       { return (_length); }
  inline int getOffset() const       { return (_offset); }
  inline const void *getBuf() const  { return (_buf.getPtr()); }

  // set offset
  inline void setOffset(const int offset) { _offset = offset; }
  
  // print header
  // If num is not specified, it is not printed

  void printHeader(ostream &out, const int num = -1,
		   const string &spacer = "") const;

  // get the error string

  const string &getErrStr() const { return (_errStr); }
  
protected:

private:
  
  int _type; // part type
  int _length; // length of part data
  int _offset; // offset in assembled buffer

  MemBuf _buf;
  string _errStr;

};

#endif
