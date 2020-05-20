/**
 * @file Volume.hh
 * @brief Volume data
 * @class Volume
 * @brief Volume data
 */

#ifndef VOLUME_HH
#define VOLUME_HH

#include "Parms.hh"
#include "TemplateLookupMgr.hh"
#include <FiltAlgVirtVol/VirtVolVolume.hh>
#include <ctime>
#include <vector>
class Sweep;

//------------------------------------------------------------------
class Volume : public VirtVolVolume
{
  friend class Sweep;
public:

  Volume(void);

  /**
   * Constructor
   */
  Volume(const Parms &parms, int argc, char **argv);

  /**
   * Destructor
   */
  virtual ~Volume(void);


  #define FILTALG_DERIVED
  #include <rapmath/VolumeDataVirtualMethods.hh>
  #undef FILTALG_DERIVED

  #include <FiltAlgVirtVol/VirtVolVolumeVirtualMethods.hh>

  /**
   * Triggering method. Waits till new data triggers a return.
   *
   * @param[out] t   time that was triggered.
   *
   * @return true if a time was triggered, false for no more triggering.
   */
  bool trigger(time_t &t);

  
protected:
private:

  static const std::string _verticalConsistencyStr;
  static const std::string _parmsTemplateStr;

  Parms _parms;

  std::vector<TemplateLookupMgr> _templates;

  MathUserData *_computeVerticalConsistency(const std::string &fieldName);
  MathUserData *_computeParmsTemplate(const std::string &xstr,
				      const std::string &ystr,
				      const std::string &yoffsetStr);

};

#endif
