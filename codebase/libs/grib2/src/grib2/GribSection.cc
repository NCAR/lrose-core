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
///////////////////////////////////////////////////
// GribSection
//
// Used wgrib by Wesley Ebisuzaki at NOAA as
// reference (http://wesley.wwb.noaa.gov/wgrib.html)
//
// $Id: GribSection.cc,v 1.10 2016/03/03 18:38:02 dixon Exp $
//
///////////////////////////////////////////////////
#include <math.h>
#include <grib2/GribSection.hh>

using namespace std;

namespace Grib2 {

//
// Constants
//     used to unpack the reference value.  It converts
//     a 32 bit number unpacked from 4 bytes into
//     an ieee 32 bit floating point number

const fl64 GribSection::TWO_POWER_MINUS_23   = 0.00000011920928955078125;
const fl64 GribSection::TWO_POWER_MINUS_126  
                   = 0.0000000000000000000000000000000000000117549435082228750796873654;
const fl64 GribSection::TWO_POWER_23   = 8388608;
const fl64 GribSection::TWO_POWER_126  = 8.507059173+37;  //?

const ui32 GribSection::MASK_ONE       = 0x80000000;  // 10000000000000000000000000000000 binary
const si32 GribSection::MASK_TWO       = 0x7F800000;  // 01111111100000000000000000000000 binary
const si32 GribSection::MASK_THREE     = 0x007FFFFF;  // 00000000011111111111111111111111 binary

const ui32 GribSection::U4MISSING = 0xFFFFFFFF;  // 11111111111111111111111111111111 binary
const si32 GribSection::S4MISSING = 0xFFFFFFFF;  // 11111111111111111111111111111111 binary

GribSection::GribSection() 
{
  _sectionLen     = 0;
  _sectionNum     = 0;
}

GribSection::~GribSection() 
{

}


fl32 GribSection::rdIeee (si32 ieee)
// SUBPROGRAM:    rdieee 
//   PRGMMR: Gilbert         ORG: W/NP11    DATE: 2000-05-09
//
// ABSTRACT: This subroutine reads a real value in 
//   32-bit IEEE floating point format.
//
{

  fl32 val = 0.0;
  fl32 temp;
  si32 sign_bit = 0;
  fl32 sign = 0.0;
  si32 exponent = 0;
  si32 mantissa = 0;


  // Extract sign bit, exponent, and mantissa

  sign_bit = (ieee & MASK_ONE) >> 31;
  exponent = (ieee & MASK_TWO) >> 23;
  mantissa = (ieee & MASK_THREE);

  if (sign_bit == 1) 
     sign = -1.0;
  else
     sign = 1.0;

  if (exponent == 255)
       val = sign*(1E+37);

  if (exponent == 0) {
      if (mantissa != 0)
        val = sign *
           TWO_POWER_MINUS_23 * TWO_POWER_MINUS_126 * (float) mantissa;
      else
        val = 0.0;
  }

  if ((exponent > 0) && (exponent < 255)) {
    temp = pow(2.0, exponent - 127);   
    val = sign*temp*(1.0+(TWO_POWER_MINUS_23 * (float) mantissa));
  }

  return (val);

}

si32 GribSection::mkIeee (fl32 a)
// SUBPROGRAM:    mkieee 
//   PRGMMR: Gilbert         ORG: W/NP11    DATE: 2000-05-09
//
// ABSTRACT: This subroutine stores a real value in 
//   32-bit IEEE floating point format.
//
{
  si32 sign = 0;
  si32 exponent = 0;
  si32 mantissa = 0;
  fl32 alog2 = log(2.0);
  fl32 atemp;

  if (a == 0) {
    return(0);
  }
        
  //
  //  Set Sign bit (bit 31 - leftmost bit)
  //
  if (a < 0.0) {
    sign = MASK_ONE;
    atemp = a >= 0 ? a : -a;
  } else {
    sign = 0;
    atemp = a;
  }
  //
  //  Determine exponent n with base 2
  //
  int n = (int)floor(log(atemp)/alog2);
  exponent = n + 127;
  if (n > 127) exponent = 255;     // overflow
  if (n < -127) exponent = 0;
  //  set exponent bits ( bits 30-23 )
  exponent = exponent << 23;
  //
  //  Determine Mantissa
  // 
  if (exponent != 255) {
    if (exponent != 0) {
      atemp = (atemp/pow(2.0,n))-1.0;
    } else {
      atemp = atemp*TWO_POWER_126;
    }
    mantissa = int((atemp*TWO_POWER_23) + .5);
  } else {
    mantissa = 0;
  }
  return(sign | exponent | mantissa);
}

int GribSection::_upkUnsigned2( ui08 a, ui08 b ) 
{ return ((int) ( (a << 8) + b) ); }

int GribSection::_upkSigned2( ui08 a, ui08 b )
{ return ( (a & 128 ? -1 : 1) *
	   (int) (((a & 127) << 8) + b) ); }

int GribSection::_upkSigned3( ui08 a, ui08 b, ui08 c ) 
{ return( (a & 128 ? -1 : 1) * 
	  (int) (((a & 127) << 16) + (b << 8) + c) ); }

int GribSection::_upkSigned4( ui08 a, ui08 b, ui08 c, ui08 d )
{ return ( (a & 128 ? -1 : 1) * (ui32) ( ((a & 127) << 24) + (b << 16) + (c << 8) + d )); }

int GribSection::_upkUnsigned4( ui08 a, ui08 b, ui08 c, ui08 d )
{ return ((ui32) ( (a << 24) + (b << 16) + (c << 8) + d )); }

ui64 GribSection::_upkUnsigned5( ui08 a, ui08 b, ui08 c, ui08 d, ui08 e )
{ 
  ui64 aa;
  
  aa = (ui64) a;
  return ((ui64) ( (aa << 32) +  (b << 24) + (c << 16) + (d << 8) + e )); 
}

void GribSection::_pkSigned2( const int value, ui08 *buffer ) 
{
  // Clear out the original byte values
  buffer[0] = 0;
  buffer[1] = 0;
  
  int k = (value >= 0) ? value : (-value) | (1U << 15);
  buffer[0] = (k >>  8) & 255;
  buffer[1] = (k      ) & 255;
  
}

void GribSection::_pkUnsigned2( const int value, ui08 *buffer ) 
{
  // Clear out the original byte values
  buffer[0] = 0;
  buffer[1] = 0;
  
  buffer[0] = (value >>  8) & 255;
  buffer[1] = (value      ) & 255;
}


void GribSection::_pkSigned3( const int value, ui08 *buffer ) 
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

int GribSection::_upkUnsigned3( ui08 a, ui08 b, ui08 c )
{ return ((int) ( (a << 16) + (b << 8) + c )); }

void GribSection::_pkUnsigned3( const int value, ui08 *buffer ) 
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

void GribSection::_pkSigned4( const int value, ui08 *buffer ) 
{
  // Clear out the original byte values
  buffer[0] = 0;
  buffer[1] = 0;
  buffer[2] = 0;
  buffer[3] = 0;
  
  int k = value >= 0 ? value : (-value) | (1U << 31);
  buffer[0] = (k >> 24) & 255;
  buffer[1] = (k >> 16) & 255;
  buffer[2] = (k >>  8) & 255;
  buffer[3] = (k      ) & 255;
}


void GribSection::_pkUnsigned4( const int value, ui08 *buffer ) 
{
  // Clear out the original byte values
  buffer[0] = 0;
  buffer[1] = 0;
  buffer[2] = 0;
  buffer[3] = 0;
  
  buffer[0] = (value >> 24) & 255;
  buffer[1] = (value >> 16) & 255;
  buffer[2] = (value >>  8) & 255;
  buffer[3] = (value      ) & 255;
}

void GribSection::_pkUnsigned5( const ui64 value, ui08 *buffer ) 
{
  // Clear out the original byte values
  buffer[0] = 0;
  buffer[1] = 0;
  buffer[2] = 0;
  buffer[3] = 0;
  buffer[4] = 0;
  
  buffer[0] = (value >> 32) & 255;
  buffer[1] = (value >> 24) & 255;
  buffer[2] = (value >> 16) & 255;
  buffer[3] = (value >>  8) & 255;
  buffer[4] = (value      ) & 255;
}

ui64 GribSection::_upkUnsigned8( ui08 a, ui08 b, ui08 c, ui08 d, ui08 e, ui08 f, ui08 g, ui08 h)
{ 
  ui64 aa, bb, cc, dd;
  //, bb, cc, dd;
  
  aa =  a;
  bb =  b;
  cc = c;
  dd = d;
  
  return ((ui64) ( (aa << 56) + (bb << 48) + (cc << 40) + (dd << 32) + (e << 24) + (f << 16) + (g << 8) + h )); 
}

ui64 GribSection::_upkUnsigned6( ui08 a, ui08 b, ui08 c, ui08 d, ui08 e, ui08 f )
{ 
  ui64 aa, bb;
  //, bb, cc, dd;
  //bb = b;
  //cc = c;
  //dd = d;
  
  aa =  a;
  bb =  b;
  
  return ((ui64) ( (aa << 32) +  (bb << 24) + (c << 16) + (d << 8) + e )); 
}

} // namespace

