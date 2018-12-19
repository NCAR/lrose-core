#include <RayData2.hh>
#include <CircularLookupHandler.hh>
#include <rapmath/ProcessingNode.hh>
#include <radar/RadxApp.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxSweep.hh>
#include <toolsa/LogStream.hh>
#include <algorithm>

//------------------------------------------------------------------
RayData2::RayData2(void) :
  MathData(), _rays(NULL), _lookup(NULL)
{
}

//------------------------------------------------------------------
RayData2::RayData2(const RayData &r, int index,
		   const CircularLookupHandler *lookup) :
  MathData(), _rays(r._rays), _lookup(lookup)
{
  const vector<RadxSweep *> sweeps = r._vol.getSweeps();
  _i0 = sweeps[index]->getStartRayIndex();
  _i1 = sweeps[index]->getEndRayIndex();
}  

//------------------------------------------------------------------
RayData2::~RayData2(void)
{
  // nothing owned by this class
}

//------------------------------------------------------------------
std::vector<std::pair<std::string, std::string> >
RayData2::userUnaryOperators(void) const
{
  std::vector<std::pair<std::string, std::string> > ret;

  string s;
  s = "v=Variance2d(field) 2d Variance of input field\n"
    "  uses variance_radius_km parameter";
  ret.push_back(pair<string,string>("Variance2d", s));
    
  return ret;
}

//------------------------------------------------------------------
// virtual
int RayData2::numData(void) const
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
void RayData2::finishProcessingNode(int index, VolumeData *vol)
{
  // add all the derived data to this sweep
  _updateSweep();

  // clear out derived data
  _data.clear();
}

//------------------------------------------------------------------
// virtual
bool RayData2::synchInputsAndOutputs(const std::string &output,
				    const std::vector<std::string> &inputs)
{
  // look for input data and return false if one or more missing
  RayLoopSweepData *m=NULL;
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
    RayLoopSweepData *m = _match(inputs[i]);
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
MathLoopData * RayData2::dataPtr(const std::string &name)
{
  return (MathLoopData *)_match(name);
}

//------------------------------------------------------------------
// virtual
const MathLoopData * RayData2::dataPtrConst(const std::string &name) const
{
  return (const MathLoopData *)_matchConst(name);
}

//------------------------------------------------------------------
const MathUserData *RayData2::userDataPtrConst(const std::string &name) const
{
  return NULL;
}

//------------------------------------------------------------------
MathUserData *RayData2::userDataPtr(const std::string &name)
{
  return NULL;
}

//------------------------------------------------------------------
// virtual 
bool RayData2::processUserLoopFunction(ProcessingNode &p)
{
  string keyword;
  if (!p.isUserUnaryOp(keyword))
  {
    return false;
  }

  if (keyword == "Variance2d")
  {
    return _processVariance2d(*(p.unaryOpArgs()));
  }
  else
  {
    printf("Unknown keyword %s\n", keyword.c_str());
    return false;
  }
}

//------------------------------------------------------------------
MathUserData *RayData2::processUserLoop2dFunction(const UnaryNode &p)
{
  LOG(ERROR) << "Not implemented for this class";
  return NULL;
}

//------------------------------------------------------------------
// virtual
bool RayData2::synchUserDefinedInputs(const std::string &userKey,
				      const std::vector<std::string> &names)
{
  _inps.clear();
  _outputSweep = NULL;
  if (!_needToSynch(userKey))
  {
    return true;
  }
  for (size_t i=0; i<names.size(); ++i)
  {
    RayLoopSweepData *m = _refToData(names[i], false);
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
bool RayData2::storeMathUserData(const std::string &name, MathUserData *s){
  LOG(WARNING) << "RayData2 does not support user data yet storing " << name;
  return false;
}

//------------------------------------------------------------------
// virtual
bool RayData2::smooth(MathLoopData *l,
		      std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

// virtual
//------------------------------------------------------------------
bool RayData2::smoothDBZ(MathLoopData *l,
		      std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

//------------------------------------------------------------------
// virtual
bool RayData2::stddev(MathLoopData *l,
		      std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}
//------------------------------------------------------------------
// virtual
bool RayData2::fuzzy(MathLoopData *l,
		     std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

//------------------------------------------------------------------
// virtual
bool RayData2::average(MathLoopData *l,
		     std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

//------------------------------------------------------------------
// virtual
bool RayData2::max_expand(MathLoopData *l,
			 std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

//------------------------------------------------------------------
// virtual
bool RayData2::weighted_average(MathLoopData *l,
				std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

bool RayData2::mask(MathLoopData *l,
		     std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

bool RayData2::median(MathLoopData *l,
		     std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

bool RayData2::weighted_angle_average(MathLoopData *l,
		     std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

bool RayData2::expand_angles_laterally(MathLoopData *l,
		     std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

bool RayData2::clump(MathLoopData *l,
		     std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

bool RayData2::mask_missing_to_missing(MathLoopData *out,
				       std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

bool RayData2::trapezoid(MathLoopData *out,
			 std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

bool RayData2::s_remap(MathLoopData *out,
		       std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

bool RayData2::max(MathLoopData *out,
		   std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

//------------------------------------------------------------------
RayLoopSweepData *RayData2::_refToData(const std::string &name,
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
  RayLoopSweepData r;
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
RayLoopSweepData *RayData2::_exampleData(const std::string &name)
{
  RayLoopSweepData *s = _refToData(name, true);
  if (s == NULL)
  {
    RayLoopSweepData r;
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
bool RayData2::_processVariance2d(std::vector<ProcessingNode *> &args)
{
  if (_lookup == NULL)
  {
    LOG(ERROR) << "No lookup";
    return false;
  }  

  const MathLoopData  *data = loadData(args, 0);
  if (data == NULL)
  {
    LOG(ERROR) << "Wrong interface";
    return false;
  }

  const RayLoopSweepData *rdata = dynamic_cast<const RayLoopSweepData *>(data);

  // // look for the one input in the _inps
  // RayLoopSweepData *inp = NULL;
  // for (size_t i=0; i<_inps.size(); ++i)
  // {
  //   if (_inps[i]->getName() == dataName)
  //   {
  //     inp = _inps[i];
  //     break;
  //   }
  // }

  // if (inp == NULL)
  // {
  //   LOG(ERROR) << "Data misaligned";
  //   return false;
  // }

  // the output should already be in place, so just go for it
  return _outputSweep->variance2d(*rdata, *_lookup);
}

//------------------------------------------------------------------
RayLoopSweepData *RayData2::_match(const std::string &n) 
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
const RayLoopSweepData *RayData2::_matchConst(const std::string &n) const
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
bool RayData2 ::_needToSynch(const std::string &userKey) const
{
  if (userKey == "Variance2d")
  {
    // this loop filter has inputs
    return true;
  }

  // everything else has inputs
  return true;
}

//---------------------------------------------------------------
void RayData2::_updateSweep(void)
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
void RayData2::_updateSweepOneDataset(int k)
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
bool RayData2::_newLoopData(const std::string &name,
			    RayLoopSweepData &ret, bool warn) const
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
  ret = RayLoopSweepData(name, _i0, _i1, rays);
  return true;
}

//------------------------------------------------------------------
bool RayData2::_anyLoopData(RayLoopSweepData &ret) const
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

  
