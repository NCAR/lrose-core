/**
 * @file Variance1dFilter.hh
 * @brief Variance1d(field,npt,maxPercentMissing) = 
 *        variance over npt points,with output set missing if too many missing
 * @class Variance1dFilter
 * @brief Variance1d(field,npt,maxPercentMissing) = 
 *        variance over npt points,with output set missing if too many missing
 */

#ifndef Variance1d_FILTER_H
#define Variance1d_FILTER_H

#include <string>
#include <vector>

class RadxAppRayLoopData;
class RadxRay;
class RayxData;


class Variance1dFilter
{
public:
  /**
   * Constructor sets members to inputs
   */
  inline Variance1dFilter(double npt, double maxPctMissing) :
    _npt(npt), _maxPctMissing(maxPctMissing) {}

  /**
   * Destructor
   */
  inline ~Variance1dFilter() {}
  
  /**
   * Do the filter
   * @param[in] data
   * @param[out] output
   */
  bool filter(const RayxData &data, RadxAppRayLoopData *output);
    
private:
  double _npt;            /**< Number of points in variance */
  double _maxPctMissing;  /**< Max percent of points missing to still 
			   *   take variance */
};

#endif
