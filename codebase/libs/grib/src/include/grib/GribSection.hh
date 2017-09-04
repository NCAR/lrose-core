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
////////////////////////////////////////////////
// GribSection
//
// Used wgrib by Wesley Ebisuzaki at NOAA as
// reference (http://wesley.wwb.noaa.gov/wgrib.html)
//
///////////////////////////////////////////////
#ifndef _GRIB_SECTION
#define _GRIB_SECTION

#include <string>
#include <cstdio>
#include <ostream>
#include <dataport/port_types.h>
using namespace std;

class GribSection {

public:

  GribSection();
  virtual ~GribSection();

  inline int getSize() { return( _nBytes); }

  // Unpack two bytes into an unsigned value.
  // Store it as an integer.
  static inline int _upkUnsigned2( ui08 a, ui08 b ) 
  { return ((int) ( (a << 8) + b )); }

  // Unpack three bytes into an unsigned value.
  // Store it as an integer.
  static inline int _upkUnsigned3( ui08 a, ui08 b, ui08 c )
  { return ((int) ( (a << 16) + (b << 8) + c )); }

protected:

  int _nBytes;
  int _nReservedBytes;
  ui08 *_reserved;

  //
  // Unpack two bytes into a signed value. 
  // Store it as an integer.
  //
  inline int _upkSigned2( ui08 a, ui08 b ) 
  { return ( (a & 128 ? -1 : 1) * 
	     (int) (((a & 127) << 8) + b) ); }

  //
  // Pack a signed integer into two bytes.
  // The passed pointer points to the space where the
  // integer should be written.
  //
  inline void _pkSigned2( const int value, ui08 *buffer ) 
  {
    // Clear out the original byte values
    buffer[0] = 0;
    buffer[1] = 0;
  
#ifdef NOTNOW 

    // Update each byte
    if (value < 0)
      buffer[0] = 128;
    int value_abs = abs(value);
    
    buffer[0] = buffer[0] & (value_abs >> 8);
    buffer[1] = value_abs & 255;

    if (value < 0)
      buffer[0] |= 128;
#endif

    int k = (value >= 0) ? value : (-value) | (1U << 15);
    buffer[0] = (k >>  8) & 255;
    buffer[1] = (k      ) & 255;

  }

  //
  //
  // Pack an unsigned integer into two bytes.
  // The passed pointer points to the space where the
  // integer should be written.
  //
  inline void _pkUnsigned2( const int value, ui08 *buffer ) 
  {
    // Clear out the original byte values
    buffer[0] = 0;
    buffer[1] = 0;
  
#ifdef NOTNOW 

    // Update each byte
    buffer[0] = value >> 8;
    buffer[1] = value & 255;
#endif
    buffer[0] = (value >>  8) & 255;
    buffer[1] = (value      ) & 255;
  }

  //
  // Unpack three bytes into a signed value.
  // Store it as an integer.
  //
  inline int _upkSigned3( ui08 a, ui08 b, ui08 c ) 
  { return( (a & 128 ? -1 : 1) * 
	    (int) (((a & 127) << 16) + (b << 8) + c) ); }

  //
  // Pack a signed integer into three bytes.
  // The passed pointer points to the space where the
  // integer should be written.
  //
  inline void _pkSigned3( const int value, ui08 *buffer ) 
  {
    // Clear out the original byte values
    buffer[0] = 0;
    buffer[1] = 0;
    buffer[2] = 0;
    
    int k = value >= 0 ? value : (-value) | (1U << 23);
    buffer[0] = (k >> 16) & 255;
    buffer[1] = (k >>  8) & 255;
    buffer[2] = (k      ) & 255;
    
  }

  //
  // Pack an unsigned integer into three bytes.
  // The passed pointer points to the space where the
  // integer should be written.
  //
  inline void _pkUnsigned3( const int value, ui08 *buffer ) 
  {
    // Clear out the original byte values
    buffer[0] = 0;
    buffer[1] = 0;
    buffer[2] = 0;
    
#ifdef NOTNOW 

    // Update each byte
    buffer[0] = value >> 16;
    buffer[1] = (value >> 8) & 255;
    buffer[2] = value & 255;
#endif
    buffer[0] =   (value >> 16) & 255;
    buffer[1] = (value >>  8) & 255;
    buffer[2] = (value      ) & 255;
  }

private: 

};

#endif

