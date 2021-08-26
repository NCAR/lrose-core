/**
 * @file Sweep.cc
 */

//------------------------------------------------------------------
#include "Sweep.hh"
#include "Volume.hh"
#include "LineDetection.hh"
#include "ShearDetection.hh"
#include "EllipHandler.hh"
#include "EnhanceHandler.hh"
#include "RegHandler.hh"
#include "RegCombHandler.hh"
#include <euclid/GridAlgs.hh>
#include <rapmath/MathParser.hh>
#include <rapmath/ProcessingNode.hh>
#include <rapmath/UnaryNode.hh>
#include <toolsa/LogStream.hh>
#include <cmath>

const std::string Sweep::_lineDetStr = "LineDet";
const std::string Sweep::_lineDirStr = "LineDir";
const std::string Sweep::_shearDetStr = "ShearDet";
const std::string Sweep::_shearDirStr = "ShearDir";
const std::string Sweep::_ellipStr = "Ellip";
const std::string Sweep::_ellipOrientStr = "EllipOrient";
const std::string Sweep::_ellipConfStr = "EllipConf";
const std::string Sweep::_enhanceStr = "Enhance";
const std::string Sweep::_enhanceDirStr = "EnhanceDir";
const std::string Sweep::_regionStr = "Region";
const std::string Sweep::_regCombStr = "RegComb";
const std::string Sweep::_historyStr = "History";
const std::string Sweep::_maxAgeMinutesStr = "MaxAgeMinutes";

//------------------------------------------------------------------
Sweep::Sweep(void) : VirtVolSweep()
{
}

//------------------------------------------------------------------
Sweep::Sweep(const Volume &volume, int index, double vlevel) :
  VirtVolSweep(volume, index, vlevel)
{
  _parms = volume._parms;
  _oldData = OldSweepData(volume._oldData, index);
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

  ret.push_back(FunctionDef(_lineDirStr, "d", "field,nx,ny,nthread",
			    "Thin line best rotation degrees. nx = box length, ny=box width, nthread = number of threads"));
  ret.push_back(FunctionDef(_lineDetStr, "d", "field,dir,nx,ny,nthread",
			    "Thin line detection interest.  dir=bestdirection, nx=box length, ny=box width, nthread = number of threads"));
    
  ret.push_back(FunctionDef(_shearDirStr, "d", "field,nx,ny,nthread",
			    "Radial shear best rotation degrees.  nx = box length, ny=box width, nthread = number of threads"));
  ret.push_back(FunctionDef(_shearDetStr, "d",
			    "field,shear_dir,line_dir,nx,ny,nthread",
			    "Radial shear detction interest.  shear_dir=best direction shear, line_dir=bestdirection line, nx = box length, ny=box width, nthread = number of threads"));
    
  ret.push_back(FunctionDef(_ellipOrientStr, "o", "field, nx, ny, nthread",
			    "Elliptical best rotation degrees.  nx=length, ny=width, nthread = number of threads"));

  ret.push_back(FunctionDef(_ellipStr, "e", "field, orient, nx, ny, nthread",
			    "Elliptical interest. orient=best ellip orientation, nx=length, ny=width, nthread=number of threads"));

  ret.push_back(FunctionDef(_ellipConfStr, "c", "field,dir,nx, ny, nthread",
			    "Elliptical confidence. dir=best ellip orientation, nx=length, ny=width, nthread=number of threads"));
  ret.push_back(FunctionDef(_enhanceDirStr, "d", 
			    "field,len,wid,nangles,nthread",
			    "Enhance rotation degrees. len=length,width=width, nangles=#ofangles, nthread=number of threads"));
  ret.push_back(FunctionDef(_enhanceStr, "e","field,len,wid,nangles,nthread",
			    "Enhance interest. len=length,width=width, nangles=#ofangles, nthread=number of threads"));
  ret.push_back(FunctionDef(_regionStr, "r",  
			    "hot,reg,full,min_mean,min_max,min_min,min_area,min_hot_area",
			    "hot = interest is high, reg=clumps, full=largerclumps, min_min=Minimum mean length, min_max=minimum max length, min_min=minimum min length, min_area=minimum clump area (pixsq) "
			    " min_hot_area=minimum 'hotspot' area (pixsq)"));

  ret.push_back(FunctionDef(_regCombStr, "r",
			    "reg, dir, extension, angleDiff, minHit",
			    ""));
  ret.push_back(FunctionDef(_historyStr, "r",
			    "field, dt0, w0, dt1, w1, ....", ""));

  ret.push_back(FunctionDef(_maxAgeMinutesStr, "s", "field",
			    "minutes back with maximum data value of input field"));
  return ret;
}

//------------------------------------------------------------------
bool Sweep::synchInputsAndOutputs(const std::string &output,
				  const std::vector<std::string> &inputs)
{
  bool haveAll = false;
  if (!synchGriddedInputsAndOutputs(output, inputs, haveAll))
  {
    return false;
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

  if (keyword == _lineDirStr)
  {
    return _processLineDir(*(p.unaryOpArgs()));
  }
  if (keyword == _lineDetStr)
  {
    return _processLineDet(*(p.unaryOpArgs()));
  }
  if (keyword == _shearDirStr)
  {
    return _processShearDir(*(p.unaryOpArgs()));
  }
  if (keyword == _shearDetStr)
  {
    return _processShearDet(*(p.unaryOpArgs()));
  }
  if (keyword == _ellipOrientStr)
  {
    return _processEllipOrient(*(p.unaryOpArgs()));
  }
  if (keyword == _ellipStr)
  {
    return _processEllip(*(p.unaryOpArgs()));
  }    
  if (keyword == _ellipConfStr)
  {
    return _processEllipConf(*(p.unaryOpArgs()));
  }    
  if (keyword == _enhanceStr)
  {
    return _processEnhance(*(p.unaryOpArgs()));
  }    
  if (keyword == _enhanceDirStr)
  {
    return _processEnhanceDirection(*(p.unaryOpArgs()));
  }    
  if (keyword == _regionStr)
  {
    return _processRegion(*(p.unaryOpArgs()));
  }    
  if (keyword == _regCombStr)
  {
    return _processRegComb(*(p.unaryOpArgs()));
  }

  if (keyword == _historyStr)
  {
    return _processHistory(*(p.unaryOpArgs()));
  }

  if (keyword == _maxAgeMinutesStr)
  {
    return _processMaxAgeMinutes(*(p.unaryOpArgs()));
  }
  else
  {
    return processVirtVolUserLoopFunction(p);
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
  vector<string> args = p.getUnaryNodeArgStrings();
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
bool Sweep::_processLineDir(std::vector<ProcessingNode *> &args)
{
  // field, nx, ny, nthread
  
  const MathLoopData *data;
  double nx, ny, nthread;
  if (!loadDataAndThreeNumbers(args, &data, nx, ny, nthread))
  {
    return false;
  }

  // pull a grid2d out of the inputs
  const GriddedData *input = (const GriddedData *)data;

  Grid2d g(*input);
  LineDetection ld(nx, ny, nthread, _parms);
  ld.processDir(input, &g);
  _outputSweep->dataCopy(g);
  return true;
}

//------------------------------------------------------------------
bool Sweep::_processLineDet(std::vector<ProcessingNode *> &args)
{
  // field, direction, nx, ny, nthread

  vector<const MathLoopData *> data;
  vector<double> values;

  if (!loadMultiDataAndMultiValues(args, 2, data, 3, values))
  {
    return false;
  }

  double nx, ny, nthread;
  nx = values[0];
  ny = values[1];
  nthread = values[2];

  // pull a grid2d out of the inputs
  const GriddedData *input = (const GriddedData *)(data[0]);
  const GriddedData *dir = (const GriddedData *)(data[1]);

  Grid2d g(*input);
  LineDetection ld(nx, ny, nthread, _parms);
  ld.processDet(input, dir, &g);
  _outputSweep->dataCopy(g);
  return true;
}

//------------------------------------------------------------------
bool Sweep::_processShearDir(std::vector<ProcessingNode *> &args)
{
  // field, secondary, nx, ny, nthread

  vector<const MathLoopData *> data;
  vector<double> values;

  if (!loadMultiDataAndMultiValues(args, 2, data, 3, values))
  {
    return false;
  }

  double nx, ny, nthread;
  nx = values[0];
  ny = values[1];
  nthread = values[2];

  // pull a grid2d out of the inputs
  const GriddedData *input = (const GriddedData *)(data[0]);
  const GriddedData *secondary = (const GriddedData *)(data[1]);


  Grid2d g(*input);
  ShearDetection sd(nx, ny, nthread, _parms, _proj);
  sd.processDir(input, secondary, &g);
  _outputSweep->dataCopy(g);
  return true;
}

//------------------------------------------------------------------
bool Sweep::_processEllipOrient(std::vector<ProcessingNode *> &args)
{
  // expect field, nx, ny nthread
  const MathLoopData *data;
  double nx, ny, nthread;
  if (!loadDataAndThreeNumbers(args, &data, nx, ny, nthread))
  {
    return false;
  }

  // pull a grid2d out of the inputs
  const GriddedData *input = (const GriddedData *)(data);

  Grid2d g(*input);
  EllipHandler sd(nx, ny, nthread, _parms);
  sd.processDir(input, &g);
  _outputSweep->dataCopy(g);
  return true;
}

//------------------------------------------------------------------
bool Sweep::_processEllip(std::vector<ProcessingNode *> &args)
{
  // expect field, orientation, nx, ny nthread

  vector<const MathLoopData *> data;
  vector<double> values;

  if (!loadMultiDataAndMultiValues(args, 2, data, 3, values))
  {
    return false;
  }

  double nx, ny, nthread;
  nx = values[0];
  ny = values[1];
  nthread = values[2];

  // pull a grid2d out of the inputs
  const GriddedData *input = (const GriddedData *)(data[0]);
  const GriddedData *orient = (const GriddedData *)(data[1]);

  Grid2d g(*input);
  EllipHandler sd(nx, ny, nthread, _parms);
  sd.process(input, orient, &g);
  _outputSweep->dataCopy(g);
  return true;
}

//------------------------------------------------------------------
bool Sweep::_processEllipConf(std::vector<ProcessingNode *> &args)
{
  // expect field, orientation, nx, ny nthread
  // expect field, orientation, nx, ny nthread

  vector<const MathLoopData *> data;
  vector<double> values;

  if (!loadMultiDataAndMultiValues(args, 2, data, 3, values))
  {
    return false;
  }

  double nx, ny, nthread;
  nx = values[0];
  ny = values[1];
  nthread = values[2];

  // pull a grid2d out of the inputs
  const GriddedData *input = (const GriddedData *)(data[0]);
  const GriddedData *orient = (const GriddedData *)(data[1]);

  Grid2d g(*input);
  EllipHandler sd(nx, ny, nthread, _parms);
  sd.processConf(input, orient, &g);
  _outputSweep->dataCopy(g);
  return true;
}

//------------------------------------------------------------------
bool Sweep::_processShearDet(std::vector<ProcessingNode *> &args)
{
  // expect field, dir, secondary, nx, ny nthread

  vector<const MathLoopData *> data;
  vector<double> values;

  if (!loadMultiDataAndMultiValues(args, 3, data, 3, values))
  {
    return false;
  }

  double nx, ny, nthread;
  nx = values[0];
  ny = values[1];
  nthread = values[2];

  // pull a grid2d out of the inputs
  const GriddedData *input = (const GriddedData *)(data[0]);
  const GriddedData *dir = (const GriddedData *)(data[1]);
  const GriddedData *secondary = (const GriddedData *)(data[2]);


  Grid2d g(*input);
  ShearDetection sd(nx, ny, nthread, _parms, _proj);
  sd.processDet(input, dir, secondary, &g);
  _outputSweep->dataCopy(g);
  return true;
}

//------------------------------------------------------------------
bool Sweep::_processEnhance(std::vector<ProcessingNode *> &args)
{
  // expect field, len, width, numAngles, numThreads (fuzzyF pulled out
  // of main params)
  const MathLoopData *data;
  double nx, ny, nthread, nangles;
  if (!loadDataAndFourNumbers(args, &data, nx, ny, nangles, nthread))
  {
    return false;
  }

  // pull a grid2d out of the inputs
  const GriddedData *input = (const GriddedData *)data;

  // same units for input and output, this is a rescale alg
  Grid2d g(*input);
  EnhanceHandler e(nx, ny, nangles, _parms._enhanceFuzzy, nthread);
  e.process(input, &g);
  _outputSweep->dataCopy(g);
  return true;
}

//------------------------------------------------------------------
bool Sweep::_processEnhanceDirection(std::vector<ProcessingNode *> &args)
{
  // expect field, len, width, numAngles, numThreads (fuzzyF pulled out
  // of main params)
  // expect field, len, width, numAngles, numThreads (fuzzyF pulled out
  // of main params)
  const MathLoopData *data;
  double nx, ny, nthread, nangles;
  if (!loadDataAndFourNumbers(args, &data, nx, ny, nangles, nthread))
  {
    return false;
  }

  // pull a grid2d out of the inputs
  const GriddedData *input = (const GriddedData *)data;

  Grid2d g(*input);
  EnhanceHandler e(nx, ny, nangles, _parms._enhanceFuzzy, nthread);
  e.processDir(input, &g);
  _outputSweep->dataCopy(g);
  return true;
}

//------------------------------------------------------------------
bool Sweep::_processRegion(std::vector<ProcessingNode *> &args)
{
  // expect hotspots, regions, full, min_mean_len, min_max_len, min_min_len,
  // min_area, min_hot_area
  
  vector<const MathLoopData *> data;
  vector<double> values;

  if (!loadMultiDataAndMultiValues(args, 3, data, 5, values))
  {
    return false;
  }

  double min_mean_len = values[0];
  double min_max_len = values[1];
  double min_min_len = values[2];
  double min_area = values[3];
  double min_hot_area = values[4];


  // pull grid2d out of the inputs
  const GriddedData *hot = (const GriddedData *)(data[0]);
  const GriddedData *reg = (const GriddedData *)(data[1]);
  const GriddedData *full = (const GriddedData *)(data[2]);

  Grid2d g(*reg);

  RegHandler rH(min_mean_len, min_max_len, min_min_len, min_area, min_hot_area);
  rH.process(hot, reg, full, &g);
  _outputSweep->dataCopy(g);
  return true;
}

//------------------------------------------------------------------
bool Sweep::_processRegComb(std::vector<ProcessingNode *> &args)
{
  // expect reg, dir, extension,anglediff,minhit
  
  vector<const MathLoopData *> data;
  vector<double> values;

  if (!loadMultiDataAndMultiValues(args, 2, data, 3, values))
  {
    return false;
  }

  double extension = values[0];
  double angleDiff = values[1];
  double minHit = values[2];


  // pull grid2d out of the inputs
  const GriddedData *reg = (const GriddedData *)(data[0]);
  const GriddedData *dir = (const GriddedData *)(data[1]);

  const Grid2d *greg = dynamic_cast<const Grid2d *>(reg);
  const Grid2d *gdir = dynamic_cast<const Grid2d *>(dir);

  Grid2d g(*reg);

  RegCombHandler rH(extension, angleDiff, minHit);
  rH.process(greg, gdir, &g);
  _outputSweep->dataCopy(g);
  return true;
}

//------------------------------------------------------------------
bool Sweep::_processHistory(std::vector<ProcessingNode *> &args)
{
  //s = "r=History(field, dt0, w0, dt1, w1, ....)";
  // expect an odd number of inputs, as shown above
  const MathLoopData *ldata;
  vector<pair<double,double> > dw;
  if (!loadDataAndPairs(args, &ldata, dw))
  {
    return false;
  }
  // want at least 2 pairs in this case
  if (dw.size() < 1)
  {
    LOG(ERROR) << "Bad interface";
    return false;
  }

  // and pull out the name
  string dataName = getDataName(args, 0);
  
  FuzzyF dtToWt(dw);
  for (size_t i=0; i<_oldData.size(); ++i)
  {
    if (_oldData[i].fieldMatch(dataName))
    {
      vector<Grid2d> inps;
      vector<double> weights;
      if (_oldData[i].constructWeightedFields(_time, dtToWt, inps, weights))
      {    
	if (inps.empty())
	{
	  _outputSweep->setAllMissing();
	  return true;
	}
	else
	{
	  GridAlgs g(inps[0]);
	  g.weightedAverage(inps, weights);
	  _outputSweep->dataCopy(g);
	  return true;
	}
      }
      else
      {
	LOG(ERROR) << "No weighted field for " << i;
	return false;
      }
    }
  }
  LOG(ERROR) << "No match for " << dataName << " in old data";
  return false;
}

//------------------------------------------------------------------
bool Sweep::_processMaxAgeMinutes(std::vector<ProcessingNode *> &args)
{
  string dataName = getDataName(args, 0);
  if (dataName.empty())
  {
    LOG(ERROR) << " NO name";
    return false;
  }
  
  for (size_t i=0; i<_oldData.size(); ++i)
  {
    if (_oldData[i].fieldMatch(dataName))
    {
      Grid2d age;
      if (_oldData[i].constructAgeWithMax(_time, age))
      {    
	_outputSweep->dataCopy(age);
	return true;
      }
    }
  }
  LOG(ERROR) << "No match for " << dataName << " in old data";
  _outputSweep->setAllMissing();
  return false;
}


