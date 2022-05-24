#include "Volume.hh"
#include "RayData1.hh"
#include "AzGradientStateSpecialData.hh"

#include <radar/RadxAppCircularLookupHandler.hh>
#include <radar/RadxAppSweepDataSimple.hh>
#include <radar/RadxApp.hh>
#include <rapmath/FunctionDef.hh>
#include <rapmath/UnaryNode.hh>
#include <rapmath/FloatUserData.hh>
#include <toolsa/LogStream.hh>
#include <algorithm>

const std::string Volume::_volAverageStr = "VolAverage";
const std::string Volume::_volAzGradientStateStr = "VolAzGradientState";

//------------------------------------------------------------------
Volume::Volume(void) :  RadxAppVolume(), _lookup(NULL)
{
}

//------------------------------------------------------------------
Volume::Volume(const RadxAppParms *parms, int argc, char **argv):
  RadxAppVolume(parms, argc, argv),  _lookup(NULL)
{
}

//------------------------------------------------------------------
Volume::~Volume(void)
{
  _rayDataNames.clear();
}

//------------------------------------------------------------------
std::vector<FunctionDef> Volume::userUnaryOperators(void) const
{
  std::vector<FunctionDef> ret;
  ret.push_back(FunctionDef(_volAverageStr, "v", "Prt or Nsamples",
			    "Average value over the entire volume"));
  ret.push_back(FunctionDef(_volAzGradientStateStr, "v", "",
			    "Creates the internal state used in AzGradient filter, based on the entire volume"));
  return ret;
}

//------------------------------------------------------------------
void Volume::initialize(const RadxAppCircularLookupHandler *lookup)
{
  _lookup = lookup;
  _special = SpecialUserData();
  _rayDataNames.clear();
}

//------------------------------------------------------------------
bool Volume::needToSynch(const std::string &userKey) const
{
  if (userKey == _volAverageStr)
  {
    // the input name is not something to synch to
    return false;
  }
  else if (userKey == _volAzGradientStateStr)
  {
    // there should be no args so not here
    return false;
  }

  // everything else has inputs
  return true;
}

//------------------------------------------------------------------
bool Volume::hasData(const std::string &userKey,
		      const std::string &name, bool suppressWarn)
{
  if (userKey == _volAverageStr)
  {
    // special case
    LOG(DEBUG) << "Special case? ";
    return false;
  }
  else if (userKey == _volAzGradientStateStr)
  {
    // there should be no args so not here
    LOG(ERROR) << "should not be here";
    return false;
  }

  // can't pull out of state, not in _ray and not in _data
  if (!suppressWarn)
  {
    LOG(ERROR) << "Unknown key " << userKey;
  }
  return NULL;
}    


//------------------------------------------------------------------
// virtual
MathData *Volume::initializeProcessingNode(int index, bool twoD) const
{
  MathData *ret=NULL;
  if (twoD)
  {
    RadxAppSweepDataSimple *rd =
      new RadxAppSweepDataSimple(*this, index, _lookup);
    ret = (MathData *)rd;
  }
  else
  {
    RayData1 *rd = new RayData1(*this, index);
    ret = (MathData *)rd;
  }
  return ret;
}

//------------------------------------------------------------------
// virtual
MathUserData *Volume::processUserVolumeFunction(const UnaryNode &p)
{
  // pull out the keyword
  string keyword;
  if (!p.getUserUnaryKeyword(keyword))
  {
    return NULL;
  }
  vector<string> args = p.getUnaryNodeArgStrings();
  
  if (keyword == _volAverageStr)
  {
    // expect one string argument which is either Prt or NSamples
    if (args.size() != 1)
    {
      LOG(ERROR) << "Wrong number of args want one got " << args.size();
      return NULL;
    }
    return _volumeAverage(args[0]);
  }
  else if (keyword == _volAzGradientStateStr)
  {
    // no arguments, can just go for it
    return _volumeAzGradientState();
  }
  else
  {
    LOG(ERROR) << "Keyword is invalid " << keyword;
    return NULL;
  }
}

//------------------------------------------------------------------
void Volume::addNew(int zIndex, const MathData *s)
{
  // this does nothing in radx format
}

//------------------------------------------------------------------
// virtual
bool Volume::storeMathUserData(const std::string &name, MathUserData *s)
{
  return _special.store(name, s);
}

//------------------------------------------------------------------
MathUserData *Volume::_volumeAverage(const std::string &name) const
{
  double mean = 0.0;
  if (name == "Prt")
  {
    for (size_t i=0; i<_rays->size(); ++i)
    {
      mean += (*_rays)[i]->getPrtSec();
    }
  }
  else if (name == "NSamples")
  {
    for (size_t i=0; i<_rays->size(); ++i)
    {
      mean += (*_rays)[i]->getNSamples();
    }
  }
  else
  {
    LOG(ERROR) << "unknown volume variable " << name << " want Prt or NSamples";
    return NULL;
  }
  if (!_rays->empty())
  {
    double v = mean/static_cast<double>(_rays->size());
    FloatUserData *ret = new FloatUserData(v);
    return (MathUserData *)ret;
  }
  else
  {
    LOG(ERROR) << " no data";
    return NULL;
  }
}

//------------------------------------------------------------------
MathUserData *Volume::_volumeAzGradientState(void) const
{
  AzGradientStateSpecialData *a = new AzGradientStateSpecialData(_vol);
  return (MathUserData *)a;
}

