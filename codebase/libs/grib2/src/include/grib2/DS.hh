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
 * @file DS.hh
 * @brief Section 7, Data Section
 * @author Jason Craig
 * @author Gary Blackburn
 * @date   Aug 2006
 */

#ifndef _GRIB2_DS_HH
#define _GRIB2_DS_HH

#include <grib2/constants.h>
#include <grib2/GribSection.hh>
#include <grib2/Grib2Record.hh>
#include <grib2/DataTemp.hh>
#include <dataport/port_types.h>
#include <grib2/Template7.41.hh>
#include <grib2/Template7.4000.hh>
#include <grib2/Template7.0.hh>
#include <grib2/Template7.2.hh>

using namespace std;

namespace Grib2 {
/** 
 * @class DS
 * @brief Section 7, Data Section
 *
 * Data Section (DS) is section 7 in Grib2.
 *
 * See http://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_sect7.shtml
 */
class DS : public GribSection
{

public:

  /** 
   * @brief Internal constructor used during grib2 encoding/decoding
   *
   * @param[in] sectionsPtr Struct containing pointers to all
   * sections that come before this one.
   */
  DS (Grib2Record::Grib2Sections_t sectionsPtr);

  ~DS();
  
  /** @brief Unpack the Data Section
   *  @param[in] dsPtr Pointer to start of section
   *  @return Either GRIB_SUCCESS or GRIB_FAILURE */
  int unpack( ui08 *dsPtr );

  /** @brief Encodes a data set and stores it internally
   *  @param[in] dataPtr Pointer to data set to encode
   *  @return Either GRIB_SUCCESS or GRIB_FAILURE */
  int encode( fl32 *dataPtr );

  /** @brief Pack up the Data Section
   *  @param[in] dsPtr Pointer to start of location to pack to
   *  @return Either GRIB_SUCCESS or GRIB_FAILURE */
  int pack( ui08 *dsPtr );

  /** @brief Print to stream/file all information in the data section */
  void print(FILE *) const;

  /** @brief Returns the unpacked-unbitmaped data 
   * This operation will unpack the data.  Use freeData() to reclaim memory. */
  fl32 *getData();
  
  /** @brief Deletes internal data object, should be used after pack() or getData() */
  void freeData();

  /** @brief Get a single value from a packed bit string. 
   *  @param[in] in Pointer to character input
   *  @param[out] iout Pointer to output
   *  @param[in] iskip Initial number of bits to skip
   *  @param[in] bitsPerVal Number of bits to take */
  static inline void gbit  (ui08 *in, si32 *iout, si32 iskip, si32 bitsPerVal)
  { gbits(in, iout, iskip, bitsPerVal, 0, 1); }

  /** @brief Extract arbitrary size values from a packed bit string,
   *    right justifying each value in the unpacked iout array.
   *  @param[in] in Pointer to character array input
   *  @param[out] iout Pointer to unpacked array output
   *  @param[in] iskip Initial number of bits to skip
   *  @param[in] bitsPerVal Number of bits to take
   *  @param[in] nskip Additional number of bits to skip on each iteration
   *  @param[in] n Number of iterations */
  static void gbits (ui08 *in, si32 *iout, si32 iskip, si32 bitsPerVal, si32 nskip, si32 n);

  /** @brief Put a single value into a packed bit string 
   *  @param[out] out Pointer to output location
   *  @param[in] in Pointer to input
   *  @param[in] iskip Initial number of bits to skip
   *  @param[in] bitsPerVal Number of bits to take */
  static inline void sbit  (ui08 *out, si32 *in, si32 iskip, si32 bitsPerVal)
  { sbits(out, in, iskip, bitsPerVal, 0, 1); }

  /** @brief Put arbitrary size values into a packed bit string,
   *    taking the low order bits from each value in the unpacked array.
   *  @param[out] out Pointer to packed array output
   *  @param[in] in Pointer to unpacked array input
   *  @param[in] iskip Initial number of bits to skip
   *  @param[in] bitsPerVal Number of bits to take
   *  @param[in] nskip Additional number of bits to skip on each iteration
   *  @param[in] n Number of iterations */
  static void sbits (ui08 *out, si32 *in, si32 iskip, si32 bitsPerVal, si32 nskip, si32 n);
  
  
private:

  /** @brief Struct containing pointers to all other sections of the Grib2 file. 
   * Only sections appearing before the DS however will have valid pointers */
  Grib2Record::Grib2Sections_t _sectionsPtr;

  /** @brief Template number of DataTemp */
  si32 _drsTemplateNum;

  /** @brief The unpacked unbitmaped data */
  // fl32 *_data;

  /** @brief The DataTemp stores additional infromation needed for the encoding type */
  DataTemp *_dataTemp;

  typedef enum {
    /** Data has been decoded, ready for reding */
    UNPACK,
    /** Data has been encoded, ready for packing */
    ENCODE,
    /** Data has been packed */
    PACK,
    /** Data has been read but not decoded */
    READ,
    /** No data */
    NONE
  } data_status_t;

  /** @brief Internal flag that keeps track of what stage the the data is currently in */
  data_status_t _data_status;

  /** @brief Pointer to read data before being decoded */
  ui08 *_readDataPtr;

};

} // namespace Grib2

#endif
