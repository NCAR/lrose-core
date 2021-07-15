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
// MdvxRemapLut.cc
//
// An object of this class is used to hold the lookup table for
// computing grid remapping.
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 1999
//
//////////////////////////////////////////////////////////
//
// See <Mdv/MdvxRemapLut.hh> for details.
//
///////////////////////////////////////////////////////////

#include <Mdv/MdvxRemapLut.hh>
#include <toolsa/pjg.h>
using namespace std;

////////////////////////////////////////////////////////////////////////
// Default constructor
//
// You need to call computeOffsets()
// before using the lookup table.

MdvxRemapLut::MdvxRemapLut()
  
{
  _offsetsComputed = false;
}

////////////////////////////////////////////////////////////////////////
// constructor which computes lookup table

MdvxRemapLut::MdvxRemapLut(const MdvxProj &proj_source,
			   const MdvxProj &proj_target)
  
{
  _offsetsComputed = false;
  computeOffsets(proj_source, proj_target);
}

/////////////////////////////
// Destructor

MdvxRemapLut::~MdvxRemapLut()

{
  return;
}

///////////////////////////////
// compute lookup table offsets

void MdvxRemapLut::computeOffsets(const MdvxProj &proj_source,
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

  if (!coordsDiffer && _offsetsComputed) {
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

  _offsetsComputed = true;

  return;

}

