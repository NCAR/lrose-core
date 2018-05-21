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
/************************************************************************
 * file_repeat_day.hh : header file for the file_repeat_day program class.
 *
 * RAP, NCAR, Boulder CO
 *
 * November 1998
 *
 * Niles Oien, modified by Mike Dixon
 *
 ************************************************************************/

#ifndef file_repeat_day_HH
#define file_repeat_day_HH

#include "Args.hh"
#include "Params.hh"
#include <vector>
#include <string>
#include <toolsa/udatetime.h>
#include <toolsa/DateTime.hh>
#include <toolsa/file_io.h>
using namespace std;

class FileInfo {
public:
  date_time_t time;
  string path;
};

class file_repeat_day

{

public:

  // constructor
  
  file_repeat_day(int argc, char **argv);
  
  // Destructor

  ~file_repeat_day(void);
  
  // Run the program.

  int Run();
  
  // Flag indicating whether the program status is currently okay.

  bool isOK;
  
private:
  
  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;

  int _prevJulDay;
  
  vector<FileInfo> _inputFiles;

  static const size_t YYYYMMDDHH_LEN = 10;
  static const size_t YYYYMMDDHHMMSS_LEN = 14;
  static const size_t HH_YYYYMMDDHHMMSS_LEN = 17;
  static const size_t YYYYMMDD_HHMM_LEN = 13;

  // functions

  int _runRepeatDay();
  int _runRepeatMonth();
  int _runForDayInMonth(DateTime refTime);
  int _processDay(int timeIndex);

  int _scanInputDir(const string &inDirPath);

  int _computeFileTime(const string &filePath,
                       const string &fname,
                       date_time_t &ftime);

  int _findStartIndex();
  time_t _computeOutputTime(int itime);
  time_t _computeWakeupTime(int itime);

  int _computeOutputPath(int itime,
			 const char *inputPath,
			 time_t outputTime,
			 char *outputPath);
  
  int _copyFile(const char *inputPath,
		const char *outputPath,
		time_t outputTime);
    
  int _copyAsciiFile(FILE *ifp,
	             FILE *ofp,
		     time_t outputTime);
    
  int _straightCopy(FILE *ifp,
		    FILE *ofp,
		    time_t outputTime);
    
  int _copyNetcdfFile(const char *inputPath,
	             const char *outputPath,
		     time_t outputTime);
    
  void _substituteTime(char *line,
		       time_t outputTime);

  void _substituteStartExpire(char *line,
			      time_t outputTime);

  void _tokenize(const string &str,
                 const string &spacer,
                 vector<string> &toks);

  void _callNcap2(const vector<string> &args);  

  void _execScript(const vector<string> &args,
		   const char *script_to_call);
  
  void _killAsRequired(pid_t pid,
		       time_t terminate_time);

  
  int _buildCmdList(time_t output_time,
		    string &cmd_list);
};



#endif


