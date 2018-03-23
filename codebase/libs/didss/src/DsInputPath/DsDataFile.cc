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
// DsDataFile.cc : Thread-safe location and identification of data files.
//
// Paddy McCarty, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// February 1999
//
//////////////////////////////////////////////////////////
//
// 
//
///////////////////////////////////////////////////////////

#include <sys/stat.h>
#include <unistd.h>

#include <toolsa/umisc.h>
#include <toolsa/file_io.h>
#include <toolsa/ReadDir.hh>
#include <toolsa/toolsa_macros.h>
#include <didss/DsDataFile.hh>
using namespace std;

bool DsDataFile::_isDebug(false);
bool DsDataFile::_isVerbose(false);
int DsDataFile::_maxForecastLeadDays = DsDataFile::getMaxForecastLeadDays();

DsDataFile::DsDataFile() 
           : _dataDir(""),
             _fileTime(DateTime::NEVER),
             _fileType(DS_UNKNOWN_FILE_TYPE),
             _fileSuffix("")
{
}

DsDataFile::DsDataFile(const string & dataDir,
		       const DateTime & fileTime,
                       const string & fileSuffix,
		       DsDataFile::FileType fileType)
           : _dataDir(dataDir),
             _fileTime(fileTime),
             _fileType(fileType),
             _fileSuffix(fileSuffix)
{
}


// virtual
DsDataFile::~DsDataFile()
{
}

// Get the latest data file.
//   If prevTime is supplied, only returns a file if it is newer than
//   that time.
//   
//   Returns pointer to DsDataFile that must be freed, on success.
//   Returns NULL if there is no such file.
// 
// static 
DsDataFile * DsDataFile::getLatest(const string &dataDir,
                                   const string &fileSuffix /* = ""*/,
                                   const DateTime * prevTime /* = NULL*/ )
{

  // first read the data directory, compiling a sorted list of
  // date strings

  set<string, greater<string> > daySet;
  
  // open dir
  ReadDir allDir;
  if (allDir.open(dataDir.c_str())) {
    if (_isDebug) {
      cerr << "Could not open dir for getLatest(...): "
	   << "    " << dataDir << endl;
    }
    return NULL;
  }
 
  // Iterate through the entries, building up a sorted list

  struct dirent * allEnt;
  for (allEnt = allDir.read(); allEnt != NULL; allEnt = allDir.read()) {
    
    // Skip files starting with '.' or '_'.
    if (allEnt->d_name[0] == '.' || allEnt->d_name[0] == '_') {
      if (_isVerbose) {
	cerr << "  Skipping invalid dir: " << allEnt->d_name << endl;
      }
      continue;
    }
    
    // scan date

    int year, month, day;
    if (sscanf(allEnt->d_name, "%4d%2d%2d",
	       &year, &month, &day) != 3) {
      if (_isVerbose) {
	cerr << "    Could not get the date info "
	     << "from file name: " << allEnt->d_name << endl;
      }
      continue;
    }

    // check date

    if (month < 1  || month > 12  || day < 1  || day > 31)    {
      if (_isDebug) {
	cerr << "    Illegal ymd info from the file name. y: "
	     << year << " m: " << month << " d: " << day << endl;
      }
      continue;
    }

    daySet.insert(daySet.begin(), allEnt->d_name);
    
  } // allEnt
  allDir.close();
  
  if (_isVerbose) {
    cerr << "getLatest: valid day set: " << endl;
    set<string, greater<string> >::iterator ii;
    for (ii = daySet.begin(); ii != daySet.end(); ii++) {
      cerr << "  " << *ii << endl;
    } // ii
  }

  if (daySet.size() == 0) {
    if (_isDebug) {
      cerr << "  No valid entries found, dir: " << dataDir << endl;
    }
    return NULL;
  }
  
  // is the latest entry a day file?

  const string &dayName = *(daySet.begin());
  string latestEntryPath = dataDir + PATH_DELIM + dayName;
  struct stat latestEntryStat;
  if (ta_stat(latestEntryPath.c_str(), &latestEntryStat)) {
    if (_isDebug) {
      cerr << "  LatestEntry does not exist: " << latestEntryPath << endl;
    }
    return NULL;
  }
  if (S_ISREG(latestEntryStat.st_mode)) {

    // regular file, must be a day file

    // check suffix
    
    if (fileSuffix.size() > 0) {
      const char *suffix = strrchr(dayName.c_str(), '.');
      if (suffix == NULL || (strcmp(suffix + 1, fileSuffix.c_str()))) {
	if (_isVerbose) {
	  cerr << "  Rejecting, wrong suffix: " << dataDir
	       << PATH_DELIM << dayName << endl;
	  cerr << "  Required suffix: " << fileSuffix << endl;
	}
	return NULL;
      }
    }
    
    int year, month, day;
    if (sscanf(dayName.c_str(), "%4d%2d%2d", &year, &month, &day) != 3) {
      return NULL;
    }
    DateTime fileDateTime(year, month, day, 0, 0, 0);
    DsDataFile * latestFile = new DsDataFile(dataDir, fileDateTime,
					     fileSuffix, DS_DAILY_FILE);
    return latestFile;
  }

  // check if we have forecast files or not

  bool isForecast;
  if (_get_latest_isForecast(dataDir, daySet, isForecast)) {
    return NULL;
  }
  
  if (isForecast) {
    return _get_latest_forecast(dataDir, daySet, fileSuffix);
  } else {
    return _get_latest_non_forecast(dataDir, dayName, fileSuffix);
  }

}

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
// static 

DsDataFile *
DsDataFile::getClosestToDateTime(const string & dataDir,
				 const DateTime & fileTime,
				 size_t marginSecs,
				 SearchType searchType,
				 const string & fileSuffix /* = ""*/ )

{
    DateTime startSearch(fileTime);
    DateTime endSearch(fileTime);

    // Determine the fileType of data being retrieved. If a forecast time
    //   is supplied, it is forecast data, otherwise it's based on the 
    //   file suffix.
    FileType fileType;
    if (fileTime.isForecastTime()) {
        fileType = DS_FORECAST_FILE;
    }
    else {
        fileType = _get_type_for_suffix(fileSuffix);
    }

    if (fileType == DS_FORECAST_FILE) {

        // 
        // Searching forecast data. Adjust the lead times.
        //   The search happens *within* the given gen time.
        // 

        DeltaTime leadDeltaTime = *fileTime.getLeadDeltaTime();
        size_t origDuration  = leadDeltaTime.getDurationInSeconds();
        size_t startDuration = origDuration;
        size_t endDuration   = origDuration;

        if (marginSecs > origDuration) {
            startDuration = 0;
        }
        endDuration += marginSecs;

        switch (searchType) {
          case DS_SEARCH_CENTERED:
            startSearch.setLeadSecs(startDuration);
            endSearch.setLeadSecs(endDuration);
            break;
          case DS_SEARCH_BEFORE:
            startSearch.setLeadSecs(startDuration);
            break;
          case DS_SEARCH_AFTER:
            endSearch.setLeadSecs(endDuration);
            break;
          default:
            break;
        }
    }
    else {
        // 
        // Searching measured data. Adjust the beginning and ending times.
        // 

        switch (searchType) {
          case DS_SEARCH_CENTERED:
            startSearch -= (long) marginSecs;
            endSearch += (long) marginSecs;
            break;
          case DS_SEARCH_BEFORE:
            startSearch -= (long) marginSecs;
            break;
          case DS_SEARCH_AFTER:
            endSearch += (long) marginSecs;
            break;
          default:
            break;
        }
    }

    if (_isVerbose) {
        cerr << "Searching for closest ";
        switch (fileType) {
          case DS_FORECAST_FILE:     cerr << "DS_FORECAST_FILE"; break;
          case DS_MULTI_FILE:        cerr << "DS_MULTI_FILE"; break;
          case DS_DAILY_FILE:        cerr << "DS_DAILY_FILE"; break;
          case DS_UNKNOWN_FILE_TYPE: cerr << "DS_UNKNOWN_FILE_TYPE"; break;
        }
        cerr << ":" << endl;
        cerr << "    Data Dir: " << dataDir << endl;
        cerr << "    fileTime: " << fileTime << endl;
        cerr << "    marginSecs: " << marginSecs << endl;
        cerr << "    fileSuffix: " << fileSuffix << endl;
        cerr << "    startSearch: " << startSearch << endl;
        cerr << "    endSearch: " << endSearch << endl << endl;
    }

    // Get all the possible files in the range.
    vector<DsDataFile *> choices = getRange(dataDir, startSearch, endSearch,
                                            fileSuffix, fileType);

    // Test for the easy case.
    if (choices.size() < 1) {
        if (_isVerbose) {
            cerr << "        No potential choices found!" << endl;
        }
        return NULL;
    }

    // Not an easy answer. Iterate to find the closest.
    DsDataFile closest = *choices[0];
    size_t diff = getTimeDiff(closest.getFileTime(), fileTime);
    for (size_t i = 1; i < choices.size(); i++) {
        DsDataFile * currFile = choices[i];

        if (_isVerbose) {
            cerr << "        Potential choice: "
                 << currFile->getFileStr() << endl;
        }

        size_t curDiff = getTimeDiff(currFile->getFileTime(), fileTime);

        if (curDiff < diff) {
            diff = curDiff;
            closest = *currFile;
        }

    }

    DsDataFile * returnFile = new DsDataFile(closest);

    for (size_t i = 0; i < choices.size(); i++) {
      delete choices[i];
    }
    
    return returnFile;
}

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
// static 
DsDataFile * DsDataFile::getClosestToValid(const string & dataDir,
					   const time_t & targetTime,
					   size_t marginSecs,
					   SearchType searchType,
					   const string & fileSuffix /* = ""*/ )
{

    DateTime fileTime(targetTime);
    DateTime startSearch(targetTime);
    DateTime endSearch(targetTime);

    // Get any measured data files which match the criteria.
    //  If any are found, return the closest one. Done.
    // 
    switch (searchType) {
      case DS_SEARCH_CENTERED:
        startSearch -= (long) marginSecs;
        endSearch += (long) marginSecs;
        break;
      case DS_SEARCH_BEFORE:
        startSearch -= (long) marginSecs;
        break;
      case DS_SEARCH_AFTER:
        endSearch += (long) marginSecs;
        break;
      default:
        break;
    }

    if (_isVerbose) {
        cerr <<
	  "getClosestToValid: Searching for closest DS_MULTI_FILE:" << endl;
        cerr << "    Data Dir: " << dataDir << endl;
        cerr << "    fileTime: " << fileTime << endl;
        cerr << "    marginSecs: " << marginSecs << endl;
        cerr << "    fileSuffix: " << fileSuffix << endl;
        cerr << "    startSearch: " << startSearch << endl;
        cerr << "    endSearch: " << endSearch << endl << endl;
    }

    // Get all the possible files in the range.
    vector<DsDataFile *> choices = getRange(dataDir, startSearch, endSearch,
                                            fileSuffix, DS_MULTI_FILE);

    // Any files? If so, return the closest.
    if (choices.size() > 0) {
        DsDataFile closest = *choices[0];
        size_t diff = getTimeDiff(closest.getFileTime(), fileTime);
        for (size_t i = 1; i < choices.size(); i++) {
            DsDataFile * currFile = choices[i];
    
            if (_isVerbose) {
                cerr << "        Potential choice: "
                     << currFile->getFileStr() << endl;
            }
    
            size_t curDiff = getTimeDiff(currFile->getFileTime(), fileTime);
    
            if (curDiff < diff) {
                diff = curDiff;
                closest = *currFile;
            }
        }

	// set return val and clean up

        DsDataFile * returnFile = new DsDataFile(closest);
        for (size_t i = 0; i < choices.size(); i++) {
            delete choices[i];
        }
        return returnFile;

    }
    else {
        if (_isVerbose) {
            cerr << "---> No potential DS_MULTI_FILE choices found!" << endl;
        }
    }

    return getForecastClosestToValid(dataDir, targetTime, marginSecs,
				     searchType, fileSuffix, NULL);

}

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

DsDataFile *
DsDataFile::getForecastClosestToValid(const string & dataDir,
				      const time_t & targetTime,
				      const size_t & marginSecs,
				      const SearchType searchType,
				      const string & fileSuffix /* = ""*/,
				      const DateTime * prevForecastTime /* = NULL*/ )

{

  DateTime fileTime(targetTime);
  DateTime startSearch(targetTime);
  DateTime endSearch(targetTime);
  
  switch (searchType) {
  case DS_SEARCH_CENTERED:
    startSearch -= (long) marginSecs;
    endSearch += (long) marginSecs;
    break;
  case DS_SEARCH_BEFORE:
    startSearch -= (long) marginSecs;
    break;
  case DS_SEARCH_AFTER:
    endSearch += (long) marginSecs;
    break;
  default:
    break;
  }
  
  vector <DsDataFile * >
    rangeList = _get_fcasts_in_range(dataDir,
				     startSearch.utime(),
				     endSearch.utime(),
				     fileSuffix);
  
  if (rangeList.size() < 1) {
    return NULL;
  }

  // Process the list to find the forecasts
  // closest to the specified time.
  DsDataFile * closest = rangeList[0];
  DateTime closestTime = closest->getFileTime();
  size_t closestDiff = labs( targetTime - closestTime.forecastUtime() );
  vector <DsDataFile * > closestList;
  closestList.push_back(closest);
  
  for (size_t i = 1; i < rangeList.size(); i++) {
    DsDataFile * currFile = rangeList[i];
    DateTime currTime = currFile->getFileTime();
    size_t currDiff = labs( targetTime - currTime.forecastUtime() );
    
    if (currDiff < closestDiff) {
      // Throw away the old list contents.
      closestList.clear();
      
      // Add the current, since it is now closest.
      closest = currFile;
      closestTime = currTime;
      closestDiff = currDiff;
      closestList.push_back(currFile);
    }
    else if (currDiff == closestDiff) {
      closestList.push_back(currFile);
    }
  }
  
  // Now find the best forecast within the set of closest.
  if (closestList.size() == 1) {
    closest = closestList[0];
    closestTime = closest->getFileTime();
  }
  else if (closestList.size() > 1) {
    closest = closestList[0];
    closestTime = closest->getFileTime();
    int closestLeadSecs =
      closestTime.getLeadDeltaTime()->getDurationInSeconds();
    for (size_t i = 1; i < closestList.size(); i++) {
      DsDataFile * currFile = closestList[i];
      DateTime currTime = currFile->getFileTime();
      int currLeadSecs =
	currTime.getLeadDeltaTime()->getDurationInSeconds();
      
      if (currLeadSecs < closestLeadSecs) {
	closest = currFile;
	closestTime = currTime;
	closestLeadSecs = currLeadSecs;
      }
    }
  }
  else {
    cerr << "ERROR: Algorithm failure in getForecastClosestToValid()"
	 << endl;
    closest = NULL;
    closestTime = DateTime(0, 0);
  }

  // Check if the located forecast is "better" than the previous,
  //   which means that it is closer to the target time.
  // 
  bool betterThanPrev = true;
  if (prevForecastTime != NULL) {
    closestDiff = labs( targetTime - closestTime.forecastUtime() );
    size_t prevDiff =
      labs(targetTime - prevForecastTime->forecastUtime());
    if (closestDiff >= prevDiff) {
      betterThanPrev = false;
    }
    else if (closestDiff == prevDiff) {
      long prevLead =
	prevForecastTime->getLeadDeltaTime()->getDurationInSeconds();
      long closestLead =
	closestTime.getLeadDeltaTime()->getDurationInSeconds();
      if (closestLead >= prevLead) {
	betterThanPrev = false;
      }
    }
  }

  // set return val

  DsDataFile * returnFile = NULL;
  if (betterThanPrev && closest != NULL) {
    returnFile = new DsDataFile(*closest);
  }

  // clean up

  for (size_t i = 0; i < rangeList.size(); i++) {
    delete rangeList[i];
  }
  
  return returnFile;

}

// Get the "best" data file for the specified valid time, using the
//   supplied search type and margin. Best means that the file has the
//   shortest lead time of any file in the specified range. Proximity to
//   the target time is used only as a tie-breaker if more than one
//   file has the same shortest lead time.
//
//   If prevForecastTime is supplied, only returns a file if there
//   is a "better" forecast than the one supplied.
//
//    o Searches within any forecast data in the dataDir. All
//        forecasts for times within the specified range are compared, and the
//        one with the shortest lead time is returned. Note that all
//        forecasts which fall within the specifed range are given
//        equal weight unless the lead-time test yields non-unique results.
//        in this case, the files with the shortest lead time are compared
//        to determine which file falls closest to the exact specified time.
//
//        The reason for letting lead-time drive the selection process of
//        the closest file is in order to select the best forecast. We want
//        to prevent a 48-hour forecast for 12:02pm from beating a 2-hour
//        forecast for 12:05pm when the user has asked for the closest
//        forecast for 12:00pm within 30 minutes.
//
//        This has a drawback, however, if the specified range is large.
//        If the desired time is 12:00pm, and a range of six hours is given,
//        this routine will return a 1-hour forecast for 5:00pm over a
//        3-hour forecast for 12:00pm.
// 
// static 
DsDataFile *
DsDataFile::getBestForecast(const string & dataDir,
			    const time_t & targetTime,
			    const size_t & marginSecs,
			    const SearchType searchType,
			    const string & fileSuffix /* = ""*/,
			    const DateTime * prevForecastTime /* = NULL*/ )
{
    DateTime fileTime(targetTime);
    DateTime startSearch(targetTime);
    DateTime endSearch(targetTime);

    switch (searchType) {
      case DS_SEARCH_CENTERED:
        startSearch -= (long) marginSecs;
        endSearch += (long) marginSecs;
        break;
      case DS_SEARCH_BEFORE:
        startSearch -= (long) marginSecs;
        break;
      case DS_SEARCH_AFTER:
        endSearch += (long) marginSecs;
        break;
      default:
        break;
    }

    vector <DsDataFile * >
      rangeList = _get_fcasts_in_range(dataDir,
				       startSearch.utime(),
				       endSearch.utime(),
				       fileSuffix);

    if (rangeList.size() < 1) {
        return NULL;
    }

    // Process the list to find the forecasts with the shortest lead time.
    //   
    DsDataFile * shortest = rangeList[0];
    DateTime shortestTime = shortest->getFileTime();
    size_t shortestLead =
      shortestTime.getLeadDeltaTime()->getDurationInSeconds();
    vector <DsDataFile * > shortestList;
    shortestList.push_back(shortest);
        
    for (size_t i = 1; i < rangeList.size(); i++) {
      DsDataFile * currFile = rangeList[i];
      DateTime currTime = currFile->getFileTime();
      size_t currLead =
	currTime.getLeadDeltaTime()->getDurationInSeconds();
      
      if (currLead < shortestLead) {
	// Throw away the old list contents.
	shortestList.clear();
	
	// Add the current, since it is now shortest.
	shortest = currFile;
	shortestTime = currTime;
	shortestLead = currLead;
	shortestList.push_back(currFile);
      }
      else if (currLead == shortestLead) {
	shortestList.push_back(currFile);
      }
    }
    
    // Now find the closest forecast within the set of shortest lead times.
    if (shortestList.size() == 1) {
      shortest = shortestList[0];
      shortestTime = shortest->getFileTime();
    }
    else if (shortestList.size() > 1) {
      shortest = shortestList[0];
      shortestTime = shortest->getFileTime();
      int shortestDiff = labs(targetTime - shortestTime.forecastUtime());
      for (size_t i = 1; i < shortestList.size(); i++) {
	DsDataFile * currFile = shortestList[i];
	DateTime currTime = shortest->getFileTime();
	int currDiff = labs(targetTime - currTime.forecastUtime());
	
	if (currDiff < shortestDiff) {
	  shortest = currFile;
	  shortestTime = currTime;
	  shortestDiff = currDiff;
	}
      }
    }
    else {
      cerr << "ERROR: Algorithm failure in getBestForecast()"
	   << endl;
      shortest = NULL;
      shortestTime = DateTime(0, 0);
    }
    
    // Check if the located forecast is "better" than the previous,
    //   which means that it has a shorter lead time.
    // 
    bool betterThanPrev = true;
    if (prevForecastTime != NULL) {
      shortestLead =
	shortestTime.getLeadDeltaTime()->getDurationInSeconds();
      size_t prevLead =
	prevForecastTime->getLeadDeltaTime()->getDurationInSeconds();
      if (shortestLead >= prevLead) {
	betterThanPrev = false;
      }
      else if (shortestLead == prevLead) {
	size_t shortestDiff =
	  labs( targetTime - shortestTime.forecastUtime() );
	size_t prevDiff =
	  labs(targetTime - prevForecastTime->forecastUtime());
	if (shortestDiff >= prevDiff) {
	  betterThanPrev = false;
	}
      }
    }
    
    DsDataFile * returnFile = NULL;
    if (betterThanPrev && shortest != NULL) {
      returnFile = new DsDataFile(*shortest);
    }
    for (size_t i = 0; i < rangeList.size(); i++) {
      delete rangeList[i];
    }
    
    return returnFile;

}

// Get the positive time difference between two files, in seconds. 
//   If the times are forecast times, just compares the lead times.
//
//   Returns DS_UINT_MAX (0xffffffff) if the comparison is not allowed,
//               i.e. the times are different types, or both are forecast
//               times but have different gen times.
// static

size_t DsDataFile::getTimeDiff(const DateTime & t1, const DateTime & t2)
{
    if (t1.isForecastTime() != t2.isForecastTime()) {
        return DS_UINT_MAX;
    }

    if (t1.isForecastTime()) {
        ssize_t diff = t1.getLeadDeltaTime()->getDurationInSeconds() -
          t2.getLeadDeltaTime()->getDurationInSeconds();
        return labs(diff);
    }
    else {
        time_t diff = t1 - t2;
        if (diff == DateTime::NEVER) {
            return DS_UINT_MAX;
        }

        // Convert diff to unsigned.
	// Does not violate covariance (smaller range)
        return (size_t) labs(diff);
    }
}


// Get all data files in the indicated range.
//
// static
vector<DsDataFile *> DsDataFile::getRange(const string & dataDir,
                                          const DateTime & startSearch,
                                          const DateTime & endSearch,
                                          const string & fileSuffix /* = ""*/,
                                          FileType fileType /* = DS_MULTI_FILE*/ )
{
    vector<DsDataFile *> list;

    if (startSearch.isForecastTime() != endSearch.isForecastTime()) {
        if (_isDebug) {
            cerr << "Start and end times do not have same type." << endl;
        }
        return list;
    }
    if ((fileType == DS_FORECAST_FILE) != startSearch.isForecastTime()) {
        if (_isDebug) {
            cerr << "fileType and date types don't match." << endl;
        }
        return list;
    }

    if (fileType == DS_DAILY_FILE) {

      // The file is DAILY type.

      _get_range_daily(dataDir, startSearch, endSearch,
		       fileSuffix, fileType, list);
      
    } else {

      // The file is either MULTI or FORECAST type.
 
      _get_range_multi(dataDir, startSearch, endSearch,
		       fileSuffix, fileType, list);
      
    }
    
    return list;
}

// static
DsDataFile::FileType DsDataFile::_get_type_for_suffix(const string & suffix)
{
    if (suffix == "mdv") {
        return DS_MULTI_FILE; // Assume multi for mdv, though forecast allowed.
    }
    else if (suffix == "spdb") {
        return DS_DAILY_FILE; // Spdb is always daily.
    }
    else {
        return DS_UNKNOWN_FILE_TYPE;
    }
}

bool DsDataFile::exists() const
{
    if (_fileType == DS_UNKNOWN_FILE_TYPE) {
        return false;
    }

    if (_dataDir.size() <= 0) {
        return false;
    }

    if ( !_fileTime.isValid() ) {
        return false;
    }

    // Stat the data file.
    struct stat statusStruct;
    int status = ta_stat(getFileStr().c_str(), &statusStruct);
    if (status < 0) {                                              
        return false;
    }

    // Make sure it's a regular file.
    if ( !S_ISREG(statusStruct.st_mode) ) {
        return false;
    }

    // The file exists!
    return true;
}

string DsDataFile::getFileStr() const
{
    if (_fileType == DS_UNKNOWN_FILE_TYPE) {
        return "";
    }

    if (_dataDir.size() <= 0) {
        return "";
    }

    if ( !_fileTime.isValid() ) {
        return "";
    }

    string fullFileName(_dataDir);
           fullFileName += PATH_DELIM;

    if (_fileType == DS_DAILY_FILE) {
        // Daily file. Concatenate the dataDir with the file name and suffix.

        fullFileName += _fileTime.getDateStrPlain();
    }
    else {
        string dayInfo(_fileTime.getDateStrPlain());
        string secInfo(_fileTime.getTimeStrPlain());

        if (_fileType == DS_FORECAST_FILE) {
            // 
            // This is a forecast time:
            //    _dataDir/YYYYMMDD/g_HHMMSS/f_00000180.suffix
            // 

            // Deal with the lead time.
            const DeltaTime * leadDelta = _fileTime.getLeadDeltaTime();
            if (leadDelta == NULL) {
                return "";
            }
            char buf[20];
            sprintf(buf, "%08ld", leadDelta->getDurationInSeconds());

            fullFileName += dayInfo + PATH_DELIM;
            fullFileName += "g_" + secInfo + PATH_DELIM;
            fullFileName += "f_";
            fullFileName += buf;
        }
        else {
            // 
            // Not a forecast time. File is in the dayInfo directory:
            //    _dataDir/YYYYMMDD/HHMMSS.suffix
            // 
            fullFileName += dayInfo + PATH_DELIM;
            fullFileName += secInfo;
        }
    }

    // Add the suffix, if any.
    if (_fileSuffix.size() > 0) {
        fullFileName += "." + _fileSuffix;
    }

    return fullFileName;
}

/////////////////////////////////////////////
// get the max forecast time interval in days
// This affects the search efficiency.

int DsDataFile::getMaxForecastLeadDays()
{
  char *leadStr = getenv("MAX_FORECAST_LEAD_DAYS");
  if (leadStr != NULL) {
    int leadDays;
    if (sscanf(leadStr, "%d", &leadDays) == 1) {
      return leadDays;
    }
  }
  return _maxForecastLeadDaysDefault;
}

// Get all daily files in the indicated range.
//
// static
int DsDataFile::_get_range_daily(const string & dataDir,
				 const DateTime & startSearch,
				 const DateTime & endSearch,
				 const string & fileSuffix,
				 FileType fileType,
				 vector<DsDataFile *> &list)

{

  string pathString = dataDir + PATH_DELIM;

  // Iterate through the files in the main _dataDir,
  //   looking for files in the range.

  ReadDir dayDir;
  if (dayDir.open(pathString.c_str())) {                 
    if (_isDebug) {
      cerr << "Could not open the data dir for a daily file." << endl;
    }
    return -1;
  }                                                    
  
  // Iterate through the entries.                               
  struct dirent * dayEnt;
  for (dayEnt = dayDir.read(); dayEnt != NULL; dayEnt = dayDir.read()) {

    if (_isVerbose) {
      cerr << "Found a file: " << dayEnt->d_name << endl;
    }
    
    // Skip files starting with '.' or '_'.
    if (dayEnt->d_name[0] == '.' || dayEnt->d_name[0] == '_') {
      if (_isVerbose) {
	cerr << "    Skipping file starting with "
	     << "a dot or underscore." << endl;
      }
      continue;
    }
    
    // Skip files not ending in the suffix.
    if (fileSuffix.size() > 0 && 
	strstr(dayEnt->d_name, fileSuffix.c_str()) == NULL) {
      if (_isVerbose) {
	cerr << "    Skipping file not ending  with "
	     << "suffix: " << fileSuffix << endl;
      }
      continue;
    }
    
    // Get the time components out of the file name.

    int year, month, day;
    if (sscanf(dayEnt->d_name, "%4d%2d%2d", &year, &month, &day) != 3) {
      if (_isVerbose) {
	cerr << "    Could not get the time components "
	     << "from file name." << dayEnt->d_name << endl;
      }
      continue;
    }
    
    if (month <= 0 || month > 12 || day <= 0  || day > 31)    {
      if (_isDebug) {
	cerr << "    Illegal ymd info from the file name. y: "
	     << year << " m: " << month << " d: " << day << endl;
      }
      continue;
    }
    
    // See if the file is in range.

    DateTime FileTime(year, month, day, 0, 0, 0);
    
    if (FileTime.utime() < startSearch.utime() ||
	FileTime.utime() > endSearch.utime()) {
      if (_isVerbose) {
	cerr << "---> Time out of range!" << endl;
      }
      continue;
    }
    
    // Add the file to the list.
    DateTime fileDateTime(FileTime.utime());
    DsDataFile * newFile = new DsDataFile(dataDir, fileDateTime,
					  fileSuffix, fileType);
    
    if (_isVerbose) {
      cerr << "===> Accepted: " << newFile->getFileStr() << endl;
    }
    
    list.push_back(newFile);

  }
  
  dayDir.close();
  
  return 0;

}


// Get all multi or forecast files in the indicated range.
//
// static
int DsDataFile::_get_range_multi(const string & dataDir,
				 const DateTime & startSearch,
				 const DateTime & endSearch,
				 const string & fileSuffix,
				 FileType fileType,
				 vector<DsDataFile *> &list)

{
  
  // 
  // The file is either MULTI or FORECAST type.
  // 
  
  size_t startDay = startSearch.utime() / 86400;
  size_t endDay = endSearch.utime() / 86400;

  for (size_t iday = startDay; iday <= endDay; iday++) {

    DateTime currDay(iday*86400);
    string dayString = currDay.getDateStrPlain();
    string dayDirPath = dataDir + PATH_DELIM + dayString;
    
    ReadDir dayDir;
    if (dayDir.open(dayDirPath.c_str())) {
//       if (_isDebug) {
// 	cerr << "Could not open dir for a forecast or multi file: "
// 	     << "    " << dayDirPath << endl;
//       }
      // Do not bail, as there may be other dirs available.
      continue;
    }
    
    // Iterate through the entries.

    struct dirent * dayEnt;
    for (dayEnt = dayDir.read(); dayEnt != NULL; dayEnt = dayDir.read()) {
      
      if (_isVerbose) {
	cerr << "Found a file: " << dayEnt->d_name << endl;
      }
      
      // Skip files starting with '.' or '_'.
      if (dayEnt->d_name[0] == '.' || dayEnt->d_name[0] == '_') {
	if (_isVerbose) {
	  cerr << "    Skipping file starting with "
	       << "a dot or underscore." << endl;
	}
	continue;
      }

      // scan the name for the time
      // Skip the first two characters of forecast filenames.

      char *dname = dayEnt->d_name;
      if (fileType == DS_FORECAST_FILE) {
	dname += 2;
      }
      
      int hour, min, sec;
      if (sscanf(dname, "%2d%2d%2d", &hour, &min, &sec) != 3) {
	if (_isVerbose) {
	  cerr << "    Could not get the time info "
	       << "from file name: " << dayEnt->d_name << endl;
	}
	continue;
      }

      if (hour < 0 || hour > 23 ||
	  min < 0  || min > 59  ||
	  sec < 0  || sec > 59)    {
	if (_isDebug) {
	  cerr << "    Illegal hms info from the file name. h: "
	       << hour << " m: " << min << " s: " << sec << endl;
	}
	continue;
      }
      
      // See if the file is in range.
      //   Note that this is not safe for thread b/c udate_time
      //   has static

      DateTime FileTime(currDay.getYear(),
			currDay.getMonth(),
			currDay.getDay(),
			hour, min, sec);
      
      if (FileTime.utime() < startSearch.utime() ||
	  FileTime.utime() > endSearch.utime()) {
	
	if (_isVerbose) {
	  cerr << "--->Time out of range!" << endl;
	}
	continue;
      }
      
      // 
      // Passed all the tests. 
      //   dayEnt represents a file or forecast dir that is in range.
      // 

      if (fileType == DS_FORECAST_FILE) {

	// forecast type - read dir
	
	_get_range_fcast(dataDir, startSearch, endSearch, fileSuffix,
			 fileType, dayDirPath, dayEnt->d_name,
			 FileTime, list);
	
      } else {

	// Not a forecast. Just put the file in the list.
	
	DateTime fileDateTime(FileTime.utime());
	DsDataFile * newFile = new DsDataFile(dataDir, fileDateTime,
					      fileSuffix, fileType);
	
	if (_isVerbose) {
	  cerr << "===> Accepted: " << newFile->getFileStr()
	    << endl;
	}
	
	list.push_back(newFile);

      } // if (fileType == DS_FORECAST_FILE)
      
    } // for (dayEnt= readdir(dayDIR) ...
    
    dayDir.close();
    
  } // iday
  
  return 0;

}

// Get all forecast files in the indicated range.
//
// static
int DsDataFile::_get_range_fcast(const string & dataDir,
				 const DateTime & startSearch,
				 const DateTime & endSearch,
				 const string & fileSuffix,
				 FileType fileType,
				 const string &dayDirPath,
				 const string &subdirName,
				 const DateTime &FileTime,
				 vector<DsDataFile *> &list)
{

  // Determine if this dir is the start or end dir.
  //   If so, must compare the lead times.
  //   For other dirs we can grab *all* the files.
  // 
  
  if (_isVerbose) {
    cerr << "    Searching Dir: " << subdirName << endl;
  }
  
  bool isStartDir = false;
  bool isEndDir = false;
  if (startSearch.getHour() == FileTime.getHour() &&
      startSearch.getMin()  == FileTime.getMin() &&
      startSearch.getSec()  == FileTime.getSec()) {
    isStartDir = true;
  }
  if (endSearch.getHour() == FileTime.getHour() &&
      endSearch.getMin()  == FileTime.getMin()  &&
      endSearch.getSec()  == FileTime.getSec()) {
    isEndDir = true;
  }
  
  // Determine the lead seconds ahead of time.
  size_t startLeadSecs =
    startSearch.getLeadDeltaTime()->getDurationInSeconds();
  size_t endLeadSecs =
    endSearch.getLeadDeltaTime()->getDurationInSeconds();
  
  // Iterate through the dir and look at all the files.
  
  string genDirPath(dayDirPath);
  genDirPath += PATH_DELIM;
  genDirPath += subdirName;
  
  ReadDir genDir;
  if (genDir.open(genDirPath.c_str())) {
    if (_isDebug)
      cerr << "Error opening forecast directory " << genDirPath << endl;
    return -1;
  }
  
  // Iterate through the entries.
  
  struct dirent * genEnt;
  for (genEnt = genDir.read(); genEnt != NULL; genEnt = genDir.read()) {
    
    if (_isVerbose) {
      cerr << "    Found a file: " << genDirPath
	   << PATH_DELIM << genEnt->d_name << endl;
    }
    
    unsigned int leadSecs;
    
    // Skip files starting with '.' or '_'.
    if (genEnt->d_name[0] == '.' || genEnt->d_name[0] == '_') {
      if (_isVerbose) {
	cerr << "    Skipping file starting with a "
	     << "dot or underscore." << endl;
      }
      continue;
    }
    
    // skip the leading "f_"
    
    if (sscanf(genEnt->d_name, "f_%8u", &leadSecs) != 1) {
      if (_isVerbose) {
	cerr << "    Could not get lead secs info "
	     << "from file name." << endl;
      }
      continue;
    }
    
    if (isStartDir && leadSecs < startLeadSecs) {
      if (_isVerbose) {
	cerr << "    Skipping file b/c it's in start "
	     << "dir and low lead secs." << endl;
      }
      continue;
    }
    
    if (isEndDir && leadSecs > endLeadSecs) {
      if (_isVerbose) {
	cerr << "    Skipping file b/c it's in end "
	     << "dir and high lead secs." << endl;
      }
      continue;
    }
    
    DateTime fileDateTime(FileTime.utime(), leadSecs);
    DsDataFile * newFile = new DsDataFile(dataDir, fileDateTime,
					  fileSuffix, fileType);
    
    if (_isVerbose) {
      cerr << "===> Accepted: " << newFile->getFileStr()
	<< endl;
    }
    
    list.push_back(newFile);
  }
  
  genDir.close();
  return 0;

}

// static

vector <DsDataFile * >
DsDataFile::_get_fcasts_in_range(const string & dataDir,
				 const time_t & startSearch,
				 const time_t & endSearch,
				 const string & fileSuffix)
{

    vector<DsDataFile *> list;

    // compute day search limits

    time_t firstDirDate =
      (startSearch / SECS_IN_DAY - _maxForecastLeadDays) * SECS_IN_DAY;
    if (_isVerbose) {
      cerr << "firstDirDate: " << DateTime::str(firstDirDate) << endl;
      cerr << "endSearch: " << DateTime::str(endSearch) << endl;
    }

    // open dir
    ReadDir allDir;
    if (allDir.open(dataDir.c_str())) {
      if (_isDebug) {
	cerr << "Could not open dir for _get_fcasts_in_range(...): "
	     << "    " << dataDir << endl;
      }
      return list;
    }
 
    // Iterate through the entries.
    struct dirent * allEnt;
    for (allEnt = allDir.read(); allEnt != NULL; allEnt = allDir.read()) {
      
      // Skip files starting with '.' or '_'.
      // Date must have 8 chars - yyyymmdd
      if (allEnt->d_name[0] == '.' || allEnt->d_name[0] == '_' ||
	  strlen(allEnt->d_name) != 8) {
	if (_isVerbose) {
	  cerr << "  Skipping invalid dir: " << allEnt->d_name << endl;
	}
	continue;
      }

      int year, month, day;
      if (sscanf(allEnt->d_name, "%4d%2d%2d", &year, &month, &day) != 3) {
	if (_isVerbose) {
	  cerr << "    Could not get the date info "
	       << "from file name: " << allEnt->d_name << endl;
	}
	continue;
      }
      
      if (month < 1  || month > 12  || day < 1  || day > 31)    {
	if (_isDebug) {
	  cerr << "    Illegal ymd info from the file name. y: "
	       << year << " m: " << month << " d: " << day << endl;
	}
	continue;
      }

      DateTime dirDate(year, month, day, 0, 0, 0);
      if (dirDate.utime() < firstDirDate || dirDate.utime() > endSearch) {
 	if (_isVerbose) {
	  cerr << "Rejecting date as outside search limits: "
	       << dirDate.dtime() << endl;
	}
	continue;
      }
      
      // Read this day dir and get any files out of it.
      
      string dayDirPath(dataDir);
      dayDirPath += PATH_DELIM;
      dayDirPath += allEnt->d_name;
      
      if (_get_fcasts_in_range_daydir(dataDir, startSearch, endSearch,
				      fileSuffix, list, dayDirPath,
				      year, month, day)) {
	continue;
      }
      
      
    }

    allDir.close();

    return list;

}

///////////////////////////////////////////
// Check through day dir
// for use by _get_fcasts_in_range()
// static

int DsDataFile::_get_fcasts_in_range_daydir(const string & dataDir,
					    const time_t & startSearch,
					    const time_t & endSearch,
					    const string & fileSuffix,
					    vector<DsDataFile *> &list,
					    const string &dayDirPath,
					    int year, int month, int day)
{

  ReadDir dayDir;
  
  if (dayDir.open(dayDirPath.c_str())) {                 
    if (_isDebug) {
      cerr << "Could not open dir: " << dayDirPath << endl;
    }
    return -1;
  }
 
  // Iterate through the entries.
  
  struct dirent * dayEnt;
  for (dayEnt = dayDir.read(); dayEnt != NULL; dayEnt = dayDir.read()) {
    
    if (_isVerbose) {
      cerr << "Found a file: " << dayEnt->d_name << endl;
    }

    // Skip files which do not start with g_
    // time must have 8 chars - g_hhmmss
    if (dayEnt->d_name[0] != 'g' || dayEnt->d_name[1] != '_' ||
	strlen(dayEnt->d_name) != 8) {
      if (_isVerbose) {
	cerr << "    Skipping invalid subdir: " << dayEnt->d_name << endl;
      }
      continue;
    }

    // scan time from name, skipping "g_"

    int hour, min, sec;
    if (sscanf(dayEnt->d_name, "g_%2d%2d%2d",
	       &hour, &min, &sec) != 3) {
      if (_isVerbose) {
	cerr << "    Could not get the hour info "
	     << "from file name: " << dayEnt->d_name << endl;
      }
      continue;
    }
	
    if (hour < 0 || hour > 23 ||
	min < 0  || min > 59  ||
	sec < 0  || sec > 59)    {
      
      if (_isDebug) {
	cerr << "    Illegal hms info from the file name. h: "
	     << hour << " m: " << min << " s: " << sec << endl;
      }
      continue;
    }
 
    // Iterate through the gen dir and look at all the files.
    
    string genDirPath(dayDirPath);
    genDirPath += PATH_DELIM;
    genDirPath += dayEnt->d_name;
    
    if (_get_fcasts_in_range_gendir(dataDir, startSearch, endSearch,
				    fileSuffix, list, genDirPath,
				    year, month, day, hour, min, sec)) {
      continue;
    }
	    
  } // readdir

  dayDir.close();

  return 0;

}

///////////////////////////////////////////
// Check through gen dir for forecast files
// for use by _get_fcasts_in_range()
// static

int DsDataFile::_get_fcasts_in_range_gendir(const string & dataDir,
					    const time_t & startSearch,
					    const time_t & endSearch,
					    const string & fileSuffix,
					    vector<DsDataFile *> &list,
					    const string &genDirPath,
					    int year, int month, int day,
					    int hour, int min, int sec)
     
{
  
  // Iterate through the dir and look at all the files.

  ReadDir genDir;
  if (genDir.open(genDirPath.c_str())) {
    if (_isDebug) {
      cerr << "Could not open generate files dir: " << genDirPath << endl;
    }
    return -1;
  }
    
  // Iterate through the entries.

  struct dirent * genEnt;
  for (genEnt = genDir.read(); genEnt != NULL; genEnt = genDir.read()) {
    
    unsigned int leadSecs;
 
    // Skip files which are not of the form f_ssssssss.mdv
    
    if (sscanf(genEnt->d_name, "f_%8u", &leadSecs) != 1) {
      if (_isVerbose) {
	cerr << "    Could not get lead secs info "
	     << "from file: " << genEnt->d_name << endl;
      }
      continue;
    }

    DateTime etime(year, month, day, hour, min, sec);
    time_t utime = etime.utime();

    // Test if the file is within range.
    if ((time_t) (utime + leadSecs) < startSearch ||
	(time_t) (utime + leadSecs) > endSearch)     {
      if (_isVerbose) {
	cerr << "---> Time out of range: "
	     << genDirPath << PATH_DELIM
	     << genEnt->d_name << endl;
      }
      continue;
    }
    
    DateTime fileDateTime(utime, leadSecs);
    DsDataFile * newFile = new DsDataFile(dataDir,
					  fileDateTime,
					  fileSuffix,
					  DS_FORECAST_FILE);
    
    if (_isVerbose) {
      cerr << "===> Accepted: " << newFile->getFileStr() << endl;
    }
    list.push_back(newFile);

  } // readdir

  genDir.close();

  return 0;

}

// check if we have forecast files or not
//
// Returns 0 on success, -1 on failure (no relevant files found)
  
int DsDataFile::
_get_latest_isForecast(const string &dataDir,
		       const set<string, greater<string> > &daySet,
		       bool &isForecast)

{
  
  // Iterate through the days

  set<string, greater<string> >::iterator ii;
  for (ii = daySet.begin(); ii != daySet.end(); ii++) {
    
    string dayDirPath = dataDir + PATH_DELIM + *ii;
    ReadDir dayDir;
    if (dayDir.open(dayDirPath.c_str())) {                 
      if (_isDebug) {
	cerr << "Could not open dir: " << dayDirPath << endl;
      }
      return -1;
    }
    
    // Iterate through the entries.
    struct dirent * dayEnt;
    for (dayEnt = dayDir.read(); dayEnt != NULL; dayEnt = dayDir.read()) {

      // non-forecast entries have hhmmss

      int hour, min, sec;
      if (sscanf(dayEnt->d_name, "%2d%2d%2d",
		 &hour, &min, &sec) == 3) {
	if (_isVerbose) {
	  cerr << "  Found non-forecast entry: " << dayDirPath
	       << PATH_DELIM <<  dayEnt->d_name << endl;
	}
	isForecast = false;
	dayDir.close();
	return 0;
      }

      // forecast entries have g_hhmmss
      
      if (sscanf(dayEnt->d_name, "g_%2d%2d%2d",
		 &hour, &min, &sec) == 3) {
	if (_isVerbose) {
	  cerr << "  Found forecast entry: " << dayDirPath
	       << PATH_DELIM <<  dayEnt->d_name << endl;
	}
	isForecast = true;
	dayDir.close();
	return 0;
      }
      
    } // dayEnt
    dayDir.close();
    
  } // ii

  return -1;

}


// get latest for non-forecast data

DsDataFile *
DsDataFile::_get_latest_non_forecast(const string &dataDir,
				     const string dayName,
				     const string &fileSuffix)
     
{
  
  int year, month, day;
  if (sscanf(dayName.c_str(), "%4d%2d%2d", &year, &month, &day) != 3) {
    return NULL;
  }
  string dayDirPath = dataDir + PATH_DELIM + dayName;

  if (_isVerbose) {
    cerr << "DsDataFile::_get_latest_non_forecast" << endl;
    cerr << "  Searching dataDir: " << dayDirPath << endl;
  }

  // loop through the entries, finding the latest one

  ReadDir dayDir;
  if (dayDir.open(dayDirPath.c_str())) {                 
    if (_isDebug) {
      cerr << "Could not open dir: " << dayDirPath << endl;
    }
    return NULL;
  }
    
  // Iterate through the entries, looking through the latest entry

  time_t latestTime = -1;
  string latestName;
  struct dirent * dayEnt;
  for (dayEnt = dayDir.read(); dayEnt != NULL; dayEnt = dayDir.read()) {

    // non-forecast entries are 'hhmmss.ext'
    
    int hour, min, sec;
    bool success = false;
    if (fileSuffix.size() > 0) {
      char suffix[256];
      if (sscanf(dayEnt->d_name, "%2d%2d%2d.%s",
		 &hour, &min, &sec, suffix) == 4) {
	if (_isVerbose) {
	  cerr << "  Found non-forecast entry: " << dayDirPath
	       << PATH_DELIM <<  dayEnt->d_name << endl;
	}
	if (strcmp(fileSuffix.c_str(), suffix)) {
	  if (_isVerbose) {
	    cerr << "  Rejecting, wrong suffix: " << dayDirPath
		 << PATH_DELIM <<  dayEnt->d_name << endl;
	    cerr << "  Required suffix: " << fileSuffix << endl;
	  }
	  continue;
	}
	success = true;
      }
    } else {
      if (sscanf(dayEnt->d_name, "%2d%2d%2d",
		 &hour, &min, &sec) == 3) {
	if (_isVerbose) {
	  cerr << "  Found non-forecast entry: " << dayDirPath
	       << PATH_DELIM <<  dayEnt->d_name << endl;
	}
	success = true;
      }
    } // if (fileSuffix.size() > 0)
    
    if (success) {
      DateTime ltime(year, month, day, hour, min, sec);
      if (ltime.utime() > latestTime) {
	latestTime = ltime.utime();
	latestName = dayEnt->d_name;
      }
    } else {
      if (_isVerbose) {
	cerr << "  Rejecting entry: " << dayDirPath
	     << PATH_DELIM <<  dayEnt->d_name << endl;
      }
    }

  } // dayEnt

  if (latestTime < 0) {
    if (_isVerbose) {
      cerr << "  No suitable file found: " << dayDirPath << endl;
    }
    return NULL;
  }
  
  if (_isVerbose) {
    cerr << "  latest path: " << dayDirPath
	 << PATH_DELIM << latestName << endl;
  }

  DateTime fileDateTime(latestTime);
  DsDataFile * latestFile = new DsDataFile(dataDir, fileDateTime,
					   fileSuffix, DS_MULTI_FILE);
  
  return latestFile;
  
}

// get latest for non-forecast data

DsDataFile *
DsDataFile::_get_latest_forecast(const string &dataDir,
				 const set<string, greater<string> > &daySet,
				 const string &fileSuffix)

{

  int count = 0;
  time_t latestTime = -1;
  int latestLeadSecs = -1;
  string latestName;
  set<string, greater<string> >::iterator ii;
  
  for (ii = daySet.begin(); ii != daySet.end(); ii++) {

    const string &dayName = *ii;
    string dayDirPath = dataDir + PATH_DELIM + dayName;
    
    int year, month, day;
    if (sscanf(dayName.c_str(), "%4d%2d%2d",
	       &year, &month, &day) != 3) {
      if (_isVerbose) {
	cerr << "    Could not get the date info "
	     << "from day name: " << dayName << endl;
      }
      continue;
    }

    if (_isVerbose) {
      cerr << "DsDataFile::_get_latest_forecast" << endl;
      cerr << "  Searching dataDir: " << dayDirPath << endl;
    }

    // loop through the entries, finding the latest one

    ReadDir dayDir;
    if (dayDir.open(dayDirPath.c_str())) {                 
      if (_isDebug) {
	cerr << "Could not open dir: " << dayDirPath << endl;
      }
      return NULL;
    }
    
    // Iterate through the entries, looking through the latest entry
    
    struct dirent * dayEnt;
    for (dayEnt = dayDir.read(); dayEnt != NULL; dayEnt = dayDir.read()) {
      
      // forecast gen times are 'g_hhmmss'
      
      int hour, min, sec;
      if (sscanf(dayEnt->d_name, "g_%2d%2d%2d",
		 &hour, &min, &sec) == 3) {
	if (_isVerbose) {
	  cerr << "  Found gentime entry: " << dayDirPath
	       << PATH_DELIM <<  dayEnt->d_name << endl;
	}
	DateTime entryTime(year, month, day, hour, min, sec);
	_get_latest_scan_leadtimes(dayDirPath, dayEnt->d_name,
				   fileSuffix, entryTime,
				   latestTime, latestLeadSecs, latestName);
      } else {
	if (_isVerbose) {
	  cerr << "  Rejecting gen time entry: " << dayDirPath
	       << PATH_DELIM <<  dayEnt->d_name << endl;
	}
      }
      
    } // dayEnt
    dayDir.close();
    
    // break out when we have gone back far enough
    count++;
    if (count > _maxForecastLeadDays) {
      break;
    }

  } // ii
  
  if (latestTime < 0) {
    if (_isVerbose) {
      cerr << "  No suitable file found: " << dataDir << endl;
    }
    return NULL;
  }
  
  if (_isVerbose) {
    cerr << "  latest path: " << dataDir
	 << PATH_DELIM << latestName << endl;
  }
  
  DateTime fileDateTime(latestTime -latestLeadSecs, latestLeadSecs);
  DsDataFile * latestFile = new DsDataFile(dataDir, fileDateTime,
					   fileSuffix, DS_FORECAST_FILE);
  
  return latestFile;

}


int DsDataFile::_get_latest_scan_leadtimes(const string &dayDirPath,
					   const string &entryName,
					   const string &fileSuffix,
					   const DateTime &entryTime,
					   time_t &latestTime,
					   int &latestLeadSecs,
					   string &latestName)

{

  // Iterate through the dir and look at all the files.

  ReadDir genDir;
  string genDirPath = dayDirPath + PATH_DELIM + entryName;
  
  if (_isVerbose) {
    cerr << "    _get_latest_scan_leadtimes: genDirPath: : "
	 << genDirPath << endl;
  }

  if (genDir.open(genDirPath.c_str())) {
    if (_isDebug) {
      cerr << "Could not open generate files dir: " << genDirPath << endl;
    }
    return -1;
  }
    
  // Iterate through the entries.
  
  struct dirent * genEnt;
  for (genEnt = genDir.read(); genEnt != NULL; genEnt = genDir.read()) {
    
    unsigned int leadSecs;
    
    // Skip files which are not of the form f_ssssssss.ext
    
    if (sscanf(genEnt->d_name, "f_%8u", &leadSecs) != 1) {
      if (_isVerbose) {
	cerr << "    Could not get lead secs info "
	     << "from file: " << genEnt->d_name << endl;
      }
      continue;
    }

    // check suffix
    
    if (fileSuffix.size() > 0) {
      char *suffix = strrchr(genEnt->d_name, '.');
      if (suffix == NULL || (strcmp(suffix + 1, fileSuffix.c_str()))) {
	if (_isVerbose) {
	  cerr << "  Rejecting, wrong suffix: " << genDirPath
	       << PATH_DELIM << genEnt->d_name << endl;
	  cerr << "  Required suffix: " << fileSuffix << endl;
	}
	continue;
      }
    }
    
    if (_isVerbose) {
      cerr << "    Found lead time entry: " << genDirPath
	   << PATH_DELIM << genEnt->d_name << endl;
    }

    // compute latest time

    time_t ltime = entryTime.utime() + leadSecs;
    if (ltime > latestTime) {
      latestTime = ltime;
      latestLeadSecs = leadSecs;
      latestName = entryName + PATH_DELIM + genEnt->d_name;
    }

  } // readdir
  genDir.close();

  return 0;

}
