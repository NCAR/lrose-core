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
// TrStorm.hh
//
// TrStorm class - used for storing storm status in tracking
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 1999
//
///////////////////////////////////////////////////////////////

#ifndef TrStorm_HH
#define TrStorm_HH

#include "TrTrack.hh"
#include <titan/TitanStormFile.hh>
#include <toolsa/MemBuf.hh>
#include <vector>
using namespace std;

////////////////////////////////
// TrStorm

class TrStorm {
  
public:

  // struct definitions

  typedef struct {
    double overlap;
    int storm_num;
    int complex_track_num;
    int n_simple_tracks;
  } track_match_t;

  typedef struct {
  
    int sum_simple_tracks;
    int consolidated_complex_num;
    int match;
    
    int n_match;
    int n_proj_runs;
    
    bool starts;
    bool stops;
    bool continues;
    bool has_split;
    bool has_merger;
    bool checked;
    
    double sum_overlap;
    
  } status_t;

  // constructor

  TrStorm();

  // destructor
  
  virtual ~TrStorm();

  // data members

  status_t status;
  TrTrack track;
  TrTrack::bounding_box_t box_for_overlap;
  TrTrack::props_t current;
  track_match_t *match_array;
  storm_file_run_t *proj_runs;

  // member functions

  // alloc_match_array

  void alloc_match_array(int n_match);

  // alloc_storm_runs

  void alloc_proj_runs(int n_proj_runs);

  // load storm props
  // Returns 0 on success, -1 on failure. Failure indicates that
  // storm runs are not available
  
  int load_props(int storm_num,
		 const date_time_t &scan_time,
		 TitanStormFile &sfile);

  // update times for an existing track

  void update_times(date_time_t *dtime,
		    vector<track_utime_t> &track_utime,
		    int n_forecasts);
     
protected:
  
private:

  MemBuf _match_buf;
  MemBuf _proj_runs_buf;

};

#endif
