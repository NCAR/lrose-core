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
 * @file LogFile.cc
 */

//----------------------------------------------------------------
#include <toolsa/LogFile.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/Path.hh>
#include <sstream>
#include <iostream>
#include <fstream>
#include <cstdlib>

using std::ofstream;

/**
 * A static pointer used to make this a singleton class
 */
static LogFile *pLogFile = NULL;

//----------------------------------------------------------------
void LogFile::initPointer(void)
{
  if (pLogFile == NULL)
  {
    pLogFile = new LogFile();
  }
}

//----------------------------------------------------------------
void LogFile::freePointer(void)
{
  if (pLogFile != NULL)
  {
    delete pLogFile;
    pLogFile = NULL;
  }
}

//----------------------------------------------------------------
LogFile *LogFile::getPointer(void)
{
  initPointer();
  return pLogFile;
}

//----------------------------------------------------------------
LogFile::LogFile() :
  _logPath(""),
  _logFileIsOpen(false),
  _logFileStream(NULL),
  _logFileIsSet(false),
  _app("unknown"),
  _instance("unknown"),
  _logFileYear(0),
  _logFileMonth(0),
  _logFileDay(0)
{
  pthread_mutex_init(&_printMutex, NULL);
}

//----------------------------------------------------------------
LogFile::~LogFile()
{
  finish();
  pthread_mutex_destroy(&_printMutex);
}

//----------------------------------------------------------------
void LogFile::init(const std::string &app, const std::string &instance,
		   const std::string &logPath)
{
  pthread_mutex_lock(&_printMutex);
  string lp;
  if (logPath.empty())
  {
    lp = getenv("LOGFILE_DIR");
    if (lp.empty())
    {
      cerr << "LOGFILE_DIR environment not set, using LOG_DIR";
      lp = getenv("LOG_DIR");
      if (lp.empty())
      {
	cerr << "LOG_DIR environment not set, using curent home directory";
	lp = "$HOME";
      }
    }
  }
  else
  {
    lp = logPath;
  }

  if (_logFileIsSet)
  {
    if (app != _app || _instance != instance)
    {
      cerr << "ERROR inconsistent initialization app/instance"
	   << " local = " << _app << "/" << _instance
	   << " input = " << app << "/" << instance
	   << " Keep local" << endl;
    }
    if (lp != _logPath)
    {
      cerr << "ERROR inconsistent initialization logPath"
	   << " local = " << _logPath 
	   << " input = " << lp
	   << " Keep local" << endl;
      lp = _logPath;
    }	
  }
  else
  {
    _logFileIsSet = true;
    _logPath = lp;
    _app = app;
    _instance = instance;
  }
  pthread_mutex_unlock(&_printMutex);
}

//----------------------------------------------------------------
void LogFile::finish(void)
{
  pthread_mutex_lock(&_printMutex);
  if (_logFileIsOpen)
  {
    _logFileIsOpen = false;
    _logFileIsSet = false;
    _logFileStream->close();
    delete _logFileStream;
  }
  pthread_mutex_unlock(&_printMutex);
}

//----------------------------------------------------------------
bool LogFile::logFileLog(const std::string &s)
{
  pthread_mutex_lock(&_printMutex);
  if (_logPath.empty())
  {
    pthread_mutex_unlock(&_printMutex);
    return false;
  }
  
  time_t t = time(0);
  DateTime dt(t);
  int year = dt.getYear();
  int month = dt.getMonth();
  int day = dt.getDay();
  
  bool redo = false;
  if (_logFileIsOpen && (year != _logFileYear || month != _logFileMonth ||
			 day != _logFileDay))
  {
    _logFileStream->close();
    delete _logFileStream;
    redo = true;
  }
  else if (!_logFileIsOpen)
  {
    redo = true;
  }

  if (redo)
  {
    char buf[1000];
    sprintf(buf,"%s/%04d%02d%02d", _logPath.c_str(), year, month, day);
    string path = buf;
    sprintf(buf, "%s.%s.Log", _app.c_str(), _instance.c_str());
    string fname = buf;
    Path p(path, fname);
    p.makeDirRecurse();
    string fullpath = path + "/";
    fullpath = fullpath + fname;
    _logFileYear = year;
    _logFileMonth = month;
    _logFileDay = day;
    _logFileStream = new ofstream(fullpath.c_str());
    _logFileIsOpen = _logFileStream->is_open();
  }

  if (_logFileIsOpen)
  {
    (*_logFileStream) << s << endl;
    pthread_mutex_unlock(&_printMutex);
    return true;
  }
  else
  {
    pthread_mutex_unlock(&_printMutex);
    return false;
  }
}

