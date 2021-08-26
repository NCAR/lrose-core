/**
 * @file Volume.cc
 */

//------------------------------------------------------------------
#include "Volume.hh"
#include "Sweep.hh"
#include <rapmath/UnaryNode.hh>
#include <rapmath/FunctionDef.hh>
#include <toolsa/LogStream.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/pmu.h>
#include <toolsa/port.h>
#include <cstdlib>

//------------------------------------------------------------------
Volume::Volume(void) :
  VirtVolVolume()
{
}

//------------------------------------------------------------------
Volume::Volume(const Parms &parms, int argc, char **argv):
  VirtVolVolume(&parms, argc, argv), _parms(parms)
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
  // for each old data input, load in older data now
  _oldData = OldData();
  for (size_t i=0; i<_parms._oldData.size(); ++i)
  {
    _initializeOldInput(t, _parms._oldData[i]);
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
bool Volume::storeMathUserData(const std::string &name, MathUserData *v)
{
  return storeMathUserDataVirtVol(name, v);
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
bool
Volume::virtVolSynchUserInputs(const std::string &userKey,
			       const std::vector<std::string> &names)
{
   LOG(ERROR) << "Unknown keyword " << userKey;
   return false;
}

//------------------------------------------------------------------
MathUserData *Volume::processUserVolumeFunction(const UnaryNode &p)
{
  return processVirtVolUserVolumeFunction(p);
}

//------------------------------------------------------------------
void Volume::_initializeOldInput(const time_t &t,
				 const std::pair<std::string,int> &p)
{
  string url = _parms.matchingOutputUrl(p.first);
  if (url.empty())
  {
    LOG(ERROR) << "No matching URL for old input " << p.first;
    return;
  }
  int maxBack = p.second;
  _oldData.addField(t, p.first, url, maxBack, _parms);
}  
