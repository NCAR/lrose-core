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
// RlanLocator.hh
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2016
//
///////////////////////////////////////////////////////////////
//
// Find gates contaminated with RLAN interference
// in dual pol data
//
///////////////////////////////////////////////////////////////

#ifndef RlanLocator_H
#define RlanLocator_H

#include <pthread.h>
#include <vector>
#include <radar/InterestMap.hh>
#include <toolsa/TaArray.hh>
using namespace std;

class RlanLocator {
  
public:

  // constructor
  
  RlanLocator();

  // destructor
  
  ~RlanLocator();

  // set debugging state

  void setDebug(bool state) { _debug = state; }

  //////////////////////////////////////////////////////
  // set parameters for rlan location

  // set kernel size for rlan location
  
  void setNGatesKernel(int val) {
    _nGatesKernel = val;
  }

  // Set interest maps for identifying rlan
  
  void setInterestMapPhaseChangeErrorForRlan
    (const vector<InterestMap::ImPoint> &pts,
     double weight);
  
  void setInterestMapSnrSdevForRlan
    (const vector<InterestMap::ImPoint> &pts,
     double weight);
  
  void setInterestMapNcpMeanForRlan
    (const vector<InterestMap::ImPoint> &pts,
     double weight);
  
  void setInterestThresholdForRlan(double val);
  
  ////////////////////////////////////////////////////////////////
  // set min ngates for computing median

  void setMinNGatesRayMedian(int minNGatesRayMedian);

  ///////////////////////////////////////////////////////////////
  // set the ray properties
  // must be called before locate()
  // Set the nyquist if it is known.
  
  void setRayProps(time_t timeSecs, 
                   double nanoSecs,
                   double elevation, 
                   double azimuth,
                   int nGates,
                   double startRangeKm,
                   double gateSpacingKm,
                   double wavelengthM,
                   double nyquist = -9999.0);

  ///////////////////////////////////////////////////////////////
  // set the available fields
  // if field is not available, set to NULL
  // must be called before locate()
  
  void setFields(double *snr,
                 double *vel,
                 double *width,
                 double *ncp,
                 double *zdr,
                 double missingVal);
  
  //////////////////////////////////////////////
  // perform rlan location
  //
  // mfields: array of Moments Fields computed before
  //          calling this method
  //
  // Must call setRayProps first
  //
  // The following must be set in mfields prior to calling:
  //
  //   phase_for_rlan
  //   snr_for_rlan
  //   ncp
  
  void locate();

  // get results - after running locate
  // these arrays span the gates from 0 to nGates-1

  const bool *getRlanFlag() const { return _rlanFlag; }
  
  const double *getAccumPhaseChange() const { return _accumPhaseChange; }
  const double *getPhaseChangeError() const { return _phaseChangeError; }

  const double *getSnr() const { return _snr; }
  const double *getVel() const { return _vel; }
  const double *getWidth() const { return _width; }
  const double *getNcp() const { return _ncp; }
  const double *getZdr() const { return _zdr; }

  const double *getPhase() const { return _phase; }
  const double *getSnrMode() const { return _snrMode; }
  const double *getSnrDMode() const { return _snrDMode; }
  const double *getZdrMode() const { return _zdrMode; }
  const double *getZdrDMode() const { return _zdrDMode; }
  const double *getNcpMean() const { return _ncpMean; }

  ////////////////////////////////////
  // print parameters for debugging
  
  void printParams(ostream &out);

protected:
private:

  // debugging
  
  bool _debug;

  // ray properties

  time_t _timeSecs;
  double _nanoSecs;
  double _elevation;
  double _azimuth;
  int _nGates;
  double _startRangeKm;
  double _gateSpacingKm;
  double _wavelengthM;
  double _nyquist;

  // arrays for input and computed data
  // and pointers to those arrays

  double _missingVal;

  TaArray<double> _snr_;
  double *_snr;
  bool _snrAvail;
  
  TaArray<double> _vel_;
  double *_vel;
  bool _velAvail;
  
  TaArray<double> _phase_;
  double *_phase;
  bool _phaseAvail;
  
  TaArray<double> _width_;
  double *_width;
  bool _widthAvail;

  TaArray<double> _ncp_;
  double *_ncp;
  bool _ncpAvail;

  TaArray<double> _zdr_;
  double *_zdr;
  bool _zdrAvail;
  
  // results

  TaArray<bool> _rlanFlag_;
  bool *_rlanFlag;

  TaArray<double> _accumPhaseChange_;
  double *_accumPhaseChange;

  TaArray<double> _phaseChangeError_;
  double *_phaseChangeError;

  TaArray<double> _snrMode_;
  double *_snrMode;

  TaArray<double> _snrDMode_;
  double *_snrDMode;

  TaArray<double> _zdrMode_;
  double *_zdrMode;

  TaArray<double> _zdrDMode_;
  double *_zdrDMode;

  TaArray<double> _ncpMean_;
  double *_ncpMean;

  int _minNGatesRayMedian;

  // gate limits for computing stats along a ray

  vector<size_t> _startGate;
  vector<size_t> _endGate;

  // fuzzy logic for rlan detection

  int _nGatesKernel;

  InterestMap *_interestMapPhaseChangeErrorForRlan;
  InterestMap *_interestMapSnrSdevForRlan;
  InterestMap *_interestMapNcpMeanForRlan;
  double _weightPhaseChangeErrorForRlan;
  double _weightSnrSdevForRlan;
  double _weightNcpMeanForRlan;
  double _interestThresholdForRlan;

  // private methods
  
  double _computePhaseChangeError(int startGate, int endGate);
  void _computeDeltaMode(const string &fieldName,
                         const double *vals, double *mode, double *dMode);
  void _computeSdevInRange(const double *vals, double *sdevs);
  void _computeMeanInRange(const double *vals, double *means);
  double _computeMean(const vector<double> &vals);
  double _computeMedian(const vector<double> &vals);
  void _createDefaultInterestMaps();

};

#endif
