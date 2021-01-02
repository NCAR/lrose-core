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
// TitanTrackData.h: TitanTrackData handling
//
// Serves out track and track_entry data.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1998
//
//////////////////////////////////////////////////////////

#ifndef TitanTrackData_hh
#define TitanTrackData_hh

#include "TrackData.hh"
#include <titan/track.h>
#include <toolsa/mem.h>
using namespace std;

class TitanTrackData : public TrackData {
  
public:

  // constructor
       
  TitanTrackData (const string &prog_name, const Params &params,
		  DsInputPath &input);
  
  // Destructor
  
  virtual ~TitanTrackData();
  
  // load up data for next track

  int loadNextTrack(int *no_more_tracks_p);

  // load up data for next entry in track

  int loadNextEntry(int *no_more_entries_p);

protected:
  
private:

  storm_file_handle_t _s_handle;
  track_file_handle_t _t_handle;

  titan_grid_t _titanGrid;
  bool _titanGridInitialized;

  int _iComplex;
  int _iSimple;
  int _iEntry;
  
  int _openFiles(char *track_file_path);
  void _closeFiles();
  int _loadGrid();
  

};

#endif
