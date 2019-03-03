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

class RadxAppRayLoopData;
class RadxRay;
class RayxData;

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
   * @param[in] scr  'SCR' input field
   * @param[in] vel  velocity input field
   * @param[in] width width input field
   * @param[in] scale     param
   * @param[in] vr_shape  param
   * @param[in] sw_shape  param
   * @param[out] output  Where to store filter output
   * @return true if successful
   *
   */
  bool filter(const RayxData &scr, const RayxData &vel, const RayxData &width,
	      double scale, double vr_shape, double sw_shape,
	      RadxAppRayLoopData *output);
    
private:
};

#endif
