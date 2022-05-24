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
// TrackEntry.cc
//
// Track entry access
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// January 1998
//
//////////////////////////////////////////////////////////

#include "TrackEntry.hh"
#include <toolsa/str.h>
using namespace std;

//////////////
// Constructor

TrackEntry::TrackEntry (const char *prog_name,
			int debug,
			int complex_track_num,
			track_file_handle_t *t_handle,
			rf_partial_track_t *ptrack)
  
{
  
  // initialize basic

  _progName = STRdup(prog_name);
  _debug = debug;
  _complexTrackNum = complex_track_num;
  _t_handle = t_handle;
  _pTrack = ptrack;
  OK = TRUE;

  // read in complex params
  
  if(RfReadComplexTrackParams(_t_handle, _complexTrackNum, TRUE,
			      "TrackEntry::TrackEntry") != R_SUCCESS) {
    OK = FALSE;
    return;
  }

  // initialize indices

  _simpleIndex = 0;
  _entryIndex = 0;
  
}

/////////////
// destructor

TrackEntry::~TrackEntry ()
  
{

  // free up memory

  STRfree(_progName);

}

//////////////////////////////////////////
// next()
//
// load up next valid entry
//
// Returns pointer to entry on success, NULL on failure
//
///////////////////////////////////////////

track_file_entry_t *TrackEntry::next()

{

  /*
   * simple tracks in this complex track
   */
  
  for (int isimple = _simpleIndex;
       isimple < _t_handle->complex_params->n_simple_tracks; isimple++) {

    // if start of simple track, position track at start
    
    if (_entryIndex == 0) {

      int simple_track_num =
	_t_handle->simples_per_complex[_complexTrackNum][isimple];
      
      if(RfRewindSimpleTrack(_t_handle, simple_track_num,
			     "TrackEntry::next") != R_SUCCESS) {
	return (NULL);
      }

    }
    
    /*
     * loop through the track entries
     */
    
    for (int ientry = _entryIndex;
	 ientry < _t_handle->simple_params->duration_in_scans; ientry++) {
      
      if (RfReadTrackEntry(_t_handle, "TrackEntry::next") != R_SUCCESS) {
	return (NULL);
      }
      
      if (RfEntryInPartial(_pTrack, _t_handle->entry)) {
	
	_simpleIndex = isimple;
	_entryIndex = ientry + 1;
	return (_t_handle->entry);

      }
      
    } /* ientry */

    _entryIndex = 0;
    
  } /* isimple */
  
  return (NULL);
  
}

