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
 * @file BMS.hh
 * @brief Section 6, Bit Map Section
 * @author Jason Craig
 * @author Gary Blackburn
 * @date   Aug 2006
 */

#ifndef _GRIB2_BMS_
#define _GRIB2_BMS_

#include <grib2/constants.h>
#include <grib2/GribSection.hh>

using namespace std;

namespace Grib2 {
/** 
 * @class BMS
 * @brief Section 6, Bit Map Section
 *
 * Bit Map Section (BMS) is section 6 in Grib2.
 *
 * See http://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_sect6.shtml
 */
class BMS: public GribSection {

public:

  /** @brief Bit Map Constructor
   *  @param[in] bitMapType Indicates type of bitmap, 
   *    0     = BitMap present and defined
   *    1-254 = Predefined BitMaps
   *    255   = No BitMap
   *  @param[in] grid_size Size of the data / BitMap
   *  @param[in] bit_map Pointer to the BitMap, NULL if bitMapType != 0 */
  BMS(si32 bitMapType = 255, si32 grid_size = 0, si32 *bit_map = NULL);

  ~BMS();
   
  /** @brief Unpacks the Bit Map
   *  @param[in] bmsPtr Pointer to start of the BMS section
   *  @return Either Grib2::GRIB_SUCCESS or Grib2::GRIB_FAILURE */
  int unpack( ui08 *bmsPtr );

  /** @brief Packs up the Bit Map
   *  @param[in] bmsPtr Pointer to start of location to pack to
   *  @return Either Grib2::GRIB_SUCCESS or Grib2::GRIB_FAILURE */
  int pack( ui08 *bmsPtr );
  
  /** @brief Get pointer to the bit map */
  inline si32* getBitMap() { return _bitMap; };

  /** @brief Get the bit map indicator 
   *     0     = BitMap present and defined
   *     1-254 = Predefined BitMaps
   *     255   = No BitMap */
  
  inline si32 getBitMapIndicator() { return _bitMapIndicator; };
  
  /** @brief Get the size of the bit map */
  inline si32 getBitMapSize() { return _sectionLen - 6; };
  
  /** @brief Print to stream/file all information contained in the BMS section */
  void print(FILE *, const bool print_bitmap = false) const;

private:

  /** @brief Pre-defined bitmap / bitmap present / no bitmap */
  si32 _bitMapIndicator;

  /** @brief Size of the bitmap */
  si32 _gridSz;

  /** @brief Pointer to the bitmap */
  si32 *_bitMap;
   
};

} // namespace Grib2

#endif
