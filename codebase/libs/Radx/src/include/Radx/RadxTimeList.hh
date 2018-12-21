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
// Mdv/RadxTimeList.hh
//
// Handles the time list functionality for Radx.
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2010
//
////////////////////////////////////////////////////////////////////

#ifndef RadxTimeList_hh
#define RadxTimeList_hh

#include <string>
#include <vector>
#include <set>
#include <Radx/RadxTime.hh>
using namespace std;

class RadxTimeList
{

public:

  // Specifying mode for time lists
  //
  // MODE_INTERVAL: interval data times in a given time range.
  // MODE_FIRST: the first time in the data set
  // MODE_LAST: the last time in the data set
  // MODE_CLOSEST: get closest data time to search time,
  //   within search_margin
  // MODE_FIRST_BEFORE: get first time at or before search_time,
  //   within search_margin
  // MODE_FIRST_AFTER: get first time at or after search_time,
  //   within search_margin

  typedef enum {
    
    MODE_UNDEFINED          = -1,
    MODE_INTERVAL           = 0,
    MODE_FIRST              = 1,
    MODE_LAST               = 2,
    MODE_CLOSEST            = 3,
    MODE_FIRST_BEFORE       = 4,
    MODE_FIRST_AFTER        = 5
  
  } time_list_mode_t;
  
  //////////////
  // constructor
  
  RadxTimeList();
  
  //////////////
  // destructor
  
  virtual ~RadxTimeList();
  
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
  // setModeInterval
  //
  // Set the time list so that it finds all of the interval data
  // times between the start and end times.
  
  void setModeInterval(RadxTime start_time,
                       RadxTime end_time);
  
  /////////////////////////////////////////////////////////////////
  // setModeFirst
  //
  // Set the time list mode so that it finds the first available data time
  
  void setModeFirst();
  
  /////////////////////////////////////////////////////////////////
  // SetModeLast
  //
  // set the time list mode so that it finds the last available data time
  
  void setModeLast();
  
  /////////////////////////////////////////////////////////////////
  // SetModeClosest
  //
  // set the time list mode so that it finds the closest available data time
  // to the search time within the search margin
  
  void setModeClosest(RadxTime search_time, double time_margin);
  
  /////////////////////////////////////////////////////////////////
  // SetModeFirstBefore
  //
  // set the time list mode so that it finds the first available data time
  // before the search time within the search margin
  
  void setModeFirstBefore(RadxTime search_time, double time_margin);
  
  /////////////////////////////////////////////////////////////////
  // SetModeFirstAfter
  //
  // set the time list mode so that it finds the first available data time
  // after the search time within the search margin
  
  void setModeFirstAfter(RadxTime search_time, double time_margin);
  
  /// Set aggregation into volume on read.
  /// If true, individual sweep files will be aggregated into a
  /// volume when the files are read
  
  void setReadAggregateSweeps(bool val);

  /////////////////////////////////////////////////
  // Set check for latest valid file mod time.
  // Option to check file modify times and only use
  // files with mod times before a given time.
  
  void setCheckLatestValidModTime(bool doCheck,
                                  RadxTime latest_valid_mod_time) {
    _checkLatestValidModTime = doCheck;
    _latestValidModTime = latest_valid_mod_time;
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
  
  int compile();
  
  // clear error string
  
  void clearErrStr() const;

  // get functions
  
  const string &getDir() const { return _dir;}
  time_list_mode_t getMode() const { return _mode;}
  RadxTime getStartTime() const { return _startTime; }
  RadxTime getEndTime() const { return _endTime; }
  RadxTime getSearchTime() const { return _searchTime; }
  double getTimeMargin() const { return _timeMargin; }

  const vector<RadxTime> &getValidTimes() const { return _fileStartTimes; }
  const vector<string> &getPathList() const { return _pathList; }
  string getErrStr() const { return _errStr; }
  
  ///////////////////////////////
  // print/get time list request
  
  void printRequest(ostream &out) const;
  string getRequestString() const;

  ///////////////////////////////////////////////////////////////
  // Set functions
  // WARNING - do not use these without fully understanding the
  // classes and side effects.

  void setDir(const string &dir) { _dir = dir; }
  void addValidTime(RadxTime time) { _fileStartTimes.push_back(time); }

  // set the required file extension
  // if empty this is not used

  void setFileExt(const string &ext) { _fileExt = ext; }
  
  // Get time for Dorade file path.
  // Sets doradeTime object.
  // Returns 0 on success, -1 on failure
  
  static int getDoradeTime(const string &path, RadxTime &doradeTime);
  void getFirstAndLastTime(RadxTime &fileStartTime, RadxTime &fileEndTime);

protected:
private:

  // STL type definitions
  
  class TimePath {
  public:
    RadxTime fileStartTime;
    RadxTime fileEndTime;
    string path;
    TimePath(const RadxTime &start, const RadxTime &end, const string &p) :
            fileStartTime(start), fileEndTime(end), path(p) {}
  };
  
  class TimePathCompare {
  public:
    bool operator()(const TimePath &a, const TimePath &b) const {
      return a.fileStartTime < b.fileStartTime;
    }
  };

  typedef set<TimePath, TimePathCompare > TimePathSet;

  // members

  mutable string _errStr;
  time_list_mode_t _mode;
  string _dir;
  RadxTime _startTime;
  RadxTime _endTime;
  RadxTime _searchTime;
  double _timeMargin;
  string _fileExt;

  vector<RadxTime> _fileStartTimes;
  vector<string> _pathList;

  
  bool _readAggregateSweeps;

  bool _checkLatestValidModTime;
  RadxTime _latestValidModTime;

  // private funtions

  void _compileInterval(const string &topDir);
  void _compileFirst(const string &topDir);
  void _compileLast(const string &topDir);
  void _compileClosest(const string &topDir);
  void _compileFirstBefore(const string &topDir);
  void _compileFirstAfter(const string &topDir);
  
  void _searchForValid(const string &topDir,
                       RadxTime startTime,
                       RadxTime endTime,
                       TimePathSet &timePaths);
  
  void _searchDayRange(const string &dir,
                       int startDay,
                       int endDay,
                       RadxTime startTime,
                       RadxTime endTime,
                       TimePathSet &timePaths);
  
  void _searchDay(const string &dayDir,
                  const RadxTime &midday,
                  RadxTime startTime,
                  RadxTime endTime,
                  TimePathSet &timePaths);
  
  void _searchTopDir(const string &dir,
                     int startDay,
                     int endDay,
                     RadxTime startTime,
                     RadxTime endTime,
                     TimePathSet &timePaths);
  
  void _addValid(const string &dir,
                 const RadxTime &midday,
                 const string &fileName,
                 RadxTime startTime,
                 RadxTime endTime,
                 TimePathSet &timePaths);
  
  void _addFirst(const string &dir,
		 TimePathSet &timePaths);
  
  void _addLast(const string &dir,
		TimePathSet &timePaths);
  
  void _addClosest(const string &topDir,
                   RadxTime searchTime,
                   RadxTime startTime,
                   RadxTime endTime,
                   TimePathSet &timePaths);

  void _getDayDirs(const string &topDir,
		   TimePathSet &dayDirs);
  
  bool _isValidFile(const string &path);
  
  void _makeSweepVolumesUnique(TimePathSet &timePaths);
  int _getVolNum(const string &fileName);

};

#endif

    
