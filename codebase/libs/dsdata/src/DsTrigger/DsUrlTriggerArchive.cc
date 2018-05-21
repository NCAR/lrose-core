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
 * @file DsUrlTriggerArchive.cc
 */
#include <dsdata/DsUrlTrigger.hh>
#include <toolsa/TaTriggerLog.hh>
#include <toolsa/LogStream.hh>
#include <toolsa/DateTime.hh>
#include <Mdv/DsMdvx.hh>
#include <Spdb/DsSpdb.hh>

//----------------------------------------------------------------
DsUrlTrigger::DsUrlTriggerObject::DsUrlTriggerArchive::DsUrlTriggerArchive()
{
  _url = "none";
  _isSpdb = false;
}

//----------------------------------------------------------------
DsUrlTrigger::DsUrlTriggerObject::
DsUrlTriggerArchive::DsUrlTriggerArchive(const string &url, bool isSpdb)
{
  _url = url;
  _isSpdb = isSpdb;
}

//----------------------------------------------------------------
DsUrlTrigger::DsUrlTriggerObject::DsUrlTriggerArchive::~DsUrlTriggerArchive()
{
}

//----------------------------------------------------------------
bool DsUrlTrigger::DsUrlTriggerObject::
DsUrlTriggerArchive::next(const DsUrlTriggerSubsample &s, time_t &t, int &lt)
{
  while (_times_index != _times.end())
  {
    t = _times_index->_gt;
    lt = _times_index->_lt;
    ++_times_index;
    if (s.timeOk(t, lt))
    {
      return true;
    }
    else
    {
      LOGC(TaTriggerLog::name()) << DateTime::strn(t) << " not wanted";
    }
  }    
  LOGC(TaTriggerLog::name()) << "no more archive mode times";
  return false;
}

//----------------------------------------------------------------
void DsUrlTrigger::DsUrlTriggerObject::
DsUrlTriggerArchive::initObs(const time_t &t0, const time_t &t1)
{
  if (_isSpdb)
  {
    _initObsSpdb(t0, t1);
  }
  else
  {
    _initObsMdv(t0, t1);
  }
}


//----------------------------------------------------------------
void DsUrlTrigger::DsUrlTriggerObject::
DsUrlTriggerArchive::initFcstGen(const time_t &t0, const time_t &t1)
{
  // not sure this is why it is
  initFcstLead(t0, t1);
}

//----------------------------------------------------------------
void DsUrlTrigger::DsUrlTriggerObject::
DsUrlTriggerArchive::initFcstLead(const time_t &t0, const time_t &t1)
{
  if (_isSpdb)
  {
    _initFcstLeadSpdb(t0, t1);
  }
  else
  {
    _initFcstLeadMdv(t0, t1);
  }
}

//----------------------------------------------------------------
bool DsUrlTrigger::DsUrlTriggerObject::DsUrlTriggerArchive::rewind(void)
{
  if (_times.empty())
  {
    LOG(WARNING) << "archive times values empty";
  }
  _times_index = _times.begin();
  return true;
}


//----------------------------------------------------------------
void DsUrlTrigger::DsUrlTriggerObject::
DsUrlTriggerArchive::_initObsMdv(const time_t &t0, const time_t &t1)
{
  DsMdvx D;

  // one day out is plenty.
  D.setTimeListModeValid(_url, t0, t1);
  D.compileTimeList();

  // get all the times (obs data times)
  vector<time_t> v = D.getTimeList();

  // construct the data from that with lead times all 0
  for (size_t i=0; i<v.size(); ++i)
  {
    if (v[i] > t1 || v[i] < t0)
    {
      continue;
    }
    DsFcstTime f(v[i], 0);
    _times.push_back(f);
  }

  // sort into increasing order
  _times.sort(DsFcstTime::lessOrEqual);

  // init the index
  _times_index = _times.begin();
}

//----------------------------------------------------------------
void DsUrlTrigger::DsUrlTriggerObject::
DsUrlTriggerArchive::_initObsSpdb(const time_t &t0, const time_t &t1)
{
  DsSpdb D;
  _times.clear();

  // one day out is plenty.
  if (D.compileTimeList(_url, t0, t1) != 0)
  {
    // nothing, silent for now
    return;
  }

  // get all the times (obs data times)
  vector<time_t> v = D.getTimeList();

  // construct the data from that with lead times all 0
  for (size_t i=0; i<v.size(); ++i)
  {
    if (v[i] > t1 || v[i] < t0)
    {
      continue;
    }
    DsFcstTime f(v[i], 0);
    _times.push_back(f);
  }

  // sort into increasing order
  _times.sort(DsFcstTime::lessOrEqual);

  // init the index
  _times_index = _times.begin();
}

//----------------------------------------------------------------
void DsUrlTrigger::DsUrlTriggerObject::
DsUrlTriggerArchive::_initFcstLeadMdv(const time_t &t0, const time_t &t1)
{
  DsMdvx D;

  // 7 days out is plenty, assumes at most a forecast lead time of one week
  D.setTimeListModeValidMultGen(_url, t0, t1 + 7*24*3600);
  D.compileTimeList();

  // get all the gen and valid times
  vector<time_t> gt = D.getGenTimes();
  vector<time_t> v = D.getTimeList();

  // construct the data from these
  for (size_t i=0; i<gt.size(); ++i)
  {
    if (gt[i] > t1 || gt[i] < t0)
    {
      continue;
    }
    DsFcstTime f(gt[i], v[i]-gt[i]);
    _times.push_back(f);
  }

  // sort into increasing order
  _times.sort(DsFcstTime::lessOrEqual);

  // initialize the index
  _times_index = _times.begin();
}

//----------------------------------------------------------------
void DsUrlTrigger::DsUrlTriggerObject::
DsUrlTriggerArchive::_initFcstLeadSpdb(const time_t &t0, const time_t &t1)
{
  DsSpdb D;
  _times.clear();

  // get all data in the interval, namely all data for which 
  // gen time = (time - data_type2) is in range (time = valid time, data_type2
  // is lead time).
  // Assuming lead times cannot be negative, and assuming lead times
  // are less than 1 week at max, set bounds appropriately.  
  // (Make 1 week some kind of adjustable member, maybe later... dave albo)

  time_t min_valid_time = t0;
  time_t max_valid_time = t1 + 7*24*3600;

  if (D.getInterval(_url, min_valid_time, max_valid_time) != 0)
  {
    // return silently
  }

  // for each thing found, figure out gen and lead time and add in, if
  // gen time is in range.
  const vector<Spdb::chunk_t> &chunks = D.getChunks();
  int nChunks = static_cast<int>(chunks.size());
  for (int ii = 0; ii < nChunks; ii++)
  {
    time_t validtime = chunks[ii].valid_time;
    int lt = chunks[ii].data_type2;
    time_t gentime = validtime - lt;
    if (gentime >= t0 && gentime <= t1)
    {
      DsFcstTime f(gentime, lt);
      _times.push_back(f);
    }
  }

  // sort into increasing order
  _times.sort(DsFcstTime::lessOrEqual);
  
#ifdef NOTNOW
  printf("Times:\n");
  
  std::list <DsFcstTime>::const_iterator i;
  for (i=_times.begin(); i!=_times.end(); ++i)
  {
    i->print();
  }
#endif

  // initialize the index
  _times_index = _times.begin();
}

