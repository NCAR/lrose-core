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
// ClumpGrid.cc
//
// ClumpGrid class - combines a clump with grid geometry so that
// computations may be done on the clump using that grid geometry.
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2021
//
///////////////////////////////////////////////////////////////

#include <euclid/ClumpGrid.hh>
#include <toolsa/umisc.h>
#include <rapmath/math_macros.h>
#include <cassert>
#include <iostream>
using namespace std;

///////////////
// constructors
//

ClumpGrid::ClumpGrid()
{
  _initDone = FALSE;
}

/////////////
// destructor
//

ClumpGrid::~ClumpGrid()

{
  assert (_initDone);
}

//////////////
// initializer
//

void ClumpGrid::init(const Clump_order *clump_order,
                     int nx, int ny, int nz,
                     double dx, double dy, double dz,
                     double minx, double miny, double minz,
                     bool isLatLon,
		     int start_ix, int start_iy)

{

  // set private grid geom

  grid.setNx(nx);
  grid.setNy(ny);
  grid.setNz(nz);

  grid.setDx(dx);
  grid.setDy(dy);
  grid.setDz(dz);

  grid.setMinx(minx);
  grid.setMiny(miny);
  grid.setMinz(minz);

  grid.setIsLatLon(isLatLon);

  // compute the bounding box for the clump, and create
  // an array of intervals relative to these bounds.

  _shrinkWrap(clump_order);

  // set the various public measures of clump position and bounding size

  nX = _maxIx - _minIx + 1;
  nY = _maxIy - _minIy + 1;

  startIx = start_ix + _minIx;
  startIy = start_iy + _minIy;

  offsetX = startIx * grid.dx();
  offsetY = startIy * grid.dy();
  
  startX = grid.minx() + offsetX;
  startY = grid.miny() + offsetY;

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

void ClumpGrid::_shrinkWrap(const Clump_order *clump)

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

void ClumpGrid::_computeGeometry()

{

  if (grid.isLatLon()) {
    
    // latlon grid

    // we need to multiply by
    // a (111.12 squared) to get km2 for area. The delta_z is
    // set nominally to 1.0, so area and volume will be the same.
    // The volume and area computations are adjusted later for the
    // latitude of the storm.
    
    dX = grid.dx();
    dY = grid.dy() * KM_PER_DEG_AT_EQ;
    _dXAtEquator = grid.dx() * KM_PER_DEG_AT_EQ;

    _dAreaAtEquator =
      (grid.dx() * grid.dy()) *
      (KM_PER_DEG_AT_EQ * KM_PER_DEG_AT_EQ);
    
    _dVolAtEquator = _dAreaAtEquator * grid.dz();
    
    // compute the volumetric y centroid

    double sumy = 0.0, n = 0.0;
    for (int intv = 0; intv < nIntervals; intv++) {
      const Interval &intvl = intervals[intv];
      sumy += (double) intvl.row_in_plane * (double) intvl.len;
      n += (double) intvl.len;
    }
    double vol_centroid_y = (sumy / n) * grid.dy() + grid.miny();
    double latitude_factor = cos(vol_centroid_y * DEG_TO_RAD);

    dVolAtCentroid = _dVolAtEquator * latitude_factor;
    dAreaAtCentroid = _dAreaAtEquator * latitude_factor;
    dAreaEllipse = grid.dx() * grid.dy();
    
    if (grid.nz() <= 1) {
      clumpSize = nPoints * dAreaAtCentroid;
    } else {
      clumpSize = nPoints * dVolAtCentroid;
    }

    kmPerGridUnit =  (_dXAtEquator * latitude_factor + dY) / 2.0;

  } else {
  
    // projection-based (km) grid
    
    dX = grid.dx();
    dY = grid.dy();
    _dAreaFlat = dX * dY;
    _dVolFlat = _dAreaFlat * grid.dz();
    dAreaEllipse = _dAreaFlat;
    
    dAreaAtCentroid = _dAreaFlat;
    dVolAtCentroid = _dVolFlat;
    
    if (grid.nz() <= 1) {
      clumpSize = nPoints * _dAreaFlat;
    } else {
      clumpSize = nPoints * _dVolFlat;
    }
    
    kmPerGridUnit = (dX + dY) / 2.0;

  }
    
}


