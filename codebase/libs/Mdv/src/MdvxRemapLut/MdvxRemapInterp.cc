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
#include <toolsa/mem.h>
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
  _coordSource = &_projSource.getCoord();
  _coordTarget = &_projTarget.getCoord();
}

////////////////////////////////////////////////////////////////////////
// constructor which computes lookup table

MdvxRemapInterp::MdvxRemapInterp(const MdvxProj &proj_source,
                                 const MdvxProj &proj_target)
  
{
  _lutComputed = false;
  _coordSource = &_projSource.getCoord();
  _coordTarget = &_projTarget.getCoord();
  _computeLut(proj_source, proj_target);
}

/////////////////////////////
// Destructor

MdvxRemapInterp::~MdvxRemapInterp()

{
  return;
}

///////////////////////////////////////////////////////////
// interpolate a field, remapping to target projection
//
// Creates a field, returns pointer to that field.
// Memory ownership passes back to caller.
// The returned field must be freed by the caller.
//
// Side effect - the source field is converted to FLOAT32
// and uncompressed

MdvxField *MdvxRemapInterp::interpField(MdvxField &sourceFld)
  
{
  
  // uncompress source and convert source to FLOAT32
  
  sourceFld.convertType(Mdvx::ENCODING_FLOAT32,
                        Mdvx::COMPRESSION_NONE);
  
  // create source projection object

  const Mdvx::field_header_t &fhdrSource = sourceFld.getFieldHeader();
  const Mdvx::vlevel_header_t &vhdrSource = sourceFld.getVlevelHeader();
  MdvxProj proj_source(fhdrSource);
  
  // check this object has been previosuly initialized

  assert(_lutComputed);
  
  // make sure lookup table is up to date
  
  _computeLut(proj_source, _projTarget);
  
  // create field to be returned

  MdvxField *targetFld = new MdvxField;

  // set the field and vlevel headers in target

  targetFld->setFieldHeader(fhdrSource);
  targetFld->setVlevelHeader(vhdrSource);

  // we first interpolate the source planes, in (x,y),
  // leaving the heights unchanged

  // allocate 3d volume stage1
  
  int nz1 = fhdrSource.nz;
  int ny1 = _projTarget.getCoord().ny;
  int nx1 = _projTarget.getCoord().nx;
  fl32 ***vol1 = (fl32 ***) ucalloc3(nz1, ny1, nx1, sizeof(fl32)); // 3D pointer

  // initialize stage 1 to missing
  
  size_t nvol1 = nz1 * ny1 * nx1;
  fl32 *ptr1 = **vol1; // 1D pointer
  fl32 miss = fhdrSource.missing_data_value;
  for (size_t ii = 0; ii < nvol1; ii++, ptr1++) {
    *ptr1 = miss;
  }

  // interpolate in (x,y), one z plane at a time

  size_t nPtsPlaneSource = _coordSource->ny * _coordSource->nx;
  for (int iz = 0; iz < nz1; iz++) {

    fl32 *sourceStart = (fl32 *) sourceFld.getVol() + iz * nPtsPlaneSource;
    fl32 *vol1Start = *vol1[iz];
    
    // loop through (x,y) lookup table
    
    for (size_t ii = 0; ii < _xyLut.size(); ii++) {
      // find lookup entry
      xy_lut_t &lut = _xyLut[ii];
      // get field values at each corner
      fl32 val_ul = *(sourceStart + lut.pt_ul.sourceIndex);
      fl32 val_ur = *(sourceStart + lut.pt_ur.sourceIndex);
      fl32 val_ll = *(sourceStart + lut.pt_ll.sourceIndex);
      fl32 val_lr = *(sourceStart + lut.pt_lr.sourceIndex);
      if (val_ul != miss && val_ur != miss &&
          val_ll != miss && val_lr != miss) {
        // interpolate in x dim
        fl32 mean_upper = val_ul * lut.pt_ul.wtx + val_ur * lut.pt_ur.wtx;
        fl32 mean_lower = val_ll * lut.pt_ll.wtx + val_lr * lut.pt_lr.wtx;
        // interpolate in y dim
        fl32 mean2D = mean_upper * lut.pt_ul.wty + mean_lower * lut.pt_ll.wty;
        vol1Start[lut.targetIndex] = mean2D;
      } else {
        vol1Start[lut.targetIndex] = miss; // not really needed, since already initialized to missing
      }
    } // ii

  } // iz

  // allocate 3d volume stage2
  
  int nz2 = _projTarget.getCoord().nz;
  int ny2 = _projTarget.getCoord().ny;
  int nx2 = _projTarget.getCoord().nx;
  fl32 ***vol2 = (fl32 ***) ucalloc3(nz2, ny2, nx2, sizeof(fl32)); // 3D pointer
  
  // initialize stage 2 to missing
  
  size_t nvol2 = nz2 * ny2 * nx2;
  fl32 *ptr2 = **vol2; // 1D pointer
  for (size_t ii = 0; ii < nvol2; ii++, ptr2++) {
    *ptr2 = miss;
  }

  // interpolate the data onto the target Z dimension

  // free volumes
  
  ufree3((void ***) vol1);
  ufree3((void ***) vol2);
  
  // return interpolated field
  // this must be freed by the caller.
  
  return targetFld;
  
}

////////////////////////////////////
// compute lookup table coefficients

void MdvxRemapInterp::_computeLut(const MdvxProj &proj_source,
                                  const MdvxProj &proj_target)
  
{

  // check if source coords differ
  
  bool coordsDiffer = false;

  if (!_lutComputed) {

    _projSource = proj_source;
    _projTarget = proj_target;
    coordsDiffer = true;

  } else {

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

  // compute lookups

  _computeXyLookup();
  _computeZLookup();
  
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

  size_t targetIndex = 0;
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

      double wtx_left = sourceIx - ix_left;
      double wtx_right = 1.0 - wtx_left;
      
      double wty_lower = sourceIy - iy_lower;
      double wty_upper = 1.0 - wty_lower;
      
      // fill out lookup details
      
      xy_lut_t lut;
      
      lut.pt_ul.xx = coordSource.minx + ix_left * coordSource.dx;
      lut.pt_ul.yy = coordSource.miny + iy_upper * coordSource.dy;
      
      lut.pt_ur.xx = lut.pt_ul.xx + coordSource.dx;
      lut.pt_ur.yy = lut.pt_ul.yy;
      
      lut.pt_ll.xx = lut.pt_ul.xx;
      lut.pt_ll.yy = lut.pt_ul.yy - coordSource.dy;
      
      lut.pt_lr.xx = lut.pt_ll.xx + coordSource.dx;
      lut.pt_lr.yy = lut.pt_ll.yy;
      
      lut.pt_ul.sourceIndex = iy_upper * coordSource.nx + ix_left;
      lut.pt_ur.sourceIndex = iy_upper * coordSource.nx + ix_right;

      lut.pt_ll.sourceIndex = iy_lower * coordSource.nx + ix_left;
      lut.pt_lr.sourceIndex = iy_lower * coordSource.nx + ix_right;

      lut.pt_ul.wtx = wtx_left;
      lut.pt_ur.wtx = wtx_right;
      lut.pt_ll.wtx = wtx_left;
      lut.pt_lr.wtx = wtx_right;

      lut.pt_ul.wty = wty_upper;
      lut.pt_ur.wty = wty_upper;
      lut.pt_ll.wty = wty_lower;
      lut.pt_lr.wty = wty_lower;

      lut.targetIndex = targetIndex;

      // add to vector
      
      _xyLut.push_back(lut);
      
    } // ix

  } // iy

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

