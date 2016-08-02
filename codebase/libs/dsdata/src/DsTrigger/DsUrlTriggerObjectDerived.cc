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
 * @file DsDsUrlTriggerObjectDerived.cc
 */

#include <dsdata/DsUrlTrigger.hh>
#include <Mdv/DsMdvx.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/pmu.h>
#include <toolsa/LogStream.hh>
#include <toolsa/TaTriggerLog.hh>
#include <unistd.h>
#include <list>
#include <algorithm>
#include <sstream>

using std::string;

// one day
#define LOOKAHEAD_SECONDS (24*3600)

// recent lookback (2 hours)
#define LOOKBACK_SHORT_SECONDS (7200)

// long lookback (1 week)
#define LOOKBACK_LONG_SECONDS  (7*24*3600)

// minimum number of minutes recent lookback has been stable (on startup)
// when longer lookback has more lead times before accepting the shorter
// number of leads.
#define MIN_STABILITY_MINUTES (90.0)

// # of triggering cycles with a smaller # of leads needed to accept
#define MIN_SHORTER_LEAD_CYCLES 4

//----------------------------------------------------------------
static string _buildErrorMessage(const vector<int> &ivt, 
				 const vector<int> &fcst_lt)
{
  string ret="";
  if (ivt.size() != fcst_lt.size())
  {
    std::ostringstream buf;
    buf << "Wanted " << fcst_lt.size() << " leads got " << ivt.size();
    ret = buf.str();
  }
  else
  {
    ret = "Inconsistent lead times between wanted and recieved";
  }
  return ret;
}

//----------------------------------------------------------------
static vector<int> _availableLeadTimes(const string &url, const time_t &gt)
{
  DsMdvx D;
  D.setTimeListModeForecast(url, gt);
  D.compileTimeList();
  vector<time_t> vt = D.getValidTimes();
  // make vector of lead seconds from this
  vector<int> ivt;
  for (size_t i=0; i<vt.size(); ++i)
  {
    ivt.push_back(vt[i]-gt);
  }
  return ivt;
}

//----------------------------------------------------------------
static int _numAvailableLeadTimes(const string &url, const time_t &gt)
{
  vector<int> vt = _availableLeadTimes(url, gt);
  return static_cast<int>(vt.size());
}

//----------------------------------------------------------------
static vector<int> _leadTimesAtGentime(const list <DsFcstTime> &times,
				       const time_t &gt)
{
  list<DsFcstTime>::const_iterator i;
  vector<int> ret;
  for (i=times.begin(); i!=times.end(); ++i)
  {
    if (i->_gt == gt)
    {
      ret.push_back(i->_lt);
    }
  }
  return ret;
}

//----------------------------------------------------------------
static double _nminStableNleadTimes(const time_t &gt_max, 
				    const vector<time_t> &gt,
				    const list<DsFcstTime> &times)
{
  // for each gentime, build up the lead times and look for longest list,
  // moving backwards
  vector<time_t>::const_reverse_iterator igt;
  bool in_run = false;
  int wanted_nlead = 0;
  double nmin = 0;
  for (igt=gt.rbegin(); igt!=gt.rend(); ++igt)
  {
    if (*igt == gt_max)
    {
      in_run = true;
      vector<int> lt = _leadTimesAtGentime(times, *igt);
      wanted_nlead = static_cast<int>(lt.size());
      nmin = 0;
    }
    else if (*igt < gt_max)
    {
      if (in_run)
      {
	vector<int> lt = _leadTimesAtGentime(times, *igt);
	if (static_cast<int>(lt.size()) != wanted_nlead)
	{
	  break;
	}
	else
	{
	  nmin = static_cast<double>(gt_max - *igt)/60.0;
	}
      }
    }
  }
  return nmin;
}

//----------------------------------------------------------------
static vector<int> _longestLeadtimeList(const time_t &t0, const time_t &t1,
					const vector<time_t> &gt,
					const list<DsFcstTime> &times,
					time_t &gt_best)
{
  vector<int> lt_best;

  // for each gentime, build up the lead times and look for longest list,
  // moving backwards
  vector<time_t>::const_reverse_iterator igt;
  gt_best = -1;
  for (igt=gt.rbegin(); igt!=gt.rend(); ++igt)
  {
    if (*igt >= t0 && *igt <= t1)
    {
      vector<int> lt = _leadTimesAtGentime(times, *igt);
      if (lt.size() > lt_best.size())
      {
	gt_best = *igt;
	lt_best = lt;
      }
    }
  }
  return lt_best;
}

//----------------------------------------------------------------
static vector<time_t> _uniqueGentimes(const list <DsFcstTime> &times)
{
  list<DsFcstTime>::const_iterator i;
  vector<time_t> ret;
  for (i=times.begin(); i!=times.end(); ++i)
  {
    if (find(ret.begin(), ret.end(), i->_gt) == ret.end())
    {
      ret.push_back(i->_gt);
    }
  }
  return ret;
}

//----------------------------------------------------------------
static list <DsFcstTime> _allTimes(const string &url, const time_t &t0,
				   const time_t &t1)
{
  DsMdvx D;
  list<DsFcstTime> times;

  // retrive all the gen/valid times and store gen/lead to times list
  D.setTimeListModeValidMultGen(url, t0, t1);
  D.compileTimeList();
  vector<time_t> gt = D.getGenTimes();
  vector<time_t> v = D.getTimeList();
  for (size_t i=0; i<gt.size(); ++i)
  {
    if (gt[i] > t1 || gt[i] < t0)
    {
      continue;
    }
    DsFcstTime f(gt[i], v[i]-gt[i]);
    times.push_back(f);
  }

  // sort into increasing order
  times.sort(DsFcstTime::lessOrEqual);
  return times;
}


//----------------------------------------------------------------
DsUrlTrigger::DsUrlTriggerObs::DsUrlTriggerObs() : DsUrlTriggerObject()
{
}

//----------------------------------------------------------------
DsUrlTrigger::DsUrlTriggerObs::DsUrlTriggerObs(const string &trigger_url) :
  DsUrlTriggerObject(trigger_url)
{
}

//----------------------------------------------------------------
DsUrlTrigger::DsUrlTriggerObs::DsUrlTriggerObs(const time_t &t0,
					       const time_t &t1,
					       const string &trigger_url,
					       bool isSpdb) :
  DsUrlTriggerObject(t0, t1, trigger_url, isSpdb)
{
}

//----------------------------------------------------------------
DsUrlTrigger::DsUrlTriggerObs::~DsUrlTriggerObs()
{
}

//----------------------------------------------------------------
bool DsUrlTrigger::DsUrlTriggerObs::nextTime(const DsUrlTriggerSubsample &sub,
					     time_t &t)
{
  int lt;

  // make a copy of subsampling and set full lead times just in
  // case there is leadtime subsampling that will prevent triggering
  // (with assumed obs lead time = 0)
  DsUrlTriggerSubsample s(sub);
  s.leadtimeSubsamplingDeactivate();

  if (_is_realtime)
  {
    return _realtime.next(s, t, lt);
  }
  else
  {
    return _archive.next(s, t, lt);
  }
}

//----------------------------------------------------------------
bool DsUrlTrigger::DsUrlTriggerObs::nextTime(const DsUrlTriggerSubsample &sub,
					     time_t &t, int &lt)
{
  LOG(WARNING) << "wrong method for Obs triggering";
  return false;
}

//----------------------------------------------------------------
void DsUrlTrigger::DsUrlTriggerObs::archiveInit(const time_t &t0,
						const time_t &t1)
{
  _archive.initObs(t0, t1);
}

//----------------------------------------------------------------
DsUrlTrigger::DsUrlTriggerFcstGen::DsUrlTriggerFcstGen() : DsUrlTriggerObject()
{
  _last_gt = -1;
  _current_gt = -1;
}

//----------------------------------------------------------------
DsUrlTrigger::
DsUrlTriggerFcstGen::DsUrlTriggerFcstGen(const string &trigger_url) :
  DsUrlTriggerObject(trigger_url)
{
  _last_gt = -1;
  _current_gt = -1;
  _initFcstAll();
}

//----------------------------------------------------------------
DsUrlTrigger::
DsUrlTriggerFcstGen::DsUrlTriggerFcstGen(const time_t &t0, const time_t &t1,
					 const string &trigger_url,
					 bool isSpdb) :
  DsUrlTriggerObject(t0, t1, trigger_url, isSpdb)
{
  _last_gt = -1;
  _current_gt = -1;
}

//----------------------------------------------------------------
DsUrlTrigger::DsUrlTriggerFcstGen::~DsUrlTriggerFcstGen()
{
}

//----------------------------------------------------------------
bool DsUrlTrigger::
DsUrlTriggerFcstGen::nextTime(const DsUrlTriggerSubsample &sub, time_t &t)
{
  int lt;
  bool first = true;
  vector<pair<time_t, int> > gt_nlt;
  if (_is_realtime)
  {
    while (true)
    {
      // trigger next t/lt
      if (!_realtime.next(sub, t, lt))
      {
	// no more to trigger
	return false;
      }
      LOGC(TaTriggerLog::name()) << "Triggered " << DateTime::strn(t) 
				 << "+" << lt;
      if (first)
      {
	first = false;
	_current_gt = t;
	LOGC(TaTriggerLog::name()) << "setting current_gt to input";
      }
      else
      {
	LOGC(TaTriggerLog::name()) << "not first time not setting current_gt";
      }

      time_t t_return;
      bool keep_first;
      if (_checkTime(sub, t, lt, gt_nlt, t_return, keep_first))
      {
	t = t_return;
	return true;
      }
      if (keep_first)
      {
	first = true;
      }
    }
  }

  else
  {
    while (true)
    {
      if (!_archive.next(sub, t, lt))
      {
	return false;
      }
      if (_last_gt != t)
      {
	// a new gen time
	_last_gt = t;
	return true;
      }
    }
  }
}

//----------------------------------------------------------------
bool DsUrlTrigger::
DsUrlTriggerFcstGen::nextTime(const DsUrlTriggerSubsample &sub, time_t &t,
			      int &lt)
{
  LOG(WARNING) << "wrong method for FcstGen triggering";
  return false;
}

//----------------------------------------------------------------
void DsUrlTrigger::DsUrlTriggerFcstGen::archiveInit(const time_t &t0,
						    const time_t &t1)
{
  _archive.initFcstGen(t0, t1);
}

//----------------------------------------------------------------
void DsUrlTrigger::DsUrlTriggerFcstGen::_initFcstAll(void)
{
  while (!_setLeadTimesFromData())
  {
    LOG(ERROR) << "Failed to set lead times " << _url;
    PMU_auto_register("_init_fcst_all");
    sleep(10);
  }
}

//----------------------------------------------------------------
bool DsUrlTrigger::DsUrlTriggerFcstGen::_setLeadTimesFromData(void)
{
  // go back and forward (to capture lead times), build up all times
  time_t tnow = time(0);
  time_t t1 = tnow + LOOKAHEAD_SECONDS;
  time_t t0 = tnow - LOOKBACK_LONG_SECONDS;
  list <DsFcstTime> times = _allTimes(_url, t0, t1);;
  list<DsFcstTime>::iterator i;
  // get the set of unique gen times
  vector<time_t> gt = _uniqueGentimes(times);
  if (static_cast<int>(gt.size()) < 3)
  {
    // wait for 3 distinct gen times, use largest of
    // them (1st might be incomplete, last might also, so middle one)
    LOG(ERROR) << "need at least 3 unique gentimes, have only " << gt.size();
    return false;
  }
  
  // get the longest lead time list total
  time_t gt_best;
  vector<int> lt_best = _longestLeadtimeList(t0, t1, gt, times, gt_best);

  // get longest lead time list in recent past
  time_t gt_best_recent;
  time_t tmax = *gt.rbegin();
  vector<int> lt_best_recent = _longestLeadtimeList(tmax-LOOKBACK_SHORT_SECONDS,
						    tmax, gt, times, 
						    gt_best_recent);

  int nbest = static_cast<int>(lt_best.size());
  int nbest_recent = static_cast<int>(lt_best_recent.size());
  if (nbest_recent == 0 && nbest == 0)
  {
    LOG(ERROR) << "no lead times when there should be";
    return false;
  }
  if (nbest_recent > 0 && nbest == 0)
  {
    LOG(ERROR) << "no lead times when there should be";
    return false;
  }
  if (nbest_recent == 0 && nbest > 0)
  {
    // nothing recent go with older stuff
    return _setLeadTimesFromData(lt_best, gt_best);
  }

  if (nbest_recent >= nbest)
  {
    // recent is bigger than past stuff (or same)
    // go with recent stuff
    return _setLeadTimesFromData(lt_best_recent, gt_best_recent);
  }

  // here when recent is smaller than past stuff.
  double nstable_min = _nminStableNleadTimes(gt_best_recent, gt, times);
  LOG(WARNING) << nbest_recent << " lead times at " 
	       << DateTime::strn(gt_best_recent);
  LOG(WARNING) << nbest << " lead times at " 
	       << DateTime::strn(gt_best);
  LOG(DEBUG) << "stable for " << nstable_min << " minutes";
  if (nstable_min >= MIN_STABILITY_MINUTES)
  {
    LOG(DEBUG) << "stability is long enough to switch";
    return _setLeadTimesFromData(lt_best_recent, gt_best_recent);
  }
  else
  {
    LOG(WARNING) << "Wait till stable for " 
		 << MIN_STABILITY_MINUTES << " minutes";
    return false;
  }
}

//----------------------------------------------------------------
bool DsUrlTrigger::
DsUrlTriggerFcstGen::_checkTime(const DsUrlTriggerSubsample &sub,
				const time_t &t, const int lt,
				vector<pair<time_t,int> > &gt_nlt,
				time_t &tret, bool &keep_first)
{
  if (t == _last_gt)
  {
    // this is the same gen time already returned (already complete).
    LOGC(TaTriggerLog::name()) << "Gen time already returned";
    keep_first = true;
    // check if number of lead times seems to be going up
    vector<int> vt = _availableLeadTimes(_url, _last_gt);
    if (vt.size() > _fcst_lt.size())
    {
      LOG(ERROR) << "number of leads increasing, redo lead times";
      _setLeadTimesFromData(vt, _last_gt);
      // in case it grows some more, or get a new gen time
      return false;
    }
    else
    {
      // waiting for a new gen time
      LOGC(TaTriggerLog::name()) << "wait for new gen";
      return false;
    }
  }
  else
  {
    keep_first = false;
  }
  
  if (t != _current_gt)
  {
    // never got all lead times, and now have a new gen time
    LOG(ERROR) << "Incomplete gen time %s" << DateTime::strn(_current_gt);
    LOG(ERROR) << _error_message;
    // store the # of leads and see if it is a new stable situation
    int nlt = _numAvailableLeadTimes(_url, _current_gt);
    gt_nlt.push_back(pair<time_t, int>(_current_gt, nlt));
    if (_adjustLeadtimesIfStable(gt_nlt))
    {
      gt_nlt.clear();
      tret = _current_gt;
      _current_gt = t;
      return true;
    }
    else
    {
      _current_gt = t;
      return false;
    }
  }

  LOGC(TaTriggerLog::name()) << "time = current_gt..work in progress";
    
  // here t is something we are working on now.
  if (_gentimeIsComplete(sub, t))
  {
    _last_gt = t;
    tret = t;
    LOGC(TaTriggerLog::name()) << "this is it";
    return true;
  }
  else
  {
    LOGC(TaTriggerLog::name()) << "not complete";
    return false;
  }
}

//----------------------------------------------------------------
bool DsUrlTrigger::
DsUrlTriggerFcstGen::_adjustLeadtimesIfStable(const vector<pair<time_t,int> >
					      &gt_nlt)
{
  int n = static_cast<int>(gt_nlt.size());
  if (n < MIN_SHORTER_LEAD_CYCLES)
  {
    LOG(WARNING) << "Stable for " << n << "gen time cycles, want at least " 
		 << MIN_SHORTER_LEAD_CYCLES;
    return false;
  }
  
  int nlt=gt_nlt[n-1].second;
  int j, i;
  for (i=n-2,j=1; i>=0; --i,++j)
  {
    int nlti = gt_nlt[i].second;
    if (nlti != nlt)
    {
      LOG(WARNING) << "Stable for " << j << "gen time cycles, want at least " 
		   << MIN_SHORTER_LEAD_CYCLES;
      return false;
    }
    if (++j >= MIN_SHORTER_LEAD_CYCLES)
    {
      break;
    }
  }
  // same # of leads for 4 or more times that is it.
  // get the lead times
  vector<int> ivt = _availableLeadTimes(_url, gt_nlt[0].first);
  LOG(DEBUG) << "Stable for " << gt_nlt.size() << " gen time cycles, "
	     << nlt << " lead times, adjusting now";
  return _setLeadTimesFromData(ivt, gt_nlt[0].first);
}
  
//----------------------------------------------------------------
bool DsUrlTrigger::
DsUrlTriggerFcstGen::_setLeadTimesFromData(const vector<int> &lt,
					   const time_t &gt)
{
  if (lt.size() < 2)
  {
    LOG(WARNING) << "need at least 2 lead times to compute lead time set, got "
		 << lt.size() << " at " << DateTime::strn(gt);
    return false;
  }
  int fcst_dt0 = lt[0];
  int fcst_dt = lt[1] - lt[0];
  int nfcst = static_cast<int>(lt.size());
  LOG(DEBUG) << "set dt0=" << fcst_dt0 << " dt=" << fcst_dt << " n=" << nfcst
	     << " from data at gen_time=" << DateTime::strn(gt);
  _fcst_lt.clear();
  for (int i=0; i<nfcst; ++i)
  {
    _fcst_lt.push_back(fcst_dt0 + i*fcst_dt);
  }
  return true;
}

//----------------------------------------------------------------
bool DsUrlTrigger::
DsUrlTriggerFcstGen::_gentimeIsComplete(const DsUrlTriggerSubsample &s,
					const time_t &t)
{
  vector<int> ivt = _availableLeadTimes(_url, t);

  int nnew = static_cast<int>(ivt.size());
  int nold = static_cast<int>(_fcst_lt.size());
  if (nnew > nold)
  {
    LOG(WARNING) << "Number of leads went from " << nold << " to " << nnew 
		 << ",rebuilding lead list";

    // if the # of leadtimes has increased, need to see what happens
    // next, meanwhile build up what we have as the list
    _setLeadTimesFromData(ivt, t);
    return false;
  }

  // if lead time subsampling, see if have those, otherwise see if
  // have all lead times.
  bool ret;
  if (s.isLeadSubsampling())
  {
    ret = s.leadtimesComplete(ivt, _error_message);
  }
  else
  {
    ret = (ivt == _fcst_lt);
    if (!ret)
    {
      _error_message = _buildErrorMessage(ivt, _fcst_lt);
    }
  }
  if (ret)
  {
    LOGC(TaTriggerLog::name()) << "Is complete";
    return true;
  }
  else
  {
    LOGC(TaTriggerLog::name()) << "Not yet complete";
    return false;
  }
}



//----------------------------------------------------------------
DsUrlTrigger::
DsUrlTriggerFcstLead::DsUrlTriggerFcstLead() : DsUrlTriggerObject()
{
}

//----------------------------------------------------------------
DsUrlTrigger::
DsUrlTriggerFcstLead::DsUrlTriggerFcstLead(const string &trigger_url) :
  DsUrlTriggerObject(trigger_url)
{
}

//----------------------------------------------------------------
DsUrlTrigger::
DsUrlTriggerFcstLead::DsUrlTriggerFcstLead(const time_t &t0, const time_t &t1,
					   const string &trigger_url,
					   bool isSpdb) : 
  DsUrlTriggerObject(t0, t1, trigger_url, isSpdb)
{
}

//----------------------------------------------------------------
DsUrlTrigger::DsUrlTriggerFcstLead::~DsUrlTriggerFcstLead()
{
}

//----------------------------------------------------------------
bool
 DsUrlTrigger::DsUrlTriggerFcstLead::nextTime(const DsUrlTriggerSubsample &sub,
					      time_t &t)
{
  int lt;
  // return when there is anything triggered, no matter what the gen time
  // is, ignoring the lead time
  return nextTime(sub, t, lt);
}

//----------------------------------------------------------------
bool DsUrlTrigger::
DsUrlTriggerFcstLead::nextTime(const DsUrlTriggerSubsample &sub,
			       time_t &t, int &lt)
{
  if (_is_realtime)
  {
    return _realtime.next(sub, t, lt);
  }
  else
  {
    return _archive.next(sub, t, lt);
  }
}

//----------------------------------------------------------------
void DsUrlTrigger::DsUrlTriggerFcstLead::archiveInit(const time_t &t0,
						     const time_t &t1)
{
  _archive.initFcstLead(t0, t1);
}

