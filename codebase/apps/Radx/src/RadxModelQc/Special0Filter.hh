/**
 * @file Special0Filter.hh
 * @brief  A special filter Special0(width,meanPrt,meanNSamples) = 
 *         10*log10(1 + sqrt(1/(width*(4*sqrt(PI)*meanPrt*meanNsamples/0.1))))
 * @class Special0Filter
 * @brief  A special filter Special0(width,meanPrt,meanNSamples) = 
 *         10*log10(1 + sqrt(1/(width*(4*sqrt(PI)*meanPrt*meanNsamples/0.1))))
 */

#ifndef SPECIAL0_FILTER_H
#define SPECIAL0_FILTER_H
#include <string>
#include <vector>

class RadxAppRayLoopData;
class RadxRay;
class RayxData;


class Special0Filter
{
public:
  /**
   * Empty constructor
   */
  inline Special0Filter() {}

  /**
   * Empty destructor
   */
  inline ~Special0Filter() {}
  
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
