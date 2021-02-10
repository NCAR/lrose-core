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
 * @file LogMsgStreamInit.cc
 */

#include <toolsa/LogMsgStreamInit.hh>
#include <toolsa/LogStreamInit.hh>
#include <toolsa/LogMsgInit.hh>
#include <iostream>


//----------------------------------------------------------------------
void LogMsgStreamInit::init(bool debug, bool debugVerbose, bool realtime,
			    bool showFile)
{
  LogMsgInit::init(debug, debugVerbose, realtime, showFile);
  LogStreamInit::init(debug, debugVerbose, realtime, showFile);
}

//----------------------------------------------------------------------
void LogMsgStreamInit::setLogFile(const std::string &app,
				  const std::string &instance,
				  const std::string &logPath)
{
  // LogMsgInit::setLogFile(app, instance, logPath);
  std::cerr << "WARNING LogMsg logfile writing is not implemented" << std::endl;
  std::cerr << "        Consider switching to LogStream" << std::endl;
  LogStreamInit::setLogFile(app, instance, logPath);
}

//----------------------------------------------------------------------
void LogMsgStreamInit::showAllSeverityKeys(bool showAll)
{
  LogMsgInit::showAllSeverityKeys(showAll);
  LogStreamInit::showAllSeverityKeys(showAll);
}

//----------------------------------------------------------------------
void LogMsgStreamInit::setDebug(bool state)
{
  LogMsgInit::setDebug(state);
  LogStreamInit::setDebug(state);
}

//----------------------------------------------------------------------
void LogMsgStreamInit::setDebugVerbose(bool state)
{
  LogMsgInit::setDebugVerbose(state);
  LogStreamInit::setDebugVerbose(state);
}

//----------------------------------------------------------------------
void LogMsgStreamInit::setTrigger(bool state)
{
  LogMsgInit::setTrigger(state);
  LogStreamInit::setTrigger(state);
}

//----------------------------------------------------------------------
void LogMsgStreamInit::setThreading(bool state)
{
  LogMsgInit::setThreading(state);
  LogStreamInit::setThreading(state);
}

//----------------------------------------------------------------------
void LogMsgStreamInit::setWarning(bool state)
{
  LogMsgInit::setWarning(state);
  LogStreamInit::setWarning(state);
}

//----------------------------------------------------------------------
void LogMsgStreamInit:: setError(bool state)
{
  LogMsgInit::setError(state);
  LogStreamInit::setError(state);
}

//----------------------------------------------------------------------
void LogMsgStreamInit::setFatal(bool state)
{
  LogMsgInit::setFatal(state);
  LogStreamInit::setFatal(state);
}

//----------------------------------------------------------------------
void LogMsgStreamInit::setSpecial(bool state)
{
  LogMsgInit::setSpecial(state);

  // stream version does not have SPECIAL
  // LogStreamInit::setSpecial(state);
}

//----------------------------------------------------------------------
void LogMsgStreamInit::setSevere(bool state)
{
  LogMsgInit::setSevere(state);
  LogStreamInit::setSevere(state);
}

//----------------------------------------------------------------------
void LogMsgStreamInit::setTimestamp(bool state)
{
  LogMsgInit::setTimestamp(state);
  LogStreamInit::setTimestamp(state);
}

//----------------------------------------------------------------------
void LogMsgStreamInit::setClassAndMethod(bool state)
{
  LogMsgInit::setClassAndMethod(state);
  LogStreamInit::setClassAndMethod(state);
}

//----------------------------------------------------------------------
void LogMsgStreamInit::destroy(void)
{
  LogMsgInit::destroy();
  LogStreamInit::destroy();
}

