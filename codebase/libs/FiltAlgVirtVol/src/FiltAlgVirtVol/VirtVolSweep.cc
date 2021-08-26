/**
 * @file VirtVolSweep.cc
 */

//------------------------------------------------------------------
#include <FiltAlgVirtVol/VirtVolSweep.hh>
#include <FiltAlgVirtVol/VirtVolVolume.hh>
#include <FiltAlgVirtVol/PolarCircularFilter.hh>
#include <FiltAlgVirtVol/VirtVolFuzzy.hh>
#include <euclid/GridAlgs.hh>
#include <euclid/Grid2dClump.hh>
#include <rapmath/MathParser.hh>
#include <rapmath/ProcessingNode.hh>
#include <rapmath/TrapFuzzyF.hh>
#include <rapmath/SFuzzyF.hh>
#include <rapmath/FuzzyF.hh>
#include <rapmath/UnaryNode.hh>
#include <radar/NcarParticleId.hh>
#include <radar/KdpBringi.hh>
#include <radar/KdpFilt.hh>
#include <toolsa/LogStream.hh>
#include <cmath>
#include <algorithm>

const std::string VirtVolSweep::_percentLessThanStr = "PercentLessThan";
const std::string VirtVolSweep::_largePositiveNegativeStr = "LargePositiveNegative";
const std::string VirtVolSweep::_smoothPolarStr = "SmoothPolar";
const std::string VirtVolSweep::_smoothWithThreshPolarStr = "SmoothWithThreshPolar";
const std::string VirtVolSweep::_dilatePolarStr = "DilatePolar";
const std::string VirtVolSweep::_medianPolarStr = "MedianPolar";
const std::string VirtVolSweep::_azimuthalPolarShearStr = "AzShearPolar";
const std::string VirtVolSweep::_percentOfAbsMaxStr = "PercentOfAbsMax";
const std::string VirtVolSweep::_clumpFiltStr = "ClumpFilt";
const std::string VirtVolSweep::_virtVolFuzzyStr = "VirtVolFuzzy";
const std::string VirtVolSweep::_expandInMaskStr = "ExpandInMask";

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
static void _azShear(const Grid2d &input, int r, int a, int aplus,
		     int aminus, Grid2d &g)
{
  double vplus, vminus;
  if (input.getValue(r, aplus, vplus) &&
      input.getValue(r, aminus, vminus))
  {
    g.setValue(r, a, vplus - vminus);
  }
}
//------------------------------------------------------------------
VirtVolSweep::VirtVolSweep(void) : _grid2d(NULL)
{
}

//------------------------------------------------------------------
VirtVolSweep::VirtVolSweep(const VirtVolVolume &volume,
			   int index, double vlevel)
{
  _time = volume._time;
  _proj = volume.proj();
  _clockwise = volume.clockwise();
  _vlevel = vlevel;
  _vlevelIndex = index;
  _grid2d = volume.get2d(index);
  _parms = volume._parms;
  _inpSpecial = volume._special;
}

//------------------------------------------------------------------
VirtVolSweep::~VirtVolSweep(void)
{
}

//------------------------------------------------------------------
std::vector<FunctionDef> VirtVolSweep::virtVolUserUnaryOperators(void)
{
  std::vector<FunctionDef> ret;
  ret.push_back(FunctionDef(_percentLessThanStr, "P", "data, template, min",
			    "At each point output the percent of data < min within the circular template"));
  ret.push_back(FunctionDef(_largePositiveNegativeStr, "P", "data, template, minvalue",
			    "At each point output = (np-nn)/(np+nn) where np = number of points > minvalue and nn = number of points < -minvalue, within the circular template"));
  ret.push_back(FunctionDef(_smoothPolarStr, "M", "data,template",
			    "At each point output = average of data values within the circular template"));
  ret.push_back(FunctionDef(_smoothWithThreshPolarStr, "M", "data,template,minthresh,uninteresting",
			    "At each point output = average of data values within circular template. If all data within the circle at a point is < minthresh, the output is set to uninteresting"));
  ret.push_back(FunctionDef(_dilatePolarStr, "M", "data,template",
			    "At each point output = max of data values within circular template"));
  ret.push_back(FunctionDef(_medianPolarStr, "M", "data,template, binmin, binmax, bindelta",
			    "At each point output = median of data values within circular template, where median is computed using histogram bins as specified"));
  ret.push_back(FunctionDef(_percentOfAbsMaxStr, "M", "data, floorMax",
			    "let max = the maximum absolute value of all data, or floorMax if this maximum < floorMax.  At each point output = data/max, ranging from -1 to 1"));
  ret.push_back(FunctionDef(_azimuthalPolarShearStr, "M", "data, widthKm, clockwise",
			    " At each point output = difference in value at the points widthKm azimuthally away from the point.  If clockwise=1 take diff in clockwise direction, otherwise counterclockwise"));
  ret.push_back(FunctionDef(_clumpFiltStr, "M", "shapes, minv, minpctaboveminv",
			    "Build disjoint clumps then within each clump count the percent of data above minv,removing clumps where that pecent is less than minpctaboveminv.\n"
			    "For the kept clumps output data is a passthrough value"));
  ret.push_back(FunctionDef(_virtVolFuzzyStr, "M", "data, fuzzyParms",
			    "At each point, output is the result of Applying the fuzzy function specified by fuzzyParms to the data at that point"));
  ret.push_back(FunctionDef(_expandInMaskStr, "M", "data, mask",
			    "At each point if data is not missing output is set to input.  If data is missing and mask is not missing, output is set to the nearest local non-missing value."));
  return ret;
}

//------------------------------------------------------------------
const MathUserData *
VirtVolSweep::specialDataPtrConst(const std::string &name) const
{
  return _special.matchingDataPtrConst(name);
}

//------------------------------------------------------------------
MathUserData *VirtVolSweep::specialDataPtr(const std::string &name)
{
  return _special.matchingDataPtr(name);
}

//------------------------------------------------------------------
bool VirtVolSweep::isCircular(void) const
{
  Mdvx::coord_t coord = _proj.getCoord();
  return fabs(coord.ny*coord.dy >= 358);
}

//------------------------------------------------------------------
int VirtVolSweep::numData(void) const
{
  return (*_grid2d)[0].getNdata();
}

//------------------------------------------------------------------
void VirtVolSweep::finishProcessingNode(int index, VolumeData *v)
{
  VirtVolVolume *vol = (VirtVolVolume *)v;
  vol->addNewSweep(index, *this);
  v->addNew(index, (MathData *)this);
}

//------------------------------------------------------------------
bool VirtVolSweep::
synchGriddedInputsAndOutputs(const std::string &output,
			     const std::vector<std::string> &inputs,
			     bool &haveAll)
{
  // try to get inputs into the _data
  GriddedData *m=NULL;
  for (size_t i=0; i<inputs.size(); ++i)
  {
    m = _refToData(inputs[i], true);
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
    GriddedData *m = _match(inputs[i]);
    if (m != NULL)
    {
      _inps.push_back(m);
    }
  }

  haveAll = (_outputSweep != NULL && _inps.size() == inputs.size());
  return true;
}

//------------------------------------------------------------------
bool VirtVolSweep::isSynchedInput(const std::string &name) const
{
  for (size_t i=0; i<_inps.size(); ++i)
  {
    if (name == _inps[i]->getName())
    {
      return true;
    }
  }
  return false;
}

//------------------------------------------------------------------
MathLoopData *VirtVolSweep::dataPtr(const std::string &name)
{
  return (MathLoopData *)_match(name);
}

//------------------------------------------------------------------
const MathLoopData *
VirtVolSweep::dataPtrConst(const std::string &name) const
{
  return (const MathLoopData *)_matchConst(name);
}

//------------------------------------------------------------------
const MathUserData *
VirtVolSweep::userDataPtrConst(const std::string &name) const
{
  return specialDataPtrConst(name);
}

//------------------------------------------------------------------
MathUserData *VirtVolSweep::userDataPtr(const std::string &name)
{
  return specialDataPtr(name);
}

//------------------------------------------------------------------
bool VirtVolSweep::storeMathUserData(const std::string &name,
				     MathUserData *v)
{
  return _special.store(name, v);
}

//------------------------------------------------------------------
bool VirtVolSweep::processVirtVolUserLoopFunction(ProcessingNode &p)
{
  string keyword;
  if (!p.isUserUnaryOp(keyword))
  {
    return false;
  }

  if (keyword == _percentLessThanStr)
  {
    return _processPercentLessThan(*(p.unaryOpArgs()));
  }
  else if (keyword == _largePositiveNegativeStr)
  {
    return _processLargePositiveNegative(*(p.unaryOpArgs()));
  }
  else if (keyword == _smoothPolarStr)
  {
    return _processSmoothPolar(*(p.unaryOpArgs()));
  }
  else if (keyword == _smoothWithThreshPolarStr)
  {
    return _processSmoothWithThreshPolar(*(p.unaryOpArgs()));
  }
  else if (keyword == _dilatePolarStr)
  {
    return _processDilatePolar(*(p.unaryOpArgs()));
  }
  else if (keyword == _medianPolarStr)
  {
    return _processMedianPolar(*(p.unaryOpArgs()));
  }
  else if (keyword == _azimuthalPolarShearStr)
  {
    return _processAzimuthalPolarShear(*(p.unaryOpArgs()));
  }
  else if (keyword == _percentOfAbsMaxStr)
  {
    return _processPercentOfAbsMax(*(p.unaryOpArgs()));
  }
  else if (keyword == _clumpFiltStr)
  {
    return _processClumpFilt(*(p.unaryOpArgs()));
  }
  else if (keyword == _virtVolFuzzyStr)
  {
    return _processFuzzy(*(p.unaryOpArgs()));
  }
  else if (keyword == _expandInMaskStr)
  {
    return _processExpandInMask(*(p.unaryOpArgs()));
  }
  else
  {
    printf("Unknown keyword %s\n", keyword.c_str());
    return false;
   }
}

//------------------------------------------------------------------
bool VirtVolSweep::smooth(MathLoopData *out,
			  std::vector<ProcessingNode *> &args) const
{
  // expect field, nx, ny as args
  double nx, ny;
  const MathLoopData *lfield;
  if (!loadDataValueValue(args, &lfield, nx, ny))
  {
    return false;
  }

  // this does not smooth at edges
  const GriddedData *gd = (const GriddedData *)lfield;
  GridAlgs g(*gd);
  g.smoothThreaded(nx, ny, 8);

  // I think out should be same thing as _outputSweep?
  GriddedData *output = (GriddedData *)out;
  
  output->dataCopy(g);
  return true;
}

//------------------------------------------------------------------
bool VirtVolSweep::smoothDBZ(MathLoopData *out,
			     std::vector<ProcessingNode *> &args) const
{
  // expect field, nx, ny as args
  double nx, ny;
  const MathLoopData *lfield;
  if (!loadDataValueValue(args, &lfield, nx, ny))
  {
    return false;
  }

  const GriddedData *gd = (const GriddedData *)lfield;
  GridAlgs g(*gd);
  g.db2linear();
  // this does not smooth at edges
  g.smooth(nx, ny);
  g.linear2db();
  
  // I think out should be same thing as _outputSweep?
  GriddedData *output = (GriddedData *)out;
  
  output->dataCopy(g);
  return true;
}

//------------------------------------------------------------------
bool VirtVolSweep::stddev(MathLoopData *out,
			  std::vector<ProcessingNode *> &args) const
{
  // expect field, nx, ny as args
  double nx, ny;
  const MathLoopData *lfield;
  if (!loadDataValueValue(args, &lfield, nx, ny))
  {
    return false;
  }

  const GriddedData *gd = (const GriddedData *)lfield;

  GridAlgs g(*gd);
  g.sdev(nx, ny);
  //g.sdevThreaded(nx, ny, 8);
  
  _outputSweep->dataCopy(g);
  return true;
}

//------------------------------------------------------------------
bool VirtVolSweep::fuzzy(MathLoopData *out,
			 std::vector<ProcessingNode *> &args) const
{
  
  const MathLoopData *lfield;
  vector<pair<double,double> > fuzzyPairs;
  if (!loadDataAndPairs(args, &lfield, fuzzyPairs))
  {
    return false;
  }
  const GriddedData *gd = (const GriddedData *)lfield;

  // create a fuzzy function from that and apply it
  FuzzyF fuzzy(fuzzyPairs);

  GridAlgs g(*gd);
  g.fuzzyRemap(fuzzy);
  
  _outputSweep->dataCopy(g);
  return true;
}

//------------------------------------------------------------------
bool VirtVolSweep::average(MathLoopData *out,
			   std::vector<ProcessingNode *> &args) const
{
  vector<const MathLoopData *> fields;
  double nx;
  if (!loadValueAndMultiData(args, nx, fields))
  {
    return false;
  }

  vector<GridAlgs> inps;
  for (size_t i=0; i<fields.size(); ++i)
  {
    const GriddedData *gd = (const GriddedData *)fields[i];
    GridAlgs d(*gd);
    if (nx > 0)
    {
      d.adjust(nx, -1);
    }
    inps.push_back(d);
  }

  GridAlgs g(inps[0]);
  g.setAllMissing();
  GridAlgs counts(inps[0]);
  counts.setAllToValue(0.0);

  for (size_t i=0; i<inps.size(); ++i)
  {
    g.add(inps[i], counts);
  }
  g.divide(counts);
  
  _outputSweep->dataCopy(g);
  return true;
}

//------------------------------------------------------------------
bool VirtVolSweep::max(MathLoopData *out,
		       std::vector<ProcessingNode *> &args) const
{
  vector<const MathLoopData *> lfields;
  if (!loadMultiData(args, lfields))
  {
    return false;
  }

  const GriddedData *gd = (const GriddedData *)lfields[0];
  GridAlgs g(*gd);

  for (size_t i=1; i<lfields.size(); ++i)
  {
    gd = (const GriddedData *)lfields[i];
    g.max(*gd);
  }
  
  _outputSweep->dataCopy(g);
  return true;
}

//------------------------------------------------------------------
bool VirtVolSweep::mask(MathLoopData *out,
			std::vector<ProcessingNode *> &args) const
{
  const MathLoopData *ldata;
  vector<pair<double,double> > ranges;
  
  // expect 0th arg to be input field name, rest of args are pairs of ranges
  // so better be at least 3 args, and better be an odd #
  if (!loadDataAndPairs(args, &ldata, ranges))
  {
    return false;
  }

  // pull a grid2d out of the inputs
  const GriddedData *data = (const GriddedData *)ldata;
  GridAlgs g(*data);
  for (size_t i=0; i<ranges.size(); ++i)
  {
    g.maskRange(*data,  ranges[i].first, ranges[i].second);
  }
  
  _outputSweep->dataCopy(g);
  return true;
}

//-----------------------------------------------------------------------
bool
VirtVolSweep::mask_missing_to_missing(MathLoopData *out,
				      std::vector<ProcessingNode *> &args) const
{
  // expect 2 args, 1st is input data, 2nd is input mask
  const MathLoopData *ldata, *lmask;
  if (!loadDataData(args, &ldata, &lmask))
  {
    return false;
  }
  const GriddedData *data = (const GriddedData *)ldata;
  GridAlgs g(*data);
  data =  (const GriddedData *)lmask;
  g.maskMissingToMissing(*data);
  _outputSweep->dataCopy(g);
  return true;
}

//-----------------------------------------------------------------------
bool VirtVolSweep::trapezoid(MathLoopData *out,
			     std::vector<ProcessingNode *> &args) const
{
  // expect 5 args, input data, and 4 parameters a,b,c,d

  const MathLoopData *ldata;
  double a, b, c, d;

  if (!loadDataAndFourNumbers(args, &ldata, a, b, c, d))
  {
    return false;
  }

  TrapFuzzyF fz(a, b, c, d);
  const GriddedData *data = (const GriddedData *)ldata;
  Grid2d g(*data);
  for (int i=0; i<g.getNdata(); ++i)
  {
    double v;
    if (g.getValue(i, v))
    {
      v = fz.apply(v);
      g.setValue(i, v);
    }
  }
  _outputSweep->dataCopy(g);
  return true;
}

//-----------------------------------------------------------------------
bool VirtVolSweep::s_remap(MathLoopData *out,
			   std::vector<ProcessingNode *> &args) const
{
  // expect 3 args, input data, and 2 parameters a,b
  // expect field, nx, ny as args
  double a, b;
  const MathLoopData *lfield;
  if (!loadDataValueValue(args, &lfield, a, b))
  {
    return false;
  }
  const GriddedData *gd = (const GriddedData *)lfield;
  SFuzzyF fz(a, b);
  Grid2d g(*gd);
  for (int i=0; i<g.getNdata(); ++i)
  {
    double v;
    if (g.getValue(i, v))
    {
      v = fz.apply(v);
      g.setValue(i, v);
    }
  }
  _outputSweep->dataCopy(g);
  return true;
}

//------------------------------------------------------------------
bool VirtVolSweep::max_expand(MathLoopData *out,
			      std::vector<ProcessingNode *> &args) const
{
  // expect 3 args, input data, and nx, ny
  double nx, ny;
  const MathLoopData *lfield;
  if (!loadDataValueValue(args, &lfield, nx, ny))
  {
    return false;
  }
  const GriddedData *gd = (const GriddedData *)lfield;

  GridAlgs g(*gd);
  g.maxExpand(nx, ny);
  //g.sdevThreaded(nx, ny, 8);
  
  _outputSweep->dataCopy(g);
  return true;
}

//------------------------------------------------------------------
bool VirtVolSweep::
expand_angles_laterally(MathLoopData *out,
			std::vector<ProcessingNode *> &args) const
{
  // expect field, npt
  double npt;
  const MathLoopData *lfield;
  if (!loadDataValue(args, &lfield, npt))
  {
    return false;
  }
  const GriddedData *gd = (const GriddedData *)lfield;

  GridAlgs g(*gd);
  g.expandLaterally(npt);
  //g.sdevThreaded(nx, ny, 8);
  
  _outputSweep->dataCopy(g);
  return true;
}

//------------------------------------------------------------------
bool
VirtVolSweep::clump(MathLoopData *out,
		    std::vector<ProcessingNode *> &args) const
{
  // expect field, npt
  double npt;
  const MathLoopData *lfield;
  if (!loadDataValue(args, &lfield, npt))
  {
    return false;
  }
  const GriddedData *gd = (const GriddedData *)lfield;

  Grid2d g(*gd);
  g.setAllMissing();
  Grid2dClump c(*gd);
  std::vector<clump::Region_t> clumps = c.buildRegions();
  for (size_t i=0; i<clumps.size(); ++i)
  {
    clump::Region_citer_t c;
    if ((int)(clumps[i].size()) > npt)
    {
      for (c=clumps[i].begin(); c!=clumps[i].end(); ++c)
      {
	g.setValue(c->first, c->second, gd->getValue(c->first, c->second));
      }
    }
  }
  _outputSweep->dataCopy(g);
  return true;
}

//------------------------------------------------------------------
bool VirtVolSweep::median(MathLoopData *out,
			  std::vector<ProcessingNode *> &args) const
{
  // expect input data, and 5 parameters nx, ny, binMin, binMax, binDelta
  const MathLoopData *ldata;
  double nx, ny, binMin, binMax, binDelta;


  if (!loadDataAndFiveNumbers(args, &ldata, nx, ny, binMin, binMax, binDelta))
  {
    return false;
  }
  const GriddedData *gd = (const GriddedData *)ldata;
  GridAlgs g(*gd);

  // put it here
  g.median(nx, ny, binMin, binMax, binDelta);
  _outputSweep->dataCopy(g);
  return true;
}

//------------------------------------------------------------------
bool VirtVolSweep::weighted_average(MathLoopData *out,
				    std::vector<ProcessingNode *> &args) const
{

  // expect 0th arg to be number of x to skip, then pairs of field/weight
  // so need odd args, at least 3
  double nx;
  vector<const MathLoopData *> data;
  vector<double> weights;
  if (!loadNumberAndDataNumberPairs(args, nx, data, weights))
  {
    return false;
  }

  vector<Grid2d> inps;
  for (size_t i=0; i<data.size(); ++i)
  {
    const GriddedData *gd = (const GriddedData *)data[i];

    // pull a grid2d out of the inputs
    GridAlgs d(*gd);
    if (nx > 0)
    {
      d.adjust(nx, -1);
    }
    Grid2d d2d(d);
    inps.push_back(d2d);
  }

  GridAlgs g(inps[0]);
  g.weightedAverage(inps, weights);
  
  _outputSweep->dataCopy(g);
  return true;
}

//------------------------------------------------------------------
bool
VirtVolSweep::weighted_angle_average(MathLoopData *out,
				     std::vector<ProcessingNode *> &args) const
{
  // expect 0th arg to be 1 (360) or 0 (180), then pairs of field/weight
  // so need odd args, at least 3

  double which;
  vector<const MathLoopData *> data;
  vector<double> weights;
  if (!loadNumberAndDataNumberPairs(args, which, data, weights))
  {
    return false;
  }

  bool is360;
  if (which == 1)
  {
    is360 = true;
  }
  else if (which == 0)
  {
    is360 = false;
  }
  else
  {
    LOG(ERROR) << "Expect first arg to be 1 (360 average) or 0 (180 average)";
    return false;
  }
  vector<Grid2d> inps;
  for (size_t i=0; i<data.size(); i+=2)
  {
    const GriddedData *gd = (const GriddedData *)data[i];
    Grid2d d2d(*gd);
    inps.push_back(d2d);
  }

  GridAlgs g(inps[0]);
  g.weightedAngleAverage(inps, weights, is360);

  _outputSweep->dataCopy(g);
  return true;
}

//------------------------------------------------------------------
GriddedData *VirtVolSweep::_refToData(const std::string &name,
				      bool suppressWarn)
{
  // try to pull out of existing data derived
  for (size_t i=0; i<_data.size(); ++i)
  {
    if (name == _data[i].getName())
    {
      return &(_data[i]);
    }
  }
      
  // try to pull out of input data, and if so copy to output data
  for (size_t i=0; i<_grid2d->size(); ++i)
  {
    if (name == (*_grid2d)[i].getName())
    {
      _data.push_back((*_grid2d)[i]);
      size_t k = _data.size();
      return &(_data[k-1]);
    }
  }
  
  // can't pull out of state, not in input data and not in _data
  if (!suppressWarn)
  {
    printf("ERROR retrieving data for %s\n", name.c_str());
  }
  return NULL;
}    

//------------------------------------------------------------------
GriddedData *VirtVolSweep::_exampleData(const std::string &name)
{
  // see if already there
  GriddedData *s = _refToData(name, true);
  if (s == NULL)
  {
    GriddedData r((*_grid2d)[0]);
    r.setName(name);
    _data.push_back(r);
    s = _refToData(name, true);
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
GriddedData *VirtVolSweep::_match(const std::string &n) 
{
  // try to pull out of existing rays in data
  for (size_t i=0; i<_data.size(); ++i)
  {
    if (n == _data[i].getName())
    {
      return &(_data[i]);
    }
  }
  return NULL;
}      

//------------------------------------------------------------------
const GriddedData *VirtVolSweep::_matchConst(const std::string &n) const
{
  // try to pull out of existing rays in data
  for (size_t i=0; i<_data.size(); ++i)
  {
    if (n == _data[i].getName())
    {
      return &(_data[i]);
    }
  }
  return NULL;
}      

//------------------------------------------------------------------
bool VirtVolSweep::_loadGridValueValue(std::vector<ProcessingNode *> &args,
				       const GriddedData **field, double &nx,
				       double &ny) const
{
  if (args.size() != 3)
  {
    LOG(ERROR) << "Wrong number of args want 3 got " << args.size();
    return false;
  }
  string dataName = args[0]->leafName();
  if (dataName.empty())
  {
    LOG(ERROR) << " NO named data for arg0";
    return false;
  }

  if (!args[1]->getValue(nx))
  {
    LOG(ERROR) << "No value in arg1";
    return false;
  }
  if (!args[2]->getValue(ny))
  {
    LOG(ERROR) << "No value in arg2";
    return false;
  }

  // pull a grid2d out of the inputs
  *field = _matchConst(dataName);
  if (*field == NULL)
  {
    LOG(ERROR) << "No data to go with " << dataName;
    return false;
  }
  return true;
}

//------------------------------------------------------------------
bool VirtVolSweep::
_loadGridandPairs(std::vector<ProcessingNode *> &args,
		  const GriddedData **field,
		  std::vector<std::pair<double,double> > &pairs) const
{
  // expect an odd # of args  field x,y,x,y,..x,y
  int n = (int)args.size();
  if (n%2 == 0)
  {
    LOG(ERROR) << "Expect odd number of args";
    return false;
  }
  string dataName = args[0]->leafName();
  if (dataName.empty())
  {
    LOG(ERROR) << " NO named first arg";
    return false;
  }
  // pull a grid2d out of the inputs
  *field = _matchConst(dataName);
  if (*field == NULL)
  {
    LOG(ERROR) << "No data to go with " << dataName;
    return false;
  }

  pairs.clear();
  for (int i=1; i<n; i+=2)
  {
    double x, y;
    if (!args[i]->getValue(x))
    {
      LOG(ERROR) << "No value in arg position" << i;
      return false;
    }
    if (!args[i+1]->getValue(y))
    {
      LOG(ERROR) << "No value in arg position << i+1";
      return false;
    }
    pairs.push_back(pair<double,double>(x,y));
  }
  return true;
}

//------------------------------------------------------------------
bool VirtVolSweep::
_loadValueAndMultiFields(std::vector<ProcessingNode *> &args,
			 double &value,
			 std::vector<const GriddedData *> &fields) const
{
  // expect 0th arg to be a number, rest of args are
  // fields, so better be at least 3 args
  if (args.size() < 3)
  {
    LOG(ERROR) << "Expect at least 3 args";
    return false;
  }

  if (!args[0]->getValue(value))
  {
    LOG(ERROR) << "No value in position 0";
    return false;
  }

  fields.clear();
  for (size_t i=1; i<args.size(); ++i)
  {
    string dataName = args[i]->leafName();
    if (dataName.empty())
    {
      LOG(ERROR) << " NO name in position " << i;
      return false;
    }
    const GriddedData *field = _matchConst(dataName);
    if (field == NULL)
    {
      LOG(ERROR) << "No data for " << dataName;
      return false;
    }
    fields.push_back(field);
  }
  return true;
}

//------------------------------------------------------------------
bool VirtVolSweep::
_loadMultiFields(std::vector<ProcessingNode *> &args,
		 std::vector<const GriddedData *> &fields) const
{
  if (args.size() < 2)
  {
    LOG(ERROR) << "Expect at least 2 args";
    return false;
  }

  fields.clear();
  for (size_t i=0; i<args.size(); ++i)
  {
    string dataName = args[i]->leafName();
    if (dataName.empty())
    {
      LOG(ERROR) << " NO name in position " << i;
      return false;
    }
    const GriddedData *field = _matchConst(dataName);
    if (field == NULL)
    {
      LOG(ERROR) << "No data for " << dataName;
      return false;
    }
    fields.push_back(field);
  }
  return true;
}

//------------------------------------------------------------------
bool VirtVolSweep::_processSmoothPolar(std::vector<ProcessingNode *> &args)
{
  const MathLoopData *data;
  MathUserData *udata;

  if (!loadDataAndUserData(args, &data, &udata))
  {
    LOG(ERROR) << "Wrong interface";
    return false;
  }

  const PolarCircularTemplate *pt = (const PolarCircularTemplate *)udata;


  const GriddedData *input = (const GriddedData *)data;
  Grid2d g(*input);
  g.changeMissingAndData(-99);
  //g.setAllMissing();

  PolarCircularFilter::smooth(g, *pt);
  _outputSweep->dataCopy(g);
  return true;
}

//------------------------------------------------------------------
bool VirtVolSweep::_processSmoothWithThreshPolar(std::vector<ProcessingNode *> &args)
{
  const MathLoopData *data;
  MathUserData *udata;

  if (args.size() != 4)
  {
    LOG(ERROR) << "Need 4 inputs";
    return false;
  }
  string dataName = args[0]->leafName();
  if (dataName.empty())
  {
    LOG(ERROR) << " NO name in arg 0";
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
    LOG(ERROR) << " NO name in arg 1";
    return false;
  }
  udata = userDataPtr(dataName);
  if (udata == NULL)
  {
    LOG(ERROR) << "No data for" << dataName;
    return false;
  }

  double thresh, uninteresting;

  if (!args[2]->getValue(thresh))
  {
    LOG(ERROR) << "No value in arg position 2";
    return false;
  }
  
  if (!args[3]->getValue(uninteresting))
  {
    LOG(ERROR) << "No value in arg position 3";
    return false;
  }

  const PolarCircularTemplate *pt = (const PolarCircularTemplate *)udata;
  const GriddedData *input = (const GriddedData *)data;
  Grid2d g(*input);
  //g.changeMissingAndData(-99);
  PolarCircularFilter::smoothWithThresh(g, *pt, thresh, uninteresting);
  _outputSweep->dataCopy(g);
  return true;
}

//------------------------------------------------------------------
bool VirtVolSweep::_processDilatePolar(std::vector<ProcessingNode *> &args)
{
  const MathLoopData *data;
  MathUserData *udata;

  if (!loadDataAndUserData(args, &data, &udata))
  {
    LOG(ERROR) << "Wrong interface";
    return false;
  }

  const PolarCircularTemplate *pt = (const PolarCircularTemplate *)udata;


  const GriddedData *input = (const GriddedData *)data;
  Grid2d g(*input);
  g.changeMissingAndData(-99);
  //g.setAllMissing();


  PolarCircularFilter::dilate(g, *pt);
  _outputSweep->dataCopy(g);
  return true;
}

//------------------------------------------------------------------
bool VirtVolSweep::_processMedianPolar(std::vector<ProcessingNode *> &args)
{
  const MathLoopData *data;
  MathUserData *udata;
  double binMin, binMax, binDelta;
  
  if (args.size() != 5)
  {
    LOG(ERROR) << "Need 5 inputs";
    return false;
  }
  string dataName = args[0]->leafName();
  if (dataName.empty())
  {
    LOG(ERROR) << " NO name in arg 0";
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
    LOG(ERROR) << " NO name in arg 1";
    return false;
  }
  udata = userDataPtr(dataName);
  if (udata == NULL)
  {
    LOG(ERROR) << "No data for" << dataName;
    return false;
  }
  if (!args[2]->getValue(binMin))
  {
    LOG(ERROR) << "No value in arg position 2";
    return false;
  }
  if (!args[3]->getValue(binMax))
  {
    LOG(ERROR) << "No value in arg position 3";
    return false;
  }
  if (!args[4]->getValue(binDelta))
  {
    LOG(ERROR) << "No value in arg position 4";
    return false;
  }

  const PolarCircularTemplate *pt = (const PolarCircularTemplate *)udata;


  const GriddedData *input = (const GriddedData *)data;
  Grid2d g(*input);
  g.changeMissingAndData(-99);


  PolarCircularFilter::median(g, *pt, binMin, binMax, binDelta);
  _outputSweep->dataCopy(g);
  return true;
}

//------------------------------------------------------------------
bool VirtVolSweep::_processPercentLessThan(std::vector<ProcessingNode *> &args)
{
  const MathLoopData *data;
  MathUserData *udata;
  double min;

  if (!loadDataAndUserDataAndValue(args, &data, &udata, min))
  {
    LOG(ERROR) << "Wrong interface";
    return false;
  }

  const PolarCircularTemplate *pt = (const PolarCircularTemplate *)udata;
  const GriddedData *input = (const GriddedData *)data;
  Grid2d g(*input);
  g.changeMissingAndData(-99);
  //g.setAllMissing();

  PolarCircularFilter::percentLessThan(g, *pt, min);
  _outputSweep->dataCopy(g);
  return true;
    
}

//------------------------------------------------------------------
bool VirtVolSweep::_processLargePositiveNegative(std::vector<ProcessingNode *> &args)
{
  const MathLoopData *data;
  MathUserData *udata;
  double minValue;

  if (!loadDataAndUserDataAndValue(args, &data, &udata, minValue))
  {
    LOG(ERROR) << "Wrong interface";
    return false;
  }

  const PolarCircularTemplate *pt = (const PolarCircularTemplate *)udata;

  const GriddedData *input = (const GriddedData *)data;
  Grid2d g(*input);
  g.changeMissingAndData(-99);

  PolarCircularFilter::largePosNeg(g, *pt, minValue);
  _outputSweep->dataCopy(g);
  return true;
    
}

//--------------------------------------------------------------------
bool VirtVolSweep::_processPercentOfAbsMax(std::vector<ProcessingNode *> &args)
{
  const MathLoopData *data;
  double floorMax;
  
  int n = (int)args.size();
  if (n != 2)
  {
    LOG(ERROR) << "Need 2 args";
    return false;
  }

  if (!loadDataValue(args, &data, floorMax))
  {
    LOG(ERROR) << "Wrong interface";
    return false;
  }
  
  const GriddedData *input = (const GriddedData *)data;
  // loop through data to get max abs value, which we might use as nyquist
  double max;
  bool first;
  
  for (int i=0; i<input->getNdata(); ++i)
  {
    double v;
    if (input->getValue(i, v))
    {
      double fv = fabs(v);
      if (first)
      {
	first = false;
	max = fv;
      }
      else
      {
	if (fv > max)
	{
	  max = fv;
	}
      }
    }
  }
  if (first)
  {
    max = floorMax;
  }
  else
  {
    if (max < floorMax)
    {
      max = floorMax;
    }
  }

  Grid2d g(*input);
  g.changeMissing(-99);
  g.setAllMissing();

  for (int i=0; i<input->getNdata(); ++i)
  {
    double v;
    if (input->getValue(i, v))
    {
      // should go from -1 to 1
      g.setValue(i, v/max);
    }
  }
  _outputSweep->dataCopy(g);
  return true;
}

//--------------------------------------------------------------------
bool VirtVolSweep::_processAzimuthalPolarShear(std::vector<ProcessingNode *> &args)
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

//--------------------------------------------------------------------
bool VirtVolSweep::_processClumpFilt(std::vector<ProcessingNode *> &args)
{
  const MathLoopData *data;
  double minv, minpct;

  if (!loadDataValueValue(args, &data, minv, minpct))
  {
    LOG(ERROR) << "bad interface";
    return false;
  }
  const GriddedData *input = (const GriddedData *)data;
  Grid2d g(*input);
  g.changeMissing(-99);
  g.setAllMissing();

  Grid2dClump c(*input);
  std::vector<clump::Region_t> clumps = c.buildRegions();
  for (size_t i=0; i<clumps.size(); ++i)
  {
    clump::Region_citer_t c;
    double count = 0;
    double good = 0;
    for (c=clumps[i].begin(); c!=clumps[i].end(); ++c)
    {
      double v;
      if (input->getValue(c->first, c->second, v))
      {
	count ++;
	if (v >= minv)
	{
	  ++good;
	}
      }
    }
    if (count == 0)
    {
      // don't even save this clump
    }
    else
    {
      double pct = good/count;
      if (pct >= minpct)
      {
	// keep this clump
	for (c=clumps[i].begin(); c!=clumps[i].end(); ++c)
	{
	  double v;
	  if (input->getValue(c->first, c->second, v))
	  {
	    g.setValue(c->first, c->second, v);
	  }
	}
      }
    }	  
  }
  
  _outputSweep->dataCopy(g);
  return true;
}

//------------------------------------------------------------------
bool VirtVolSweep::_processFuzzy(std::vector<ProcessingNode *> &args)
{
  const MathLoopData *data;
  MathUserData *udata;

  if (!loadDataAndUserData(args, &data, &udata))
  {
    LOG(ERROR) << "Wrong interface";
    return false;
  }

  const VirtVolFuzzy *f = (const VirtVolFuzzy *)udata;
  const GriddedData *input = (const GriddedData *)data;
  Grid2d g(*input);
  g.changeMissingAndData(-99);
  for (int i=0; i<g.getNdata(); ++i)
  {
    double v;
    if (input->getValue(i, v))
    {
      g.setValue(i, f->apply(v));
    }
  }
  _outputSweep->dataCopy(g);
  return true;
    
}


//-----------------------------------------------------------------------
bool
VirtVolSweep::_processExpandInMask(std::vector<ProcessingNode *> &args) 
{
  // expect 2 args, 1st is input data, 2nd is input mask
  const MathLoopData *ldata, *lmask;
  if (!loadDataData(args, &ldata, &lmask))
  {
    return false;
  }
  const GriddedData *data = (const GriddedData *)ldata;
  GridAlgs g(*data);
  data =  (const GriddedData *)lmask;
  g.expandInMask(*data);
  _outputSweep->dataCopy(g);
  return true;
}



