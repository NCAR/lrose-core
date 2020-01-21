// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// © University Corporation for Atmospheric Research (UCAR) 2009-2010. 
// All rights reserved.  The Government's right to use this data and/or 
// software (the "Work") is restricted, per the terms of Cooperative 
// Agreement (ATM (AGS)-0753581 10/1/08) between UCAR and the National 
// Science Foundation, to a "nonexclusive, nontransferable, irrevocable, 
// royalty-free license to exercise or have exercised for or on behalf of 
// the U.S. throughout the world all the exclusive rights provided by 
// copyrights.  Such license, however, does not include the right to sell 
// copies or phonorecords of the copyrighted works to the public."   The 
// Work is provided "AS IS" and without warranty of any kind.  UCAR 
// EXPRESSLY DISCLAIMS ALL OTHER WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
// ANY IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
// PURPOSE.  
//  
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
#include <toolsa/copyright.h>
/**
 * @file  LineGustFront.hh
 * @brief Information for a point in relation to one segment of a gust front 
 * @class LineGustFront
 * @brief Information for a point in relation to one segment of a gust front 
 */

# ifndef    LINE_GUST_FRONT_H
# define    LINE_GUST_FRONT_H

#include "Xyz.hh"
#include "LineAndPoint.hh"
class GustFront;
class Data;

//----------------------------------------------------------------
class LineGustFront : public LineAndPoint
{
public:

  /**
   * Constructor
   */
  LineGustFront(void);

  /**
   * Destructor
   */
  virtual ~LineGustFront(void);

  /**
   * Build up internal state for inputs
   *
   * @param[in] gf  Algorithm
   * @param[in] loc  Point in space (meters)
   * @param[in,out] is_inside  Set to true if the point lies within the region
   *                           perpendicular to the segment and directly
   *                           in front or behind, not out to the side
   *
   * @return true if could build state
   */
  bool segment(const GustFront &gf, const Xyz &loc,  bool &is_inside);

  /**
   * @return velocity vector (m/s)
   */
  inline Xyz getVel(void) const {return _vel;}

  /**
   * Maximize reflectivity in data using local state
   *
   * @param[in] scale  Scale factor to apply to local dbz
   * @param[in,out] data
   */
  void setDbz(double scale, Data &data) const;

  /**
   * Add the local _vel scaled by _distance_inside*_scale to wind, and add
   * _distance_inside to denom input
   *
   * @param[in,out] wind  Wind to modify
   * @param[in,out] denom  Denom to modify
   */
  void accumlateInsideWind(Xyz &wind, double &denom) const;

  /**
   * @return _vel, scaled by _scale
   */
  Xyz getScaledWind(void) const;

  /**
   * @return true if local dbz value is equal to input value, and
   * distance_beyond_side is 0  (it is a thin line in the right place)
   *
   * @param[in] dbz  Value to compare against
   */
  bool fullInsideDbz(double dbz) const;

  /**
   * Compute and return reflectivity when point is outside but in the
   * thin line, returning the scaled reflecivity value
   *
   * @param[in] zeroth  True if local segment is 0th one of the gust front,
   *                    false if the segment is the last one of the gust front
   * @param[in] gf  Algorithm
   * @param[in] z  point height meters
   * @param[out] ref  returned value
   *
   * @return true if the point is outside and in the thin line and
   *         ref was computed and returned.
   */
  bool dbzOutside(bool zeroth, const GustFront &gf, double z,double &ref) const;

private:

  /**
   * Scale factor computed based on _distance_beyond_side
   */
  double _scale;

  double _dbz;    /**< Thin line dbz constant value */
  bool _is_dbz;   /**< True if inside thin line, which is assumed to extend
		   *   infinitely far parallel to the line segment */
  Xyz _vel;       /**< Wind value at the point, using model that extends
		   *   infinitely far parallel to line segment */

  /*
   * Apply the gust front model at x (ahead or behind) z and compute
   * mag, dir, and dbz
   */
  void  _model(const GustFront &gf, const double z, double &mag, Xyz &dir);

  /*
   * Set dbz value associated with the input gustfront at x (ahead or behind)
   * and z (above ground).
   */
  void _setDbz(const GustFront &gf, double x, double z);

};

# endif 
