/**
 * @file Parms.cc
 */

//------------------------------------------------------------------
#include "Parms.hh"
#include "Colide.hh"
#include <toolsa/LogStream.hh>
#include <vector>

//------------------------------------------------------------------
Parms::Parms() : FiltAlgParms(), Params()
{
}

//------------------------------------------------------------------
Parms::Parms(const std::string &parmFileName,
	     bool expandEnv) :
  FiltAlgParms(parmFileName, expandEnv), Params()
{
  TDRP_warn_if_extra_params(FALSE);
  char **o = NULL;
  int expand = 0;
  if (expandEnv)
  {
    expand = 1;
  }
    
  if (Params::load(parmFileName.c_str(), o, expand, 0))
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
  for (int i=0; i<line_detect_side_fuzzy_n; ++i)
  {
    x.push_back(_line_detect_side_fuzzy[i].x);
    y.push_back(_line_detect_side_fuzzy[i].y);
  }
  _lineDetectSide = FuzzyF(x, y);
  x.clear();
  y.clear();

  for (int i=0; i<line_detect_center_fuzzy_n; ++i)
  {
    x.push_back(_line_detect_center_fuzzy[i].x);
    y.push_back(_line_detect_center_fuzzy[i].y);
  }
  _lineDetectCenter = FuzzyF(x, y);
  x.clear();
  y.clear();
  
  for (int i=0; i<line_detect_std_fuzzy_n; ++i)
  {
    x.push_back(_line_detect_std_fuzzy[i].x);
    y.push_back(_line_detect_std_fuzzy[i].y);
  }
  _lineDetectStd = FuzzyF(x, y);

  x.clear();
  y.clear();
  for (int i=0; i<shear_detect_side_fuzzy_n; ++i)
  {
    x.push_back(_shear_detect_side_fuzzy[i].x);
    y.push_back(_shear_detect_side_fuzzy[i].y);
  }
  _shearDetectSide = FuzzyF(x, y);

  x.clear();
  y.clear();
  for (int i=0; i<ellip_conf_fuzzy_n; ++i)
  {
    x.push_back(_ellip_conf_fuzzy[i].x);
    y.push_back(_ellip_conf_fuzzy[i].y);
  }
  _ellipConf = FuzzyF(x, y);

  
  x.clear();
  y.clear();
  for (int i=0; i<enhance_fuzzy_n; ++i)
  {
    x.push_back(_enhance_fuzzy[i].x);
    y.push_back(_enhance_fuzzy[i].y);
  }
  _enhanceFuzzy = FuzzyF(x, y);

  for (int i=0; i<old_data_n; ++i)
  {
    _oldData.push_back(pair<string,int>(_old_data[i].fieldName,
					_old_data[i].maxSecondsBack));
  }
}

//------------------------------------------------------------------
Parms::~Parms()
{
}

//------------------------------------------------------------------
void Parms::printOperators(void) const
{
  Colide alg;
  alg.getAlgorithm()->printOperators();
}

//------------------------------------------------------------------
void Parms::printHelp(void)
{
  FiltAlgParms::printHelp();
}

//------------------------------------------------------------------
void Parms::printParams(tdrp_print_mode_t printMode)
{
  Params::print(stdout, printMode);
  FiltAlgParms::printParams(printMode);
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
