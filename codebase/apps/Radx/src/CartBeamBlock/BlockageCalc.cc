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
///////////////////////////////////////////////////////////////
//
// BlockageCalc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2026
//
///////////////////////////////////////////////////////////////
//
// Compute the blockage up to a Cartesian (x,y) grid point
// for each of the Cartesian levels.
//
// We use one of these objects per compute thread.
//
///////////////////////////////////////////////////////////////

#include <toolsa/pjg_flat.h>
#include <radar/BeamHeight.hh>
#include "BlockageCalc.hh"
#include "DemProvider.hh"
#include "BeamPowerPattern.hh"
using namespace euclid;

///////////////////////////////////////////////////////////////////////////
// constructor

BlockageCalc::BlockageCalc(const Params &params,
                           const DemProvider &dem,
                           const BeamPowerPattern &pattern) :
        _params(params),
        _dem(dem),
        _pattern(pattern)
{
  
}

///////////////////////////////////////////////////////////////////////////
// destructor


BlockageCalc::~BlockageCalc(void)
{

}

// initialize geometry for computations

void BlockageCalc::initGeom(double maxRangeKm,
                            double rangeResKm,
                            const vector<double> &zCartKm,
                            int nBeamPatternEl,
                            int nBeamPatternAz)

{

  // initialize values
  
  _maxRangeKm = maxRangeKm;
  _rangeResKm = rangeResKm;
  _nRangeAlloc = (int) ((_maxRangeKm / _rangeResKm) + 1);
  _zCartKm = zCartKm;
  _nZ = _zCartKm.size();
  _nAz = _pattern.getNAz();
  
  _rangeKm.resize(_nRangeAlloc);
  _kmToDeg.resize(_nRangeAlloc);
  for (size_t ii = 0; ii < _nRangeAlloc; ii++) {
    _rangeKm[ii] = _rangeResKm * (ii + 0.5);
    _kmToDeg[ii] = (1.0 / _rangeKm[ii]) * (180.0 / M_PI);
  }
  _nRange = _nRangeAlloc;
  
  // create 2D points array
  
  _azRangePts.clear();
  _azRangePts.alloc(_nAz, _nRangeAlloc);
  
  _patternAz.resize(_pattern.getNAz());
  _cartEl.resize(_nZ);

  _maxElIndexBlocked.alloc(_nZ, _nAz);
  _maxElIndexBlockedPlane.resize(_nZ);
  _blockedBelow.resize(_nRange);

}

//////////////////////////////////////////////////////////////////////////
// set radar location

void BlockageCalc::setRadarLoc(double radarLatDeg,
                               double radarLonDeg,
                               double radarHtKm)

{

  _radarLatDeg = radarLatDeg;
  _radarLonDeg = radarLonDeg;
  _radarHtKm = radarHtKm;
  
  _beamHt.setInstrumentHtKm(_radarHtKm);
  if (_params.override_standard_pseudo_earth_radius) {
    _beamHt.setPseudoRadiusRatio(_params.pseudo_earth_radius_ratio);
  }

}

//////////////////////////////////////////////////////////////////////////
// compute fraction blocked for each plane at a specified grid point

int BlockageCalc::getBlockage(double lat, double lon,
                              double gndRangeKm, double azDeg,
                              vector<double> &fractionBlocked)
  
{

  fractionBlocked.resize(_nZ);
  
  // initialize geometry for this cart point
      
  if (_initForGridPoint(lat, lon, gndRangeKm, azDeg)) {
    cerr << "ERROR - BlockageCalc::getBlockage" << endl;
    return -1;
  }

  // loop through the azimuths in the pattern
  
  for (size_t iaz = 0; iaz < _nAz; iaz++) {

    // compute the max index blocked at each plane
    
    std::fill(_maxElIndexBlockedPlane.begin(),
              _maxElIndexBlockedPlane.end(), -1);
    _getMaxElIndexBlockedPlane(iaz, _maxElIndexBlockedPlane);

    // load up 2D array of blocked indexes
    
    for (size_t iz = 0; iz < _nZ; iz++) {
      _maxElIndexBlocked[iz][iaz] = _maxElIndexBlockedPlane[iz];
    } // iz

  } // iaz

  // sum up the pattern blockages for each plane

  for (size_t iz = 0; iz < _nZ; iz++) {

    double sumFraction = 0.0;
    for (size_t iaz = 0; iaz < _nAz; iaz++) {
      int elIndexBlocked = _maxElIndexBlocked[iz][iaz];
      double frac = _pattern.getPower(elIndexBlocked, iaz);
      sumFraction += frac;
    }
    
    fractionBlocked[iz] = sumFraction;
    
  } // iz

  return 0;
  
}

//////////////////////////////////////////////////////////////////////////
// fill out the array geometry for a specified grid point

int BlockageCalc::_initForGridPoint(double lat, double lon,
                                    double gndRangeKm, double azDeg)
  
{

  // determine how many range intervals to use
  
  if (gndRangeKm > _maxRangeKm) {
    cerr << "ERROR - BlockageCalc::calcGeom" << endl;
    cerr << "  Target point range: " << gndRangeKm << endl;
    cerr << "    exceeds max range: " << _maxRangeKm << endl;
    return -1;
  }
  _nRange = (int) ((gndRangeKm / _rangeResKm) + 0.5);
  
  ////////////////////
  // compute geometry

  // center azimuth
  
  _azCenter = EuclidAngle::fromDegrees(azDeg);
  
  // pattern azimuths
  
  for (size_t iaz = 0; iaz < _nAz; iaz++) {
    _patternAz[iaz] = _azCenter + _pattern.getAzimuthOffset(iaz);
  } // iaz

  // elevation for each Cartesian plane
  
  for (size_t iz = 0; iz < _nZ; iz++) {
    double elDeg = _beamHt.computeElevationDeg(_zCartKm[iz], gndRangeKm);
    _cartEl[iz] = EuclidAngle::fromDegrees(elDeg);
  }

  // lat/lon and terrain ht
  
  for (size_t iaz = 0; iaz < _nAz; iaz++) {
    for (size_t irange = 0; irange < _nRange; irange++) {
      AzRangePoint &pt = _azRangePts[iaz][irange];

      // location
      
      double lat, lon;
      PJGLatLonPlusRTheta(_radarLatDeg, _radarLonDeg,
                          _rangeKm[irange], _patternAz[iaz].degrees(),
                          &lat, &lon);
      pt.lat = lat;
      pt.lon = lon;

      // terrain height

      pt.terrainHtKm = _dem.getTerrainHtM(lat, lon) / 1000.0;
      
    } // iaz
  } // irange

  return 0;

}

//////////////////////////////////////////////////////////////////////////
// compute the maximum elevation index blocked for a pattern azimuth

void BlockageCalc::_getMaxElIndexBlockedPlane(size_t iaz,
                                              vector<int> &maxElIndexBlockedPlane)
  
{

  // initialize flag to indicate whether the Z plane below is blocked
  
  std::fill(_blockedBelow.begin(), _blockedBelow.end(), true);
  
  // loop through increasing elevations, for each cart height

  for (size_t iz = 0; iz < _nZ; iz++) {

    double cartHtRelKm = _zCartKm[iz] - _radarHtKm;
    double elDeg = _cartEl[iz].degrees();
    int maxElIndexInRange = -1;
    
    // loop through increasing range
    
    for (size_t irange = 0; irange < _nRange; irange++) {

      // check if height below was not blocked
      // if not, then this one cannot be blocked either
      // so we can short-circuit this logic

      if (!_blockedBelow[irange]) {
        continue;
      }
      
      // compute elevation angle for this Cart height at this range
      
      double gndRangeKm = _rangeKm[irange];
      double slantRangeKm = sqrt(gndRangeKm * gndRangeKm + cartHtRelKm * cartHtRelKm);

      // get beam height 

      double beamHtKm = _beamHt.computeHtKm(elDeg, slantRangeKm);
      double terrainHtKm = _azRangePts[iaz][irange].terrainHtKm;

      double terrainRelHtKm = terrainHtKm - beamHtKm;
      double terrainRelDeg = terrainRelHtKm * _kmToDeg[irange];

      int terrainElIndex = (_pattern.getNEl() / 2) +
        (int) std::round(terrainRelDeg / _pattern.getDeltaEl().degrees());

      int maxIndexBlocked = -1;
      if (terrainElIndex < 0) {
        maxIndexBlocked = -1;
        _blockedBelow[irange] = false;
      } else if (terrainElIndex > (int) _pattern.getNEl() - 1) {
        maxIndexBlocked = _pattern.getNEl() - 1;
      } else {
        maxIndexBlocked = terrainElIndex;
      }

      if (maxIndexBlocked >= 0) {
        maxElIndexInRange = std::max(maxElIndexInRange, maxIndexBlocked);
      }
      
    } // irange

    maxElIndexBlockedPlane[iz] = std::max(maxElIndexBlockedPlane[iz], maxElIndexInRange);
  
  } // iz
    
}
