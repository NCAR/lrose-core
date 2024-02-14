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
// Cmd.hh
//
// Cmd object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2005
//
///////////////////////////////////////////////////////////////

#ifndef Cmd_hh
#define Cmd_hh

#include <string>
#include <vector>
#include "Params.hh"
#include <radar/InterestMap.hh>
#include <radar/GateData.hh>
#include <radar/ForsytheRegrFilter.hh>

using namespace std;

////////////////////////
// This class

class Cmd {
  
public:

  // constructor

  Cmd(const string &prog_name,
      const Params &params,
      vector<GateData *> &gate_data);
  
  // destructor
  
  ~Cmd();

  // set range geometry

  void setRangeGeometry(double startRangeKm, double gateSpacingKm) {
    _startRangeKm = startRangeKm;
    _gateSpacingKm = gateSpacingKm;
  }

  // set the transmit/receive mode

  void setXmitRcvMode(iwrf_xmit_rcv_mode_t val) { _xmitRcvMode = val; }

  // Create interest maps.
  // These are static on the class, and should be created before any
  // beams are constructed.
 
  int createInterestMaps(const Params &params);

  // set number of samples in dwell

  void setNSamples(int val) { _nSamples = val; }
  
  // set the number of samples assuming a rectangular window
  // this will be fewer than nSamples if the window in use
  // is not rectangular
  
  void setNSamplesRect(int val) { _nSamplesRect = val; }
  
  // compute CMD
  
  void compute(int nGates, const RadarMoments *mom,
               bool useDualPol, bool useRhohvTest);

protected:
  
private:

  string _progName;
  const Params &_params;

  // gate data vector - belongs to the MomentsMgr which created this class
  
  iwrf_xmit_rcv_mode_t _xmitRcvMode;
  vector<GateData *> &_gateData;

  // range geometry

  double _startRangeKm;
  double _gateSpacingKm;

  // interest maps

  InterestMap *_tdbzInterestMap;
  InterestMap *_spinInterestMap;
  InterestMap *_cpaInterestMap;
  InterestMap *_zdrSdevInterestMap;
  InterestMap *_phidpSdevInterestMap;
  InterestMap *_rhohvTestInterestMap;

  // phidp computations

  class PhidpState {
  public:
    void init(double missingValue) {
      missing = true;
      phidp = missingValue;
      xx = 0.0;
      yy = 0.0;
      meanxx = 0.0;
      meanyy = 0.0;
      phidpMean = missingValue;
      phidpSdev = missingValue;
    }
    bool missing;
    double phidp;
    double xx;
    double yy;
    double meanxx;
    double meanyy;
    double phidpMean;
    double phidpSdev;
  };

  vector<PhidpState> _phidpStates;
  bool _phidpFoldsAt90;
  double _phidpFoldVal, _phidpFoldRange;

  // regression filter for rhohv test
  
  ForsytheRegrFilter _regr;

  // number of samples
  
  int _nSamples;
  int _nSamplesRect;
  
  // private functions
  
  int _createInterestMaps(const Params &params);
  void _prepareForTdbzAndSpin(int nGates);
  void _computeTdbz(int nGates);
  void _computeSpin(int nGates);
  void _computeMaxTdbzAndSpinInterest(int nGates);
  void _computeZdrSdev(int nGates);
  void _computePhidpSdevOld(int nGates);
  void _computePhidpSdevNew(int nGates);
  void _computePhidpFoldingRange(int nGates);
  void _computeRhohvTest(const RadarMoments *mom, int nGates);
  void _applyRegrFiltFixed(const vector<RadarComplex_t> &iq,
                           vector<RadarComplex_t> &filt);
  void _applyRegrFiltStag(const vector<RadarComplex_t> &iq,
                          vector<RadarComplex_t> &filt);
  int _convertInterestMapToVector(const string &label,
                                  const Params::interest_map_point_t *map,
                                  int nPoints,
                                  vector<InterestMap::ImPoint> &pts);
  void _applyGapFilter(int nGates);
  void _applyGapFilterCVersion(int nGates);
  void _applySpeckleFilterCVersion(int nGates);
  void _applyInfillFilter(int nGates);
  void _applySpeckleFilter(int nGates);
  void _runSpeckleFilter(int nGates, int minRunLen, double modThreshold);
  void _applyNexradSpikeFilter(int nGates);
};

#endif

