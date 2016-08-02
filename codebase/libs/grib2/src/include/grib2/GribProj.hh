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
 * @file GribProj.hh
 * @author Jason Craig
 * @author Gary Blackburn
 * @date   Aug 2006
 */

#ifndef _GRIB2_GRIB_PROJECTION
#define _GRIB2_GRIB_PROJECTION

#include <dataport/port_types.h>
#include <grib2/GribSection.hh>

using namespace std;

namespace Grib2 {

/** 
 * @class GribProj
 *
 * GribProj is an abstract class for grib Grid Definition Templates.
 * As an abstract class no object can be declared as type GribProj, it
 * can only be used with as a reference or pointer.
 *
 * Known derived classes include:  LatLonProj.hh, PolarStereoProj.hh, MercatorProj.hh and LambertConfProj.hh
 *
 * The GribProj /Grid Definition Template is part of the Grid Definition Section (GDS).
 *
 * @note The pack and unpack routines are static methods in GribSection.hh
 *
 */
class GribProj {

public:
  /** 
   * @brief Defualt Constructor usually called only from derived classes.
   */
  GribProj();

  virtual ~GribProj();

  /** @brief Unpack a GribProj template
   *  @param[in] projPtr Pointer to start of template
   *  @return Either GRIB_SUCCESS or GRIB_FAILURE */
  virtual int unpack( ui08 *projPtr ) = 0;

  /** @brief Pack up this GribProj template
   *  @param[in] projPtr Pointer to start of location to pack template
   *  @return Either GRIB_SUCCESS or GRIB_FAILURE */
  virtual int pack( ui08 *projPtr ) = 0;

  /** @brief Print to stream/file all information for this template */
  virtual void print (FILE *) const = 0;

  /** @brief Get the width of data in this projection */
  virtual si32 getWidth() = 0;

  /** @brief Get the height of data in this projection */
  virtual si32 getHeight() = 0;

  /** @brief Get the scanning mode of data */
  virtual si32 getIscan() = 0;

  /** @brief Get the packed data template size */
  virtual si32 getTemplateSize() = 0;

protected:


private: 

};

} // namespace Grib2

#endif


