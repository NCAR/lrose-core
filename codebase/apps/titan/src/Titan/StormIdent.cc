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
///////////////////////////////////////////////////////////////
// StormIdent.cc
//
// StormIdent object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 1998
//
///////////////////////////////////////////////////////////////

#include "StormIdent.hh"
#include "DataTimes.hh"
#include "Sounding.hh"

#include <toolsa/umisc.h>
#include <toolsa/file_io.h>
#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <toolsa/Path.hh>
#include <toolsa/DateTime.hh>
#include <dsserver/DsLdataInfo.hh>
#include <Mdv/DsMdvxTimes.hh>

#include <sys/types.h>
#include <sys/file.h>
#include <fcntl.h>
using namespace std;

const int StormIdent::maxEigDim = 3;
const float StormIdent::missingVal = -9999.0;

// Constructor

StormIdent::StormIdent(const string &prog_name,
		       const Params &params) :
  Worker(prog_name, params),
  _inputMdv(_progName, _params),
  _identify(_progName, _params, _inputMdv, _sfile)
  
{

  _fatalError = false;

}

// destructor

StormIdent::~StormIdent()

{

}

//////////////////////////////////////////////////
// runRealtime

int StormIdent::runRealtime()
{
  
  // register with procmap
  
  PMU_auto_register("StormIdent::runRealtime");

  // get the latest available data time
  
  DataTimes dataTimes(_progName, _params);
  time_t latestTime = dataTimes.getLatest();

  // compute the startup and restart times

  time_t overlapStartTime;
  time_t startTime;
  time_t restartTime;
  
  DataTimes::computeRestartTime(latestTime,
				_params.restart_time.hour,
				_params.restart_time.min,
				_params.restart_overlap_period,
				overlapStartTime,
				startTime,
				restartTime);
  
  if (_params.debug) {
    cerr << "******************* RESTART *************************" << endl;
    cerr << "  latestTime: " << DateTime::str(latestTime) << endl;
    cerr << "  overlapStartTime: " << DateTime::str(overlapStartTime) << endl;
    cerr << "  startTime: " << DateTime::str(startTime) << endl;
    cerr << "  restartTime: " << DateTime::str(restartTime) << endl;
    cerr << "*****************************************************" << endl;
  }
  
  // load up the list of data times available in archive so far
  
  DsMdvxTimes mdvTimes;
  mdvTimes.setArchive(_params.input_url,
		      overlapStartTime, latestTime);
  const vector<time_t> &times = mdvTimes.getArchiveList();
  
  // load header file path - name is based on start time
  
  _loadHeaderFilePath(startTime);
  
  // load storm params - this is used for checking with existing
  // files to ensure the params have not changed.
  
  storm_file_params_t sparams;
  _loadStormParams(&sparams);
  
  // check for existing storm file, set current scan num and time
  
  int currentScan = -1;
  time_t currentTime = -1;
  
  if (_openAndCheck(_headerFilePath, &sparams) == 0) {
    // get current scan number analyzed
    _findCurrentScan(mdvTimes, currentScan, currentTime);
  }

  // prepare storm file
  
  if (currentScan < 0) {
    // prepare new file
    if (_prepareNew(_headerFilePath, &sparams)) {
      return -1;
    }
  } else {
    // prepare old file
    if (_prepareOld(_headerFilePath, currentScan)) {
      _prepareNew(_headerFilePath, &sparams);
      return -1;
    }
  }

  // Prepare track file.
  
  StormTrack tracking(_progName, _params, _headerFilePath);
  if (_params.perform_tracking && currentScan >= 0) {
    if (tracking.PrepareForAppend()) {
      _prepareNew(_headerFilePath, &sparams);
      return -1;
    }
  }
  
  // process data found on disk but not yet processed
  
  int scanNum = currentScan;
  time_t prevTime = latestTime;

  for (int itime = currentScan + 1; itime < (int) times.size(); itime++) {
    if (_processScan(scanNum + 1, times[itime], tracking)) {
      cerr << "ERROR - StormIdent::runRealtime" << endl;
      if (_fatalError) {
	_prepareNew(_headerFilePath, &sparams);
	return -1;
      }
    } else {
      scanNum++;
      prevTime = times[itime];
      // write latest data info for last time processed
      if (itime == (int) times.size() - 1) {
	_writeLdataInfo();
      }
    }
  } // itime
  
  // realtime loop - restart condition will be checked in the loop
  
  while (true) {
    
    PMU_auto_register("Processing realtime data");
    
    // get the latest time available
    // will block until new data arrives

    if (_params.debug) {
      cerr << "  -->> Getting new data <<--" << endl;
    }

    latestTime = dataTimes.getNext();

    if (_params.debug) {
      cerr << "  -->> latestTime: " << utimstr(latestTime) << endl;
      cerr << "  -->>   prevTime: " << utimstr(prevTime) << endl;
    }
    
    if (latestTime == prevTime) {
      if (_params.debug) {
	cerr << "  ---->> Times equal - skipping ..." << endl;
      }
      continue;
    }
    
    // if elapsed time is negative, data is from the past
    // so ignore this scan
    
    if (latestTime < prevTime) {
      fprintf(stderr, "WARNING: %s\n", _progName.c_str());
      fprintf(stderr, "  Scan time is from the past\n");
      fprintf(stderr, "  Prev time: %s\n", utimstr(prevTime));
      fprintf(stderr, "  This time: %s\n", utimstr(latestTime));
      fprintf(stderr, "  Ignoring this scan ...\n");
      continue;
    }
	
    // Get a new times list. If we are keeping up with the data 
    // the new list will contain only a single entry
    
    mdvTimes.setArchive(_params.input_url, prevTime + 1, latestTime);
    
    // process the times list
    
    for (int itime = 0; itime < (int) times.size(); itime++) {
      
      char pmu_message[128];
      sprintf(pmu_message, "Processing %s\n", utimstr(times[itime]));
      PMU_auto_register(pmu_message);
      
      // check restart condition
      
      time_t scanTime = times[itime];
      if (scanTime > restartTime) {
	return 0;
      }
      
      // check for time reversal
      
      if (scanNum >= 0) {
	
	time_t elapsed_time = scanTime - prevTime;
	
	// if elapsed time is negative, data is from past
        // so do not process this data
	
	if (elapsed_time < 0) {
	  fprintf(stderr, "WARNING: %s\n", _progName.c_str());
	  fprintf(stderr, "  Scan time is from the past\n");
	  fprintf(stderr, "  Prev time: %s\n", utimstr(prevTime));
	  fprintf(stderr, "  This time: %s\n", utimstr(scanTime));
	  fprintf(stderr, "  Ignoring this scan...\n");
	  continue;
	}
	
      } // if (scanNum > 0)
      
      // process this scan

      if (_processScan(scanNum + 1, scanTime, tracking)) {

	if (_fatalError) {
	  cerr << "ERROR - StormIdent::runRealtime" << endl;
	  _prepareNew(_headerFilePath, &sparams);
	  return -1;
	}

      } else {
	
	// write latest data info file
	
	_writeLdataInfo();
	
	// increment scan num
	
	scanNum++;
	
	// store prev time
	
	prevTime = scanTime;
	
      }
	
    } // itime
  
  } // while 

  return 0;

}

      
//////////////////////////////////////////////////
// runArchive

int StormIdent::runArchive(time_t overlap_start_time,
			   time_t start_time,
			   time_t end_time)

{

  if (_params.debug) {
    cerr << "***************** ARCHIVE MODE **********************" << endl;
    cerr << "  overlapStartTime: "
	 << DateTime::str(overlap_start_time) << endl;
    cerr << "  startTime: " << DateTime::str(start_time) << endl;
    cerr << "  endTime: " << DateTime::str(end_time) << endl;
    cerr << "*****************************************************" << endl;
  }
  
  // register with procmap
  
  PMU_auto_register("StormIdent::runArchive");
  
  // create data times object

  DsMdvxTimes mdvTimes;
  mdvTimes.setArchive(_params.input_url,
		      overlap_start_time, end_time);
  
  // load header file path based on start time
  
  _loadHeaderFilePath(start_time);
  
  // load storm params - this is used for checking with existing
  // files to ensure the params have not changed.
  
  storm_file_params_t sparams;
  _loadStormParams(&sparams);
  
  // prepare new storm file
  
  if (_prepareNew(_headerFilePath, &sparams)) {
    return -1;
  }

  // create storm track object in case we need it
  
  StormTrack tracking(_progName, _params, _headerFilePath);
  
  // loop through the input radar MDV files
  
  int scanNum = 0;
  time_t scanTime;
  
  while (mdvTimes.getNext(scanTime) == 0) {
    
    PMU_auto_register("Processing forecast data");
    
    // process this scan

    if (_processScan(scanNum, scanTime, tracking) == 0) {

      if (_fatalError) {

	cerr << "ERROR - StormIdent::runForecast" << endl;
	return -1;

      } else {

	_writeLdataInfo();

	// increment scan num
	
	scanNum++;
	
      }

    } // if (_processScan( ...
      
  } // while (mdvTimes.getNext(scanTime) == 0)

  return 0;

}

//////////////////////////////////////////////////
// runForecast

int StormIdent::runForecast(time_t gen_time)

{

  if (_params.debug) {
    cerr << "***************** FORECAST MODE **********************" << endl;
    cerr << "  genTime: " << DateTime::str(gen_time) << endl;
    cerr << "*****************************************************" << endl;
  }
  
  // register with procmap
  
  PMU_auto_register("StormIdent::runForecast");
  
  // create data times object

  DsMdvxTimes mdvTimes;
  mdvTimes.setArchiveFcst(_params.input_url, gen_time, gen_time);
  
  // load header file path based on start time
  
  _loadHeaderFilePath(gen_time);
  
  // load storm params - this is used for checking with existing
  // files to ensure the params have not changed.
  
  storm_file_params_t sparams;
  _loadStormParams(&sparams);
  
  // prepare new storm file
  
  if (_prepareNew(_headerFilePath, &sparams)) {
    return -1;
  }
  
  // create storm track object in case we need it
  
  StormTrack tracking(_progName, _params, _headerFilePath);
  
  // loop through the input radar MDV files
  
  int scanNum = 0;
  time_t scanTime;

  while (mdvTimes.getForecastTime(scanTime, scanNum) == 0) {
    
    PMU_auto_register("Processing archive data");
    
    // process this scan

    if (_params.debug) {
      cerr << "-->> Got forecast time: " << DateTime::strm(scanTime) << endl;
    }
    
    if (_processScan(scanNum, scanTime, tracking) == 0) {
      
      if (_fatalError) {
        cerr << "ERROR - StormIdent::runArchive" << endl;
        return -1;
      }

      _writeLdataInfo();
      scanNum++;
	
    } // if (_processScan( ...
      
  } // while (mdvTimes.getNext(scanTime) == 0)

  return 0;

}

//////////////////
// process a scan

int StormIdent::_processScan(int scan_num, time_t scan_time,
			     StormTrack &tracking)

{

  char pmu_message[128];
  sprintf(pmu_message, "Processing %s\n", utimstr(scan_time));
  PMU_auto_register(pmu_message);
  
  _fatalError = false;
    
  // read in MDV data
  
  if (_inputMdv.read(scan_time)) {
    cerr << "ERROR - StormIdent::_identAndTrack" << endl;
    cerr << "  Cannot read data for time: " << utimstr(scan_time) << endl;
    return -1;
  }

  // read in sounding

  Sounding &sndg = Sounding::inst();
  sndg.retrieveTempProfile(scan_time);

  // identify storms
  
  if (_identify.run(scan_num)) {
    _fatalError = false;
    return -1;
  }
    
  // perform tracking
  
  if (_params.perform_tracking) {
    if (scan_num == 0) {
      if (tracking.PrepareNewFile()) {
	if (tracking.fatalError()) {
	  _fatalError = true;
	} else {
	  _fatalError = false;
	}
	return -1;
      }
    } else {
      if (tracking.TrackLastScan()) {
	if (tracking.fatalError()) {
	  _fatalError = true;
	} else {
	  _fatalError = false;
	}
	return -1;
      }
    }
  }

  return 0;
  
}
    
/////////////////////////////////////////
// _loadHeaderFilePath()

void StormIdent::_loadHeaderFilePath(const time_t file_date)

{

  date_time_t path_time;
  path_time.unix_time = file_date;
  uconvert_from_utime(&path_time);

  sprintf(_headerFilePath, "%s%s%.4d%.2d%.2d.%s",
	  _params.storm_data_dir,
	  PATH_DELIM,
	  path_time.year, path_time.month, path_time.day,
	  STORM_HEADER_FILE_EXT);
  
}

/////////////////////////////////////////////////////
// _loadStormParams()
//
// Load up storm file params

void StormIdent::_loadStormParams(storm_file_params_t *sparams)

{
  
  // zero out

  memset (sparams, 0, sizeof(storm_file_params_t));

  // fill in parts of file header
  
  sparams->low_dbz_threshold = _params.low_dbz_threshold;

  sparams->high_dbz_threshold = _params.high_dbz_threshold;

  sparams->hail_dbz_threshold = _params.hail_dbz_threshold;

  sparams->dbz_hist_interval = _params.dbz_hist_interval;

  sparams->base_threshold = _params.base_threshold;

  sparams->top_threshold = _params.top_threshold;

  sparams->min_storm_size = _params.min_storm_size;
 
  sparams->max_storm_size = _params.max_storm_size;
  
  sparams->z_p_coeff = _params.ZR.coeff;
  
  sparams->z_p_exponent = _params.ZR.expon;
  
  sparams->z_m_coeff = _params.ZM.coeff;
  
  sparams->z_m_exponent = _params.ZM.expon;

  sparams->sectrip_vert_aspect = _params.sectrip_vert_aspect;

  sparams->sectrip_horiz_aspect = _params.sectrip_horiz_aspect;

  sparams->sectrip_orientation_error = _params.sectrip_orientation_error;

  sparams->n_poly_sides = N_POLY_SIDES;

  sparams->poly_start_az = 0.0;

  sparams->poly_delta_az = 360.0 / (double) N_POLY_SIDES;

  sparams->vel_available = _params.vel_available;

  sparams->hail_z_m_coeff = _params.hail_ZM.coeff;
  
  sparams->hail_z_m_exponent = _params.hail_ZM.expon;

  // Indication of how to interpret the add_on union in gprops

  if (_params.hail_detection_mode == Params::HAIL_METRICS) {
    sparams->gprops_union_type = UNION_HAIL;
  } else if (_params.hail_detection_mode == Params::NEXRAD_HDA) {
    sparams->gprops_union_type = UNION_NEXRAD_HDA;
  } else {
    sparams->gprops_union_type = UNION_NONE;
  }

  if (_params.set_dbz_threshold_for_tops) {
    sparams->tops_dbz_threshold = _params.tops_dbz_threshold;
  } else {
    sparams->tops_dbz_threshold = _params.low_dbz_threshold;
  }
  
  if (_params.precip_computation_mode ==
      Params::PRECIP_FROM_COLUMN_MAX) {
    sparams->precip_computation_mode = TITAN_PRECIP_FROM_COLUMN_MAX;
    sparams->precip_plane_ht = -9999;
  } else if (_params.precip_computation_mode ==
             Params::PRECIP_AT_SPECIFIED_HT) {
    sparams->precip_computation_mode = TITAN_PRECIP_AT_SPECIFIED_HT;
    sparams->precip_plane_ht = _params.precip_plane_ht;
  } else if (_params.precip_computation_mode ==
             Params::PRECIP_AT_LOWEST_VALID_HT) {
    sparams->precip_computation_mode = TITAN_PRECIP_AT_LOWEST_VALID_HT;
    sparams->precip_plane_ht = _params.base_threshold;
  } else if (_params.precip_computation_mode ==
             Params::PRECIP_FROM_LOWEST_AVAILABLE_REFL) {
    sparams->precip_computation_mode = TITAN_PRECIP_FROM_LOWEST_AVAILABLE_REFL;
    sparams->precip_plane_ht = _params.base_threshold;
  }

}

///////////////////////////////////////////////////////////////////////////
//
// Prepares the storm data file. If the file does not exist, it is created
// and the header is prepared and written. If the file exists and the params
// in the header do not match the current ones, the old file is overwritten.
// If the file exists and the parameters do match, the old file is opened for
// read/write. The file is read and the scans in it are compared with those
// in the time_list. The scan number and file markers are positioned
// immediately after the last scan which matches the file list.
// Processing will continue from this point.
//
///////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////
//
// Open file and check parameters. If params do not
// match, or there are no scans in file, we need to
// start a new file.
//
// Returns 0 on success, -1 on failure

int StormIdent::_openAndCheck(const char *header_file_path,
			      const storm_file_params_t *expected_params)
  
{
  
  // check if the file exists - if it does not exist, go to
  // end of loop
  
  struct stat file_stat;

  if (stat(header_file_path, &file_stat)) {
    // file does not exist, so no scans match
    return (-1);
  }
  
  // try to open the file read/write
  
  if (_sfile.OpenFiles("r", header_file_path,
		       STORM_DATA_FILE_EXT)) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      fprintf(stderr, "%s:StormIdent::_openAndCheck\n",
	      _progName.c_str());
      fprintf(stderr, "Storm file corrupted - preparing new file.\n");
    }
    return (-1);
  }
  
  // read in storm properties file header
  
  if (_sfile.ReadHeader()) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      fprintf(stderr, "%s:StormIdent::_openAndCheck\n",
	      _progName.c_str());
      fprintf(stderr, "Storm file corrupted - preparing new file.\n");
    }
    return (-1);
  }
  
  // compare the old file params with the current ones - if they do
  // not match, no scans match
  
  if (memcmp(&_sfile._header.params, expected_params,
	     (size_t) sizeof(storm_file_params_t))) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      fprintf(stderr,
	      "WARNING - %s:StormIdent::_openAndCheck\n",
	      _progName.c_str());
      fprintf(stderr, "Parameters do not match.\n");
      fprintf(stderr, "New file will be started.\n");
    }
    return (-1);
  }

  return (0);

}
  
/////////////////////////////////////////////////////////////////////
//
// Find the current scan, the latest scan in the file
// which matches the timelist.
//
// Returns 0 on success, -1 on error.
//
// Sets current_scan and current_time on success.

int StormIdent::_findCurrentScan(const DsMdvxTimes &time_list,
				 int &current_scan,
				 time_t &current_time)
  
{

  // Search through the storms file for last time
  // which matches time list
  
  current_scan = -1;
  current_time = 0;
  const vector<time_t> &times = time_list.getArchiveList();
  
  for (int iscan = 0; iscan < _sfile._header.n_scans; iscan++) {
    
    if (iscan > (int) times.size() - 1) {
      return -1;
    }
    
    // read in scan
    
    if (_sfile.ReadScan(iscan)) {
      cerr << "ERROR - " << _progName
	   << "StormIdent::_findCurrentScan" << endl;
      cerr << _sfile.getErrStr() << endl;
      return -1;
    }
    
    if (times[iscan] != _sfile._scan.time) {
      if (iscan == 0) {
	return -1;
      }
      current_scan = iscan - 1;
      current_time = times[current_scan];
      return 0;
    }
    
  } // iscan

  if (_sfile._header.n_scans == 0) {
    return -1;
  }

  current_scan = _sfile._header.n_scans - 1;
  current_time = times[current_scan];

  if (current_scan >= 0 && _params.debug) {
    cerr << "  Storm file already contains scans 0 through "
	 << current_scan << endl;
    cerr << "  First scan time: " << utimstr(times[0]) << endl;
    cerr << "  Last  scan time: " << utimstr(current_time) << endl;
  }

  return 0;

}
//////////////////////
// Prepare a new file
//
// Returns 0 on success, -1 on failure

int StormIdent::_prepareNew(const char *header_file_path,
			    const storm_file_params_t *sparams)
     
{
  
  // open the file write/read
  
  if (_sfile.OpenFiles("w+", header_file_path,
		       STORM_DATA_FILE_EXT)) {
    cerr << "ERROR - " << _progName << "StormIdent::_prepareNew" << endl;
    cerr << _sfile.getErrStr() << endl;
    return(-1);
  }
  
  // copy storm file params to header
  
  _sfile._header.params = *sparams;
  
  // write storm file header
  
  if (_sfile.WriteHeader()) {
    cerr << "ERROR - " << _progName << "StormIdent::_prepareNew" << endl;
    cerr << _sfile.getErrStr() << endl;
    return(-1);
  }
  
  // set the data file at the correct point for data writes -
  // after the label
  
  if (_sfile.SeekStartData()) {
    cerr << "ERROR - " << _progName << "StormIdent::_prepareNew" << endl;
    cerr << _sfile.getErrStr() << endl;
    return(-1);
  }
  
  // flush the buffer
  
  _sfile.FlushFiles();

  return (0);
  
}

//////////////////////
// prepareOld()
//
// Prepare a old file
//
// Returns 0 on success, -1 on failure

int StormIdent::_prepareOld(const char *header_file_path,
			    int current_scan_num)
     
{

  int trunc_flag;
  int init_data_len = 0;
  int init_header_len = 0;

  // open the file read/write
  
  if (_sfile.OpenFiles("r+", header_file_path, STORM_DATA_FILE_EXT)) {
    cerr << "ERROR - " << _progName << "StormIdent::_prepareOld" << endl;
    cerr << _sfile.getErrStr() << endl;
    return(-1);
  }
  
  // read in header
  
  if (_sfile.ReadHeader()) {
    cerr << "ERROR - " << _progName << "StormIdent::_prepareOld" << endl;
    cerr << _sfile.getErrStr() << endl;
    return(-1);
  }
  
  // read in last valid scan
  
  if (_sfile.ReadScan(current_scan_num)) {
    cerr << "ERROR - " << _progName << "StormIdent::_prepareOld" << endl;
    cerr << "  Reading in last valid scan #: " << current_scan_num << endl;
    cerr << _sfile.getErrStr() << endl;
    return(-1);
  }
  
  // check whether file will need truncation
  
  if (current_scan_num < _sfile._header.n_scans - 1) {
    init_data_len = _sfile._scan.last_offset + 1;
    init_header_len = (R_FILE_LABEL_LEN + sizeof(storm_file_header_t) +
		       (current_scan_num + 1) * sizeof(si32));
    trunc_flag = TRUE;
  } else {
    trunc_flag =  FALSE;
  }
  
  // copy scan time to header as end time
  
  _sfile._header.end_time = _sfile._scan.time;
  
  // set other parameters
  
  _sfile._header.n_scans = current_scan_num + 1;
  
  // write storm file header
  
  if (_sfile.WriteHeader()) {
    cerr << "ERROR - " << _progName << "StormIdent::_prepareOld" << endl;
    cerr << _sfile.getErrStr() << endl;
    return(-1);
  }
  
  // truncate if necessary, and position the file
  
  if (trunc_flag) {
    
    if (_sfile.TruncateHeaderFile(init_header_len)) {
      cerr << "ERROR - " << _progName << "StormIdent::_prepareOld" << endl;
      cerr << _sfile.getErrStr() << endl;
      return (-1);
    }

    if (_sfile.TruncateDataFile(init_data_len)) {
      cerr << "ERROR - " << _progName << "StormIdent::_prepareOld" << endl;
      cerr << _sfile.getErrStr() << endl;
      return (-1);
    }
    
  }
  
  // position at end of file
  
  if (_sfile.SeekEndData()) {
    cerr << "ERROR - " << _progName << "StormIdent::_prepareOld" << endl;
    cerr << _sfile.getErrStr() << endl;
    return(-1);
  }
  
  // flush the buffer
  
  _sfile.FlushFiles();
  
  return (0);
  
}

//////////////////////////////
// writeLdataInfo()
//
// Write latest data info

int StormIdent::_writeLdataInfo()

{
  
  // parse storm file path to get base part of path
  // (i.e. name without extension)

  Path path(_sfile._header_file_path);

  // write info

  string outputDir = _params.storm_data_dir;
  if (outputDir[0] != '/' && outputDir[0] != '.') {
    outputDir = "./";
    outputDir += _params.storm_data_dir;
    cerr << "WARNING - _writeLdataInfo" << endl;
    cerr << "  storm_data_dir is not absolute path: "
         << _params.storm_data_dir << endl;
    cerr << "  Generally this should start with '.' or '/'" << endl;
    cerr << "  Writing _latest_data_info to: " << outputDir << endl;
  }
  DsLdataInfo ldata(outputDir,
		    _params.debug >= Params::DEBUG_EXTRA);
  
  ldata.setDataFileExt(STORM_HEADER_FILE_EXT);
  ldata.setWriter(_progName.c_str());
  ldata.setRelDataPath(path.getFile().c_str());
  ldata.setUserInfo1(path.getBase().c_str());

  if (ldata.write(_sfile._header.end_time, "titan")) {
    cerr << "ERROR - " << _progName << "StormIdent::_writeLdataInfo" << endl;
    cerr << "Cannot write index file to dir: "
	 << outputDir << endl;
    return -1;
  }
  
  return 0;
  
}

