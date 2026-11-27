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
#include <radar/RadxAppRayData.hh>
#include <rapmath/MathData.hh>

class VolumeBase;
class RadxPersistentClutter;
class RadxAppRayLoopData;
class RadxRay;

//------------------------------------------------------------------
class RayData1 : public RadxAppRayData
{
public:

  /**
   * Empty
   */
  RayData1(void);

  /**
   * Set up for index'th ray in the RadxVol
   *
   * @param[in] r The volume object
   * @param[in] index  Ray index
   */
  RayData1(const VolumeBase &r, int index);

  /**
   * Destructor
   */
  virtual ~RayData1(void);

  #include <radar/RadxAppMathDataVirtualMethods.hh>

protected:
private:

  static const std::string _histoAccumStr;  /**< User unary */

  // RadxRay *_ray;          /**< Pointer to ray (main one ) */
  RadxPersistentClutter *_p; /**< Pointer passed in */

  bool _processAccumHisto(std::vector<ProcessingNode *> &args) const;
};

#endif
