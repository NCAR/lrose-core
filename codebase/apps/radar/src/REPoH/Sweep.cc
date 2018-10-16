/**
 * @file Sweep.cc
 */

//------------------------------------------------------------------
#include "Sweep.hh"
#include "AsciiOutput.hh"
#include "KernelGrids.hh"
#include "Kernels.hh"
#include "Volume.hh"
#include "ClumpAssociate.hh"
#include "ClumpRegions.hh"
#include "CloudGaps.hh"
#include "FIRFilter.hh"
#include <euclid/GridAlgs.hh>
#include <euclid/Grid2dClump.hh>
#include <rapmath/MathParser.hh>
#include <rapmath/ProcessingNode.hh>
#include <rapmath/TrapFuzzyF.hh>
#include <rapmath/SFuzzyF.hh>
#include <rapmath/FuzzyF.hh>
#include <rapmath/UnaryNode.hh>
#include <toolsa/LogStream.hh>
#include <cmath>
#include <algorithm>

//------------------------------------------------------------------
Sweep::Sweep(const Volume &volume, int index, double vlevel) :
  _mdvInfo(volume._mdvInfo),
  _kernelOutputs(volume._kernelOutputs),
  _asciiOutputs(volume._asciiOutputs),
  _time(volume._time),
  _repohParms(volume._repohParms),
  _vlevel(vlevel),
  _vlevelIndex(index),
  _inputGrids(volume._data.get2d(index)),
  _derivedData(index)
{
}

//------------------------------------------------------------------
Sweep::~Sweep(void)
{
}

//------------------------------------------------------------------
int Sweep::numData(void) const
{
  return _mdvInfo.numData();
}

//------------------------------------------------------------------
void Sweep::finishProcessingNode(int index, VolumeData *v)
{
  Volume *vol = (Volume *)v;
  vol->addNew(index, *this);
}

//------------------------------------------------------------------
bool Sweep::synchInputsAndOutputs(const std::string &output,
				  const std::vector<std::string> &inputs)
{
  bool ret = true;

  // look for input data and return false if one or more missing
  // this step can copy from _inputData to _data, if needed at this step
  if (!_stageInputs(inputs))
  {
    ret = false;
  }

  // set pointer to output which is one of several data types
  if (!_stageOutput(output))
  {
    ret = false;
  }
  return ret;
}

//------------------------------------------------------------------
MathLoopData *Sweep::dataPtr(const std::string &name)
{
  return (MathLoopData *)_derivedData.refToData(name);
}

//------------------------------------------------------------------
const MathLoopData *Sweep::dataPtrConst(const std::string &name) const
{
  return (const MathLoopData *)_derivedData.refToData(name);
}

//------------------------------------------------------------------
bool Sweep::processUserLoopFunction(ProcessingNode &p)
{
  string keyword;
  if (!p.isUserUnaryOp(keyword))
  {
    return false;
  }

  // if (keyword == "TextureX")
  // {
  //   return _processTextureX(*(p.unaryOpArgs()));
  // }
  // else if (keyword == "stddev_no_overlap")
  // {
  //   return _processStdDevNoOverlap(*(p.unaryOpArgs()));
  // }
  // else if (keyword == "median_no_overlap")
  // {
  //   return _processMedianNoOverlap(*(p.unaryOpArgs()));
  // }
  // else if (keyword == "snr_from_dbz")
  // {
  //   return _processSnrFromDbz(*(p.unaryOpArgs()));
  // }
  if (keyword == "clumps_to_grid")
  {
    return _clumpsToGrid(*(p.unaryOpArgs()));
  }
  else if (keyword == "remove_small_clumps")
  {
    return _processRemoveSmallClumps(*(p.unaryOpArgs()));
  }
  else if (keyword == "get_edge")
  {
    return _getEdge(*(p.unaryOpArgs()));
  }
  else if (keyword == "get_outside")
  {
    return _getOutside(*(p.unaryOpArgs()));
  }
  else if (keyword == "inverse_mask")
  {
    return _inverseMask(*(p.unaryOpArgs()));
  }
  else if (keyword == "centerpoints")
  {
    return _centerPoints(*(p.unaryOpArgs()));
  }
  else if (keyword == "compute_attenuation")
  {
    return _attenuation(*(p.unaryOpArgs()));
  }
  else if (keyword == "npt_between_good")
  {
    return _nptBetweenGood(*(p.unaryOpArgs()));
  }
  else if (keyword == "total_attenuation")
  {
    return _totalAttenuation(*(p.unaryOpArgs()));
  }
  else if (keyword == "average_attenuation")
  {
    return _averageAttenuation(*(p.unaryOpArgs()));
  }
  else if (keyword == "sumZ")
  {
    return _sumZ(*(p.unaryOpArgs()));
  }
  else if (keyword == "compute_humidity")
  {
    return _humidity(*(p.unaryOpArgs()));
  }
  else if (keyword == "FIR")
  {
    return _fir(*(p.unaryOpArgs()));
  }
  else
  {
    printf("Unknown keyword %s\n", keyword.c_str());
    return false;
  }
}

//------------------------------------------------------------------
MathUserData *Sweep::processUserLoop2dFunction(const UnaryNode &p)
{
  string keyword;
  if (!p.getUserUnaryKeyword(keyword))
  {
    return NULL;
  }
  vector<string> args = p.getUnaryNodeArgStrings();
  if (keyword == "create_clumps")
  {
    if (args.size() != 1)
    {
      LOG(ERROR) << "Wrong, want 1 arg got " << args.size();
      return NULL;
    }
    return _createClumps(args[0]);
  }
  else if (keyword == "associate_clumps")
  {
    if (args.size() != 3)
    {
      LOG(ERROR) << "Wrong, want 3 args got " << args.size();
      return NULL;
    }
    return _processAssociateClumps(args[0], args[1], args[2]);
  }
  else if (keyword == "organize_grids")
  {
    if (args.size() != 9)
    {
      LOG(ERROR) << "Wrong, want 9 args got " << args.size();
      return NULL;
    }
    return _organize(args[0], args[1], args[2], args[3], args[4], args[5],
		     args[6], args[7], args[8]);
  }
  else if (keyword == "build_gaps")
  {
    if (args.size() != 2)
    {
      LOG(ERROR) << "Wrong, want 2 args got " << args.size();
      return NULL;
    }
    return _processBuildGaps(args[0], args[1]);
  }
  else if (keyword == "remove_small_gaps")
  {
    if (args.size() != 2)
    {
      LOG(ERROR) << "Wrong, want 2 args got " << args.size();
      return NULL;
    }
    return _removeSmallGaps(args[0], args[1]);
  }
  else if (keyword == "filter_so_not_too_far_inside")
  {
    if (args.size() != 4)
    {
      LOG(ERROR) << "Wrong, want 4 args got " << args.size();
      return NULL;
    }
    return _filterGapsInside(args[0], args[1],args[2], args[3]);
  }
  else if (keyword == "kernel_build")
  {
    if (args.size() != 4)
    {
      LOG(ERROR) << "Wrong want 4 args got " << args.size();
      return NULL;
    }
    return _kernelBuild(args[0], args[1], args[2], args[3]);
  }
  else if (keyword == "filter_kernels")
  {
    if (args.size() != 1)
    {
      LOG(ERROR) << "Wrong want 1 args got " << args.size();
      return NULL;
    }
    return _kernelFilter(args[0]);
  }
  else if (keyword == "kernels_to_genpoly")
  {
    if (args.size() != 1)
    {
      LOG(ERROR) << "Wrong want 1 args got " << args.size();
      return NULL;
    }
    return _kernelToGenPoly(args[0]);
  }
  else if (keyword == "set_ascii_output")
  {
    if (args.size() != 1)
    {
      LOG(ERROR) << "Wrong want 1 args got " << args.size();
      return NULL;
    }
    return _processAsciiOutput(args[0]);
  }
  else
  {
    LOG(ERROR) << "Keyword not known " << keyword;
    return NULL;
  }
}

//------------------------------------------------------------------
bool Sweep::synchUserDefinedInputs(const std::string &userKey,
				   const std::vector<std::string> &names)
{
  _outputSweep = NULL;
  if (!_needToSynch(userKey))
  {
    return true;
  }

  if (userKey == "clumps_to_grid")
  {
    // the one input is special data, see if we have it
    if (names.size() != 1)
    {
      LOG(ERROR) << "Inputs messed up";
      return false;
    }
    if (!_special.hasName(names[0]))
    {
      LOG(ERROR) << "Input not there " << names[0];
      return false;
    }
  }
  else if (userKey == "remove_small_clumps")
  {
    // expect 3 args, 1st arg is a grid, 2nd is a user data
    if (names.size() != 3)
    {
      LOG(ERROR) << "Inputs messed up";
      return false;
    }

    GriddedData *m = _refToData(names[0], false);
    if (m == NULL)
    {
      LOG(ERROR) << "Cannot synch";
      return false;
    }
    if (!_special.hasName(names[1]))
    {
      LOG(ERROR) << "Input not there " << names[1];
      return false;
    }
  }
  else if (userKey == "associate_clumps")
  {
    // expect 3 args, 1st arg is a ClumpRegions (user data), 2nd & 3rd are grids
    if (names.size() != 3)
    {
      LOG(ERROR) << "Inputs messed up";
      return false;
    }

    GriddedData *m = _refToData(names[1], false);
    if (m == NULL)
    {
      LOG(ERROR) << "Cannot synch";
      return false;
    }
    m = _refToData(names[2], false);
    if (m == NULL)
    {
      LOG(ERROR) << "Cannot synch";
      return false;
    }
    if (!_special.hasName(names[0]))
    {
      LOG(ERROR) << "Input not there " << names[1];
      return false;
    }
  }
  else if (userKey == "remove_small_gaps")
  {
    // expect 2 args:  gaps, number
    if (names.size() != 2)
    {
      LOG(ERROR) << "Inputs messed up";
      return false;
    }

    if (!_special.hasName(names[0]))
    {
      LOG(ERROR) << "Input not there " << names[1];
      return false;
    }
  }
  else if (userKey == "get_edge")
  {
    // expect 1 arg:  gaps
    if (names.size() != 1)
    {
      LOG(ERROR) << "Inputs messed up";
      return false;
    }
    if (!_special.hasName(names[0]))
    {
      LOG(ERROR) << "Input not there " << names[1];
      return false;
    }
  }
  else if (userKey == "get_outside")
  {
    // expect 1 arg:  gaps
    if (names.size() != 1)
    {
      LOG(ERROR) << "Inputs messed up";
      return false;
    }
    if (!_special.hasName(names[0]))
    {
      LOG(ERROR) << "Input not there " << names[1];
      return false;
    }
  }
  else if (userKey == "filter_so_not_too_far_inside")
  {
    // expect 4 args:  gaps, clumps, associated clumps, number
    if (names.size() != 4)
    {
      LOG(ERROR) << "Inputs messed up";
      return false;
    }

    GriddedData *m = _refToData(names[1], false);
    if (m == NULL)
    {
      LOG(ERROR) << "Cannot synch";
      return false;
    }
    if (!_special.hasName(names[0]))
    {
      LOG(ERROR) << "Input not there " << names[1];
      return false;
    }
    if (!_special.hasName(names[2]))
    {
      LOG(ERROR) << "Input not there " << names[1];
      return false;
    }
  }
  else if (userKey == "organize_grids")
  {
    // all the inputs (9) should be grids that are available
    if (names.size() != 9)
    {
      LOG(ERROR) << "Inputs wrong";
      return false;
    }    
    for (size_t i=0; i<9; ++i)
    {
      GriddedData *m = _refToData(names[i], false);
      if (m == NULL)
      {
	LOG(ERROR) << "Cannot synch";
	return false;
      }
    }
  }
  else if (userKey == "set_ascii_output")
  {
    // expect 1 arg
    if (names.size() != 1)
    {
      LOG(ERROR) << "Inputs messed up";
      return false;
    }

    if (!_special.hasName(names[0]))
    {
      LOG(ERROR) << "Input not there " << names[1];
      return false;
    }
  }
  else if (userKey == "remove_small_clumps")
  {
    // expect 3 args, 1st arg is a grid, 2nd is a user data
    if (names.size() != 3)
    {
      LOG(ERROR) << "Inputs messed up";
      return false;
    }

    GriddedData *m = _refToData(names[0], false);
    if (m == NULL)
    {
      LOG(ERROR) << "Cannot synch";
      return false;
    }
    if (!_special.hasName(names[1]))
    {
      LOG(ERROR) << "Input not there " << names[1];
      return false;
    }
  }

  else if (userKey == "km_between_good")
  {
    // expect 2 args, both grids
    if (names.size() != 2)
    {
      LOG(ERROR) << "Inputs messed up";
      return false;
    }
    GriddedData *m = _refToData(names[0], false);
    if (m == NULL)
    {
      LOG(ERROR) << "Cannot synch";
      return false;
    }
    m = _refToData(names[1], false);
    if (m == NULL)
    {
      LOG(ERROR) << "Cannot synch";
      return false;
    }
  }
  else if (userKey == "total_attenuation")
  {
    // expect 2 args, both grids
    if (names.size() != 2)
    {
      LOG(ERROR) << "Inputs messed up";
      return false;
    }
    GriddedData *m = _refToData(names[0], false);
    if (m == NULL)
    {
      LOG(ERROR) << "Cannot synch";
      return false;
    }
    m = _refToData(names[1], false);
    if (m == NULL)
    {
      LOG(ERROR) << "Cannot synch";
      return false;
    }
  }
  else if (userKey == "average_attenuation")
  {
    // expect 2 args, both grids
    if (names.size() != 2)
    {
      LOG(ERROR) << "Inputs messed up";
      return false;
    }
    GriddedData *m = _refToData(names[0], false);
    if (m == NULL)
    {
      LOG(ERROR) << "Cannot synch";
      return false;
    }
    m = _refToData(names[1], false);
    if (m == NULL)
    {
      LOG(ERROR) << "Cannot synch";
      return false;
    }
  }
  else if (userKey == "sumZ")
  {
    // expect 3 args, first 2 grids, last a number
    if (names.size() != 3)
    {
      LOG(ERROR) << "Inputs messed up";
      return false;
    }
    GriddedData *m = _refToData(names[0], false);
    if (m == NULL)
    {
      LOG(ERROR) << "Cannot synch";
      return false;
    }
    m = _refToData(names[1], false);
    if (m == NULL)
    {
      LOG(ERROR) << "Cannot synch";
      return false;
    }
    if (!MathParser::isFloat(names[2]))
    {
      LOG(ERROR) << "Not a number";
      return false;
    }
  }
  else
  {
    // assume all grids and numerical input
    for (size_t i=0; i<names.size(); ++i)
    {
      GriddedData *m = _refToData(names[i], false);
      if (m == NULL)
      {
	LOG(ERROR) << "Cannot synch";
	return false;
      }
    }
  }

  // passed all tests
  return true;
}

//------------------------------------------------------------------
bool Sweep::storeMathUserData(const std::string &name, MathUserData *v)
{
  return _special.store(name, v);
}

//------------------------------------------------------------------
bool Sweep::smooth(MathLoopData *out,
		   std::vector<ProcessingNode *> &args) const
{
  // expect field, nx, ny as args
  if (args.size() != 3)
  {
    LOG(ERROR) << "Bad interface";
    return false;
  }
  string dataName = args[0]->leafName();
  if (dataName.empty())
  {
    LOG(ERROR) << " NO name";
    return false;
  }
  double nx, ny;
  if (!args[1]->getValue(nx))
  {
    LOG(ERROR) << "No value";
    return false;
  }
  if (!args[2]->getValue(ny))
  {
    LOG(ERROR) << "No value";
    return false;
  }

  // pull a grid2d out of the inputs
  const GriddedData *input = _derivedData.refToData(dataName);

  // this does not smooth at edges
  GridAlgs g(*input);
  g.smoothThreaded(nx, ny, 8);

  // I think out should be same thing as _outputSweep?
  GriddedData *output = (GriddedData *)out;
  
  output->dataCopy(g);
  return true;
}

//------------------------------------------------------------------
bool Sweep::smoothDBZ(MathLoopData *out,
		      std::vector<ProcessingNode *> &args) const
{
  // expect field, nx, ny as args
  if (args.size() != 3)
  {
    LOG(ERROR) << "Bad interface";
    return false;
  }
  string dataName = args[0]->leafName();
  if (dataName.empty())
  {
    LOG(ERROR) << " NO name";
    return false;
  }
  double nx, ny;
  if (!args[1]->getValue(nx))
  {
    LOG(ERROR) << "No value";
    return false;
  }
  if (!args[2]->getValue(ny))
  {
    LOG(ERROR) << "No value";
    return false;
  }

  // pull a grid2d out of the inputs
  const GriddedData *input = _derivedData.refToData(dataName);

  GridAlgs g(*input);
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
bool Sweep::stddev(MathLoopData *out,
		   std::vector<ProcessingNode *> &args) const
{
  // expect field, nx, ny as args
  if (args.size() != 3)
  {
    LOG(ERROR) << "Bad interface";
    return false;
  }
  string dataName = args[0]->leafName();
  if (dataName.empty())
  {
    LOG(ERROR) << " NO name";
    return false;
  }
  double nx, ny;
  if (!args[1]->getValue(nx))
  {
    LOG(ERROR) << "No value";
    return false;
  }
  if (!args[2]->getValue(ny))
  {
    LOG(ERROR) << "No value";
    return false;
  }

  // pull a grid2d out of the inputs
  const GriddedData *input = _derivedData.refToData(dataName);

  GridAlgs g(*input);
  g.sdev(nx, ny);
  //g.sdevThreaded(nx, ny, 8);
  
  _outputSweep->dataCopy(g);
  return true;
}

//------------------------------------------------------------------
bool Sweep::fuzzy(MathLoopData *out,
		  std::vector<ProcessingNode *> &args) const
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
    LOG(ERROR) << " NO name";
    return false;
  }

  vector<pair<double,double> > xy;
  for (int i=1; i<n; i+=2)
  {
    double x, y;
    if (!args[i]->getValue(x))
    {
      LOG(ERROR) << "No value";
      return false;
    }
    if (!args[i+1]->getValue(y))
    {
      LOG(ERROR) << "No value";
      return false;
    }
    xy.push_back(pair<double,double>(x,y));
  }

  // create a fuzzy function from that and apply it
  FuzzyF fuzzy(xy);
  

  // pull a grid2d out of the inputs
  const GriddedData *input = _derivedData.refToData(dataName);

  GridAlgs g(*input);
  g.fuzzyRemap(fuzzy);
  
  _outputSweep->dataCopy(g);
  return true;
}

//------------------------------------------------------------------
bool Sweep::average(MathLoopData *out,
		    std::vector<ProcessingNode *> &args) const
{
  // expect 0th arg to be number of x to skip, rest of args are
  // fields, so better be at least 3 args
  if (args.size() < 3)
  {
    LOG(ERROR) << "Expect at least 3 args";
    return false;
  }

  double nx;
  if (!args[0]->getValue(nx))
  {
    LOG(ERROR) << "No value";
    return false;
  }

  vector<GridAlgs> inps;
  for (size_t i=1; i<args.size(); ++i)
  {
    string dataName = args[i]->leafName();
    if (dataName.empty())
    {
      LOG(ERROR) << " NO name";
      return false;
    }
    // pull a grid2d out of the inputs
    GridAlgs d(*(_derivedData.refToData(dataName)));
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
bool Sweep::max(MathLoopData *out,
		std::vector<ProcessingNode *> &args) const
{
  // expect args are fields, so better have at least 2
  // fields, so better be at least 3 args
  if (args.size() < 2)
  {
    LOG(ERROR) << "Expect at least 2 args";
    return false;
  }

  vector<GridAlgs> inps;
  for (size_t i=0; i<args.size(); ++i)
  {
    string dataName = args[i]->leafName();
    if (dataName.empty())
    {
      LOG(ERROR) << " NO name";
      return false;
    }
    // pull a grid2d out of the inputs
    GridAlgs d(*(_derivedData.refToData(dataName)));
    inps.push_back(d);
  }

  GridAlgs g(inps[0]);
  for (size_t i=1; i<inps.size(); ++i)
  {
    g.max(inps[i]);
  }
  
  _outputSweep->dataCopy(g);
  return true;
}

//------------------------------------------------------------------
bool Sweep::weighted_average(MathLoopData *out,
			     std::vector<ProcessingNode *> &args) const
{
  // expect 0th arg to be number of x to skip, then pairs of field/weight
  // so need odd args, at least 3
  if (args.size() < 3)
  {
    LOG(ERROR) << "Expect at least 3 args";
    return false;
  }
  if ((int)(args.size() %2) == 0)
  {
    LOG(ERROR) << "Expect odd # of args";
    return false;
  }

  double nx;
  if (!args[0]->getValue(nx))
  {
    LOG(ERROR) << "No value";
    return false;
  }

  vector<GridAlgs> inps;
  vector<double> weights;
  for (size_t i=1; i<args.size(); i+=2)
  {
    string dataName = args[i]->leafName();
    if (dataName.empty())
    {
      LOG(ERROR) << " NO name";
      return false;
    }
    // pull a grid2d out of the inputs
    GridAlgs d(*(_derivedData.refToData(dataName)));
    if (nx > 0)
    {
      d.adjust(nx, -1);
    }
    inps.push_back(d);

    double w;
    if (!args[i+1]->getValue(w))
    {
      LOG(ERROR) << "No value";
      return false;
    }
    weights.push_back(w);
  }

  GridAlgs g(inps[0]);
  g.setAllMissing();

  double sum_wt = 0.0;
  for (size_t i=0; i<inps.size(); ++i)
  {
    double w = weights[i];
    sum_wt += w;
    GridAlgs gi(inps[i]);
    gi.multiply(w);
    g.add(gi);
  }
  g.multiply(1.0/sum_wt);
  
  _outputSweep->dataCopy(g);
  return true;
}

//------------------------------------------------------------------
bool Sweep::mask(MathLoopData *out,
		 std::vector<ProcessingNode *> &args) const
{
  // expect 0th arg to be input field name, rest of args are pairs of ranges
  // so better be at least 3 args, and better be an odd #
  if (args.size() < 3)
  {
    LOG(ERROR) << "Expect at least 3 args";
    return false;
  }

  int n = (int)(args.size());
  if (n%2 == 0)
  {
    LOG(ERROR) << "Expect odd # of args";
    return false;
  }
  string dataName = args[0]->leafName();
  if (dataName.empty())
  {
    LOG(ERROR) << " NO name";
    return false;
  }

  vector<pair<double,double> > ranges;
  for (int i=1; i<n; i+=2)
  {
    double x0, x1;
    if (!args[i]->getValue(x0))
    {
      LOG(ERROR) << "No value";
      return false;
    }
    if (!args[i+1]->getValue(x1))
    {
      LOG(ERROR) << "No value";
      return false;
    }
    ranges.push_back(pair<double,double>(x0,x1));
  }

  // pull a grid2d out of the inputs
  const GriddedData *data = _derivedData.refToData(dataName);
  if (data == NULL)
  {
    LOG(ERROR) << "No data";
    return false;
  }
  GridAlgs g(*data);
  for (size_t i=0; i<ranges.size(); ++i)
  {
    g.maskRange(*data,  ranges[i].first, ranges[i].second);
  }
  
  _outputSweep->dataCopy(g);
  return true;
}

//------------------------------------------------------------------
bool Sweep::mask_missing_to_missing(MathLoopData *out,
				    std::vector<ProcessingNode *> &args) const
{
  // expect 2 args, 1st is input data, 2nd is input mask
  if (args.size() != 2)
  {
    LOG(ERROR) << "Need 2 inputs";
    return false;
  }
  string dataName = args[0]->leafName();
  if (dataName.empty())
  {
    LOG(ERROR) << " NO name";
    return false;
  }
  const GriddedData *data = _derivedData.refToData(dataName);
  if (data == NULL)
  {
    LOG(ERROR) << "No data";
    return false;
  }
  string maskName = args[1]->leafName();
  if (maskName.empty())
  {
    LOG(ERROR) << " NO name";
    return false;
  }
  const GriddedData *mask = _derivedData.refToData(maskName);
  if (mask == NULL)
  {
    LOG(ERROR) << "No data";
    return false;
  }
  GridAlgs g(*data);
  g.maskMissingToMissing(*mask);
  _outputSweep->dataCopy(g);
  return true;
}

//------------------------------------------------------------------
bool Sweep::trapezoid(MathLoopData *out,
		      std::vector<ProcessingNode *> &args) const
{
  // expect 5 args, input data, and 4 parameters a,b,c,d
  if (args.size() != 5)
  {
    LOG(ERROR) << "Need 5 inputs";
    return false;
  }
  string dataName = args[0]->leafName();
  if (dataName.empty())
  {
    LOG(ERROR) << " NO name";
    return false;
  }
  const GriddedData *data = _derivedData.refToData(dataName);
  if (data == NULL)
  {
    LOG(ERROR) << "No data";
    return false;
  }

  double a, b, c, d;
  if (!args[1]->getValue(a))
  {
    LOG(ERROR) << "No number";
    return false;
  }
  if (!args[2]->getValue(b))
  {
    LOG(ERROR) << "No number";
    return false;
  }
  if (!args[3]->getValue(c))
  {
    LOG(ERROR) << "No number";
    return false;
  }
  if (!args[4]->getValue(d))
  {
    LOG(ERROR) << "No number";
    return false;
  }

  TrapFuzzyF fz(a, b, c, d);
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

//------------------------------------------------------------------
bool Sweep::s_remap(MathLoopData *out,
		    std::vector<ProcessingNode *> &args) const
{
  // expect 3 args, input data, and 2 parameters a,b
  if (args.size() != 3)
  {
    LOG(ERROR) << "Need 4 inputs";
    return false;
  }
  string dataName = args[0]->leafName();
  if (dataName.empty())
  {
    LOG(ERROR) << " NO name";
    return false;
  }
  const GriddedData *data = _derivedData.refToData(dataName);
  if (data == NULL)
  {
    LOG(ERROR) << "No data";
    return false;
  }

  double a, b;
  if (!args[1]->getValue(a))
  {
    LOG(ERROR) << "No number";
    return false;
  }
  if (!args[2]->getValue(b))
  {
    LOG(ERROR) << "No number";
    return false;
  }

  SFuzzyF fz(a, b);
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

//------------------------------------------------------------------
bool Sweep::_stageInputs(const std::vector<std::string> &inputs)
{
  bool ret = true;
  GriddedData *m=NULL;
  for (size_t i=0; i<inputs.size(); ++i)
  {
    m = _refToData(inputs[i], false);
    if (m == NULL)
    {
      LOG(ERROR) << "Cannot synch input " << inputs[i];
      ret = false;
    }
  }

  // double check:
  for (size_t i=0; i<inputs.size(); ++i)
  {
    m = _derivedData.refToData(inputs[i]);
    if (m == NULL)
    {
      LOG(ERROR) << " No synch for input " << inputs[i];
      ret = false;
    }
  }
  return ret;
}
  
//------------------------------------------------------------------
bool Sweep::_stageOutput(const std::string &output)
{
  // point to output
  _outputKernels = NULL;
  _outputAscii = NULL;
  _outputSweep = NULL;

  // is the output spdb kernel output or ascii output?
  _outputKernels = _kernelOutputs.refToKernelOutput(output, true);
  if (_outputKernels != NULL)
  {
    return true;
  }

  _outputAscii = _asciiOutputs.refToAsciiOutput(output, true);
  if (_outputAscii != NULL)
  {
    return true;
  }
  
  // the output should be a grid
  // try to create an output data pointer, anything will do as template.
  if (_exampleData(output) == NULL)
  {
    LOG(ERROR) << "Cannot synch for output " << output;
    return false;
  }
  _outputSweep = _derivedData.refToData(output);
  if (_outputSweep == NULL)
  {
    LOG(ERROR) << "Difficulty synching data";
    return false;
  }
  return true;
}

//------------------------------------------------------------------
GriddedData *Sweep::_refToData(const std::string &name,  bool suppressWarn)
{
  // try to pull out of existing derived _derivedData
  GriddedData *ret = NULL;
  ret = _derivedData.refToData(name);
  if (ret != NULL)
  {
    return ret;
  }
  
  // try to pull out of input data, and if so copy to output data
  ret = _inputGrids.refToData(name);
  if (ret != NULL)
  {
    _derivedData.addField(*ret);
    ret = _derivedData.refToData(name);
    if (ret != NULL)
    {
      return ret;
    }
  }
  
  // can't pull out of state, not in input data and not in _derivedData
  if (!suppressWarn)
  {
    printf("ERROR retrieving data for %s\n", name.c_str());
  }
  return NULL;
}    

//------------------------------------------------------------------
GriddedData *Sweep::_exampleData(const std::string &name)
{
  // see if already there
  GriddedData *s = _refToData(name, true);
  if (s == NULL)
  {
    // not already there
    GriddedData r(_inputGrids[0]);
    r.setName(name);
    _derivedData.addField(r);
    s = _derivedData.refToData(name);
  }
  if (s == NULL)
  {
    LOG(ERROR) << "No data created for " << name;
  }
  return s;
}

//------------------------------------------------------------------
bool Sweep::_needToSynch(const std::string &userKey) const
{
  // everything has inputs
  return true;
}

// //------------------------------------------------------------------
// bool Sweep::_processTextureX(std::vector<ProcessingNode *> &args)
// {
//   // expect field, nx, ny as args
//   if (args.size() != 3)
//   {
//     LOG(ERROR) << "Bad interface";
//     return false;
//   }

//   double nx, ny;
//   string dataName = args[0]->leafName();
//   if (dataName.empty())
//   {
//     LOG(ERROR) << " NO name";
//     return false;
//   }
//   if (!args[1]->getValue(nx))
//   {
//     LOG(ERROR) << "No value";
//     return false;
//   }
//   if (!args[2]->getValue(ny))
//   {
//     LOG(ERROR) << "No value";
//     return false;
//   }

//   // pull a grid2d out of the inputs
//   const GriddedData *input = _derivedData.refToData(dataName);

//   GridAlgs g(*input);
//   g.textureThreaded(nx, ny, 8, true);
  
//   _outputSweep->dataCopy(g);
//   return true;
// }
// //------------------------------------------------------------------
// bool Sweep::_processStdDevNoOverlap(std::vector<ProcessingNode *> &args)
// {
//   // expect field, nx, ny as args
//   if (args.size() != 3)
//   {
//     LOG(ERROR) << "Bad interface";
//     return false;
//   }

//   double nx, ny;
//   string dataName = args[0]->leafName();
//   if (dataName.empty())
//   {
//     LOG(ERROR) << " NO name";
//     return false;
//   }
//   if (!args[1]->getValue(nx))
//   {
//     LOG(ERROR) << "No value";
//     return false;
//   }
//   if (!args[2]->getValue(ny))
//   {
//     LOG(ERROR) << "No value";
//     return false;
//   }

//   // pull a grid2d out of the inputs
//   const GriddedData *input = _derivedData.refToData(dataName);

//   GridAlgs g(*input);
//   g.sdevNoOverlap(nx, ny);
  
//   _outputSweep->dataCopy(g);
//   return true;
// }

// //------------------------------------------------------------------
// bool Sweep::_processMedianNoOverlap(std::vector<ProcessingNode *> &args)
// {
//   // expect field, nx, ny , bin0, bin1, bindelta as args
//   if (args.size() != 6)
//   {
//     LOG(ERROR) << "Bad interface";
//     return false;
//   }

//   double nx, ny, bin_min, bin_max, bin_delta;
//   string dataName = args[0]->leafName();
//   if (dataName.empty())
//   {
//     LOG(ERROR) << " NO name";
//     return false;
//   }
//   if (!args[1]->getValue(nx))
//   {
//     LOG(ERROR) << "No value";
//     return false;
//   }
//   if (!args[2]->getValue(ny))
//   {
//     LOG(ERROR) << "No value";
//     return false;
//   }
//   if (!args[3]->getValue(bin_min))
//   {
//     LOG(ERROR) << "No value";
//     return false;
//   }
//   if (!args[4]->getValue(bin_max))
//   {
//     LOG(ERROR) << "No value";
//     return false;
//   }
//   if (!args[5]->getValue(bin_delta))
//   {
//     LOG(ERROR) << "No value";
//     return false;
//   }

//   // pull a grid2d out of the inputs
//   const GriddedData *input = _derivedData.refToData(dataName);

//   GridAlgs g(*input);
//   g.medianNoOverlap(nx, ny, bin_min, bin_max, bin_delta, true);
//   _outputSweep->dataCopy(g);
//   return true;
// }

// //------------------------------------------------------------------
// bool Sweep::_processSnrFromDbz(std::vector<ProcessingNode *> &args)
// {
//   // expect field, noiseDbzAt100km
//   if (args.size() != 2)
//   {
//     LOG(ERROR) << "Bad interface";
//     return false;
//   }

//   double noiseAt100Km;
//   string dataName = args[0]->leafName();
//   if (dataName.empty())
//   {
//     LOG(ERROR) << " NO name";
//     return false;
//   }
//   if (!args[1]->getValue(noiseAt100Km))
//   {
//     LOG(ERROR) << "No value";
//     return false;
//   }

//   // pull a grid2d out of the inputs
//   const GriddedData *input = _derivedData.refToData(dataName);

//   vector<double> noiseDbz = _mdvInfo.noisePerRange(noiseAt100Km);

//   Grid2d snr(*input);
//   for (int y=0; y<_mdvInfo.ny(); ++y)
//   {
//     for (int x=0; x<_mdvInfo.nx(); ++x)
//     {
//       double v;
//       if (input->getValue(x, y, v))
//       {
// 	snr.setValue(x, y, v - noiseDbz[x]);
//       }
//       else
//       {
// 	snr.setMissing(x, y);
//       }
//     }
//   }
//   _outputSweep->dataCopy(snr);
//   return true;
// }

// #ifdef NOTDEF
// bool Sweep::_processClumping(std::vector<ProcessingNode *> &args)
// {
  
//   // expect two args, create_clumps_and_map_to_colors(clumps, clumped)"
//   if (args.size() != 2)
//   {
//     LOG(ERROR) << "Bad interface";
//     return false;
//   }

//   string dataName = args[0]->leafName();
//   if (dataName.empty())
//   {
//     LOG(ERROR) << " NO name";
//     return false;
//   }

//   // pull a grid2d out of the inputs
//   const GriddedData *input = _matchConst(dataName);

//   // see if it is either PID clumping or regular clumping
//   string typeName = args[1]->leafName();
//   if (typeName.empty())
//   {
//     LOG(ERROR) << "No Name";
//     return false;
//   }
//   bool pid_clumping = (typeName == "pid_clumping");

//   // copy it for processing into a clumping object
//   Grid2dClump clumped(*input);

//   // go for it
//   if (pid_clumping)
//   {
//     _doClump(clumped, _r);
//   }
//   else
//   {
//     _doClump(clumped, _r);
//   }
//   _outputSweep->dataCopy(clumped);
//   return true;
// }
// #endif

bool Sweep::_processRemoveSmallClumps(std::vector<ProcessingNode *> &args)
{
   if (args.size() != 3)
   {
     LOG(ERROR) << "Bad interface";
     return false;
   }

   // pull a grid2d out of the inputs
   string dataName = args[0]->leafName();
   if (dataName.empty())
   {
     LOG(ERROR) << " NO name";
     return false;
   }
  const GriddedData *input = _derivedData.refToData(dataName);
   if (input == NULL)
   {
     LOG(ERROR) << "No data";
     return false;
   }
   
   // also some regions
   string name = args[1]->leafName();
   if (name.empty())
   {
     LOG(ERROR) << " NO name";
     return false;
   }
   MathUserData *u = _special.matchingDataPtr(name);
   if (u == NULL)
   {
     LOG(ERROR) << "No data";
     return false;
   }
   ClumpRegions *regions = (ClumpRegions *)u;

   double minPt;
   if (!args[2]->getValue(minPt))
   {
     LOG(ERROR) << "No value";
     return false;
   }

   // now can go for it
   // copy it for processing into a clumping object
   Grid2d fclump(*input);
   
   for (size_t i=0; i<regions->size(); ++i)
   {
     regions->removeSmallClump(i, minPt, fclump);
   }

   _outputSweep->dataCopy(fclump);
   return true;
}
    
bool Sweep::_getEdge(std::vector<ProcessingNode *> &args)
{
  // one arg which is gaps
  if (args.size() != 1)
  {
     LOG(ERROR) << "Bad interface";
     return false;
   }

   string dataName = args[0]->leafName();
   if (dataName.empty())
   {
     LOG(ERROR) << " NO name";
     return false;
   }
  MathUserData *u = _special.matchingDataPtr(dataName);
  if (u == NULL)
  {
    LOG(ERROR) << "No data";
    return NULL;
  }
  CloudGaps *gap = (CloudGaps *)u;
  Grid2d edge = gap->edge(_outputSweep->getNx(), _outputSweep->getNy());
  _outputSweep->dataCopy(edge);
  return true;
}


bool Sweep::_getOutside(std::vector<ProcessingNode *> &args)
{
  // one arg which is gaps
  if (args.size() != 1)
  {
     LOG(ERROR) << "Bad interface";
     return false;
   }

   string dataName = args[0]->leafName();
   if (dataName.empty())
   {
     LOG(ERROR) << " NO name";
     return false;
   }
  MathUserData *u = _special.matchingDataPtr(dataName);
  if (u == NULL)
  {
    LOG(ERROR) << "No data";
    return NULL;
  }
  CloudGaps *gap = (CloudGaps *)u;
  Grid2d outside = gap->outside(_outputSweep->getNx(), _outputSweep->getNy());
  _outputSweep->dataCopy(outside);
  return true;
}


MathUserData *Sweep::_processAssociateClumps(const std::string &reg,
					     const std::string &clump0,
					     const std::string &clump1)
{
  const GriddedData *data0 = _derivedData.refToData(clump0);
  if (data0 == NULL)
  {
    LOG(ERROR) << "No input";
    return NULL;
  }
  const GriddedData *data1 = _derivedData.refToData(clump1);
  if (data1 == NULL)
  {
    LOG(ERROR) << "No input";
    return NULL;
  }

  MathUserData *u = _special.matchingDataPtr(reg);
  if (u == NULL)
  {
    LOG(ERROR) << "No data";
    return NULL;
  }
  ClumpRegions *regions = (ClumpRegions *)u;
  
 ClumpAssociate *ca = new ClumpAssociate(*regions, *data0, *data1);
  return (MathUserData *)ca;
}

MathUserData *Sweep::_processBuildGaps(const std::string &input,
				       const std::string &depth) 
{
  const GriddedData *data = _derivedData.refToData(input);
  if (data == NULL)
  {
    LOG(ERROR) << "No input";
    return NULL;
  }

  double leadingEdgeDepth;
  if (!MathParser::isFloat(depth))
  {
    LOG(ERROR) << "Not a float";
    return NULL;
  }
  
  if (sscanf(depth.c_str(), "%lf", &leadingEdgeDepth) != 1)
  {
    LOG(ERROR) << "Not float " << depth;
    return NULL;
  }

  CloudGaps *gaps = new CloudGaps();
  for (int y=0; y<data->getNy(); ++y)
  {
    gaps->addGaps(y, data->getNx(), *data, leadingEdgeDepth);
  }
  return (MathUserData *)gaps;
}

//------------------------------------------------------------------
MathUserData *Sweep::_createClumps(const std::string &dataName)
{
  // pull a grid2d out of the inputs
  const GriddedData *input = _derivedData.refToData(dataName);
  if (input == NULL)
  {
    LOG(ERROR) << "No input";
    return NULL;
  }
  ClumpRegions *r = new ClumpRegions(*input);
  return (MathUserData *)r;
}

//------------------------------------------------------------------
MathUserData *Sweep::_removeSmallGaps(const std::string &gaps,
				      const std::string &mingap)
{
  MathUserData *u = _special.matchingDataPtr(gaps);
  if (u == NULL)
  {
    LOG(ERROR) << "No data";
    return NULL;
  }
  CloudGaps *cgaps = (CloudGaps *)u;

  double min;
  if (!MathParser::isFloat(mingap))
  {
    LOG(ERROR) << "Not a float";
    return NULL;
  }
    
  if (sscanf(mingap.c_str(), "%lf", &min) != 1)
  {
    LOG(ERROR) << "Not float " << mingap;
    return NULL;
  }

  Mdvx::coord_t coord = _mdvInfo.proj().getCoord();
  int min_gridpt = (int)(min/coord.dx);

  CloudGaps *ret = new CloudGaps(*cgaps);
  ret->filter(min_gridpt);
  return (MathUserData *)ret;
}
  
//------------------------------------------------------------------
MathUserData *Sweep::_filterGapsInside(const std::string &gaps,
				       const std::string &pidClumps,
				       const std::string &associatedClumps,
				       const std::string &maxPenetration)
{
  MathUserData *u;
  u = _special.matchingDataPtr(gaps);
  if (u == NULL)
  {
    LOG(ERROR) << "No data";
    return NULL;
  }
  CloudGaps *cgaps = (CloudGaps *)u;

  const GriddedData *clumps = _derivedData.refToData(pidClumps);
  if (clumps == NULL)
  {
    LOG(ERROR) << "No input";
    return NULL;
  }
  
  u = _special.matchingDataPtr(associatedClumps);
  if (u == NULL)
  {
    LOG(ERROR) << "No data";
    return NULL;
  }
  ClumpAssociate *ca = (ClumpAssociate *)u;

  double max;
  if (!MathParser::isFloat(maxPenetration))
  {
    LOG(ERROR) << "Not a float";
    return NULL;
  }
  if (sscanf(maxPenetration.c_str(),"%lf", &max) != 1)
  {
    LOG(ERROR) << "Not float " << maxPenetration;
    return NULL;
  }
  Mdvx::coord_t coord = _mdvInfo.proj().getCoord();
  int maxP = (int)(max/coord.dx);
  
  CloudGaps *ret = new CloudGaps(*cgaps);
  ret->filter(*clumps, *ca, maxP);
  return (MathUserData *)ret;
}

//------------------------------------------------------------------
bool Sweep::_clumpsToGrid(std::vector<ProcessingNode *> &args)
{
   if (args.size() != 1)
   {
     LOG(ERROR) << "Bad interface";
     return false;
   }

   string name = args[0]->leafName();
   if (name.empty())
   {
     LOG(ERROR) << " NO name";
     return false;
   }

   MathUserData *u = _special.matchingDataPtr(name);
   if (u == NULL)
   {
     LOG(ERROR) << "No data";
     return false;
   }
   ClumpRegions *regions = (ClumpRegions *)u;

   _outputSweep->setAllMissing();
   for (size_t i=0; i<regions->size(); ++i)
   {
     double color = indexToColor(i);
     regions->setValues(i, color, *_outputSweep);
   }
   return true;
}

bool Sweep::_inverseMask(std::vector<ProcessingNode *> &args)
{
   if (args.size() != 1)
   {
     LOG(ERROR) << "Bad interface";
     return false;
   }

   string name = args[0]->leafName();
   if (name.empty())
   {
     LOG(ERROR) << " NO name";
     return false;
   }

   MathUserData *u = _special.matchingDataPtr(name);
   if (u == NULL)
   {
     LOG(ERROR) << "No data";
     return false;
   }
   ClumpRegions *regions = (ClumpRegions *)u;

   _outputSweep->setAllToValue(1.0);
   double mv = _outputSweep->getMissing();
   for (size_t i=0; i<regions->size(); ++i)
   {
     (*regions)[i].toGrid(*_outputSweep, mv);
   }
   return true;
}

MathUserData *Sweep::_organize(const std::string &pid,
			       const std::string &snoise,
			       const std::string &knoise,
			       const std::string &sdbz,
			       const std::string &kdbz,
			       const std::string &szdr,
			       const std::string &srhohv,
			       const std::string &kdbzAdjusted,
			       const std::string &dbzDiff)
{
  const Grid2d *pidG, *snoiseG, *knoiseG, *sdbzG, *kdbzG;
  const Grid2d *szdrG, *srhohvG, *kdbzAdjustedG, *dbzDiffG;
    
  pidG = _derivedData.refToData(pid);
  snoiseG = _derivedData.refToData(snoise);
  knoiseG = _derivedData.refToData(knoise);
  sdbzG = _derivedData.refToData(sdbz);
  kdbzG = _derivedData.refToData(kdbz);
  szdrG = _derivedData.refToData(szdr);
  srhohvG = _derivedData.refToData(srhohv);
  kdbzAdjustedG = _derivedData.refToData(kdbzAdjusted);
  dbzDiffG = _derivedData.refToData(dbzDiff);
  
  if (pidG == NULL || snoiseG == NULL || knoiseG == NULL ||
      sdbzG == NULL || kdbzG == NULL || szdrG == NULL ||
      srhohvG == NULL || kdbzAdjustedG == NULL || dbzDiffG == NULL)
  {
    LOG(ERROR) << "Missing input";
    return NULL;
  }
  KernelGrids *kg = new KernelGrids(&sdbzG, &kdbzG, &szdrG, &pidG, &snoiseG,
				    &knoiseG, &srhohvG, &kdbzAdjustedG,
				    &dbzDiffG);
  return (MathUserData *)kg;
}

			       
MathUserData *Sweep::_kernelFilter(const std::string &kernels)
{
  MathUserData *u = _special.matchingDataPtr(kernels);
  if (u == NULL)
  {
    LOG(ERROR) << "No data";
    return NULL;
  }
  Kernels *k = (Kernels *)u;
  Kernels *kfilt = new Kernels(*k);
  kfilt->filterToGood();
  return (MathUserData *)kfilt;
}

MathUserData *Sweep::_kernelBuild(const std::string &outsideMask,
				  const std::string &gaps,
				  const std::string &clumpReg,
				  const std::string &kgrids)
{
  const GriddedData *mask = _derivedData.refToData(outsideMask);
  if (mask == NULL)
  {
    LOG(ERROR) << "No data";
    return NULL;
  }

  MathUserData *u;
  u = _special.matchingDataPtr(gaps);
  if (u == NULL)
  {
    LOG(ERROR) << "No data";
    return NULL;
  }
  CloudGaps *cgaps = (CloudGaps *)u;

  u = _special.matchingDataPtr(clumpReg);
  if (u == NULL)
  {
    LOG(ERROR) << "No data";
    return NULL;
  }
  ClumpRegions *regions = (ClumpRegions *)u;

  u = _special.matchingDataPtr(kgrids);
  if (u == NULL)
  {
    LOG(ERROR) << "No data";
    return NULL;
  }
  KernelGrids *kg = (KernelGrids *)u;

  Kernels *ret = new Kernels();

  for (int i=0; i<cgaps->num(); ++i)
  {
    const CloudGap &g = cgaps->ith_cloudgap(i);
    // build mask to far and near edge points
    Grid2d mask_far, mask_near;
    if (!_kernel_mask(g, *regions, true, mask_far))
    {
      continue;
    }
    if (!_kernel_mask(g, *regions, false, mask_near))
    {
      continue;
    }

    // build kernel pair for this gap
    KernelPair kp(_vlevel, mask_far, mask_near, *mask, g,
		  _repohParms._main, _mdvInfo.proj());
    if (kp.isBigEnough())
    {
      int id = ret->nextAvailableId();
      Mdvx::coord_t coord = _mdvInfo.proj().getCoord();
      kp.finishProcessing(_time, _vlevel, *kg, _repohParms._main, coord.dx,
			  id, id+1);
      ret->append(kp);
      ret->incrementNextAvailableId(2);
    }
  }
  return (MathUserData *)ret;
}

bool Sweep::_centerPoints(std::vector<ProcessingNode *> &args)
{
   if (args.size() != 1)
   {
     LOG(ERROR) << "Bad interface";
     return false;
   }

   string name = args[0]->leafName();
   if (name.empty())
   {
     LOG(ERROR) << " NO name";
     return false;
   }

  MathUserData *u;
  u = _special.matchingDataPtr(name);
  if (u == NULL)
  {
    LOG(ERROR) << "No data";
    return NULL;
  }
  Kernels *k = (Kernels *)u;

  // set grid to all missing
  _outputSweep->setAllMissing();

  for (size_t i=0; i<k->size(); ++i)
  {
    (*k)[i].centerpointToGrid(*_outputSweep);
  }
  return true;
}

/*----------------------------------------------------------------*/
bool Sweep::_attenuation(std::vector<ProcessingNode *> &args)
{
   if (args.size() != 1)
   {
     LOG(ERROR) << "Bad interface";
     return false;
   }

   string name = args[0]->leafName();
   if (name.empty())
   {
     LOG(ERROR) << " NO name";
     return false;
   }

  MathUserData *u;
  u = _special.matchingDataPtr(name);
  if (u == NULL)
  {
    LOG(ERROR) << "No data";
    return NULL;
  }
  Kernels *k = (Kernels *)u;

  k->computeAttenuation(_mdvInfo.dx(), *_outputSweep);
  return (MathUserData *)k;
}

/*----------------------------------------------------------------*/
bool Sweep::_totalAttenuation(std::vector<ProcessingNode *> &args)
{
  if (args.size() != 2)
  {
    LOG(ERROR) << "Bad interface";
    return false;
  }

  string name = args[0]->leafName();
  if (name.empty())
  {
    LOG(ERROR) << " NO name";
    return false;
  }
  GriddedData *DWR = _refToData(name);
  if (DWR == NULL)
  {
    LOG(ERROR) << "No data";
    return false;
  }

  name = args[1]->leafName();
  if (name.empty())
  {
    LOG(ERROR) << " NO name";
    return false;
  }
  GriddedData *clumps = _refToData(name);
  if (DWR == NULL)
  {
    LOG(ERROR) << "No data";
    return false;
  }

  GridAlgs a(*_outputSweep);
  a.totalAttenuation(*clumps, *DWR, 2.0, _mdvInfo.dx());
  _outputSweep->dataCopy(a);
  return true;
}

/*----------------------------------------------------------------*/
bool Sweep::_averageAttenuation(std::vector<ProcessingNode *> &args)
{
  if (args.size() != 2)
  {
    LOG(ERROR) << "Bad interface";
    return false;
  }

  string name = args[0]->leafName();
  if (name.empty())
  {
    LOG(ERROR) << " NO name";
    return false;
  }
  GriddedData *extent = _refToData(name);
  if (extent == NULL)
  {
    LOG(ERROR) << "No data";
    return false;
  }

  name = args[1]->leafName();
  if (name.empty())
  {
    LOG(ERROR) << " NO name";
    return false;
  }
  GriddedData *atotal = _refToData(name);
  if (atotal == NULL)
  {
    LOG(ERROR) << "No data";
    return false;
  }

  GridAlgs a(*_outputSweep);
  a.averageAttenuation(*extent, *atotal);
  _outputSweep->dataCopy(a);
  return true;
}

/*----------------------------------------------------------------*/
bool Sweep::_nptBetweenGood(std::vector<ProcessingNode *> &args)
{
  if (args.size() != 2)
  {
    LOG(ERROR) << "Bad interface";
    return false;
  }

  string name = args[0]->leafName();
  if (name.empty())
  {
    LOG(ERROR) << " NO name";
    return false;
  }
  GriddedData *DWR = _refToData(name);
  if (DWR == NULL)
  {
    LOG(ERROR) << "No data";
    return false;
  }

  name = args[1]->leafName();
  if (name.empty())
  {
    LOG(ERROR) << " NO name";
    return false;
  }
  GriddedData *clumps = _refToData(name);
  if (clumps == NULL)
  {
    LOG(ERROR) << "No data";
    return false;
  }

  GridAlgs a(*_outputSweep);
  int minPt = (int)(2.0/_mdvInfo.dx());
  a.nptBetweenGoodDataPointsX(*clumps, *DWR, minPt);
  _outputSweep->dataCopy(a);
  return true;
}

/*----------------------------------------------------------------*/
bool Sweep::_sumZ(std::vector<ProcessingNode *> &args)
{
  if (args.size() != 3)
  {
    LOG(ERROR) << "Bad interface";
    return false;
  }

  string name = args[0]->leafName();
  if (name.empty())
  {
    LOG(ERROR) << " NO name";
    return false;
  }
  GriddedData *Zs = _refToData(name);
  if (Zs == NULL)
  {
    LOG(ERROR) << "No data";
    return false;
  }

  name = args[1]->leafName();
  if (name.empty())
  {
    LOG(ERROR) << " NO name";
    return false;
  }
  GriddedData *extent = _refToData(name);
  if (extent == NULL)
  {
    LOG(ERROR) << "No data";
    return false;
  }

  double p;
  if (!args[2]->getValue(p))
  {
    LOG(ERROR) << "No value";
    return false;
  }

  GridAlgs a(*_outputSweep);
  a.sumZ(*Zs, *extent, p);
  _outputSweep->dataCopy(a);
  return true;
}

/*----------------------------------------------------------------*/
bool Sweep::_humidity(std::vector<ProcessingNode *> &args)
{
   if (args.size() != 1)
   {
     LOG(ERROR) << "Bad interface";
     return false;
   }

   string name = args[0]->leafName();
   if (name.empty())
   {
     LOG(ERROR) << " NO name";
     return false;
   }

  MathUserData *u;
  u = _special.matchingDataPtr(name);
  if (u == NULL)
  {
    LOG(ERROR) << "No data";
    return NULL;
  }
  Kernels *k = (Kernels *)u;

  k->computeHumidity(_mdvInfo.dx(), *_outputSweep);
  return true;
}

/*----------------------------------------------------------------*/
bool Sweep::_fir(std::vector<ProcessingNode *> &args)
{
   if (args.size() != 1)
   {
     LOG(ERROR) << "Bad interface";
     return false;
   }

   string name = args[0]->leafName();
   if (name.empty())
   {
     LOG(ERROR) << " NO name";
     return false;
   }
   const GriddedData *inp = _derivedData.refToData(name);
   if (inp == NULL)
   {
     LOG(ERROR) << "No data";
     return false;
   }

   if (_outputSweep == NULL)
   {
     LOG(ERROR) << "No output data";
     return false;
   }

   GridAlgs g;
   FIRFilter filt;
   if (filt.filter(*inp, g))
   {
     _outputSweep->dataCopy(g);
     return true;
   }
   else
   {
     LOG(ERROR) << "No FIR filter";
     return false;
   }
}

/*----------------------------------------------------------------*/
MathUserData *Sweep::_processAsciiOutput(const std::string &kernels)
{
  if (_outputAscii == NULL)
  {
    LOG(ERROR) << "No output ascii set up";
    return NULL;
  }

  MathUserData *u;
  u = _special.matchingDataPtr(kernels);

  if (u == NULL)
  {
    LOG(ERROR) << "No data";
    return NULL;
  }
  Kernels *k = (Kernels *)u;

  string s = k->asciiOutput(_vlevel, _mdvInfo.proj());
  _outputAscii->appendNoCr(s);

  // make a new object, because the returned 'special value' is stored
  // here, and then deleted when the Sweep object is deleted. In other words
  // this is kind of lame within the design
  AsciiOutput *ret = new AsciiOutput(*_outputAscii);
  return (MathUserData *)ret;
}

/*----------------------------------------------------------------*/
// build a mask for the kernel points associated with a gap
bool Sweep::_kernel_mask(const CloudGap &gap, 
			 const ClumpRegions &regions,
			 const bool is_far, Grid2d &mask) const
{
  // initialize the mask to all missing
  mask = Grid2d(_inputGrids[0]);  
  mask.setAllMissing();

  if (gap.isClosest() && !is_far)
  {
    // no actual kernel points at origin
    return true;
  }
  
  // pull out the color
  double color = gap.getValue(is_far);

  // and use color to get index into regions
  int index = Sweep::colorToIndex(color);
  if (index < 0 || index >= (int)regions.size())
  {
    LOG(ERROR) << "index " << index << " out of range [0," << 
	 (int)regions.size()-1;
    return false;
  }

  // make a mask with the appropriate region points
  regions[index].toGrid(mask, 1.0);

  // const clump::Region_t *ri = &_r[index];

  // for (int i=0; i<(int)ri->size(); ++i)
  // {
  //   int xi, yi;
  //   xi = (*ri)[i].first;
  //   yi = (*ri)[i].second;
  //   mask.setValue(xi, yi, 1.0);
  // }
  return true;
}



MathUserData *Sweep::_kernelToGenPoly(const std::string &kernels)
{
  MathUserData *u;
  u = _special.matchingDataPtr(kernels);
  if (u == NULL)
  {
    LOG(ERROR) << "No data";
    return NULL;
  }
  Kernels *k = (Kernels *)u;

  // the output should be in place as a KernelOutputs pointer
  if (_outputKernels == NULL)
  {
    LOG(ERROR) << "Output not set";
    return NULL;
  }

  // make this Kernels object part of the _outputKernels, which is owned
  // by the volume
  _outputKernels->storeKernel(_vlevelIndex, *k);

  // make a new object, because the returned 'special value' is stored
  // here, and then deleted when the Sweep object is deleted. In other words
  // this is kind of lame within the design
  KernelOutput *k2 = new KernelOutput(*_outputKernels);
  return (MathUserData *)k2;
}
