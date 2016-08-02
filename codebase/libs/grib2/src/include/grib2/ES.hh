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
 * @file ES.hh
 * @brief Section 8, End Section
 * @author Jason Craig
 * @author Gary Blackburn
 * @date   Aug 2006
 */

#ifndef _GRIB2_ES_HH
#define _GRIB2_ES_HH

#include <grib2/GribSection.hh>
#include <grib2/constants.h>

using namespace std;

namespace Grib2 {
/** 
 * @class ES
 * @brief Section 8, End Section
 *
 * End Section (ES) is section 8 in Grib2.
 *
 * See http://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_sect8.shtml
 */
class ES : public GribSection
{

public:
  /**
   * @brief Default and only constructor
   */
  ES();

  ~ES();
  
  /** @brief Unpacks the End Section
   *  @param[in] esPtr Pointer to start of the ES section
   *  @return Either Grib2::GRIB_SUCCESS or Grib2::GRIB_FAILURE */
  int unpack( ui08 *esPtr );

  /** @brief Packs up the End Section
   *  @param[in] esPtr Pointer to start of location to pack to
   *  @return Either Grib2::GRIB_SUCCESS or Grib2::GRIB_FAILURE */
  int pack( ui08 *esPtr );
  
  /** @brief Print to stream/file all information contained in the ES section */
  void print(FILE *) const;
  
private:
  
};

} // namespace Grib2

#endif
