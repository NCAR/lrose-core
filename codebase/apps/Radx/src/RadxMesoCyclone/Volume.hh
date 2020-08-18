/**
 * @file Volume.hh
 * @brief Volume data
 * @class Volume
 * @brief Volume data
 */

#ifndef VOLUME_HH
#define VOLUME_HH

#include "Parms.hh"
#include <FiltAlgVirtVol/VirtVolVolume.hh>
#include <ctime>
#include <vector>
class Sweep;

//------------------------------------------------------------------
class Volume : public VirtVolVolume
{
  friend class Sweep;
public:

  /**
   * Constructor. Empty
   */
  Volume(void);

  /**
   * Constructor
   * @param[in] parms  App params
   * @param[in] argc  Command args
   * @param[in] argv  Command args
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

  /**
   * Output including 2d fields and products
   * @param[in] t  Time
   */
  void mesoOutput(const time_t &t);
  
protected:
private:

  /**
   * String for a special function
   */
  static const std::string _parmsTemplateStr;

  /**
   * Params for the app
   */
  Parms _parms;

  /**
   * The special URL for shapes
   */
  std::string _shapesUrl;

  /**
   * THe name of the shape special data
   */
  std::string _shapesName;

 MathUserData *_computeParmsTemplate(const std::string &xstr,
				      const std::string &ystr,
				      const std::string &yoffsetStr);

};

#endif
