/**
 * @file RayCloudEdge.hh
 * @brief Range of indices in cloud and just outside cloud along a beam
 * @class RayCloudEdge
 * @brief Range of indices in cloud and just outside cloud along a beam
 */
#ifndef RAY_CLOUD_EDGE_HH
#define RAY_CLOUD_EDGE_HH

#include "RaySubset.hh"
#include <string>
#include <cstdio>

class Grid2d;

class RayCloudEdge
{
public:

  /**
   * At the radar
   *
   * @param[in] y  Azimuth index
   */
  inline RayCloudEdge(int y) : _y(y), _cloud(y, false), _outside(y, true),
			       _value(0), _movingIn(true) {}

  /**
   * An edge away from radar
   *
   * @param[in] y  Azimuth index
   * @param[in] cloud    Cloud range of indices
   * @param[in] outside  Outside cloud range of indices
   * @param[in] value   The value for this cloud = clump color
   * @param[in] movingIn  True if at near edge of cloud, false if at far edge
   *
   * IF at the near edge, 'outside' points are even closer, if at the far edge
   * outside points are even further
   */
  inline RayCloudEdge(int y, const RaySubset &cloud, const RaySubset &outside,
		      double value, bool movingIn) :
    _y(y), _cloud(cloud), _outside(outside), _value(value), _movingIn(movingIn)
  {}
    
  /**
   * Destructor
   */
  inline ~RayCloudEdge(void) {}

  /**
   * @return description string
   */
  std::string sprint(void) const;

  /**
   * Debug print to stdout 
   */
  void print(void) const;

  /**
   * LOG(DEBUG_VERGOSE) the sprint() output
   */
  void log(const std::string &msg) const;

  /**
   * Write the local _value to the input grid along the ray in the cloud
   * @param[in,out] data
   */
  void toGrid(Grid2d &data) const
  {
    _cloud.toGrid(data, _value);
  }

  /**
   * Write the local _value to the input grid along the ray outside the cloud
   * @param[in,out] data
   */
  void toOutsideGrid(Grid2d &data) const
  {
    _outside.toGrid(data, _value);
  }

  /**
   * @return the local value
   */
  inline double getValue(void) const {return _value;}


  /**
   * @return true if this edge is at the radar
   */
  inline bool isAtRadar(void) const
  {
    return _cloud.isCloudAtRadar() && _outside.isOutsideAtRadar();
  }

  /**
   * @return the nearest cloud x index
   */
  inline int closestCloudPoint(void) const {return _cloud.getX0();}

  /**
   * @return the farthest cloud x index
   */
  inline int farthestCloudPoint(void) const {return _cloud.getX1();}

  /**
   * @return true if this edge is looking into the cloud
   */
  inline bool movingIn(void) const
  {
    return _movingIn;
  }

  /**
   * @return the y (beam) index
   */
  inline int beamIndex(void) const {return _y;}

protected:
private:

  int _y;              /**< beam index */
  RaySubset _cloud;   /**< Points inside the cloud */
  RaySubset _outside; /**< Points just outside the cloud */
  double _value;      /**< Clump value for cloud */
  bool _movingIn;      /**< True if this edge is closest to radar */

};

#endif
