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
//////////////////////////////////////////////////////////
// RadxTimelist.cc
//
// Timelist routines for Radx class
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2005
//
//////////////////////////////////////////////////////////

#include <Radx/RadxTimeList.hh>
#include <Radx/RadxStr.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxReadDir.hh>
#include <iomanip>
#include <iterator>
#include <algorithm>
#include <sys/stat.h>
#include <cerrno>
#include <cmath>
#include <cstdio>
using namespace std;

/////////////////////////////////////////////////////////////////
// constructor

RadxTimeList::RadxTimeList()

{
  clearMode();
  clearData();
  _readAggregateSweeps = false;
  _checkLatestValidModTime = false;
}

/////////////////////////////////////////////////////////////////
// destructor

RadxTimeList::~RadxTimeList()

{
  
}

/////////////////////////
// clear the error string

void RadxTimeList::clearErrStr() const
{
  _errStr = "";
}

/////////////////////////////////////////////////////////////////
// setModeInterval
//
// Set the time list so that it finds all of the interval data
// times between the start and end times.
// For forecast data where multiple forecasts exist for the same
// interval time, a single interval time will be returned.

void RadxTimeList::setModeInterval(RadxTime start_time,
                                   RadxTime end_time)
  
{
  
  clearMode();
  _mode = MODE_INTERVAL;
  _startTime = start_time;
  _endTime = end_time;

}

/////////////////////////////////////////////////////////////////
// setModeFirst
//
// set the time list mode so that it finds the first data time

void RadxTimeList::setModeFirst()

{

  clearMode();
  _mode = MODE_FIRST;

}

/////////////////////////////////////////////////////////////////
// setModeLast
//
// set the time list mode so that it finds the last data time

void RadxTimeList::setModeLast()

{

  clearMode();
  _mode = MODE_LAST;

}

/////////////////////////////////////////////////////////////////
// SetModeClosest
//
// set the time list mode so that it finds the closest available data time
// to the search time within the search margin

void RadxTimeList::setModeClosest(RadxTime search_time, double time_margin)
  
{

  clearMode();
  _mode = MODE_CLOSEST;
  _searchTime = search_time;
  _timeMargin = time_margin;

}

/////////////////////////////////////////////////////////////////
// SetModeFirstBefore
//
// set the time list mode so that it finds the first available data time
// before the search time within the search margin

void RadxTimeList::setModeFirstBefore(RadxTime search_time, double time_margin)

{

  clearMode();
  _mode = MODE_FIRST_BEFORE;
  _searchTime = search_time;
  _timeMargin = time_margin;

}

/////////////////////////////////////////////////////////////////
// SetModeFirstAfter
//
// set the time list mode so that it finds the first available data time
// after the search time within the search margin

void RadxTimeList::setModeFirstAfter(RadxTime search_time,
				     double time_margin)
  
{

  clearMode();
  _mode = MODE_FIRST_AFTER;
  _searchTime = search_time;
  _timeMargin = time_margin;

}

/////////////////////////////////////////////////////////////////
// clearMode
//
// clear out time list mode info

void RadxTimeList::clearMode()

{

  _mode = MODE_UNDEFINED;
  _startTime.clear();
  _endTime.clear();
  _searchTime.clear();
  _timeMargin = 0.0;

}

/////////////////////////////////////////////////////////////////
// clear all data

void RadxTimeList::clearData()

{
  _fileStartTimes.clear();
  _pathList.clear();
}

/////////////////////////////////////////////////////////////////
// Set aggregation into volume on read.
// If true, individual sweep files will be aggregated into a
// volume when the files are read
  
void RadxTimeList::setReadAggregateSweeps(bool val)

{
  _readAggregateSweeps = val;
}

//////////////////////////////////////////////////////////
// compile time list
//
// Compile list for the specified mode.
//
// You must call one of the mode set functions above before calling
// this function.
//
// Returns 0 on success, -1 on failure
// getErrStr() retrieves the error string.
  
int RadxTimeList::compile()

{

  clearData();
  clearErrStr();
  
  if (_mode == MODE_UNDEFINED) {
    _errStr += "ERROR - RadxTimeList::compile\n";
    _errStr += "  Undefined mode.\n";
    _errStr += "  You must set the mode before calling this routine.\n";
    return -1;
  }

  // set top dir
  
  string topDir = _dir;

  // check that the directory exists

  struct stat dirStat;
  if (stat(topDir.c_str(), &dirStat)) {
    int errNum = errno;
    _errStr += "ERROR - RadxTimeList::compileTimeList\n";
    _errStr += "  Cannot stat requested directory.\n";
    RadxStr::addStr(_errStr, topDir.c_str());
    RadxStr::addStr(_errStr, strerror(errNum));
    return -1;
  }

  // compile depending on mode

  switch (_mode) {
    case MODE_INTERVAL:
      _compileInterval(topDir);
      break;
    case MODE_FIRST:
      _compileFirst(topDir);
      break;
    case MODE_LAST:
      _compileLast(topDir);
      break;
    case MODE_CLOSEST:
      _compileClosest(topDir);
      break;
    case MODE_FIRST_BEFORE:
      _compileFirstBefore(topDir);
      break;
    case MODE_FIRST_AFTER:
      _compileFirstAfter(topDir);
      break;
    case MODE_UNDEFINED:
      break;
  } // switch

  return 0;

}

//////////////////////////
// print time list request

void RadxTimeList::printRequest(ostream &out) const

{

  out << getRequestString() << endl;

}

//////////////////////////
// get request string

string RadxTimeList::getRequestString() const

{

  string req;
  char text[1024];

  req += "Radx time list request\n";
  req += "----------------------\n";
  
  if (_mode == MODE_INTERVAL) {
    req += "  Mode: times in interval\n";
  } else if (_mode == MODE_FIRST) {
    req += "  Mode: first time\n";
  } else if (_mode == MODE_LAST) {
    req += "  Mode: last time\n";
  } else if (_mode == MODE_CLOSEST) {
    req += "  Mode: closest time\n";
  } else if (_mode == MODE_FIRST_BEFORE) {
    req += "  Mode: first_before time\n";
  } else if (_mode == MODE_FIRST_AFTER) {
    req += "  Mode: first_after time\n";
  }

  if (_dir.size() > 0) {
    req += (string("  dir: ") + _dir + "\n");
  }

  if (_mode == MODE_INTERVAL) {
    req += "  start time: " + _startTime.asString() + "\n";
    req += "  end time: " + _endTime.asString() + "\n";
  } else if (_mode == MODE_CLOSEST ||
             _mode == MODE_FIRST_BEFORE ||
             _mode == MODE_FIRST_AFTER) {
    req += "  search time: " + _searchTime.asString() + "\n";
    sprintf(text, "    time margin(secs): %lg\n", _timeMargin);
    req += text;
  }

  req += "  readAggregateSweeps: " +
    string((_readAggregateSweeps?"Y":"N")) + "\n";
  req += "  checkLatestValidModTime: " +
    string((_checkLatestValidModTime?"Y":"N")) + "\n";

  return req;

}

///////////////////////////////////////////////////
// compile interval

void RadxTimeList::_compileInterval(const string &topDir)

{

  // search for interval times

  TimePathSet timePaths;
  _searchForValid(topDir, _startTime, _endTime, timePaths);

  // fill time lists

  TimePathSet::iterator ii;
  for (ii = timePaths.begin(); ii != timePaths.end(); ii++) {
    _fileStartTimes.push_back(ii->fileStartTime);
    _pathList.push_back(ii->path);
  } // ii
  
}

///////////////////////////////////////////////////
// compile first

void RadxTimeList::_compileFirst(const string &topDir)

{

  // get the first time

  TimePathSet timePaths;
  _addFirst(topDir, timePaths);

  if (timePaths.size() > 0) {
    // interval time found, use the first one
    TimePathSet::iterator ii = timePaths.begin();
    _fileStartTimes.push_back(ii->fileStartTime);
    _pathList.push_back(ii->path);
  }

}

///////////////////////////////////////////////////
// compile last

void RadxTimeList::_compileLast(const string &topDir)

{

  // get the last time

  TimePathSet timePaths;
  _addLast(topDir, timePaths);
  
  if (timePaths.size() > 0) {
    // interval time found, use the last one
    TimePathSet::reverse_iterator ii = timePaths.rend();
    _fileStartTimes.push_back(ii->fileStartTime);
    _pathList.push_back(ii->path);
  }

}

///////////////////////////////////////////////////
// compile closest

void RadxTimeList::_compileClosest(const string &topDir)

{

  // use best forecast search, centered on the search time

  TimePathSet timePaths;
  RadxTime startTime = _searchTime - _timeMargin;
  RadxTime endTime = _searchTime + _timeMargin;
  
  _addClosest(topDir, _searchTime, startTime, endTime, timePaths);

  // fill time lists

  if (timePaths.size() > 0) {
    TimePathSet::iterator best = timePaths.begin();
    _fileStartTimes.push_back(best->fileStartTime);
    _pathList.push_back(best->path);
  }

}

///////////////////////////////////////////////////
// compile first before

void RadxTimeList::_compileFirstBefore(const string &topDir)

{

  // use best forecast search, ending at the search time

  TimePathSet timePaths;
  RadxTime startTime = _searchTime - _timeMargin;
  RadxTime endTime = _searchTime;
  
  _addClosest(topDir, _searchTime, startTime, endTime, timePaths);
    
  // fill time lists

  if (timePaths.size() > 0) {
    TimePathSet::iterator best = timePaths.begin();
    _fileStartTimes.push_back(best->fileStartTime);
    _pathList.push_back(best->path);
  }

}

///////////////////////////////////////////////////
// compile first after

void RadxTimeList::_compileFirstAfter(const string &topDir)

{

  // use best forecast search, starting at the search time
  
  TimePathSet timePaths;
  RadxTime startTime = _searchTime;
  RadxTime endTime = _searchTime + _timeMargin;
  
  _addClosest(topDir, _searchTime, startTime, endTime, timePaths);
    
  // fill time lists

  if (timePaths.size() > 0) {
    TimePathSet::iterator best = timePaths.begin();
    _fileStartTimes.push_back(best->fileStartTime);
    _pathList.push_back(best->path);
  }

}

///////////////////////////////////////////////////
// Search for interval times

void RadxTimeList::_searchForValid(const string &topDir,
                                   RadxTime startTime,
                                   RadxTime endTime,
                                   TimePathSet &timePaths)
  
{

  // search through days in time range
  
  int startDay = startTime.utime() / RadxTime::RADX_SECS_IN_DAY;
  if (startTime.utime() < 0) {
    startDay -= 1;
  }
  int endDay = endTime.utime() / RadxTime::RADX_SECS_IN_DAY;
  if (endTime.utime() < 0) {
    endDay -= 1;
  }
  
  TimePathSet all;

  _searchDayRange(topDir, startDay, endDay,
                  startTime, endTime, all);

  if (_fileStartTimes.size() == 0) {
    _searchTopDir(topDir, startDay, endDay,
                  startTime, endTime, all);
  }

  if (all.size() < 1) {
    return;
  }

  // compute file end times from start time of next file

  vector<RadxTime> fileEndTimes;
  RadxTime fStartTime;
  RadxTime fEndTime;
  RadxTime prevStart;
  RadxTime prevEnd;
  double timeSpan = 1.0;
  bool firstIter = true;
  for (TimePathSet::iterator ii = all.begin(); ii != all.end(); ++ii) {
    fStartTime = ii->fileStartTime;
    fEndTime =   ii->fileEndTime;
    if (!firstIter) {
      if (prevStart != prevEnd) {
	fileEndTimes.push_back(prevEnd);
      } else {
	fileEndTimes.push_back(fStartTime);
      }
      timeSpan = fStartTime - prevStart;
    }
    
    prevStart = fStartTime;
    prevEnd = fEndTime;

    if (firstIter) {
      firstIter = false;
    }
  }
  
  if (!all.empty()) {
    // catch last time
    if (prevStart != prevEnd) {
      fileEndTimes.push_back(prevEnd);
    } else {
      fileEndTimes.push_back(prevStart + timeSpan);
    }
  }

  // check file times overlap desired range

  TimePathSet::iterator mm = all.begin();
  size_t count = 0;
  for (; mm != all.end(); mm++, count++) {
    TimePath tpath = *mm;
    tpath.fileEndTime = fileEndTimes[count];
    if (tpath.fileStartTime <= endTime && tpath.fileEndTime >= startTime) {
      timePaths.insert(timePaths.end(), tpath);
    }
  } // mm

  // for sweep files, provide times for end of unique volumes

  if (_readAggregateSweeps) {
    _makeSweepVolumesUnique(timePaths);
  }
  
}

///////////////////////////////////////////////////
// Search through the directories in the time range

void RadxTimeList::_searchDayRange(const string &dir,
                                   int startDay,
                                   int endDay,
                                   RadxTime startTime,
                                   RadxTime endTime,
                                   TimePathSet &timePaths)
  
{
  
  // loop through day directories looking for suitable files
  
  for (int iday = startDay; iday <= endDay; iday++) {

    RadxTime midday(iday * RadxTime::RADX_SECS_IN_DAY +
		    RadxTime::RADX_SECS_IN_DAY / 2);
    char dayDir[RadxPath::RADX_MAX_PATH_LEN];

    // normal format

    sprintf(dayDir, "%s%s%.4d%.2d%.2d",
	    dir.c_str(), RadxPath::RADX_PATH_DELIM,
	    midday.getYear(), midday.getMonth(), midday.getDay());
    _searchDay(dayDir, midday, startTime, endTime, timePaths);

    // extended format
    
    sprintf(dayDir, "%s%s%.4d%s%.4d%.2d%.2d",
	    dir.c_str(), RadxPath::RADX_PATH_DELIM, 
            midday.getYear(), RadxPath::RADX_PATH_DELIM, 
	    midday.getYear(), midday.getMonth(), midday.getDay());
    _searchDay(dayDir, midday, startTime, endTime, timePaths);

  } // iday

}

///////////////////////////////////////////////////
// Search the top dir for interval files - i.e. in the 
// case where there is no day sub dir

void RadxTimeList::_searchTopDir(const string &dir,
                                 int startDay,
                                 int endDay,
                                 RadxTime startTime,
                                 RadxTime endTime,
                                 TimePathSet &timePaths)
  
{
  
  // loop through day directories looking for suitable files
  
  for (int iday = startDay; iday <= endDay; iday++) {
    
    RadxTime midday(iday * RadxTime::RADX_SECS_IN_DAY +
		    RadxTime::RADX_SECS_IN_DAY / 2);
    
    // normal format
    
    _searchDay(dir, midday, startTime, endTime, timePaths);

    // extended format
    
    _searchDay(dir, midday, startTime, endTime, timePaths);

  } // iday

}

///////////////////////////////////////////////////
// Search a day directory for interval files

void RadxTimeList::_searchDay(const string &dayDir,
                              const RadxTime &midday,
                              RadxTime startTime,
                              RadxTime endTime,
                              TimePathSet &timePaths)
  
{

  RadxReadDir rdir;
  if (rdir.open(dayDir.c_str()) == 0) {
    
    // Loop thru directory looking for the data file names
    // or forecast directories
    
    struct dirent *dp;
    for (dp = rdir.read(); dp != NULL; dp = rdir.read()) {
      
      // exclude dir entries beginning with '.'
      
      if (dp->d_name[0] == '.') {
	continue;
      }
      
      _addValid(dayDir, midday, dp->d_name, startTime, endTime, timePaths);
      
    } // dp
    
    rdir.close();

  } // if (rdir ...
  
}

///////////////////////////////////////////////////
// add valid time

void RadxTimeList::_addValid(const string &dayDir,
                             const RadxTime &midday,
                             const string &entryName,
                             RadxTime startTime,
                             RadxTime endTime,
                             TimePathSet &timePaths)
  
{

  // exclude entry names which are too short
  
  if (entryName.size() < 6) {
    return;
  }
  
  // find first digit in entry name - if no digits, return now

  const char *start = NULL;
  for (size_t ii = 0; ii < entryName.size(); ii++) {
    if (isdigit(entryName[ii])) {
      start = entryName.c_str() + ii;
      break;
    }
  }
  if (!start) return;
  const char *end = start + strlen(start);

  // get time

  RadxTime fileStartTime;
  RadxTime fileEndTime;
  int year, month, day, hour, min, sec, msec;
  int eyear, emonth, eday, ehour, emin, esec, emsec;
  char cc, ecc;
  while (start < end - 6) {
    if (strncmp(entryName.c_str(), "swp.", 4) == 0) {
      // dorade sweep file
      RadxTime doradeTime;
      if (getDoradeTime(entryName, doradeTime)) {
        return;
      }
      fileStartTime = doradeTime;
      fileEndTime = doradeTime;
      break;
    } else if (sscanf(start, "%4d%2d%2d%1c%2d%2d%2d.%3d_to_%4d%2d%2d%1c%2d%2d%2d.%3d",
                      &year, &month, &day, &cc, &hour, &min, &sec, &msec,
                      &eyear, &emonth, &eday, &ecc, &ehour, &emin, &esec, &emsec) == 16) {
      // start/end format - yyyymmdd_hhmmss.mmm_to_yyyymmdd_hhmmss.mmm
      if (year < 1900 || month < 1 || month > 12 || day < 1 || day > 31) {
        return;
      }
      if (hour < 0 || hour > 23 || min < 0 || min > 59 || sec < 0 || sec > 59) {
        return;
      }
      if (eyear < 1900 || emonth < 1 || emonth > 12 || eday < 1 || eday > 31) {
        return;
      }
      if (ehour < 0 || ehour > 23 || emin < 0 || emin > 59 || esec < 0 || esec > 59) {
        return;
      }
      if (msec > 999) {
        msec = 0;
      }
      if (emsec > 999) {
        emsec = 0;
      }
      RadxTime stime(year, month, day, hour, min, sec, msec / 1000.0);
      RadxTime etime(eyear, emonth, eday, ehour, emin, esec, emsec / 1000.0);
      fileStartTime = stime;
      fileEndTime = etime;
      break;
    } else if (sscanf(start, "%4d%2d%2d%1c%2d%2d%2d",
                      &year, &month, &day, &cc, &hour, &min, &sec) == 7) {
      // extended format - yyyymmdd_hhmmss
      if (year < 1900 || month < 1 || month > 12 || day < 1 || day > 31) {
        return;
      }
      if (hour < 0 || hour > 23 || min < 0 || min > 59 || sec < 0 || sec > 59) {
        return;
      }
      RadxTime etime(year, month, day, hour, min, sec);
      fileStartTime = etime;
      fileEndTime = etime;
      break;
    } else if (sscanf(start, "%4d%2d%2d%1c%2d%2d",
                      &year, &month, &day, &cc, &hour, &min) == 6) {
      // extended format - yyyymmdd_hhmmss
      if (year < 1900 || month < 1 || month > 12 || day < 1 || day > 31) {
        return;
      }
      if (hour < 0 || hour > 23 || min < 0 || min > 59) {
        return;
      }
      RadxTime etime(year, month, day, hour, min, 0);
      fileStartTime = etime;
      fileEndTime = etime;
      break;
    } else if (sscanf(start, "%2d%2d%2d", &hour, &min, &sec) == 3) {
      // normal format - yyyymmdd/hhmmss
      if (hour < 0 || hour > 23 || min < 0 || min > 59 || sec < 0 || sec > 59) {
        return;
      }
      RadxTime etime(midday);
      etime.setTime(hour, min, sec);
      fileStartTime = etime;
      fileEndTime = etime;
      break;
    }
    start++;
  }

  if (fileStartTime.utime() == 0) {
    return;
  }
	
  // check that the file is a valid candidate
  
  RadxPath fpath(dayDir, entryName);

  if (!_isValidFile(fpath.getPath())) {
    return;
  }
  
  // insert the file
  
  string pathStr(fpath.getPath());
  TimePath tpath(fileStartTime, fileEndTime, pathStr);
  timePaths.insert(timePaths.end(), tpath);

}


/////////////////////////////////////
// getthe first and last date, time of data files
// 

void RadxTimeList::getFirstAndLastTime(RadxTime &fileStartTime, RadxTime &fileEndTime) {
  int ntimes = _fileStartTimes.size();
  if (ntimes >= 2) {
    fileStartTime = _fileStartTimes.at(0);
    fileEndTime = _fileStartTimes.at(ntimes-1);
  }  else {
    // send a warning, but fail safely by sending
    // some start and end time
    fileStartTime = RadxTime::ZERO;
    fileEndTime = RadxTime::NOW;
    _errStr += "WARNING - RadxTimeList::getFirstAndLastTime\n";
    _errStr += "  less than two data files found\n";
  }
}


/////////////////////////////////////
// add first file to set

void RadxTimeList::_addFirst(const string &topDir,
			     TimePathSet &timePaths)
  
{
  
  // get day dirs in time order
  
  TimePathSet dayDirs;
  _getDayDirs(topDir, dayDirs);
  
  // search for interval times in each day
  // we stop as soon as we find a interval time
  
  TimePathSet::iterator ii;
  for (ii = dayDirs.begin(); ii != dayDirs.end(); ii++) {
    
    RadxTime midday(ii->fileStartTime);
    const string &dayDir = ii->path;

    // look for non-forecast files
    
    TimePathSet tmpSet;
    _searchDay(dayDir, midday, 0, 0, tmpSet);
    
    if (tmpSet.size() > 0) {
      // interval times found, insert the first one into the set
      // and return
      timePaths.insert(timePaths.end(), *(tmpSet.begin()));
      return;
    }
    
  } // ii

}

/////////////////////////////////////
// add last file to set

void RadxTimeList::_addLast(const string &topDir,
			    TimePathSet &timePaths)
  
{

#ifdef NOT_YET
  // try _latest_data_info file
  
  LdataInfo ldata(topDir);
  if (ldata.read(-1) == 0) {
    RadxPath lpath(topDir, ldata.getRelDataPath());
    // check that this file is less that 1 day old
    struct stat fstat;
    if (ta_stat(lpath.getPath().c_str(), &fstat) == 0) {
      RadxTime now = time(NULL);
      int age = now - fstat.st_mtime;
      if (age < RadxTime::SECS_IN_DAY) {
	TimePath tpath(ldata.getLatestTime() + ldata.getLeadTime(),
		       ldata.getLatestTime(),
		       lpath.getPath());
	timePaths.insert(timePaths.end(), tpath);
	return;
      }
    }
  }
#endif
  
  // get day dirs in time order

  TimePathSet dayDirs;
  _getDayDirs(topDir, dayDirs);
  
  // search each day in reverse order
  // stop as soon as we find a interval file

  TimePathSet::reverse_iterator ii;
  for (ii = dayDirs.rbegin(); ii != dayDirs.rend(); ii++) {
    
    RadxTime midday(ii->fileStartTime);
    const string &dayDir = ii->path;

    TimePathSet tmpSet;
    _searchDay(dayDir, midday, 0, 0, tmpSet);
    
    if (tmpSet.size() > 0) {
      // interval times found, insert the last one into the set
      // and return
      timePaths.insert(timePaths.end(), *(tmpSet.rbegin()));
      return;
    }

  } // ii

}

///////////////////////////////////////////////////
// add clsest valid time to search time

void RadxTimeList::_addClosest(const string &topDir,
                               RadxTime searchTime,
                               RadxTime startTime,
                               RadxTime endTime,
                               TimePathSet &timePaths)

{

  // search for all interval times within the search range
  
  TimePathSet interval;
  _searchForValid(topDir, startTime, endTime, interval);
  if (interval.size() < 1) {
    return;
  }
  
  // find the result closest to the search time
  
  TimePathSet::iterator best = interval.begin();
  double minDiff = 1.0e9;
  TimePathSet::iterator ii;
  for (ii = interval.begin(); ii != interval.end(); ii++) {
    double diff = fabs(searchTime - ii->fileStartTime);
    if (diff <= minDiff) {
      minDiff = diff;
      best = ii;
    }
  } // ii
  
  timePaths.insert(timePaths.begin(), *best);

}

/////////////////////////////////////
// get day dirs for top dir

void RadxTimeList::_getDayDirs(const string &topDir,
			       TimePathSet &dayDirs)
  
{
  
  // set up dirs in time order
  
  RadxReadDir rdir;
  if (rdir.open(topDir.c_str())) {
    return;
  }
  
  // Loop thru directory looking for subdir names which represent dates
  
  struct dirent *dp;
  for (dp = rdir.read(); dp != NULL; dp = rdir.read()) {
    
    // exclude dir entries and files beginning with '.'
    
    if (dp->d_name[0] == '.') {
      continue;
    }

    // is this a yyyy directory - using extended paths?
    // if so, call this routine recursively

    if (strlen(dp->d_name) == 4) {
      int yyyy;
      if (sscanf(dp->d_name, "%4d", &yyyy) == 1) {
        // year dir, call this recursively
        string yyyyDir = topDir;
        yyyyDir += RadxPath::RADX_PATH_DELIM;
        yyyyDir += dp->d_name;
        _getDayDirs(yyyyDir, dayDirs);
      }
      continue;
    }

    // exclude dir entries too short for yyyymmdd
    
    if (strlen(dp->d_name) < 8 || dp->d_name[0] == '.') {
      continue;
    }

    // check that subdir name is in the correct format
    
    int year, month, day;
    if (sscanf(dp->d_name, "%4d%2d%2d", &year, &month, &day) != 3) {
      continue;
    }
    if (year < 1900 || month < 1 || month > 12 || day < 1 || day > 31) {
      continue;
    }
    
    RadxTime midday(year, month, day, 12, 0, 0);
    RadxPath dayDirPath(topDir, dp->d_name);
    string pathStr(dayDirPath.getPath());
    TimePath tpath(midday, midday, pathStr);
    dayDirs.insert(dayDirs.end(), tpath);

  } // for (dp ...
  
  rdir.close();
    
}

///////////////////////////////////////////
// check if this is a valid file to include

bool RadxTimeList::_isValidFile(const string &path)
  
{

  RadxPath P(path);

  // check extension

  if (_fileExt.size() > 0) {
    if (P.getExt() != _fileExt) {
      return false;
    }
  }

#ifdef NOTNOW
  string filename = P.getFile();
  if (filename.find(".nc") == string::npos &&
      filename.find(".cdf") == string::npos &&
      filename.find(".mdv") == string::npos &&
      filename.find(".uf") == string::npos &&
      filename.find(".rtd") == string::npos &&
      filename.find(".rapic") == string::npos &&
      filename.find(".RAW") == string::npos &&
      filename.find("swp") == string::npos) {
    return false;
  }
  if (filename.find("swp") != string::npos &&
      filename.find("IDL") != string::npos) {
    // dorade idle or rhi volumes
    // cerr << "XXXXXXXXXXXXXX filename: " << filename << endl;
    return false;
  }
  if (filename.find("swp") != string::npos &&
      filename.find("RHI") != string::npos) {
    // dorade idle or rhi volumes
    // cerr << "YYYYYYYYYYYYYYYY filename: " << filename << endl;
    return false;
  }
#endif

  // Get the file status since this will be used to perform
  // some other tests

  struct stat fstat;
  if (!RadxPath::doStat(path.c_str(), fstat)) {
    return false;
  }

  // Check the file size is non-zero.

  if (fstat.st_size == 0) {
    return false;
  }

  // Check this is a directory

  if ((fstat.st_mode & S_IFMT) == S_IFDIR) {
    return false;
  }

  // check mod time if needed

  if (_checkLatestValidModTime) {
    if (fstat.st_mtime > _latestValidModTime.utime()) {
      return false;
    }
  }

  return true;

}

///////////////////////////////////////////
// get time for Dorade file path
// Returns 0 on success, -1 on failure

int RadxTimeList::getDoradeTime(const string &path, RadxTime &doradeTime)
  
{

  // Dorade sweep file format:
  //   swp.yymmddhhmmss.... (1900 - 1999) or  swp.yyymmddhhmmss... (2000+)
  
  // find "swp." string

  const char *sweepStr = strstr(path.c_str(), "swp.");
  if (sweepStr == NULL) {
    return -1;
  }

  // tokenize path
  // file name is swp.yyymmddhhmmss.name.millisecs.angle_scantype_volnum

  vector<string> toks;
  RadxStr::tokenize(path, ".", toks);
  const char *timeStr = toks[1].c_str();
  
  int year, month, day, hour, min, sec;
  if (timeStr[0] == '1' || timeStr[0] == '2') {
    // year 2000+
    if (sscanf(timeStr, "%3d%2d%2d%2d%2d%2d",
               &year, &month, &day, &hour, &min, &sec) != 6) {
      return -1;
    }
  } else {
    // year 1900 - 1999
    if (sscanf(timeStr, "%2d%2d%2d%2d%2d%2d",
               &year, &month, &day, &hour, &min, &sec) != 6) {
      return -1;
    }
  }

  // dorade year is relative to 1990

  year += 1900;

  if (month < 1 || month > 12 || day < 1 || day > 31) {
    return -1;
  }
  if (hour < 0 || hour > 23 || min < 0 || min > 59 || sec < 0 || sec > 59) {
    return -1;
  }

  // get millisecs
  // file name is swp.yyymmddhhmmss.name.millisecs.angle_scantype_volnum

  int msecs = 0;
  double subSecs = 0.0;
  if ((sscanf(toks[3].c_str(), "%d", &msecs)) == 1) {
    subSecs = msecs / 1000.0;
  }
    
  doradeTime.set(year, month, day, hour, min, sec, subSecs);

  return 0;

}

///////////////////////////////////////////////////////////
// for sweep files, provide times for end of unique volumes

void RadxTimeList::_makeSweepVolumesUnique(TimePathSet &timePaths)
  
{

  // more than 1 path?

  if (timePaths.size() < 2) {
    return;
  }

  // do we have sweep files? If not, return now.
  
  const TimePath &first = *timePaths.begin();
  RadxPath fpath(first.path);
  if (fpath.getFile().find("swp") == string::npos) {
    // file name does not contain 'swp'
    return;
  }

  // build a set with only the last entry per volume

  TimePathSet unique;
  for (TimePathSet::iterator ii = timePaths.begin();
       ii != timePaths.end(); ii++) {

    // find vol number
    
    RadxPath path(ii->path);
    int volNum = _getVolNum(path.getFile());
    if (volNum == 0) continue;

    // find last entry with the same volume number
    
    TimePath last = *ii;
    TimePathSet::iterator lpos = ii++;
    for (TimePathSet::iterator jj = ii; jj != timePaths.end(); jj++, lpos++) {
      RadxPath upath(jj->path);
      int uVolNum = _getVolNum(upath.getFile());
      if (uVolNum == 0) break;;
      if (uVolNum != volNum) {
        break;
      }
      last = *jj;
    }

    // insert the unique entry

    unique.insert(unique.end(), last);

    // start new search at this point

    ii = lpos;

  }

  // make time paths unique

  timePaths = unique;

}

//////////////////////////////////////////////////////////
// get the volume number from the file name

int RadxTimeList::_getVolNum(const string &fileName)
  
{

  size_t vpos = fileName.find("_v");
  string vstr = fileName.substr(vpos);
  int volNum = 0;
  if (sscanf(vstr.c_str(), "_v%d", &volNum) != 1) {
    return 0;
  }
  return volNum;
}

