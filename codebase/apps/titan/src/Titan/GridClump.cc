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
// GridClump.cc
//
// GridClump class - wraps a clump with an mdv grid so that
// computations may be done on the clump with the grid geometry.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 1998
//
///////////////////////////////////////////////////////////////

#include "GridClump.hh"
#include <toolsa/umisc.h>
#include <rapmath/math_macros.h>
#include <cassert>
using namespace std;

///////////////
// constructors
//

GridClump::GridClump()
{
  _initDone = FALSE;
}

GridClump::GridClump(Clump_order *clump,
		     const titan_grid_t &titan_grid,
		     int start_ix, int start_iy)

{
  init(clump, titan_grid, start_ix, start_iy);
}

/////////////
// destructor
//

GridClump::~GridClump()

{
  assert (_initDone);
}

//////////////
// initializer
//

void GridClump::init(Clump_order *clump_order,
		     const titan_grid_t &titan_grid,
		     int start_ix, int start_iy)

{

  grid = titan_grid;

  // compute the bounding box for the clump, and create
  // an array of intervals relative to these bounds.

  _shrinkWrap(clump_order);

  // set the various measures of clump position and bounding size

  nX = _maxIx - _minIx + 1;
  nY = _maxIy - _minIy + 1;

  startIx = start_ix + _minIx;
  startIy = start_iy + _minIy;

  offsetX = startIx * grid.dx;
  offsetY = startIy * grid.dy;

  startX = grid.minx + offsetX;
  startY = grid.miny + offsetY;

  // compute the geometry

  _computeGeometry();

  _initDone = TRUE;

}

////////////////////////////////////////////////////////////////
// _shrinkWrap()
//
// Compute the grid indices which bound the clump.
// Create a set of intervals relative to the bounding box.
//

void GridClump::_shrinkWrap(Clump_order *clump)

{

  // determine the spatial limits

  _minIx = 1000000000;
  _minIy = 1000000000;
  _maxIx = 0;
  _maxIy = 0;

  for (int intv = 0; intv < clump->size; intv++) {
    Interval *intvl = clump->ptr[intv];
    _minIx = MIN(intvl->begin, _minIx);
    _maxIx = MAX(intvl->end, _maxIx);
    _minIy = MIN(intvl->row_in_plane, _minIy);
    _maxIy = MAX(intvl->row_in_plane, _maxIy);
  }

  // load up intervals, adjusting for the limits

  nIntervals = clump->size;
  nPoints = clump->pts;
  intervals.reserve(nIntervals);

  for (int intv = 0; intv < clump->size; intv++) {
    Interval *intvl = clump->ptr[intv];
    intervals[intv] = *intvl;
    intervals[intv].row_in_plane -= _minIy;
    intervals[intv].begin -= _minIx;
    intervals[intv].end -= _minIx;
  }

}


////////////////////////////////////////////////////////////////
// _computeGeometry()
//
// Compute the geometry related to the clump.
//

void GridClump::_computeGeometry()

{

  if (grid.proj_type == TITAN_PROJ_FLAT) {
    
    // flat grid
    
    _isLatLon = FALSE;

    _dX = grid.dx;
    _dY = grid.dy;
    _dAreaFlat = _dX * _dY;
    _dVolFlat = _dAreaFlat * grid.dz;
    dAreaEllipse = _dAreaFlat;
    
    dAreaAtCentroid = _dAreaFlat;
    dVolAtCentroid = _dVolFlat;

    if (grid.nz <= 1) {
      stormSize = nPoints * _dAreaFlat;
    } else {
      stormSize = nPoints * _dVolFlat;
    }

    kmPerGridUnit = (_dX + _dY) / 2.0;

  } else {
    
    // latlon grid

    // latlon data has a lat/lon grid, so we need to multiply by
    // a (111.12 squared) to get km2 for area. The delta_z is
    // set nominally to 1.0, so area and volume will be the same.
    // The volume and area computations are adjusted later for the
    // latitude of the storm.
    
    _isLatLon = TRUE;

    _dX = grid.dx;
    _dY = grid.dy * KM_PER_DEG_AT_EQ;
    _dXAtEquator = grid.dx * KM_PER_DEG_AT_EQ;

    _dAreaAtEquator =
      (grid.dx * grid.dy) *
      (KM_PER_DEG_AT_EQ * KM_PER_DEG_AT_EQ);
    
    _dVolAtEquator = _dAreaAtEquator * grid.dz;
    
    // compute the volumetric y centroid

    double sumy = 0.0, n = 0.0;
    for (int intv = 0; intv < nIntervals; intv++) {
      const Interval &intvl = intervals[intv];
      sumy += (double) intvl.row_in_plane * (double) intvl.len;
      n += (double) intvl.len;
    }
    double vol_centroid_y = (sumy / n) * grid.dy + grid.miny;
    double latitude_factor = cos(vol_centroid_y * DEG_TO_RAD);

    dVolAtCentroid = _dVolAtEquator * latitude_factor;
    dAreaAtCentroid = _dAreaAtEquator * latitude_factor;
    dAreaEllipse = grid.dx * grid.dy;
    
    if (grid.nz <= 1) {
      stormSize = nPoints * dAreaAtCentroid;
    } else {
      stormSize = nPoints * dVolAtCentroid;
    }

    kmPerGridUnit =  (_dXAtEquator * latitude_factor + _dY) / 2.0;

  }
  
}


