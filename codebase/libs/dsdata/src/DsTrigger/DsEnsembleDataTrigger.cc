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
 * @file DsEnsembleDataTrigger.cc
 */
#include <dsdata/DsEnsembleDataTrigger.hh>
#include <dsdata/DsEnsembleLeadTrigger.hh>
#include <toolsa/LogStream.hh>
#include <cstdio>
using std::string;

//------------------------------------------------------------------
DsEnsembleDataTrigger::DsEnsembleDataTrigger(void) : 
  _trigger(NULL), _standard(true), _format("%02d"), _mode(SIMPLE)
{
}

//------------------------------------------------------------------
DsEnsembleDataTrigger::DsEnsembleDataTrigger(const std::string &urlHeader, 
					     int nmember,
					     std::vector<int> leadSeconds,
					     const std::string &format,
					     Trigger_mode_t mode) :
  _trigger(NULL), _standard(true), _urlHeader(urlHeader),
  _urlRemainder(""), _format(format), _mode(mode)
{
  _buildUrls(nmember);
  DsEnsembleLeadTrigger *lt;
  switch (_mode)
  {
  case LEADTIME:
    lt = new DsEnsembleLeadTrigger(_urls, leadSeconds);
    _trigger = dynamic_cast<DsEnsembleAnyTrigger *>(lt);
    break;
  case SIMPLE:
  default:
    _trigger = new DsEnsembleAnyTrigger(_urls, leadSeconds);
    break;
  }
}

//------------------------------------------------------------------
DsEnsembleDataTrigger::DsEnsembleDataTrigger(const std::string &urlHeader, 
					     const std::string &urlRemainder,
					     int nmember,
					     std::vector<int> leadSeconds,
					     const std::string &format,
					     Trigger_mode_t mode) :
  _trigger(NULL), _standard(false), _urlHeader(urlHeader),
  _urlRemainder(urlRemainder), _format(format), _mode(mode)
{
  _buildUrls(nmember);
  DsEnsembleLeadTrigger *lt;
  switch (_mode)
  {
  case LEADTIME:
    lt = new DsEnsembleLeadTrigger(_urls, leadSeconds);
    _trigger = dynamic_cast<DsEnsembleAnyTrigger *>(lt);
    break;
  case SIMPLE:
  default:
    _trigger = new DsEnsembleAnyTrigger(_urls, leadSeconds);
    break;
  }
}

//------------------------------------------------------------------
DsEnsembleDataTrigger::DsEnsembleDataTrigger(const time_t &t0, const time_t &t1,
					     const std::string &urlHeader, 
					     int nmember,
					     std::vector<int> leadSeconds,
					     const std::string &format,
					     Trigger_mode_t mode) :
  _trigger(NULL), _standard(true), _urlHeader(urlHeader),
  _urlRemainder(""), _format(format), _mode(mode)
{
  _buildUrls(nmember);
  DsEnsembleLeadTrigger *lt;
  switch (_mode)
  {
  case LEADTIME:
    lt = new DsEnsembleLeadTrigger(t0, t1, _urls, leadSeconds);
    _trigger = dynamic_cast<DsEnsembleAnyTrigger *>(lt);
    break;
  case SIMPLE:
  default:
    _trigger = new DsEnsembleAnyTrigger(t0, t1, _urls, leadSeconds);
    break;
  }
}

//------------------------------------------------------------------
DsEnsembleDataTrigger::DsEnsembleDataTrigger(const time_t &t0, const time_t &t1,
					     const std::string &urlHeader, 
					     const std::string &urlRemainder,
					     int nmember,
					     std::vector<int> leadSeconds,
					     const std::string &format,
					     Trigger_mode_t mode) :
  _trigger(NULL), _standard(false), _urlHeader(urlHeader),
  _urlRemainder(urlRemainder), _format(format), _mode(mode)
{
  _buildUrls(nmember);
  DsEnsembleLeadTrigger *lt;
  switch (_mode)
  {
  case LEADTIME:
    lt = new DsEnsembleLeadTrigger(t0, t1, _urls, leadSeconds);
    _trigger = dynamic_cast<DsEnsembleAnyTrigger *>(lt);
    break;
  case SIMPLE:
  default:
    _trigger = new DsEnsembleAnyTrigger(t0, t1, _urls, leadSeconds);
    break;
  }
}

//------------------------------------------------------------------
DsEnsembleDataTrigger::~DsEnsembleDataTrigger(void)
{
  if (_trigger != NULL)
  {
    delete _trigger;
    _trigger = NULL;
  }
}

//------------------------------------------------------------------
void DsEnsembleDataTrigger::setDebug(void)
{
  _trigger->setDebug();
}

//------------------------------------------------------------------
bool DsEnsembleDataTrigger::trigger(time_t &gt, int &lt, int &member)
{
  if (_mode != SIMPLE)
  {
    LOG(ERROR) << "Wrong triggering method, mode not SIMPLE";
    return false;
  }

  bool hasData=false;
  while (!hasData)
  {
    string url;
    if (!_trigger->nextTime(gt, lt, url, hasData))
    {
      return false;
    }
    if (hasData)
    {
      member = parseUrl(url);;
      return member >= 0;
    }
  }
  return false;
}

//------------------------------------------------------------------
bool DsEnsembleDataTrigger::trigger(time_t &gt, int &lt,
				    std::vector<int> &members,
				    bool &complete)
{
  members.clear();

  if (_mode != LEADTIME)
  {
    LOG(ERROR) << "Wrong triggering method, mode not LEADTIME";
    return false;
  }

  DsEnsembleLeadTrigger *t = dynamic_cast<DsEnsembleLeadTrigger *>(_trigger);
  std::vector<std::string> urls;
  bool status = t->nextTrigger(gt, lt, urls, complete);
  for (size_t j=0; j<urls.size(); ++j)
  {
    int m = parseUrl(urls[j]);
    if (m >= 0)
    {
      members.push_back(m);
    }
  }
  return status;
}

//------------------------------------------------------------------
int DsEnsembleDataTrigger::parseUrl(const std::string &url) const
{
  if (_standard)
  {
    // expect _urlHeader/e_<#>
    string header = _urlHeader + "/e_";
    string mstring = url.substr(header.size());
    // mstring = mstring.substr(0, mstring.size() - _urlRemainder.size());
    int member;
    if (sscanf(mstring.c_str(), _format.c_str(), &member) != 1)
    {
      LOG(ERROR) << "scanning for member number in URL:" <<  url;
      return -1;
    }
    else
    {
      return member;
    }
  }
  else
  {
    // gather up the stuff that should be the number
    string mstring = url.substr(_urlHeader.size());
    mstring = mstring.substr(0, mstring.size() - _urlRemainder.size());
    int member;
    if (sscanf(mstring.c_str(), _format.c_str(), &member) != 1)
    {
      LOG(ERROR) << "scanning for member number in URL:" <<  url;
      return -1;
    }
    else
    {
      return member;
    }
  }    
}
 
//------------------------------------------------------------------
void DsEnsembleDataTrigger::setMaximumAgeSeconds(const int maxSeconds)
{
  _trigger->setMaximumAgeSeconds(maxSeconds);
}

//------------------------------------------------------------------
void DsEnsembleDataTrigger::setTriggerMaxWaitSeconds(const int maxSeconds)
{
  _trigger->setTriggerMaxWaitSeconds(maxSeconds);
}

//------------------------------------------------------------------
void DsEnsembleDataTrigger::setMaxSecondsBeforeTimeout(const int maxSeconds)
{
  if (_mode != LEADTIME)
  {
    LOG(WARNING) << "Not in LEADTIME mode, ignored";
    return;
  }
  DsEnsembleLeadTrigger *t = dynamic_cast<DsEnsembleLeadTrigger *>(_trigger);
  t->setMaxSecondsBeforeTimeout(maxSeconds);
}

//------------------------------------------------------------------
void DsEnsembleDataTrigger::setMaxSecondsBeforeDisable(const int maxSeconds)
{
  if (_mode != LEADTIME)
  {
    LOG(WARNING) << "Not in LEADTIME mode, ignored";
    return;
  }
  DsEnsembleLeadTrigger *t = dynamic_cast<DsEnsembleLeadTrigger *>(_trigger);
  t->setMaxSecondsBeforeDisable(maxSeconds);
}

//------------------------------------------------------------------
void DsEnsembleDataTrigger::setPersistantDisable(const bool status)
{
  if (_mode != LEADTIME)
  {
    LOG(WARNING) << "Not in LEADTIME mode, ignored";
    return;
  }
  DsEnsembleLeadTrigger *t = dynamic_cast<DsEnsembleLeadTrigger *>(_trigger);
  t->setPersistantDisable(status);
}

//------------------------------------------------------------------
void DsEnsembleDataTrigger::filterNames(const std::string &remove)
{
  _trigger->filterNames(remove);
}

//------------------------------------------------------------------
std::string DsEnsembleDataTrigger::memberName(int member) const
{
  if (_standard)
  {
    return memberName(_urlHeader, member, _format);
  }
  else
  {
    return memberName(_urlHeader, _urlRemainder, member, _format);
  }
}

//------------------------------------------------------------------
std::string DsEnsembleDataTrigger::memberName(const std::string &urlHeader, 
					      int member,
					      const std::string &format)
{
  char buf[100];
  sprintf(buf, format.c_str(), member);
  string url = urlHeader + "/e_";
  url += buf;
  return url;
}

//------------------------------------------------------------------
std::string DsEnsembleDataTrigger::memberName(const std::string &urlHeader, 
					      const std::string &urlRemainder,
					      int member,
					      const std::string &format)
{
  char buf[100];
  sprintf(buf, format.c_str(), member);
  string url = urlHeader + buf;
  url += urlRemainder;
  return url;
}

//------------------------------------------------------------------
void DsEnsembleDataTrigger::_buildUrls(int nmember) 
{
  _urls.clear();
  for (int i=0; i<nmember; ++i)
  {
    _urls.push_back(memberName(i+1));
  }
}

