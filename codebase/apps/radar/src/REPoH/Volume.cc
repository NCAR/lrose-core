/**
 * @file Volume.cc
 */

//------------------------------------------------------------------
#include "Volume.hh"
#include "Sweep.hh"
#include <toolsa/LogStream.hh>
#include <cstdlib>

//------------------------------------------------------------------
Volume::Volume(const RepohParms &rparms, const VirtVolParms &parms, int argc,
	       char **argv):
  _trigger(parms, argc, argv), _parms(parms), _repohParms(rparms)
{
}

//------------------------------------------------------------------
Volume::~Volume(void)
{
}

//------------------------------------------------------------------
bool Volume::trigger(time_t &t)
{
  if (!_trigger.trigger(t))
  {
    LOG(DEBUG) << "no more triggering";
    return false;
  }

  _time = t;


  // try to initialize state using any of the inputs
  bool didInit = false;
  for (size_t i=0; i<_parms._inputs.size(); ++i)
  {
    if (_initialInitializeInput(t, _parms._inputs[i]))
    {
      didInit = true;
      break;
    }
  }
  if (!didInit)
  {
    LOG(ERROR) << "Could not init";
    return false;
  }

  // now load in volume data for each url
  for (size_t i=0; i<_parms._inputs.size(); ++i)
  {
    if (!_mdvInfo.initializeInput(t, _parms._inputs[i], _repohParms._main,
				  _data))
    {
      return false;
    }
  }

  if (!_kernelOutputs.initialize(_repohParms._main, _parms, _mdvInfo.nz()))
  {
    return false;
  }
  
  if (!_asciiOutputs.initialize(_repohParms._main, _parms, t))
  {
    return false;
  }

  return true;
}

//------------------------------------------------------------------
void Volume::clear(void)
{
  _data.clear();
}

//------------------------------------------------------------------
void Volume::addNew(int zIndex, const Sweep &s)
{
  const GridFields &newD = s.derivedDataRef();
  for (size_t i=0; i<newD.size(); ++i)
  {
    string name = newD[i].getName();
    string ename;
    if (_parms.outputInternal2ExternalName(name, ename))
    {
      bool exists = _data.fieldExists(name, zIndex);
      if (!exists)
      {
	LOG(DEBUG) << "Adding field " << name << " to state, z=" << zIndex;
	_data.addField(newD[i], zIndex);
      }
    }
  }
}

//------------------------------------------------------------------
void Volume::output(const time_t &t)
{
  // for each output url
  for (size_t i=0; i<_parms._outputs.size(); ++i)
  {
    _mdvInfo.outputToUrl(t, _parms._outputs[i], _data);
  }

  _kernelOutputs.output(t, _mdvInfo.proj());
}

//------------------------------------------------------------------
int Volume::numProcessingNodes(bool twoD) const
{
  if (twoD)
  {
    return (int)(_data.size());
  }
  else
  {
    return 0;
  }
}

//------------------------------------------------------------------
MathData *Volume::initializeProcessingNode(int index, bool twoD) const
{
  MathData *ret = NULL;
  if (twoD)
  {
    Sweep *sweep = new Sweep(*this, index, _mdvInfo.vlevel(index));
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
  return false;
}


//------------------------------------------------------------------
bool Volume::_initialInitializeInput(const time_t &t, const UrlSpec &u)
{
  if (_mdvInfo.initialInitializeInput(t, u, _repohParms._main))
  {
    _data.initialize(_mdvInfo.nz());
    return true;
  }
  else
  {
    return false;
  }
}
