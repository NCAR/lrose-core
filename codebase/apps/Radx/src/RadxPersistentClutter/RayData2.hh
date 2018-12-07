/**
 * @file RayData2.hh 
 * @brief Container for the processing of one sweep
 * @class RayData1
 * @brief Container for the processing of one sweep
 *
 * A placeholder
 */

#ifndef RAY_DATA_2_H
#define RAY_DATA_2_H
#include <rapmath/MathData.hh>
class RayData;

//------------------------------------------------------------------
class RayData2 : public MathData
{
public:

  RayData2(void);

  /**
   * Set up for index'th sweep in the RadxVol
   *
   * @param[in] The volume object
   * @param[in] index  Ray index
   */
  RayData2(const RayData &r, int index);

  /**
   * Destructor
   */
  virtual ~RayData2(void);

  #include <rapmath/MathDataVirtualMethods.hh>
  
protected:
private:

};

#endif
