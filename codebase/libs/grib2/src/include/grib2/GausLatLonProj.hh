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
 * @file GausLatLonProj.hh
 * @author Jason Craig
 * @author Gary Blackburn
 * @date   Oct 2006
 */

#ifndef _GRIB2_GAUSLATLON_PROJ 
#define _GRIB2_GAUSLATLON_PROJ

#include <cstdio>
#include <grib2/PortTypes.hh>
#include <grib2/GribProj.hh>
#include <grib2/constants.h>

using namespace std;

namespace Grib2 {

/** 
 * @class GausLatLonProj
 *
 * A derived GribProj class for Gaussian Latitude/Longitude
 * Projections.
 *
 * See http://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_temp3-40.shtml
 *
 * @note The pack and unpack routines are static methods in Grib2::GribSection
 */
class GausLatLonProj: public GribProj {

public:
  /**
   * @brief Default constructor for use in passing object to GribFile.addField()
   *
   * @note After creation.. all public values variables below should be set
   * before passing this object to GribFile. 
   */
  GausLatLonProj();

  virtual ~GausLatLonProj();

  /** @brief Unpack a GausLatLon Projection Template 
   *  @param[in] projPtr Pointer to start of template
   *  @return Either GRIB_SUCCESS or GRIB_FAILURE */
  virtual int unpack (g2_ui08 *projPtr);

  /** @brief Pack up this GausLatLon Projection Template
   *  @param[in] projPtr Pointer to start of location to pack template
   *  @return Either GRIB_SUCCESS or GRIB_FAILURE */
  virtual int pack (g2_ui08 *projPtr);

  /** @brief Print to stream/file all information for this template */
  virtual void print (FILE *) const;

  /** @brief Get the width of data in this projection */
  virtual g2_si32 getWidth() { return _ni; };

  /** @brief Get the height of data in this projection */
  virtual g2_si32 getHeight() { return _nj; };

  /** @brief Get the scanning mode of data */
  virtual g2_si32 getIscan() { return _scanModeFlag; };

  /** @brief Get the packed data template size */
  virtual g2_si32 getTemplateSize() { return LAT_LON_SIZE; };

  /** @brief Function to calculate the longitudes (columns) given a row
   * Only needed if grid is quasi regular, di = -1.0 and ni = -1
   * @param[in] lons Address of g2_fl32 Pointer which will be created to hold lon array, caller must free memory.
   * @param[in] rowIndex Index of row to calculate longitudes at
   * &param[out] Number of columns in this row
   */
  int getQuasiLons(g2_fl32 **lons, int rowIndex);

  /** @brief Function to calculate the latitudes of each row
   * @param[in] lats Address of g2_fl32 Pointer which will be created to hold lat array, caller must free memory.
   */
  void getGaussianLats(g2_fl32 **lats);

  /** @brief Shape of the Earth
   *
   * See http://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_table3-2.shtml */
   // 0 for spherical earth with radius = 6367.47 km
   // 1 to define a spherical radius with radius _radiusScaleValue
   // 2 for oblate spheriod with major axis 6378.160 km, minor axis 6356.775 km
   // 3 to define oblate spheriod with _majorAxisScaleValue and _minorAxisScaleValue
   // 4 for oblate spheriod with major axis 6378.1370 km, minor axis 6356.752314 km
   // 6 for spherical earth with radius = 6371.2290 km
  g2_si32 _earthShape;

  /** @brief Scale Factor for _radiusScaleValue      (0 if _earthShape != 1) */
  g2_si32 _radiusScaleFactor;

  /** @brief Spherical earth value * _radiusScaleFactor in km (0 if _earthShape != 1) */
  g2_si32 _radiusScaleValue;

  /** @brief Scale Factor for _majorAxisScaleValue   (0 if _earthShape != 3) */
  g2_si32 _majorAxisScaleFactor;

  /** @brief Earth Major axis value * _majorAxisScaleFactor in km (0 if _earthShape != 3) */
  g2_si32 _majorAxisScaleValue;

  /** @brief Scale Factor for _minorAxisScaleValue   (0 if _earthShape != 3) */
  g2_si32 _minorAxisScaleFactor;

  /** @brief Earth Minor axis value * _minorAxisScaleFactor in km (0 if _earthShape != 3) */
  g2_si32 _minorAxisScaleValue;

  /** @brief Number of points along a parallel (line of latitude) */
  g2_ui32 _ni;

  /** @brief Maximum number of points along a parallel, Only valid when _ni is 0 (reduced grid) */
  g2_ui32 _maxNi;

  /** @brief Number of points along a meridian (line of longitude) */
  g2_ui32 _nj;

  /** @brief Basic angle overrides the default GDS::DEGREES_SCALE_FACTOR */
  g2_fl32 _basicAngleProdDomain;

  /** @brief Defines extreme longitudes / latitudes and direction increments */
  g2_fl32 _basicAngleSubdivisions;

  /** @brief Latitude of first point */
  g2_fl32 _la1;

  /** @brief Longitude of first point */
  g2_fl32 _lo1;

  /** @brief i and j direction flags and u and v component directions 
   *
   * See http://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_table3-3.shtml */
  g2_ui08 _resolutionFlag;

  /** @brief Latitude of last grid point */
  g2_fl32 _la2;

  /** @brief Longitude of last grid point */
  g2_fl32 _lo2;

  /** @brief i (Latitude) direction increment */
  g2_fl32 _di;

  /** @brief Number of paralells between a pole and equater
   * The number of parallels between a pole and the equator is used 
   * to establish the variable (Gaussian) spacing of the parallels */
  g2_si32 _nParalells;

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
  g2_ui08 _scanModeFlag;

  /** @brief List of number of points along each meridian or parallel
   * Only present for quasi-regular grids, _ni missing or _nj missing
   * Only quasi-regular grid along parallels, latitudes, are implemented */
  g2_si32 *_pointsList;

protected:


private: 

  double _gord(int n, double x);

  static const g2_si32 LAT_LON_SIZE;

};

} // namespace Grib2

#endif

