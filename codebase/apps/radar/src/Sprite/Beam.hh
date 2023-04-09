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
// Beam.hh
//
// Beam object
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2019
//
///////////////////////////////////////////////////////////////

#ifndef Beam_hh
#define Beam_hh

#include <pthread.h>
#include <string>
#include <vector>
#include <deque>
#include <cstdio>
#include <toolsa/DateTime.hh>
#include <radar/RadarMoments.hh>
#include <radar/RadarComplex.hh>
#include <radar/IwrfTsInfo.hh>
#include <radar/IwrfTsPulse.hh>
#include <radar/GateData.hh>
#include <radar/MomentsFields.hh>
#include <radar/KdpFilt.hh>

#include "Params.hh"

using namespace std;

////////////////////////
// This class

class Beam {
  
public:

  // Constructor
  
  Beam(const string &progName,
       const Params &params);
  
  // copy constructor
  
  Beam(const Beam &rhs);

  // Assignment.
  
  Beam& operator=(const Beam &rhs);
  
  // initialize the pulse sequence
  
  void setPulses(bool isRhi,
                 int nSamples,
                 int nGates,
                 int nGatesPrtLong,
                 bool beamIsIndexed,
                 double angularResolution,
                 bool isAlternating,
                 bool isStagPrt,
                 double prt,
                 double prtLong,
                 const IwrfTsInfo &opsInfo,
                 const deque<const IwrfTsPulse *> &pulses);

  // destructor
  
  ~Beam();

  // compute moments
  
  int computeMoments(Params::fft_window_t windowType
                     = Params::FFT_WINDOW_VONHANN);

  // set methods

  void setCalib(const IwrfCalib &calib);
  
  // get methods

  int getNSamples() const { return _nSamples; }
  int getNSamplesHalf() const { return _nSamplesHalf; }
  
  double getEl() const { return _el; }
  double getAz() const { return _az; }
  double getTargetAngle() const { return _targetAngle; }
  double getPrt() const { return _prt; }
  bool getIsStagPrt() const { return _isStagPrt; }
  int getStagM() const { return _stagM; }
  int getStagN() const { return _stagN; }
  double getPrtShort() const { return _prtShort; }
  double getPrtLong() const { return _prtLong; }
  double getPulseWidth() const { return _pulseWidth; }
  double getNyquist() const { return _nyquist; }

  double getMeasXmitPowerDbmH() const { return _measXmitPowerDbmH; }
  double getMeasXmitPowerDbmV() const { return _measXmitPowerDbmV; }
  
  time_t getTimeSecs() const { return _timeSecs; }
  time_t getNanoSecs() const { return _nanoSecs; }
  const DateTime &getTime() const { return _time; }
  
  int getScanMode() const;
  int getSweepNumber() const { return _sweepNum; }
  int getVolumeNumber() const { return _volNum; }

  bool getBeamIsIndexed() const { return _beamIsIndexed; }
  double getAngularResolution() const { return _angularResolution; }

  bool getIsAlternating() const { return _isAlternating; }

  int getNGates() const { return _nGates; }
  int getNGatesOut() const { return _nGatesOut; }
  int getNGatesPrtShort() const { return _nGatesPrtShort; }
  int getNGatesPrtLong() const { return _nGatesPrtLong; }

  int getGateNum(double range) const;
  double getRange(int gateNum) const;
  double getClosestRange(double range, int &gateNum) const;
  double getMaxRange() const;

  double getStartRangeKm() const { return _startRangeKm; }
  double getGateSpacingKm() const { return _gateSpacingKm; }

  iwrf_xmit_rcv_mode_t getXmitRcvMode() const { return _xmitRcvMode; }
  bool getIsDualPol() const { return _dualPol; }
  bool getIsSwitchingReceiver() const { return _switchingReceiver; }

  inline double getUnambigRange() const { return _unambigRange; }

  const IwrfTsInfo &getInfo() const { return _opsInfo; }

  const MomentsFields* getOutFields() const { return _outFields; }
  const MomentsFields* getOutFieldsF() const { return _outFieldsF; }

  const IwrfCalib &getCalib() const { return _calib; }

  const iwrf_platform_georef_t &getGeoref() const { return _georef; }
  bool getGeorefActive() const { return _georefActive; }

  const vector<GateData *> getGateData() const { return _gateData; }
  inline const fl32* *getIqChan0() const { return _iqChan0; }
  inline const fl32* *getIqChan1() const { return _iqChan1; }

  const RadarMoments *getMoments() { return _mom; }

  double getAntennaRate();
  int getRegrOrder();

  const vector<RadarComplex_t> &getDelta12() const { return _txDelta12; }
  const vector<double> &getPhaseDiffs() const { return _phaseDiffs; }
  const vector<IwrfTsPulse::burst_phase_t> &getBurstPhases() const
  {
    return  _burstPhases;
  }

protected:
  
private:

  static const double _missingDbl;

  string _progName;
  const Params &_params;

  // pulse vector

  deque<const IwrfTsPulse *> _pulses;

  // scan mode

  bool _isRhi;
  
  // number of samples

  int _nSamples;
  int _nSamplesHalf;
  int _nSamplesAlloc;

  // number of gates, gate geometry
  
  int _nGates; // input data
  int _nGatesOut; // output
  int _nGatesOutAlloc; // output

  // time and location

  time_t _timeSecs;
  int _nanoSecs;
  DateTime _time;

  double _el;
  double _az;
  double _targetAngle;

  iwrf_scan_mode _scanMode;
  iwrf_xmit_rcv_mode_t _xmitRcvMode;
  int _sweepNum;
  int _volNum;

  // range geometry

  double _startRangeKm;
  double _gateSpacingKm;

  // platform georeferencing - for moving platforms

  iwrf_platform_georef_t _georef;
  bool _georefActive;

  // atmospheric attenuation

  // const AtmosAtten *_atmosAtten;

  // modes

  bool _beamIsIndexed;
  double _angularResolution;
  bool _isAlternating;
  bool _dualPol;
  bool _switchingReceiver;

  // prt and pulse width

  double _prt;
  double _nyquist;
  double _pulseWidth;
  double _unambigRange;

  // staggered PRT mode

  bool _isStagPrt;
  double _prtShort;
  double _prtLong;
  int _nGatesPrtShort;
  int _nGatesPrtLong;
  int _nGatesStagPrt;
  double _nyquistPrtShort;
  double _nyquistPrtLong;
  int _stagM;
  int _stagN;

  // measured xmit power

  double _measXmitPowerDbmH;
  double _measXmitPowerDbmV;
  
  // manager for computing moments

  IwrfTsInfo _opsInfo;
  double _wavelengthM;

  // calibration

  IwrfCalib _calib;

  // Moments computations
  
  RadarMoments *_mom;

  // window

  Params::fft_window_t _fftWindowType;
  double *_window;
  double *_windowHalf;
  
  // R values, at various lags, for the windows
  // used to correct R values used in width computations

  double _windowR1;
  double _windowR2;
  double _windowR3;
  double _windowHalfR1;
  double _windowHalfR2;
  double _windowHalfR3;

  // KDP

  KdpFilt _kdp;

  TaArray<double> _snrArray_;
  TaArray<double> _dbzArray_;
  TaArray<double> _zdrArray_;
  TaArray<double> _rhohvArray_;
  TaArray<double> _phidpArray_;

  double *_snrArray;
  double *_dbzArray;
  double *_zdrArray;
  double *_rhohvArray;
  double *_phidpArray;

  // moments output fields at each gate

  TaArray<MomentsFields> _outFields_;
  MomentsFields *_outFields;

  TaArray<MomentsFields> _outFieldsF_;
  MomentsFields *_outFieldsF;

  // internal fields for moments computations
  
  TaArray<MomentsFields> _compFields_;
  MomentsFields *_compFields;

  TaArray<MomentsFields> _compFieldsF_;
  MomentsFields *_compFieldsF;

  // gate data - internal to object

  bool _haveChan1;
  vector<GateData *> _gateData;
  
  // channel data

  TaArray<const fl32 *> _iqChan0_;
  const fl32* *_iqChan0;

  TaArray<const fl32 *> _iqChan1_;
  const fl32* *_iqChan1;

  // FFTs

  RadarFft *_fft;
  RadarFft *_fftHalf;

  // FFT support - staggered mode
  
  RadarFft *_fftStag;
  
  // regression clutter filtering

  ForsytheRegrFilter *_regr;
  ForsytheRegrFilter *_regrHalf;

  // regression clutter filtering - staggered prt

  ForsytheRegrFilter *_regrStag;

  // antenna rate

  double _beamAzRate;
  double _beamElRate;

  // sz 864 phase code support

  vector<RadarComplex_t> _txDelta12;
  vector<double> _phaseDiffs;
  vector<IwrfTsPulse::burst_phase_t> _burstPhases;

  // private functions

  void _init();
  void _freeWindows();
  int _getVolNum();
  int _getSweepNum();
  void _computeMoments();
  void _filterMoments();

  void _computeTripNcp();
  void _computeMomSpH();
  void _computeMomSpV();
  void _computeMomSpStagPrt();

  void _computeMomDpAltHvCoCross();
  void _computeMomDpAltHvCoOnly();
  void _computeMomDpSimHvStagPrt();
  void _computeMomDpSimHv();
  void _computeMomDpHOnly();
  void _computeMomDpVOnly();

  void _filterSpH();
  void _filterSpV();
  void _filterSpStagPrt();
  void _filterAdapSpStagPrt();
  void _filterRegrSpStagPrt();
  void _filterDpAltHvCoCross();
  void _filterDpAltHvCoOnly();
  void _filterDpSimHvFixedPrt();
  void _filterDpSimHvStagPrt();
  void _filterDpHOnlyFixedPrt();
  void _filterDpHOnlyStagPrt();
  void _filterDpVOnlyFixedPrt();
  void _filterDpVOnlyStagPrt();
  
  void _computeWindowRValues();
  void _overrideOpsInfo();
  void _computeWindows();
  void _initMomentsObject();
  void _fftInit();
  void _regrInit();

  void _kdpInit();
  void _kdpCompute(bool isFiltered);

  void _allocGateData(int nGates);
  void _freeGateData();
  void _initFieldData();
  void _loadGateIq(const fl32 **iqChan0, const fl32 **iqChan1);
  void _loadGateIqStagPrt(const fl32 **iqChan0, const fl32 **iqChan1);
  void _initStagPrt(int nGatesPrtShort, int nGatesPrtLong,
                    double prtShort, double prtLong);

  void _copyDataToOutputFields();
  int _checkCalib();

  void _cleanUpStagVel();
  void _performClutterFiltering();
  
  double _computeClutPower(const MomentsFields &unfiltered,
                           const MomentsFields &filtered);

  void _computeBeamAzRate();
  void _computeBeamElRate();
  
  void _computePhaseDiffs
    (const deque<const IwrfTsPulse *> &pulseQueue, int maxTrips);
  
  // copy method for assignment and copy constructor

  Beam & _copy(const Beam &rhs);

};

#endif

