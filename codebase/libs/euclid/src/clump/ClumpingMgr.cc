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
// ClumpingMgr.cc
//
// ClumpingMgr class
//
// Provides services for run identification and clumping.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 1998
//
// Copied over from apps/titan/src/Titan/Clumping class
//
///////////////////////////////////////////////////////////////

#include <euclid/ClumpingMgr.hh>
#include <euclid/ClumpingDualThresh.hh>
#include <euclid/ClumpProps.hh>
#include <toolsa/umisc.h>
#include <iostream>
using namespace std;

//////////////
// constructor
//

ClumpingMgr::ClumpingMgr()

{

  _rowh = NULL;
  _nRowsAlloc = 0;
  _intervals = NULL;
  _nIntervalsAlloc = 0;
  
  _nIntOrderAlloc = 0;
  _intervalOrder = NULL;

  _clumps = NULL;
  _nClumps = 0;

  _dualT = NULL;

}

/////////////
// destructor
//

ClumpingMgr::~ClumpingMgr()

{

  EG_free_clumps(&_nIntOrderAlloc, &_clumps, &_intervalOrder);
  EG_free_rowh(&_nRowsAlloc, &_rowh);
  EG_free_intervals(&_intervals, &_nIntervalsAlloc);

  if (_dualT) {
    delete _dualT;
  }

}

///////////////////////////////////////////
// findIntervals()
//
// Find the run intervals in a 2D data grid
//
// returns number of intervals
//

int ClumpingMgr::findIntervals(int nx, int ny,
                               const unsigned char *data_grid,
                               int byte_threshold)

{
  
  _allocRowh(ny);
  int nInt = EG_find_intervals(ny, nx, data_grid,
			       &_intervals, &_nIntervalsAlloc,
			       _rowh, byte_threshold);

  return nInt;
}

// for floats

int ClumpingMgr::findIntervals(int nx, int ny,
                               const fl32 *data_grid,
                               fl32 threshold)

{

  _allocRowh(ny);
  int nInt = EG_find_intervals_float(ny, nx, data_grid,
                                     &_intervals, &_nIntervalsAlloc,
                                     _rowh, threshold);

  return nInt;
}

///////////////////////////////////////////
// findIntervals3D()
//
// Find the run intervals in a 3D data grid
//
// returns number of intervals
//

int ClumpingMgr::findIntervals3D(int nx, int ny, int nz,
                                 const unsigned char *data_grid,
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

  return nInt;

}

// for floats

int ClumpingMgr::findIntervals3D(int nx, int ny, int nz,
                                 const fl32 *data_grid,
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

  return nInt;

}

/////////////////////////////////////////////////////////
// edm2D()
//
// Compute the EDM (Euclidean Distance Measure) of clumps,
// load grid.
//
//

void ClumpingMgr::edm2D(int nx, int ny,
                        unsigned char *edm_grid)
  
{
  EG_edm_2d(_rowh, edm_grid, nx, ny, 1);
}

//////////////////////////////////////////
// erode()
//
// Erode clumps, put result in eroded grid
//

void ClumpingMgr::erode(int nx, int ny, unsigned char *eroded_grid,
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
// performClumpingMgr
//
// returns number of clumps
//

int ClumpingMgr::performClumping(int nx, int ny, int nz,
                                 const unsigned char *data_grid,
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
		  &_clumps, &_intervalOrder);
  
  // set clump ids to NULL_ID
  
  EG_reset_clump_id(_intervals, nInt);
  
  // clump
  
  _nClumps = EG_rclump_3d(_rowh, nrows_per_plane, nplanes, TRUE,
                          min_overlap, _intervalOrder, _clumps);

  return _nClumps;
  
}
    
///////////////////////////////////////////////////
// performClumping - float grid
//
// returns number of clumps
//

int ClumpingMgr::performClumping(int nx, int ny, int nz,
                                 const fl32 *data_grid,
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
		  &_clumps, &_intervalOrder);
  
  // set clump ids to NULL_ID
  
  EG_reset_clump_id(_intervals, nInt);
  
  // clump
  
  _nClumps = EG_rclump_3d(_rowh, nrows_per_plane, nplanes, TRUE,
                          min_overlap, _intervalOrder, _clumps);

  return _nClumps;
  
}
    
///////////////////////
// _allocRowh()
//
// allocate row headers
//

void ClumpingMgr::_allocRowh(int nrows_per_vol)

{
  EG_alloc_rowh(nrows_per_vol, &_nRowsAlloc, &_rowh);
}

/////////////////////////////////////
// set whether to use dual thresholds

void ClumpingMgr::setUseDualThresholds(double secondary_threshold,
                                       double min_fraction_all_parts,
                                       double min_fraction_each_part,
                                       double min_area_each_part,
                                       double min_clump_volume,
                                       double max_clump_volume,
                                       bool debug /* = false */)

{

  if (_dualT == NULL) {
    _dualT = new ClumpingDualThresh;
  }

  _dualT->setSecondaryThreshold(secondary_threshold);
  _dualT->setMinFractionAllParts(min_fraction_all_parts);
  _dualT->setMinFractionEachPart(min_fraction_each_part);
  _dualT->setMinAreaEachPart(min_area_each_part);
  _dualT->setMinClumpVolume(min_clump_volume);
  _dualT->setMaxClumpVolume(max_clump_volume);
  _dualT->setDebug(debug);

}

// Perform clumping specifying the input geom and input data array.
// If needed, call setUseDualThresholds().
// Return: fills out clumps vector.

void ClumpingMgr::loadClumpVector(PjgGridGeom &inputGeom, 
                                  const fl32 *inputData,
                                  double primary_threshold,
                                  int min_grid_overlap,
                                  vector<ClumpProps> &clumpVec)

{

  clumpVec.clear();

  // set input geom
  
  _gridGeom = inputGeom;
  
  // perform clumping

  _nClumps = performClumping(_gridGeom.nx(), _gridGeom.ny(), _gridGeom.nz(),
                             inputData, min_grid_overlap, primary_threshold);
  
  // load up clump vector
  
  cerr << "AAAAAAAAAAAAAAAAAAAA _nClumps: " << _nClumps << endl;

  if (_dualT != NULL) {

    // use dual thresholds
    
    _dualT->setInputData(inputGeom, inputData);
    
    const Clump_order *clump = getClumps();
    for (int iclump = 0; iclump < _nClumps; iclump++, clump++) {
      ClumpProps cprops;
      cprops.init(clump, _gridGeom, 0, 0);
      int n_sub_clumps = _dualT->compute(cprops);
      cerr << "BBBBBB iclump, n_sub_clumps: " << iclump << ", " << n_sub_clumps << endl;
      if (n_sub_clumps == 1) {
        clumpVec.push_back(cprops);
      } else {
	for (int i = 0; i < n_sub_clumps; i++) {
	  clumpVec.push_back(_dualT->subClumps()[i]);
	}
      }
    } // iclump

  } else {

    // only use primary threshold

    const Clump_order *clump = getClumps();
    for (int iclump = 0; iclump < _nClumps; iclump++, clump++) {
      ClumpProps cprops;
      cprops.init(clump, _gridGeom, 0, 0);
      clumpVec.push_back(cprops);
    } // iclump

  } // if (_dualT != NULL)

  cerr << "AAAAAAAAAAAAAAAAAAAA clumpVec.size(): " << clumpVec.size() << endl;

}

/////////////////////////////////////////////
// get debug grids from using dual threshold

const fl32 *ClumpingMgr::getDualThreshCompFileGrid() const {
  if (_dualT == NULL) {
    return NULL;
  }
  return _dualT->getCompFileGrid();
}

const ui08 *ClumpingMgr::getDualThreshAllFileGrid() const {
  if (_dualT == NULL) {
    return NULL;
  }
  return _dualT->getAllFileGrid();
}

const ui08 *ClumpingMgr::getDualThreshValidFileGrid() const {
  if (_dualT == NULL) {
    return NULL;
  }
  return _dualT->getValidFileGrid();
}

const ui08 *ClumpingMgr::getDualThreshGrownFileGrid() const {
  if (_dualT == NULL) {
    return NULL;
  }
  return _dualT->getGrownFileGrid();
}

