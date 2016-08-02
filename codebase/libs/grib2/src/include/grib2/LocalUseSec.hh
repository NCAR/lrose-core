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
 * @file LocalUseSec.hh
 * @brief Section 2, Local Use Section 
 * @author Jason Craig
 * @author Gary Blackburn
 * @date   Aug 2006
 */

#ifndef _GRIB2_LOCALUSE_SECTION
#define _GRIB2_LOCALUSE_SECTION

#include <grib2/GribSection.hh>
#include <grib2/constants.h>

using namespace std;

namespace Grib2 {
/** 
 * @class LocalUseSec
 * @brief Section 2, Local Use Section 
 *
 * Local Use Section is section 2 in Grib2.
 *
 * See http://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_sect2.shtml
 */
class LocalUseSec : public GribSection {

public:
  /**
   * @brief Default constructor
   */
  LocalUseSec();

  /**
   * @brief Constructor for saving a block of data in the local use section
   * @param[in] dataSize Size of the local use data, in bytes
   * @param[in] localUseData Pointer to the local use data to save
   */
  LocalUseSec(si32 dataSize, ui08 *localUseData);

  ~LocalUseSec();
  
  /** @brief Unpacks the Local Use Section
   *  @param[in] idPtr Pointer to start of the section
   *  @return Either Grib2::GRIB_SUCCESS or Grib2::GRIB_FAILURE */
  int unpack( ui08 *idPtr );

  /** @brief Packs up the Local Use Section
   *  @param[in] idPtr Pointer to start of location to pack to
   *  @return Either Grib2::GRIB_SUCCESS or Grib2::GRIB_FAILURE */
  int pack( ui08 *idPtr );

  /** @brief Print to stream/file all information contained in the Local Use Section */
  void print(FILE *) const;

  /** @brief Get the size of the Local Use data block 
   *  @return Size of data block in bytes */    
  inline si32 getLocalUseLength() { return( _sectionLen - 5); };

  /** @brief Get a pointer to the local use data block */
  inline ui08 *getLocalUse() { return( _localUse ); }

  /** @brief Sets the block of data in the local use section
   *  @param[in] dataSize Size of the local use data, in bytes
   *  @param[in] data Pointer to the local use data to save
   */
  void setLocalUse (const si32 dataSize, ui08 *data);
  
private:

  /** @brief Pointer containing the local use data block if used */
  ui08 *_localUse;

};

} // namespace Grib2

#endif
