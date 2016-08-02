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
////////////////////////////////////////////////////////////////////
// Mdv/MdvxTimeList.hh
//
// Handles the time list functionality for Mdvx.
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2005
//
////////////////////////////////////////////////////////////////////

#ifndef MdvxTimeList_hh
#define MdvxTimeList_hh

#include <string>
#include <vector>
#include <set>
#include <toolsa/DateTime.hh>
using namespace std;

class MdvxTimeList
{

public:

  // Specifying mode for time lists
  //
  // MODE_VALID: valid data times in a given time range.
  //   For model data, if more than 1 gen time produces data
  //   for the same valid time, only a single time is returned.
  // MODE_GENERATE: model generate times within a given time range.
  // MODE_FORECAST: model foreast times for a given gen time.
  // MODE_FIRST: the first time in the data set
  // MODE_LAST: the last time in the data set
  // MODE_GEN_PLUS_FCASTS: gen times within a given time range, plus
  //   all forecast times for each gen time.
  // MODE_VALID_MULT_GEN: valid times in a given data range.
  //   Often different gen times produce forecasts at the same valid time,
  //   i.e., the results overlap. In this case, there will be multiple
  //   instances of the same valid time, each with a different gen time.
  // MODE_CLOSEST: get closest data time to search time,
  //   within search_margin
  // MODE_FIRST_BEFORE: get first time at or before search_time,
  //   within search_margin
  // MODE_FIRST_AFTER: get first time at or after search_time,
  //   within search_margin
  // MODE_BEST_FORECAST: get best forecast time closest to search time,
  //   withing search_margin.
  // MODE_SPECIFIED_FCAST: get forecast generated at search time,
  //   closest to forecast time, within search_margin

  typedef enum {
    
    MODE_UNDEFINED          = -1,
    MODE_VALID              = 0,
    MODE_GENERATE           = 1,
    MODE_FORECAST           = 2,
    MODE_FIRST              = 3,
    MODE_LAST               = 4,
    MODE_GEN_PLUS_FCASTS    = 5,
    MODE_VALID_MULT_GEN     = 6,
    MODE_CLOSEST            = 7,
    MODE_FIRST_BEFORE       = 8,
    MODE_FIRST_AFTER        = 9,
    MODE_BEST_FCAST         = 10,
    MODE_SPECIFIED_FCAST    = 11
  
  } time_list_mode_t;
  
  //////////////
  // constructor
  
  MdvxTimeList();
  
  //////////////
  // destructor
  
  virtual ~MdvxTimeList();
  
  /////////////////////////////////////////////////////////////////
  // Before calling compileTimeList(), you must first clear the
  // time list mode and then set the mode with one of the
  // set functions.
  //
  // For data sets which could switch on the domain,
  // you must also set the domain using setReadHorizLimits().
  
  /////////////////////////////////////////////////////////////////
  // clearMode
  //
  // clear out time list mode info - do this first
  
  void clearMode();
  
  /////////////////////////////////////////////////////////////////
  // clear all data
  
  void clearData();

  /////////////////////////////////////////////////////////////////
  // setModeValid
  //
  // Set the time list so that it finds all of the valid data
  // times between the start and end times.
  // For forecast data where multiple forecasts exist for the same
  // valid time, a single valid time will be returned.
  
  void setModeValid(const string &dir,
		    time_t start_time,
		    time_t end_time);
  
  /////////////////////////////////////////////////////////////////
  // setModeGen
  //
  // Set the time list so that it finds all of the
  // generate times between the start and end times
  
  void setModeGen(const string &dir,
		  time_t start_gen_time,
		  time_t end_gen_time);
  
  /////////////////////////////////////////////////////////////////
  // setModeForecast
  //
  // Set the time list so that it returns all of the forecast
  // times for the given generate time.
  
  void setModeForecast(const string &dir,
		       time_t gen_time);
  
  /////////////////////////////////////////////////////////////////
  // setModeGenPlusForecasts
  //
  // Set the time list so that it finds all of the
  // generate times between the start and end gen times.
  // Then, for each generate time, all of the forecast times are
  // found. These are made available in the
  // _forecastTimesArray, which is represented by vector<vector<time_t> >
  
  void setModeGenPlusForecasts(const string &dir,
			       time_t start_gen_time,
			       time_t end_gen_time);
  
  /////////////////////////////////////////////////////////////////
  // setModeValidMultGen
  //
  // Set the time list so that it finds all of the forecasts
  // within the time interval specified. For each forecast found
  // the associated generate time is also determined.
  // The forecast times will be available in the _validTimes array.
  // The generate times will be available in the _genTimes array.
  
  void setModeValidMultGen(const string &dir,
			   time_t start_time,
			   time_t end_time);
  
  /////////////////////////////////////////////////////////////////
  // setModeFirst
  //
  // Set the time list mode so that it finds the first available data time
  
  void setModeFirst(const string &dir);
  
  /////////////////////////////////////////////////////////////////
  // SetModeLast
  //
  // set the time list mode so that it finds the last available data time
  
  void setModeLast(const string &dir);
  
  /////////////////////////////////////////////////////////////////
  // SetModeClosest
  //
  // set the time list mode so that it finds the closest available data time
  // to the search time within the search margin
  
  void setModeClosest(const string &dir,
		      time_t search_time, int time_margin);
  
  /////////////////////////////////////////////////////////////////
  // SetModeFirstBefore
  //
  // set the time list mode so that it finds the first available data time
  // before the search time within the search margin
  
  void setModeFirstBefore(const string &dir,
			  time_t search_time, int time_margin);
  
  /////////////////////////////////////////////////////////////////
  // SetModeFirstAfter
  //
  // set the time list mode so that it finds the first available data time
  // after the search time within the search margin
  
  void setModeFirstAfter(const string &dir,
			 time_t search_time, int time_margin);
  
  /////////////////////////////////////////////////////////////////
  // setModeBestForecast
  //
  // Set the time list so that it returns the best forecast
  // for the search time, within the time margin
  
  void setModeBestForecast(const string &dir,
			   time_t search_time, int time_margin);
  
  /////////////////////////////////////////////////////////////////
  // setModeSpecifiedForecast
  //
  // Set the time list so that it returns the forecast for the given
  // generate time, closest to the search time, within the time margin
  
  void setModeSpecifiedForecast(const string &dir, time_t gen_time,
				time_t search_time, int time_margin);
  
  /////////////////////////////////////////////////
  // Set check for latest valid file mod time.
  // Option to check file modify times and only use
  // files with mod times before a given time.
  
  void setCheckLatestValidModTime(time_t latest_valid_mod_time) {
    _checkLatestValidModTime = true;
    _latestValidModTime = latest_valid_mod_time;
  }
    
  void clearCheckLatestValidModTime() {
    _checkLatestValidModTime = false;
    _latestValidModTime = 0;
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
  
  void setConstrainFcastLeadTimes(int min_lead_time,
				  int max_lead_time,
				  bool request_by_gen_time = false);
  
  void clearConstrainFcastLeadTimes();
  
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
  
  void setValidTimeSearchWt(double wt);
  void clearValidTimeSearchWt();
  double getValidTimeSearchWt() const;

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
  
  int compile();
  
  // clear error string
  
  void clearErrStr() const;

  // get functions
  
  const string &getDir() const { return _dir;}
  time_list_mode_t getMode() const { return _mode;}
  time_t getStartTime() const { return _startTime; }
  time_t getEndTime() const { return _endTime; }
  time_t getGenTime() const { return _genTime; }
  time_t getSearchTime() const { return _searchTime; }
  int getTimeMargin() const { return _timeMargin; }
  bool checkLatestValidModTime() const { return _checkLatestValidModTime; }
  time_t getLatestValidModTime() const { return _latestValidModTime; }

  bool getConstrainFcastLeadTimes() const { return _constrainFcastLeadTimes; }
  int getMinFcastLeadTime() const { return _minFcastLeadTime; }
  int getMaxFcastLeadTime() const { return _maxFcastLeadTime; }
  bool getSpecifyFcastByGenTime() const { return _specifyFcastByGenTime; }

  const vector<time_t> &getValidTimes() const { return _validTimes; }
  const vector<time_t> &getGenTimes() const { return _genTimes; }
  const vector<string> &getPathList() const { return _pathList; }
  bool hasForecasts() const { return _hasForecasts; }
  const vector<vector<time_t> > &getForecastTimesArray() const {
    return _forecastTimesArray;
  }
  string getErrStr() const { return _errStr; }
  
  //////////////////////////
  // print time list request
  
  void printRequest(ostream &out);

  ///////////////////////////////////////////////////////////////
  // Set functions for Mdv-related classes
  // WARNING - do not use these without fully understanding the
  // classes and side effects.

  void setDir(const string &dir) { _dir = dir; }
  void setHasForecasts(bool state) { _hasForecasts = state; }
  void addValidTime(time_t time) { _validTimes.push_back(time); }
  void addGenTime(time_t time) { _genTimes.push_back(time); }
  void copyValidTimesToGenTimes() { _genTimes = _validTimes; }
  void addForecastTimes(const vector<time_t> forecastTimes) {
    _forecastTimesArray.push_back(forecastTimes);
  }
  
  // Get time for Dorade file path.
  // Sets doradeTime object.
  // Returns 0 on success, -1 on failure
  
  static int getDoradeTime(const string &path, DateTime &doradeTime);
  
protected:
private:

  // STL type definitions

  class TimePath {
  public:
    time_t validTime;
    time_t genTime;
    string path;
    TimePath(time_t v, time_t g, const string &p) :
      validTime(v), genTime(g), path(p) {}
  };
  
  class TimePathCompare {
  public:
    bool operator()(const TimePath &a, const TimePath &b) const {
      if (a.validTime != b.validTime) {
	return a.validTime < b.validTime;
      } else {
	return a.genTime < b.genTime;
      }
    }
  };

  typedef set<TimePath, TimePathCompare > TimePathSet;

  // members

  mutable string _errStr;
  time_list_mode_t _mode;
  string _dir;
  time_t _startTime;
  time_t _endTime;
  time_t _genTime;
  time_t _searchTime;
  int _timeMargin;

  bool _checkLatestValidModTime;

  time_t _latestValidModTime;

  bool _constrainFcastLeadTimes;
  int _minFcastLeadTime;
  int _maxFcastLeadTime;
  bool _specifyFcastByGenTime;

  bool _hasForecasts;

  double _validTimeSearchWt;

  vector<time_t> _validTimes;
  vector<time_t> _genTimes;
  vector<string> _pathList;
  vector<vector<time_t> > _forecastTimesArray;
  
  // private funtions

  void _checkHasForecasts(const string &topDir);
  void _compileValid(const string &topDir);
  void _compileGenerate(const string &topDir);
  void _compileForecast(const string &topDir);
  void _compileForecastForSubDir(const string &subDir);

  void _compileFirst(const string &topDir);
  void _compileLast(const string &topDir);
  void _compileGenPlusFcasts(const string &topDir);
  void _compileValidMultGen(const string &topDir);
  void _compileClosest(const string &topDir);
  void _compileFirstBefore(const string &topDir);
  void _compileFirstAfter(const string &topDir);
  void _compileBestForecast(const string &topDir);
  void _compileSpecifiedForecast(const string &topDir);
  void _compileSpecForecastForSubDir(const string &subDir);
  
  void _searchForValid(const string &topDir,
		       time_t startTime,
		       time_t endTime,
		       TimePathSet &timePaths);
  
  void _searchDayRangeForValid(const string &dir,
			       int startDay,
			       int endDay,
			       time_t startTime,
			       time_t endTime,
			       TimePathSet &timePaths);
  
  void _searchDayForValid(const string &dayDir,
			  const DateTime &midday,
			  bool checkTimeRange,
			  time_t startTime,
			  time_t endTime,
			  TimePathSet &timePaths);
  
  void _searchDayGen(const string &dayDir,
		     const DateTime &midday,
		     bool checkTimeRange,
		     time_t startTime,
		     time_t endTime,
		     TimePathSet &timePaths);
  
  void _addValid(const string &dir,
		 const DateTime &midday,
		 const string &fileName,
		 bool checkTimeRange,
		 time_t startTime,
		 time_t endTime,
		 TimePathSet &timePaths);
  
  void _addValidFromGenSubdir(const string &dayDir,
			      const DateTime &midday,
			      const string &genSubDir,
			      bool checkTimeRange,
			      time_t startTime,
			      time_t endTime,
			      TimePathSet &timePaths);
  
  void _addGen(const string &dir,
	       const DateTime &midday,
	       const string &fileName,
	       bool checkTimeRange,
	       time_t startTime,
	       time_t endTime,
	       TimePathSet &timePaths);
  
  void _addForecast(const string &dir,
		    time_t genTime,
		    bool checkTimeRange,
		    time_t startTime,
		    time_t endTime,
		    TimePathSet &timePaths);
  
  void _addFirst(const string &dir,
		 TimePathSet &timePaths);
  
  void _addLast(const string &dir,
		TimePathSet &timePaths);
  
  void _addBestValid(const string &topDir,
		     time_t searchTime,
		     time_t startTime,
		     time_t endTime,
		     TimePathSet &timePaths);

  void _addBestForecast(const string &topDir,
			time_t searchTime,
			time_t startTime,
			time_t endTime,
			TimePathSet &timePaths);

  void _getDayDirs(const string &topDir,
		   TimePathSet &dayDirs);
  
  int _addForecastTimesArray(const string &dir,
			     vector<time_t> &genTimes,
			     vector<vector<time_t> > &ftarray);
  
  bool _validFile(const string &path);
  
  void _makeSweepVolumesUnique(TimePathSet &timePaths);
  int _getVolNum(const string &fileName);

};

#endif

    
