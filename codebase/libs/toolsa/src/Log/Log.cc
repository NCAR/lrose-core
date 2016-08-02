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
////////////////////////////////////////////////////////////////////////////////
//
// Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA
// January 1998
//
// $Id: Log.cc,v 1.19 2016/03/03 18:00:25 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////


#include <cstdio>
#include <ctime>
#include <toolsa/os_config.h>
#include <toolsa/DateTime.hh>
#include <toolsa/Log.hh>
#include <toolsa/file_io.h>
using namespace std;

Log::Log()
{
   init();
}

Log::Log( const string &appName, const char *instance )
{
   init();
   setApplication( appName, instance );
}

Log::~Log()
{
   clear();
}

void
Log::init()
{
   suffix = "log";
   useSuffix = true;
   appendMode = false;
   lockFile = false;
   dayMode = false;
   dayInUse = -1;
   hourMode = false;
   hourInUse = -1;
   _lockFd = NULL;

}

void 
Log::clear()
{
   if ( log.isValid() ) {
      logFile.close();
      log.clear();
   }
}

void Log::setApplication( const string &appName, const char *instance )
{
   saysWho = appName;

   if ( instance ) {
      saysWho += ".";
      saysWho += instance;
   }
}

void Log::setSuffix( const string &mySuffix )
{
   suffix = mySuffix;
}

void Log::noSuffix( )
{
   useSuffix = false;
}

int 
Log::setOutputDir( const char* outputDir )
{

   //
   // Degenerate case (removed by CSM - doesn't allow for file in ./)
   //
   // if ( log.getDirectory() == outputDir )
  //    return 0;
   
   //
   // Clear out any existing output file
   //
   clear();

   //
   // If a new log directory was specified... 
   //
   if ( outputDir  &&  !ISEMPTY( outputDir ) ) {

      outDir = outputDir;

      //
      // Use saysWho (if it exists) as the basename of the log file
      //
      if ( saysWho.size() ) {
	 logName = saysWho;
         if (useSuffix) {
	   logName += ".";
	   logName += suffix;
	 }
      } else {
	 logName = suffix;
      }

      //
      // open the output file
      //
      if (openFile()) {
	return -1;
      }

      //
      // It worked, keep the name of the new file
      //
      log.setPath( logPath );

   }
   
   return 0;
}

int 
Log::openFile()
{

  if (dayMode) {

    // in day mode, open new file if day has changed.
    // dayInUse is initialized to -1, so the file will
    // always be opened the first time through.

    time_t now = time(NULL);
    int thisDay = now / 86400;

    if (thisDay != dayInUse || !logFile.is_open()) {

      // day has changed, reopen file

      if (logFile.is_open()) {
	logFile.close();
      }
      DateTime dayTime(thisDay * 86400);
      int year, month, day;
      dayTime.getAll(&year, &month, &day);
      char dayStr[64];
      sprintf(dayStr, "%.4d%.2d%.2d", year, month, day);
      logPath = outDir;
      logPath += PATH_DELIM;
      logPath += dayStr;
      ta_makedir_recurse(logPath.c_str());
      logPath += PATH_DELIM;
      logPath += logName;

      if (appendMode) {
	logFile.open( logPath.c_str(), ios::app );
      } else {
	logFile.open( logPath.c_str() );
      }
      if ( logFile.is_open() ) {
	dayInUse = thisDay;
      }

    }

  } else if (hourMode) {

    // in hour mode, open a new file if hour has changed.
    // hourInUse is initialized to -1, so the file will
    // always be opened the first time through.

    time_t now = time(NULL);
    int thisHour = now / 3600;

    if (thisHour != hourInUse || !logFile.is_open()) {

      // hour has changed, reopen file

      if (logFile.is_open()) {
	logFile.close();
      }
      DateTime hourTime(thisHour * 3600);
      int year, month, day, hour;
      hourTime.getAll(&year, &month, &day, &hour);
      char dayStr[64];
      sprintf(dayStr, "%.4d%.2d%.2d", year, month, day);
      char hourStr[4];
      sprintf(hourStr, "%.2d", hour);

      logPath = outDir;
      logPath += PATH_DELIM;
      logPath += dayStr;
      ta_makedir_recurse(logPath.c_str());
      logPath += PATH_DELIM;
      logPath += logName;
      logPath += ".";
      logPath += hourStr; 

      if (appendMode) {
	logFile.open( logPath.c_str(), ios::app );
      } else {
	logFile.open( logPath.c_str() );
      }
      if ( logFile.is_open() ) {
	hourInUse = thisHour;
      }

    }

  } else {

    if (!logFile.is_open()) {

      ta_makedir_recurse(outDir.c_str());

      logPath = outDir;
      logPath += PATH_DELIM;
      logPath += logName;

      if (appendMode) {
	logFile.open( logPath.c_str(), ios::app );
      } else {
	logFile.open( logPath.c_str() );
      }

    }

  }

  if (!logFile.is_open()) {
    return -1;
  } else {
    return 0;
  }


}

void
Log::closeFile()
{
  if (logFile.is_open()) {
    logFile.close();
  }
}

void
Log::_lock()
{

  if (!lockFile) {
    return;
  }
  _lockFd = fopen(logPath.c_str(), "a+");
  ta_lock_file(logPath.c_str(), _lockFd, "w");

}

void
Log::_unlock()
{

  if (!lockFile) {
    return;
  }
  ta_unlock_file(logPath.c_str(), _lockFd);
  fclose(_lockFd);
  _lockFd = NULL;

}
