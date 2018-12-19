/**
 * @file Sweep.cc
 */

//------------------------------------------------------------------
#include "Sweep.hh"
#include "Volume.hh"
#include "AsciiOutput.hh"
#include "KernelGrids.hh"
#include "Kernels.hh"
#include "ClumpAssociate.hh"
#include "ClumpRegions.hh"
#include "CloudGaps.hh"
#include "FIRFilter.hh"
#include <euclid/GridAlgs.hh>
#include <euclid/Grid2dClump.hh>
#include <rapmath/MathParser.hh>
#include <rapmath/ProcessingNode.hh>
#include <rapmath/UnaryNode.hh>
#include <toolsa/LogStream.hh>
#include <cmath>
#include <algorithm>

//------------------------------------------------------------------
Sweep::Sweep(void)  : SweepMdv()
{
}

//------------------------------------------------------------------
Sweep::Sweep(const Volume &volume, int index, double vlevel) :
  SweepMdv(volume, index, vlevel),
  _kernelOutputs(volume._kernelOutputs),
  _asciiOutputs(volume._asciiOutputs),
  _parms(volume._parms)
{

  Mdvx::coord_t coord = _proj.getCoord();
  _dx = coord.dx;
}

//------------------------------------------------------------------
Sweep::~Sweep(void)
{
}

//------------------------------------------------------------------
std::vector<std::pair<std::string, std::string> >
Sweep::userUnaryOperators(void) const
{
  std::vector<std::pair<std::string, std::string> > ret;

  ret.push_back(pair<string,string>("create_clumps_and_map_to_colors","clumped = clumped"));
  ret.push_back(pair<string,string>("create_clumps", "clump_regions = create_clumps(clumps)"));
  ret.push_back(pair<string,string>("clumps_to_grid", "clumps_to_grid(clump_regions)"));
  ret.push_back(pair<string,string>("remove_small_clumps", "clumpFilt = remove_small_clumps(clumps, minPt)"));
  ret.push_back(pair<string,string>("associate_clumps","associate_clumps(clumps, pclumps)"));
  ret.push_back(pair<string,string>("build_gaps", "build_gaps"));
  ret.push_back(pair<string,string>("get_edge", "get_edge"));
  ret.push_back(pair<string,string>("get_outside","get_outside"));
  ret.push_back(pair<string,string>("remove_small_gaps", "remove_small_gaps"));
  ret.push_back(pair<string,string>("filter_so_not_too_far_inside","filter_so_not_too_far_inside"));
  ret.push_back(pair<string,string>("inverse_mask","inverse_mask(clump_regions)"));
  ret.push_back(pair<string,string>("kernel_build", "kernel_build(time, outside_mask, gaps)"));
  ret.push_back(pair<string,string>("centerpoints","centerpoints(kernels)"));
  ret.push_back(pair<string,string>("compute_attenuation", "compute_attenuation(kernels)"));
  ret.push_back(pair<string,string>("compute_humidity", "compute_humidity(kernels)"));
  ret.push_back(pair<string,string>("set_ascii_output","set_ascii_output(kernels)"));
  ret.push_back(pair<string,string>("filter_kernels", "filter_kernels(kernels)"));
  ret.push_back(pair<string,string>("organize_grids",
				    "organize_grids(pid,snoise,knoise,sdbz,kdbz,szdr,srhohv,kdbzAdjusted)"));
  ret.push_back(pair<string,string>("kernels_to_genpoly", "kernels_to_genpoly(filtered_kernels, outside)"));
  return ret;
}

//------------------------------------------------------------------
bool Sweep::synchInputsAndOutputs(const std::string &output,
				  const std::vector<std::string> &inputs)
{
  bool haveAll;
  if (!synchGriddedInputsAndOutputs(output, inputs, haveAll))
  {
    return false;
  }
  bool ret = true;

  // set pointer to output which is one of several data types
  if (!_stageOutput(output))
  {
    ret = false;
  }
  return ret;
}

//------------------------------------------------------------------
bool Sweep::processUserLoopFunction(ProcessingNode &p)
{
  string keyword;
  if (!p.isUserUnaryOp(keyword))
  {
    return false;
  }
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
  _inps.clear();
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
bool Sweep::_stageOutput(const std::string &output)
{
  // point to output
  _outputKernels = NULL;
  _outputAscii = NULL;
  // _outputSweep = NULL;

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
  
  return true;
}

//------------------------------------------------------------------
bool Sweep::_needToSynch(const std::string &userKey) const
{
  // everything has inputs
  return true;
}

//------------------------------------------------------------------
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
   //  const GriddedData *input = _derivedData.refToData(dataName);
  const GriddedData *input = _matchConst(dataName);
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
    
//------------------------------------------------------------------
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


//------------------------------------------------------------------
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

//------------------------------------------------------------------
MathUserData *Sweep::_processAssociateClumps(const std::string &reg,
					     const std::string &clump0,
					     const std::string &clump1)
{
  const GriddedData *data0 = _matchConst(clump0);
  if (data0 == NULL)
  {
    LOG(ERROR) << "No input";
    return NULL;
  }
  const GriddedData *data1 = _matchConst(clump1);
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

//------------------------------------------------------------------
MathUserData *Sweep::_processBuildGaps(const std::string &input,
				       const std::string &depth) 
{
  const GriddedData *data = _matchConst(input);
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
  const GriddedData *input = _matchConst(dataName);
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


  int min_gridpt = (int)(min/_dx);

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

  const GriddedData *clumps = _matchConst(pidClumps);
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
  // Mdvx::coord_t coord = _proj.getCoord();
  int maxP = (int)(max/_dx);
  
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
    
  pidG = _matchConst(pid);
  snoiseG = _matchConst(snoise);
  knoiseG = _matchConst(knoise);
  sdbzG = _matchConst(sdbz);
  kdbzG = _matchConst(kdbz);
  szdrG = _matchConst(szdr);
  srhohvG = _matchConst(srhohv);
  kdbzAdjustedG = _matchConst(kdbzAdjusted);
  dbzDiffG = _matchConst(dbzDiff);
  
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

			       
//------------------------------------------------------------------
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

//------------------------------------------------------------------
MathUserData *Sweep::_kernelBuild(const std::string &outsideMask,
				  const std::string &gaps,
				  const std::string &clumpReg,
				  const std::string &kgrids)
{
  const GriddedData *mask = _matchConst(outsideMask);
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
    KernelPair kp(_vlevel, mask_far, mask_near, *mask, g, _parms, _proj);
    if (kp.isBigEnough())
    {
      int id = ret->nextAvailableId();
      kp.finishProcessing(_time, _vlevel, *kg, _parms, _dx, id, id+1);
      ret->append(kp);
      ret->incrementNextAvailableId(2);
    }
  }
  return (MathUserData *)ret;
}

//------------------------------------------------------------------
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

  k->computeAttenuation(_dx, *_outputSweep);
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
  a.totalAttenuation(*clumps, *DWR, 2.0, _dx);
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
  int minPt = (int)(2.0/_dx);
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

  k->computeHumidity(_dx, *_outputSweep);
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
   const GriddedData *inp = _matchConst(name);
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

  string s = k->asciiOutput(_vlevel, _proj);
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
  mask = Grid2d((*_grid2d)[0]);  
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
