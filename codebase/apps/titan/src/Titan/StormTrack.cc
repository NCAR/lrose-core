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
// StormTrack.cc
//
// StormTrack object
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 1999
//
///////////////////////////////////////////////////////////////

#include "StormTrack.hh"
#include "TrOverlaps.hh"
#include "TrConsolidate.hh"
#include "TrForecast.hh"
#include <toolsa/pmu.h>
#include <toolsa/Path.hh>
#include <rapmath/umath.h>
using namespace std;

// Constructor

StormTrack::StormTrack(const string &prog_name, const Params &params,
		       const string &storm_header_path) :
  Worker(prog_name, params)
  
{

  _stormHeaderPath = storm_header_path;
  _stateFilePath = _params.storm_data_dir;
  _stateFilePath += PATH_DELIM;
  _stateFilePath += "_tracking.state";

  _filePrepared = false;
  
  _prev_scan_entry_offset = 0;
  _write_in_progress = false;

}

// destructor

StormTrack::~StormTrack()

{
  _clearStorms1();
  _clearStorms2();
  _closeFiles();
}

///////////////////////
// clear storms1 vector

void StormTrack::_clearStorms1()

{
  for (size_t ii = 0; ii < _storms1.size(); ii++) {
    delete _storms1[ii];
  }
  _storms1.clear();
}

///////////////////////
// clear storms2 vector

void StormTrack::_clearStorms2()

{
  for (size_t ii = 0; ii < _storms2.size(); ii++) {
    delete _storms2[ii];
  }
  _storms2.clear();
}

//////////////////////////////////////////////////
// ReTrack()
//
// Re-track a storm file

int StormTrack::ReTrack ()

{

  _fatalError = false;

  // open the files

  if (_openFiles("w+", _stormHeaderPath.c_str())) {
    cerr << "ERROR - StormTrack::ReTrack" << endl;
    cerr << "  Cannot open storm file: " << _stormHeaderPath << endl;
    return -1;
  }

  // set up scan 0

  if (_setupScan(0)) {
    cerr << "ERROR - StormTrack::ReTrack" << endl;
    cerr << "  Cannot setup scan 0 for file: " << _stormHeaderPath << endl;
    return -1;
  }
  _time1 = _scan_time;

  // load the storm props for scan 0 into array _storms1

  _clearStorms1();
  if (_loadStormProps(_storms1)) {
    cerr << "ERROR - StormTrack::ReTrack" << endl;
    return -1;
  }

  // allocate the track_utime vector
  
  _trackUtime.clear();
  for (int i = 0; i < _n_storms; i++) {
    track_utime_t ttime;
    MEM_zero(ttime);
    _trackUtime.push_back(ttime);
  }
  
  // prepare a new track file
  
  if (_prepareTrackFile()) {
    cerr << "ERROR - StormTrack::ReTrack" << endl;
    return -1;
  }
  
  // loop through scans

  int nScans = _sfile.header().n_scans;
  for (int iscan = 1; iscan < nScans; iscan++) {
    
    // set up this scan

    if (_setupScan(iscan)) {
      cerr << "ERROR - StormTrack::ReTrack" << endl;
      cerr << "  Cannot setup scan " << iscan
	   << " for file: " << _stormHeaderPath << endl;
      return -1;
    }
    _time2 = _scan_time;

    // load the storm props for this scan into _storms2
    
    _clearStorms2();
    if (_loadStormProps(_storms2)) {
      cerr << "ERROR - StormTrack::ReTrack" << endl;
      cerr << "  Cannot load storm props" << endl;
      return -1;
    }
    
    // init arrays for tracking

    _initForTracking();

    // track

    if (_track(iscan)) {
      cerr << "ERROR - StormTrack::ReTrack" << endl;
      cerr << "  Cannot track scan number: " << iscan << endl;
      return -1;
    }
    
    // copy the continuing tracks from _storms2 across to
    // _storms1, freeing the rest

    _transferStorms();
    
  } // iscan

  _tfile.Reinit();
  _sfile.CloseFiles();
  _tfile.CloseFiles();

  return 0;

}

//////////////////////////////////////////////////
// PrepareNewFile()
//
// Prepare a new track file

int StormTrack::PrepareNewFile()

{

  _fatalError = false;

  // open the files and lock

  if (_openFiles("w+", _stormHeaderPath.c_str())) {
    cerr << "ERROR - StormTrack::PrepareNewFile" << endl;
    cerr << "  Cannot open storm file: " << _stormHeaderPath << endl;
    return -1;
  }
  if (_tfile.LockHeaderFile("w")){
    cerr << "ERROR - " << _progName << "::StormTrack::PrepareNewFile" << endl;
    cerr << _tfile.getErrStr() << endl;
    return -1;
  }

  // set up scan 0

  if (_setupScan(0)) {
    cerr << "ERROR - StormTrack::PrepareNewFile" << endl;
    cerr << "  Cannot setup scan 0 for file: " << _stormHeaderPath << endl;
    return -1;
  }
  _time1 = _scan_time;

  // load the storm props for scan 0 into array _storms1
  
  _clearStorms1();
  if (_loadStormProps(_storms1)) {
    cerr << "ERROR - StormTrack::PrepareNewFile" << endl;
    return -1;
  }

  // allocate the track_utime vector

  _trackUtime.clear();
  for (int i = 0; i < _n_storms; i++) {
    track_utime_t ttime;
    MEM_zero(ttime);
    _trackUtime.push_back(ttime);
  }
  
  // prepare a new track file
  
  if (_prepareTrackFile()) {
    cerr << "ERROR - StormTrack::PrepareNewFile" << endl;
    return -1;
  }

  // save the current state to file

  if (_saveCurrentState()) {
    cerr << "ERROR - StormTrack::PrepareNewFile" << endl;
    cerr << "  Cannot save current state" << endl;
    return -1;
  }

  // set flag

  _filePrepared = true;

  // unlock files

  if (_tfile.UnlockHeaderFile()){
    cerr << "ERROR - " << _progName << "::StormTrack::PrepareNewFile" << endl;
    cerr << _tfile.getErrStr() << endl;
    return -1;
  }
  
  return 0;

}

//////////////////////////////////////////////////
// PrepareForAppend()
//
// Prepare existing track file for appending

int StormTrack::PrepareForAppend ()

{

  _fatalError = false;

  int startScan;

  if (_readPrevState()) {
    if (PrepareNewFile()) {
      cerr << "ERROR - StormTrack::PrepareForAppend" << endl;
      cerr << "  Read prev state failed, "
	   << "and we cannot prepare a new file" << endl;
      return -1;
    }
    startScan = 1;
  } else {
    startScan = _tfile._header.last_scan_num + 1;
    if (_tfile.SeekEndData()) {
      cerr << "ERROR - " << _progName
	   << "::StormTrack::PrepareForAppend" << endl;
      cerr << _tfile.getErrStr() << endl;
      return -1;
    }
  }

  // unset flag
  
  _filePrepared = false;
  
  // loop through existing scans

  int nScans = _sfile.header().n_scans;

  for (int iscan = startScan; iscan < nScans; iscan++) {
    
    // lock the files
    
    if (_tfile.LockHeaderFile("w")){
      cerr << "ERROR - " << _progName
	   << "::StormTrack::PrepareForAppend" << endl;
      cerr << _tfile.getErrStr() << endl;
      return -1;
    }

    // set up this scan

    if (_setupScan(iscan)) {
      cerr << "ERROR - StormTrack::PrepareForAppend" << endl;
      cerr << "  Cannot setup scan " << iscan
	   << " for file: " << _stormHeaderPath << endl;
      return -1;
    }
    _time2 = _scan_time;

    // load the storm props for this scan into _storms2
    
    _clearStorms2();
    if (_loadStormProps(_storms2)) {
      cerr << "ERROR - StormTrack::PrepareForAppend" << endl;
      cerr << "  Cannot load storm props" << endl;
      return -1;
    }
    
    // init arrays for tracking

    _initForTracking();

    // track
    
    if (_track(iscan)) {
      cerr << "ERROR - StormTrack::PrepareForAppend" << endl;
      cerr << "  Cannot track scan number: " << iscan << endl;
      _removeCurrentStateFile();
      return -1;
    }
    
    // copy the continuing tracks from _storms2 across to
    // _storms1, freeing the rest
    
    _transferStorms();
    
    // save the current state to file
    
    if (_saveCurrentState()) {
      cerr << "ERROR - StormTrack::PrepareForAppend" << endl;
      cerr << "  Cannot save current state" << endl;
      return -1;
    }

    // unlock the files

    if (_tfile.UnlockHeaderFile()){
      cerr << "ERROR - " << _progName
	   << "::StormTrack::PrepareForAppend" << endl;
      cerr << _tfile.getErrStr() << endl;
      return -1;
    }

  } // iscan

  // set flag

  _filePrepared = true;

  return 0;

}

//////////////////////////////////////////////////
// TrackLastScan()
//
// Track the last scan in storm file

int StormTrack::TrackLastScan ()

{

  _fatalError = false;

  if (!_filePrepared) {
    cerr << "ERROR - StormTrack::TrackLastScan" << endl;
    cerr << "  File has not been prepared yet." << endl;
    return -1;
  }

  // lock the files
  
  if (_tfile.LockHeaderFile("w")){
    cerr << "ERROR - " << _progName
	 << "::StormTrack::TrackLastScan" << endl;
    cerr << _tfile.getErrStr() << endl;
    return -1;
  }

  // set up latest scan

  if (_setupScan(-1)) {
    cerr << "ERROR - StormTrack::TrackLastScan" << endl;
    cerr << "  Cannot setup latest scan "
	 << " for file: " << _stormHeaderPath << endl;
    return -1;
  }
  _time2 = _scan_time;

  // load the storm props for this scan into _storms2
  
  _clearStorms2();
  if (_loadStormProps(_storms2)) {
    cerr << "ERROR - StormTrack::TrackLastScan" << endl;
    cerr << "  Cannot load storm props" << endl;
    return -1;
  }
  
  // init arrays for tracking
  
  _initForTracking();
  
  // track
  
  if (_track(_scan_num)) {
    cerr << "ERROR - Titan::StormTrack::TrackLastScan" << endl;
    cerr << "  Cannot track scan number: " << _scan_num << endl;
    _removeCurrentStateFile();
    return -1;
  }
  
  // copy the continuing tracks from _storms2 across to
  // _storms1, freeing the rest
  
  _transferStorms();
  
  // save the current state to file
  
  if (_saveCurrentState()) {
    cerr << "ERROR - Titan::StormTrack::TrackLastScan" << endl;
    cerr << "  Cannot save current state" << endl;
    return -1;
  }
  
  // unlock the files
  
  if (_tfile.UnlockHeaderFile()){
    cerr << "ERROR - " << _progName
	 << "::StormTrack::TrackLastScan" << endl;
    cerr << _tfile.getErrStr() << endl;
    return -1;
  }

  return (0);

}

/////////////////////
// _loadStormProps()
//
// load storm props
//
// Returns 0 on success, -1 on failure.
//

int StormTrack::_loadStormProps(vector<TrStorm*> &storms)

{
     
  for (int istorm = 0; istorm < _n_storms; istorm++) {
    TrStorm *storm = new TrStorm;
    if (storm->load_props(istorm, _scan_time, _sfile)) {
      cerr << "ERROR - StormTrack::_loadStormProps" << endl;
      cerr << "  Storm proj runs not available" << endl;
      delete storm;
      return -1;
    }
    storm->status.match = -1;
    storms.push_back(storm);
  }
  return 0;

}

/******************************************************************
 * _initForTracking()
 *
 * reallocate and initialize vectors
 */

void StormTrack::_initForTracking()

{
  
  // vector reallocation

  int nToAdd =
    (_tfile._header.n_simple_tracks + _storms2.size()) - _trackUtime.size();
  
  for (int i = 0; i < nToAdd; i++) {
    track_utime_t ttime;
    MEM_zero(ttime);
    _trackUtime.push_back(ttime);
  }
  
  // reset storms1 array values
    
  for (size_t istorm = 0; istorm < _storms1.size(); istorm++) {
    TrStorm &storm = *_storms1[istorm];
    storm.status.sum_overlap = 0.0;
    storm.status.sum_simple_tracks = 0;
    storm.status.n_match = 0;
    storm.status.match = -1;
  }

}

//////////////////////////////////////////////////
// _prepareTrackFile()
//
// Prepare a new track file

int StormTrack::_prepareTrackFile()

{

  /*
   * initialize the file header
   */

  _initTrackHeader(_tfile._header);

  /*
   * set up intial file position for track data file
   */

  if (_tfile.SeekStartData()) {
    cerr << "ERROR - " << _progName << "::StormTrack::_prepareTrackFile" << endl;
    cerr << _tfile.getErrStr() << endl;
    return -1;
  }

  /*
   * initialize new tracks for all the storms
   */
    
  long first_entry_offset = 0;
  int scan_num = 0;
  _setHeaderInvalid();
  
  for (size_t istorm = 0; istorm < _storms1.size(); istorm++) {
    
    _storms1[istorm]->track.init_new(_tfile,
				     &_time1,
				     _trackUtime,
				     scan_num,
				     true, -1, 0, 0,
				     &_time1,
				     _params.debug >= Params::DEBUG_EXTRA);
    
    _storms1[istorm]->update_times(&_time1,
				   _trackUtime,
				   _params.tracking_forecast_weights_n);
    
    if (_writeTrack(&_time1,
		    _trackUtime,
		    *_storms1[istorm],
		    istorm,
		    scan_num)) {
      cerr << "ERROR - StormTrack::_prepareTrackFile" << endl;
      return -1;
    }
    
    if (istorm == 0) {
      first_entry_offset = _entry_offset;
    }
    
  } /* istorm */

  _tfile.AllocScanIndex(scan_num + 1);
  
  _tfile._scan_index[scan_num].utime = _time1.unix_time;
  _tfile._scan_index[scan_num].n_entries = _storms1.size();
  _tfile._scan_index[scan_num].first_entry_offset = first_entry_offset;
  
  if (_setHeaderValid(scan_num, _stateTag)) {
    cerr << "ERROR - StormTrack::_prepareTrackFile" << endl;
    return -1;
  }
  
  return 0;

}

///////////////////////////////////////////////////////////////////
// _track()
//
// performs tracking algorithm from previous scan to this scan

int StormTrack::_track(int scan_num)
  
{
  
  PMU_auto_register("in _track()");

  // compute delta time in hours and mins
  
  int d_secs = _time2.unix_time - _time1.unix_time;
  double d_hours = (double) d_secs / 3600.0;
  
  // debug print
  
  if (_params.debug) {
    fprintf(stderr, "------------ tracking ------------\n");
    fprintf(stderr, "Tracking scan %d to scan %d\n",
	    (int) (scan_num - 1), (int) scan_num);
    fprintf(stderr, "  Time 1: %2d/%2d/%2d %2d:%2d:%2d, nstorms %d\n",
	    _time1.year, _time1.month, _time1.day,
	    _time1.hour, _time1.min, _time1.sec, (int) _storms1.size());
    fprintf(stderr, "  Time 2: %2d/%2d/%2d %2d:%2d:%2d, nstorms %d\n",
	    _time2.year, _time2.month, _time2.day,
	    _time2.hour, _time2.min, _time2.sec, (int) _storms2.size());
    fprintf(stderr, "  Dtime (secs) : %d\n", (int) d_secs);
  }
  
  if (d_secs <= 0) {
    fprintf(stderr, "ERROR - %s::StormTrack::_track\n", _progName.c_str());
    fprintf(stderr, "Illegal condition - Dtime <= 0\n");
    _fatalError = true;
    return -1;
  }

  // match the storms from time1 to time2 - if match_scans returns failure,
  // set track numbers to next available ones, and reinitialize forecast
  
  if (d_secs <= _params.tracking_max_delta_time &&
      _storms1.size() != 0 && _storms2.size() != 0) {
      
    // compute bounding box limits for each current storm
    // and for the forecast position of each previous storm
    
    _loadBounds(d_hours);
    
    // load up overlap areas between polygons from time 1 and
    // polygons at time 2
    
    TrOverlaps overlaps(_progName, _params);
    overlaps.find(_sfile, _storms1, _storms2, d_hours);

    // check overlap matches for max number of parents and children
    
    _checkMatches();

    // perform optimal match on storms which were not matched
    // using overlaps
    
    _matchStorms(d_hours);
    
    // resolve matches for those storms which do not have overlaps
    // but had a match identified by match_storms()
    
    _resolveMatches();

    // consolidate complex tracks

    TrConsolidate consolidate(_progName, _params);
    if (consolidate.run(_tfile, _storms1, _storms2, _trackUtime)) {
      cerr << "ERROR - StormTrack::_track()" << endl;
      cerr << "  Cannot consolidate tracks" << endl;
      return -1;
    }
    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      _printMatches();
    }
    
  } else {
    
    for (size_t jstorm = 0; jstorm < _storms2.size(); jstorm++) {
      _storms2[jstorm]->status.starts = true;
    }
    
  } // if (d_secs ...
  
  // set header invalid in case program fails before writes are
  // complete

  _setHeaderInvalid();
  
  // set up the tracks at time 2
  
  if (_updateTracks(&_time2, scan_num, d_hours)) {
    cerr << "ERROR - StormTrack::_track()" << endl;
    cerr << "  Cannot update tracks" << endl;
    return -1;
  }
  
  // update the track times and compute the forecasts
  
  TrForecast forecast(_progName, _params);
  
  for (size_t jstorm = 0; jstorm < _storms2.size(); jstorm++) {
    
    _storms2[jstorm]->update_times(&_time2, _trackUtime,
				   _params.tracking_forecast_weights_n);
    
    forecast.compute(_sfile, *_storms2[jstorm], jstorm);
    
  } // jstorm
  
  if (_storms2.size() > 0) {

    // compute speed and direction
    
    forecast.compute_speed_and_dirn(_sfile, _storms2);
    
  }
  
  // write the tracks

  long first_entry_offset = 0;
  for (size_t jstorm = 0; jstorm < _storms2.size(); jstorm++) {
    
    if (_writeTrack(&_time2,
		    _trackUtime,
		    *_storms2[jstorm],
		    jstorm,
		    scan_num)) {
      cerr << "ERROR - StormTrack::_track" << endl;
      return -1;
    }
    
    if (jstorm == 0) {
      first_entry_offset = _entry_offset;
    }
    
  } // jstorm
  
  _tfile.AllocScanIndex(scan_num + 1);
  
  _tfile._scan_index[scan_num].utime = _time2.unix_time;
  _tfile._scan_index[scan_num].n_entries = _storms2.size();
  _tfile._scan_index[scan_num].first_entry_offset = first_entry_offset;
  
  // set header valid and write to file
    
  _setHeaderValid(scan_num, _stateTag);

  return 0;

}

////////////////////////////////////////////////////////////////////
// _transferStorms()
//
// Copy continuing storms across from _storms2 to _storms1, freeing
// the rest.

void StormTrack::_transferStorms()

{

  _clearStorms1();
  for (size_t j = 0; j < _storms2.size(); j++) {
    _storms1.push_back(_storms2[j]);
  }
  _storms2.clear();
  _time1 = _time2;

}

/////////////////////////////////////////////////////////////
// _loadBounds()
//
// Compute the bounding limits for the projected area of each
// storm, either in it's present or forecast state
 
void StormTrack::_loadBounds(double d_hours)
  
{
  
  double current_x, current_y, current_area;
  double forecast_x, forecast_y, forecast_area;
  double dx_dt, dy_dt, darea_dt;
  double length_ratio;
  double current_minx, current_miny, current_maxx, current_maxy;

  const titan_grid_t &grid = _sfile.scan().grid;
  double fcast_minx, fcast_miny, fcast_maxx, fcast_maxy;
  
  for (size_t istorm = 0; istorm < _storms1.size(); istorm++) {

    /*
     * for storms in prev scan, compute the bounding limits
     * based on the forecast as well as the current position
     */

    TrStorm &storm = *_storms1[istorm];
	    
    dx_dt = storm.track.status.dval_dt.proj_area_centroid_x;
    dy_dt = storm.track.status.dval_dt.proj_area_centroid_y;
    darea_dt = storm.track.status.dval_dt.proj_area;
	    
    current_x = storm.current.proj_area_centroid_x;
    current_y = storm.current.proj_area_centroid_y;
    current_area = storm.current.proj_area;
	    
    forecast_x = current_x + dx_dt * d_hours;
    forecast_y = current_y + dy_dt * d_hours;
	    
    forecast_area = current_area + darea_dt * d_hours;
    if (forecast_area < 1.0) {
      forecast_area = 1.0;
    }

    if (current_area > 0) {
      length_ratio = pow((forecast_area / current_area), 0.5);
    } else {
      length_ratio = 1.0;
    }

    current_minx =
      grid.minx + (storm.current.bound.min_ix - 0.5) * grid.dx;
    current_miny =
      grid.miny + (storm.current.bound.min_iy - 0.5) * grid.dy;
    current_maxx =
      grid.minx + (storm.current.bound.max_ix + 0.5) * grid.dx;
    current_maxy =
      grid.miny + (storm.current.bound.max_iy + 0.5) * grid.dy;

    fcast_minx =
      forecast_x - (current_x - current_minx) * length_ratio;
    fcast_miny =
      forecast_y - (current_y - current_miny) * length_ratio;
    fcast_maxx =
      forecast_x - (current_x - current_maxx) * length_ratio;
    fcast_maxy =
      forecast_y - (current_y - current_maxy) * length_ratio;

    storm.box_for_overlap.min_ix =
      (si32) ((fcast_minx - grid.minx) / grid.dx + 0.5);
    storm.box_for_overlap.min_iy =
      (si32) ((fcast_miny - grid.miny) / grid.dy + 0.5);
    storm.box_for_overlap.max_ix =
      (si32) ((fcast_maxx - grid.minx) / grid.dx + 0.5);
    storm.box_for_overlap.max_iy =
      (si32) ((fcast_maxy - grid.miny) / grid.dy + 0.5);

    storm.track.status.forecast_x = forecast_x;
    storm.track.status.forecast_y = forecast_y;
    storm.track.status.forecast_area = forecast_area;
    storm.track.status.forecast_length_ratio = length_ratio;

  } /* istorm */

  for (size_t jstorm = 0; jstorm < _storms2.size(); jstorm++) {

    /*
     * for storms in current scan, bounding box is based
     * on the current scan
     */

    TrStorm &storm = *_storms2[jstorm];
    storm.box_for_overlap = storm.current.bound;

  } // jstorm

}

/////////////////////////////////////////////////////////////////
//
// saves the current state to be used in the case of a restart
//
// Returns 0 on success, -1 on failure
//

int StormTrack::_saveCurrentState()

{

  int iret = 0;
  int flag;
  FILE *fp;

  // open state file

  if ((fp = fopen(_stateFilePath.c_str(), "w")) == NULL) {
    fprintf(stderr, "%s::StormTrack::_saveCurrentState\n",
	    _progName.c_str());
    fprintf(stderr, "Cannot create state file\n");
    perror(_stateFilePath.c_str());
    return (-1);
  }

  // write a FALSE flag, which is changed to TRUE when save
  // is complete
  
  flag = FALSE;
  ufwrite((char *) &flag, sizeof(int), 1, fp);
  
  // write the state data

  if (ufwrite(&_stateTag, sizeof(si32), 1, fp) != 1) {
    iret = -1;
  }

  if (ufwrite(&_tfile._header.last_scan_num,
	      sizeof(si32), 1, fp) != 1) {
    iret = -1;
  }
  
  si32 nstorms1 = _storms1.size();
  if (ufwrite(&nstorms1, sizeof(si32), 1, fp) != 1) {
    iret = -1;
  }

  if (ufwrite(&_time1, sizeof(date_time_t), 1, fp) != 1) {
    iret = -1;
  }

  for (size_t istorm = 0; istorm < _storms1.size(); istorm++) {
    if (ufwrite(&_storms1[istorm]->track.status,
		sizeof(TrTrack::status_t), 1, fp) != 1) {
      iret = -1;
    }
  }
  
  si32 nsimple = _tfile._header.n_simple_tracks;
  if (ufwrite(&nsimple, sizeof(si32), 1, fp) != 1) {
    iret = -1;
  }

  for (int i = 0; i < nsimple; i++) {
    if (ufwrite(&_trackUtime[i], sizeof(track_utime_t), 1, fp) != 1) {
      iret = -1;
    }
  }

  // write a TRUE flag to indicate that save is successful
  
  if (iret == 0) {
    flag = TRUE;
    fseek(fp, 0L, 0);
    if (ufwrite((char *) &flag, sizeof(int), 1, fp) != 1) {
      iret = -1;
    }
  }
  
  // close file
  
  fclose(fp);

  return (iret);

}

///////////////////////////////////////////////////////////////////////
//
// reads the previous state from file
//
// returns 0 if successful, -1 if not
///

int StormTrack::_readPrevState()

{

  int flag;
  si32 tag;
  si32 nstorms1;
  si32 n_simple_tracks;
  si32 last_scan_num;
  track_file_header_t theader;
  FILE *fp;


  // open files

  if (_openFiles("r+", _stormHeaderPath.c_str())) {
    return (-1);
  }

  // check that track file is valid
  
  if (!_tfile._header.file_valid) {
    _closeFiles();
    return (-1);
  }

  // initialize a test header and compare the header params
  // against the existing file. If they are different, return
  // error

  _initTrackHeader(theader);
  
  if (memcmp(&theader.params,
	     &_tfile._header.params,
	     sizeof(track_file_params_t))) {
    return (-1);
  }
      
  // open state file

  if ((fp = fopen(_stateFilePath.c_str(), "r")) == NULL)
    return (-1);
  
  // read  flag - if not TRUE, return because the file was not
  // correctly written
  
  if (ufread(&flag, sizeof(int), 1, fp) != 1) {
    fclose(fp);
    return (-1);
  }
  
  if (!flag) {
    fclose(fp);
    return (-1);
  }

  // read the state tag, compare it with the track file code

  if (ufread(&tag, sizeof(si32), 1, fp) != 1) {
    fclose(fp);
    return (-1);
  }

  if (tag != _tfile._header.modify_code) {
    fclose(fp);
    return (-1);
  }

  // read the last scan num

  if (ufread(&last_scan_num, sizeof(si32), 1, fp) != 1) {
    fclose(fp);
    return (-1);
  }

  if (last_scan_num >= _sfile.header().n_scans) {
    if (_params.debug >= Params::DEBUG_NORM) {
      fprintf(stderr, "Start tracking again\n");
      fprintf(stderr, "Last_scan_num, n_scans : %d, %d\n",
	      (int) last_scan_num, (int) _sfile.header().n_scans);
    }
    fclose(fp);
    return (-1);
  }
  
  // read in state data
  
  if (ufread(&nstorms1, sizeof(si32), 1, fp) != 1) {
    fclose(fp);
    return (-1);
  }
  
  if (ufread(&_time1, sizeof(date_time_t), 1, fp) != 1) {
    fclose(fp);
    return (-1);
  }
  
  _clearStorms1();
  for (int istorm = 0; istorm < nstorms1; istorm++) {
    TrStorm *storm = new TrStorm;
    if (ufread(&storm->track.status, sizeof(TrTrack::status_t), 1, fp) != 1) {
      fclose(fp);
      return (-1);
    }
    _storms1.push_back(storm);
  } // istorm 

  if (ufread(&n_simple_tracks, sizeof(si32), 1, fp) != 1) {
    fclose(fp);
    return (-1);
  }
  if (n_simple_tracks != _tfile._header.n_simple_tracks) {
    fclose(fp);
    return (-1);
  }
  
  _trackUtime.clear();
  for (int i = 0; i < n_simple_tracks; i++) {
    track_utime_t tr_utime;
    if (ufread(&tr_utime, sizeof(track_utime_t), 1, fp) != 1) {
      fclose(fp);
      return (-1);
    }
    _trackUtime.push_back(tr_utime);
  }
  
  // success

  fclose (fp);
  return (0);

}

///////////////////////////////////////////////////////////
// Remove the current state file

void StormTrack::_removeCurrentStateFile()

{

  fprintf(stderr, "WARNING - %s::StormTrack::_removeCurrentStateFile\n",
	  _progName.c_str());
  fprintf(stderr, "Removing current state file '%s'\n",
	  _stateFilePath.c_str());
  if (unlink(_stateFilePath.c_str())) {
    if (_params.debug >= Params::DEBUG_NORM) {    
      fprintf(stderr, "WARNING - %s::StormTrack::_removeCurrentStateFile\n",
	      _progName.c_str());
      fprintf(stderr, "Cannot remove state file\n");
      perror(_stateFilePath.c_str());
    }
  }

}

////////////////////////////////////////
// open files
//
// returns 0 on success, -1 on failure
//

int StormTrack::_openFiles(const char *access_mode,
			   const char *storm_header_path)
  
{
  
  /*
   * close prev storm file if open, open new storm file
   */
  
  _sfile.CloseFiles();
  if (_sfile.OpenFiles("r", _stormHeaderPath.c_str())) {
    cerr << "ERROR - " << _progName << "::StormTrack::_openFiles" << endl;
    cerr << _sfile.getErrStr() << endl;
    return -1;
  }
  
  /*
   * read in storm header and first scan
   */
  
  if (_sfile.ReadHeader()) {
    cerr << "ERROR - " << _progName << "::StormTrack::_openFiles" << endl;
    cerr << _sfile.getErrStr() << endl;
    return -1;
  }
  
  if (_sfile.ReadScan(0)) {
    cerr << "ERROR - " << _progName << "::StormTrack::_openFiles" << endl;
    cerr << _sfile.getErrStr() << endl;
    return -1;
  }
  
  /*
   * compute the track file path, which is the same as the storm file
   * path except for the extension
   */
  
  Path stormPath(_stormHeaderPath);
  char trackHeaderPath[MAX_PATH_LEN];
  sprintf(trackHeaderPath, "%s%s%s%s%s",
	  stormPath.getDirectory().c_str(),
	  PATH_DELIM,
	  stormPath.getBase().c_str(),
	  ".",
	  TRACK_HEADER_FILE_EXT);
  
  /*
   * open track file for writing and subsequent reading
   */
  
  _tfile.CloseFiles();
  
  if (_tfile.OpenFiles(access_mode, trackHeaderPath, TRACK_DATA_FILE_EXT)) {
    cerr << "ERROR - " << _progName << "::StormTrack::_openFiles" << endl;
    cerr << _tfile.getErrStr() << endl;
    return -1;
  }
  
  // initialize the header

  _initTrackHeader(_tfile._header);

  /*
   * on read, read in track file header
   */

  if (*access_mode == 'r') {
    if (_tfile.ReadHeader()) {
      cerr << "ERROR - " << _progName << "::StormTrack::_openFiles" << endl;
      cerr << _tfile.getErrStr() << endl;
      return -1;
    }
    if (_tfile.ReadSimplesPerComplex()) {
      cerr << "ERROR - " << _progName << "::StormTrack::_openFiles" << endl;
      cerr << _tfile.getErrStr() << endl;
      return -1;
    }
  }

  return (0);

}

//////////////
// close files
//

void StormTrack::_closeFiles()

{

  _sfile.CloseFiles();
  _tfile.CloseFiles();

}

///////////////////////
// _initTrackHeader()
//
// Initialize track file header
//

void StormTrack::_initTrackHeader(track_file_header_t &t_header)

{

  MEM_zero(t_header);
  
  Path stormPath(_stormHeaderPath);
  
  sprintf(_tfile._header.header_file_name,
	  "%s.%s",
	  stormPath.getBase().c_str(),
	  TRACK_HEADER_FILE_EXT);
  
  sprintf(_tfile._header.data_file_name,
	  "%s.%s",
	  stormPath.getBase().c_str(),
	  TRACK_DATA_FILE_EXT);
  
  ustrncpy(_tfile._header.storm_header_file_name,
	   stormPath.getFile().c_str(),
	   R_LABEL_LEN);

  t_header.params.grid_type = _sfile.scan().grid.proj_type;
  t_header.params.nweights_forecast = _params.tracking_forecast_weights_n;
  
  if (_params.tracking_forecast_type == Params::TREND) {
    t_header.params.forecast_type = FORECAST_BY_TREND;
  } else if (_params.tracking_forecast_type == Params::PARABOLIC) {
    t_header.params.forecast_type = FORECAST_BY_PARABOLA;
  } else {
    t_header.params.forecast_type = FORECAST_BY_REGRESSION;
  }

  for (int iweight  = 0;
       iweight < _params.tracking_forecast_weights_n; iweight++) {
    t_header.params.forecast_weights[iweight] =
      _params._tracking_forecast_weights[iweight];
  }

  t_header.params.weight_distance = 
    _params.tracking_weight_distance;
  
  t_header.params.weight_delta_cube_root_volume = 
    _params.tracking_weight_delta_cube_root_volume;
  
  t_header.params.max_tracking_speed = 
    _params.tracking_max_speed;
  
  t_header.params.max_delta_time = _params.tracking_max_delta_time;
  
  t_header.params.min_history_for_valid_forecast =
    _params.tracking_min_history_for_valid_forecast;

  t_header.params.max_speed_for_valid_forecast = 
    _params.tracking_max_speed_for_valid_forecast;
  
  t_header.params.parabolic_growth_period = 
    _params.tracking_parabolic_growth_period;
  
  t_header.params.spatial_smoothing =
    _params.tracking_spatial_smoothing;
  
  t_header.params.smoothing_radius = 
    _params.tracking_smoothing_radius;
  
  t_header.params.use_runs_for_overlaps =
    _params.tracking_use_runs_for_overlaps;

  t_header.params.scale_forecasts_by_history =
    _params.tracking_scale_forecasts_by_history;

  t_header.params.min_fraction_overlap = 0;
  
  t_header.params.min_sum_fraction_overlap = 
    _params.tracking_min_sum_fraction_overlap;
  
  t_header.n_simple_tracks = 0;
  t_header.n_complex_tracks = 0;

  t_header.max_simple_track_num = 0;
  t_header.max_complex_track_num = 0;

  t_header.data_file_size = 0;
  
  t_header.max_parents = MAX_PARENTS;
  t_header.max_children = MAX_CHILDREN;
  t_header.max_nweights_forecast = MAX_NWEIGHTS_FORECAST;
  
}

/**************************************************************************
 * _writeTrack.c
 *
 * write out an existing track
 *
 * Sets _entry_offset
 *
 * Returns 0 on success, -1 on failure
 *
 *******************************************************************************/

int StormTrack::_writeTrack(date_time_t *dtime,
			    vector<track_utime_t> &track_utime,
			    TrStorm &storm,
			    int storm_num,
			    int scan_num)
     
{
  
  TrTrack &track = storm.track;
  int simple_track_num = track.status.simple_track_num;
  int complex_track_num = track.status.complex_track_num;

  /*
   * set up entry
   */
  
  track_file_entry_t &t_entry = _tfile._entry;

  t_entry.simple_track_num = simple_track_num;
  t_entry.complex_track_num = complex_track_num;
  t_entry.scan_num = scan_num;
  t_entry.storm_num = storm_num;
  t_entry.history_in_scans = track.status.history_in_scans;
  t_entry.history_in_secs = track.status.history_in_secs;
  t_entry.scan_origin = track.status.scan_origin;
  t_entry.duration_in_scans = track.status.duration_in_scans;
  t_entry.duration_in_secs = track.status.duration_in_secs;
  t_entry.forecast_valid = track.status.forecast_valid;
  t_entry.time = _sfile.scan().time;
  t_entry.time_origin = track.status.time_origin.unix_time;

  /*
   * load up forecast rates of change
   */

  t_entry.dval_dt.proj_area_centroid_x =
    track.status.dval_dt.proj_area_centroid_x;
  t_entry.dval_dt.proj_area_centroid_y =
    track.status.dval_dt.proj_area_centroid_y;
  t_entry.dval_dt.vol_centroid_z = track.status.dval_dt.vol_centroid_z;
  t_entry.dval_dt.refl_centroid_z = track.status.dval_dt.refl_centroid_z;
  t_entry.dval_dt.top = track.status.dval_dt.top;
  t_entry.dval_dt.dbz_max = track.status.dval_dt.dbz_max;
  t_entry.dval_dt.volume = track.status.dval_dt.volume;
  t_entry.dval_dt.precip_flux = track.status.dval_dt.precip_flux;
  t_entry.dval_dt.mass = track.status.dval_dt.mass;
  t_entry.dval_dt.proj_area = track.status.dval_dt.proj_area;
  t_entry.dval_dt.smoothed_proj_area_centroid_x =
    track.status.dval_dt.smoothed_proj_area_centroid_x;
  t_entry.dval_dt.smoothed_proj_area_centroid_y =
    track.status.dval_dt.smoothed_proj_area_centroid_y;
  t_entry.dval_dt.smoothed_speed = track.status.dval_dt.smoothed_speed;
  t_entry.dval_dt.smoothed_direction = track.status.dval_dt.smoothed_direction;

  /*
   * write this entry to file, returning the offset of the
   * entry in the file. The track.status.entry_offset tells the
   * write routine where to seek for the previous entry
   * written. entry_offset is the offset for this entry.
   */
  
  if (storm_num == 0) {
    _prev_scan_entry_offset = 0;
  }
  
  _entry_offset = _tfile.WriteEntry(track.status.entry_offset,
				    _prev_scan_entry_offset);
  
  if (_entry_offset < 0) {
    cerr << "ERROR - " << _progName << "::StormTrack::_writeTrack" << endl;
    cerr << _tfile.getErrStr() << endl;
    return -1;
  }

  track.status.entry_offset = _entry_offset;
  _prev_scan_entry_offset = _entry_offset;

  /*
   * read the track params back in, update and rewrite
   */

  if (_tfile.ReadSimpleParams(simple_track_num)) {
    cerr << "ERROR - " << _progName << "::StormTrack::_writeTrack" << endl;
    cerr << _tfile.getErrStr() << endl;
    return -1;
  }
  
  simple_track_params_t &st_params = _tfile._simple_params;

  st_params.end_scan = scan_num;
  st_params.history_in_scans = track.status.history_in_scans;
  st_params.history_in_secs = track.status.history_in_secs;
  st_params.duration_in_scans =
    st_params.end_scan - st_params.start_scan + 1;
  st_params.duration_in_secs = track.status.duration_in_secs;
  
  if (track.status.duration_in_scans == 1) {
    st_params.first_entry_offset = track.status.entry_offset;
  }
  
  st_params.end_time = dtime->unix_time;
  track_utime[simple_track_num].end_simple = dtime->unix_time;
  
  if (_tfile.WriteSimpleParams(simple_track_num)) {
    cerr << "ERROR - " << _progName << "::StormTrack::_writeTrack" << endl;
    cerr << _tfile.getErrStr() << endl;
    return -1;
  }
  
  /*
   * update the complex track
   */

  if (_tfile.ReadComplexParams(complex_track_num, false)) {
    cerr << "ERROR - " << _progName << "::StormTrack::_writeTrack" << endl;
    cerr << _tfile.getErrStr() << endl;
    return -1;
  }
  
  /*
   * update this complex track if it has not already been
   * brought up to date
   */

  complex_track_params_t &ct_params = _tfile._complex_params;

  if ((ct_params.end_scan != scan_num) ||
      ct_params.duration_in_scans == 0) {
    
    ct_params.end_scan = scan_num;

    ct_params.duration_in_scans = 
      ct_params.end_scan - ct_params.start_scan + 1;

    track_utime[complex_track_num].end_complex = dtime->unix_time;
      
    if (ct_params.duration_in_scans == 1) {

      ct_params.duration_in_secs = 0;

    } else {

      ct_params.duration_in_secs = (int)
	(((double) dtime->unix_time -
	  (double) track_utime[complex_track_num].start_complex) *
	 (ct_params.duration_in_scans /
	  (ct_params.duration_in_scans - 1.0)) + 0.5);

    }
      
    ct_params.end_time = dtime->unix_time;

    /*
     * rewrite the amended track params
     */
  
    if (_tfile.WriteComplexParams(complex_track_num)) {
      cerr << "ERROR - " << _progName << "::StormTrack::_writeTrack" << endl;
      cerr << _tfile.getErrStr() << endl;
      return -1;
    }
  
  } /* if (ct_params.end_scan != scan_num) */

  return (0);

}

/****************************************************************
 * _setupScan()
 *
 * Reads in the file header and the scan_header and
 * sets up the variables for tracking
 *
 * If scan_num is positive, sets up for this scan.
 * If scan_num is negative, sets up for the last scan.
 *
 * Sets _scan_num, _scan_time and and _n_storms.
 * Use getScanNum(), getScanTime() and getNStorms() to access these members.
 *
 * returns 0 on success, -1 on failure.
 *
 */

int StormTrack::_setupScan(int scan_num)

{
  
  /*
   * re-open storm file to force re-reads (in LINUX the re-read
   * without re-open does not seem to work)
   */
  
  _sfile.CloseFiles();

  if (_sfile.OpenFiles("r", _stormHeaderPath.c_str())) {
    cerr << "ERROR - " << _progName << "::StormTrack::_setupScan" << endl;
    cerr << _sfile.getErrStr() << endl;
    return -1;
  }

  /*
   * Read in file header, to get the latest file information
   */

  if (_sfile.ReadHeader()) {
    cerr << "ERROR - " << _progName << "::StormTrack::_setupScan" << endl;
    cerr << _sfile.getErrStr() << endl;
    return -1;
  }

  if (scan_num < 0) {
    _scan_num = _sfile.header().n_scans - 1;
  } else {
    _scan_num = scan_num;
  }
  
  /*
   * read in data for this scan
   */
  
  if (_sfile.ReadScan(_scan_num)) {
    cerr << "ERROR - " << _progName << "::StormTrack::_setupScan" << endl;
    cerr << _sfile.getErrStr() << endl;
    return -1;
  }

  /*
   * store current scan time and number of storms
   */
  
  _scan_time.unix_time = _sfile.scan().time;
  uconvert_from_utime(&_scan_time);
  _n_storms = _sfile.scan().nstorms;

  return (0);

}

/****************************************************************
 * _setHeaderInvalid()
 *
 * sets file header invalid while data writes are in progress
 * so that it can be determined if the writes were completed
 *
 * Returns 0 on success, -1 on failure.
 */

int StormTrack::_setHeaderInvalid()

{

  _write_in_progress = true;

  /*
   * Clear valid flag and write header
   */

  _tfile._header.file_valid = false;
    
  if (_tfile.WriteHeader()) {
    cerr << "ERROR - " << _progName << "::StormTrack::_setHeaderInvalid" << endl;
    cerr << _tfile.getErrStr() << endl;
    return -1;
  }
  _tfile.FlushFiles();

  return (0);

}

/****************************************************************
 * _setHeaderValid()
 *
 * sets file header to valid after data writes are in progress
 * so that it can be determined that the writes were completed
 *
 * Returns 0 on success, -1 on failure.
 */

int StormTrack::_setHeaderValid(int scan_num,
				time_t &state_tag)
  
{

  /*
   * set up state tag as the current unix time and store in file
   * header. Set valid flag.
   */
  
  state_tag = time(NULL);
  _tfile._header.modify_code = state_tag;
  _tfile._header.file_valid = true;
  
  _tfile._header.last_scan_num = scan_num;
  _tfile._header.n_scans = _tfile._header.last_scan_num + 1;
  
  if (_tfile.WriteHeader()) {
    cerr << "ERROR - " << _progName << "::StormTrack::__setHeaderValid" << endl;
    cerr << _tfile.getErrStr() << endl;
    return -1;
  }
  _tfile.FlushFiles();
  
  _write_in_progress = false;

  return (0);
  
}
