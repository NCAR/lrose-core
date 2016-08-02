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
/////////////////////////////////////////////////////////////
// StormFile.cc: StormFile handling
//
// Keeps list of available storm files.
// Finds the storm file for a given time.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 1997
//
//////////////////////////////////////////////////////////

#include "StormFile.hh"
#include <dirent.h>
#include <titan/storm.h>
#include <toolsa/str.h>
#include <toolsa/file_io.h>
using namespace std;

// Constructor

StormFile::StormFile (const char *prog_name,
		      int debug,
		      const char *storm_data_dir)

{

  // load up storm file times

  int year, month, day;
  char ext[32];
  char storm_file_path[MAX_PATH_LEN];
  struct dirent *dp;
  DIR *dirp;
  storm_file_time_t *ftime;
  storm_file_handle_t s_handle;

  // initialize

  isOK = TRUE;
  _progName = STRdup(prog_name);
  _stormDataDir = STRdup(storm_data_dir);
  _debug = debug;
  _fileTimes = NULL;
  _nFileTimes = 0;
  
  // initialize storm file handle
  
  RfInitStormFileHandle(&s_handle, _progName);

  // open storm data directory

  if((dirp = opendir(_stormDataDir)) == NULL) {
    fprintf(stderr, "ERROR - %s:StormFile\n", _progName);
    fprintf(stderr, "Cannot open storm_data_dir\n");
    perror (_stormDataDir);
    isOK = FALSE;
    RfFreeStormFileHandle(&s_handle);
    return;
  }
   
  // Loop thru directory looking for the storm header files
  
  for(dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
    
    /*
     * exclude dir entries and files beginning with '.'
     */

    if (dp->d_name[0] == '.') {
      continue;
    }
    
    /*
     * check that the dir name is in the correct format
     */
    
    if (sscanf(dp->d_name, "%4d%2d%2d.%s",
	       &year, &month, &day, ext) != 4) {
      continue;
    }
    
    if (strcmp(ext, STORM_HEADER_FILE_EXT)) {
      continue;
    }

    /*
     * name is in correct format. Therefore, read it and 
     * get start and end times.
     */
    
    sprintf(storm_file_path, "%s%s%s", _stormDataDir,
	    PATH_DELIM, dp->d_name);

    if (RfOpenStormFiles (&s_handle, "r", storm_file_path,
			  (char *) NULL, "load_storm_times")) {
      
      if (_debug) {
	fprintf(stderr, "WARNING - %s:StormFile::StormFile\n", _progName);
	fprintf(stderr, "Cannot open storm file.\n");
	perror(storm_file_path);
      }
      continue;

    }

    /*
     * read in storm properties file header
     */
    
    if (RfReadStormHeader(&s_handle, "load_storm_times")) {
      if (_debug) {
	fprintf(stderr, "WARNING - %s:StormFile::StormFile\n", _progName);
	fprintf(stderr, "Cannot read storm file header.\n");
	perror(storm_file_path);
      }
      RfCloseStormFiles(&s_handle, "load_storm_times");
      continue;
    }
    
    /*
     * header read in OK, set file times
     */
    
    _inc_alloc_times_array();
    ftime = _fileTimes + _nFileTimes - 1;
    ftime->storm_file_path = STRdup(storm_file_path);
    ftime->start_time = s_handle.header->start_time;
    ftime->end_time = s_handle.header->end_time;

    if (_debug) {
      fprintf(stderr, "File %s, start %s, end %s\n",
	      ftime->storm_file_path,
	      utimstr(ftime->start_time),
	      utimstr(ftime->end_time));
    }

    RfCloseStormFiles(&s_handle, "load_storm_times");

  } // dp
  
  closedir(dirp);

  RfFreeStormFileHandle(&s_handle);
  
}

// Destructor

StormFile::~StormFile()

{

  STRfree(_progName);
  STRfree(_stormDataDir);

  if (_fileTimes != NULL) {
    for (int i = 0; i < _nFileTimes; i++) {
      STRfree(_fileTimes[i].storm_file_path);
    }
    ufree(_fileTimes);
  }

}

////////////////////////////////////////
// get_path()
//
// get path for given time
//
// returns path on success, NULL on failure
////////////////////////////////////////

char *StormFile::get_path(time_t t)

{

  storm_file_time_t *ftime;
  int i;
  
  ftime = _fileTimes;
  for (i = 0; i < _nFileTimes; i++, ftime++) {
    if (ftime->start_time <= t && ftime->end_time >= t) {
      return (ftime->storm_file_path);
    }
  }

  return (NULL);
  
}

//////////////////////////////////////
// _inc_alloc_times_array()
//
// increment times array memory
//////////////////////////////////////

void StormFile::_inc_alloc_times_array()

{

  _nFileTimes++;
  if (_fileTimes == NULL) {
    _fileTimes = (storm_file_time_t *)
      umalloc(_nFileTimes * sizeof(storm_file_time_t));
  } else {
    _fileTimes = (storm_file_time_t *)
      urealloc(_fileTimes, _nFileTimes * sizeof(storm_file_time_t));
  }
}


