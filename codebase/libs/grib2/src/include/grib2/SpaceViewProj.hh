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
 * @file SpaceViewProj.hh
 * @author Jason Craig
 * @date   Jun 2016
 */

#ifndef _GRIB2_SPACEVIEW_PROJ 
#define _GRIB2_SPACEVIEW_PROJ

#include <cstdio>
#include <dataport/port_types.h>
#include <grib2/GribProj.hh>
#include <grib2/constants.h>

using namespace std;

namespace Grib2 {

/** 
 * @class SpaceViewProj
 *
 * A derived GribProj class for Space View Perspective or Orthographic Projections.
 *
 * See http://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_temp3-90.shtml
 *
 * @note The pack and unpack routines are static methods in Grib2::GribSection
 */
class SpaceViewProj: public GribProj {

public:
  /**
   * @brief Default constructor for use in passing object to GribFile.addField()
   *
   * @note After creation.. all public value variables below should be set
   * before passing this object to GribFile. 
   */
  SpaceViewProj();

  virtual ~SpaceViewProj();

  /** @brief Unpack a SpaceView Projection Template 
   *  @param[in] projPtr Pointer to start of template
   *  @return Either GRIB_SUCCESS or GRIB_FAILURE */
  virtual int unpack (ui08 *projPtr);

  /** @brief Pack up this SpaceView Projection Template
   *  @param[in] projPtr Pointer to start of location to pack template
   *  @return Either GRIB_SUCCESS or GRIB_FAILURE */
  virtual int pack (ui08 *projPtr);

  /** @brief Print to stream/file all information for this template */
  virtual void print (FILE *) const;

  /** @brief Get the width of data in this projection */
  virtual si32 getWidth() { return _nx; };

  /** @brief Get the height of data in this projection */
  virtual si32 getHeight() { return _ny; };

  /** @brief Get the scanning mode of data */
  virtual si32 getIscan() { return _scanModeFlag; };

  /** @brief Get the packed data template size */
  virtual si32 getTemplateSize() { return LAT_LON_SIZE; };

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

  /** @brief Number of points along x-axis (columns) */
  ui32 _nx;

  /** @brief Number of points along y-axis (rows or lines) */
  ui32 _ny;

  /** @brief Latitude of sub-satellite point */
  fl32 _lap;

  /** @brief Longitude of sub-satellite point */
  fl32 _lop;

  /** @brief i and j direction flags and u and v component directions 
   *
   * See http://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_table3-3.shtml */
  ui08 _resolutionFlag;

  /** @brief Apparent diameter of Earth in grid lengths, in x-direction */
  ui32 _dx;

  /** @brief Apparent diameter of Earth in grid lengths, in y-direction */
  ui32 _dy;

  /** @brief X-coordinate of sub-satellite point */
  ui32 _xp;

  /** @brief Y-coordinate of sub-satellite point */
  ui32 _yp;

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

  /** @brief Orientation of the grid 
   *  The angle between the increasing y-axis and the meridian of the sub-satellite 
   *  point in the direction of increasing latitude
   **/
  fl32 _lov;

  /** @brief Altitude of the camera from the Earth's centre
   * Measured in units of the Earth's (equatorial)
   * radius multiplied by a scale factor of 10^6  (missing = all bits set to 1 = infinite distance)
   * The apparent angular size of the Earth will be given by 2 × arcsin ((10^6 )/Nr). 
   **/
  fl32 _nr;

  /** @brief X-coordinate of origin of sector image **/
  ui32 _xo;

  /** @brief Y-coordinate of origin of sector i **/
  ui32 _yo;


protected:


private: 

  static const si32 LAT_LON_SIZE;

};

} // namespace Grib2

#endif

