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
#include <map>

class Parms;
class RadxTimeMedian;

//------------------------------------------------------------------
class Volume : public RadxAppVolume
{
  friend class RayData1;

public:

  /**
   * Empty
   */
  Volume(void);

  /**
   * @param[in] parms
   * @param[in] argc
   * @param[in] argv
   */
  Volume(const Parms *parms, int argc, char **argv);

  /**
   * Destructor
   */
  virtual ~Volume(void);

  /**
   * Virtual methods from base class
   */
  #define FILTALG_DERIVED
  #include <rapmath/VolumeDataVirtualMethods.hh>
  #undef FILTALG_DERIVED

  /**
   * Virtual methods for radx volumes
   */
  #include <radar/RadxAppVolumeVirtualMethods.hh>

  /**
   * Initialize, done for each volume
   */
  void initialize(void);

  /**
   * Final processing done on last volume
   */
  void processLast(void);

  /**
   * Copy the base class RadxAppVolume into input object,
   * but not the derived members. The derived members are set to empty
   *
   * @param[in] templateVolume  The object to copy into from local 
   */
  void copy(Volume &templateVolume) const;

  /**
   * Copy the input base class RadxAppVolume into local object,
   * but not the derived members. The derived members are untouched.
   *
   * @param[in] templateVolume  The object to copy from into local 
   */
  void setFrom(const Volume &templateVolume);

  /**
   * @return pointer to the alg object
   */
  inline RadxTimeMedian *algPtr(void) {return _p;}
  /**
   * @return pointer to the alg object
   */
  inline RadxTimeMedian *algPtrConst(void) const {return _p;}

  /**
   * @return reference to the special (non Radx) data
   */
  inline const SpecialUserData &specialRef(void) const {return _special;}

protected:
private:

  SpecialUserData _special;      /**< Special (non Radx) data */
  RadxTimeMedian *_p;            /**< Algorithm pointer */

  MathUserData *_volumeInit(void);
  MathUserData *_volumeFinish(void);
  
};

#endif
