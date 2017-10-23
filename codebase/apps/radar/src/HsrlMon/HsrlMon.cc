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
// HsrlMon.cc
//
// HsrlMon object
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
// Oct 2015
// 
///////////////////////////////////////////////////////////////
//
// HsrlMon read UW HSRL raw data files in NetCDF format,
// extracts data for monitoring, and then writes out text 
// files summarizing the monitoring information.
// This is intended for transmission to the field catalog
//
////////////////////////////////////////////////////////////////

#include "HsrlMon.hh"
#include "Names.hh"
#include "RawFile.hh"
#include <dsserver/DsLdataInfo.hh>
#include <didss/DsInputPath.hh>
#include <toolsa/pmu.h>
#include <toolsa/file_io.h>

#include <cmath>  
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>

#include <sys/types.h>
#include <dirent.h>

using namespace std;

// Constructor

HsrlMon::HsrlMon(int argc, char **argv)
  
{

  OK = TRUE;
  
  // set programe name

  _progName = "HsrlMon";
  
  // parse command line args
  
  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args." << endl;
    OK = FALSE;
    return;
  }
  
  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv,
			   _args.override.list, &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters." << endl;
    OK = FALSE;
    return;
  }

}

// destructor

HsrlMon::~HsrlMon()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int HsrlMon::Run()
{

  if (_params.mode == Params::FILELIST) {
    return _runFilelist();
  } else if (_params.mode == Params::REALTIME) {
    return _runRealtime();
  } else {
    return _runArchive();
  }

}

//////////////////////////////////////////////////
// Run in filelist mode

int HsrlMon::_runFilelist()
{

  // loop through the input file list
  
  int iret = 0;
  
  for (int ii = 0; ii < (int) _args.inputFileList.size(); ii++) {
    
    string inputPath = _args.inputFileList[ii];
    if (_processFileFromList(inputPath)) {
      iret = -1;
    }

  }

  return iret;

}

//////////////////////////////////////////////////
// Run in realtime mode

int HsrlMon::_runRealtime()
{
  
  if (_params.debug) {
    cerr << "INFO - HsrlMon" << endl;
    cerr << "  Running in REALTIME mode" << endl;
  }

  // init process mapper registration
  
  PMU_auto_init(_progName.c_str(), _params.instance,
                PROCMAP_REGISTER_INTERVAL);

  int interval = _params.realtime_interval_secs;
  int delay = _params.realtime_delay_secs;
  time_t latestTime = time(NULL);
  
  // initialize the schedule if required
  
  RadxTime _realtimeScheduledTime;

  if (_realtimeScheduledTime.utime() == 0) {
    time_t nextUtime = ((latestTime / interval) + 1) * interval;
    _realtimeScheduledTime.set(nextUtime + delay);
    if (_params.debug) {
      cerr << "Next scheduled time for realtime mode: " 
           << _realtimeScheduledTime.asString() << endl;
    }
  }

  int iret = 0;
  while (true) {

    PMU_auto_register("zzzzzzzzzz");
    time_t now = time(NULL);
    
    // check if we are at scheduled time

    if (now > _realtimeScheduledTime.utime()) {
      
      time_t monitorStartTime = now - delay - _params.monitoring_interval_secs;
      time_t monitorEndTime = monitorStartTime + _params.monitoring_interval_secs - 1;
      
      if (_performMonitoring(monitorStartTime, monitorEndTime)) {
        iret = -1;
      }
      
      // set next scheduled time
      
      time_t nextUtime = ((now / interval) + 1) * interval;
      _realtimeScheduledTime.set(nextUtime + delay);
      if (_params.debug) {
        cerr << "Next scheduled time: "
             << _realtimeScheduledTime.asString() << endl;
      }

    } // if (now > _realtimeScheduledTime)
      
  } // while

  return iret;

}

//////////////////////////////////////////////////
// Run in archive mode

int HsrlMon::_runArchive()
{
  
  if (_params.debug) {
    cerr << "INFO - HsrlMon" << endl;
    cerr << "  Running in ARCHIVE mode" << endl;
  }

  int iret = 0;

  time_t monitorStartTime = _args.startTime;
  while (monitorStartTime < _args.endTime) {

    time_t monitorEndTime = monitorStartTime + _params.monitoring_interval_secs - 1;
    if (monitorEndTime > _args.endTime) {
      monitorEndTime = _args.endTime;
    }

    if (_performMonitoring(monitorStartTime, monitorEndTime)) {
      iret = -1;
    }
    
    monitorStartTime += _params.monitoring_interval_secs;

  } // while

  return iret;
      
}

//////////////////////////////////////////////////
// Process a file from the input list
// Returns 0 on success, -1 on failure

int HsrlMon::_processFileFromList(const string &filePath)
{

  if (_params.debug) {
    cerr << "INFO - HsrlMon::_processFileFromList" << endl;
    cerr << "  File path: " << filePath << endl;
  }
  
  // check we have an HSRL file

  RawFile inFile(_params);
  if (!inFile.isRawHsrlFile(filePath)) {
    cerr << "ERROR - HsrlMon::_processFile" << endl;
    cerr << "  Not an HSRL file: " << filePath << endl;
    return -1;
  }

  // get the start and end time of the data in the file

  time_t dataStartTime = 0, dataEndTime = 0;
  if (!inFile.getStartAndEndTimes(filePath, dataStartTime, dataEndTime)) {
    cerr << "ERROR - HsrlMon::_processFile" << endl;
    cerr << "  Cannot read times from file: " << filePath << endl;
    return -1;
  }

  // loop through time periods in the file

  time_t monitorStartTime = dataStartTime;
  
  while (monitorStartTime < dataEndTime) {
    
    time_t monitorEndTime = monitorStartTime + _params.monitoring_interval_secs;
    if (monitorEndTime > dataEndTime) {
      monitorEndTime = dataEndTime;
    }
    
    if (_performMonitoring(filePath,
                           monitorStartTime,
                           monitorEndTime)) {
      return -1;
    }
    
    monitorStartTime += _params.monitoring_interval_secs;
    
  } // while

  return 0;

}

//////////////////////////////////////////////////
// Perform monitoring for a specified time
// Returns 0 on success, -1 on failure

int HsrlMon::_performMonitoring(time_t startTime,
                                time_t endTime)
{

  if (_params.debug) {
    cerr << "HsrlMon::_performMonitoring" << endl;
    cerr << "  Input dir: " << _params.input_dir << endl;
    cerr << "  Start time: " << RadxTime::strm(startTime) << endl;
    cerr << "  End time: " << RadxTime::strm(endTime) << endl;
  }

  // find the appropriate files

  vector<string> filePaths;
  _findFiles(startTime, endTime, filePaths);
  if (filePaths.size() == 0) {
    cerr << "ERROR - HsrlMon::_performMonitoring" << endl;
    cerr << "  No files found" << endl;
    cerr << "  Input dir: " << _params.input_dir << endl;
    cerr << "  Start time: " << RadxTime::strm(startTime) << endl;
    cerr << "  End time: " << RadxTime::strm(endTime) << endl;
    return -1;
  }

  int iret = 0;
  for (size_t ii = 0; ii < filePaths.size(); ii++) {
    if (_performMonitoring(filePaths[ii], startTime, endTime)) {
      iret = -1;
    }
  }

  return iret;

}

////////////////////////////////////////////////////////
// Perform monitoring for specified file and times
// Returns 0 on success, -1 on failure

int HsrlMon::_performMonitoring(const string &filePath,
                                time_t startTime,
                                time_t endTime)
{

  if (_params.debug) {
    cerr << "================>> HsrlMon::_performMonitoring" << endl;
    cerr << "  filePath: " << filePath << endl;
    cerr << "  start time: " << RadxTime::strm(startTime) << endl;
    cerr << "  end time: " << RadxTime::strm(endTime) << endl;
    cerr << "======================================================" << endl;
  }

  return 0;

}

////////////////////////////////////////////////////////
// Find appropriate files for selected times
// Returns 0 on success, -1 on failure

int HsrlMon::_findFiles(time_t startTime,
                        time_t endTime,
                        vector<string> &filePaths)

{

  filePaths.clear();
  
  RadxTime stime(startTime);
  char startDir[1024];
  sprintf(startDir, "%s%s%.4d%s%.2d%s%.2d%s%s",
          _params.input_dir, PATH_DELIM,
          stime.getYear(), PATH_DELIM,
          stime.getMonth(), PATH_DELIM,
          stime.getDay(), PATH_DELIM,
          _params.files_sub_dir);

  RadxTime etime(endTime);
  char endDir[1024];
  sprintf(endDir, "%s%s%.4d%s%.2d%s%.2d%s%s",
          _params.input_dir, PATH_DELIM,
          etime.getYear(), PATH_DELIM,
          etime.getMonth(), PATH_DELIM,
          etime.getDay(), PATH_DELIM,
          _params.files_sub_dir);

  if (_params.debug) {
    cerr << "Searching for files:" << endl;
    cerr << "==> startDir: " << startDir << endl;
    cerr << "==> endDir: " << endDir << endl;
  }

  _findFilesForDay(startTime, endTime, startDir, filePaths);
  if (strcmp(startDir, endDir)) {
    // ends on different date
    _findFilesForDay(startTime, endTime, endDir, filePaths);
  }

  return 0;

}

////////////////////////////////////////////////////////
// Find appropriate files for specified day
// Returns 0 on success, -1 on failure

int HsrlMon::_findFilesForDay(time_t startTime,
                              time_t endTime,
                              string dayDir,
                              vector<string> &filePaths)
  
{

  if (_params.debug) {
    cerr << "Searching for files in dir: " << dayDir << endl;
  }

  DIR *dirp;
  if ((dirp = opendir(dayDir.c_str())) == NULL) {
    cerr << "ERROR - HsrlMon::_findFilesForDay" << endl;
    cerr << "  Cannot open dir: " << dayDir << endl;
    return -1;
  }

  vector<string> filesFound;

  struct dirent *dp;
  for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
    
    // exclude dir entries and files beginning with '.'
    
    if (dp->d_name[0] == '.')
      continue;
    
    // compute path
    
    string path = dayDir;
    path += PATH_DELIM;
    path += dp->d_name;
    
    if (ta_stat_is_file(path.c_str())) {
      
      // file - check time
      
      RadxTime fileStartTime;
      if (RawFile::getTimeFromPath(path, fileStartTime)) {
        // date/time not in file name
        continue;
      }
      filesFound.push_back(path);

    } // if (ta_stat_is_dir ...
    
  } // endfor - dp
  
  closedir(dirp);

  // sort the file paths

  sort(filesFound.begin(), filesFound.end());

  // compute the files start and end times

  vector<time_t> filesStartTime;
  vector<time_t> filesEndTime;

  for (size_t ii = 0; ii < filesFound.size(); ii++) {
    RadxTime fileStartTime;
    RawFile::getTimeFromPath(filesFound[ii], fileStartTime);
    RadxTime fileEndTime = fileStartTime + _params.max_file_time_span_secs;
    if (ii != filesFound.size() - 1) {
      RawFile::getTimeFromPath(filesFound[ii+1], fileEndTime);
    }
    filesStartTime.push_back(fileStartTime.utime());
    filesEndTime.push_back(fileEndTime.utime() - 1);
  }
      
  // for (size_t ii = 0; ii < filesFound.size(); ii++) {
  //   cerr << "1111111111111 path, start, end: "
  //        << filesFound[ii] << ", "
  //        << RadxTime::strm(filesStartTime[ii]) << ", "
  //        << RadxTime::strm(filesEndTime[ii]) << endl;
  // }

  // select appropriate files
  
  for (size_t ii = 0; ii < filesFound.size(); ii++) {
    if (filesStartTime[ii] < endTime &&
        filesEndTime[ii] >= startTime) {
      filePaths.push_back(filesFound[ii]);
      // cerr << "----------------------------------------------" << endl;
      // cerr << "2222222222 using path, start, end: "
      //      << filesFound[ii] << ", "
      //      << RadxTime::strm(filesStartTime[ii]) << ", "
      //      << RadxTime::strm(filesEndTime[ii]) << endl;
    }
  } // ii

  return 0;
  

}

