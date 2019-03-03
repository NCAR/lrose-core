/**
 * @file Volume.hh 
 * @brief Volume data handler for Radx data, contains entire volume
 *
 * @class Volume
 * @brief Volume data handler for Radx data, contains entire volume
 */

#ifndef VOLUME_H
#define VOLUME_H
#include <radar/RadxAppVolume.hh>
#include <rapmath/SpecialUserData.hh>
#include <vector>
#include <string>

class MathUserData;
class RadxAppCircularLookupHandler;

//------------------------------------------------------------------
class Volume : public RadxAppVolume
{
public:

  /**
   * Empty constructor
   */
  Volume(void);

  /**
   * construct base class using these inputs, lookup pointer remains NULL
   * @param[in] parms  Pointer to parameters
   * @param[in] argc  Number of args
   * @param[in] argv  args
   */
  Volume(const RadxAppParms *parms, int argc, char **argv);

  /**
   * Destructor
   */
  virtual ~Volume(void);

  /**
   * Virtual methods from VolumeData base class
   */
  #define FILTALG_DERIVED
  #include <rapmath/VolumeDataVirtualMethods.hh>
  #undef FILTALG_DERIVED

  /**
   * Virtual methods from RadxAppVolume base class
   */
  #include <radar/RadxAppVolumeVirtualMethods.hh>

  /**
   * Set lookup pointer to input, and clear out local state
   * @param[in] lookup  Pointer to store
   */
  void initialize(const RadxAppCircularLookupHandler *lookup);

  /**
   * @return reference to special (non Radx) data
   */
  inline const SpecialUserData &specialRef(void) const {return _special;}

protected:
private:

  static const std::string _volAverageStr;         /**< User unary */
  static const std::string _volAzGradientStateStr; /**< User unary */

  /**
   * Lookup object for 2 dimensional variance
   */
  const RadxAppCircularLookupHandler *_lookup;

  /**
   * Names of derived fields, used to synchronize
   */
  std::vector<std::string> _rayDataNames;

  /**
   * The names/pointers to special data fields, not Radx data
   */
  SpecialUserData _special;

  MathUserData *_volumeAverage(const std::string &name) const;
  MathUserData *_volumeAzGradientState(void) const;
};

#endif
