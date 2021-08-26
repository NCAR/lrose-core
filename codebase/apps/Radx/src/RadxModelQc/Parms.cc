/**
 * @file Parms.cc
 */

//------------------------------------------------------------------
#include "Parms.hh"
#include "HeaderParams.hh"
#include "RadxModelQc.hh"
#include <toolsa/LogStream.hh>
#include <vector>

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
  RadxModelQc alg;
  alg.printOperators();
}

//------------------------------------------------------------------
void Parms::setFiltersFromParms(void)
{
  _fixedConstants.clear();
  for (int i=0; i<fixed_const_n; ++i)
  {
    _fixedConstants.push_back(_fixed_const[i]);
  }
  
  _userData.clear();
  for (int i=0; i<user_data_n; ++i)
  {
    _userData.push_back(_user_data[i]);
  }
  
  _volumeBeforeFilters.clear();
  for (int i=0; i<volume_before_filter_n; ++i)
  {
    _volumeBeforeFilters.push_back(_volume_before_filter[i]);
  }
  
  _sweepFilters.clear();
  for (int i=0; i<sweep_filter_n; ++i)
  {
    _sweepFilters.push_back(_sweep_filter[i]);
  }
  
  _rayFilters.clear();
  for (int i=0; i<ray_filter_n; ++i)
  {
    _rayFilters.push_back(_ray_filter[i]);
  }
  
  _volumeAfterFilters.clear();
  // for (int i=0; i<volume_after_filter_n; ++i)
  // {
  //   _volumeAfterFilters.push_back(_volume_after_filter[i]);
  // }
}
