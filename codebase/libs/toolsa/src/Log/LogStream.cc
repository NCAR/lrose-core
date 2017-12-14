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
 * @file LogStream.cc
 */

//----------------------------------------------------------------
#include <toolsa/LogStream.hh>
#include <toolsa/DateTime.hh>
#include <sstream>
#include <iostream>

/**
 * A static pointer used to make this a singleton class
 */
static LogState *_state = NULL;

//----------------------------------------------------------------
LogStream::LogStream(const std::string &fname, const int line,
		     const std::string &method, Log_t logT)
{
  _active = LOG_STREAM_IS_ENABLED(logT);
  if (_active)
  {
    string severityString = _setSeverityString(logT);
    _setHeader(severityString, fname, line, method, logT);
  }
}

//----------------------------------------------------------------
LogStream::LogStream(const std::string &fname, const int line,
		     const std::string &method, const std::string &name)
{
  // if the named type is not present, add it now and disable it
  LOG_STREAM_ADD_CUSTOM_TYPE_IF_NEW(name);
  _active = LOG_STREAM_IS_ENABLED(name);
  if (_active)
  {
    _setHeader(name, fname, line, method);
  }
}

//----------------------------------------------------------------
LogStream::LogStream(const std::string &fname, const int line,
		     const std::string &method)
{
  // go with FORCE as the default
  _active = true;
  if (_active)
  {
    string severityString = _setSeverityString(FORCE);
    _setHeader(severityString, fname, line, method, FORCE);
  }
}

//----------------------------------------------------------------
LogStream::~LogStream()
{
  if (_active)
  {
    LOG_STREAM_LOCK();

    if (LOG_STREAM_IS_COUT())
    {
      std::cout << _buf.str() << std::endl;
    }
    else
    {
      std::cerr << _buf.str() << std::endl;
    }
    LOG_STREAM_UNLOCK();
  }
}

//----------------------------------------------------------------
void LogStream::_setHeader(const std::string &severityString,
			   const std::string &fname, 
			   const int line, const std::string &method,
			   Log_t logT)
{
  if (LOG_STREAM_TIMESTAMP_ENABLED())
  {
    DateTime dt(time(0));
    _buf << dt.getTimeStr(false) << " ";
  }
  _buf << severityString;
  if (LOG_STREAM_CLASSMETHOD_ENABLED() && logT != PRINT)
  {
    _buf << fname << "[" << line << "]:" << method << "()::";
  }
}
	     
//----------------------------------------------------------------
void LogStream::_setHeader(const std::string &severityString,
			   const std::string &fname, 
			   const int line, const std::string &method)
{
  if (LOG_STREAM_TIMESTAMP_ENABLED())
  {
    DateTime dt(time(0));
    _buf << dt.getTimeStr(false) << " ";
  }
  _buf << severityString;
  if (LOG_STREAM_CLASSMETHOD_ENABLED())
  {
    _buf << fname << "[" << line << "]:" << method << "()::";
  }
}
	     
//-----------------------------------------------------------------
std::string LogStream::_setSeverityString(Log_t logT)
{
  string ret = "         ";
  if (LOG_STREAM_SHOW_ALL_SEVERITY_KEYS_ENABLED())
  {
    switch (logT)
    {
    case DEBUG:
      ret =   " DEBUG    ";
      break;
    case DEBUG_VERBOSE:
      ret =   " VERBOSE  ";
      break;
    case ERROR:
      ret =   " ERROR    ";
      break;
    case WARNING:
      ret =   " WARNING  ";
      break;
    case FATAL:
      ret =   " FATAL    ";
      break;
    case SEVERE:
      ret =   " SEVERE   ";
      break;
    default:
      break;
    }
  }
  else
  {
    switch (logT)
    {
    case DEBUG:
      break;
    case DEBUG_VERBOSE:
      break;
    case ERROR:
      ret =    " ERROR   ";
      break;
    case WARNING:
      ret =    " WARNING ";
      break;
    case FATAL:
      ret =    " FATAL   ";
      break;
    case SEVERE:
      ret =    " SEVERE  ";
      break;
    default:
      break;
    }
  }
  return ret;
}

//----------------------------------------------------------------
void LogState::initPointer(void)
{
  if (_state == NULL)
  {
    _state = new LogState();
  }
}

//----------------------------------------------------------------
void LogState::freePointer(void)
{
  if (_state != NULL)
  {
    delete _state;
    _state = NULL;
  }
}

//----------------------------------------------------------------
LogState *LogState::getPointer(void)
{
  initPointer();
  return _state;
}

//----------------------------------------------------------------
LogState::LogState() :
  _logRealTime(true),
  _logClassAndMethod(true),
  _logShowAllSeverityKeys(true),
  _logToCout(true)
{
  _enabled[LogStream::DEBUG] = true;
  _enabled[LogStream::DEBUG_VERBOSE] = false;
  _enabled[LogStream::ERROR] = true;
  _enabled[LogStream::WARNING] = true;
  _enabled[LogStream::FATAL] = true;
  _enabled[LogStream::SEVERE] = true;
  _enabled[LogStream::PRINT] = true;
  pthread_mutex_init(&_printMutex, NULL);
}

//----------------------------------------------------------------
LogState::~LogState()
{
  pthread_mutex_destroy(&_printMutex);
}

//----------------------------------------------------------------
void LogState::init(const bool debug, const bool debugVerbose,
		     const bool realtime, const bool file)
{
  setLogging(LogStream::DEBUG, debug);
  setLogging(LogStream::DEBUG_VERBOSE, debugVerbose);
  setLoggingTimestamp(realtime);
  setLoggingClassAndMethod(file);
}

//----------------------------------------------------------------
void LogState::setLogging(const LogStream::Log_t severity, const bool state)
{ 
  if (severity == LogStream::FORCE || severity == LogStream::PRINT)
  {
    // take no action, these cannot be disabled
    return;  
  }
  _enabled[severity] = state;
}

//----------------------------------------------------------------
void LogState::setLogging(const std::string &name, const bool state)
{ 
  // note it is added if not yet there
  _customEnabled[name] = state;
}

//----------------------------------------------------------------
void LogState::setLoggingTimestamp(const bool state)
{
  _logRealTime = state;
}

//----------------------------------------------------------------
void LogState::setLoggingClassAndMethod(const bool state)
{
  _logClassAndMethod = state;
}

//----------------------------------------------------------------
void LogState::setLoggingShowAllSeverityKeys(const bool state)
{
  _logShowAllSeverityKeys = state;
}

//----------------------------------------------------------------
bool LogState::isEnabled(LogStream::Log_t severity) const
{
  if (_enabled.find(severity) == _enabled.end())
  {
    return false;
  }
  else
  {
    return _enabled.find(severity)->second;
  }
}


//----------------------------------------------------------------
bool LogState::isEnabled(const std::string &name) const
{
  if (_customEnabled.find(name) == _customEnabled.end())
  {
    return false;
  }
  else
  {
    return _customEnabled.find(name)->second;
  }
}

//----------------------------------------------------------------
void LogState::setVerbose(void)
{
  _enabled[LogStream::DEBUG_VERBOSE] = true;
}

//----------------------------------------------------------------
void LogState::clearVerbose(void)
{
  _enabled[LogStream::DEBUG_VERBOSE] = false;
}

//----------------------------------------------------------------
void LogState::addCustomTypeIfNew(const std::string &s)
{
  if (_customEnabled.find(s) == _customEnabled.end())
  {
    _customEnabled[s] = false;
  }
}

//----------------------------------------------------------------
void LogState::lock(void)
{
  pthread_mutex_lock(&_printMutex);
}

//----------------------------------------------------------------
void LogState::unlock(void)
{
  pthread_mutex_unlock(&_printMutex);
}

//----------------------------------------------------------------
void LogState::setCout(void)
{
  _logToCout = true;
}

//----------------------------------------------------------------
void LogState::setCerr(void)
{
  _logToCout = false;
}

