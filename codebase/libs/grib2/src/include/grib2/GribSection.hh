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
/**
 * @file GribSection.hh
 * @author Jason Craig
 * @date   Aug 2006
 */

#ifndef _GRIB2_GRIB_SECTION
#define _GRIB2_GRIB_SECTION

#include <string>
#include <cstdio>
#include <dataport/port_types.h>

using namespace std;

namespace Grib2 {
/** 
 * @class GribSection
 *
 * GribSection is an abstract base class for all sections of the Grib2 file.
 *
 * The sections of a Grib2 file are: IndicatorSec, IdSec, LocalUseSec, GDS,
 * PDS, DRS,  BMS, DS, and ES
 *
 * This class contains Pack and Unpack functions which are used by all sections.
 *
 */
class GribSection {

public:

  /** @brief As an abstract base class this constructor should only be called from subclasses */
  GribSection();

  virtual ~GribSection();

  /** @brief Returns the size of the section
   *  @return Size of packed section in bytes */
  inline int getSize() { return( _sectionLen); }

  // IEEE Conversion Routines

  /** @brief Read an ieee 32-bit floating point number */
  static fl32 rdIeee (si32 ieee);

  /** @brief Create an ieee 32-bit floating point value */
  static si32 mkIeee (fl32 a);

  // UnPack Routines

  /** @brief Unpack two bytes into a unsigned value. 
   * Store it as an integer. */
  static int _upkUnsigned2( ui08 a, ui08 b );

  /** @brief Unpack two bytes into a signed value.
   * Store it as an integer. */
  static int _upkSigned2( ui08 a, ui08 b );

  /** @brief Unpack three bytes into a unsigned value. 
   * Store it as an integer. */
  static int _upkUnsigned3( ui08 a, ui08 b, ui08 c );
 
  /** @brief Unpack three bytes into a signed value.
   * Store it as an integer. */
  static int _upkSigned3( ui08 a, ui08 b, ui08 c );

  /** Unpack four bytes into an unsigned value.
   * Store it as an integer. */
  static int _upkUnsigned4( ui08 a, ui08 b, ui08 c, ui08 d );

  /** @brief Unpack four bytes into an signed value.
   * Store it as an integer. */
  static int _upkSigned4( ui08 a, ui08 b, ui08 c, ui08 d );

  /** @brief Unpack five bytes into an unsigned 64 bit value.
   * Store it as an ui64. */
  static ui64 _upkUnsigned5( ui08 a, ui08 b, ui08 c, ui08 d, ui08 e );

  /** @brief Unpack six bytes into an unsigned 64 bit value.
   * Store it as an ui64. */
  static ui64 _upkUnsigned6( ui08 a, ui08 b, ui08 c, ui08 d, ui08 e, ui08 f );

  /** @brief Unpack eight bytes into an unsigned 64 bit value.
   * Store it as an ui64. */
  static ui64 _upkUnsigned8( ui08 a, ui08 b, ui08 c, ui08 d, ui08 e, ui08 f, ui08 g, ui08 h);

  // Pack Routines

  /** Pack a signed integer into two bytes.
   * @param[in] value Integer to pack.
   * @param[out] buffer Pointer to the space where the integer should be written. */
  static void _pkSigned2( const int value, ui08 *buffer );

  /** Pack a unsigned integer into two bytes.
   * @param[in] value Integer to pack.
   * @param[out] buffer Pointer to the space where the integer should be written. */
  static void _pkUnsigned2( const int value, ui08 *buffer );

  /** Pack a signed integer into three bytes.
   * @param[in] value Integer to pack.
   * @param[out] buffer Pointer to the space where the integer should be written. */
  static void _pkSigned3( const int value, ui08 *buffer );

  /** Pack a unsigned integer into three bytes.
   * @param[in] value Integer to pack.
   * @param[out] buffer Pointer to the space where the integer should be written. */
  static void _pkUnsigned3( const int value, ui08 *buffer );

  /** Pack a signed integer into four bytes.
   * @param[in] value Integer to pack.
   * @param[out] buffer Pointer to the space where the integer should be written. */
  static void _pkSigned4( const int value, ui08 *buffer );

  /** Pack a unsigned integer into four bytes.
   * @param[in] value Integer to pack.
   * @param[out] buffer Pointer to the space where the integer should be written. */
  static void _pkUnsigned4( const int value, ui08 *buffer );

  /** Pack a unsigned integer into five bytes.
   * @param[in] value Integer to pack.
   * @param[out] buffer Pointer to the space where the integer should be written. */
  static void _pkUnsigned5( const ui64 value, ui08 *buffer );

  /** @brief 2^-23 as a float */
  static const fl64 TWO_POWER_MINUS_23;
  /** @brief 2^-126 as a float */
  static const fl64 TWO_POWER_MINUS_126;
  /** @brief 2^23 as a float */
  static const fl64 TWO_POWER_23;
  /** @brief 2^126 as a float */
  static const fl64 TWO_POWER_126;
  /** @brief Mask of the Sign bit in a IEEE value */
  static const ui32 MASK_ONE;
  /** @brief Mask of the exponent bits in a IEEE value */
  static const si32 MASK_TWO;
  /** @brief Mask of the mantissa bits in a IEEE value */
  static const si32 MASK_THREE;
  /** @brief Unsigned 4 byte missing value (all bits 1's) */
  static const ui32 U4MISSING;
  /** @brief Signed 4 byte missing value (all bits 1's) */
  static const si32 S4MISSING;

protected:

  friend class Template7_pt_2;

  /** @brief This sections packed length */
  si32 _sectionLen;
  /** @brief This sections Grib2 section number */
  si32 _sectionNum;

  /** @brief Set the section length of this section
   *  @param[in] size  Size of section in bytes */
  inline void setSize(int size) { _sectionLen = size; }

private: 

};

} // namespace Grib2

#endif

