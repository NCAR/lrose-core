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
/////////////////////////////////////////////////////////////////////////////
// MdvxRemapInterp.cc
//
// An object of this class is used to hold the lookup table
// for interpolation from one MdvxProj grid mapping to another.
//
// The constructor sets the target coords.
// interpField() passes in the source field, from which we deduce the coords.
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
#include <cmath>
using namespace std;

////////////////////////////////////////////////////////////////////////
// constructor

MdvxRemapInterp::MdvxRemapInterp()
  
{

  // initialize
  
  _lutComputed = false;
  _targetSet = false;
  _coordSource = &_projSource.getCoord();
  _coordTarget = &_projTarget.getCoord();

}

////////////////////////////////////////////////////////////////////////
// set target coordinates

void MdvxRemapInterp::setTargetCoords(const MdvxProj &projTgt,
                                      const vector<double> &vlevelsTgt)
  
{

  bool coordsDiffer = false;
  
  // check if proj has changed
  
  const Mdvx::coord_t &thisCoord = _projTarget.getCoord();
  const Mdvx::coord_t &inCoord = projTgt.getCoord();
  if (memcmp(&thisCoord, &inCoord, sizeof(Mdvx::coord_t))) {
    // copy in projection
    coordsDiffer = true;
  }

  if (vlevelsTgt != _vlevelsTarget) {
    coordsDiffer = true;
  }
  
  if (!coordsDiffer) {
    // no need to change anything
    return;
  }

  // re-initialize
  
  _lutComputed = false;
  _projTarget = projTgt;
  _vlevelsTarget = vlevelsTgt;
  _xyLut.clear();
  _zLut.clear();
  _targetSet = true;
  
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

  assert(_targetSet);

  // uncompress source and convert source to FLOAT32
  
  sourceFld.convertType(Mdvx::ENCODING_FLOAT32,
                        Mdvx::COMPRESSION_NONE);
  
  // create source projection object
  
  const Mdvx::field_header_t &fhdrSource = sourceFld.getFieldHeader();
  const Mdvx::vlevel_header_t &vhdrSource = sourceFld.getVlevelHeader();
  MdvxProj projSource(fhdrSource);
  vector<double> vlevelsSource = sourceFld.getVlevels();
  
  // make sure lookup table is up to date
  
  _computeLut(projSource, vlevelsSource);
  
  // we first interpolate the source planes, in (x,y),
  // leaving the heights unchanged

  // allocate 3d volume stage1
  
  int nz1 = _vlevelsSource.size();
  int ny1 = _coordTarget->ny;
  int nx1 = _coordTarget->nx;
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
        // not really needed, since already initialized to missing
        vol1Start[lut.targetIndex] = miss;
      }
    } // ii

  } // iz

  // allocate 3d volume stage2
  
  int nz2 = _vlevelsTarget.size();
  int ny2 = _coordTarget->ny;
  int nx2 = _coordTarget->nx;
  size_t npts2 = nz2 * ny2 * nx2;
  fl32 ***vol2 = (fl32 ***) ucalloc3(nz2, ny2, nx2, sizeof(fl32)); // 3D pointer
  
  // initialize stage 2 to missing
  
  size_t nvol2 = nz2 * ny2 * nx2;
  fl32 *ptr2 = **vol2; // 1D pointer
  for (size_t ii = 0; ii < nvol2; ii++, ptr2++) {
    *ptr2 = miss;
  }

  // interpolate each (x,y) column onto the target Z dimension

  for (int iy = 0; iy < _coordTarget->ny; iy++) {
    for (int ix = 0; ix < _coordTarget->nx; ix++) {

      for (int iz = 0; iz < (int) _vlevelsTarget.size(); iz++) {
        
        z_lut_t &lut = _zLut[iz];

        fl32 valLower = vol1[lut.indexLower][iy][ix];
        fl32 valUpper = vol1[lut.indexUpper][iy][ix];

        if (valLower != miss && valUpper != miss) {
          double valInterp = valLower * lut.wtLower + valUpper * lut.wtUpper;
          // cerr << "lower, upper, interp: " << valLower << ", " << valUpper << ", " << valInterp << endl;
          // if (std::isnan(valInterp)) {
          //   cerr << "*";
          // }
          vol2[iz][iy][ix] = valInterp;
        } else {
          // not really needed, since already initialized to missing
          vol2[iz][iy][ix] = miss;
        }
        
      } // iz
      
    } // ix
  } // iy

  // create field to be returned

  MdvxField *targetFld = new MdvxField;

  // set the field and vlevel headers in target

  Mdvx::field_header_t fhdr2 = fhdrSource;
  Mdvx::vlevel_header_t vhdr2 = vhdrSource;

  fhdr2.nz = _coordTarget->nz;
  for (int ii = 0; ii < fhdr2.nz; ii++) {
    vhdr2.level[ii] = _vlevelsTarget[ii];
  }
  _projTarget.syncXyToFieldHdr(fhdr2);
  
  // add headers
  
  targetFld->setFieldHeader(fhdr2);
  targetFld->setVlevelHeader(vhdr2);

  // cerr << "222222222222222222222222222222222222222" << endl;
  // Mdvx::printFieldHeader(fhdr2, cerr);
  // cerr << "222222222222222222222222222222222222222" << endl;

  // add data - use ** to get to the 1D array underneath the 3D version
  
  targetFld->setVolData((void *) **vol2, npts2, Mdvx::ENCODING_FLOAT32);

  // free volumes
  
  ufree3((void ***) vol1);
  ufree3((void ***) vol2);
  
  // return interpolated field
  // this must be freed by the caller.
  
  return targetFld;
  
}

////////////////////////////////////
// compute lookup table coefficients

void MdvxRemapInterp::_computeLut(const MdvxProj &projSrc,
                                  const vector<double> &vlevelsSrc)
  
{

  // check if source coords differ
  
  bool coordsDiffer = false;

  // check if we need to compute lut
  
  if (!_lutComputed) {
    coordsDiffer = true;
  }
  
  const Mdvx::coord_t &thisCoord = _projSource.getCoord();
  const Mdvx::coord_t &inCoord = projSrc.getCoord();
  if (memcmp(&thisCoord, &inCoord, sizeof(Mdvx::coord_t))) {
    // copy in projection
    coordsDiffer = true;
  }

  if (vlevelsSrc != _vlevelsSource) {
    coordsDiffer = true;
  }
  
  if (!coordsDiffer) {
    // no need to recompute
    return;
  }

  // save source info
  
  _projSource = projSrc;
  _vlevelsSource = vlevelsSrc;
  
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

  _xyLut.clear();

  size_t targetIndex = 0;
  for (int iy = 0; iy < _coordTarget->ny; iy++) {

    double targetY = _coordTarget->miny + iy * _coordTarget->dy;

    for (int ix = 0; ix < _coordTarget->nx; ix++, targetIndex++) {

      double targetX = _coordTarget->minx + ix * _coordTarget->dx;

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
          sourceIx > _coordSource->nx || sourceIy > _coordSource->ny) {
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
      
      lut.pt_ul.xx = _coordSource->minx + ix_left * _coordSource->dx;
      lut.pt_ul.yy = _coordSource->miny + iy_upper * _coordSource->dy;
      
      lut.pt_ur.xx = lut.pt_ul.xx + _coordSource->dx;
      lut.pt_ur.yy = lut.pt_ul.yy;
      
      lut.pt_ll.xx = lut.pt_ul.xx;
      lut.pt_ll.yy = lut.pt_ul.yy - _coordSource->dy;
      
      lut.pt_lr.xx = lut.pt_ll.xx + _coordSource->dx;
      lut.pt_lr.yy = lut.pt_ll.yy;
      
      lut.pt_ul.sourceIndex = iy_upper * _coordSource->nx + ix_left;
      lut.pt_ur.sourceIndex = iy_upper * _coordSource->nx + ix_right;

      lut.pt_ll.sourceIndex = iy_lower * _coordSource->nx + ix_left;
      lut.pt_lr.sourceIndex = iy_lower * _coordSource->nx + ix_right;

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

  _zLut.clear();
  
  for (size_t iz = 0; iz < _vlevelsTarget.size(); iz++) {
    
    z_lut_t lut;
    lut.zz = _vlevelsTarget[iz];
    
    if (lut.zz <= _vlevelsSource[0]) {

      // target z is below source lower plane
      
      lut.indexLower = 0;
      lut.indexUpper = 0;
      lut.zLower = _vlevelsSource[0];
      lut.zUpper = _vlevelsSource[0];
      lut.wtLower = 0.0;
      lut.wtUpper = 1.0;

      _zLut.push_back(lut);
      continue;
      
    }
    
    if (lut.zz >= _vlevelsSource[_vlevelsSource.size() - 1]) {

      // target z is above source upper plane
      
      lut.indexLower = _vlevelsSource.size() - 1;
      lut.indexUpper = _vlevelsSource.size() - 1;
      lut.zLower = _vlevelsSource[_vlevelsSource.size() - 1];
      lut.zUpper = _vlevelsSource[_vlevelsSource.size() - 1];
      lut.wtLower = 1.0;
      lut.wtUpper = 0.0;

      _zLut.push_back(lut);
      continue;
      
    }
    
    // we are within the source zlevel limits
    
    for (size_t jz = 1; jz < _vlevelsSource.size(); jz++) {
      double zLower = _vlevelsSource[jz-1];
      double zUpper = _vlevelsSource[jz];
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
