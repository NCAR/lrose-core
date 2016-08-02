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
 * @file DsEnsembleLeadTrigger.cc
 */
#include <dsdata/DsEnsembleLeadTrigger.hh>
#include <Mdv/DsMdvx.hh>
#include <toolsa/LogStream.hh>
#include <toolsa/TaTriggerLog.hh>
#include <toolsa/pmu.h>
#include <algorithm>

using std::vector;
using std::string;

//----------------------------------------------------------------
static bool _add_string(vector<string> &l, const string &s)
{
  if (find(l.begin(), l.end(), s) == l.end())
  {
    l.push_back(s);
    return true;
  }
  else
  {
    return false;
  }
}

//----------------------------------------------------------------
static bool _remove_from_string(vector<string> &l, const string &s)
{
  vector<string>::iterator j;
  j = find(l.begin(), l.end(), s);
  if (j != l.end())
  {
    l.erase(j);
    return true;
  }
  else
  {
    return false;
  }
}

//----------------------------------------------------------------
static int _matching_int(const vector<int> l, const int i)
{
  vector<int>::const_iterator j;
  j = find(l.begin(), l.end(), i);
  if (j == l.end())
  {
    return -1;
  }
  else
  {
    return j - l.begin();
  }
}

//----------------------------------------------------------------
static int _matching_string(const vector<string> l, const string &s)
{
  vector<string>::const_iterator i;
  i = find(l.begin(), l.end(), s);
  if (i == l.end())
  {
    return -1;
  }
  else
  {
    return i - l.begin();
  }
}

//----------------------------------------------------------------
DsEnsembleLeadTrigger::DsEnsembleLeadTrigger(void) : 
  DsEnsembleAnyTrigger(),
  _max_seconds_before_disable(ensembleLeadTrigger::disable_seconds),
  _max_seconds_before_timeout(ensembleLeadTrigger::timeout_seconds),
  _persistant_disable(ensembleLeadTrigger::persistant_disable),
  _gen_time(-1),
  _real_time_0(-1)
{
}

//----------------------------------------------------------------
DsEnsembleLeadTrigger::
DsEnsembleLeadTrigger(const vector<string> &url,
			const vector<int> &leadSeconds) : 
  DsEnsembleAnyTrigger(url, leadSeconds),
  _urls(url),
  _lead_times(leadSeconds),
  _max_seconds_before_disable(ensembleLeadTrigger::disable_seconds),
  _max_seconds_before_timeout(ensembleLeadTrigger::timeout_seconds),
  _persistant_disable(ensembleLeadTrigger::persistant_disable),
  _gen_time(-1),
  _real_time_0(-1)
{
  for (size_t i=0; i<leadSeconds.size(); ++i)
  {
    _lead_time_state.push_back(LeadTimeState(leadSeconds[i]));
  }

  for (size_t i=0; i<url.size(); ++i)
  {
    _last_trigger_time[url[i]] = -1;
  }
}

//----------------------------------------------------------------
DsEnsembleLeadTrigger::DsEnsembleLeadTrigger(const time_t &t0,
					     const time_t &t1,
					     const vector<string> &url,
					     const vector<int> &leadSeconds) :
  DsEnsembleAnyTrigger(t0, t1, url, leadSeconds),
  _urls(url),
  _lead_times(leadSeconds),
  _max_seconds_before_disable(ensembleLeadTrigger::disable_seconds),
  _max_seconds_before_timeout(ensembleLeadTrigger::timeout_seconds),
  _persistant_disable(ensembleLeadTrigger::persistant_disable),
  _gen_time(-1),
  _real_time_0(-1)
{
}

//----------------------------------------------------------------
DsEnsembleLeadTrigger::~DsEnsembleLeadTrigger(void)
{
}

//------------------------------------------------------------------
void
DsEnsembleLeadTrigger::setMaxSecondsBeforeTimeout(const int maxSeconds)
{
  _max_seconds_before_timeout = maxSeconds;
}
  
//------------------------------------------------------------------
void
DsEnsembleLeadTrigger::setMaxSecondsBeforeDisable(const int maxSeconds)
{
  _max_seconds_before_disable = maxSeconds;
}
  
//------------------------------------------------------------------
void
DsEnsembleLeadTrigger::setPersistantDisable(const bool status)
{
  _persistant_disable = status;
}
  
//----------------------------------------------------------------
bool DsEnsembleLeadTrigger::nextTrigger(time_t &t, int &lt,
					std::vector<std::string> &url,
					bool &complete)
{
  if (DsEnsembleAnyTrigger::isArchiveMode())
  {
    return DsEnsembleAnyTrigger::archiveNextGenLeadTime(t, lt, url, complete);
  }
  else
  {
    return _realtime_next(t, lt, url, complete);
  }
}

//----------------------------------------------------------------
bool DsEnsembleLeadTrigger::_realtime_next(time_t &t, int &lt,
					   std::vector<std::string> &url,
					   bool &complete)
{
  if (!_triggered_que.empty())
  {
    if (_nextQuedTrigger(t, lt, url, complete))
    {
      return true;
    }
  }

  time_t ti;
  int lti;
  std::string urli;
  bool hasData;
  while (DsEnsembleAnyTrigger::nextTime(ti, lti, urli, hasData))
  {
    PMU_auto_register("Triggered");
    _process(ti, lti, urli, hasData);
    _add_to_que();
    if (!_triggered_que.empty())
    {
      return _nextQuedTrigger(t, lt, url, complete);
    }
  }
  return false;
}

//----------------------------------------------------------------
bool DsEnsembleLeadTrigger::_nextQuedTrigger(time_t &t, int &lt,
					     std::vector<std::string> &url,
					     bool &complete)
{
  if (_triggered_que.empty())
  {
    return false;
  }

  Trigger_t ti = _triggered_que.front();
  _triggered_que.pop_front();

  t = ti.t;
  lt = ti.lt;
  url = ti.url;
  complete = ti.complete;
  return true;
}

//----------------------------------------------------------------
void DsEnsembleLeadTrigger::_process(const time_t &t, const int lt,
					  const std::string &url,
					  const bool &hasData)
{
  if (hasData)
  {
    _process_data(t, lt, url);
  }
  else
  {
    _process_timeout(url);
  }
}

//----------------------------------------------------------------
void DsEnsembleLeadTrigger::_process_data(const time_t &t, const int lt,
					  const std::string &url)
{
  LOGC(TaTriggerLog::name()) << "Triggered " << DateTime::strn(t) 
			     << "+" << lt << " for " << url;

  int ind = _matching_int(_lead_times, lt);
  if (ind < 0)
  {
    LOGC(TaTriggerLog::name()) << "Ignore unwanted lead time";
    return;
  }

  if (t != _gen_time)
  {
    _start_new_gen_time(t);
  }

  // see which URLs have not triggered any lead times yet, and disable if
  // too long
  time_t t0 = time(0);
  if (t0 - _real_time_0 > _max_seconds_before_disable)
  {
    _disable_urls();
  }

  // this URL is no longer timed out, if it was
  _remove_from_string(_timeout_urls, url);

  // this URL is no longer disabled, if it was disabled, and the flag is
  // set to reenable it.
  if (!_persistant_disable)
  {
    if (_remove_from_string(_disabled_urls, url))
    {
      LOGC(TaTriggerLog::name()) << "URL re-enabled " << url;
    }
  }

  if (_matching_string(_disabled_urls, url) >= 0)
  {
    // currently disabled, don't do anything with this
    LOGC(TaTriggerLog::name()) << "Ignore disabled URL " << url;
  }
  else
  {
    // here finally update state for this url at this lead time
    _last_trigger_time[url] = t0;
    _lead_time_state[ind].update(url);
  }
}

//----------------------------------------------------------------
void DsEnsembleLeadTrigger::_process_timeout(const std::string &url)
{
  LOG(DEBUG_VERBOSE) << "URL has no data from trigger " << url;

  time_t t0 = _last_trigger_time[url];
  time_t t = time(0);
  if (t0 == -1)
  {
    if (_real_time_0 == -1)
    {
      // nothing yet, carry on
      return;
    }
    // nothing yet for this lead time so compare overall first time
    // to this time and declare timeout if too big
    if (t - _real_time_0 > _max_seconds_before_timeout)
    {
      if (_matching_string(_timeout_urls, url) < 0)
      {
	LOG(WARNING) << "Timing out URL " << url;
	_timeout_urls.push_back(url);
      }
    }
  }
}

//----------------------------------------------------------------
void DsEnsembleLeadTrigger::_add_to_que(void)
{
  // see if anything is now complete and ready to go
  for (size_t i=0; i<_lead_time_state.size(); ++i)
  {
    if (_lead_time_state[i].should_process(_urls, _timeout_urls,
					   _disabled_urls))
    {
      _push_new_trigger(i);
      _lead_time_state[i].setProcessed();
    }
  }
}

//----------------------------------------------------------------
void DsEnsembleLeadTrigger::_push_new_trigger(const int i)
{
  Trigger_t ti;
  ti.t = _gen_time;
  ti.lt = _lead_time_state[i]._lead_seconds;
  ti.url = _lead_time_state[i]._urls;
  ti.complete = (_lead_time_state[i]._urls.size() == _urls.size());
  _triggered_que.push_back(ti);
}

//----------------------------------------------------------------
void DsEnsembleLeadTrigger::_start_new_gen_time(const time_t &t)
{
  if (_gen_time != -1)
  {
    for (size_t i=0; i<_lead_time_state.size(); ++i)
    {
      if (!_lead_time_state[i]._processed)
      {
	// any lead time not yet done should get done now
	_push_new_trigger(i);
      }
      _lead_time_state[i].init();
    }

    // no timeouts, they clear out every time
    _timeout_urls.clear();

    // disabled URLs are not disabled anymore
    _disabled_urls.clear();
  }

  for (size_t i=0; i<_urls.size(); ++i)
  {
    // no first time anymore for any url
    _last_trigger_time[_urls[i]] = -1;
  }

  _gen_time = t;
  _real_time_0 = time(0);
}


//----------------------------------------------------------------
void DsEnsembleLeadTrigger::_disable_urls(void)
{
  for (size_t i=0; i<_urls.size(); ++i)
  {
    bool good = false;
    for (size_t j=0; j<_lead_time_state.size(); ++j)
    {
      if (_lead_time_state[j].hasUrl(_urls[i]))
      {
	good = true;
	break;
      }
    }
    if (!good)
    {
      if (_add_string(_disabled_urls, _urls[i]))
      {
	LOG(WARNING) << "Disabling URL due to no input " << _urls[i];
      }
    }
  }
}

//----------------------------------------------------------------
LeadTimeState::LeadTimeState(const int lead_seconds) :
  _lead_seconds(lead_seconds), _processed(false)
{
}

//----------------------------------------------------------------
LeadTimeState::~LeadTimeState()
{
}

//----------------------------------------------------------------
void LeadTimeState::init(void)
{
  _processed = false;
  _urls.clear();
}

//----------------------------------------------------------------
void LeadTimeState::update(const std::string &url)
{
  if (_urls.empty())
  {
    _urls.push_back(url);
  }
  else
  {
    if (find(_urls.begin(), _urls.end(), url) != _urls.end())
    {
      LOG(ERROR) << url << " already on the list for lead=" << _lead_seconds;
      return;
    }
    _urls.push_back(url);
  }
}

//----------------------------------------------------------------
bool
LeadTimeState::should_process(const std::vector<std::string> &urls,
			      const std::vector<std::string> &timeout_urls,
			      const std::vector<std::string> &disabled_urls) const
{
  if (_processed)
  {
    return false;
  }

  for (size_t i=0; i<urls.size(); ++i)
  {
    if (find(_urls.begin(), _urls.end(), urls[i]) == _urls.end())
    {
      // not in place
      if (find(timeout_urls.begin(), timeout_urls.end(),
	       urls[i]) == timeout_urls.end())
      {
	// has not yet timed out
	if (find(disabled_urls.begin(), disabled_urls.end(),
	       urls[i]) == disabled_urls.end())
	{
	  // not disabled either
	  return false;
	}
      }
    }
  }
  // everything either in place or timed out or disabled
  return true;
}


//----------------------------------------------------------------
bool LeadTimeState::hasUrl(const std::string &url) const
{
  return find(_urls.begin(), _urls.end(), url) != _urls.end();
}

