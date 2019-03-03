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
// DsInputPath.cc : Input data system file handling
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1998
//
//////////////////////////////////////////////////////////
//
// See <didss/DsInputPath.hh> for details.
//
///////////////////////////////////////////////////////////

#include <cerrno>
#include <cstdio>
#include <string>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <sys/stat.h>
#include <ctype.h>

#include <toolsa/file_io.h>
#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <toolsa/Path.hh>
#include <toolsa/TaArray.hh>
#include <toolsa/TaStr.hh>
#include <didss/DsInputPath.hh>
#include <didss/RapDataDir.hh>
#include <didss/DataFileNames.hh>
using namespace std;


/////////////////////////////
// Constructor - Archive mode
//
// Pass in a list of file paths.
//

DsInputPath::DsInputPath (const string &prog_name,
			  bool debug,
			  int n_files,
			  char **file_paths)

{

  _init();
  _prog_name = prog_name;
  _debug = debug;
  _mode = ARCHIVE_MODE;
  
  // load up file paths map - paths sorted alphbetically
  
  for (int i = 0; i < n_files; i++) {
    char *path = file_paths[i];
    if (path != NULL) {
      if (ta_stat_exists(path)) {
        _insertPathPair(path, getDataTime(path));
      } else {
        cerr << "ERROR - DsInputPath::DsInputPath" << endl;
        cerr << "  File does not exist: " << path << endl;
      }
    }
  }

  // load up path vector for use in nextArchive() method
  
  for (PathTimeIter ii = _pathTimeList.begin();
       ii !=  _pathTimeList.end(); ii++) {
    _pathList.push_back(ii->first);
  }

  // initialize posn to start of map

  reset();

}

/////////////////////////////
// Constructor - Archive mode
//
// Pass in a vector of file paths.
//

DsInputPath::DsInputPath (const string &prog_name,
			  bool debug,
			  const vector<string> &file_paths,
                          bool sort /* = true */)

{

  _init();
  _prog_name = prog_name;
  _debug = debug;
  _mode = ARCHIVE_MODE;

  if (sort) {

    // load up file paths map
    
    for (size_t i = 0; i < file_paths.size(); i++) {
      const string &path = file_paths[i];
      if (ta_stat_exists(path.c_str())) {
        _insertPathPair(path, getDataTime(path));
      } else {
        cerr << "ERROR - DsInputPath::DsInputPath" << endl;
        cerr << "  File does not exist: " << path << endl;
      }
    }
    
    // load up path vector for use in nextArchive() method
    
    for (PathTimeIter ii = _pathTimeList.begin();
         ii !=  _pathTimeList.end(); ii++) {
      _pathList.push_back(ii->first);
    }

  } else {

    // load up path vector for use in nextArchive() method
    // use unsorted list

    _pathList = file_paths;

  }

  // initialize posn to start of map
  
  reset();
  
}

/////////////////////////////
// Constructor - Archive mode
//
// Pass in data directory and start and end times.
//

DsInputPath::DsInputPath (const string &prog_name,
			  bool debug,
			  const string &input_dir,
			  time_t start_time,
			  time_t end_time, bool use_fcst_gentime)

{

  _init();
  _prog_name = prog_name;
  _debug = debug;
  _input_dir = input_dir;

  if (use_fcst_gentime)
     _mode = ARCHIVE_FCST_GENTIME_MODE;
  else
     _mode = ARCHIVE_MODE;

  // load by time, if that fails load by day
  
  if (_load_pathlist_archive_by_time(_input_dir, start_time, end_time)) {
    _load_pathlist_archive_by_name(_input_dir, start_time, end_time, 0);
  }

  // load up path vector for use in nextArchive() method
  
  for (PathTimeIter ii = _pathTimeList.begin();
       ii !=  _pathTimeList.end(); ii++) {
    const string &path = ii->first;
    _pathList.push_back(path);
  }

  // initialize posn to start of map
  
  reset();
  
}

//////////////////////////////
// Constructor - realtime mode
//
// Pass in (a) the input directory to be watched.
//         (b) the max valid age for a realtime file (secs)
//             the routine will wait for a file with the age
//             less than this.
//         (c) pointer to heartbeat_func. If NULL this is ignored.
//             If non-NULL, this is called once per second while
//             the routine is waiting for new data.
//         (d) use_ldata_info flag. If true, we use the latest_data_info
//             file, if false we watch the directory recursively
//             for new files.
//         (e) latest_file_only flag. Only applies if use_ldata_info is
//             false. If set, the routine returns the latest file.
//             If false, it returns the earliest file which is younger than
//             the max valid age and which has not been used yet.
//
// If use_ldata_info is false, next() will recurse looking for files
// in subdirectories. However, you need to understand the way in which
// directory modify times are set. A directory modify time is set only when
// a file is added or removed from the directory, not if an existing file
// is modified. next() will traverse into a subdirectory if is has been 
// modified within the last max_file_age secs.

DsInputPath::DsInputPath (const string &prog_name,
			  bool debug,
			  const string &input_dir,
			  int max_file_age,
			  DsInput_heartbeat_t heartbeat_func,
			  bool use_ldata_info /* = true */,
			  bool latest_file_only /* = true */) :
        _ldata(input_dir)
  
{

  _init();
  _prog_name = prog_name;
  _debug = debug;
  _input_dir = input_dir;
  _max_file_age = max_file_age;
  _max_dir_age = max_file_age;
  _heartbeat_func = heartbeat_func;
  _use_ldata_info = use_ldata_info;
  _latest_file_only = latest_file_only;
  _file_quiescence_secs = 5;
  _mode = REALTIME_MODE;
  _timePathPosn = _timePathList.begin();
  reset();
  
}

///////////////////////////////
// Constructor - Triggered mode
//
// Pass in data directory.
//

DsInputPath::DsInputPath (const string &prog_name,
			  bool debug,
			  const string &input_dir):
        _ldata(input_dir, debug)
{

  _init();
  _prog_name = prog_name;
  _debug = debug;
  _input_dir = input_dir;
  _max_file_age = -1;
  _file_quiescence_secs = 5;
  _mode = TRIGGERED_MODE;
  _timePathPosn = _timePathList.begin();
  reset();
  
}

////////////////////////////////
// Default Constructor - private
//

DsInputPath::DsInputPath ()

{

  _init();
  reset();

}

/////////////
// Destructor

DsInputPath::~DsInputPath()
{

}

////////////////////////////////
// Initialize private members
//

void DsInputPath::_init()

{

  _check_sub_str = false;
  _mode = ARCHIVE_MODE;
  _max_file_age = -1;
  _max_dir_age = 300;
  _max_recursion_depth = 5;
  _dir_scan_sleep_secs = 5;
  _file_quiescence_secs = 5;
  _strict_scan = false;
  _recurse = true;
  _follow_links = true;
  _debug = false;
  _use_ldata_info = true;
  _latest_file_only = true;
  _latest_time_used = -1;
  _save_latest_read_info = false;
  _heartbeat_func = PMU_auto_register;
  _pathPosn = 0;
  _use_inotify = false;
  _inotifyFd = -1;
  
}

/////////////////////////
// reset to start of list
// 
// Archive mode only.

void DsInputPath::reset()

{
  _pathPosn = 0;
}

/////////////////////////
// step back by one entry
// 
// Archive mode only.
//
// Returns 0 on success, -1 if already at start of list

int DsInputPath::stepBack()

{
  if (_pathPosn == 0) {
    return -1;
  }
  _pathPosn--;
  return 0;
}

///////////////////////////////////////
// set the required data file extension
//
// If set, only files with this extension will be returned

void DsInputPath::setSearchExt(const string &search_ext)

{
  _search_ext = search_ext;
}

///////////////////////////////////////
// set a substring that must be in the filename
// for it to be considered valid.
//
// If set, only files with this string in
// their full path will be returned. Applies
// only to realtime mode with no ldata_info active.
// Added by Niles to read MM5 data from a directory in
// which files for several domains appeared at once.
//
void DsInputPath::setSubString(const string &subString)

{
  _check_sub_str = true;
  _sub_str = subString;
}


///////////////////////////////////////////////////////////
// set the directory scan sleep interval - secs
//
// This only applies to REALTIME mode, when the latest_data_info
// file is NOT being used. This is the time the search routine
// sleeps between each scan of the directory looking for
// new data. The default is 5, and the maximum is 50,
// so that the routine will register with the procmap
// at least once per minute.

void DsInputPath::setDirScanSleep(int dir_scan_sleep_secs /* = 5*/ )

{
  if (dir_scan_sleep_secs > 50) {
    _dir_scan_sleep_secs = 50;
  } else if (dir_scan_sleep_secs < 1) {
    _dir_scan_sleep_secs = 1;
  } else {
    _dir_scan_sleep_secs = dir_scan_sleep_secs;
  }
}

///////////////////////////////////////////////////////////
// When Set, Only scan subdirs that look like daily subdirs
//
// This only applies to REALTIME mode, when the latest_data_info
// file is NOT being used. Subdirs must begin with:
// YYYYMMDD or g_YYYYMMDD in order ro qualify for scanning.

void DsInputPath::setStrictDirScan(bool value /* =  true*/)
{
    _strict_scan = value;
}

///////////////////////////////////////////////////////////
// set the file quiescence time - secs
//
// This only applies to REALTIME mode, when the latest_data_info
// file is NOT being used. The next() routine, when searching for
// new files, waits for files to be quiescent before assuming they
// are complete on disk. This is the number of secs of quiescence
// we wait.

void DsInputPath::setFileQuiescence(int file_quiescence_secs /* = 5*/ )

{
  if (file_quiescence_secs < 1) {
    _file_quiescence_secs = 1;
  } else {
    _file_quiescence_secs = file_quiescence_secs;
  }
}

///////////////////////////////////////////////////////////
// Set max age for scanning subdirectories - secs.
//
// This only applies to REALTIME mode, when the latest_data_info
// file is NOT being used.
//
// Defaults to max_file_age from constructor.

void DsInputPath::setMaxDirAge(int age)

{
  _max_dir_age = age;
}

///////////////////////////////////////////////////////////
// set the recursion flag (default is true)
//
// This only applies to REALTIME mode, when the latest_data_info
// file is NOT being used. The next() routine, when searching for
// new files, will recurse down new directories unless the
// recursion flag is set to false.

void DsInputPath::setRecursion(bool recursionFlag /* = true*/ )
{
   _recurse = recursionFlag;
}

///////////////////////////////////////////////////////////
// set the follow_links flag (default is true)
//
// This only applies to REALTIME mode, when the latest_data_info
// file is NOT being used. The next() routine, when searching for
// new files, will decide whether to follow symbolic links
// based on the value of this flag

void DsInputPath::setFollowLinks(bool followLinksFlag /*= true*/ )
{
  _follow_links = followLinksFlag;
}

///////////////////////////////////////////////////////////
// set whether to use inotify for watching for new files
// default is true
//
// This only applies to REALTIME mode, when the latest_data_info
// file is NOT being used. The next() routine, when searching for
// new files, will use inotify instead of actively scanning the
// directories for new files. This is more efficient.
// The default is for this to state to be set.

void DsInputPath::setUseInotify(bool useInotifyFlag /*= true */)
{
  _use_inotify = useInotifyFlag;
}

///////////////////////////////////////////////////////////
// set the max recursion depth.
//
// Only applies is recursion is set on.
// This is the maximum depth, below input_dir, to which the
// recursive scan will be carried out.
// Defaults to 20.

void DsInputPath::setMaxRecursionDepth(int max_depth)
{
  _max_recursion_depth = max_depth;
}

///////////////////////////////////////////////////////////
// Option to set _latest_file_only flag
//
// If set to true, only the latest file found will be 
// added to the list of available files.
// If false, all files since the last active time will be
// added to the list.

void DsInputPath::setLatestFileOnly(bool latest_only /* = true */ )
{
  _latest_file_only = latest_only;
}

///////////////////////////////////////////////////////////
// Option to save the latest read info.
//
// If set to true, the latest read info will be saved out in
// a file to preserve state in case the application dies. On
// restart, the latest read info will be read in to re-initialize
// the object, so that data will not be lost.

void DsInputPath::setSaveLatestReadInfo(const string &label,
					bool save /* = true */)

{

  _save_latest_read_info = save;
  
  if (save) {

    if (_use_ldata_info) {

      _ldata.setSaveLatestReadInfo(label, _max_file_age, save);

    } else {

      // in this mode, we look for more than the latest file
      
      _latest_file_only = false;
      
      // set up the file for saving the latest read info
      
      _latest_read_info.setDir(RapDataDir.tmpLocation());
      _latest_read_info.setDebug(_debug);
      char name[MAX_PATH_LEN];
      sprintf(name, "latest_read_info.%s.%s",
	      label.c_str(), _input_dir.c_str());
      char *delim;
      while ((delim = strstr(name, PATH_DELIM)) != NULL) {
	for (size_t ii = 0; ii < strlen(PATH_DELIM); ii++) {
	  delim[ii] = '_';
	}
      }
      _latest_read_info.setLdataFileName(name);
      _latest_read_info.setUseFmq(false);
      _latest_read_info.setUseAscii(false);
      
      // try reading the file, to recover previous state
      
      if (_latest_read_info.readForced(_max_file_age) == 0) {
	_latest_time_used = _latest_read_info.getLatestValidTime();
	if (_debug) {
	  cerr << "---->> Recovering previous read state" << endl;
	  cerr << "Latest read info object:" << endl;
	  _latest_read_info.printFull(cerr);
	}
      }

    } // if (_use_ldata_info) 

  } // if (save) 

}

//////////////////////////////////////////////////
// get next file path
//
// Realtime and Archive modes only
//
// If block is true, blocks until data is available.
// If block is false, returns immediately if no data is available.
// Blocking is only applicable in Realtime mode.
//
// Returns the path of the next available file.
// If no file is available, returns NULL.

char *DsInputPath::next(bool block /* = true */)

{

  // loop until we get (a) a file which matches the extension and substr,
  // or (b) an error

  while (true) {

    switch(_mode)  {
      
    case ARCHIVE_MODE:case ARCHIVE_FCST_GENTIME_MODE:
      if (_nextArchive()) {
	return NULL;
      }
      break;
      
    case REALTIME_MODE :
      
      if (_use_ldata_info) {
	if (_nextRealtimeLdata(block)) {
	  return NULL;
	}
      } else if (_use_inotify) {
	if (_nextRealtimeInotify(block)) {
	  return NULL;
	}
      } else {
	if (_nextRealtimeNoLdata(block)) {
	  return NULL;
	}
      }
      break;
      
    case TRIGGERED_MODE :
      return NULL;
      
    } //  switch(_mode)

    if (_hasExt(_returned_path) && _hasSubStr(_returned_path)) {
      return (char *) _returned_path.c_str();
    }
    
  } // while

}

//////////////////////////////////////////////////////////////
// get latest file written to the input directory
//
// Triggered mode or realtime mode with latest_data_info only
//
// returns NULL on failure

char *DsInputPath::latest(bool force /* = true*/ )
{

  if (_mode == ARCHIVE_MODE || !_use_ldata_info) {
    cerr << "WARNING - DsInputPath::latest" << endl;
    cerr << "  latest() only applies to REALTIME and TRIGGERED mode "
	 << "with latest_data_info" << endl;
    return NULL;
  }

  // Get the latest file in the directory
  
  if (force) {
    if (_ldata.readForced(_max_file_age)) {
      return NULL;
    }
  } else {
    if (_ldata.read(_max_file_age)) {
      return NULL;
    }
  }
  
  // latest data - check file is there

  _returned_path = _ldata.getDataPath();
  
  if (ta_stat_is_file(_returned_path.c_str())) {
    return ((char *) _returned_path.c_str());
  }

  return NULL;

}

/////////////////////////////////////////////////////
// get new file written to the input directory since
// given last data time.
//
// Triggered mode or realtime mode with latest_data_info only
//
// returns NULL on failure

char *DsInputPath::new_data(time_t last_data_time)

{

  if (_mode == ARCHIVE_MODE || !_use_ldata_info) {
    cerr << "WARNING - DsInputPath::new_data" << endl;
    cerr << "  new_data() only applies to REALTIME and TRIGGERED mode "
	 << "with latest_data_info" << endl;
    return NULL;
  }

  // Get the latest file in the directory
  
  if (_ldata.readForced(_max_file_age)) {
    return NULL;
  }
  
  // See if the data is new.
  
  if (_ldata.getLatestTime() <= last_data_time) {
    return NULL;
  }
    
  // new data - check file is there

  _returned_path = _ldata.getDataPath();
  
  if (ta_stat_is_file(_returned_path.c_str())) {
    return ((char *) _returned_path.c_str());
  }

  return NULL;

}

/////////////////////////////////////////////////////////////////
// get closest file to given time within the given time limits.
//
// If more that 1 path has the same time, returns the last
// path with that time.
//
// On success, returns the name of the closest file.  This pointer
// points to memory which should NOT be freed by the caller.
// On failure, returns NULL.
//
// Returns the data_time of the file (based on the file name) in
// the argument list.

char *DsInputPath::getClosest(time_t search_time,
			      time_t start_time,
			      time_t end_time,
			      time_t *data_time) const

{

  // construct temporary object containing all paths
  // within the search time

  DsInputPath all(_prog_name, _debug, _input_dir,
		  start_time, end_time);
  
  // no paths found? return error
  
  if (all._pathTimeList.size() == 0) {
    return NULL;
  }
  
  // find the one closest in time to the search time
  
  PathTimeIter start = all._pathTimeList.begin();
  _returned_path = (*start).first;
  *data_time = (*start).second;
  int minDiff = abs(search_time - *data_time);
  
  PathTimeIter ii;
  for (ii = all._pathTimeList.begin(); ii != all._pathTimeList.end(); ii++) {
    int diff = abs(search_time - (*ii).second);
    if (diff <= minDiff) {
      _returned_path = (*ii).first;
      *data_time = (*ii).second;
      minDiff = diff;
    }
  } // ii
  
  return((char *) _returned_path.c_str());

}

/////////////////////////////////////////////////////////////////
// get closest file to given time within the given time margin
//
// If more that 1 path has the same time, returns the last
// path with that time.
//
// On success, returns the name of the closest file.  This pointer
// points to memory which should NOT be freed by the caller.
// On failure, returns NULL.
//
// Returns the data time of the file (based on the file name) in
// the argument list.

char *DsInputPath::getClosest(time_t search_time,
			      int max_time_offset,
			      time_t *data_time) const
{

  return getClosest(search_time,
		    search_time - max_time_offset,
		    search_time + max_time_offset,
		    data_time);

}

////////////////////////////////////////////////////////////////////
// get closest file to given time within the given time margin, wait
// if there is currently no data within the time margin.
//
// If more that 1 path has the same time, returns the last
// path with that time.
//
// On success, returns the name of the closest file.  This pointer
// points to memory which should NOT be freed by the caller.
// On failure, returns NULL.
//
// Returns the data time of the file (based on the file name) in
// the argument list.

char *
DsInputPath::getClosestBlocking(time_t search_time,
				int max_time_offset,
				DsInput_heartbeat_t heartbeat_func,
				time_t *data_time) const
{

  // can we get closest immediately?
  
  char *closestPath =
    getClosest(search_time, max_time_offset, data_time);
  if (closestPath != NULL) {
    return closestPath;
  }

  // Wait for more data until we pass the maximum data time

  time_t now = time(NULL);
  time_t expire_time = search_time + max_time_offset;
  while (now < expire_time) {

    if (_ldata.readForced(_max_file_age) == 0) {
      time_t latest_time = _ldata.getLatestTime();
      if (abs(latest_time - search_time) < max_time_offset) {
	*data_time = latest_time;
	_returned_path = _ldata.getDataPath();
	return((char *) _returned_path.c_str());
      }
    }
    
    if (heartbeat_func ) {
      heartbeat_func("DsInputPath::getClosestBlocking: waiting for data");
    }
    
    sleep(1);
    now = time(NULL);

  } // while
  
  return NULL;

}

//////////////////////////////////////////////////////////////////
// get latest data file before the given time, but within the
// given offset.
//
// If more that 1 path has the same time, returns the last
// path with that time.
//
// On success, returns the name of the closest file.  This pointer
// points to static memory which should not be freed by the caller.
// On failure, returns NULL.
//
// Returns the data time of the file (based on the file name) in
// the argument list.

char *DsInputPath::getFirstBefore(time_t search_time,
				  int max_time_offset,
				  time_t *data_time) const
{

  return getClosest(search_time,
		    search_time - max_time_offset,
		    search_time,
		    data_time);
  
}

///////////////////////////////////////////////////////////////
// get latest data file after the given time, but within the
// given offset.
//
// If more that 1 path has the same time, returns the last
// path with that time.
//
// On success, returns the name of the closest file.  This pointer
// points to memory which should NOT be freed by the caller.
// On failure, returns NULL.
//
// Returns the data time of the file (based on the file name) in
// the argument list.

char *DsInputPath::getFirstAfter(time_t search_time,
				 int max_time_offset,
				 time_t *data_time) const
{

  return getClosest(search_time,
		    search_time,
		    search_time + max_time_offset,
		    data_time);
  
}

/////////////////////////////////////////////////////////////////////
// Get the data time information from the given file path.
//
// The following formats are supported:
//
//     */yyyymmdd/g_hhmmss/f_llllllll.ext
//     */yyyymmdd/hhmmss.ext
//     */*yyyymmdd?hhmmss*
//     */*yyyymmddhhmmss*
//     */*yyyymmddhhmm*
//     */*yyyymmddhh.tmhhmm (mm5 forecast)
//     */*yyyymmddhh*
//     */*yyyyjjjhh*
//     */*yyyymmdd?hhmm*
//     */*yyyymmdd?hh*
//     */*yyyymmdd*
//     */*yyjjj*
//
// Returns 0 on success, -1 on error.
// On success, sets data_time.

int DsInputPath::getGenTime(const string& file_path,
			    time_t &data_time)
{
  bool date_only;
  return DataFileNames::getDataTime(file_path, data_time, date_only,true);
}


int DsInputPath::getGenTime(const string& file_path,
			     time_t &data_time,
			     bool &date_only)
{
  return DataFileNames::getDataTime(file_path, data_time, date_only,true);
}

int DsInputPath::getDataTime(const string& file_path,
			     time_t &data_time)
{
  bool date_only;
  return DataFileNames::getDataTime(file_path, data_time, date_only);
}

int DsInputPath::getDataTime(const string& file_path,
			     time_t &data_time,
			     bool &date_only)
{
  return DataFileNames::getDataTime(file_path, data_time, date_only);
}

/////////////////////////////////////////////////////////////////////
// Get the data time information from the given file path.
// Returns file time on success, -1 on error.

time_t DsInputPath::getDataTime(const string &file_path)
{
  time_t data_time;
  if (getDataTime(file_path, data_time)) {
    return -1;
  } else {
    return data_time;
  }
}

time_t DsInputPath::getDataTime(const char *file_path)
{
  if (file_path == NULL) {
    return -1;
  }
  time_t data_time;
  if (getDataTime(file_path, data_time)) {
    return -1;
  } else {
    return data_time;
  }
}

/////////////////////////////////////////////////////////////
// try loading up path list using time-based file naming
//
// Returns 0 on success, -1 on failure

int DsInputPath::_load_pathlist_archive_by_time(const string &input_dir,
						time_t start_time,
						time_t end_time)
  
{

  // compute start and end day
  
  int start_day = start_time / 86400;
  int end_day = end_time / 86400;

  // initialize flags

  bool haveValidTimeFormat = false;
  bool haveForecastFormat = false;

  // loop through days

  for (int iday = start_day; iday <= end_day; iday++) {
    
    // load up file paths for this day
    
    _load_pathlist_day(input_dir,
		       iday, start_time, end_time,
		       haveValidTimeFormat, haveForecastFormat);
    
  } // endfor - iday

  // if we have forecast data, go back through previous days
  // until we do not add any extra paths

  if (haveForecastFormat) {
    size_t nSoFar = 0;
    int iday = start_day - 1;
    while ((nSoFar < _pathTimeList.size()) && (iday > start_day - 30)) {
      nSoFar = _pathTimeList.size();
      _load_pathlist_day(input_dir,
			 iday, start_time, end_time,
			 haveValidTimeFormat, haveForecastFormat);
      iday--;
    } // while
  } // if (haveForecastFormat)
  
  if (_pathTimeList.size() > 0 || haveValidTimeFormat || haveForecastFormat) {
    return 0;
  } else {
    return -1;
  }

}
  
/////////////////////////////////////////////////////////////
// try loading up paths by name, recursively searching
// directories
//
// Returns 0 on success, -1 on failure

int DsInputPath::_load_pathlist_archive_by_name(const string &input_dir,
						time_t start_time,
						time_t end_time,
						int depth)
  
{
  
  // check we don't recurse too far
  
  if (depth > _max_recursion_depth) {
    return 0;
  }
  
  DIR *dirp;
  if ((dirp = opendir(input_dir.c_str())) == NULL) {
    return -1;
  }

  struct dirent *dp;
  for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
    
    // exclude dir entries and files beginning with '.'

    if (dp->d_name[0] == '.')
      continue;

    // compute path

    string path = input_dir;
    path += PATH_DELIM;
    path += dp->d_name;

    if (ta_stat_is_dir(path.c_str())) {
      
      // directory, so call recursively if it passes test

      if(_scanThisDir(dp->d_name, -1)) {
	_load_pathlist_archive_by_name(path, start_time, end_time, depth + 1);
      }
      
    } else {
      
      // file - check time
      
      if (_mode == ARCHIVE_FCST_GENTIME_MODE){
	time_t gen_time;
	if (getGenTime(path, gen_time) == 0) {
	  if (gen_time >= start_time && gen_time <= end_time) {
	    _insertPathPair(path, gen_time);
	  }
	}
      }
      else{
	time_t data_time;
	if (getDataTime(path, data_time) == 0) {
	  if (data_time >= start_time && data_time <= end_time) {
	    _insertPathPair(path, data_time);
	  }
	}
      }
      
    } // if (ta_stat_is_dir ...
    
  } // endfor - dp
  
  closedir(dirp);
  
  if (_pathTimeList.size() > 0) {
    return 0;
  } else {
    return -1;
  }

}
  
/////////////////////////////////////////////////////////////
// load up paths for a given day between start and end times
//
// have_valid_time_format and have_forecast_format are
// mutually exclusive - we go with the first type we
// find

void DsInputPath::_load_pathlist_day(const string &input_dir,
				     int day_num,
				     time_t start_time,
				     time_t end_time,
				     bool &have_valid_time_format,
				     bool &have_forecast_format)
  
{

  date_time_t day_time;
  day_time.unix_time = day_num * SECS_IN_DAY;
  uconvert_from_utime(&day_time);
  
  // compute subdir path
  
  char daydir_path[MAX_PATH_LEN];
  sprintf(daydir_path, "%s%s%.4d%.2d%.2d",
	  _input_dir.c_str(), PATH_DELIM,
	  day_time.year, day_time.month, day_time.day);
  
  // load up file paths for this day
    
  DIR *dirp;
  if ((dirp = opendir(daydir_path)) == NULL) {
    return;
  }
  
  struct dirent *dp;
  for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {

    // exclude dir entries and files beginning with '.'

    if (dp->d_name[0] == '.')
      continue;
    
    // check for forecast generate time format if we have not yet had
    // valid time data
    
    if (!have_valid_time_format) {

      int hour, min, sec;
      if (sscanf(dp->d_name, "g_%2d%2d%2d",
		 &hour, &min, &sec) == 3) {
	string gendir_path = daydir_path;
	gendir_path += PATH_DELIM;
	gendir_path += dp->d_name;
	_load_gen(gendir_path, start_time, end_time);
	have_forecast_format = true;
	continue;
      }

    } // if (!have_valid_time_format) 
    
    // check for valid time format

    if (!have_forecast_format) {

      // compute path and check that time is within limits

      string path = daydir_path;
      path += PATH_DELIM;
      path += dp->d_name;
      
      time_t data_time;
      bool date_only;
      if (getDataTime(path, data_time, date_only) == 0) {
	if (!date_only) {
	  if (data_time >= start_time && data_time <= end_time) {
	    // insert in map
	    _insertPathPair(path, data_time);
	  }
	  // set valid format flag, so we don't need to check for
	  // forecast format in future calls
	  have_valid_time_format = true;
	}
      }
    } // if (!have_forecast_format)

  } // endfor - dp
  
  closedir(dirp);
  
  return;
  

}

//////////////////////////////////////////////////////////////////
// load up paths for a given gen time, between start and end times

void DsInputPath::_load_gen(const string &gendir_path,
			    time_t start_time,
			    time_t end_time)
  
{
  
  DIR *dirp;
  if ((dirp = opendir(gendir_path.c_str())) == NULL) {
    return;
  }
  
  struct dirent *dp;
  for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {

    // exclude dir entries and files beginning with '.'

    if (dp->d_name[0] == '.')
      continue;
    
    // check for lead time format
    
    int lead_time;
    if (sscanf(dp->d_name, "f_%8d", &lead_time) != 1) {
      continue;
    }

    string path = gendir_path;
    path += PATH_DELIM;
    path += dp->d_name;
    
    
    if ( _mode == ARCHIVE_FCST_GENTIME_MODE){
      time_t gen_time;
      if (getGenTime(path, gen_time) == 0) {
	if (gen_time >= start_time && gen_time <= end_time) {
	  // insert in map
	  _insertPathPair(path, gen_time);
	}
      }
    }
    else{
      time_t data_time;
      if (getDataTime(path, data_time) == 0) {
	if (data_time >= start_time && data_time <= end_time) {
	  // insert in map
	  _insertPathPair(path, data_time);
	}
      }  
    }
  } // endfor - dp
  
  closedir(dirp);
  
  return;
  
}

////////////////////////////////////
// get next file path - archive mode
//
// Returns 0 on success, -1 on failure
// On success loads path in _returned_path

int DsInputPath::_nextArchive()

{

  // go through the file name vector
  
  while (_pathPosn != (int) _pathList.size()) {
    
    const string &path = _pathList[_pathPosn];
    _pathPosn++;
    
    // Make sure the file exists before returning it
    
    if (ta_stat_is_file(path.c_str())) {
      _returned_path = path;
      return 0;
    }
    
    // File does not exist
    // Check to see if the filename is that of a compressed
    // file, and if so, has the file been uncompressed. If so,
    // return the uncompressed file name.

    size_t len = path.size();

    // has it been compressed?
    
    _returned_path = path + ".Z";
    if (ta_stat_is_file(_returned_path.c_str())) {
      return 0;
    }

    // has it been gzipped?

    _returned_path = path + ".gz";
    if (ta_stat_is_file(_returned_path.c_str())) {
      return 0;
    }

    // look for .gz ext - has it been uncompressed?
    
    size_t gzpos = path.find(".gz", len - 3);
    if (gzpos != string::npos) {
      _returned_path.assign(path, 0, len - 3);
      if (ta_stat_is_file(_returned_path.c_str())) {
	return 0;
      }
    }
    
    // look for .Z ext - has it been uncompressed?
    
    size_t zpos = path.find(".Z", len - 2);
    if (zpos != string::npos) {
      _returned_path.assign(path, 0, len - 2);
      if (ta_stat_is_file(_returned_path.c_str())) {
	return 0;
      }
    }

  } // while
    
  // If we reach this point, we have returned all of the
  // existing files.

  return -1;

}

/////////////////////////////////////////////////////
// get next file path - realtime mode - ldata active
//
// Returns 0 on success, -1 on failure
// On success loads path in _returned_path

int DsInputPath::_nextRealtimeLdata(bool block)

{

  while (true) {

    // in realtime mode wait for change in latest info
    // sleep 1 second between tries.
    
    if (block) {
      _ldata.readBlocking(_max_file_age, 1000, _heartbeat_func);
    } else {
      if (_ldata.read(_max_file_age)) {
	return -1;
      }
    }
    
    // new data

    _returned_path = _ldata.getDataPath();
    if(_returned_path != "unknown") {
      return 0;
    }
    
    // no luck

    cerr << "ERROR - DsInputPath::_nextRealtimeLdata" << endl;
    cerr << "  Problem reading latest data from dir: "
         << _input_dir << endl;
    cerr << "Ldata triggers, but no corresponding files." << endl;
    
    if (_heartbeat_func) {
      _heartbeat_func("DsInputPath: waiting for realtime files");
    }

  }

  return -1;

}

////////////////////////////////////////////////////////
// get next file path - realtime mode - ldata not active
// using inotify

// Returns 0 on success, -1 on failure
// On success loads path in _returned_path

int DsInputPath::_nextRealtimeInotify(bool block)

{

  // NOTE - MAX OSX does not support inotify

#ifdef __APPLE__

  return _nextRealtimeNoLdata(block);

#else

  while (true) {

    // first check if we already have files in the queue

    if (_inotifyFileQueue.size() > 0) {
      
      // take next file in list
      
      _returned_path = _inotifyFileQueue.back();
      _inotifyFileQueue.pop_back();
      _latest_time_used = (*_timePathPosn).first;
      _timePathPosn++;

      if (_debug) {
        cerr << "  -->> next path: " << _returned_path << endl;
        cerr << "  -->> latest_time_used: "
             << utimstr(_latest_time_used) << endl;
      }
      
      // save state?
      
      if (_save_latest_read_info) {
        
        // strip the dir from the returned path to get relative file path
	
	string relDataPath;
	Path::stripDir(_input_dir, _returned_path, relDataPath);
	_latest_read_info.setRelDataPath(relDataPath.c_str());
	if (_latest_read_info.write(_latest_time_used)) {
	  cerr << "WARNING - DsInputPath::_nextRealtimeNoLdata" << endl;
	  cerr << "  Cannot write _latest_read_info file to keep state" << endl;
	}
        
      } // if (_save_latest_read_info)
      
      // fill out LdataInfo
      
      _fillLdataInfo(_returned_path.c_str());
      
      return 0;
      
    } // if (_timePathPosn != _timePathList.end())

    // no files in list, need to watch for new files
    // this will add any new files to the queue
    
    int iret = _loadTimelistInotify(block);
    if (_inotifyFileQueue.size() > 0) {
      // first time through, we loaded files from scanning dir
      continue;
    }

    // check for error

    if (iret == -2) {
      // error - so use dir read instead
      cerr << "  Using nextRealtimeNoLdata() instead" << endl;
      _use_inotify = false;
      return _nextRealtimeNoLdata(block);
    }

    // check for timeout

    if (iret == -1) {
      // timeout
      if (!block) {
        return -1;
      }
    }
    
  } // while

  // NOTE - MAX OSX does not support inotify

#endif // ifdef __APPLE__
    
}

////////////////////////////////////////////////////////
// get next file path - realtime mode - ldata not active
//
// Returns 0 on success, -1 on failure
// On success loads path in _returned_path

int DsInputPath::_nextRealtimeNoLdata(bool block)

{

  while (true) {

    // no latest_data_info
    // recursively search the directory for new files

    // first load up list of available files
    
    if (_timePathPosn == _timePathList.end()) {
      _load_timelist_realtime(_input_dir, 0);
    }
    
    if (_timePathPosn != _timePathList.end()) {

      // take next file in list

      _returned_path = (*_timePathPosn).second;
      _latest_time_used = (*_timePathPosn).first;
      _timePathPosn++;

      if (_debug) {
        cerr << "  -->> next path: " << _returned_path << endl;
        cerr << "  -->> latest_time_used: "
             << utimstr(_latest_time_used) << endl;
      }
      
      // save state?
      
      if (_save_latest_read_info) {
        
        // strip the dir from the returned path to get relative file path
	
	string relDataPath;
	Path::stripDir(_input_dir, _returned_path, relDataPath);
	_latest_read_info.setRelDataPath(relDataPath.c_str());
	if (_latest_read_info.write(_latest_time_used)) {
	  cerr << "WARNING - DsInputPath::_nextRealtimeNoLdata" << endl;
	  cerr << "  Cannot write _latest_read_info file to keep state" << endl;
	}

      } // if (_save_latest_read_info)

      // fill out LdataInfo
      
      _fillLdataInfo(_returned_path.c_str());

      return 0;
      
    } // if (_timePathPosn != _timePathList.end())

    // return with error now if non-blocking
    
    if (!block) {
      return -1;
    }
    
    if (_heartbeat_func) {
         _heartbeat_func("DsInputPath - waiting for files");
    }
    if (_debug) {
      cerr << "-->> Sleeping between scans, _dir_scan_sleep_secs: "
	   << _dir_scan_sleep_secs << endl;
    }
    for (int ii = 0; ii < _dir_scan_sleep_secs; ii++) {
      if(_heartbeat_func){
         _heartbeat_func("DsInputPath - waiting for files");
      }
      umsleep(1000);
    }

  } // while
    
}

////////////////////////////////////////////////////////////
// Load time list in realtime mode
//

void DsInputPath::_load_timelist_realtime(const string &input_dir,
					  int depth)
  
{

  // do we have files left from last time
  
  if (depth == 0) {
    if (_timePathPosn != _timePathList.end()) {
      // still files left in the pathlist from last time
      return;
    }
  }

  // check we don't recurse too far
  
  if (depth > _max_recursion_depth) {
    return;
  }
  
  DIR *dirp;
  if ((dirp = opendir(input_dir.c_str())) == NULL) {
    if (_debug) {
      int errNum = errno;
      cerr << "ERROR: DsInputPath::_load_timelist_realtime" << endl;
      cerr << "  Cannot open dir: " << input_dir << endl;
      cerr << "  " << strerror(errNum) << endl;
    }
    return;
  }
  
  // read directory looking for data files to be transferred

  time_t now = time(NULL);
  struct dirent *dp;
  for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
    
    // exclude dir entries and files beginning with '.' and '_'
    
    if (dp->d_name[0] == '.') {
      continue;
    }
    if (dp->d_name[0] == '_') {
      continue;
    }
    if (strstr(dp->d_name, "latest_data_info") != NULL) {
      continue;
    }

    // check file time
    
    char filePath[MAX_PATH_LEN];
    sprintf(filePath, "%s%s%s", input_dir.c_str(), PATH_DELIM, dp->d_name);
    struct stat fileStat;
    if (ta_stat(filePath, &fileStat)) {
      if (_debug) {
	int errNum = errno;
	cerr << "WARNING: DsInputPath::_load_timelist_realtime" << endl;
	cerr << "  Cannot stat file: " << filePath << endl;
	cerr << "  " << strerror(errNum) << endl;
      }
      continue;
    }

    time_t file_time = fileStat.st_mtime;
    if (!S_ISDIR(fileStat.st_mode)) {
      if (file_time < _latest_time_used) {
	continue;
      }
    }

    // check links

    if (!_follow_links) {
      if (S_ISLNK(fileStat.st_mode)) {
	if (_debug) {
	  cerr << "-->> Ignoring symbolic link dir, depth, name: "
	       << input_dir << ", " << depth << ", " << dp->d_name << endl;
	}
	continue;
      }
    }
    
    // compute age

    int age = now - file_time;

    // for directories, search recursively, if pass strict test
    
    if (S_ISDIR(fileStat.st_mode)) {
      if (_recurse && _scanThisDir(dp->d_name, age)) {
	char dirPath[MAX_PATH_LEN];
	sprintf(dirPath, "%s%s%s", input_dir.c_str(), PATH_DELIM, dp->d_name);
	if (_debug) {
	  cerr << "-->> Scanning dir, age, depth: "
	       << dirPath << ", " << age << ", " << (depth + 1) << endl;
	}
	_load_timelist_realtime(dirPath, depth + 1);
      }
      continue;
    }
    
    if (!S_ISREG(fileStat.st_mode)) {
      continue;
    }

    if (age > _max_file_age || age < _file_quiescence_secs) {
      continue;
    }
    
    // Check substring and extension if appropriate

    if (!_hasSubStr(dp->d_name)) {
      continue;
    }

    if (!_hasExt(dp->d_name)) {
      continue;
    }
    
    // add to path list
    
    _insertTimePair(file_time, filePath);
    
  } // dp
  
  closedir(dirp);

  if (depth == 0) {

    // remove any entries from previous list
    
    bool done = false;
    while (!done) {
      TimePathIter ii, jj;
      done = true;
      for (ii = _timePathList.begin(); ii != _timePathList.end(); ii++) {
	for (jj = _prevPathTimeList.begin(); jj != _prevPathTimeList.end(); jj++) {
	  if ((*ii).first == (*jj).first) {
	    _timePathList.erase(ii);
	    done = false;
	    break; // from jj
	  } // if
	} // jj
	if (!done) {
	  break; // from ii
	}
      } // ii
    } // while
    
    // if current path list is empty but prev path list is not, then
    // at least a second has gone by since a file arrived.
    // Therefore increment _latest_time_used by 1 so that the previous
    // files are not found again on the next scan

    if (_timePathList.size() == 0 && _prevPathTimeList.size() > 0) {
      _latest_time_used++;
    }

    // copy current list to previous list, for checking next time
    
    _prevPathTimeList = _timePathList;
    
    // trim list is required
    
    if (_latest_file_only) {
      while (_timePathList.size() > 1) {
	_timePathList.erase(_timePathList.begin());
      }
    }
      
    // set position to start of pathlist
    
    _timePathPosn = _timePathList.begin();

    if (_debug) {
      cerr << "DsInputPath completed scan of directory" << endl;
      cerr << "dir: " << input_dir << endl;
      cerr << "     _timePathList.size(): " << _timePathList.size() << endl;
      TimePathIter ii;
      for (ii = _timePathList.begin(); ii != _timePathList.end(); ii++) {
	cerr << "      path, time: "
	     << (*ii).second << ", "
	     << utimstr((*ii).first) << endl;
      } // ii
      cerr << "    _latest_time_used: " << utimstr(_latest_time_used) << endl;
    }

  }
  
}


////////////////////////////////////
// test for file extension, if set
//

bool DsInputPath::_hasExt(const string &path)

{

  // extension set?

  if (_search_ext.size() == 0) {
    return true;
  }

  // comma-delimited list? or single value?

  if (_search_ext.find(",") == string::npos) {
    return _hasExt(path, _search_ext);
  }

  // extension is a comma-delimited list
  // tokenize it

  vector<string> extensions;
  TaStr::tokenize(_search_ext, ",", extensions);
  if (extensions.size() < 1) {
    return true;
  }

  // check eack of the extensions

  for (size_t ii = 0; ii < extensions.size(); ii++) {
    if (_hasExt(path, extensions[ii])) {
      return true;
    }
  }

  // did not find it

  return false;

}

////////////////////////////////////
// test for file extension, if set
//

bool DsInputPath::_hasExt(const string &path, string ext)

{

  if (ext.size() == 0) {
    return true;
  }

  size_t extLen = ext.size();

  try {

    // uncompressed file
    
    string thisExt = path.substr(path.size() - extLen, extLen);
    if (thisExt == ext) {
      return true;
    }
    
    // gzipped file
    
    string gzipExt = path.substr(path.size() - 3, 3);
    thisExt = path.substr(path.size() - extLen - 3, extLen);
    if (gzipExt == ".gz" && thisExt == ext) {
      return true;
    }
    
    // compressed file
    
    string zExt = path.substr(path.size() - 2, 2);
    thisExt = path.substr(path.size() - extLen - 2, extLen);
    if (zExt == ".Z" && thisExt == ext) {
      return true;
    }

  } catch (out_of_range err) {

    // extension length exceeds path len
    
  }
    
  if (_debug) {
    cerr << "DEBUG - DsInputPath::_hasExt" << endl;
    cerr << "  Does not have extension: " << ext << endl;
    cerr << "  Path: " << path << endl;
  }

  return false;

}


//////////////////////////////////////
// test for file sub string, if set
//

bool DsInputPath::_hasSubStr(const string &path)

{

  if (!_check_sub_str) {
    return true;
  }

  if (_sub_str.size() == 0) {
    return true;
  }

  // get file name

  Path pathObj(path);
  string name = pathObj.getFile();

  // does this contain the substr?

  if (name.find(_sub_str) == string::npos) {
    if (_debug) {
      cerr << "DEBUG - DsInputPath::_hasSubStr" << endl;
      cerr << "  Does not contain sub-string: " << _sub_str << endl;
      cerr << "  Path: " << path << endl;
    }
    return false;
  }

  return true;

}

////////////////////////////////////
// test for scanning subdirectory

bool DsInputPath::_scanThisDir(char* name, int age)

{

  if (_max_dir_age > 0 && age > _max_dir_age) {
    return false;
  }

  // never include CVS dirs
  
  if (strncmp(name, "CVS", 3) == 0) {
    return false;
  }
  
  // exclude dir entries beginning with '.'
  
  if (name[0] == '.') {
    return false;
  }
  
  // Always passes if no strict check requested.
  
  if(_strict_scan == false) {
    return true;
  }

  // Check for Forecast Subdirs.
  char *ptr = name;
  if(strncmp(ptr,"g_",2) == 0) {
    ptr += 2;
  }

  // Throw out short dirs (not needed, next test will find the NULL)
  // if(strlen(ptr) < 8) {
  //   return false;
  // }

  // Check to make sure the next 8 chars are digits.
  for(int i = 0; i < 8; i++, ptr++) {
    if(! isdigit(*ptr)) {
      return false;
    }
  }
  
  // Passes all the tests.

  return true;

}


////////////////////////////////////////////////
// Insert the path/time pair, provided the file
// exists and is a plain file or link.
//

void DsInputPath::_insertPathPair(const string &path, time_t file_time)
  
{

  // insert if file exists

  if (ta_stat_is_file(path.c_str())) {
    PathTimePair pathTime;
    pathTime.first = path;
    pathTime.second = file_time;
    _pathTimeList.insert(pathTime);
  }

}

////////////////////////////////////////////////
// Insert the time/path pair, provided the file
// exists and is a plain file or link.
//

void DsInputPath::_insertTimePair(time_t file_time, const string &path)
  
{

  // insert if file exists

  if (ta_stat_is_file(path.c_str())) {
    TimePathPair timePath;
    timePath.first = file_time;
    timePath.second = path;
    _timePathList.insert(timePath);
  }

}

///////////////////////////////////////////////////////////
// Fill out the LdataInfo member.
//
// This is done for the case in which no _latest_data_info
// file is available. However, clients may wish to use the
// LdataInfo member.

void DsInputPath::_fillLdataInfo(const char *latest_path)
  
{

  // try to decode time from file name.
  // if not successful, use modify time

  time_t ltime = time(NULL);
  if (getDataTime(latest_path, ltime)) {
    // stat the file
    struct stat lstat;
    if (ta_stat(latest_path, &lstat) == 0) {
      ltime = lstat.st_mtime;
    }
  }

  // strip the dir from the returned path to get relative file path
  
  string relDataPath;
  Path::stripDir(_input_dir, latest_path, relDataPath);

  // set the _ldata fields which are applicable

  _ldata.setLatestTime(ltime);
  _ldata.setRelDataPath(relDataPath.c_str());
  Path lpath(latest_path);
  _ldata.setDataFileExt(lpath.getExt().c_str());
  _ldata.setIsFcast(false);

}

///////////////////////////////////////////////////////
// NOTE - MAC OSX does not support inotify

#ifndef __APPLE__

////////////////////////////////////////////////////////
// load up list of new files using inotify
// Returns 0 on success, -1 on timeout, -2 on failure

int DsInputPath::_loadTimelistInotify(bool block)

{
  
  if (_inotifyFd < 0) {
    if (_inotifyInit()) {
      return -2;
    }
    if (_inotifyFileQueue.size() > 0) {
      // startup files available
      return 0;
    }
  }
  
  int bufLen = (100 * (sizeof(struct inotify_event) + 1024 + 1));
  TaArray<char> buf_;
  char *buf = buf_.alloc(bufLen);
  
  while (true) {
    
    // select on file descriptor
    
    int iret =
      ta_fd_read_select(_inotifyFd, _dir_scan_sleep_secs * 1000);
    
    // ERROR?
    
    if (iret < -1) {
      int errNum = errno;
      cerr << "ERROR - DsInputPath::_loadTimelistInotify" << endl;
      cerr << "  Cannot select on inotify file descriptore" << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -2;
    }

    // TIMEOUT?
    
    if (iret == -1) {
      if (_debug) {
        cerr << "==>> DsInputPath::_loadTimelistInotify, timeout ..." << endl;
      }
      // return with timeout code now if non-blocking
      if (!block) {
        return -1;
      }
      // call heartbeat function
      if (_heartbeat_func) {
        _heartbeat_func("DsInputPath - waiting for files");
      }
      continue;
    }
    
    // READ INOTIFY EVENTS

    ssize_t numRead = read(_inotifyFd, buf, bufLen);
    if (numRead < 1) {
      int errNum = errno;
      cerr << "ERROR - DsInputPath::_loadTimelistInotify" << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -2;
    }
    if (_debug) {
      cerr << "==>> Read " << numRead << " bytes from inotify fd: " << _inotifyFd << endl;
    }
     
    /* Process all of the events in buffer returned by read() */
    
    for (char *p = buf; p < buf + numRead; ) {
      struct inotify_event *event = (struct inotify_event *) p;
      _handleInotifyEvent(event);
      p += sizeof(struct inotify_event) + event->len;
    }

    return 0;
     
  } // while
 
  return -2;

}

///////////////////////////////////////////////////////////
// Initialize inotify mode

int DsInputPath::_inotifyInit()
  
{

  // create inotify instance

  _inotifyFd = inotify_init();
  if (_inotifyFd == -1) {
    int errNum = errno;
    cerr << "ERROR - DsInputPath::_inotifyInit()" << endl;
    cerr << "ERROR in inotify_init()" << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  // load up list of dirs to watch

  vector<string> dirList;
  _loadInotifySubDirs(_input_dir, 0, dirList);

  int watchFlags = IN_CLOSE_WRITE | IN_MOVED_TO | IN_DELETE_SELF | IN_CREATE;
  for (size_t ii = 0; ii < dirList.size(); ii++) {
    int wd = inotify_add_watch(_inotifyFd, dirList[ii].c_str(), watchFlags);
    if (wd >= 0) {
      if (_debug) {
        cerr << "==>> watching dir: " << dirList[ii] << ", using wd: " << wd << endl;
      }
      _watchList[wd] = dirList[ii];
    }
  }

  return 0;

}
   
////////////////////////////////////////////////////////////
// load list of directories to watch
//

void DsInputPath::_loadInotifySubDirs(const string &dir,
                                      int depth,
                                      vector<string> &dirList)
  
{
  
  // initialize with depth 0 dir
  
  if (depth == 0) {
    dirList.push_back(dir);
  }
  
  // check we don't recurse too far
  
  if (depth > _max_recursion_depth) {
    return;
  }
  
  DIR *dirp;
  if ((dirp = opendir(dir.c_str())) == NULL) {
    if (_debug) {
      int errNum = errno;
      cerr << "WARNING: DsInputPath::_loadInotifySubDirs" << endl;
      cerr << "  Cannot open dir: " << dir << endl;
      cerr << "  " << strerror(errNum) << endl;
    }
    return;
  }
  
  // read directory looking for sub dirs

  time_t now = time(NULL);
  struct dirent *dp;
  for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {

    // exclude dir entries and files beginning with '.'
    
    if (dp->d_name[0] == '.') {
      continue;
    }

    // exclude entries that start with _
    
    if (dp->d_name[0] == '_') {
      if (_debug) {
        cerr << "Ignoring entries starting with '_': " << dp->d_name << endl;
      }
      continue;
    }

    // check file type
    
    char entryPath[MAX_PATH_LEN];
    sprintf(entryPath, "%s%s%s", dir.c_str(), PATH_DELIM, dp->d_name);
    struct stat fileStat;
    if (ta_stat(entryPath, &fileStat)) {
      if (_debug) {
	int errNum = errno;
        cerr << "WARNING: DsInputPath::_loadInotifySubDirs" << endl;
	cerr << "  Cannot stat file: " << entryPath << endl;
	cerr << "  " << strerror(errNum) << endl;
      }
      continue;
    }
    
    // check links
    
    if (!_follow_links) {
      if (S_ISLNK(fileStat.st_mode)) {
	if (_debug) {
	  cerr << "-->> Ignoring symbolic link dir: " << entryPath << endl;
	}
	continue;
      }
    }
    
    // for directories, search recursively
    
    if (S_ISDIR(fileStat.st_mode)) {
      dirList.push_back(entryPath);
      _loadInotifySubDirs(entryPath, depth + 1, dirList);
    }

    // for normal files add to list so they are avaialble at startup
    
    if (!S_ISREG(fileStat.st_mode)) {
      continue;
    }
    
    time_t file_time = fileStat.st_mtime;
    int age = now - file_time;
    if (age > _max_file_age || age < _file_quiescence_secs) {
      continue;
    }
    
    if (!_hasSubStr(dp->d_name)) {
      continue;
    }

    if (!_hasExt(dp->d_name)) {
      continue;
    }
    
    // add to path list
    
    _insertTimePair(file_time, entryPath);
    
  } // dp
  
  closedir(dirp);

  // load up path list for inotify, in time order

  if (depth == 0) {
    for (TimePathIter ii = _timePathList.begin(); ii != _timePathList.end(); ii++) {
      string path = ii->second;
      _inotifyFileQueue.push_front(path);
    }
  }
  
}

////////////////////////////////////////////////////////////
// handle inotify event
//

void DsInputPath::_handleInotifyEvent(struct inotify_event *event)

{
  
  if (_debug) {
    _printInotifyEvent(event, cerr);
  }
  
  // new file?
  
  if ((event->mask & IN_CLOSE_WRITE) || (event->mask & IN_MOVED_TO)) {
    
    // handle new file

    string dir(_watchList[event->wd]);
    string fileName(event->name);
    string filePath(dir);
    filePath += PATH_DELIM;
    filePath += fileName;
    
    if (_debug) {
      cerr << "New file found: " << filePath << endl;
    }
    
    // exclude files that start with _
    
    if (fileName[0] == '_') {
      if (_debug) {
        cerr << "Ignoring file starting with '_': " << fileName << endl;
      }
      return;
    }
    
    // get file stats
    
    struct stat fileStat;
    if (ta_stat(filePath.c_str(), &fileStat)) {
      if (_debug) {
        int errNum = errno;
        cerr << "WARNING: DsInputPath::_handleInotifyEvent" << endl;
        cerr << "  Cannot stat file: " << filePath << endl;
        cerr << "  " << strerror(errNum) << endl;
      }
      return;
    }

    // check for links
    
    if (!_follow_links) {
      if (S_ISLNK(fileStat.st_mode)) {
        if (_debug) {
          cerr << "-->> Ignoring symbolic link, file:: "
               << filePath << endl;
        }
        return;
      }
    }

    // check file properties

    if (!S_ISREG(fileStat.st_mode)) {
      return;
    }
    if (!_hasSubStr(fileName)) {
      return;
    }
    if (!_hasExt(fileName)) {
      return;
    }
    
    // add to path list

    _inotifyFileQueue.push_front(filePath);

    return;
    
  } // if ((event->mask & IN_CLOSE_WRITE) ...
  
  // new dir?

  if ((event->mask & IN_CREATE) && (event->mask & IN_ISDIR)) {
    
    string dir(_watchList[event->wd]);
    string subDir(event->name);
    string newDirPath(dir);
    newDirPath += PATH_DELIM;
    newDirPath += subDir;
    
    if (_debug) {
      cerr << "Created dir: " << newDirPath << endl;
    }
    
    // exclude dirs that start with _
    
    if (subDir[0] == '_') {
      if (_debug) {
        cerr << "Ignoring dir starting with '_': " << subDir << endl;
      }
      return;
    }

    // check for symbolic links
    
    if (!_follow_links) {
      struct stat dirStat;
      if (ta_stat(newDirPath.c_str(), &dirStat)) {
        if (_debug) {
          int errNum = errno;
          cerr << "WARNING: DsInputPath::_handleInotifyEvent" << endl;
          cerr << "  Cannot stat dir: " << newDirPath << endl;
          cerr << "  " << strerror(errNum) << endl;
        }
        return;
      }
      if (S_ISLNK(dirStat.st_mode)) {
        if (_debug) {
          cerr << "-->> Ignoring symbolic link, dir: " << newDirPath << endl;
        }
        return;
      }
    }
    
    // add dir to watch list

    int watchFlags = IN_CLOSE_WRITE | IN_MOVED_TO | IN_DELETE_SELF | IN_CREATE;
    int wd = inotify_add_watch(_inotifyFd, newDirPath.c_str(), watchFlags);
    if (wd >= 0) {
      if (_debug) {
        cerr << "==>> watching dir: " << newDirPath << ", using wd: " << wd << endl;
      }
      _watchList[wd] = newDirPath;
    }
    
    return;
    
  } // if ((event->mask & IN_CREATE) ...

  // dir deleted?
  
  if (event->mask & IN_DELETE_SELF) {
    
    string deletedDir(_watchList[event->wd]);
    
    if (_debug) {
      cerr << "Deleted dir: " << deletedDir << endl;
    }

    inotify_rm_watch(_inotifyFd, event->wd);
    _watchList.erase(event->wd);

    return;
    
  } // if ((event->mask & IN_DELETE_SELF)
  
}

///////////////////////////////////////////////////////////
// print Inotify event

void DsInputPath::_printInotifyEvent(struct inotify_event *event, 
                                     ostream &out)
  
{

  out << "========= inotify event ============" << endl;
  out << "    wd = " << event->wd << endl;
  if (event->cookie > 0) {
    out << "  cookie = " << event->cookie << endl;
  }
  
  out << "  mask = ";
  if (event->mask & IN_ACCESS)        out << "IN_ACCESS ";
  if (event->mask & IN_ATTRIB)        out << "IN_ATTRIB ";
  if (event->mask & IN_CLOSE_NOWRITE) out << "IN_CLOSE_NOWRITE ";
  if (event->mask & IN_CLOSE_WRITE)   out << "IN_CLOSE_WRITE ";
  if (event->mask & IN_CREATE)        out << "IN_CREATE ";
  if (event->mask & IN_DELETE)        out << "IN_DELETE ";
  if (event->mask & IN_DELETE_SELF)   out << "IN_DELETE_SELF ";
  if (event->mask & IN_IGNORED)       out << "IN_IGNORED ";
  if (event->mask & IN_ISDIR)         out << "IN_ISDIR ";
  if (event->mask & IN_MODIFY)        out << "IN_MODIFY ";
  if (event->mask & IN_MOVE_SELF)     out << "IN_MOVE_SELF ";
  if (event->mask & IN_MOVED_FROM)    out << "IN_MOVED_FROM ";
  if (event->mask & IN_MOVED_TO)      out << "IN_MOVED_TO ";
  if (event->mask & IN_OPEN)          out << "IN_OPEN ";
  if (event->mask & IN_Q_OVERFLOW)    out << "IN_Q_OVERFLOW ";
  if (event->mask & IN_UNMOUNT)       out << "IN_UNMOUNT ";
  out << endl;
  
  out << "    dir: " << _watchList[event->wd] << endl;
  if (event->len > 0) {
    out << "        name = " << event->name << endl;
  }

  out << "====================================" << endl;

}

#endif

// END OF INOTIFY - MAX OSX does not support inotify
/////////////////////////////////////////////////////////
