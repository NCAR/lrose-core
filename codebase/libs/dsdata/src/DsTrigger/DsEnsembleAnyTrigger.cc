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
 * @file DsEnsembleAnyTrigger.cc
 */
#include <dsdata/DsEnsembleAnyTrigger.hh>
#include <dsdata/DsLdataTrigger.hh>
#include <toolsa/LogStream.hh>
#include <toolsa/LogMsgStreamInit.hh>
#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>
#include <toolsa/TaThreadSimplePolling.hh>
#include <toolsa/TaTriggerLog.hh>
#include <algorithm>

using std::vector;
using std::string;

//----------------------------------------------------------------
TaThread *ThreadAny::clone(int i)
{
  TaThreadSimplePolling *tp = new TaThreadSimplePolling(i);
  tp->setThreadMethod(DsEnsembleAnyTrigger::compute);
  tp->setThreadContext(_context);
  return (TaThread *)tp;
}

//----------------------------------------------------------------
DsEnsembleAnyTrigger::DsEnsembleAnyTrigger(void) :
  ThreadAny(),
  _sleepSeconds(ensembleAnyTrigger::defaultSleepSeconds),
  _archive_mode(false),
  _archive_index(-1)
{
  ThreadAny::setContext(this);

  LogMsgStreamInit::setThreading(false);
  LogMsgStreamInit::setTrigger(false);
}

//----------------------------------------------------------------
DsEnsembleAnyTrigger::
DsEnsembleAnyTrigger(const vector<string> &url,
		     const vector<int> &leadSeconds) :
  ThreadAny(),
  _sleepSeconds(ensembleAnyTrigger::defaultSleepSeconds),
  _archive_mode(false),
  _archive_index(-1),
  _urls(url)
{
  for (size_t i=0; i<url.size(); ++i)
  {
    DsEnsembleAnyTrigger1 *t1 = new DsEnsembleAnyTrigger1(url[i],
							  leadSeconds,
							  this);
    _elem.push_back(t1); //DsEnsembleAnyTrigger1(url[i], leadSeconds, this));
  }

  ThreadAny::setContext(this);
  ThreadAny::init(static_cast<int>(url.size()), false);
  LogMsgStreamInit::setThreading(false);
  LogMsgStreamInit::setTrigger(false);
}

//----------------------------------------------------------------
DsEnsembleAnyTrigger::
DsEnsembleAnyTrigger(const time_t &t0, const time_t &t1,
		     const vector<string> &url,
		     const vector<int> &leadSeconds) :
  ThreadAny(),
  _sleepSeconds(ensembleAnyTrigger::defaultSleepSeconds),
  _archive_mode(true),
  _archive_index(-1),
  _urls(url)
{
  // get all gen/lead times in range for all URls and order
  for (size_t i=0; i<url.size(); ++i)
  {
    DsMdvx D;
    D.setTimeListModeGen(url[i], t0, t1);
    D.compileTimeList();
    vector<time_t> gt = D.getTimeList();
    for (size_t j=0; j<gt.size(); ++j)
    {
      D.setTimeListModeForecast(url[i], gt[j]);
      D.compileTimeList();
      vector<time_t> vt = D.getValidTimes();
      for (size_t k=0; k<vt.size(); ++k)
      {
	int lt = vt[k] - gt[j];
	if (find(leadSeconds.begin(), leadSeconds.end(),
		 lt) != leadSeconds.end())
	{
	  _archive_events.push_back(AnyTriggerData(url[i], gt[j], lt));
	}
      }
    }
  }
      
  sort(_archive_events.begin(), _archive_events.end(),
       AnyTriggerData::lessThan);
  _archive_index = -1;
}

//----------------------------------------------------------------
DsEnsembleAnyTrigger::~DsEnsembleAnyTrigger(void)
{
  for (size_t i=0; i<_elem.size(); ++i)
  {
    delete _elem[i];
  }
}

//------------------------------------------------------------------
void DsEnsembleAnyTrigger::setDebug(void)
{
  LogMsgStreamInit::setThreading(true);
  LogMsgStreamInit::setTrigger(true);
}
  
//------------------------------------------------------------------
void DsEnsembleAnyTrigger::filterNames(const std::string &remove)
{
  for (size_t i=0; i<_elem.size(); ++i)
  {
    _elem[i]->filterName(remove);
  }
}

//------------------------------------------------------------------
void DsEnsembleAnyTrigger::setSleepSeconds(const int seconds)
{
  _sleepSeconds = seconds;
  for (size_t i=0; i<_elem.size(); ++i)
  {
    _elem[i]->setSleepSeconds(seconds);
  }
}

//----------------------------------------------------------------
void DsEnsembleAnyTrigger::setMaximumAgeSeconds(const int maxSeconds)
{
  for (size_t i=0; i<_elem.size(); ++i)
  {
    _elem[i]->setMaximumAgeSeconds(maxSeconds);
  }
}

//------------------------------------------------------------------
void DsEnsembleAnyTrigger::setTriggerMaxWaitSeconds(const int maxSeconds)
{
  for (size_t i=0; i<_elem.size(); ++i)
  {
    _elem[i]->setTriggerMaxWaitSeconds(maxSeconds);
  }
}
  
//------------------------------------------------------------------
void DsEnsembleAnyTrigger::compute(void *ti)
{
  DsEnsembleAnyTrigger1 *algInfo = 
    static_cast<DsEnsembleAnyTrigger1 *>(ti);

  string name = algInfo->getName();

  LOGC(TaTriggerLog::name()) << name << " processing now";

  algInfo->process();

  if (algInfo->hasData())
  {
    LOGC(TaTriggerLog::name()) << name << " Done processing now " 
			       << DateTime::strn(algInfo->getTime())
			       << "+" << algInfo->getLead();
  }
  else
  {
    LOGC(TaTriggerLog::name()) << name << " Done processing now, no data";
  }    
}

//----------------------------------------------------------------
bool DsEnsembleAnyTrigger::archiveNextGenLeadTime(time_t &t, int &lt, 
						  std::vector<std::string> &url,
						  bool &complete)
{
  url.clear();

  if (!_archive_mode)
  {
    LOG(ERROR) << "Not in archive mode";
    return false;
  }
  
  // start at current pointer and get next thing
  string urli;

  bool hasData;

  if (!_nextArchiveTime(t, lt, urli, hasData))
  {
    // no more data period
    return false;
  }

  url.push_back(urli);

  time_t ti;
  int lti;
  
  // now take advantage of order and increment archive state until the
  // gen or lead changes, filling in urls
  while (_nextArchiveTime(ti, lti, urli, hasData))
  {
    if (ti != t || lti != lt)
    {
      // back up by one for next call
      --_archive_index;
      complete = (url.size() == _urls.size());
      return true;
    }
    else
    {
      url.push_back(urli);
    }
  }
  complete = url.size() == _urls.size();
  return true;
}

//----------------------------------------------------------------
bool DsEnsembleAnyTrigger::nextTime(time_t &t, int &lt, std::string &url,
				    bool &hasData)
{
  if (_archive_mode)
  {
    return _nextArchiveTime(t, lt, url, hasData);
  }
  else
  {
    return _nextRealTime(t, lt, url, hasData);
  }
}

//----------------------------------------------------------------
bool DsEnsembleAnyTrigger::_nextArchiveTime(time_t &t, int &lt,
					    std::string &url,
					    bool &hasData)
{
  if (++_archive_index >= static_cast<int>(_archive_events.size()))
  {
    return false;
  }
  else
  {
    _archive_events[_archive_index].getValues(t, lt, url);
    hasData = true;
    return true;
  }
}
      

//----------------------------------------------------------------
bool DsEnsembleAnyTrigger::_nextRealTime(time_t &t, int &lt, std::string &url,
					bool &hasData)
{
  static bool first = true;
  if (first)
  {
    first = false;
    for (size_t i = 0; i<_elem.size(); ++i)
    {
      LOGC(TaTriggerLog::name()) << "creating a thread for " << 
	_elem[i]->sprintState();
      ThreadAny::thread(i, (void *)_elem[i]);
      //ThreadAny::thread(i, (void *)&_elem[i]);
    }
  }

  DsEnsembleAnyTrigger1 info;
  LOGC(TaTriggerLog::name()) << "begin waiting for any one thread";
  if (ThreadAny::waitForAnyOneThread(&info, DsEnsembleAnyTrigger1::copy))
  {
    if (info.hasData())
    {
      t = info.getTime();
      lt = info.getLead();
      url = info.getUrl();
      hasData = true;
      LOGC(TaTriggerLog::name()) << "Got one thread to finish, " 
				 << DateTime::strn(t) << "+" << lt << "," 
				 << url;
    }
    else
    {
      hasData = false;
      url = info.getUrl();
      LOGC(TaTriggerLog::name()) << "Got one thread to finish, NO DATA," 
				 << url;
    }
    return true;
  }
  else
  {
    return false;
  }
}


//----------------------------------------------------------------
DsEnsembleAnyTrigger1::DsEnsembleAnyTrigger1() :
  _url("none"),
  _name("none"),
  _has_data(false),
  _gt(-1),
  _lt(-1),
  _maxAgeSeconds(ensembleAnyTrigger::maxValidAgeSeconds),
  _sleepSeconds(ensembleAnyTrigger::defaultSleepSeconds),
  _maxWaitSeconds(ensembleAnyTrigger::maxWaitSeconds),
  _leadTimeSeconds(),
  _trigger(NULL),
  _alg(NULL)
{
}

//----------------------------------------------------------------
DsEnsembleAnyTrigger1::
DsEnsembleAnyTrigger1(const DsEnsembleAnyTrigger1 &u) :
  _url(u._url),
  _name(u._name),
  _has_data(u._has_data),
  _gt(u._gt),
  _lt(u._lt),
  _maxAgeSeconds(u._maxAgeSeconds),
  _sleepSeconds(u._sleepSeconds),
  _maxWaitSeconds(ensembleAnyTrigger::maxWaitSeconds),
  _leadTimeSeconds(u._leadTimeSeconds),
  _trigger(NULL),
  _alg(u._alg)
{
  _trigger = new DsLdataTrigger();
  _trigger->init(_url, _maxAgeSeconds,  PMU_auto_register, -1);
}

//----------------------------------------------------------------
DsEnsembleAnyTrigger1::
DsEnsembleAnyTrigger1(const std::string &url,
		      const vector<int> &leadTimeSeconds,
		      DsEnsembleAnyTrigger *alg) :
  _url(url),
  _name(url),
  _has_data(false),
  _gt(-1),
  _lt(-1),
  _maxAgeSeconds(ensembleAnyTrigger::maxValidAgeSeconds),
  _sleepSeconds(ensembleAnyTrigger::defaultSleepSeconds),
  _maxWaitSeconds(ensembleAnyTrigger::maxWaitSeconds),
  _leadTimeSeconds(leadTimeSeconds),
  _trigger(NULL),
  _alg(alg)
{
  _trigger = new DsLdataTrigger();
  _trigger->init(_url, _maxAgeSeconds,  PMU_auto_register, -1);
}

//----------------------------------------------------------------
DsEnsembleAnyTrigger1 &
DsEnsembleAnyTrigger1::operator=(const DsEnsembleAnyTrigger1 &u)
{
  if (&u == this)
  {
    return *this;
  }

  _url = u._url;
  _name = u._name;
  _has_data = u._has_data;
  _gt = u._gt;
  _lt = u._lt;
  _maxAgeSeconds = u._maxAgeSeconds;
  _sleepSeconds = u._sleepSeconds;
  _maxWaitSeconds = u._maxWaitSeconds;
  _leadTimeSeconds = u._leadTimeSeconds;
  _alg = u._alg;
  if (_trigger != NULL)
  {
    delete _trigger;
    _trigger = NULL;
  }
  _trigger = new DsLdataTrigger();
  _trigger->init(_url, _maxAgeSeconds,  PMU_auto_register, -1);
  return *this;
}

//----------------------------------------------------------------
bool
DsEnsembleAnyTrigger1::operator==(const DsEnsembleAnyTrigger1 &u) const
{
  // don't check _trigger pointer or thread start time
  return (_url == u._url &&
	  _name == u._name &&
	  _has_data == u._has_data &&
	  _gt == u._gt &&
	  _lt == u._lt &&
	  _maxAgeSeconds == u._maxAgeSeconds &&
	  _sleepSeconds == u._sleepSeconds &&
	  _maxWaitSeconds == u._maxWaitSeconds &&
	  _leadTimeSeconds == u._leadTimeSeconds &&
	  _alg == u._alg);
}

//----------------------------------------------------------------
DsEnsembleAnyTrigger1::~DsEnsembleAnyTrigger1(void)
{
  if (_trigger != NULL)
  {
    delete _trigger;
    _trigger = NULL;
  }
}

//----------------------------------------------------------------
std::string DsEnsembleAnyTrigger1::sprintState(void) const
{
  return _sprintState();
}

//----------------------------------------------------------------
void DsEnsembleAnyTrigger1::filterName(const std::string &remove)
{
  size_t i = _name.find(remove);
  if (i == string::npos)
  {
    return;
  }
  _name.erase(i, remove.size());
}

//----------------------------------------------------------------
void DsEnsembleAnyTrigger1::setSleepSeconds(const int seconds)
{
  _sleepSeconds = seconds;
}

//----------------------------------------------------------------
void DsEnsembleAnyTrigger1::setMaximumAgeSeconds(const int maxSeconds)
{
  if (_maxAgeSeconds != maxSeconds)
  {
    _maxAgeSeconds = maxSeconds;
    if (_trigger != NULL)
    {
      _trigger->init(_url, _maxAgeSeconds,  PMU_auto_register, -1);
    }
  }
}

//----------------------------------------------------------------
void DsEnsembleAnyTrigger1::setTriggerMaxWaitSeconds(const int maxSeconds)
{
  _maxWaitSeconds = maxSeconds;
}

//----------------------------------------------------------------
void DsEnsembleAnyTrigger1::process(void)
{
  LOGC(TaTriggerLog::name()) << _name << " Wait for trigger";
  _has_data = _next_time_sequence();
  if (_has_data)
  {
    LOGC(TaTriggerLog::name()) << _name << " New time = " 
			       << DateTime::strn(_gt) << "+" << _lt;
  }
  else
  {
    LOGC(TaTriggerLog::name()) << _name << " Timeout or error";
  }
}

//----------------------------------------------------------------
void DsEnsembleAnyTrigger1::copy(void *input, void *output)
{
  DsEnsembleAnyTrigger1 *in, *out;

  in = static_cast<DsEnsembleAnyTrigger1 *>(input);

  if (in->_has_data)
  {
    LOGC(TaTriggerLog::name()) << "input url:" << in->_url
			       << "  time:" << DateTime::strn(in->_gt)
			       << "  lead:" << in->_lt;
  }
  else
  {
    LOGC(TaTriggerLog::name()) << "input url:" << in->_url << " Nodata";
  }

  out = static_cast<DsEnsembleAnyTrigger1 *>(output);
  out->_url = in->_url;
  out->_name = in->_name;
  out->_leadTimeSeconds = in->_leadTimeSeconds;
  out->_maxAgeSeconds = in->_maxAgeSeconds;
  out->_sleepSeconds = in->_sleepSeconds;
  out->_gt = in->_gt;
  out->_lt = in->_lt;
  out->_has_data = in->_has_data;
  out->_trigger = NULL;
}

//----------------------------------------------------------------
std::string 
DsEnsembleAnyTrigger1::_sprintState(void) const
{
  string s;
  if (_has_data)
  {
    char buf[1000];
    sprintf(buf, "%s  Has data %s+%d", _name.c_str(), 
	    DateTime::strn(_gt).c_str(), _lt);
    s = buf;
  }
  else
  {
    s = _name + "  No data";
  }
  return s;
}

//----------------------------------------------------------------
bool DsEnsembleAnyTrigger1::_next_time_sequence(void)
{
  time_t gt;
  int lt;

  int totalSleep=0;

  while (true)
  {
    PMU_auto_register("_next_time_sequence");
    if (_trigger->endOfData())
    {
      LOGC(TaTriggerLog::name()) << "end of data " << _name;
      return false;
    }

    LOGC(TaTriggerLog::name()) << "triggering " << _name;
    DsTrigger *T = dynamic_cast<DsTrigger *>(_trigger);
    TriggerInfo i;
    if (T->next(i) != 0)
    {
      LOGC(TaTriggerLog::name()) << "call to next empty return, "
				 << _name << " sleep " << _sleepSeconds
				 << " total sleep " << totalSleep;
      char buf[1000];
      sprintf(buf, "_next_time_sequence_sleep(%d of %d)", _sleepSeconds,
	      totalSleep);
      PMU_auto_register(buf);
      sleep(_sleepSeconds);
      totalSleep += _sleepSeconds;
      if (totalSleep >= _maxWaitSeconds)
      {
	LOGC(TaTriggerLog::name()) << _name << " has timed out";
	return false;
      }
    }
    else
    {
      gt = i.getIssueTime();
      time_t vt = i.getForecastTime();
      lt = vt - gt;
      LOGC(TaTriggerLog::name()) << "next gave " << DateTime::strn(gt) << "+"
				 << lt << ", " << _name;
      if (gt != _gt || lt != _lt)
      {
	LOGC(TaTriggerLog::name()) << "NEW GEN OR LEAD NOW " 
				   << _name << " "  << DateTime::strn(gt);
	break;
      }
      else
      {
	LOGC(TaTriggerLog::name()) << "Same gen and lead as before, "
				   << _name;
	char buf[1000];
	sprintf(buf, "_next_time_sequence_sleep(%d of %d)", _sleepSeconds,
		totalSleep);
	PMU_auto_register(buf);
	sleep(_sleepSeconds);
	totalSleep += _sleepSeconds;
	if (totalSleep >= _maxWaitSeconds)
	{
	  LOG(DEBUG) << _name << " has timed out";
	  return false;
	}
      }
    }
  }
  _gt = gt;
  _lt = lt;
  LOGC(TaTriggerLog::name()) << "returning now, should be complete, "
			     << _name << ", " << DateTime::strn(_gt);
  return true;
}


//----------------------------------------------------------------
AnyTriggerData::AnyTriggerData(void) : _gt(-1), _lt(-1)
{
}

//----------------------------------------------------------------
AnyTriggerData::AnyTriggerData(const std::string &url, const time_t &gt,
			       int lt) : _url(url), _gt(gt), _lt(lt)
{
}

//----------------------------------------------------------------
AnyTriggerData::~AnyTriggerData(void)
{
}

//----------------------------------------------------------------
void AnyTriggerData::getValues(time_t &t, int &lt, std::string &url) const
{
  t = _gt;
  lt = _lt;
  url = _url;
}

//----------------------------------------------------------------
bool AnyTriggerData::lessThan(const AnyTriggerData &d0,
			      const AnyTriggerData &d1)
{
  if (d0._gt < d1._gt)
  {
    return true;
  }
  else if (d0._gt > d1._gt)
  {
    return false;
  }
  else
  {
    if (d0._lt < d1._lt)
    {
      return true;
    }
    else if (d0._lt > d1._lt)
    {
      return false;
    }
    else
    {
      return (d0._url < d1._url);
    }
  }
}
