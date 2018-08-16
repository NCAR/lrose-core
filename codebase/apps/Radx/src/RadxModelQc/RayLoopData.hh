/**
 * @file RayLoopData.hh 
 * @brief Container for one ray of data for one field
 *
 * @class RayLoopData
 * @brief Container for one ray of data for one field
 *        
 */

#ifndef RAY_LOOP_DATA_H
#define RAY_LOOP_DATA_H
#include <vector>
#include <string>
#include <Radx/RayxData.hh>
#include <rapmath/MathLoopData.hh>

//------------------------------------------------------------------
class RayLoopData : public MathLoopData, public RayxData
{
public:

  RayLoopData(const RayxData &r);

  /**
   * Destructor
   */
  virtual ~RayLoopData(void);

  #include <rapmath/MathLoopDataVirtualMethods.hh>

protected:
private:

};

#endif
