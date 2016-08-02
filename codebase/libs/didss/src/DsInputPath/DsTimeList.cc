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
// DsTimeList.cc
//
// Time list class for files in the DIDSS infrastructure
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 2000
//
//////////////////////////////////////////////////////////

#include <didss/DsTimeList.hh>
#include <didss/DsInputPath.hh>
#include <didss/LdataInfo.hh>
#include <toolsa/TaStr.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/ReadDir.hh>
#include <toolsa/file_io.h>
#include <toolsa/toolsa_macros.h>
#include <sys/stat.h>
#include <cerrno>
#include <functional>
#include <algorithm>
using namespace std;

//////////////////////
// Default Constructor
//

DsTimeList::DsTimeList()

{
  clearList();
  _debug = false;
}

/////////////////////////////
// Copy constructor
//

DsTimeList::DsTimeList(const DsTimeList &rhs)

{
  if (this != &rhs) {
    _copy(rhs);
  }
}

/////////////////////////////
// Destructor

DsTimeList::~DsTimeList()

{
}

/////////////////////////////
// Assignment
//

DsTimeList &DsTimeList::operator=(const DsTimeList &rhs)

{
  return _copy(rhs);
}

/////////////////////////////
// copy
//

DsTimeList &DsTimeList::_copy(const DsTimeList &rhs)

{

  if (&rhs == this) {
    return *this;
  }

  // copy members

  _errStr = rhs._errStr;
  _debug = rhs._debug;
  _dir = rhs._dir;
  _startTime = rhs._startTime;
  _endTime = rhs._endTime;
  _genTime = rhs._genTime;
  _timeList = rhs._timeList;
  _hasFcasts = rhs._hasFcasts;

  return *this;

}

/////////////////////////////////////////////////////////////////////
// clear the list
// Note: this is called automatically at the start of each get() call

void DsTimeList::clearList()
  
{
  _hasFcasts = false;
  _timeList.erase(_timeList.begin(), _timeList.end());
}

/////////////////////////////////////////////////////////////////
// getValid
//
// get the time list of all of the valid data
// times between the start and end times
//
// If purge is true, multiple time entries are purged from the
// list. These can occur with forecast data, when forecasts
// from multiple gen times refer to the same valid time.
//
// Returns 0 on success, -1 on error.
// Retrieve error with GetErrStr()

int DsTimeList::getValid(const string &dir,
			 const time_t start_time,
			 const time_t end_time,
			 bool purge /* = true*/ )

{

  _errStr = "ERROR - DsTimeList::getValid";
  TaStr::AddStr(_errStr, "  Dir: ", dir);
  TaStr::AddStr(_errStr, "  Start time: ", DateTime::str(start_time));
  TaStr::AddStr(_errStr, "  End   time: ", DateTime::str(end_time));

  // check dir exists

  if (!ta_stat_is_dir(dir.c_str())) {
    TaStr::AddStr(_errStr, "  Dir does not exist: ", dir);
    cerr << _errStr << endl;
    return -1;
  }

  _dir = dir;
  _startTime = start_time;
  _endTime = end_time;

  // compile list and sort

  clearList();
  if (_isByDay()) {
    _compileByDay(false);
  } else {
    _compileFlat(_dir);
  }

  if (purge) {
    _purgeMultEntries();
  } else {
    sort(_timeList.begin(), _timeList.end());
  }

  return 0;
  
}

/////////////////////////////////////////////////////////////////
// getGen
//
// get the time list of all of the forecast generate times
// between the start and end times
//
// Returns 0 on success, -1 on error.
// Retrieve error with GetErrStr()

int DsTimeList::getGen(const string &dir,
		       const time_t start_time,
		       const time_t end_time)

{

  _errStr = "ERROR - DsTimeList::getGen";
  TaStr::AddStr(_errStr, "  Dir: ", dir);
  TaStr::AddStr(_errStr, "  Start time: ", DateTime::str(start_time));
  TaStr::AddStr(_errStr, "  End   time: ", DateTime::str(end_time));

  // check dir exists

  if (!ta_stat_is_dir(dir.c_str())) {
    TaStr::AddStr(_errStr, "  Dir does not exist: ", dir);
    cerr << _errStr << endl;
    return -1;
  }

  _dir = dir;
  _startTime = start_time;
  _endTime = end_time;

  // compile list and sort

  clearList();
  _compileByDay(true);
  sort(_timeList.begin(), _timeList.end());

  return 0;

}

/////////////////////////////////////////////////////////////////
// getLead
//
// get the time list of all the lead times for the given generate
// time. time values are loaded in the list. To compute the
// lead time subtract the gen_time.
//
// Returns 0 on success, -1 on error.
// Retrieve error with GetErrStr()

int DsTimeList::getLead(const string &dir,
			const time_t gen_time)
  
{
 
  _errStr = "ERROR - DsTimeList::getLead";
  TaStr::AddStr(_errStr, "  Dir: ", dir);
  TaStr::AddStr(_errStr, "  Gen time: ", DateTime::str(gen_time));

  // check dir exists

  if (!ta_stat_is_dir(dir.c_str())) {
    TaStr::AddStr(_errStr, "  Dir does not exist: ", dir);
    cerr << _errStr << endl;
    return -1;
  }

  _dir = dir;
  _genTime = gen_time;

  // compile list and sort

  clearList();
  _compileLead();
  sort(_timeList.begin(), _timeList.end());
  
  return 0;

}

/////////////////////////////////////////////////////////////////
// getStartAndEnd
//
// Returns 0 on success, -1 on error.
// Retrieve error with GetErrStr()
//
// On success, sets start_time and end_time.

int DsTimeList::getStartAndEnd(const string &dir,
				time_t &start_time,
				time_t &end_time)

{
  
  _errStr = "ERROR - DsTimeList::getStartAndEnd";
  TaStr::AddStr(_errStr, "  Dir: ", dir);

  // check dir exists

  if (!ta_stat_is_dir(dir.c_str())) {
    TaStr::AddStr(_errStr, "  Dir does not exist: ", dir);
    cerr << _errStr << endl;
    return -1;
  }

  _dir = dir;

  // get start and end times

  if (_isByDay()) {
    if (_getStartAndEndByDay(start_time, end_time)) {
      _errStr += "  Cannot find start and end time, day structure\n";
      cerr << _errStr << endl;
      return -1;
    }
  } else {
    bool found = false;
    _getStartAndEndFlat(_dir, found, start_time, end_time);
    if (!found) {
      _errStr += "  Cannot find start and end time, flat structure\n";
      cerr << _errStr << endl;
      return -1;
    }
  }

  clearList();
  _timeList.push_back(start_time);
  _timeList.push_back(end_time);

  return 0;

}

//////////////////
// print time list

void DsTimeList::print(ostream &out)

{
  
  out << "DsTimeList" << endl;
  out << "----------" << endl;
  
  for (size_t ii = 0; ii < _timeList.size(); ii++) {
    cerr << DateTime::str(_timeList[ii]) << endl;
  }

}

/////////////////////////////////////////////
// compile for VALID or GENERATE time lists

void DsTimeList::_compileByDay(bool is_gen)
  
{

  
  // loop through day directories looking for suitable files
  
  int startDay = _startTime / 86400;
  int endDay = _endTime / 86400;

  for (int iday = startDay; iday <= endDay; iday++) {
    
    date_time_t midday;
    midday.unix_time = iday * 86400 + 43200;
    uconvert_from_utime(&midday);
    
    char dayDirPath[MAX_PATH_LEN];
    
    sprintf(dayDirPath, "%s%s%.4d%.2d%.2d",
	    _dir.c_str(), PATH_DELIM,
	    midday.year, midday.month, midday.day);
    
    ReadDir rdir;
    if (rdir.open(dayDirPath) == 0) {
      
      // Loop thru directory looking for the data file names
      // or forecast directories
      
      struct dirent *dp;
      for (dp = rdir.read(); dp != NULL; dp = rdir.read()) {
	
	// exclude dir entries and files beginning with '.'
	
	if (dp->d_name[0] == '.')
	  continue;
	
	// check that subdir name is in the correct format
	
	int hour, min, sec;
	char fileExt[16];
	
	bool isForecast;
	if (dp->d_name[0] == 'g' && dp->d_name[1] == '_') {
	  isForecast = true;
	  if (sscanf(dp->d_name + 2, "%2d%2d%2d",
		     &hour, &min, &sec) != 3)
	    continue;
	} else {
	  isForecast = false;
	  if (sscanf(dp->d_name, "%2d%2d%2d.%s",
		     &hour, &min, &sec, fileExt) != 4)
	    continue;
	}
	
	if (hour < 0 || hour > 23 || min < 0 || min > 59 ||
	    sec < 0 || sec > 59)
	  continue;
	
	date_time_t etime;
	etime = midday;
	etime.hour = hour;
	etime.min = min;
	etime.sec = sec;
	uconvert_to_utime(&etime);
	time_t entryTime = etime.unix_time;
	
	if (isForecast) {
	  
	  _hasFcasts = true;
	  
	  if (is_gen) {
	    _timeList.push_back(entryTime);
	  } else {
	    _addForecasts(dayDirPath, dp->d_name,
			  entryTime);
	  }

	} else {
	  
	  if (!is_gen) {
	    if (_startTime <= entryTime &&
		_endTime >= entryTime) {
	      _timeList.push_back(entryTime);
	    }
	  }

	}
	
      } // dp
      
      rdir.close();

    } // if (dirp ...
    
  } // iday

}

///////////////////////////
// add forecast valid times

void DsTimeList::_addForecasts(const string &dayDirPath,
			       const string &genSubDir,
			       time_t gen_time)
  
{
  
  string fcastDirPath = dayDirPath;
  fcastDirPath += PATH_DELIM;
  fcastDirPath += genSubDir;
  
  ReadDir rdir;
  if (rdir.open(fcastDirPath.c_str()) == 0) {
    
    // Loop through directory looking for the data file names
    
    struct dirent *dp;
    for (dp = rdir.read(); dp != NULL; dp = rdir.read()) {

      // exclude dir entries and files beginning with '.'
      
      if (dp->d_name[0] == '.')
	continue;
      
      // check that subdir name is in the correct format
      
      if (dp->d_name[0] != 'f' || dp->d_name[1] != '_')
	continue;

      int leadTime;
      char fileExt[16];
      if (sscanf(dp->d_name + 2, "%8d.%s", &leadTime, fileExt) != 2) {
	continue;
      }

      time_t fcastTime = gen_time + leadTime;
      if (_startTime <= fcastTime && _endTime >= fcastTime) {
	_timeList.push_back(fcastTime);
      }
      
    } // dp

    rdir.close();
      
  } // if (dirp ...
    
}

/////////////////////////////////////////////
// compile for flat directory structure

void DsTimeList::_compileFlat(const string &dir)
  
{
  
  // read through dir

  ReadDir rdir;
  if (rdir.open(dir.c_str()) == 0) {
    
    // Loop thru directory looking for subdir names which represent dates
    
    struct dirent *dp;
    for (dp = rdir.read(); dp != NULL; dp = rdir.read()) {
      
      // exclude dir entries and files beginning with '.'
      
      if (dp->d_name[0] == '.')
	continue;
      
      // compute path
      
      string path = dir;
      path += PATH_DELIM;
      path += dp->d_name;
      
      // directory??
      
      if (ta_stat_is_dir(path.c_str())) {

	// directory, so call recursively if it is not a date/time
	time_t dir_time;
	if (DsInputPath::getDataTime(path, dir_time) != 0) {
	  _compileFlat(path);
	  continue;
	}

      } else if (ta_stat_is_file(path.c_str())) {

	// file - check time

	time_t file_time;
	if (DsInputPath::getDataTime(path, file_time) == 0) {
	  if (file_time >= _startTime && file_time <= _endTime) {
	    _timeList.push_back(file_time);
	  }
	}

      } // if (ta_stat_is_dir ...

    } // for (dp ...
    
    rdir.close();
      
  }

}

//////////////////////////////////////
// compile list of forecast lead times

void DsTimeList::_compileLead()
  
{

  date_time_t gtime;
  gtime.unix_time = _genTime;
  uconvert_from_utime(&gtime);
  
  char genDirPath[MAX_PATH_LEN];
  
  sprintf(genDirPath, "%s%s%.4d%.2d%.2d%sg_%.2d%.2d%.2d",
	  _dir.c_str(), PATH_DELIM,
	  gtime.year, gtime.month, gtime.day, PATH_DELIM,
	  gtime.hour, gtime.min, gtime.sec);
  
  ReadDir rdir;
  if (rdir.open(genDirPath) == 0) {
    
    // Loop through directory looking for the data file names
    
    struct dirent *dp;
    for (dp = rdir.read(); dp != NULL; dp = rdir.read()) {
      
      // exclude dir entries and files beginning with '.'
      
      if (dp->d_name[0] == '.')
	continue;
      
      // check that subdir name is in the correct format
      
      if (dp->d_name[0] != 'f' || dp->d_name[1] != '_')
	continue;

      int leadTime;
      char fileExt[16];
      if (sscanf(dp->d_name + 2, "%8d.%s", &leadTime, fileExt) != 2) {
	continue;
      }

      time_t fcastTime = _genTime + leadTime;
      _timeList.push_back(fcastTime);

    } // dp

    rdir.close();
      
  } // if (dirp ...

}

//////////////////////////////
// get start time and end time

int DsTimeList::_getStartAndEndByDay(time_t &start_time,
				     time_t &end_time)
  
{

  // compile list of day dir times

  vector< time_t > dayTimes;
  
  ReadDir rdir;
  if (rdir.open(_dir.c_str()) == 0) {
    
    // Loop thru directory looking for subdir names which represent dates
    
    struct dirent *dp;
    for (dp = rdir.read(); dp != NULL; dp = rdir.read()) {
      
      // exclude dir entries and files beginning with '.'
      
      if (dp->d_name[0] == '.')
	continue;
      
      // check that subdir name is in the correct format
      
      int year, month, day;
      
      if (sscanf(dp->d_name, "%4d%2d%2d", &year, &month, &day) != 3) {
	continue;
      }
      
      if (year < 1900 || year > 3000 || month < 1 || month > 12 ||
	  day < 1 || day > 31) {
	continue;
      }

      DateTime dtime(year, month, day);
      dayTimes.push_back(dtime.utime());
      
    } // for (dp ...
    
    rdir.close();
      
  }

  // sort day times
  
  sort(dayTimes.begin(), dayTimes.end());
  
  // search for start day with valid times, by setting the start and
  // end times and using _compileByDay()

  bool start_found = false;
  clearList();
  for (size_t i = 0; i < dayTimes.size(); i++) {
    _startTime = dayTimes[i];
    _endTime = _startTime + 86399;
    _compileByDay(false);
    if (_timeList.size() > 0) {
      sort(_timeList.begin(), _timeList.end());
      start_time = _timeList[0];
      start_found = true;
      break;
    }
  } // i

  if (!start_found) {
    _errStr += "  Cannot find start time";
    return -1;
  }

  // search for end time - we must continue until the end time does not
  // change because with forecast data sometimes previous days hold
  // data with a later valid time
  
  bool end_found = false;
  end_time = start_time;
  for (int i = (int) dayTimes.size() - 1; i >= 0; i--) {
    _startTime = dayTimes[i];
    _endTime = _startTime + 86399;
    clearList();
    _compileByDay(false);
    if (_timeList.size() > 0) {
      end_found = true;
      sort(_timeList.begin(), _timeList.end());
      size_t nn = _timeList.size();
      if (end_time <= _timeList[nn - 1]) {
	end_time = _timeList[nn - 1];
      } else {
	break;
      }
    }
  } // i
  
  if (!end_found) {
    _errStr += "  Cannot find end time";
    return -1;
  }

  return 0;

}

/////////////////////////////////////////////////
// get start time and end time - flat dir struct

void DsTimeList::_getStartAndEndFlat(const string &dir,
				     bool &found,
				     time_t &start_time,
				     time_t &end_time)
  
{
  
  
  // read through dir
  
  ReadDir rdir;
  if (rdir.open(dir.c_str()) == 0) {
    
    // Loop thru directory looking for subdir names which represent dates
    
    struct dirent *dp;
    for (dp = rdir.read(); dp != NULL; dp = rdir.read()) {
      
      // exclude dir entries and files beginning with '.'
      
      if (dp->d_name[0] == '.')
	continue;
      
      // compute path
      
      string path = dir;
      path += PATH_DELIM;
      path += dp->d_name;
      
      // directory??
      
      if (ta_stat_is_dir(path.c_str())) {
	
	// directory, so call recursively if it is not a date/time
	time_t dir_time;
	if (DsInputPath::getDataTime(path, dir_time) != 0) {
	  _getStartAndEndFlat(path, found, start_time, end_time);
	  continue;
	}
	
      } else if (ta_stat_is_file(path.c_str())) {

	// file - check time
	
	time_t file_time;
	if (DsInputPath::getDataTime(path, file_time) == 0) {
	  if (!found) {
	    start_time = file_time;
	    end_time = file_time;
	    found = true;
	  } else {
	    start_time = MIN(start_time, file_time);
	    end_time = MAX(end_time, file_time);
	  }
	}

      } // if (ta_stat_is_dir ...

    } // for (dp ...
    
    rdir.close();
    
  }

}

//////////////////////////////////////////////////////////
// determine whether data is stored in flat dir structure
// or by day
//
// If by day, return true, else false

bool DsTimeList::_isByDay()
  
{

  ReadDir topDir;
  if (topDir.open(_dir.c_str()) == 0) {
    
    // Loop thru directory looking for subdir names which represent dates
    
    struct dirent *topDp;
    for (topDp = topDir.read(); topDp != NULL; topDp = topDir.read()) {
      
      // exclude dir entries and files beginning with '.'
      
      if (topDp->d_name[0] == '.')
	continue;
      
      // check that subdir name is in the correct format
      
      int year, month, day;
	
      if (sscanf(topDp->d_name, "%4d%2d%2d", &year, &month, &day) != 3) {
	continue;
      }
	
      if (year < 1900 || year > 3000 || month < 1 || month > 12 ||
	  day < 1 || day > 31) {
	continue;
      }

      // compute day path

      string dayPath = _dir;
      dayPath += PATH_DELIM;
      dayPath += topDp->d_name;

      if (!ta_stat_is_dir(dayPath.c_str())) {
	continue;
      }

      // read through day dir, looking for time files or gen time dirs
      
      ReadDir dayDir;
      if (dayDir.open(dayPath.c_str()) == 0) {
	
	// Loop thru directory looking for subdir names which represent dates
	
	struct dirent *dayDp;
	for (dayDp = dayDir.read(); dayDp != NULL; dayDp = dayDir.read()) {

	  // exclude dir entries and files beginning with '.'
	  
	  if (dayDp->d_name[0] == '.')
	    continue;
	  
	  // check that subdir name is in the correct format
	  
	  int hour, min, sec;
	  if (sscanf(dayDp->d_name,
		     "%2d%2d%2d", &hour, &min, &sec) == 3) {
	    if (hour < 0 || hour > 23 || min < 0 || min > 59  ||
		sec < 0 || sec > 59) {
	      continue;
	    }
	    string timePath = dayPath;
	    timePath += PATH_DELIM;
	    timePath += dayDp->d_name;
	    if (ta_stat_is_file(timePath.c_str())) {
	      // found time file
	      return true;
	    }
	  } else if (sscanf(dayDp->d_name,
			    "g_%2d%2d%2d", &hour, &min, &sec) == 3) {

	    if (hour < 0 || hour > 23 || min < 0 || min > 59  ||
		sec < 0 || sec > 59) {
	      continue;
	    }
	    string genPath = dayPath;
	    genPath += PATH_DELIM;
	    genPath += dayDp->d_name;
	    if (ta_stat_is_dir(genPath.c_str())) {
	      // found gen directory
	      return true;
	    }
	  }

	} // dayDp

	dayDir.close();
	
      } // if (dayDir.open ...
      
    } // topDp ...
    
    topDir.close();
      
  } // if (topDir.open ...

  return false;

}

/////////////////////////////////////////////////////////////////
// purgeMultEntries
//
// Purge the list of multiple entries at the same time.

void DsTimeList::_purgeMultEntries()

{

  // copy the vector to a set, which will only copy unique entries

  typedef set<time_t, less<time_t> > time_set_t;
  time_set_t tset;
  for (size_t i = 0; i < _timeList.size(); i++) {
    tset.insert(tset.end(), _timeList[i]);
  }
  
  // clear the list

  clearList();

  // copy the set back to the list

  time_set_t::iterator ii;
  for (ii = tset.begin(); ii != tset.end(); ii++) {
    _timeList.push_back(*ii);
  }

}

