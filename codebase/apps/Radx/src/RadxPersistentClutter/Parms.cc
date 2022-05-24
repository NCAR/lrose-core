/**
 * @file Parms.cc
 */

//------------------------------------------------------------------
#include "Parms.hh"
#include "HeaderParams.hh"
#include "Alg.hh"
#include <toolsa/LogStream.hh>
#include <vector>

const std::string Parms::_volInitStr = "VolInit";
const std::string Parms::_volFinishStr = "VolFinish";
const std::string Parms::_histoAccumStr = "AccumulateHisto";
const std::string Parms::_initStatus = "statusInit";
const std::string Parms::_finishStatus = "statusFinish";

static std::string _noArgsFilter(const std::string &lhs, const std::string &func)
{
  string ret = lhs;
  ret = ret + " = ";
  ret = ret + func;
  ret = ret + "()";
  return ret;
}

static std::string _oneArgFilter(const std::string &lhs, const std::string &func, const std::string &arg)
{
  string ret = lhs;
  ret = ret + " = ";
  ret = ret + func;
  ret = ret + "(";
  ret = ret + arg;
  ret = ret + ")";
  return ret;
}

//------------------------------------------------------------------
Parms::Parms() : RadxAppParms(), Params()
{
}

//------------------------------------------------------------------
Parms::Parms(const std::string &parmFileName,  bool expandEnv) :
  RadxAppParms(parmFileName, expandEnv), Params()
{
  TDRP_warn_if_extra_params(FALSE);
  char **o = NULL;
  int env;
  if (expandEnv)
  {
    env = 1;
  }
  else
  {
    env = 0;
  }
  if (Params::load(parmFileName.c_str(), o, env, 0))
  {
    LOG(ERROR) << "Loading app params";
    exit(1);
  }
  TDRP_warn_if_extra_params(TRUE);
  
  if (!RadxAppParms::isOk())
  {
    LOG(ERROR) << "Loading RadxApp params";
    exit(1);
  }    
}

//------------------------------------------------------------------
Parms::Parms(const std::string &parmFileName,
	     const std::vector<std::string> &files,
	     bool expandEnv) :
  RadxAppParms(parmFileName, files, expandEnv), Params()
{
  TDRP_warn_if_extra_params(FALSE);
  char **o = NULL;
  int env;
  if (expandEnv)
  {
    env = 1;
  }
  else
  {
    env = 0;
  }
  if (Params::load(parmFileName.c_str(), o, env, 0))
  {
    LOG(ERROR) << "Loading app params";
    exit(1);
  }
  TDRP_warn_if_extra_params(TRUE);
  
  if (!RadxAppParms::isOk())
  {
    LOG(ERROR) << "Loading RadxApp params";
    exit(1);
  }    
}

//------------------------------------------------------------------
Parms::~Parms()
{
}

//------------------------------------------------------------------
void Parms::printParams(tdrp_print_mode_t printMode)
{

  HeaderParams h;
  h.print(stdout, printMode);
  RadxAppParms::printParams(printMode);
  Params::print(stdout, printMode);
}

//------------------------------------------------------------------
void Parms::printHelp(void)
{
  RadxAppParms::printHelp();
}

//------------------------------------------------------------------
void Parms::printOperators(void) const
{
  Alg alg;
  alg.printOperators();
}

//------------------------------------------------------------------
void Parms::setFiltersFromParms(void)
{
  _fixedConstants.clear();
  _userData.clear();
  _volumeBeforeFilters.clear();
  _sweepFilters.clear();
  _rayFilters.clear();
  _volumeAfterFilters.clear();

  _userData.push_back(_initStatus);
  _userData.push_back(_finishStatus);
  string appVolumeBeforeFilter = _noArgsFilter(_initStatus, _volInitStr);
  string appVolumeAfterFilter = _noArgsFilter(_finishStatus, _volFinishStr);
  _volumeBeforeFilters.push_back(appVolumeBeforeFilter);
  _volumeAfterFilters.push_back(appVolumeAfterFilter);
  string rayFilter = _oneArgFilter("HistoData", _histoAccumStr, input_field);
  _rayFilters.push_back(rayFilter);
  LOG(DEBUG) << "Volume before:    " << appVolumeBeforeFilter;
  LOG(DEBUG) << "Ray filter   :    " << rayFilter;
  LOG(DEBUG) << "Volume after:     " << appVolumeAfterFilter;
}

