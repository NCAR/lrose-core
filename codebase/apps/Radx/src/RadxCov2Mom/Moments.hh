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
// Moments.hh
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2012
//
///////////////////////////////////////////////////////////////
//
// Moments computation - for multi-threading
// There is one object per thread.
//
///////////////////////////////////////////////////////////////

#ifndef Moments_HH
#define Moments_HH

#include "Params.hh"
#include <radar/RadarMoments.hh>
#include <radar/AlternatingVelocity.hh>
#include <radar/InterestMap.hh>
#include <radar/NoiseLocator.hh>
#include <radar/KdpFilt.hh>
#include <radar/AtmosAtten.hh>
#include <radar/IwrfCalib.hh>
#include <toolsa/TaArray.hh>
class RadxRay;
class RadxField;
#include <pthread.h>
using namespace std;

class Moments {
  
public:

  // constructor
  
  Moments(const Params &params);

  // destructor
  
  ~Moments();

  // compute the moments for given covariance ray
  // storing results in moments ray
  //
  // Creates moments ray and returns it.
  // It must be freed by caller.
  //
  // Returns NULL on error.
  
  RadxRay *compute(const RadxRay *covRay,
                   const IwrfCalib &calib,
                   double measXmitPowerDbmH,
                   double measXmitPowerDbmV,
                   double wavelengthM,
                   double radarHtKm);

  // get noise estimates

  double getMedianNoiseDbmHc() const { return _noise.getMedianNoiseDbmHc(); }
  double getMedianNoiseDbmVc() const { return _noise.getMedianNoiseDbmVc(); }
  double getMedianNoiseDbmHx() const { return _noise.getMedianNoiseDbmHx(); }
  double getMedianNoiseDbmVx() const { return _noise.getMedianNoiseDbmVx(); }
  const NoiseLocator &getNoise() const { return _noise; }

  // get current ray properties
  
  time_t getTimsSecs() const { return _timeSecs; }
  double getNanoSecs() const { return _nanoSecs; }
  double getAzimuthDeg() const { return _azimuth; }
  double getElevationDeg() const { return _elevation; }
  int getNGates() const { return _nGates; }
  double getRadarHtKm() const { return _radarHtKm; }
  double getStartRangeKm() const { return _startRangeKm; }
  double getGateSpacingKm() const { return _gateSpacingKm; }
  double getWavelengthM() const { return _wavelengthM; }
  double getNyquistMps() const { return _nyquist; }
  
  // status
  
  bool OK;

  // define field name for censoring

  static string censorFlagFieldName;

protected:
private:

  const Params &_params;

  // current ray properties
  
  time_t _timeSecs;
  double _nanoSecs;
  double _azimuth;
  double _elevation;
  
  // calibration
  
  IwrfCalib _calib;
  
  // moments computations object

  RadarMoments _rmom;
  AtmosAtten _atmosAtten;
  
  // moments field data
  
  int _nGates;
  double _radarHtKm;
  double _startRangeKm, _gateSpacingKm;
  TaArray<MomentsFields> _momFields;
  AlternatingVelocity _altVel;
  NoiseLocator _noise;
  double _wavelengthM;
  double _nyquist;
  
  // covariances

  class Covars {

  public:
    
    double lag0Hc;
    double lag0Hx;
    double lag0Vc;
    double lag0Vx;

    RadarComplex_t lag1Hc;
    RadarComplex_t lag1Vc;

    RadarComplex_t lag2Hc;
    RadarComplex_t lag2Vc;

    RadarComplex_t lag3Hc;
    RadarComplex_t lag3Vc;

    RadarComplex_t lag0VcHx;
    RadarComplex_t lag0HcVx;
    RadarComplex_t lag0VxHx;
    
    RadarComplex_t lag1VcHc;
    RadarComplex_t lag1HcVc;
    
    RadarComplex_t rvvhh0;

    void init() {

      lag0Hc = 0.0;
      lag0Hx = 0.0;
      lag0Vc = 0.0;
      lag0Vx = 0.0;
      
      lag1Hc.re = 0.0;
      lag1Hc.im = 0.0;
      lag1Vc.re = 0.0;
      lag1Vc.im = 0.0;

      lag2Hc.re = 0.0;
      lag2Hc.im = 0.0;
      lag2Vc.re = 0.0;
      lag2Vc.im = 0.0;

      lag3Hc.re = 0.0;
      lag3Hc.im = 0.0;
      lag3Vc.re = 0.0;
      lag3Vc.im = 0.0;

      lag0VcHx.re = 0.0;
      lag0VcHx.im = 0.0;
      lag0HcVx.re = 0.0;
      lag0HcVx.im = 0.0;
      lag0VxHx.re = 0.0;
      lag0VxHx.im = 0.0;

      lag1VcHc.re = 0.0;
      lag1VcHc.im = 0.0;
      lag1HcVc.re = 0.0;
      lag1HcVc.im = 0.0;

      rvvhh0.re = 0.0;
      rvvhh0.im = 0.0;

    }

  };

  TaArray<Covars> _covars;

  // kdp

  KdpFilt _kdp;

  // input arrays for computing KDP

  TaArray<double> _snrArray_;
  TaArray<double> _dbzArray_;
  TaArray<double> _zdrArray_;
  TaArray<double> _kdpArray_;
  TaArray<double> _ldrArray_;
  TaArray<double> _rhohvArray_;
  TaArray<double> _phidpArray_;

  double *_snrArray;
  double *_dbzArray;
  double *_zdrArray;
  double *_kdpArray;
  double *_ldrArray;
  double *_rhohvArray;
  double *_phidpArray;

  // censoring
  
  TaArray<bool> _censorFlag_;
  bool *_censorFlag;

  // transmit power

  double _measXmitPowerDbmH;
  double _measXmitPowerDbmV;

  // debug printing

  static pthread_mutex_t _debugPrintMutex;

  // private methods
  
  int _getCovariancesSinglePol(const RadxRay *covRay);
  int _getCovariancesDpAltHvCoOnly(const RadxRay *covRay);
  int _getCovariancesDpAltHvCoCross(const RadxRay *covRay);
  int _getCovariancesDpSimHv(const RadxRay *covRay);
  int _getCovariancesDpHOnly(const RadxRay *covRay);
  int _getCovariancesDpVOnly(const RadxRay *covRay);

  void _computeMomSinglePol();
  void _computeMomDpAltHvCoOnly();
  void _computeMomDpAltHvCoCross();
  void _computeMomDpSimHv();
  void _computeMomDpHOnly();
  void _computeMomDpVOnly();

  void _computeNoiseSinglePol(RadxRay *ray);
  void _computeNoiseDpAltHvCoOnly(RadxRay *ray);
  void _computeNoiseDpAltHvCoCross(RadxRay *ray);
  void _computeNoiseDpSimHv(RadxRay *ray);
  void _computeNoiseDpHOnly(RadxRay *ray);
  void _computeNoiseDpVOnly(RadxRay *ray);

  const RadxField *_getField(const RadxRay *covRay,
                             int fieldId,
                             bool required = true);
  
  string _fieldId2Str(int fieldId);

  void _censorByNoiseFlag();
  void _censorBySnrNcp();
  void _censorFields(int gateNum);
  void _despeckleCensoring();
  
  void _loadOutputFields(RadxRay *momRay);
    
  void _setNoiseFields();

  int _convertInterestParamsToVector(const string &label,
                                     const Params::interest_map_point_t *map,
                                     int nPoints,
                                     vector<InterestMap::ImPoint> &pts);

  void _momInit();
  int _noiseInit();

  void _kdpInit();
  void _kdpCompute();

  void _allocMomentsArrays();
  void _loadMomentsArrays();

};

#endif
