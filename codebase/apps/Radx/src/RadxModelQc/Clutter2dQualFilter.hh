/**
 * @file Clutter2dQualFilter.hh
 * @brief The  clutter 2 dimensional quality filter
 * @class Clutter2dQualFilter
 * @brief The  clutter 2 dimensional quality filter
 */
#ifndef CLUTTER2D_QUAL_FILTER_H
#define CLUTTER2D_QUAL_FILTER_H

#include <string>
#include <vector>

class RayLoopData;
class RadxRay;

class Clutter2dQualFilter
{
public:
  /**
   * Constructor
   */
  inline Clutter2dQualFilter() {}

  /**
   * Destructor
   */
  inline ~Clutter2dQualFilter() {}
  
  /**
   * Perform filter on input, write to last arg
   *
   * output = SCR*(1-exp(-scale*|VEL|*1.5 + WIDTH*0.5))
   *
   * @param[in] scrName  Name of 'SCR' input field
   * @param[in] velName  Name of velocity input field
   * @param[in] widthName  Name of width input field
   * @param[in] scale     param
   * @param[in] vr_shape  param
   * @param[in] sw_shape  param
   * @param[in] ray  The current ray
   * @param[in] data  The derived data, so far
   * @param[out] output  
   * @return true if successful
   *
   */
  bool filter(const std::string &scrName, const std::string &velName,
	      const std::string &widthName, double scale, double vr_shape,
	      double sw_shape, const RadxRay *ray,
	      const std::vector<RayLoopData> &data, RayLoopData *output);
    
private:
};

#endif
