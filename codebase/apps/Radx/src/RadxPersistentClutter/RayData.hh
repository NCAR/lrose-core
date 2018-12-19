/**
 * @file RayData.hh 
 * @brief Volume data handler for Radx data, contains entire volume
 *
 * @class RayData
 * @brief Volume data handler for Radx data, contains entire volume
 */

#ifndef RAY_DATA_H
#define RAY_DATA_H
#include <radar/RadxAppVolume.hh>
#include <vector>
#include <string>
#include <map>

//------------------------------------------------------------------
class RayData : public RadxAppVolume
{
  friend class RayData1;
  friend class RayData2;

public:

  RayData(void);
  RayData(const RadxAppParms *parms, int argc, char **argv);
  RayData(const RayData &r);
  void operator=(const RayData &r);

  /**
   * Destructor
   */
  virtual ~RayData(void);

  /**
   * Virtual methods from base class
   */
  #define FILTALG_DERIVED
  #include <rapmath/VolumeDataVirtualMethods.hh>
  #undef FILTALG_DERIVED

protected:
private:

};

#endif
