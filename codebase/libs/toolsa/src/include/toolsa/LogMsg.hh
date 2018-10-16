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
 * @file LogMsg.hh
 * @brief Logging of information to stdout
 * @class LogMsg
 * @brief Logging of information to stdout
 *
 * The logging is handled by a static pointer to a LogMsg object through use
 * of a small set of macro calls, which will create the pointer if need be.
 * The logging state is thus shared by all software that uses the macros.
 * 
 * Two main logging macros are used to produce log output:
 *   - LOG() - logged message is a simple string.
 *   - LOGF() - logged message is a formatted string with any number of args.
 *
 * Logged output optionally includes the file name, line number, and function
 * name
 *   - 'file[line]::method()::'
 *
 *
 * Logged output optionally includes the real time hour/minute/second at which
 * it was produced.
 *
 *
 * A number of severity types are enumerated  (LogMsg::Severity_t).  Each
 * severity type can be enabled or disabled (except for FORCE which cannot
 * be disabled.  If a severity type is enabled, all LOG() or LOGF() calls
 * that have this severity type will be written to stdout.  It disabled, such
 * calls will produce no output.
 *
 * The following are enabled by default: FORCE, PRINT, DEBUG, WARNING, ERROR,
 * SEVERE, FATAL
 *
 * The following are disabled by default: DEBUG_VERBOSE, TRIGGER, THREAD,
 * SPECIAL
 *
 * The following severity types produce output that includes the string
 * that is equivalent to the severity type:
 *       - WARNING
 *       - ERROR
 *       - FATAL
 * For example LOG(LogMsg::WARNING, "message")  might produce this output:
 *
 *       hh:mm:ss file[line]::method()::WARNING message
 * 
 * State consists of boolean yes/no logging for every "severity" level
 * in the LogMsg class, and another boolean to control whether real time
 * stamps are put into log messages, and a 3rd boolean for file/method info.
 */

#ifndef	LOG_MSG_H
#define	LOG_MSG_H

#include <string>
#include <vector>
#include <pthread.h>

/**
 * Log a fixed string message if the severity type is enabled.
 *
 * @param[in] severity  Severity level
 * @param[in] msg  The fixed message to log
 *
 * @note '\n' is appended to the logged output
 */
#define LOG(severity, msg) (LogMsg::getPointer()->log(__FILE__, __LINE__, __FUNCTION__, (severity), (msg)))

/**
 * Log a formatted formatted message if the severity type is enabled
 *
 * @param[in] severity  Severity level
 * @param[in] format  The printf style format string to use.
 *
 * The remaining arguments are those that get formatted.
 *
 * @note '\n' is appended to the logged output
 */
#define LOGF(severity, format, ...) (LogMsg::getPointer()->logf(__FILE__, __LINE__, __FUNCTION__, (severity), (format), __VA_ARGS__))

/**
 * Log the accumulated information that was build up using LOG_ACCUMULATE
 * or LOG_ACCUMULATEF macros, if the severity level is enabled.
 *
 * @param[in] severity  Severity level
 *
 * All the accumulated information is cleared out by this macro
 */
#define LOG_ACCUM(severity) (LogMsg::getPointer()->logAccum(__FILE__, __LINE__, __FUNCTION__, (severity)))

/**
 * Store a string internally for later logging.
 *
 * @param[in] msg  Fixed message
 */
#define LOG_ACCUMULATE(msg) (LogMsg::getPointer()->accumulate((msg)))

/**
 * Store a formatted string internally for later logging.
 *
 * @param[in] format  The printf style format string
 *
 * The remaining args are those that get formatted to produce the stored string.
 */
#define LOG_ACCUMULATEF(format, ...) (LogMsg::getPointer()->accumulatef((format),__VA_ARGS__))

/**
 * Initialize state for commonly variable items
 *
 * @param[in] debug  True to enable DEBUG, false to disable.
 * @param[in] debugVerbose  True to enable DEBUG_VERBOSE, false to disable.
 * @param[in] realtime  True for showing real time in logged message
 * @param[in] showFile  True for showing file/line/method in logged messages
 *
 */
#define LOG_INIT(debug,debugVerbose,realtime,showFile) (LogMsg::getPointer()->init((debug),(debugVerbose),(realtime),(showFile)))

/**
 * Disable putting of real time stamps into logged messages
 *
 */
#define LOG_DISABLE_TIME_STAMP() (LogMsg::getPointer()->setLoggingTimestamp(false))

/**
 * Disable a particular severity type
 *
 * @param[in] s  Severity type
 */
#define LOG_DISABLE_SEVERITY(s) (LogMsg::getPointer()->setSeverityLogging((s),false))

/**
 * Enable a particular severity type
 *
 * @param[in] s  Severity type
 */
#define LOG_ENABLE_SEVERITY(s) (LogMsg::getPointer()->setSeverityLogging((s),true))

/**
 * Enable a particular severity type
 *
 * @param[in] s  Severity type
 * @param[in] b  True or false
 */
#define LOG_SET_SEVERITY(s, b) (LogMsg::getPointer()->setSeverityLogging((s),(b)))

/**
 * @return true if input severity type is enabled
 *
 * @param[in] s  Severity type
 */
#define LOG_IS_ENABLED(s) (LogMsg::getPointer()->isEnabled((s)))

/**
 * Set time stamp status to input
 *
 */
#define LOG_SET_TIMESTAMP(s) (LogMsg::getPointer()->setLoggingTimestamp((s)))

/**
 * Set class/method status to input
 *
 */
#define LOG_SET_CLASS_AND_METHOD(s) (LogMsg::getPointer()->setLoggingClassAndMethod((s)))

/**
 * Set showing all severity output types in the messages, if true
 * If false, show only WARNING, ERROR, SEVERE, FATAL
 */
#define LOG_SET_SHOW_ALL_SEVERITY_KEYS(s) (LogMsg::getPointer()->setLoggingShowAllSeverityKeys((s)))

class LogMsg
{
public:

  /**
   * Initialize the singleton pointer to point to an object.
   */
  static void initPointer(void);

  /**
   * Free the singleton pointer if it has been created.
   */
  static void freePointer(void);

  /**
   * @return  A pointer to the singleton object
   * @note This method will create the pointer if it needs to
   */
  static LogMsg *getPointer(void);

  /**
   * @enum Severity_t
   * @brief Types of severity which can be included in log messages.
   * User can turn on and off the various severities, except for FORCE
   * and PRINT.  These remain on no matter what, i.e. they are always
   * enabled.
   *
   * The following severity values produce an indication of the
   * severity in the logged messages, prepended to the rest of the message.
   *    WARNING, ERROR, SEVERE, FATAL
   * The other values do not do this.
   */
  typedef enum
  {
    FORCE=0,              /**< Force (must display this one no matter what) */
    PRINT=1,              /**< Force,but with no extra header information */
    DEBUG=2,              /**< Debugging */
    DEBUG_VERBOSE=3,      /**< Verbose debugging */
    TRIGGER=4,            /**< Triggering specific messages */
    THREAD=5,             /**< Threading specific messages */
    SPECIAL=6,            /**< Special (for any customized use) */
    WARNING=7,            /**< Warning message */
    ERROR=8,              /**< Error message */
    SEVERE=9,             /**< Severe error message */
    FATAL=10,              /**< Fatal error */
    NUM=11                 /**< Number of severity states */
  } Severity_t;    


  /**
   * Log an unformatted message of a particular severity if that severity
   * is enabled. If the severity is disabled, the method has no action.
   *
   * @param[in] fname  File name of calling routine
   * @param[in] line  Line number in calling routine
   * @param[in] method  Method  Name of calling method
   * @param[in] severity  Severity value
   * @param[in] msg  String to log
   */
  void log(const std::string &fname, const int line,
	   const std::string &method, const Severity_t severity,
	   const std::string &msg);

  /** 
   * Log a formatted message with a particular severity. If that severity is
   * enabled. If the severity is disabled, the method has no action.
   *
   * @param[in] fname  File name of calling routine
   * @param[in] line  Line number in calling routine
   * @param[in] method  Method  Name of calling method
   * @param[in] severity  the severity level
   * @param[in] format the printf formatting to use
   *
   * the remaining arguments are variable
   */
  void logf(const std::string &fname, const int line,
	    const std::string &method, const Severity_t severity,
	    std::string format, ... );

  /**
   * Set severity status for the debugging states, and set other state variables
   *
   * @param[in] debug  True to show DEBUG messages
   * @param[in] debugVerbose  True to show DEBUG_VERBOSE messages
   * @param[in] realtime  True to show the real time
   * @param[in] showFile  True to show class/method information
   */
  void init(const bool debug, const bool debugVerbose, const bool realtime,
	    const bool showFile);

  /**
   * Set a particular severity message logging state to input value.
   * If state=true, logging of messages with this severity will show up.
   * If state=true, logging of messages with this severity will not show up.
   *
   * @param[in] severity  A severity level
   * @param[in] state  The status to give this level
   */
  void setSeverityLogging(const Severity_t severity, const bool state);

  /**
   * Set state for inclusion of real time into logged messages.
   * If state=true, messages will show real time they were logged.
   * If state=false, messages will not show real time they were logged.
   *
   * @param[in] state True to turn on logging of time stamps false to not
   */
  void setLoggingTimestamp(const bool state);

  /**
   * Set state for inclusion of class and method names into logged messages
   * If state=true, messages will show class::method() in messages.
   * If state=false, messages will not show this.
   *
   * @param[in] state  True to turn on logging of class/method
   */
  void setLoggingClassAndMethod(const bool state);

  /**
   * Set state for showing all severity keys in logged messages, or not
   * If state=true, messages will show DEBUG, VERBOSE, strings as part of all enabled
   *                messages
   * If state=false, messages will show WARNING, ERROR, SEVERE, and FATAL only. Other
   *                 enabled messages will not have a type string 
   *
   * @param[in] state  True to turn on showing all keys
   */
  void setLoggingShowAllSeverityKeys(const bool state);

  /**
   * @return true if input severity level is enabled
   *
   * @param[in] severity  The severity
   */
  bool isEnabled(const Severity_t severity) const;

  /**
   * Accumulate input information into a local string without logging, with
   * a formatted argument list
   * @param[in] format  The format for the variable args
   */
  void accumulatef(std::string format, ...) const;

  /**
   * Accumulate input information into a local string without logging,
   * for a fixed string
   *
   * @param[in] value  The fixed string
   */
  void accumulate(const std::string &value) const;

  /**
   * Log info accumulated using accumulate/accumulatef methods,
   * then clear out accumulated info 
   *
   * @param[in] fname  File name of calling routine
   * @param[in] line  Line number in calling routine
   * @param[in] method  Name of the calling method
   * @param[in] severity  Severity level
   */
  void logAccum(const std::string &fname, const int line, 
 		const std::string &method, const Severity_t severity);

  /**
   * Temporarily set verbose debugging
   */
  void setVerbose(void);

  /**
   * Turn off verbose debugging
   */
  void clearVerbose(void);

  /**
   * Temporarily set a particular severity message logging state to input value.
   * To be used in debugging particular classes
   *
   * @param[in] severity   A severity
   * @param[in] state   True or false
   */
  void setTempSeverityLogging(const Severity_t severity, const bool state);

  /**
   * Set a particular severity message logging state back to state before call
   * to set_temp_severity_logging, to be used in debugging particular classes
   *
   * @param[in] severity  Severity to revert to original state
   */
  void unsetTempSeverityLogging(const Severity_t severity);

protected:

private:

  pthread_mutex_t _printMutex;


  /**
   * State for enabling or disabling the various states.
   */
  bool pSeverityEnabled[NUM];

  /**
   * True to log the real time values in the log messages
   */
  bool pLogRealTime;

  /**
   * True to log class and method names in log messages
   */
  bool pLogClassAndMethod;

  /**
   * True to put severity key strings into logged messages for all types
   */
  bool pLogShowAllKeys;

  /**
   * Temporary storage for the severity/status pairs
   */
  std::vector<std::pair<Severity_t, bool> > pTempOriginalState;

  /**
   * Constructor, no class name (class is the empty string.)
   *
   * Logging of all message severities is set to the singleton state,
   * the initial default state is every severity is enabled.
   *
   * Logging does include real time stamp (can be changed)
   */
  LogMsg(void);

  /**
   * Destructor
   */
  ~LogMsg(void);

  /**
   * Log a string to logging output, with configured time/class/method
   * and type info.
   *
   * @param[in] msg  The string to log
   */
  void pLog(const std::string &msg);

  /**
   * Form and return the header for a message
   * @param[in] fname  File name of calling routine
   * @param[in] line  Line number in calling routine
   * @param[in] method  The method calling
   * @param[in] severity  Severity to revert to original state
   *
   * @return Header string
   */
  std::string pHeader(const std::string &fname, const int line,
		      const std::string &method,
		      const Severity_t severity) const;

  /**
   * @return the string associated with a particular severity level
   * when not all are shown
   * @param[in] severity  The severity
   */
  std::string pSeverityString(const Severity_t severity) const;

  /**
   * @return the string associated with a particular severity level
   * when all are shown
   *
   * @param[in] severity  The severity
   */
  std::string pSeverityStringAll(const Severity_t severity) const;
};

#endif
