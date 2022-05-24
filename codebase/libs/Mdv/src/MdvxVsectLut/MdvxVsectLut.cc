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
// MdvxVsectLut.cc
//
// An object of this class is used to hold the lookup table for
// computing vertical sections.
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 1999
//
//////////////////////////////////////////////////////////
//
// See <Mdv/MdvxVsectLut.hh> for details.
//
///////////////////////////////////////////////////////////

#include <Mdv/MdvxVsectLut.hh>
#include <toolsa/pjg.h>
#include <cmath>
using namespace std;

////////////////////////////////////////////////////////////////////////
// Default constructor
//
// You need to call computeSamplePts() and
// computeOffsets() before using the lookup table.

MdvxVsectLut::MdvxVsectLut()
  
{
  _offsetsComputed = false;
  _weightsComputed = false;
  _nSamplesRequested = 0;
}

////////////////////////////////////////////////////////////////////////
// constructor which computes lookup table

MdvxVsectLut::MdvxVsectLut(const vector<Mdvx::vsect_waypt_t> &waypts,
			   const int n_samples,
			   const MdvxProj &proj)
  
{
  _offsetsComputed = false;
  _weightsComputed = false;
  _nSamplesRequested = 0;
  computeSamplePts(waypts, n_samples);
  computeOffsets(proj);
}

/////////////////////////////
// Destructor

MdvxVsectLut::~MdvxVsectLut()

{
}

/////////////////////
// compute sample pts

void MdvxVsectLut::computeSamplePts(const vector<Mdvx::vsect_waypt_t> &waypts,
				    int n_samples)
  
{

  // compute segments

  _wayPts = waypts;
  _totalLength = 0.0;
  double count = 0.0;
  _segments.erase(_segments.begin(), _segments.end());
  for (size_t i = 1; i < _wayPts.size(); i++) {
    Mdvx::vsect_segment_t seg;
    PJGLatLon2RTheta(_wayPts[i-1].lat, _wayPts[i-1].lon,
                     _wayPts[i].lat, _wayPts[i].lon,
                     &seg.length, &seg.azimuth);
    _totalLength += seg.length;
    count++;
    _segments.push_back(seg);
  }

  // compute dkm from length and number of sampling points
  
  _nSamplesRequested = n_samples;
  if (count == 0) {
    _dxKm = 1.0;
  } else {
    _dxKm = _totalLength / n_samples;
  }
  
  // load up the lat/lon for each sample point
  
  _samplePts.erase(_samplePts.begin(), _samplePts.end());

  if (_wayPts.size() == 1) {
    // only 1 way point - single point request
    Mdvx::vsect_samplept_t pt;
    pt.lat = _wayPts[0].lat;
    pt.lon = _wayPts[0].lon;
    pt.segNum = 0;
    _samplePts.push_back(pt);
  } else {
    // more than 1 way point - segment request
    double posInSeg = _dxKm / 2.0;
    double lengthSoFar = posInSeg;
    for (size_t i = 0; i < _segments.size(); i++) {
      while (posInSeg < _segments[i].length) {
        double lat, lon;
        PJGLatLonPlusRTheta(_wayPts[i].lat, _wayPts[i].lon,
                            posInSeg, _segments[i].azimuth,
                            &lat, &lon);
        Mdvx::vsect_samplept_t pt;
        pt.lat = lat;
        pt.lon = lon;
        pt.segNum = i;
        _samplePts.push_back(pt);
        posInSeg += _dxKm;
        lengthSoFar += _dxKm;
      }
      posInSeg -= _segments[i].length;
    } // i
  }

}

///////////////////////////////
// compute lookup table offsets
//
// Must call computeSamplePts() first

void MdvxVsectLut::computeOffsets(const MdvxProj &proj)

{

  _offsets.clear();
  
  // loop through the sample points
  
  for (size_t i = 0; i < _samplePts.size(); i++) {
    int64_t offset;
    if (proj.latlon2arrayIndex(_samplePts[i].lat, _samplePts[i].lon,
			       offset, true) == 0) {
      _offsets.push_back(offset);
    } else {
      _offsets.push_back(-1);
    }
  }

  _offsetsComputed = true;
  _proj = proj;

}

////////////////////////////////
// compute lookup table offsets
//
// Does not recompute if no changes have occurred.

void MdvxVsectLut::computeOffsets(const vector<Mdvx::vsect_waypt_t> &waypts,
				  const int n_samples,
				  const MdvxProj &proj)

{

  // do not recompute if nothing has changed

  if (_geometryChanged(waypts, n_samples, proj)) {
    computeOffsets(proj);
  }

}

////////////////////////////////////
// compute lookup table weights
//
// Must call computeSamplePts() first

void MdvxVsectLut::computeWeights(const MdvxProj &proj)

{

  _weights.clear();
  const Mdvx::coord_t coord = proj.getCoord();
  
  // loop through the sample points
  
  for (size_t i = 0; i < _samplePts.size(); i++) {
    
    MdvxVsectLutEntry entry;
    double xx, yy;

    if (proj.latlon2xyIndex(_samplePts[i].lat, _samplePts[i].lon,
			    xx, yy, true) == 0) {
      
      if (xx >= 0.0 && xx <= (coord.nx - 1) &&
	  yy >= 0.0 && yy <= (coord.ny - 1)) {

	int ix = (int) xx;
	int iy = (int) yy;

	double diff_x = xx - ix;
	double diff_y = yy - iy;
	double dx, dy, dist;
	
	// SW corner
	
	dx = diff_x;
	dy = diff_y;
	dist = sqrt(dx * dx + dy * dy);
	if (dist == 0.0) {
	  dist = 1.0e-10;
	}
	double sw_inv = 1.0 / dist;

	// SE corner
	
	dx = 1.0 - diff_x;
	dist = sqrt(dx * dx + dy * dy);
	if (dist == 0.0) {
	  dist = 1.0e-10;
	}
	double se_inv = 1.0 / dist;

	// NE corner
	
	dy = 1.0 - diff_y;
	dist = sqrt(dx * dx + dy * dy);
	if (dist == 0.0) {
	  dist = 1.0e-10;
	}
	double ne_inv = 1.0 / dist;

	// NW corner
	
	dx = diff_x;
	dist = sqrt(dx * dx + dy * dy);
	if (dist == 0.0) {
	  dist = 1.0e-10;
	}
	double nw_inv = 1.0 / dist;

	// compute inverse-weighted sum

	double sum_inv = sw_inv + nw_inv + ne_inv + se_inv;

	// load up lut entry

	int64_t index = iy * coord.nx + ix;

	entry.offsets[0] = index;
	entry.wts[0] = sw_inv / sum_inv;

	entry.offsets[1] = index + 1;
	entry.wts[1] = se_inv / sum_inv;
	
	entry.offsets[2] = index + 1 + coord.nx;
	entry.wts[2] = ne_inv / sum_inv;

	entry.offsets[3] = index + coord.nx;
	entry.wts[3] = nw_inv / sum_inv;

	entry.set = true;

      }

    }
    
    _weights.push_back(entry);

  } // i

  _weightsComputed = true;
  _proj = proj;

}

////////////////////////////////
// compute lookup table weights
//
// Does not recompute if no changes have occurred.

void MdvxVsectLut::computeWeights(const vector<Mdvx::vsect_waypt_t> &waypts,
				  const int n_samples,
				  const MdvxProj &proj)

{

  // do not recompute if nothing has changed

  if (_geometryChanged(waypts, n_samples, proj)) {
    computeWeights(proj);
  }

}

////////////////////////////////
// check for change in geometry
//

bool MdvxVsectLut::_geometryChanged(const vector<Mdvx::vsect_waypt_t> &waypts,
				    const int n_samples,
				    const MdvxProj &proj)

{

  // check if waypts differ

  bool samplePtsDiffer = false;
  if (n_samples != _nSamplesRequested) {
    samplePtsDiffer = true;
  } else if (waypts.size() != _wayPts.size()) {
    samplePtsDiffer = true;
  } else {
    for (size_t i = 0; i < waypts.size(); i++) {
      if (memcmp(&waypts[i], &_wayPts[i], sizeof(Mdvx::vsect_waypt_t))) {
	samplePtsDiffer = true;
	break;
      }
    }
  }
  if (samplePtsDiffer) {
    computeSamplePts(waypts, n_samples);
  }

  // check if coord differs
  
  const Mdvx::coord_t &thisCoord = _proj.getCoord();
  const Mdvx::coord_t &inCoord = proj.getCoord();
  bool coordsDiffer;
  if (memcmp(&thisCoord, &inCoord, sizeof(Mdvx::coord_t))) {
    coordsDiffer = true;
  } else {
    coordsDiffer = false;
  }

  // do not recompute if nothing has changed

  if (samplePtsDiffer || coordsDiffer || !_offsetsComputed) {
    return true;
  } else {
    return false;
  }

}

