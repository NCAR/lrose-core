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
// MdvInputPath.cc : Input MDV file handling
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1998
//
//////////////////////////////////////////////////////////
//
// See <Mdv/mdv/MdvInputPath.h> for details.
//
///////////////////////////////////////////////////////////

#include <Mdv/mdv/MdvInputPath.h>
#include <toolsa/umisc.h>
#include <toolsa/str.h>
#include <dirent.h>
using namespace std;

/////////////////////////////
// Constructor - Archive mode
//
// Pass in a list of file paths.
//

MdvInputPath::MdvInputPath (char *prog_name,
			    int debug,
			    int n_files,
			    char **file_paths)

{

  // initialize

  _progName = STRdup(prog_name);
  _debug = (debug? 1 : 0);
  
  _fileNum = 0;
  _nFiles = n_files;
  _archiveMode = TRUE;

  // set up membuf for file paths array

  _mbufPaths = MEMbufCreate();
  for (int i = 0; i < _nFiles; i++) {
    char *path = (char *) umalloc(strlen(file_paths[i]) + 1);
    strcpy(path, file_paths[i]);
    MEMbufAdd(_mbufPaths, &path, sizeof(char *));
  }
  _filePaths = (char **) MEMbufPtr(_mbufPaths);

  // sort the paths

  qsort(_filePaths, _nFiles, sizeof(char *), _comparePaths);

}

/////////////////////////////
// Constructor - Archive mode
//
// Pass in data directory and start and end times.
//

MdvInputPath::MdvInputPath (char *prog_name,
			    int debug,
			    char *input_dir,
			    time_t start_time,
			    time_t end_time)

{

  // initialize
  
  _progName = STRdup(prog_name);
  _debug = (debug? 1 : 0);
  
  _fileNum = 0;
  _nFiles = 0;
  _archiveMode = TRUE;

  // set up membuf for file paths array

  _mbufPaths = MEMbufCreate();

  // compute start and end day

  int start_day = start_time / 86400;
  int end_day = end_time / 86400;
  
  // loop through days

  for (int iday = start_day; iday <= end_day; iday++) {
    
    date_time_t midday;
    midday.unix_time = iday * 86400 + 43200;
    uconvert_from_utime(&midday);

    // compute subdir path
    
    char subdir_path[MAX_PATH_LEN];
    
    sprintf(subdir_path, "%s%s%.4d%.2d%.2d",
	    input_dir, PATH_DELIM,
	    midday.year, midday.month, midday.day);

    // load up file paths for this day

    _loadDay(subdir_path, &midday, start_time, end_time);

  } // iday
  
  _filePaths = (char **) MEMbufPtr(_mbufPaths);

  // sort the paths

  qsort(_filePaths, _nFiles, sizeof(char *), _comparePaths);

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

MdvInputPath::MdvInputPath (char *prog_name,
			    int debug,
			    char *input_dir,
			    int max_valid_age,
			    MdvInput_heartbeat_t heartbeat_func)
		      

{

  // initialize

  _progName = STRdup(prog_name);
  _debug = (debug? 1 : 0);
  
  _inputDir = input_dir;
  _maxAge = max_valid_age;
  _heartbeatFunc = heartbeat_func;
  _archiveMode = FALSE;

  // init latest data handle
  
  LDATA_init_handle(&_lastData, _progName, _debug);

}

/////////////
// Destructor

MdvInputPath::~MdvInputPath()

{

  STRfree(_progName);

  if (_archiveMode) {

    // free up paths and membuf
    
    for (int i = 0; i < _nFiles; i++) {
      ufree(_filePaths[i]);
    }
    MEMbufDelete(_mbufPaths);

  } else {

    // free up latest data info handle

    LDATA_free_handle(&_lastData);

  }

}

/////////////////////
// get next file path
//
// returns NULL on failure

char *MdvInputPath::next()

{
  
  if (_archiveMode) {

    // in archive mode, go through the file list

    if (_fileNum < _nFiles) {
      _fileNum++;
      return (_filePaths[_fileNum - 1]);
    } else {
      return (NULL);
    }

  } else {

    // in realtime mode wait for change in latest info
    // sleep 1 second between tries.

    LDATA_info_read_blocking(&_lastData, _inputDir,
			     _maxAge, 1000,
			     (LDATA_heartbeat_t) _heartbeatFunc);

    // new data
    
    sprintf(_inputPath,
	    "%s%s%.4d%.2d%.2d%s%.2d%.2d%.2d.%s",
	    _inputDir,
	    PATH_DELIM,
	    _lastData.ltime.year, _lastData.ltime.month, _lastData.ltime.day,
	    PATH_DELIM,
	    _lastData.ltime.hour, _lastData.ltime.min, _lastData.ltime.sec,
	    _lastData.info.file_ext);

    return (_inputPath);

  } // if (archiveMode)

  return (NULL);

}

/////////////////////////
// reset to start of list
// 
// Archive mode only.

void MdvInputPath::reset()

{
  if (_archiveMode) {
    _fileNum = 0;
  }
}

//////////////////////////////////////
// _loadDay
//
// Load up file paths for a given day

void MdvInputPath::_loadDay(char *subdir_path,
			    date_time_t *midday,
			    time_t start_time,
			    time_t end_time)

{

  struct dirent *dp;
  DIR *dirp;

  if((dirp = opendir(subdir_path)) == NULL) {
    return;
  }

  for(dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
    
    // exclude dir entries and files beginning with '.'
    
    if (dp->d_name[0] == '.') {
      continue;
    }
      
    // check that the dir name is in the correct format
    
    int hour, min, sec;

    if (sscanf(dp->d_name, "%2d%2d%2d",
	       &hour, &min, &sec) != 3) {
      continue;
    }
      
    if (hour < 0 || hour > 23 || min < 0 || min > 59 ||
	sec < 0 || sec > 59) {
      continue;
    }

    // check file time is within limits

    date_time_t file_time = *midday;
    file_time.hour = hour;
    file_time.min = min;
    file_time.sec = sec;
    uconvert_to_utime(&file_time);

    if (file_time.unix_time < start_time ||
	file_time.unix_time > end_time) {
      continue;
    }

    // name is in correct format. Therefore, accept it
    
    char *path = (char *) umalloc(strlen(subdir_path) +
				  strlen(PATH_DELIM) +
				  strlen(dp->d_name) + 1);
    
    sprintf(path, "%s%s%s", subdir_path, PATH_DELIM, dp->d_name);

    // add to mem buffer
    
    MEMbufAdd(_mbufPaths, &path, sizeof(char *));
    _nFiles++;

  } // dp
  
  closedir(dirp);

  return;
}

///////////////////////////////////////////////
// define function to be used for sorting paths
//

int MdvInputPath::_comparePaths(const void *v1, const void *v2)

{

  char *path1 = *((char **) v1);
  char *path2 = *((char **) v2);

  return (strcmp(path1, path2));

}


