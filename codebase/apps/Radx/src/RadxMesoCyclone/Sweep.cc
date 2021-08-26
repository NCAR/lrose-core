/**
 * @file Sweep.cc
 */

//------------------------------------------------------------------
#include "Sweep.hh"
#include "Volume.hh"
#include "MesoTemplate.hh"
#include <FiltAlgVirtVol/PolarCircularFilter.hh>
#include <FiltAlgVirtVol/PolarCircularTemplate.hh>
#include <FiltAlgVirtVol/VirtVolFuzzy.hh>
#include <euclid/GridAlgs.hh>
#include <euclid/Grid2dClump.hh>
#include <rapmath/MathParser.hh>
#include <rapmath/ProcessingNode.hh>
#include <rapmath/UnaryNode.hh>
#include <rapmath/FuzzyF.hh>
#include <toolsa/LogStream.hh>
#include <cmath>

const std::string Sweep::_mesoTemplateStr = "MesoTemplate";
  
//------------------------------------------------------------------
Sweep::Sweep(void) : VirtVolSweep()
{
}
//------------------------------------------------------------------
Sweep::Sweep(const Volume &volume, int index, double vlevel) :
  VirtVolSweep(volume, index, vlevel)
{
  _parms = volume._parms;
  _inpSpecial = volume._special;
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
  ret.push_back(FunctionDef(_mesoTemplateStr, "M", "VEL,template, minPctGood, minDiff, minPctLarge,fuzzyF",
			    "A complex app filter.  At each point use the template to get two sets of data, one on each side of the point. If less than minPctGood points have valid data, set output to missing, otherwise ave1 = average of data on one side, ave2 = average of data on the other side. "
			    "if |ave1-ave2| > minDiff, and if percent of data from one side that is greater than ave1 and percent of data from the other side that is greater than ave2 are both greater than minPctLarge, then set the output to the result of applying the fuzzy function to |ave1-ave2|. Otherwise set the output to missing"));
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
bool Sweep::_processMesoTemplate(std::vector<ProcessingNode *> &args)
{
  const MathLoopData *data;
  MathUserData *udata1, *udata2;
  if (args.size() != 6)
  {
    LOG(ERROR) << "Need 6 inputs";
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
  udata1 = userDataPtr(dataName);
  if (udata1 == NULL)
  {
    LOG(ERROR) << "No data for" << dataName;
    return false;
  }


  double minPctGood, minDiff, minPctLarge;
  if (!args[2]->getValue(minPctGood))
    {
      LOG(ERROR) << "No value in arg position 2";
      return false;
    }

  if (!args[3]->getValue(minDiff))
    {
      LOG(ERROR) << "No value in arg position 3";
      return false;
    }

  if (!args[4]->getValue(minPctLarge))
    {
      LOG(ERROR) << "No value in arg position 4";
      return false;
    }


  dataName = args[5]->leafName();
  if (dataName.empty())
  {
    LOG(ERROR) << " NO name in arg 5";
    return false;
  }
  udata2 = userDataPtr(dataName);
  if (udata2 == NULL)
  {
    LOG(ERROR) << "No data for" << dataName;
    return false;
  }

  const GriddedData *input = (const GriddedData *)data;
  Grid2d g(*input);
  g.changeMissing(-99);
  g.setAllMissing();

  TemplateLookupMgr *t = (TemplateLookupMgr *)udata1;
  VirtVolFuzzy *ff = (VirtVolFuzzy *)udata2;

  MesoTemplate m(t, minPctGood, minDiff, minPctLarge, ff);
  m.apply(*input, *this, g);
  
  _outputSweep->dataCopy(g);
  return true;
    
}

//------------------------------------------------------------------
bool Sweep::_needToSynch(const std::string &userKey) const
{
  return true;
}

