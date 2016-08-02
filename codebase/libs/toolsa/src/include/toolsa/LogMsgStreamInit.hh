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
 * @file LogMsgStreamInit.hh
 * @brief Initialization of LogMsg and LogStream using a wrapper class
 * with static methods
 * @class LogMsgStreamInit
 * @brief Initialization of LogMsg and LogStream using a wrapper class
 * with static methods
 */

#ifndef LOG_MSG_STREAM_INIT_HH
#define LOG_MSG_STREAM_INIT_HH

class LogMsgStreamInit
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
   * Set SPECIAL flag to input
   */
  static void setSpecial(bool state);

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

#ifdef NOTDEF
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
 * Enable a particular custom logging type. If the type is not present, it is
 * added and then enabled.
 *
 * @param[in] s  Custom logging type string
 */
#define LOG_STREAM_ENABLE_CUSTOM_TYPE(s) (LogState::getPointer()->setLogging((s),true))

/**
 * Set type to a status
 *
 * @param[in] s  type
 * @param[in] v  boolean
 */
#define LOG_STREAM_SET_TYPE(s,v) (LogState::getPointer()->setLogging(LogStream::s,(v)))


/**
 * Set custom type to a status
 *
 * @param[in] s  custom type string
 * @param[in] v  boolean
 */
#define LOG_STREAM_SET_CUSTOM_TYPE(s,v) (LogState::getPointer()->setLogging((s),(v)))

/**
 * Set cerr as stream to use
 */
#define LOG_STREAM_TO_CERR() (LogState::getPointer()->setCerr())

/**
 * Set coutas stream to use
 */
#define LOG_STREAM_TO_COUT() (LogState::getPointer()->setCout())


/**
 * Create a LogStream object to stream to. 
 *
 * @param[in] s  Logging type
 */
#define LOG(s) LogStream(__FILE__, __LINE__, __FUNCTION__, LogStream::s)

/**
 * Create a LogStream object to stream to. 
 *
 * @param[in] s  Logging type, format known
 */
#define LOGT(s) LogStream(__FILE__, __LINE__, __FUNCTION__, s)

/**
 * Create a LogStream object to stream to.
 *
 * @param[in] s  Custom logging string
 *
 * If the custom type is not present, it is added and enabled.
 */
#define LOGC(s) LogStream(__FILE__, __LINE__, __FUNCTION__, s)

/**
 * Create a LogStream object to stream to. 
 *
 * The log type is set to FORCE.
 */
#define LOG0  LogStream(__FILE__, __LINE__, __FUNCTION__)

/**
 * @class LogStream
 * @brief Logging of information to a stream
 */
class LogStream
{
public:
  typedef enum
  {
    DEBUG,
    DEBUG_VERBOSE,
    ERROR,
    FATAL,
    FORCE,
    PRINT,
    SEVERE,
    WARNING
  } Log_t;

  /**
   * @param[in] fname  File name
   * @param[in] line  Line number
   * @param[in] method Method name
   * @param[in] logT  The logging type
   */
  LogStream(const std::string &fname, const int line, const std::string &method,
	    Log_t logT);

  /**
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
   * @param[in] fname  File name
   * @param[in] line  Line number
   * @param[in] method Method name
   *
   * The Log_t is set to FORCE
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

private:

  std::ostringstream _buf;  /**< String stream storage */
  bool _active;             /**< True if logging will actually happen */

  void _setHeader(const std::string &fname, 
		  const int line, const std::string &method,
		  Log_t level);
  void _setHeader(const std::string &fname, 
		  const int line, const std::string &method);
};


/**
 * @class LogState
 * @brief Internal LogS state is kept here
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

  inline bool isCout(void) const {return _logToCout;}


protected:
private:

  pthread_mutex_t _printMutex;

  /**
   * State for enabling or disabling the various states.
   */
  std::map<LogStream::Log_t, bool>  _enabled;

  /**
   * State for enabling or disabling custom types
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
   * True to use cout, false to use cerr
   */
  bool _logToCout;

  /**
   * Constructor, no class name (class is the empty string.)
   *
   * Logging of all message severities is set to the singleton state,
   * the initial default state is every severity is enabled.
   *
   * Logging does include real time stamp (can be changed)
   */
  LogState(void);

  /**
   * Destructor
   */
  ~LogState(void);
};


/**
 * private macro
 */
#define LOG_STREAM_IS_ENABLED(s) (LogState::getPointer()->isEnabled((s)))

/**
 * private macro
 */
#define LOG_STREAM_TIMESTAMP_ENABLED() (LogState::getPointer()->timestampIsEnabled())

/**
 * private macro
 */
#define LOG_STREAM_CLASSMETHOD_ENABLED() (LogState::getPointer()->classMethodEnabled())

/**
 * private macro
 */
#define LOG_STREAM_LOCK() (LogState::getPointer()->lock())

/**
 * private macro
 */
#define LOG_STREAM_UNLOCK() (LogState::getPointer()->unlock())

/**
 * Add a custom logging type if it is not there, and if it was added, disable it
 *
 * @param[in] s  String
 */
#define LOG_STREAM_ADD_CUSTOM_TYPE_IF_NEW(s) (LogState::getPointer()->addCustomTypeIfNew((s)))

#define LOG_STREAM_IS_COUT() (LogState::getPointer()->isCout())
#endif

#endif
