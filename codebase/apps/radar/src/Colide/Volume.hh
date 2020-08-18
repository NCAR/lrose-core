/**
 * @file Volume.hh
 * @brief Volume data
 * @class Volume
 * @brief Volume data
 */

#ifndef VOLUME_HH
#define VOLUME_HH

#include "Parms.hh"
#include "OldDataHandler.hh"
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
   * Empty
   */
  Volume(void);

  /**
   * Constructor
   * @param[in] rparms
   * @param[in] argc
   * @param[in] argv
   */
  Volume(const Parms &rparms, int argc, char **argv);

  /**
   * Destructor
   */
  virtual ~Volume(void);


  /**
   * Base class virtual methods
   */
  #define FILTALG_DERIVED
  #include <rapmath/VolumeDataVirtualMethods.hh>
  #undef FILTALG_DERIVED

  #include <FiltAlgVirtVol/VirtVolVolumeVirtualMethods.hh>

  /**
   * Trigger a new volume
   * @param[out] t  TIme for the new volume
   * @return true for did trigger
   */
  bool trigger(time_t &t);

protected:
private:

  /**
   * Parameters
   */
  Parms _parms;

  /**
   * Old data handler
   */
  OldData _oldData;

  void _initializeOldInput(const time_t &t,
			   const std::pair<std::string,int> &p);
};

#endif
