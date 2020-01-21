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

////////////////////////////////////////////
// constructor

RhiOrient::RhiOrient(const Params &params,
                     RadxVol &readVol,
                     double azimuth,
                     double startRangeKm,
                     double gateSpacingKm,
                     double radarAltKm,
                     const vector<double> &gridZLevels) :
        _params(params),
        _readVol(readVol),
        _nSweeps(readVol.getNSweeps()),
        _nGates(readVol.getMaxNGates()),
        _azimuth(azimuth),
        _startRangeKm(startRangeKm),
        _gateSpacingKm(gateSpacingKm),
        _radarAltKm(radarAltKm),
        _gridZLevels(gridZLevels),
        _nZ(gridZLevels.size())

{

  _success = false;
  _nRange = _nGates;
  DBZ_BAD = -200.0;

  // initialize beamHeight computations

  if (_params.override_standard_pseudo_earth_radius) {
    _beamHt.setPseudoRadiusRatio(_params.pseudo_earth_radius_ratio);
  }
  _beamHt.setInstrumentHtKm(_radarAltKm);

}

////////////////////////////////////////////
// destructor

RhiOrient::~RhiOrient()
{
  clear();
}

////////////////////////////////////////////
// clear out memory

void RhiOrient::clear()
{
  _success = false;
  _clearArrays();
}

////////////////////////////////////////////
// compute echo orientation

void RhiOrient::computeEchoOrientation()
{

  // allocate the working arrays

  _allocArrays();

  // load up the synthetic RHI

  if (_loadSyntheticRhi()) {
    _success = false;
    return;
  }
  
  // load up reflectivity grids in H anv V

  if (_loadDbzH()) {
    _success = false;
    return;
  }
  if (_loadDbzV()) {
    _success = false;
    return;
  }

  // load up standard deviation of DBZ in H and V

  if (_loadSdevH()) {
    _success = false;
    return;
  }
  if (_loadSdevV()) {
    _success = false;
    return;
  }

}

///////////////////////////////////////////////////////////
// allocate the required arrays

void RhiOrient::_allocArrays()
{

  // allocate
  
  _dbzH.resize(_nZ);
  _dbzV.resize(_nZ);
  
  _sdevDbzH.resize(_nZ);
  _sdevDbzV.resize(_nZ);

  for (size_t iz = 0; iz < _nZ; iz++) {
    _dbzH[iz].resize(_nRange);
    _dbzV[iz].resize(_nRange);
    _sdevDbzH[iz].resize(_nRange);
    _sdevDbzV[iz].resize(_nRange);
  }
  
  // initialize
  
  for (size_t iz = 0; iz < _nZ; iz++) {
    for (size_t irange = 0; irange < _nRange; irange++) {
      _dbzH[iz][irange] = NAN;
      _dbzV[iz][irange] = NAN;
      _sdevDbzH[iz][irange] = NAN;
      _sdevDbzV[iz][irange] = NAN;
    }
  }

}


///////////////////////////////////////////////////////////
// clear the working arrays

void RhiOrient::_clearArrays()
{
  
  for (size_t iz = 0; iz < _dbzH.size(); iz++) {
    _dbzH[iz].clear();
  }
  _dbzH.clear();

  for (size_t iz = 0; iz < _dbzV.size(); iz++) {
    _dbzV[iz].clear();
  }
  _dbzV.clear();

  for (size_t iz = 0; iz < _sdevDbzH.size(); iz++) {
    _sdevDbzH[iz].clear();
  }
  _sdevDbzH.clear();

  for (size_t iz = 0; iz < _sdevDbzV.size(); iz++) {
    _sdevDbzV[iz].clear();
  }
  _sdevDbzV.clear();

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

///////////////////////////////////////////////////////////////
/// Load up DBZ-H grid
/// This grid will be used to compute the sdev of dbz horizontally
/// Returns 0 on success
/// Returns -1 on error - i.e. if DBZ field not found

int RhiOrient::_loadDbzH()
  
{
  
  // loop through the rays, bottom to top
  
  for (size_t iray = 0; iray < _rays.size(); iray++) {

    RadxRay *ray = _rays[iray];
    RadxField *dbzField = ray->getField(_params.echo_orientation_dbz_field_name);
    if (dbzField == NULL) {
      cerr << "ERROR - RhiOrient::_loadDbzH()" << endl;
      cerr << "  Cannot find field on ray: "
           << _params.echo_orientation_dbz_field_name << endl;
      cerr << "  elev, az: " 
           << ray->getElevationDeg() << ", " << ray->getAzimuthDeg() << endl;
      return -1;
    }
    dbzField->convertToFl32();
    Radx::fl32 *dbz = dbzField->getDataFl32();
    Radx::fl32 dbzMiss = dbzField->getMissingFl32();

    // loop through the gates
    // look for intersection with the grid level in height
    // start at bottom level

    double elev = ray->getElevationDeg();
    size_t zIndex = 0;
    double zzGrid = _gridZLevels[zIndex];
    double zzPrev = zzGrid - 1.0;
    double dbzPrev = dbz[0];
    
    for (size_t igate = 0; igate < ray->getNGates(); igate++) {
      
      double dbzVal = dbz[igate];
      
      // compute height and ground distance along RHI
      
      double range = _startRangeKm + igate * _gateSpacingKm;
      double zz = _beamHt.computeHtKm(elev, range);
      double xx = _beamHt.getGndRangeKm();
      int xIndex = (int) (xx / _gateSpacingKm);

      if (zz < zzGrid) {
        // still below the grid level, continue
        continue;
      }

      // this is the first gate above the grid level
      // interp to get dbz val or use the non-missing val
      // store the dbz val

      if (dbzVal != dbzMiss && dbzPrev != dbzMiss) {
        // both available, interp
        double zzDelta = zz - zzPrev;
        double wtBelow = (zz - zzGrid) / zzDelta;
        double wtAbove = 1.0 - wtBelow;
        double dbzInterp = dbzVal * wtAbove + dbzPrev * wtBelow;
        _dbzH[zIndex][xIndex] = dbzInterp;
      } else if (dbzVal != dbzMiss) {
        // use latest dbz
        _dbzH[zIndex][xIndex] = dbzVal;
      } else if (dbzPrev != dbzMiss) {
        // use previous dbz
        _dbzH[zIndex][xIndex] = dbzPrev;
      } else {
        // neither available, set to missing
        _dbzH[zIndex][xIndex] = DBZ_BAD;
      }

      // prepare for next level
      
      zIndex++;
      if (zIndex == _nZ) {
        // at top of grid, so no need to process more gates
        break;
      }
      
      zzGrid = _gridZLevels[zIndex];
      zzPrev = zz;
      dbzPrev = dbzVal;

    } // igate
      
  } // iray

  return 0;

}

///////////////////////////////////////////////////////////////
/// Load up DBZ-V grid
/// This grid will be used to compute the sdev of dbz vertically
/// Returns 0 on success
/// Returns -1 on error - i.e. if DBZ field not found

int RhiOrient::_loadDbzV()
  
{
  
  // loop through the RHI rays, bottom to top
  
  for (size_t iray = 0; iray < _rays.size(); iray++) {

    // get ray and dbz field

    RadxRay *ray = _rays[iray];
    RadxField *dbzField = ray->getField(_params.echo_orientation_dbz_field_name);
    if (dbzField == NULL) {
      cerr << "ERROR - RhiOrient::_loadDbzV()" << endl;
      cerr << "  Cannot find field on ray: "
           << _params.echo_orientation_dbz_field_name << endl;
      cerr << "  elev, az: " 
           << ray->getElevationDeg() << ", " << ray->getAzimuthDeg() << endl;
      return -1;
    }
    dbzField->convertToFl32();
    Radx::fl32 *dbz = dbzField->getDataFl32();
    Radx::fl32 dbzMiss = dbzField->getMissingFl32();
    
    // loop through the grid in range (this is ground range)
    
    double elev = ray->getElevationDeg();
    double cosElev = cos(elev * Radx::DegToRad);

    for (size_t ix = 0; ix < _nRange; ix++) {
      
      double gndRange = (ix + 0.5) * _gateSpacingKm;

      // compute slant radar range

      double slantRange = gndRange / cosElev;

      // get the gate for this range

      int igateClosest = (int) ((slantRange - _startRangeKm) / _gateSpacingKm + 0.5);
      if (igateClosest < 0) {
        continue;
      }
      if (igateClosest > (int) _nGates - 1) {
        break;
      }

      // get the dbz for this gate

      double dbzClosest = dbz[igateClosest];
      
      // compute beam height and z grid index at this gate
      
      double zz = _beamHt.computeHtKm(elev, slantRange);
      int zIndex = _getZIndex(zz);
      
      // store the dbz in the grid

      if (dbzClosest != dbzMiss) {
        _dbzV[zIndex][igateClosest] = dbzClosest;
      } else {
        _dbzV[zIndex][igateClosest] = DBZ_BAD;
      }

    } // ix

  } // iray

  return 0;

}

///////////////////////////////////////////////////////////////
/// Load up DBZ-SDEV-H grid from DBZ-H grid
/// Returns 0 on success
/// Returns -1 on error

int RhiOrient::_loadSdevH()
  
{
  
  // loop through the levels, bottom to top
  
  for (size_t iz = 0; iz < _nZ; iz++) {
    
    // loop through the gates
    // loading up the dbz vals from ray intersections
    // into a vector, along with their gate numbers

    vector<double> dbzVals;
    vector<int> gateNums;
    for (size_t irange = 0; irange < _nRange; irange++) {
      double dbzVal = _dbzH[iz][irange];
      if (!std::isnan(dbzVal)) {
        dbzVals.push_back(dbzVal);
        gateNums.push_back(irange);
      }
    } // irange
    
    size_t nDbz = dbzVals.size();
    if (nDbz < 2) {
      continue;
    }

    // load up sdev vector, to match dbz vals
    
    vector<double> sdevDbzH;

    // compute sdev for first val - just use 2 pts
    
    double sdev = _computeSdev2(dbzVals[0], dbzVals[1]);
    sdevDbzH.push_back(sdev);

    // compute sdev for interior vals

    if (nDbz >= 3) {
      for (size_t ii = 1; ii < nDbz - 2; ii++) {
        sdev = _computeSdev3(dbzVals[ii-1], dbzVals[ii], dbzVals[ii+1]);
        sdevDbzH.push_back(sdev);
      }
    }
    
    // compute sdev for last val - just use 2 pts
    
    sdev = _computeSdev2(dbzVals[nDbz-2], dbzVals[nDbz-1]);
    sdevDbzH.push_back(sdev);

    assert(sdevDbzH.size() == nDbz);
    
    // save these computed sdev values in the array
    
    for (size_t ii = 0; ii < nDbz; ii++) {
      _sdevDbzH[iz][gateNums[ii]] = sdevDbzH[ii];
    }

    // interpolate for the intermediate grid locations
    
    for (size_t ii = 1; ii < nDbz; ii++) {
      size_t gateLower = gateNums[ii-1];
      size_t gateUpper = gateNums[ii];
      ssize_t nMiss = gateUpper - gateLower - 1;
      if (nMiss < 1) {
        continue;
      }
      double sdevDiff = sdevDbzH[ii] - sdevDbzH[ii-1];
      double sdevDeltaPerGate = sdevDiff / (nMiss + 1.0);
      for (ssize_t jj = 1; jj <= nMiss; jj++) {
        double sdevInterp = sdevDbzH[ii-1] + sdevDeltaPerGate * jj;
        _sdevDbzH[iz][gateNums[ii+jj]] = sdevInterp;
      }
    }

  } // iz

  return 0;

}

///////////////////////////////////////////////////////////////
/// Load up DBZ-SDEV-V grid from DBZ-V grid
/// Returns 0 on success
/// Returns -1 on error

int RhiOrient::_loadSdevV()
  
{
  
  // loop through the gates
  
  for (size_t irange = 0; irange < _nRange; irange++) {
    
    // loop through the levels
    // loading up the dbz vals from ray intersections
    // into a vector, along with their level number

    vector<double> dbzVals;
    vector<int> levelNums;
    for (size_t iz = 0; iz < _nZ; iz++) {
      double dbzVal = _dbzV[iz][irange];
      if (!std::isnan(dbzVal)) {
        dbzVals.push_back(dbzVal);
        levelNums.push_back(iz);
      }
    } // irange
    
    size_t nDbz = dbzVals.size();
    if (nDbz < 2) {
      continue;
    }

    // load up sdev vector, to match dbz vals
    
    vector<double> sdevDbzV;

    // compute sdev for first val - just use 2 pts
    
    double sdev = _computeSdev2(dbzVals[0], dbzVals[1]);
    sdevDbzV.push_back(sdev);

    // compute sdev for interior vals

    if (nDbz >= 3) {
      for (size_t ii = 1; ii < nDbz - 2; ii++) {
        sdev = _computeSdev3(dbzVals[ii-1], dbzVals[ii], dbzVals[ii+1]);
        sdevDbzV.push_back(sdev);
      }
    }
    
    // compute sdev for last val - just use 2 pts
    
    sdev = _computeSdev2(dbzVals[nDbz-2], dbzVals[nDbz-1]);
    sdevDbzV.push_back(sdev);

    assert(sdevDbzV.size() == nDbz);
    
    // save these computed sdev values in the array
    
    for (size_t ii = 0; ii < nDbz; ii++) {
      _sdevDbzV[levelNums[ii]][irange] = sdevDbzV[ii];
    }

    // interpolate for the intermediate grid locations
    
    for (size_t ii = 1; ii < nDbz; ii++) {
      size_t levelLower = levelNums[ii-1];
      size_t levelUpper = levelNums[ii];
      ssize_t nMiss = levelUpper - levelLower - 1;
      if (nMiss < 1) {
        continue;
      }
      double sdevDiff = sdevDbzV[ii] - sdevDbzV[ii-1];
      double sdevDeltaPerLevel = sdevDiff / (nMiss + 1.0);
      for (ssize_t jj = 1; jj <= nMiss; jj++) {
        double sdevInterp = sdevDbzV[ii-1] + sdevDeltaPerLevel * jj;
        _sdevDbzV[levelNums[ii+jj]][irange] = sdevInterp;
      }
    }

  } // iz

  return 0;

}

///////////////////////////////////////////////////////////////
/// Get closest Z index to given ht
/// Returns index on success, -1 on failure

int RhiOrient::_getZIndex(double zz)
  
{

  double gridMinZ = _gridZLevels[0];
  double gridMaxZ = _gridZLevels[_nZ-1];
  if (zz < gridMinZ || zz > gridMaxZ) {
    return -1;
  }
  
  int zIndex = -1;
  double minDelta = 1.0e6;
  
  for (size_t iz = 0; iz < _nZ; iz++) {
    double delta = fabs(zz - _gridZLevels[iz]);
    if (delta < minDelta) {
      zIndex = iz;
      minDelta = delta;
    } else {
      break;
    }
  } // iz

  return zIndex;

}

///////////////////////////////////////////////////////////////
/// Compute sdev for 2 points

double RhiOrient::_computeSdev2(double val1, double val2)
{
  double sum = val1 + val2;
  double sumsq = val1 * val1 + val2 * val2;
  double nn = 2.0;
  double sdev = _computeSdevFromSums(sum, sumsq, nn);
  return sdev;
}

///////////////////////////////////////////////////////////////
/// Compute sdev for 3 points

double RhiOrient::_computeSdev3(double val1, double val2, double val3)
{
  double sum = val1 + val2 + val3;
  double sumsq = val1 * val1 + val2 * val2 + val3 * val3;
  double nn = 3.0;
  double sdev = _computeSdevFromSums(sum, sumsq, nn);
  return sdev;
}

///////////////////////////////////////////////////////////////
/// Compute sdev from sum and sumsq

double RhiOrient::_computeSdevFromSums(double sum, double sumsq, double nn)

{
  double comp = nn * sumsq - sum * sum;
  if (comp <= 0.0 || nn < 2.0) {
    return 0.0;
  } else {
    return (sqrt(comp) / (nn - 1.0));
  }
}

