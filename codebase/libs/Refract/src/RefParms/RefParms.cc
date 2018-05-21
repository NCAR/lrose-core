/**
 * @file RefParms.cc
 */
#include <Refract/RefParms.hh>
#include <Refract/RefractInput.hh>
#include <dsdata/DsLdataTrigger.hh>
#include <dsdata/DsTimeListTrigger.hh>
#include <dsdata/DsUrlTrigger.hh>
#include <toolsa/LogStream.hh>
#include <toolsa/pmu.h>

// static vars
typedef enum { PARM_PRINT=0, PARM_LOAD=1, PARM_PRINT_AND_LOAD=2} Print_t;
static std::string sAppName;
static Print_t sParamType;
static std::string sParamPath;
static bool sParamPrint;
static bool sParamLoad;
static bool sParamArchive = false;
static time_t sParamArchiveT0;
static time_t sParamArchiveT1;

//------------------------------------------------------------------
static Print_t sSetTdrpFlag(int argc, char **argv)
{
  bool load=false, print=false;

  for (int i=0; i<argc; ++i)
  {
    if (strcmp(argv[i], "-params") == 0)
    {
      load = true;
    }
    else if (strcmp(argv[i], "-print_params") == 0)
    {
      print = true;
    }
  }

  if (load && print)
  {
    return PARM_PRINT_AND_LOAD;
  }
  else if (load && !print)
  {
    return PARM_LOAD;
  }
  else if ((!load) && print)
  {
    return PARM_PRINT;
  }
  else
  {
    return PARM_LOAD;
  }
}


//------------------------------------------------------------------
RefParms::RefParms() : RefParams(), _ok(false)
{
}

//------------------------------------------------------------------
RefParms::RefParms(const RefParams &P) : 
  RefParams(P), _ok(true)
{
}

//------------------------------------------------------------------
RefParms::RefParms(const std::string &fname)
{
  if (RefParams::load(fname.c_str(), NULL, true, false))
  {
    LOG(ERROR) << "Loading params from " << fname;
    _ok = false;
  }
  else
  {
    _ok = true;
  }
}

//------------------------------------------------------------------
RefParms::~RefParms()
{
}

//------------------------------------------------------------------
bool RefParms::parmInit(int argc, char **argv)
{

  // turn off warning because parameter file has multiple uses.
  TDRP_warn_if_extra_params(FALSE);

  sAppName = argv[0];
  sParamType = sSetTdrpFlag(argc, argv);
  sParamPath = "";
  sParamPrint = false;
  sParamLoad = false;
  bool error;
  DsUrlTrigger::checkArgs(argc, argv, sParamArchiveT0,
			  sParamArchiveT1, sParamArchive, error);
  if (error)
  {
    LOG(ERROR) << "Checking args";
    return false;
  }
  for (int i=0; i<argc; ++i)
  {
    if (strcmp(argv[i], "-print_params") == 0)
    {
      sParamPrint = true;
    }
    else if (strcmp(argv[i], "-params") == 0)
    {
      if (i >= argc-1)
      {
	LOG(ERROR) << "-params was last arg";
	return false;
      }
      else
      {
	sParamPath = argv[i+1];
	sParamLoad = true;
      }
    }
  }
  return true;
}

//------------------------------------------------------------------
void RefParms::parmFinish(const RefParams &p)
{
  // turn back on the warnings
  TDRP_warn_if_extra_params(TRUE);

  // if printParams is in there, exit now.
  if (sParamPrint)
  {
    exit(0);
  }

  // init the logging 
  bool realtime = true;
  LOG_STREAM_INIT(p.debug, p.verbose, realtime,
		  p.debug_show_source_code_method_and_line);
}

//------------------------------------------------------------------
bool RefParms::isParmPrint(void)
{
  return sParamPrint;
}

//------------------------------------------------------------------
bool RefParms::isParmLoad(void)
{
  return sParamLoad;
}

//------------------------------------------------------------------
string RefParms::parmPath(void)
{
  return sParamPath;
}

//------------------------------------------------------------------
bool RefParms::isPrintMode(void)
{
  return sParamType == PARM_PRINT;
}

//------------------------------------------------------------------
bool RefParms::isPrintAndLoadMode(void)
{
  return sParamType == PARM_PRINT_AND_LOAD;
}


//------------------------------------------------------------------
bool RefParms::initTrigger(DsTrigger **trigger) const
{
  switch (trigger_mode)
  {
  case RefParams::TIME_LIST :
  {
    if (!sParamArchive)
    {
      LOG(ERROR) << "triggering mode is TIME_LIST, but not times on cmd line";
      LOG(ERROR) << "Expect one of: ";
      LOG(ERROR) << "  -interval yyyymmddhhmmss yyyymmddhhmmss";
      LOG(ERROR) << "  -start \"yyyy mm dd hh mm ss\" "
	"-end \"yyyy mm dd hh mm ss\"";
      return false;
    }
    DateTime start_time = sParamArchiveT0;
    DateTime end_time = sParamArchiveT1;
    LOG(DEBUG) << "Initializing TIME_LIST trigger: ";
    LOG(DEBUG) << "   URL: " << input_url;
    LOG(DEBUG) << "   start time: " << start_time;
    LOG(DEBUG) << "   end time: " << end_time; 
    
    DsTimeListTrigger *ltrigger = new DsTimeListTrigger();
    if (ltrigger->init(input_url, sParamArchiveT0, sParamArchiveT1) != 0)
    {
      LOG(ERROR) << "Initializing TIME_LIST trigger:";
      LOG(ERROR) << "   URL: " << input_url;
      LOG(ERROR) << "   start time: " << start_time;
      LOG(ERROR) << "   end time: " << end_time; 
      LOG(ERROR) << ltrigger->getErrStr();
      delete ltrigger;
      return false;
    }
    *trigger = ltrigger;
    break;
  }
  case RefParams::LATEST_DATA :
  {
    LOG(DEBUG) << "Initializing LATEST_DATA trigger: URL = " << input_url;
    
    DsLdataTrigger *ltrigger = new DsLdataTrigger();
    if (ltrigger->init(input_url, max_valid_secs, PMU_auto_register) != 0)
    {
      LOG(ERROR) << "initializing LATEST_DATA trigger, URL = "
		 << input_url;
      LOG(ERROR) << ltrigger->getErrStr();
      delete ltrigger;
      return false;
    }
    *trigger = ltrigger;
    break;
  }  
  } /* endswitch - _refparms.trigger_mode */

  return true;
}

/*********************************************************************
 * _initInput
 */

RefractInput *RefParms::initInput(void) const
{
  if (!specify_elevation_by_index)
  {
    return new RefractInput(raw_iq_in_input,
			    raw_i_field_name,
			    raw_q_field_name,
			    niq_field_name,
			    aiq_field_name,
			    quality_source == QUALITY_FROM_WIDTH,
			    quality_field_name,
			    snr_in_input,
			    snr_field_name,
			    power_field_name,
			    input_niq_scale,
			    invert_target_angle_sign,
			    elevation_angle.min_angle,
			    elevation_angle.max_angle,
			    num_azim,
			    num_range_bins,
			    _debug_latlon[0],
			    _debug_latlon[1],
			    debug_npt);
  }
  else
  {
    return new RefractInput(raw_iq_in_input,
			    raw_i_field_name,
			    raw_q_field_name,
			    niq_field_name,
			    aiq_field_name,
			    quality_source == QUALITY_FROM_WIDTH,
			    quality_field_name,
			    snr_in_input,
			    snr_field_name,
			    power_field_name,
			    input_niq_scale,
			    invert_target_angle_sign,
			    elevation_num,
			    num_azim,
			    num_range_bins,
			    _debug_latlon[0],
			    _debug_latlon[1],
			    debug_npt);
  }
}

