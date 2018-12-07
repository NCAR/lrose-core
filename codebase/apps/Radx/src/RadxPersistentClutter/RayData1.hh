/**
 * @file RayData1.hh 
 * @brief Container for the processing of one ray
 * @class RayData1
 * @brief Container for the processing of one ray
 *
 * This is in only as a placeholder
 */

#ifndef RAY_DATA_1_H
#define RAY_DATA_1_H
#include <rapmath/MathData.hh>
class RayData;

//------------------------------------------------------------------
class RayData1 : public MathData
{
public:

  RayData1(void);

  /**
   * Set up for index'th ray in the RadxVol
   *
   * @param[in] The volume object
   * @param[in] index  Ray index
   */
  RayData1(const RayData &r, int index);

  /**
   * Destructor
   */
  virtual ~RayData1(void);

  #include <rapmath/MathDataVirtualMethods.hh>
  
protected:
private:


};

#endif
