/**
 * @file RadxAppSweeData.cc
 */
#include <radar/RadxAppSweepData.hh>
#include <radar/RadxAppVolume.hh>
#include <rapmath/FunctionDef.hh>
#include <rapmath/ProcessingNode.hh>
#include <radar/RadxApp.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxSweep.hh>
#include <toolsa/LogStream.hh>

const std::string RadxAppSweepData::_variance2dStr = "Variance2d";

//------------------------------------------------------------------
RadxAppSweepData::RadxAppSweepData(void) :
  MathData(), _rays(NULL), _lookup(NULL)
{
}

//------------------------------------------------------------------
RadxAppSweepData::RadxAppSweepData(const RadxAppVolume &r, int index,
		   const RadxAppCircularLookupHandler *lookup) :
  MathData(), _rays(r._rays), _lookup(lookup)
{
  const vector<RadxSweep *> sweeps = r._vol.getSweeps();
  _i0 = sweeps[index]->getStartRayIndex();
  _i1 = sweeps[index]->getEndRayIndex();
}  

//------------------------------------------------------------------
RadxAppSweepData::~RadxAppSweepData(void)
{
  // nothing owned by this class
}

//------------------------------------------------------------------
std::vector<FunctionDef> RadxAppSweepData::userUnaryOperators(void) const
{
  std::vector<FunctionDef> ret;
  ret.push_back(FunctionDef(_variance2dStr, "v", "field", 
			    "2d Variance of input field, "
			    "  uses variance_radius_km parameter which must be defined as a constant"));
  radxappAppendUnaryOperators(ret);
  return ret;
}

//------------------------------------------------------------------
// virtual
int RadxAppSweepData::numData(void) const
{
  if (_outputSweep == NULL)
  {
    LOG(ERROR) << " No sweep to look at";
    return 0;
  }
  return _outputSweep->numData();
}

//------------------------------------------------------------------
// virtual
void RadxAppSweepData::finishProcessingNode(int index, VolumeData *vol)
{
  // add all the derived data to this sweep
  _updateSweep();

  // clear out derived data
  _data.clear();
}

//------------------------------------------------------------------
// virtual
bool RadxAppSweepData::synchInputsAndOutputs(const std::string &output,
				    const std::vector<std::string> &inputs)
{
  // look for input data and return false if one or more missing
  RadxAppSweepLoopData *m=NULL;
  for (size_t i=0; i<inputs.size(); ++i)
  {
    m = _refToData(inputs[i], true);
    if (m == NULL)
    {
      LOG(ERROR) << "Cannot synch input " << inputs[i];
      return false;
    }
  }

  // try to create an output data pointer, anything will do as template
  m = _exampleData(output);
  if (m == NULL)
  {
    LOG(ERROR) << "Cannot synch for output " << output;
    return false;
  }

  // now set all the local values
  _inps.clear();
  _outputSweep = _match(output);
  for (size_t i=0; i<inputs.size(); ++i)
  {
    RadxAppSweepLoopData *m = _match(inputs[i]);
    if (m != NULL)
    {
      _inps.push_back(m);
    }
  }
  if (_outputSweep == NULL || _inps.size() != inputs.size())
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
MathLoopData * RadxAppSweepData::dataPtr(const std::string &name)
{
  return (MathLoopData *)_match(name);
}

//------------------------------------------------------------------
// virtual
const MathLoopData *
RadxAppSweepData::dataPtrConst(const std::string &name) const
{
  return (const MathLoopData *)_matchConst(name);
}

//------------------------------------------------------------------
const MathUserData *
RadxAppSweepData::userDataPtrConst(const std::string &name) const
{
  return NULL;
}

//------------------------------------------------------------------
MathUserData *RadxAppSweepData::userDataPtr(const std::string &name)
{
  return NULL;
}

//------------------------------------------------------------------
// virtual 
bool RadxAppSweepData::processUserLoopFunction(ProcessingNode &p)
{
  string keyword;
  if (!p.isUserUnaryOp(keyword))
  {
    return false;
  }

  if (keyword == _variance2dStr)
  {
    return _processVariance2d(*(p.unaryOpArgs()));
  }
  else
  {
    return radxappUserLoopFunction(keyword, p);
  }
    // printf("Unknown keyword %s\n", keyword.c_str());
    // return false;
  // }
}

//------------------------------------------------------------------
MathUserData *
RadxAppSweepData::processUserLoopFunctionToUserData(const UnaryNode &p)
{
  return radxappUserLoopFunctionToUserData(p);
}

//------------------------------------------------------------------
// virtual
bool RadxAppSweepData::synchUserDefinedInputs(const std::string &userKey,
				      const std::vector<std::string> &names)
{
  // if (!_needToSynch(userKey))
  // {
  //   return true;
  // }
  // for (size_t i=0; i<names.size(); ++i)
  // {
  //   RadxAppSweepLoopData *m = _refToData(names[i], false);
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
bool RadxAppSweepData::storeMathUserData(const std::string &name,
					 MathUserData *s){
  LOG(WARNING) <<
    "RadxAppSweepData does not support user data yet storing " << name;
  return false;
}

//------------------------------------------------------------------
// virtual
bool RadxAppSweepData::smooth(MathLoopData *l,
		      std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

// virtual
//------------------------------------------------------------------
bool RadxAppSweepData::smoothDBZ(MathLoopData *l,
		      std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

//------------------------------------------------------------------
// virtual
bool RadxAppSweepData::stddev(MathLoopData *l,
		      std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}
//------------------------------------------------------------------
// virtual
bool RadxAppSweepData::fuzzy(MathLoopData *l,
		     std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

//------------------------------------------------------------------
// virtual
bool RadxAppSweepData::average(MathLoopData *l,
		     std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

//------------------------------------------------------------------
// virtual
bool RadxAppSweepData::max_expand(MathLoopData *l,
			 std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

//------------------------------------------------------------------
// virtual
bool RadxAppSweepData::weighted_average(MathLoopData *l,
				std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

bool RadxAppSweepData::mask(MathLoopData *l,
		     std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

bool RadxAppSweepData::median(MathLoopData *l,
		     std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

bool RadxAppSweepData::weighted_angle_average(MathLoopData *l,
		     std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

bool RadxAppSweepData::expand_angles_laterally(MathLoopData *l,
		     std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

bool RadxAppSweepData::clump(MathLoopData *l,
		     std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

bool RadxAppSweepData::mask_missing_to_missing(MathLoopData *out,
		       std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

bool RadxAppSweepData::trapezoid(MathLoopData *out,
			 std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

bool RadxAppSweepData::s_remap(MathLoopData *out,
		       std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

bool RadxAppSweepData::max(MathLoopData *out,
		   std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

//------------------------------------------------------------------
RadxAppSweepLoopData *RadxAppSweepData::_refToData(const std::string &name,
						   bool suppressWarn)
{
  // try to pull out of existing data
  for (size_t i=0; i<_data.size(); ++i)
  {
    if (name == _data[i].getName())
    {
      return &_data[i];
    }
  }
      
  // try to pull out of input data
  RadxAppSweepLoopData r;
  if (_newLoopData(name, r, !suppressWarn))
  {
    _data.push_back(r);
    size_t k = _data.size();
    return &(_data[k-1]);
  }

  // can't pull out of state, not in input data and not in _data
  if (!suppressWarn)
  {
    printf("ERROR retrieving data for %s\n", name.c_str());
  }
  return NULL;
}    

//------------------------------------------------------------------
RadxAppSweepLoopData *RadxAppSweepData::_exampleData(const std::string &name)
{
  RadxAppSweepLoopData *s = _refToData(name, true);
  if (s == NULL)
  {
    RadxAppSweepLoopData r;
    if (_anyLoopData(r))
    {
      r.modifyForOutput(name, "units");
      _data.push_back(r);
      s = _refToData(name, true);
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
bool RadxAppSweepData::_processVariance2d(std::vector<ProcessingNode *> &args)
{
  if (_lookup == NULL)
  {
    LOG(ERROR) << "No lookup, cannot compute variance";
    return false;
  }  

  const MathLoopData  *data = loadData(args, 0);
  if (data == NULL)
  {
    LOG(ERROR) << "Wrong interface";
    return false;
  }

  const RadxAppSweepLoopData *
    rdata = dynamic_cast<const RadxAppSweepLoopData *>(data);

  // the output should already be in place, so just go for it
  return _outputSweep->variance2d(*rdata, *_lookup);
}

//------------------------------------------------------------------
RadxAppSweepLoopData *RadxAppSweepData::_match(const std::string &n) 
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
const RadxAppSweepLoopData *
RadxAppSweepData::_matchConst(const std::string &n) const
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

// //------------------------------------------------------------------
// bool RadxAppSweepData ::_needToSynch(const std::string &userKey) const
// {
//   if (userKey == _variance2dStr)
//   {
//     // this loop filter has inputs
//     return true;
//   }

//   // everything else has inputs
//   return true;
// }

//---------------------------------------------------------------
void RadxAppSweepData::_updateSweep(void)
{
  // take all the local data and add to the sweep.
  // note that here should make sure not deleting a result (check names)
  if (_data.empty())
  {
    return;
  }

  for (size_t k=0; k<_data.size(); ++k)
  {
    _updateSweepOneDataset(k);
  }
}

//------------------------------------------------------------------
void RadxAppSweepData::_updateSweepOneDataset(int k)
{
  // get the name of the 0th data, and number of gates, which can change
  string name = _data[k].getName();
  int nGatesPrimary = _data[k].getNGates();
  Radx::fl32 *data = new Radx::fl32[nGatesPrimary];
  int j=0;
  for (int i=_i0; i<=_i1; ++i)
  {
    _data[k].retrieveRayData(j, data, nGatesPrimary);
    (*_rays)[i]->addField(name, _data[k].getUnits(), _data[k].getNpoints(j),
			  _data[k].getMissing(), data, true);
    j++;
  }
  delete [] data;
}


//------------------------------------------------------------------
bool RadxAppSweepData::_newLoopData(const std::string &name,
			    RadxAppSweepLoopData &ret, bool warn) const
{
  vector<RayxData> rays;
  for (int i=_i0; i<=_i1; ++i)
  {
    RayxData r;
    if (RadxApp::retrieveRay(name, *(*_rays)[i], r, false))
    {
      rays.push_back(r);
    }
    else
    {
      if (warn) LOG(ERROR) << "No data for " << name;
      return false;
    }
  }
  ret = RadxAppSweepLoopData(name, _i0, _i1, rays);
  return true;
}

//------------------------------------------------------------------
bool RadxAppSweepData::_anyLoopData(RadxAppSweepLoopData &ret) const
{
  RayxData r;
  string name;
  if (RadxApp::retrieveAnyRay(*(*_rays)[_i0], r))
  {
    name = r.getName();
    return _newLoopData(name, ret, true);
  }
  else
  {
    LOG(ERROR) << "No data";
    return false;
  }
}

  
