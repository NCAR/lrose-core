/**
 * @file Volume.hh
 * @brief Volume data
 * @class Volume
 * @brief Volume data
 */

#ifndef VOLUME_HH
#define VOLUME_HH

#include "Parms.hh"
#include "KernelOutputs.hh"
#include "AsciiOutputs.hh"
#include <FiltAlgVirtVol/VirtVolVolume.hh>
#include <ctime>
#include <vector>

//------------------------------------------------------------------
class Volume : public VirtVolVolume
{
  friend class Sweep;

public:

  Volume(void);

  /**
   * Constructor
   */
  Volume(const Parms &rparms, int argc, char **argv);

  /**
   * Destructor
   */
  virtual ~Volume(void);

  #define FILTALG_DERIVED
  #include <rapmath/VolumeDataVirtualMethods.hh>
  #undef FILTALG_DERIVED

#include <FiltAlgVirtVol/VirtVolVolumeVirtualMethods.hh>

  // virtual bool synchUserInputsForMdv(const std::string &userKey,
  // 				     const std::vector<std::string> &names);
  /**
   * Triggering method. Waits till new data triggers a return.
   *
   * This method does almost everything
   *
   * @param[out] t   time that was triggered.
   * @return true if a time was triggered, false for no more triggering.
   */
  bool trigger(time_t &t);

  void repohOutput(const time_t &t);

protected:
private:

  Parms _parms;

  // Kernel output
  KernelOutputs _kernelOutputs;

  // Ascii output
  AsciiOutputs _asciiOutputs;

};

#endif
