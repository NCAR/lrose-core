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
/*********************************************************************
 * file_repeat_day.cc: A program that simulates a realtime data source.
 *
 * RAP, NCAR, Boulder CO
 *
 * May 1999
 *
 * Holin Tsai
 *
 *********************************************************************/

#include <cstdio>
#include <cerrno>
//#include <signal.h>
#include <csignal>
#include <sys/wait.h>
#include <cstring>
#include <algorithm>
#include <functional>

#include "file_repeat_day.hh"

#include <dsserver/DsLdataInfo.hh>
#include <toolsa/TaArray.hh>
#include <toolsa/pmu.h>
#include <toolsa/umisc.h>
#include <toolsa/Path.hh>
using namespace std;

//////////////////////////////////////////////////
// Constructor

file_repeat_day::file_repeat_day(int argc, char **argv)

{

  isOK = true;

  // set programe name
  
  _progName = "file_repeat_day";
  ucopyright((char *) _progName.c_str());
  
  // get command line args
  
  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }
  
  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = false;
    return;
  }

  // initialize data

  _prevJulDay = -1;

  // initialize process registration
  
  PMU_auto_init(_progName.c_str(), _params.instance,
		PROCMAP_REGISTER_INTERVAL);

}


/////////////////////////////
// Destructor

file_repeat_day::~file_repeat_day()
{

  // unregister process

  PMU_auto_unregister();

}

/////////////////////////////////////
// Run()

int file_repeat_day::Run()
{

  PMU_auto_register("Run");

  int iret = 0;

  if (_params.RepeatMonth) {
    
    iret = _runRepeatMonth();
    
  } else {
    
    iret = _runRepeatDay();
    
  }

  return iret;

}

/////////////////////////////////////
// repeat day
//
// Input data is from a single day

int file_repeat_day::_runRepeatDay()
{

  PMU_auto_register("_runRepeatDay");

  // compute input dir
  
  char inDirPath[MAX_PATH_LEN];
  if (_params.FilenameType == Params::HHMMSS ||
      _params.FilenameType == Params::SDIR_PRE_YYYYMMDD_HHMM_POST ||
      _params.FilenameType == Params::SDIR_PRE_YYYYMMDDhhmmss_POST) {
    sprintf(inDirPath, "%s%s%.4d%.2d%.2d",
	    _params.InDir, PATH_DELIM,
	    _params.InYear, _params.InMonth, _params.InDay);
  } else {
    sprintf(inDirPath, "%s", _params.InDir);
  }
  
  // scan the input directory, to set up the time list

  if (_scanInputDir(inDirPath)) {
    cerr << "ERROR - file_repeat_day::_runRepeatDay" << endl;
    cerr << "  Cannot scan input dir: " << inDirPath << endl;
    return -1;
  }

  // loop for days
  
  while (true) {

    // get the starting index
  
    int timeIndex = _findStartIndex();
  
    PMU_auto_register("_runRepeatDay");

    if (_processDay(timeIndex)) {
      cerr << "ERROR - file_repeat_day::_runRepeatDay" << endl;
      return -1;
    }

    // wait for change in day

    time_t now = time(NULL);
    DateTime midday(now - SECS_IN_DAY / 2);
    int day = midday.getDay();
    while (true) {
      DateTime dnow(time(NULL));
      if (dnow.getDay() != day) {
	break;
      }
      PMU_auto_register("Zzzz....");
      umsleep(1000);
    }

  } // while (true)
  
  return 0;

}

/////////////////////////////////////
// _runRepeatMonth()
//
// Input data is from a complete month

int file_repeat_day::_runRepeatMonth()
{

  PMU_auto_register("_runRepeatMonth");

  while (true) {

    // compute the current julian day

    DateTime now(time(NULL));
    int julDay = now.utime() / SECS_IN_DAY;

    if (julDay != _prevJulDay) {
      if (_runForDayInMonth(now)) {
        return -1;
      }
      _prevJulDay = julDay;
    }

    // wait for change in day

    while (true) {
      DateTime then(time(NULL));
      int julDayThen = then.utime() / SECS_IN_DAY;
      if (julDayThen == julDay) {
	PMU_auto_register("Zzzz....");
	umsleep(1000);
      }
    }
    
  } // while

  return 0;

}

//////////////////////////////////////////////
// _runForDayInMonth()
//
// Run for day in month, given the reference date/time

int file_repeat_day::_runForDayInMonth(DateTime refTime)
{

  PMU_auto_register("_runForDayInMonth");

  // determine the input dir to use
  // scan available dates, find the one with the day closest to the refTime
  
  DIR *dirp;
  if ((dirp = opendir(_params.InDir)) == NULL) {
    fprintf(stderr,
            "Cannot open input directory '%s'\n", _params.InDir);
    return -1;
  }

  int minDayDiff = 1000;
  int bestDay = -1;
  string bestDayDir;
  
  struct dirent *dp;
  for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {

    // exclude dir entries and files beginning with '.'
    
    if (dp->d_name[0] == '.') {
      continue;
    }
    
    // check is dir
    
    char dayPath[MAX_PATH_LEN];
    sprintf(dayPath, "%s%s%s",
	    _params.InDir, PATH_DELIM, dp->d_name);
    if (!ta_stat_is_dir(dayPath)) {
      continue;
    }
    
    // get year, month, day

    int year, month, day;
    if (sscanf(dp->d_name, "%4d%2d%2d", &year, &month, &day) != 3) {
      cerr << "WARNING - file_repeat_day::_runForDayInMonth" << endl;
      cerr << "  Bad day directory: " << dp->d_name << endl;
      cerr << "  Input dir: " << _params.InDir << endl;
      continue;
    }
    
    int dayDiff = abs(day - refTime.getDay());
    if (dayDiff < minDayDiff) {
      bestDay = day;
      minDayDiff = dayDiff;
      bestDayDir = dayPath;
    }
    
  } // endfor dp
  closedir(dirp);

  if (bestDay < 1) {
    cerr << "ERROR - file_repeat_day::_runForDayInMonth" << endl;
    cerr << "  Cannot find suitable day dir" << endl;
    cerr << "  Ref time: " << DateTime::strm(refTime.utime()) << endl;
    cerr << "  Input dir: " << _params.InDir << endl;
    return -1;
  }
  
  // scan the input directory, to set up the time list

  if (_scanInputDir(bestDayDir)) {
    cerr << "ERROR - file_repeat_day::_runForDayInMonth" << endl;
    cerr << "  Cannot scan input dir: " << bestDayDir << endl;
    return -1;
  }

  // get the starting index

  int timeIndex = _findStartIndex();

  // process this day

  if (_processDay(timeIndex)) {
    cerr << "ERROR - file_repeat_day::_runForDayInMonth" << endl;
    return -1;
  }

  return 0;

}


//////////////////////////////////////////////
// process a single day
//
// Run for day in month, given the reference date/time

int file_repeat_day::_processDay(int timeIndex)
{

  while (timeIndex < (int) _inputFiles.size()) {

    PMU_auto_register("_processDay");

    // get the output time
    
    time_t outputTime = _computeOutputTime(timeIndex);

    // compute the paths
    
    const char *inputPath = _inputFiles[timeIndex].path.c_str();
    char outputPath[MAX_PATH_LEN];
    if (_computeOutputPath(timeIndex, inputPath, outputTime, outputPath)) {
      cerr << "ERROR - file_repeat_day::_runRepeatDay()" << endl;
      cerr << "  Cannot compute output file path" << endl;
      return -1;
    }
    
    if (_params.debug) {
      cerr << "  input path: " << inputPath << endl;
      cerr << "  output path: " << outputPath << endl;
    }

    // copy the file across
    
    if (_copyFile(inputPath, outputPath, outputTime)) {
      cerr << "ERROR - file_repeat_day::_runRepeatDay()" << endl;
      cerr << "  Cannot copy files" << endl;
      cerr << "  input path: " << inputPath << endl;
      cerr << "  output path: " << outputPath << endl;
      return -1;
    }

    if (_params.WriteLData) {
      DsLdataInfo ldata(_params.OutDir);
      ldata.setWriter("file_repeat_day");
      string relDataPath;
      Path::stripDir(_params.OutDir, outputPath, relDataPath);
      ldata.setRelDataPath(relDataPath.c_str());
      if (_params.FilenameType != Params::MOD_TIME) {
	ldata.setDataFileExt(_params.FileExtension);
      }
      if (ldata.write(outputTime)) {
	cerr << "ERROR - file_repeat_day::_runRepeatDay()" << endl;
	cerr << "  Cannot write latest data info" << endl;
	cerr << "  output dir: " << _params.OutDir << endl;
	cerr << "  time: " << utimstr(outputTime) << endl;
	return -1;
      }
    }
    
    // move ahead in the time list
   
    int prevIndex = timeIndex;
    timeIndex++;
    if (timeIndex > (int) (_inputFiles.size() - 1)) {
      // done for this day
      return 0;
    }
    
    // Are there multiple input files for one time?
    // Is this time the same as the previous time?

    bool do_sleep=true;

    if (_params.MultipleFilesForOneTime) {
      if ((_inputFiles[timeIndex].time.hour == _inputFiles[prevIndex].time.hour) &&
	  (_inputFiles[timeIndex].time.min == _inputFiles[prevIndex].time.min) &&
	  (_inputFiles[timeIndex].time.sec == _inputFiles[prevIndex].time.sec)) {
	do_sleep=false;
	// Is there a specific sleep between serving these multiple files
	if (_params.SleepSecsBetweenFilesForOneDataTime > 0) {
	  if (_params.debug) {
	    cerr << "Sleeping between files for same data time: "
                 << _params.SleepSecsBetweenFilesForOneDataTime << " secs" << endl;
	  }
	  sleep(_params.SleepSecsBetweenFilesForOneDataTime);
	}
      }
    }
      
    // compute the wake-up time and sleep for awhile
    if (do_sleep) {

      time_t wakeUpTime = _computeWakeupTime(timeIndex);
      time_t now = time(NULL);
      while (now < wakeUpTime) {
	PMU_auto_register("Zzzz....");
	umsleep(1000);
	now = time(NULL);
      }
    }
    
  } // while

  return 0;

}
  
/////////////////////////////////////////////////////////////
// class for comparing date_time_t structs - used for sorting

class fileTimeCompare { // compare date_time_t entries in FileInfo
public:
  bool operator() (const FileInfo &a, const FileInfo &b) const;
};

bool fileTimeCompare::operator()
  (const FileInfo &a, const FileInfo &b) const
{
  if (a.time.unix_time < b.time.unix_time) {
    return true;
  } else {
    return false;
  }
}

///////////////////////////////////////////////
// scan the input directory for a list of times
//
// returns 0 on success, -1 on failure

int file_repeat_day::_scanInputDir(const string &inDirPath)

{
  
  DIR *dirp;
  if ((dirp = opendir(inDirPath.c_str())) == NULL) {
    cerr << "Cannot open directory: " << inDirPath << endl;
    return -1;
  }

  struct dirent *dp;
  for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {

    // exclude dir entries and files beginning with '.'

    if (dp->d_name[0] == '.') {
      continue;
    }

    // compute path

    string filePath = inDirPath;
    filePath += PATH_DELIM;
    filePath += dp->d_name;

    // directory??
    
    if (ta_stat_is_dir(filePath.c_str())) {
      continue;
    }
    if (_params.debug){
      cerr << "Found file " << dp->d_name << endl;
    }

    // compute file time

    date_time_t ftime;
    if (_computeFileTime(filePath, dp->d_name, ftime)) {
      continue;
    }
    FileInfo info;
    info.time = ftime;
    info.path = filePath;
    _inputFiles.push_back(info);

  } // endfor dp
  closedir(dirp);
  
  //
  // Sort the times into ascending order.
  //

  sort(_inputFiles.begin(), _inputFiles.end(), fileTimeCompare());
  
  if (_params.debug) {
    cerr << "  File list:" << endl;
    for (size_t ii = 0; ii < _inputFiles.size(); ii++) {
      fprintf(stderr, "  %d: %.2d:%.2d:%.2d %s\n",
	      (int)ii,
	      _inputFiles[ii].time.hour,
	      _inputFiles[ii].time.min,
	      _inputFiles[ii].time.sec,
	      _inputFiles[ii].path.c_str());
    }
  }

  if (_inputFiles.size() == 0) {
    cerr << "ERROR - file_repeat_day::scanInputDir()" << endl;
    cerr << "  No time files found" << endl;
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////////////////
// scan the file name for time, based on type

int file_repeat_day::_computeFileTime(const string &filePath,
                                      const string &fname,
                                      date_time_t &ftime)

{

  date_time_t tmpTime;
  char ext[64];

  switch (_params.FilenameType) {
    
    case Params::HHMMSS: {
      
      // Files are named HHMMSS.ext
      
      if (4==sscanf(fname.c_str(),"%2d%2d%2d.%s",
		    &tmpTime.hour, &tmpTime.min,&tmpTime.sec, ext)){
	if (!strcmp(ext, _params.FileExtension)) {
	  ftime.year=1970;
	  ftime.month=1;
	  ftime.day=1;
	  ftime.hour = tmpTime.hour;
	  ftime.min = tmpTime.min;
	  ftime.sec = tmpTime.sec;
	  uconvert_to_utime(&ftime);
          return 0;
	}
      }
      break;
    }
      
    case Params::YYYYMMDDHHMM: {
      
      // Files are named YYYYMMDDHHMM.ext
      
      if (6==sscanf(fname.c_str(),"%4d%2d%2d%2d%2d.%s",
		    &tmpTime.year, &tmpTime.month, &tmpTime.day,
		    &tmpTime.hour, &tmpTime.min, ext)){
	if (!strcmp(ext, _params.FileExtension) &&
	    (tmpTime.day == _params.InDay) && 
	    (tmpTime.month == _params.InMonth) &&
	    (tmpTime.year == _params.InYear)){
	  ftime.year=1970;
	  ftime.month=1;
	  ftime.day=1;
	  ftime.hour = tmpTime.hour;
	  ftime.min = tmpTime.min;
	  ftime.sec = 0;
	  uconvert_to_utime(&ftime);
          return 0;
	}
      }
      break;
    }
      
    case Params::YYYYMMDDHHMMSS: {
      
      // Files are named prefixYYYYMMDDHHMMSS.ext
      // Separate the prefix from the rest of the filename
      
      size_t name_len=fname.size();
      size_t prefix_len=strlen(_params.FilePrefix);
      
      if (name_len > prefix_len) {
        string prefix_str=fname.substr(0, prefix_len);
        string after_prefix=fname.substr(prefix_len, name_len-prefix_len);
        
        if (7==sscanf(after_prefix.c_str(),"%4d%2d%2d%2d%2d%2d.%s",
                      &tmpTime.year, &tmpTime.month, &tmpTime.day,
                      &tmpTime.hour, &tmpTime.min, &tmpTime.sec, ext)) {
          if (!strcmp(prefix_str.c_str(), _params.FilePrefix) &&
              (tmpTime.month == _params.InMonth) &&
              (tmpTime.year == _params.InYear)){
            ftime.year=1970;
            ftime.month=1;
            ftime.day=1;
            ftime.hour = tmpTime.hour;
            ftime.min = tmpTime.min;
            ftime.sec = tmpTime.sec;
            uconvert_to_utime(&ftime);
            return 0;
          }
        }
      }
      break;
    }

    case Params::MMDD_HHMM: {
      
      // Files are named MMDD_HHMM.ext
      
      if (5==sscanf(fname.c_str(),"%2d%2d_%2d%2d.%s",
		    &tmpTime.month, &tmpTime.day,
		    &tmpTime.hour, &tmpTime.min, ext)){
	if (!strcmp(ext, _params.FileExtension) &&
	    (tmpTime.day == _params.InDay) && 
	    (tmpTime.month == _params.InMonth)){
	  ftime.year=1970;
	  ftime.month=1;
	  ftime.day=1;
	  ftime.hour = tmpTime.hour;
	  ftime.min = tmpTime.min;
	  ftime.sec = 0;
	  uconvert_to_utime(&ftime);
          return 0;
	}
      }
      break;
    }

    case Params::XMMDDHH_MM: {
      
      // Files are named prefixMMDDHH.MM
      // Separate the prefix from the rest of the filename
      
      size_t name_len=fname.size();
      size_t prefix_len=strlen(_params.FilePrefix);
      
      if (name_len > prefix_len) {
        string prefix_str=fname.substr(0, prefix_len);
        string after_prefix=fname.substr(prefix_len, name_len-prefix_len);
        
        if (4==sscanf(after_prefix.c_str(),"%2d%2d%2d.%2d",
                      &tmpTime.month, &tmpTime.day,
                      &tmpTime.hour, &tmpTime.min)) {
          if (!strcmp(prefix_str.c_str(), _params.FilePrefix) &&
              (tmpTime.day == _params.InDay) && 
              (tmpTime.month == _params.InMonth)) {
            ftime.year=1970;
            ftime.month=1;
            ftime.day=1;
            ftime.hour = tmpTime.hour;
            ftime.min = tmpTime.min;
            ftime.sec = 0;
            uconvert_to_utime(&ftime);
            return 0;
          }
        }
      }
      break;
    }

    case Params::NNNYYYYMMDDHHMM: {
      
      // Files are named nnnYYYYMMDDHHMM.ext

      int junk;
      
      if (7==sscanf(fname.c_str(),"%3d%4d%2d%2d%2d%2d.%s",
                    &junk,
                    &tmpTime.year, &tmpTime.month, &tmpTime.day,
                    &tmpTime.hour, &tmpTime.min, ext)){
        if (!strcmp(ext, _params.FileExtension) &&
            (tmpTime.day == _params.InDay) && 
            (tmpTime.month == _params.InMonth) &&
            (tmpTime.year == _params.InYear)){
          ftime.year=1970;
          ftime.month=1;
          ftime.day=1;
          ftime.hour = tmpTime.hour;
          ftime.min = tmpTime.min;
          ftime.sec = 0;
          uconvert_to_utime(&ftime);
          return 0;
        }
      }
      break;
    }

    case Params::YYYY_MM_DD_HHMM: {

      // Files are named YYYY-MM-DD_HHMM.ext
      // have to use the _ instead of - in the enum

      bool found=false;
      if ((_params.FilePreservePreExtension) &&
          (fname.size() > 15)) {
        vector<string> toks;
        _tokenize(fname, ".", toks);
        string lastTok=toks[toks.size()-1];
        if ((5==sscanf(fname.c_str(),"%4d-%2d-%2d_%2d%2d",
                       &tmpTime.year, &tmpTime.month, &tmpTime.day,
                       &tmpTime.hour, &tmpTime.min)) &&
            (1==sscanf(lastTok.c_str(), "%s", ext))) {
          found=true;
        }
      }
      else {
        if (6==sscanf(fname.c_str(),"%4d-%2d-%2d_%2d%2d.%s",
                      &tmpTime.year, &tmpTime.month, &tmpTime.day,
                      &tmpTime.hour, &tmpTime.min, ext)){
          found=true;
        }
      }
      if ((found) && 
          (!strcmp(ext, _params.FileExtension) &&
           (tmpTime.day == _params.InDay) && 
           (tmpTime.month == _params.InMonth) &&
           (tmpTime.year == _params.InYear))) {
        ftime.year=1970;
        ftime.month=1;
        ftime.day=1;
        ftime.hour = tmpTime.hour;
        ftime.min = tmpTime.min;
        ftime.sec = 0;
        uconvert_to_utime(&ftime);
        return 0;
      }
      break;
    }

    case Params::XYYYYMMDD_HHMMSS: {
      
      // Files are named prefixYYYYMMDD_HHMMSS.ext
      // Separate the prefix from the rest of the filename
      
      size_t name_len=fname.size();
      size_t prefix_len=strlen(_params.FilePrefix);
      
      if (name_len > prefix_len) {
        string prefix_str=fname.substr(0, prefix_len);
        string after_prefix=fname.substr(prefix_len, name_len-prefix_len);
        
        if (7==sscanf(after_prefix.c_str(),"%4d%2d%2d_%2d%2d%2d.%s",
                      &tmpTime.year, &tmpTime.month, &tmpTime.day,
                      &tmpTime.hour, &tmpTime.min, &tmpTime.sec, ext)) {
          if (!strcmp(prefix_str.c_str(), _params.FilePrefix) &&
              (tmpTime.month == _params.InMonth) &&
              (tmpTime.year == _params.InYear)){
            ftime.year=1970;
            ftime.month=1;
            ftime.day=1;
            ftime.hour = tmpTime.hour;
            ftime.min = tmpTime.min;
            ftime.sec = tmpTime.sec;
            uconvert_to_utime(&ftime);
            return 0;
          }
        }
      }
      break;
    }


    case Params::XYYYYMMDD_HHMMSSZ: {
      
      // Files are named prefixYYYYMMDD_HHMMSSZ.ext
      // Separate the prefix from the rest of the filename

      size_t name_len=fname.size();
      size_t prefix_len=strlen(_params.FilePrefix);
      
      if (name_len > prefix_len) {
        string prefix_str=fname.substr(0, prefix_len);
        string after_prefix=fname.substr(prefix_len, name_len-prefix_len);
        
        if (6==sscanf(after_prefix.c_str(),"%4d%2d%2d_%2d%2d%2d",
                      &tmpTime.year, &tmpTime.month, &tmpTime.day,
                      &tmpTime.hour, &tmpTime.min, &tmpTime.sec)) {
          if (!strcmp(prefix_str.c_str(), _params.FilePrefix) &&
              (tmpTime.month == _params.InMonth) &&
              (tmpTime.year == _params.InYear)){
            ftime.year=1970;
            ftime.month=1;
            ftime.day=1;
            ftime.hour = tmpTime.hour;
            ftime.min = tmpTime.min;
            ftime.sec = tmpTime.sec;
            uconvert_to_utime(&ftime);
            return 0;
          }
        }
      }
      break;
    }

    case Params::PRE_YYYYMMDDhhmmss_POST: {
      
      // Files are named <prefix>YYYYMMDDhhmmss<suffix>.ext
      // Separate the prefix from the rest of the filename

      size_t name_len = fname.size();
      size_t prefix_len = strlen(_params.FilePrefix);
      
      if (name_len > prefix_len) {
        string prefix_str = fname.substr(0, prefix_len);
        string after_prefix = fname.substr(prefix_len, name_len-prefix_len);
        
        if (7==sscanf(after_prefix.c_str(),"%4d%2d%2d%2d%2d%2d%s",
                      &tmpTime.year, &tmpTime.month, &tmpTime.day,
                      &tmpTime.hour, &tmpTime.min, &tmpTime.sec, ext)) {
          if (!strcmp(prefix_str.c_str(), _params.FilePrefix) &&
              (tmpTime.month == _params.InMonth) &&
              (tmpTime.year == _params.InYear)){

            ftime.year = 1970;
            ftime.month = 1;
            ftime.day = 1;
            ftime.hour = tmpTime.hour;
            ftime.min = tmpTime.min;
            ftime.sec = tmpTime.sec;
            uconvert_to_utime(&ftime);
            return 0;
          }
        }
      }
      break;
    }

    case Params::PRE_YYYYMMDDhh_POST: {
      
      // Files are named <prefix>YYYYMMDDhh<suffix>.ext
      // Separate the prefix from the rest of the filename

      size_t name_len = fname.size();
      size_t prefix_len = strlen(_params.FilePrefix);
      
      if (name_len > prefix_len) {
        string prefix_str = fname.substr(0, prefix_len);
        string after_prefix = fname.substr(prefix_len, name_len-prefix_len);
        
        if (5==sscanf(after_prefix.c_str(),"%4d%2d%2d%2d%s",
                      &tmpTime.year, &tmpTime.month, &tmpTime.day,
                      &tmpTime.hour, ext)) {
          if (!strcmp(prefix_str.c_str(), _params.FilePrefix) &&
              (tmpTime.month == _params.InMonth) &&
              (tmpTime.year == _params.InYear)){

            ftime.year = 1970;
            ftime.month = 1;
            ftime.day = 1;
            ftime.hour = tmpTime.hour;
            ftime.min = 0;
            ftime.sec = 0;
            uconvert_to_utime(&ftime);
            return 0;
          }
        }
      }
      break;
    }

    case Params::PRE_hh_YYYYMMDDhhmmss_POST: {
      
      // Files are named <prefix>hh-YYYYMMDDhhmmss<suffix>.ext
      // Separate the prefix from the rest of the filename

      size_t name_len = fname.size();
      size_t prefix_len = strlen(_params.FilePrefix);
      
      if (name_len > prefix_len) {
        string prefix_str = fname.substr(0, prefix_len);
        string after_prefix = fname.substr(prefix_len, name_len-prefix_len);
	int first_hour;

        if (8==sscanf(after_prefix.c_str(),"%2d-%4d%2d%2d%2d%2d%2d%s",
                      &first_hour, &tmpTime.year, &tmpTime.month, &tmpTime.day,
                      &tmpTime.hour, &tmpTime.min, &tmpTime.sec, ext)) {
          if (!strcmp(prefix_str.c_str(), _params.FilePrefix) &&
	      first_hour == tmpTime.hour &&
              (tmpTime.month == _params.InMonth) &&
              (tmpTime.year == _params.InYear)){

            ftime.year = 1970;
            ftime.month = 1;
            ftime.day = 1;
            ftime.hour = tmpTime.hour;
            ftime.min = tmpTime.min;
            ftime.sec = tmpTime.sec;
            uconvert_to_utime(&ftime);
            return 0;
          }
        }
      }
      break;
    }

    case Params::SDIR_PRE_YYYYMMDDHHMMSS: {
      
      // Files are named prefixYYYYMMDDHHMMSS.ext
      // Separate the prefix from the rest of the filename
      
      size_t name_len=fname.size();
      size_t prefix_len=strlen(_params.FilePrefix);
      
      if (name_len > prefix_len) {
        string prefix_str=fname.substr(0, prefix_len);
        string after_prefix=fname.substr(prefix_len, name_len-prefix_len);
        
        if (7==sscanf(after_prefix.c_str(),"%4d%2d%2d%2d%2d%2d.%s",
                      &tmpTime.year, &tmpTime.month, &tmpTime.day,
                      &tmpTime.hour, &tmpTime.min, &tmpTime.sec, ext)) {
          if (!strcmp(prefix_str.c_str(), _params.FilePrefix) &&
              (tmpTime.month == _params.InMonth) &&
              (tmpTime.year == _params.InYear)){
            ftime.year=1970;
            ftime.month=1;
            ftime.day=1;
            ftime.hour = tmpTime.hour;
            ftime.min = tmpTime.min;
            ftime.sec = tmpTime.sec;
            uconvert_to_utime(&ftime);
            return 0;
          }
        }
      }
      break;
    }


    case Params::SDIR_PRE_YYYYMMDD_HHMM_POST: {
      
      // Files are named prefixYYYYMMDD_HHMM...
      // Separate the prefix from the rest of the filename
      
      size_t name_len=fname.size();
      size_t prefix_len=strlen(_params.FilePrefix);
      
      if (name_len > prefix_len) {
        string prefix_str=fname.substr(0, prefix_len);
        string after_prefix=fname.substr(prefix_len, name_len-prefix_len);
        
        if (6==sscanf(after_prefix.c_str(),"%4d%2d%2d_%2d%2d%s",
                      &tmpTime.year, &tmpTime.month, &tmpTime.day,
                      &tmpTime.hour, &tmpTime.min, ext)) {
          if (!strcmp(prefix_str.c_str(), _params.FilePrefix) &&
              (tmpTime.month == _params.InMonth) &&
              (tmpTime.year == _params.InYear)){
            ftime.year=1970;
            ftime.month=1;
            ftime.day=1;
            ftime.hour = tmpTime.hour;
            ftime.min = tmpTime.min;
            ftime.sec = 0;
            uconvert_to_utime(&ftime);
            return 0;
          }
        }
      }
      break;
    }


    case Params::SDIR_PRE_YYYYMMDDhhmmss_POST: {
      
      // Files are named prefixYYYYMMDDhhmmss...
      // Separate the prefix from the rest of the filename
      
      size_t name_len=fname.size();
      size_t prefix_len=strlen(_params.FilePrefix);

      if (name_len > prefix_len) {
        string prefix_str=fname.substr(0, prefix_len);
        string after_prefix=fname.substr(prefix_len, name_len-prefix_len);
        
        if (7==sscanf(after_prefix.c_str(),"%4d%2d%2d%2d%2d%2d%s",
                      &tmpTime.year, &tmpTime.month, &tmpTime.day,
                      &tmpTime.hour, &tmpTime.min, &tmpTime.sec, ext)) {
          if (!strcmp(prefix_str.c_str(), _params.FilePrefix) &&
              (tmpTime.month == _params.InMonth) &&
              (tmpTime.year == _params.InYear)){
            ftime.year=1970;
            ftime.month=1;
            ftime.day=1;
            ftime.hour = tmpTime.hour;
            ftime.min = tmpTime.min;
            ftime.sec = tmpTime.sec;
            uconvert_to_utime(&ftime);
            return 0;
          }
        }
      }
      break;
    }


    case Params::RENAME_NO_TIME: {
      
      // Files are named <prefix>YYYYMMDDhhmmss.ext
      // Separate the prefix from the rest of the filename

      size_t name_len = fname.size();
      size_t prefix_len = strlen(_params.FilePrefix);
      
      if (name_len > prefix_len) {
        string prefix_str = fname.substr(0, prefix_len);
        string after_prefix = fname.substr(prefix_len, name_len-prefix_len);
        
        if (7==sscanf(after_prefix.c_str(),"%4d%2d%2d%2d%2d%2d%s",
                      &tmpTime.year, &tmpTime.month, &tmpTime.day,
                      &tmpTime.hour, &tmpTime.min, &tmpTime.sec, ext)) {
          if (!strcmp(prefix_str.c_str(), _params.FilePrefix) &&
              (tmpTime.month == _params.InMonth) &&
              (tmpTime.year == _params.InYear)){

            ftime.year = 1970;
            ftime.month = 1;
            ftime.day = 1;
            ftime.hour = tmpTime.hour;
            ftime.min = tmpTime.min;
            ftime.sec = tmpTime.sec;
            uconvert_to_utime(&ftime);
            return 0;
          }
        }
      }
      break;
    }

    case Params::MOD_TIME: {

      // Files times are obtained from mod time
      struct stat fileStat;
      if (ta_stat(filePath.c_str(), &fileStat) == 0) {
        ftime.unix_time = fileStat.st_mtime;
        uconvert_from_utime(&ftime);
        ftime.year=1970;
        ftime.month=1;
        ftime.day=1;
        ftime.hour = ftime.hour;
        ftime.min = ftime.min;
        ftime.sec = ftime.sec;
        uconvert_to_utime(&ftime);
        return 0;
      }
      break;
    }

  } // endswitch
  
  return -1;

}

/////////////////////////////////////////////////////////////
// return the index in the time list which is just before now

int file_repeat_day::_findStartIndex()

{

  date_time_t ref;
  ugmtime(&ref);
  ref.year = 1970;
  ref.month = 1;
  ref.day = 1;
  uconvert_to_utime(&ref);

  int startSameTime = 0;
  int istart = 0;
  for (size_t ii = 0; ii < _inputFiles.size(); ii++) {

    if (ii == 0) {
      startSameTime = 0;
    } else {
      if (_inputFiles[ii].time.unix_time != _inputFiles[startSameTime].time.unix_time) {
	startSameTime=ii;
      }
    }
	
    if (ref.unix_time > _inputFiles[ii].time.unix_time) {
      istart = ii;
    } else {
      break;
    }
  } //endfor

  if ((_params.MultipleFilesForOneTime) && (istart != startSameTime)) {
    istart = startSameTime;
  }
    
  if (_params.debug) {
    cerr << " -->> starting index: " << istart << endl;
  }

  return istart;

}

///////////////////////////////
// compute the next wakeup time

time_t file_repeat_day::_computeOutputTime(int itime)
  
{
  
  date_time_t output;
  ugmtime(&output);
  
  output.hour = _inputFiles[itime].time.hour;
  output.min = _inputFiles[itime].time.min;
  output.sec = _inputFiles[itime].time.sec;

  uconvert_to_utime(&output);
  
  if (_params.debug) {
    cerr << "Output time: " << utimstr(output.unix_time) << endl;
  }
  
  return output.unix_time;

}

///////////////////////////////
// compute the next wakeup time

time_t file_repeat_day::_computeWakeupTime(int itime)

{
  
  date_time_t wakeup;
  ugmtime(&wakeup);
  
  if (itime == 0) {
    wakeup.day += 1;
  }

  wakeup.hour = _inputFiles[itime].time.hour;
  wakeup.min = _inputFiles[itime].time.min;
  wakeup.sec = _inputFiles[itime].time.sec;
  
  uconvert_to_utime(&wakeup);

  if (_params.debug) {
    cerr << "Next wakeup: " << utimstr(wakeup.unix_time) << endl;
  }

  return wakeup.unix_time;

}

////////////////////////////////////////////////////
// compute the input and output paths
//
// Returns 0 on success, -1 on failure

int file_repeat_day::_computeOutputPath(int itime,
					const char *inputPath,
					time_t outputTime,
					char *outputPath)
  
{
  
  date_time_t outTime;
  outTime.unix_time = outputTime;
  uconvert_from_utime(&outTime);
  char outputDir[MAX_PATH_LEN];
  sprintf(outputDir, "%s", _params.OutDir);
  
  switch (_params.FilenameType){
    
  case Params::HHMMSS:
    sprintf(outputDir, "%s%s%.4d%.2d%.2d",
	    _params.OutDir, PATH_DELIM,
	    outTime.year, outTime.month, outTime.day);
    sprintf(outputPath, "%s%s%.2d%.2d%.2d.%s",
	    outputDir, PATH_DELIM,
            outTime.hour, outTime.min, outTime.sec,
	    _params.FileExtension);
    break;

  case Params::YYYYMMDDHHMM:
    sprintf(outputPath, "%s%s%.4d%.2d%.2d%.2d%.2d.%s",
	    _params.OutDir, PATH_DELIM,
	    outTime.year, outTime.month, outTime.day,
            outTime.hour, outTime.min,
	    _params.FileExtension);
    break;

  case Params::YYYYMMDDHHMMSS:
    sprintf(outputPath, "%s%s%s%.4d%.2d%.2d%.2d%.2d%.2d.%s",
	    _params.OutDir, PATH_DELIM,
	    _params.FilePrefix, 
	    outTime.year, outTime.month, outTime.day,
            outTime.hour, outTime.min, outTime.sec,
	    _params.FileExtension);
    break;
    
  case Params::MMDD_HHMM:
    sprintf(outputPath, "%s%s%.2d%.2d_%.2d%.2d.%s",
	    _params.OutDir, PATH_DELIM,
	    outTime.month, outTime.day,
            outTime.hour, outTime.min,
	    _params.FileExtension);
    break;

  case Params::XMMDDHH_MM:
    sprintf(outputPath, "%s%s%s%.2d%.2d%.2d.%.2d",
	    _params.OutDir, PATH_DELIM,
	    _params.FilePrefix, 
	    outTime.month, outTime.day,
            outTime.hour, outTime.min);
    break;

  case Params::NNNYYYYMMDDHHMM:
    {
      Path inPath(inputPath);
      string fname=inPath.getFile();
      string nnn=fname.substr(0,3);
      sprintf(outputPath, "%s%s%s%.4d%.2d%.2d%.2d%.2d.%s",
	      _params.OutDir, PATH_DELIM, nnn.c_str(),
	      outTime.year, outTime.month, outTime.day,
	      outTime.hour, outTime.min,
	      _params.FileExtension);
    }
    break;

  case Params::YYYY_MM_DD_HHMM:
    {
      if (_params.FilePreservePreExtension) {
	Path inPath(inputPath);
	string fname=inPath.getFile();
	vector <string>toks;
	_tokenize(fname, ".", toks);
	sprintf(outputPath, "%s%s%.4d-%.2d-%.2d_%.2d%.2d.%s.%s",
		_params.OutDir, PATH_DELIM,
		outTime.year, outTime.month, outTime.day,
		outTime.hour, outTime.min,
		toks[toks.size() - 2].c_str(),
		_params.FileExtension);
      }
      else {
	sprintf(outputPath, "%s%s%.4d-%.2d-%.2d_%.2d%.2d.%s",
		_params.OutDir, PATH_DELIM,
		outTime.year, outTime.month, outTime.day,
		outTime.hour, outTime.min,
		_params.FileExtension);
      }
    }
    break;

  case Params::XYYYYMMDD_HHMMSS:
    sprintf(outputPath, "%s%s%s%.4d%.2d%.2d_%.2d%.2d%.2d.%s",
	    _params.OutDir, PATH_DELIM,
	    _params.FilePrefix, 
	    outTime.year, outTime.month, outTime.day,
            outTime.hour, outTime.min, outTime.sec,
	    _params.FileExtension);
    break;

  case Params::XYYYYMMDD_HHMMSSZ:
    sprintf(outputPath, "%s%s%s%.4d%.2d%.2d_%.2d%.2d%.2d%s.%s",
	    _params.OutDir, PATH_DELIM,
	    _params.FilePrefix, 
	    outTime.year, outTime.month, outTime.day,
            outTime.hour, outTime.min, outTime.sec,
	    _params.FileSuffix, _params.FileExtension);
    break;


  case Params::PRE_YYYYMMDDhhmmss_POST:
    {
      Path inPath(inputPath);
      string fname = inPath.getFile();
      size_t prefix_len = strlen(_params.FilePrefix) + YYYYMMDDHHMMSS_LEN; 
      string suffix = fname.substr(prefix_len);

      sprintf(outputPath, "%s%s%s%.4d%.2d%.2d%.2d%.2d%.2d%s",
	      _params.OutDir, PATH_DELIM,
	      _params.FilePrefix, 
	      outTime.year, outTime.month, outTime.day,
	      outTime.hour, outTime.min, outTime.sec,
	      suffix.c_str());
    }
    break;

  case Params::PRE_YYYYMMDDhh_POST:
    {
      Path inPath(inputPath);
      string fname = inPath.getFile();
      size_t prefix_len = strlen(_params.FilePrefix) + YYYYMMDDHH_LEN; 
      string suffix = fname.substr(prefix_len);

      sprintf(outputPath, "%s%s%s%.4d%.2d%.2d%.2d%s",
	      _params.OutDir, PATH_DELIM,
	      _params.FilePrefix, 
	      outTime.year, outTime.month, outTime.day,
	      outTime.hour, suffix.c_str());
    }
    break;


  case Params::PRE_hh_YYYYMMDDhhmmss_POST:
    {
      Path inPath(inputPath);
      string fname = inPath.getFile();
      size_t prefix_len = strlen(_params.FilePrefix) + HH_YYYYMMDDHHMMSS_LEN; 
      string suffix = fname.substr(prefix_len);

      sprintf(outputPath, "%s%s%s%.2d-%.4d%.2d%.2d%.2d%.2d%.2d%s",
	      _params.OutDir, PATH_DELIM,
	      _params.FilePrefix, outTime.hour,
	      outTime.year, outTime.month, outTime.day,
	      outTime.hour, outTime.min, outTime.sec,
	      suffix.c_str());
    }
    break;


  case Params::SDIR_PRE_YYYYMMDDHHMMSS:
    sprintf(outputDir, "%s%s%.4d%.2d%.2d",
	    _params.OutDir, PATH_DELIM,
	    outTime.year, outTime.month, outTime.day);
    sprintf(outputPath, "%s%s%s%.4d%.2d%.2d%.2d%.2d%.2d.%s",
	    outputDir, PATH_DELIM,
	    _params.FilePrefix, 
	    outTime.year, outTime.month, outTime.day,
            outTime.hour, outTime.min, outTime.sec,
	    _params.FileExtension);
    break;

  case Params::SDIR_PRE_YYYYMMDD_HHMM_POST:
    {
      Path inPath(inputPath);
      string fname = inPath.getFile();
      size_t prefix_len = strlen(_params.FilePrefix) + YYYYMMDD_HHMM_LEN; 
      string suffix = fname.substr(prefix_len);

      sprintf(outputDir, "%s%s%.4d%.2d%.2d",
	      _params.OutDir, PATH_DELIM,
	      outTime.year, outTime.month, outTime.day);
      sprintf(outputPath, "%s%s%s%.4d%.2d%.2d_%.2d%.2d%s",
	      outputDir, PATH_DELIM,
	      _params.FilePrefix, 
	      outTime.year, outTime.month, outTime.day,
	      outTime.hour, outTime.min,
	      suffix.c_str());
    }
    break;

  case Params::SDIR_PRE_YYYYMMDDhhmmss_POST:
    {
      Path inPath(inputPath);
      string fname = inPath.getFile();
      size_t prefix_len = strlen(_params.FilePrefix) + YYYYMMDDHH_LEN; 
      string suffix = fname.substr(prefix_len);

      sprintf(outputDir, "%s%s%.4d%.2d%.2d",
	      _params.OutDir, PATH_DELIM,
	      outTime.year, outTime.month, outTime.day);
      sprintf(outputPath, "%s%s%s%.4d%.2d%.2d%.2d%.2d%.2d%s",
	      outputDir, PATH_DELIM,
	      _params.FilePrefix, 
	      outTime.year, outTime.month, outTime.day,
	      outTime.hour, outTime.min, outTime.sec,
	      suffix.c_str());
    }
    break;

  case Params::RENAME_NO_TIME:
    {
      Path inPath(inputPath);
      sprintf(outputPath, "%s%s%s",
	      _params.OutDir, PATH_DELIM, _params.OutFile);
    }
    break;

    
  case Params::MOD_TIME:
    {
      Path inPath(inputPath);
      sprintf(outputPath, "%s%s%s",
	      _params.OutDir, PATH_DELIM, inPath.getFile().c_str());
    }
    break;

  } // switch

  // make the output dir

  if (ta_makedir_recurse(outputDir)) {
    cerr << "ERROR - file_repeat_day::_computePaths" << endl;
    cerr << "  Cannot make dir: " << outputDir << endl;
    return -1;
  }

  return 0;
  
}

////////////////////////////////////////////////////
// copy from the input to the output
//
// Returns 0 on success, -1 on failure

int file_repeat_day::_copyFile(const char *inputPath,
			       const char *outputPath,
			       time_t outputTime)
  
{

  if((_params.FileType != Params::ASCII) && (_params.FileType != Params::NETCDF)) {
    cerr << "ERROR - file_repeat_day::_copyFile" << endl;
    cerr << "   Unknown file type." << endl;
    return -1;
  }
 
  int retVal = 0;

  if((_params.FileType == Params::NETCDF) && _params.OverwriteDay) {

    retVal =  _copyNetcdfFile(inputPath, outputPath, outputTime);

  }
  else {
	
    FILE *ifp;
    if ((ifp=fopen(inputPath, "rb")) == NULL) {
      int errNum = errno;
      cerr << "ERROR - file_repeat_day::_copyFile" << endl;
      cerr << "   Cannot open input file: " << inputPath << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
    
    FILE *ofp;
    if ((ofp=fopen(outputPath, "wb")) == NULL) {
      int errNum = errno;
      cerr << "ERROR - file_repeat_day::_copyFile" << endl;
      cerr << "   Cannot open output file: " << outputPath << endl;
      cerr << "  " << strerror(errNum) << endl;
      fclose(ifp);
      return -1;
    }

    if (_params.OverwriteDay) {
      
	retVal = _copyAsciiFile(ifp, ofp, outputTime);
    }
    else {

      retVal = _straightCopy(ifp, ofp, outputTime);


    }

    fclose(ifp);
    fclose(ofp);
  }

  return retVal;
}

  
////////////////////////////////////////////////////
// copy ASCII file from the input to the output
//
// Returns 0 on success, -1 on failure

int file_repeat_day::_copyAsciiFile(FILE *ifp,
				    FILE *ofp,
				    time_t outputTime)
  
{
  // copy while overwriting date/time fields as appropriate
  
  char line[4096];
  
  while (fgets(line, 4096, ifp) != NULL) {
    
    //
    // As odd as it may seem, instances of raw files with lines that
    // begin with '\0'. To get cstdio to write these lines the null
    // character has to be replaced.
    //
    if(line[0] == '\0') {
      line[0] = ' ';
    }
    
    _substituteTime(line, outputTime);
    _substituteStartExpire(line, outputTime);
    if (fputs(line, ofp) == EOF) {
      int errNum = errno;
      cerr << "ERROR - file_repeat_day::_copyFile" << endl;
      cerr << "   Cannot write to file" << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
  }
  return 0;
}

////////////////////////////////////////////////////
// straight copy from the input to the output
//
// Returns 0 on success, -1 on failure

int file_repeat_day::_straightCopy(FILE *ifp,
				   FILE *ofp,
				   time_t outputTime)
  
{
  int c;
  while ((c = fgetc(ifp)) != EOF) {
    if (fputc(c, ofp) == EOF) {
      int errNum = errno;
      cerr << "ERROR - file_repeat_day::_copyFile" << endl;
      cerr << "   Cannot write to file " << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
  } // while
  return 0;
}

////////////////////////////////////////////////////
// copy NetCDF file from the input to the output
//
// Returns 0 on success, -1 on failure

int file_repeat_day::_copyNetcdfFile(const char *inputPath,
				     const char *outputPath,
				     time_t outputTime)
  
{
  // build the command list for ncap2
  string command;
  _buildCmdList(outputTime, command);
  
  cerr << "ncap2 command -- " <<  command << endl;

  // build argument list
  vector< string > args;
  args.push_back(string("-s"));
  args.push_back(command);
  args.push_back(string(inputPath));
  args.push_back(string(outputPath));
  _callNcap2(args);
  
  return 0;
}

/////////////////////////////////
// substitute time fields in line
//
// We are looking for fields like:
//  161501 or 161501Z
// where 16 is the day, 15 the hour and 01 the min.

void file_repeat_day::_substituteTime(char *line,
				      time_t outputTime)

{

  if (strstr(line, "AMDAR") != NULL) {
    printf("In an AMDAR.\n");
  }

  // output day
  date_time_t outTime;
  outTime.unix_time = outputTime;
  uconvert_from_utime(&outTime);
  char outDayStr[4];
  sprintf(outDayStr, "%.2d", outTime.day);

  // set up current day string

  char inDayStr[4];
  sprintf(inDayStr, "%.2d", _params.InDay);

  // search for time fields in line

  char *current = line;
  int len = strlen(line);

  while (current < line + len) {

    // must start with day
    
    char *dateTime = strstr(current, inDayStr);
    if (dateTime == NULL) {
      return;
    }
    
    // need at least 7 chars
    if (((line + len) - dateTime) < 7) {
      return;
    }

    // white-space before?
    
    if (dateTime != line) {
      if (!isspace(dateTime[-1])) {
	current += 2;
	continue;
      }
    }
    
    // white-space or Z after or a '/'?
    
    if (!isspace(dateTime[6]) && (dateTime[6] != 'Z') && (dateTime[4] != '/')) {
      current += 2;
      continue;
    }

    // completed hack to handle valid and expire times in the form dd1hh1/dd2hh2
    // code copied from _substituteStartExpire
    if (dateTime[4] == '/') {
      int day1, hour1;
      int day2, hour2;

      current = dateTime;

      char outDay2Str[4];
      if (outTime.day < 31) {
	sprintf(outDay2Str, "%.2d", outTime.day + 1);
      } else {
	sprintf(outDay2Str, "%.2d", 1);
      }
      if (sscanf(current, "%2d%2d/%2d%2d",
		 &day1, &hour1, &day2, &hour2) == 4) {
	
	char *slash = strstr(current, "/");
	if (slash != NULL && strlen(slash) >= 5) {
	  
	  memcpy(slash - 4, outDayStr, 2);
	  
	  if (day2 == day1) {
	    memcpy(slash + 1, outDayStr, 2);
	  } else {
	    memcpy(slash + 1, outDay2Str, 2);
	  }
	}
	current += 9;
	continue;
      }
    }

    // break test into an hour and minute test
    if (!isdigit(dateTime[2]) ||
	!isdigit(dateTime[3])) {
      current += 2;
      continue;
    }

    // some bulletins express time in day and hour only. AMDAR is an example.
    // For AMDAR look for \r\r in minute position
    if ((!isdigit(dateTime[4])  && (dateTime[4] != '\r')) ||
	(!isdigit(dateTime[5]) && (dateTime[5] != '\r'))) {
      current += 2;
      continue;
    }

    // success

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Found date field: " << line;
    }

    // substitute day for today

    dateTime[0] = outDayStr[0];
    dateTime[1] = outDayStr[1];

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Substituted line: " << line;
    }

    current += 6;

  } // while

}

/////////////////////////////////////////
// substitute start/expire fields in line
//
// We are looking for fields like:
//  161500/183000 or 1615/1830

void file_repeat_day::_substituteStartExpire(char *line,
					     time_t outputTime)
  
{

  if (strstr(line, "SIGMET") == NULL &&
        strstr(line, "AIRMET") == NULL) {
      return;
  }

  // output day

  date_time_t outTime;
  outTime.unix_time = outputTime;
  uconvert_from_utime(&outTime);
  char day1Str[4];
  char day2Str[4];
  sprintf(day1Str, "%.2d", outTime.day);
  if (outTime.day < 31) {
    sprintf(day2Str, "%.2d", outTime.day + 1);
  } else {
    sprintf(day2Str, "%.2d", 1);
  }
  
  // set up current day string

  char inDayStr[4];
  sprintf(inDayStr, "%.2d", _params.InDay);
  
  // search for start/expirt time fields in line

  char *current = line;
  int len = strlen(line);

  while (current < line + len) {
    
    int day1, hour1, min1;
    int day2, hour2, min2;

    if (sscanf(current, "%2d%2d%2d/%2d%2d%2d",
	       &day1, &hour1, &min1,
	       &day2, &hour2, &min2) == 6) {

      char *slash = strstr(current, "/");
      if (slash != NULL && strlen(slash) >= 7) {
	memcpy(slash - 6, day1Str, 2);
	if (day1 == day2) {
	  memcpy(slash + 1, day1Str, 2);
	} else {
	  memcpy(slash + 1, day2Str, 2);
	}
      }
      current += 12;
    }
    else if (sscanf(current, "%2d%2d/%2d%2d",
	       &day1, &hour1, &day2, &hour2) == 4) {

      char *slash = strstr(current, "/");
      if (slash != NULL && strlen(slash) >= 5) {

	memcpy(slash - 4, day1Str, 2);

	if (day2 == day1) {
	  memcpy(slash + 1, day1Str, 2);
	} else {
	  memcpy(slash + 1, day2Str, 2);
	}
      }
      current += 8;
    }

    current++;
    
  } // while
}

//////////////////////////////////////////////
// tokenize a string into a vector of strings

void file_repeat_day::_tokenize(const string &str,
				const string &spacer,
				vector<string> &toks)
  
{
  toks.clear();
  size_t pos = 0;
  bool done = false;
  while (!done) {
    size_t start = str.find_first_not_of(spacer, pos);
    size_t end = str.find_first_of(spacer, start);
    if (start == string::npos) {
      done = true;
    } else {
      string tok;
      tok.assign(str, start, end - start);
      toks.push_back(tok);
    }
    pos = end;
  }
}

//////////////////////////////////////////////
// tokenize a string into a vector of strings

void file_repeat_day::_callNcap2(const vector<string> &args)  
{
  string pmuStr = "Starting ncap2: " + string(_params.ncap2Path);
  PMU_force_register(pmuStr.c_str());

  if (_params.debug != Params::DEBUG_OFF) {
    cerr << pmuStr << endl;
  }
  
  // Fork a child to run the script
  
  time_t start_time = time(NULL);
  time_t terminate_time = start_time + 30;
  pid_t childPid = fork();
  
  if (childPid == 0) {
    
    // this is the child process, so exec the script
    
    _execScript(args, _params.ncap2Path);
    
    // exit
    
    if (_params.debug != Params::DEBUG_OFF) {
      cerr << "Child process exiting ..." << endl;
    }

    _exit(0);

  }

  // this is the parent

  if (_params.debug != Params::DEBUG_OFF) {
    cerr << endl;
    cerr << "Ncap2 started, child pid: " << childPid << endl;
  }
  
    
  // script in the foreground - so we must wait for it
  // to complete
    
  while (true) {
      
    int status;
    if (waitpid(childPid, &status,
		(int) (WNOHANG | WUNTRACED)) == childPid) {
      // child exited
      time_t end_time = time(NULL);
      int runtime = (int) (end_time - start_time);
      pmuStr = _params.ncap2Path + string(" took ") + to_string(runtime) + string("secs");
      PMU_force_register(pmuStr.c_str());
      if (_params.debug != Params::DEBUG_OFF) {
	cerr << "Child exited, pid: " << childPid << endl;
	cerr << "  Runtime in secs: " << runtime << endl;
      }
      return;
    }
      
    // script is still running
      
    pmuStr = _params.ncap2Path + string(" running");
    PMU_auto_register(pmuStr.c_str());
      
    // child still running - kill it as required

    _killAsRequired(childPid, terminate_time);

    // sleep for a bit
      
    umsleep(50);
      
  } // while
    
}

//////////////////////
// execute the script

void file_repeat_day::_execScript(const vector<string> &args,
				  const char *script_to_call)  
{

  // set up execvp args - this is a null-terminated array of strings
  
  int narray = (int) args.size() + 2;
  TaArray<const char *> argArray_;
  const char **argArray = argArray_.alloc(narray);
  argArray[0] = script_to_call;
  for (int ii = 0; ii < (int) args.size(); ii++) {
    argArray[ii+1] = args[ii].c_str();
  }
  argArray[narray-1] = NULL;
  
  if (_params.debug != Params::DEBUG_OFF) {
    cerr << "Calling execvp with following args:" << endl;
    for (int i = 0; i < narray; i++) {
      cerr << "  " << argArray[i] << endl;
    }
  }
    
  // execute the command
  
  execvp(argArray[0], (char **) argArray);
  
}

//////////////////////////
// kill child as required
//

void file_repeat_day::_killAsRequired(pid_t pid,
				      time_t terminate_time)
{
  
  time_t now = time(NULL);

  if (now < terminate_time) {
    return;
  }

  // Time to terminate script, will be reaped elsewhere
  
  if (_params.debug != Params::DEBUG_OFF) {
    cerr << "Child has run too long, pid: " << pid << endl;
    cerr << "  Sending child kill signal" << endl;
  }
  
  char pmuStr[4096];
  sprintf(pmuStr, "Killing pid %d", pid);
  PMU_force_register(pmuStr);
  
  if(kill(pid,SIGKILL)) {
    perror("kill: ");
  }

}

int file_repeat_day::_buildCmdList(time_t output_time,
				   string& cmd_list)
{
  date_time_t outTime;
  outTime.unix_time = output_time;
  uconvert_from_utime(&outTime);
  char ncValue[128];

  // example of running ncap2 in this instance is
  //    ncap2 -s "base_date=20170925;base_time=210000;" in.nc out.nc
  cmd_list = "\"";
  
  for(int i = 0; i < _params.NetVars_n;  i++) {

    switch(_params._NetVars[i].format) {

    case Params::NC_HHmmss:
      sprintf(ncValue, "%s=%.2d%.2d%.2d;", _params._NetVars[i].name, outTime.hour,
	      outTime.min, outTime.sec);
      break;
      
    case Params::NC_HHmm:
      sprintf(ncValue, "%s=%.2d%.2d;", _params._NetVars[i].name,outTime.hour,
	      outTime.min);
      break;
      
    case Params::NC_HH:
      sprintf(ncValue, "%s=%.2d;", _params._NetVars[i].name, outTime.hour);
      break;
      
    case Params::NC_mm:
      sprintf(ncValue, "%s=%.2d;", _params._NetVars[i].name, outTime.min);
      break;
      
    case Params::NC_ss:
      sprintf(ncValue, "%s=%.2d;", _params._NetVars[i].name, outTime.sec);
      break;
      
    case Params::NC_YYYYMMDD:
      sprintf(ncValue, "%s=%.4d%.2d%.2d;", _params._NetVars[i].name, outTime.year,
	      outTime.month, outTime.day);
      break;
      
    case Params::NC_YYYYMMDDhh:
      sprintf(ncValue, "%s=%.4d%.2d%.2d%.2d;", _params._NetVars[i].name, outTime.year,
	      outTime.month, outTime.day, outTime.hour);
      break;
      
    case Params::NC_YYYYMMDDhhmm:
      sprintf(ncValue, "%s=%.4d%.2d%.2d%.2d%.2d;", _params._NetVars[i].name, outTime.year,
	      outTime.month, outTime.day, outTime.hour, outTime.min);
      break;
      
    case Params::NC_YYYYMMDDHHmmss:
      sprintf(ncValue, "%s=%.4d%.2d%.2d%.2d%.2d%.2d;", _params._NetVars[i].name,
	      outTime.year, outTime.month, outTime.day, outTime.hour, outTime.min,
	      outTime.sec);
      break;
      
    case Params::NC_YYYY:
      sprintf(ncValue, "%s=%.4d;", _params._NetVars[i].name, outTime.year);
      break;
      
    case Params::NC_MM:
      sprintf(ncValue, "%s=%.2d;", _params._NetVars[i].name, outTime.month);
      break;
      
    case Params::NC_DD:
      sprintf(ncValue, "%s=%.2d;", _params._NetVars[i].name, outTime.day);
      break;
      
    case Params::NC_UTIME:
      sprintf(ncValue, "%s=%ld;", _params._NetVars[i].name, outTime.unix_time);
      break;
      
    default:
      cerr << "ERROR - file_repeat_day::_copyNetcdfFile" << endl;
      cerr << " Unknown TimeDateFormat_t in  NetVars" << endl;      
      return -1;
    }

    cmd_list += string(ncValue);
  }
  
  cmd_list += "\"";

    return 0;
}
