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
// DsDataFile.hh : Thread-safe location and identification of data files.
//
// Paddy McCarty, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// February 1999
//
//////////////////////////////////////////////////////////
//
// The DsDataFile class provides a set of functionality for finding
//   and representing data files used in the didss system.
// 
// Static methods on this class provide different search mechanisms for
//   a single desired file, such as getLatest() and getClosest(). Each of
//   these methods require that a fileType and file suffix be passed in
//   to the method call. The file type determines which kind of temporal
//   arrangement the data may be found in, such as data grouped in daily
//   files under the main data dir, or arranged in files under daily
//   directories. If the fileSuffix is empty, the files will be treated
//   as suffix-less.
// 
// The static methods return intantiations of the DsDataFile class. Objects
//   of this type refer to files which exist on the filesystem. Calling
//   getFileStr() on these objects will return an absolute path to the 
//   related file.
//   
// I did not create subclasses for each of the different FileTypes, as
//   I thought this would be too confusing for the benefit it would
//   provide. Instead, several methods have logic for switching based 
//   on the intended type. If subclasses are used later, the get() methods
//   could have factory functionality, and return an instantiation of
//   the appropriate type.
//   
///////////////////////////////////////////////////////////

#ifndef DsDataFileINCLUDED
#define DsDataFileINCLUDED

#include <toolsa/DateTime.hh>

#include <string>
#include <vector>
#include <set>
using namespace std;

class DsDataFile {

public:

  enum FileType {
    DS_UNKNOWN_FILE_TYPE = -1, // Unknown file type.
    DS_DAILY_FILE,    // One file per day -- all in the _dataDir/.
    DS_MULTI_FILE,    // Multiple files per day -- in dirs _dataDir/YYYMMDD/.
    DS_FORECAST_FILE  // Forecast data -- in dirs _dataDir/YYYMMDD/g_HHMMSS/.
  };

  enum SearchType {
    DS_SEARCH_CENTERED = 0,
    DS_SEARCH_BEFORE  = 1,
    DS_SEARCH_AFTER   = 2
  };

protected:
  // Static debug flags -- because they are used in static methods.
  static bool _isDebug;
  static bool _isVerbose;

public:

  DsDataFile();
  DsDataFile(const string & dataDir, const DateTime & fileTime,
             const string & fileSuffix, DsDataFile::FileType fileType);
  virtual ~DsDataFile();

  // Get the latest data file.
  //   If prevTime is supplied, only returns a file if it is newer than
  //   that time.
  //   
  //   Returns pointer to DsDataFile that must be freed, on success.
  //   Returns NULL if there is no such file.
  // 
  static DsDataFile * getLatest(const string & dataDir,
                                const string & fileSuffix = "",
                                const DateTime * prevTime = NULL);

  // getClosestToDateTime()
  //
  // Get the data file closest to the specified time, using the 
  //   supplied DataTime object and time margin.
  //   If the data being searched is forecast data,
  //   the search will only occur within the
  //   specified gen time for the data.
  // 
  //   Note that this method keys off the kind of DateTime object
  //   which is passed in -- it looks for forecast data if it is
  //   a forecast DateTime.
  // 
  //   Returns pointer to DsDataFile that must be freed, on success.
  //   Returns NULL if there is no such file.
  // 
  static DsDataFile * getClosestToDateTime(const string & dataDir,
					   const DateTime & fileTime,
					   size_t marginSecs,
					   SearchType searchType,
					   const string & fileSuffix = "");

  // getClosestToValid()
  //
  // Get the data file closest to the specified valid time, using the 
  //   supplied search type and margin. This method figures out which type of
  //   data it is dealing with by performing a directory search.
  //   It then does the appropriate search for a match on 
  //   the valid time.
  //
  //   If both non-forecast and forecast data exist in the directory, it
  //   searches first for suitable non-forecast data. If that fails
  //   the forecast data is searched.
  //
  //   For forecast data, the search is made for the valid time
  //   (gen_time + lead_time) closest to the search time. If the 
  //   best match yields multiple files at the same valid time, the
  //   file with the shortest lead time is selected.
  //   
  //   If you want to search based on generate time and lead time
  //   instead of valid time, use getBestForecast() instead.
  //
  //   Returns pointer to DsDataFile that must be freed, on success.
  //   Returns NULL if there is no such file.
  // 
  static DsDataFile * getClosestToValid(const string & dataDir,
					const time_t & targetTime,
					size_t marginSecs,
					SearchType searchType,
					const string & fileSuffix = "");
  
  // getForecastClosestToValid()
  //
  // Get the forecast file closest to the specified valid time, using the 
  //   supplied search type and margin.
  //
  //   The search is made for the valid time
  //   (gen_time + lead_time) closest to the search time. If the 
  //   best match yields multiple files at the same valid time, the
  //   file with the shortest lead time is selected.
  //   
  //   If you want to search based on generate time and lead time
  //   instead of valid time, use getClosestToDateTime() instead.
  //
  //   Returns pointer to DsDataFile that must be freed, on success.
  //   Returns NULL if there is no such file.
  // 
  // static 
  
  static DsDataFile *
  getForecastClosestToValid(const string & dataDir,
			    const time_t & targetTime,
			    const size_t & marginSecs,
			    const SearchType searchType,
			    const string & fileSuffix = "",
			    const DateTime * prevForecastTime = NULL);

  // Get the data file representing the best forecast for a time.
  //   The "best" forecast is that with the shortest lead time.
  //   If prevForecastTime is supplied, only returns a file if there
  //   is a "better" forecast than the one supplied.
  // 
  //   See additional comments in the implementation file.
  // 
  //   Returns pointer to DsDataFile that must be freed, on success.
  //   Returns NULL if there is no such file.
  // 
  static DsDataFile *
  getBestForecast(const string & dataDir,
		  const time_t & targetTime,
		  const size_t & maxLeadSecs,
		  const SearchType searchType,
		  const string & fileSuffix = "",
		  const DateTime * prevForecastTime = NULL);
  
  // Get the positive time difference between two files, in seconds.
  //   If the times are forecast times, just compares the lead times.
  //                                                                       
  //   Returns DS_UINT_MAX (0xffffffff) if the comparison is not allowed,
  //               i.e. the times are different types, or both are forecast
  //               times but have different gen times.
  //                                                                  
  static size_t getTimeDiff(const DateTime & t1, const DateTime & t2);

  // Get all data files in the specified time range.
  // 
  static vector<DsDataFile * > getRange(const string & dataDir,
                                        const DateTime & startSearch,
                                        const DateTime & endSearch,
                                        const string & fileSuffix = "",
                                        FileType fileType = DS_MULTI_FILE);

  static const unsigned int DS_UINT_MAX = 0xffffffff;

  // get the max forecast time interval in days
  // This affects the search efficiency.
  
  static int getMaxForecastLeadDays();

  static void setDebug(bool deb)   { _isDebug = deb; }
  static void setVerbose(bool ver) { _isVerbose = ver;
                                     if (ver) setDebug(ver); }

  bool     exists() const;
  string   getFileStr() const;
  DateTime getFileTime() const   { return _fileTime; }
  FileType getFileType() const   { return _fileType; }
  string   getFileSuffix() const { return _fileSuffix; }

  inline DsDataFile & operator= (const DsDataFile & orig);

protected:

  // data

  string   _dataDir;
  DateTime _fileTime;
  FileType _fileType;
  string   _fileSuffix;

  static int _maxForecastLeadDays;
  static const int _maxForecastLeadDaysDefault = 10;

  // prototypes

  static DsDataFile::FileType _get_type_for_suffix(const string & suffix);

  static int _get_range_daily(const string & dataDir,
			      const DateTime & startSearch,
			      const DateTime & endSearch,
			      const string & fileSuffix,
			      FileType fileType,
			      vector<DsDataFile *> &list);

  static int _get_range_multi(const string & dataDir,
			      const DateTime & startSearch,
			      const DateTime & endSearch,
			      const string & fileSuffix,
			      FileType fileType,
			      vector<DsDataFile *> &list);
  
  static int _get_range_fcast(const string & dataDir,
			      const DateTime & startSearch,
			      const DateTime & endSearch,
			      const string & fileSuffix,
			      FileType fileType,
			      const string &dayDirPath,
			      const string &subdirName,
			      const DateTime &FileTime,
			      vector<DsDataFile *> &list);
  
  static vector<DsDataFile * >
  _get_fcasts_in_range(const string & dataDir,
		       const time_t & startSearch,
		       const time_t & endSearch,
		       const string & fileSuffix);
  
  static int _get_fcasts_in_range_daydir(const string & dataDir,
					 const time_t & startSearch,
					 const time_t & endSearch,
					 const string & fileSuffix,
					 vector<DsDataFile *> &list,
					 const string &dayDirPath,
					 int year, int month, int day);

  static int _get_fcasts_in_range_gendir(const string & dataDir,
					 const time_t & startSearch,
					 const time_t & endSearch,
					 const string & fileSuffix,
					 vector<DsDataFile *> &list,
					 const string &genDirPath,
					 int year, int month, int day,
					 int hour, int min, int sec);

  static int
  _get_latest_isForecast(const string &dataDir,
			 const set<string, greater<string> > &daySet,
			 bool &isForecast);

  static DsDataFile *
  _get_latest_non_forecast(const string &dataDir,
			   const string dayName,
			   const string &fileSuffix);

  static DsDataFile *
  _get_latest_forecast(const string &dataDir,
		       const set<string, greater<string> > &daySet,
		       const string &fileSuffix);
  
  static int
  _get_latest_scan_leadtimes(const string &dayDirPath,
			     const string &entryName,
			     const string &fileSuffix,
			     const DateTime &entryTime,
			     time_t &latestTime,
			     int &latestLeadSecs,
			     string &latestName);

};

inline DsDataFile & DsDataFile::operator= (const DsDataFile & orig)
{
    _dataDir = orig._dataDir;
    _fileTime = orig._fileTime;
    _fileType = orig._fileType;
    _fileSuffix = orig._fileSuffix;
    return *this;
}

#endif
