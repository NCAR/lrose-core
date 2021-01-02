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
// TrackEntry.h
//
// Track Entry object - easy access to track entries in order
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 1997
//
//////////////////////////////////////////////////////////

#ifndef TrackEntry_h
#define TrackEntry_h

#include <toolsa/umisc.h>
#include <titan/storm.h>
#include <titan/track.h>
#include <titan/partial_track.h>

class TrackEntry {
  
public:
  
  // constructor
       
  TrackEntry (char *prog_name,
	      int debug,
	      int complex_track_num,
	      track_file_handle_t *t_handle,
	      rf_partial_track_t *ptrack);
  
  // get next entry
  
  track_file_entry_t *next();

  // destructor
  
  ~TrackEntry();
  
  int OK;

protected:
  
private:

  int _debug;

  char *_progName;

  int _complexTrackNum;
  int _simpleIndex;
  int _entryIndex;

  track_file_handle_t *_t_handle;
  rf_partial_track_t *_pTrack;

};

#endif
