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
// DataTimes.cc
//
// DataTimes class
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 1998
//
///////////////////////////////////////////////////////////////

#include "DataTimes.hh"
#include <toolsa/umisc.h>
#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <titan/storm.h>
#include <cassert>
using namespace std;

///////////////////////////////////////////////////
// constructor - no times specified - REALTIME mode
//

DataTimes::DataTimes(char *prog_name, Params *params)

{

  assert (params->mode == Params::REALTIME);

  _progName = STRdup(prog_name);
  _params = params;
  _realtimeMode = true;
  
  _startTime = 0;
  _endTime = 0;
  _restartTime = 0;
  _stormFileDate = 0;

  LDATA_init_handle(&_ldata, _progName, FALSE);

}

////////////////////////////////////////////////
// constructor - times specified - ARCHIVE mode
//

DataTimes::DataTimes(char *prog_name, Params *params,
		     time_t start_time, time_t end_time)

{

  assert (params->mode == Params::ARCHIVE);
  
  _progName = STRdup(prog_name);
  _params = params;
  _realtimeMode = false;
  
  _startTime = start_time;
  _endTime = end_time;
  if (_params->auto_restart) {
    _setRestartTime();
  } else {
    _restartTime = -1;
  }
  _setStormFileDate(_startTime);

  LDATA_init_handle(&_ldata, _progName, FALSE);

}

/////////////
// destructor
//

DataTimes::~DataTimes()

{

  STRfree(_progName);
  LDATA_free_handle(&_ldata);

}

//////////////////////////////
// getLatest
//
// Get latest input data time

time_t DataTimes::getLatest()

{
  
  LDATA_info_read_blocking(&_ldata,
			   _params->rdata_dir,
			   _params->max_realtime_valid_age,
			   2000,
			   PMU_auto_register);

  return (_ldata.info.latest_time);
  
}

/////////////////////
// getStormFileDate()
//

time_t DataTimes::getStormFileDate()

{
  assert(_stormFileDate != 0);
  return(_stormFileDate);
}

/////////////////////
// _setStormFileDate()
//

void DataTimes::_setStormFileDate(time_t date)

{
  _stormFileDate = date;
}

///////////////////
// getRestartTime()
//

time_t DataTimes::getRestartTime()

{
  assert(_restartTime != 0);
  return (_restartTime);
}
     
////////////////////
// _setRestartTime()
//

void DataTimes::_setRestartTime()

{

  date_time_t rtime;

  // start with the restart interval plus 1 day

  if (_params->restart_no_delay) {
    rtime.unix_time =
      _startTime + _params->restart_overlap_period;
  } else {
    rtime.unix_time =
      _startTime + _params->restart_overlap_period + 86400;
  }
  uconvert_from_utime(&rtime);
  
  // set the hour and min
  
  rtime.hour = _params->restart_time.hour;
  rtime.min = _params->restart_time.min;
  rtime.sec = 0;
  uconvert_to_utime(&rtime);

  // if less that 1 day after start time, increase by 1 day

  if (!_params->restart_no_delay &&
      (rtime.unix_time - _startTime < 86400)) {
    rtime.unix_time += 86400;
  }
  
  if (_realtimeMode) {
    _restartTime = rtime.unix_time;
  } else {
    if (rtime.unix_time <= _endTime) {
      _restartTime = rtime.unix_time;
    } else {
      _restartTime = -1;
    }
  }
  
  if (_params->debug) {
    fprintf(stderr, "Start_time: %s\n", utimstr(_startTime ));
    fprintf(stderr, "_restartTime: %s\n", utimstr(_restartTime ));
    fprintf(stderr, "Overlap_period (secs): %d\n",
	    (int) _params->restart_overlap_period);
  }

}

///////////////////////////////////////////////////
// getStartupTimeLimits()
//
// Get the start and end times for program startup
//

void DataTimes::getStartupTimeLimits(time_t *start_time_p, time_t *end_time_p)

{

  if (_realtimeMode) {
    _setTimeLimitsRealtime();
  }

  if (_params->debug >= Params::DEBUG_NORM) {
    fprintf(stderr, "Prep data start time: %s\n",
	    utimstr(_startTime));
    fprintf(stderr, "Prep data end time: %s\n",
	    utimstr(_endTime));
    fprintf(stderr, "Storm file date: %s\n",
	    utimstr(_stormFileDate));
  }

  *start_time_p = _startTime;
  *end_time_p = _endTime;
  return;

}

///////////////////////////
// _setTimeLimitsRealtime()

void DataTimes::_setTimeLimitsRealtime()

{

  // get latest data time, set end time to this
  
  LDATA_info_read_blocking(&_ldata,
			   _params->rdata_dir,
			   _params->max_realtime_valid_age,
			   2000,
			   PMU_auto_register);
  
  _endTime = _ldata.info.latest_time;

  // if a storm file exists for this time, set the start time
  // from that file
  
  int iret = _setStartTimeFromStormFile();
  
  // if this was successful, return now
  
  if (iret == 0) {
    _setStormFileDate(_endTime);
    _setRestartTime();
    return;
  }

  if (_params->auto_restart) {

    if (_params->debug >= Params::DEBUG_NORM) {
      fprintf(stderr, "Auto_restart - computing start time\n");
    }

    // start time is restart_overlap_time before the start of
    // current day

    date_time_t dtime;
    dtime = _ldata.ltime;
    dtime.hour = 0;
    dtime.min = 0;
    dtime.sec = 0;
    uconvert_to_utime(&dtime);
    _startTime = dtime.unix_time - _params->restart_overlap_period;
    _setStormFileDate(_endTime);
    _setRestartTime();
    
  } else {

    if (_params->debug >= Params::DEBUG_NORM) {
      fprintf(stderr,
	      "Not auto_restart - searching for start time in data\n");
    }

    _searchBackwardsForStartTime(_endTime);
    _setStormFileDate(_startTime);
    _setRestartTime();
    
  }
  
}

///////////////////////////////
// _setStartTimeFromStormFile()
//
// Returns 0 on success, -1 on failure

int DataTimes::_setStartTimeFromStormFile()


{

  // set up file path for file
  
  char header_file_path[MAX_PATH_LEN];
  sprintf(header_file_path, "%s%s%.4d%.2d%.2d.%s",
	  _params->storm_data_dir,
	  PATH_DELIM,
	  _ldata.ltime.year, _ldata.ltime.month, _ldata.ltime.day,
	  STORM_HEADER_FILE_EXT);
  
  if (_params->debug >= Params::DEBUG_NORM) {
    fprintf(stderr, "DataTimes::_setStartTimeFromStormFile\n");
    fprintf(stderr, "Searching for storm file: %s\n", header_file_path);
  }
  
  // initialize new storm file handle
  
  storm_file_handle_t s_handle;
  RfInitStormFileHandle(&s_handle, _progName);
  
  // check if storm file exists

  struct stat hdr_stat;
  if (stat(header_file_path, &hdr_stat) < 0) {
    // file does not exist
    return (-1);
  }
  
  // open new storm properties file

  if (RfOpenStormFiles (&s_handle, "r",
			header_file_path,
			STORM_DATA_FILE_EXT,
			"DataTimes::_setStartTimeFromStormFile")) {

    return (-1);
  }

  if (_params->debug >= Params::DEBUG_NORM) {
    fprintf(stderr, "Found and opened file.\n");
  }
  
  // read in header
  
  if (RfReadStormHeader(&s_handle, "DataTimes::_setStartTimeFromStormFile")) {
    return (-1);
  }

  // determine start_time

  _startTime = s_handle.header->start_time;

  // close file and free up

  RfCloseStormFiles (&s_handle,
		     "DataTimes::_setStartTimeFromStormFile");
  RfFreeStormHeader(&s_handle, "DataTimes::_setStartTimeFromStormFile");
  RfFreeStormFileHandle(&s_handle);

  return (0);

}

///////////////////////////////////////////////////////////////////////
// _searchBackwardsForStartTime()
//
// This routine searches back through the data files on disk for the 
// first break in the data which exceeds the max_missing_data_gap.
// 
// The method is as follows: a day is divided up into a number of
// periods, the period length being max_missing_data_gap. When a 
// file is found, it is placed in the relevant period. If no files
// fall in a given period, there must be a gap at least as long
// as the period length.
// The start time of the most recent such period is returned as the
// start time for the algorithm.
//

typedef struct {
  int have_data;
  si32 start_time;
} time_period_t;

void DataTimes::_searchBackwardsForStartTime(time_t end_time)

{


  DIR *dirp;
  struct dirent	*dp;

  if (_params->debug >= Params::DEBUG_NORM) {
    fprintf(stderr, "_searchBackwardsForStartTime\n");
  }
  
  // initialize periods - if the division is not exact ignore the last
  // period in the day because it will be shorter than the rest

  int period_len = _params->max_missing_data_gap;
  int nperiods = 86400 / period_len;
  time_period_t *periods = (time_period_t *) umalloc
    ((ui32) (nperiods * sizeof(time_period_t)));

  time_t latest_start_time = end_time;

  // set up format string

  char format_str[20];
  sprintf(format_str, "%s%s", "%2d%2d%2d.", "mdv");

  // move back through julian dates

  date_time_t d_end_time;
  d_end_time.unix_time = end_time;
  uconvert_from_utime(&d_end_time);
  int julday = ujulian_date(d_end_time.day,
			    d_end_time.month,
			    d_end_time.year);

  for (int ijul = julday; ijul >= 0; ijul--) {

    // compute the calendar date for this julian date
    
    date_time_t file_time;
    ucalendar_date(ijul,
		   &file_time.day, &file_time.month, &file_time.year);
    
    // compute the start_of_day time
    
    date_time_t start_of_day;
    start_of_day = file_time;
    start_of_day.hour = 0;
    start_of_day.min = 0;
    start_of_day.sec = 0;
    uconvert_to_utime(&start_of_day);

    // initialize the periods array

    for (int iperiod = 0; iperiod < nperiods; iperiod++) {
      periods[iperiod].have_data = FALSE;
      periods[iperiod].start_time =
	start_of_day.unix_time + iperiod * period_len;
    } // iperiod
    
    // compute directory name for data with this date
    
    char dirname[MAX_PATH_LEN];
    sprintf(dirname, "%s%s%.4d%.2d%.2d",
	    _params->rdata_dir, PATH_DELIM,
	    file_time.year, file_time.month, file_time.day);
    
    // read through the directory to get the file names

    if ((dirp = opendir (dirname)) == NULL) {
      goto done;
    }

    if (_params->debug >= Params::DEBUG_NORM) {
      fprintf(stderr, "searching dir %s\n", dirname);
    }
  
    int nfiles = 0;

    for (dp = readdir (dirp); dp != NULL; dp = readdir (dirp)) {

      // exclude dir entries and files beginning with '.'

      if (dp->d_name[0] == '.') {
	continue;
      }

      // check that the file name is in the correct format
      
      if (sscanf(dp->d_name, format_str,
		 &file_time.hour, &file_time.min, &file_time.sec) != 3) {
	
	if (_params->debug >= Params::DEBUG_NORM) {
	  fprintf(stderr,
		  "WARNING - %s:_searchBackwardsForStartTime\n",
		  _progName);
	  fprintf(stderr, "File name '%s' invalid\n", dp->d_name);
	}
	continue;
	
      }
      
      if (!uvalid_datetime(&file_time)) {
	
	if (_params->debug >= Params::DEBUG_NORM) {
	  fprintf(stderr,
		  "WARNING - %s:_searchBackwardsForStartTime\n",
		  _progName);
	  fprintf(stderr, "File name '%s' invalid\n", dp->d_name);
	}
	continue;
	
      }
      
      // file name is in correct format, therefore accept it.
      // Compute the file time

      uconvert_to_utime(&file_time);

      // compute the period into which this file time falls

      int iperiod =
	(file_time.unix_time - start_of_day.unix_time) / period_len;

      if (iperiod > nperiods - 1) {
	iperiod = nperiods - 1;
      }
      
      periods[iperiod].have_data = TRUE;

      nfiles++;

    } // readdir
  
    // close the directory file
    
    closedir(dirp);
    
    // if no files have been found, return latest start time found
    
    if (nfiles == 0) {
      goto done;
    }
    
    // search for latest period without data
    
    for (int iperiod = nperiods - 1; iperiod >= 0; iperiod--) {

      if (periods[iperiod].start_time <= end_time &&
	  !periods[iperiod].have_data) {
	latest_start_time = periods[iperiod].start_time;
	goto done;
      }

    }

    latest_start_time = start_of_day.unix_time;

  } // ijul

  if (_params->debug >= Params::DEBUG_NORM) {
    fprintf(stderr, "start_time : %s\n", utimstr(latest_start_time));
  }
  
done:
  ufree ((char *) periods);
  _startTime = latest_start_time;
  
}


