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
 * @file DsEnsembleGenTrigger.cc
 */
#include <dsdata/DsEnsembleGenTrigger.hh>
#include <dsdata/DsLdataTrigger.hh>
#include <Mdv/DsMdvx.hh>
#include <toolsa/LogStream.hh>
#include <toolsa/LogMsgStreamInit.hh>
#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>
#include <toolsa/TaThreadSimple.hh>
#include <toolsa/TaTriggerLog.hh>
#include <algorithm>
#include <sstream>
#include <iomanip>

using std::vector;
using std::string;

//----------------------------------------------------------------
TaThread *ThreadAll::clone(int i)
{
  TaThreadSimple *t = new TaThreadSimple(i);
  t->setThreadMethod(DsEnsembleGenTrigger::compute);
  t->setThreadContext(_context);
  return (TaThread *)t;
}

//----------------------------------------------------------------
DsEnsembleGenTrigger::DsEnsembleGenTrigger(void) :
  ThreadAll(),
  _threadComplete(false),
  _threadCompleteTime(-1),
  _archiveMode(false),
  _sleepSeconds(ensembleGenTrigger::defaultSleepSeconds)
{
  ThreadAll::setContext(this);
  LogMsgStreamInit::setThreading(false);
  LogMsgStreamInit::setTrigger(false);
}

//----------------------------------------------------------------
DsEnsembleGenTrigger::
DsEnsembleGenTrigger(const vector<string> &url,
		       const vector<int> &leadSeconds,
		       const int maxSecondsDuringGenTime,
		       const int maxSecondsAfterThreadComplete,
		       const bool orderedLeadTimes) :
  ThreadAll(),
  _threadComplete(false),
  _threadCompleteTime(-1),
  _archiveMode(false),
  _sleepSeconds(ensembleGenTrigger::defaultSleepSeconds)
{
  for (size_t i=0; i<url.size(); ++i)
  {
    
    _elem.push_back(DsEnsembleGenTrigger1(url[i], leadSeconds,
					    maxSecondsDuringGenTime,
					    maxSecondsAfterThreadComplete,
					    orderedLeadTimes, this));
  }

  ThreadAll::setContext(this);
  ThreadAll::init(static_cast<int>(url.size()), false);
  LogMsgStreamInit::setThreading(false);
  LogMsgStreamInit::setTrigger(false);
}

//----------------------------------------------------------------
DsEnsembleGenTrigger::
DsEnsembleGenTrigger(const time_t &t0, const time_t &t1,
		       const vector<string> &url,
		       const vector<int> &leadSeconds) :
  ThreadAll(),
  _threadComplete(false),
  _threadCompleteTime(-1),
  _archiveMode(true),
  _sleepSeconds(ensembleGenTrigger::defaultSleepSeconds)
{
  for (size_t i=0; i<url.size(); ++i)
  {
    _elem.push_back(DsEnsembleGenTrigger1(t0, t1, url[i],
					    leadSeconds, this));
  }
  ThreadAll::setContext(this);
  LogMsgStreamInit::setThreading(false);
  LogMsgStreamInit::setTrigger(false);
}

//----------------------------------------------------------------
DsEnsembleGenTrigger::~DsEnsembleGenTrigger(void)
{
}

//------------------------------------------------------------------
void DsEnsembleGenTrigger::setDebug(void)
{
  LogMsgStreamInit::setThreading(true);
  LogMsgStreamInit::setTrigger(true);
}
  
//------------------------------------------------------------------
void DsEnsembleGenTrigger::filterNames(const std::string &remove)
{
  for (size_t i=0; i<_elem.size(); ++i)
  {
    _elem[i].filterName(remove);
  }
}

//------------------------------------------------------------------
void DsEnsembleGenTrigger::setSleepSeconds(const int seconds)
{
  _sleepSeconds = seconds;
  for (size_t i=0; i<_elem.size(); ++i)
  {
    _elem[i].setSleepSeconds(seconds);
  }
}

//----------------------------------------------------------------
void DsEnsembleGenTrigger::setMaximumAgeSeconds(const int maxSeconds)
{
  for (size_t i=0; i<_elem.size(); ++i)
  {
    _elem[i].setMaximumAgeSeconds(maxSeconds);
  }
}

//------------------------------------------------------------------
void DsEnsembleGenTrigger::compute(void *ti)
{
  DsEnsembleGenTrigger1 *algInfo = 
    static_cast<DsEnsembleGenTrigger1 *>(ti);

  algInfo->processGenTime();

  // IN THIS CASE WE DO NOT DELETE THE INFO
}

//----------------------------------------------------------------
bool DsEnsembleGenTrigger::nextTime(time_t &t)
{
  // get a target gen time
  if (!_initializeGenTime(t))
  {
    LOG(DEBUG) << "No more data";
    return false;
  }

  // set state so no thread is complete
  _threadComplete = false;
  _threadCompleteTime = -1;

  // tell each thread that this is the gen time to shoot for,
  // then fire off that thread
  for (size_t i = 0; i<_elem.size(); ++i)
  {
    _elem[i].setWantedGenTime(t);
    ThreadAll::thread(i, (void *)&_elem[i]);
  }
  ThreadAll::waitForThreads();

  // is there anything more?
  bool noMoreData = true;
  for (size_t i=0; i<_elem.size(); ++i)
  {
    if (!_elem[i].noMoreData())
    {
      noMoreData = false;
      break;
    }
  }
  if (noMoreData)
  {
    LOG(DEBUG) << "No more data";
    return false;
  }

  PMU_auto_register("finish");

  // evaluate each thread to get the best gen time
  t = -1;
  for (size_t i = 0; i<_elem.size(); ++i)
  {
    _evaluate(_elem[i], i, t);
  }
  if (t == -1)
  {
    LOG(ERROR) << "Somehow no URL had triggering";
  }
  return t != -1;
}

//----------------------------------------------------------------
void DsEnsembleGenTrigger::setThreadComplete(void)
{
  ThreadAll::lockForIO();
  if (!_threadComplete)
  {
    _threadComplete = true;
    LOGC(TaTriggerLog::name()) << "First Thread has completed";
    _threadCompleteTime = time(0);
  }
  ThreadAll::unlockAfterIO();
}

//----------------------------------------------------------------
bool DsEnsembleGenTrigger::threadComplete(time_t &doneTime)
{
  ThreadAll::lockForIO();
  bool status = false;
  doneTime = -1;
  if (_threadComplete)
  {
    doneTime = _threadCompleteTime;
    status = true;
  }
  ThreadAll::unlockAfterIO();
  return status;
}

//----------------------------------------------------------------
time_t DsEnsembleGenTrigger::currentGen(const int i) const
{
  return _elem[i].currentGen();
}

//----------------------------------------------------------------
std::vector<int> DsEnsembleGenTrigger::currentLeads(const int i) const
{
  return _elem[i].currentLeads();
}

//----------------------------------------------------------------
bool DsEnsembleGenTrigger::_initializeGenTime(time_t &t)
{
  if (_archiveMode)
  {
    return _initializeGenTimeArchive(t);
  }
  else
  {
    return _initializeGenTimeRealtime(t);
  }
}

//----------------------------------------------------------------
bool DsEnsembleGenTrigger::_initializeGenTimeArchive(time_t &t)
{
  t = -1;
  for (size_t i = 0; i<_elem.size(); ++i)
  {
    PMU_auto_register("init");

    time_t ti;
    if (_elem[i].startGenTime(ti))
    {
      if (t == -1)
      {
	t = ti;
      }
      else
      {
	// minimize gen time in archive mode
	if (ti < t)
	{
	  t = ti;
	}
      }
    }
  }
  if (t != -1)
  {
    LOGC(TaTriggerLog::name()) << "Target Gen time = " << DateTime::strn(t);
    return true;
  }
  else
  {
    return false;
  }
}

//----------------------------------------------------------------
bool DsEnsembleGenTrigger::_initializeGenTimeRealtime(time_t &t)
{
  t = -1;
  LOGC(TaTriggerLog::name()) << "Trying to get something to trigger";
  while (t == -1)
  {
    PMU_auto_register("Wait for good data");

    // keep trying till get some trigger
    for (size_t i = 0; i<_elem.size(); ++i)
    {
      time_t ti;
      if (_elem[i].startGenTime(ti))
      {
	// got something
	if (t == -1)
	{
	  t = ti;
	}
	else
	{
	  // maximize gen time in real time
	  if (ti > t)
	  {
	    t = ti;
	  }
	}
      }
    }
    if (t == -1)
    {
      sleep(_sleepSeconds);
    }
  }

  LOGC(TaTriggerLog::name()) << "Target Gen time = " << DateTime::strn(t);
  return true;
}

//----------------------------------------------------------------
void DsEnsembleGenTrigger::_evaluate(DsEnsembleGenTrigger1 &e,
				       size_t i, time_t &t)
{
  if (e.noMoreData())
  {
    // dont change t
    return;
  }
  if (!e.hasData())
  {
    LOGC(TaTriggerLog::name()) << "Element has no data, " << e.sprintState();
    // dont change t
    return;
  }
  time_t ti = e.currentGen();
  if (t == -1)
  {
    // change t
    t = ti;
    LOGC(TaTriggerLog::name()) << "Element has data, time=" 
			       << DateTime::strn(t) << ", " << e.sprintState();
    return;
  }
  if (t != ti)
  {
    // maybe change t
    LOG(ERROR) << "Gen times don't match " << DateTime::strn(t) << " " 
	       <<  DateTime::strn(ti);
    if (t < ti)
    {
      for (size_t j=0; j<i; ++j)
      {
	if (_elem[j].hasData() && !_elem[j].noMoreData())
	{
	  if (_elem[j].currentGen() == t)
	  {
	    _elem[j].clear();
	  }
	}
      }
      t = ti;
    }
    else
    {
      e.clear();
    }
  }
}

//----------------------------------------------------------------
DsEnsembleGenTrigger1::DsEnsembleGenTrigger1() :
  _url("none"),
  _name("none"),
  _leadTimeSeconds(),
  _orderedLeadTimes(false),
  _maxAgeSeconds(ensembleGenTrigger::maxValidAgeSeconds),
  _maxSecondsDuringGenTime(0),
  _maxSecondsAfterThreadComplete(0),
  _sleepSeconds(ensembleGenTrigger::defaultSleepSeconds),
  _wantedGt(-1),
  _isForwardGt(false),
  _forwardGt(-1),
  _gt(-1),
  _availableLtSeconds(),
  _archive(false),
  _archiveGenTimesInRange(),
  _nextArchiveIndex(-1),
  _trigger(NULL),
  _noMoreData(true),
  _hasData(false),
  _threadStartTime(-1),
  _alg(NULL)
{
}

//----------------------------------------------------------------
DsEnsembleGenTrigger1::
DsEnsembleGenTrigger1(const DsEnsembleGenTrigger1 &u) :
  _url(u._url),
  _name(u._name),
  _leadTimeSeconds(u._leadTimeSeconds),
  _orderedLeadTimes(u._orderedLeadTimes),
  _maxAgeSeconds(u._maxAgeSeconds),
  _maxSecondsDuringGenTime(u._maxSecondsDuringGenTime),
  _maxSecondsAfterThreadComplete(u._maxSecondsAfterThreadComplete),
  _sleepSeconds(u._sleepSeconds),
  _wantedGt(u._wantedGt),
  _isForwardGt(u._isForwardGt),
  _forwardGt(u._forwardGt),
  _gt(u._gt),
  _availableLtSeconds(u._availableLtSeconds),
  _archive(u._archive),
  _archiveGenTimesInRange(u._archiveGenTimesInRange),
  _nextArchiveIndex(u._nextArchiveIndex),
  _trigger(NULL),
  _noMoreData(u._noMoreData),
  _hasData(u._hasData),
  _threadStartTime(u._threadStartTime),
  _alg(u._alg)
{
  if (!u._archive)
  {
    _trigger = new DsLdataTrigger();
    _trigger->init(_url, _maxAgeSeconds,  PMU_auto_register, -1);
  }
}

//----------------------------------------------------------------
DsEnsembleGenTrigger1::
DsEnsembleGenTrigger1(const std::string &url,
			const vector<int> &leadTimeSeconds,
			const int maxSecondsDuringGenTime,
			const int maxSecondsAfterThreadComplete,
			bool orderedLeadTimes, DsEnsembleGenTrigger *alg) :
  _url(url),
  _name(url),
  _leadTimeSeconds(leadTimeSeconds),
  _orderedLeadTimes(orderedLeadTimes),
  _maxAgeSeconds(ensembleGenTrigger::maxValidAgeSeconds),
  _maxSecondsDuringGenTime(maxSecondsDuringGenTime),
  _maxSecondsAfterThreadComplete(maxSecondsAfterThreadComplete),
  _sleepSeconds(ensembleGenTrigger::defaultSleepSeconds),
  _wantedGt(-1),
  _isForwardGt(false),
  _forwardGt(-1),
  _gt(-1),
  _availableLtSeconds(),
  _archive(false),
  _archiveGenTimesInRange(),
  _nextArchiveIndex(-1),
  _trigger(NULL),
  _noMoreData(false),
  _hasData(false),
  _threadStartTime(-1),
  _alg(alg)
{
  _trigger = new DsLdataTrigger();
  _trigger->init(_url, _maxAgeSeconds,  PMU_auto_register, -1);
}

//----------------------------------------------------------------
DsEnsembleGenTrigger1::
DsEnsembleGenTrigger1(const time_t &t0, const time_t &t1,
			const std::string &url,
			const vector<int> &leadSeconds,
			DsEnsembleGenTrigger *alg) :
  _url(url),
  _name(url),
  _leadTimeSeconds(leadSeconds),
  _orderedLeadTimes(true),
  _maxAgeSeconds(ensembleGenTrigger::maxValidAgeSeconds),
  _maxSecondsDuringGenTime(0),
  _maxSecondsAfterThreadComplete(0),
  _sleepSeconds(ensembleGenTrigger::defaultSleepSeconds),
  _wantedGt(-1),
  _isForwardGt(false),
  _forwardGt(-1),
  _gt(-1),
  _availableLtSeconds(),
  _archive(true),
  _archiveGenTimesInRange(),
  _nextArchiveIndex(-1),
  _trigger(NULL),
  _noMoreData(false),
  _hasData(false),
  _threadStartTime(-1),
  _alg(alg)
{
  DsMdvx D;
  // one day out is plenty.
  D.setTimeListModeGen(_url, t0, t1 + 24*3600);
  D.compileTimeList();

  // get all the gen and valid times
  vector<time_t> gt = D.getGenTimes();

  // construct the data from these
  for (size_t i=0; i<gt.size(); ++i)
  {
    if (gt[i] > t1 || gt[i] < t0)
    {
      continue;
    }
    _archiveGenTimesInRange.push_back(gt[i]);
  }
  if (_archiveGenTimesInRange.empty())
  {
    LOG(ERROR) <<  _name << " has no archive data in range";
    _noMoreData = true;
  }
  _nextArchiveIndex = 0;
}

//----------------------------------------------------------------
DsEnsembleGenTrigger1 &
DsEnsembleGenTrigger1::operator=(const DsEnsembleGenTrigger1 &u)
{
  if (&u == this)
  {
    return *this;
  }

  _url = u._url;
  _name = u._name;
  _leadTimeSeconds = u._leadTimeSeconds;
  _orderedLeadTimes = u._orderedLeadTimes;
  _maxAgeSeconds = u._maxAgeSeconds;
  _maxSecondsDuringGenTime = u._maxSecondsDuringGenTime;
  _maxSecondsAfterThreadComplete = u._maxSecondsAfterThreadComplete;
  _sleepSeconds = u._sleepSeconds;
  _wantedGt = u._wantedGt;
  _isForwardGt = u._isForwardGt;
  _forwardGt = u._forwardGt;
  _gt = u._gt;
  _availableLtSeconds = u._availableLtSeconds;
  _archive = u._archive;
  _archiveGenTimesInRange = u._archiveGenTimesInRange;
  _nextArchiveIndex = u._nextArchiveIndex;
  _noMoreData = u._noMoreData;
  _hasData = u._hasData;
  _threadStartTime = u._threadStartTime;
  _alg = u._alg;

  if (_trigger != NULL)
  {
    delete _trigger;
    _trigger = NULL;
  }
  if (!u._archive)
  {
    _trigger = new DsLdataTrigger();
    _trigger->init(_url, _maxAgeSeconds,  PMU_auto_register, -1);
  }
  return *this;
}

//----------------------------------------------------------------
bool DsEnsembleGenTrigger1::operator==(const DsEnsembleGenTrigger1 &u) const
{
  // don't check _trigger pointer or thread start time
  return (_url == u._url &&
	  _name == u._name &&
	  _leadTimeSeconds == u._leadTimeSeconds &&
	  _orderedLeadTimes == u._orderedLeadTimes &&
	  _maxAgeSeconds == u._maxAgeSeconds &&
	  _maxSecondsDuringGenTime == u._maxSecondsDuringGenTime &&
	  _maxSecondsAfterThreadComplete == u._maxSecondsAfterThreadComplete &&
	  _sleepSeconds == u._sleepSeconds &&
	  _wantedGt == u._wantedGt &&
	  _isForwardGt == u._isForwardGt &&
	  _forwardGt == u._forwardGt &&
	  _gt == u._gt &&
	  _availableLtSeconds == u._availableLtSeconds &&
	  _archive == u._archive &&
	  _archiveGenTimesInRange == u._archiveGenTimesInRange &&
	  _nextArchiveIndex == u._nextArchiveIndex &&
	  _noMoreData == u._noMoreData &&
	  _hasData == u._hasData &&
	  _alg == u._alg);
}

//----------------------------------------------------------------
DsEnsembleGenTrigger1::~DsEnsembleGenTrigger1(void)
{
  if (_trigger != NULL)
  {
    delete _trigger;
    _trigger = NULL;
  }
}

//----------------------------------------------------------------
std::string DsEnsembleGenTrigger1::sprintState(void) const
{
  return _sprintState();
}

//----------------------------------------------------------------
void DsEnsembleGenTrigger1::filterName(const std::string &remove)
{
  size_t i = _name.find(remove);
  if (i == string::npos)
  {
    return;
  }
  _name.erase(i, remove.size());
}

//----------------------------------------------------------------
void DsEnsembleGenTrigger1::setSleepSeconds(const int seconds)
{
  _sleepSeconds = seconds;
}

//----------------------------------------------------------------
void DsEnsembleGenTrigger1::setMaximumAgeSeconds(const int maxSeconds)
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
void DsEnsembleGenTrigger1::clear(void)
{
  _hasData = false;
  _availableLtSeconds.clear();
}

//----------------------------------------------------------------
bool DsEnsembleGenTrigger1::startGenTime(time_t &gt)
{
  clear();
  if (_noMoreData)
  {
    LOGC(TaTriggerLog::name()) << _name << " - No more data";
    return false;
  }
  if (_archive)
  {
    return _startGenTimeArchive(gt);
  }
  else
  {
    return _startGenTimeRealtime(gt);
  }
}

//----------------------------------------------------------------
void DsEnsembleGenTrigger1::setWantedGenTime(const time_t &gt)
{
  _wantedGt = gt;
}

//----------------------------------------------------------------
void DsEnsembleGenTrigger1::processGenTime(void)
{
  LOGC(TaTriggerLog::name()) << _name << " starting gen time";
  clear();
  _threadStartTime = time(0);
  if (_noMoreData)
  {
    LOGC(TaTriggerLog::name()) << _name << " - No more data";
    return;
  }
  if (_archive)
  {
    _nextArchive();
  }
  else
  {
    _nextRealtime();
  }
}

//----------------------------------------------------------------
bool DsEnsembleGenTrigger1::_startGenTimeArchive(time_t &gt)
{
  if (_nextArchiveIndex >= 
      static_cast<int>(_archiveGenTimesInRange.size()))
  {
    return false;
  }
  else
  {
    gt = _archiveGenTimesInRange[_nextArchiveIndex];
    return true;
  }
}

//----------------------------------------------------------------
bool DsEnsembleGenTrigger1::_startGenTimeRealtime(time_t &gt)
{
  if (_isForwardGt)
  {
    // copy state from last time into current situation
    gt = _forwardGt;
    _isForwardGt = false;
    _forwardGt = -1;
    return true;
  }


  time_t t;

  DsMdvx D;
  t = time(0);
  if (_gt == 1)
  {
    // back from right now up to maximum
    D.setTimeListModeGen(_url, t - _maxAgeSeconds, t + 3600);
    LOGC(TaTriggerLog::name()) << _name << " looking back from now " 
			       << _maxAgeSeconds << " seconds";
 }
  else
  {
    // from one second after last gen time up to maximum
    D.setTimeListModeGen(_url, _gt+1, t+3600);
    LOGC(TaTriggerLog::name()) << _name <<
      " looking from last gen to 1 hour into future [" <<
      DateTime::strn(_gt+1) << "," << DateTime::strn(t+3600) << "]";
  }
  D.compileTimeList();
  vector<time_t> gtInRange = D.getGenTimes();
  if (gtInRange.empty())
  {
    LOGC(TaTriggerLog::name()) << _name << " - nothing";
    return false;
  }
  else
  {
    t = *gtInRange.rbegin();
    if (t == _gt)
    {
      // ignore this, wait for a new one
      LOG(WARNING) << _name << " Ignoring, already processed gen="
		   << DateTime::strn(t);
      return false;
    }
    gt = t;
    LOGC(TaTriggerLog::name()) << _name << " - data at " << DateTime::strn(t);
    return true;
  }
}

//----------------------------------------------------------------
void DsEnsembleGenTrigger1::_nextArchive(void)
{
  PMU_auto_register("_nextArchive");

  // seems this design can skip some gen times?
  // maybe not due to _startGenTimeArchive
  vector<time_t>::const_iterator i;
  i = find(_archiveGenTimesInRange.begin(),
	   _archiveGenTimesInRange.end(), _wantedGt);
  if (i == _archiveGenTimesInRange.end())
  {
    LOG(WARNING) << _name << " No data at gen time " 
		 << DateTime::strn(_wantedGt);
    return;
  }
  _nextArchiveIndex = i - _archiveGenTimesInRange.begin();
  if (_nextArchiveIndex >= 
      static_cast<int>(_archiveGenTimesInRange.size()))
  {
    _noMoreData = true;
    return;
  }
  _gt = _archiveGenTimesInRange[_nextArchiveIndex++];
  _hasData = true;

  DsMdvx D;
  D.setTimeListModeForecast(_url, _gt);
  D.compileTimeList();
  vector<time_t> vt = D.getValidTimes();
  _availableLtSeconds.clear();
  for (size_t i=0; i<vt.size(); ++i)
  {
    int lt = vt[i] - _gt;
    if (find(_leadTimeSeconds.begin(), _leadTimeSeconds.end(), lt) !=
	_leadTimeSeconds.end())
    {
      _availableLtSeconds.push_back(vt[i] - _gt);
    }
  }
}

//----------------------------------------------------------------
void DsEnsembleGenTrigger1::_nextRealtime(void)
{
  _initForWantedGenTime();
  _printState();

  bool doSleep = false;
  while (_canAddToGenTime(doSleep))
  {
    if (doSleep)
    {
      PMU_auto_register("_nextRealTime");
      sleep(_sleepSeconds);
      if (_shouldGiveUp())
      {
	LOG(WARNING) << _name << " No completion..timeout";
	return;
      }
    }
  }
  LOGC(TaTriggerLog::name()) << _name << " Cannot add to thread";
  _alg->setThreadComplete();
}

//----------------------------------------------------------------
void DsEnsembleGenTrigger1::_initForWantedGenTime(void)
{
  _gt = _wantedGt;
  DsMdvx D;
  D.setTimeListModeForecast(_url, _gt);
  D.compileTimeList();
  vector<time_t> vt = D.getValidTimes();
  _availableLtSeconds.clear();
  for (size_t i=0; i<vt.size(); ++i)
  {
    int lt = vt[i] - _gt;
    if (find(_leadTimeSeconds.begin(), _leadTimeSeconds.end(), lt) !=
	_leadTimeSeconds.end())
    {
      _availableLtSeconds.push_back(vt[i] - _gt);
    }
  }
  _hasData = !_availableLtSeconds.empty();
}

//----------------------------------------------------------------
bool DsEnsembleGenTrigger1::_canAddToGenTime(bool &doSleep)
{
  doSleep = false;

  // is everything here now?
  if (_genIsComplete())
  {
    LOGC(TaTriggerLog::name()) << _name << " Gen " << DateTime::strn(_gt) 
			       << " is complete";
    return false;
  }

  time_t t;
  int lt;

  Trigger_t status = _doTrigger(t, lt);
  bool ret = true;
  switch (status)
  {
  case END_OF_DATA:
    LOGC(TaTriggerLog::name()) << _name << " end of data";
    _noMoreData = true;
    ret = false;
    break;
  case NO_NEW_DATA:
    LOGC(TaTriggerLog::name()) << _name << " No new data";
    doSleep = true;
    ret = true;
    break;
  case GOT_DATA:
    if (t == _gt)
    {
      if (find(_leadTimeSeconds.begin(), _leadTimeSeconds.end(), lt) !=
	  _leadTimeSeconds.end())
      {
	_availableLtSeconds.push_back(lt);
	sort(_availableLtSeconds.begin(), _availableLtSeconds.end());
	LOGC(TaTriggerLog::name()) << _name << " Add "
				   << DateTime::strn(_gt) << "+"
				   << lt;
	_hasData = true;
	_printState();
      }
      ret = true;
    }
    else if (t < _gt)
    {
      LOGC(TaTriggerLog::name()) << _name << " Ignor data from old gen time "
				 << DateTime::strn(t) << "+" << lt;
      ret = true;
    } 
    else // t > _gt
    {
      LOGC(TaTriggerLog::name()) << _name << 
	" Triggered new gen time while processing older, "
				 << DateTime::strn(t) << "+" << lt;
      LOGC(TaTriggerLog::name()) << _name << " Stop thread for gen="
				 << DateTime::strn(_gt);
      _printState();
      _isForwardGt = true;
      _forwardGt= t;
      ret = true;
    }
    break;
  }      
  return ret;
}

//----------------------------------------------------------------
bool DsEnsembleGenTrigger1::_genIsComplete(void) const
{
  if (_availableLtSeconds.empty())
  {
    return false;
  }

  if (_availableLtSeconds == _leadTimeSeconds)
  {
    LOGC(TaTriggerLog::name()) << _name << " gen is complete " 
			       << DateTime::strn(_gt);
    return true;
  }
  if (_orderedLeadTimes)
  {
    if (*_availableLtSeconds.rbegin() == *_leadTimeSeconds.rbegin())
    {
      LOGC(TaTriggerLog::name()) << _name 
				 << " gen has last lead and is complete " 
				 << DateTime::strn(_gt);
      return true;
    }
  }
  return false;
}

//----------------------------------------------------------------
bool DsEnsembleGenTrigger1::_shouldGiveUp(void)
{
  if (_isForwardGt)
  {
    return true;
  }

  // lock and check the shared thingy.
  time_t doneTime;
  if (_alg->threadComplete(doneTime))
  {
    if (time(0) - doneTime > _maxSecondsAfterThreadComplete)
    {
      LOGC(TaTriggerLog::name()) << _name 
				 << " Timeout after another thread completed";
      return true;
    }
  }
  else
  {
    if (time(0) - _threadStartTime > _maxSecondsDuringGenTime)
    {
      LOGC(TaTriggerLog::name()) << _name 
				 << " Timeout for this thread";
      return true;
    }
  }
  return false;
}

//----------------------------------------------------------------
DsEnsembleGenTrigger1::Trigger_t
DsEnsembleGenTrigger1::_doTrigger(time_t &gt, int &lt)
{
  if (_trigger->endOfData())
  {
    LOGC(TaTriggerLog::name()) << _name << " end of data";
    return END_OF_DATA;
  }
  
  DsTrigger *T = dynamic_cast<DsTrigger *>(_trigger);
  TriggerInfo i;
  if (T->next(i) != 0)
  {
    LOGC(TaTriggerLog::name()) << _name << " No new data";
    return NO_NEW_DATA;
  }
  else
  {
    gt = i.getIssueTime();
    time_t vt = i.getForecastTime();
    lt = vt - gt;
    return GOT_DATA;
  }
}


//----------------------------------------------------------------
void DsEnsembleGenTrigger1::_printState(void) const
{
  std::ostringstream buf;
  for (size_t i=0; i<_availableLtSeconds.size(); ++i)
  {
    double h = static_cast<double>(_availableLtSeconds[i])/3600.0;
    buf << " " << std::setprecision(2) << h;
  }

  LOGC(TaTriggerLog::name()) << _name << " " << DateTime::strn(_gt)
			     << "+(" << buf.str() << ")";
}

//----------------------------------------------------------------
std::string 
DsEnsembleGenTrigger1::_sprintState(void) const
{
  string s;
  if (_hasData)
  {
    s = "Has data";
  }
  else
  {
    s = "No data";
  }

  char buf[1000];
  sprintf(buf, "%s    %s %s, nlead=%d", _name.c_str(), s.c_str(), 
	  DateTime::strn(_gt).c_str(), 
	  static_cast<int>(_availableLtSeconds.size()));
  std::string ret = buf;
  return ret;
}
