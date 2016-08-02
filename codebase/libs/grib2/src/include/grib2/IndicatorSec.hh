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
 * @file IndicatorSec.hh
 * @brief Section 0, Indicator Section
 * @author Jason Craig
 * @author Gary Blackburn
 * @date   Aug 2006
 */

#ifndef _GRIB2_INDICATOR_SECTION
#define _GRIB2_INDICATOR_SECTION

#include <grib2/GribSection.hh>
#include <grib2/constants.h>

using namespace std;

namespace Grib2 {
/** 
 * @class IndicatorSec
 * @brief Section 0, Indicator Section
 *
 * Indicator Section is section 0 in Grib2.
 *
 * See http://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_sect0.shtml
 */
class IndicatorSec : public GribSection {

public:
  /**
   * @brief Default and only constructor
   */
  IndicatorSec();

  ~IndicatorSec(){};

  /** @brief Unpacks the Indicator Section
   *  @param[in] idPtr Pointer to start of the section
   *  @return Either Grib2::GRIB_SUCCESS or Grib2::GRIB_FAILURE */
  int unpack( ui08 *idPtr );

  /** @brief Packs up the Indicator Section
   *  @param[in] idPtr Pointer to start of location to pack to
   *  @return Either Grib2::GRIB_SUCCESS or Grib2::GRIB_FAILURE */
  int pack( ui08 *idPtr );

  /** @brief Print to stream/file all information contained in the Indicator Section */
  void print(FILE *) const;  
  
  // Get Functions

  /** @brief Get the total size of the packed grib message */
  inline ui64 getTotalSize() { return( _numMsgBytes ); }

  /**  @brief Get the grib edition number (Should be number 2) */
  inline si32 getEditionNum() { return( _editionNum ); }

  /** @brief Get the discipline number of the processed data.
   *
   *  See http://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_table0-0.shtml */
  inline si32 getDisciplineNum() { return( _disciplineNum ); }

  // Set Functions

  /** @brief Set the total size of this grib message */
  inline void setTotalSize (const ui64 new_size) { _numMsgBytes = new_size; }

  /** @brief Set the grib edition number for this message */
  inline void setEditionNum (const si32 edition_num) { _editionNum = edition_num; }

  /** @brief Set the discipline number of the processed data.
   *
   *  See http://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_table0-0.shtml */
  inline void setDisciplineNum (const si32 discipline) { _disciplineNum = discipline; }
  
private:
  /** @brief Total length of GRIB message in octets (including Section 0) */
  ui64 _numMsgBytes;

  /** @brief GRIB Edition Number (2) */
  si32 _editionNum;

  /** @brief Discipline - GRIB Master Table Number
   *
   * 0  = Meteorological Products
   * 1  = Hydrological Products
   * 2  = Land Surface Products
   * 3  = Space Products
   * 10 = Oceanographic Products  */
  si32 _disciplineNum;

};

} // namespace Grib2

#endif
