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
#include <toolsa/copyright.h>
/**
 * @file LogMsg.cc
 */

//----------------------------------------------------------------
#include <cstdarg>
#include <string>
#include <vector>
#include <cstdio>
#include <toolsa/LogMsg.hh>
#include <toolsa/DateTime.hh>

#define ARRAY_LEN_LONG 1024

using std::string;
using std::vector;
using std::pair;

/**
 * A buffer in which to accumulate log info prior to logging
 */
static string sAccum="";

/**
 * A static pointer used to make this a singleton class
 */
static LogMsg *pLogMsg = NULL;

//----------------------------------------------------------------
static void _log(const string &s)
{
  printf("%s\n", s.c_str());
}

//----------------------------------------------------------------
void LogMsg::initPointer(void)
{
  if (pLogMsg == NULL)
  {
    pLogMsg = new LogMsg();
  }
}

//----------------------------------------------------------------
void LogMsg::freePointer(void)
{
  if (pLogMsg != NULL)
  {
    delete pLogMsg;
    pLogMsg = NULL;
  }
}

//----------------------------------------------------------------
LogMsg *LogMsg::getPointer(void)
{
  initPointer();
  return pLogMsg;
}

//----------------------------------------------------------------
LogMsg::LogMsg() :
  pLogRealTime(true),
  pLogClassAndMethod(true),
  pLogShowAllKeys(true)
{
  for (int i=0; i<NUM; ++i)
  {
    pSeverityEnabled[i] = true;
  }

  // turn off the ones that default to not set
  setSeverityLogging(LogMsg::DEBUG_VERBOSE, false);
  setSeverityLogging(LogMsg::TRIGGER, false);
  setSeverityLogging(LogMsg::THREAD, false);
  setSeverityLogging(LogMsg::SPECIAL, false);

  pthread_mutex_init(&_printMutex, NULL);
}

//----------------------------------------------------------------
LogMsg::~LogMsg()
{
  pthread_mutex_destroy(&_printMutex);
}

//----------------------------------------------------------------
void LogMsg::log(const std::string &fname, const int line,
                 const string &method, const Severity_t severity,
                 const string &msg)
{
  if (!pSeverityEnabled[static_cast<int>(severity)])
  {
    return;
  }

  string lmsg;
  
  if (pLogShowAllKeys)
  {
    lmsg = pSeverityStringAll(severity);
  }
  else
  {
    lmsg = pSeverityString(severity);
  }

  lmsg += pHeader(fname, line, method, severity);
  lmsg += msg;
  pLog(lmsg);
}

//----------------------------------------------------------------
void LogMsg::logf(const std::string &fname, const int line,
                  const string &method,const  Severity_t severity, 
                  string format, ...)
{
  if (!pSeverityEnabled[static_cast<int>(severity)])
  {
    return;
  }

  string msg;
  
  if (pLogShowAllKeys)
  {
    msg = pSeverityStringAll(severity);
  }
  else
  {
    msg = pSeverityString(severity);
  }

  msg += pHeader(fname, line, method, severity);

  va_list args;
  va_start( args, format );
  char buf[ARRAY_LEN_LONG];
  vsprintf( buf, format.c_str(), args );
  msg += buf;
  va_end( args );
  pLog(msg);
}

//----------------------------------------------------------------
void LogMsg::init(const bool debug, const bool debugVerbose,
		  const bool realtime, const bool file)
{
  setSeverityLogging(LogMsg::DEBUG, debug);
  setSeverityLogging(LogMsg::DEBUG_VERBOSE, debugVerbose);
  setLoggingTimestamp(realtime);
  setLoggingClassAndMethod(file);
}

//----------------------------------------------------------------
void LogMsg::setSeverityLogging(const Severity_t severity, const bool state)
{ 
  if (severity == FORCE || severity == PRINT)
  {
    // take no action
    return;  
  }
  pSeverityEnabled[severity] = state;
}

//----------------------------------------------------------------
void LogMsg::setLoggingTimestamp(const bool state)
{
  pLogRealTime = state;
}

//----------------------------------------------------------------
void LogMsg::setLoggingClassAndMethod(const bool state)
{
  pLogClassAndMethod = state;
}

void LogMsg::setLoggingShowAllSeverityKeys(const bool state)
{
  pLogShowAllKeys = state;
}

//----------------------------------------------------------------
bool LogMsg::isEnabled(const Severity_t severity) const
{
  return pSeverityEnabled[severity];
}

//----------------------------------------------------------------
void LogMsg::accumulate(const string &msg) const
{
  sAccum += msg;
}

//----------------------------------------------------------------
void LogMsg::accumulatef(string format, ...) const
{
  va_list args;
  va_start( args, format );
  char buf[ARRAY_LEN_LONG];
  vsprintf( buf, format.c_str(), args );
  sAccum += buf;
  va_end( args );
}

//----------------------------------------------------------------
void LogMsg::logAccum(const std::string &fname, const int line,
		       const string &method, const Severity_t severity)
{
  if (!pSeverityEnabled[severity])
  {
    sAccum.clear();
    return;
  }

  string msg;
  if (pLogShowAllKeys)
  {
    msg = pSeverityStringAll(severity);
  }
  else
  {
    msg = pSeverityString(severity);
  }
  msg += pHeader(fname, line, method, severity);
  msg += sAccum;
  sAccum.clear();
  pLog(msg);
}

//----------------------------------------------------------------
void LogMsg::setVerbose(void)
{
  setSeverityLogging(LogMsg::DEBUG_VERBOSE, true);
}

//----------------------------------------------------------------
void LogMsg::clearVerbose(void)
{
  setSeverityLogging(LogMsg::DEBUG_VERBOSE, false);
}

//----------------------------------------------------------------
void LogMsg::setTempSeverityLogging(const Severity_t severity, const bool state)
{
  if (severity == PRINT || severity == FORCE)
  {
    // take no action
    return;
  }

  // done like this so you can have more than one set (into a vector)
  // Get this one in there. If already there, set its state to input
  // if not, add it.
  bool b = isEnabled(severity);
  vector<pair<Severity_t, bool> >::iterator i;
  bool done = false;
  for (i = pTempOriginalState.begin(); i!= pTempOriginalState.end(); ++i)
  {
    if (i->first == severity)
    {
      i->second = b;
      done = true;
      break;
    }
  }
  if (!done)
  {
    pTempOriginalState.push_back(pair<Severity_t, bool>(severity, b));
  }

  // now change the state to input
  setSeverityLogging(severity, state);
}

//----------------------------------------------------------------
void LogMsg::unsetTempSeverityLogging(const Severity_t severity)
{
  // undo what the previous method did.
  if (severity == PRINT || severity == FORCE)
  {
    // take no action
    return;
  }

  vector<pair<Severity_t, bool> >::iterator i;
  bool done = false;
  for (i = pTempOriginalState.begin(); i!= pTempOriginalState.end(); ++i)
  {
    if (i->first == severity)
    {
      setSeverityLogging(severity, i->second);
      pTempOriginalState.erase(i);
      done = true;
      break;
    }
  }
  if (!done)
  {
    printf("ERROR in unset_temp_severity_logging, state not found for %s\n",
	   pSeverityStringAll(severity).c_str());
  }
}

//----------------------------------------------------------------
void LogMsg::pLog(const string &msg)
{
  pthread_mutex_lock(&_printMutex);
  if (pLogRealTime)
  {
    DateTime dt(time(0));
    string s = dt.getTimeStr(false);
    s += " ";
    s += msg;
    _log(s);
  }
  else
  {
    _log(msg);
  }
  pthread_mutex_unlock(&_printMutex);
}

//----------------------------------------------------------------
string LogMsg::pHeader(const string &fname, const int line,
		       const string &method, const Severity_t severity) const
{
  if (!pLogClassAndMethod)
  {
    return "";
  }

  string ret = "";
  if (severity != PRINT)
  {
    char buf[ARRAY_LEN_LONG];
    sprintf(buf, "%s[%d]:", fname.c_str(), line);
    ret = buf;
    ret += method;
    ret += "()::";
  }
  return ret;
}

//----------------------------------------------------------------
string LogMsg::pSeverityString(const Severity_t severity) const
{
  string ret = "";
  switch (severity)
  {
  case LogMsg::WARNING:
    ret = "WARNING ";
    break;
  case LogMsg::ERROR:
    ret = "ERROR ";
    break;
  case LogMsg::SEVERE:
    ret = "SEVERE ";
    break;
  case LogMsg::FATAL:
    ret = "FATAL ";
    break;
  default:
    // all other cases have no output
    break;
  }
  return ret;
}
//----------------------------------------------------------------
string LogMsg::pSeverityStringAll(const Severity_t severity) const
{
  string ret = "";
  switch (severity)
  {
  case DEBUG:
    ret = "DEBUG   ";
    break;
  case DEBUG_VERBOSE:
    ret = "VERBOSE ";
    break;
  case TRIGGER:
    ret = "TRIGGER ";
    break;
  case THREAD:
    ret = "THREAD  ";
    break;
  case SPECIAL:
    ret = "SPECIAL ";
    break;
  case LogMsg::WARNING:
    ret = "WARNING ";
    break;
  case LogMsg::ERROR:
    ret = "ERROR   ";
    break;
  case LogMsg::SEVERE:
    ret = "SEVERE  ";
    break;
  case LogMsg::FATAL:
    ret = "FATAL   ";
    break;
  default:
    // all other cases have no output
    break;
  }
  return ret;
}
