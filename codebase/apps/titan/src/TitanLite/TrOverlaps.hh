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
// TrOverlaps.hh
//
// TrOverlaps class - overlaps for tracking
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 1999
//
///////////////////////////////////////////////////////////////

#ifndef TrOverlaps_HH
#define TrOverlaps_HH

#include <vector>
#include "Worker.hh"
#include "TrStorm.hh"
using namespace std;

////////////////////////////////
// TrOverlaps

class TrOverlaps : public Worker {
  
public:

  // constructor

  TrOverlaps(const string &prog_name, const Params &params);

  // destructor
  
  virtual ~TrOverlaps();

  ////////////////
  // find overlaps
  
  void find(const TitanStormFile &sfile,
	    vector<TrStorm*> &storms1,
	    vector<TrStorm*> &storms2,
	    double d_hours);

protected:
  
private:

  int _n_tmp_grid_alloc;
  ui08 *_tmp_grid_array;

  int _n_overlap_grid_alloc;
  ui08 *_overlap_grid_array;

  // functions

  int compute_overlap(ui08 *overlap_grid, int npoints_grid);
  
  void init_tmp_grid(int nbytes);
  
  void init_overlap_grid(int nbytes);
  
  int load_current_poly(const TitanStormFile &sfile,
			TrStorm &storm,
			TrTrack::bounding_box_t *both,
			int nx, int ny,
			ui08 *data_grid,
			int val);
  
  int load_current_runs(TrStorm &storm,
			TrTrack::bounding_box_t *both,
			int nx,
			ui08 *data_grid,
			int val);
  
  int load_forecast_poly(const TitanStormFile &sfile,
			 TrStorm &storm,
			 TrTrack::bounding_box_t *both,
			 int nx, int ny,
			 ui08 *data_grid,
			 int val);
  
  int load_forecast_runs(TrStorm &storm,
			 TrTrack::bounding_box_t *both,
			 int nx, int ny,
			 const titan_grid_t &grid,
			 ui08 *overlap_grid,
			 ui08 *tmp_grid);
  
  void load_overlaps(const TitanStormFile &sfile,
		     TrStorm &storm1,
		     TrStorm &storm2,
		     int istorm, int jstorm,
		     TrTrack::bounding_box_t *box1,
		     TrTrack::bounding_box_t *box2);
  
  void print_overlap(ui08 *overlap_grid, int nx, int ny);
  
  void add_overlap(TrStorm &storms1,
		   TrStorm &storms2,
		   int istorm, int jstorm,
		   double area_overlap);

};

#endif



