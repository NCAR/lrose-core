/**
 * @file SweepMdv.cc
 */

//------------------------------------------------------------------
#include <FiltAlgVirtVol/SweepMdv.hh>
#include <FiltAlgVirtVol/VolumeMdv.hh>
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

//------------------------------------------------------------------
SweepMdv::SweepMdv(void) : _grid2d(NULL)
{
}

//------------------------------------------------------------------
SweepMdv::SweepMdv(const VolumeMdv &volume, int index, double vlevel)
{
  _time = volume._time;
  _proj = volume._proj;
  _clockwise = volume.clockwise();
  _vlevel = vlevel;
  _vlevelIndex = index;
  _grid2d = volume.get2d(index);
  _parms = volume._parms;
  _inpSpecial = volume._special;
}

//------------------------------------------------------------------
SweepMdv::~SweepMdv(void)
{
}

//------------------------------------------------------------------
const MathUserData *SweepMdv::specialDataPtrConst(const std::string &name) const
{
  return _special.matchingDataPtrConst(name);
}

//------------------------------------------------------------------
MathUserData *SweepMdv::specialDataPtr(const std::string &name)
{
  return _special.matchingDataPtr(name);
}

//------------------------------------------------------------------
bool SweepMdv::isCircular(void) const
{
  Mdvx::coord_t coord = _proj.getCoord();
  return fabs(coord.ny*coord.dy >= 358);
}

//------------------------------------------------------------------
int SweepMdv::numData(void) const
{
  return (*_grid2d)[0].getNdata();
}

//------------------------------------------------------------------
void SweepMdv::finishProcessingNode(int index, VolumeData *v)
{
  VolumeMdv *vol = (VolumeMdv *)v;
  vol->addNewMdv(index, *this);
  v->addNew(index, (MathData *)this);
}

//------------------------------------------------------------------
bool
SweepMdv::synchGriddedInputsAndOutputs(const std::string &output,
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
bool SweepMdv::isSynchedInput(const std::string &name) const
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
MathLoopData *SweepMdv::dataPtr(const std::string &name)
{
  return (MathLoopData *)_match(name);
}

//------------------------------------------------------------------
const MathLoopData *SweepMdv::dataPtrConst(const std::string &name) const
{
  return (const MathLoopData *)_matchConst(name);
}

//------------------------------------------------------------------
const MathUserData *SweepMdv::userDataPtrConst(const std::string &name) const
{
  return specialDataPtrConst(name);
}

//------------------------------------------------------------------
MathUserData *SweepMdv::userDataPtr(const std::string &name)
{
  return specialDataPtr(name);
}

//------------------------------------------------------------------
bool SweepMdv::storeMathUserData(const std::string &name, MathUserData *v)
{
  return _special.store(name, v);
}

//------------------------------------------------------------------
bool SweepMdv::smooth(MathLoopData *out,
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
bool SweepMdv::smoothDBZ(MathLoopData *out,
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
bool SweepMdv::stddev(MathLoopData *out,
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
bool SweepMdv::fuzzy(MathLoopData *out,
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
bool SweepMdv::average(MathLoopData *out,
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
bool SweepMdv::max(MathLoopData *out,
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
bool SweepMdv::mask(MathLoopData *out,
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
SweepMdv::mask_missing_to_missing(MathLoopData *out,
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
bool SweepMdv::trapezoid(MathLoopData *out,
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
bool SweepMdv::s_remap(MathLoopData *out,
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
bool SweepMdv::max_expand(MathLoopData *out,
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
bool
SweepMdv::expand_angles_laterally(MathLoopData *out,
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
SweepMdv::clump(MathLoopData *out,
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
    for (c=clumps[i].begin(); c!=clumps[i].end(); ++c)
    {
      g.setValue(c->first, c->second, gd->getValue(c->first, c->second));
    }
  }
  _outputSweep->dataCopy(g);
  return true;
}

//------------------------------------------------------------------
bool SweepMdv::median(MathLoopData *out,
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
bool SweepMdv::weighted_average(MathLoopData *out,
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
bool SweepMdv::weighted_angle_average(MathLoopData *out,
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
GriddedData *SweepMdv::_refToData(const std::string &name,  bool suppressWarn)
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
GriddedData *SweepMdv::_exampleData(const std::string &name)
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
GriddedData *SweepMdv::_match(const std::string &n) 
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
const GriddedData *SweepMdv::_matchConst(const std::string &n) const
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


bool SweepMdv::_loadGridValueValue(std::vector<ProcessingNode *> &args,
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

bool SweepMdv::
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

bool SweepMdv::
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

bool SweepMdv::
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
