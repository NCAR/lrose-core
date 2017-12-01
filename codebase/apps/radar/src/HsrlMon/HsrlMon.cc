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
#include "RawFile.hh"
#include <dsserver/DsLdataInfo.hh>
#include <didss/DsInputPath.hh>
#include <toolsa/pmu.h>
#include <toolsa/file_io.h>
#include <toolsa/Path.hh>

#include <cmath>
#include <cerrno>
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

  // create monitoring fields

  if (_params.monitoring_fields_n < 1) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  No monitoring_fields specified." << endl;
    OK = FALSE;
    return;
  }

  for (int ii = 0; ii < _params.monitoring_fields_n; ii++) {
    const Params::monitoring_field_t &pfield = _params._monitoring_fields[ii];
    MonField *field = new MonField(_params, pfield.name, pfield.qualifier, 
                                   pfield.minValidValue, pfield.maxValidValue,
                                   pfield.note);
    _monFields.push_back(field);
  }

}

// destructor

HsrlMon::~HsrlMon()

{

  for (size_t ii = 0; ii < _monFields.size(); ii++) {
    delete _monFields[ii];
  }
  
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
  
  RadxTime realtimeScheduledTime;

  if (realtimeScheduledTime.utime() == 0) {
    time_t nextUtime = ((latestTime / interval) + 1) * interval;
    realtimeScheduledTime.set(nextUtime + delay);
    if (_params.debug) {
      cerr << "Next scheduled time for realtime mode: " 
           << realtimeScheduledTime.asString() << endl;
    }
  }

  int iret = 0;
  while (true) {

    PMU_auto_register("zzzzzzzzzz");
    time_t now = time(NULL);
    
    // check if we are at scheduled time

    if (now > realtimeScheduledTime.utime()) {
      
      _monitorStartTime = now - delay - _params.monitoring_interval_secs;
      _monitorEndTime = _monitorStartTime + _params.monitoring_interval_secs - 1;
      
      if (_performMonitoring(_monitorStartTime, _monitorEndTime)) {
        iret = -1;
      }
      
      // set next scheduled time
      
      time_t nextUtime = ((now / interval) + 1) * interval;
      realtimeScheduledTime.set(nextUtime + delay);
      if (_params.debug) {
        cerr << "Next scheduled time: "
             << realtimeScheduledTime.asString() << endl;
      }

    } // if (now > realtimeScheduledTime)
    
    umsleep(1000);
      
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

  _monitorStartTime = _args.startTime;
  while (_monitorStartTime < _args.endTime) {

    _monitorEndTime = _monitorStartTime + _params.monitoring_interval_secs - 1;
    if (_monitorEndTime > _args.endTime) {
      _monitorEndTime = _args.endTime;
    }

    if (_performMonitoring(_monitorStartTime, _monitorEndTime)) {
      iret = -1;
    }
    
    _monitorStartTime += _params.monitoring_interval_secs;

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

  RawFile rawFile(_params);
  if (!rawFile.isRawHsrlFile(filePath)) {
    cerr << "ERROR - HsrlMon::_processFile" << endl;
    cerr << "  Not an HSRL file: " << filePath << endl;
    return -1;
  }

  // get the start and end time of the data in the file

  time_t dataStartTime = 0, dataEndTime = 0;
  if (!rawFile.getStartAndEndTimes(filePath, dataStartTime, dataEndTime)) {
    cerr << "ERROR - HsrlMon::_processFile" << endl;
    cerr << "  Cannot read times from file: " << filePath << endl;
    return -1;
  }

  // loop through time periods in the file

  _monitorStartTime = dataStartTime;
  while (_monitorStartTime < dataEndTime) {
    
    _monitorEndTime = _monitorStartTime + _params.monitoring_interval_secs;
    if (_monitorEndTime > dataEndTime) {
      _monitorEndTime = dataEndTime;
    }
    
    // initialize
    
    _initMonFields();

    // add to stats

    if (_addToMonitoring(filePath,
                         _monitorStartTime,
                         _monitorEndTime)) {
      return -1;
    }

    // print out

    if (_params.debug) {
      _printStats(stderr);
    }
    
    if (_params.write_stats_files) {
      _writeStatsFile();
    }

    _monitorStartTime += _params.monitoring_interval_secs;
    
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
    cerr << "==================================================" << endl;
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

  _initMonFields();
  int iret = 0;
  for (size_t ii = 0; ii < filePaths.size(); ii++) {
    if (_addToMonitoring(filePaths[ii], startTime, endTime)) {
      iret = -1;
    }
  }
  
  if (_params.debug) {
    _printStats(stderr);
  }
  
  if (_params.write_stats_files) {
    _writeStatsFile();
  }
  
  if (_params.debug) {
    cerr << "==================================================" << endl;
  }

  return iret;

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
    cerr << "  Searching for files:" << endl;
    cerr << "    startDir: " << startDir << endl;
    cerr << "    endDir: " << endDir << endl;
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
      
  // select appropriate files
  
  for (size_t ii = 0; ii < filesFound.size(); ii++) {
    if (filesStartTime[ii] < endTime &&
        filesEndTime[ii] >= startTime) {
      filePaths.push_back(filesFound[ii]);
    }
  } // ii

  return 0;
  

}

////////////////////////////////////////////////////////
// Add to monitoring stats for specified file and times
// Returns 0 on success, -1 on failure

int HsrlMon::_addToMonitoring(const string &filePath,
                              time_t startTime,
                              time_t endTime)
{
  
  if (_params.debug) {
    cerr << "    adding monitoring stats from file: " << filePath << endl;
  }

  // open file

  RawFile rawFile(_params);
  if (rawFile.openAndReadTimes(filePath)) {
    cerr << "ERROR - HsrlMon::_addToMonitoring" << endl;
    cerr << rawFile.getErrStr() << endl;
    return -1;
  }

  int startTimeIndex = rawFile.getTimeIndex(startTime);
  int endTimeIndex = rawFile.getTimeIndex(endTime);
  
  for (size_t ii = 0; ii < _monFields.size(); ii++) {
    rawFile.appendMonStats(*_monFields[ii], startTimeIndex, endTimeIndex);
  }

  return 0;

}

////////////////////////////////////////////////////////
// Initialize the monitoring fields

void HsrlMon::_initMonFields()
{

  for (size_t ii = 0; ii < _monFields.size(); ii++) {
    _monFields[ii]->clear();
  }

}

////////////////////////////////////////////////////////
// Print the stats

void HsrlMon::_printStats(FILE *out)

{
  
  if (_monFields[0]->getNn() < 3) {
    cerr << "WARNING - HsrlMon::_printStats" << endl;
    cerr << "  No data found" << endl;
    cerr << "  monitorStartTime: " << RadxTime::strm(_monitorStartTime) << endl;
    cerr << "  monitorEndTime: " << RadxTime::strm(_monitorEndTime) << endl;
  }

  fprintf(out,
          "========================================"
          " HSRL MONITORING "
          "========================================\n");

  char label[128];
  sprintf(label, "Monitor start time - end time, N = %d",
          (int) (_monFields[0]->getNn()));

  fprintf(out, "%45s:   %s - %s\n",
          label,
          RadxTime::strm(_monitorStartTime).c_str(), 
          RadxTime::strm(_monitorEndTime).c_str());

  fprintf(out, "%45s  %10s %10s %10s %10s  %s\n",
          "", "Min", "Max", "Mean", "Range", "Notes");
          
  for (size_t ii = 0; ii < _monFields.size(); ii++) {
    _monFields[ii]->computeStats();
    _monFields[ii]->printStats(out);
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      _monFields[ii]->printStatsDebug(out);
    }
  }

  fprintf(out,
          "========================================"
          "================="
          "========================================\n");
  
}

////////////////////////////////////////////////////////
// Write out the stats files

void HsrlMon::_writeStatsFile()

{

  if (_monFields[0]->getNn() < 3) {
    if (_params.debug) {
      cerr << "WARNING - HsrlMon::_writeStatsFiles" << endl;
      cerr << "  No data found" << endl;
      cerr << "  monitorStartTime: " << RadxTime::strm(_monitorStartTime) << endl;
      cerr << "  monitorEndTime: " << RadxTime::strm(_monitorEndTime) << endl;
    }
    return;
  }

  //////////////////////
  // compute output dir
  
  string outputDir(_params.stats_output_dir);
  RadxTime fileTime(_monitorEndTime);
  char dayStr[1024];
  if (_params.stats_write_to_day_dir) {
    sprintf(dayStr, "%.4d%.2d%.2d",
            fileTime.getYear(),
            fileTime.getMonth(),
            fileTime.getDay());
    outputDir += PATH_DELIM;
    outputDir += dayStr;
  }
  
  // make sure output dir exists

  if (ta_makedir_recurse(outputDir.c_str())) {
    int errNum = errno;
    cerr << "ERROR - HsrlMon::_writeStatsFile()" << endl;
    cerr << "  Cannot create output dir: " << outputDir << endl;
    cerr << "  " << strerror(errNum) << endl;
    return;
  }

  /////////////////////
  // compute file name

  string fileName;
  
  // category
  
  if (strlen(_params.stats_file_name_category) > 0) {
    fileName += _params.stats_file_name_category;
  }
  
  // platform

  if (strlen(_params.stats_file_name_platform) > 0) {
    fileName += _params.stats_file_name_delimiter;
    fileName += _params.stats_file_name_platform;
  }

  // time
  
  if (_params.stats_include_time_part_in_file_name) {
    fileName += _params.stats_file_name_delimiter;
    char timeStr[1024];
    if (_params.stats_include_seconds_in_time_part) {
      sprintf(timeStr, "%.4d%.2d%.2d%.2d%.2d%.2d",
              fileTime.getYear(),
              fileTime.getMonth(),
              fileTime.getDay(),
              fileTime.getHour(),
              fileTime.getMin(),
              fileTime.getSec());
    } else {
      sprintf(timeStr, "%.4d%.2d%.2d%.2d%.2d",
              fileTime.getYear(),
              fileTime.getMonth(),
              fileTime.getDay(),
              fileTime.getHour(),
              fileTime.getMin());
    }
    fileName += timeStr;
  }

  // field label

  if (_params.stats_include_field_label_in_file_name) {
    fileName += _params.stats_file_name_delimiter;
    fileName += _params.stats_file_field_label;
  }

  // extension

  fileName += ".";
  fileName += _params.stats_file_name_extension;

  // compute output path

  string outputPath(outputDir);
  outputPath += PATH_DELIM;
  outputPath += fileName;

  // open the file

  FILE *out = fopen(outputPath.c_str(), "w");
  if (out == NULL) {
    int errNum = errno;
    cerr << "ERROR - HsrlMon::_writeStatsFile()" << endl;
    cerr << "  Cannot open file: " << outputPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return;
  }

  // write the file

  _printStats(out);

  fclose(out);
  
  if (_params.debug) {
    cerr << "==>> saved stats to file: " << outputPath << endl;
  }

  // write latest data info
  
  if (_params.stats_write_latest_data_info) {
    
    DsLdataInfo ldataInfo(_params.stats_output_dir);
    
    string relPath;
    Path::stripDir(_params.stats_output_dir, outputPath, relPath);
    
    if(_params.debug) {
      ldataInfo.setDebug();
    }
    ldataInfo.setLatestTime(fileTime.utime());
    ldataInfo.setWriter("HsrlMon");
    ldataInfo.setDataFileExt(_params.stats_file_name_extension);
    ldataInfo.setDataType(_params.stats_file_name_extension);
    ldataInfo.setRelDataPath(relPath);
    
    if(ldataInfo.write(fileTime.utime())) {
      cerr << "ERROR - HsrlMon::_writeStatsFile()" << endl;
      cerr << "  Cannot write _latest_data_info to dir: " << outputDir << endl;
      return;
    }
    
  } // if (_params.stats_write_latest_data_info)
  
}

