/**
 * @file KernelOutputs.hh
 * @brief All the kernel output stuff, one per url
 * @class KernelOutputs
 * @brief All the kernel output stuff, one per url
 */

#ifndef KERNEL_OUTPUTS_H
#define KERNEL_OUTPUTS_H

// #include "KernelOutputsTrigger.hh"
// #include "KernelOutputsMdvInfo.hh"
// #include "RepohParms.hh"
#include "KernelOutput.hh"
#include "Parms.hh"
// #include "AsciiOutput.hh"
// #include "GridFieldsAll.hh"
// #include <FiltAlgVirtVol/GriddedData.hh>
// #include <FiltAlgVirtVol/VirtVolParms.hh>
// // #include <Mdv/MdvxProj.hh>
// // #include <Mdv/MdvxRadar.hh>
// // #include <Mdv/Mdvx.hh>
// #include <rapmath/KernelOutputsData.hh>
// #include <ctime>
#include <vector>

// class DsUrlTrigger;
// class DsMdvx;
// class Sweep;

class VirtVolParms;


//------------------------------------------------------------------
class KernelOutputs 
{

public:

  /**
   * Constructor
   */
  KernelOutputs(void);

  /**
   * Destructor
   */
  virtual ~KernelOutputs(void);
  void clear(void);

  bool initialize(const Parms &parms, int nz);
  void output(const time_t &t, const MdvxProj &proj) const;
  KernelOutput *refToKernelOutput(const std::string &name, bool suppressWarn);
  
protected:
private:


  // output Kernel stuff, one per url, pointers so we can
  // communicate with Sweep class
  std::vector<KernelOutput *> _kernelOutput;

  bool _setupKernelOutput(const RepohParams::Kernel_output_t &p,
			  const VirtVolParms &vparms, int nz);
  void _clear(void);
};

#endif
