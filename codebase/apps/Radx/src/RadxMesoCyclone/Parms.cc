/**
 * @file Parms.cc
 */

//------------------------------------------------------------------
#include "Parms.hh"
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
  
  vector<double> x, y;
   for (int i=0; i<detect_side_fuzzy_n; ++i)
   {
     x.push_back(_detect_side_fuzzy[i].x);
     y.push_back(_detect_side_fuzzy[i].y);
   }
   _detectSide = FuzzyF(x, y);
   x.clear();
   y.clear();
   for (int i=0; i<nyquist_fuzzy_n; ++i)
   {
     x.push_back(_nyquist_fuzzy[i].x);
     y.push_back(_nyquist_fuzzy[i].y);
   }
   _nyquistFuzzy = FuzzyF(x,y);

   x.clear();
   y.clear();
   for (int i=0; i<radial_fuzzy_n; ++i)
   {
     x.push_back(_radial_fuzzy[i].x);
     y.push_back(_radial_fuzzy[i].y);
   }
   _radialFuzzy = FuzzyF(x,y);
}

//------------------------------------------------------------------
Parms::~Parms()
{
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
  Params::print(stdout, printMode);
  FiltAlgParms::printParams(printMode);
}

//------------------------------------------------------------------
void Parms::printHelp(void)
{
  FiltAlgParms::printHelp();
}

//------------------------------------------------------------------
void Parms::printInputOutputs(void) const
{
  printf("INPUTS:\n");
  for (int i=0; i<input_n; ++i)
  {
    printf("\t%s\n", _input[i]);
  }

  printf("OUTPUTS:\n");
  for (int i=0; i<output_n; ++i)
  {
    printf("\t%s\n", _output[i]);
  }
}

//-----------------------------------------------------------------------
void Parms::printOperators(void) const
{
  RadxMesoCyclone alg;
  alg.getAlgorithm()->printOperators();
}

