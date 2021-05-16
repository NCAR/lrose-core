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
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 1998
//
///////////////////////////////////////////////////////////////

#ifndef TrackEntry_H
#define TrackEntry_H

#include "Entity.hh"
#include <Mdv/MdvxProj.hh>
using namespace std;

class TrackEntry : public Entity {
  
public:

  // constructor

  TrackEntry(const string &prog_name, const Params &params);

  // destructor
  
  ~TrackEntry();

  // compute

  int comps_init(storm_file_handle_t *s_handle,
		 track_file_handle_t *t_handle);

  int compute(storm_file_handle_t *s_handle,
	      track_file_handle_t *t_handle);

  int comps_done(storm_file_handle_t *s_handle,
		 track_file_handle_t *t_handle);

protected:

  bool _labelsPrinted;
  
  void _printLabels(storm_file_handle_t *s_handle);

  void _compute_percent_gt_refl(storm_file_handle_t *s_handle,
				storm_file_global_props_t *gprops,
				double dbz_threshold,
				double &percent_vol_gt,
				double &percent_area_gt);

  void _computeAll(storm_file_handle_t *s_handle,
		  track_file_handle_t *t_handle,
		   date_time_t &entry_time);
  
  void _compute_speed_and_dirn(bool isLatLon,
                               double storm_x,
                               double storm_y,
                               double dx_dt,
                               double dy_dt,
                               double &speed,
                               double &dirn,
                               double &UU,
                               double &VV);
  
  void _computeHist(storm_file_handle_t *s_handle,
		    track_file_handle_t *t_handle,
		    date_time_t &entry_time);
  

private:

};

#endif

