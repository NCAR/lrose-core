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
RayData1::RayData1(void) : MathData(), _ray(NULL), _ray0(NULL), _ray1(NULL)
{
}

//------------------------------------------------------------------
RayData1::RayData1(const RayData &r, int index) :
  MathData(), _ray(NULL), _ray0(NULL), _ray1(NULL)
{
  _ray = (*r._rays)[index];
  _ray0 = NULL;
  _ray1 = NULL;
  int nr = (int)r._rays->size();
  if (index == 0)
  {
    if (nr > 1)
    {
      _ray1 = (*r._rays)[1];
    }
  }
  else if (index == nr-1)
  {
    _ray0 = (*r._rays)[nr-2];
  }
  else
  {
    if (nr > 2)
    {
      _ray0 = (*r._rays)[index-1];
      _ray1 = (*r._rays)[index+1];
    }
  }

  // transfer values from volume object to here
  _special = r._special;
  _special.setOwnership(false);
}  

//------------------------------------------------------------------
RayData1::~RayData1(void)
{
  // nothing owned by this class
}

//------------------------------------------------------------------
std::vector<std::pair<std::string, std::string> >
RayData1::userUnaryOperators(void) const
{
  std::vector<std::pair<std::string, std::string> > ret;

  string s;
  s = "v=AzGradient(field, azgradstate, bias) = Azimuthal Gradient\n"
    "  azgradstate = 2d state object,  bias = added to results\n"
    "  NOTE this only works for input radar fields, not derived fields";
  ret.push_back(pair<string,string>("AzGradient", s));

  s = "v=Qscale(field, scale, top, low) = exp(-scale*(field/topv-lowv/topv)^2)";
  ret.push_back(pair<string,string>("Qscale", s));

  s = "v=OneMinusQscale(field, scale, top, low)= 1-Qscale(field,scale,top,low)";
  ret.push_back(pair<string,string>("OneMinusQscale", s));

  s = "v=CLUTTER_2D_QUAL(scr, vel, width, scale, vr_shape, sw_shape)\n"
    "  1 - exp(-scale*(|vel|*vr_shape + width*sw_shape))";
  ret.push_back(pair<string,string>("CLUTTER_2D_QUAL", s));

  s = "v=Special0(width, meanPrt, meanNSamples) = \n"
    "  10*log10(1 + sqrt(1/(width*(4*sqrt(PI)*meanPrt*meanNsamples/0.1))))";
  ret.push_back(pair<string,string>("Special0", s));

  s = "v=Special1(width, meanPrt, meanNSamples) =\n"
    "  0.107/(8*meanPrt*meanNsamples*sqrt(PI)*width)";
  ret.push_back(pair<string,string>("Special1", s));

  s = "v=FIR(field) = FIR filter with hardwired coefficients, interpolation";
  ret.push_back(pair<string,string>("FIR", s));

  s = "v=Variance1d(field, npt, maxPercentMissing) 1d Variance of input field\n"
    "  variance over npt points, with output set missing if too many missing";
  ret.push_back(pair<string,string>("Variance1d", s));
  return ret;
}

// //------------------------------------------------------------------
// const MathUserData *RayData1::specialDataPtrConst(const std::string &name) const
// {
//   for (size_t i=0; i<_specialInpNames.size(); ++i)
//   {
//     if (_specialInpNames[i] == name)
//     {
//       return _specialInps[i];
//     }
//   }
//   return NULL;
// }

// //------------------------------------------------------------------
// MathUserData *RayData1::specialDataPtr(const std::string &name)
// {
//   for (size_t i=0; i<_specialInpNames.size(); ++i)
//   {
//     if (_specialInpNames[i] == name)
//     {
//       return _specialInps[i];
//     }
//   }
//   return NULL;
// }


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
  // add all the derived data to the main ray (input vol not needed because
  // the local object points to the same data)
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
const MathUserData *RayData1::userDataPtrConst(const std::string &name) const
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
MathUserData *RayData1::userDataPtr(const std::string &name)
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
  _specialInp = SpecialUserData(false);
  
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
  return _special.store(name, s);
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
bool RayData1::weighted_angle_average(MathLoopData *l,
				      std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

//------------------------------------------------------------------
// virtual
bool RayData1::median(MathLoopData *l,
		      std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

//------------------------------------------------------------------
// virtual
bool RayData1::max(MathLoopData *l,
		      std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

//------------------------------------------------------------------
// virtual
bool RayData1::max_expand(MathLoopData *l,
			 std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

bool RayData1::expand_angles_laterally(MathLoopData *l,
		     std::vector<ProcessingNode *> &args) const
{
  LOG(ERROR) << "Not implemented";
  return false;
}

bool RayData1::clump(MathLoopData *l,
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
  RayxData *r = RadxApp::retrieveRayPtr(name, *_ray, false);
  if (r != NULL)
  {
    // add this ray to the data state and return that
    _data.push_back(RayLoopData(*r));
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
RayLoopData *RayData1::_exampleData(const std::string &name)
{
  // see if it is already sitting there:
  RayLoopData *s = (RayLoopData *)_refToData(name, true);
  if (s == NULL)
  {
    // not already in place, so we create a new one
    const RayxData *r = RadxApp::retrieveAnyRayPtr(*_ray);
    if (r != NULL)
    {
      // set it to the correct values
      RayLoopData store(*r);
      delete r;
      RadxApp::modifyRayForOutput(store, name, "units", store.getMissing());
      _data.push_back(store);
      // point to it for return
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
bool RayData1::_processAzGradient(std::vector<ProcessingNode *> &args)
{
  string name = getDataName(args, 0);
  const MathLoopData *data;
  MathUserData *udata;
  double v;
  if (!loadDataAndUserDataAndValue(args, &data, &udata, v))
  {
    return false;
  }
  

  
  AzGradientStateSpecialData *state = (AzGradientStateSpecialData *)udata;
  
  int nState = state->nstate();
  if (nState < 1)
  {
    LOG(ERROR) << "State is empty";
    return false;
  }


  AzGradientFilter filter;
  const RayxData *r = dynamic_cast<const RayxData *>(data);

  return filter.filter(*state, v, *r, name,
		       *this,
		       _outputRay);
}

//------------------------------------------------------------------
bool RayData1::_processSpecial0(std::vector<ProcessingNode *> &args)
{
  const MathLoopData *ldata;
  MathUserData *meanPrt, *meanNsamples;

  if (!loadDataAndTwoUserDatas(args, &ldata, &meanPrt, &meanNsamples))
  {
    return false;
  }

  double meanPrtV, meanNsamplesV;
  if (!meanPrt->getFloat(meanPrtV) || !meanNsamples->getFloat(meanNsamplesV))
  {
    printf("No values\n");
    return false;
  }
  
  Special0Filter filter;
  
  const RayxData *width = dynamic_cast<const RayxData *>(ldata);

  return filter.filter(*width, meanPrtV, meanNsamplesV, _outputRay);
}

//------------------------------------------------------------------
bool RayData1::_processSpecial1(std::vector<ProcessingNode *> &args)
{
  const MathLoopData *ldata;
  MathUserData *meanPrt, *meanNsamples;

  if (!loadDataAndTwoUserDatas(args, &ldata, &meanPrt, &meanNsamples))
  {
    return false;
  }

  double meanPrtV, meanNsamplesV;
  if (!meanPrt->getFloat(meanPrtV) || !meanNsamples->getFloat(meanNsamplesV))
  {
    printf("No values\n");
    return false;
  }
  
  Special1Filter filter;
  
  const RayxData *width = dynamic_cast<const RayxData *>(ldata);
  return filter.filter(*width, meanPrtV, meanNsamplesV, _outputRay);
}

//------------------------------------------------------------------
bool RayData1::_processFIR(std::vector<ProcessingNode *> &args) const
{
  const MathLoopData *d = loadData(args, 0);
  const RayxData *rd = dynamic_cast<const RayxData *>(d);

  
  FIRFilter filter;
  return filter.filter(*rd, "INTERP", _outputRay);
}

//------------------------------------------------------------------
bool RayData1::_processVariance1d(std::vector<ProcessingNode *> &args) const
{
  const MathLoopData *data;
  double npt, maxPctMissing;
  if (!loadDataValueValue(args, &data, npt, maxPctMissing))
  {
    return false;
  }

  Variance1dFilter filter(npt, maxPctMissing);
  const RayxData *rd = dynamic_cast<const RayxData *>(data);

  
  return filter.filter(*rd, _outputRay);
}

//------------------------------------------------------------------
bool RayData1::_processQscale(std::vector<ProcessingNode *> &args,
			     bool subtractFromOne) const
{
  const MathLoopData *data;
  double scale, topv, lowv;
  if (!loadDataAndThreeNumbers(args, &data, scale, topv, lowv))
  {
    return false;
  }
  // if (args.size() != 4)
  // {
  //   printf("bad args\n");
  //   return false;
  // }

  // // get the variable to work on
  // string name = args[0]->leafName();
  // if (name.empty())
  // {
  //   LOG(ERROR) << " NO name";
  //   return false;
  // }

  // double scale, topv, lowv;
  // if (args[1]->getValue(scale) &&
  //     args[2]->getValue(topv) &&
  //     args[3]->getValue(lowv))
  // {
  //   // printf("Qscale %s %lf %lf %lf\n",
  //   // 	   name.c_str(), scale, topv, lowv);
  // }
  // else
  // {
  //   LOG(ERROR) << "Getting qscale args";
  //   return false;
  // }
  
  const RayxData *rd = dynamic_cast<const RayxData *>(data);
  QscaleFilter filter;
  return filter.filter(*rd, scale, topv, lowv, subtractFromOne,
		       _outputRay);
}

//------------------------------------------------------------------
// clutter2dqual(scr, vel, width, 0.69, 1.5, 0.5)
// clutter2dqual(scr, vel, width, scale, vr_shape, sw_shape)

bool RayData1::_processClutter2dQual(std::vector<ProcessingNode *> &args) const
{
  vector<const MathLoopData *> data;
  vector<double> values;

  if (!loadMultiDataAndMultiValues(args, 3, data, 3, values))
  {
    return false;
  }
  double scale, vr_shape, sw_shape;
  scale = values[0];
  vr_shape = values[1];
  sw_shape = values[2];
  const RayxData *scr = dynamic_cast<const RayxData *>(data[0]);
  const RayxData *vel = dynamic_cast<const RayxData *>(data[1]);
  const RayxData *width = dynamic_cast<const RayxData *>(data[2]);
  
  // if (args.size() != 6)
  // {
  //   printf("bad args\n");
  //   return false;
  // }

  // // get the variables to work on
  // string scrName = args[0]->leafName();
  // if (scrName.empty())
  // {
  //   LOG(ERROR) << " NO name";
  //   return false;
  // }
  // string velName = args[2]->leafName();
  // if (velName.empty())
  // {
  //   LOG(ERROR) << " NO name";
  //   return false;
  // }

  // string widthName = args[4]->leafName();
  // if (widthName.empty())
  // {
  //   LOG(ERROR) << " NO name";
  //   return false;
  // }

  // if (args[1]->getValue(scale) &&
  //     args[3]->getValue(vr_shape) &&
  //     args[5]->getValue(sw_shape))
  // {
  //   // printf("Qscale %s %lf %lf %lf\n",
  //   // 	   name.c_str(), scale, topv, lowv);
  // }
  // else
  // {
  //   LOG(ERROR) << "Getting qscale args";
  //   return false;
  // }
  
  Clutter2dQualFilter filter;
  return filter.filter(*scr, *vel, *width, scale, vr_shape, sw_shape,
		       _outputRay);
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
bool RayData1::_synchInput(const std::string &name)
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
    if (_special.hasName(input))
    {
      _specialInp.store(input, _special.matchingDataPtr(input));
    }
  }
}
