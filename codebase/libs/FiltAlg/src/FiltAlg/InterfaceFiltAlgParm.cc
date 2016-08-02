// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/**
 * @file InterfaceFiltAlgParm.cc
 */

//------------------------------------------------------------------
#include <FiltAlg/InterfaceFiltAlgParm.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/pmu.h>
#include <toolsa/port.h>
#include <dsdata/DsUrlTrigger.hh>
#include <toolsa/LogMsgStreamInit.hh>
#include <toolsa/LogStream.hh>

typedef enum {PARM_PRINT=0, PARM_LOAD=1, PARM_PRINT_AND_LOAD=2} FiltAlg_print_t;
static string _app_name;
static FiltAlg_print_t _param_type;
static string _param_path;
static bool _param_print;
static bool _param_load;
static bool _param_archive = false;
static time_t _param_archive_t0;
static time_t _param_archive_t1;
static bool _print_triggering=FALSE;

static DsUrlTrigger *_T = NULL;

//------------------------------------------------------------------
static FiltAlg_print_t _set_tdrp_flag(int argc, char **argv)
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
InterfaceFiltAlgParm::InterfaceFiltAlgParm(void)
{
}

//------------------------------------------------------------------
InterfaceFiltAlgParm::~InterfaceFiltAlgParm(void)
{
}

//------------------------------------------------------------------
bool InterfaceFiltAlgParm::driver_init(const string &app_name,
				       const FiltAlgParams &p, 
				       void cleanup(int))
{
  PMU_auto_init(app_name.c_str(), p.instance, 60);
  PORTsignal(SIGQUIT, cleanup);
  PORTsignal(SIGTERM, cleanup);
  PORTsignal(SIGINT, cleanup);
  PORTsignal(SIGPIPE, (PORTsigfunc)SIG_IGN);

  // set up debugging state for logging
  LogMsgStreamInit::init(p.debug_mode == FiltAlgParams::DEBUG ||
			 p.debug_mode == FiltAlgParams::DEBUG_VERBOSE,
			 p.debug_mode == FiltAlgParams::DEBUG_VERBOSE,
			 true, true);
	      
  _print_triggering = p.debug_triggering;
  if (_print_triggering)
  {
    LogMsgStreamInit::setThreading(true);
  }

  LOG(DEBUG) << "setup";
  return true;
}

//------------------------------------------------------------------
void InterfaceFiltAlgParm::finish(void)
{
  PMU_auto_unregister();
  if (_T != NULL)
  {
    delete _T;
    _T = NULL;
  }
}

//------------------------------------------------------------------
bool InterfaceFiltAlgParm::driver_trigger(const FiltAlgParams &p,
					  time_t &t)
{
  LOG(DEBUG_VERBOSE) << "------before trigger-----";
  bool stat = true;

  if (_T == NULL)
  {
    if (_param_archive)
    {
      _T = new DsUrlTrigger(_param_archive_t0, _param_archive_t1,
			    p.trigger_url, DsUrlTrigger::OBS,
			    _print_triggering);
    }
    else
    {
      _T = new DsUrlTrigger(p.trigger_url, DsUrlTrigger::OBS,
			    _print_triggering);
    }
  }

  stat = _T->nextTime(t);
  if (stat)
  {
    LOG(DEBUG) << "-------Triggered " << DateTime::strn(t) << " ----------";
  }
  else
  {
    LOG(DEBUG) << "no more triggering";
    delete _T;
    _T = NULL;
  }
  return stat;
}

//------------------------------------------------------------------
bool InterfaceFiltAlgParm::load_params(FiltAlgParams &p)
{
  if (_param_type == PARM_PRINT)
  {
    p.print(stdout, PRINT_VERBOSE);
    return true;
  }
  if (p.load(_param_path.c_str(), NULL, !_param_print, false) != 0)
  {
    LOG(ERROR) << "loading file " << _param_path;
    return false;
  }

  if (_param_type == PARM_PRINT_AND_LOAD)
  {
    p.print(stdout, PRINT_VERBOSE);
    return true;
  }
  if (!p.checkAllSet(stdout))
  {
    LOG(ERROR) << "file " << _param_path 
	       << " has missing parms. All params must be set";
    return false;
  }

  return true;
}

//------------------------------------------------------------------
bool InterfaceFiltAlgParm::parm_init(int argc, char **argv)
{

  // turn off warning because param file has multiple uses.
  TDRP_warn_if_extra_params(FALSE);

  _app_name = argv[0];
  _param_type = _set_tdrp_flag(argc, argv);
  _param_path = "";
  _param_print = false;
  _param_load = false;
  _param_archive = get_archive_cmdarg_range(argc, argv,_param_archive_t0,
					    _param_archive_t1);
  for (int i=0; i<argc; ++i)
  {
    if (strcmp(argv[i], "-print_params") == 0)
    {
      _param_print = true;
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
	_param_path = argv[i+1];
	_param_load = true;
      }
    }
  }
  return true;
}

//------------------------------------------------------------------
void InterfaceFiltAlgParm::parm_finish(void)
{
  // turn back on the warnings
  TDRP_warn_if_extra_params(TRUE);

  // if print_params is in there, exit now.
  if (_param_print)
  {
    exit(0);
  }
}

//------------------------------------------------------------------
const char *InterfaceFiltAlgParm::parm_path(void)
{
  return _param_path.c_str();
}

//------------------------------------------------------------------
bool InterfaceFiltAlgParm::is_parm_load(void)
{
  return _param_load;
}

//------------------------------------------------------------------
bool InterfaceFiltAlgParm::is_parm_print(void)
{
  return _param_print;
}

//------------------------------------------------------------------
bool InterfaceFiltAlgParm::create_example_params(const string &fname)
{
  FiltAlgParams P;
  FILE *fp = fopen(fname.c_str(), "w");
  if (fp == NULL)
  {
    LOG(ERROR) << "opening file " << fname;
    return false;
  }
  P.print(fp, PRINT_LONG);
  fclose(fp);
  return true;
}

//------------------------------------------------------------------
bool InterfaceFiltAlgParm::get_archive_cmdarg_range(int argc, char **argv,
						    time_t &t0, time_t &t1)
{
  bool archive, error;
  if (DsUrlTrigger::checkArgs(argc, argv, t0, t1, archive, error))
  {
    return archive;
  }
  else
  {
    return false;
  }
}

//------------------------------------------------------------------
time_t InterfaceFiltAlgParm::most_recent_time(void)
{
  if (_param_archive)
  {
    return _param_archive_t1;
  }
  else
  {
    return time(0);
  }
}

//------------------------------------------------------------------
time_t InterfaceFiltAlgParm::oldest_time(void)
{
  if (_param_archive)
  {
    return _param_archive_t0;
  }
  else
  {
    return time(0);
  }
}

//------------------------------------------------------------------
void InterfaceFiltAlgParm::set_triggering_print(const bool stat)
{
  _print_triggering = stat;
}

//------------------------------------------------------------------
bool InterfaceFiltAlgParm::param_type_is_print(void)
{
  return _param_type == PARM_PRINT;
}

//------------------------------------------------------------------
bool InterfaceFiltAlgParm::param_type_is_print_and_load(void)
{
  return _param_type == PARM_PRINT_AND_LOAD;
}
