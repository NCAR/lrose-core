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
// TitanTrackData.cc: TitanTrackData handling
//
// Serves out TITAN tracks and entries
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1998
//
//////////////////////////////////////////////////////////

#include "TitanTrackData.hh"
using namespace std;

// Constructor

TitanTrackData::TitanTrackData (const string &prog_name, const Params &params,
				DsInputPath &input) :
  TrackData(prog_name, params, input)
  
{

  MEM_zero(_titanGrid);
  _titanGridInitialized = false;

  // initialize storm file handle
  
  RfInitStormFileHandle(&_s_handle, (char *) _progName.c_str());
  
  // initialize track file handle
  
  RfInitTrackFileHandle(&_t_handle, (char *) _progName.c_str());
  
  // open initial track file
  
  char *track_file_path = _input.next();
  if (track_file_path == NULL) {
    fprintf(stderr, "ERROR - %s:TitanTrackData::TitanTrackData\n",
	    _progName.c_str());
    fprintf(stderr, "No track files available\n");
    OK = FALSE;
    return;
  }
  
  if (_openFiles(track_file_path)) {
    fprintf(stderr, "ERROR - %s:TitanTrackData::TitanTrackData\n",
	    _progName.c_str());
    fprintf(stderr, "Cannot open track file %s\n", track_file_path);
    OK = FALSE;
    return;
  }
  
  return;

}

// Destructor

TitanTrackData::~TitanTrackData()

{

  _closeFiles();

  RfFreeStormFileHandle(&_s_handle);
  RfFreeTrackFileHandle(&_t_handle);

}

//////////////////////////////////////////
// loadNextTrack()
//
// Load up next track.
//
// Sets no_more_tracks when all complex tracks have been read.
//
// Returns 0 on success, -1 on failure.
//
//////////////////////////////////////////

int TitanTrackData::loadNextTrack(int *no_more_tracks_p)

{

  *no_more_tracks_p = FALSE;

  char *track_file_path = NULL;

  if (_iComplex >= _t_handle.header->n_complex_tracks) {

    // no tracks left - close storm and track files
    
    _closeFiles();
    
    // open new track file
    
    track_file_path = _input.next();
    if (track_file_path == NULL) {
      *no_more_tracks_p = TRUE;
      return (0);
    }

    if (_openFiles(track_file_path)) {
      fprintf(stderr, "ERROR - %s:TitanTrackData::loadNextTrack\n",
	      _progName.c_str());
      fprintf(stderr, "Cannot open track file %s\n", track_file_path);
      return (-1);
    }

    // check that we have tracks

    if (_t_handle.header->n_complex_tracks == 0) {
      return (-1);
    }
    
  }

  // read in complex track

  int complex_track_num = _t_handle.complex_track_nums[_iComplex];
  _iComplex++;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    fprintf(stderr, "Processing complex track number %5d ...\n",
	    complex_track_num);
  }

  if(RfReadComplexTrackParams(&_t_handle, complex_track_num,
			      TRUE, "TitanTrackData::loadNextTrack")) {
    if (_params.debug) {
      fprintf(stderr, "WARNING - %s:TitanTrackData::loadNextTrack\n",
	      _progName.c_str());
      fprintf(stderr, "RfReadComplexTrackParams failed, complex_track_num %d,"
	      " file path %s\n", complex_track_num, track_file_path);
    }
    return(-1);
  }
 
  // set track quantities

  complex_track_params_t *ct_params = _t_handle.complex_params;
  startTime = ct_params->start_time;
  endTime = ct_params->end_time;
  durationInScans = ct_params->duration_in_scans;
  durationInSecs = ct_params->duration_in_secs;
  scanIntervalSecs = (double) durationInSecs / (durationInScans - 1.0);

  // read in simple track params
  
  _iSimple = 0;

  int simple_track_num =
    _t_handle.simples_per_complex[ct_params->complex_track_num][_iSimple];
  _iSimple++;
  
  if(RfRewindSimpleTrack(&_t_handle, simple_track_num,
			 "TitanTrackData::loadNextTrack")) {
    if (_params.debug) {
      fprintf(stderr, "WARNINGS - %s:TitanTrackData::loadNextTrack\n",
	      _progName.c_str());
      fprintf(stderr, "RfRewindSimpleTrack failed, simple_track_num %d,"
	      " file path %s\n", simple_track_num, track_file_path);
    }
    return(-1);
  }
    
  // initialize entry num

  _iEntry = 0;
  
  return (0);

}

//////////////////////////////////////////
// loadNextEntry()
//
// Load up next track entry.
//
// Sets no_more_entries when all entries have been read.
//
// returns 0 on success, -1 on failure 
//
//////////////////////////////////////////

int TitanTrackData::loadNextEntry(int *no_more_entries_p)

{

  *no_more_entries_p = FALSE;

  complex_track_params_t *ct_params = _t_handle.complex_params;
  
  // check that there is data still to be read

  if (_iSimple >= _t_handle.complex_params->n_simple_tracks &&
      _iEntry >= _t_handle.simple_params->duration_in_scans) {
    *no_more_entries_p = TRUE;
    return (0);
  }

  if (_iEntry >= _t_handle.simple_params->duration_in_scans) {
    
    // read in next simple track params
    
    int simple_track_num =
      _t_handle.simples_per_complex[ct_params->complex_track_num][_iSimple];
    _iSimple++;
    _iEntry = 0;
    
    if(RfRewindSimpleTrack(&_t_handle, simple_track_num,
			   "TitanTrackData::loadNextEntry")) {
      return(-1);
    }
    
  } // if (_iEntry >= ...

  // read in next entry

  _iEntry++;
  if (RfReadTrackEntry(&_t_handle, "TitanTrackData::loadNextEntry")) {
    return (-1);
  }

  track_file_entry_t *entry = _t_handle.entry;
  entryHistoryInScans = entry->history_in_scans;
  entryTime = entry->time;

  // read in storm props
  
  if (RfReadStormScan(&_s_handle, entry->scan_num,
		      "TitanTrackData::loadNextEntry")) {
    return (-1);
  }
  if (RfReadStormProps(&_s_handle, entry->storm_num,
		       "TitanTrackData::loadNextEntry")) {
    return(-1);
  }

  // load up grid

  if (_loadGrid()) {
    return -1;
  }

  // set entry properties

  storm_file_global_props_t *gprops = _s_handle.gprops + entry->storm_num;

  centroidX = gprops->proj_area_centroid_x;
  centroidY = gprops->proj_area_centroid_y;
  tops = gprops->top;
  volume = gprops->volume;
  area = gprops->proj_area;
  precipArea = gprops->precip_area;
  dbzMax = gprops->dbz_max;

  precipFlux = gprops->precip_flux;
  // double precipVolM3 = precipFlux * scanIntervalSecs;
  // double precipDepthMm = 0.0;
  // if (precipArea > 0) {
  //   precipDepthMm = (precipVolM3 / precipArea) / 1000.0;
  // }

  track_file_forecast_props_t *fprops = &entry->dval_dt;

  dxDt = fprops->proj_area_centroid_x;
  dyDt = fprops->proj_area_centroid_y;

  storm_file_params_t *sparams = &_s_handle.header->params;

  lowDbzThreshold = sparams->low_dbz_threshold;
  dbzHistInterval = sparams->dbz_hist_interval;
  nDbzIntervals = gprops->n_dbz_intervals;

  // ellipse props

  majorRadius = gprops->proj_area_major_radius;
  minorRadius = gprops->proj_area_minor_radius;

  // check that aspect ratio is not ridiculous, which happens when
  // all points line up, in which case use circle

  if (majorRadius / minorRadius > 100.0) {
    double ellipse_area = gprops->proj_area;
    majorRadius = sqrt(ellipse_area / M_PI);
    minorRadius = majorRadius;
  }

  ellipseOrientation = gprops->proj_area_orientation;

  // load up histogram

  if (areaHist != NULL) {
    ufree(areaHist);
  }
  areaHist = (double *) umalloc(nDbzIntervals * sizeof(double));
  for (int i = 0; i < nDbzIntervals; i++) {
    areaHist[i] = _s_handle.hist[i].percent_area;
  }

  // load up runs

  if (projRuns != NULL) {
    ufree(projRuns);
  }
  nProjRuns = gprops->n_proj_runs;
  int nbytes = nProjRuns * sizeof(storm_file_run_t);
  projRuns = (storm_file_run_t *) umalloc(nbytes);
  memcpy(projRuns, _s_handle.proj_runs, nbytes);

  return (0);

}

//////////////////////////////////////////
// _openFiles()
//
// Open track and storm files.
//
// returns 0 on success, -1 on failure 
//
//////////////////////////////////////////

int TitanTrackData::_openFiles(char *track_file_path)

{

  fprintf(stderr, "Processing track file %s\n", track_file_path);
    
  // open track properties files
  
  if (RfOpenTrackFiles (&_t_handle, "r", track_file_path,
			(char *) NULL,
			"TitanTrackData::_openFiles")) {
    return (-1);
  }
  
  /*
   * read in track properties file header
   */
  
  if (RfReadTrackHeader(&_t_handle, "TitanTrackData::_openFiles")) {
    return (-1);
  }
  if (RfReadSimplesPerComplex(&_t_handle, "TitanTrackData::_openFiles")) {
    return (-1);
  }
  
  // compute storm file path
  
  char storm_file_path[MAX_PATH_LEN];
  path_parts_t track_path_parts;
  uparse_path(track_file_path, &track_path_parts);
  sprintf(storm_file_path, "%s%s",
	  track_path_parts.dir,
	  _t_handle.header->storm_header_file_name);
  ufree_parsed_path(&track_path_parts);
  
  // open storm properties files
  
  if (RfOpenStormFiles (&_s_handle, "r", storm_file_path,
			(char *) NULL,
			"TitanTrackData::_openFiles")) {
    return (-1);
  }
  
  /*
   * read in storm properties file header
   */
  
  if (RfReadStormHeader(&_s_handle, "TitanTrackData::_openFiles")) {
    return (-1);
  }

  _iComplex = 0;

  return (0);

}

//////////////////////////////////////////
// _closeFiles()
//
// Close track and storm files.
//
//////////////////////////////////////////

void TitanTrackData::_closeFiles()
  
{
  
  // close files
  
  RfCloseStormFiles(&_s_handle, "TitanTrackData::_closeFiles");
  RfCloseTrackFiles(&_t_handle, "TitanTrackData::_closeFiles");
  
}

//////////////////////////////////////////
// _loadGrid()
//
// Load up the titan grid.
//
// Returns 0 on success, -1 on failure.
//
//////////////////////////////////////////

int TitanTrackData::_loadGrid()

{
 
  if (_titanGridInitialized) {

    if (memcmp(&_titanGrid, &_s_handle.scan->grid, sizeof(titan_grid_t))) {
      
      cerr << "ERROR - TitanTrackData::_loadGrid" << endl;
      cerr << "  Scan header grid has changed." << endl;
      cerr << endl;

      cerr << "    Initial grid: " << endl;
      TITAN_print_grid(stderr, "    ", &_titanGrid);
      cerr << endl;

      cerr << "    Latest grid: " << endl;
      TITAN_print_grid(stderr, "    ", &_s_handle.scan->grid);
      cerr << endl;

      return -1;

    }

  } else {

    _titanGrid = _s_handle.scan->grid;
    _titanGridInitialized = true;

    _coord.proj_type = _titanGrid.proj_type;
    _coord.proj_origin_lat = _titanGrid.proj_origin_lat;
    _coord.proj_origin_lon = _titanGrid.proj_origin_lon;
    int ksize = _params.smoothing_kernel_size;
    _coord.minx = _titanGrid.minx +
      _titanGrid.dx * (ksize / 2);
    _coord.miny = _titanGrid.miny +
      _titanGrid.dy * (ksize / 2);
    _coord.dx = _titanGrid.dx * ksize;
    _coord.dy = _titanGrid.dy * ksize;
    _coord.nx = _titanGrid.nx / ksize + 1;
    _coord.ny = _titanGrid.ny / ksize + 1;


  }

  return 0;

}
