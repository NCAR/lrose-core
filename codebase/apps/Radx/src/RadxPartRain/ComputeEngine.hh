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
// ComputeEngine.hh
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2012
//
///////////////////////////////////////////////////////////////
//
// ComputeEngine computation - for multi-threading
// There is one object per thread.
//
///////////////////////////////////////////////////////////////

#ifndef ComputeEngine_HH
#define ComputeEngine_HH

#include "Params.hh"
#include <radar/KdpFilt.hh>
#include <radar/KdpBringi.hh>
#include <radar/PrecipRate.hh>
#include <radar/NcarParticleId.hh>
#include <radar/TempProfile.hh>
#include <radar/AtmosAtten.hh>
#include <Radx/RadxArray.hh>
#include <Radx/RadxTime.hh>
class RadxRay;
class RadxField;
#include <pthread.h>
using namespace std;

class ComputeEngine {
  
public:
  
  // struct for self-consistency results

  typedef struct {
    RadxTime rtime;
    double elevation;
    double azimuth;
    int runStart;
    int runEnd;
    double dbzCorrection;
    double zdrCorrection;
    double rangeStart;
    double rangeEnd;
    double accumObs;
    double accumEst;
    double dbzBias;
    double accumCorrelation;
  } self_con_t;

  // constructor
  
  ComputeEngine(const Params &params, int id);

  // destructor
  
  ~ComputeEngine();

  // compute the moments for given covariance ray
  // storing results in moments ray
  //
  // Creates moments ray and returns it.
  // It must be freed by caller.
  //
  // Returns NULL on error.
  
  RadxRay *compute(RadxRay *covRay,
                   double radarHtKm,
                   double wavelengthM,
                   const TempProfile *tempProfile);

  // after calling compute, retrieve the zdrm bias array
  
  const vector<double> &getZdrInIceResults() const { return _zdrInIceResults; }
  const vector<double> &getZdrInBraggResults() const { return _zdrInBraggResults; }
  const vector<double> &getZdrmInIceResults() const { return _zdrmInIceResults; }
  const vector<double> &getZdrmInBraggResults() const { return _zdrmInBraggResults; }

  // after calling compute, retrieve the self consistency results

  const vector<self_con_t> &getSelfConResults() const { return _selfConResults; }

  bool OK;
  
protected:
private:

  static const double missingDbl;
  const Params &_params;
  int _id;

  // current ray properties
  
  time_t _timeSecs;
  double _nanoSecs;
  double _azimuth;
  double _elevation;

  // radar properties

  double _radarHtKm;
  double _wavelengthM;

  // moments field data
  
  size_t _nGates;
  double _startRangeKm, _gateSpacingKm;
  double _nyquist;
  
  // input arrays for moments

  RadxArray<double> _snrArray_;
  RadxArray<double> _dbzArray_;
  RadxArray<double> _velArray_;
  RadxArray<double> _zdrArray_;
  RadxArray<double> _zdrmArray_;
  RadxArray<double> _zdpArray_;
  RadxArray<double> _kdpArray_;
  RadxArray<double> _kdpZZdrArray_;
  RadxArray<double> _kdpCondArray_;
  RadxArray<double> _kdpBringiArray_;
  RadxArray<double> _kdpConstrainedArray_;
  RadxArray<double> _kdpWithPsobArray_;
  RadxArray<double> _ldrArray_;
  RadxArray<double> _rhohvArray_;
  RadxArray<double> _rhohvNncArray_;
  RadxArray<double> _phidpArray_;
  RadxArray<double> _rhoVxHxArray_;

  double *_snrArray;
  double *_dbzArray;
  double *_velArray;
  double *_zdrArray;
  double *_zdrmArray;
  double *_zdpArray;
  double *_kdpArray;
  double *_kdpZZdrArray;
  double *_kdpCondArray;
  double *_kdpBringiArray;
  double *_ldrArray;
  double *_rhohvArray;
  double *_rhohvNncArray;
  double *_phidpArray;
  double *_rhoVxHxArray;

  // arrays for computing KDP, PID and PRECIP

  RadxArray<double> _rateZ_;
  RadxArray<double> _rateZSnow_;
  RadxArray<double> _rateZZdr_;
  RadxArray<double> _rateKdp_;
  RadxArray<double> _rateKdpZdr_;
  RadxArray<double> _rateHybrid_;
  RadxArray<double> _ratePid_;
  RadxArray<double> _rateHidro_;
  RadxArray<double> _rateBringi_;

  RadxArray<int> _pidArray_;
  RadxArray<int> _pidArray2_;
  RadxArray<double> _pidInterest_;
  RadxArray<double> _pidInterest2_;
  RadxArray<double> _tempForPid_;

  double *_rateZ;
  double *_rateZSnow;
  double *_rateZZdr;
  double *_rateKdp;
  double *_rateKdpZdr;
  double *_rateHybrid;
  double *_ratePid;
  double *_rateHidro;
  double *_rateBringi;

  int *_pidArray;
  int *_pidArray2;
  double *_pidInterest;
  double *_pidInterest2;
  double *_tempForPid;

  // atmospheric attenuation

  AtmosAtten _atmos;

  // kdp

  KdpFilt _kdp;
  KdpBringi _kdpBringi;

  // pid

  NcarParticleId _pid;
  const TempProfile *_tempProfile;

  // precip rate

  PrecipRate _rate;

  // ZDR bias
  
  RadxArray<double> _zdrInIce_;
  double *_zdrInIce;
  RadxArray<double> _zdrInBragg_;
  double *_zdrInBragg;

  RadxArray<double> _zdrmInIce_;
  double *_zdrmInIce;
  RadxArray<double> _zdrmInBragg_;
  double *_zdrmInBragg;
  
  RadxArray<int> _zdrFlagInIce_;
  int *_zdrFlagInIce;
  RadxArray<int> _zdrFlagInBragg_;
  int *_zdrFlagInBragg;

  vector<double> _zdrInIceResults;
  vector<double> _zdrInBraggResults;
  vector<double> _zdrmInIceResults;
  vector<double> _zdrmInBraggResults;

  // self consistency

  int _selfConMinNoGapNGates;
  int _selfConMinCombinedNGates;
  vector<self_con_t> _selfConResults;

  RadxArray<double> _kdpFromFilt_;
  double *_kdpFromFilt;

  RadxArray<double> _kdpEst_;
  double *_kdpEst;

  // debug printing

  static pthread_mutex_t _debugPrintMutex;

  // private methods
  
  const RadxField *_getField(const RadxRay *inputRay,
                             const string &fieldName);
  
  void _loadOutputFields(RadxRay *inputRay,
                         RadxRay *derivedRay);
    
  void _kdpInit();
  void _kdpCompute();

  int _pidInit();
  void _pidCompute();
  
  void _precipInit();
  void _precipCompute();

  void _allocMomentsArrays();
  void _allocPidArrays();
  void _allocPrecipArrays();
  void _allocZdrBiasArrays();
  void _allocSelfConArrays();

  int _loadMomentsArrays(RadxRay *inputRay);
  int _loadFieldArray(RadxRay *inputRay,
                      const string &fieldName,
                      bool required,
                      double *array);

  void _computeZdpArray();

  void _computeSnrFromDbz();

  void _censorNonPrecip(RadxField &field);

  void _accumForZdrBiasInIce();
  void _accumForZdrBiasInBragg();

  void _runSelfConsistencyCheck();
  void _doCheckSelfConsistency(int gateStart, int gateEnd);

  double _computeKdpPowerLaw(double zz,
                             double zdrLin,
                             double aa, double bb, double cc);
  
  double _computeKdpPolynomial(double zz,
                               double zdrDb,
                               double a0, double a1, double a2, double a3);
  
  void _writeSelfConRunDataToFile(int runStart,
                                  int runEnd,
                                  double rangeStart,
                                  double rangeEnd,
                                  double phidpAccumObs,
                                  double phidpAccumEst,
                                  double dbzBias,
                                  double accumCorrelation,
                                  const double *dbzCorr,
                                  const double *zdrCorr,
                                  const double *zdrTerm,
                                  const double *phidpEst);

  double _getPlotVal(double val, double valIfMissing);

};

#endif
