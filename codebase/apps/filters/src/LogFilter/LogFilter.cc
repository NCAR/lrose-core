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
///////////////////////////////////////////////////////////////
//
// LogFilter.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2000
//
///////////////////////////////////////////////////////////////
//
// LogFilter reads stdin and copies it to log files, adding
// time information as appropriate.
//
////////////////////////////////////////////////////////////////

#include "LogFilter.hh"
#include <toolsa/MsgLog.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/umisc.h>
#include <toolsa/file_io.h>
using namespace std;

LogFilter::LogFilter (int argc, char **argv)
     
{

  isOK = true;

  // set programe name

  _progName = "LogFilter";
  // ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }
  return;
}

//////////////
// destructor
//

LogFilter::~LogFilter ()

{
  return;
}

////////////////////////////////////////////
// run
//
// Read stdin, write to log file.
//

int LogFilter::Run()
{

  // close stdout and stderr

  fclose(stdout);
  // fclose(stderr);

  // message log for output

  MsgLog msgLog;
  if (_args.instance.size() > 0) {
    msgLog.setApplication(_args.procName.c_str(),
			  _args.instance.c_str());
  } else {
    msgLog.setApplication(_args.procName.c_str());
  }
  msgLog.setSuffix("log");
  msgLog.setAppendMode();
  if (_args.hourMode) {
    msgLog.setHourMode();
  } else if (!_args.singleFileMode) {
    msgLog.setDayMode();
  }

  msgLog.setOutputDir(_args.logDir.c_str());

  // start time
  time_t startTime = time(NULL);
  
  // ending time
  time_t endTime = 0;
  if (_args.tInterval > 0) {
    endTime = (((startTime / _args.tInterval) + 1) * _args.tInterval);
    if (_args.debug) {
      cerr << "End of current period: " << utimstr(endTime) << endl;
    }
  }

  bool forever = true;
  
  while (forever) {

    if (feof(stdin)) {
      //  msgLog.openFile();
      //  msgLog.postMsg(">>> LogFilter :: stdin was closed <<<\n" );
      //  msgLog.closeFile();
      if (!(_args.keepRunningOnError)) return 0;
    }

    int iret = ta_read_select(stdin, 10000);

    if (iret == -2) {
      msgLog.openFile();
      msgLog.postMsg(">>> LogFilter :: ta_read_select returned -2 <<<\n" );
      msgLog.closeFile();
      // error
      if (!(_args.keepRunningOnError)) return 0;
    }

    if (iret >= 0) {

      // select succeeded - read next line
      
      char lineIn[4096];
      char lineOut[8192];
      if (fgets(lineIn, 4096, stdin) != NULL) {
	// strip line feed
	lineIn[strlen(lineIn)-1] = '\0';
        if (_args.timeStampEachLine) {
          DateTime now(time(NULL));
          sprintf(lineOut, "%.2d:%.2d:%.2d - %s",
                  now.getHour(), now.getMin(), now.getSec(), lineIn);
        } else {
          strcpy(lineOut, lineIn);
        }
	msgLog.openFile();
	msgLog.postLine(lineOut);
	msgLog.closeFile();
      }

    }

    if (endTime > 0) {

      // check if we should output time stamp
	
      time_t now = time(NULL);
      if (now >= endTime) {
	if (_args.debug) {
	  cerr << ">>> LogFilter time stamp: "
	       << DateTime::str(endTime, false) << endl;
	}
	msgLog.openFile();
	msgLog.postMsg(">>> LogFilter time stamp: %s <<<",
		       DateTime::str(endTime, false).c_str());
	msgLog.closeFile();
	endTime += _args.tInterval;
	if (_args.debug) {
	  cerr << "End of current period: " << utimstr(endTime) << endl;
	}
      } // if (now >= endTime)
      
    }

  } // while
  
  return 0;

}



