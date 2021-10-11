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
// ClumpProps.cc
//
// ClumpProps class - combines a clump with grid geometry so that
// computations may be done on the clump using that grid geometry.
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2021
//
///////////////////////////////////////////////////////////////

#include <euclid/ClumpProps.hh>
#include <toolsa/umisc.h>
#include <rapmath/math_macros.h>
#include <cassert>
#include <iostream>
#include <algorithm>
using namespace std;

///////////////
// constructors
//

ClumpProps::ClumpProps()
{
  _initDone = FALSE;
}

/////////////
// destructor
//

ClumpProps::~ClumpProps()

{
  // assert (_initDone);
}

//////////////
// initializer
//

void ClumpProps::init(const Clump_order *clump,
                      const PjgGridGeom &gridGeom,
                      int startIx, int startIy)

{

  _clump = clump;
  _nIntervals = _clump->size;

  if(clump->size > 0) {
    Interval *intvl = clump->ptr[0];
    _id = intvl->id;
  } else {
    _initDone = FALSE;
    return;
  }

  _gridGeom = gridGeom;
  _startIxGlobal = startIx;
  _startIyGlobal = startIy;
  
  // compute the bounding box for the clump, and create
  // an array of intervals relative to these bounds.

  _shrinkWrap();

  // compute projected area grid

  _compute2DGrid();

  // compute the properties

  _computeProps();

  _initDone = TRUE;

}

////////////////////////////////////////////////////////////////
// _shrinkWrap()
//
// Compute the grid indices which bound the clump.
// Create a set of intervals relative to the bounding box.
//

void ClumpProps::_shrinkWrap()

{

  // determine the spatial limits

  _minIxGlobal = 1000000000;
  _minIyGlobal = 1000000000;
  _maxIxGlobal = 0;
  _maxIyGlobal = 0;

  for (int intv = 0; intv < _clump->size; intv++) {
    const Interval *intvl = _clump->ptr[intv];
    _minIxGlobal = MIN(intvl->begin, _minIxGlobal);
    _maxIxGlobal = MAX(intvl->end, _maxIxGlobal);
    _minIyGlobal = MIN(intvl->row_in_plane, _minIyGlobal);
    _maxIyGlobal = MAX(intvl->row_in_plane, _maxIyGlobal);
  }

  // load up intervals, adjusting for the limits

  _intvLocal.reserve(_clump->size);
  
  for (int intv = 0; intv < _clump->size; intv++) {
    const Interval *intvl = _clump->ptr[intv];
    _intvLocal[intv] = *intvl;
    _intvLocal[intv].row_in_plane -= _minIyGlobal;
    _intvLocal[intv].begin -= _minIxGlobal;
    _intvLocal[intv].end -= _minIxGlobal;
  }

  // set the various public measures of clump position and bounding size

  _nXLocal = _maxIxGlobal - _minIxGlobal + 1;
  _nYLocal = _maxIyGlobal - _minIyGlobal + 1;

  _startIxLocal = _startIxGlobal + _minIxGlobal;
  _startIyLocal = _startIyGlobal + _minIyGlobal;
  
  _offsetX = _startIxLocal * _gridGeom.dx();
  _offsetY = _startIyLocal * _gridGeom.dy();
  
  _startXLocal = _gridGeom.minx() + _offsetX;
  _startYLocal = _gridGeom.miny() + _offsetY;

}


////////////////////////////////////////////////////////////////
// _compute2DGrid()
//
// Compute the projected area grid
//

void ClumpProps::_compute2DGrid()
  
{

  // initialize

  _grid2DArray.alloc(_nYLocal, _nXLocal);
  memset(_grid2DArray.dat1D(), 0, _nYLocal * _nXLocal);
  _grid2DVals = _grid2DArray.dat2D();

  // populate projected area grid - local

  for (size_t intv = 0; intv < _intvLocal.size(); intv++) {
    const Interval &intvl = _intvLocal[intv];
    int iy = intvl.row_in_plane;
    for (int ix = intvl.begin; ix <= intvl.end; ix++) {
      _grid2DVals[iy][ix] = 1;
    }
  }

  _nPoints2D = 0;
  for (int iy = 0; iy < _nYLocal; iy++) {
    for (int ix = 0; ix < _nXLocal; iy++) {
      if (_grid2DVals[iy][ix] == 1) {
        _nPoints2D++;
      }
    } 
  } 

  // compute centroid relative to global grid

  double sumx = 0.0, sumy = 0.0, sumz = 0.0, nn = 0.0;
  for (int intv = 0; intv < _clump->size; intv++) {
    const Interval *intvl = _clump->ptr[intv];
    int iz = intvl->plane;
    int iy = intvl->row_in_plane;
    double zz = _gridGeom.zKm(iz);
    double yy = _gridGeom.miny() + iy * _gridGeom.dy();
    for (int ix = intvl->begin; ix <= intvl->end; ix++) {
      double xx = _gridGeom.minx() + iy * _gridGeom.dx();
      sumx += xx;
      sumy += yy;
      sumz += zz;
      nn++;
    }
  }

  _centroidX = sumx / nn;
  _centroidY = sumy / nn;
  _centroidZ = sumz / nn;

  // determine the scale in km
  // based on whether this is a (lat,lon) or (km,km) grid
  
  if (_gridGeom.isLatLon()) {
    
    // latlon grid

    // we need to multiply by
    // a (111.12 squared) to get km2 for area. The delta_z is
    // set nominally to 1.0, so area and volume will be the same.
    // The volume and area computations are adjusted later for the
    // latitude of the storm.
    
    _dXKmAtCentroid =
      _gridGeom.dx() * KM_PER_DEG_AT_EQ * cos(_centroidY * DEG_TO_RAD);
    _dYKmAtCentroid = _gridGeom.dy() * KM_PER_DEG_AT_EQ;

  } else {
  
    // projection-based (km) grid
    
    _dXKmAtCentroid = _gridGeom.dx();
    _dYKmAtCentroid = _gridGeom.dy();

  }

  // compute delta area and vol
  
  _dAreaAtCentroid = _dXKmAtCentroid * _dYKmAtCentroid;

  _dVolAtCentroid.clear();
  for (size_t iz = 0; iz < nZ(); iz++) {
    _dVolAtCentroid.push_back(_dAreaAtCentroid * _gridGeom.zKm(iz));
  }

}

////////////////////////////////////////////////////////////////
// Compute the properties of this clump.
//

void ClumpProps::_computeProps()

{

  // projected area
  
  _projAreaKm2 = 0.0;
  _nPoints2D = 0;
  for (int iy = 0; iy < _nYLocal; iy++) {
    for (int ix = 0; ix < _nXLocal; ix++) {
      if (_grid2DVals[iy][ix]) {
        _nPoints2D++;
        _projAreaKm2 += _dAreaAtCentroid;
      }
    } // ix
  } // iy

  // volume
  
  _nPoints3D = _clump->pts;
  _volumeKm3 = 0.0;
  for (int intv = 0; intv < _clump->size; intv++) {
    const Interval *intvl = _clump->ptr[intv];
    _volumeKm3 += intvl->len * _dVolAtCentroid[intvl->plane];
  }

  // clump size - generic
  // area for 2D, volume for 3D
  
  if (_gridGeom.nz() <= 1) {
    _clumpSize = _projAreaKm2;
  } else {
    _clumpSize = _volumeKm3;
  }

  // vertical extent
  
  _minZKm = 9999.0;
  _maxZKm = -9999.0;

  for (int intv = 0; intv < _clump->size; intv++) {
    
    const Interval *intvl = _clump->ptr[intv];
    
    int iz = intvl->plane;
    double zKm = _gridGeom.zKm(iz);
    _minZKm = min(zKm, _minZKm);
    _maxZKm = max(zKm, _maxZKm);

  }

  _vertExtentKm = _maxZKm - _minZKm;

}


