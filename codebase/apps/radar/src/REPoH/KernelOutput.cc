/**
 * @file KernelOutput.cc
 */
#include "KernelOutput.hh"
#include <Mdv/Mdvx.hh>
#include <Mdv/MdvxProj.hh>
#include <toolsa/LogStream.hh>


//---------------------------------------------------------------------------
KernelOutput::KernelOutput(const std::string &name, int numVlevelIndex,
			   bool filtered, bool outside,
			   const std::string &url) :
  _name(name), _filtered(filtered), _outside(outside),
  _numVlevel(numVlevelIndex)
{
  for (int i=0; i<numVlevelIndex; ++i)
  {
    // create the subdir and add that to the urls and add an empty kernels
    char buf[100];
    sprintf(buf, "/%02d", i);
    string s = buf;
    _url.push_back(url + s);
    _kernels.push_back(Kernels());
  }
}

//---------------------------------------------------------------------------
KernelOutput::~KernelOutput(void)
{
}

//---------------------------------------------------------------------------
bool KernelOutput::getFloat(double &v) const
{
  return false;
}

//---------------------------------------------------------------------------
void KernelOutput::storeKernel(int vlevelIndex, const Kernels &k)
{
  _kernels[vlevelIndex] = k;
}

//---------------------------------------------------------------------------
void KernelOutput::output(const time_t &t, const MdvxProj &proj) const
{
  for (size_t i=0; i<_kernels.size(); ++i)
  {
    LOG(DEBUG) << "Output kernel " << i << " of " << _kernels.size()
	       << " url = " << _url[i];
    _kernels[i].writeGenpoly(_url[i], t, _outside, proj);
  }
}
