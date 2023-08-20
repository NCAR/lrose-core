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
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2005
//
///////////////////////////////////////////////////////////////

#ifndef Beam_hh
#define Beam_hh

#include <pthread.h>
#include <string>
#include <vector>
#include <cstdio>
#include <memory>
#include <radar/RadarMoments.hh>
#include <radar/RadarComplex.hh>
#include <radar/IwrfTsInfo.hh>
#include <radar/IwrfTsPulse.hh>
#include <radar/GateData.hh>
#include <radar/MomentsFields.hh>
#include <radar/AlternatingVelocity.hh>
#include <radar/InterestMap.hh>
#include <radar/NoiseLocator.hh>
#include <radar/AlternatingVelocity.hh>
#include <radar/KdpFilt.hh>
#include <radar/KdpBringi.hh>
#include <radar/AtmosAtten.hh>
#include <radar/PhaseCoding.hh>
#include <radar/DwellSpectra.hh>
#include "Params.hh"
#include "Cmd.hh"
#include "MomentsMgr.hh"
using namespace std;

////////////////////////
// This class

class Beam {
  
public:

  // scan mode for determining PPI vs RHI operations
  
  typedef enum {
    SCAN_TYPE_UNKNOWN,
    SCAN_TYPE_PPI,
    SCAN_TYPE_RHI,
    SCAN_TYPE_VERT
  } scan_type_t;
  
  // Constructor
  
  Beam(const string &progName,
       const Params &params);
        
  // initialize before use
  
  void init(const MomentsMgr &mmgr,
            int nSamples,
            int nSamplesEffective,
            int nGates,
            int nGatesPrtLong,
            bool beamIsIndexed,
            double angularResolution,
            double meanPointingAngle,
            scan_type_t scanType,
            double antennaRate,
            bool isAlternating,
            bool isStagPrt,
            double prt,
            double prtLong,
            bool applyPhaseDecoding,
            iwrf_xmit_rcv_mode_t xmitRcvMode,
            bool endOfSweepFlag,
            bool endOfVolFlag,
            const AtmosAtten &atmosAtten,
            const IwrfTsInfo &opsInfo,
            const vector<shared_ptr<IwrfTsPulse>> &pulses);

  // destructor
  
  ~Beam();

  // compute moments
  
  void computeMoments();

  // set methods

  void setTargetEl(double el) { _targetEl = el; }
  void setCalib(const IwrfCalib &calib);
  void setStatusXml(const string &statusXml);
  void appendStatusXml(const string &extraXml);

  void setSweepNum(int val) { _sweepNum = val; }
  void setVolNum(int val) { _volNum = val; }

  // get methods

  int getNSamples() const { return _nSamples; }
  int getNSamplesEffective() const { return _nSamplesEffective; }

  double getEl() const { return _el; }
  double getAz() const { return _az; }
  double getTargetEl() const { return _targetEl; }
  double getTargetAz() const { return _targetAz; }
  double getPrt() const { return _prt; }
  bool getIsStagPrt() const { return _isStagPrt; }
  int getStagM() const { return _stagM; }
  int getStagN() const { return _stagN; }
  double getPrtShort() const { return _prtShort; }
  double getPrtLong() const { return _prtLong; }
  double getPulseWidth() const { return _pulseWidth; }
  double getNyquist() const { return _nyquist; }
  double getUnambigRangeKm() const;

  double getMeasXmitPowerDbmH() const { return _measXmitPowerDbmH; }
  double getMeasXmitPowerDbmV() const { return _measXmitPowerDbmV; }
  
  time_t getTimeSecs() const { return _timeSecs; }
  long int getNanoSecs() const { return _nanoSecs; }
  double getDoubleTime() const { return _dtime; }
  
  int getScanMode() const;
  int getFollowMode() const { return _followMode; }
  int getSweepNum() const { return _sweepNum; }
  int getVolNum() const { return _volNum; }
  bool getEndOfSweepFlag() const { return _endOfSweepFlag; }
  bool getEndOfVolFlag() const { return _endOfVolFlag; }
  double getAntennaRate() const { return _antennaRate; }
  bool getAntennaTransition() const { return _antennaTransition; }

  bool getBeamIsIndexed() const { return _beamIsIndexed; }
  double getAngularResolution() const { return _angularResolution; }

  bool getIsAlternating() const { return _isAlternating; }

  int getNGates() const { return _nGates; }
  int getNGatesOut() const { return _nGatesOut; }
  double getMaxRangeKm() const;

  double getStartRangeKm() const { return _startRangeKm; }
  double getGateSpacingKm() const { return _gateSpacingKm; }

  iwrf_xmit_rcv_mode_t getXmitRcvMode() const { return _xmitRcvMode; }
  bool getIsDualPol() const { return _dualPol; }
  bool getIsSwitchingReceiver() const { return _switchingReceiver; }

  const IwrfTsInfo &getOpsInfo() const { return _opsInfo; }

  const MomentsFields* getFields() const { return _fields; }
  const MomentsFields* getFieldsF() const { return _fieldsF; }

  const IwrfCalib &getCalib() const { return _calib; }
  const string &getStatusXml() const { return _statusXml; }

  const iwrf_platform_georef_t &getGeoref() const { return _georef; }
  bool getGeorefActive() const { return _georefActive; }

  bool hasMissingPulses() const { return _hasMissingPulses; }

  // manage threading

  static void setUpThreads(const Params &params);
  static void cleanUpThreads();

protected:
  
private:

  static const double _missingDbl;
  static int _nWarnings;

  string _progName;
  const Params &_params;

  MomentsMgr _mmgr;

  // pulse vector

  vector<shared_ptr<IwrfTsPulse>> _pulses;
  bool _hasMissingPulses;
  static pthread_mutex_t _pulseUnpackMutex;

  // number of samples

  int _nSamples; // nsamples making adjustment for window
  int _nSamplesHalf;
  int _nSamplesAlloc;
  int _nSamplesEffective; // effective nsamples adjusting for window

  // number of gates, gate geometry
  
  int _nGates; // input data
  int _nGatesOut; // output
  int _nGatesOutAlloc; // output

  // time and location

  time_t _timeSecs;
  long int _nanoSecs;
  double _dtime;

  double _meanPointingAngle;
  double _el;
  double _az;
  double _targetEl;
  double _targetAz;

  int _scanMode;
  int _followMode;
  int _sweepNum;
  int _volNum;
  bool _endOfSweepFlag;
  bool _endOfVolFlag;
  
  scan_type_t _scanType;
  double _antennaRate;
  bool _antennaTransition;

  // range geometry

  double _startRangeKm;
  double _gateSpacingKm;

  // platform georeferencing - for moving platforms

  iwrf_platform_georef_t _georef;
  bool _georefActive;

  // atmospheric attenuation

  const AtmosAtten *_atmosAtten;

  // modes

  bool _beamIsIndexed;
  double _angularResolution;
  bool _isAlternating;
  iwrf_xmit_rcv_mode_t _xmitRcvMode;
  bool _dualPol;
  bool _switchingReceiver;

  // prt and pulse width

  double _prt;
  double _nyquist;
  double _pulseWidth;

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

  // moments fields at each gate

  MomentsFields *_fields;
  MomentsFields *_fieldsF;

  // calibration

  IwrfCalib _calib;

  // status XML

  string _statusXml;

  // Moments computations
  
  RadarMoments *_mom;
  RadarMoments *_momStagPrt;
  PhaseCoding _pcode;

  bool _checkForWindfarms;
  double _minSnrForWindfarmCheck;
  double _minCpaForWindfarmCheck;

  // window

  double *_window;
  double *_windowHalf;
  double *_windowVonHann;
  
  // R values, at various lags, for the windows
  // used to correct R values used in width computations

  double _windowR1;
  double _windowR2;
  double _windowR3;
  double _windowHalfR1;
  double _windowHalfR2;
  double _windowHalfR3;

  // phase code support

  bool _applyPhaseDecoding;
  bool _applySz1;
  RadarComplex_t *_txDelta12;
  vector<double> _phaseDiffs;

  vector<IwrfTsPulse::burst_phase_t> _burstPhases;

  // CMD object
  
  Cmd *_cmd;

  // KDP

  bool _needKdp;
  bool _needKdpFiltered;
  KdpFilt _kdp;
  KdpBringi _kdpB;

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

  // noise

  NoiseLocator *_noise;

  // fields for moments

  TaArray<MomentsFields> _momFields_;
  MomentsFields *_momFields;

  TaArray<MomentsFields> _momFieldsF_;
  MomentsFields *_momFieldsF;

  // alternating velocity

  AlternatingVelocity _altVel;
  
  // censoring
  
  TaArray<bool> _censorFlags_;
  bool *_censorFlags;

  // gate data

  vector<GateData *> _gateData;
  
  // FFTs

  static pthread_mutex_t _fftMutex;
  RadarFft *_fft;
  RadarFft *_fftHalf;

  // FFT support - staggered mode
  
  RadarFft *_fftStag;
  
  // regression clutter filtering

  ForsytheRegrFilter *_regr;
  ForsytheRegrFilter *_regrHalf;

  // regression clutter filtering - staggered prt

  ForsytheRegrFilter *_regrStag;

  // spectral CMD

  bool _specCmdValid;
  DwellSpectra _specCmd;

  // debug printing
  
  static pthread_mutex_t _debugPrintMutex;

 // private functions
  
  void _freeWindows();
  void _prepareForComputeMoments();
  int _getVolNum();
  int _getSweepNum();
  void _computeMoments();
  void _filterMoments();

  void _computeTripNcp();
  void _computeMomSpH();
  void _computeMomSpV();
  void _computeMomSpSz();
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
  void _filterSpSz864();
  void _filterDpAltHvCoCross();
  void _filterDpAltHvCoOnly();
  void _filterDpSimHvFixedPrt();
  void _filterDpSimHvStagPrt();
  void _filterDpHOnlyFixedPrt();
  void _filterDpHOnlyStagPrt();
  void _filterDpVOnlyFixedPrt();
  void _filterDpVOnlyStagPrt();

  int _specCmdInit();
  void _filtSpecCmdSimHv();
  
  void _computeWindowRValues();
  void _overrideOpsInfo();
  void _computeWindows();
  void _initMomentsObject(RadarMoments *mom);

  void _kdpInit();
  void _kdpCompute(bool isFiltered);
  void _kdpComputeBringi(bool isFiltered);

  void _computeVelocityCorrectedForVertMotion();
  void _computeVelocityCorrectedForMotion();
  double _correctForNyquist(double vel);
  void _computeWidthCorrectedForMotion();

  int _noiseInit();
  int _convertInterestParamsToVector(const string &label,
                                     const Params::interest_map_point_t *map,
                                     int nPoints,
                                     vector<InterestMap::ImPoint> &pts);


  void _checkAntennaTransition(const vector<shared_ptr<IwrfTsPulse>> &pulses);
  void _allocGateData(int nGates);
  void _freeGateData();
  void _initFieldData();
  void _loadGateIq(const fl32 **iqChan0, const fl32 **iqChan1);
  void _loadGateIqStagPrt(const fl32 **iqChan0, const fl32 **iqChan1);
  void _initStagPrt(int nGatesPrtShort, int nGatesPrtLong,
                    double prtShort, double prtLong);

  void _copyDataToOutputFields();
  int _checkCalib();

  int _correctCalibGainsForTemp();
  int _correctHcrVRxGainForTemp();

  double _getTempFromTagList(const string &tagList) const;
  double _getDeltaGainFromXml(const string &xml,
                              const string &tagList) const;
  
  void _applyTimeDomainFilter(const RadarComplex_t *iq,
                              RadarComplex_t *filtered) const;
  void _applyMedianFilterToCPA(int nGates);
  void _applyMedianFilterToZDR(int nGates);
  void _applyMedianFilterToRHOHV(int nGates);
  void _cleanUpStagVel();
  void _performClutterFiltering();
  void _performClutterFilteringSz();
  void _fixAltClutVelocity();
  void _fixAltClutVelocityFiltered();
  
  double _computeClutPower(const MomentsFields &unfiltered,
                           const MomentsFields &filtered);

  void _printSelectedMoments();

  void _computePhaseDiffs
    (const vector<shared_ptr<IwrfTsPulse>> &pulseQueue, int maxTrips);

  void _setNoiseFields();
  void _censorByNoiseFlag();
  void _censorBySnrNcp();
  void _censorFields(MomentsFields &mfield);
  void _fillInCensoring(size_t nGates);
    
  void _cohereTrip1ToTrip2
    (RadarComplex_t *iq,
     const vector<IwrfTsPulse::burst_phase_t> burstPhases);

  double _getCorrectedAz(double az);
  double _getCorrectedEl(double el);

  void _correctAltitudeForEgm();

};

#endif

