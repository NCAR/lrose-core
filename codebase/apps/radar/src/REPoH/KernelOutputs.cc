/**
 * @file KernelOutputs.cc
 */
#include "KernelOutputs.hh"
#include "RepohParams.hh"
#include <FiltAlgVirtVol/VirtVolParms.hh>
#include <toolsa/LogStream.hh>

KernelOutputs::KernelOutputs()
{
}

KernelOutputs::~KernelOutputs()
{
}

void KernelOutputs::clear(void)
{
  _clear();
}

bool KernelOutputs::initialize(const Parms &parms, int nz)
{
  _clear();

  // set up kernel output for each URL
  for (int i=0; i<parms.kernel_output_n; ++i)
  {
    if (!_setupKernelOutput(parms._kernel_output[i], parms, nz))
    {
      return false;
    }
  }
  return true;
}

void KernelOutputs::output(const time_t &t, const MdvxProj &proj) const
{
  for (size_t i=0; i<_kernelOutput.size(); ++i)
  {
    LOG(DEBUG) << "Output Kernel information " << i << " of "
	       << _kernelOutput.size();
    _kernelOutput[i]->output(t, proj);
  }
}

//------------------------------------------------------------------
KernelOutput *KernelOutputs::refToKernelOutput(const std::string &name,
					       bool suppressWarn)
{
  for (size_t i=0; i<_kernelOutput.size(); ++i)
  {
    if (_kernelOutput[i]->nameMatch(name))
    {
      return _kernelOutput[i];
    }
  }
  if (!suppressWarn)
  {
    printf("ERROR retrieving kernel output data for %s\n", name.c_str());
  }
  return NULL;
}


bool KernelOutputs::_setupKernelOutput(const RepohParams::Kernel_output_t &p,
				       const VirtVolParms &vparms, int nz)
{
  for (size_t j=0; j<vparms._outputUrl.size(); ++j)
  {
    if (vparms._outputUrl[j].nameMatch(p.name))
    {
      if (vparms._outputUrl[j].url_type != UrlParams::DATABASE)
      {
	LOG(ERROR) << "Inconsistent use of kernel output data";
	return false;
      }
      _kernelOutput.push_back(new
			      KernelOutput(p.name, nz,
					   p.filtered, p.outside,
					   vparms._outputUrl[j].url));
      return true;
    }
  }

  LOG(ERROR) << "Did not find a match for " << p.name;
  return false;
}
    

void KernelOutputs::_clear(void)
{
  for (size_t i=0; i<_kernelOutput.size(); ++i)
  {
    delete _kernelOutput[i];
  }
  _kernelOutput.clear();
}

