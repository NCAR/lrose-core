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
// MemBuf.cc
//
// Memory buffer class.
//
// This class provides access to an automatically-resizing
// accumulating buffer.
//
// Derived from the membuf functions.
//
// Mike Dixon, RAP, NCAR, POBox 3000, Boulder, CO, 80307, USA
//
// March 1999
//
////////////////////////////////////////////////////////////////////


#include <toolsa/MemBuf.hh>

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#include <toolsa/mem.h>
#include <iostream>
using namespace std;

//////////////////////
// default constructor
//

MemBuf::MemBuf()

{ 
  _buf = NULL;
  _nalloc = 0;
  _len = 0;
  _allowShrink = true;
} 

///////////////////
// copy constructor
//

MemBuf::MemBuf(const MemBuf &other)

{ 
  if (this == &other) {
    return;
  }
  _nalloc = other._nalloc;
  _len = other._len;
  _buf = (char *) ucalloc(1, _nalloc);
  memcpy(_buf, other._buf, _len);
} 

//////////////
// Destructor

MemBuf::~MemBuf()

{
  free();
}


///////////////////////////////////////////////////////////////
// Reset the memory buffer - sets current length to 0
// Zero's out allocated buffer memory.
//

void MemBuf::reset()

{
  _len = 0;
  memset(_buf, 0, _nalloc);
}

void MemBuf::setAllowShrink(bool allowshrink /* = true */)
{
  _allowShrink = allowshrink;
}

///////////////////////////////////////////////////////////////
// 
// Prepare a buffer by allocating or reallocating a starting size.
// This is done if you want to read data directly into the buffer.
// Note that this routine sets things up so that the internal buffer
// looks like the data has already been added, although that is left
// up to the calling routine (i.e. the buffer length is numbytes
// after the call to this routine).
//
// This routine does not change the existing parts of the buffer,
// only adjusts the size.
//
// Buffer is resized as necessary.
//
// Returns pointer to user buffer.
//

void *MemBuf::prepare(const size_t numbytes)

{
  alloc(numbytes);
  _len = numbytes;
  return(_buf);
}

void *MemBuf::reserve(const size_t numbytes)

{
  return(prepare(numbytes));
}

///////////////////////////////////////////////////////////////
// 
// Load numbytes from source array into start of target buffer.
//
// Buffer is resized as necessary.
//
// Returns pointer to user buffer.
// 

void *MemBuf::load(void const *source, const size_t numbytes)

{
  reset();
  return(add(source, numbytes));
}

///////////////////////////////////////////////////////////////
//
// Add numbytes from source array onto end of buffer.
//
// Buffer is resized as necessary.
//
// Returns pointer to user buffer.
///////////////////////////////////////////////////////////////

void *MemBuf::add(void const *source, const size_t numbytes)

{
  if (numbytes > 0 && source != NULL) {
    grow(numbytes);
    memcpy(_buf + _len, source, numbytes);
    _len += numbytes;
  }
  return(_buf);
}

///////////////////////////////////////////////////////////////
//
// Concat the contents of another MemBuf onto this one.
//
// Returns pointer to user buffer.
// 

void *MemBuf::concat(const MemBuf &other)

{
  return (add(other._buf, other._len));
}

///////////////////////////////////////////////////////////////
// 
// assignment
//

void MemBuf::operator=(const MemBuf &other)

{
  free();
  add(other._buf, other._len);
}

///////////////////////////////////////////////////////////////
// Check available space, alloc as needed
//
// Returns pointer to user buffer.

void *MemBuf::alloc(const size_t nbytes_total)

{
  
  size_t new_alloc;
  
  if(nbytes_total > _nalloc) {

    new_alloc = MAX(_nalloc * 2, nbytes_total);
    _buf = (char*) urealloc(_buf, new_alloc);
    _nalloc = new_alloc;
    
  } else if (_allowShrink && (nbytes_total < _nalloc / 2)) {
    
    new_alloc = _nalloc / 2;
    _buf = (char*) urealloc(_buf, new_alloc);
    _nalloc = new_alloc;
    if (_len > _nalloc) {
      _len = _nalloc;
    }
    
  }
  
  return (_buf);

}

///////////////////////////////////////////////////////////////
// Free up allocated space.

void MemBuf::free()

{

  if (_buf != NULL) {
    ufree(_buf);
    _buf = NULL;
  }
  _nalloc = 0;
  _len = 0;

}

///////////////////////////////////////////////////////////////
// Check available space, grow if needed
//
// Returns pointer to user buffer.
// 

void *MemBuf::grow(const size_t nbytes_needed)

{
  size_t nbytes_total = _len + nbytes_needed;
  alloc(nbytes_total);
  return (_buf);
}

///////////////////////////////////////////////////////////////
// Print - for internal debugging
// 

void MemBuf::print(ostream &out)

{
  out << "_len: " << _len << endl;
  out << "_nalloc: " << _nalloc << endl;
  out << "_buf: " << (void *)_buf << endl;	// (char *) doesn't print
}

