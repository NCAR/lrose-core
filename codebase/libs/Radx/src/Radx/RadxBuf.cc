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
// RadxBuf.cc
//
// Memory buffer class.
//
// This class provides access to an automatically-resizing
// accumulating buffer.
//
// Mike Dixon, RAP, NCAR, POBox 3000, Boulder, CO, 80307, USA
//
// March 2010
//
////////////////////////////////////////////////////////////////////

#include <Radx/RadxBuf.hh>
#include <cstring>

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#include <iostream>
using namespace std;

//////////////////////
// default constructor
//

RadxBuf::RadxBuf()

{ 
  _buf = NULL;
  _nalloc = 0;
  _len = 0;
  _allowShrink = true;
} 

///////////////////
// copy constructor
//

RadxBuf::RadxBuf(const RadxBuf &other)

{ 
  if (this == &other) {
    return;
  }
  _nalloc = other._nalloc;
  _len = other._len;
  _buf = new char[_nalloc];
  memset(_buf, 0, _nalloc);
  memcpy(_buf, other._buf, _len);
} 

//////////////
// Destructor

RadxBuf::~RadxBuf()

{
  clear();
}


///////////////////////////////////////////////////////////////
// Reset the memory buffer - sets current length to 0
// Zero's out allocated buffer memory.
//

void RadxBuf::reset()

{
  _len = 0;
  if (_nalloc > 0) {
    memset(_buf, 0, _nalloc);
  }
}

void RadxBuf::setAllowShrink(bool allowshrink /* = true */)
{
  _allowShrink = allowshrink;
}

///////////////////////////////////////////////////////////////
// 
// Reserve a buffer by allocating or reallocating a starting size.
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

void *RadxBuf::reserve(const size_t numbytes)

{
  alloc(numbytes);
  _len = numbytes;
  return _buf;
}

///////////////////////////////////////////////////////////////
// 
// Load numbytes from source array into start of target buffer.
//
// Buffer is resized as necessary.
//
// Returns pointer to user buffer.
// 

void *RadxBuf::load(const void *source, const size_t numbytes)

{
  reset();
  return add(source, numbytes);
}

///////////////////////////////////////////////////////////////
//
// Add numbytes from source array onto end of buffer.
//
// Buffer is resized as necessary.
//
// Returns pointer to user buffer.
///////////////////////////////////////////////////////////////

void *RadxBuf::add(const void *source, const size_t numbytes)

{
  if (numbytes > 0 && source != NULL) {
    grow(numbytes);
    memcpy(_buf + _len, source, numbytes);
    _len += numbytes;
  }
  return _buf;
}

///////////////////////////////////////////////////////////////
//
// Concat the contents of another RadxBuf onto this one.
//
// Returns pointer to user buffer.
// 

void *RadxBuf::concat(const RadxBuf &other)

{
  return add(other._buf, other._len);
}

///////////////////////////////////////////////////////////////
// 
// assignment
//

void RadxBuf::operator=(const RadxBuf &other)

{
  clear();
  add(other._buf, other._len);
}

///////////////////////////////////////////////////////////////
// Check available space, alloc as needed
//
// Returns pointer to user buffer.

void *RadxBuf::alloc(const size_t nbytes_total)

{
  
  if (_buf == NULL) {

    _nalloc = nbytes_total;
    _buf = new char[_nalloc];
    memset(_buf, 0,  _nalloc);

  } else if (nbytes_total > _nalloc) {

    size_t new_alloc = MAX(_nalloc * 2, nbytes_total);
    char *save = _buf;
    _buf = new char[new_alloc];
    memcpy(_buf, save, _nalloc);
    delete[] save;
    _nalloc = new_alloc;
    
  } else if (_allowShrink && (nbytes_total < _nalloc / 2)) {
    
    size_t new_alloc = _nalloc / 2;
    char *save = _buf;
    _buf = new char[new_alloc];
    memcpy(_buf, save, new_alloc);
    delete[] save;
    _nalloc = new_alloc;
    if (_len > _nalloc) {
      _len = _nalloc;
    }
    
  }
  
  return _buf;

}

///////////////////////////////////////////////////////////////
// Free up allocated space.

void RadxBuf::clear()

{

  if (_buf != NULL) {
    delete[] _buf;
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

void *RadxBuf::grow(const size_t nbytes_needed)

{
  size_t nbytes_total = _len + nbytes_needed;
  alloc(nbytes_total);
  return _buf;
}

///////////////////////////////////////////////////////////////
// Print - for internal debugging
// 

void RadxBuf::print(ostream &out)

{
  out << "_len: " << _len << endl;
  out << "_nalloc: " << _nalloc << endl;
  out << "_buf: " << (void *)_buf << endl;	// (char *) doesn't print
}

