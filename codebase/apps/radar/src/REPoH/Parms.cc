/**
 * @file Parms.cc
 */

//------------------------------------------------------------------
#include "Parms.hh"
#include "HeaderParams.hh"
#include "Repoh.hh"
#include <toolsa/LogStream.hh>
#include <vector>

//------------------------------------------------------------------
Parms::Parms() : FiltAlgParms(), RepohParams()
{
}

//------------------------------------------------------------------
Parms::Parms(const std::string &parmFileName,  bool expandEnv) :
  FiltAlgParms(parmFileName, expandEnv), RepohParams()
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
  if (RepohParams::load(parmFileName.c_str(), o, env, 0))
  {
    LOG(ERROR) << "Loading app params";
    exit(1);
  }
  TDRP_warn_if_extra_params(TRUE);
  
  if (!FiltAlgParms::isOk())
  {
    LOG(ERROR) << "Loading filtAlg params";
    exit(1);
  }    
}

//------------------------------------------------------------------
Parms::~Parms()
{
}

//------------------------------------------------------------------
void Parms::printOperators(void) const
{
  Repoh alg;
  alg.getAlgorithm()->printOperators();
}

//------------------------------------------------------------------
void Parms::printHelp(void)
{
  FiltAlgParms::printHelp();
}

//------------------------------------------------------------------
void Parms::printParams(tdrp_print_mode_t mode)
{
  HeaderParams h;
  h.print(stdout, mode);
  FiltAlgParms::printParams(mode);
  RepohParams::print(stdout, mode);
}

//------------------------------------------------------------------
void Parms::setFiltersFromParms(void) 
{
  _fixedConstants.clear();
  _userData.clear();
  for (int i=0; i<user_data_n; ++i)
  {
    _userData.push_back(_user_data[i]);
  }
  _volumeBeforeFilters.clear();
  _sweepFilters.clear();
  for (int i=0; i<filter_n; ++i)
  {
    _sweepFilters.push_back(_filter[i]);
  }
  _volumeAfterFilters.clear();
}
