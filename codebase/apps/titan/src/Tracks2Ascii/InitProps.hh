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
// InitProps.h
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 1998
//
///////////////////////////////////////////////////////////////

#ifndef InitProps_H
#define InitProps_H

#include "Entity.hh"
using namespace std;

typedef struct {
  time_t time;
  int scan_num;
  int nparts;
  double range;
  double x, y;
  double area, volume, mass, precip_flux;
} init_props_t;

class InitProps : public Entity {
  
public:

  // constructor

  InitProps(const string &prog_name, const Params &params);

  // destructor
  
  ~InitProps();

  // compute

  int comps_init(storm_file_handle_t *s_handle,
		 track_file_handle_t *t_handle);

  int compute(storm_file_handle_t *s_handle,
	      track_file_handle_t *t_handle);

  int comps_done(storm_file_handle_t *s_handle,
		 track_file_handle_t *t_handle);

protected:
  
private:

  int _nScansAlloc;
  int _nScansFound;
  init_props_t *_iprops;

  void _computeTrend(time_t *time0,
		     double *variable0,
		     double *trend_p);

  void _computeMean(double *variable0,
		    double *mean_p);

};

#endif

