#include "RayData1.hh"
#include "RayData.hh"
#include "AzGradientFilter.hh"
#include "QscaleFilter.hh"
#include "Clutter2dQualFilter.hh"
#include "Special0Filter.hh"
#include "Special1Filter.hh"
#include "FIRFilter.hh"
#include "Variance1dFilter.hh"
#include "AzGradientStateSpecialData.hh"
#include "RayLoopData.hh"
#include <rapmath/ProcessingNode.hh>
#include <rapmath/UnaryNode.hh>
#include <rapmath/FloatUserData.hh>
#include <rapmath/MathUserData.hh>
#include <radar/RadxApp.hh>
#include <Radx/RadxRay.hh>
#include <toolsa/LogStream.hh>
#include <algorithm>

//------------------------------------------------------------------
RayData1::RayData1(const RayData &r, int index) :
  MathData(), _ray(NULL), _ray0(NULL), _ray1(NULL)
{
  _ray = r._rays[index];
  _ray0 = NULL;
  _ray1 = NULL;
  int nr = (int)r._rays.size();
  if (index == 0)
  {
    if (nr > 1)
    {
      _ray1 = r._rays[1];
    }
  }
  else if (index == nr-1)
  {
    _ray0 = r._rays[nr-2];
  }
  else
  {
    if (nr > 2)
    {
      _ray0 = r._rays[index-1];
      _ray1 = r._rays[index+1];
    }
  }

  // transfer values from volume object to here
  _specialName = r._specialName;
  _specialValue = r._specialValue;
}  

//------------------------------------------------------------------
RayData1::~RayData1(void)
{
  // nothing owned by this class
}

//------------------------------------------------------------------
// virtual
int RayData1::numData(void) const
{
  if (_outputRay == NULL)
  {
    LOG(ERROR) << " No ray to look at";
    return 0;
  }
  return _outputRay->numData();
}

//------------------------------------------------------------------
// virtual
void RayData1::finishProcessingNode(int index, VolumeData *vol)
{
  // add all the derived data to this ray
  _updateRay();

  // clear out derived data
  _data.clear();
}

//------------------------------------------------------------------
// virtual
bool RayData1::synchInputsAndOutputs(const std::string &output,
				    const std::vector<std::string> &inputs)
{
  _inps.clear();
  _outputRay = NULL;
  _specialInpNames.clear();
  _specialInps.clear();

  // look for input data and return false if one or more missing
  bool ok = true;
  for (size_t i=0; i<inputs.size(); ++i)
  {
    if (!_isInput(inputs[i]))
    {
      LOG(ERROR) << "Cannot synch input " << inputs[i];
      ok = false;
    }
  }
  if (!ok)
  {
    return false;
  }

  // try to create an output data pointer, anything will do as template
  _outputRay = _exampleData(output);
  if (_outputRay == NULL)
  {
    LOG(ERROR) << "Cannot synch for output " << output;
    return false;
  }

  // now set all the local values:  _inps, _specialInps, _specialInpNames

  for (size_t i=0; i<inputs.size(); ++i)
  {
    _setLocalInput(inputs[i]);
  }
  if ((_inps.size() + _specialInps.size()) != inputs.size())
  {
    LOG(ERROR) << "Difficulty synching data";
    return false;
  }
  else
  {
    return true;
  }
}
    
//------------------------------------------------------------------
// virtual
MathLoopData * RayData1::dataPtr(const std::string &name)
{
  return (MathLoopData *)_match(name);
}

//------------------------------------------------------------------
// virtual
const MathLoopData * RayData1::dataPtrConst(const std::string &name) const
{
  return (const MathLoopData *)_matchConst(name);
}

//------------------------------------------------------------------
// virtual 
bool RayData1::processUserLoopFunction(ProcessingNode &p)
{
  string keyword;
  if (!p.isUserUnaryOp(keyword))
  {
    return false;
  }

  if (keyword == "AzGradient")
  {
    return _processAzGradient(*(p.unaryOpArgs()));
  }
  else if (keyword == "Qscale")
  {
    return _processQscale(*(p.unaryOpArgs()), false);
  }
  else if (keyword == "OneMinusQscale")
  {
    return _processQscale(*(p.unaryOpArgs()), true);
  }
  else if (keyword == "CLUTTER_2D_QUAL")
  {
    return _processClutter2dQual(*(p.unaryOpArgs()));
  }
  else if (keyword == "Special0")
  {
    return _processSpecial0(*(p.unaryOpArgs()));
  }
  else if (keyword == "Special1")
  {
    return _processSpecial1(*(p.unaryOpArgs()));
  }
  else if (keyword == "FIR")
  {
    return _processFIR(*(p.unaryOpArgs()));
  }
  else if (keyword == "Variance1d")
  {
    return _processVariance1d(*(p.unaryOpArgs()));
  }
  else
  {
    printf("Unknown keyword %s\n", keyword.c_str());
    return false;
  }
}

//------------------------------------------------------------------
MathUserData *RayData1::processUserLoop2dFunction(const UnaryNode &p)
{
  LOG(ERROR) << "Not implemented for this class";
  return NULL;
}

//------------------------------------------------------------------
// virtual
bool RayData1::synchUserDefinedInputs(const std::string &userKey,
				      const std::vector<std::string> &names)
{
  _inps.clear();
  _outputRay = NULL;
  _specialInpNames.clear();
  _specialInps.clear();
  
  if (!_needToSynch(userKey))
  {
    return true;
  }
  for (size_t i=0; i<names.size(); ++i)
  {
    RayLoopData *m = _refToData(names[i], false);
    if (m == NULL)
    {
      LOG(ERROR) << "Cannot synch";
      return false;
    }
    else
    {
      _inps.push_back(m);
    }
  }
  return true;
}

//------------------------------------------------------------------
// virtual
bool RayData1::storeMathUserData(const std::string &name, MathUserData *s)
{
  if (find(_specialName.begin(), _specialName.end(), name) !=
      _specialName.end())
  {
    printf("ERROR double storing of %s\n", name.c_str());
    return false;
  }
  else
  {
    _specialName.push_back(name);
    _specialValue.push_back(s);
    return true;
  }
}

//------------------------------------------------------------------
// virtual
bool RayData1::smooth(MathLoopData *l,
		      std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}
//------------------------------------------------------------------
// virtual
bool RayData1::smoothDBZ(MathLoopData *l,
		      std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

//------------------------------------------------------------------
// virtual
bool RayData1::stddev(MathLoopData *l,
		      std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}
//------------------------------------------------------------------
// virtual
bool RayData1::fuzzy(MathLoopData *l,
		     std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

//------------------------------------------------------------------
// virtual
bool RayData1::average(MathLoopData *l,
		     std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

//------------------------------------------------------------------
// virtual
bool RayData1::weighted_average(MathLoopData *l,
				std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

//------------------------------------------------------------------
// virtual
bool RayData1::mask(MathLoopData *l,
		     std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

bool RayData1::mask_missing_to_missing(MathLoopData *out,
				       std::vector<ProcessingNode *> &args) const
{
  return false;
}

bool RayData1::trapezoid(MathLoopData *out,
			 std::vector<ProcessingNode *> &args) const
{
  return false;
}

bool RayData1::s_remap(MathLoopData *out,
		       std::vector<ProcessingNode *> &args) const
{
  return false;
}

//------------------------------------------------------------------
RayLoopData *RayData1::_refToData(const std::string &name, bool suppressWarn)
{
  // try to pull out of existing rays in data
  for (size_t i=0; i<_data.size(); ++i)
  {
    if (name == _data[i].getName())
    {
      return &_data[i];
    }
  }
      
  // try to pull out of input ray
  RayxData r;
  if (RadxApp::retrieveRay(name, *_ray, r, false))
  {
    // add this ray to the data state and return that
    _data.push_back(RayLoopData(r));
    size_t k = _data.size();
    return &(_data[k-1]);
  }

  // can't pull out of state, not in _ray and not in _data
  if (!suppressWarn)
  {
    printf("ERROR retrieving data for %s\n", name.c_str());
  }
  return NULL;
}    

//------------------------------------------------------------------
RayLoopData *RayData1::_exampleData(const std::string &name)
{
  RayLoopData *s = (RayLoopData *)_refToData(name, true);
  if (s == NULL)
  {
    RayxData r;
    if (RadxApp::retrieveAnyRay(*_ray, r))
    {
      RadxApp::modifyRayForOutput(r, name, "units", r.getMissing());
      RayLoopData store(r);
      _data.push_back(store);
      s = (RayLoopData *)_refToData(name, true);
    }
  }
  if (s != NULL)
  {
    return s;
  }
  else
  {
    LOG(ERROR) << "No data created for " << name;
    return NULL;
  }
}

//------------------------------------------------------------------
bool RayData1::_processAzGradient(std::vector<ProcessingNode *> &args) const
{
  if (args.size() != 3)
  {
    printf("bad args\n");
    return false;
  }

  // get the state (arg[1])
  string name = args[1]->leafName();
  if (name.empty())
  {
    LOG(ERROR) << " NO name";
    return false;
  }
  const MathUserData *s=NULL;
  for (size_t i=0; i<_specialInpNames.size(); ++i)
  {
    if (_specialInpNames[i] == name)
    {
      s = _specialInps[i];
      break;
    }
  }
  if (s == NULL)
  {
    printf("No state data\n");
    return false;
  }

  // interpret as what it should be
  const AzGradientStateSpecialData *state = (AzGradientStateSpecialData *)s;
  int nState = state->nstate();
  if (nState < 1)
  {
    LOG(ERROR) << "State is empty";
    return false;
  }

  double v;
  if (!args[2]->getValue(v))
  {
    LOG(ERROR) << "No value";
    return false;
  }

  name = args[0]->leafName();
  if (name.empty())
  {
    LOG(ERROR) << " NO name";
    return false;
  }

  AzGradientFilter filter;
  return filter.filter(*state, v, name, _ray0, _ray1, _ray, _data,
		       _outputRay);
}

//------------------------------------------------------------------
bool RayData1::_processSpecial0(std::vector<ProcessingNode *> &args) const
{
  if (args.size() != 3)
  {
    printf("bad args\n");
    return false;
  }

  // get the state (arg[1])
  string widthName = args[0]->leafName();
  if (widthName.empty())
  {
    LOG(ERROR) << " NO name";
    return false;
  }
  string meanPrtName = args[1]->leafName();
  if (meanPrtName.empty())
  {
    LOG(ERROR) << " NO name";
    return false;
  }
  string meanNsamplesName = args[2]->leafName();
  if (meanNsamplesName.empty())
  {
    LOG(ERROR) << " NO name";
    return false;
  }

  const MathUserData *meanPrt=NULL;
  const MathUserData *meanNsamples=NULL;
  for (size_t i=0; i<_specialInpNames.size(); ++i)
  {
    if (_specialInpNames[i] == meanPrtName)
    {
      meanPrt = _specialInps[i];
    }
    if (_specialInpNames[i] == meanNsamplesName)
    {
      meanNsamples = _specialInps[i];
    }
  }
  if (meanPrt == NULL || meanNsamples == NULL)
  {
    printf("No state data\n");
    return false;
  }

  double meanPrtV, meanNsamplesV;
  if (!meanPrt->getFloat(meanPrtV) || !meanNsamples->getFloat(meanNsamplesV))
  {
    printf("No values\n");
    return false;
  }
  
  Special0Filter filter;
  return filter.filter(widthName, meanPrtV, meanNsamplesV, _ray, _data,
		       _outputRay);
}

//------------------------------------------------------------------
bool RayData1::_processSpecial1(std::vector<ProcessingNode *> &args) const
{
  if (args.size() != 3)
  {
    printf("bad args\n");
    return false;
  }

  // get the state (arg[1])
  string widthName = args[0]->leafName();
  if (widthName.empty())
  {
    LOG(ERROR) << " NO name";
    return false;
  }
  string meanPrtName = args[1]->leafName();
  if (meanPrtName.empty())
  {
    LOG(ERROR) << " NO name";
    return false;
  }
  string meanNsamplesName = args[2]->leafName();
  if (meanNsamplesName.empty())
  {
    LOG(ERROR) << " NO name";
    return false;
  }

  const MathUserData *meanPrt=NULL;
  const MathUserData *meanNsamples=NULL;
  for (size_t i=0; i<_specialInpNames.size(); ++i)
  {
    if (_specialInpNames[i] == meanPrtName)
    {
      meanPrt = _specialInps[i];
    }
    if (_specialInpNames[i] == meanNsamplesName)
    {
      meanNsamples = _specialInps[i];
    }
  }
  if (meanPrt == NULL || meanNsamples == NULL)
  {
    printf("No state data\n");
    return false;
  }

  double meanPrtV, meanNsamplesV;
  if (!meanPrt->getFloat(meanPrtV) || !meanNsamples->getFloat(meanNsamplesV))
  {
    printf("No values\n");
    return false;
  }
  
  Special1Filter filter;
  return filter.filter(widthName, meanPrtV, meanNsamplesV, _ray, _data,
		       _outputRay);
}

//------------------------------------------------------------------
bool RayData1::_processFIR(std::vector<ProcessingNode *> &args) const
{
  if (args.size() != 1)
  {
    printf("bad args\n");
    return false;
  }

  string dataName = args[0]->leafName();
  if (dataName.empty())
  {
    LOG(ERROR) << " NO name";
    return false;
  }
  
  FIRFilter filter;
  return filter.filter(dataName, "INTERP", _ray, _data,
		       _outputRay);
}

//------------------------------------------------------------------
bool RayData1::_processVariance1d(std::vector<ProcessingNode *> &args) const
{
  if (args.size() != 3)
  {
    printf("bad args\n");
    return false;
  }

  string dataName = args[0]->leafName();
  if (dataName.empty())
  {
    LOG(ERROR) << " NO name";
    return false;
  }
  
  double npt;
  if (!args[1]->getValue(npt))
  {
    LOG(ERROR) << "No value";
    return false;
  }

  double maxPctMissing;
  if (!args[2]->getValue(maxPctMissing))
  {
    LOG(ERROR) << "No value";
    return false;
  }
  
  Variance1dFilter filter(npt, maxPctMissing);
  return filter.filter(dataName, _ray, _data, _outputRay);
}

//------------------------------------------------------------------
bool RayData1::_processQscale(std::vector<ProcessingNode *> &args,
			     bool subtractFromOne) const
{
  if (args.size() != 4)
  {
    printf("bad args\n");
    return false;
  }

  // get the variable to work on
  string name = args[0]->leafName();
  if (name.empty())
  {
    LOG(ERROR) << " NO name";
    return false;
  }

  double scale, topv, lowv;
  if (args[1]->getValue(scale) &&
      args[2]->getValue(topv) &&
      args[3]->getValue(lowv))
  {
    // printf("Qscale %s %lf %lf %lf\n",
    // 	   name.c_str(), scale, topv, lowv);
  }
  else
  {
    LOG(ERROR) << "Getting qscale args";
    return false;
  }
  
  QscaleFilter filter;
  return filter.filter(name, scale, topv, lowv, subtractFromOne,
		       _ray, _data, _outputRay);
}

//------------------------------------------------------------------
// clutter2dqual(scr, 0.69, vel, 1.5, width, 0.5)
// clutter2dqual(scr, scale, vel, vr_shape, width, sw_shape)

bool RayData1::_processClutter2dQual(std::vector<ProcessingNode *> &args) const
{
  if (args.size() != 6)
  {
    printf("bad args\n");
    return false;
  }

  // get the variables to work on
  string scrName = args[0]->leafName();
  if (scrName.empty())
  {
    LOG(ERROR) << " NO name";
    return false;
  }
  string velName = args[2]->leafName();
  if (velName.empty())
  {
    LOG(ERROR) << " NO name";
    return false;
  }

  string widthName = args[4]->leafName();
  if (widthName.empty())
  {
    LOG(ERROR) << " NO name";
    return false;
  }

  double scale, vr_shape, sw_shape;
  if (args[1]->getValue(scale) &&
      args[3]->getValue(vr_shape) &&
      args[5]->getValue(sw_shape))
  {
    // printf("Qscale %s %lf %lf %lf\n",
    // 	   name.c_str(), scale, topv, lowv);
  }
  else
  {
    LOG(ERROR) << "Getting qscale args";
    return false;
  }
  
  Clutter2dQualFilter filter;
  return filter.filter(scrName, velName, widthName, scale, vr_shape, sw_shape,
		       _ray, _data, _outputRay);
}


//---------------------------------------------------------------
void RayData1::_updateRay(void)
{
  // take all the data and add to the ray.
  // note that here should make sure not deleting a result (check names)
  if (_data.empty())
  {
    return;
  }
  int nGatesPrimary = _ray->getNGates();
  Radx::fl32 *data = new Radx::fl32[nGatesPrimary];


  // now add in all the other ones
  for (size_t i=0; i<_data.size(); ++i)
  {
    _data[i].retrieveData(data, nGatesPrimary);
    _ray->addField(_data[i].getName(), _data[i].getUnits(),
		   _data[i].getNpoints(),
		   _data[i].getMissing(), data, true);
  }
  delete [] data;
}

//---------------------------------------------------------------
RayLoopData *RayData1::_match(const std::string &n) 
{
  // try to pull out of existing rays in data
  for (size_t i=0; i<_data.size(); ++i)
  {
    if (n == _data[i].getName())
    {
      return &_data[i];
    }
  }
  return NULL;
}      

//---------------------------------------------------------------
const RayLoopData *RayData1::_matchConst(const std::string &n) const
{
  // try to pull out of existing rays in data
  for (size_t i=0; i<_data.size(); ++i)
  {
    if (n == _data[i].getName())
    {
      return &_data[i];
    }
  }
  return NULL;
}      

//------------------------------------------------------------------
bool RayData1::_needToSynch(const std::string &userKey) const
{
  if (userKey == "AzGradient")
  {
    // this loop filter has inputs
    return true;
  }
  else if (userKey == "Qscale")
  {
    // this loop filter has inputs
    return true;
  }
  else if (userKey == "OneMinusQscale")
  {
    // this loop filter has inputs
    return true;
  }
  else if (userKey == "CLUTTER_2D_QUAL")
  {
    // this loop filter has inputs
    return true;
  }
  else if (userKey == "Special0")
  {
    // this loop filter has inputs
    return true;
  }
  else if (userKey == "Special1")
  {
    // this loop filter has inputs
    return true;
  }
  else if (userKey == "FIR")
  {
    // this loop filter has inputs
    return true;
  }
  else if (userKey == "Variance1d")
  {
    // this loop filter has inputs
    return true;
  }

  // everything else has inputs
  return true;
}


//---------------------------------------------------------------
bool RayData1::_isInput(const std::string &name)
{
  if (_refToData(name, true) == NULL)
  {
    // try special data instead
    for (size_t j=0; j<_specialName.size(); ++j)
    {
      if (_specialName[j] == name)
      {
	return true;
      }
    }
    return false;
  }
  else
  {
    return true;
  }
}

//---------------------------------------------------------------
void RayData1::_setLocalInput(const std::string &input)
{
  RayLoopData *m = _match(input);
  if (m != NULL)
  {
    _inps.push_back(m);
  }
  else
  {
    // try special
    for (size_t j=0; j<_specialName.size(); ++j)
    {
      if (_specialName[j] == input)
      {
	_specialInpNames.push_back(_specialName[j]);
	_specialInps.push_back(_specialValue[j]);
      }
    }
  }
}
