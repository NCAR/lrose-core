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

#ifndef BLOCKAGE_CALC_HH
#define BLOCKAGE_CALC_HH

#include "Params.hh"
#include <vector>
#include <euclid/EuclidAngle.hh>
#include <toolsa/TaArray2D.hh>

class BeamHeight;
class DemProvider;
class BeamPowerPattern;

class BlockageCalc
{
public:

  // constructor
  
  BlockageCalc(const Params &params,
               BeamHeight &beamHt,
               const DemProvider &dem,
               const BeamPowerPattern &pattern);
  
  // destructor
  
  virtual ~BlockageCalc();

  // initialize geometry for computations

  void initGeom(double maxRangeKm,
                double rangeResKm,
                const vector<double> &zCartKm,
                int nBeamPatternEl,
                int nBeamPatternAz);
  
  // set radar location

  void setRadarLoc(double radarLatDeg, double radarLonDeg, double radarHtKm);
  
  // compute fraction blocked for each plane at a specified grid point
  
  int getBlockage(double lat, double lon,
                  double gndRangeKm, double azDeg,
                  vector<double> &fractionBlocked);
  
protected:
  
private:
  
  const Params &_params;
  BeamHeight &_beamHt;
  const DemProvider &_dem;
  const BeamPowerPattern &_pattern;

  double _maxRangeKm;
  double _rangeResKm;
  size_t _nRangeAlloc;
  
  size_t _nZ;
  size_t _nAz;
  size_t _nRange;
  
  vector<double> _rangeKm;
  vector<double> _kmToDeg;
  vector<double> _zCartKm;
  euclid::EuclidAngle _azCenter;
  vector<euclid::EuclidAngle> _patternAz;
  vector<euclid::EuclidAngle> _cartEl;
  TaArray2D<int> _maxElIndexBlocked;

  // radar location

  double _radarLatDeg, _radarLonDeg, _radarHtKm;
  
  // array of attributes in az and range
  // the dimensions will be [nAz][nRange]
  
  class AzRangePoint {
  public:
    double lat;
    double lon;
    double terrainHtKm;
  };
  TaArray2D<AzRangePoint> _azRangePts;

  // fill out the array geometry for a specified grid point
  
  int _initForGridPoint(double lat, double lon,
                        double gndRangeKm, double azDeg);

  // compute the maximum elevation index blocked for each
  // Cartesian height and specified relative az
  
  void _getMaxElIndexBlockedPlane(size_t iaz,
                                  vector<int> &maxElIndexBlockedPlane);
  
};

#endif

