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
//////////////////////////////////////////////////////////
// MdvxRemapInterp.cc
//
// An object of this class is used to hold the data for interpolation
// from one MdvxProj grid mapping to another.
//
// This method uses bilinear interpolation for remapping in 3-D.
//
// Mike Dixon, EOL, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// December 2025
//
//////////////////////////////////////////////////////////
//
// See <Mdv/MdvxRemapInterp.hh> for details.
//
///////////////////////////////////////////////////////////

#include <Mdv/MdvxRemapInterp.hh>
#include <toolsa/pjg.h>
#include <cassert>
using namespace std;

////////////////////////////////////////////////////////////////////////
// Default constructor
//
// You need to call computeOffsets()
// before using the lookup table.

MdvxRemapInterp::MdvxRemapInterp()
  
{
  _lutComputed = false;
}

////////////////////////////////////////////////////////////////////////
// constructor which computes lookup table

MdvxRemapInterp::MdvxRemapInterp(const MdvxProj &proj_source,
                                 const MdvxProj &proj_target)
  
{
  _lutComputed = false;
  computeLut(proj_source, proj_target);
}

/////////////////////////////
// Destructor

MdvxRemapInterp::~MdvxRemapInterp()

{
  return;
}

////////////////////////////////////
// compute lookup table coefficients

void MdvxRemapInterp::computeLut(const MdvxProj &proj_source,
                                 const MdvxProj &proj_target)
  
{

  // check if source coords differ
  
  bool coordsDiffer = false;

  const Mdvx::coord_t &thisSource = _projSource.getCoord();
  const Mdvx::coord_t &inSource = proj_source.getCoord();
  if (memcmp(&thisSource, &inSource, sizeof(Mdvx::coord_t))) {
    // copy in projection
    _projSource = proj_source;
    coordsDiffer = true;
  }

  const Mdvx::coord_t &thisTarget = _projTarget.getCoord();
  const Mdvx::coord_t &inTarget = proj_target.getCoord();
  if (memcmp(&thisTarget, &inTarget, sizeof(Mdvx::coord_t))) {
    // copy in projection
    _projTarget = proj_target;
    coordsDiffer = true;
  }

  if (!coordsDiffer && _lutComputed) {
    return;
  }
  
  // Set the projection longitude conditioning so we can handle
  // cases where the input and output projections normalize the
  // longitudes differently.
  //
  // We condition the target projection based on the source, bcause
  // we perform the mapping from the target back to the source
  // and it is the xy2latlon() call which conditions the longitude

  double refLon = 0.0;
  if (_projSource.getProjType() == Mdvx::PROJ_LATLON) {
    // use the mid longitude of the grid
    const Mdvx::coord_t &coord = _projSource.getCoord();
    refLon = coord.minx + coord.nx * coord.dx / 2.0;
  } else {
    // use the longitude of the origin
    const Mdvx::coord_t &coord = _projSource.getCoord();
    refLon = coord.proj_origin_lon;
  }
  _projTarget.setConditionLon2Ref(true, refLon);

  // compute lookup offsets

  _sourceOffsets.clear();
  _targetOffsets.clear();
  
  // loop through the target

  double yy = inTarget.miny;
  int64_t targetIndex = 0;
  for (int64_t iy = 0; iy < inTarget.ny; iy++, yy += inTarget.dy) {
    
    double xx = inTarget.minx;
    for (int64_t ix = 0; ix < inTarget.nx;
	 ix++, xx += inTarget.dx, targetIndex++) {
      
      // get lat/lon of target point
      
      double lat, lon;
      _projTarget.xy2latlon(xx, yy, lat, lon);

      // get index of source point
      
      int64_t sourceIndex;
      if (_projSource.latlon2arrayIndex(lat, lon, sourceIndex) == 0) {
        // add mapping
	_sourceOffsets.push_back(sourceIndex);
	_targetOffsets.push_back(targetIndex);
      }
      
    } // ix

  } // iy

  _lutComputed = true;

  return;

}

////////////////////////////////////////////////////////
// Compute XY interpolation lookup table

void MdvxRemapInterp::_computeXyLookup()
  
{

  const Mdvx::coord_t &coordSource = _projSource.getCoord();
  const Mdvx::coord_t &coordTarget = _projTarget.getCoord();

  _xyLut.clear();

  size_t targetIndex;
  for (int iy = 0; iy < coordTarget.ny; iy++) {

    double targetY = coordTarget.miny + iy * coordTarget.dy;

    for (int ix = 0; ix < coordTarget.nx; ix++, targetIndex++) {

      double targetX = coordTarget.minx + ix * coordTarget.dx;

      // compute lat,lon of target point
      
      double targetLat, targetLon;
      _projTarget.xy2latlon(targetX, targetY, targetLat, targetLon);

      // compute x,y of source point, in source grid units

      double sourceX, sourceY;
      _projSource.latlon2xy(targetLat, targetLon, sourceX, sourceY);

      // compute indices of target point, in source grid

      double sourceIx, sourceIy;
      _projSource.xy2xyIndex(sourceX, sourceY, sourceIx, sourceIy);

      // check location is valid
      // only interpolate points that are completely within the source grid

      if (sourceIx < 0.0 || sourceIy < 0.0 ||
          sourceIx > coordSource.nx || sourceIy > coordSource.ny) {
        continue;
      }

      // compute indices of surrounding points

      int ix_left = (int) sourceIx;
      int ix_right = ix_left + 1;
      
      int iy_lower = (int) sourceIy;
      int iy_upper = iy_lower + 1;

      // compute interpolation weights

      double wtx_lower = sourceIx - ix_left;
      double wtx_upper = 1.0 - wtx_lower;
      
      double wty_lower = sourceIy - iy_lower;
      double wty_upper = 1.0 - wty_lower;
      
      // fill out lookup details details
      
      xy_lut_t lut;
      
      lut.entry_ul.pt.xx = coordSource.minx + ix_left * coordSource.dx;
      lut.entry_ul.pt.yy = coordSource.miny + iy_upper * coordSource.dy;
      
      lut.entry_ur.pt.xx = lut.entry_ul.pt.xx + coordSource.dx;
      lut.entry_ur.pt.yy = lut.entry_ul.pt.yy;
      
      lut.entry_ll.pt.xx = lut.entry_ul.pt.xx;
      lut.entry_ll.pt.yy = lut.entry_ul.pt.yy - coordSource.dy;
      
      lut.entry_lr.pt.xx = lut.entry_ll.pt.xx + coordSource.dx;
      lut.entry_lr.pt.yy = lut.entry_ll.pt.yy;
      
      // add to vector
      
      _xyLut.push_back(lut);
      
    } // ix

  } // iy

  for (int iz = 0; iz < coordTarget.nz; iz++) {

    z_lut_t lut;
    lut.zz = _vlevelTarget.level[iz];
    
    if (lut.zz <= _vlevelSource.level[0]) {

      // target z is below source lower plane
      
      lut.indexLower = 0;
      lut.indexUpper = 0;
      lut.zLower = _vlevelSource.level[0];
      lut.zUpper = _vlevelSource.level[0];
      lut.wtLower = 0.0;
      lut.wtUpper = 1.0;

      continue;
      
    }
    
    if (lut.zz >= _vlevelSource.level[coordSource.nz - 1]) {

      // target z is above source upper plane
      
      lut.indexLower = coordSource.nz - 1;
      lut.indexUpper = coordSource.nz - 1;
      lut.zLower = _vlevelSource.level[coordSource.nz - 1];
      lut.zUpper = _vlevelSource.level[coordSource.nz - 1];
      lut.wtLower = 1.0;
      lut.wtUpper = 0.0;

      continue;
      
    }
    
    // we are within the source zlevel limits
    
    for (int jz = 1; jz < coordSource.nz; jz++) {
      double zLower = _vlevelSource.level[jz-1];
      double zUpper = _vlevelSource.level[jz];
      if (lut.zz >= zLower && lut.zz <= zUpper) {
        lut.indexLower = jz-1;
        lut.indexUpper = jz;
        lut.zLower = zLower;
        lut.zUpper = zUpper;
        lut.wtLower = (zUpper - lut.zz) / (zUpper - zLower);
        lut.wtUpper = 1.0 - lut.wtLower;
        // break;
      }
    } // jz

    _zLut.push_back(lut);
    
  } // iz

}

////////////////////////////////////////////////////////
// Compute Z interpolation lookup table

void MdvxRemapInterp::_computeZLookup()
  
{

  const Mdvx::coord_t &coordSource = _projSource.getCoord();
  const Mdvx::coord_t &coordTarget = _projTarget.getCoord();

  assert(coordSource.nz <= MDV64_MAX_VLEVELS);
  assert(coordTarget.nz <= MDV64_MAX_VLEVELS);
  
  _zLut.clear();
  
  for (int iz = 0; iz < coordTarget.nz; iz++) {

    z_lut_t lut;
    lut.zz = _vlevelTarget.level[iz];
    
    if (lut.zz <= _vlevelSource.level[0]) {

      // target z is below source lower plane
      
      lut.indexLower = 0;
      lut.indexUpper = 0;
      lut.zLower = _vlevelSource.level[0];
      lut.zUpper = _vlevelSource.level[0];
      lut.wtLower = 0.0;
      lut.wtUpper = 1.0;

      continue;
      
    }
    
    if (lut.zz >= _vlevelSource.level[coordSource.nz - 1]) {

      // target z is above source upper plane
      
      lut.indexLower = coordSource.nz - 1;
      lut.indexUpper = coordSource.nz - 1;
      lut.zLower = _vlevelSource.level[coordSource.nz - 1];
      lut.zUpper = _vlevelSource.level[coordSource.nz - 1];
      lut.wtLower = 1.0;
      lut.wtUpper = 0.0;

      continue;
      
    }
    
    // we are within the source zlevel limits
    
    for (int jz = 1; jz < coordSource.nz; jz++) {
      double zLower = _vlevelSource.level[jz-1];
      double zUpper = _vlevelSource.level[jz];
      if (lut.zz >= zLower && lut.zz <= zUpper) {
        lut.indexLower = jz-1;
        lut.indexUpper = jz;
        lut.zLower = zLower;
        lut.zUpper = zUpper;
        lut.wtLower = (zUpper - lut.zz) / (zUpper - zLower);
        lut.wtUpper = 1.0 - lut.wtLower;
        // break;
      }
    } // jz

    _zLut.push_back(lut);
    
  } // iz

}

