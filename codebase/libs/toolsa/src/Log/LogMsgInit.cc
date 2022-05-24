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
#include <toolsa/LogMsgInit.hh>
#include <toolsa/LogMsg.hh>

//----------------------------------------------------------------------
void LogMsgInit::init(bool debug, bool debugVerbose, bool showRealtime,
		      bool showClassAndMethod)
{
  LOG_INIT(debug, debugVerbose, showRealtime, showClassAndMethod);
}

// //----------------------------------------------------------------------
// void LogMsgInit::setLogFile(const std::string &app, const std::string &instance,
// 			    const std::string &logPath)
// {
//   LOG_TO_LOGFILE(app, instance, logPath);
// }

//----------------------------------------------------------------------
void LogMsgInit::showAllSeverityKeys(bool showAll)
{
  LOG_SET_SHOW_ALL_SEVERITY_KEYS(showAll);
}


//----------------------------------------------------------------------
void LogMsgInit::setDebug(bool state)
{
  LOG_SET_SEVERITY(LogMsg::DEBUG, state);
}

//----------------------------------------------------------------------
void LogMsgInit::setDebugVerbose(bool state)
{
  LOG_SET_SEVERITY(LogMsg::DEBUG_VERBOSE, state);
}

//----------------------------------------------------------------------
void LogMsgInit::setTrigger(bool state)
{
  LOG_SET_SEVERITY(LogMsg::TRIGGER, state);
}

//----------------------------------------------------------------------
void LogMsgInit::setThreading(bool state)
{
  LOG_SET_SEVERITY(LogMsg::THREAD, state);
}

//----------------------------------------------------------------------
void LogMsgInit::setWarning(bool state)
{
  LOG_SET_SEVERITY(LogMsg::WARNING, state);
}

//----------------------------------------------------------------------
void LogMsgInit::setError(bool state)
{
  LOG_SET_SEVERITY(LogMsg::ERROR, state);
}

//----------------------------------------------------------------------
void LogMsgInit::setFatal(bool state)
{
  LOG_SET_SEVERITY(LogMsg::FATAL, state);
}

//----------------------------------------------------------------------
void LogMsgInit::setSpecial(bool state)
{
  LOG_SET_SEVERITY(LogMsg::SPECIAL, state);
}

//----------------------------------------------------------------------
void LogMsgInit::setSevere(bool state)
{
  LOG_SET_SEVERITY(LogMsg::SEVERE, state);
}

//----------------------------------------------------------------------
void LogMsgInit::setTimestamp(bool state)
{
  LOG_SET_TIMESTAMP(state);
}

//----------------------------------------------------------------------
void LogMsgInit::setClassAndMethod(bool state)
{
  LOG_SET_CLASS_AND_METHOD(state);
}

//----------------------------------------------------------------------
void LogMsgInit::destroy(void)
{
  LogMsg::freePointer();
}
