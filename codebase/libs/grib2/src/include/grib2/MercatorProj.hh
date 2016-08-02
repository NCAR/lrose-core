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
 * @file MercatorProj.hh
 * @author Jason Craig
 * @author Gary Blackburn
 * @date   Aug 2006
 */

#ifndef _GRIB2_MERCATOR_PROJ 
#define _GRIB2_MERCATOR_PROJ

#include <cstdio>
#include <dataport/port_types.h>
#include <grib2/GribProj.hh>
#include <grib2/constants.h>

using namespace std;

namespace Grib2 {

/** 
 * @class MercatorProj
 *
 * A derived GribProj class for Mercator Projections
 *
 * See http://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_temp3-10.shtml
 *
 * @note The pack and unpack routines are static methods in Grib2::GribSection
 */
class MercatorProj: public GribProj {

public:
  /**
   * @brief Default constructor for use in passing object to GribFile.addField()
   *
   * @note After creation.. all public values variables below should be set
   * before passing this object to GribFile. 
   */
  MercatorProj();

  virtual ~MercatorProj();

  /** @brief Unpack a Mercator Projection Template 
   *  @param[in] projPtr Pointer to start of template
   *  @return Either GRIB_SUCCESS or GRIB_FAILURE */
  virtual int unpack (ui08 *projPtr);

  /** @brief Pack up this Mercator Projection Template
   *  @param[in] projPtr Pointer to start of location to pack template
   *  @return Either GRIB_SUCCESS or GRIB_FAILURE */
  virtual int pack (ui08 *projPtr);

  /** @brief Print to stream/file all information for this template */
  virtual void print (FILE *) const;

  /** @brief Get the width of data in this projection */
  virtual si32 getWidth() { return _ni; };

  /** @brief Get the height of data in this projection */
  virtual si32 getHeight() { return _nj; };

  /** @brief Get the scanning mode of data */
  virtual si32 getIscan() { return _scanModeFlag; };

  /** @brief Get the packed data template size */
  virtual si32 getTemplateSize() { return MERCATOR_SIZE; };


  /** @brief Shape of the Earth
   *
   * See http://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_table3-2.shtml */
   // 0 for spherical earth with radius = 6367.47 km
   // 1 to define a spherical radius with radius _radiusScaleValue
   // 2 for oblate spheriod with major axis 6378.160 km, minor axis 6356.775 km
   // 3 to define oblate spheriod with _majorAxisScaleValue and _minorAxisScaleValue
   // 4 for oblate spheriod with major axis 6378.1370 km, minor axis 6356.752314 km
   // 6 for spherical earth with radius = 6371.2290 km
  si32 _earthShape;

  /** @brief Scale Factor for _radiusScaleValue      (0 if _earthShape != 1) */
  si32 _radiusScaleFactor;

  /** @brief Spherical earth value * _radiusScaleFactor in km (0 if _earthShape != 1) */
  si32 _radiusScaleValue;

  /** @brief Scale Factor for _majorAxisScaleValue   (0 if _earthShape != 3) */
  si32 _majorAxisScaleFactor;

  /** @brief Earth Major axis value * _majorAxisScaleFactor in km (0 if _earthShape != 3) */
  si32 _majorAxisScaleValue;

  /** @brief Scale Factor for _minorAxisScaleValue   (0 if _earthShape != 3) */
  si32 _minorAxisScaleFactor;

  /** @brief Earth Minor axis value * _minorAxisScaleFactor in km (0 if _earthShape != 3) */
  si32 _minorAxisScaleValue;

  /** @brief Number of points along a parallel (line of latitude) */
  ui32 _ni;

  /** @brief Number of points along a meridian (line of longitude) */
  ui32 _nj;

  /** @brief Latitude of first point */
  fl32 _la1;

  /** @brief Longitude of first point */
  fl32 _lo1;

  /** @brief i and j direction flags and u and v component directions 
   *
   * The resolution flags (bits 3-4 Flag table 3.3) are not applicable
   * See http://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_table3-3.shtml */
  ui08 _resolutionFlag;

  /** @brief LaD - latitude where Dx and Dy are specified */
  fl32 _lad;

  /** @brief Latitude of last point */
  fl32 _la2;

  /** @brief Longitude of last point */
  fl32 _lo2;

  /** @brief Scanning mode flag
   *
   * See http://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_table3-4.shtml */
   // BitNumber  Value   Meaning
   // 1          0       Points scan in +i direction, from pole to equator
   //            1       Points scan in -i direction, from equator to pole
   // 2          0       Points scan in +j direction, from west to east
   //            1       Points scan in -j direction, from east to west
   // 3          0       Adjacent points in i direction are consecutive
   //            1       Adjacent points in j direction are consecutive
  ui08 _scanModeFlag;

  /** @brief lov - Orientation of the grid
   * LoV is the angle between i direction on the map and the Equator
   * Limited to the range of  0 to 90 degrees; 
   * if the angle of orientation of the grid is neither 0 nor 90 degrees, Di and Dj must be equal to each other. */
  fl32 _lov;

  /** @brief i (Latitude) direction increment */
  fl32 _di;

  /** @brief j (longitude direction increment */
  fl32 _dj;

protected:


private: 

  static const si32 MERCATOR_SIZE;

};

} // namespace Grib2

#endif

