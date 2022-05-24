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
 * @file DsUrlTrigger.cc
 */
#include <dsdata/DsUrlTrigger.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/LogStream.hh>
#include <toolsa/LogMsgStreamInit.hh>
#include <cstring>
#include <cstdlib>
#include <cstdio>

using std::string;

//----------------------------------------------------------------
DsUrlTrigger::DsUrlTrigger()
{
  LogMsgStreamInit::setTrigger(false);
  _mode = OBS;
  _trigger = NULL;
}

//----------------------------------------------------------------
DsUrlTrigger::DsUrlTrigger(const string &trigger_url, const Trigger_t mode,
			   const bool debug)
{
  LogMsgStreamInit::setTrigger(debug);
  _mode = mode;
  _initRealTime(trigger_url);
}

//----------------------------------------------------------------
DsUrlTrigger::DsUrlTrigger(const time_t &t0, const time_t &t1,
			   const string &trigger_url, const Trigger_t mode,
			   const bool debug, const bool isSpdb)
{
  LogMsgStreamInit::setTrigger(debug);
  _mode = mode;
  _initArchive(trigger_url, t0, t1, isSpdb);

}

//----------------------------------------------------------------
DsUrlTrigger::DsUrlTrigger(int argc, char **argv,
			   const std::string &trigger_url, 
			   const Trigger_t mode, const bool debug,
			   const bool isSpdb)
{
  LogMsgStreamInit::setTrigger(debug);
  _mode = mode;

  time_t t0, t1;
  bool archive=false;
  bool error = false;
  if (!checkArgs(argc, argv, t0, t1, archive, error))
  {
    if (error)
    {
      LOG(FATAL) << "Parsing command args";
    }
    exit(1);
  }

  if (archive)
  {
    _initArchive(trigger_url, t0, t1, isSpdb);
  }
  else
  {
    _initRealTime(trigger_url);
  }
}

//----------------------------------------------------------------
DsUrlTrigger::~DsUrlTrigger()
{
  if (_trigger != NULL)
  {
    delete _trigger;
  }
}

//----------------------------------------------------------------
bool DsUrlTrigger::setNowait(void)
{
  DsUrlTriggerSubsample::gentimeSubsamplingClear();

  if (_mode == FCST_GEN)
  {
    LOG(ERROR) << "can't set nowait when in FCST_GEN mode";
    return false;
  }
  return _trigger->setNowait();
}

//----------------------------------------------------------------
bool DsUrlTrigger::setMaxValidAge(const int max_valid_age)
{
  return _trigger->setMaxValidAge(max_valid_age);
}

//----------------------------------------------------------------
void DsUrlTrigger::setDebugging(const bool value)
{
  LogMsgStreamInit::setTrigger(value);
}

//----------------------------------------------------------------
bool DsUrlTrigger::nextTime(time_t &t)
{
  if (_mode == FCST_LEAD)
  {
    LOG(ERROR) << "Wrong method called in FCST_LEAD mode";
    return false;
  }
  return _trigger->nextTime(*this, t);
}

//----------------------------------------------------------------
bool DsUrlTrigger::nextTime(time_t &t, int &lt)
{
  if (_mode != FCST_LEAD)
  {
    LOG(ERROR) << "Wrong method called, want FCST_LEAD mode, got " <<
      sprintMode(_mode);
    return false;
  }
  return _trigger->nextTime(*this, t, lt);
}

//----------------------------------------------------------------
bool DsUrlTrigger::nextData(time_t &t, string &fname)
{
  return _trigger->nextData(*this, t, fname);
}

//----------------------------------------------------------------
bool DsUrlTrigger::rewind(void)
{
  return _trigger->rewind();
}

//----------------------------------------------------------------
int DsUrlTrigger::defaultMaxValidAge(void)
{
  return DsUrlTriggerObject::defaultMaxValidAge();
}

//----------------------------------------------------------------
bool DsUrlTrigger::checkArgs(int argc, char **argv, time_t &t0, time_t &t1,
			     bool &archive, bool &error)
{
  if (hasHelpArg(argc, argv))
  {
    printf("USAGE : %s [-print_params to get parameters]\n", argv[0]);
    printf("  -h or -- or -? to get this help message\n");
    printf("Archive mode:\n");
    printf("  -interval YYYYMMDDhhmmss YYYYMMDDhhmmss\n");
    printf("  -start \"yyyy mm dd hh mm ss\" -end \"yyyy mm dd hh mm ss\"\n");
    printf("  -params parmfilename\n");
    return false;
  }

  archive = false;
  error = false;
  bool t0_set=false, t1_set=false;

  int y, m, d, h, min, sec;

  for (int i=1; i<argc; )
  {
    if (!strcmp(argv[i], "-start"))
    {
      if (i + 1 >= argc)
      {
	LOG(ERROR) << "-start without subsequent time specification";
	error = true;
	return false;
      }
      if (sscanf(argv[i+1], "%4d %2d %2d %2d %2d %2d", &y, &m, &d, 
		 &h, &min, &sec) != 6)
      {
	LOG(ERROR) << "Parsing '" << argv[i+1] << "' as 'yyyy mm dd hh mm ss'";
	error = true;
	return false;
      }
      else
      {
	DateTime dt(y, m, d, h, min, sec);
	t0 = dt.utime();
	t0_set = true;
      }
      i = i+2;
    }      
    else if (!strcmp(argv[i], "-end"))
    {
      if (i + 1 >= argc)
      {
	LOG(ERROR) << "-end without subsequent time specification";
	error = true;
	return false;
      }
      if (sscanf(argv[i+1], "%4d %2d %2d %2d %2d %2d", &y, &m, &d, 
		 &h, &min, &sec) != 6)
      {
	LOG(ERROR) << "Parsing '" << argv[i+1] << "' as 'yyyy mm dd hh mm ss'";
	error = true;
	return false;
      }
      else
      {
	DateTime dt(y, m, d, h, min, sec);
	t1 = dt.utime();
	t1_set = true;
      }
      i = i+2;
    }      
    else if (!strcmp(argv[i], "-interval"))
    {
      if (i+2 >= argc)
      {
	LOG(ERROR) << "-interval without 2 subsequent times";
	error = true;
	return false;
      }
	  
      if (sscanf(argv[i+1], "%4d%2d%2d%2d%2d%2d", &y, &m, &d, 
		 &h, &min, &sec) != 6)
      {
	LOG(ERROR) << "Parsing '" << argv[i+1] << "' as 'yyyymmddhhmmss'";
	error = true;
	return false;
      }
      else
      {
	DateTime dt(y, m, d, h, min, sec);
	t0 = dt.utime();
	t0_set = true;
      }

      if (sscanf(argv[i+2], "%4d%2d%2d%2d%2d%2d", &y, &m, &d, 
		 &h, &min, &sec) != 6)
      {
	LOG(ERROR) << "Parsing '" << argv[i+2] << "' as 'yyyymmddhhmmss'";
	error = true;
	return false;
      }
      else
      {
	DateTime dt(y, m, d, h, min, sec);
	t1 = dt.utime();
	t1_set = true;
      }
      i = i+3;
    }
    else
    {
      i++;
    }
  }
  if ((t0_set && !t1_set) || (t1_set && !t0_set))
  {
    LOG(ERROR) << "Did not set both start and end time";
    error = true;
    return false;
  }
  archive = (t0_set && t1_set);
  return true;
}

//----------------------------------------------------------------
std::string DsUrlTrigger::sprintMode(Trigger_t t)
{
  string s = "";
  switch (t)
  {
  case OBS:
    s = "OBS";
    break;
  case FCST_GEN:
    s = "FCST_GEN";
    break;
  case FCST_LEAD:
    s = "FCST_LEAD";
    break;
  default:
    s = "UNKNOWN";
    break;
  }
  return s;
}

//----------------------------------------------------------------
bool DsUrlTrigger::hasHelpArg(int argc, char **argv)
{
  for (int i=1; i<argc; ++i)
  {
    if ( (!strcmp(argv[i], "-h")) ||
         (!strcmp(argv[i], "--")) ||
         (!strcmp(argv[i], "-?")) )
    {
      return true;
    }
  }
  return false;
}

//----------------------------------------------------------------
void DsUrlTrigger::_initRealTime(const std::string &trigger_url)
{
  DsUrlTriggerObs *u;
  DsUrlTriggerFcstGen *fg;
  DsUrlTriggerFcstLead *fl;
  switch (_mode)
  {
  case OBS:
    u = new DsUrlTriggerObs(trigger_url);
    _trigger = dynamic_cast<DsUrlTriggerObject *>(u);
    break;
  case FCST_GEN:
    fg = new DsUrlTriggerFcstGen(trigger_url);
    _trigger = dynamic_cast<DsUrlTriggerObject *>(fg);
    break;
  case FCST_LEAD:
    fl = new DsUrlTriggerFcstLead(trigger_url);
    _trigger = dynamic_cast<DsUrlTriggerObject *>(fl);
    break;
  default:
    LOG(FATAL) << "bad input";
    exit(-1);
  }
}

//----------------------------------------------------------------
void DsUrlTrigger::_initArchive(const std::string &trigger_url,
				const time_t &t0, const time_t &t1,
				const bool is_spdb)
{
  DsUrlTriggerObs *u;
  DsUrlTriggerFcstGen *fg;
  DsUrlTriggerFcstLead *fl;
  switch (_mode)
  {
  case OBS:
    u = new DsUrlTriggerObs(t0, t1, trigger_url, is_spdb);
    _trigger = dynamic_cast<DsUrlTriggerObject *>(u);
    break;
  case FCST_GEN:
    fg = new DsUrlTriggerFcstGen(t0, t1, trigger_url, is_spdb);
    _trigger = dynamic_cast<DsUrlTriggerObject *>(fg);
    break;
  case FCST_LEAD:
    fl = new DsUrlTriggerFcstLead(t0, t1, trigger_url, is_spdb);
    _trigger = dynamic_cast<DsUrlTriggerObject *>(fl);
    break;
  default:
    LOG(FATAL) << "bad input";
    exit(-1);
  }
  if (!_trigger)
  {
    LOG(FATAL) << "Did not create viable object";
    exit(-1);
  }
  _trigger->archiveInit(t0, t1);
}
