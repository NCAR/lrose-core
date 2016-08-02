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
// $Id: Log.hh,v 1.18 2016/03/03 18:00:26 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////
#ifndef	_LOG_INC_
#define	_LOG_INC_


#include <string>
#include <cstdio>
#include <fstream>
#include <toolsa/Path.hh>
using namespace std;

class Log {
public:
   Log();
   Log( const string &appName,
        const char *instance = NULL );
   virtual ~Log();

   void      setApplication( const string &appName, 
                             const char *instance = NULL );
   void      setSuffix( const string &suffix ) ;
   void      noSuffix( );
   void      useFileLocking() { lockFile = true; }
   void      setAppendMode() { appendMode = true; }

   // In day mode, a new file is created each day.
   // Files are opened to append to day file.
   // To invoke day mode, setDayMode() must be called before
   // setOutputDir()

   void      setDayMode(bool day_mode = true) { dayMode = day_mode; }

   // In hour mode, a new file is created each hour.
   // Files are opened to append to each hour file.
   // To invoke hour mode, setHourMode() must be called before
   // setOutputDir()

   // if hourMode & dayMode are both true, then dayMode overrides hourMode

   void      setHourMode(bool hour_mode = true) { hourMode = hour_mode; }

   // note: setOutputDir calls openFile

   int       setOutputDir( const char *outputDir );

   // open and close the log file

   int       openFile(); // note: no action if already open
   void      closeFile();

   // are we using a file?

   bool      isOutputToFile(){ return( log.isValid() ); }

protected:

   string    saysWho;
   string    suffix;

   bool      useSuffix;
   bool      appendMode;
   bool      lockFile;

   bool      dayMode;
   int       dayInUse;

   bool      hourMode;
   int       hourInUse;

   ofstream  logFile;

   void      _lock();
   void      _unlock();
   FILE      *_lockFd;

   const char* getLogPath(void) { return logPath.c_str(); }
    
private:
   string    outDir;
   string    logName;
   string    logPath;
   Path      log;

   void      init();
   void      clear();

};

#endif
