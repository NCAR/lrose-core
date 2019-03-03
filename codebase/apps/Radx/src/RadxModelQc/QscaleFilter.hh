/**
 * @file QscaleFilter.hh
 * @brief Perform 'Qscale' filter on inputs
 * @class QscaleFilter
 * @brief Perform 'Qscale' filter on inputs
 *
 * Qscale = exp(-coeff*(x/topv - lowv/topv)^2)
 */

#ifndef QSCALE_FILTER_H
#define QSCALE_FILTER_H
#include <string>
#include <vector>

class RadxAppRayLoopData;
class RayxData;
class RadxRay;


class QscaleFilter
{
public:
  /**
   * Constructor
   */
  QscaleFilter();

  /**
   * Destructor
   */
  ~QscaleFilter();
  
  /**
   * Perform filter using inputs, output to last arg
   * @param[in] name  Field to use
   * @param[in] scale  param
   * @param[in] topv  param
   * @param[in] lowv  param
   * @param[in] subtractFromOne  If true result is subtracted from 1
   * @param[in] ray  The ray being processing
   * @param[in] data  The derived fields
   * @param[out] output
   * @return true for success
   */
  bool filter(const RayxData &data, double scale, double topv,
	      double lowv, bool subtractFromOne, RadxAppRayLoopData *output);
    
private:
};

#endif
