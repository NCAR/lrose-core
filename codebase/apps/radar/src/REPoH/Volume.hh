/**
 * @file Volume.hh
 * @brief Volume data
 * @class Volume
 * @brief Volume data
 */

#ifndef VOLUME_HH
#define VOLUME_HH

#include "VolumeTrigger.hh"
#include "VolumeMdvInfo.hh"
#include "RepohParms.hh"
#include "KernelOutputs.hh"
#include "AsciiOutputs.hh"
#include "GridFieldsAll.hh"
#include <FiltAlgVirtVol/GriddedData.hh>
#include <FiltAlgVirtVol/VirtVolParms.hh>
// #include <Mdv/MdvxProj.hh>
// #include <Mdv/MdvxRadar.hh>
// #include <Mdv/Mdvx.hh>
#include <rapmath/VolumeData.hh>
#include <ctime>
#include <vector>

class DsUrlTrigger;
class DsMdvx;
class Sweep;

//------------------------------------------------------------------
class Volume : public VolumeData
{
  friend class Sweep;

public:

  /**
   * Constructor
   */
  Volume(const RepohParms &rparms, const VirtVolParms &parms,
	 int argc, char **argv);

  /**
   * Destructor
   */
  virtual ~Volume(void);

  #include <rapmath/VolumeDataVirtualMethods.hh>

  /**
   * Triggering method. Waits till new data triggers a return.
   *
   * This method does almost everything
   *
   * @param[out] t   time that was triggered.
   * @return true if a time was triggered, false for no more triggering.
   */
  bool trigger(time_t &t);

  /**
   * Add all new gridded data fields from a Sweep at a height to the volume
   * as new _data entries
   */
  void addNew(int zIndex, const Sweep &s);

  void output(const time_t &t);
  void clear(void);

protected:
private:


  VolumeTrigger _trigger;
  VolumeMdvInfo _mdvInfo;  
  VirtVolParms _parms;
  RepohParms _repohParms;
  time_t _time;

  // all fields, all heights
  GridFieldsAll _data;

  // Kernel output
  KernelOutputs _kernelOutputs;

  // Ascii output
  AsciiOutputs _asciiOutputs;

  bool _initialInitializeInput(const time_t &t, const UrlSpec &u);
};

#endif
