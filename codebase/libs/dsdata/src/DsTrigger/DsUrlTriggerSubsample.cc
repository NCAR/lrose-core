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
 * @file DsUrlTriggerSubsample.cc
 */
#include <dsdata/DsUrlTrigger.hh>
#include <toolsa/DateTime.hh>
#include <algorithm>
#include <cstdio>

//----------------------------------------------------------------
DsUrlTriggerSubsample::DsUrlTriggerSubsample()
{
}

//----------------------------------------------------------------
DsUrlTriggerSubsample::~DsUrlTriggerSubsample()
{
}

//----------------------------------------------------------------
void DsUrlTriggerSubsample::gentimeSubsamplingSet(const vector<int> &wanted)
{
  _trigger_minutes = wanted;
}

//----------------------------------------------------------------
void DsUrlTriggerSubsample::gentimeSubsamplingClear(void)
{
  _trigger_minutes.clear();
}

//----------------------------------------------------------------
void DsUrlTriggerSubsample::leadtimeSubsamplingActivate(const double lt0,
							const double lt1,
							const double dlt)
{
  // build the _lt_seconds vector
  _lt_seconds.clear();
  for (double min=lt0; min<=lt1; min+=dlt)
  {
    int s = static_cast<int>(min*60.0);
    _lt_seconds.push_back(s);
  }
}

//----------------------------------------------------------------
void DsUrlTriggerSubsample::leadtimeSubsamplingDeactivate(void)
{
  _lt_seconds.clear();
}

//----------------------------------------------------------------
bool DsUrlTriggerSubsample::timeOk(const time_t &t, const int lt) const
{
  if (!timeOk(t))
  {
    return false;
  }
  if (_lt_seconds.empty())
  {
    return true;
  }
  return (find(_lt_seconds.begin(), _lt_seconds.end(), lt) !=
	  _lt_seconds.end());
}

//----------------------------------------------------------------
bool DsUrlTriggerSubsample::timeOk(const time_t &t) const
{
  if (_trigger_minutes.empty())
  {
    return true;
  }
  DateTime d(t);
  int minute = d.getMin();
  return (find(_trigger_minutes.begin(), _trigger_minutes.end(), minute) !=
	  _trigger_minutes.end());
}

//----------------------------------------------------------------
bool DsUrlTriggerSubsample::leadtimesComplete(const vector<int> available_lt,
					      string &err) const
{
  bool stat = true;
  err = "missing lt:";
  // see if each wanted lead time is present or not
  for (size_t i=0; i<_lt_seconds.size(); ++i)
  {
    if (find(available_lt.begin(), available_lt.end(),
	     _lt_seconds[i]) == available_lt.end())
    {
      // this lead time is not present, fail
      stat = false;
      char buf[100];
      sprintf(buf, "%d ", _lt_seconds[i]);
      err += buf;
    }
  }
  return stat;
}
