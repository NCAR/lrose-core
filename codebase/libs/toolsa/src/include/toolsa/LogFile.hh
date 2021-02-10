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
 * @file LogFile.hh
 * @brief Logging of information to logfiles
 * @class LogFile
 * @brief Logging of information to logfiles
 *
 * The logging is handled by a static pointer to a LogFile object through use
 * of a small set of macro calls, which will create the pointer if need be.
 * The logging state is thus shared by all software that uses the macros, i.e. 
 * each app should log everything to the same log file
 */
#ifndef	LOG_FILE_H
#define	LOG_FILE_H

#include <string>
#include <vector>
#include <pthread.h>

/**
 * Initialize writing to log files
 *
 * @param[in] a  App
 * @param[in] i  Instance
 * @param[in] p  logfile Path
 */
#define LOGFILE_INIT(a,i,p) (LogFile::getPointer()->init((a),(i),(p)))

/**
 * Log a string to the logfile
 *
 * @param[in] s  String, fully formed but without endl
 */
#define LOGFILE_LOG(s) (LogFile::getPointer()->logFileLog((s)))

/**
 * Destroy the local state, closing files and freeing memory
 */
#define LOGFILE_DESTRUCT() (LogFile::getPointer()->finish())


/**
 * @class LogFile
 * @brief contains state information for log files
 */
class LogFile
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
  static LogFile *getPointer(void);

  /**
   * Initialize state
   *
   * @param[in] app  App name, used for logfile naming
   * @param[in] instance Instance used for logfile naming
   * @param[in] logPath  top path for log files, if empty use $LOGFILE_DIR,
   *                     and if that is not set us $LOG_DIR, and if that is
   *                     not set use $HOME
   */
  void init(const std::string &app, const std::string &instance,
	    const std::string &logPath="");

  /**
   * Close files and free memory
   */
  void finish(void);

  /**
   * Log the input string to the logfile
   *
   * @param[in] s  String 
   */
  bool logFileLog(const std::string &s);

protected:

private:

  /**
   * used to lock/unlock when writing or changing logfile state
   */
  pthread_mutex_t _printMutex;


  std::string _logPath;            /**< Top path for logfiles */
  bool _logFileIsOpen;             /**< True if a logfile is open */
  std::ofstream *_logFileStream;   /**< The stream for the logfile if open */
  bool _logFileIsSet;              /**< True if app/instance/logpath are set */
  std::string _app;                /**< App name */
  std::string _instance;           /**< Instance */
  int _logFileYear;                /**< Current year */
  int _logFileMonth;               /**< Current month */
  int _logFileDay;                 /**< Current day */

  /**
   * Constructor
   * Initializes members
   */
  LogFile(void);

  /**
   * Destructor
   */
  ~LogFile(void);

};

#endif

