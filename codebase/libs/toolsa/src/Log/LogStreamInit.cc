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
#include <toolsa/LogStreamInit.hh>
#include <toolsa/LogStream.hh>
#include <toolsa/TaThreadLog.hh>
#include <toolsa/TaTriggerLog.hh>

//----------------------------------------------------------------------
void LogStreamInit::init(bool debug, bool debugVerbose, bool showRealtime,
			 bool showClassAndMethod)
{
  LOG_STREAM_INIT(debug, debugVerbose, showRealtime, showClassAndMethod);
}

//----------------------------------------------------------------------
void LogStreamInit::setLogFile(const std::string &app,
			       const std::string &instance,
			       const std::string &logPath)
{
  LOG_STREAM_TO_LOGFILE(app, instance, logPath);
}

//----------------------------------------------------------------------
void LogStreamInit::showAllSeverityKeys(bool showAll)
{
  LOG_STREAM_SET_SHOW_ALL_SEVERITY_KEYS(showAll);
}

//----------------------------------------------------------------------
void LogStreamInit::setDebug(bool state)
{
  LOG_STREAM_SET_TYPE(DEBUG, state);
}

//----------------------------------------------------------------------
void LogStreamInit::setDebugVerbose(bool state)
{
  LOG_STREAM_SET_TYPE(DEBUG_VERBOSE, state);
}

//----------------------------------------------------------------------
void LogStreamInit::setTrigger(bool state)
{
  LOG_STREAM_SET_CUSTOM_TYPE(TaTriggerLog::name(), state);
}

//----------------------------------------------------------------------
void LogStreamInit::setThreading(bool state)
{
  LOG_STREAM_SET_CUSTOM_TYPE(TaThreadLog::name(), state);
}

//----------------------------------------------------------------------
void LogStreamInit::setWarning(bool state)
{
  LOG_STREAM_SET_TYPE(WARNING, state);
}

//----------------------------------------------------------------------
void LogStreamInit::setError(bool state)
{
  LOG_STREAM_SET_TYPE(ERROR, state);
}

//----------------------------------------------------------------------
void LogStreamInit::setFatal(bool state)
{
  LOG_STREAM_SET_TYPE(FATAL, state);
}

//----------------------------------------------------------------------
void LogStreamInit::setSevere(bool state)
{
  LOG_STREAM_SET_TYPE(SEVERE, state);
}

//----------------------------------------------------------------------
void LogStreamInit::setTimestamp(bool state)
{
  LOG_STREAM_SET_TIMESTAMP(state);
}

//----------------------------------------------------------------------
void LogStreamInit::setClassAndMethod(bool state)
{
  LOG_STREAM_SET_CLASS_AND_METHOD(state);
}


//----------------------------------------------------------------------
void LogStreamInit::destroy(void)
{
  LogState::freePointer();
}
