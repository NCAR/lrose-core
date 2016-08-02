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
 * @file RayxMapping.hh
 * @brief  Mapping volume azimuth elevations to handle overlaps
 * @class RayxMapping
 * @brief Mapping volume azimuth elevations to handle overlaps
 *
 * Map input beams to a fixed set of azimuths and elevations, within tolerances.
 * Used to seek out (and maybe destroy) multiple beams that map to the same
 * azimuth/elevation.
 */
#ifndef RayxMapping_H
#define RayxMapping_H

#include <Radx/RadxAzElev.hh>
#include <vector>
class RadxRay;
using namespace std;

class RayxMapping
{
  
public:

  /**
   * constructor, empty
   */  
  RayxMapping(void);

  /**
   * constructor, with values
   *
   * @param[in] nelev  Number of elevation angles
   * @param[in] elev  The elevations, degrees
   * @param[in] az_tolerance_degrees  Allowed error in azimuths
   * @param[in] elev_tolerance_degrees  Allowed error in elevation angles
   *
   * Stores all the input elevations as individual _elev entries
   */  
  RayxMapping(const int nelev, const double *elev,
             const double az_tolerance_degrees,
             const double elev_tolerance_degrees);

  /**
   * destructor
   */
  ~RayxMapping(void);

  /**
   * Add a ray to the state
   * @param[in] ray
   *
   * If the rays azimuth is not in the state, that is added to _az
   * Look up the elevation that matches the ray's elevation (within tolerance).
   * If no match is found it is an error with return of false.
   * If match is found, the az/elev pair is added to state as _azelev (unique)
   * or _azelevMulti (already in state).
   *
   * @return true unless elev not found
   */
  bool add(const RadxRay &ray);

  /**
   * @return true if this az/elev is shared by more than one beam in volume
   *  after taking into account tolerances (if it is in _azelevMulti).
   * @param[in] az
   * @param[in] elev
   */
  bool isMulti(const double az, const double elev) const;

  /**
   * @return true if this az/elev is shared by more than one beam in volume
   *  after taking into account tolerances (if it is in _azelevMulti).
   * @param[in] ae 
   */
  bool isMulti(const RadxAzElev &ae) const;

  /**
   * @return RadxAzElev object that matches inputs
   * @param[in] az
   * @param[in] elev
   * 
   * Returns empty object if no match found
   */
  RadxAzElev match(const double az, const double elev) const;

protected:
private:

  double _az_tolerance_degrees;     /**< Allowed slop */
  double _elev_tolerance_degrees;   /**< Allowed slop */
  std::vector<double> _az;          /**< The azimuths that are in the volume*/
  std::vector<double> _elev;        /**< The elevs that are configured for */
  std::vector<RadxAzElev> _azelev;      /**< The az/elev pairs that are in map */
  std::vector<RadxAzElev> _azelevMulti; /**< The az/elev pairs non-unique */

  static bool _match(const double a, const vector<double> &v, 
                     const double tolerance, double &amatch);

};

#endif
