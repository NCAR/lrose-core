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
 * @file DsUrlTriggerRealtime.cc
 */
#include <dsdata/DsUrlTrigger.hh>
#include <toolsa/TaTriggerLog.hh>
#include <toolsa/LogStream.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/pmu.h>
#include <dsdata/DsLdataTrigger.hh>

//----------------------------------------------------------------
DsUrlTrigger::DsUrlTriggerObject::DsUrlTriggerRealtime::DsUrlTriggerRealtime()
{
  _init();
}

//----------------------------------------------------------------
DsUrlTrigger::DsUrlTriggerObject::
DsUrlTriggerRealtime::DsUrlTriggerRealtime(const string &trigger_url)
{
  _init();
  _url = trigger_url;
  _LT = new DsLdataTrigger();
  _LT->init(trigger_url, _max_valid_age, PMU_auto_register, _delay_msec);
}

//----------------------------------------------------------------
DsUrlTrigger::DsUrlTriggerObject::
DsUrlTriggerRealtime::DsUrlTriggerRealtime(const DsUrlTriggerRealtime &u)
{
  _url = u._url;
  _max_valid_age = u._max_valid_age;
  _delay_msec = u._delay_msec;
  if (_url.empty())
  {
    _LT = NULL;
  }
  else
  {
    _LT = new DsLdataTrigger();
    _LT->init(_url, _max_valid_age, PMU_auto_register, _delay_msec);
  }
  _last_time = u._last_time;
  _last_fname = u._last_fname;
}

//----------------------------------------------------------------
DsUrlTrigger::DsUrlTriggerObject::DsUrlTriggerRealtime & 
DsUrlTrigger::DsUrlTriggerObject::
DsUrlTriggerRealtime::operator=(const DsUrlTriggerRealtime &u)
{
  if (this == &u)
  {
    return *this;
  }

  _url = u._url;
  _max_valid_age = u._max_valid_age;
  _delay_msec = u._delay_msec;
  if (_LT != NULL)
  {
    delete _LT;
  }
  if (_url.empty())
  {
    _LT = NULL;
  }
  else
  {
    _LT = new DsLdataTrigger();
    _LT->init(_url, _max_valid_age, PMU_auto_register, _delay_msec);
  }

  _last_time = u._last_time;
  _last_fname = u._last_fname;
  return *this;
}

//----------------------------------------------------------------
DsUrlTrigger::DsUrlTriggerObject::DsUrlTriggerRealtime::~DsUrlTriggerRealtime()
{
  if (_LT != NULL)
  {
    delete _LT;
  }
}

//----------------------------------------------------------------
void DsUrlTrigger::DsUrlTriggerObject::DsUrlTriggerRealtime::setNowait(void)
{
  _delay_msec = -1;
  if (_LT != NULL)
  {
    delete _LT;
  }
  _LT = new DsLdataTrigger();
  _LT->init(_url, _max_valid_age, PMU_auto_register, _delay_msec);
}

//----------------------------------------------------------------
void DsUrlTrigger::DsUrlTriggerObject::
DsUrlTriggerRealtime::setMaxValidAge(const int max_valid_age)
{
  _max_valid_age = max_valid_age;
  if (_LT != NULL)
  {
    delete _LT;
  }
  _LT = new DsLdataTrigger();
  _LT->init(_url, _max_valid_age, PMU_auto_register, _delay_msec);
}

//----------------------------------------------------------------
bool DsUrlTrigger::DsUrlTriggerObject::
DsUrlTriggerRealtime::next(const DsUrlTriggerSubsample &s, time_t &t, int &lt)
{
  DsUrlTriggerRealtime::Read_status_t stat;
  while (true)
  {
    stat = _nextTimeSequence(s, t, lt);
    if (stat == WANTED)
    {
      return true;
    }
    else if (stat == ABORT)
    {
      return false;
    }
  }
}

//----------------------------------------------------------------
void DsUrlTrigger::DsUrlTriggerObject::DsUrlTriggerRealtime::firstTrigger(void)
{
  time_t t;
  int lt;
  DsUrlTriggerSubsample s;
  if (next(s, t, lt))
  {
    LOG(DEBUG) << "skipping initial trigger " << DateTime::strn(t) << "+" << lt;
  }
  else
  {
    LOG(DEBUG) << "initial trigger failed";
  }
}

//----------------------------------------------------------------
string DsUrlTrigger::DsUrlTriggerObject::
DsUrlTriggerRealtime::currentFilename(void) const
{
  return _last_fname;
}

//----------------------------------------------------------------
int DsUrlTrigger::DsUrlTriggerObject::
DsUrlTriggerRealtime::defaultMaxValidAge(void)
{
  return 5000;
}

//----------------------------------------------------------------
void DsUrlTrigger::DsUrlTriggerObject::DsUrlTriggerRealtime::_init(void)
{
  _url = "";
  _LT = NULL;
  _last_time = DsFcstTime();
  _last_fname = "unknown";
  _delay_msec = 5000;
  _max_valid_age = defaultMaxValidAge();
}

//----------------------------------------------------------------
DsUrlTrigger::DsUrlTriggerObject::DsUrlTriggerRealtime::Read_status_t
DsUrlTrigger::DsUrlTriggerObject::
DsUrlTriggerRealtime::_nextTimeSequence(const DsUrlTriggerSubsample &s,
					time_t &t, int &lt)
{
  if (_LT->endOfData())
  {
    LOGC(TaTriggerLog::name()) << "end of data";
    return ABORT;
  }

  LOGC(TaTriggerLog::name()) << "triggering";
  if (_LT->next() != 0)
  {
    LOGC(TaTriggerLog::name()) << "call to next empty return";
    return ABORT;
  }

  // DsTrigger *T = dynamic_cast<DsTrigger *>(_LT);
  TriggerInfo i = _LT->getTriggerInfo();

  t = i.getIssueTime();
  time_t vt = i.getForecastTime();
  lt = vt-t;
  DsFcstTime ft(t, lt);
  if (s.timeOk(t, lt))
  {
    if (ft != _last_time)
    {
      _last_time = ft;
      _last_fname = i.getFilePath();
      LOGC(TaTriggerLog::name()) << _last_time.sprint();
      return WANTED;
    }
    else
    {
      LOGC(TaTriggerLog::name()) << ft.sprint() << " repeat of previous";
      return UNWANTED;
    }
  }
  else
  {
    LOGC(TaTriggerLog::name()) << ft.sprint() << " not wanted";
    return UNWANTED;
  }	  
}
