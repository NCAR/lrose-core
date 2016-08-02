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
// MdvxTimelist.cc
//
// Timelist routines for Mdvx class
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2005
//
//////////////////////////////////////////////////////////

#include <Mdv/MdvxTimeList.hh>
#include <Mdv/Mdvx.hh>
#include <toolsa/TaStr.hh>
#include <toolsa/file_io.h>
#include <didss/RapDataDir.hh>
#include <didss/LdataInfo.hh>
#include <toolsa/ReadDir.hh>
#include <toolsa/str.h>
#include <iomanip>
#include <iterator>
#include <algorithm>
#include <sys/stat.h>
#include <cerrno>
using namespace std;

/////////////////////////////////////////////////////////////////
// constructor

MdvxTimeList::MdvxTimeList()

{
  clearMode();
  clearData();
  clearCheckLatestValidModTime();
  clearConstrainFcastLeadTimes();
  clearValidTimeSearchWt();
}

/////////////////////////////////////////////////////////////////
// destructor

MdvxTimeList::~MdvxTimeList()

{
  
}

/////////////////////////
// clear the error string

void MdvxTimeList::clearErrStr() const
{
  _errStr = "";
}

/////////////////////////////////////////////////////////////////
// setModeValid
//
// Set the time list so that it finds all of the valid data
// times between the start and end times.
// For forecast data where multiple forecasts exist for the same
// valid time, a single valid time will be returned.

void MdvxTimeList::setModeValid(const string &dir,
				time_t start_time,
				time_t end_time)
  
{
  
  clearMode();
  _mode = MODE_VALID;
  _dir = dir;
  _startTime = start_time;
  _endTime = end_time;

}

/////////////////////////////////////////////////////////////////
// setModeGen
//
// Set the time list mode so that it finds all of the
// generate times between the start and end gen times

void MdvxTimeList::setModeGen(const string &dir,
			      time_t start_gen_time,
			      time_t end_gen_time)
  
{
  
  clearMode();
  _mode = MODE_GENERATE;
  _dir = dir;
  _startTime = start_gen_time;
  _endTime = end_gen_time;
  
}

/////////////////////////////////////////////////////////////////
// setModeForecast
//
// Set the time list so that it finds all of the forecast
// times for the given generate time

void MdvxTimeList::setModeForecast(const string &dir,
				   time_t gen_time)

{

  clearMode();
  _mode = MODE_FORECAST;
  _dir = dir;
  _genTime = gen_time;

}

/////////////////////////////////////////////////////////////////
// setModeGenPlusForecasts
//
// Set the time list so that it finds all of the
// generate times between the start and end gen times.
// Then, for each generate time, all of the forecast times are
// found. These are made available in the
// _forecastTimesArray, which is represented by vector<vector<time_t> >

void MdvxTimeList::setModeGenPlusForecasts(const string &dir,
					   time_t start_gen_time,
					   time_t end_gen_time)

{
  
  clearMode();
  _mode = MODE_GEN_PLUS_FCASTS;
  _dir = dir;
  _startTime = start_gen_time;
  _endTime = end_gen_time;
  
}

/////////////////////////////////////////////////////////////////
// setModeValidMultGen
//
// Set the time list so that it finds all of the forecasts
// within the time interval specified. For each forecast found
// the associated generate time is also determined.
// The forecast times will be available in the _validTimes array.
// The generate times will be available in the _genTimes array.
  
void
MdvxTimeList::setModeValidMultGen(const string &dir,
				  time_t start_time,
				  time_t end_time)

{
  
  clearMode();
  _mode = MODE_VALID_MULT_GEN;
  _dir = dir;
  _startTime = start_time;
  _endTime = end_time;
  
}
  
/////////////////////////////////////////////////////////////////
// setModeFirst
//
// set the time list mode so that it finds the first data time

void MdvxTimeList::setModeFirst(const string &dir)

{

  clearMode();
  _mode = MODE_FIRST;
  _dir = dir;

}

/////////////////////////////////////////////////////////////////
// setModeLast
//
// set the time list mode so that it finds the last data time

void MdvxTimeList::setModeLast(const string &dir)

{

  clearMode();
  _mode = MODE_LAST;
  _dir = dir;

}

/////////////////////////////////////////////////////////////////
// SetModeClosest
//
// set the time list mode so that it finds the closest available data time
// to the search time within the search margin

void MdvxTimeList::setModeClosest(const string &dir,
				  time_t search_time, int time_margin)
  
{

  clearMode();
  _mode = MODE_CLOSEST;
  _dir = dir;
  _searchTime = search_time;
  _timeMargin = time_margin;

}

/////////////////////////////////////////////////////////////////
// SetModeFirstBefore
//
// set the time list mode so that it finds the first available data time
// before the search time within the search margin

void MdvxTimeList::setModeFirstBefore(const string &dir,
				      time_t search_time, int time_margin)

{

  clearMode();
  _mode = MODE_FIRST_BEFORE;
  _dir = dir;
  _searchTime = search_time;
  _timeMargin = time_margin;

}

/////////////////////////////////////////////////////////////////
// SetModeFirstAfter
//
// set the time list mode so that it finds the first available data time
// after the search time within the search margin

void MdvxTimeList::setModeFirstAfter(const string &dir,
				     time_t search_time,
				     int time_margin)
  
{

  clearMode();
  _mode = MODE_FIRST_AFTER;
  _dir = dir;
  _searchTime = search_time;
  _timeMargin = time_margin;

}

/////////////////////////////////////////////////////////////////
// setModeBestForecast
//
// Set the time list so that it returns the best forecast
// for the search time, within the time margin

void MdvxTimeList::setModeBestForecast(const string &dir,
				       time_t search_time,
				       int time_margin)

{

  clearMode();
  _mode = MODE_BEST_FCAST;
  _dir = dir;
  _searchTime = search_time;
  _timeMargin = time_margin;

}
  
/////////////////////////////////////////////////////////////////
// setModeSpecifiedForecast
//
// Set the time list so that it returns the forecast for the given
// generate time, closest to the search time, within the time margin

void MdvxTimeList::setModeSpecifiedForecast(const string &dir,
					    time_t gen_time,
					    time_t search_time,
					    int time_margin)

{

  clearMode();
  _mode = MODE_SPECIFIED_FCAST;
  _dir = dir;
  _genTime = gen_time;
  _searchTime = search_time;
  _timeMargin = time_margin;

}
  
/////////////////////////////////////////////////////////////////
// clearMode
//
// clear out time list mode info

void MdvxTimeList::clearMode()

{

  _mode = MODE_UNDEFINED;
  _dir = "";
  _startTime = 0;
  _endTime = 0;
  _genTime = 0;
  _searchTime = 0;
  _timeMargin = 0;

}

/////////////////////////////////////////////////////////////////
// clear all data

void MdvxTimeList::clearData()

{
  _validTimes.clear();
  _genTimes.clear();
  _pathList.clear();
  _forecastTimesArray.clear();
  _hasForecasts = false;
}

/////////////////////////////////////////////////////////////////
// Constrain the lead times to be included in the time list.
//
// Only forecast lead times within the specified limits will
// be included.
//
// Also, you can specify to request the data by generate time
// rather than valid time. The valid time will be computed as
// the request_time plus the mean of the min and max lead times.

void
MdvxTimeList::setConstrainFcastLeadTimes(int min_lead_time,
					 int max_lead_time,
					 bool request_by_gen_time)
  
{
  _constrainFcastLeadTimes = true;
  _minFcastLeadTime = min_lead_time;
  _maxFcastLeadTime = max_lead_time;
  _specifyFcastByGenTime = request_by_gen_time;
}
  
void
MdvxTimeList::clearConstrainFcastLeadTimes()
  
{
  _constrainFcastLeadTimes = false;
  _minFcastLeadTime = 0;
  _maxFcastLeadTime = 0;
  _specifyFcastByGenTime = false;
}

///////////////////////////////////////////////////////////
// Set the weight given to valid time differences, over gen
// time differences, when searching for the best forecast
// for a specified time
//
// Finding the best forecast for a given time is tricky. Do
// you care more about differences between the valid time
// and the requested time, or do you want to give more weight
// to the closest gen time.
//
// The default value is 2.5. This works well for most situations.
// 
// If the time between model runs is long (say 6 hours) as compared
// to the time between model output times (say 30 mins) then you
// need to increase the weight to say 25. Setting it to 100
// will weight the decision very heavily in favor of the diff
// between the valid and requested time, and put very little
// weight on which model run to use.

void MdvxTimeList::setValidTimeSearchWt(double wt)
{
  _validTimeSearchWt = wt;
}

void MdvxTimeList::clearValidTimeSearchWt()
{
  _validTimeSearchWt = -1;
}

double MdvxTimeList::getValidTimeSearchWt() const
{
  return _validTimeSearchWt;
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
  
int MdvxTimeList::compile()

{

  clearData();
  clearErrStr();
  
  if (_mode == MODE_UNDEFINED) {
    _errStr += "ERROR - MdvxTimeList::compile\n";
    _errStr += "  Undefined mode.\n";
    _errStr += "  You must set the mode before calling this routine.\n";
    return -1;
  }

  // adjust search time as appropriate if specified by gen time

  if (_constrainFcastLeadTimes && _specifyFcastByGenTime) {
    int meanLeadTime = (_minFcastLeadTime + _maxFcastLeadTime) / 2;
    switch (_mode) {
    case MODE_VALID:
    case MODE_VALID_MULT_GEN:
      _startTime += meanLeadTime;
      _endTime += meanLeadTime;
      break;
    case MODE_CLOSEST:
    case MODE_FIRST_BEFORE:
    case MODE_FIRST_AFTER:
    case MODE_BEST_FCAST:
      _searchTime += meanLeadTime;
      break;
    default: {}
    } // switch
  } // if (_constrainFcastLeadTimes ...
  
  // fill out dir with RAP_DATA_DIR as appropriate
  
  string topDir;
  RapDataDir.fillPath(_dir, topDir);

  // check that the directory exists

  struct stat dirStat;
  if (stat(topDir.c_str(), &dirStat)) {
    int errNum = errno;
    _errStr += "ERROR - MdvxTimeList::compileTimeList\n";
    _errStr += "  Cannot stat requested directory.\n";
    TaStr::AddStr(_errStr, topDir.c_str(), strerror(errNum));
    return -1;
  }

  // check for forecast format

  _checkHasForecasts(topDir);
  
  // compile depending on mode

  switch (_mode) {
  case MODE_VALID:
    _compileValid(topDir);
    break;
  case MODE_GENERATE:
    _compileGenerate(topDir);
    break;
  case MODE_FORECAST:
    _compileForecast(topDir);
    break;
  case MODE_FIRST:
    _compileFirst(topDir);
    break;
  case MODE_LAST:
    _compileLast(topDir);
    break;
  case MODE_GEN_PLUS_FCASTS:
    _compileGenPlusFcasts(topDir);
    break;
  case MODE_VALID_MULT_GEN:
    _compileValidMultGen(topDir);
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
  case MODE_BEST_FCAST:
    _compileBestForecast(topDir);
    break;
  case MODE_SPECIFIED_FCAST:
    _compileSpecifiedForecast(topDir);
    break;
  case MODE_UNDEFINED:
    break;
  } // switch

  // for the special case of specifying the fcast by gen time,
  // copy the gen times over into the valid time array

  if (_constrainFcastLeadTimes && _specifyFcastByGenTime) {
    _validTimes = _genTimes;
  }

  return 0;

}

//////////////////////////
// print time list request

void MdvxTimeList::printRequest(ostream &out)

{
  
  out << "Mdvx time list request" << endl;
  out << "----------------------" << endl;
  
  if (_mode == MODE_VALID) {
    out << "  Mode: valid times" << endl;
  } else if (_mode == MODE_GENERATE) {
    out << "  Mode: generate times" << endl;
  } else if (_mode == MODE_FORECAST) {
    out << "  Mode: forecast times" << endl;
  } else if (_mode == MODE_GEN_PLUS_FCASTS) {
    out << "  Mode: gen times plus all forecast times" << endl;
  } else if (_mode == MODE_VALID_MULT_GEN) {
    out << "  Mode: valid times along with multiple gen times" << endl;
  } else if (_mode == MODE_FIRST) {
    out << "  Mode: first time" << endl;
  } else if (_mode == MODE_LAST) {
    out << "  Mode: last time" << endl;
  } else if (_mode == MODE_CLOSEST) {
    out << "  Mode: closest time" << endl;
  } else if (_mode == MODE_FIRST_BEFORE) {
    out << "  Mode: first_before time" << endl;
  } else if (_mode == MODE_FIRST_AFTER) {
    out << "  Mode: first_after time" << endl;
  } else if (_mode == MODE_BEST_FCAST) {
    out << "  Mode: best_forecast time" << endl;
  } else if (_mode == MODE_SPECIFIED_FCAST) {
    out << "  Mode: specified_forecast time" << endl;
  }
  
  if (_constrainFcastLeadTimes) {
    out << "  Constrain forecast lead times: TRUE" << endl;
    out << "    Min lead time: " << _minFcastLeadTime << endl;
    out << "    Max lead time: " << _maxFcastLeadTime << endl;
    if (_specifyFcastByGenTime) {
      out << "    Specify search by gen time: TRUE" << endl;
    }
  }

  out << "  dir: " << _dir << endl;
  
  if (_mode == MODE_VALID ||
      _mode == MODE_GENERATE ||
      _mode == MODE_GEN_PLUS_FCASTS ||
      _mode == MODE_VALID_MULT_GEN) {
    out << "  start time: " << utimstr(_startTime) << endl;
    out << "  end time: " << utimstr(_endTime) << endl;
  } else if (_mode == MODE_FORECAST) {
    out << "  gen time: " << utimstr(_genTime) << endl;
  }

}

///////////////////////////////////////////////////
// check for forecast format

void MdvxTimeList::_checkHasForecasts(const string &topDir)

{

  _hasForecasts = false;

  // get day dirs in time order

  TimePathSet dayDirs;
  _getDayDirs(topDir, dayDirs);
  
  // search each day in reverse order
  // stop as soon as we find a valid file
  
  TimePathSet::reverse_iterator ii;
  for (ii = dayDirs.rbegin(); ii != dayDirs.rend(); ii++) {
    
    DateTime midday(ii->validTime);
    const string &dayDir = ii->path;
    
    ReadDir rdir;
    if (rdir.open(dayDir.c_str())) {
      return;
    }
      
    // Loop thru directory looking for the data file names
    // or forecast directories
    
    struct dirent *dp;
    for (dp = rdir.read(); dp != NULL; dp = rdir.read()) {
      
      // exclude dir entries beginning with '.'
      
      if (dp->d_name[0] == '.') {
	continue;
      }
      
      // is this in yyyymmdd format?

      int hour, min, sec;
      if (sscanf(dp->d_name, "%2d%2d%2d", &hour, &min, &sec) == 3) {
	if (hour >= 0 && hour <= 23 && min >= 0 && min <= 59 &&
	    sec >= 0 && sec <= 59) {
	  _hasForecasts = false;
	  rdir.close();
	  return;
	}
      }
      
      // is this in g_yyyymmdd format?
      
      if (sscanf(dp->d_name, "g_%2d%2d%2d", &hour, &min, &sec) == 3) {
	if (hour >= 0 && hour <= 23 && min >= 0 && min <= 59 &&
	    sec >= 0 && sec <= 59) {
	  _hasForecasts = true;
	  rdir.close();
	  return;
	}
      }
      
    } // dp
    
    rdir.close();

  } // ii

}

///////////////////////////////////////////////////
// compile valid

void MdvxTimeList::_compileValid(const string &topDir)

{

  // search for valid times

  TimePathSet timePaths;
  _searchForValid(topDir, _startTime, _endTime, timePaths);

  // fill time lists

  if (_hasForecasts) {

    // fill out data arrays with result, removing duplicate
    // valid times which exist for multiple gen times
    
    TimePathSet::iterator ii;
    time_t prevValidTime = 0;
    for (ii = timePaths.begin(); ii != timePaths.end(); ii++) {
      if (_validTimes.size() > 0 && ii->validTime == prevValidTime) {
	if (ii->genTime > _genTimes[_genTimes.size() - 1]) {
	  _validTimes[_validTimes.size() - 1] = ii->validTime;
	  _genTimes[_genTimes.size() - 1] = ii->genTime;
	  _pathList[_pathList.size() - 1] = ii->path;
	}
      } else {
	_validTimes.push_back(ii->validTime);
	_genTimes.push_back(ii->genTime);
	_pathList.push_back(ii->path);
      }
      prevValidTime = ii->validTime;
    } // ii

  } else {

    TimePathSet::iterator ii;
    for (ii = timePaths.begin(); ii != timePaths.end(); ii++) {
      _validTimes.push_back(ii->validTime);
      _genTimes.push_back(ii->genTime);
      _pathList.push_back(ii->path);
    } // ii

  }

}

///////////////////////////////////////////////////
// compile generate

void MdvxTimeList::_compileGenerate(const string &topDir)

{

  if (!_hasForecasts) {
    return;
  }

  TimePathSet timePaths;

  // loop through day directories looking for valid dates
  
  int startDay = _startTime / SECS_IN_DAY;
  if (_startTime < 0) {
    startDay -= 1;
  }
  int endDay = _endTime / SECS_IN_DAY;
  if (_endTime < 0) {
    endDay -= 1;
  }
  
  for (int iday = startDay; iday <= endDay; iday++) {
    
    DateTime midday(iday * SECS_IN_DAY + SECS_IN_DAY / 2);
    char dayDir[MAX_PATH_LEN];

    // normal format

    sprintf(dayDir, "%s%s%.4d%.2d%.2d",
	    topDir.c_str(), PATH_DELIM,
	    midday.getYear(), midday.getMonth(), midday.getDay());
    _searchDayGen(dayDir, midday, true, _startTime, _endTime, timePaths);

    // extended format
    
    sprintf(dayDir, "%s%s%.4d%s%.4d%.2d%.2d",
	    topDir.c_str(), PATH_DELIM, midday.getYear(), PATH_DELIM,
	    midday.getYear(), midday.getMonth(), midday.getDay());
    _searchDayGen(dayDir, midday, true, _startTime, _endTime, timePaths);
    
  } // iday
  
  // fill out data arrays with result
  
  TimePathSet::iterator ii;
  for (ii = timePaths.begin(); ii != timePaths.end(); ii++) {
    _validTimes.push_back(ii->validTime);
    _genTimes.push_back(ii->genTime);
    _pathList.push_back(ii->path);
  }

}

///////////////////////////////////////////////////
// compile forecast

void MdvxTimeList::_compileForecast(const string &topDir)

{

  if (!_hasForecasts) {
    return;
  }

  // compute sub dir name for specified gen time

  DateTime genTime(_genTime);
  char subDir[MAX_PATH_LEN];

  // normal paths

  sprintf(subDir, "%s%s%.4d%.2d%.2d%sg_%.2d%.2d%.2d",
	  topDir.c_str(),
	  PATH_DELIM,
	  genTime.getYear(), genTime.getMonth(), genTime.getDay(),
	  PATH_DELIM,
	  genTime.getHour(), genTime.getMin(), genTime.getSec());
  
  _compileForecastForSubDir(subDir);

  // extended paths

  sprintf(subDir, "%s%s%.4d%s%.4d%.2d%.2d%sg_%.2d%.2d%.2d",
	  topDir.c_str(), PATH_DELIM,
	  genTime.getYear(), PATH_DELIM,
	  genTime.getYear(), genTime.getMonth(), genTime.getDay(),
	  PATH_DELIM,
	  genTime.getHour(), genTime.getMin(), genTime.getSec());
  
  _compileForecastForSubDir(subDir);

}

void MdvxTimeList::_compileForecastForSubDir(const string &subDir)

{

  // add all forecast to the set - do not check times

  TimePathSet timePaths;
  _addForecast(subDir, _genTime, false, 0, 0, timePaths);
  
  // fill out data arrays with result
  
  TimePathSet::iterator ii;
  for (ii = timePaths.begin(); ii != timePaths.end(); ii++) {
    _validTimes.push_back(ii->validTime);
    _genTimes.push_back(ii->genTime);
    _pathList.push_back(ii->path);
  }

}

///////////////////////////////////////////////////
// compile first

void MdvxTimeList::_compileFirst(const string &topDir)

{

  // get the first time

  TimePathSet timePaths;
  _addFirst(topDir, timePaths);

  if (timePaths.size() > 0) {
    // valid time found, use the first one
    TimePathSet::iterator ii = timePaths.begin();
    _validTimes.push_back(ii->validTime);
    _genTimes.push_back(ii->genTime);
    _pathList.push_back(ii->path);
  }

}

///////////////////////////////////////////////////
// compile last

void MdvxTimeList::_compileLast(const string &topDir)

{

  // get the last time

  TimePathSet timePaths;
  _addLast(topDir, timePaths);
  
  if (timePaths.size() > 0) {
    // valid time found, use the last one
    TimePathSet::reverse_iterator ii = timePaths.rend();
    _validTimes.push_back(ii->validTime);
    _genTimes.push_back(ii->genTime);
    _pathList.push_back(ii->path);
  }

}

///////////////////////////////////////////////////
// compile genPlusFcasts

void MdvxTimeList::_compileGenPlusFcasts(const string &topDir)

{

  // compile the list of generate times

  _compileGenerate(topDir);

  // add in forecast times array

  _addForecastTimesArray(_dir, _genTimes, _forecastTimesArray);

}

///////////////////////////////////////////////////
// compile validMultGen

void MdvxTimeList::_compileValidMultGen(const string &topDir)

{

  // search for valid times
  
  TimePathSet timePaths;
  _searchForValid(topDir, _startTime, _endTime, timePaths);

  // fill out data arrays with result
  
  TimePathSet::iterator ii;
  for (ii = timePaths.begin(); ii != timePaths.end(); ii++) {
    _validTimes.push_back(ii->validTime);
    _genTimes.push_back(ii->genTime);
    _pathList.push_back(ii->path);
  } // ii

}

///////////////////////////////////////////////////
// compile closest

void MdvxTimeList::_compileClosest(const string &topDir)

{

  // use best forecast search, centered on the search time

  TimePathSet timePaths;
  time_t startTime = _searchTime - _timeMargin;
  time_t endTime = _searchTime + _timeMargin;
  
  if (_hasForecasts) {
    _addBestForecast(topDir, _searchTime, startTime, endTime, timePaths);
  } else {
    _addBestValid(topDir, _searchTime, startTime, endTime, timePaths);
  }

  // fill time lists

  if (timePaths.size() > 0) {
    TimePathSet::iterator best = timePaths.begin();
    _validTimes.push_back(best->validTime);
    _genTimes.push_back(best->genTime);
    _pathList.push_back(best->path);
  }

}

///////////////////////////////////////////////////
// compile first before

void MdvxTimeList::_compileFirstBefore(const string &topDir)

{

  // use best forecast search, ending at the search time

  TimePathSet timePaths;
  time_t startTime = _searchTime - _timeMargin;
  time_t endTime = _searchTime;
  
  if (_hasForecasts) {
    _addBestForecast(topDir, _searchTime, startTime, endTime, timePaths);
  } else {
    _addBestValid(topDir, _searchTime, startTime, endTime, timePaths);
  }
    
  // fill time lists

  if (timePaths.size() > 0) {
    TimePathSet::iterator best = timePaths.begin();
    _validTimes.push_back(best->validTime);
    _genTimes.push_back(best->genTime);
    _pathList.push_back(best->path);
  }

}

///////////////////////////////////////////////////
// compile first after

void MdvxTimeList::_compileFirstAfter(const string &topDir)

{

  // use best forecast search, starting at the search time
  
  TimePathSet timePaths;
  time_t startTime = _searchTime;
  time_t endTime = _searchTime + _timeMargin;
  
  if (_hasForecasts) {
    _addBestForecast(topDir, _searchTime, startTime, endTime, timePaths);
  } else {
    _addBestValid(topDir, _searchTime, startTime, endTime, timePaths);
  }
    
  // fill time lists

  if (timePaths.size() > 0) {
    TimePathSet::iterator best = timePaths.begin();
    _validTimes.push_back(best->validTime);
    _genTimes.push_back(best->genTime);
    _pathList.push_back(best->path);
  }

}

///////////////////////////////////////////////////
// compile best forecast

void MdvxTimeList::_compileBestForecast(const string &topDir)

{

  if (!_hasForecasts) {
    return;
  }

  TimePathSet timePaths;
  time_t startTime = _searchTime - _timeMargin;
  time_t endTime = _searchTime + _timeMargin;
  _addBestForecast(topDir, _searchTime, startTime, endTime, timePaths);
  
  if (timePaths.size() > 0) {
    TimePathSet::iterator best = timePaths.begin();
    _validTimes.push_back(best->validTime);
    _genTimes.push_back(best->genTime);
    _pathList.push_back(best->path);
  }

}

///////////////////////////////////////////////////
// compile specified forecast

void MdvxTimeList::_compileSpecifiedForecast(const string &topDir)

{
  
  // compute sub dir name for specified gen time
  
  DateTime genTime(_genTime);
  char subDir[MAX_PATH_LEN];

  // normal paths

  sprintf(subDir, "%s%s%.4d%.2d%.2d%sg_%.2d%.2d%.2d",
	  topDir.c_str(),
	  PATH_DELIM,
	  genTime.getYear(), genTime.getMonth(), genTime.getDay(),
	  PATH_DELIM,
	  genTime.getHour(), genTime.getMin(), genTime.getSec());
  _compileSpecForecastForSubDir(subDir);

  // extended paths
  
  sprintf(subDir, "%s%s%.4d%s%.4d%.2d%.2d%sg_%.2d%.2d%.2d",
	  topDir.c_str(), PATH_DELIM,
	  genTime.getYear(), PATH_DELIM,
	  genTime.getYear(), genTime.getMonth(), genTime.getDay(),
	  PATH_DELIM,
	  genTime.getHour(), genTime.getMin(), genTime.getSec());
  _compileSpecForecastForSubDir(subDir);

}

///////////////////////////////////////////////////
// compile specified forecast for sub dir

void MdvxTimeList::_compileSpecForecastForSubDir(const string &subDir)

{
  
  // add forecasts to the set within the time range
  
  TimePathSet timePaths;
  time_t startTime = _searchTime - _timeMargin;
  time_t endTime = _searchTime + _timeMargin;
  
  _addForecast(subDir, _genTime, true,
	       startTime, endTime, timePaths);
  
  if (timePaths.size() == 0) {
    return;
  }

  // find best forecast
  
  TimePathSet::iterator best = timePaths.begin();
  double minDiff = 1.0e9;
  TimePathSet::iterator ii;
  for (ii = timePaths.begin(); ii != timePaths.end(); ii++) {
    double diff = fabs((double) _searchTime - ii->validTime);
    if (diff <= minDiff) {
      minDiff = diff;
      best = ii;
    }
  } // ii
  
  _validTimes.push_back(best->validTime);
  _genTimes.push_back(best->genTime);
  _pathList.push_back(best->path);
  
}

///////////////////////////////////////////////////
// Search for valid times

void MdvxTimeList::_searchForValid(const string &topDir,
				   time_t startTime,
				   time_t endTime,
				   TimePathSet &timePaths)
  
{
  
  // search through days in time range
  
  int startDay = startTime / SECS_IN_DAY;
  if (startTime < 0) {
    startDay -= 1;
  }
  int endDay = endTime / SECS_IN_DAY;
  if (endTime < 0) {
    endDay -= 1;
  }
  
  _searchDayRangeForValid(topDir, startDay, endDay,
			  startTime, endTime, timePaths);
  
  // for forecast data, go back a day at a time,
  // until no more valid times are added. This is done because valid
  // times can come from generate times well in the past

  if (_hasForecasts) {
    size_t nTimes = timePaths.size();
    for (int iday = startDay - 1; iday > startDay - 365; iday--) {
      _searchDayRangeForValid(topDir, iday, iday,
			      startTime, endTime, timePaths);
      if (timePaths.size() == nTimes && nTimes > 0) {
	break;
      }
      nTimes = timePaths.size();
    }
  }

  // for sweep files, provide times for end of unique volumes
  
  _makeSweepVolumesUnique(timePaths);
  
}

///////////////////////////////////////////////////
// Search through the directories in the time range

void MdvxTimeList::_searchDayRangeForValid(const string &dir,
					   int startDay,
					   int endDay,
					   time_t startTime,
					   time_t endTime,
					   TimePathSet &timePaths)
  
{
  
  // loop through day directories looking for suitable files
  
  for (int iday = startDay; iday <= endDay; iday++) {
    
    DateTime midday(iday * SECS_IN_DAY + SECS_IN_DAY / 2);
    char dayDir[MAX_PATH_LEN];

    // normal format

    sprintf(dayDir, "%s%s%.4d%.2d%.2d",
	    dir.c_str(), PATH_DELIM,
	    midday.getYear(), midday.getMonth(), midday.getDay());
    _searchDayForValid(dayDir, midday, true, startTime, endTime, timePaths);

    // extended format
    
    sprintf(dayDir, "%s%s%.4d%s%.4d%.2d%.2d",
	    dir.c_str(), PATH_DELIM, midday.getYear(), PATH_DELIM, 
	    midday.getYear(), midday.getMonth(), midday.getDay());
    _searchDayForValid(dayDir, midday, true, startTime, endTime, timePaths);

  } // iday

}

///////////////////////////////////////////////////
// Search a day directory for valid files

void MdvxTimeList::_searchDayForValid(const string &dayDir,
				      const DateTime &midday,
				      bool checkTimeRange,
				      time_t startTime,
				      time_t endTime,
				      TimePathSet &timePaths)
  
{

  ReadDir rdir;
  if (rdir.open(dayDir.c_str()) == 0) {
    
    // Loop thru directory looking for the data file names
    // or forecast directories
    
    struct dirent *dp;
    for (dp = rdir.read(); dp != NULL; dp = rdir.read()) {
      
      // exclude dir entries beginning with '.'
      
      if (dp->d_name[0] == '.') {
	continue;
      }
      
      if (_hasForecasts) {
	_addValidFromGenSubdir(dayDir, midday, dp->d_name,
			       checkTimeRange, startTime, endTime,
			       timePaths);
      } else {
	_addValid(dayDir, midday, dp->d_name,
		  checkTimeRange, startTime, endTime, timePaths);
      }
      
    } // dp
    
    rdir.close();

  } // if (rdir ...
  
}

///////////////////////////////////////////////////
// Search a day directory for generate sub-dirs

void MdvxTimeList::_searchDayGen(const string &dayDir,
				 const DateTime &midday,
				 bool checkTimeRange,
				 time_t startTime,
				 time_t endTime,
				 TimePathSet &timePaths)
  
{

  ReadDir rdir;
  if (rdir.open(dayDir.c_str()) == 0) {
    
    // Loop thru directory looking for the data file names
    // or forecast directories
    
    struct dirent *dp;
    for (dp = rdir.read(); dp != NULL; dp = rdir.read()) {
      
      // exclude dir entries beginning with '.'
      
      if (dp->d_name[0] == '.') {
	continue;
      }

      _addGen(dayDir, midday, dp->d_name,
	      checkTimeRange, startTime, endTime, timePaths);
      
    } // dp
      
    rdir.close();

  } // if (rdir ...
  
}

///////////////////////////////////////////////////
// add valid time

void MdvxTimeList::_addValid(const string &dayDir,
			     const DateTime &midday,
			     const string &entryName,
			     bool checkTimeRange,
			     time_t startTime,
			     time_t endTime,
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
  
  // get time

  time_t entryTime = 0;
  int year, month, day, hour, min, sec;
  const char *end = start + strlen(start);
  char spacer;
  while (start < end - 6) {
    if (strncmp(entryName.c_str(), "swp.", 4) == 0) {
      // dorade sweep file
      DateTime doradeTime;
      if (getDoradeTime(entryName, doradeTime)) {
        return;
      }
      // do not include IDLE files
      if (strstr(entryName.c_str(), "IDL") != NULL) {
        return;
      }
      entryTime = doradeTime.utime();
      break;
    } else if (entryName.find(".rapic") != string::npos) {
      // rapic sweep file
      // format - yyyymmddhmm
      if ((sscanf(start, "%4d%2d%2d%2d%2d",
                  &year, &month, &day, &hour, &min) == 5)) {
        if (year < 1900 || month < 1 || month > 12 || day < 1 || day > 31) {
          return;
        }
        if (hour < 0 || hour > 23 || min < 0 || min > 59) {
          return;
        }
        DateTime etime(year, month, day, hour, min, 0);
        entryTime = etime.utime();
        break;
      }
    } else if (entryName.find(".RAW") != string::npos) {
      // SIGMET RAW volume file
      // format - SRyyymmddhmmss
      if ((sscanf(start, "%3d%2d%2d%2d%2d%2d",
                  &year, &month, &day, &hour, &min, &sec) == 6)) {
        year += 1900;
        if (month < 1 || month > 12 || day < 1 || day > 31) {
          return;
        }
        if (hour < 0 || hour > 23 || min < 0 || min > 59 || sec < 0 || sec > 59) {
          return;
        }
        DateTime etime(year, month, day, hour, min, 0);
        entryTime = etime.utime();
        break;
      }
    } else if ((sscanf(start, "%4d%2d%2d%1c%2d%2d%2d",
                       &year, &month, &day, &spacer, &hour, &min, &sec) == 7)) {
      // format - yyyymmdd?hhmmss
      if (year < 1900 || month < 1 || month > 12 || day < 1 || day > 31) {
        return;
      }
      if (hour < 0 || hour > 23 || min < 0 || min > 59 || sec < 0 || sec > 59) {
        return;
      }
      DateTime etime(year, month, day, hour, min, sec);
      entryTime = etime.utime();
      break;
    } else if (sscanf(start, "%2d%2d%2d", &hour, &min, &sec) == 3) {
      // normal format - yyyymmdd/hhmmss
      if (hour < 0 || hour > 23 || min < 0 || min > 59 || sec < 0 || sec > 59) {
        return;
      }
      DateTime etime(midday);
      etime.setTime(hour, min, sec);
      entryTime = etime.utime();
      break;
    }
    start++;
  }

  if (entryTime == 0) {
    return;
  }
	
  // check that the time is within range
  
  if (checkTimeRange) {
    if (entryTime < startTime || entryTime > endTime) {
      return;
    }
  }
  
  // check that the file is a valid candidate
  
  Path fpath(dayDir, entryName);

  if (!_validFile(fpath.getPath())) {
    return;
  }
  
  // insert the file
  
  string pathStr(fpath.getPath());
  TimePath tpath(entryTime, 0, pathStr);
  timePaths.insert(timePaths.end(), tpath);

}

///////////////////////////////////////////////////
// add valid times from generate subdirectory
// format is g_hhmmss/f_llllllll

void MdvxTimeList::_addValidFromGenSubdir(const string &dayDir,
					  const DateTime &midday,
					  const string &genSubDir,
					  bool checkTimeRange,
					  time_t startTime,
					  time_t endTime,
					  TimePathSet &timePaths)
  
{
  
  
  // exclude entry names which are too short
  
  if (genSubDir.size() < 8) {
    return;
  }
  
  // get time
  
  int hour, min, sec;
  if (sscanf(genSubDir.c_str(), "g_%2d%2d%2d", &hour, &min, &sec) != 3) {
    return;
  }
  if (hour < 0 || hour > 23 || min < 0 || min > 59 || sec < 0 || sec > 59) {
    return;
  }
  
  DateTime gtime(midday);
  gtime.setTime(hour, min, sec);
  time_t genTime = gtime.utime();
  
  // add forecast files from this subdirectory
  
  Path subDir(dayDir, genSubDir);
  _addForecast(subDir.getPath(), genTime, checkTimeRange,
	       startTime, endTime, timePaths);
  
}

///////////////////////////////////////////////////
// compile for GEN or GEN_PLUS_FCASTS

void MdvxTimeList::_addGen(const string &dayDir,
			   const DateTime &midday,
			   const string &subDirName,
			   bool checkTimeRange,
			   time_t startTime,
			   time_t endTime,
			   TimePathSet &timePaths)
  
{
  
  // exclude entries which are too short
  
  if ((subDirName.size() < 8) || (subDirName[0] == '.')) {
    return;
  }
  
  // check that subdir name is in the correct format
  // format is g_hhmmss
  
  int hour, min, sec;
  if (sscanf(subDirName.c_str(), "g_%2d%2d%2d", &hour, &min, &sec) != 3) {
    return;
  }
  if (hour < 0 || hour > 23 || min < 0 || min > 59 || sec < 0 || sec > 59) {
    return;
  }

  DateTime gtime(midday);
  gtime.setTime(hour, min, sec);
  time_t genTime = gtime.utime();
  
  // check that the time is within range
  
  if (checkTimeRange) {
    if (genTime < startTime || genTime > endTime) {
      return;
    }
  }
  
  // check that there are valid files in this directory
  
  TimePathSet tmpSet;
  Path subDir(dayDir, subDirName);
  _addForecast(subDir.getPath(), genTime, false, 0, 0, tmpSet);
  if (tmpSet.size() == 0) {
    return;
  }

  // add this directory to the list of gen times

  string pathStr(subDir.getPath());
  TimePath tpath(genTime, genTime, pathStr);
  timePaths.insert(timePaths.end(), tpath);
  
}

///////////////////////////////////////////////
// add forecast times

void MdvxTimeList::_addForecast(const string &dir,
				time_t genTime,
				bool checkTimeRange,
				time_t startTime,
				time_t endTime,
				TimePathSet &timePaths)
  
{

  ReadDir rdir;
  if (rdir.open(dir.c_str())) {
    return;
  }
    
  // Loop through directory looking for the data file names
  
  struct dirent *dp;
  for (dp = rdir.read(); dp != NULL; dp = rdir.read()) {

    // exclude files beginning with '.', or are too short
    
    if ((strlen(dp->d_name) < 10) || (dp->d_name[0] == '.')) {
      continue;
    }

    // Check for
    //   normal format "f_xxxxxxxx"
    //   or extended forecast format "yyyymmdd_g_hhmmss_f_xxxxxxxx"
    //   or valid format "yyyymmdd_hhmmss"

    time_t validTime = 0;
    int year, month, day, hour, min, sec;
    int leadTime = 0;
    char spacer;
    
    char *start = dp->d_name;
    const char *end = start + strlen(start);
    bool found = false;

    while (start < end - 8) {

      if (sscanf(start, "f_%8d", &leadTime) == 1) {
        // normal format "f_xxxxxxxx"
        validTime = genTime + leadTime;
        found = true;
        break;
      }
      
      if (sscanf(start, "%4d%2d%2d_g_%2d%2d%2d_f_%8d",
                 &year, &month, &day, &hour, &min, &sec, &leadTime) == 7) {
        // extended forecast format "yyyymmdd_g_hhmmss_f_xxxxxxxx"
        DateTime gtime(year, month, day, hour, min, sec);
        validTime = gtime.utime() + leadTime;
        found = true;
        break;
      }
      
      if (sscanf(start, "%4d%2d%2d%1c%2d%2d%2d",
                 &year, &month, &day, &spacer, &hour, &min, &sec) == 7) {
        // valid format "yyyymmdd_hhmmss"
        DateTime vtime(year, month, day, hour, min, sec);
        validTime = vtime.utime();
        leadTime = validTime - genTime;
        found = true;
        break;
      }

      start++;

    } // while
    
    if (!found) continue;

    // check the lead time
    
    if (_constrainFcastLeadTimes) {
      if (leadTime < _minFcastLeadTime || leadTime > _maxFcastLeadTime) {
        continue;
      }
    }
    
    // check time range
    
    if (checkTimeRange) {
      if (validTime < startTime || validTime > endTime) {
	continue;
      }
    }
    
    // reject files which are not valid
    
    Path fpath(dir, dp->d_name);
    if (!_validFile(fpath.getPath())) {
      continue;
    }
    
    // all checks pass, so add to set

    string pathStr(fpath.getPath());
    TimePath tpath(validTime, genTime, pathStr);
    timePaths.insert(timePaths.end(), tpath);

  } // dp
  
  rdir.close();
  
}

/////////////////////////////////////
// add first file to set

void MdvxTimeList::_addFirst(const string &topDir,
			     TimePathSet &timePaths)
  
{
  
  // get day dirs in time order
  
  TimePathSet dayDirs;
  _getDayDirs(topDir, dayDirs);
  
  // search for valid times in each day
  // we stop as soon as we find a valid time
  
  TimePathSet::iterator ii;
  for (ii = dayDirs.begin(); ii != dayDirs.end(); ii++) {
    
    DateTime midday(ii->validTime);
    const string &dayDir = ii->path;

    // look for non-forecast files
    
    TimePathSet tmpSet;
    _searchDayForValid(dayDir, midday, false, 0, 0, tmpSet);
    
    if (tmpSet.size() > 0) {
      // valid times found, insert the first one into the set
      // and return
      timePaths.insert(timePaths.end(), *(tmpSet.begin()));
      return;
    }
    
  } // ii

}

/////////////////////////////////////
// add last file to set

void MdvxTimeList::_addLast(const string &topDir,
			    TimePathSet &timePaths)
  
{

  // try _latest_data_info file
  
  LdataInfo ldata(topDir);
  if (ldata.read(-1) == 0) {
    Path lpath(topDir, ldata.getRelDataPath());
    // check that this file is less that 1 day old
    struct stat fstat;
    if (ta_stat(lpath.getPath().c_str(), &fstat) == 0) {
      time_t now = time(NULL);
      int age = now - fstat.st_mtime;
      if (age < SECS_IN_DAY) {
	TimePath tpath(ldata.getLatestTime() + ldata.getLeadTime(),
		       ldata.getLatestTime(),
		       lpath.getPath());
	timePaths.insert(timePaths.end(), tpath);
	return;
      }
    }
  }
  
  // get day dirs in time order

  TimePathSet dayDirs;
  _getDayDirs(topDir, dayDirs);
  
  // search each day in reverse order
  // stop as soon as we find a valid file

  TimePathSet::reverse_iterator ii;
  for (ii = dayDirs.rbegin(); ii != dayDirs.rend(); ii++) {
    
    DateTime midday(ii->validTime);
    const string &dayDir = ii->path;

    TimePathSet tmpSet;
    _searchDayForValid(dayDir, midday, false, 0, 0, tmpSet);
    
    if (tmpSet.size() > 0) {
      // valid times found, insert the last one into the set
      // and return
      timePaths.insert(timePaths.end(), *(tmpSet.rbegin()));
      return;
    }

  } // ii

}

///////////////////////////////////////////////////
// add best valid time

void MdvxTimeList::_addBestValid(const string &topDir,
				 time_t searchTime,
				 time_t startTime,
				 time_t endTime,
				 TimePathSet &timePaths)

{

  if (_hasForecasts) {
    return;
  }

  // search for all valid times within the search range
  
  TimePathSet valid;
  _searchForValid(topDir, startTime, endTime, valid);
  if (valid.size() < 1) {
    return;
  }
  
  // find the result closest to the search time
  
  TimePathSet::iterator best = valid.begin();
  double minDiff = 1.0e9;
  TimePathSet::iterator ii;
  for (ii = valid.begin(); ii != valid.end(); ii++) {
    double diff = fabs((double) searchTime - ii->validTime);
    if (diff <= minDiff) {
      minDiff = diff;
      best = ii;
    }
  } // ii
  
  timePaths.insert(timePaths.begin(), *best);

}

///////////////////////////////////////////////////
// add best forecast

void MdvxTimeList::_addBestForecast(const string &topDir,
				    time_t searchTime,
				    time_t startTime,
				    time_t endTime,
				    TimePathSet &timePaths)

{

  if (!_hasForecasts) {
    return;
  }

  // search for all valid times within the search range
  
  TimePathSet valid;
  _searchForValid(topDir, startTime, endTime, valid);
  if (valid.size() < 1) {
    return;
  }
  
  // Find the result closest to the search time and gen time.
  // We compute the 'distance' on two time axes from the actual data.
  // We weight the valid time difference more than the gen time diff
  //   vDiff = diff between search time and valid time
  //   gDiff = diff between search time and gen time
  //   vDiffWt = vDiff * 2.5, or the value to which it is set

  time_t refValidTime = searchTime;
  time_t refGenTime = searchTime;
  if (_constrainFcastLeadTimes) {
    int meanLead = (_minFcastLeadTime + _maxFcastLeadTime) / 2;
    refGenTime -= meanLead;
  }

  time_t latestGenTime = 0;
  TimePathSet::iterator closest = valid.begin();
  {

    double minDiffsq = 1.0e99;
    TimePathSet::iterator ii;

    for (ii = valid.begin(); ii != valid.end(); ii++) {

      if (ii->genTime > searchTime) {
	// ignore forecasts from the future
	continue;
      }

      double vDiff = (double) refValidTime - (double) ii->validTime;
      double wt = 2.5;
      if (_validTimeSearchWt > 0) {
        wt = _validTimeSearchWt;
      }

      double vDiffWeighted = vDiff * wt;
      double gDiff = (double) refGenTime - (double) ii->genTime;
      double diffsq = (vDiffWeighted * vDiffWeighted + gDiff * gDiff);
      
      if (diffsq < minDiffsq) {
	minDiffsq = diffsq;
	latestGenTime = ii->genTime;
	closest = ii;
      } else if (diffsq == minDiffsq) {
	if (ii->genTime > latestGenTime) {
	  latestGenTime = ii->genTime;
	  closest = ii;
	}
      }

    } // ii

  }
  
  timePaths.insert(timePaths.begin(), *closest);

}

/////////////////////////////////////
// get day dirs for top dir

void MdvxTimeList::_getDayDirs(const string &topDir,
			       TimePathSet &dayDirs)
  
{
  
  // set up dirs in time order
  
  ReadDir rdir;
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
        yyyyDir += PATH_DELIM;
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
    
    DateTime midday(year, month, day, 12, 0, 0);
    Path dayDirPath(topDir, dp->d_name);
    string pathStr(dayDirPath.getPath());
    TimePath tpath(midday.utime(), 0, pathStr);
    dayDirs.insert(dayDirs.end(), tpath);

  } // for (dp ...
  
  rdir.close();
    
}

////////////////////////////////////////////
// compile the array of forecast times lists

int MdvxTimeList::_addForecastTimesArray(const string &dir,
					 vector<time_t> &genTimes,
					 vector<vector<time_t> > &ftarray)
  
{

  int iret = 0;
  
  // clear forecast times array

  ftarray.clear();
  
  // go through the gen times, requesting the forecast time list
  // and adding it to the array
  // Use a temporary object to gather the time lists
  
  MdvxTimeList tmp;

  for (size_t ii = 0; ii < genTimes.size(); ii++) {

    // set up vector for forecast times
    
    vector<time_t> forecastTimes;
    
    // request forecast times for the given generate time
    
    tmp.setModeForecast(dir, genTimes[ii]);
    if (_checkLatestValidModTime) {
      tmp.setCheckLatestValidModTime(_latestValidModTime);
    }
    if (tmp.compile() == 0) {
      const vector<time_t> ftimes = tmp.getValidTimes();
      for (size_t jj = 0; jj < ftimes.size(); jj++) {
	forecastTimes.push_back(ftimes[jj]);
      }
    } else {
      iret = -1;
    }
    
    ftarray.push_back(forecastTimes);
    
  } // ii

  return iret;

}

///////////////////////////////////////////
// check if this is a valid file to include

bool MdvxTimeList::_validFile(const string &path)
  
{

  Path P(path);
  string filename = P.getFile();
  bool isRadxFile = Mdvx::isRadxFile(path);
  
  if (filename.find(".mdv") == string::npos &&
      filename.find(".nc") == string::npos && // netCDF
      !isRadxFile) {
    return false;
  }
  if (filename.find("swp") != string::npos &&
      (filename.find("IDL") != string::npos ||
       filename.find("RHI") != string::npos)) {
    // dorade idle or rhi volumes
    // cerr << "TODO - NEEDS WORK - Mike Dixon" << endl;
    return false;
  }

  // file extension OK? Ignore .buf files

  if (path.find(".mdv.buf") != string::npos) {
    return false;
  }

  // Get the file status since this will be used to perform
  // some other tests

  struct stat fstat;
  if (ta_stat(path.c_str(), &fstat)) {
    return false;
  }

  // Check the file size.  If the file doesn't contain a master header
  // then we don't want to return it (files without any field
  // headers - n_fields == 0 - are unusual but possible). Only
  // apply this test to uncompressed files since compression
  // may decrease the file size.
  // If a file is compressed, only test that its size is non-zero.

  if (strcmp(path.c_str() + strlen(path.c_str()) -4, ".mdv" )) {
    // Filename does not end in .mdv, so possibly compressed,
    // only test for non-zero size
    if (fstat.st_size == 0) return false;
  } else {
    // Filename ends in ".mdv", uncompressed,
    // test size against master header
    if (fstat.st_size <
        (int)(sizeof(Mdvx::master_header_t))) {
      return false;
    }
  }
  
  if (_checkLatestValidModTime) {

    // does the file exist?

    if (!S_ISREG(fstat.st_mode)) {
      return false;
    }

    // check mod time

    if (fstat.st_mtime > _latestValidModTime) {
      return false;
    }

  } // if (_checkLatestValidModTime)

  return true;

}

///////////////////////////////////////////
// get time for Dorade file path
// Returns 0 on success, -1 on failure

int MdvxTimeList::getDoradeTime(const string &path, DateTime &doradeTime)
  
{

  // find "swp." string

  const char *sweepStr = strstr(path.c_str(), "swp.");
  if (sweepStr == NULL) {
    return -1;
  }
  const char *start = sweepStr + 4;

  // Dorade sweep file format:
  //   swp.yymmddhhmmss.... (1900 - 1999) or  swp.yyymmddhhmmss... (2000+)
  
  int year, month, day, hour, min, sec;
  if (start[0] == '1' || start[0] == '2') {
    // year 2000+
    if (sscanf(start, "%3d%2d%2d%2d%2d%2d",
               &year, &month, &day, &hour, &min, &sec) != 6) {
      return -1;
    }
  } else {
    // year 1900 - 1999
    if (sscanf(start, "%2d%2d%2d%2d%2d%2d",
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
    
  doradeTime.set(year, month, day, hour, min, sec);

  return 0;

}

///////////////////////////////////////////////////////////
// for sweep files, provide times for end of unique volumes

void MdvxTimeList::_makeSweepVolumesUnique(TimePathSet &timePaths)
  
{

  // more than 1 path?

  if (timePaths.size() < 2) {
    return;
  }

  // do we have sweep files? If not, return now.
  
  const TimePath &first = *timePaths.begin();
  Path fpath(first.path);
  if (fpath.getFile().find("swp") == string::npos) {
    // file name does not contain 'swp'
    return;
  }

  // build a set with only the last entry per volume

  TimePathSet unique;
  for (TimePathSet::iterator ii = timePaths.begin();
       ii != timePaths.end(); ii++) {

    // find vol number
    
    Path path(ii->path);
    int volNum = _getVolNum(path.getFile());
    if (volNum == 0) continue;

    // find last entry with the same volume number
    
    TimePath last = *ii;
    TimePathSet::iterator lpos = ii++;
    for (TimePathSet::iterator jj = ii; jj != timePaths.end(); jj++, lpos++) {
      Path upath(jj->path);
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

int MdvxTimeList::_getVolNum(const string &fileName)
  
{

  size_t vpos = fileName.find("_v");
  string vstr = fileName.substr(vpos);
  int volNum = 0;
  if (sscanf(vstr.c_str(), "_v%d", &volNum) != 1) {
    return 0;
  }
  return volNum;
}

