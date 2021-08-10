/**
 * @file Parms.cc
 */

//------------------------------------------------------------------
#include "Parms.hh"
#include "HeaderParams.hh"
#include "RadxMesoCyclone.hh"
#include <toolsa/LogStream.hh>
#include <vector>

//------------------------------------------------------------------
Parms::Parms() : FiltAlgParms(), Params()
{
}

//------------------------------------------------------------------
Parms::Parms(const std::string &parmFileName,  bool expandEnv) :
  FiltAlgParms(parmFileName, expandEnv), Params()
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
  
  if (!FiltAlgParms::isOk())
  {
    LOG(ERROR) << "Loading filtAlg params";
    exit(1);
  }    

  _output2dUrl = UrlParms(output_url_2d);
  if (!_output2dUrl.isOk())
  {
    LOG(ERROR) << "Loading url parms from " << output_url_2d;
    exit(1);
  }
}

//------------------------------------------------------------------
Parms::~Parms()
{
}

//------------------------------------------------------------------
void Parms::setFiltersFromParms(void) 
{
  _fixedConstants.clear();
  _fixedConstantNames.clear();
  for (int i=0; i<fixed_const_n; ++i)
  {
    addFixedConstant(_fixed_const[i]);
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
  for (int i=0; i<filter_n; ++i)
  {
    _sweepFilters.push_back(_filter[i]);
  }
  
  _volumeAfterFilters.clear();
  for (int i=0; i<volume_after_filter_n; ++i)
  {
    _volumeAfterFilters.push_back(_volume_after_filter[i]);
  }
}

//------------------------------------------------------------------
void Parms::printParams(tdrp_print_mode_t printMode)
{
  HeaderParams h;
  h.print(stdout, printMode);
  FiltAlgParms::printParams(printMode);
  Params::print(stdout, printMode);
}

//------------------------------------------------------------------
void Parms::printHelp(void)
{
  FiltAlgParms::printHelp();
}

//-----------------------------------------------------------------------
void Parms::printOperators(void) const
{
  RadxMesoCyclone alg;
  alg.getAlgorithm()->printOperators();
}

