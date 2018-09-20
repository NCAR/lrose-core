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
// HcrVelFirFilt.hh
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2018
//
//////////////////////////////////////////////////////////////////////////
//
// HcrVelFirFilt filters surface velocity as estimated by the class
// HcrSurfaceVel.
//
// It applies FIR filtering to the velocity of the surface echo.
//
//////////////////////////////////////////////////////////////////////////

#ifndef HcrVelFirFilt_HH
#define HcrVelFirFilt_HH

#include <string>
#include <vector>
#include <deque>
#include <Radx/RadxTime.hh>
class RadxRay;
using namespace std;

class HcrVelFirFilt {
  
public:

  // constructor
  
  HcrVelFirFilt();

  // destructor
  
  ~HcrVelFirFilt();

  // set methods

  void setDebug(bool val) { _debug = val; }
  void setVerbose(bool val) { _verbose = val; }

  // initialzie the FIR filters
  // also sets filterType to FIR
  
  void initFirFilters(int stage1FilterLen,
                      const double *filtCoeffStage1,
                      int spikeFilterLen,
                      const double *filtCoeffSpike,
                      int finalFilterLen,
                      const double *filtCoeffFinal);

  void initFirFilters(const vector<double> &filtCoeffStage1,
                      const vector<double> &filtCoeffSpike,
                      const vector<double> &filtCoeffFinal);
  
  void setSpikeFilterDifferenceThreshold(double val) {
    _spikeFilterDifferenceThreshold = val; 
  }
                                 
  // Process an incoming ray, filtering the surface vel.
  // Returns 0 on success, -1 on failure.
  // On success, call getSurfaceVelocity() to get computed surface velocity
  
  int filterRay(RadxRay *ray, 
                double velSurf,
                double dbzSurf,
                double rangeToSurf);

  // get results, on condition that processRay() returns success, i.e. 0
  
  bool velocityIsValid() const { return _velIsValid; }
  RadxRay *getFiltRay() { return _filtRay; }

  // get intermediate fields from filtering step
  // the following return NAN if not available

  double getVelMeasured() const;
  double getVelStage1() const;
  double getVelSpike() const;
  double getVelCond() const;
  double getVelFilt() const;
  double getRangeToSurface() const;
  double getDbzSurf() const;

protected:
private:

  // default FIR filters

  static const int Stage1FilterDefaultLen = 21;
  static const double stage1FilterDefault[Stage1FilterDefaultLen];

  static const int SpikeFilterDefaultLen = 101;
  static const double spikeFilterDefault[SpikeFilterDefaultLen];

  static const int FinalFilterDefaultLen = 41;
  static const double finalFilterDefault[FinalFilterDefaultLen];

  // debugging

  bool _debug;
  bool _verbose;

  // storing incoming rays long enough to compute filtered results

  deque<RadxRay *> _filtRays;
  
  // FIR filtering

  double _spikeFilterDifferenceThreshold;

  vector<double> _filtCoeffStage1;
  vector<double> _filtCoeffSpike;
  vector<double> _filtCoeffFinal;
  
  size_t _lenStage1, _lenStage1Half;
  size_t _lenSpike, _lenSpikeHalf;
  size_t _lenFinal, _lenFinalHalf;

  size_t _condIndex;
  size_t _finalIndex;
  size_t _filtBufLen;

  size_t _nValid;

  double *_dbzSurf;
  double *_rangeToSurf;
  double *_velSurfArray;

  double *_filteredStage1;
  double *_filteredSpike;
  double *_filteredCond;
  double *_filteredFinal;

  // results
  
  bool _velIsValid;
  RadxRay *_filtRay;

  // methods

  void _initFromFirFilters();
  void _applyStage1Filter();
  void _applySpikeFilter();
  void _applyFinalFilter();
  void _computeConditionedValue();
  void _shiftArraysBy1();

};

#endif
