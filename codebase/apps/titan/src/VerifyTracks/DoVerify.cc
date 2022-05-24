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
// DoVerify.cc
//
// DoVerify object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
//
// Nov 2005
//
///////////////////////////////////////////////////////////////
//
// Verify against TITAN tracks themselves
//
////////////////////////////////////////////////////////////////

#include "DoVerify.hh"
#include "VerifyTracks.hh"
#include <toolsa/str.h>
#include <rapmath/math_macros.h>
#include <rapmath/umath.h>
#include <Mdv/Mdvx.hh>
#include <Mdv/MdvxField.hh>

using namespace std;

// Constructor

DoVerify::DoVerify(const string &prog_name,
                   const Params &params) :
        _progName(prog_name),
        _params(params)

{

  _gridType = -1;
  _nx = -1;
  _ny = -1;
  _nbytesGrid = 0;
  _forecastGrid = NULL;
  _truthGrid = NULL;
  _visited = NULL;

  // initialize stats structs

  MEM_zero(_totalCount);
  MEM_zero(_totalStats);

  // initialize storm and track file handles

  RfInitStormFileHandle(&_sHandle, _progName.c_str());
  RfInitTrackFileHandle(&_tHandle, _progName.c_str());

  return;

}

//////////////
// destructor

DoVerify::~DoVerify()
  
{

  // free up

  if (_forecastGrid) {
    ufree2((void **) _forecastGrid);
  }
  if (_truthGrid) {
    ufree2((void **) _truthGrid);
  }
  if (_visited) {
    ufree(_visited);
  }

}

//////////////////////////
// process this track file

int DoVerify::processFile(const char *trackFilePath)
  
{
  
  if (_params.debug) {
    cerr << "Processing track file: " << trackFilePath << endl;
  }

  
  if (_openFiles(trackFilePath)) {
    cerr << "ERROR - DoVerify::_processFile" << endl;
    cerr << " Opening file" << endl;
    return -1;
  }

  if (_loadScanTimes()) {
    cerr << "ERROR - DoVerify::_processFile" << endl;
    cerr << " Loading scan times" << endl;
    return -1;
  }

  if (_params.verify_method == Params::VERIFY_MDV) {
    if (_performMdvVerification()) {
      cerr << "ERROR - DoVerify::_processFile" << endl;
      cerr << " Performing MDV verification" << endl;
      return -1;
    }
  } else {
    if (_performVerification()) {
      cerr << "ERROR - DoVerify::_processFile" << endl;
      cerr << " Performing verification" << endl;
      return -1;
    }
  }

  // save verification parameters

  _saveVerificationParameters();

  // close files

  RfCloseTrackFiles(&_tHandle, "processFile");
  RfCloseStormFiles(&_sHandle, "processFile");

  return 0;

}

//////////////////////////////
// open storm and track files

int DoVerify::_openFiles(const char *trackFilePath)

{

  int grid_type;
  char storm_file_path[MAX_PATH_LEN];
  path_parts_t track_path_parts;

  // open track properties files
  
  if (RfOpenTrackFiles (&_tHandle, "r+",
			trackFilePath,
			NULL,
			"open_files")) {
    
    fprintf(stderr, "ERROR - %s:open_files\n", _progName.c_str());
    fprintf(stderr, "Opening track file '%s'.\n",
	    trackFilePath);
    perror(trackFilePath);
    return -1;

  }

  // read in track file header
  
  if (RfReadTrackHeader(&_tHandle, "open_files")) {
    cerr << "ERROR - DoVerify::_openFiles" << endl;
    cerr << " Reading track header" << endl;
    return -1;
  }
  
  // compute storm data file path

  uparse_path(trackFilePath, &track_path_parts);
  
  sprintf(storm_file_path, "%s%s",
	  track_path_parts.dir,
	  _tHandle.header->storm_header_file_name);

  // open storm file

  if (RfOpenStormFiles (&_sHandle, "r",
			storm_file_path,
			(char *) NULL,
			"open_files")) {
    
    fprintf(stderr, "ERROR - %s:open_files\n", _progName.c_str());
    fprintf(stderr, "Opening storm file '%s'.\n",
	    storm_file_path);
    perror(storm_file_path);
    ufree_parsed_path(&track_path_parts);
    return -1;

  }

  // read in storm properties file header

  if (RfReadStormHeader(&_sHandle, "open_files")) {
    ufree_parsed_path(&track_path_parts);
    cerr << "ERROR - DoVerify::_openFiles" << endl;
    cerr << " Reading storm header" << endl;
    return -1;
  }

  // read in first scan

  if (RfReadStormScan(&_sHandle, 0, "open_files")) {
    ufree_parsed_path(&track_path_parts);
    cerr << "ERROR - DoVerify::_openFiles" << endl;
    cerr << " Reading storm scan" << endl;
    return -1;
  }

  // set the grid details

  if (_nx < 0) {

    if (_params.specify_verify_grid) {

      _nx = _params.verify_grid.nx;
      _ny = _params.verify_grid.ny;
      _minx = _params.verify_grid.minx;
      _miny = _params.verify_grid.miny;
      _dx = _params.verify_grid.dx;
      _dy = _params.verify_grid.dy;

    } else {

      _nx = _sHandle.scan->grid.nx;
      _ny = _sHandle.scan->grid.ny;
      _minx = _sHandle.scan->grid.minx;
      _miny = _sHandle.scan->grid.miny;
      _dx = _sHandle.scan->grid.dx;
      _dy = _sHandle.scan->grid.dy;

    }

    // allocate grids

    _nbytesGrid = _nx * _ny;
    _forecastGrid = (ui08 **) ucalloc2(_ny, _nx, sizeof(ui08));
    _truthGrid = (ui08 **) ucalloc2(_ny, _nx, sizeof(ui08));
    
  } else {

    if (!_params.specify_verify_grid) {
      if (_nx != _sHandle.scan->grid.nx ||
          _ny != _sHandle.scan->grid.ny ||
          _minx != _sHandle.scan->grid.minx ||
          _miny != _sHandle.scan->grid.miny ||
          _dx != _sHandle.scan->grid.dx ||
          _dy != _sHandle.scan->grid.dy) {
        fprintf(stderr, "ERROR - %s:open_files\n", _progName.c_str());
        fprintf(stderr, "  Opening storm file '%s'.\n", storm_file_path);
        fprintf(stderr, "  Grid has changed\n");
        ufree_parsed_path(&track_path_parts);
        return -1;
      }
    }

  } // if (_nx < 0)

  // set the grid type and check for consistency

  grid_type = _sHandle.scan->grid.proj_type;
  
  if (_gridType < 0) {
    
    _gridType = grid_type;

  } else {

    if (grid_type != _gridType) {
      fprintf(stderr, "ERROR - %s:open_files.\n", _progName.c_str());
      fprintf(stderr, "  Grid type has changed.\n");
      ufree_parsed_path(&track_path_parts);
      return -1;
    }

  } /* if (file_num ... */

  // free resources

  ufree_parsed_path(&track_path_parts);

  return 0;

}

///////////////////////////
// load up scan times array

int DoVerify::_loadScanTimes()

{

  _scanTimes.clear();

  for (int iscan = 0; iscan < _sHandle.header->n_scans; iscan++) {
    
    if (RfReadStormScan(&_sHandle, iscan, "load_scan_time")) {
      fprintf(stderr, "ERROR - %s:loadScanTimes\n", _progName.c_str());
      fprintf(stderr, "  Cannot read storm scan.\n");
      fprintf(stderr, "  File %s\n", _sHandle.header_file_path);
      return -1;
    }

    date_time_t stime;
    stime.unix_time = _sHandle.scan->time;
    uconvert_from_utime(&stime);
    _scanTimes.push_back(stime);

  } /* iscan */

  return 0;

}

///////////////////////////
// perform the verification

int DoVerify::_performVerification()

{
  
  int verify_valid;
  
  long icomplex, isimple, istorm;
  long iscan, jscan;
  long nvalid, nverify;
  long nstorms;
  long n_scans;
  long forecast_scan = 0;
  long complex_track_num;
  long simple_track_num;
  long n_simple_tracks;
  long first_scan_searched, last_scan_searched;
  
  double time_diff, min_time_diff;
  double now, forecast_time;
  double forecast_lead_time, closest_lead_time;
  double verify_min_history;
  
  complex_track_params_t *ct_params;

  // storm_file_params_t *sparams;
  track_file_params_t *tparams;
  track_file_forecast_props_t props_current;
  track_file_forecast_props_t props_forecast;
  track_file_forecast_props_t props_verify;

  vt_stats_t complex_stats, file_stats;
  vt_simple_track_t *stracks, *strack;
  vt_storm_t *storm;
  vt_count_t count;
  vt_count_t file_count;
  vt_count_t complex_count;

  n_scans = _sHandle.header->n_scans;
  // sparams = &_sHandle.header->params;
  tparams = &_tHandle.header->params;
  
  MEM_zero(file_count);
  MEM_zero(file_stats);

  // Set the verify_min_history. If the 'verify_before_forecast_time'
  // flag is set, verify_min_history is set to 0. This allows the
  // inclusion of storms which are too young to have been forecast using
  // this system, and hence verifies all convective activity. If the
  // flag is not set, verify_min_history is set to the sum of the
  // forecast_lead_time and the min_valid_history, since this is
  // the minimum history for a forecast to be valid.
  
  if (_params.verify_before_forecast_time) {
    verify_min_history = 0.0;
  } else {
    verify_min_history =
      _params.forecast_lead_time + _params.forecast_min_history;
  }

  // loop through the complex tracks
  
  for (icomplex = 0;
       icomplex < _tHandle.header->n_complex_tracks; icomplex++) {
    
    // initialize
    
    MEM_zero(complex_count);
    MEM_zero(complex_stats);

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      fprintf(stderr,
              "=========================================================\n");
    }
   
    // read in the complex track params
    
    complex_track_num = _tHandle.complex_track_nums[icomplex];
    if(RfReadComplexTrackParams(&_tHandle, complex_track_num, TRUE,
				"_performVerification")) {
      return -1;
    }
    
    ct_params = _tHandle.complex_params;
    
    // initialize flags and counters for track problems
    
    ct_params->n_top_missing = 0;
    ct_params->n_range_limited = 0;
    ct_params->start_missing = FALSE;
    ct_params->end_missing = FALSE;
    ct_params->volume_at_start_of_sampling = 0;
    ct_params->volume_at_end_of_sampling = 0;

    // set flags if track existed at start or end of sampling
    
    if (_tHandle.complex_params->start_time == _sHandle.header->start_time) {
      ct_params->start_missing = TRUE;
    }
    
    if (_tHandle.complex_params->end_time == _sHandle.header->end_time) {
      ct_params->end_missing = TRUE;
    }
    
    // allocate array for simple tracks
    
    n_simple_tracks = _tHandle.complex_params->n_simple_tracks;
    stracks = (vt_simple_track_t *) umalloc
      ((ui32) (n_simple_tracks * sizeof(vt_simple_track_t)));
    
    // read in simple tracks
    
    for (isimple = 0; isimple < n_simple_tracks; isimple++) {
      
      strack = stracks + isimple;
      
      simple_track_num =
	_tHandle.simples_per_complex[complex_track_num][isimple];
      
      // read in simple track params and prepare entries for reading
      
      if(RfRewindSimpleTrack(&_tHandle, simple_track_num,
			     "_performVerification")) {
	return -1;
      }
      
      // find last descendant, insert relevant values in the
      // simple params struct and store
      
      if (_findLastDescendant(0)) {
        cerr << "ERROR - _performVerification" << endl;
        cerr << "  _findLastDescendent failed" << endl;
        return -1;
      }
      
      // copy simple params
      
      strack->params = *_tHandle.simple_params;
      nstorms = _tHandle.simple_params->duration_in_scans;
      
      // allocate space for the entries
      
      strack->storms = (vt_storm_t *) umalloc(nstorms * sizeof(vt_storm_t));
      
      for (istorm = 0; istorm < nstorms; istorm++) {
	
	storm = strack->storms + istorm;
	
        // read in track entry, copy the structs to the local
	// array elements
	
	if (RfReadTrackEntry(&_tHandle, "_performVerification"))
	  return -1;
	
	if (RfReadStormScan(&_sHandle, _tHandle.entry->scan_num,
			    "_performVerification"))
	  return -1;
	
	if (RfReadStormProps(&_sHandle, _tHandle.entry->storm_num,
			     "_performVerification"))
	  return -1;
        
	memcpy (&storm->entry, _tHandle.entry,
		sizeof(track_file_entry_t));
	
	memcpy (&storm->gprops, (_sHandle.gprops + _tHandle.entry->storm_num),
		sizeof(storm_file_global_props_t));
	
	storm->runs = (storm_file_run_t *) umalloc
	  ((storm->gprops.n_runs * sizeof(storm_file_run_t)));
	
	memcpy ((void *) storm->runs,
		(void *) _sHandle.runs,
		(size_t) (storm->gprops.n_runs * sizeof(storm_file_run_t)));

        // allocate the lookup tables which relate the scan cartesian
	// grid to the verification grid

	storm->x_lookup = (long *) umalloc
	  (_sHandle.scan->grid.nx * sizeof(long));

	storm->y_lookup = (long *) umalloc
	  (_sHandle.scan->grid.ny * sizeof(long));

        // compute the lookup tables

	_computeLookup(&_sHandle.scan->grid,
		       storm->x_lookup, storm->y_lookup);
        
        // update flags for storm status
	
	if (storm->gprops.top_missing)
	  ct_params->n_top_missing++;
	
	if (storm->gprops.range_limited)
	  ct_params->n_range_limited++;

	if (storm->entry.time == _sHandle.header->start_time) {
	  ct_params->volume_at_start_of_sampling += storm->gprops.volume;
	}
	
	if (storm->entry.time == _sHandle.header->end_time) {
	  ct_params->volume_at_end_of_sampling += storm->gprops.volume;
	}
	
      } // istorm
      
    } // isimple
    
    // Set the first_scan_searched variable. If the
    // verify_before_forecast_time flag is set, the search begins at
    // the first scan in the storm file, because we need to consider
    // forecasts made before the track starts.
    // If the flag is not set, the search starts at the
    // start of the complex track
    
    if (_params.verify_before_forecast_time)
      first_scan_searched = 0;
    else
      first_scan_searched = _tHandle.complex_params->start_scan;

    // now loop through all of the scans in the complex track
    
    for (iscan = first_scan_searched;
	 iscan <= _tHandle.complex_params->end_scan; iscan++) {
      
      // initialize forecast and verification grids, etc
      
      memset (*_forecastGrid, 0, _nbytesGrid);
      memset (*_truthGrid, 0, _nbytesGrid);
      memset (&props_current, 0, sizeof(track_file_forecast_props_t));
      memset (&props_forecast, 0, sizeof(track_file_forecast_props_t));
      memset (&props_verify, 0, sizeof(track_file_forecast_props_t));
      nvalid = 0;
      nverify = 0;
      
      // compute times
      
      now = (double) _scanTimes[iscan].unix_time;
      forecast_time = now + (double) _params.forecast_lead_time;
      
      // Set the last_scan_searched variable. If the verify_after_track_dies
      // flag is set, the search ends at the last scan in the
      // storm file, because we need to consider scans even after the
      // track has terminated. If the flag is not set, the search ends
      // at the end of the complex track

      if (_params.verify_after_track_dies)
	last_scan_searched = n_scans - 1;
      else
	last_scan_searched = _tHandle.complex_params->end_scan;

      // find scan number which best corresponds to the forecast time
      
      min_time_diff = LARGE_DOUBLE;
      
      for (jscan = iscan;
	   jscan <= last_scan_searched; jscan++) {
	
	time_diff =
	  fabs((double) _scanTimes[jscan].unix_time - forecast_time);
	
	if (time_diff < min_time_diff) {
	  
	  forecast_scan = jscan;
	  min_time_diff = time_diff;
	  
	} // if (time_diff < min_time_diff)
	
      } // jscan
      
      closest_lead_time =
	(double) _scanTimes[forecast_scan].unix_time - now;
      
      // check the forecast lead time to determine whether there is
      // track data close to that time - if not, set the verify_valid
      // flag to FALSE
      
      if (fabs(closest_lead_time - _params.forecast_lead_time) <=
	  _params.search_time_margin) {
	
	forecast_lead_time = closest_lead_time;
	verify_valid = TRUE;
	
      } else {
	
	forecast_lead_time = _params.forecast_lead_time;
	verify_valid = FALSE;
	
      }
      
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	
	fprintf(stderr, "=========================\n"); 
	fprintf(stderr, "forecast_lead_time : %g\n", forecast_lead_time);
	fprintf(stderr, "forecast_scan : %ld\n", forecast_scan);
	fprintf(stderr, "verify_valid : %d\n", verify_valid);
	
      }

      if (verify_valid) {
	
	vector<vt_entry_index_t> valid;

        // search through all of the entries in the track
	
	for (isimple = 0; isimple < n_simple_tracks; isimple++) {
	  
	  strack = stracks + isimple;
	  
	  for (istorm = 0;
	       istorm < strack->params.duration_in_scans; istorm++) {
	    
	    storm = strack->storms + istorm;
	    
	    if (storm->entry.scan_num == iscan &&
		storm->entry.history_in_secs >=
		_params.forecast_min_history) {
	      
	      // this storm is at the current scan number, and its
	      // duration exceeds that for starting the forecast, so
	      // compute the forecast and update the forecast grid
	      
              if (_params.debug >= Params::DEBUG_VERBOSE) {
		
                fprintf(stderr, "Computing Forecast\n");
		fprintf(stderr,
                        "Simple num %ld, ientry %ld, scan %ld, time %s\n",
                        (long) strack->params.simple_track_num, istorm,
                        (long) storm->entry.scan_num,
                        utimstr(storm->entry.time));
		
		
	      } // if (_params.debug >= Params::DEBUG_VERBOSE)
	      
              // load up the forecast props

	      if (_loadProps(&storm->entry,
			     &storm->gprops,
			     forecast_lead_time,
			     &props_forecast)) {
		
                // load up the current props

		_loadProps(&storm->entry,
			   &storm->gprops,
			   0.0,
			   &props_current);
		
		// there is a valid forecast, so proceed.
		// The forecast is considered valid it the
		// forecast volume exceeds the volume
		// threshold
		
		load_forecast_grid(&storm->entry,
				   &storm->gprops,
				   forecast_lead_time,
				   _forecastGrid);
		
		vt_entry_index_t vt_entry_index;
		vt_entry_index.isimple = isimple;
		vt_entry_index.istorm = istorm;
		valid.push_back(vt_entry_index);
		nvalid++;
		
	      } // if (load_props (......
	      
	    } // if (storm->entry.scan_num ....
	    
            if ((storm->entry.scan_num == forecast_scan) &&
		storm->entry.history_in_secs >= verify_min_history) {
	      
	      // this storm is at the forecast time, so
	      // update the truth grids
	      
	      if (_params.debug >= Params::DEBUG_VERBOSE) {
		
		fprintf(stderr, "Computing Verification\n");
		fprintf(stderr,
			"Simple num %ld, ientry %ld, scan %ld, time %s\n",
			(long) strack->params.simple_track_num, istorm,
			(long) storm->entry.scan_num,
			utimstr(storm->entry.time));
		
		
	      } // if (_params.debug >= Params::DEBUG_VERBOSE)
	      
	      // load props for this scan - the lead time is set to zero
	      
	      _loadProps(&storm->entry,
			 &storm->gprops,
			 0.0,
			 &props_verify);
	      
	      nverify++;
	      
	      load_truth_grid(storm, _truthGrid);
	      
	    } // if (storm->entry.scan_num ....
	    
	  } // istorm
	  
	} // isimple
	
        if ((nvalid > 0) ||
	    (nverify > 0 && _params.verify_before_forecast_time)) {
	  
	  // compute the contingency data, summing counters for the
	  // complex track params
	  
	  compute_contingency_data(_forecastGrid,
				   _truthGrid,
				   _nbytesGrid,
				   &count);
	  
	  increment_count(&complex_count, &count);
	  
	  if (_params.debug >= Params::DEBUG_VERBOSE)
	    debug_print(stracks, nvalid, valid,
			_forecastGrid,
			_truthGrid,
			&count);
	  
	  // Compute the errors between the forecast and
	  // verification data
	  //
	  // The storm entries are updated to include the verification
	  // results in the track file - this is then rewritten.
	  
	  compute_errors(nverify,
			 &props_current,
			 &props_forecast,
			 &props_verify,
			 &complex_stats,
			 &file_stats,
			 &_totalStats);
	  
	} // if (nvalid > 0 ... )
	
      } // if (verify_valid)
      
    } // iscan

    // rewrite the track entries

    strack = stracks;

    for (isimple = 0; isimple < n_simple_tracks; isimple++) {
      
      storm = strack->storms;

      for (istorm = 0;
	   istorm < strack->params.duration_in_scans; istorm++) {
	
	memcpy (_tHandle.entry,	&storm->entry,
		sizeof(track_file_entry_t));
	    
	if (RfRewriteTrackEntry(&_tHandle,
				"_performVerification"))
	  return -1;
	
	storm++;

      } // istorm

      strack++;
	
    } // isimple
      
    // update counts for the file

    increment_count(&file_count, &complex_count);

    // set contingency data in complex params

    if (_params.forecast_shape == Params::FORECAST_ELLIPSE) {
      set_contingency(&complex_count,
                      &_tHandle.complex_params->ellipse_verify);
    } else {
      set_contingency(&complex_count,
                      &_tHandle.complex_params->polygon_verify);
    }
    
    // compute the root mean squared error for the forecast data

    compute_stats(&complex_stats);
    
    load_file_stats(tparams,
		    &complex_stats,
		    &_tHandle.complex_params->n_samples_for_forecast_stats,
		    &_tHandle.complex_params->forecast_bias,
		    &_tHandle.complex_params->forecast_rmse);
    
    // rewrite complex track params

    if(RfWriteComplexTrackParams(&_tHandle,
                                 _tHandle.complex_track_nums[icomplex],
				 "_performVerification")) {
      return -1;
    }

    // free up structs
    
    strack = stracks;

    for (isimple = 0; isimple < n_simple_tracks; isimple++) {
      
      storm = strack->storms;

      for (istorm = 0;
	   istorm < strack->params.duration_in_scans; istorm++) {
	ufree((char *) storm->runs);
	ufree((char *) storm->x_lookup);
	ufree((char *) storm->y_lookup);
	storm++;
      } // istorm
      
      ufree((char *) strack->storms);
      strack++;
      
    } // isimple
    
    ufree((char *) stracks);

  } // icomplex
  
  // update total counts

  increment_count(&_totalCount, &file_count);
  
  // set contingency data in file header

  if (_params.forecast_shape == Params::FORECAST_ELLIPSE) {
    set_contingency(&file_count,
                    &_tHandle.header->ellipse_verify);
  } else {
    set_contingency(&file_count,
                    &_tHandle.header->polygon_verify);
  }

  // compute the root mean squared error for the file forecast data,
  // store in header

  compute_stats(&file_stats);
  
  load_file_stats(tparams,
		  &file_stats,
		  &_tHandle.header->n_samples_for_forecast_stats,
		  &_tHandle.header->forecast_bias,
		  &_tHandle.header->forecast_rmse);
  
  // rewrite header

  if(RfWriteTrackHeader(&_tHandle, "_performVerification")) {
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////
// find last descendent of track

int DoVerify::_findLastDescendant(int level)

{

  long i;
  long child_track_num;
  simple_track_params_t params_copy;

  // make local copy of the simple params

  params_copy = *_tHandle.simple_params;

  // if level 0, copy simple params to _lastParams, which is static
  // and initialize visited array

  if (level == 0) {

    _lastParams = *_tHandle.simple_params;

    if (_visited == NULL) {
      _visited = (ui08 *) umalloc(_tHandle.header->n_simple_tracks);
    } else {
      _visited = (ui08 *) urealloc(_visited, _tHandle.header->n_simple_tracks);
    }

    memset(_visited, 0, _tHandle.header->n_simple_tracks);

  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE)
    fprintf(stderr, "findLastDescendant, level %d, "
	    "simple_track_num %ld, end_scan %ld\n",
	    level, (long) params_copy.simple_track_num,
	    (long) params_copy.end_scan);
  
  if (params_copy.nchildren > 0) {
    
    // for each child, read in the simple params, and find the
    // last descendant for that child

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      fprintf(stderr, "===> simple_track %d has children: ",
	      _tHandle.simple_params->simple_track_num);
      for (i = 0; i < params_copy.nchildren; i++) {
	fprintf(stderr, "%d ", params_copy.child[i]);
      }
      fprintf(stderr, " <===\n");
    }

    for (i = 0; i < params_copy.nchildren; i++) {
      
      child_track_num = params_copy.child[i];

      if (_visited[child_track_num] == FALSE) {
        
	if (_params.debug >= Params::DEBUG_VERBOSE)
	  fprintf(stderr, "---> simple_track %d, recursing to child %ld\n",
		  _tHandle.simple_params->simple_track_num,
		  child_track_num);
	
	if(RfReadSimpleTrackParams(&_tHandle, child_track_num,
				   "find_last_descendant")) {
	  fprintf(stderr, "Error on recursive level %d\n", level);
	  return -1;
	}
	
	_findLastDescendant(level + 1);

	_visited[child_track_num] = TRUE;

      }

    } // i

  } else {

    // no children, so check to see if the end scan of this simple
    // track is later than the static _lastParams end scan. If so
    // copy to _lastParams

    if (_tHandle.simple_params->end_scan > _lastParams.end_scan) {
      _lastParams = *_tHandle.simple_params;
      if (_params.debug >= Params::DEBUG_VERBOSE)
	fprintf(stderr, "Updating last params, last_scan = %ld\n",
		(long) _lastParams.end_scan);
    }
    
  } // if (params_copy.nchildren > 0)

  // if level 0, set the last_descendant params, copy the params back

  if (level == 0) {
    
    params_copy.last_descendant_simple_track_num =
      _lastParams.simple_track_num;
    params_copy.last_descendant_end_scan = _lastParams.end_scan;
    params_copy.last_descendant_end_time = _lastParams.end_time;

    *_tHandle.simple_params = params_copy;
    
    if(RfWriteSimpleTrackParams(&_tHandle,
				_tHandle.simple_params->simple_track_num,
				"find_last_descendant")) {
      fprintf(stderr, "Error on recursive level %d\n", level);
      return -1;
    }
    
    if (_params.debug >= Params::DEBUG_VERBOSE)
      fprintf(stderr, "EXITING, last_scan = %ld\n",
	      (long) _tHandle.simple_params->last_descendant_end_scan);
    
  }

  return 0;

}

////////////////////////
// compute lookup tables

void DoVerify::_computeLookup(titan_grid_t *grid,
                              long *x_lookup,
                              long *y_lookup)

{

  long ix, iy;
  long jx, jy;

  double xx, yy;

  xx = grid->minx;

  for (ix = 0; ix < grid->nx; ix++) {

    jx = (long) ((xx - _minx) / _dx + 0.5);	
    
    if (jx >= 0 && jx < _nx)
      *x_lookup = jx;
    else
      *x_lookup = -1;

    xx += grid->dx;
    x_lookup++;

  } // ix

  yy = grid->miny;

  for (iy = 0; iy < grid->ny; iy++) {

    jy = (long) ((yy - _miny) / _dy + 0.5);	
    
    if (jy >= 0 && jy < _ny)
      *y_lookup = jy;
    else
      *y_lookup = -1;

    yy += grid->dy;
    y_lookup++;

  } // iy

}

////////////////////////
// load up props

bool DoVerify::_loadProps(const track_file_entry_t *entry,
                          const storm_file_global_props_t *gprops,
                          double lead_time,
                          track_file_forecast_props_t *f_props)

{

  long i;

  double dval_dt, delta_val;
  double f_volume;
  double lead_time_hr;
  double growth_period_hr;
  double forecast_area, current_area;
  double darea_dt;
  double f_props_val;

  fl32 *current_ptr;
  fl32 *f_props_ptr;
  fl32 *dval_dt_ptr;

  // const storm_file_params_t *sparams;
  track_file_forecast_props_t current;
  const track_file_forecast_props_t *fprops;
  // const track_file_params_t *tparams;
  
  // sparams = &_sHandle.header->params;
  // tparams = &_tHandle.header->params;
  fprops = &entry->dval_dt;
  lead_time_hr = lead_time / 3600.0;
  growth_period_hr = _params.time_to_zero_growth / 3600.0;

  // compute the forecast area

  current_area = gprops->proj_area;
  darea_dt = fprops->proj_area;
  forecast_area = current_area + lead_time_hr * darea_dt;

  // return now if the forecast area is less than 1

  if (forecast_area < 1.0)
    return (FALSE);
  
  // set current vals in struct

  current.dbz_max = gprops->dbz_max;
  current.volume = gprops->volume;
  current.precip_flux = gprops->precip_flux;
  current.mass = gprops->mass;
  current.proj_area = gprops->proj_area;

  // weight the posiitional parameters by volume

  current.proj_area_centroid_x = gprops->proj_area_centroid_x;
  current.proj_area_centroid_y = gprops->proj_area_centroid_y;
  current.vol_centroid_z = gprops->vol_centroid_z;
  current.refl_centroid_z = gprops->refl_centroid_z;
  current.top = gprops->top;

  // compute forecast volume

  f_volume = (current.volume + fprops->volume * lead_time_hr);

  // return FALSE if volume less than 1 
  // *rjp 10 Apr 2002
  // Note This needs to be checked further. 
  // fprops->volume should be not be negative at this point.

  if (f_volume < 1.0)
    return(FALSE) ;    

  // move through the structs, using pointers to the
  // data elements
  
  current_ptr = (fl32 *) &current;
  f_props_ptr = (fl32 *) f_props;
  dval_dt_ptr = (fl32 *) fprops;

  for (i = 0; i < N_MOVEMENT_PROPS_FORECAST; i++) {
    
    // compute the forecast movement props based on
    // current vals and forecast
    // rates of change - these are weighted by volume
    
    delta_val = *dval_dt_ptr * lead_time_hr;
    f_props_val = (*current_ptr + delta_val);
    
    *f_props_ptr += f_props_val * f_volume;
    
    current_ptr++;
    f_props_ptr++;
    dval_dt_ptr++;
    
  } // i 
  
  for (i = 0; i < N_GROWTH_PROPS_FORECAST; i++) {
    
    // compute the forecast growth props based on current vals and forecast
    // rate of change. If forecast value goes negative, set to zero.
    
    dval_dt = *dval_dt_ptr;

    if (_params.growth_function == Params::GROWTH_PARABOLIC) {

      // parabolic growth, linear decay

      if (dval_dt < 0.0) {
	
        // linear decay - negative growth rate
	
	delta_val = dval_dt * lead_time_hr;
	
      } else {
	
        // parabolic growth curve - positive growth rate
	
	delta_val = (dval_dt * lead_time_hr -
		     (dval_dt * lead_time_hr * lead_time_hr) /
		     (growth_period_hr * 2.0));

      }
	
    } else {
      
      // linear growth / decay
      
      delta_val = dval_dt * lead_time_hr;
      
    } // if (_params.growth_function == Params::GROWTH_PARABOLIC)
      
    f_props_val = (*current_ptr + delta_val);
    
    if (f_props_val < 0.0)
      f_props_val = 0.0;
    
    *f_props_ptr += f_props_val;
    
    current_ptr++;
    f_props_ptr++;
    dval_dt_ptr++;
    
  } // i
  
  return (TRUE);

}

/////////////////////////////////////////////////////////////////////
// loads the forecast grid based on the forecast for the current storm

void DoVerify::load_forecast_grid(track_file_entry_t *entry,
                                  storm_file_global_props_t *gprops,
                                  double lead_time,
                                  ui08 **grid)

{

  storm_file_params_t *sparams;
  track_file_params_t *tparams;

  sparams = &_sHandle.header->params;
  tparams = &_tHandle.header->params;
			    
  if (_params.forecast_shape == Params::FORECAST_ELLIPSE) {

    load_ellipse_forecast_grid(sparams,
			       tparams,
			       _sHandle.scan,
			       gprops,
			       entry,
			       lead_time,
			       grid);

  } else if (_params.forecast_shape == Params::FORECAST_POLYGON) {

    load_polygon_forecast_grid(sparams,
			       tparams,
			       _sHandle.scan, gprops,
			       entry,
			       lead_time, grid);
    
  }

}

/**************************************************************************
 * load_truth_grid()
 *
 **************************************************************************/

void DoVerify::load_truth_grid(vt_storm_t *storm,
                               ui08 **grid)

{

  storm_file_params_t *sparams;
  sparams = &_sHandle.header->params;
  
  if (_params.verify_method == Params::VERIFY_ELLIPSE) {
    
    load_ellipse_truth_grid(sparams, _sHandle.scan,
			    &storm->gprops, grid);
    
  } else if (_params.verify_method == Params::VERIFY_POLYGON) {
    
    load_polygon_truth_grid(sparams, _sHandle.scan,
			    &storm->gprops, grid);
    
  } else if (_params.verify_method == Params::VERIFY_RUNS) {
    
    load_runs_truth_grid(storm, grid);
    
  }

}

/****************************************************************************
 * load_ellipse_forecast_grid.c
 *
 * loads the forecast grid based on the ellipse forecast for the current storm
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * January 1992
 *
 ****************************************************************************/

void DoVerify::load_ellipse_forecast_grid(storm_file_params_t *sparams,
                                          track_file_params_t *tparams,
                                          storm_file_scan_header_t *scan,
                                          storm_file_global_props_t *gprops,
                                          track_file_entry_t *entry,
                                          double lead_time,
                                          ui08 **grid)
  
{

  double f_area;
  double f_proj_area;
  double f_centroid_x;
  double f_centroid_y;
  double f_ellipse_scale;
  double f_major_radius;
  double f_minor_radius;
  double f_orientation ;
  double lead_time_hr, growth_period_hr;

  track_file_forecast_props_t *fprops = &entry->dval_dt;

  lead_time_hr = lead_time / 3600.0;
  growth_period_hr = _params.time_to_zero_growth / 3600.0;

  /*
   * compute the area forecast - only update the forecast
   * grid if this is greater than 1.0
   */

  f_area = gprops->proj_area + fprops->proj_area * lead_time_hr;
  
  if (f_area >= 1.0) {
    
    if (_params.growth_function == Params::GROWTH_ZERO) {
      
      f_proj_area = gprops->proj_area;

    } else if (_params.growth_function == Params::GROWTH_PARABOLIC) {

      if (fprops->proj_area < 0.0) {

	/*
	 * linear on decay
	 */

	f_proj_area = gprops->proj_area +
	  fprops->proj_area * lead_time_hr;

      } else {

	/*
	 * parabloic on growth
	 */
	
	f_proj_area = gprops->proj_area +
	  fprops->proj_area * lead_time_hr -
	  ((fprops->proj_area * lead_time_hr * lead_time_hr) /
	   (growth_period_hr * 2.0));
	
      }
	
    } else {
      
      f_proj_area = gprops->proj_area +
	fprops->proj_area * lead_time_hr;

    }
    
    if (f_proj_area < 1.0) {
      f_proj_area = 1.0;
    }
    
    f_centroid_x =
      (gprops->proj_area_centroid_x +
       fprops->proj_area_centroid_x * lead_time_hr);
    
    f_centroid_y =
      (gprops->proj_area_centroid_y +
       fprops->proj_area_centroid_y * lead_time_hr);
    
    f_ellipse_scale =
      (sqrt(f_proj_area / gprops->proj_area));
    
    f_major_radius =
      gprops->proj_area_major_radius * f_ellipse_scale;
    
    f_minor_radius =
      gprops->proj_area_minor_radius * f_ellipse_scale;
    
    f_orientation =  gprops->proj_area_orientation;
    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      
      fprintf(stderr, "forecast ellipse\n");
      fprintf(stderr,
	      "x, y, maj_r, min_r, theta : %g, %g, %g, %g, %g\n",
	      f_centroid_x, f_centroid_y,
	      f_major_radius, f_minor_radius, f_orientation);
      
    }

    /*
     * update the grid
     */
    
    load_ellipse_grid(f_centroid_x, f_centroid_y,
		      f_major_radius, f_minor_radius,
		      f_orientation,
		      grid);
    
  } /* if (f_area >= 1.0) */
  
}

/***************************************************************************
 * load_ellipse_truth_grid()
 *
 * loads the ellipse truth grid based on the ellipse props
 * for the actual storm at the forecast time
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * January 1992
 *
 ***************************************************************************/

void
  DoVerify::load_ellipse_truth_grid(storm_file_params_t *sparams,
                                    storm_file_scan_header_t *scan,
                                    storm_file_global_props_t *gprops,
                                    ui08 **grid)

{

  double proj_area;
  double centroid_x;
  double centroid_y;
  double major_radius;
  double minor_radius;
  double orientation ;

  proj_area = gprops->proj_area;
    
  if (proj_area < 1.0) {
    proj_area = 1.0;
    
  }

  centroid_x = gprops->proj_area_centroid_x;
  centroid_y = gprops->proj_area_centroid_y;
  major_radius = gprops->proj_area_major_radius;
  minor_radius = gprops->proj_area_minor_radius;
  orientation = gprops->proj_area_orientation;

  if (_params.debug >= Params::DEBUG_VERBOSE) {

    fprintf(stderr, "verification ellipse\n");
    fprintf(stderr,
	    "x, y, maj_r, min_r, theta : %g, %g, %g, %g, %g\n",
	    centroid_x, centroid_y,
	    major_radius, minor_radius, orientation);

  }

  /*
   * update the grid
   */
    
  load_ellipse_grid(centroid_x, centroid_y,
		    major_radius, minor_radius,
		    orientation,
		    grid);
  
}

/*********************************************************************
 * load_ellipse_grid()
 *
 * Flags regions in the grid with 1's if the given
 * projected area ellipse crosses into or contains the region.
 *
 * The method uses 3 steps.
 *
 * 1) Flag the region containing the ellipse centroid.
 *
 * 2) Consider all vertical grid lines which intersect the ellipse.
 *    Flag all regions on either side of such a line for that
 *    line segment which crosses the ellipse.
 *
 * 3) Consider all horizontal grid lines which intersect the ellipse.
 *    Flag all regions on either side of such a line for that
 *    line segment which crosses the ellipse.
 *
 * RAP, NCAR, Boulder CO
 *
 * November 1991
 *
 * Mike Dixon
 *
 *********************************************************************/

void DoVerify::load_ellipse_grid(double ellipse_x,
                                 double ellipse_y,
                                 double major_radius,
                                 double minor_radius,
                                 double axis_rotation,
                                 ui08 **grid)

{

  long ix, iy, ix1, iy1, ix2, iy2;
  long start_ix, start_iy;
  long end_ix, end_iy;

  double grid_rotation, theta;
  double slope_prime, intercept_prime;
  double sin_rotation, cos_rotation, tan_rotation;

  double xprime1, yprime1, xprime2, yprime2;
  double x_1, y_1, x_2, y_2;
  double start_x, start_y;
  double end_x, end_y;
  double line_x, line_y;

  /*
   * compute the grid_rotation, taking care to avoid 0, pi/2 and
   * pi, so that the trig functions will not fail. Remember that
   * the axis_rotation is relative to True North, and we need to
   * compute the grid rotation relative to the mathmatically
   * conventional axes
   */

  theta = 90.0 - axis_rotation;

  if (theta == 0.0)
    grid_rotation = SMALL_ANGLE;
  else if (theta == 90.0)
    grid_rotation = ALMOST_PI_BY_TWO;
  else if (theta == -90.0)
    grid_rotation = - ALMOST_PI_BY_TWO;
  else if (theta == 180.0 || theta == -180.0)
    grid_rotation = ALMOST_PI;
  else
    grid_rotation = theta * DEG_TO_RAD;

  sin_rotation = sin(grid_rotation);
  cos_rotation = cos(grid_rotation);
  tan_rotation = tan(grid_rotation);
  
  /*
   * compute the start and end x and y - these values are
   * chosen for a circle of radius major_radius, which will
   * enclose the ellipse
   */

  start_x = ellipse_x - major_radius;
  start_y = ellipse_y - major_radius;

  end_x = ellipse_x + major_radius;
  end_y = ellipse_y + major_radius;

  /*
   * set the end and start grid indices
   */

  start_ix = (long) ((start_x - _minx) / _dx + 0.49);
  start_ix = MAX(start_ix, 0);

  start_iy = (long) ((start_y - _miny) / _dy + 0.49);
  start_iy = MAX(start_iy, 0);

  end_ix = (long) ((end_x - _minx) / _dx + 0.51);
  end_ix = MIN(end_ix, _nx - 1);

  end_iy = (long) ((end_y - _miny) / _dy + 0.51);
  end_iy = MIN(end_iy, _ny - 1);

  /*
   * flag the grid region which contains the ellipse centroid
   */

  ix = (long) ((ellipse_x - _minx) / _dx + 0.5);
  iy = (long) ((ellipse_y - _miny) / _dy + 0.5);

  if (ix >= start_ix && ix <= end_ix &&
      iy >= start_iy && iy <= end_iy)
    grid[iy][ix] = 1;

  /*
   * loop through the vertical lines which intersect the ellipse
   */

  for (ix = start_ix; ix < end_ix; ix++) {

    /*
     * compute the slope and intercept of this line in the
     * transformed coordinate system with ths origin at the
     * center of the ellipse and the x-axis along the major
     * axis. The prime values refer to the transformed
     * coord system.
     */

    
    line_x = _minx + ((double) ix + 0.5) * _dx;

    slope_prime = 1.0 / tan_rotation;

    intercept_prime  = - (line_x - ellipse_x) / sin_rotation;

    if (uline_through_ellipse(major_radius, minor_radius,
			      slope_prime, intercept_prime,
			      &xprime1, &yprime1,
			      &xprime2, &yprime2) == TRUE) {

      /*
       * transform the points back into grid coords
       */

      y_1 = ellipse_y + xprime1 * sin_rotation + yprime1 * cos_rotation;
      y_2 = ellipse_y + xprime2 * sin_rotation + yprime2 * cos_rotation;

      if  (y_1 <= y_2) {

	iy1 = (long) ((y_1 - _miny) / _dy + 0.5);
	iy2 = (long) ((y_2 - _miny) / _dy + 0.5);

      } else {

	iy1 = (long) ((y_2 - _miny) / _dy + 0.5);
	iy2 = (long) ((y_1 - _miny) / _dy + 0.5);

      }

      iy1 = MAX(iy1, 0);
      iy2 = MIN(iy2, _ny - 1);

      for (iy = iy1; iy <= iy2; iy++) {

	grid[iy][ix] = 1;
	grid[iy][ix + 1] = 1;

      } /* iy */

    } /* if (uline_through_ellipse(major_radius ... */

  } /* ix */

  /*
   * loop through the horizontal lines which intersect the ellipse
   */

  for (iy = start_iy; iy < end_iy; iy++) {

    /*
     * compute the slope and intercept of this line in the
     * transformed coordinate system with ths origin at the
     * center of the ellipse and the x-axis along the major
     * axis. The prime values refer to the transformed
     * coord system.
     */

    
    line_y = _miny + ((double) iy + 0.5) * _dy;

    slope_prime = - tan_rotation;

    intercept_prime  = (line_y - ellipse_y) / cos_rotation;

    if (uline_through_ellipse(major_radius, minor_radius,
			      slope_prime, intercept_prime,
			      &xprime1, &yprime1,
			      &xprime2, &yprime2) == TRUE) {

      /*
       * transform the points back into grid coords
       */

      x_1 = ellipse_x + xprime1 * cos_rotation - yprime1 * sin_rotation;
      x_2 = ellipse_x + xprime2 * cos_rotation - yprime2 * sin_rotation;

      if  (x_1 <= x_2) {

	ix1 = (long) ((x_1 - _minx) / _dx + 0.5);
	ix2 = (long) ((x_2 - _minx) / _dx + 0.5);

      } else {

	ix1 = (long) ((x_2 - _minx) / _dx + 0.5);
	ix2 = (long) ((x_1 - _minx) / _dx + 0.5);

      }

      ix1 = MAX(ix1, 0);
      ix2 = MIN(ix2, _nx - 1);

      for (ix = ix1; ix <= ix2; ix++) {

	grid[iy][ix] = 1;
	grid[iy + 1][ix] = 1;

      } /* ix */

    } /* if (uline_through_ellipse(major_radius ... */

  } /* iy */

}

/***********************************************************************
 * load_polygon_forecast_grid()
 *
 * loads the forecast grid based on the polygon forecast
 * for the current storm
 *
 ***********************************************************************/

void
  DoVerify::load_polygon_forecast_grid(storm_file_params_t *sparams,
                                       track_file_params_t *tparams,
                                       storm_file_scan_header_t *scan,
                                       storm_file_global_props_t *gprops,
                                       track_file_entry_t *entry,
                                       double lead_time,
                                       ui08 **grid)

{
  
  long ix, iy, iray;
  long n_sides;
  long min_ix, min_iy, max_ix, max_iy;

  double f_area;
  double f_proj_area;
  double f_polygon_scale;
  double cart_dx, cart_dy;
  double start_az, delta_az;
  double start_az_rad, delta_az_rad;
  double theta, range[N_POLY_SIDES];
  double x, y;
  double search_x, search_y;
  double min_x, min_y, max_x, max_y;
  double lead_time_hr, growth_period_hr;
  
  vt_point_t f_centroid;
  
  track_file_forecast_props_t *fprops = &entry->dval_dt;
  
  lead_time_hr = lead_time / 3600.0;
  growth_period_hr = _params.time_to_zero_growth / 3600.0;

  /*
   * compute the area forecast - only update the forecast
   * grid if this is greater than 1.0
   */
  
  f_area =
    (gprops->proj_area + fprops->proj_area * lead_time_hr);
  
  if (f_area >= 1.0) {
    
    if (_params.growth_function == Params::GROWTH_ZERO) {

      f_proj_area = gprops->proj_area;

    } else if (_params.growth_function == Params::GROWTH_PARABOLIC) {

      if (fprops->proj_area < 0.0) {

	/*
	 * linear on decay
	 */

	f_proj_area = gprops->proj_area +
	  fprops->proj_area * lead_time_hr;

      } else {

	/*
	 * parabloic on growth
	 */
	
	f_proj_area = gprops->proj_area +
	  fprops->proj_area * lead_time_hr -
	  ((fprops->proj_area * lead_time_hr * lead_time_hr) /
	   (growth_period_hr * 2.0));
	
      }
	
    } else {
      
      f_proj_area = gprops->proj_area +
	fprops->proj_area * lead_time_hr;

    }
    
    if (f_proj_area < 1.0)
      f_proj_area = 1.0;
    
    f_centroid.x =
      (gprops->proj_area_centroid_x +
       fprops->proj_area_centroid_x * lead_time_hr);
    
    f_centroid.y =
      (gprops->proj_area_centroid_y +
       fprops->proj_area_centroid_y * lead_time_hr);
    
    f_polygon_scale =
      (sqrt(f_proj_area / gprops->proj_area));
    
    /*
     * compute the polygon points
     */
    
    cart_dx = scan->grid.dx;
    cart_dy = scan->grid.dy;
    
    start_az = sparams->poly_start_az;
    delta_az = sparams->poly_delta_az;
    start_az_rad = start_az * DEG_TO_RAD;
    delta_az_rad = delta_az * DEG_TO_RAD;
    n_sides = sparams->n_poly_sides;
    
    theta = start_az_rad;
    
    min_x = LARGE_DOUBLE;
    max_x = -LARGE_DOUBLE;
    min_y = LARGE_DOUBLE;
    max_y = -LARGE_DOUBLE;
    
    for (iray = 0; iray < n_sides; iray++) {
      
      range[iray] =
	gprops->proj_area_polygon[iray] * f_polygon_scale;
    
      x = f_centroid.x + range[iray] * sin(theta) * cart_dx;
      y = f_centroid.y + range[iray] * cos(theta) * cart_dy;
      
      min_x = MIN(x, min_x);
      max_x = MAX(x, max_x);
      min_y = MIN(y, min_y);
      max_y = MAX(y, max_y);
      
      theta += delta_az_rad;
      
    } /* iray */
    
    min_ix = (long) ((min_x - _minx) / _dx + 0.5) - 1; 
    max_ix = (long) ((max_x - _minx) / _dx + 0.5) + 1;
    min_iy = (long) ((min_y - _miny) / _dy + 0.5) - 1;
    max_iy = (long) ((max_y - _miny) / _dy + 0.5) + 1;
    
    CONSTRAIN_VAL(min_ix, 1, _nx - 1);
    CONSTRAIN_VAL(max_ix, 1, _nx - 1);
    CONSTRAIN_VAL(min_iy, 1, _ny - 1);
    CONSTRAIN_VAL(max_iy, 1, _ny - 1);
    
    search_y = _miny + ((double) min_iy - 0.5) * _dy;
    
    for (iy = min_iy; iy <= max_iy; iy++) {
      
      search_x = _minx + ((double) min_ix - 0.5) * _dx;
      
      for (ix = min_ix; ix <= max_ix; ix++) {
	
	if (point_in_polygon(f_centroid.x, f_centroid.y, range,
			     start_az, delta_az, n_sides,
			     cart_dx, cart_dy,
			     search_x, search_y)) {
	  
	  grid[iy-1][ix-1] = TRUE;
	  grid[iy-1][ix] = TRUE;
	  grid[iy][ix-1] = TRUE;
	  grid[iy][ix] = TRUE;
	  
	}
	
	search_x += _dx;
	
      } /* ix */
      
      search_y += _dy;
      
    } /* iy */
    
  } /* if (f_area >= 1.0) */
  
}

/*************************************************************************
 * load_polygon_truth_grid()
 *
 * loads the truth grid based on the polygon for the current storm
 *
 *************************************************************************/

void DoVerify::load_polygon_truth_grid(storm_file_params_t *sparams,
                                       storm_file_scan_header_t *scan,
                                       storm_file_global_props_t *gprops,
                                       ui08 **grid)

{

  long ix, iy, iray;
  long n_sides;
  long min_ix, min_iy, max_ix, max_iy;

  double cart_dx, cart_dy;
  double start_az, delta_az;
  double start_az_rad, delta_az_rad;
  double theta, range[N_POLY_SIDES];
  double x, y;
  double search_x, search_y;
  double min_x, min_y, max_x, max_y;

  vt_point_t centroid;

  centroid.x = gprops->proj_area_centroid_x;
  centroid.y = gprops->proj_area_centroid_y;
    
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    fprintf(stderr, "\ntruth polygon\n");
    fprintf(stderr,
	    "x, y : %g, %g\n", centroid.x, centroid.y);
  }

  /*
   * compute the polygon points
   */
  
  cart_dx = scan->grid.dx;
  cart_dy = scan->grid.dy;
  
  start_az = (sparams->poly_start_az);
  delta_az = (sparams->poly_delta_az);
  start_az_rad = start_az * DEG_TO_RAD;
  delta_az_rad = delta_az * DEG_TO_RAD;
  n_sides = sparams->n_poly_sides;
  
  theta = start_az_rad;

  min_x = LARGE_DOUBLE;
  max_x = -LARGE_DOUBLE;
  min_y = LARGE_DOUBLE;
  max_y = -LARGE_DOUBLE;

  for (iray = 0; iray < n_sides; iray++) {
    
    range[iray] = gprops->proj_area_polygon[iray];
    x = centroid.x + range[iray] * sin(theta) * cart_dx;
    y = centroid.y + range[iray] * cos(theta) * cart_dy;
    
    min_x = MIN(x, min_x);
    max_x = MAX(x, max_x);
    min_y = MIN(y, min_y);
    max_y = MAX(y, max_y);

    theta += delta_az_rad;
    
  } /* iray */
  
  min_ix = (long) ((min_x - _minx) / _dx + 0.5) - 1; 
  max_ix = (long) ((max_x - _minx) / _dx + 0.5) + 1;
  min_iy = (long) ((min_y - _miny) / _dy + 0.5) - 1;
  max_iy = (long) ((max_y - _miny) / _dy + 0.5) + 1;

  CONSTRAIN_VAL(min_ix, 1, _nx - 1);
  CONSTRAIN_VAL(max_ix, 1, _nx - 1);
  CONSTRAIN_VAL(min_iy, 1, _ny - 1);
  CONSTRAIN_VAL(max_iy, 1, _ny - 1);
  
  search_y = _miny + ((double) min_iy - 0.5) * _dy;
  
  for (iy = min_iy; iy <= max_iy; iy++) {
    
    search_x = _minx + ((double) min_ix - 0.5) * _dx;
    
    for (ix = min_ix; ix <= max_ix; ix++) {
      
      if (point_in_polygon(centroid.x, centroid.y, range,
			   start_az, delta_az, n_sides,
			   cart_dx, cart_dy,
			   search_x, search_y)) {
	  
	grid[iy-1][ix-1] = TRUE;
	grid[iy-1][ix] = TRUE;
	grid[iy][ix-1] = TRUE;
	grid[iy][ix] = TRUE;
	
      }
      
      search_x += _dx;
      
    } /* ix */
    
    search_y += _dy;
    
  } /* iy */
  
}

/**************************************************************************
 * load_runs_truth_grid()
 *
 **************************************************************************/

void DoVerify::load_runs_truth_grid(vt_storm_t *storm,
                                    ui08 **grid)
  
{

  long i, ix, irun;
  long lx, ly;
  long *x_lookup, *y_lookup;

  storm_file_run_t *run;

  x_lookup = storm->x_lookup;
  y_lookup = storm->y_lookup;

  /*
   * loop through runs
   */

  run = storm->runs;
  
  for (irun = 0; irun < storm->gprops.n_runs; irun++) {

    ly = y_lookup[run->iy];

    if (ly >= 0) {

      ix = run->ix;

      for (i = 0; i < run->n; i++) {

	lx = x_lookup[ix];
      
	if (lx >= 0)
	  grid[ly][lx] = TRUE;

	ix++;

      } /* i */

    } /* if (ly >= 0) */
    
    run++;

  } /* irun */

}

/*************************************************************************
 *
 * point_in_polygon()
 *
 * Tests if a point is within the storm polygon
 *
 * The polygon is specified as a series of radial distances from the
 * centroid. The units are in terms of the grid units in which the
 * polygon was originally derived.
 *
 ****************************************************************************/

int DoVerify::point_in_polygon(double centroid_x,
                               double centroid_y,
                               double *radials,
                               double start_az,
                               double delta_az,
                               long n_sides,
                               double grid_dx,
                               double grid_dy,
                               double search_x,
                               double search_y)
  
{

  long az_num, next_az_num;

  double delta_x;
  double delta_y;
  double dist, bdist;
  double az_pt, az1;
  double l1, l2, dl;

  /*
   * get the search point relative to the centroid in
   * grid coords
   */

  delta_x = (search_x - centroid_x) / grid_dx;
  delta_y = (search_y - centroid_y) / grid_dy;
  
  dist = sqrt (delta_x * delta_x + delta_y * delta_y);
  
  if (delta_x == 0.0 && delta_y == 0.0) {
    az_pt = 0.0;
  } else {
    az_pt = atan2(delta_x, delta_y) * RAD_TO_DEG;
    if (az_pt < 0)
      az_pt += 360.0;
  }

  /*
   * calculate the index of the ray just below the point in the polygon
   */
  
  az_num = (long) ((az_pt - start_az) / delta_az);
  
  if (az_num < 0)
    az_num += n_sides;

  next_az_num = (az_num + 1) % n_sides;

  /*
   * get the lengths of the line polygon rays on either side of the
   * point - these are in grid coords
   */

  l1 = radials[az_num];
  l2 = radials[next_az_num];
  dl = l2 - l1;

  /*
   * interpolate the ray lengths to determine the boundary distance
   * from the centroid at the azimuth of the point
   */

  az1 = start_az + delta_az * (double) az_num;

  bdist = l1 + ((az_pt - az1) / delta_az) * dl;

  /*
   * if the boundary distance is greater than the distance of the
   * point from the centroid, the point is  within the polygon
   */

  if (dist <= bdist)
    return (TRUE);
  else
    return (FALSE);

}

/****************************************************************
 * compute contingency values
 */

void DoVerify::compute_contingency_data(ui08 **forecast_grid,
                                        ui08 **truth_grid,
                                        long nbytes_grid,
                                        vt_count_t *count)

{

  if (_params.force_xy_fit) {
    
    /*
     * best fit computations - grid data is moved so create best fit
     * of one set over the other
     */
    
    compute_best_fit(forecast_grid, truth_grid, count);
    
  } else {
    
    compute_as_is(forecast_grid, truth_grid, nbytes_grid, count);
    
  }
  
}

/**********************************************************************
 * compute_best_fit()
 *
 * best fit computations - grid data is moved so create best xy fit
 * of one set relative to the other
 */

void DoVerify::compute_best_fit(ui08 **forecast_grid,
                                ui08 **truth_grid,
                                vt_count_t *count)
  
{
  
  ui08 *truth, *forecast;
  
  long ix, iy;
  long n_succ = 0, n_fail = 0, n_false = 0, n_non = 0;
  long nx_compare, ny_compare;
  long startx_forecast, starty_forecast;
  long startx_truth, starty_truth;
  long xoffset, yoffset;

  double x, y;
  double forecast_meanx, forecast_meany;
  double truth_meanx, truth_meany;
  double forecast_sumx = 0.0, forecast_sumy = 0.0, forecast_n = 0.0;
  double truth_sumx = 0.0, truth_sumy = 0.0, truth_n = 0.0;
  double xdiff, ydiff;

  /*
   * compute the mean (x, y) for the data in each grid
   */

  y = _miny;
  
  for (iy = 0; iy < _ny; iy++) {

    x = _minx;
    truth = truth_grid[iy];
    forecast = forecast_grid[iy];
    
    for (ix = 0; ix < _nx; ix++) {
      
      if (*forecast) {
	forecast_sumx += x;
	forecast_sumy += y;
	forecast_n++;
      }

      if (*truth) {
	truth_sumx += x;
	truth_sumy += y;
	truth_n++;
      }

      x += _dx;
      truth++;
      forecast++;

    } /* ix */

    y += _dy;

  } /* iy */
  
  forecast_meanx = forecast_sumx / forecast_n;
  forecast_meany = forecast_sumy / forecast_n;

  truth_meanx = truth_sumx / truth_n;
  truth_meany = truth_sumy / truth_n;

  /*
   * compute the offsets required to match the centroids of the data
   * when performing the grid comparison
   */

  xdiff = truth_meanx - forecast_meanx;
  ydiff = truth_meany - forecast_meany;

  xoffset = (long) floor (xdiff / _dx + 0.5);
  yoffset = (long) floor (ydiff / _dy + 0.5);

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    fprintf(stderr, "compute_contingency_data\n");
    fprintf(stderr, "xoffset, yoffset = %ld, %ld\n", xoffset, yoffset);
  }

  nx_compare = _nx - abs((int) xoffset);
  ny_compare = _ny - abs((int) yoffset);

  if (xoffset >= 0) {
    startx_forecast = 0;
    startx_truth = xoffset;
  } else {
    startx_forecast = -xoffset;
    startx_truth = 0;
  }
    
  if (yoffset >= 0) {
    starty_forecast = 0;
    starty_truth = yoffset;
  } else {
    starty_forecast = -yoffset;
    starty_truth = 0;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {

    fprintf(stderr, "forecast startx, starty ' %ld, %ld\n",
	    startx_forecast, starty_forecast);

    fprintf(stderr, "truth startx, starty ' %ld, %ld\n",
	    startx_truth, starty_truth);

    fprintf(stderr, "compare nx, ny = %ld, %ld\n",
	    nx_compare, ny_compare);

  }

  for (iy = 0; iy < ny_compare; iy++) {
    
    truth = truth_grid[starty_truth + iy] + startx_truth;
    forecast = forecast_grid[starty_forecast + iy] + startx_forecast;
    
    for (ix = 0; ix < nx_compare; ix++) {

      if (*forecast && *truth) {
	
	n_succ++;
	
      } else if (!*forecast && *truth) {

	n_fail++;
      
      } else if (*forecast && !*truth) {
	
	n_false++;

      } else {

	n_non++;
	
      }
      
      truth++;
      forecast++;

    } /* ix */

  } /* iy */
  
  count->n_success = (double) n_succ;
  count->n_failure = (double) n_fail;
  count->n_false_alarm = (double) n_false;
  count->n_non_event = (double) n_non;

}

/************************************************************************
 * compute_as_is()
 *
 * Normal contingency table computations - point criterion
 */


void DoVerify::compute_as_is(ui08 **forecast_grid,
                             ui08 **truth_grid,
                             long nbytes_grid,
                             vt_count_t *count)
  
{
  
  register ui08 *truth, *forecast;
  register long i;
  register long n_succ = 0, n_fail = 0, n_false = 0, n_non = 0;

  /*
   * set pointer to start of grids
   */
  
  truth = *truth_grid;
  forecast = *forecast_grid;
  
  /*
   * loop through grid points
   */
  
  for (i = 0; i < nbytes_grid; i++) {
    
    if (*forecast && *truth) {
	
      n_succ++;
	
    } else if (!*forecast && *truth) {

      n_fail++;
      
    } else if (*forecast && !*truth) {
	
      n_false++;

    } else {

      n_non++;
	
    }
      
    truth++;
    forecast++;
      
  } /* i */

  count->n_success = (double) n_succ;
  count->n_failure = (double) n_fail;
  count->n_false_alarm = (double) n_false;
  count->n_non_event = (double) n_non;

}

/////////////////////////////////////////////////////
// increment the count in one struct based on another

void DoVerify::increment_count(vt_count_t *general,
                               vt_count_t *specific)

{

  general->n_success += specific->n_success;
  general->n_failure += specific->n_failure;
  general->n_false_alarm += specific->n_false_alarm;
  general->n_non_event += specific->n_non_event;

}

////////////////////////////////////////////////////
// debug printing

void DoVerify::debug_print(vt_simple_track_t *stracks,
                           long ncurrent,
			   const vector<vt_entry_index_t> &current,
                           ui08 **forecast_grid,
                           ui08 **runs_truth_grid,
                           vt_count_t *count)

{
  
  long i;
  vt_simple_track_t *strack;
  vt_storm_t *storm;

  fprintf(stderr, "Complex track number %ld\n",
	  (long) _tHandle.complex_params->complex_track_num);
    
  fprintf(stderr, "From %s to %s\n",
	  utimstr(_tHandle.complex_params->start_time),
	  utimstr(_tHandle.complex_params->end_time));
    
  fprintf(stderr, "%ld current storm(s)\n", ncurrent);
    
  for (i = 0; i < ncurrent; i++) {

    strack = stracks + current[i].isimple;
    storm = strack->storms + current[i].istorm;

    fprintf(stderr,
	    "Simple num %ld, ientry %ld, scan %ld\n",
	    (long) strack->params.simple_track_num,
	    current[i].istorm,
	    (long) storm->entry.scan_num);

  } /* i */

  VerifyTracks::print_grid(stderr, "Verification grid:",
                           forecast_grid, runs_truth_grid, _nx, _ny);
  
  VerifyTracks::print_contingency_table(stderr, "Contingency table:", count);

}

/******************************************************************
 * compute errors
 */

void DoVerify::compute_errors(long nverify,
                              track_file_forecast_props_t *props_current,
                              track_file_forecast_props_t *props_forecast,
                              track_file_forecast_props_t *props_verify,
                              vt_stats_t *complex_stats,
                              vt_stats_t *file_stats,
                              vt_stats_t *total_stats)

{

  int grid_type;
  double mean_lat, cos_lat;
  double x_scale_factor, y_scale_factor;
  double delta_r; /* rjp 9 Apr 2002 */

  /*
   * for LATLON grids compute the scale factors for
   * converting diffs in lat/lon into km
   */

  grid_type = _sHandle.scan->grid.proj_type;
  
  if (grid_type == TITAN_PROJ_LATLON) {

    /*
     * compute mean lat
     */

    mean_lat =
      props_current->proj_area_centroid_y / props_current->volume;
    cos_lat = cos(mean_lat * DEG_TO_RAD);
    x_scale_factor = KM_PER_DEG_AT_EQUATOR * cos_lat;
    y_scale_factor = KM_PER_DEG_AT_EQUATOR;

  } else {

    x_scale_factor = 1.0;
    y_scale_factor = 1.0;

  }

  /*
   * load up errors for selected props
   */

  if (nverify > 0) {

    /*
     * speed and direction comps
     */

    compute_spd_and_dirn(props_current,
			 props_forecast,
			 x_scale_factor,
			 y_scale_factor);
    
    compute_spd_and_dirn(props_current,
			 props_verify,
			 x_scale_factor,
			 y_scale_factor);
    
    load_errors(&props_forecast->proj_area_centroid_x,
		props_forecast, props_verify,
                complex_stats, file_stats, total_stats,
		x_scale_factor, TRUE, FALSE);
    load_errors(&props_forecast->proj_area_centroid_y,
		props_forecast, props_verify,
                complex_stats, file_stats, total_stats,
		y_scale_factor, TRUE, FALSE);
    load_errors(&props_forecast->vol_centroid_z,
                props_forecast, props_verify,
                complex_stats, file_stats, total_stats,
                1.0, TRUE, FALSE);
    load_errors(&props_forecast->refl_centroid_z,
                props_forecast, props_verify,
                complex_stats, file_stats, total_stats,
                1.0, TRUE, FALSE);
    load_errors(&props_forecast->top,
                props_forecast, props_verify,
                complex_stats, file_stats, total_stats,
                1.0, TRUE, FALSE);
    load_errors(&props_forecast->smoothed_speed,
                props_forecast, props_verify,
                complex_stats, file_stats, total_stats,
                1.0, FALSE, FALSE);
    load_errors(&props_forecast->smoothed_direction,
                props_forecast, props_verify,
                complex_stats, file_stats, total_stats,
                1.0, FALSE, TRUE);

    /* 
     * calculate distance between forecast and verify locations 
     * and load into error stats structures
     * rjp 9 Apr 2002
     */ 
    delta_r=compute_dist_error(props_forecast,
                               props_verify,
                               x_scale_factor,
                               y_scale_factor);

    complex_stats->sum_dist_error += delta_r ;
    complex_stats->sum_sq_dist_error += delta_r*delta_r ;
    file_stats->sum_dist_error += delta_r ;
    file_stats->sum_sq_dist_error += delta_r*delta_r ;
    total_stats->sum_dist_error += delta_r ;
    total_stats->sum_sq_dist_error += delta_r*delta_r ;

    /*
     * increment counters
     */
    
    complex_stats->n_movement++;
    file_stats->n_movement++;
    total_stats->n_movement++;

  } /* if (nverify > 0) */

  load_errors(&props_forecast->dbz_max,
              props_forecast, props_verify,
              complex_stats, file_stats, total_stats,
              1.0, FALSE, FALSE);
  load_errors(&props_forecast->volume,
              props_forecast, props_verify,
              complex_stats, file_stats, total_stats,
              1.0, FALSE, FALSE);
  load_errors(&props_forecast->precip_flux,
              props_forecast, props_verify,
              complex_stats, file_stats, total_stats,
              1.0, FALSE, FALSE);
  load_errors(&props_forecast->mass,
              props_forecast, props_verify,
              complex_stats, file_stats, total_stats,
              1.0, FALSE, FALSE);
  load_errors(&props_forecast->proj_area,
              props_forecast, props_verify,
              complex_stats, file_stats, total_stats,
              1.0, FALSE, FALSE);

  /*
   * increment counters
   */

  complex_stats->n_growth++;
  file_stats->n_growth++;
  total_stats->n_growth++;

}


/*********************************************************************
 * compute_spd_and_dirn()
 *
 * compute the speed and direction from dx and dy
 *
 *********************************************************************/

void DoVerify::compute_spd_and_dirn(track_file_forecast_props_t *current,
                                    track_file_forecast_props_t *future,
                                    double x_scale_factor,
                                    double y_scale_factor)

{

  double dx_km, dy_km;
  double speed, direction;
  double xx1, xx2, yy1, yy2;

  /*
   * get start and end coords - the positional parameters
   * are weighted by volume
   */

  if (current->volume != 0) {
    xx1 = current->proj_area_centroid_x / current->volume;
    yy1 = current->proj_area_centroid_y / current->volume;
  } else {
    xx1 = 0.0;
    yy1 = 0.0;
  }

  if (future->volume != 0) {
    xx2 = future->proj_area_centroid_x / future->volume;
    yy2 = future->proj_area_centroid_y / future->volume;
  } else {
    xx2 = 0.0;
    yy2 = 0.0;
  }

  dx_km = (xx2 - xx1) * x_scale_factor;
  dy_km = (yy2 - yy1) * y_scale_factor;

  speed = sqrt(dx_km * dx_km + dy_km * dy_km);

  if (dx_km == 0.0 && dy_km == 0.0)
    direction = 0.0;
  else
    direction = atan2(dx_km, dy_km) * RAD_TO_DEG;

  if (direction < 0.0)
    direction += 360.0;

  future->smoothed_speed = speed;
  future->smoothed_direction = direction;

}


/*********************************************************************
 * compute_dist_error()
 *
 * compute the distance between forecast and verify positions
 * from dx and dy
 *
 *********************************************************************/

double DoVerify::compute_dist_error(track_file_forecast_props_t *forecast,
                                    track_file_forecast_props_t *verify,
                                    double x_scale_factor,
                                    double y_scale_factor)

{

  double dx_km, dy_km, range;
  double xx1, xx2, yy1, yy2;

  /*
   * get forecast and verify coords 
   * Note the positional parameters are weighted by volume
   */

  xx1 = forecast->proj_area_centroid_x / forecast->volume;
  yy1 = forecast->proj_area_centroid_y / forecast->volume;

  xx2 = verify->proj_area_centroid_x / verify->volume;
  yy2 = verify->proj_area_centroid_y / verify->volume;

  dx_km = (xx2 - xx1) * x_scale_factor;
  dy_km = (yy2 - yy1) * y_scale_factor;

  range = sqrt(dx_km * dx_km + dy_km * dy_km);

  return(range);

}


/**********************************************************************
 * load_errors()
 * 
 * compute and load absolute and normalized errors
 */

void DoVerify::load_errors(fl32 *forecast_ptr,
                           track_file_forecast_props_t *props_forecast,
                           track_file_forecast_props_t *props_verify,
                           vt_stats_t *complex_stats,
                           vt_stats_t *file_stats,
                           vt_stats_t *total_stats,
                           double scale_factor,
                           int vol_weighted,
                           int dirn_prop)
		      
{

  long offset;
  double forecast_val, verify_val;
  double norm_error;
  double sq_error;
  double error;
  double norm_sq_error;

  /*
   * compute the offset into the struct
   */

  offset = forecast_ptr - (fl32 *) props_forecast;

  /*
   * movement props are weighted by volume
   */
  
  if (vol_weighted) {

    if (props_forecast->volume == 0.0) {
      forecast_val = 0.0;
    } else {
      forecast_val = *forecast_ptr / props_forecast->volume;
    }

    if (props_verify->volume == 0.0) {
      verify_val = 0.0;
      forecast_val = 0.0;
    } else {
      verify_val = *((fl32 *) props_verify + offset) / props_verify->volume;
    }

  } else {

    forecast_val = *forecast_ptr;
    verify_val = *((fl32 *) props_verify + offset);

  }

  forecast_val *= scale_factor;
  verify_val *= scale_factor;

  error = forecast_val - verify_val;

  /*
   * for direction, allow for the problems at the North line
   */

  if (dirn_prop) {
    if (error > 180.0)
      error -= 360.0;
    else if (error < -180.0)
      error += 360.0;
  } /* if (dirn_prop) */

  sq_error = error * error;

  if (verify_val != 0.0) {
    norm_error = error / verify_val;
    norm_sq_error = sq_error / (verify_val * verify_val);
  } else {
    norm_error = 0.0;
    norm_sq_error = 0.0;
  } 

  *((fl32 *) &complex_stats->sum_error + offset) += error;
  *((fl32 *) &complex_stats->sum_sq_error + offset) += sq_error;
  *((fl32 *) &complex_stats->norm_sum_error + offset) += norm_error;
  *((fl32 *) &complex_stats->norm_sum_sq_error + offset) += norm_sq_error;
  
  *((fl32 *) &file_stats->sum_error + offset) += error;
  *((fl32 *) &file_stats->sum_sq_error + offset) += sq_error;
  *((fl32 *) &file_stats->norm_sum_error + offset) += norm_error;
  *((fl32 *) &file_stats->norm_sum_sq_error + offset) += norm_sq_error;
  
  *((fl32 *) &total_stats->sum_error + offset) += error;
  *((fl32 *) &total_stats->sum_sq_error + offset) += sq_error;
  *((fl32 *) &total_stats->norm_sum_error + offset) += norm_error;
  *((fl32 *) &total_stats->norm_sum_sq_error + offset) += norm_sq_error;

  /*
   * correlation coefficient
   */

  *((fl32 *) &total_stats->sumx + offset) += verify_val;
  *((fl32 *) &total_stats->sumx2 + offset) += verify_val * verify_val;
  *((fl32 *) &total_stats->sumy + offset) += forecast_val;
  *((fl32 *) &total_stats->sumy2 + offset) += forecast_val * forecast_val;
  *((fl32 *) &total_stats->sumxy + offset) += verify_val * forecast_val;
  
}

/******************************************************************
 * sets the contingency struct
 */

void DoVerify::set_contingency
  (vt_count_t *count, track_file_contingency_data_t *cont)
   
{
  
  cont->n_success = (fl32) count->n_success;
  cont->n_failure = (fl32) count->n_failure;
  cont->n_false_alarm = (fl32) count->n_false_alarm;

}
/*********************************************************************
 * computes the errors for forecast data
 */

void DoVerify::compute_stats(vt_stats_t *stats)

{

  /*
   * compute stats
   */

  if (stats->n_movement > 0) {
    
    load_stats(stats, &stats->bias.proj_area_centroid_x, stats->n_movement);
    load_stats(stats, &stats->bias.proj_area_centroid_y, stats->n_movement);
    load_stats(stats, &stats->bias.vol_centroid_z, stats->n_movement);
    load_stats(stats, &stats->bias.refl_centroid_z, stats->n_movement);
    load_stats(stats, &stats->bias.top, stats->n_movement);
    load_stats(stats, &stats->bias.smoothed_speed, stats->n_movement);
    load_stats(stats, &stats->bias.smoothed_direction, stats->n_movement);
    
  } /* if (stats->n_movement > 0) */

  if (stats->n_growth > 0) {

    load_stats(stats, &stats->bias.dbz_max, stats->n_growth);
    load_stats(stats, &stats->bias.volume, stats->n_growth);
    load_stats(stats, &stats->bias.precip_flux, stats->n_growth);
    load_stats(stats, &stats->bias.mass, stats->n_growth);
    load_stats(stats, &stats->bias.proj_area, stats->n_growth);

  } /* if (Stats->n_growth > 0) */
  
}

/*****************************************************************
 * load_stats()
 */

void DoVerify::load_stats(vt_stats_t *stats, fl32 *prop_p, double n)

{
  
  long offset;
  double sumx, sumx2;
  double sumy, sumy2;
  double sumxy;
  double corr;
  double num, denomsq;

  /*
   * compute the offset into the struct
   */

  offset = prop_p - (fl32 *) &stats->bias;

  /*
   * bias and rmse
   */

  *((fl32 *) &stats->bias + offset) =
    *((fl32 *) &stats->sum_error + offset) / n;

  *((fl32 *) &stats->rmse + offset) =
    sqrt(*((fl32 *) &stats->sum_sq_error + offset) / n);

  *((fl32 *) &stats->norm_bias + offset) =
    *((fl32 *) &stats->norm_sum_error + offset) / n;

  *((fl32 *) &stats->norm_rmse + offset) =
    sqrt(*((fl32 *) &stats->norm_sum_sq_error + offset) / n);

  /*
   * correlation coefficient
   */

  sumx = *((fl32 *) &stats->sumx + offset);
  sumx2 = *((fl32 *) &stats->sumx2 + offset);
  sumy = *((fl32 *) &stats->sumy + offset);
  sumy2 = *((fl32 *) &stats->sumy2 + offset);
  sumxy = *((fl32 *) &stats->sumxy + offset);
  
  denomsq = (n * sumx2 - sumx * sumx) * (n * sumy2 - sumy * sumy);
  num = n * sumxy - sumx * sumy;

  if (denomsq > 0.0) {
    corr = num / sqrt(denomsq);
  } else {
    corr = 0.0;
  }
  
  *((fl32 *) &stats->corr + offset) = corr;
  
}

/*****************************************************************
 * load_file_stats()
 */

void DoVerify::load_file_stats(track_file_params_t *tparams,
                               vt_stats_t *stats,
                               si32 *n_samples,
                               track_file_forecast_props_t *bias,
                               track_file_forecast_props_t *rmse)
  
{

  *n_samples = (int) stats->n_growth;

  if (*n_samples == 0) {

    memset((void *) bias, 0, sizeof(track_file_forecast_props_t));
    memset((void *) rmse, 0, sizeof(track_file_forecast_props_t));

  } else {

    *bias = stats->bias;
    *rmse = stats->rmse;

  }

}

/////////////////////////////////////////////////////
// perform the verification using MDV files for truth

int DoVerify::_performMdvVerification()

{
  
  // loop through the scans in the storm file

  for (int iscan = 0; iscan < _sHandle.header->n_scans; iscan++) {

    // read in storm scan

    if (RfReadStormScan(&_sHandle, iscan, "_performMdvVerification")) {
      return -1;
    }
    storm_file_scan_header_t *scan = _sHandle.scan;

    if (_params.debug) {
      cerr << "Reading scan at time: " << utimstr(scan->time) << endl;
    }

    // read the verification file, if it exists, at the
    // scan time plus the forecast lead time
    // load up the truth grid
    
    if (_readMdvTruthFile(scan)) {
      continue;
    }

    // read in track scan entries

    if (RfReadTrackScanEntries(&_tHandle, iscan, "_performMdvVerification")) {
      return -1;
    }
    int nEntries = _tHandle.scan_index[iscan].n_entries;

    // initialize the forecast grid
    
    memset (*_forecastGrid, 0, _nbytesGrid);
    
    // loop through the entries, adding to forecast grid as we go
    
    track_file_entry_t *entry = _tHandle.scan_entries;
    
    for (int ientry = 0; ientry < nEntries; ientry++, entry++) {
   
      storm_file_global_props_t *gprops = _sHandle.gprops + entry->storm_num;
      
      load_forecast_grid(entry, gprops,
                         _params.forecast_lead_time,
                         _forecastGrid);
      
    } // ientry
    
    // compute contingency data for this grid
    
    vt_count_t count;
    compute_contingency_data(_forecastGrid,
                             _truthGrid,
                             _nbytesGrid,
                             &count);
	  
    // increment total count
    
    increment_count(&_totalCount, &count);
          
  } // iscan

  return 0;

}

////////////////////////////////////////////////////////////
// read in MDV truth file

int DoVerify::_readMdvTruthFile(const storm_file_scan_header_t *scan)
  
{

  memset (*_truthGrid, 0, _nbytesGrid);
  
  time_t verifyTime = (time_t) scan->time + _params.forecast_lead_time;
  Mdvx mdvx;
  mdvx.setReadTime(Mdvx::READ_CLOSEST,
                   _params.truth_mdv_dir,
                   _params.truth_mdv_time_margin,
                   verifyTime);
  mdvx.addReadField(_params.truth_mdv_field);
  mdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  mdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  mdvx.setReadComposite();

  if (scan->grid.proj_type == TITAN_PROJ_FLAT) {
    mdvx.setReadRemapFlat(_nx, _ny, _minx, _miny, _dx, _dy,
                          scan->grid.proj_origin_lat,
                          scan->grid.proj_origin_lon,
                          0.0);
  } else {
    mdvx.setReadRemapLatlon(_nx, _ny, _minx, _miny, _dx, _dy);
  }

  if (mdvx.readVolume()) {
    if (_params.debug) {
      cerr << "Cannot find MDV file at time: " << utimstr(verifyTime) << endl;
    } else if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "ERROR - DoVerify::_readMdvFile" << endl;
      cerr << "  Cannot read in truth file" << endl;
      cerr << "  " << mdvx.getErrStr() << endl;
    }
    return -1;
  }

  if (_params.debug) {
    cerr << "Reading verification file: " << mdvx.getPathInUse() << endl;
  }

  // load up truth grid if truth exceeds threshold

  MdvxField *fld = mdvx.getField(_params.truth_mdv_field);

  fl32 *truth = (fl32 *) fld->getVol();
  fl32 threshold = _params.truth_mdv_threshold;

  for (int iy = 0; iy < _ny; iy++) {
    for (int ix = 0; ix < _nx; ix++, truth++) {
      if (*truth >= threshold) {
        _truthGrid[iy][ix] = 1;
      }
    }
  }

  return 0;

}

//////////////////////////////////////////////////////////
// save verification parameters in the track file header

void DoVerify::_saveVerificationParameters()

{

  titan_grid_t *grid = &_tHandle.header->verify.grid;
  *grid = _sHandle.scan->grid;

  grid->proj_params.flat.rotation = 0.0;
  
  grid->nx = _nx;
  grid->ny = _ny;
  grid->nz = 1;
  
  grid->dz_constant = TRUE;
  
  grid->minx = _minx;
  grid->miny = _miny;
  
  grid->dx = _dx;
  grid->dy = _dy;
  grid->dz = 1.0;

  int grid_type = _sHandle.scan->grid.proj_type;
  if (grid_type == TITAN_PROJ_LATLON) {
    strcpy(grid->unitsx, "km");
    strcpy(grid->unitsy, "km");
  } else {
    strcpy(grid->unitsx, "deg");
    strcpy(grid->unitsy, "deg");
  }
  strcpy(grid->unitsz, "km");

  _tHandle.header->verify.verification_performed = TRUE;
  
  _tHandle.header->verify.end_time = _sHandle.header->end_time;
  
  _tHandle.header->verify.forecast_lead_time =
    _params.forecast_lead_time;

  _tHandle.header->verify.forecast_lead_time_margin =
    _params.search_time_margin;

  _tHandle.header->verify.forecast_min_history =
    _params.forecast_min_history;
  
  RfWriteTrackHeader(&_tHandle, "saveVerificationParameters");

}
