/**
 * @file Volume.cc
 */

//------------------------------------------------------------------
#include "Volume.hh"
#include "Sweep.hh"
#include <toolsa/LogStream.hh>
#include <cstdlib>

//------------------------------------------------------------------
Volume::Volume(void) : VolumeMdv()
{
}

//------------------------------------------------------------------
Volume::Volume(const Parms &parms, int argc, char **argv):
  VolumeMdv(&parms, argc, argv),
  _parms(parms)
{
}

//------------------------------------------------------------------
Volume::~Volume(void)
{
}

//------------------------------------------------------------------
std::vector<std::pair<std::string, std::string> >
Volume::userUnaryOperators(void) const
{
  std::vector<std::pair<std::string, std::string> > ret;
  return ret;
}

//------------------------------------------------------------------
bool Volume::trigger(time_t &t)
{
  if (!triggerMdv(t))
  {
    return false;
  }
  if (!_kernelOutputs.initialize(_parms, _nz))
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
  const SweepMdv *smdv = (const SweepMdv *)s;
  addNewMdv(zIndex, *smdv);
}
//------------------------------------------------------------------
void Volume::repohOutput(const time_t &t)
{
  // // for each output url
  // for (size_t i=0; i<_parms._outputs.size(); ++i)
  // {
  //   _mdvInfo.outputToUrl(t, _parms._outputs[i], _data);
  // }

  VolumeMdv::output(t);
  _kernelOutputs.output(t, _proj);
}

//------------------------------------------------------------------
MathData *Volume::initializeProcessingNode(int index, bool twoD) const
{
  MathData *ret = NULL;
  if (twoD)
  {
    Sweep *sweep = new Sweep(*this, index, _vlevel[index]);
    ret = (MathData *)sweep;
  }
  else
  {
    LOG(ERROR) << "One dimensional not implemented";
  }
  return ret;
}

//------------------------------------------------------------------
bool Volume::synchUserDefinedInputs(const std::string &userKey,
				    const std::vector<std::string> &names)
{
  return false;
}


//------------------------------------------------------------------
MathUserData *Volume::processUserVolumeFunction(const UnaryNode &p)
{
  return NULL;
}

//------------------------------------------------------------------
bool Volume::storeMathUserData(const std::string &name, MathUserData *v)
{
  return storeMathUserDataMdv(name, v);
}
