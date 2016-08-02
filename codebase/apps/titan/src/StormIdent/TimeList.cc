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
// TimeList.cc
//
// TimeList class
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 1998
//
///////////////////////////////////////////////////////////////

#include "TimeList.hh"
#include <toolsa/umisc.h>
#include <toolsa/str.h>
#include <toolsa/DateTime.hh>
using namespace std;

//////////////
// constructor
//

TimeList::TimeList(char *prog_name, Params *params)

{

  OK = TRUE;
  _progName = STRdup(prog_name);
  _params = params;
  times = NULL;
  nTimes = 0;
  _nAlloc = 0;

}

/////////////
// destructor
//

TimeList::~TimeList()

{

  if (times != NULL) {
    ufree(times);
  }
  STRfree(_progName);

}

//////////////////
// load()
//
// load up the times

void TimeList::load(time_t start_time,
		    time_t end_time,
		    time_t restart_time /* = -1*/ )

{

  // in archive mode with restart on, amend the end time to the
  // next restart time

  if (restart_time != -1) {
    if (_params->debug) {
      cerr << "Resetting end time to: "
	   << DateTime::str(restart_time) << endl;
    }
    end_time = restart_time;
  }

  // initialize

  nTimes = 0;
  
  // set up format string

  char format_str[20];
  sprintf(format_str, "%s%s", "%2d%2d%2d.", "mdv");

  // compute start and end days

  date_time_t startTime, endTime;
  startTime.unix_time = start_time;
  uconvert_from_utime(&startTime);
  endTime.unix_time = end_time;
  uconvert_from_utime(&endTime);
  
  int start_julday =
    ujulian_date(startTime.day, startTime.month, startTime.year);
  
  int end_julday =
    ujulian_date(endTime.day, endTime.month, endTime.year);
  
  // move through julian dates

  for (int ijul = start_julday; ijul <= end_julday; ijul++) {
    
    // compute the calendar date for this julian date
    
    date_time_t file_time;

    ucalendar_date(ijul,
		   &file_time.day, &file_time.month, &file_time.year);
    
    /*
     * compute directory name for data with this date
     */
    
    char dirname[MAX_PATH_LEN];
    
    sprintf(dirname, "%s%s%.4d%.2d%.2d",
	    _params->rdata_dir, PATH_DELIM,
	    file_time.year, file_time.month, file_time.day);
    
    // open directory file for reading - if the directory does
    // not exist, this phase is done
    
    DIR *dirp;
    if ((dirp = opendir (dirname)) == NULL) {
      continue;
    }
    
    // read through the directory

    struct dirent *dp;

    for (dp = readdir (dirp); dp != NULL; dp = readdir (dirp)) {

      // exclude dir entries and files beginning with '.'
      
      if (dp->d_name[0] == '.') {
	continue;
      }
	
      // check that the file name is in the correct format
      
      if (sscanf(dp->d_name, format_str,
		 &file_time.hour, &file_time.min, &file_time.sec) != 3) {
	if (_params->debug >= Params::DEBUG_NORM) {
	  fprintf(stderr, "WARNING - %s:TimeList::get\n", _progName);
	  fprintf(stderr, "File name '%s' invalid\n", dp->d_name);
	}
	continue;
      }
      
      // check for valid date & time
      
      if (!uvalid_datetime(&file_time)) {
	if (_params->debug >= Params::DEBUG_NORM) {
	  fprintf(stderr, "WARNING - %s:TimeList::get\n", _progName);
	  fprintf(stderr, "File name '%s' invalid\n", dp->d_name);
	}
	continue;
      }
      
      // file name is in correct format. Therefore, accept it
      
      uconvert_to_utime(&file_time);
      
      if (start_time <= file_time.unix_time &&
	  end_time >= file_time.unix_time) {
	
	_checkAlloc(nTimes + 1);
	times[nTimes] = file_time.unix_time;
	nTimes++;
	
      } // if (start_time <= file_time.unix_time ...
      
    } // while

    // close the directory file

    closedir(dirp);

  } // ijul

  // sort the time list
  
  qsort((void *) times, nTimes, sizeof(time_t),	_ftimesCompare);

  // print out time list

  if (_params->debug >= Params::DEBUG_VERBOSE) {
    fprintf(stderr, "\nFull time list:\n");
    for (int i = 0; i < nTimes; i++) {
      fprintf(stderr, "Time %d: %s\n", i, utimstr(times[i]));
    }
  }

}

//////////////////
// print()
//
// print the list

void TimeList::print(FILE *out) const

{
  
  for (int i = 0; i < nTimes; i++) {
    fprintf(out, "Time %d: %s\n", i,
	    utimstr(times[i]));
  }

}

////////////////
// _checkAlloc()

#define TIME_LIST_DELTA_ALLOC 50

void TimeList::_checkAlloc(int ntimes)

{

  if (ntimes > _nAlloc) {
    if (_nAlloc == 0) {
      _nAlloc = TIME_LIST_DELTA_ALLOC;
      times = (time_t *) umalloc(_nAlloc * sizeof(time_t));
    } else {
      _nAlloc += TIME_LIST_DELTA_ALLOC;
      times = (time_t *) urealloc(times,
				 _nAlloc * sizeof(time_t));
    }
  }
  
}

////////////////////////////////////////
// define function to be used for sorting

int TimeList::_ftimesCompare(const void *v1, const void *v2)

{

  time_t *t1 = (time_t *) v1;
  time_t *t2 = (time_t *) v2;
  
  return (*t1 - *t2);

}


