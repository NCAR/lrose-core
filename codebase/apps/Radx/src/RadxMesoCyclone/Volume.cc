/**
 * @file Volume.cc
 */

//------------------------------------------------------------------
#include "Volume.hh"
#include "Sweep.hh"
#include <FiltAlgVirtVol/GridUserData.hh>
#include <rapmath/UnaryNode.hh>
#include <rapmath/FunctionDef.hh>
#include <toolsa/LogMsgStreamInit.hh>
#include <toolsa/LogStream.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/pmu.h>
#include <toolsa/port.h>
#include <cstdlib>

const std::string Volume::_verticalConsistencyStr = "VerticalConsistency";
const std::string Volume::_parmsTemplateStr = "ParmsTemplate";
  
//------------------------------------------------------------------
Volume::Volume(void) :  VirtVolVolume()
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
  ret.push_back(FunctionDef(_verticalConsistencyStr, "M", "field", ""));
  ret.push_back(FunctionDef(_parmsTemplateStr, "M", "x, y, offset", ""));
  return ret;
}

//------------------------------------------------------------------
bool Volume::trigger(time_t &t)
{
  if (!triggerVirtVol(t))
  {
    return false;
  }
  _templates.clear();
  for (int i=0; i<_parms.meso_template_n; ++i)
  {
    TemplateLookupMgr t(_parms._meso_template[i].x, _parms._meso_template[i].y,
			_parms._meso_template[i].yOffset,
			*this);
    _templates.push_back(t);
  }

  return true;
}

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

bool Volume::virtVolSynchUserInputs(const std::string &userKey,
				    const std::vector<std::string> &names)
{
  if (userKey == _verticalConsistencyStr)
  {
    // expect one arg
    if (names.size() != 1)
    {
      LOG(ERROR) << "Expect one arg for " << _verticalConsistencyStr;
      return false;
    }
    else
    {
      return true;
    }
  }
  else if (userKey == _parmsTemplateStr)
  {
    // // expect 3 args
    // if (names.size() != 3)
    // {
    //   LOG(ERROR) << "Expect 3 args for " << _parmsTemplateStr;
    //   return false;
    // }
    // else
    // {
      return true;
    // }
  }
  else
  {
    LOG(ERROR) << "Unknown keyword " << userKey;
    return false;
  }
}

MathUserData *Volume::processUserVolumeFunction(const UnaryNode &p)
{
    // pull out the keyword
  string keyword;
  if (!p.getUserUnaryKeyword(keyword))
  {
    return NULL;
  }
  vector<string> args = p.getUnaryNodeArgStrings();
  
  if (keyword == _verticalConsistencyStr)
  {
    // expect one string argument which is either Prt or NSamples
    if (args.size() != 1)
    {
      LOG(ERROR) << "Wrong number of args want one got " << args.size();
      return NULL;
    }
    return _computeVerticalConsistency(args[0]);
  }
  else if (keyword == _parmsTemplateStr)
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
    LOG(ERROR) << "Keyword is invalid " << keyword;
    return NULL;
  }
}

void Volume::addNew(int zIndex, const MathData *s)
{
  const VirtVolSweep *smdv = (const VirtVolSweep *)s;
  addNewSweep(zIndex, *smdv);
}

bool Volume::storeMathUserData(const std::string &name, MathUserData *v)
{
  return storeMathUserDataVirtVol(name, v);
}

#ifdef NEEDTOFIXTHIS
  // expect this is the vertical consistency data, it is all we have
  // WEAK!
  // make a copy of the field and rename
  GridUserData *u = (GridUserData *)v;
  GriddedData g(*u);
  g.setName(name);
  // replicate it as a 3d field and store.
  for (int i=0; i<VirtVolVolume::nz(); ++i)
  {
    VirtVolVolume::addNewGrid(i, g);
  }
  
  // store this because we have to, but it is a throwaway.
  // again, bad design
  return storeMathUserDataVirtVol(name, v);
#endif

//------------------------------------------------------------------
MathUserData *Volume::
_computeVerticalConsistency(const std::string &fieldName)
{
  std::vector<GriddedData> f = getField3d(fieldName);
  if (f.empty())
  {
    LOG(ERROR) << "No data";
    return NULL;
  }

  Grid2d vc(f[0]);
  vc.setAllMissing();
  // this will be an interest image with mapping from 0 (not consistent)
  // to 1 (completely consistent)

  for (int i=0; i<vc.getNdata(); ++i)
  {
    double ngood=0, nbad=0, min=0, max=0;
    bool first = true;
    for (size_t j=0; j<f.size(); ++j)
    {
      double v;
      if (f[j].getValue(i, v))
      {
	if (first)
	{
	  first = false;
	  min = max = v;
	}
	else
	{
	  if (v < min) min = v;
	  if (v > max) max = v;
	}
	++ngood;
      }
      else
      {
	++nbad;
      }
    }
    // come up with a score based on results
    if (ngood > 0)
    {
      double v = ngood/(ngood+nbad);
      // later do something with max and min
      vc.setValue(i, v);
    }
  }
  // take this and create a new output field, same name for now
  GridUserData *g = new GridUserData(vc, fieldName);
  return (MathUserData *)g;
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
