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
// Clumping.cc
//
// Clumping class
//
// Provides services for run identification and clumping.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 1998
//
///////////////////////////////////////////////////////////////

#include "Clumping.hh"
#include <toolsa/umisc.h>
using namespace std;

//////////////
// constructor
//

Clumping::Clumping(const string &prog_name) :
  _progName(prog_name)

{

  OK = TRUE;

  _rowh = NULL;
  _nRowsAlloc = 0;
  _intervals = NULL;
  _nIntervalsAlloc = 0;
  
  _nIntOrderAlloc = 0;
  _intervalOrder = NULL;

  clumps = NULL;
  nClumps = 0;

}

/////////////
// destructor
//

Clumping::~Clumping()

{

  EG_free_clumps(&_nIntOrderAlloc, &clumps, &_intervalOrder);
  EG_free_rowh(&_nRowsAlloc, &_rowh);
  EG_free_intervals(&_intervals, &_nIntervalsAlloc);

}

///////////////////////////////////////////
// findIntervals()
//
// Find the run intervals in a 2D data grid
//
// returns number of intervals
//

int Clumping::findIntervals(int nx, int ny,
			    unsigned char *data_grid,
                            int byte_threshold)

{

  _allocRowh(ny);
  int nInt = EG_find_intervals(ny, nx, data_grid,
			       &_intervals, &_nIntervalsAlloc,
			       _rowh, byte_threshold);

  return (nInt);
}

// for floats

int Clumping::findIntervals(int nx, int ny,
			    fl32 *data_grid,
                            fl32 threshold)

{

  _allocRowh(ny);
  int nInt = EG_find_intervals_float(ny, nx, data_grid,
                                     &_intervals, &_nIntervalsAlloc,
                                     _rowh, threshold);

  return (nInt);
}

///////////////////////////////////////////
// findIntervals3D()
//
// Find the run intervals in a 3D data grid
//
// returns number of intervals
//

int Clumping::findIntervals3D(int nx, int ny, int nz,
			      unsigned char *data_grid,
			      int byte_threshold)

{

  int nrows_per_plane = ny;
  int nplanes = nz;
  int nrows_per_vol = nrows_per_plane * nplanes;
  int ncols = nx;

  _allocRowh(ny * nz);
  int nInt = EG_find_intervals_3d(nplanes, nrows_per_vol, nrows_per_plane,
				  ncols, data_grid,
				  &_intervals, &_nIntervalsAlloc,
				  _rowh, byte_threshold);

  return (nInt);

}

// for floats

int Clumping::findIntervals3D(int nx, int ny, int nz,
			      fl32 *data_grid,
			      fl32 threshold)

{

  int nrows_per_plane = ny;
  int nplanes = nz;
  int nrows_per_vol = nrows_per_plane * nplanes;
  int ncols = nx;

  _allocRowh(ny * nz);
  int nInt = EG_find_intervals_3d_float(nplanes, 
                                        nrows_per_vol, nrows_per_plane,
                                        ncols, data_grid,
                                        &_intervals, &_nIntervalsAlloc,
                                        _rowh, threshold);

  return (nInt);

}

/////////////////////////////////////////////////////////
// edm2D()
//
// Compute the EDM (Euclidean Distance Measure) of clumps,
// load grid.
//
//

void Clumping::edm2D(int nx, int ny,
		     unsigned char *edm_grid)
  
{
  EG_edm_2d(_rowh, edm_grid, nx, ny, 1);
}

//////////////////////////////////////////
// erode()
//
// Erode clumps, put result in eroded grid
//

void Clumping::erode(int nx, int ny, unsigned char *eroded_grid,
		     int erosion_threshold)

{

  if (erosion_threshold > 0) {
    EG_erode_lesser_2d(_rowh, eroded_grid, nx, ny, erosion_threshold);
  }
    
  for (int value = erosion_threshold - 1; value > 0; value--) {
    EG_erode_lesser_or_equal_2d(_rowh, eroded_grid, nx, ny, value);
  }
  
  EG_erode_bridges_2d(_rowh, eroded_grid, nx, ny);

}

///////////////////////////////////////////////////
// performClumping
//
// returns number of clumps
//

int Clumping::performClumping(int nx, int ny, int nz,
			      unsigned char *data_grid,
			      int min_overlap,
			      int byte_threshold)

{

  int nrows_per_plane = ny;
  int nplanes = nz;
  int nrows_per_vol = nrows_per_plane * nplanes;
  int ncols = nx;
  
  // alloc row headers

  _allocRowh(nrows_per_vol);
  
  // find the intervals in the grid
  
  int nInt = EG_find_intervals_3d(nplanes,
				  nrows_per_vol,
				  nrows_per_plane,
				  ncols,
				  data_grid,
				  &_intervals, &_nIntervalsAlloc,
				  _rowh, byte_threshold);
  
  // allocate space for intervals and clumps
  
  EG_alloc_clumps(nInt, &_nIntOrderAlloc,
		  &clumps, &_intervalOrder);
  
  // set clump ids to NULL_ID
  
  EG_reset_clump_id(_intervals, nInt);
  
  // clump
  
  nClumps = EG_rclump_3d(_rowh, nrows_per_plane, nplanes, TRUE,
			 min_overlap, _intervalOrder, clumps);

  return (nClumps);
  
}
    
///////////////////////////////////////////////////
// performClumping - float grid
//
// returns number of clumps
//

int Clumping::performClumping(int nx, int ny, int nz,
			      fl32 *data_grid,
			      int min_overlap,
			      fl32 threshold)

{

  int nrows_per_plane = ny;
  int nplanes = nz;
  int nrows_per_vol = nrows_per_plane * nplanes;
  int ncols = nx;
  
  // alloc row headers

  _allocRowh(nrows_per_vol);
  
  // find the intervals in the grid
  
  int nInt = EG_find_intervals_3d_float(nplanes,
                                        nrows_per_vol,
                                        nrows_per_plane,
                                        ncols,
                                        data_grid,
                                        &_intervals, &_nIntervalsAlloc,
                                        _rowh, threshold);
  
  // allocate space for intervals and clumps
  
  EG_alloc_clumps(nInt, &_nIntOrderAlloc,
		  &clumps, &_intervalOrder);
  
  // set clump ids to NULL_ID
  
  EG_reset_clump_id(_intervals, nInt);
  
  // clump
  
  nClumps = EG_rclump_3d(_rowh, nrows_per_plane, nplanes, TRUE,
			 min_overlap, _intervalOrder, clumps);

  return (nClumps);
  
}
    
///////////////////////
// _allocRowh()
//
// allocate row headers
//

void Clumping::_allocRowh(int nrows_per_vol)

{
  EG_alloc_rowh(nrows_per_vol, &_nRowsAlloc, &_rowh);
}



