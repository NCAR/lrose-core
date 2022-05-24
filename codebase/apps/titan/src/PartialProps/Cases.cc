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
// Cases.cc : Case handling
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 1997
//
//////////////////////////////////////////////////////////

#include "Cases.hh"
#include "Properties.hh"
#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>
#include <cassert>
using namespace std;

//////////////
// Constructor

Cases::Cases (const char *prog_name,
	      bool debug,
	      bool verbose,
	      const char *case_file_path,
	      double altitude_threshold,
	      const char *output_dir)
  
{

  // initialize

  _progName = STRdup(prog_name);
  _debug = debug;
  _verbose = verbose;
  _caseFilePath = STRdup(case_file_path);
  _altitudeThreshold = altitude_threshold;
  _outputDir = STRdup(output_dir);

  isOK = TRUE;

  // read in case file

  _cases.setDebug(_verbose);

  if (_cases.readCaseFile(_caseFilePath)) {
    fprintf(stderr, "ERROR - %s:Cases::Cases\n", _progName);
    fprintf(stderr, "Cannot read in case track file\n");
    isOK = FALSE;
    return;
  }
  reset();

  if (verbose) {
    _cases.print(cerr);
  }
  
  // init indices for storm and track files

  if (_init_indices()) {
    isOK = FALSE;
    return;
  }

}

/////////////
// Destructor

Cases::~Cases()

{

  // free up memory

  STRfree(_progName);
  STRfree(_caseFilePath);
  STRfree(_outputDir);

}

////////////////////////////////////////
// reset()
//
// Set back to case 0
//
////////////////////////////////////////

void Cases::reset()
  
{
  _caseIndex = 0;
}

////////////////////////////////////////
// next()
//
// get next case
//
// returns 0 on success, -1 on failure
////////////////////////////////////////

int Cases::next(SeedCaseTracks::CaseTrack *this_case)
  
{

  if (_caseIndex >= _cases.getNCases()) {
    return (-1);
  } else {
    _cases.getCase(_caseIndex, *this_case);
    _caseIndex++;
    return (0);
  }

}

////////////////////////////////////////
// process()
//
// process this case
//
// returns 0 on success, -1 on failure
////////////////////////////////////////

int Cases::process(SeedCaseTracks::CaseTrack *this_case,
		   char *storm_file_path)
     
{

  char track_file_path[MAX_PATH_LEN];
  char *ext;

  PMU_auto_register("In Cases::process");

  // compute track header file path
  
  strcpy(track_file_path, storm_file_path);
  ext = strstr(track_file_path, STORM_HEADER_FILE_EXT);
  assert (ext != NULL);
  strcpy(ext, TRACK_HEADER_FILE_EXT);

  if (_verbose) {
    fprintf(stderr, "----> Processing case %d, time %s\n", 
	    this_case->num, utimstr(this_case->ref_time));
    fprintf(stderr, "       Storm file path: %s\n", storm_file_path);
    fprintf(stderr, "       Track file path: %s\n", track_file_path);
  }
  
  /*
   * open storm and track files
   */
  
  if (_open_storm_and_track_files(storm_file_path,
				  track_file_path)) {
    return (-1);
  }

  // tag the tree containing the partial track
  
  if (RfFindPartialTrack(&_pTrack,
			 this_case->ref_time,
			 this_case->ref_minus_start,
			 this_case->end_minus_ref,
			 this_case->complex_track_num,
			 this_case->simple_track_num)) {
    RfCloseStormFiles(&_s_handle, "process_cases");
    RfCloseTrackFiles(&_t_handle, "process_cases");
    return (-1);
  }

  if (_debug) {
    cerr << "============= Got partial track =============" << endl;
    cerr << "  Ref time: " << DateTime::strm(this_case->ref_time) << endl;
    RfPrintPartialTrack(stderr, "PARTIAL TRACK", "  ", &_pTrack);
    cerr << "=============================================" << endl;
  }

  // compute properties

  Properties props(_progName, _verbose, this_case, _altitudeThreshold,
		   _outputDir, &_s_handle, &_t_handle, &_pTrack);

  if (!props.isOK) {
    return (-1);
  }
  
  if (props.compute()) {
    return (-1);
  }

  // print properties

  props.print();

  /*
   * close files
   */

  RfCloseStormFiles(&_s_handle, "Cases::process");
  RfCloseTrackFiles(&_t_handle, "Cases::process");
  
  return (0);

}

/////////////////////////////////////////////////
// _init_indices()
//
// initializes the storm and track file indices
//
// Returns 0 on success, -1 on failure
//
////////////////////////////////////////////////

int Cases::_init_indices()

{

  // initialize storm file handle
  
  RfInitStormFileHandle(&_s_handle, _progName);
  
  // initialize track file handle
  
  RfInitTrackFileHandle(&_t_handle, _progName);

  // initialize partial track handle

  RfInitPartialTrack(&_pTrack,
		     &_t_handle, _progName, _verbose);

  // set grid type to illegal value

  _gridType = -999;

  return (0);
      
}

//////////////////////////////////////////////
// _open_storm_and_track_files()
//
// Opens the files, reads in the headers
//
// Returns 0 on success, -1 on failure
//
//////////////////////////////////////////////

int Cases::_open_storm_and_track_files(char *storm_file_path,
				       char *track_file_path)

{

  int grid_type;

  // open track properties files
  
  if (RfOpenTrackFiles (&_t_handle, "r",
			track_file_path,
			(char *) NULL,
			"Cases::_open_storm_and_track_files")) {
    return(-1);
  }

  // read in track file header

  if (RfReadTrackHeader(&_t_handle, "_open_storm_and_track_files")) {
    RfCloseTrackFiles(&_t_handle, "Cases::_open_storm_and_track_files");
    return(-1);
  }

  // open storm file
  
  if (RfOpenStormFiles (&_s_handle, "r",
			storm_file_path,
			(char *) NULL,
			"Case::_open_storm_and_track_files")) {
    RfCloseTrackFiles(&_t_handle, "Cases::_open_storm_and_track_files");
    return (-1);
  }

  // read in storm properties file header

  if (RfReadStormHeader(&_s_handle, "Cases::_open_storm_and_track_files")) {
    RfCloseStormFiles(&_s_handle, "Cases::_open_storm_and_track_files");
    RfCloseTrackFiles(&_t_handle, "Cases::_open_storm_and_track_files");
    return(-1);
  }

  // read in first scan

  if (RfReadStormScan(&_s_handle, 0, "Cases::_open_storm_and_track_files")) {
    RfCloseStormFiles(&_s_handle, "Cases::_open_storm_and_track_files");
    RfCloseTrackFiles(&_t_handle, "Cases::_open_storm_and_track_files");
    return(-1);
  }

  // set the grid type and check for consistency

  grid_type = _s_handle.scan->grid.proj_type;

  if (_gridType == -999) {
    
    _gridType = grid_type;
    
  } else {

    if (grid_type != _gridType) {
      fprintf(stderr, "ERROR - %s:Cases::_open_storm_and_track_files.\n",
	      _progName);
      fprintf(stderr, "Mixed data grid types.\n");
      fprintf(stderr, "File %s\n", _s_handle.header_file_path);
      return(-1);
    }

  }

  return (0);

}
