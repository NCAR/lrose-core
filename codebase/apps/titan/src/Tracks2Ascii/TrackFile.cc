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
// TrackFile.cc: TrackFile handling
//
// Keeps list of available storm files.
// Finds the storm file for a given time.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 1998
//
//////////////////////////////////////////////////////////

#include "TrackFile.hh"
#include <toolsa/str.h>
using namespace std;

// Constructor

TrackFile::TrackFile (const string &prog_name, const Params &params,
		      Entity *entity,
		      char *file_path) :
  _progName(prog_name), _params(params)

{

  // initialize
  
  OK = TRUE;
  _entity = entity;
  _filePath = STRdup(file_path);
  _nComplex = 0;
  _nSimple = 0;

  // initialize storm file handle
  
  RfInitStormFileHandle(&_s_handle, (char *) _progName.c_str());

  // initialize track file handle
  
  RfInitTrackFileHandle(&_t_handle, (char *) _progName.c_str());

  // open track properties files
  
  if (RfOpenTrackFiles (&_t_handle, "r", _filePath,
 			(char *) NULL, "TrackFile::process")) {
    OK = FALSE;
    return;
  }
  
  /*
   * read in track properties file header
   */
  
  if (RfReadTrackHeader(&_t_handle, "TrackFile::process")) {
    OK = FALSE;
    return;
  }
  if (RfReadSimplesPerComplex(&_t_handle, "TrackFile::process")) {
    OK = FALSE;
    return;
  }
  
  // compute storm file path

  char storm_file_path[MAX_PATH_LEN];
  path_parts_t track_path_parts;
  uparse_path(_filePath, &track_path_parts);
  sprintf(storm_file_path, "%s%s",
	  track_path_parts.dir,
	  _t_handle.header->storm_header_file_name);
  ufree_parsed_path(&track_path_parts);
  
  // open storm properties files
  
  if (RfOpenStormFiles (&_s_handle, "r", storm_file_path,
			NULL, "TrackFile::process")) {
    OK = FALSE;
    return;
  }
  
  // read in storm properties file header
  
  if (RfReadStormHeader(&_s_handle, "TrackFile::process")) {
    OK = FALSE;
    return;
  }

  return;

}

// Destructor

TrackFile::~TrackFile()

{

  // close files

  RfCloseStormFiles(&_s_handle, "TrackFile::process");
  RfCloseTrackFiles(&_t_handle, "TrackFile::process");

  // free handles

  RfFreeStormFileHandle(&_s_handle);
  RfFreeTrackFileHandle(&_t_handle);

  // free strings

  STRfree(_filePath);

}

////////////////////////////////////////
// process()
//
// Process file
//
// returns 0 on success, -1 on failure
////////////////////////////////////////

int TrackFile::process()

{

  // loop through complex tracks

  for (int icomplex = 0;
       icomplex < _t_handle.header->n_complex_tracks; icomplex++) {
    
    // read in the complex track params
    
    int complex_track_num = _t_handle.complex_track_nums[icomplex];
    
    if(RfReadComplexTrackParams(&_t_handle, complex_track_num, TRUE,
				"TrackFile::process") != R_SUCCESS) {
      return(-1);
    }
    
    // check if this track is required for analysis
    
    int track_required = TRUE;

    if (_params.specify_complex_track_num &&
	complex_track_num != _params.complex_track_num) {
      track_required = FALSE;
    }

    if (_t_handle.complex_params->n_simple_tracks == 1 &&
	!_params.use_simple_tracks) {
      track_required = FALSE;
    }

    if (_t_handle.complex_params->n_simple_tracks > 1 && 
	!_params.use_complex_tracks) {
      track_required = FALSE;
    }
    
    if (_t_handle.complex_params->duration_in_secs <
	_params.min_duration) {
      if (_params.debug >= Params::DEBUG_VERBOSE)
	fprintf(stderr, "Track %ld rejected - duration = %ld scans\n",
		(long) _t_handle.complex_params->complex_track_num,
		(long) _t_handle.complex_params->duration_in_scans);
      track_required = FALSE;
    }
    
    if (_params.check_too_close &&
	(_t_handle.complex_params->n_top_missing >
	 _params.max_nscans_too_close)) {
      if (_params.debug >= Params::DEBUG_VERBOSE)
	fprintf(stderr, "Track %d rejected - n_too_close = %d\n",
		(int) _t_handle.complex_params->complex_track_num,
		(int) _t_handle.complex_params->n_top_missing);
      track_required = FALSE;
    }
    
    if (_params.check_too_far &&
	_t_handle.complex_params->n_range_limited >
	_params.max_nscans_too_far) {
      if (_params.debug >= Params::DEBUG_VERBOSE)
	fprintf(stderr, "Track %d rejected - n_too_far = %d\n",
		(int) _t_handle.complex_params->complex_track_num,
		(int) _t_handle.complex_params->n_range_limited);
      track_required = FALSE;
    }
    
    if (_params.check_vol_at_start &&
	_t_handle.complex_params->volume_at_start_of_sampling >
	_params.max_vol_at_start) {
      if (_params.debug >= Params::DEBUG_VERBOSE)
	fprintf(stderr,
		"Track %d rejected - vol_at_start_of_sampling = %d\n",
		(int) _t_handle.complex_params->complex_track_num,
		(int) _t_handle.complex_params->volume_at_start_of_sampling);
      track_required = FALSE;
    }
    
    if (_params.check_vol_at_end &&
	_t_handle.complex_params->volume_at_end_of_sampling >
	_params.max_vol_at_end) {
      if (_params.debug >= Params::DEBUG_VERBOSE)
	fprintf(stderr,
		"Track %d rejected - vol_at_end_of_sampling = %d\n",
		(int) _t_handle.complex_params->complex_track_num,
		(int) _t_handle.complex_params->volume_at_end_of_sampling);
      track_required = FALSE;
    }
    
    if (track_required) {

      if (_t_handle.complex_params->n_simple_tracks == 1) {
	_nSimple++;
      } else {
	_nComplex++;
      }

      // skip now if count only is required

      if (_params.count_only) {
	continue;
      }
	
      _entity->comps_init(&_s_handle, &_t_handle);
	
      // read in simple tracks
      
      for (int isimple = 0;
	   isimple < _t_handle.complex_params->n_simple_tracks; isimple++) {
	
	int simple_track_num =
	  _t_handle.simples_per_complex[complex_track_num][isimple];
	  
	if (_params.specify_simple_track_num &&
	    simple_track_num != _params.simple_track_num) {
	  continue;
	}
	
	// read in simple track params and prepare entries for reading
	
	if(RfRewindSimpleTrack(&_t_handle, simple_track_num,
			       "TrackFile::process")) {
	  return(-1);
	}
	  
	int nentries = _t_handle.simple_params->duration_in_scans;
	  
	// loop through the entries
	  
	for (int ientry = 0; ientry < nentries; ientry++) {
	    
	  // read in track entry
	  
	  if (RfReadTrackEntry(&_t_handle, "TrackFile::process")) {
	    return(-1);
	  }
	    
	  if (RfReadStormScan(&_s_handle, _t_handle.entry->scan_num,
			      "TrackFile::process")) {
	    return(-1);
	  }
	  
	  _entity->compute(&_s_handle, &_t_handle);
	  
	} // ientry
	
      } // isimple
	
      _entity->comps_done(&_s_handle, &_t_handle);
      
    } // if (track_required)
    
  } // icomplex

  return (0);
  
}

