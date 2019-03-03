/**
 * @file Special1Filter.hh
 * @brief A special filter Special1(width,meanPrt,meanNSamples)=
 *        0.107/(8*meanPrt*meanNsamples*sqrt(PI)*width)
 * @class Special1Filter
 * @brief A special filter Special1(width,meanPrt,meanNSamples)=
 *        0.107/(8*meanPrt*meanNsamples*sqrt(PI)*width)
 */

#ifndef SPECIAL1_FILTER_H
#define SPECIAL1_FILTER_H
#include <string>
#include <vector>

class RadxAppRayLoopData;
class RadxRay;
class RayxData;

class Special1Filter
{
public:
  /**
   * Empty constructor
   */
  inline Special1Filter() {}

  /**
   * Empty destructor
   */
  inline ~Special1Filter() {}
  
  /**
   * Do the filter
   *
   * @param[in] width  Data
   * @param[in] meanPrt  param
   * @param[in] meanNsamples  param
   * @param[out] output  Output data object to write to
   */
  bool filter(const RayxData &width, double meanPrt, double meanNsamples,
	      RadxAppRayLoopData *output);
    
private:
};

#endif
