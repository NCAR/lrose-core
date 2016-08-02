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
#include "FileLock.hh"
#include "Shmem.hh"
#include "DataTimes.hh"
#include "InputMdv.hh"
#include "TimeList.hh"
#include "StormFile.hh"
#include "Identify.hh"

#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>
using namespace std;

// Constructor

StormIdent::StormIdent(int argc, char **argv)

{

  OK = TRUE;
  _args = NULL;
  _params = NULL;
  _fileLock = NULL;
  _dataTimes = NULL;
  _inputMdv = NULL;
  shMem = NULL;
  _headerFilePath = NULL;

  // set programe name

  _progName = STRdup("StormIdent");
  ucopyright(_progName);

  // get command line args

  _args = new Args(argc, argv, _progName);
  if (!_args->OK) {
    fprintf(stderr, "ERROR: %s\n", _progName);
    fprintf(stderr, "Problem with command line args\n");
    OK = FALSE;
    return;
  }

  // get TDRP params

  _params = new Params();
  _paramsPath = "unknown";
  if (_params->loadFromArgs(argc, argv,
			    _args->override.list,
			    &_paramsPath)) {
    fprintf(stderr, "ERROR: %s\n", _progName);
    fprintf(stderr, "Problem with TDRP parameters\n");
    OK = FALSE;
    return;
  }

  // check start and end in ARCHIVE mode

  if ((_params->mode == Params::ARCHIVE) &&
      (_args->startTime == 0 || _args->endTime == 0)) {
    fprintf(stderr, "ERROR - %s\n", _progName);
    fprintf(stderr,
	    "In ARCHIVE mode start and end times must be specified.\n");
    fprintf(stderr, "Run '%s -h' for usage\n", _progName);
    OK = FALSE;
    return;
  }

  // init process mapper registration

  PMU_auto_init(_progName, _params->instance, PROCMAP_REGISTER_INTERVAL);

  // create file lock

  _fileLock = new FileLock(_progName, _params);
  if (!_fileLock || !_fileLock->OK) {
    OK = FALSE;
    return;
  }

  // create and initialize the shared memory if we are
  // tracking

  if (_params->perform_tracking) {

    shMem = new Shmem(_progName, _params);
    if (!shMem || shMem->create()) {
      OK = FALSE;
      return;
    }

    // start storm tracking if required

    if (_params->start_storm_track) {
      char command_line[BUFSIZ];
      STRncopy(command_line, _params->storm_track_command_line, BUFSIZ);
      // make sure there is a trailing &
      if (strchr(command_line, '&') == NULL) {
	STRconcat(command_line, " &", BUFSIZ);
      }
      system(command_line);
      // wait 1 sec
      uusleep(1000000);
    }

 }

  // create data times object

  if (_params->mode == Params::REALTIME) {
    _dataTimes = new DataTimes(_progName, _params);
  } else {
    // ARCHIVE mode
    _dataTimes = new DataTimes(_progName, _params,
			       _args->startTime, _args->endTime);
  }

  // create input MDV file object

  _inputMdv = new InputMdv(_progName, _params);
  if (!_inputMdv || !_inputMdv->OK) {
    OK = FALSE;
    return;
  }

  // create time list object

  _timeList = new TimeList(_progName, _params);
  if (!_timeList || !_timeList->OK) {
    OK = FALSE;
    return;
  }

  // create storm file object

  _stormFile = new StormFile(_progName, _params);
  if (!_stormFile || !_stormFile->OK) {
    OK = FALSE;
    return;
  }

  // create storm identification object

  _identify = new Identify(_progName, _params, _inputMdv, _stormFile);
  if (!_identify || !_identify->OK) {
    OK = FALSE;
    return;
  }

  
  // malloc space for header path
  
  int nbytes_path = (strlen(_params->storm_data_dir) + 
		     strlen(PATH_DELIM) +
		     strlen(STORM_HEADER_FILE_EXT) + 10);
  _headerFilePath = (char *) umalloc(nbytes_path);

  return;

}

// destructor

StormIdent::~StormIdent()

{

  // unregister process

  PMU_auto_unregister();

  // free up

  if (_identify) {
    delete(_identify);
  }
  if (_stormFile) {
    delete(_stormFile);
  }
  if (_timeList) {
    delete(_timeList);
  }
  if (_inputMdv) {
    delete(_inputMdv);
  }
  if (_dataTimes) {
    delete(_dataTimes);
  }
  if (shMem) {
    delete(shMem);
  }
  if (_fileLock) {
    delete(_fileLock);
  }
  if (_params) {
    delete(_params);
  }
  if (_args) {
    delete(_args);
  }
  if (_headerFilePath) {
    STRfree(_headerFilePath);
  }
  STRfree(_progName);

}

//////////////////////////////////////////////////
// Run

int StormIdent::Run ()
{

  // print startup message

  _printStartupMessage();

  // register with procmap

  PMU_auto_register("StormIdent::Run");

  // get the start and end times for the analysis
  
  time_t start_time, end_time;
  _dataTimes->getStartupTimeLimits(&start_time, &end_time);
  
  // load up the list of times for the analysis
  
  _timeList->load(start_time, end_time, _dataTimes->getRestartTime());
  
  if (_params->mode == Params::ARCHIVE) {
    if (_timeList->nTimes < 1) {
      fprintf(stderr, "NOTE - %s:_run - ARCHIVE mode\n", _progName);
      fprintf(stderr, "No times found\n");
      if (_params->auto_restart &&
	  _dataTimes->getRestartTime() != -1 &&
	  start_time < _dataTimes->getRestartTime() &&
	  start_time < _args->endTime) {
	time_t restart_time = _dataTimes->getRestartTime();
	_newStartTime = (restart_time - _params->restart_overlap_period);
	cerr << "  end_time: " << utimstr(end_time) << endl;
	cerr << "  restart_time: " << utimstr(restart_time) << endl;
	cerr << "  _newStartTime: " << utimstr(_newStartTime) << endl;
	cerr << "  Restarting .... " << endl;
	return EXIT_AND_RESTART;
      } else {
	return (0);
      }
    }
  }
  
  // get date on which file name is based

  time_t file_date = _dataTimes->getStormFileDate();
  
  // load header file path

  _loadHeaderFilePath(file_date);

  // load storm params - this is used for checking with existing
  // files to ensure the params have not changed.
  
  storm_file_params_t sparams;
  _loadStormParams(&sparams);

  // check storm file

  int prev_scan_num;
  int time_list_start_posn = 0;

  if (_stormFile->openAndCheck(_headerFilePath, &sparams) == 0) {

    // get prev scan number analyzed - this also modifies
    // time_list and ntimes_in_list so that the list 
    // contains only entries which have not yet been
    // analyzed.
    
    prev_scan_num = _stormFile->getPrevScan(*_timeList, time_list_start_posn);

  } else {

    // old file invalid

    prev_scan_num = -1;

  }

  // prepare file

  if (prev_scan_num < 0) {
    // prepare new file
    if (_stormFile->prepareNew(_headerFilePath, &sparams)) {
      return (-1);
    }
  } else {
    // prepare old file
    if (_stormFile->prepareOld(_headerFilePath, prev_scan_num)) {
      return (-1);
    }
  }
    
  // If tracking is to be performed, prepare the storm track file
  // for appending by tracking up to the current scan
  
  if (_params->perform_tracking) {
    if (prev_scan_num >= 0) {
      if (shMem->performTracking(_headerFilePath, PREPARE_FOR_APPEND)) {
	return (-1);
      }
    }
  }
  
  // call archive or realtime processing
  
  if (_params->mode == Params::ARCHIVE) {
    
    return (_runArchive(prev_scan_num, start_time, end_time,
			time_list_start_posn));
    
  } else {

    return (_runRealtime(prev_scan_num, end_time,
			 time_list_start_posn));
    
  }

  return (0);

}

      
/////////////////////////////
// startup message to stdout

#define BOOL_STR(a) (a == 0? "false" : "true")

void StormIdent::_printStartupMessage()

{

  fprintf(stdout, "\n");
  fprintf(stdout, "STORM_IDENT\n");
  fprintf(stdout, "===========\n");
  fprintf(stdout, "\n");
  
  fprintf(stdout, "Low dBZ threshold : %g\n",
	  _params->low_dbz_threshold);
  fprintf(stdout, "High dBZ threshold : %g\n",
	  _params->high_dbz_threshold);
  fprintf(stdout, "dBZ hist interval : %g\n",
	  _params->dbz_hist_interval);
  fprintf(stdout, "hail dBZ threshold : %g\n",
	  _params->hail_dbz_threshold);
  fprintf(stdout, "Base threshold (km) : %g\n",
	  _params->base_threshold);
  fprintf(stdout, "Top threshold (km) : %g\n",
	  _params->top_threshold);

  fprintf(stdout, "Min storm size (km2 or km3) : %g\n",
	    _params->min_storm_size);
  fprintf(stdout, "Max storm size (km2 or km3) : %g\n",
	  _params->max_storm_size);

  fprintf(stdout, "Z-R coefficient : %g\n", _params->ZR.coeff);
  fprintf(stdout, "Z-R exponent : %g\n", _params->ZR.expon);

  fprintf(stdout, "Z-M coefficient : %g\n", _params->ZM.coeff);
  fprintf(stdout, "Z-M exponent : %g\n", _params->ZM.expon);

  fprintf(stdout, "2nd trip vert aspect : %g\n",
	  _params->sectrip_vert_aspect);
  fprintf(stdout, "2nd trip horiz aspect : %g\n",
	  _params->sectrip_horiz_aspect);
  fprintf(stdout, "2nd trip orientation error : %g\n",
	  _params->sectrip_orientation_error);
  fprintf(stdout, "Velocity data available? : %s\n",
	  BOOL_STR(_params->vel_available));
  
  fprintf(stdout, "\n\n");

}
  
/////////////////////////////////////////
// _loadHeaderFilePath()

void StormIdent::_loadHeaderFilePath(const time_t file_date)

{

  date_time_t path_time;
  path_time.unix_time = file_date;
  uconvert_from_utime(&path_time);

  sprintf(_headerFilePath, "%s%s%.4d%.2d%.2d.%s",
	  _params->storm_data_dir,
	  PATH_DELIM,
	  path_time.year, path_time.month, path_time.day,
	  STORM_HEADER_FILE_EXT);
  
}

////////////////
// _runArchive()

int StormIdent::_runArchive(int prev_scan_num,
			    time_t start_time,
			    time_t end_time,
			    int time_list_start_posn)

{

  // loop through the input radar MDV files

  int scan_num = prev_scan_num + 1;
  
  for (int itime = time_list_start_posn; itime < _timeList->nTimes; itime++) {

    PMU_auto_register("Processing archive data");
    
    // read in MDV data
      
    if (_inputMdv->read(_timeList->times[itime]) == 0) {

      // identify storms
      
      if (_identify->perform(scan_num)) {
	return (-1);
      }
      
      // perform tracking
      
      if (_params->perform_tracking) {
	if (scan_num == 0) {
	  shMem->performTracking(_headerFilePath, PREPARE_NEW_FILE);
	} else {
	  shMem->performTracking(_headerFilePath, TRACK_LAST_SCAN);
	}
      }
      
      // write latest data info file
      
      _stormFile->writeLdataInfo();
      
      // increment scan num
      
      scan_num++;

    } // if (_inputMdv->read(_timeList->times[itime]) == 0)
      
  } // itime

  // restart?

  if (_params->auto_restart &&
      _dataTimes->getRestartTime() != -1 &&
      end_time > _timeList->times[_timeList->nTimes - 1]) {

    if (_params->debug) {
      cerr << "------->> Archive mode restart" << endl;
    }
    
    if (_prepareRestart() == 0) {
      return (EXIT_AND_RESTART);
    } else {
      if (_params->debug >= Params::DEBUG_NORM) {
	fprintf(stderr, "WARNING: %s:StormIdent::_runRealtime\n",
		_progName);
	fprintf(stderr, "Cannot restart - continuing\n");
      }
    }
    
  }
  
  return (0);

}

/////////////////
// _runRealtime()

int StormIdent::_runRealtime(int prev_scan_num,
			     time_t end_time,
			     int time_list_start_posn)

{

  // initialize

  int start_posn = time_list_start_posn;
  time_t endTime = end_time;
  time_t startTime;

  int scan_num = prev_scan_num + 1;
  int first_pass = TRUE;
  time_t prev_time = 0;
    
  // loop forever, getting a new times list each loop - under normal
  // circumstances the list will contain only a single entry

  int forever = TRUE;
  while (forever) {
    
    PMU_auto_register("Processing realtime data");
    
    // process the times list
    
    for (int itime = start_posn;
	 itime < _timeList->nTimes; itime++) {
      
      char pmu_message[128];
      sprintf(pmu_message, "Processing time %d\n", itime);
      PMU_auto_register(pmu_message);
      
      // on subsequent passes, check for restart conditions
      
      if (!first_pass) {
	
	time_t elapsed_time = _timeList->times[itime] - prev_time;
	
	// if elapsed time is negative exit and restart
	
	if (elapsed_time < 0) {
	  fprintf(stderr, "NOTE: %s\n", _progName);
	  fprintf(stderr, "Restarting - time moved into the past\n");
	  fprintf(stderr, "Prev time: %s\n", utimstr(prev_time));
	  fprintf(stderr, "This time: %s\n", utimstr(_timeList->times[itime]));
	  return(EXIT_AND_RESTART);
	}
	
	// if program is in auto-restart mode and the restart time
	// has passed, restart

	if (_params->auto_restart &&
	    _timeList->times[itime] >= _dataTimes->getRestartTime()) {
	  
	  if (_prepareRestart() == 0) {
	    return (EXIT_AND_RESTART);
	  } else {
	    if (_params->debug >= Params::DEBUG_NORM) {
	      fprintf(stderr, "WARNING: %s:StormIdent::_runRealtime\n",
		      _progName);
	      fprintf(stderr, "Cannot restart - continuing\n");
	    }
	  }
	  
	} // if (_params->auto_restart ...
	
      } else {
	
	first_pass = FALSE;
	
      } // if (!first_pass)
      
      // read in MDV data
      
      if (_inputMdv->read(_timeList->times[itime]) == 0) {
	
	// identify storms
	
	if (_identify->perform(scan_num)) {
	  return (-1);
	}

	// perform tracking
	
	if (_params->perform_tracking) {
	  if (scan_num == 0) {
	    shMem->performTracking(_headerFilePath, PREPARE_NEW_FILE);
	  } else {
	    shMem->performTracking(_headerFilePath, TRACK_LAST_SCAN);
	  }
	}
	
	// write latest data info file
	
	_stormFile->writeLdataInfo();
	
	// increment scan num
	
	scan_num++;

	// store prev time
	
	prev_time = _timeList->times[itime];
	
      } // if (_inputMdv->read(_timeList->times[itime]) == 0)
      
    } // itime
  
    // get a new times list each loop - under normal
    // circumstances the list will contain only a single entry
    
    startTime = endTime + 1;
    endTime = _dataTimes->getLatest();
    _timeList->load(startTime, endTime);
    start_posn = 0;

  } // while (forever)

  return (0);

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
  
  sparams->low_dbz_threshold = _params->low_dbz_threshold;

  sparams->high_dbz_threshold = _params->high_dbz_threshold;

  sparams->hail_dbz_threshold = _params->hail_dbz_threshold;

  sparams->dbz_hist_interval = _params->dbz_hist_interval;

  sparams->base_threshold = _params->base_threshold;

  sparams->top_threshold = _params->top_threshold;

  sparams->min_storm_size = _params->min_storm_size;
 
  sparams->check_morphology = _params->check_morphology;

  sparams->morphology_erosion_threshold =
    _params->morphology_erosion_threshold;

  sparams->morphology_refl_divisor =
    _params->morphology_refl_divisor;

  sparams->check_tops = _params->check_tops;

  sparams->min_radar_tops = _params->min_radar_tops;

  sparams->tops_edge_margin = _params->tops_edge_margin;

  sparams->max_storm_size = _params->max_storm_size;
  
  sparams->z_p_coeff = _params->ZR.coeff;
  
  sparams->z_p_exponent = _params->ZR.expon;
  
  sparams->z_m_coeff = _params->ZM.coeff;
  
  sparams->z_m_exponent = _params->ZM.expon;

  sparams->sectrip_vert_aspect = _params->sectrip_vert_aspect;

  sparams->sectrip_horiz_aspect = _params->sectrip_horiz_aspect;

  sparams->sectrip_orientation_error = _params->sectrip_orientation_error;

  sparams->n_poly_sides = N_POLY_SIDES;

  sparams->poly_start_az = 0.0;

  sparams->poly_delta_az = 360.0 / (double) N_POLY_SIDES;

  sparams->vel_available = _params->vel_available;

}

////////////////////
// _prepareRestart()
//
// Do a restart on the program - this only occurs in realtime mode
//

int StormIdent::_prepareRestart()

{

  // determine new_start_time - the new file overlaps the old
  // one by restart_overlap_period
  
  time_t restart_time = _dataTimes->getRestartTime();
  _newStartTime = (restart_time -
		   _params->restart_overlap_period);

  // check that we have data after the new_restart_time - if not,
  // do not prepare a new storm file
  
  if (_newStartTime < _stormFile->handle.header->end_time) {
    if (_prepareRestartStormFile(restart_time, _newStartTime) == -1) {
      return (-1);
    }
  }
  
  RfCloseStormFiles(&_stormFile->handle, "StormIdent::_prepareRestart");
  
  if (_params->remove_old_files_on_restart) {
    
    if (unlink(_stormFile->handle.header_file_path)) {
      if (_params->debug >= Params::DEBUG_NORM) {
	fprintf(stderr, "WARNING - %s:StormIdent::_prepareRestart\n",
		_progName);
	fprintf(stderr, "Cannot remove old storm header file\n");
	perror(_stormFile->handle.header_file_path);
      }
    }
    
    if (unlink(_stormFile->handle.data_file_path)) {
      if (_params->debug >= Params::DEBUG_NORM) {
	fprintf(stderr, "WARNING - %s:StormIdent::_prepareRestart\n",
		_progName);
	fprintf(stderr, "Cannot remove old storm data file\n");
	perror(_stormFile->handle.data_file_path);
      }
    }
    
  } // _params->remove_old_files_on_restart
  
  return (0);

}

/////////////////////////////////////////////////////////////////////
// _prepareRestartStormFile()
//
// Prepares the storm file for a next date - copies over a set number
// of scans from the previous day's data to provide some history
// 
// Returns 0 if success, -1 if cannot prepare new file. The latter
// condition applies when the old file and new file have the same
// name, in which case we don't want to start a new file.
//

int StormIdent::_prepareRestartStormFile(time_t restart_time,
					 time_t new_start_time)

{

  if (_params->debug >= Params::DEBUG_VERBOSE) {
    fprintf(stderr,
	    "%s:_prepareRestartStormFile - creating file for next day\n",
	    _progName);
    fprintf(stderr, "Restart time is %s\n", utimstr(restart_time));
  }
  
  // read in header
  
  if (RfReadStormHeader(&_stormFile->handle, "_prepareRestartStormFile")) {
    fprintf(stderr, "ERROR - %s:_prepareRestartStormFile\n",
	    _progName);
    fprintf(stderr, "Reading old file header.\n");
    return(-1);
  }
  
  // determine the date for the new file form the restart time
  
  date_time_t file_path_time;
  file_path_time.unix_time = restart_time;
  uconvert_from_utime(&file_path_time);
  
  // set up file path for new file

  path_parts_t old_path_parts;
  uparse_path(_stormFile->handle.header_file_path, &old_path_parts);

  char new_file_path[MAX_PATH_LEN];
  sprintf(new_file_path, "%s%.2d%.2d%.2d%s",
	  old_path_parts.dir,
	  file_path_time.year, file_path_time.month, file_path_time.day,
	  old_path_parts.ext);

  ufree_parsed_path(&old_path_parts);

  if (_params->debug >= Params::DEBUG_VERBOSE) {
    fprintf(stderr, "New file : %s\n", new_file_path);
  }

  // if the old and new files have the same name, don't
  // prepare new file - return error condition
  
  if (strcmp(new_file_path, _stormFile->handle.header_file_path) == 0) {
    return (-1);
  }

  // initialize new storm file handle
  
  storm_file_handle_t new_s_handle;
  RfInitStormFileHandle(&new_s_handle, _progName);

  // open new storm properties file
  
  if (RfOpenStormFiles (&new_s_handle, "w",
			new_file_path,
			STORM_DATA_FILE_EXT,
			"_prepareRestartStormFile")) {
    
    fprintf(stderr, "ERROR - %s:_prepareRestartStormFile\n", _progName);
    fprintf(stderr, "Creating new storm file %s.\n", new_file_path);
    return(-1);
  }

  // allocate header

  if (RfAllocStormHeader(&new_s_handle, "_prepareRestartStormFile")) {
    return(-1);
  }

  // copy old file header to new file

  *new_s_handle.header = *_stormFile->handle.header;

  // set the new file at the correct point for data writes - after 
  // the label

  if (RfSeekStartStormData(&new_s_handle, "_prepareRestartStormFile")) {
    return(-1);
  }

  // loop through the scans to be copied

  int n_scans = _stormFile->handle.header->n_scans;
  int new_scan_num = 0;

  for (int iscan = 0; iscan < n_scans; iscan++) {

    // read in scan

    if (RfReadStormScan(&_stormFile->handle, iscan,
			"_prepareRestartStormFile")) {
      return(-1);
    }
    
    time_t scan_time = _stormFile->handle.scan->time;

    // check time - if before the new start time, skip ahead

    if (scan_time < new_start_time) {
      continue;
    }
    
    // set data in header as applicable
    
    if (new_scan_num == 0) {
      new_s_handle.header->start_time = scan_time;
    }
    new_s_handle.header->end_time = scan_time;
    new_s_handle.header->n_scans = new_scan_num + 1;
    
    if (_params->debug >= Params::DEBUG_VERBOSE) {
      fprintf(stderr, "Copying scan %d, time %s\n", iscan,
	      utimstr(scan_time));
    }
    
    // allocate scan and gprops array
    
    int nstorms = _stormFile->handle.scan->nstorms;
    
    RfAllocStormScan(&new_s_handle,
		     nstorms,
		     "_prepareRestartStormFile");
    
    // copy scan struct over to new index

    *new_s_handle.scan = *_stormFile->handle.scan;
    new_s_handle.scan->scan_num = new_scan_num;
    
    // copy over global props

    memcpy (new_s_handle.gprops,
            _stormFile->handle.gprops,
            (nstorms * sizeof(storm_file_global_props_t)));

    // loop through storms

    for (int istorm = 0; istorm < nstorms; istorm++) {
      
      storm_file_global_props_t *gprops = 
	_stormFile->handle.gprops + istorm;

      // read in layer, hist and runs props for storm
      
      if (RfReadStormProps(&_stormFile->handle, istorm,
			   "_prepareRestartStormFile")) {
	return(-1);
      }
      
      // allocate layer, hist and run arrays
      
      RfAllocStormProps(&new_s_handle,
			gprops->n_layers,
			gprops->n_dbz_intervals,
			gprops->n_runs,
			gprops->n_proj_runs,
			"_prepareRestartStormFile");
      
      // copy over the layer, hist and runs data

      memcpy ((void *) new_s_handle.layer,
              (void *) _stormFile->handle.layer,
              (size_t) (gprops->n_layers *
			sizeof(storm_file_layer_props_t)));

      memcpy ((void *) new_s_handle.hist,
              (void *) _stormFile->handle.hist,
              (size_t) (gprops->n_dbz_intervals *
			sizeof(storm_file_dbz_hist_t)));
      
      memcpy ((void *) new_s_handle.runs,
              (void *) _stormFile->handle.runs,
              (size_t) (gprops->n_runs *
			sizeof(storm_file_run_t)));
      
      memcpy ((void *) new_s_handle.proj_runs,
              (void *) _stormFile->handle.proj_runs,
              (size_t) (gprops->n_proj_runs *
			sizeof(storm_file_run_t)));
      
      // write layer, hist and runs data to file
      
      if (RfWriteStormProps(&new_s_handle, istorm,
			    "_prepareRestartStormFile")) {
	return(-1);
      }

    } // istorm
    
    // write scan header and global props

    if (RfWriteStormScan(&new_s_handle, new_scan_num,
			 "_prepareRestartStormFile")) {
      return(-1);
    }

    new_scan_num++;

  } // iscan

  // write new header

  if (RfWriteStormHeader(&new_s_handle,
			 "prepare_nextstorm_file")) {
    return(-1);
  }

  // close files

  RfCloseStormFiles (&new_s_handle, "_prepareRestartStormFile");
  
  // free resources

  RfFreeStormProps(&new_s_handle, "_prepareRestartStormFile");
  RfFreeStormScan(&new_s_handle, "_prepareRestartStormFile");
  RfFreeStormHeader(&new_s_handle, "_prepareRestartStormFile");

  return (0);

}




