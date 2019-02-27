/**
 * @file RadxAppRayLoopData.hh 
 * @brief Container for one ray of data for one field, used in algorithm
 *        when looping through rays one at a time
 * @class RadxAppRayLoopData
 * @brief Container for one ray of data for one field, used in algorithm
 *        when looping through rays one at a time
 */

#ifndef RADX_APP_RAY_LOOP_DATA_H
#define RADX_APP_RAY_LOOP_DATA_H
#include <vector>
#include <string>
#include <Radx/RayxData.hh>
#include <rapmath/MathLoopData.hh>

//------------------------------------------------------------------
class RadxAppRayLoopData : public MathLoopData, public RayxData
{
public:

  /**
   * Empty ray
   */
  RadxAppRayLoopData(void);

  /**
   * Set base class using input
   * @param[in] r
   */
  RadxAppRayLoopData(const RayxData &r);

  /**
   * Destructor
   */
  virtual ~RadxAppRayLoopData(void);

  #include <rapmath/MathLoopDataVirtualMethods.hh>

protected:
private:

};

#endif
