/**
 * @file RayData.hh 
 * @brief Volume data handler for Radx data, contains entire volume
 *
 * @class RayData
 * @brief Volume data handler for Radx data, contains entire volume
 */

#ifndef RAY_DATA_H
#define RAY_DATA_H
#include "RayLoopData.hh"
#include <radar/RadxAppVolume.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RayxData.hh>
#include <rapmath/SpecialUserData.hh>
#include <vector>
#include <string>

class MathUserData;
class CircularLookupHandler;
class RadxSweep;

//------------------------------------------------------------------
class RayData : public RadxAppVolume
{
  friend class RayData1;
  friend class RayData2;

public:

  RayData(void);
  RayData(const RadxAppParms *parms, int argc, char **argv);

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

  /**
   *
   */
  void initialize(const CircularLookupHandler *lookup);

  void trim(const std::vector<std::string> &outputKeep, bool outputAll);

protected:
private:

  const CircularLookupHandler *_lookup; /**< Lookup thing for 2dvar */

  /**
   * Names of derived fields
   */
  std::vector<std::string> _rayDataNames;

  /**
   * The names/pointers to special data fields, these are not in same
   * format as the RayData itself.
   */
  SpecialUserData _special;

  // /**
  //  * Pointers to special data to go with each such field
  //  */
  // std::vector<MathUserData *> _specialValue;

  bool _needToSynch(const std::string &userKey) const;
  MathUserData *_volumeAverage(const std::string &name) const;
  MathUserData *_volumeAzGradientState(void) const;
  bool _hasData(const std::string &userKey, const std::string &name,
		bool suppressWarn=false);
};

#endif
