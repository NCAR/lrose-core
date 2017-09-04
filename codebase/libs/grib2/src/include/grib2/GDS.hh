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
 * @file GDS.hh
 * @brief Section 3, Grid Description Section
 * @author Jason Craig
 * @author Gary Blackburn
 * @date   Aug 2006
 */

#ifndef _GRIB2_GDS_
#define _GRIB2_GDS_

#include <grib2/GribSection.hh>
#include <grib2/constants.h>
#include <grib2/GribProj.hh>

using namespace std;

namespace Grib2 {
/** 
 * @class GDS
 * @brief Section 3, Grid Description Section
 *
 * Grid Description Section is section 3 in Grib2.
 *
 * See http://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_sect3.shtml
 */
class GDS: public Grib2::GribSection {
public:
  /**
   * @brief This constructor is generally only used from Grib2::Grib2Record::unpack() for reading a Grib2 file
   */
  GDS();

  /**
   * @brief This constructor is generally only used from Grib2::Grib2Record::addField() for creating a Grib2File
   */
  GDS(si32 numberDataPoints, si32 gridDefNum, GribProj *projectionTemplate);

  ~GDS();

  /** @brief Unpacks the whole Grid Definition Section
   *  @param[in] gdsPtr Pointer to start of the GDS section
   *  @return Either Grib2::GRIB_SUCCESS or Grib2::GRIB_FAILURE */
  int unpack( ui08 *gdsPtr );

  /** @brief Packs up the Grid Definition Section
   *  @param[in] gdsPtr Pointer to start of location to pack to
   *  @return Either Grib2::GRIB_SUCCESS or Grib2::GRIB_FAILURE */
  int pack( ui08 *gdsPtr );

  /** @brief Print to stream/file all information contained in the GDS section */
  virtual void print(FILE *stream) const;

  // Get Functions

  /** @brief get the earth radius or earth major_axis and earth minor_axis
   *  @param[out] major_axis earth major axis in m if earth assumed oblate spheriod, 0 otherwise
   *  @param[out] minor_axis earth minor axis in m if earth assumed oblate spheriod, 0 otherwise
   *  @return earth radius in m if earth assumed spherical, 0 otherwise */
  fl32 getEarthRadius(fl32 &major_axis, fl32 &minor_axis);

  /** @brief Get the number of data points in the grid */
  inline si32 getNumDataPoints() { return _numDataPoints; };

  /** @brief Get the grid definition source */
  inline si32 getGriDefSource() {return _gridDefSource; };

  /** @brief Get optional list size of number of grid points */
  inline si32 getListSize() { return _listSize; };

  /** @brief Get interpretation of optiona list of number of grid points */
  inline si32 getListInterpretation() { return _quasi_regularListInterp; };

  /** @brief Get grid template number */
  inline si32 getGridID() { return _gridTemplateNum; };

  /** @brief Get width of grid */
  inline si32 getWidth() { return _projection->getWidth(); };

  /** @brief Get height of grid */
  inline si32 getHeight() { return _projection->getHeight(); };

  /** @brief Get data scanning mode */
  inline si32 getIscan() { return _projection->getIscan(); };

  /** @brief Get the grid projection pointer */
  inline GribProj *getProjection() { return _projection; };

  // Set Functions

  /** @brief Set the number of data points in grid */
  inline void setNumDataPoints(si32 numDataPoints) { _numDataPoints = numDataPoints; };

  /** @brief Set the grid definition source */
  inline void setGridDefSource(si32 gridDefSource) { _gridDefSource = gridDefSource; };

  /** @brief Set optional list of numbers defining number of points */
  inline void setListSize(si32 listSize = 0) { _listSize = listSize; };

  /** @brief Set optional list interpretation */
  inline void setListInterpretation(si32 quasi_regularListInterp = 0) { _quasi_regularListInterp = quasi_regularListInterp; };

  /** @brief Set the grid template number */
  inline void setGridID(si32 gridTemplateNum) { _gridTemplateNum = gridTemplateNum; _gridDefSource = 0; };

  /** @brief Set the grid projection pointer */
  inline void setProjection(GribProj *projection) { _projection = projection; };

  // Implemented projection IDs
  /** @brief Projection ID 0, Lat/Lon Projection */
  static const si32 EQUIDISTANT_CYL_PROJ_ID;
  /** @brief Projection ID 1, Rotated Lat/Lon Projection */
  static const si32 ROT_EQUIDISTANT_CYL_PROJ_ID;
  /** @brief Projection ID 10, Mercator Projection */
  static const si32 MERCATOR_PROJ_ID;
  /** @brief Projection ID 20, Polar Sterographic Projection */
  static const si32 POLAR_STEREOGRAPHIC_PROJ_ID;
  /** @brief Projection ID 30, Lambert Conformal Projection */
  static const si32 LAMBERT_CONFORMAL_PROJ_ID;
  /** @brief Projection ID 40, Gaussian Lat/Lon Projection */
  static const si32 GAUSSIAN_LAT_LON_PROJ_ID;
  /** @brief Projection ID 90, Space View or Orthographic Projection */
  static const si32 SPACE_VIEW_PROJ_ID;
  /** @brief Projection ID 32769, Rotated Latitude/Longitude (Arakawa Non-E Staggered grid) */
  static const si32 ROT_LAT_LON_ARAKAWA_NON_E_PROJ_ID;

  // All degree values stored in grib2 are stored with this scale factor 
  /** @brief Grib2 Degree scale factor */
  static const fl32 DEGREES_SCALE_FACTOR;
  /** @brief Grib2 Grid scale factor */
  static const fl32 GRID_SCALE_FACTOR;

protected:

  /** @brief Source of the grid definition
   *
   * Currently only pre-defined grid definitions are implemented
   * and thus this value will always be 0 */
  si32 _gridDefSource;

  /** @brief Total number of data points in the grid */
  si32 _numDataPoints;
  
  /** @brief Size of optional list of numbers of points in the grid.  
   * Used for quasi-regular grids.
   *
   * Optional list is unimplemented and should not be used */
  si32 _listSize;

  /** @brief Interpretation of optional list of numbers of points in the grid. */
  si32 _quasi_regularListInterp;

  /** @brief Template number of GribProj */
  si32 _gridTemplateNum;

  /** @brief GribProj pointers stores additional projection specific information */
  GribProj *_projection;

};

} // namespace Grib2

#endif

