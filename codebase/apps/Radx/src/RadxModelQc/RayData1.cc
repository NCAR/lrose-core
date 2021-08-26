#include "RayData1.hh"
#include "Volume.hh"
#include "AzGradientFilter.hh"
#include "QscaleFilter.hh"
#include "Clutter2dQualFilter.hh"
#include "Special0Filter.hh"
#include "Special1Filter.hh"
#include "FIRFilter.hh"
#include "Variance1dFilter.hh"
#include "AzGradientStateSpecialData.hh"
#include <rapmath/FunctionDef.hh>
#include <rapmath/ProcessingNode.hh>
#include <toolsa/LogStream.hh>

const std::string RayData1::_azGradientStr = "AzGradient";
const std::string RayData1::_qscaleStr = "Qscale";
const std::string RayData1::_oneMinusQscaleStr = "OneMinusQscale";
const std::string RayData1::_clutter2dQualStr = "CLUTTER_2D_QUAL";
const std::string RayData1::_special0Str = "Special0";
const std::string RayData1::_special1Str = "Special1";
const std::string RayData1::_FIRStr = "FIR";
const std::string RayData1::_variance1dStr = "Variance1d";

//------------------------------------------------------------------
RayData1::RayData1(void) : RadxAppRayData(), _ray0(NULL), _ray1(NULL)
{
}

//------------------------------------------------------------------
RayData1::RayData1(const Volume &r, int index) :
  RadxAppRayData(r, r.specialRef(), index), _ray0(NULL), _ray1(NULL)
{
  const std::vector<RadxRay *> *rays = r.getRaysPtr();
  int nr = rays->size();
  if (index == 0)
  {
    if (nr > 1)
    {
      _ray1 = (*rays)[1];
    }
  }
  else if (index == nr-1)
  {
    _ray0 = (*rays)[nr-2];
  }
  else
  {
    if (nr > 2)
    {
      _ray0 = (*rays)[index-1];
      _ray1 = (*rays)[index+1];
    }
  }

  // transfer values from volume object to here
  _special = r.specialRef();
  _special.setOwnership(false);
}  

//------------------------------------------------------------------
RayData1::~RayData1(void)
{
  // nothing owned by this class
}

//------------------------------------------------------------------
bool RayData1::radxappUserLoopFunction(const std::string &keyword,
				       ProcessingNode &p)
{
  if (keyword == _azGradientStr)
  {
    return _processAzGradient(*(p.unaryOpArgs()));
  }
  else if (keyword == _qscaleStr)
  {
    return _processQscale(*(p.unaryOpArgs()), false);
  }
  else if (keyword == _oneMinusQscaleStr)
  {
    return _processQscale(*(p.unaryOpArgs()), true);
  }
  else if (keyword == _clutter2dQualStr)
  {
    return _processClutter2dQual(*(p.unaryOpArgs()));
  }
  else if (keyword == _special0Str)
  {
    return _processSpecial0(*(p.unaryOpArgs()));
  }
  else if (keyword == _special1Str)
  {
    return _processSpecial1(*(p.unaryOpArgs()));
  }
  else if (keyword == _FIRStr)
  {
    return _processFIR(*(p.unaryOpArgs()));
  }
  else if (keyword == _variance1dStr)
  {
    return _processVariance1d(*(p.unaryOpArgs()));
  }
  else
  {
    LOG(ERROR) << "Unknown keyword " << keyword;
    return false;
  }
}

//------------------------------------------------------------------
MathUserData *RayData1::radxappUserLoopFunctionToUserData(const UnaryNode &p)
{
  LOG(ERROR) << "Not implemented for this class";
  return NULL;
}

//------------------------------------------------------------------
void RayData1::radxappAppendUnaryOperators(std::vector<FunctionDef> &ret) const
{
  ret.push_back(FunctionDef(_azGradientStr, "v", "field, azGradientState, bias",
			    "Azimuthal Gradient of input field, where "
			    "azGradientState = 2d state object produced by VolAzGradientState filter\n"
			    "bias = number added to output values\n"
			    "NOTE this only works for input radar fields not derived fields"));
  ret.push_back(FunctionDef(_qscaleStr, "v", "field, scale, top, low",
			    "QScale filter on a field, which is exp(-scale*(field/topv-lowv/topv)^2)"));
  ret.push_back(FunctionDef(_oneMinusQscaleStr, "v", "field, scale, top, low",
			    "1 - Qscale of a field, i.e.:  1 - exp(-scale*(field/topv-lowv/topv)^2)"));
  ret.push_back(FunctionDef(_clutter2dQualStr, "v",
			    "scr,vel,width,scale,vr_shape,sw_shape",
			    "Quality from fields scr, vel, width, with params scAle, vr_shape and sw_shape,  defined as "
			    "1 - exp(-scale*(|vel|*vr_shape + width*sw_shape).   Note that scr and scale are not used right now"));
  
  ret.push_back(FunctionDef(_special0Str, "v", "width,meanPrt,meanNSamples",
	  " A particular function:   10*log10(1+sqrt(1/(width*(4*sqrt(PI)*meanPrt*meanNsamples/0.1))))"));

  ret.push_back(FunctionDef(_special1Str, "v", "width,meanPrt,meanNSamples",
			    " A particular function:  0.107/(8*meanPrt*meanNsamples*sqrt(PI)*width)"));
  ret.push_back(FunctionDef(_FIRStr, "v", "field",
		    "FIR filter with hardwired coefficients that are in the source code, interpolation"));
  ret.push_back(FunctionDef(_variance1dStr, "v", "field,npt,maxPercentMissing",
			    "1 dimensional Variance of input field.  variance over npt points, with output set missing if more than maxPercentMissing values are missing"));
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

  return filter.filter(*state, v, *r, name, *this, _outputRay);
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
    LOG(ERROR) << "No values";
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
    LOG(ERROR) << "No values";
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
  const RayxData *rd = dynamic_cast<const RayxData *>(data);
  QscaleFilter filter;
  return filter.filter(*rd, scale, topv, lowv, subtractFromOne,
		       _outputRay);
}

//------------------------------------------------------------------
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
  
  Clutter2dQualFilter filter;
  return filter.filter(*scr, *vel, *width, scale, vr_shape, sw_shape,
		       _outputRay);
}
