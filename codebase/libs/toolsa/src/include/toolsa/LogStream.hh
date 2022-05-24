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
 * @file LogStream.hh
 * @brief Logging of information to a stream or file
 *
 * Used to log debug or output messages to cout or cerr, or to logfiles,
 * with the option of showing real time and/or showing file/line number/class
 * name in addition to the logged message. Each logged message has a type.
 * Each type can be toggled on or off so that message with the type are
 * output or not.
 *
 * A small set of fixed logging types is provided (DEBUG, ERROR, ...).
 *
 * Any number of custom logging types can be added by the user. Each custom
 * type is associated with a string, which the user specifies.
 *
 * ---------------------- Typical use -------------------------------------
 * Three main macros are all that is really needed to do logging:
 *
 *    LOG() is used with the fixed log types, for example LOG(DEBUG)
 *    LOGC() is used for custom string types, for example LOGC("MYTYPE")
 *    LOG0   has no arguments, and forces a logged output in all cases.
 *    LOGPRINT() Formatted printing, with a fixed log type
 *
 * Examples of use:
 *
 *    LOG(FATAL) << "Invalid argument";
 *    LOGC("CUSTOM") << "Message with custom type, value=" << value;
 *    LOG0 << "Message that will always appear, x=" << x << ",y=" << y;
 *    LOGPRINT(DEBUG, "int arg = %d", i);
 *    LOGPRINT(ERROR, "It didn't work");
 *
 * Each such logged message will get an endl (or '\n'),
 * so the user does not need to include that at the end.
 *
 * In cases where the log type is a variable instead of an inline enum
 * you need to use different macros:
 * 
 *    LOGV()
 *    LOGPRINTV()
 *
 * Example:
 *
 *    LogStream::Log_t t = LogStream::DEBUG;
 *    LOGV(t) << "message";
 *    LOGPRINTV(t, "message");
 *
 * 
 * ---------------------- Fixed types -------------------------------------
 * The fixed logging types that are supported:
 *    DEBUG               Normal debugging messages, enabled by default
 *    DEBUG_VERBOSE       Extra debugging messages, disabled by default
 *    ERROR               Message is prepended with the word 'ERROR ',
 *                        enabled by default
 *    FATAL               Message is prepended witht the word 'FATAL ',
 *                        enabled by default
 *    FORCE               Always logged, cannot be disabled.
 *    PRINT               Always logged, cannot be disabled, does not show
 *                        timestamps or file/class/method information.
 *    SEVERE              Message is prepended with the word 'SEVERE ',
 *                        enabled by default
 *    WARNING             Message is prepended with the word 'WARNING ',
 *                        enabled by default
 *    TRIGGER             Specific to triggering, disabled by default
 *
 * The fixed types can be individually enabled or disabled by calls to the
 * macros:  LOG_STREAM_ENABLE()  and LOG_STREAM_DISABLE(), LOG_STREAM_SET_TYPE()
 *
 * For example:  LOG_STREAM_DISABLE(WARNING)
 *               LOG_STREAM_ENABLE(DEBUG_VERBOSE)
 *               LOG_STREAM_SET_TYPE(DEBUG, false)
 *
 * ---------------------- Custom types -------------------------------------
 *
 * The user can also add custom logging types, each of which is a string,
 * by simply invoking OUTC with a particular string.  If this string is
 * not yet in place in the state, it is added and that type is enabled.
 * If it is already in place, the state of that type is used (enabled or
 * disabled).
 * 
 * Changing the state of a custom type is done with calls to these macros:
 * LOG_STREAM_DISABLE_CUSTOM_TYPE(),  LOG_STREAM_ENABLE_CUSTOM_TYPE(),
 * LOG_STREAM_SET_CUSTOM_TYPE().
 *
 * For example:  LOG_STREAM_ENABLE_CUSTOM_TYPE("SPECIAL");
 *               LOG_STREAM_DISABLE_CUSTOM_TYPE("SPECIAL");
 *               LOG_STREAM_SET_CUSTOM_TYPE("SPECIAL", true);
 *
 * If a custom type is modified, but has not yet been added to
 * state, it is added. 
 *
 * ***Note that formatted output using LOGPRINT or LOGPRINTV
 *    is not yet supported for custom types.
 *
 * ---------------------- time stamps -------------------------------------
 * 
 * Logged output can show the real time hour/minute/second at which
 * it was produced. The default is to show the time. To disable it:
 *    LOG_STREAM_DISABLE_TIME_STAMP()
 *
 * If time stamps are not disabled, every logged message is prepended with
 * 'hh:mm:ss'
 *
 * ---------------------- source code information -------------------------
 *
 * Logged output can include the file name, line number, and function
 * name - The default is to show this information. To disable it:
 *    LOG_STREAM_DISABLE_FILE_INFO()
 *
 * If the info is not disabled, every logged message is prepended with
 * 'file[line]::method()::',  
 *
 * ---------------------- severity keys --------- -------------------------
 *
 * By default, logged messages show the severity within the logged message
 * only for WARNING, ERROR, FATAL, SEVERE.
 * To change so all messages have the key (DEBUG, VERBOSE, etc): 
 *
 *   LOG_STREAM_SET_SHOW_ALL_SEVERITY_KEYS()
 *
 * --------------------- Initialization macros -----------------------------
 *
 * To initialize commonly configured settings in one call:
 *
 *   LOG_STREAM_INIT(debug,debugVerbose,realtime,showFile)
 * 
 *
 * ------------ logged output stream choices -------------------------------
 *
 * The default is to log everything to cout.  You can change to log to cerr
 * or to logfiles:
 *
 *   LOG_STREAM_TO_CERR()
 *   LOG_STREAM_TO_COUT()
 *
 *   LOG_STREAM_TO_LOGFILE(app,instance,logfilepath)
 *   LOG_STREAM_TO_DEFAULT_LOGFILE(app,instance)
 *
 *       Logfiles are written to yyyymmdd subdirectories of logfilepath,
 *       with logfile name = <app>.<instance>.log
 *
 *       If you use LOG_STREAM_TO_DEFAULT_LOGFILE, it is assumed the logfilepath
 *       is the environment variable $LOG_DIR
 *
 * ---------------------- finishing up ------------------------------------
 *
 * To free all internal memory and clean up
 *
 *  LOG_STREAM_FINISH()
 */

#ifndef LOG_S_HH
#define LOG_S_HH

#include <toolsa/LogFile.hh>
#include <map>
#include <string>
#include <sstream>
#include <pthread.h>

/**
 * Initialize. If you do not call this, the default
 * values for the logging types will be set as indicated above, namely
 * true,false,true,true.
 *
 * @param[in] debug  Boolean value for DEBUG log type
 * @param[in] debugVerbose  Boolean value for DEBUG_VERBOSE log type
 * @param[in] realtime  Boolean value for showing real time
 * @param[in] showFile  Boolean value for showing file/line/class
 */
#define LOG_STREAM_INIT(debug,debugVerbose,realtime,showFile) (LogState::getPointer()->init((debug),(debugVerbose),(realtime),(showFile)))

/**
 * Free memory and delete objects
 */
#define LOG_STREAM_FINISH() (LogState::freePointer())

/**
 * Disable putting of real time stamps into logged messages
 */
#define LOG_STREAM_DISABLE_TIME_STAMP() (LogState::getPointer()->setLoggingTimestamp(false))

/**
 * Disable putting file name,line, and class info into logged messages
 */
#define LOG_STREAM_DISABLE_FILE_INFO() (LogState::getPointer()->setLoggingClassAndMethod(false))

/**
 * Disable a particular logging type
 *
 * @param[in] s  Logging type
 */
#define LOG_STREAM_DISABLE(s) (LogState::getPointer()->setLogging(LogStream::s,false))


/**
 * Disable a particular logging type, when arg is a variable not an enum
 *
 * @param[in] s  Logging t ype
 */
#define LOG_STREAM_DISABLEV(s) (LogState::getPointer()->setLogging(s, false))

/**
 * Disable a particular custom logging type. If the type is not present, it
 * is added and then disabled.
 *
 * @param[in] s  Custom logging type string
 */
#define LOG_STREAM_DISABLE_CUSTOM_TYPE(s) (LogState::getPointer()->setLogging((s),false))

/**
 * Enable a particular logging type
 *
 * @param[in] s  Logging t ype
 */
#define LOG_STREAM_ENABLE(s) (LogState::getPointer()->setLogging(LogStream::s,true))

/**
 * Enable a particular logging type, when arg is a variable not an enum
 *
 * @param[in] s  Logging t ype
 */
#define LOG_STREAM_ENABLEV(s) (LogState::getPointer()->setLogging(s,true))

/**
 * Enable a particular custom logging type. If the type is not present, it is
 * added and then enabled.
 *
 * @param[in] s  Custom logging type string
 */
#define LOG_STREAM_ENABLE_CUSTOM_TYPE(s) (LogState::getPointer()->setLogging((s),true))

/**
 * Set type to a status
 *
 * @param[in] s  Log_t type
 * @param[in] v  boolean
 */
#define LOG_STREAM_SET_TYPE(s,v) (LogState::getPointer()->setLogging(LogStream::s,(v)))

/**
 * Set putting of real time stamps into logged messages to a status
 * @param[in] v  boolean status
 */
#define LOG_STREAM_SET_TIMESTAMP(v) (LogState::getPointer()->setLoggingTimestamp((v)))

/**
 * Set putting file name,line, and class info into logged messages to a status
 * @param[in] v  boolean status
 */
#define LOG_STREAM_SET_CLASS_AND_METHOD(v) (LogState::getPointer()->setLoggingClassAndMethod((v)))

/**
 * Set showing all severitys in the messages themselves to flag
 * @param[in] v  boolean  True to show all, false to show only the default
 *                        ones that are shown WARNING, ERRROR, FATAL, SEVERE
 */
#define LOG_STREAM_SET_SHOW_ALL_SEVERITY_KEYS(v)  (LogState::getPointer()->setLoggingShowAllSeverityKeys((v)))

/**
 * Set custom type to a status
 *
 * @param[in] s  custom type string
 * @param[in] v  boolean status
 */
#define LOG_STREAM_SET_CUSTOM_TYPE(s,v) (LogState::getPointer()->setLogging((s),(v)))

/**
 * Set cerr as the stream to use, turns off writing to logfiles
 */
#define LOG_STREAM_TO_CERR() (LogState::getPointer()->setCerr())

/**
 * Set cout stream to use, turns off writing to log files
 */
#define LOG_STREAM_TO_COUT() (LogState::getPointer()->setCout())

/**
 * Set logfile mode
 * @param[in] a  App name
 * @param[in] i  Instance
 * @param[in] p  Path for logfiles
 */
#define LOG_STREAM_TO_LOGFILE(a,i,p) (LogState::getPointer()->setLogFile((a),(i),(p)))

/**
 * Set logfile mode
 * @param[in] a  App name
 * @param[in] i  Instance
 *
 * Path for logfiles is $LOG_DIR
 */
#define LOG_STREAM_TO_DEFAULT_LOGFILE(a,i) (LogState::getPointer()->setLogFile((a),(i)))

/**
 * Do formatted logging, with inputs:
 * @param[in] s  Logging type enum
 * @param[in] format  a string
 * @param[in] ... optional additional args to go with the format
 */
#define LOGPRINT(s, ...) (LogState::getPointer()->logprint(LogStream::s, PP_NARG(__VA_ARGS__), __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__))

/**
 * Do formatted logging, with inputs:
 * @param[in] s  Logging type variable (not represented as an enum)
 * @param[in] format  a string
 * @param[in] ... optional additional args to go with the format
 */
#define LOGPRINTV(s, ...) (LogState::getPointer()->logprint(s, PP_NARG(__VA_ARGS__), __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__))


/**
 * Create a LogStream object to stream to when input is an enum. 
 *
 * @param[in] s  Logging type enum
 */
#define LOG(s) LogStream(__FILE__, __LINE__, __FUNCTION__, LogStream::s)

/**
 * Create a LogStream object to stream to when input is a variable with
 *  enum value 
 *
 * @param[in] s  Logging type variable
 */
#define LOGV(s) LogStream(__FILE__, __LINE__, __FUNCTION__, s)

/**
 * Create a LogStream object to stream to for a custom type.
 *
 * @param[in] s  Custom logging string
 *
 * If the custom type is not present, it is added and enabled.
 */
#define LOGC(s) LogStream(__FILE__, __LINE__, __FUNCTION__, s)

/**
 * Create a LogStream object to stream to, harwired to type FORCE. 
 */
#define LOG0  LogStream(__FILE__, __LINE__, __FUNCTION__)

/**
 * Store a formatted string internally for later logging.
 *
 * @param[in] format  The printf style format string
 * @param[in] ... additional arguments that go with the format
 *
 * The remaining args are those that get formatted to produce the stored string.
 */
#define LOG_STREAM_ACCUMULATE(format, ...) (LogState::getPointer()->accumulate(PP_NARG(__VA_ARGS__), (format), __VA_ARGS__))

/**
 * Log the accumulated information that was build up using LOG_STREAM_ACCUMULATE
 * if the severity level is enabled.  Clear out the state.
 *
 * @param[in] severity  Severity level enum
 *
 * All the accumulated information is cleared out by this macro
 */
#define LOG_STREAM_ACCUM(severity) (LogState::getPointer()->logAccum(__FILE__, __LINE__, __FUNCTION__, (severity)))

/**
 * @class LogStream
 *
 * @brief Logging of information to a stream, used for logging to cout or cerr
 */
class LogStream
{
public:
  /**
   * The logging types
   */
  typedef enum
  {
    DEBUG,
    DEBUG_VERBOSE,
    ERROR,
    FATAL,
    FORCE,
    PRINT,
    SEVERE,
    WARNING,
    TRIGGER
  } Log_t;


  /**
   * Constructor - fixed type
   * @param[in] fname  File name
   * @param[in] line  Line number
   * @param[in] method Method name
   * @param[in] logT  The logging type
   */
  LogStream(const std::string &fname, const int line, const std::string &method,
	    Log_t logT);

  /**
   * Constructor - custom type
   * @param[in] fname  File name
   * @param[in] line  Line number
   * @param[in] method Method name
   * @param[in] custom  The custom logging type string
   *
   * If the custom type is not present, it is added and enabled.
   */
  LogStream(const std::string &fname, const int line, const std::string &method,
	    const std::string &custom);

  /**
   * Constructor - FORCE type
   * @param[in] fname  File name
   * @param[in] line  Line number
   * @param[in] method Method name
   */
  LogStream(const std::string &fname, const int line,
	    const std::string &method);


  /**
   * The << stream operator
   * @param[in] value  A value of any type, that is put to local stream
   */
  template <typename T>
  LogStream &operator<<(T const & value)
  {
    if (_active)
    {
      _buf << value;
    }
    return *this;
  }

  /**
   * Destructor, this is where results are streamed to cout or cerr
   */
  ~LogStream();

  /**
   * Method to convert a Log_t to a string
   * @param[in] logT
   * @return the string
   */
  static std::string setSeverityString(Log_t logT);

private:

  std::ostringstream _buf;  /**< String stream storage */
  bool _active;             /**< True if logging will actually happen */

  void _setHeader(const std::string &severityString, const std::string &fname, 
		  const int line, const std::string &method,
		  Log_t logType);
  void _setHeader(const std::string &severityString, const std::string &fname, 
		  const int line, const std::string &method);
};

/**
 * @class LogState
 *
 * @brief Internal state is kept here, such as which types are enabled and
 *        where output is going
 */
class LogState
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
  static LogState *getPointer(void);

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
   * Set a particular type of message logging state to input value.
   * If state=true, logging of messages with this type will show up.
   * If state=false, logging of messages with this type will not show up.
   *
   * @param[in] logT  A log type
   * @param[in] state  The status to give this type
   */
  void setLogging(const LogStream::Log_t logT, const bool state);

  /**
   * Set a particular custom type of message logging state to input value.
   * If state=true, logging of messages with this type will show up.
   * If state=false, logging of messages with this type will not show up.
   *
   * @param[in] name A custom log type name
   * @param[in] state  The status to give this type
   *
   * If the custom type is not present, it is added 
   */
  void setLogging(const std::string &name, const bool state);

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
   * Set state for showing all sererity types in messages or not
   * If state=true, messages will show all types
   * If state=false, messages will not show the type in the messages
   *                 except for the 'bad' ones, ERROR, FATAL, WARNING
   * @param[in] state 
   */
  void setLoggingShowAllSeverityKeys(const bool state);

  /**
   * @return true if input logging type is enabled
   *
   * @param[in] logT  The type
   */
  bool isEnabled(LogStream::Log_t logT) const;

  /**
   * @return true if input custom logging type is enabled
   * @return false if it is disabled, or not present
   *
   * @param[in] name the custom name
   */
  bool isEnabled(const std::string &name) const;

  inline bool timestampIsEnabled(void) const
  {
    return _logRealTime;
  }

  inline bool showAllSeverityKeysIsEnabled(void) const
  {
    return _logShowAllSeverityKeys;
  }
  
  inline bool classMethodEnabled(void) const 
  {
    return _logClassAndMethod;
  }

  /**
   * Set DEBUG_VERBOSE = true;
   */
  void setVerbose(void);

  /**
   * Set DEBUG_VERBOSE = false;
   */
  void clearVerbose(void);

  /**
   * Add a custom type if it is not there.
   * @param[in] s  Name for the type.
   *
   * It is disabled if it is added. If it is already there, no action is taken
   */
  void addCustomTypeIfNew(const std::string &s);

  /**
   * Lock the mutex member
   */
  void lock(void);

  /**
   * Unlock the mutex member
   */
  void unlock(void);

  /**
   * Set to log to cout
   */
  void setCout(void);

  /**
   * Set to log to cerr
   */
  void setCerr(void);

  /**
   * Set to log to a log file
   * @param[in] app  App name for logfile naming
   * @param[in] instance  Instance to use in logfile naming
   * @param[in] logPath  Top path for logfiles, if empty use $LOG_DIR
   */
  void setLogFile(const std::string &app, const std::string &instance,
		  const std::string &logPath="");
 
  /**
   * @return true if the logging goes to cout
   */
  inline bool isCout(void) const {return _logOutputType == COUT;}

  /**
   * @return true if the logging goes to cerr
   */
  inline bool isCerr(void) const {return _logOutputType == CERR;}

  /**
   * @return true if the logging goes to logfiles
   */
  inline bool isLogFile(void) const {return _logOutputType == LOGFILE;}

  /**
   * Log the input string to the logfile.
   * @param[in] s  String to log, ready to use as is
   * @return true if logged to the file, false if not because of error
   *         or incorrect settings
   */
  bool logfileLog(const std::string &s);

  /**
   * Do formatted logging
   * @param[in] severity   the logging type
   * @param[in] nargs  Number of arguments associated with formatted write,
   *                   including the format string.  Will be 1 if it is a
   *                   formatted write with no arguments, only a format
   * @param[in] fname  Source code file name
   * @param[in] line   Source code line number
   * @param[in] method Source code method name
   * @param[in] fmt    format string
   * @param[in] ... optional additional args to go with the format
   */
  void logprint(LogStream::Log_t severity,
		int nargs, const std::string &fname, const int line,
		const std::string &method, const std::string &fmt, ...);

  /**
   * Accumulate input information into a local string without logging, with
   * a formatted argument list
   * @param[in] format  The format for the variable args
   * @param[in] ...   Additional arguments to go with the format
   */
  void accumulate(int nargs, std::string format, ...);

  /**
   * Log info accumulated using the accumulate() method then clear out
   * the accumulated info 
   *
   * @param[in] fname  Source code file name
   * @param[in] line   Source code line number
   * @param[in] method Source code method name
   * @param[in] severity  Logging type
   */
  void logAccum(const std::string &fname, const int line, 
 		const std::string &method, const LogStream::Log_t severity);

protected:
private:

  /**
   * Used to lock/unlock
   */
  pthread_mutex_t _printMutex;

  /**
   * State for enabling or disabling the various states, mapped from the enum.
   */
  std::map<LogStream::Log_t, bool>  _enabled;

  /**
   * State for enabling or disabling custom types, mapped from strings
   */
  std::map<std::string, bool>  _customEnabled;

  /**
   * True to log the real time values in the log messages
   */
  bool _logRealTime;

  /**
   * True to log class and method names in log messages
   */
  bool _logClassAndMethod;

  /**
   * True to show all the severity keys as part of message
   */
  bool _logShowAllSeverityKeys;

  /**
   * Possible outputs
   */
  typedef enum {COUT, CERR, LOGFILE} Logstream_Output_t;

  /**
   * Which output is being done
   */
  Logstream_Output_t _logOutputType;

  /**
   * Buffer to accumulate into using accumulate() and to write from using
   * logAccum()
   */
  std::string _accumBuf;

  /**
   * Constructor, members set to default values
   */
  LogState(void);

  /**
   * Destructor
   */
  ~LogState(void);

  void _log(const std::string &msg);

  /**
   * Form the header string based on member settings from inputs
   *
   * @param[in] fname  Source code file name
   * @param[in] line   Source code line number
   * @param[in] method Source code method name
   * @param[in] severity  Logging type
   */
  std::string _header(const std::string &fname, const int line,
		      const std::string &method,
		      const LogStream::Log_t severity);
};


/**
 * Some private macros
 */
#define LOG_STREAM_IS_ENABLED(s) (LogState::getPointer()->isEnabled((s)))

/**
 * Add a custom logging type if it is not there, and if it was added, disable it
 *
 * @param[in] s  String
 */
#define LOG_STREAM_ADD_CUSTOM_TYPE_IF_NEW(s) (LogState::getPointer()->addCustomTypeIfNew((s)))

#define LOG_STREAM_LOCK() (LogState::getPointer()->lock())
#define LOG_STREAM_UNLOCK() (LogState::getPointer()->unlock())
#define LOG_STREAM_IS_COUT() (LogState::getPointer()->isCout())
#define LOG_STREAM_IS_CERR() (LogState::getPointer()->isCerr())
#define LOG_STREAM_IS_LOGFILE() (LogState::getPointer()->isLogFile())
#define LOG_STREAM_LOGFILE_LOG(s) (LOGFILE_LOG(s))
#define LOG_STREAM_SHOW_ALL_SEVERITY_KEYS_ENABLED()  (LogState::getPointer()->showAllSeverityKeysIsEnabled())
#define LOG_STREAM_TIMESTAMP_ENABLED() (LogState::getPointer()->timestampIsEnabled())
#define LOG_STREAM_CLASSMETHOD_ENABLED() (LogState::getPointer()->classMethodEnabled())

/**
 * Hacky stuff to get number of arguments (needed for formatted prints)
 * up to 127 arguments
 */
#define PP_NARG(...) PP_NARG_(__VA_ARGS__,PP_RSEQ_N())
#define PP_NARG_(...) PP_ARG_N(__VA_ARGS__)
#define PP_ARG_N( \
          _1, _2, _3, _4, _5, _6, _7, _8, _9,_10, \
         _11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
         _21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
         _31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
         _41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
         _51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
         _61,_62,_63,_64,_65,_66,_67,_68,_69,_70, \
         _71,_72,_73,_74,_75,_76,_77,_78,_79,_80, \
         _81,_82,_83,_84,_85,_86,_87,_88,_89,_90, \
         _91,_92,_93,_94,_95,_96,_97,_98,_99,_100, \
         _101,_102,_103,_104,_105,_106,_107,_108,_109,_110, \
         _111,_112,_113,_114,_115,_116,_117,_118,_119,_120, \
         _121,_122,_123,_124,_125,_126,_127,N,...) N
#define PP_RSEQ_N() \
         127,126,125,124,123,122,121,120, \
         119,118,117,116,115,114,113,112,111,110, \
         109,108,107,106,105,104,103,102,101,100, \
         99,98,97,96,95,94,93,92,91,90, \
         89,88,87,86,85,84,83,82,81,80, \
         79,78,77,76,75,74,73,72,71,70, \
         69,68,67,66,65,64,63,62,61,60, \
         59,58,57,56,55,54,53,52,51,50, \
         49,48,47,46,45,44,43,42,41,40, \
         39,38,37,36,35,34,33,32,31,30, \
         29,28,27,26,25,24,23,22,21,20, \
         19,18,17,16,15,14,13,12,11,10, \
         9,8,7,6,5,4,3,2,1,0

#endif
