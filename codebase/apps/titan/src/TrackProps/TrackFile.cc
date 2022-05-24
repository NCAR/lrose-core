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
// January 1998
//
//////////////////////////////////////////////////////////

#include "TrackFile.hh"
#include "Properties.hh"
#include <toolsa/str.h>
using namespace std;

// Constructor

TrackFile::TrackFile (const char *prog_name,
		      const Params &params) :
        _params(params)

{

  // initialize
  
  OK = TRUE;
  _progName = STRdup(prog_name);
  _debug = _params.debug;
  _trackCount = 0;

  // initialize storm file handle
  
  RfInitStormFileHandle(&_s_handle, _progName);

  // initialize track file handle
  
  RfInitTrackFileHandle(&_t_handle, _progName);

  return;

}

// Destructor

TrackFile::~TrackFile()

{

  STRfree(_progName);

  RfFreeStormFileHandle(&_s_handle);
  RfFreeTrackFileHandle(&_t_handle);

}

////////////////////////////////////////
// process()
//
// Process given file path
//
// returns 0 on success, -1 on failure
////////////////////////////////////////

int TrackFile::process(const char *track_file_path)

{

  fprintf(stderr, "Processing file %s\n", track_file_path);

  // open track properties files
  
  if (RfOpenTrackFiles (&_t_handle, "r", track_file_path,
			(char *) NULL, "TrackFile::process")) {
    return (-1);
  }
  
  /*
   * read in track properties file header
   */
  
  if (RfReadTrackHeader(&_t_handle, "TrackFile::process")) {
    return (-1);
  }
  if (RfReadSimplesPerComplex(&_t_handle, "TrackFile::process")) {
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
			(char *) NULL, "TrackFile::process")) {
    return (-1);
  }
  
  /*
   * read in storm properties file header
   */
  
  if (RfReadStormHeader(&_s_handle, "TrackFile::process")) {
    return (-1);
  }

  // loop through complex tracks

  Properties *props = new Properties(_progName, _params, &_s_handle, &_t_handle);
  for (int i = 0; i < _t_handle.header->n_complex_tracks; i++) {
    if (props->compute(_t_handle.complex_track_nums[i]) == 0) {
      props->print(stdout);
      _trackCount++;
    }
  }
  delete (props);

  // close files

  RfCloseStormFiles(&_s_handle, "TrackFile::process");
  RfCloseTrackFiles(&_t_handle, "TrackFile::process");

  return (0);
  
}

