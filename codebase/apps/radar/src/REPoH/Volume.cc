/**
 * @file Volume.cc
 */

//------------------------------------------------------------------
#include "Volume.hh"
#include "Sweep.hh"
#include <rapmath/FunctionDef.hh>
#include <toolsa/LogStream.hh>
#include <cstdlib>

//------------------------------------------------------------------
Volume::Volume(void) : VirtVolVolume()
{
}

//------------------------------------------------------------------
Volume::Volume(const Parms &parms, int argc, char **argv):
  VirtVolVolume(&parms, argc, argv),  _parms(parms)
{
}

//------------------------------------------------------------------
Volume::~Volume(void)
{
}

//------------------------------------------------------------------
std::vector<FunctionDef> Volume::userUnaryOperators(void) const
{
  std::vector<FunctionDef> ret;
  return ret;
}

//------------------------------------------------------------------
bool Volume::trigger(time_t &t)
{
  if (!triggerVirtVol(t))
  {
    return false;
  }
  if (!_kernelOutputs.initialize(_parms, nz()))
  {
    return false;
  }
  
  if (!_asciiOutputs.initialize(_parms, t))
  {
    return false;
  }

  return true;
}

//------------------------------------------------------------------
void Volume::addNew(int zIndex, const MathData *s)
{
  const VirtVolSweep *smdv = (const VirtVolSweep *)s;
  addNewSweep(zIndex, *smdv);
}
//------------------------------------------------------------------
void Volume::repohOutput(const time_t &t)
{
  VirtVolVolume::output(t);
  _kernelOutputs.output(t, proj());
}

//------------------------------------------------------------------
MathData *Volume::initializeProcessingNode(int index, bool twoD) const
{
  MathData *ret = NULL;
  if (twoD)
  {
    Sweep *sweep = new Sweep(*this, index, getIthVlevel(index));
    ret = (MathData *)sweep;
  }
  else
  {
    LOG(ERROR) << "One dimensional not implemented";
  }
  return ret;
}

//------------------------------------------------------------------
bool Volume::virtVolSynchUserInputs(const std::string &userKey,
				    const std::vector<std::string> &names)
{
  return false;
}


//------------------------------------------------------------------
MathUserData *Volume::processUserVolumeFunction(const UnaryNode &p)
{
  return processVirtVolUserVolumeFunction(p);
}

//------------------------------------------------------------------
bool Volume::storeMathUserData(const std::string &name, MathUserData *v)
{
  return storeMathUserDataVirtVol(name, v);
}
