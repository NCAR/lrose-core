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
                           BeamHeight &beamHt,
                           const DemProvider &dem,
                           const BeamPowerPattern &pattern) :
        _params(params),
        _beamHt(beamHt),
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
  _nAz = _pattern.getNAz();
  
  _rangeKm.resize(_nRangeAlloc);
  for (size_t ii = 0; ii < _nRangeAlloc; ii++) {
    _rangeKm[ii] = _rangeResKm * (ii + 0.5);
  }
  _nRange = _nRangeAlloc;
  
  // create 2D points array
  
  _azRangePts.clear();
  _azRangePts.alloc(_nAz, _nRangeAlloc);
  for (size_t iaz = 0; iaz < _nAz; iaz++) {
    for (size_t irange = 0; irange < _nRangeAlloc; irange++) {
      AzRangePoint &pt = _azRangePts[iaz][irange];
      pt.fracBlocked.resize(_zCartKm.size());
    } // iaz
  } // irange
  
  _patternAz.resize(_pattern.getNAz());
  _cartEl.resize(_zCartKm.size());
  
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
  
}

//////////////////////////////////////////////////////////////////////////
// fill out the array geometry for a specified grid point

int BlockageCalc::initForGridPoint(double lat, double lon,
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
  
  for (size_t iz = 0; iz < _zCartKm.size(); iz++) {
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

void BlockageCalc::computeMaxElIndexBlocked(size_t iaz)
  
{

  // loop through increasing elevations, for each cart height

  for (size_t iz = 0; iz < _zCartKm.size(); iz++) {

    // loop through increasing range
    
    for (size_t irange = 0; irange < _nRange; irange++) {
      
      double gndRangeKm = _rangeKm[irange];
      double cartHtKm = _zCartKm[iz];
      double slantRangeKm = sqrt(gndRangeKm * gndRangeKm + cartHtKm * cartHtKm);
      double elDeg = _cartEl[iz].degrees();

      double beamHtKm = _beamHt.computeHtKm(elDeg, slantRangeKm);
      double terrainHtKm = _azRangePts[iaz][irange].terrainHtKm;

      double excessHtKm = beamHtKm / terrainHtKm;
      // double excessDeg = (excessHtKm / slantRangeKm) * RAD_TO_DEG;

    } // irange
  
  } // iz
    
}

  
  
