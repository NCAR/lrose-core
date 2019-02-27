/**
 * @file RadxAppRayData.cc
 */
#include <radar/RadxAppRayData.hh>
#include <radar/RadxApp.hh>
#include <radar/RadxAppVolume.hh>
#include <Radx/RadxRay.hh>
#include <rapmath/ProcessingNode.hh>
#include <toolsa/LogStream.hh>

//------------------------------------------------------------------
RadxAppRayData::RadxAppRayData(void) : MathData(), _ray(NULL)
{
}

//------------------------------------------------------------------
RadxAppRayData::RadxAppRayData(const RadxAppVolume &r, const SpecialUserData &s,
			       int index) :
  MathData(), _ray(NULL)
{
  _ray = (*r._rays)[index];

  // transfer values from volume object to here
  _special = s;
  _special.setOwnership(false);
}  

//------------------------------------------------------------------
RadxAppRayData::~RadxAppRayData(void)
{
  // nothing owned by this class
}

//------------------------------------------------------------------
std::vector<FunctionDef> RadxAppRayData::userUnaryOperators(void) const
{
  std::vector<FunctionDef> ret;
  radxappAppendUnaryOperators(ret);
  return ret;
}

//------------------------------------------------------------------
// virtual
int RadxAppRayData::numData(void) const
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
void RadxAppRayData::finishProcessingNode(int index, VolumeData *vol)
{
  // add all the derived data to the main ray (input vol not needed because
  // the local object points to the same data)
  _updateRay();

  // clear out derived data
  _data.clear();
}

//------------------------------------------------------------------
// virtual
bool
RadxAppRayData::synchInputsAndOutputs(const std::string &output,
				      const std::vector<std::string> &inputs)
{
  _inps.clear();
  _outputRay = NULL;
  _specialInp = SpecialUserData(false);

  // look for input data and return false if one or more missing
  bool ok = true;
  for (size_t i=0; i<inputs.size(); ++i)
  {
    if (!_synchInput(inputs[i]))
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
  if ((_inps.size() + _specialInp.size()) != inputs.size())
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
MathLoopData * RadxAppRayData::dataPtr(const std::string &name)
{
  return (MathLoopData *)_match(name);
}

//------------------------------------------------------------------
// virtual
const MathLoopData * RadxAppRayData::dataPtrConst(const std::string &name) const
{
  return (const MathLoopData *)_matchConst(name);
}

//------------------------------------------------------------------
const MathUserData *
RadxAppRayData::userDataPtrConst(const std::string &name) const
{
  if (_special.hasName(name))
  {
    return _special.matchingDataPtrConst(name);
  }
  if (_specialInp.hasName(name))
  {
    return _specialInp.matchingDataPtrConst(name);
  }
  return NULL;
}

//------------------------------------------------------------------
MathUserData *RadxAppRayData::userDataPtr(const std::string &name)
{
  if (_special.hasName(name))
  {
    return _special.matchingDataPtr(name);
  }
  if (_specialInp.hasName(name))
  {
    return _specialInp.matchingDataPtr(name);
  }
  return NULL;
}

//------------------------------------------------------------------
// virtual 
bool RadxAppRayData::processUserLoopFunction(ProcessingNode &p)
{
  string keyword;
  if (!p.isUserUnaryOp(keyword))
  {
    return false;
  }

  return radxappUserLoopFunction(keyword, p);
}

//------------------------------------------------------------------
MathUserData *
RadxAppRayData::processUserLoopFunctionToUserData(const UnaryNode &p)
{
  return radxappUserLoopFunctionToUserData(p);
}

//------------------------------------------------------------------
// virtual
bool
RadxAppRayData::synchUserDefinedInputs(const std::string &userKey,
				       const std::vector<std::string> &names)
{
  // printf("Synch ray\n");
  // _inps.clear();
  // _outputRay = NULL;
  // _specialInp = SpecialUserData(false);
  
  // if (!_needToSynch(userKey))
  // {
  //    return true;
  // }
  // for (size_t i=0; i<names.size(); ++i)
  // {
  //   RadxAppRayLoopData *m = _refToData(names[i], false);
  //   if (m == NULL)
  //   {
  //     LOG(ERROR) << "Cannot synch";
  //     return false;
  //   }
  //   else
  //   {
  //     _inps.push_back(m);
  //   }
  // }
  return true;
}

//------------------------------------------------------------------
// virtual
bool RadxAppRayData::storeMathUserData(const std::string &name,
				       MathUserData *s)
{
  return _special.store(name, s);
}

//------------------------------------------------------------------
// virtual
bool RadxAppRayData::smooth(MathLoopData *l,
		      std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}
//------------------------------------------------------------------
// virtual
bool RadxAppRayData::smoothDBZ(MathLoopData *l,
		      std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

//------------------------------------------------------------------
// virtual
bool RadxAppRayData::stddev(MathLoopData *l,
		      std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}
//------------------------------------------------------------------
// virtual
bool RadxAppRayData::fuzzy(MathLoopData *l,
		     std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

//------------------------------------------------------------------
// virtual
bool RadxAppRayData::average(MathLoopData *l,
		     std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

//------------------------------------------------------------------
// virtual
bool RadxAppRayData::weighted_average(MathLoopData *l,
				std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

//------------------------------------------------------------------
// virtual
bool RadxAppRayData::weighted_angle_average(MathLoopData *l,
				      std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

//------------------------------------------------------------------
// virtual
bool RadxAppRayData::median(MathLoopData *l,
		      std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

//------------------------------------------------------------------
// virtual
bool RadxAppRayData::max(MathLoopData *l,
		      std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

//------------------------------------------------------------------
// virtual
bool RadxAppRayData::max_expand(MathLoopData *l,
			 std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

bool RadxAppRayData::expand_angles_laterally(MathLoopData *l,
		     std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

bool RadxAppRayData::clump(MathLoopData *l,
		     std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

//------------------------------------------------------------------
// virtual
bool RadxAppRayData::mask(MathLoopData *l,
		     std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

//------------------------------------------------------------------
bool RadxAppRayData::mask_missing_to_missing(MathLoopData *out,
				       std::vector<ProcessingNode *> &args) const
{
  return false;
}

//------------------------------------------------------------------
bool RadxAppRayData::trapezoid(MathLoopData *out,
			 std::vector<ProcessingNode *> &args) const
{
  return false;
}

//------------------------------------------------------------------
bool RadxAppRayData::s_remap(MathLoopData *out,
		       std::vector<ProcessingNode *> &args) const
{
  return false;
}

//------------------------------------------------------------------
RadxAppRayLoopData *RadxAppRayData::_refToData(const std::string &name,
					       bool suppressWarn)
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
  RayxData *r = RadxApp::retrieveRayPtr(name, *_ray, false);
  if (r != NULL)
  {
    // add this ray to the data state and return that
    _data.push_back(RadxAppRayLoopData(*r));
    delete r;
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
RadxAppRayLoopData *RadxAppRayData::_exampleData(const std::string &name)
{
  // see if it is already sitting there:
  RadxAppRayLoopData *s = (RadxAppRayLoopData *)_refToData(name, true);
  if (s == NULL)
  {
    // not already in place, so we create a new one
    const RayxData *r = RadxApp::retrieveAnyRayPtr(*_ray);
    if (r != NULL)
    {
      // set it to the correct values
      RadxAppRayLoopData store(*r);
      delete r;
      RadxApp::modifyRayForOutput(store, name, "units", store.getMissing());
      _data.push_back(store);
      // point to it for return
      s = (RadxAppRayLoopData *)_refToData(name, true);
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

//---------------------------------------------------------------
void RadxAppRayData::_updateRay(void)
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
RadxAppRayLoopData *RadxAppRayData::_match(const std::string &n) 
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
const RadxAppRayLoopData *
RadxAppRayData::_matchConst(const std::string &n) const
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
bool RadxAppRayData::_synchInput(const std::string &name)
{
  if (_refToData(name, true) == NULL)
  {
    // try special data instead
    if (_special.hasName(name))
    {
      return true;
    }
    return false;
  }
  else
  {
    return true;
  }
}

//---------------------------------------------------------------
void RadxAppRayData::_setLocalInput(const std::string &input)
{
  RadxAppRayLoopData *m = _match(input);
  if (m != NULL)
  {
    _inps.push_back(m);
  }
  else
  {
    // try special
    if (_special.hasName(input))
    {
      _specialInp.store(input, _special.matchingDataPtr(input));
    }
  }
}
