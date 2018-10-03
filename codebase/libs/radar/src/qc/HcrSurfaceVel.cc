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
//////////////////////////////////////////////////////////////////////////
// HcrSurfaceVel.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// October 2016
//
//////////////////////////////////////////////////////////////////////////
//
// HcrSurfaceVel is intended for use with downward-pointing airborne
// Doppler radars.
//
// HcrSurfaceVel reads in Radx moments, and computes the apparent velocity
// of the surface echo.
//
// Generally this will require some filtering. See:
//   HcrVelFirFilt
//
//////////////////////////////////////////////////////////////////////////

#include <cmath>
#include <radar/HcrSurfaceVel.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxGeoref.hh>
#include <Radx/RadxTimeList.hh>
using namespace std;

// Constructor

HcrSurfaceVel::HcrSurfaceVel()
  
{

  _debug = false;
  _verbose = false;

  _dbzFieldName = "DBZ";
  _velFieldName = "VEL";

  _maxNadirErrorDeg = 5.0;
  _minRangeToSurfaceKm = 0.5;
  _maxSurfaceHeightKm = 9.0;
  _minDbzForSurfaceEcho = 20.0;
  _nGatesForSurfaceEcho = 1;

}

// destructor

HcrSurfaceVel::~HcrSurfaceVel()

{

}

/////////////////////////////////////////////////////////////////////////
// compute surface velocity
//
// Sets vel to 0.0 if cannot determine velocity.
// Also sets dbzSurf and rangeToSurf.
// Returns 0 on success, -1 on failure.

int HcrSurfaceVel::computeSurfaceVel(const RadxRay *ray,
                                     double &velSurf,
                                     double &dbzSurf,
                                     double &rangeToSurf) const
  
{

  // init
  
  velSurf = 0.0;
  dbzSurf = NAN;
  rangeToSurf = NAN;
  
  // check elevation
  // cannot compute gnd vel if not pointing down
  
  double elev = ray->getElevationDeg();
  if ((elev > -90 + _maxNadirErrorDeg) ||
      (elev < -90 - _maxNadirErrorDeg)) {
    if (_debug) {
      cerr << "Bad elevation for finding surface, time, elev(deg): "
           << ray->getRadxTime().asString() << ", "
           << elev << endl;
    }
    return -1;
  }

  // get dbz field
  
  const RadxField *dbzField = ray->getField(_dbzFieldName);
  if (dbzField == NULL) {
    cerr << "ERROR - HcrSurfaceVel::_computeSurfaceVel" << endl;
    cerr << "  No dbz field found, field name: " << _dbzFieldName << endl;
    return -1;
  }
  const Radx::fl32 *dbzArray = dbzField->getDataFl32();
  Radx::fl32 dbzMiss = dbzField->getMissingFl32();
  
  // get vel field
  
  const RadxField *velField = ray->getField(_velFieldName);
  if (velField == NULL) {
    cerr << "ERROR - HcrSurfaceVel::_computeSurfaceVel" << endl;
    cerr << "  No vel field found, field name: " << _velFieldName << endl;
    return -1;
  }
  const Radx::fl32 *velArray = velField->getDataFl32();
  Radx::fl32 velMiss = velField->getMissingFl32();
  
  // get gate at which max dbz occurs

  const RadxGeoref *georef = ray->getGeoreference();
  double range = dbzField->getStartRangeKm();
  double drange = dbzField->getGateSpacingKm();
  double dbzMax = -9999;
  int gateForMax = -1;
  double rangeSurf = 0;
  bool foundSurface = false;
  for (size_t igate = 0; igate < dbzField->getNPoints(); igate++, range += drange) {
    if (range < _minRangeToSurfaceKm) {
      continue;
    }
    if (georef != NULL) {
      double altitudeKm = georef->getAltitudeKmMsl();
      double gateAlt = altitudeKm - range;
      if (gateAlt > _maxSurfaceHeightKm) {
        continue;
      }
    }
    Radx::fl32 dbz = dbzArray[igate];
    if (dbz != dbzMiss) {
      if (dbz > dbzMax) {
        dbzMax = dbz;
        gateForMax = igate;
        rangeSurf = range;
        foundSurface = true;
      }
    }
  }

  dbzSurf = dbzMax;
  rangeToSurf = rangeSurf;
  
  // check for sufficient power

  if (foundSurface) {
    if (dbzMax < _minDbzForSurfaceEcho) {
      foundSurface = false;
      if (_debug) {
        cerr << "WARNING - HcrSurfaceVel::_computeSurfaceVel" << endl;
        cerr << "  Ray at time: " << ray->getRadxTime().asString() << endl;
        cerr << "  Dbz max not high enough for surface detection: " << dbzMax << endl;
        cerr << "  Range to max dbz: " << rangeToSurf << endl;
      }
    }
  }

  size_t nEachSide = _nGatesForSurfaceEcho / 2;
  if (foundSurface) {
    for (size_t igate = gateForMax - nEachSide;
         igate <= gateForMax + nEachSide; igate++) {
      Radx::fl32 dbz = dbzArray[igate];
      if (dbz == dbzMiss) {
        foundSurface = false;
      }
      if (dbz < _minDbzForSurfaceEcho) {
        foundSurface = false;
      }
    }
  }

  // compute surface vel
  
  if (foundSurface) {
    double sumVelPwr = 0.0;
    double sumPwr = 0.0;
    int count = 0;
    for (size_t igate = gateForMax - nEachSide;
         igate <= gateForMax + nEachSide; igate++) {
      Radx::fl32 dbz = dbzArray[igate];
      if (dbz == velMiss) {
        continue;
      }
      double power = pow(10.0, dbz / 10.0);
      Radx::fl32 vel = velArray[igate];
      if (vel == velMiss) {
        continue;
      }
      sumVelPwr += vel * power;
      sumPwr += power;
      count++;
    }
    if (count < 1) {
      foundSurface = false;
    } else {
      velSurf = sumVelPwr / sumPwr;
    }
  }

  if (foundSurface) {
    return 0;
  } else {
    return -1;
  }

}

