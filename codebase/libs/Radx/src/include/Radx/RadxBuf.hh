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
// RadxBuf.hh
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

#ifndef RadxBuf_HH
#define RadxBuf_HH

#include <iostream> 
using namespace std;

///////////////////////////////////////////////////////////////////
/// MEMORY BUFFER CLASS
///
/// The class provides memory management for a buffer which can grow,
/// be concatenated, or shrink, and will delete allocated memory when
/// it goes out of scope.
///
/// NOTES ON USE:
///
/// The following functions return the pointer to the user
/// buffer:
///
/// alloc, grow, load, add, concat
///
/// When using these, you must set your local buffer pointer
/// to the return value. This allows for the fact that the
/// buffer position may change during a realloc.
///
/// getBufPtr() may also be used at any time to get a pointer to the
/// user buffer.

class RadxBuf {

public:

  /// Default constructor
  
  RadxBuf();

  /// Copy constructor

  RadxBuf(const RadxBuf &other);

  /// Destructor

  virtual ~RadxBuf();
 
  ////////////////////////////////////////////////////////////
  /// Reset the memory buffer - sets current length to 0.
  /// Zero's out allocated buffer memory.

  void reset();

  //////////////////////////////////////////////////////////////
  /// Set allowShrink flag.
  ///
  /// If allowShrink is false, shrinking of buffers will not be 
  /// performed

  void setAllowShrink(bool allowshrink = true);

  ////////////////////////////////////////////////////////////
  /// Reserve a buffer by allocating or reallocating a starting size.
  ///
  /// This is done if you want to read data directly into the buffer.
  /// Note that this routine sets things up so that the internal
  /// buffer looks like the data has already been added, although that
  /// is left up to the calling routine (i.e. the buffer length is
  /// numbytes after the call to this routine).
  ///
  /// This routine does not change the existing parts of the buffer,
  /// only adjusts the size.
  ///
  /// Buffer is resized as necessary.
  ///
  /// Returns pointer to user buffer.

  void *reserve(const size_t numbytes);

  ////////////////////////////////////////////////////////////
  /// Load numbytes from source array into start of target buffer.
  ///
  /// Buffer is resized as necessary.
  ///
  /// Returns pointer to user buffer.

  void *load(const void *source, const size_t numbytes);

  ////////////////////////////////////////////////////////////
  /// Add numbytes from source array onto end of buffer.
  ///
  /// Buffer is resized as necessary.
  ///
  /// Returns pointer to user buffer.

  void *add(const void *source, const size_t numbytes);

  ////////////////////////////////////////////////////////////
  /// Concat the contents of another RadxBuf onto this one.
  ///
  /// Returns pointer to user buffer.
  
  void *concat(const RadxBuf &other);

  ////////////////////////////////////////////////////////////
  /// Assignment.

  void operator=(const RadxBuf &other);
  
  ////////////////////////////////////////////////////////////
  /// Check available space, grow or shrink as needed.
  ///
  /// Returns pointer to user buffer.

  void *alloc(const size_t nbytes_total);

  ////////////////////////////////////////////////////////////
  /// Free up allocated space.

  void clear();

  ////////////////////////////////////////////////////////////
  /// Check available space, grow if needed.
  ///
  /// Returns pointer to user buffer.
  
  void *grow(const size_t nbytes_needed);

  ////////////////////////////////////////////////////////////
  /// Print for debugging.
  
  void print(ostream &out);

  ////////////////////////////////////////////////////////////
  /// Get the currently-used length of the buffer in bytes.
  
  size_t getLen() const { return (_len); }

  ////////////////////////////////////////////////////////////
  /// Get a pointer to the start of the usable buffer.
  
  void *getPtr() const { return (_buf); }
  
protected:
private:

  char *_buf; // pointer to allocated buffer
  size_t _len; // number of bytes currently used
  size_t _nalloc; // allocated size of buffer
  bool _allowShrink;


};
  
#endif 
