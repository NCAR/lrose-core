/**
 * @file Sweep.cc
 */

//------------------------------------------------------------------
#include "Sweep.hh"
#include "Volume.hh"
#include "MesoTemplate.hh"
#include "NyquistTest.hh"
#include "GridExpand.hh"
#include "FullTemplateExtent.hh"
#include <euclid/GridAlgs.hh>
#include <euclid/Grid2dClump.hh>
#include <rapmath/MathParser.hh>
#include <rapmath/ProcessingNode.hh>
#include <rapmath/UnaryNode.hh>
#include <toolsa/LogStream.hh>
#include <cmath>

const std::string Sweep::_mesoTemplateStr = "MesoTemplate";
const std::string Sweep::_nyquistTestStr = "NyquistTest";
const std::string Sweep::_azShearStr = "AzShear";
const std::string Sweep::_expandStr = "Expand";
const std::string Sweep::_clumpLocStr = "ClumpLoc";
const std::string Sweep::_clumpExtentStr = "ClumpExtent";
const std::string Sweep::_clumpFiltStr = "ClumpFilt";
const std::string Sweep::_interestInMaskStr = "InterestInMask";
//const std::string Sweep::_dist2MissingStr = "Dist2Missing";
  
//------------------------------------------------------------------
static bool _adjust(int ny, bool circular, int &a)
{
  if (a >= (int)(1.5*ny) || a < (int)(-1.5*ny))
  {
    return false;
  }
  
  if (a >= ny)
  {
    if (circular)
    {
      a = a - ny;
    }
    else
    {
      return false;
    }
  }
  if (a < 0)
  {
    if (circular)
    {
      a = a + ny;
    }
    else
    {
      return false;
    }
  }
  return true;
}

//------------------------------------------------------------------
static bool _adjust(int ny, bool circular, int &aplus, int &aminus)
{
  if (!_adjust(ny, circular, aplus))
  {
    return false;
  }
  if (!_adjust(ny, circular, aminus))
  {
    return false;
  }
  return true;
}

//------------------------------------------------------------------
Sweep::Sweep(void) : VirtVolSweep()
{
}
//------------------------------------------------------------------
Sweep::Sweep(const Volume &volume, int index, double vlevel) :
  VirtVolSweep(volume, index, vlevel)
{
  _parms = volume._parms;
  _templates = &(volume._templates);
  _special = *(_inpSpecial);
  _special.setOwnership(false);
}

//------------------------------------------------------------------
Sweep::~Sweep(void)
{
  // input special not handled here
  // the sounding pointers not handled here
  // the freezingHeight pointers not handled here
  // this histo2d pointers are not handled here
}

//------------------------------------------------------------------
std::vector<FunctionDef> Sweep::userUnaryOperators(void) const
{
  std::vector<FunctionDef> ret;
  ret.push_back(FunctionDef(_mesoTemplateStr, "M", "VEL,template",
			    ""));
  ret.push_back(FunctionDef(_nyquistTestStr, "M", "VEL,Mask,template",
			    ""));
  ret.push_back(FunctionDef(_azShearStr, "M", "VEL,widthKm,clockwise", ""));
  ret.push_back(FunctionDef(_expandStr, "M", "DBZ,nr", ""));
  ret.push_back(FunctionDef(_clumpFiltStr, "M", "data,data2", ""));
  ret.push_back(FunctionDef(_clumpExtentStr, "M", "data, template", ""));
  ret.push_back(FunctionDef(_clumpLocStr, "M", "data, template", ""));
  ret.push_back(FunctionDef(_interestInMaskStr, "M", "data, clumps, minInClump", ""));
  //ret.push_back(FunctionDef(_dist2MissingStr, "M", "DBZ,maxSearch,scale", ""));
  return ret;
}


//------------------------------------------------------------------
bool Sweep::synchInputsAndOutputs(const std::string &output,
				  const std::vector<std::string> &inputs)
{
  bool haveAll=false;
  if (!synchGriddedInputsAndOutputs(output, inputs, haveAll))
  {
    return false;
  }
  
  if (!haveAll)
  {
    size_t num = _inps.size();
    for (size_t i=0; i<inputs.size(); ++i)
    {
      if (_inpSpecial->hasName(inputs[i]))
      {
	++num;
      }
    }
    haveAll = (num == inputs.size());
  }	       
  if (_outputSweep == NULL || !haveAll)
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
bool Sweep::processUserLoopFunction(ProcessingNode &p)
{
  string keyword;
  if (!p.isUserUnaryOp(keyword))
  {
    return false;
  }

  if (keyword == _mesoTemplateStr)
  {
    return _processMesoTemplate(*(p.unaryOpArgs()));
  }
  if (keyword == _nyquistTestStr)
  {
    return _processNyquistTest(*(p.unaryOpArgs()));
  }
  if (keyword == _clumpFiltStr)
  {
    return _processClumpFilt(*(p.unaryOpArgs()));
  }
  else if (keyword == _azShearStr)
  {
    return _processAzShear(*(p.unaryOpArgs()));
  }
  else if (keyword == _expandStr)
  {
    return _processExpand(*(p.unaryOpArgs()));
  }
  else if (keyword == _clumpExtentStr)
  {
    return _processClumpExtent(*(p.unaryOpArgs()), true);
  }
  else if (keyword == _clumpLocStr)
  {
    return _processClumpExtent(*(p.unaryOpArgs()), false);
  }
  else if (keyword == _interestInMaskStr)
  {
    return _processInterestInMask(*(p.unaryOpArgs()));
  }
  // else if (keyword == _dist2MissingStr)
  // {
  //   return _processDist2Missing(*(p.unaryOpArgs()));
  // }
  else
  {
    printf("Unknown keyword %s\n", keyword.c_str());
    return false;
   }
}

//------------------------------------------------------------------
MathUserData *Sweep::processUserLoopFunctionToUserData(const UnaryNode &p)
{
  string keyword;
  if (!p.getUserUnaryKeyword(keyword))
  {
    return NULL;
  }
  LOG(ERROR) << " Unknown keyword " << keyword;
  return NULL;
}

//------------------------------------------------------------------
bool Sweep::synchUserDefinedInputs(const std::string &userKey,
				   const std::vector<std::string> &names)
{
  return true;
}

//------------------------------------------------------------------
bool Sweep::_processMesoTemplate(std::vector<ProcessingNode *> &args)
{
  const MathLoopData *data;
  // double yoffset, y, x;
  MathUserData *udata;
  // if (!loadDataAndThreeNumbers(args, &data, x, y, yoffset))
  if (!loadDataAndUserData(args, &data, &udata))
  {
    return false;
  }

  const GriddedData *input = (const GriddedData *)data;
  Grid2d g(*input);
  g.changeMissing(-99);
  g.setAllMissing();

  TemplateLookupMgr *t = (TemplateLookupMgr *)udata;

  // find a matching template
  // const TemplateLookupMgr *t=NULL;
  // for (size_t i=0; i<_templates->size(); ++i)
  // {
  //   if ((*_templates)[i].match(x, y, yoffset))
  //   {
  //     t = &((*_templates)[i]);
  //     break;
  //   }
  // }
  // if (t == NULL)
  // {
  //   LOG(ERROR) << "NO matching template";
  //   return false;
  // }

  MesoTemplate m(_parms, t);
  m.apply(*input, *this, g);
  
  _outputSweep->dataCopy(g);
  return true;
    
}

//------------------------------------------------------------------
bool Sweep::_processClumpExtent(std::vector<ProcessingNode *> &args,
				bool doExtend)
{
  const MathLoopData *data;
  MathUserData *udata;
  if (!loadDataAndUserData(args, &data, &udata))
  {
    return false;
  }

  const GriddedData *input = (const GriddedData *)data;
  Grid2d g(*input);
  g.changeMissing(-99);
  g.setAllMissing();

  const TemplateLookupMgr *t = (const TemplateLookupMgr *)udata;
  // // find a matching template
  // const TemplateLookupMgr *t=NULL;
  // for (size_t i=0; i<_templates->size(); ++i)
  // {
  //   if ((*_templates)[i].match(x, y, yoffset))
  //   {
  //     t = &((*_templates)[i]);
  //     break;
  //   }
  // }
  // if (t == NULL)
  // {
  //   LOG(ERROR) << "NO matching template";
  //   return false;
  // }

  FullTemplateExtent m(_parms, t, doExtend);
  m.apply(*input, *this, g);
  
  _outputSweep->dataCopy(g);
  return true;
    
}

//------------------------------------------------------------------
bool Sweep::_processNyquistTest(std::vector<ProcessingNode *> &args)
{
  const MathLoopData *data, *mask;
  const MathUserData *udata;

  int n = (int)args.size();
  if (n != 3)
  {
    LOG(ERROR) << "Need 3 args";
    return false;
  }
  string dataName = args[0]->leafName();
  if (dataName.empty())
  {
    LOG(ERROR) << "No named arg 0";
    return false;
  }
  data = dataPtrConst(dataName);
  if (data == NULL)
  {
    LOG(ERROR) << "No data for " << dataName;
    return false;
  }

  string maskName = args[1]->leafName();
  if (maskName.empty())
  {
    LOG(ERROR) << "No named arg 1";
    return false;
  }
  mask = dataPtrConst(maskName);
  if (mask == NULL)
  {
    LOG(ERROR) << "No data for " << maskName;
    return false;
  }

  string tempName = args[2]->leafName();
  if (tempName.empty())
  {
    LOG(ERROR) << "No named arg 2";
    return false;
  }
  udata = userDataPtrConst(tempName);
  if (udata == NULL)
  {
    LOG(ERROR) << "No data for " << tempName;
    return false;
  }
  const GriddedData *input = (const GriddedData *)data;
  const GriddedData *inpMask = (const GriddedData *)mask;
  Grid2d g(*input);
  g.changeMissing(-99);
  g.setAllMissing();

  // find a matching template
  const TemplateLookupMgr *t = (const TemplateLookupMgr *)udata;
  // const TemplateLookupMgr *t=NULL;
  // for (size_t i=0; i<_templates->size(); ++i)
  // {
  //   if ((*_templates)[i].match(x, y, yoffset))
  //   {
  //     t = &((*_templates)[i]);
  //     break;
  //   }
  // }
  // if (t == NULL)
  // {
  //   LOG(ERROR) << "NO matching template";
  //   return false;
  // }

  NyquistTest m(_parms, t);
  m.apply(*input, *inpMask, *this, g);
  
  _outputSweep->dataCopy(g);
  return true;
    
}

//------------------------------------------------------------------
bool Sweep::_processClumpFilt(std::vector<ProcessingNode *> &args)
{
  const MathLoopData *data, *data2;

  int n = (int)args.size();
  if (n != 2)
  {
    LOG(ERROR) << "Need 2 args";
    return false;
  }
  string dataName = args[0]->leafName();
  if (dataName.empty())
  {
    LOG(ERROR) << "No named arg 0";
    return false;
  }
  data = dataPtrConst(dataName);
  if (data == NULL)
  {
    LOG(ERROR) << "No data for " << dataName;
    return false;
  }

  dataName = args[1]->leafName();
  if (dataName.empty())
  {
    LOG(ERROR) << "No named arg 1";
    return false;
  }
  data2 = dataPtrConst(dataName);
  if (data2 == NULL)
  {
    LOG(ERROR) << "No data for " << dataName;
    return false;
  }

  const GriddedData *input = (const GriddedData *)data;
  const GriddedData *inp2 = (const GriddedData *)data2;

  Grid2d g(*input);
  // g.changeMissing(-99);
  // g.setAllMissing();

  Grid2dClump c(*input);
  std::vector<clump::Region_t> clumps = c.buildRegions();
  for (size_t i=0; i<clumps.size(); ++i)
  {
    clump::Region_citer_t c;
    int minx=-1, maxx=-1;
    for (c=clumps[i].begin(); c!=clumps[i].end(); ++c)
    {
      if (minx == -1)
      {
	minx = maxx = c->first;
      }
      else
      {
	int x = c->first;
	if (x < minx) minx = x;
	if (x > maxx) maxx = x;
      }
      double v;
      if (inp2->getValue(c->first, c->second, v))
      {
	double v2;
	if (g.getValue(c->first, c->second, v2))
	{
	  // the value is 1 if you are close to the nyquist, 0 if not
	  g.setValue(c->first, c->second, v2*(1.0-v));
	}
      }
      else
      {
	g.setMissing(c->first, c->second);
      }
    }

    double lengthKm = (double)(maxx-minx)*_proj.xGrid2km(1.0);  // km
    double scale = _parms._radialFuzzy.apply(lengthKm);
    
    for (c=clumps[i].begin(); c!=clumps[i].end(); ++c)
    {
      double v;
      if (g.getValue(c->first, c->second, v))
      {
	g.setValue(c->first, c->second, v*scale);
      }
    }
  }
  
  _outputSweep->dataCopy(g);
  return true;
}

//------------------------------------------------------------------
bool Sweep::_processAzShear(std::vector<ProcessingNode *> &args)
{
  const MathLoopData *data;
  double widthKm, dclockwise;
  if (!loadDataValueValue(args, &data, widthKm, dclockwise))
  {
    LOG(ERROR) << "No data";
    return false;
  }
  bool clockwise = (dclockwise == 1.0);

  bool circular = isCircular();
  const GriddedData *input = (const GriddedData *)data;

  Grid2d g(*input);
  g.changeMissing(-99);
  g.setAllMissing();

  Mdvx::coord_t c = _proj.getCoord();
  double x0Km = c.minx;
  double deltaPlus, deltaMinus;
  double dxKm = _proj.xGrid2km(1.0);  // km
  double dyDegrees = _proj.yGrid2km(1.0);  /// actualy degrees
  double dyRadians = dyDegrees*3.14159/180.0;
  
  if (clockwise)
  {
    if (VirtVolSweep::clockwise())
    {
      // want larger azimuth to be positive, smaller to be negative
      deltaMinus = widthKm/2;
      deltaPlus = -widthKm/2;
    }
    else 
    {
      // want larger azimuth to be negative, smaller to be positive
      deltaMinus = -widthKm/2;
      deltaPlus = widthKm/2;
    }
  }
  else
  {
    if (VirtVolSweep::clockwise())
    {
      // want larger azimuth to be negative, smaller to be positive
      deltaMinus = -widthKm/2;
      deltaPlus = widthKm/2;
    }
    else 
    {
      // want larger azimuth to be negative, smaller to be positive
      deltaMinus = widthKm/2;
      deltaPlus = -widthKm/2;
    }
  }
  
  // do the shear thing
  for (int r=0; r<g.getNx(); ++r)
  {
    double R = x0Km + r*dxKm;
    // want arc length to be deltaPlus/deltaMinus in km
    int kplus = (int)deltaPlus/(R*dyRadians);
    int kminus = (int)deltaMinus/(R*dyRadians);
    for (int a=0; a<g.getNy(); ++a)
    {
      if (kplus == kminus)
      {
	// skip this point
	LOG(DEBUG_VERBOSE) << " skip radius " << R;
	g.setMissing(r, a);
      }
      else
      {
	int aplus = a + kplus;
	int aminus = a + kminus;
	if (_adjust(g.getNy(), circular, aplus, aminus))
	{
	  _azShear(*input, r, a, aplus, aminus, g);
	}
      }
    }
  }  

  _outputSweep->dataCopy(g);
  return true;
    
}

//------------------------------------------------------------------
bool Sweep::_processInterestInMask(std::vector<ProcessingNode *> &args)
{
  vector<const MathLoopData *> vdata;
  vector<double> vvalue;
  if (!loadMultiDataAndMultiValues(args, 2, vdata, 1, vvalue))
  {
    LOG(ERROR) << "Loading data";
    return false;
  }
  double minInterest = vvalue[0];
  const GriddedData *interest = (const GriddedData *)(vdata[0]);
  const GriddedData *clumps = (const GriddedData *)(vdata[1]);
  GridAlgs clumpAlg(*clumps);
  Grid2d outp(*interest);
  outp.setAllMissing();
  vector<double> vvalues = clumpAlg.listValues(1000);
  LOG(DEBUG) << "Number of clumps = " << vvalues.size();
  for (size_t i=0; i<vvalues.size(); ++i)
  {
    vector<pair<int,int> > pi = clumpAlg.pointsAtValue(vvalues[i]);
    for (size_t j = 0; j < pi.size(); ++j)
    {
      double v;
      if (interest->getValue(pi[j].first, pi[j].second, v))
      {
	outp.setValue(pi[j].first, pi[j].second, v);
      }
      else
      {
	outp.setValue(pi[j].first, pi[j].second, minInterest);
      }
    }
  }
  _outputSweep->dataCopy(outp);
  return true;
  
}

//------------------------------------------------------------------
bool Sweep::_processExpand(std::vector<ProcessingNode *> &args)
{
  const MathLoopData *data;
  double dnx;
  if (!loadDataValue(args, &data, dnx))
  {
    LOG(ERROR) << "No data";
    return false;
  }
  int nx = (int)dnx;

  const GriddedData *input = (const GriddedData *)data;
  Grid2d g(*input);
  GridExpand G(nx);
  G.update(*input, _proj, g);

  // GridAlgs g(*input);
  // g.dilate(nr, naz);
  _outputSweep->dataCopy(g);
  return true;
}

// //------------------------------------------------------------------
// bool Sweep::_processDist2Missing(std::vector<ProcessingNode *> &args)
// {
//   const MathLoopData *data;
//   double dmaxSearch, dsearchScale;
//   if (!loadDataValueValue(args, &data, dmaxSearch, dsearchScale))
//   {
//     LOG(ERROR) << "No data";
//     return false;
//   }
//   int maxSearch  = (int)dmaxSearch;
//   int searchScale = (int)dsearchScale;

//   GridDistToNonMissing G(maxSearch, searchScale);
//   const GriddedData *input = (const GriddedData *)data;
//   Grid2d g(*input);
//   Grid2d v(*input);
//   G.update(*input, _proj);
//   G.distanceToNonMissing(_proj, *input, g, v);
//   _outputSweep->dataCopy(g);
//   return true;
// }

//------------------------------------------------------------------
bool Sweep::_needToSynch(const std::string &userKey) const
{
  // everything else has inputs
  return true;
}

//------------------------------------------------------------------
void Sweep::_azShear(const Grid2d &input, int r, int a, int aplus,
		     int aminus, Grid2d &g) const
{
  double vplus, vminus;
  if (input.getValue(r, aplus, vplus) &&
      input.getValue(r, aminus, vminus))
  {
    g.setValue(r, a, vplus - vminus);
  }
}
