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
 * @file LogStreamInit.hh
 * @brief Initialization of LogStream using a wrapper class with static methods
 * @class LogStreamInit
 * @brief Initialization of LogStream using a wrapper class with static methods
 */

#ifndef LOG_STREAM_INIT_HH
#define LOG_STREAM_INIT_HH

#include <string>

class LogStreamInit
{
public:
  /**
   * Initialize. If you do not call this, the default
   * values for the logging types will be set to defaults
   *
   * @param[in] debug  Boolean value for DEBUG log type
   * @param[in] debugVerbose  Boolean value for DEBUG_VERBOSE log type
   * @param[in] realtime  Boolean value for showing real time
   * @param[in] showFile  Boolean value for showing file/line/class
   */
  static void init(bool debug, bool debugVerbose, bool realtime, bool showFile);

  /**
   * Set to put output into log files
   * @param[in] app  Name of app to use in file names
   * @param[in] instance  Instance to use in file names
   * @param[in] logPath  Top directory to put log file subdirs, if empty
   *                     the default is $LOG_DIR
   */
  static void setLogFile(const std::string &app, const std::string &instance,
			 const std::string &logPath="");

  /**
   * Set so all severity keys are shown or not based on flag
   * @param[in] showAll
   */
  static void showAllSeverityKeys(bool showAll);

  /**
   * Set DEBUG flag to input
   */
  static void setDebug(bool state);

  /**
   * Set DEBUG_VERBOSE flag to input
   */
  static void setDebugVerbose(bool state);

  /**
   * Set custom triggering flag TaTriggerLog::name() to input
   */
  static void setTrigger(bool state);

  /**
   * Set custom threading flag TaThreadLog::name() to input
   */
  static void setThreading(bool state);

  /**
   * Set WARNING flag to input
   */
  static void setWarning(bool state);

  /**
   * Set ERROR flag to input
   */
  static void setError(bool state);

  /**
   * Set FATAL flag to input
   */
  static void setFatal(bool state);

  /**
   * Set SEVERE flag to input
   */
  static void setSevere(bool state);

  /**
   * Set time stamp state to input
   */
  static void setTimestamp(bool state);

  /**
   * Set logging of source code informtation to input
   */
  static void setClassAndMethod(bool state);

  /**
   * Free up memory used for logging
   */
  static void destroy(void);

private:
};

#endif
