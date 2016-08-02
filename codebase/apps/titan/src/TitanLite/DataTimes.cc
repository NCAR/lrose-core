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
#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>
#include <titan/TitanStormFile.hh>
#include <cassert>
using namespace std;

///////////////////////////////////////////////////
// constructor 
//

DataTimes::DataTimes(const string &prog_name, const Params &params) :
  Worker(prog_name, params)
  
{
  
  _startTime = 0;
  _endTime = 0;
  _stormFileDate = -1;

  _mdvxTimes.setRealtime(_params.input_url,
			 _params.max_realtime_valid_age,
			 PMU_auto_register,
			 _params.input_search_sleep_msecs);

}

/////////////
// destructor
//

DataTimes::~DataTimes()

{

}

//////////////////////////////
// getLatest
//
// Get latest input data time
//
// Forces a read

time_t DataTimes::getLatest()

{

  while (_mdvxTimes.getLatest(_latestTime)) {
    PMU_auto_register("Waiting for latest_data ... ");
    sleep (1);
  }
  return _latestTime;
  
}

//////////////////////////////
// getNext
//
// Get latest input data time
//
// Blocks until new data

time_t DataTimes::getNext()

{

  _mdvxTimes.getNext(_latestTime);
  return _latestTime;
  
}

//////////////////////////////////////////////////////
//
// compute the restart time and related times,
// given the trigger time and restart parameters

void DataTimes::computeRestartTime(time_t trigger_time, 
				   int restart_hour,
				   int restart_min,
				   int overlap_period,
				   time_t &overlap_start_time,
				   time_t &start_time,
				   time_t &restart_time)
  
{

  // first take the trigger time, compute the day
  
  date_time_t stime;
  stime.unix_time = trigger_time;
  uconvert_from_utime(&stime);

  // set the restart hour and min
  
  stime.hour = restart_hour;
  stime.min = restart_min;
  stime.sec = 0;
  uconvert_to_utime(&stime);

  // if ahead of the trigger time, move back by 1 day
  
  if (stime.unix_time > trigger_time) {
    stime.unix_time -= SECS_IN_DAY;
    uconvert_from_utime(&stime);
  }

  // set times

  start_time = stime.unix_time;
  overlap_start_time = start_time - overlap_period;
  restart_time = start_time + SECS_IN_DAY;
  
}

/////////////////////
// _setStormFileDate()
//

void DataTimes::_setStormFileDate(time_t date)

{
  _stormFileDate = date;
}

///////////////////////////
// _setTimeLimitsRealtime()

void DataTimes::_setTimeLimitsRealtime()

{

  // get latest data time, set end time to this
  
  _endTime = getLatest();

  // if a storm file exists for this time, set the start time
  // from that file
  
  int iret = _setStartTimeFromStormFile();
  
  // if this was successful, return now
  
  if (iret == 0) {
    _setStormFileDate(_startTime);
    return;
  }

  if (_params.debug >= Params::DEBUG_NORM) {
    cerr << "Auto_restart - computing start time." << endl;
    cerr << " LatestDataTime: " << DateTime::str(_latestTime) << endl;
  }
  
  // start time is restart_overlap_time before the start of
  // current day
  
  date_time_t stime;
  stime.unix_time = _latestTime;
  uconvert_from_utime(&stime);
  stime.hour = _params.restart_time.hour;
  stime.min = _params.restart_time.min;
  stime.sec = 0;
  uconvert_to_utime(&stime);
  if (stime.unix_time > _latestTime) {
    stime.unix_time -= SECS_IN_DAY;
    uconvert_from_utime(&stime);
  }
  _startTime = stime.unix_time - _params.restart_overlap_period;
  _setStormFileDate(stime.unix_time);
  
}

///////////////////////////////
// _setStartTimeFromStormFile()
//
// Returns 0 on success, -1 on failure

int DataTimes::_setStartTimeFromStormFile()


{

  // set up file path for file
  
  char header_file_path[MAX_PATH_LEN];
  date_time_t ltime;
  ltime.unix_time = _latestTime;
  uconvert_from_utime(&ltime);
  sprintf(header_file_path, "%s%s%.4d%.2d%.2d.%s",
	  _params.storm_data_dir,
	  PATH_DELIM,
	  ltime.year, ltime.month, ltime.day,
	  STORM_HEADER_FILE_EXT);
  
  if (_params.debug >= Params::DEBUG_NORM) {
    fprintf(stderr, "DataTimes::_setStartTimeFromStormFile\n");
    fprintf(stderr, "Searching for storm file: %s\n", header_file_path);
  }
  
  // check if storm file exists

  struct stat hdr_stat;
  if (stat(header_file_path, &hdr_stat) < 0) {
    // file does not exist
    return (-1);
  }
  
  // create a storm file object

  TitanStormFile sfile;
  
  // open new storm properties file

  if (sfile.OpenFiles("r", header_file_path, STORM_DATA_FILE_EXT)) {
    cerr << "ERROR - " << _progName
	 << "DataTimes::_setStartTimeFromStormFile" << endl;
    cerr << sfile.getErrStr() << endl;
    return -1;
  }

  if (_params.debug >= Params::DEBUG_NORM) {
    cerr << "Found and opened file: " << header_file_path << endl;
  }
  
  // read in header
  
  if (sfile.ReadHeader()) {
    cerr << "ERROR - " << _progName
	 << "DataTimes::_setStartTimeFromStormFile" << endl;
    cerr << sfile.getErrStr() << endl;
    return -1;
  }
  // determine start_time

  _startTime = sfile.header().start_time;

  // close file

  sfile.CloseFiles();

  return 0;

}


