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
// MemBuf.hh
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

#ifndef MemBuf_HH
#define MemBuf_HH


#include <iostream> 
using namespace std;

///////////////////////////////////////////////////////////////////
// NOTES ON USE:
//
// The following functions return the pointer to the user
// buffer:
//
// alloc, grow, load, add, concat
//
// When using these, you must set your local buffer pointer
// to the return value. This allows for the fact that the
// buffer position may change during a realloc.
//
// getBufPtr() may also be used at any time to get a pointer to the
// user buffer.
// 

class MemBuf {

public:

  // default constructor

  MemBuf();

  // copy constructor

  MemBuf(const MemBuf &other);

  // destructor

  virtual ~MemBuf();

  //__________________________________________________________
  //
  // Reset the memory buffer - sets current length to 0
  // Zero's out allocated buffer memory.
  //__________________________________________________________

  void reset();

  //__________________________________________________________
  //
  // If allowShrink is false, shrinking of buffers will not be 
  // performed
  //__________________________________________________________

  void setAllowShrink(bool allowshrink = true);

  //__________________________________________________________
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
  //__________________________________________________________

  void *prepare(const size_t numbytes);
  void *reserve(const size_t numbytes);

  //__________________________________________________________
  // 
  // Load numbytes from source array into start of target buffer.
  //
  // Buffer is resized as necessary.
  //
  // Returns pointer to user buffer.
  //__________________________________________________________

  void *load(void const *source, const size_t numbytes);

  //__________________________________________________________
  //
  // Add numbytes from source array onto end of buffer.
  //
  // Buffer is resized as necessary.
  //
  // Returns pointer to user buffer.
  //__________________________________________________________

  void *add(void const *source, const size_t numbytes);

  //__________________________________________________________
  //
  // Concat the contents of another MemBuf onto this one.
  //
  // Returns pointer to user buffer.
  //__________________________________________________________
  
  void *concat(const MemBuf &other);

  //__________________________________________________________
  // 
  // assignment
  //
  //__________________________________________________________

  void operator=(const MemBuf &other);

  //__________________________________________________________
  //
  // Check available space, grow or shrink as needed
  // Returns pointer to user buffer.
  //__________________________________________________________

  void *alloc(const size_t nbytes_total);

  //__________________________________________________________
  //
  // Free up allocated space.
  //__________________________________________________________

  void free();
  inline void clear() { free(); }

  //__________________________________________________________
  //
  // Check available space, grow if needed
  // Returns pointer to user buffer.
  //__________________________________________________________
  
  void *grow(const size_t nbytes_needed);

  //__________________________________________________________
  //
  // Print out for internal debugging
  //__________________________________________________________
  
  void print(ostream &out);

  //__________________________________________________________
  // 
  // Get the currently-used length of the buffer in bytes
  //__________________________________________________________
  
  size_t getLen() const { return (_len); }
  size_t getBufLen() const { return (_len); }

  //__________________________________________________________
  // 
  // Get a pointer to the start of the usable buffer
  //__________________________________________________________
  
  void *getPtr() const { return (_buf); }
  void *getBufPtr() const { return (_buf); }
  
protected:

  char *_buf;          // pointer to allocated buffer
  size_t _len;            // number of bytes currently used
  size_t _nalloc;          // allocated size of buffer
  bool _allowShrink;

private:

};
  
#endif 
