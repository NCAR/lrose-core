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
// RhiOrient.cc
//
// RhiOrient class.
// Compute echo orientation in a synthetic RHI
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2020
//
///////////////////////////////////////////////////////////////

#include "RhiOrient.hh"
#include <cmath>
#include <set>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxRay.hh>

RhiOrient::RhiOrient(const Params &params,
                     RadxVol &readVol,
                     double azimuth,
                     double startRangeKm,
                     double gateSpacingKm,
                     double radarAltKm,
                     const vector<double> &gridZLevels) :
        _params(params),
        _readVol(readVol),
        _azimuth(azimuth),
        _maxNGates(readVol.getMaxNGates()),
        _startRangeKm(startRangeKm),
        _gateSpacingKm(gateSpacingKm),
        _radarAltKm(radarAltKm),
        _gridZLevels(gridZLevels)

{
}

RhiOrient::~RhiOrient(void)
{
  _free();
}

void RhiOrient::_free()
{
}

void RhiOrient::computeEchoOrientation()
{

  // allocate the grids

  _dbz.clear();
  _gridError.clear();

  _dbz.resize(_gridZLevels.size());
  for (size_t iz = 0; iz < _gridZLevels.size(); iz++) {
    _dbz[iz].resize(_maxNGates);
    _gridError[iz].resize(_maxNGates);
    for (size_t ii = 0; ii < _maxNGates; ii++) {
      _dbz[iz][ii] = NAN;
      _gridError[iz][ii] = NAN;
    }
  }

  // load up the synthetic RHI

  _loadSyntheticRhi();

}

///////////////////////////////////////////////////////////////
/// Load up synthetic RHIs, by analyzing the rays in the volume.
/// Only relevant for surveillance and sector ppi-type volumes.
/// Returns 0 on success
/// Returns -1 on error - i.e. if not ppi-type scan.

int RhiOrient::_loadSyntheticRhi()
  
{

  // initialize

  _rays.clear();

  // check scan mode

  if (_readVol.checkIsRhi()) {
    return -1;
  }
  
  

  // compute azimuth margin for finding RHI rays

  double azMargin = _params.synthetic_rhis_delta_az * 2.5;
  
  // find rays that belong to this RHI
  
  const vector<RadxSweep *> &sweeps = _readVol.getSweeps();
  for (size_t isweep = 0; isweep < sweeps.size(); isweep++) {
    RadxSweep *sweep = sweeps[isweep];
    RadxRay *bestRay = NULL;
    double minDeltaAz = 9999.0;
    for (size_t jray = sweep->getStartRayIndex(); 
         jray <= sweep->getEndRayIndex(); jray++) {
      RadxRay *ray = _rays[jray];
      double deltaAz = fabs(_azimuth - ray->getAzimuthDeg());
      if (deltaAz > 180.0) {
        deltaAz = fabs(deltaAz - 360.0);
      }
      if (deltaAz < azMargin && deltaAz < minDeltaAz) {
        bestRay = ray;
        minDeltaAz = deltaAz;
      }
    } // jray
    if (bestRay != NULL) {
      _rays.push_back(bestRay);
    }
  } // isweep;

  // sanity check
  
  if (_rays.size() < 1) {
    return -1;
  }
  
  // sort the rays in elevation
  
  _sortRaysByElevation();
  
  // Compute the mean azimuth from the rays
  // sum up (x,y) coords of measured angles
  
  double sumx = 0.0;
  double sumy = 0.0;
  for (size_t iray = 0; iray < _rays.size(); iray++) {
    const RadxRay *ray = _rays[iray];
    double angle = ray->getAzimuthDeg();
    double sinVal, cosVal;
    Radx::sincos(angle * Radx::DegToRad, sinVal, cosVal);
    sumy += sinVal;
    sumx += cosVal;
  } // iray
  _meanAzimuth = atan2(sumy, sumx) * Radx::RadToDeg;

  return 0;

}

//////////////////////////////////////////////////////////  
/// Sort the rays by elevation angle, lowest to highest
/// Also sets the az of lowest ray,
/// mean azimuth and max nGates.

void RhiOrient::_sortRaysByElevation()
{
  
  // create set with sorted ray pointers
  
  set<RayPtr, SortByRayElevation> sortedRayPtrs;
  for (size_t iray = 0; iray < _rays.size(); iray++) {
    RayPtr rptr(_rays[iray]);
    sortedRayPtrs.insert(rptr);
  }
  
  // add sortedRays array in elev-sorted order
  
  vector<RadxRay *> sortedRays;
  for (set<RayPtr, SortByRayElevation>::iterator ii = sortedRayPtrs.begin();
       ii != sortedRayPtrs.end(); ii++) {
    sortedRays.push_back(ii->ptr);
  }
    
  // set _rays to sorted vector
  
  _rays = sortedRays;

}

/////////////////////////////////////////////////////////////////
// Compare rays by elevation

bool RhiOrient::SortByRayElevation::operator()
  (const RayPtr &lhs, const RayPtr &rhs) const
{
  return lhs.ptr->getElevationDeg() < rhs.ptr->getElevationDeg();
}

