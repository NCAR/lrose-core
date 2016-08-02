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
 * @file DsUrlTriggerObject.cc
 */

#include <dsdata/DsUrlTrigger.hh>
#include <toolsa/LogStream.hh>
using std::string;

//----------------------------------------------------------------
DsUrlTrigger::DsUrlTriggerObject::DsUrlTriggerObject()
{
  _url = "unknown";
  _is_realtime = false;
}

//----------------------------------------------------------------
DsUrlTrigger::DsUrlTriggerObject::DsUrlTriggerObject(const string &trigger_url)
{
  _url = trigger_url;
  _is_realtime = true;
  _realtime = DsUrlTriggerRealtime(trigger_url);
}

//----------------------------------------------------------------
DsUrlTrigger::DsUrlTriggerObject::DsUrlTriggerObject(const time_t &t0,
						     const time_t &t1,
						     const string &trigger_url,
						     bool isSpdb)
{
  _url = trigger_url;
  _is_realtime = false;
  _archive = DsUrlTriggerArchive(trigger_url, isSpdb);
}

//----------------------------------------------------------------
DsUrlTrigger::DsUrlTriggerObject::~DsUrlTriggerObject()
{
}

//----------------------------------------------------------------
bool DsUrlTrigger::DsUrlTriggerObject::setNowait(void)
{
  if (_is_realtime)
  {
    _realtime.setNowait();
    return true;
  }
  else
  {
    LOG(ERROR) << "cannot set nowait when in archive mode";
    return false;
  }
}

//----------------------------------------------------------------
bool
DsUrlTrigger::DsUrlTriggerObject::setMaxValidAge(const int max_valid_age)
{
  if (_is_realtime)
  {
    _realtime.setMaxValidAge(max_valid_age);
    return true;
  }
  else
  {
    LOG(ERROR) << "cannot set max valid age when in archive mode";
    return false;
  }
}

//----------------------------------------------------------------
bool
DsUrlTrigger::DsUrlTriggerObject::nextData(const DsUrlTriggerSubsample &sub,
					   time_t &t, string &fname)
{
  if (!_is_realtime)
  {
    LOG(ERROR) << "only works in realtime mode";
    return false;
  }
  
  if (nextTime(sub, t))
  {
    fname = _realtime.currentFilename();
    return true;
  }
  else
  {
    return false;
  }
}

//----------------------------------------------------------------
bool DsUrlTrigger::DsUrlTriggerObject::rewind(void)
{
  if (_is_realtime)
  {
    LOG(ERROR) << "only works in archive mode";
    return false;
  }
  else
  {
    return _archive.rewind();
  }
}

