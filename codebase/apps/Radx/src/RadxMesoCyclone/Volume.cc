/**
 * @file Volume.cc
 */

//------------------------------------------------------------------
#include "Volume.hh"
#include "Sweep.hh"
#include "TemplateLookupMgr.hh"
#include <FiltAlgVirtVol/GridUserData.hh>
#include <FiltAlgVirtVol/PolarCircularTemplate.hh>
#include <FiltAlgVirtVol/ShapePolygons.hh>
#include <euclid/GridAlgs.hh>
#include <rapmath/UnaryNode.hh>
#include <rapmath/FunctionDef.hh>
#include <toolsa/LogMsgStreamInit.hh>
#include <toolsa/LogStream.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/pmu.h>
#include <toolsa/port.h>
#include <cstdlib>

const std::string Volume::_parmsTemplateStr = "ParmsTemplate";
  
//------------------------------------------------------------------
Volume::Volume(void) :  VirtVolVolume()
{
}

//------------------------------------------------------------------
Volume::Volume(const Parms &parms, int argc, char **argv):
  VirtVolVolume(&parms, argc, argv),  _parms(parms)
{
  _shapesUrl = _parms.shapes_url;
  _shapesName = _parms.shapes_name;
}

//------------------------------------------------------------------
Volume::~Volume(void)
{
}

//------------------------------------------------------------------
std::vector<FunctionDef> Volume::userUnaryOperators(void) const
{
  std::vector<FunctionDef> ret;
  ret.push_back(FunctionDef(_parmsTemplateStr, "M", "x, y, offset",
			    "Create templates, each of which is two boxes of dimension x by y km centered offset km on either side of a point.  At each x the template has different content due to azimuthal (y) spacing getting bigger further away so less point are in the template"));
  return ret;
}

//------------------------------------------------------------------
bool Volume::trigger(time_t &t)
{
  if (!triggerVirtVol(t))
  {
    return false;
  }
  return true;
}

//---------------------------------------------------------------------
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

//------------------------------------------------------------------------
bool Volume::virtVolSynchUserInputs(const std::string &userKey,
				    const std::vector<std::string> &names)
{
  if (userKey == _parmsTemplateStr)
  {
    return true;
  }
  else
  {
    LOG(ERROR) << "Unknown keyword " << userKey;
    return false;
  }
}

//------------------------------------------------------------------------
MathUserData *Volume::processUserVolumeFunction(const UnaryNode &p)
{
    // pull out the keyword
  string keyword;
  if (!p.getUserUnaryKeyword(keyword))
  {
    return NULL;
  }
  vector<string> args = p.getUnaryNodeArgStrings();
  
  if (keyword == _parmsTemplateStr)
  {
    // expect 3 double arguments
    if (args.size() != 3)
    {
      LOG(ERROR) << "Wrong number of args want 3 got " << args.size();
      return NULL;
    }
    return _computeParmsTemplate(args[0], args[1], args[2]);
  }
  else
  {
    return processVirtVolUserVolumeFunction(p);
  }
}

//------------------------------------------------------------------------
void Volume::addNew(int zIndex, const MathData *s)
{
  const VirtVolSweep *smdv = (const VirtVolSweep *)s;
  addNewSweep(zIndex, *smdv);
}

//------------------------------------------------------------------------
void Volume::mesoOutput(const time_t &t)
{
  VirtVolVolume::output(t);
  VirtVolVolume::specialOutput2d(t, _parms._output2dUrl);

  MathUserData *u = _special->matchingDataPtr(_shapesName);
  if (u != NULL)
  {
    ShapePolygons *shapes = (ShapePolygons *)u;
    shapes->output(t, _parms.shapes_expire_seconds, _shapesUrl);
  }
  else
  {
    LOG(ERROR) << "No match found for " << _shapesName;
  }
}

//------------------------------------------------------------------------
bool Volume::storeMathUserData(const std::string &name, MathUserData *v)
{
  return storeMathUserDataVirtVol(name, v);
}

//------------------------------------------------------------------
MathUserData *Volume::
_computeParmsTemplate(const std::string &xstr, const std::string &ystr,
		      const std::string &yoffsetStr)
{
  int x, y, yoff;
  if (sscanf(xstr.c_str(), "%d", &x) != 1)
  {
    LOG(ERROR) << "Cannot scan " << xstr << " as an int";
    return NULL;
  }
  if (sscanf(ystr.c_str(), "%d", &y) != 1)
  {
    LOG(ERROR) << "Cannot scan " << ystr << " as an int";
    return NULL;
  }
  if (sscanf(yoffsetStr.c_str(), "%d", &yoff) != 1)
  {
    LOG(ERROR) << "Cannot scan " << yoffsetStr << " as an int";
    return NULL;
  }
  
  TemplateLookupMgr *t = new TemplateLookupMgr(x, y, yoff, *this);
  return (MathUserData *)t;
}

