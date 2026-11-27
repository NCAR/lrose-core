/**
 * @file VolumeBase.hh 
 * @brief Volume data handler for Radx data, contains RadxAppVolume
 *        A base class used in several derived contexts.
 *
 * @class VolumeBase
 * @brief Volume data handler for Radx data, contains RadxAppVolume
 *        A base class used in several derived contexts.
 */

#ifndef VOLUME_BASE_H
#define VOLUME_BASE_H

#include "Parms.hh"
#include <radar/RadxAppVolume.hh>
#include <rapmath/SpecialUserData.hh>
#include <vector>
#include <string>

class RadxPersistentClutter;

//------------------------------------------------------------------
class VolumeBase : public RadxAppVolume
{
  friend class RayData1;

public:

  /**
   * Empty
   */
  VolumeBase(void);

  /**
   * @param[in] parms
   * @param[in] argc
   * @param[in] argv
   */
  VolumeBase(const Parms *parms, int argc, char **argv);

  /**
   * Destructor
   */
  virtual ~VolumeBase(void);

  /**
   * Set all the values, same as operator=
   * @param[in] r
   */
  void setV(const VolumeBase &r);

  /**
   * Virtual methods from base class
   */
  #define FILTALG_DERIVED
  #include <rapmath/VolumeDataVirtualMethods.hh>
  #undef FILTALG_DERIVED

  #include <radar/RadxAppVolumeVirtualMethods.hh>

  /**
   * Initialize the volume, done after triggering
   */
  void initialize(void);

  /**
   * @return true if state indicates convergence
   */
  inline bool converged(void) const {return _converged;}

  /**
   * @return true if state indicates the processing is complete
   */
  inline bool done(void) const {return _state == DONE;}

  /**
   * @return true if state indicates volume should be written out
   */
  inline bool doWrite(void) const {return _doWrite;}

  /**
   * @return pointer to special (non Radx) data
   */
  inline const SpecialUserData &specialRef(void) const {return _special;}

  /**
   * Pure virtual method to initialize a volume prior to ray specific 
   * algorithmic steps
   * @return pointer to the status, which is a downcast StatusUserData ptr
   */
  virtual MathUserData *volumeInit(void) = 0;

  /**
   * Pure virtual method to finalize a volume after all the ray specific 
   * algorithmic steps
   * @return pointer to the status, which is a downcast StatusUserData ptr
   */
  virtual MathUserData *volumeFinish(void) = 0;

  /**
   * Pure virtual method to point to the algorithm object
   * @return pointer to the RadxPersistentClutter base class
   */
  virtual RadxPersistentClutter *algPtr(void) const = 0;

protected:

  /**
   * processing types
   */
  typedef enum {FIRST, FIRST_PASS, FIRST_SECOND_PASS, SECOND_PASS,DONE} State_t;


  const Parms *_parms;  /**< Parameters pointer */
  State_t _state;       /**< current state */
  bool _converged;      /**< True if converged */
  bool _doWrite;        /**< True if volume can be written */

  /**
   * The names/pointers to special data fields
   */
  SpecialUserData _special;

private:

};

#endif
