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
// Beam.cc
//
// Beam object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2005
//
///////////////////////////////////////////////////////////////
//
// Beam object holds time series and moment data.
//
////////////////////////////////////////////////////////////////

#include <cassert>
#include <iostream>
#include <algorithm>
#include <toolsa/TaArray.hh>
#include <toolsa/toolsa_macros.h>
#include <toolsa/DateTime.hh>
#include <toolsa/TaStr.hh>
#include <toolsa/TaXml.hh>
#include <toolsa/sincos.h>
#include <rapformats/ds_radar.h>
#include <radar/FilterUtils.hh>
#include "Beam.hh"
using namespace std;

const double Beam::_missingDbl = MomentsFields::missingDouble;
int Beam::_nWarnings = 0;
pthread_mutex_t Beam::_fftMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t Beam::_debugPrintMutex = PTHREAD_MUTEX_INITIALIZER;

////////////////////////////////////////////////////
// Constructor

Beam::Beam(const string &progName,
	   const Params &params) :
        _progName(progName),
        _params(params),
	_mmgr(progName, params),
        _pcode(params.debug >= Params::DEBUG_VERBOSE)
        
{

  _hasMissingPulses = false;

  _nSamples = 0;
  _nSamplesHalf = 0;
  _nSamplesAlloc = 0;

  _nGates = 0;
  _nGatesOut = 0;
  _nGatesOutAlloc = 0;

  _timeSecs = 0;
  _nanoSecs = 0;
  _dtime = 0;

  _az = 0;
  _el = 0;
  _targetEl = 0;
  _targetAz = 0;

  _scanMode = IWRF_SCAN_MODE_NOT_SET;
  _followMode = IWRF_FOLLOW_MODE_NOT_SET;
  _sweepNum = 0;
  _volNum = 0;
  _endOfSweepFlag = false;
  _endOfVolFlag = false;

  _isPpi = true;
  _antennaTransition = false;

  _startRangeKm = 0.0;
  _gateSpacingKm = 0.0;

  _beamIsIndexed = true;
  _angularResolution = 1.0;
  _isAlternating = false;
  _xmitRcvMode = IWRF_SINGLE_POL;
  _dualPol = false;
  _switchingReceiver = false;

  _prt = 0;
  _nyquist = 0;
  _pulseWidth = 0;

  _isStagPrt = false;
  _prtShort = _prt;
  _prtLong = _prt * 1.5;
  _nGatesPrtShort = 0;
  _nGatesPrtLong = 0;
  _nGatesStagPrt = 0;
  _nyquistPrtShort = 0;
  _nyquistPrtLong = 0;
  _stagM = 0;
  _stagN = 0;

  _measXmitPowerDbmH = 0;
  _measXmitPowerDbmV = 0;

  _fields = NULL;
  _fieldsF = NULL;

  _mom = NULL;
  _noise = NULL;

  _applyFiltering = true;
  _needKdp = false;
  _needKdpFiltered = false;

  _checkForWindfarms = _params.cmd_check_for_windfarm_clutter;
  _minSnrForWindfarmCheck = _params.min_snr_for_windfarm_clutter_check;
  _minCpaForWindfarmCheck = _params.min_cpa_for_windfarm_clutter_check;

  _window = NULL;
  _windowHalf = NULL;
  _windowVonHann = NULL;

  _windowR1 = 0;
  _windowR2 = 0;
  _windowR3 = 0;
  _windowHalfR1 = 0;
  _windowHalfR2 = 0;
  _windowHalfR3 = 0;

  _applyPhaseDecoding = false;
  _applySz1 = false;
  _txDelta12 = NULL;

  _cmd = new Cmd(_progName, _params, _gateData);
  
  _fft = new RadarFft();
  _fftHalf = new RadarFft();
  _fftStag = new RadarFft();

  _regr = new RegressionFilter();
  _regrHalf = new RegressionFilter();
  _regrStag = new RegressionFilter();

}

////////////////////////////////////////////////////
// Initialize before use

void Beam::init(const MomentsMgr &mmgr,
                int nSamples,
                int nSamplesEffective,
                int nGates,
                int nGatesPrtLong,
                bool beamIsIndexed,
                double angularResolution,
                double meanPointingAngle,
                bool isPpi,
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
                const vector<const IwrfTsPulse *> &pulses)
  
{

  _mmgr = mmgr;
  _nSamples = nSamples;
  _nSamplesEffective = nSamplesEffective;
  _nGates = nGates;
  _nGatesPrtShort = nGates;
  _nGatesPrtLong = nGatesPrtLong;
  _beamIsIndexed = beamIsIndexed;
  _angularResolution = angularResolution;
  _meanPointingAngle = meanPointingAngle;
  _isPpi = isPpi;
  _isAlternating = isAlternating;
  _isStagPrt = isStagPrt;
  _prt = prt;
  _prtShort = prt;
  _prtLong = prtLong;
  _applyPhaseDecoding = applyPhaseDecoding;
  _applySz1 = false;
  if (_applyPhaseDecoding) {
    if (_params.phase_decoding == Params::PHASE_DECODE_SZ1) {
      _applySz1 = true;
    }
  }
  _xmitRcvMode = xmitRcvMode;
  _endOfSweepFlag = endOfSweepFlag;
  _endOfVolFlag = endOfVolFlag;
  _atmosAtten = &atmosAtten;
  _opsInfo = opsInfo;
  _pulses = pulses;
  _georefActive = false;

  _pcode.setNSamples(_nSamples);

  // for each pulse, increase client count by 1,
  // so we can keep track of how many threads are using this pulse

  for (size_t ii = 0; ii < _pulses.size(); ii++) {
    _pulses[ii]->addClient();
  }

  // set up burst phase vector

  _burstPhases.clear();
  for (size_t ii = 0; ii < _pulses.size(); ii++) {
    _burstPhases.push_back(_pulses[ii]->getBurstPhases());
  }

  // initialize noise computations

  if (_noiseInit()) {
    cerr << "ERROR - Beam::init()" << endl;
    cerr << "  Cannot initialize noise object" << endl;
  }

}

//////////////////////////////////////////////////////////////////
// destructor

Beam::~Beam()

{

  if (_txDelta12) {
    delete[] _txDelta12;
  }

  if (_fields) {
    delete[] _fields;
  }

  if (_fieldsF) {
    delete[] _fieldsF;
  }
  
  if (_mom) {
    delete _mom;
  }
  
  if (_noise) {
    delete _noise;
  }
  
  if (_cmd) {
    delete _cmd;
  }

  if (_fft) {
    delete _fft;
    _fft = NULL;
  }
  if (_fftHalf) {
    delete _fftHalf;
    _fftHalf = NULL;
  }
  if (_fftStag) {
    delete _fftStag;
    _fftStag = NULL;
  }

  if (_regr) {
    delete _regr;
    _regr = NULL;
  }
  if (_regrHalf) {
    delete _regrHalf;
    _regrHalf = NULL;
  }
  if (_regrStag) {
    delete _regrStag;
    _regrStag = NULL;
  }
  
  _snrArray_.free();
  _dbzArray_.free();
  _phidpArray_.free();
  
  _freeWindows();
  _freeGateData();

}

//////////////////////////////////////////////////////////////////
// free up windows

void Beam::_freeWindows()

{

  if (_window) {
    delete[] _window;
    _window = NULL;
  }

  if (_windowHalf) {
    delete[] _windowHalf;
    _windowHalf = NULL;
  }

  if (_windowVonHann) {
    delete[] _windowVonHann;
    _windowVonHann = NULL;
  }

}
  
//////////////////////////////////////////////////////////////////
// release pulses for use by other threads

void Beam::_releasePulses()

{
  
  // for each pulse, decrease client count by 1, so we know how many threads
  // are using this pulse. When a pulse has no clients, it
  // can be reused
  
  for (size_t ii = 0; ii < _pulses.size(); ii++) {
    _pulses[ii]->removeClient();
  }
  _pulses.clear();

}

////////////////////////////////////////////////////
// Prepare for moments computations

void Beam::_prepareForComputeMoments()

{

  _nSamplesHalf = _nSamples / 2;
  _stagM = 2;
  _stagN = 3;
  _nGatesStagPrt = MAX(_nGatesPrtShort, _nGatesPrtLong);

  if (_isStagPrt) {
    _initStagPrt(_nGates, _nGatesPrtLong, _prt, _prtLong);
    _nGates = _nGatesStagPrt;
  } else {
    double wavelengthMeters = _opsInfo.get_radar_wavelength_cm() / 100.0;
    _nyquist = ((wavelengthMeters / _prt) / 4.0);
  }

  if (_xmitRcvMode == IWRF_SINGLE_POL) {
    _dualPol = false;
  } else {
    _dualPol = true;
  }

  _switchingReceiver = _mmgr.isSwitchingReceiver();
  _applyFiltering = _mmgr.applyClutterFilter();

  // range geometry

  _startRangeKm = _opsInfo.get_proc_start_range_km();
  _gateSpacingKm = _opsInfo.get_proc_gate_spacing_km();

  // override OpsInfo time-series values as needed
  
  _overrideOpsInfo();

  // pulse width
  
  const IwrfTsPulse *midPulse = _pulses[_nSamplesHalf];
  _pulseWidth = midPulse->getPulseWidthUs() / 1.0e6;

  // transmitter power

  _measXmitPowerDbmH = midPulse->getMeasXmitPowerDbmH();
  _measXmitPowerDbmV = midPulse->getMeasXmitPowerDbmV();

  // set time

  _timeSecs = (time_t) midPulse->getTime();
  _nanoSecs = midPulse->getNanoSecs();
  _dtime = midPulse->getFTime();

  // select the georeference from the mid pulse

  if (midPulse->getGeorefActive()) {
    _georef = midPulse->getPlatformGeoref();
    _georefActive = true;
  }

  // set elevation / azimuth

  if (_isPpi) {
    _az = _getCorrectedAz(_meanPointingAngle);
    _el = _getCorrectedEl(midPulse->getEl());
  } else {
    _el = _getCorrectedEl(_meanPointingAngle);
    _az = _getCorrectedAz(midPulse->getAz());
  }
  _targetEl = _getCorrectedEl(midPulse->getFixedEl());
  _targetAz = _getCorrectedAz(midPulse->getFixedAz());
  if (std::isnan(_targetEl) || _targetEl < -990) {
    _targetEl = _el;
  }
  if (std::isnan(_targetAz) || _targetAz < -990) {
    _targetAz = _az;
  }

#ifdef DEBUG_PRINT_SPECTRA
  GlobAz = _az;
  GlobElev = _el;
#endif

  // scan details

  _scanMode = midPulse->get_scan_mode();
  _followMode = midPulse->get_follow_mode();
  _sweepNum = _getSweepNum();
  _volNum = _getVolNum();
  if (_volNum < 0) {
    _volNum = _volNum + 65536;
  }

  // set antenna transition flag

  _checkAntennaTransition(_pulses);

  // compute number of output gates
  
  if (_applySz1) {
    _nGatesOut = _nGates * 2;
  } else if (_isStagPrt) {
    _nGatesOut = _nGatesPrtLong;
  } else {
    _nGatesOut = _nGates;
  }

  // ffts and regression filter

  // initialize the FFT objects on the threads
  // Note: the initialization is not thread safe so it 
  // must be protected by a mutex

  pthread_mutex_lock(&_fftMutex);

  _fft->init(_nSamples);
  _fftHalf->init(_nSamplesHalf);
  int nStag =
    RadarMoments::computeNExpandedStagPrt(_nSamples, _stagM, _stagN);
  _fftStag->init(nStag);
  
  // initialize the regression objects
  
  if (_params.use_polynomial_regression_clutter_filter) {
    int order = _params.regression_filter_polynomial_order;
    _regr->setup(_nSamples, order);
    _regrHalf->setup(_nSamplesHalf, order);
    _regrStag->setupStaggered(_nSamples, _stagM, _stagN, order);
  }

  pthread_mutex_unlock(&_fftMutex);

  // compute delta phases for SZ, if required
  
  if (_applySz1) {

    // compute phase differences between this pulse and previous ones
    // to prepare for cohering to multiple trips
    _computePhaseDiffs(_pulses, 4);

    if (_nSamples > _nSamplesAlloc) {
      if (_txDelta12) delete[] _txDelta12;
      _txDelta12 = new RadarComplex_t[_nSamples];
      _nSamplesAlloc = _nSamples;
    }
    for (int i = 0; i < _nSamples; i++) {
      _txDelta12[i].re = cos(_pulses[i]->getPhaseDiff0() * DEG_TO_RAD);
      _txDelta12[i].im = -1.0 * sin(_pulses[i]->getPhaseDiff0() * DEG_TO_RAD);
    }
  } // if (_applySz) 
  
  // alloc fields at each gate

  if (_nGatesOut > _nGatesOutAlloc) {

    if (_fields) delete[] _fields;
    if (_fieldsF) delete[] _fieldsF;
    if (_mom) delete _mom;

    _fields = new MomentsFields[_nGatesOut];
    _fieldsF = new MomentsFields[_nGatesOut];
    
    _mom = new RadarMoments(_nGatesOut,
                            _params.debug >= Params::DEBUG_NORM,
                            _params.debug >= Params::DEBUG_VERBOSE);

    _nGatesOutAlloc = _nGatesOut;

  }

  // initialize fields

  for (int ii = 0; ii < _nGatesOut; ii++) {
    _fields[ii].init();
    _fieldsF[ii].init();
  }

  if (_params.adjust_dbz_for_measured_xmit_power) {
    _mom->setAdjustDbzForMeasXmitPower();
  }
  if (_params.adjust_zdr_for_measured_xmit_power) {
    _mom->setAdjustZdrForMeasXmitPower();
  }
  if (_params.compute_zdr_using_snr) {
    _mom->setComputeZdrUsingSnr(true);
  }
  _mom->setMeasXmitPowerDbmH(_measXmitPowerDbmH);
  _mom->setMeasXmitPowerDbmV(_measXmitPowerDbmV);

  if (_params.spectrum_width_method == Params::WIDTH_METHOD_R0R1) {
    _mom->setSpectrumWidthMethod(RadarMoments::WIDTH_METHOD_R0R1);
  } else if (_params.spectrum_width_method == Params::WIDTH_METHOD_R1R2) {
    _mom->setSpectrumWidthMethod(RadarMoments::WIDTH_METHOD_R1R2);
  } else if (_params.spectrum_width_method == Params::WIDTH_METHOD_HYBRID) {
    _mom->setSpectrumWidthMethod(RadarMoments::WIDTH_METHOD_HYBRID);
  }

  if (_params.threshold_zdr_using_snr) {
    _mom->setMinSnrDbForZdr(_params.min_snr_db_for_zdr);
  }
  if (_params.threshold_ldr_using_snr) {
    _mom->setMinSnrDbForLdr(_params.min_snr_db_for_ldr);
  }

  // compute windows for FFTs

  _computeWindows();

  // initialize moments computations object
  
  _initMomentsObject();
  
  // KDP

  _kdpInit();

  // set up data pointer arrays - channel 0

  TaArray<const fl32 *> iqChan0_;
  const fl32* *iqChan0 = iqChan0_.alloc(_nSamples);
  for (int ii = 0; ii < _nSamples; ii++) {
    iqChan0[ii] = _pulses[ii]->getIq0();
  }
  
  // channel 1 - will be NULLs for single pol

  bool haveChan1 = true;
  for (int ii = 0; ii < _nSamples; ii++) {
    if (_pulses[ii]->getIq1() == NULL) {
      haveChan1 = false;
      break;
    }
  }

  TaArray<const fl32 *> iqChan1_;
  const fl32* *iqChan1 = NULL;
  if (haveChan1) {
    iqChan1 = iqChan1_.alloc(_nSamples);
    for (int ii = 0; ii < _nSamples; ii++) {
      iqChan1[ii] = _pulses[ii]->getIq1();
    }
  }

  // load data into gate arrays
  
  if (_isStagPrt) {
    _allocGateData(_nGatesStagPrt);
    _initFieldData();
    if (_params.swap_receiver_channels) {
      _loadGateIqStagPrt(iqChan1, iqChan0);
    } else {
      _loadGateIqStagPrt(iqChan0, iqChan1);
    }
  } else {
    _allocGateData(_nGates);
    _initFieldData();
    if (_params.swap_receiver_channels) {
      _loadGateIq(iqChan1, iqChan0);
    } else {
      _loadGateIq(iqChan0, iqChan1);
    }
  }

  // free up pulses for use by other threads

  _releasePulses();

  // initialize ray properties for noise computations

  _noise->setRayProps(_nGates, _calib, _timeSecs, _nanoSecs, _el, _az);

}

////////////////////////////////////////////////////
// Get the volume number
// compute the median volume number for the beam

int Beam::_getVolNum()

{

  vector<int> volNums;
  for (size_t ii = 0; ii < _pulses.size(); ii++) {
    volNums.push_back(_pulses[ii]->get_volume_num());
  }
  sort(volNums.begin(), volNums.end());

  return volNums[_nSamplesHalf];

}

////////////////////////////////////////////////////
// Get the sweep number
// compute the median sweep number for the beam

int Beam::_getSweepNum()

{

  vector<int> sweepNums;
  for (size_t ii = 0; ii < _pulses.size(); ii++) {
    sweepNums.push_back(_pulses[ii]->get_sweep_num());
  }
  sort(sweepNums.begin(), sweepNums.end());

  return sweepNums[_nSamplesHalf];

}


/////////////////////////////////////////////////
// compute moments
    
void Beam::computeMoments()
  
{

  _prepareForComputeMoments();

  // set calibration data on Moments object, ready for computations
  
  _mom->setCalib(_calib);
  
  // compute the moments
  
  _computeMoments();

  // apply median filter to CPA, to smooth single gates which have
  // a high CPA value
  
  _applyMedianFilterToCPA(_nGates);

  // in staggered mode, apply median filter to the nyquist unfolding
  // interval and re-compute the velocity
  
  if (_isStagPrt) {
    _cleanUpStagVel();
  }

  // apply clutter filtering if required

  if (_mmgr.applyClutterFilter()) {
    
    if (_applySz1) {
      _performClutterFilteringSz();
    } else {
      _performClutterFiltering();
    }
    
  }

  // post-processing

  bool kdpAvail = false;

  switch (_xmitRcvMode) {

    case IWRF_ALT_HV_CO_ONLY:
    case IWRF_ALT_HV_CO_CROSS:
    case IWRF_ALT_HV_FIXED_HV:
    case IWRF_SIM_HV_FIXED_HV:
    case IWRF_SIM_HV_SWITCHED_HV:
      {
        kdpAvail = true;
      }
      break;

    case IWRF_H_ONLY_FIXED_HV:
    case IWRF_V_ONLY_FIXED_HV:
      {}
      break;

    case IWRF_SINGLE_POL:
    default:
      {}
      break;
      
  }

  // compute kdp

  if (kdpAvail && _needKdp) {
    _kdpCompute(false);
    if (_params.KDP_compute_using_hubbert_bringi_method) {
      _kdpComputeBringi(false);
    }
  }
  
  if (kdpAvail && _needKdpFiltered) {
    _kdpCompute(true);
    if (_params.KDP_compute_using_hubbert_bringi_method) {
      _kdpComputeBringi(true);
    }
  }

  // compute corrected velocity

  if (_params.compute_velocity_corrected_for_platform_motion) {
    _computeVelocityCorrectedForMotion();
  }

  // applyMedianFilter as appropriate

  _applyMedianFilterToZDR(_nGates);
  _applyMedianFilterToRHOHV(_nGates);
  
#ifdef DEBUG_PRINT_SPECTRA
  _printSelectedMoments();
#endif
  
  // set the noise fields
  
  _setNoiseFields();
  
  // censor gates as required
  
  if (_params.censoring_mode == Params::CENSORING_BY_NOISE_FLAG) {
    _censorByNoiseFlag();
  } else if (_params.censoring_mode == Params::CENSORING_BY_SNR_AND_NCP) {
    _censorBySnrNcp();
  } 
  
  // copy the results to the output beam Field vectors

  _copyDataToOutputFields();

}

/////////////////////////////////////////////////
// set the calibration
    
void Beam::setCalib(const IwrfCalib &calib)

{

  _calib = calib;
  _checkCalib();

}

/////////////////////////////////////////////////
// set the status XML
    
void Beam::setStatusXml(const string &statusXml)

{

  _statusXml = statusXml;

}

////////////////////////////////////////////////
// get scan mode

int Beam::getScanMode() const

{

  int scanMode = _scanMode;

  if (_scanMode == DS_RADAR_UNKNOWN_MODE) {
    scanMode = _opsInfo.get_scan_mode();
  }

  if (scanMode < 0) {
    scanMode = DS_RADAR_SURVEILLANCE_MODE;
  }

  return scanMode;

}

////////////////////////////////////////////////
// get maximum range

double Beam::getMaxRange() const

{

  double maxRange = _opsInfo.get_proc_start_range_km() +
    _nGatesOut * _opsInfo.get_proc_gate_spacing_km();
  
  return maxRange;

}

///////////////////////////////////////////////////////////
// Compute moments at each gate

void Beam::_computeMoments()

{

  switch (_xmitRcvMode) {
      
    case IWRF_ALT_HV_CO_ONLY:
      _computeMomDpAltHvCoOnly();
      break;
      
    case IWRF_ALT_HV_CO_CROSS:
    case IWRF_ALT_HV_FIXED_HV:
      _computeMomDpAltHvCoCross();
      break;
      
    case IWRF_SIM_HV_FIXED_HV:
    case IWRF_SIM_HV_SWITCHED_HV:
      if (_isStagPrt) {
        _computeMomDpSimHvStagPrt();
      } else {
        _computeMomDpSimHv();
      }
      break;
      
    case IWRF_H_ONLY_FIXED_HV:
      _computeMomDpHOnly();
      break;
      
    case IWRF_V_ONLY_FIXED_HV:
      _computeMomDpVOnly();
      break;
      
    case IWRF_SINGLE_POL:
    default:
      if (_applySz1) {
        _computeMomSpSz();
      } else if (_isStagPrt) {
        _computeMomSpStagPrt();
      } else {
        _computeMomSp();
      }
      
  }

}

//////////////////////////////////////////////
// Compute filtered moments using the threads

void Beam::_filterMoments()
  
{

  switch (_xmitRcvMode) {
      
    case IWRF_ALT_HV_CO_ONLY:
      _filterDpAltHvCoOnly();
      break;
      
    case IWRF_ALT_HV_CO_CROSS:
    case IWRF_ALT_HV_FIXED_HV:
      _filterDpAltHvCoCross();
      break;
      
    case IWRF_SIM_HV_FIXED_HV:
    case IWRF_SIM_HV_SWITCHED_HV:
      if (_isStagPrt) {
        _filterDpSimHvStagPrt();
      } else {
        _filterDpSimHvFixedPrt();
      }
      break;
      
    case IWRF_H_ONLY_FIXED_HV:
      if (_isStagPrt) {
        _filterDpHOnlyStagPrt();
      } else {
        _filterDpHOnlyFixedPrt();
      }
      break;
      
    case IWRF_V_ONLY_FIXED_HV:
      if (_isStagPrt) {
        _filterDpVOnlyStagPrt();
      } else {
        _filterDpVOnlyFixedPrt();
      }
      break;
      
    case IWRF_SINGLE_POL:
    default:
      if (_applySz1) {
        _filterSpSz864();
      } else if (_isStagPrt) {
        _filterSpStagPrt();
      } else {
        _filterSp();
      }
      
  }

}

///////////////////////////////////////////////////////////
// Compute NCP for all trips, assuming the IQ data is
// in trip1

void Beam::_computeTripNcp()

{

  // only applies to phase decoding

  if (!_applyPhaseDecoding) {
    return;
  }
  
  // compute ncp for all appropriate trips
  
  for (int igate = 0; igate < _nGates; igate++) {
    
    GateData *gate = _gateData[igate];
    MomentsFields &fields = _momFields[igate];
    
    fields.ncp_trip1 = _mom->computeNcp(gate->iqTrip1);
    _pcode.revertFromTrip1(gate->iqTrip1, _burstPhases, gate->iqMeas);

    _pcode.cohereToTrip2(gate->iqMeas, _burstPhases, gate->iqTrip2);
    fields.ncp_trip2 = _mom->computeNcp(gate->iqTrip2);

    if (_params.phase_decoding_ntrips_check > 2) {
      _pcode.cohereToTrip3(gate->iqMeas, _burstPhases, gate->iqTrip3);
      fields.ncp_trip3 = _mom->computeNcp(gate->iqTrip3);
    } else {
      fields.ncp_trip3 = MomentsFields::missingDouble;
    }

    if (_params.phase_decoding_ntrips_check > 3) {
      _pcode.cohereToTrip4(gate->iqMeas, _burstPhases, gate->iqTrip4);
      fields.ncp_trip4 = _mom->computeNcp(gate->iqTrip4);
    } else {
      fields.ncp_trip4 = MomentsFields::missingDouble;
    }

  } // igate

}
  
///////////////////////////////////////////////////////////
// Compute moments - SP
// Single pol, data in hc

void Beam::_computeMomSp()

{

  double noisePower = _calib.getNoiseDbmHc();
  double notchWidth = _params.notch_width_for_offzero_snr;

  // copy gate fields to _momFields array

  for (int igate = 0; igate < _nGates; igate++) {
    _momFields[igate] = _gateData[igate]->fields;
  }

  // phase code processing

  if (_applyPhaseDecoding) {

    // compute ncp for all appropriate trips
    
    for (int igate = 0; igate < _nGates; igate++) {
      GateData *gate = _gateData[igate];
      memcpy(gate->iqTrip1, gate->iqhc, _nSamples * sizeof(RadarComplex_t));
    }
    _computeTripNcp();

    // mitigate second trip

    for (int igate = 0; igate < _nGates; igate++) {
      
      GateData *gate = _gateData[igate];
      MomentsFields &fields = _momFields[igate];

      // check ncp
      
      if (fields.ncp_trip2 < _params.phase_decoding_ncp_threshold) {
        continue;
      }
      
      // notch out second trip

      _pcode.applyNotch(*_fft, gate->iqTrip2, _params.phase_decoding_notch_width);
      
      // cohere back to first trip

      _pcode.revertFromTrip2(gate->iqTrip2, _burstPhases, gate->iqMeas);
      _pcode.cohereToTrip1(gate->iqMeas, _burstPhases, gate->iqhc);
      
    } // igate

  } // if (_applyPhaseDecoding ...
  
  // compute covariances and prepare for noise comps
  
  for (int igate = 0; igate < _nGates; igate++) {
    GateData *gate = _gateData[igate];
    MomentsFields &fields = _momFields[igate];
    _mom->computeCovarSinglePol(gate->iqhc, fields);
    _mom->singlePolNoisePrep(fields.lag0_hc, fields.lag1_hc, fields);
  }
  
  // identify noise regions, and compute the mean noise
  // mean noise values are stored in moments
  
  _noise->computeNoiseSinglePol(_momFields);
  _noise->addToMoments(_momFields);
  
  // override noise for moments computations
  
  double noisePowerHc = _mom->getCalNoisePower(RadarMoments::CHANNEL_HC);
  if (_params.use_estimated_noise_for_noise_subtraction) {
    _mom->setEstimatedNoiseDbmHc(_noise->getMedianNoiseDbmHc());
    noisePowerHc = pow(10.0, _noise->getMedianNoiseDbmHc() / 10.0);
  }

  // compute main moments
  
  for (int igate = 0; igate < _nGates; igate++) {
    
    GateData *gate = _gateData[igate];
    MomentsFields &fields = _momFields[igate];
      
    _mom->computeMomSinglePol(fields.lag0_hc,
                              fields.lag1_hc,
                              fields.lag2_hc,
                              fields.lag3_hc,
                              igate,
                              fields);

    // wind farm check

    if (_checkForWindfarms &&
        fields.snrhc > _minSnrForWindfarmCheck &&
        fields.cpa > _minCpaForWindfarmCheck) {
      double spectralNoise, spectralSnr;
      _mom->computeSpectralSnr(_nSamples, *_fft,
                               gate->iqhc, gate->specHc,
                               noisePowerHc,
                               spectralNoise, spectralSnr);
      gate->specHcComputed = true;
      fields.spectral_noise = 10.0 * log10(spectralNoise);
      fields.spectral_snr = 10.0 * log10(spectralSnr);
    }
    
    gate->fields.ozsnr =
      _mom->computeOzSnr(gate->iqhcOrig, _windowVonHann, _nSamples,
                         *_fft, notchWidth, noisePower);
  } // igate

  // copy back to gate data

  for (int igate = 0; igate < _nGates; igate++) {
    _gateData[igate]->fields = _momFields[igate];
  }

}

///////////////////////////////////////////////////////////
// Compute moments - SP - phase coding applies
// Single pol, data in hc

void Beam::_computeMomSpSz()

{

  for (int igate = 0; igate < _nGates; igate++) {
    GateData *gate = _gateData[igate];
    _mom->singlePolSz864(*gate, _txDelta12, igate, _nGates, *_fft);
  }

}
      
///////////////////////////////////////////////////////////
// Compute moments - SP - staggered PRT mode
// Single pol, data in hc

void Beam::_computeMomSpStagPrt()

{

  // copy gate fields to _momFields array

  for (int igate = 0; igate < _nGates; igate++) {
    _momFields[igate] = _gateData[igate]->fields;
  }

  // prepare for noise comps
  
  for (int igate = 0; igate < _nGates; igate++) {
    GateData *gate = _gateData[igate];
    MomentsFields &fields = _momFields[igate];
    _mom->singlePolStagPrtNoisePrep(gate->iqhcOrig,
                                    gate->iqhcPrtShort,
                                    gate->iqhcPrtLong,
                                    fields);
  }
  
  // identify noise regions, and compute the mean noise
  // mean noise values are stored in moments
  
  _noise->computeNoiseSinglePol(_momFields);
  _noise->addToMoments(_momFields);
  
  // override noise for moments computations
  
  if (_params.use_estimated_noise_for_noise_subtraction) {
    _mom->setEstimatedNoiseDbmHc(_noise->getMedianNoiseDbmHc());
  }

  for (int igate = 0; igate < _nGates; igate++) {
      
    GateData *gate = _gateData[igate];
    MomentsFields &fields = _momFields[igate];
      
    // compute main moments
      
    _mom->singlePolStagPrt(gate->iqhcOrig,
                           gate->iqhcPrtShort,
                           gate->iqhcPrtLong,
                           igate, false, fields);
    
  } // igate

  // copy back to gate data

  for (int igate = 0; igate < _nGates; igate++) {
    _gateData[igate]->fields = _momFields[igate];
  }

}

///////////////////////////////////////////////////////////
// Compute moments DP_ALT_HV_CO_CROSS
// Transmit alternating, receive co/cross

void Beam::_computeMomDpAltHvCoCross()
{

  // copy gate fields to _momFields array

  for (int igate = 0; igate < _nGates; igate++) {
    _momFields[igate] = _gateData[igate]->fields;
  }

  // compute covariances and prepare for noise comps
  
  for (int igate = 0; igate < _nGates; igate++) {
    GateData *gate = _gateData[igate];
    MomentsFields &fields = _momFields[igate];
    _mom->computeCovarDpAltHvCoCross(gate->iqhc, gate->iqvc,
                                     gate->iqhx, gate->iqvx, 
                                     fields);
    _mom->dpAltHvCoCrossNoisePrep(fields.lag0_hc,
                                  fields.lag0_hx,
                                  fields.lag0_vc,
                                  fields.lag0_vx,
                                  fields.lag2_hc,
                                  fields.lag2_vc,
                                  fields);
  }
  
  // identify noise regions, and compute the mean noise
  // mean noise values are stored in moments
  
  _noise->computeNoiseDpAltHvCoCross(_momFields);
  _noise->addToMoments(_momFields);

  // override noise for moments computations
  
  double noisePowerHc = _mom->getCalNoisePower(RadarMoments::CHANNEL_HC);
  // double noisePowerVc = _mom->getCalNoisePower(RadarMoments::CHANNEL_VC);

  if (_params.use_estimated_noise_for_noise_subtraction) {
    _mom->setEstimatedNoiseDbmHc(_noise->getMedianNoiseDbmHc());
    _mom->setEstimatedNoiseDbmVc(_noise->getMedianNoiseDbmVc());
    noisePowerHc = pow(10.0, _noise->getMedianNoiseDbmHc() / 10.0);
    // noisePowerVc = pow(10.0, _noise->getMedianNoiseDbmVc() / 10.0);
  }
    
  for (int igate = 0; igate < _nGates; igate++) {
      
    GateData *gate = _gateData[igate];
    MomentsFields &fields = _momFields[igate];
      
    // compute moments for this gate

    _mom->computeMomDpAltHvCoCross(fields.lag0_hc,
                                   fields.lag0_hx,
                                   fields.lag0_vc,
                                   fields.lag0_vx,
                                   fields.lag0_vchx,
                                   fields.lag0_hcvx,
                                   fields.lag1_vxhx,
                                   fields.lag1_vchc,
                                   fields.lag1_hcvc,
                                   fields.lag2_hc,
                                   fields.lag2_vc,
                                   igate, 
                                   fields);
    
    
    // compute clutter detection quantities as needed
    
    if (_checkForWindfarms &&
        fields.snrhc > _minSnrForWindfarmCheck &&
        fields.cpa > _minCpaForWindfarmCheck) {
      double spectralNoise, spectralSnr;
      _mom->computeSpectralSnr(_nSamplesHalf, *_fftHalf,
                               gate->iqhc, gate->specHc,
                               noisePowerHc,
                               spectralNoise, spectralSnr);
      gate->specHcComputed = true;
      fields.spectral_noise = 10.0 * log10(spectralNoise);
      fields.spectral_snr = 10.0 * log10(spectralSnr);
    }
      
  } // igate

  // compute the alternating velocity
  
  _altVel.computeVelAlt(_nGates, _momFields, _nyquist);

  // copy back to gate data

  for (int igate = 0; igate < _nGates; igate++) {
    _gateData[igate]->fields = _momFields[igate];
  }

}

///////////////////////////////////////////////////////////
// Compute moments DP_ALT_HV_CO_ONLY
// Transmit alternating, receive copolar only

void Beam::_computeMomDpAltHvCoOnly()
{

  // copy gate fields to _momFields array

  for (int igate = 0; igate < _nGates; igate++) {
    _momFields[igate] = _gateData[igate]->fields;
  }

  // compute covariances and prepare for noise comps
  
  for (int igate = 0; igate < _nGates; igate++) {
    GateData *gate = _gateData[igate];
    MomentsFields &fields = _momFields[igate];
    _mom->computeCovarDpAltHvCoOnly(gate->iqhc, gate->iqvc, fields);
    _mom->dpAltHvCoOnlyNoisePrep(fields.lag0_hc,
                                 fields.lag0_vc,
                                 fields.lag2_hc,
                                 fields.lag2_vc,
                                 fields);
  }
  
  // identify noise regions, and compute the mean noise
  // mean noise values are stored in moments
  
  _noise->computeNoiseDpAltHvCoOnly(_momFields);
  _noise->addToMoments(_momFields);
  
  // override noise for moments computations
  
  double noisePowerHc = _mom->getCalNoisePower(RadarMoments::CHANNEL_HC);
  // double noisePowerVc = _mom->getCalNoisePower(RadarMoments::CHANNEL_VC);

  if (_params.use_estimated_noise_for_noise_subtraction) {
    _mom->setEstimatedNoiseDbmHc(_noise->getMedianNoiseDbmHc());
    _mom->setEstimatedNoiseDbmVc(_noise->getMedianNoiseDbmVc());
    noisePowerHc = pow(10.0, _noise->getMedianNoiseDbmHc() / 10.0);
    // noisePowerVc = pow(10.0, _noise->getMedianNoiseDbmVc() / 10.0);
  }
    
  for (int igate = 0; igate < _nGates; igate++) {
      
    GateData *gate = _gateData[igate];
    MomentsFields &fields = _momFields[igate];
      
    // compute moments for this gate
      
    _mom->computeMomDpAltHvCoOnly(fields.lag0_hc,
                                  fields.lag0_vc,
                                  fields.lag1_vchc,
                                  fields.lag1_hcvc,
                                  fields.lag2_hc,
                                  fields.lag2_vc,
                                  igate, 
                                  fields);
    
    // compute clutter detection quantities as needed
      
    if (_checkForWindfarms &&
        fields.snrhc > _minSnrForWindfarmCheck &&
        fields.cpa > _minCpaForWindfarmCheck) {
      double spectralNoise, spectralSnr;
      _mom->computeSpectralSnr(_nSamplesHalf, *_fftHalf,
                               gate->iqhc, gate->specHc,
                               noisePowerHc,
                               spectralNoise, spectralSnr);
      gate->specHcComputed = true;
      fields.spectral_noise = 10.0 * log10(spectralNoise);
      fields.spectral_snr = 10.0 * log10(spectralSnr);
    }
    
  } // igate

  // compute the alternating velocity

  _altVel.computeVelAlt(_nGates, _momFields, _nyquist);

  // copy back to gate data

  for (int igate = 0; igate < _nGates; igate++) {
    _gateData[igate]->fields = _momFields[igate];
  }

}

///////////////////////////////////////////////////////////
// Compute moments DP_SIM_HV_FIXED_HV
// Simultaneous transmission, fixed receive

void Beam::_computeMomDpSimHv()
{

  // staggered PRT is a special case

  if (_isStagPrt) {
    for (int igate = 0; igate < _nGates; igate++) {
      GateData *gate = _gateData[igate];
      MomentsFields &fields = gate->fields;
      _mom->dpSimHvStagPrt(gate->iqhcOrig,
                           gate->iqvcOrig,
                           gate->iqhcPrtShort,
                           gate->iqvcPrtShort,
                           gate->iqhcPrtLong,
                           gate->iqvcPrtLong,
                           igate, false, fields);
    }
    return;
  }

  // copy gate fields to _momFields array
  
  for (int igate = 0; igate < _nGates; igate++) {
    _momFields[igate] = _gateData[igate]->fields;
  }

  // compute covariances and prepare for noise comps
  
  for (int igate = 0; igate < _nGates; igate++) {
    GateData *gate = _gateData[igate];
    MomentsFields &fields = _momFields[igate];
    _mom->computeCovarDpSimHv(gate->iqhc, gate->iqvc, fields);
    _mom->dpSimHvNoisePrep(fields.lag0_hc,
                           fields.lag0_vc,
                           fields.lag1_hc,
                           fields.lag1_vc,
                           fields);
  }
  
  // identify noise regions, and compute the mean noise
  // mean noise values are stored in moments
  
  _noise->computeNoiseDpSimHv(_momFields);
  _noise->addToMoments(_momFields);
  
  // override noise for moments computations
  
  double noisePowerHc = _mom->getCalNoisePower(RadarMoments::CHANNEL_HC);
  // double noisePowerVc = _mom->getCalNoisePower(RadarMoments::CHANNEL_VC);

  if (_params.use_estimated_noise_for_noise_subtraction) {
    _mom->setEstimatedNoiseDbmHc(_noise->getMedianNoiseDbmHc());
    _mom->setEstimatedNoiseDbmVc(_noise->getMedianNoiseDbmVc());
    noisePowerHc = pow(10.0, _noise->getMedianNoiseDbmHc() / 10.0);
    // noisePowerVc = pow(10.0, _noise->getMedianNoiseDbmVc() / 10.0);
  }
    
  for (int igate = 0; igate < _nGates; igate++) {
      
    GateData *gate = _gateData[igate];
    MomentsFields &fields = _momFields[igate];
      
    // compute moments for this gate
      
    _mom->computeMomDpSimHv(fields.lag0_hc,
                            fields.lag0_vc,
                            fields.rvvhh0,
                            fields.lag1_hc,
                            fields.lag1_vc,
                            fields.lag2_hc,
                            fields.lag2_vc,
                            fields.lag3_hc,
                            fields.lag3_vc,
                            igate,
                            fields);
    
    // compute clutter detection quantities as needed
    
    if (_checkForWindfarms &&
        fields.snrhc > _minSnrForWindfarmCheck &&
        fields.cpa > _minCpaForWindfarmCheck) {
      double spectralNoise, spectralSnr;
      _mom->computeSpectralSnr(_nSamples, *_fft,
                               gate->iqhc, gate->specHc,
                               noisePowerHc,
                               spectralNoise, spectralSnr);
      gate->specHcComputed = true;
      fields.spectral_noise = 10.0 * log10(spectralNoise);
      fields.spectral_snr = 10.0 * log10(spectralSnr);
    }
    
  } // igate

  // copy back to gate data

  for (int igate = 0; igate < _nGates; igate++) {
    _gateData[igate]->fields = _momFields[igate];
  }

}

///////////////////////////////////////////////////////////
// Compute moments DP_SIM_HV_FIXED_HV
// Simultaneous transmission, fixed receive

void Beam::_computeMomDpSimHvStagPrt()
{

  // copy gate fields to _momFields array
  
  for (int igate = 0; igate < _nGates; igate++) {
    _momFields[igate] = _gateData[igate]->fields;
  }

  // compute covariances and prepare for noise comps
  
  for (int igate = 0; igate < _nGates; igate++) {
    GateData *gate = _gateData[igate];
    MomentsFields &fields = _momFields[igate];
    _mom->dpSimHvStagPrtNoisePrep(gate->iqhcOrig,
                                  gate->iqvcOrig,
                                  gate->iqhcPrtShort,
                                  gate->iqvcPrtShort,
                                  gate->iqhcPrtLong,
                                  gate->iqvcPrtLong,
                                  fields);
  }
  
  // identify noise regions, and compute the mean noise
  // mean noise values are stored in moments
  
  _noise->computeNoiseDpSimHv(_momFields);
  _noise->addToMoments(_momFields);
  
  // override noise for moments computations
  
  if (_params.use_estimated_noise_for_noise_subtraction) {
    _mom->setEstimatedNoiseDbmHc(_noise->getMedianNoiseDbmHc());
    _mom->setEstimatedNoiseDbmVc(_noise->getMedianNoiseDbmVc());
  }
    
  for (int igate = 0; igate < _nGates; igate++) {
      
    GateData *gate = _gateData[igate];
    MomentsFields &fields = _momFields[igate];
      
    // compute moments for this gate
      
    _mom->dpSimHvStagPrt(gate->iqhcOrig,
                         gate->iqvcOrig,
                         gate->iqhcPrtShort,
                         gate->iqvcPrtShort,
                         gate->iqhcPrtLong,
                         gate->iqvcPrtLong,
                         igate, false, fields);

  } // igate

  // copy back to gate data

  for (int igate = 0; igate < _nGates; igate++) {
    _gateData[igate]->fields = _momFields[igate];
  }

}

///////////////////////////////////////////////////////////
// Compute moments DP_H_ONLY_FIXED_HV
// H transmission, fixed dual receive

void Beam::_computeMomDpHOnly()
{

  // staggered PRT is a special case

  if (_isStagPrt) {
    for (int igate = 0; igate < _nGates; igate++) {
      GateData *gate = _gateData[igate];
      MomentsFields &fields = gate->fields;
      _mom->dpHOnlyStagPrt(gate->iqhcOrig,
                           gate->iqvxOrig,
                           gate->iqhcPrtShort,
                           gate->iqvxPrtShort,
                           gate->iqhcPrtLong,
                           gate->iqvxPrtLong,
                           igate,
                           false,
                           fields);
    }
    return;
  }

  // copy gate fields to _momFields array

  for (int igate = 0; igate < _nGates; igate++) {
    _momFields[igate] = _gateData[igate]->fields;
  }

  // compute covariances and prepare for noise comps
  
  for (int igate = 0; igate < _nGates; igate++) {
    GateData *gate = _gateData[igate];
    MomentsFields &fields = _momFields[igate];
    _mom->computeCovarDpHOnly(gate->iqhc, gate->iqvx, fields);
    _mom->dpHOnlyNoisePrep(fields.lag0_hc,
                           fields.lag0_vx,
                           fields.lag1_hc,
                           fields);
  }
  
  // identify noise regions, and compute the mean noise
  // mean noise values are stored in moments
  
  _noise->computeNoiseDpHOnly(_momFields);
  _noise->addToMoments(_momFields);
  
  // override noise for moments computations
  
  double noisePowerHc = _mom->getCalNoisePower(RadarMoments::CHANNEL_HC);

  if (_params.use_estimated_noise_for_noise_subtraction) {
    _mom->setEstimatedNoiseDbmHc(_noise->getMedianNoiseDbmHc());
    noisePowerHc = pow(10.0, _noise->getMedianNoiseDbmHc() / 10.0);
  }
    
  for (int igate = 0; igate < _nGates; igate++) {
      
    GateData *gate = _gateData[igate];
    MomentsFields &fields = _momFields[igate];
      
    // compute moments for this gate
    
    _mom->computeMomDpHOnly(fields.lag0_hc, 
                            fields.lag0_vx,
                            fields.lag1_hc,
                            fields.lag2_hc,
                            fields.lag3_hc,
                            igate,
                            fields);

    // compute clutter detection quantities as needed
      
    if (_checkForWindfarms &&
        fields.snrhc > _minSnrForWindfarmCheck &&
        fields.cpa > _minCpaForWindfarmCheck) {
      double spectralNoise, spectralSnr;
      _mom->computeSpectralSnr(_nSamples, *_fft,
                               gate->iqhc, gate->specHc,
                               noisePowerHc,
                               spectralNoise, spectralSnr);
      gate->specHcComputed = true;
      fields.spectral_noise = 10.0 * log10(spectralNoise);
      fields.spectral_snr = 10.0 * log10(spectralSnr);
    }
    
  } // igate

  // copy back to gate data

  for (int igate = 0; igate < _nGates; igate++) {
    _gateData[igate]->fields = _momFields[igate];
  }

}

///////////////////////////////////////////////////////////
// Compute moments DP_V_ONLY_FIXED_HV
// V transmission, fixed dual receive

void Beam::_computeMomDpVOnly()
{

  // staggered PRT is a special case
  
  if (_isStagPrt) {
    for (int igate = 0; igate < _nGates; igate++) {
      GateData *gate = _gateData[igate];
      MomentsFields &fields = gate->fields;
      _mom->dpVOnlyStagPrt(gate->iqvcOrig,
                           gate->iqhxOrig,
                           gate->iqvcPrtShort,
                           gate->iqhxPrtShort,
                           gate->iqvcPrtLong,
                           gate->iqhxPrtLong,
                           igate, false, fields);
    }
    return;
  }

  // copy gate fields to _momFields array

  for (int igate = 0; igate < _nGates; igate++) {
    _momFields[igate] = _gateData[igate]->fields;
  }

  // compute covariances and prepare for noise comps
  
  for (int igate = 0; igate < _nGates; igate++) {
    GateData *gate = _gateData[igate];
    MomentsFields &fields = _momFields[igate];
    _mom->computeCovarDpVOnly(gate->iqvc, gate->iqhx, fields);
    _mom->dpVOnlyNoisePrep(fields.lag0_vc,
                           fields.lag0_hx,
                           fields.lag1_vc,
                           fields);
  }
  
  // identify noise regions, and compute the mean noise
  // mean noise values are stored in moments
  
  _noise->computeNoiseDpVOnly(_momFields);
  _noise->addToMoments(_momFields);
  
  // override noise for moments computations
  
  double noisePowerVc = _mom->getCalNoisePower(RadarMoments::CHANNEL_VC);

  if (_params.use_estimated_noise_for_noise_subtraction) {
    _mom->setEstimatedNoiseDbmVc(_noise->getMedianNoiseDbmVc());
    noisePowerVc = pow(10.0, _noise->getMedianNoiseDbmVc() / 10.0);
  }
    
  for (int igate = 0; igate < _nGates; igate++) {
      
    GateData *gate = _gateData[igate];
    MomentsFields &fields = _momFields[igate];
      
    // compute moments for this gate
      
    _mom->computeMomDpVOnly(fields.lag0_vc, 
                            fields.lag0_hx,
                            fields.lag1_vc,
                            fields.lag2_vc,
                            fields.lag3_vc,
                            igate,
                            fields);
    
    // compute clutter detection quantities as needed
      
    if (_checkForWindfarms &&
        fields.snrvc > _minSnrForWindfarmCheck &&
        fields.cpa > _minCpaForWindfarmCheck) {
      double spectralNoise, spectralSnr;
      _mom->computeSpectralSnr(_nSamples, *_fft,
                               gate->iqvc, gate->specVc,
                               noisePowerVc,
                               spectralNoise, spectralSnr);
      gate->specVcComputed = true;
      fields.spectral_noise = 10.0 * log10(spectralNoise);
      fields.spectral_snr = 10.0 * log10(spectralSnr);
    }
    
  } // igate

  // copy back to gate data

  for (int igate = 0; igate < _nGates; igate++) {
    _gateData[igate]->fields = _momFields[igate];
  }

}

///////////////////////////////////////////////////////////
// Filter clutter SP
// Single pol, data in hc

void Beam::_filterSp()
{

  double calibNoise = _mom->getCalNoisePower(RadarMoments::CHANNEL_HC);

  for (int igate = 0; igate < _nGates; igate++) {
      
    GateData *gate = _gateData[igate];
    MomentsFields &fields = gate->fields;
    MomentsFields &fieldsF = gate->fieldsF;

    // check if we have clutter at this gate
    
    if (!gate->fields.cmd_flag) {
      continue;
    }
      
    // filter the HC time series
    
    double spectralNoise = 1.0e-13;
    double filterRatio = 1.0;
    double spectralSnr = 1.0;

    RadarComplex_t *specHc = NULL;
    if (gate->specHcComputed) {
      specHc = gate->specHc;
    }
    
    _mom->applyClutterFilter(_nSamples,
                             *_fft,
                             *_regr,
                             _window,
                             gate->iqhcOrig,
                             gate->iqhc, specHc,
                             calibNoise,
                             gate->iqhcF, NULL,
                             filterRatio,
                             spectralNoise,
                             spectralSnr);
    
    if (filterRatio > 1.0) {
      fields.clut_2_wx_ratio = 10.0 * log10(filterRatio - 1.0);
    } else {
      fields.clut_2_wx_ratio = MomentsFields::missingDouble;
    }
    fields.spectral_noise = 10.0 * log10(spectralNoise);
    fields.spectral_snr = 10.0 * log10(spectralSnr);
    
    // compute filtered moments for this gate
    
    _mom->computeCovarSinglePol(gate->iqhcF,
                                fieldsF);
    
    _mom->computeMomSinglePol(fieldsF.lag0_hc,
                              fieldsF.lag1_hc,
                              fieldsF.lag2_hc,
                              fieldsF.lag3_hc,
                              igate,
                              fieldsF);

    // compute clutter power
    
    gate->fields.clut = _computeClutPower(fields, fieldsF);
    
  } // igate

}

///////////////////////////////////////////////////////////
// Single Pol, staggered PRT filter

void Beam::_filterSpStagPrt()
  
{

  // NOTE - regression filter is not working correctly for
  // staggered
  
  // if (_params.use_polynomial_regression_clutter_filter) {
  //   _filterRegrSpStagPrt();
  // } else {
  _filterAdapSpStagPrt();
  // }

}    

//////////////////////////////////////////////
// Single Pol, staggered PRT, regression filter

void Beam::_filterRegrSpStagPrt()
{

  double calibNoise = _mom->getCalNoisePower(RadarMoments::CHANNEL_HC);

  for (int igate = 0; igate < _nGates; igate++) {
      
    GateData *gate = _gateData[igate];
    MomentsFields &fields = gate->fields;
    MomentsFields &fieldsF = gate->fieldsF;
      
    // check if we have clutter at this gate
    
    if (!fields.cmd_flag) {
      continue;
    }
      
    // filter the short prt time series
    
    double spectralNoise = 1.0e-13;
    double filterRatio = 1.0;
    double spectralSnr = 1.0;
    bool interpAcrossNotch = _params.regression_filter_interp_across_notch;

    memcpy(gate->iqhcF, gate->iqhcOrig, _nSamples * sizeof(RadarComplex_t));

    _mom->applyRegrFilterStagPrt(_nSamples,
                                 *_fftHalf,
                                 *_regrStag,
                                 gate->iqhcOrig,
                                 calibNoise,
                                 interpAcrossNotch,
                                 gate->iqhcF,
                                 filterRatio,
                                 spectralNoise,
                                 spectralSnr);
    
    RadarMoments::separateStagIq(_nSamples,
                                 gate->iqhcF,
                                 gate->iqhcPrtShortF,
                                 gate->iqhcPrtLongF);
    
    if (filterRatio > 1.0) {
      fields.clut_2_wx_ratio = 10.0 * log10(filterRatio - 1.0);
    } else {
      fields.clut_2_wx_ratio = MomentsFields::missingDouble;
    }
    fields.spectral_noise = 10.0 * log10(spectralNoise);
    fields.spectral_snr = 10.0 * log10(spectralSnr);
    
    // compute filtered moments for this gate
    
    _mom->singlePolStagPrt(gate->iqhc,
                           gate->iqhcPrtShortF,
                           gate->iqhcPrtLongF,
                           igate, true, fieldsF);
    
    // compute clutter power
    
    fields.clut = _computeClutPower(fields, fieldsF);
    
  } // igate

}

//////////////////////////////////////////////
// Single Pol, staggered PRT, adaptive filter

void Beam::_filterAdapSpStagPrt()
{

  double calibNoise = _mom->getCalNoisePower(RadarMoments::CHANNEL_HC);
  
  for (int igate = 0; igate < _nGates; igate++) {
      
    GateData *gate = _gateData[igate];
    MomentsFields &fields = gate->fields;
    MomentsFields &fieldsF = gate->fieldsF;
      
    // check if we have clutter at this gate
    
    if (!fields.cmd_flag) {
      continue;
    }
      
    // filter the short prt time series
    
    double spectralNoise = 1.0e-13;
    double filterRatio = 1.0;
    double spectralSnr = 1.0;

    _mom->applyAdapFilterStagPrt(_nSamplesHalf,
                                 *_fftHalf,
                                 gate->iqhcPrtShort,
                                 gate->iqhcPrtLong,
                                 calibNoise,
                                 gate->iqhcPrtShortF,
                                 gate->iqhcPrtLongF,
                                 filterRatio,
                                 spectralNoise,
                                 spectralSnr);
    
    if (filterRatio > 1.0) {
      fields.clut_2_wx_ratio = 10.0 * log10(filterRatio - 1.0);
    } else {
      fields.clut_2_wx_ratio = MomentsFields::missingDouble;
    }
    fields.spectral_noise = 10.0 * log10(spectralNoise);
    fields.spectral_snr = 10.0 * log10(spectralSnr);
    
    // compute filtered moments for this gate
    
    _mom->singlePolStagPrt(gate->iqhc,
                           gate->iqhcPrtShortF,
                           gate->iqhcPrtLongF,
                           igate, true, fieldsF);
    
    // compute clutter power
    
    fields.clut = _computeClutPower(fields, fieldsF);
    
  } // igate

}

//////////////////////////////////////////////////////
// Filter clutter SP SZ864
// Single pol, SZ phase coding

void Beam::_filterSpSz864()
{

  double calibNoise = _mom->getCalNoisePower(RadarMoments::CHANNEL_HC);

  for (int igate = 0; igate < _nGates; igate++) {
      
    GateData *gate = _gateData[igate];
    MomentsFields &fields = gate->fields;
    MomentsFields &fieldsF = gate->fieldsF;
      
    // check if we have clutter at this gate
    
    if (!fields.cmd_flag) {
      continue;
    }
    
    // copy the unfiltered fields across to the filtered fields struct
    
    fieldsF = fields;
    gate->secondTripF = gate->secondTrip;
    
    // apply the clutter filter
    
    double spectralNoise = 1.0e-13;
    double filterRatio = 1.0;
    double spectralSnr = 1.0;
    _mom->applyClutterFilterSz(_nSamples,
                               *_fft,
                               *gate,
                               calibNoise,
                               filterRatio,
                               spectralNoise,
                               spectralSnr);
    
    if (filterRatio > 1.0) {
      fields.clut_2_wx_ratio = 10.0 * log10(filterRatio - 1.0);
    } else {
      fields.clut_2_wx_ratio = MomentsFields::missingDouble;
    }
    fields.spectral_noise = 10.0 * log10(spectralNoise);
    fields.spectral_snr = 10.0 * log10(spectralSnr);
    
    // compute moments
    
    _mom->singlePolSz864Filtered(*gate, igate, _nGates);
    
    // compute clutter power
    
    fields.clut =
      _computeClutPower(fields, fieldsF);
    gate->secondTrip.clut =
      _computeClutPower(gate->secondTrip, gate->secondTripF);
    
  } // igate

}

///////////////////////////////////////////////////////////
// Filter clutter DP_ALT_HV_CO_CROSS
// Transmit alternating, receive co/cross

void Beam::_filterDpAltHvCoCross()
{

  // copy gate fields to _momFieldsF array

  for (int igate = 0; igate < _nGates; igate++) {
    _momFieldsF[igate] = _gateData[igate]->fieldsF;
  }

  double calibNoise = _mom->getCalNoisePower(RadarMoments::CHANNEL_HC);
  
  for (int igate = 0; igate < _nGates; igate++) {
      
    GateData *gate = _gateData[igate];
    MomentsFields &fields = gate->fields;
    MomentsFields &fieldsF = _momFieldsF[igate];
    
    // initialize rhohv test

    if (_params.apply_rhohv_test_after_cmd) {
      fields.test2 = 0;
      fields.test3 = 0;
    }
    
    // check if CMD identified clutter at this gate
    
    if (!fields.cmd_flag) {

      // should we apply the RHOHV improvement test?

      if (!_params.apply_rhohv_test_after_cmd) {
        continue;
      }

      // are we within the RHOHV limits for the test?

      if (fields.rhohv < _params.rhohv_test_min_rhohv ||
          fields.rhohv > _params.rhohv_test_max_rhohv) {
        continue;
      }

    }
      
    // filter the HC time series, save the filter ratio
    
    TaArray<double> _specRatio;
    double *specRatio = _specRatio.alloc(_nSamplesHalf);
    double spectralNoise = 1.0e-13;
    double filterRatio = 1.0;
    double spectralSnr = 1.0;

    RadarComplex_t *specHc = NULL;
    if (gate->specHcComputed) {
      specHc = gate->specHc;
    }
    
    _mom->applyClutterFilter(_nSamplesHalf,
			     *_fftHalf,
			     *_regrHalf,
			     _windowHalf,
			     gate->iqhcOrig,
			     gate->iqhc, specHc,
			     calibNoise,
			     gate->iqhcF,
			     gate->iqhcNotched,
			     filterRatio,
			     spectralNoise,
			     spectralSnr,
			     specRatio);

    if (filterRatio > 1.0) {
      fields.clut_2_wx_ratio = 10.0 * log10(filterRatio - 1.0);
    } else {
      fields.clut_2_wx_ratio = MomentsFields::missingDouble;
    }
    fields.spectral_noise = 10.0 * log10(spectralNoise);
    fields.spectral_snr = 10.0 * log10(spectralSnr);
    
    // apply the filter ratio to other channels
    
    _mom->applyFilterRatio(_nSamplesHalf, *_fftHalf,
                           gate->iqvc, specRatio,
                           gate->iqvcF, gate->iqvcNotched);
    
    _mom->applyFilterRatio(_nSamplesHalf, *_fftHalf,
                           gate->iqhx, specRatio,
                           gate->iqhxF, NULL);
    
    _mom->applyFilterRatio(_nSamplesHalf, *_fftHalf,
                           gate->iqvx, specRatio,
                           gate->iqvxF, NULL);
    
    // compute filtered moments for this gate
    
    _mom->computeCovarDpAltHvCoCross(gate->iqhcF, gate->iqvcF,
                                     gate->iqhxF, gate->iqvxF, 
                                     fieldsF);

    _mom->computeMomDpAltHvCoCross(fieldsF.lag0_hc,
                                   fieldsF.lag0_hx,
                                   fieldsF.lag0_vc,
                                   fieldsF.lag0_vx,
                                   fieldsF.lag0_vchx,
                                   fieldsF.lag0_hcvx,
                                   fieldsF.lag1_vxhx,
                                   fieldsF.lag1_vchc,
                                   fieldsF.lag1_hcvc,
                                   fieldsF.lag2_hc,
                                   fieldsF.lag2_vc,
                                   igate, 
                                   fieldsF);
    
    // compute clutter power
    
    fields.clut = _computeClutPower(fields, fieldsF);

    if (_params.apply_rhohv_test_after_cmd) {

      // compute rhohv improvement
      double factorUnfilt = 1.0 - fields.rhohv;
      double factorFilt = 1.0 - fieldsF.rhohv;
      if (factorFilt < 0.001) {
        factorFilt = 0.001;
      }
      double rhohvImprov = factorUnfilt / factorFilt;
      fields.test2 = rhohvImprov;

      if (!fields.cmd_flag) {
        // CMD did not indicate clutter
        // check if RHOHV improvement indicates clutter
        if (rhohvImprov >= _params.rhohv_improvement_factor_threshold) {
          // yes, so use filtered data for dual pol fields
          fields.test3 = 1;
          MomentsFields filt = fieldsF;
          fieldsF = fields;
          fieldsF.zdrm = filt.zdrm;
          fieldsF.zdr = filt.zdr;
          fieldsF.ldr = filt.ldr;
          fieldsF.rhohv = filt.rhohv;
          fieldsF.phidp = filt.phidp;
        } else {
          // no clutter, so revert to unfiltered data
          fieldsF = fields;
        }
      } // if (!fields.cmd_flag)

    } // if (_params.apply_rhohv_test_after_cmd)
    
  } // igate
  
  // compute the alternating velocity

  _altVel.computeVelAlt(_nGates, _momFieldsF, _nyquist);

  // copy back to gate data

  for (int igate = 0; igate < _nGates; igate++) {
    _gateData[igate]->fieldsF = _momFieldsF[igate];
  }

}

///////////////////////////////////////////////////////////
// Filter clutter DP_ALT_HV_CO_ONLY
// Transmit alternating, receive copolar only

void Beam::_filterDpAltHvCoOnly()
{

  // copy gate fields to _momFieldsF array

  for (int igate = 0; igate < _nGates; igate++) {
    _momFieldsF[igate] = _gateData[igate]->fieldsF;
  }

  double calibNoise = _mom->getCalNoisePower(RadarMoments::CHANNEL_HC);

  for (int igate = 0; igate < _nGates; igate++) {
      
#ifdef DEBUG_PRINT_SPECTRA      
      GlobGateNum = igate;
#endif
      
    GateData *gate = _gateData[igate];
    MomentsFields &fields = gate->fields;
    MomentsFields &fieldsF = _momFieldsF[igate];
      
    // check if we have clutter at this gate
    
    if (!fields.cmd_flag) {
      continue;
    }
      
    // filter the HC time series, save the filter ratio
    
    TaArray<double> _specRatio;
    double *specRatio = _specRatio.alloc(_nSamplesHalf);
    double spectralNoise = 1.0e-13;
    double filterRatio = 1.0;
    double spectralSnr = 1.0;
    
    RadarComplex_t *specHc = NULL;
    if (gate->specHcComputed) {
      specHc = gate->specHc;
    }
    
    _mom->applyClutterFilter(_nSamplesHalf,
                             *_fftHalf,
                             *_regrHalf,
                             _windowHalf,
                             gate->iqhcOrig,
                             gate->iqhc, specHc,
                             calibNoise,
                             gate->iqhcF, gate->iqhcNotched,
                             filterRatio,
                             spectralNoise,
                             spectralSnr,
                             specRatio);
    
    if (filterRatio > 1.0) {
      fields.clut_2_wx_ratio = 10.0 * log10(filterRatio - 1.0);
    } else {
      fields.clut_2_wx_ratio = MomentsFields::missingDouble;
    }
    fields.spectral_noise = 10.0 * log10(spectralNoise);
    fields.spectral_snr = 10.0 * log10(spectralSnr);
    
    // apply the filter ratio to other channels
    
    _mom->applyFilterRatio(_nSamplesHalf, *_fftHalf,
                           gate->iqvc, specRatio, 
                           gate->iqvcF, gate->iqvcNotched);
    
    // compute filtered moments for this gate
    
    _mom->computeCovarDpAltHvCoOnly(gate->iqhcF, gate->iqvcF,
                                    fieldsF);

    _mom->computeMomDpAltHvCoOnly(fieldsF.lag0_hc,
                                  fieldsF.lag0_vc,
                                  fieldsF.lag1_vchc,
                                  fieldsF.lag1_hcvc,
                                  fieldsF.lag2_hc,
                                  fieldsF.lag2_vc,
                                  igate, 
                                  fieldsF);
    
    // compute clutter power
    
    fields.clut = _computeClutPower(fields, fieldsF);
    
  } // igate
  
  // compute the alternating velocity

  _altVel.computeVelAlt(_nGates, _momFieldsF, _nyquist);

  // copy back to gate data

  for (int igate = 0; igate < _nGates; igate++) {
    _gateData[igate]->fieldsF = _momFieldsF[igate];
  }

}

///////////////////////////////////////////////////////////
// Dual pol, sim HV, fixed PRT filter

void Beam::_filterDpSimHvFixedPrt()
{

  double calibNoise = _mom->getCalNoisePower(RadarMoments::CHANNEL_HC);

  for (int igate = 0; igate < _nGates; igate++) {
      
    GateData *gate = _gateData[igate];
    MomentsFields &fields = gate->fields;
    MomentsFields &fieldsF = gate->fieldsF;
      
    // check if we have clutter at this gate
    
    if (!fields.cmd_flag) {
      continue;
    }
      
    // filter the HC time series
    
    TaArray<double> _specRatio;
    double *specRatio = _specRatio.alloc(_nSamples);
    double spectralNoise = 1.0e-13;
    double filterRatio = 1.0;
    double spectralSnr = 1.0;

    RadarComplex_t *specHc = NULL;
    if (gate->specHcComputed) {
      specHc = gate->specHc;
    }
    
    _mom->applyClutterFilter(_nSamples,
                             *_fft,
                             *_regr,
                             _window,
                             gate->iqhcOrig,
                             gate->iqhc, specHc,
                             calibNoise,
                             gate->iqhcF, gate->iqhcNotched,
                             filterRatio,
                             spectralNoise,
                             spectralSnr,
                             specRatio);

    if (filterRatio > 1.0) {
      fields.clut_2_wx_ratio = 10.0 * log10(filterRatio - 1.0);
    } else {
      fields.clut_2_wx_ratio = MomentsFields::missingDouble;
    }
    fields.spectral_noise = 10.0 * log10(spectralNoise);
    fields.spectral_snr = 10.0 * log10(spectralSnr);
    
    // apply the filter ratio to other channel
    
    _mom->applyFilterRatio(_nSamples, *_fft,
                           gate->iqvc, specRatio,
                           gate->iqvcF, gate->iqvcNotched);
    
    // compute filtered moments for this gate
    
    _mom->computeCovarDpSimHv(gate->iqhcF, gate->iqvcF, fieldsF);
      
    _mom->computeMomDpSimHv(fieldsF.lag0_hc,
                            fieldsF.lag0_vc,
                            fieldsF.rvvhh0,
                            fieldsF.lag1_hc,
                            fieldsF.lag1_vc,
                            fieldsF.lag2_hc,
                            fieldsF.lag2_vc,
                            fieldsF.lag3_hc,
                            fieldsF.lag3_vc,
                            igate,
                            fieldsF);
    
    // compute clutter power
    
    fields.clut = _computeClutPower(fields, fieldsF);

  } // igate

}

///////////////////////////////////////////////////////////
// Dual pol, sim HV, staggered PRT filter

void Beam::_filterDpSimHvStagPrt()
{

  double calibNoise = _mom->getCalNoisePower(RadarMoments::CHANNEL_HC);

  for (int igate = 0; igate < _nGates; igate++) {
      
    GateData *gate = _gateData[igate];
    MomentsFields &fields = gate->fields;
    MomentsFields &fieldsF = gate->fieldsF;
      
    // check if we have clutter at this gate
    
    if (!fields.cmd_flag) {
      continue;
    }
      
    // filter the HC time series
    
    TaArray<double> _specRatioShort, _specRatioLong;
    double *specRatioShort = _specRatioShort.alloc(_nSamplesHalf);
    double *specRatioLong = _specRatioLong.alloc(_nSamplesHalf);
    double spectralNoise = 1.0e-13;
    double filterRatio = 1.0;
    double spectralSnr = 1.0;
    _mom->applyAdapFilterStagPrt(_nSamplesHalf,
                                 *_fftHalf,
                                 gate->iqhcPrtShort,
                                 gate->iqhcPrtLong,
                                 calibNoise,
                                 gate->iqhcPrtShortF,
                                 gate->iqhcPrtLongF,
                                 filterRatio,
                                 spectralNoise,
                                 spectralSnr,
                                 specRatioShort,
                                 specRatioLong);
    
    if (filterRatio > 1.0) {
      fields.clut_2_wx_ratio = 10.0 * log10(filterRatio - 1.0);
    } else {
      fields.clut_2_wx_ratio = MomentsFields::missingDouble;
    }
    fields.spectral_noise = 10.0 * log10(spectralNoise);
    fields.spectral_snr = 10.0 * log10(spectralSnr);
    
    // apply the filter ratio to other channel
    
    _mom->applyFilterRatio(_nSamplesHalf, *_fftHalf,
                           gate->iqvcPrtShort, specRatioShort,
                           gate->iqvcPrtShortF, NULL);
    _mom->applyFilterRatio(_nSamplesHalf, *_fftHalf,
                           gate->iqvcPrtLong, specRatioLong,
                           gate->iqvcPrtLongF, NULL);
    
    // compute filtered moments for this gate
    
    _mom->dpSimHvStagPrt(gate->iqhc,
                         gate->iqvc,
                         gate->iqhcPrtShortF,
                         gate->iqvcPrtShortF,
                         gate->iqhcPrtLongF,
                         gate->iqvcPrtLongF,
                         igate, true, fieldsF);

    // compute clutter power
    
    fields.clut = _computeClutPower(fields, fieldsF);

  } // igate

}

///////////////////////////////////////////////////////////
// Filter clutter DP_H_ONLY_FIXED_HV - fixed PRT
// Transmit H, fixed receive

void Beam::_filterDpHOnlyFixedPrt()
{

  double calibNoise = _mom->getCalNoisePower(RadarMoments::CHANNEL_HC);

  for (int igate = 0; igate < _nGates; igate++) {
      
    GateData *gate = _gateData[igate];
    MomentsFields &fields = gate->fields;
    MomentsFields &fieldsF = gate->fieldsF;
      
    // check if we have clutter at this gate
    
    if (!fields.cmd_flag) {
      continue;
    }
      
    // filter the HC time series
    
    TaArray<double> _specRatio;
    double *specRatio = _specRatio.alloc(_nSamples);
    double spectralNoise = 1.0e-13;
    double filterRatio = 1.0;
    double spectralSnr = 1.0;

    RadarComplex_t *specHc = NULL;
    if (gate->specHcComputed) {
      specHc = gate->specHc;
    }
    
    _mom->applyClutterFilter(_nSamples,
                             *_fft,
                             *_regr,
                             _window,
                             gate->iqhcOrig,
                             gate->iqhc, specHc,
                             calibNoise,
                             gate->iqhcF, NULL,
                             filterRatio,
                             spectralNoise,
                             spectralSnr,
                             specRatio);

    if (filterRatio > 1.0) {
      fields.clut_2_wx_ratio = 10.0 * log10(filterRatio - 1.0);
    } else {
      fields.clut_2_wx_ratio = MomentsFields::missingDouble;
    }
    fields.spectral_noise = 10.0 * log10(spectralNoise);
    fields.spectral_snr = 10.0 * log10(spectralSnr);
    
    // apply the filter ratio to other channel
    
    _mom->applyFilterRatio(_nSamples, *_fft,
                           gate->iqvx, specRatio, gate->iqvxF, NULL);
    
    // compute filtered moments for this gate
    
    _mom->computeCovarDpHOnly(gate->iqhcF, gate->iqvxF, fieldsF);
    
    _mom->computeMomDpHOnly(fieldsF.lag0_hc, 
                            fieldsF.lag0_vx,
                            fieldsF.lag1_hc,
                            fieldsF.lag2_hc,
                            fieldsF.lag3_hc,
                            igate,
                            fieldsF);

    // compute clutter power
    
    fields.clut = _computeClutPower(fields, fieldsF);

  } // igate

}

///////////////////////////////////////////////////////////
// Filter clutter DP_H_ONLY_FIXED_HV - staggered PRT
// Transmit H, fixed receive

void Beam::_filterDpHOnlyStagPrt()
{

  double calibNoise = _mom->getCalNoisePower(RadarMoments::CHANNEL_HC);

  for (int igate = 0; igate < _nGates; igate++) {
      
    GateData *gate = _gateData[igate];
    MomentsFields &fields = gate->fields;
    MomentsFields &fieldsF = gate->fieldsF;
      
    // check if we have clutter at this gate
    
    if (!fields.cmd_flag) {
      continue;
    }
      
    // SHORT PRT
    // filter the HC time series

    TaArray<double> _specRatioShort, _specRatioLong;
    double *specRatioShort = _specRatioShort.alloc(_nSamplesHalf);
    double *specRatioLong = _specRatioLong.alloc(_nSamplesHalf);

    double spectralNoise = 1.0e-13;
    double filterRatio = 1.0;
    double spectralSnr = 1.0;
    _mom->applyAdapFilterStagPrt(_nSamplesHalf,
                                 *_fftHalf,
                                 gate->iqhcPrtShort,
                                 gate->iqhcPrtLong,
                                 calibNoise,
                                 gate->iqhcPrtShortF,
                                 gate->iqhcPrtLongF,
                                 filterRatio,
                                 spectralNoise,
                                 spectralSnr,
                                 specRatioShort,
                                 specRatioLong);
    
    if (filterRatio > 1.0) {
      fields.clut_2_wx_ratio = 10.0 * log10(filterRatio - 1.0);
    } else {
      fields.clut_2_wx_ratio = MomentsFields::missingDouble;
    }
    fields.spectral_noise = 10.0 * log10(spectralNoise);
    fields.spectral_snr = 10.0 * log10(spectralSnr);
    
    // apply the filter ratio to other channel
    
    _mom->applyFilterRatio(_nSamplesHalf, *_fftHalf,
                           gate->iqvxPrtShort, specRatioShort,
                           gate->iqvxPrtShortF, NULL);
    
    _mom->applyFilterRatio(_nSamplesHalf, *_fftHalf,
                           gate->iqvxPrtLong, specRatioLong,
                           gate->iqvxPrtLongF, NULL);
    
    // compute filtered moments for this gate
    
    _mom->dpHOnlyStagPrt(gate->iqhc,
                         gate->iqvx,
                         gate->iqhcPrtShortF,
                         gate->iqvxPrtShortF,
                         gate->iqhcPrtLongF,
                         gate->iqvxPrtLongF,
                         igate, true, fieldsF);

    // compute clutter power
    
    fields.clut = _computeClutPower(fields, fieldsF);

  } // igate

}

///////////////////////////////////////////////////////////
// Filter clutter DP_V_ONLY_FIXED_HV - fixed PRT
// Transmit V, fixed receive

void Beam::_filterDpVOnlyFixedPrt()
{

  double calibNoise = _mom->getCalNoisePower(RadarMoments::CHANNEL_VC);
  
  for (int igate = 0; igate < _nGates; igate++) {
      
    GateData *gate = _gateData[igate];
    MomentsFields &fields = gate->fields;
    MomentsFields &fieldsF = gate->fieldsF;
      
    // check if we have clutter at this gate
    
    if (!fields.cmd_flag) {
      continue;
    }
      
    // filter the HC time series
    
    TaArray<double> _specRatio;
    double *specRatio = _specRatio.alloc(_nSamples);
    double spectralNoise = 1.0e-13;
    double filterRatio = 1.0;
    double spectralSnr = 1.0;

    RadarComplex_t *specVc = NULL;
    if (gate->specVcComputed) {
      specVc = gate->specVc;
    }
    
    _mom->applyClutterFilter(_nSamples,
                             *_fft,
                             *_regr,
                             _window,
                             gate->iqvcOrig,
                             gate->iqvc, specVc,
                             calibNoise,
                             gate->iqvcF, NULL,
                             filterRatio,
                             spectralNoise,
                             spectralSnr,
                             specRatio);

    if (filterRatio > 1.0) {
      fields.clut_2_wx_ratio = 10.0 * log10(filterRatio - 1.0);
    } else {
      fields.clut_2_wx_ratio = MomentsFields::missingDouble;
    }
    fields.spectral_noise = 10.0 * log10(spectralNoise);
    fields.spectral_snr = 10.0 * log10(spectralSnr);
    
    // apply the filter ratio to other channel
    
    _mom->applyFilterRatio(_nSamples, *_fft,
                           gate->iqhx, specRatio,
                           gate->iqhxF, NULL);
    
    // compute filtered moments for this gate
    
    _mom->computeCovarDpVOnly(gate->iqvcF, gate->iqhxF, fieldsF);
    
    _mom->computeMomDpVOnly(fieldsF.lag0_vc, 
                            fieldsF.lag0_hx,
                            fieldsF.lag1_vc,
                            fieldsF.lag2_vc,
                            fieldsF.lag3_vc,
                            igate,
                            fieldsF);

    // compute clutter power
    
    fields.clut = _computeClutPower(fields, fieldsF);

  } // igate

}

///////////////////////////////////////////////////////////
// Filter clutter DP_V_ONLY_FIXED_HV - staggered PRT
// Transmit H, fixed receive

void Beam::_filterDpVOnlyStagPrt()
{

  double calibNoise = _mom->getCalNoisePower(RadarMoments::CHANNEL_VC);
  
  for (int igate = 0; igate < _nGates; igate++) {
    
    GateData *gate = _gateData[igate];
    MomentsFields &fields = gate->fields;
    MomentsFields &fieldsF = gate->fieldsF;
    
    // check if we have clutter at this gate
    
    if (!fields.cmd_flag) {
      continue;
    }
      
    // SHORT PRT
    // filter the VC time series

    TaArray<double> _specRatioShort, _specRatioLong;
    double *specRatioShort = _specRatioShort.alloc(_nSamplesHalf);
    double *specRatioLong = _specRatioLong.alloc(_nSamplesHalf);

    double spectralNoise = 1.0e-13;
    double filterRatio = 1.0;
    double spectralSnr = 1.0;
    _mom->applyAdapFilterStagPrt(_nSamplesHalf,
                                 *_fftHalf,
                                 gate->iqvcPrtShort,
                                 gate->iqvcPrtLong,
                                 calibNoise,
                                 gate->iqvcPrtShortF,
                                 gate->iqvcPrtLongF,
                                 filterRatio,
                                 spectralNoise,
                                 spectralSnr,
                                 specRatioShort,
                                 specRatioLong);
    
    if (filterRatio > 1.0) {
      fields.clut_2_wx_ratio = 10.0 * log10(filterRatio - 1.0);
    } else {
      fields.clut_2_wx_ratio = MomentsFields::missingDouble;
    }
    fields.spectral_noise = 10.0 * log10(spectralNoise);
    fields.spectral_snr = 10.0 * log10(spectralSnr);
    
    // apply the filter ratio to other channel
    
    _mom->applyFilterRatio(_nSamplesHalf, *_fftHalf,
                           gate->iqhxPrtShort, specRatioShort,
                           gate->iqhxPrtShortF, NULL);
    
    _mom->applyFilterRatio(_nSamplesHalf, *_fftHalf,
                           gate->iqhxPrtLong, specRatioLong,
                           gate->iqhxPrtLongF, NULL);
    
    // compute filtered moments for this gate
    
    _mom->dpVOnlyStagPrt(gate->iqvc,
                         gate->iqhx,
                         gate->iqvcPrtShortF,
                         gate->iqhxPrtShortF,
                         gate->iqvcPrtLongF,
                         gate->iqhxPrtLongF,
                         igate, true, fieldsF);

    // compute clutter power
    
    fields.clut = _computeClutPower(fields, fieldsF);

  } // igate

}

//////////////////////////////////////////
// compute R1, R2 and R3 values for window

void Beam::_computeWindowRValues()
  
{

  _windowR1 = RadarMoments::computeWindowCorrelation(1, _window, _nSamples);
  _windowR2 = RadarMoments::computeWindowCorrelation(2, _window, _nSamples);
  _windowR3 = RadarMoments::computeWindowCorrelation(3, _window, _nSamples);

  _windowHalfR1 =
    RadarMoments::computeWindowCorrelation(1, _windowHalf, _nSamplesHalf);
  _windowHalfR2 =
    RadarMoments::computeWindowCorrelation(2, _windowHalf, _nSamplesHalf);
  _windowHalfR3 =
    RadarMoments::computeWindowCorrelation(3, _windowHalf, _nSamplesHalf);

  if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
    cerr << "Window R values" << endl;
    cerr << " windowR1: " << _windowR1 << endl;
    cerr << " windowR2: " << _windowR2 << endl;
    cerr << " windowR3: " << _windowR3 << endl;
    cerr << " windowHalfR1: " << _windowHalfR1 << endl;
    cerr << " windowHalfR2: " << _windowHalfR2 << endl;
    cerr << " windowHalfR3: " << _windowHalfR3 << endl;
  }

}

////////////////////////////////////////////////
// override OpsInfo time-series values as needed
  
void Beam::_overrideOpsInfo()

{

  if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
    cerr << "---->> Pulse info BEFORE overrides <<----" << endl;
    _opsInfo.print(stderr);
    cerr << "---->> End of Pulse info BEFORE overrides <<----" << endl;
  }

  if (_params.override_radar_name) {
    _opsInfo.overrideRadarName(_params.radar_name);
  }
  if (_params.override_radar_location) {
    _opsInfo.overrideRadarLocation(_params.radar_altitude_meters,
				   _params.radar_latitude_deg,
				   _params.radar_longitude_deg);
  }
  if (_params.override_gate_geometry) {
    _opsInfo.overrideGateGeometry(_params.start_range_meters,
                                  _params.gate_spacing_meters);
  }
  if (_params.override_radar_wavelength) {
    _opsInfo.overrideWavelength(_params.radar_wavelength_cm);
  }
    
  if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
    cerr << "---->> Pulse info AFTER overrides <<----" << endl;
    _opsInfo.print(stderr);
    cerr << "---->> End of Pulse info AFTER overrides <<----" << endl;
  }

}

//////////////////////////////
// compute windows for FFTs
  
void Beam::_computeWindows()

{

  _freeWindows();

  if (_mmgr.applyClutterFilter() &&
      _params.use_polynomial_regression_clutter_filter) {
    _window = RadarMoments::createWindowRect(_nSamples);
    _windowHalf = RadarMoments::createWindowRect(_nSamplesHalf);
  } else if (_mmgr.getWindowType() == Params::WINDOW_RECT) {
    if (_mmgr.applyClutterFilter()) {
      if (_nWarnings % 1000 == 0 ||
          _params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "WARNING: Iq2Dsr::Beam::Beam" << endl;
        cerr << "  For clutter filtering a window is required" << endl;
        cerr << "  VON HANN window will be used" << endl;
      }
      _nWarnings++;
      _window = RadarMoments::createWindowVonhann(_nSamples);
      _windowHalf = RadarMoments::createWindowVonhann(_nSamplesHalf);
    } else if (_mmgr.applyPhaseDecoding()) {
      if (_nWarnings % 1000 == 0 ||
          _params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "WARNING: Iq2Dsr::Beam::Beam" << endl;
        cerr << "  For phase decoding a window is required" << endl;
        cerr << "  VON HANN window will be used" << endl;
      }
      _nWarnings++;
      _window = RadarMoments::createWindowVonhann(_nSamples);
      _windowHalf = RadarMoments::createWindowVonhann(_nSamplesHalf);
    } else {
      _window = RadarMoments::createWindowRect(_nSamples);
      _windowHalf = RadarMoments::createWindowRect(_nSamplesHalf);
    }
  } else if (_mmgr.getWindowType() == Params::WINDOW_VONHANN) {
    _window = RadarMoments::createWindowVonhann(_nSamples);
    _windowHalf = RadarMoments::createWindowVonhann(_nSamplesHalf);
  } else if (_mmgr.getWindowType() == Params::WINDOW_BLACKMAN) {
    _window = RadarMoments::createWindowBlackman(_nSamples);
    _windowHalf = RadarMoments::createWindowBlackman(_nSamplesHalf);
  } else if (_mmgr.getWindowType() == Params::WINDOW_BLACKMAN_NUTTALL) {
    _window = RadarMoments::createWindowBlackmanNuttall(_nSamples);
    _windowHalf = RadarMoments::createWindowBlackmanNuttall(_nSamplesHalf);
  }

  _windowVonHann = RadarMoments::createWindowVonhann(_nSamples);

  // compute window R values, used for corrections in Spectrum width

  _computeWindowRValues();

}


//////////////////////////////////////
// initialize moments computations object
  
void Beam::_initMomentsObject()
  
{
  
  _mom->setNSamples(_nSamples);

  _mom->setApplySpectralResidueCorrection
    (_params.apply_residue_correction_in_adaptive_filter,
     _params.min_snr_db_for_residue_correction);
  
  _mom->setUseAdaptiveFilter();
  
  if (_params.use_polynomial_regression_clutter_filter) {
    _mom->setUseRegressionFilter
      (_params.regression_filter_interp_across_notch);
  } else if (_params.use_simple_notch_clutter_filter) {
    _mom->setUseSimpleNotchFilter(_params.simple_notch_filter_width_mps);
  }

  if (_params.correct_for_system_phidp) {
    _mom->setCorrectForSystemPhidp(true);
  } else {
    _mom->setCorrectForSystemPhidp(false);
  }

  if (_params.change_aiq_sign) {
    _mom->setChangeAiqSign(true);
  } else {
    _mom->setChangeAiqSign(false);
  }
  
  if (_mmgr.changeVelocitySign()) {
    _mom->setChangeVelocitySign(true);
  } else {
    _mom->setChangeVelocitySign(false);
  }
  
  if (_params.change_velocity_sign_staggered) {
    _mom->setChangeVelocitySignStaggered(true);
  } else {
    _mom->setChangeVelocitySignStaggered(false);
  }
  
  if (_params.change_phidp_sign) {
    _mom->setChangePhidpSign(true);
  } else {
    _mom->setChangePhidpSign(false);
  }
  
  if (_applySz1) {
    _mom->setSz(_params.phase_decoding_snr_threshold,
                _params.sz1_negate_phase_codes,
                _params.sz1_strong_to_weak_power_ratio_threshold,
                _params.sz1_out_of_trip_power_ratio_threshold,
                _params.sz1_out_of_trip_power_n_replicas);
  }

  if (_params.apply_db_for_db_correction) {
    _mom->setDbForDb(_params.db_for_db_ratio,
                     _params.db_for_db_threshold);
  }
  
  _mom->setWindowRValues(_windowR1,
                         _windowR2,
                         _windowR3,
                         _windowHalfR1,
                         _windowHalfR2,
                         _windowHalfR3);

  _mom->setTssNotchWidth(3);

  if (_params.cpa_compute_using_alternative) {
    _mom->setComputeCpaUsingAlt();
  }

  if (_isStagPrt) {
    _mom->initStagPrt(_prtShort,
                      _prtLong,
                      _stagM,
                      _stagN,
                      _nGatesPrtShort,
                      _nGatesPrtLong,
                      _opsInfo);
  } else {
    _mom->init(_prt, _opsInfo);
  }
  
  _mom->loadAtmosAttenCorrection(_nGatesOut, _el, *_atmosAtten);

}

//////////////////////////////////////
// initialize for KDP
  
void Beam::_kdpInit()

{

  _needKdp = false;
  for (int ii = 0; ii < _params.output_fields_n; ii++) {
    const Params::output_field_t &field = _params._output_fields[ii];
    if (field.id == Params::KDP) {
      if (field.write_unfiltered) {
        _needKdp = true;
        break;
      }
    }
  }

  _needKdpFiltered = false;
  for (int ii = 0; ii < _params.output_fields_n; ii++) {
    const Params::output_field_t &field = _params._output_fields[ii];
    if (field.id == Params::KDP) {
      if (field.write_filtered) {
        _needKdpFiltered = true;
        break;
      }
    }
  }
  
  // initialize KDP object

  if (_params.KDP_fir_filter_len == Params::FIR_LEN_125) {
    _kdp.setFIRFilterLen(KdpFilt::FIR_LENGTH_125);
  } else if (_params.KDP_fir_filter_len == Params::FIR_LEN_60) {
    _kdp.setFIRFilterLen(KdpFilt::FIR_LENGTH_60);
  } else if (_params.KDP_fir_filter_len == Params::FIR_LEN_40) {
    _kdp.setFIRFilterLen(KdpFilt::FIR_LENGTH_40);
  } else if (_params.KDP_fir_filter_len == Params::FIR_LEN_30) {
    _kdp.setFIRFilterLen(KdpFilt::FIR_LENGTH_30);
  } else if (_params.KDP_fir_filter_len == Params::FIR_LEN_20) {
    _kdp.setFIRFilterLen(KdpFilt::FIR_LENGTH_20);
  } else {
    _kdp.setFIRFilterLen(KdpFilt::FIR_LENGTH_10);
  }
  _kdp.setNGatesStats(_params.KDP_ngates_for_stats);
  _kdp.setNFiltIterUnfolded(_params.KDP_n_filt_iterations_unfolded);
  _kdp.setNFiltIterCond(_params.KDP_n_filt_iterations_conditioned);
  if (_params.KDP_use_iterative_filtering) {
    _kdp.setUseIterativeFiltering(true);
    _kdp.setPhidpDiffThreshold(_params.KDP_phidp_difference_threshold);
  }
  _kdp.setPhidpSdevMax(_params.KDP_phidp_sdev_max);
  _kdp.setPhidpJitterMax(_params.KDP_phidp_jitter_max);
  _kdp.checkSnr(_params.KDP_check_snr);
  _kdp.setSnrThreshold(_params.KDP_snr_threshold);
  _kdp.checkRhohv(_params.KDP_check_rhohv);
  _kdp.setRhohvThreshold(_params.KDP_rhohv_threshold);
  if (_params.KDP_check_zdr_sdev) {
    _kdp.checkZdrSdev(true);
  }
  _kdp.setZdrSdevMax(_params.KDP_zdr_sdev_max);
  _kdp.setMinValidAbsKdp(_params.KDP_min_valid_abs_kdp);

  if (_params.KDP_debug) {
    _kdp.setDebug(true);
  }


  if (_params.apply_precip_attenuation_correction) {
    if (_params.specify_coefficients_for_attenuation_correction) {
      _kdp.setAttenCoeffs(_params.dbz_attenuation_coefficient,
                          _params.dbz_attenuation_exponent,
                          _params.zdr_attenuation_coefficient,
                          _params.zdr_attenuation_exponent);
    } else {
      _kdp.setComputeAttenCorr(true);
    }
  }

  // initialize KDP object - hubbert/bringi

  if (_params.KDP_HB_fir_filter_len == Params::FIR_LEN_125) {
    _kdpB.setFIRFilterLen(KdpBringi::FIR_LENGTH_125);
  } else if (_params.KDP_HB_fir_filter_len == Params::FIR_LEN_30) {
    _kdpB.setFIRFilterLen(KdpBringi::FIR_LENGTH_30);
  } else if (_params.KDP_HB_fir_filter_len == Params::FIR_LEN_20) {
    _kdpB.setFIRFilterLen(KdpBringi::FIR_LENGTH_20);
  } else {
    _kdpB.setFIRFilterLen(KdpBringi::FIR_LENGTH_10);
  }
  _kdpB.setPhidpDiffThreshold(_params.KDP_HB_phidp_difference_threshold);
  _kdpB.setPhidpSdevThreshold(_params.KDP_HB_phidp_sdev_threshold);
  _kdpB.setZdrSdevThreshold(_params.KDP_HB_zdr_sdev_threshold);
  _kdpB.setRhohvWxThreshold(_params.KDP_HB_rhohv_threshold);

}

////////////////////////////////////////////////
// compute kdp from phidp, using Bringi's method

void Beam::_kdpCompute(bool isFiltered)
  
{

  // make sure arrays are allocated
  
  _snrArray = _snrArray_.alloc(_nGates);
  _dbzArray = _dbzArray_.alloc(_nGates);
  _zdrArray = _zdrArray_.alloc(_nGates);
  _rhohvArray = _rhohvArray_.alloc(_nGates);
  _phidpArray = _phidpArray_.alloc(_nGates);
  
  // copy input data from Fields into arrays
  
  if (isFiltered) {
    for (int ii = 0; ii < _nGates; ii++) {
      _snrArray[ii] = _gateData[ii]->fieldsF.snr;
      _dbzArray[ii] = _gateData[ii]->fieldsF.dbz;
      _zdrArray[ii] = _gateData[ii]->fieldsF.zdr;
      _rhohvArray[ii] = _gateData[ii]->fieldsF.rhohv;
      _phidpArray[ii] = _gateData[ii]->fieldsF.phidp;
    }
  } else {
    for (int ii = 0; ii < _nGates; ii++) {
      _snrArray[ii] = _gateData[ii]->fields.snr;
      _dbzArray[ii] = _gateData[ii]->fields.dbz;
      _zdrArray[ii] = _gateData[ii]->fields.zdr;
      _rhohvArray[ii] = _gateData[ii]->fields.rhohv;
      _phidpArray[ii] = _gateData[ii]->fields.phidp;
    }
  }

  // write KDP ray files?
  
  if (_params.KDP_write_ray_files) {
    string rayDir = _params.KDP_ray_files_dir;
    if (isFiltered) {
      rayDir += "_filt";
    }
    _kdp.setWriteRayFile(true, rayDir);
  }

  // compute KDP
  
  _kdp.compute(_timeSecs,
               _nanoSecs / 1.0e9,
               _el,
               _az,
               _opsInfo.get_radar_wavelength_cm(),
               _nGates, 
               _startRangeKm,
               _gateSpacingKm,
               _snrArray,
               _dbzArray,
               _zdrArray,
               _rhohvArray,
               _phidpArray,
               _missingDbl);
  
  // put KDP into fields objects
  
  const double *kdp = _kdp.getKdp();
  const double *psob = _kdp.getPsob();
  const double *phidpCond = _kdp.getPhidpCondFilt(); 
  const double *phidpFilt = _kdp.getPhidpFilt();
  const double *phidpSdev = _kdp.getPhidpSdev();
  const double *phidpJitter = _kdp.getPhidpJitter();
  const double *zdrSdev = _kdp.getZdrSdev();
  
  const double *dbzAttenCorr = _kdp.getDbzAttenCorr();
  const double *zdrAttenCorr = _kdp.getZdrAttenCorr();
  
  // put KDP into fields objects
  
  if (isFiltered) {
    for (int ii = 0; ii < _nGates; ii++) {
      if (kdp[ii] != _missingDbl) {
	_gateData[ii]->fieldsF.kdp = kdp[ii];
      }
      if (psob[ii] != _missingDbl) {
	_gateData[ii]->fieldsF.psob = psob[ii];
      }
      if (phidpCond[ii] != _missingDbl) {
	_gateData[ii]->fieldsF.phidp_cond = phidpCond[ii];
      }
      if (phidpFilt[ii] != _missingDbl) {
	_gateData[ii]->fieldsF.phidp_filt = phidpFilt[ii];
      }
      if (phidpSdev[ii] != _missingDbl) {
	_gateData[ii]->fieldsF.phidp_sdev_4kdp = phidpSdev[ii];
      }
      if (phidpJitter[ii] != _missingDbl) {
	_gateData[ii]->fieldsF.phidp_jitter_4kdp = phidpJitter[ii];
      }
      if (zdrSdev[ii] != _missingDbl) {
	_gateData[ii]->fieldsF.zdr_sdev_4kdp = zdrSdev[ii];
      }
      if (_params.apply_precip_attenuation_correction) {
	_gateData[ii]->fieldsF.dbz_atten_correction = dbzAttenCorr[ii];
	_gateData[ii]->fieldsF.zdr_atten_correction = zdrAttenCorr[ii];
        if (_gateData[ii]->fieldsF.dbz != _missingDbl) {
          _gateData[ii]->fieldsF.dbz_atten_corrected =
            _gateData[ii]->fieldsF.dbz + dbzAttenCorr[ii];
        }
        if (_gateData[ii]->fieldsF.zdr != _missingDbl) {
          _gateData[ii]->fieldsF.zdr_atten_corrected =
            _gateData[ii]->fieldsF.zdr + zdrAttenCorr[ii];
        }
      }
    }
  } else {
    for (int ii = 0; ii < _nGates; ii++) {
      if (kdp[ii] != _missingDbl) {
	_gateData[ii]->fields.kdp = kdp[ii];
      }
      if (psob[ii] != _missingDbl) {
	_gateData[ii]->fields.psob = psob[ii];
      }
      if (phidpCond[ii] != _missingDbl) {
	_gateData[ii]->fields.phidp_cond = phidpCond[ii];
      }
      if (phidpFilt[ii] != _missingDbl) {
	_gateData[ii]->fields.phidp_filt = phidpFilt[ii];
      }
      if (phidpSdev[ii] != _missingDbl) {
	_gateData[ii]->fields.phidp_sdev_4kdp = phidpSdev[ii];
      }
      if (phidpJitter[ii] != _missingDbl) {
	_gateData[ii]->fields.phidp_jitter_4kdp = phidpJitter[ii];
      }
      if (zdrSdev[ii] != _missingDbl) {
	_gateData[ii]->fields.zdr_sdev_4kdp = zdrSdev[ii];
      }
      if (_params.apply_precip_attenuation_correction) {
	_gateData[ii]->fields.dbz_atten_correction = dbzAttenCorr[ii];
	_gateData[ii]->fields.zdr_atten_correction = zdrAttenCorr[ii];
        if (_gateData[ii]->fields.dbz != _missingDbl) {
          _gateData[ii]->fields.dbz_atten_corrected =
            _gateData[ii]->fields.dbz + dbzAttenCorr[ii];
        }
        if (_gateData[ii]->fields.zdr != _missingDbl) {
          _gateData[ii]->fields.zdr_atten_corrected =
            _gateData[ii]->fields.zdr + zdrAttenCorr[ii];
        }
      }
    }
  }


}

////////////////////////////////////////////////
// compute kdp from phidp

void Beam::_kdpComputeBringi(bool isFiltered)
  
{

  // check if we need to do this?

  if (!_dualPol) {
    return;
  }

  // copy from Fields into arrays

  TaArray<double> snr_, dbz_, phidp_, rhohv_, zdr_;
  double *snr = snr_.alloc(_nGates);
  double *dbz = dbz_.alloc(_nGates);
  double *phidp = phidp_.alloc(_nGates);
  double *rhohv = rhohv_.alloc(_nGates);
  double *zdr = zdr_.alloc(_nGates);
  if (isFiltered) {
    for (int ii = 0; ii < _nGates; ii++) {
      snr[ii] = _gateData[ii]->fieldsF.snr;
      dbz[ii] = _gateData[ii]->fieldsF.dbz;
      phidp[ii] = _gateData[ii]->fieldsF.phidp;
      rhohv[ii] = _gateData[ii]->fieldsF.rhohv;
      zdr[ii] = _gateData[ii]->fieldsF.zdr;
    }
  } else {
    for (int ii = 0; ii < _nGates; ii++) {
      snr[ii] = _gateData[ii]->fields.snr;
      dbz[ii] = _gateData[ii]->fields.dbz;
      phidp[ii] = _gateData[ii]->fields.phidp;
      rhohv[ii] = _gateData[ii]->fields.rhohv;
      zdr[ii] = _gateData[ii]->fields.zdr;
    }
  }

  // set up array for range
  
  TaArray<double> rangeKm_;
  double *rangeKm = rangeKm_.alloc(_nGates);
  double range = _startRangeKm;
  for (int ii = 0; ii < _nGates; ii++, range += _gateSpacingKm) {
    rangeKm[ii] = range;
  }

  // compute KDP
  
  _kdpB.compute(_el, _az, _nGates, rangeKm, dbz, zdr,
                phidp, rhohv, snr, _missingDbl);
  
  const double *kdp = _kdpB.getKdp();

  // put KDP into fields objects
  
  if (isFiltered) {
    for (int ii = 0; ii < _nGates; ii++) {
      if (kdp[ii] != _missingDbl) {
	_gateData[ii]->fieldsF.kdp_hb = kdp[ii];
      }
    }
  } else {
    for (int ii = 0; ii < _nGates; ii++) {
      if (kdp[ii] != _missingDbl) {
	_gateData[ii]->fields.kdp_hb = kdp[ii];
      }
    }
  }
  
}

/////////////////////////////////////////////////
// compute velocity corrected for platform motion
  
void Beam::_computeVelocityCorrectedForMotion()

{

  // no good if no georeference available

  if (!_georefActive) {
    return;
  }

  // compute the velocity from the platform motion

  double cosEl, sinEl;
  ta_sincos(_el * DEG_TO_RAD, &sinEl, &cosEl);
  double vertCorr = 0.0;
  if (_georef.vert_velocity_mps > -9990) {
    vertCorr = _georef.vert_velocity_mps * sinEl;
  }
  
  // double cosAz, sinAz;
  // ta_sincos(_az * DEG_TO_RAD, &sinAz, &cosAz);
  // double ewCorr = 0.0;
  // if (_georef.ew_velocity_mps > -9990) {
  //   ewCorr = _georef.ew_velocity_mps * fabs(cosEl) * sinAz;
  // }
  // double nsCorr = 0.0;
  // if (_georef.ns_velocity_mps > -9990) {
  //   nsCorr = _georef.ns_velocity_mps * fabs(cosEl) * cosAz;
  // }

  // cerr << "111111 head, el, az, vert, ew, ns, vertCorr, ewCorr, nsCorr: "
  //      << _georef.heading_deg << ", "
  //      << _el << ", " << _az << ", "
  //      << _georef.vert_velocity_mps << ", "
  //      << _georef.ew_velocity_mps << ", "
  //      << _georef.ns_velocity_mps << ", "
  //      << vertCorr << ", "
  //      << ewCorr << ", "
  //      << nsCorr << endl;

  // for now just use the vertical vel correction

  double correction = vertCorr;

  if (correction == 0.0) {
    // no change needed
    return;
  }

  for (int ii = 0; ii < _nGates; ii++) {
    
    double velCorr = _gateData[ii]->fields.vel;
    if (velCorr != _missingDbl) {
      velCorr += correction;
      // check nyquist interval
      while (velCorr > _nyquist) {
        velCorr -= 2.0 * _nyquist;
      }
      while (velCorr < -_nyquist) {
        velCorr += 2.0 * _nyquist;
      }
      _gateData[ii]->fields.vel_corrected = velCorr;
    }

    velCorr = _gateData[ii]->fieldsF.vel;
    if (velCorr != _missingDbl) {
      velCorr += correction;
      // check nyquist interval
      while (velCorr > _nyquist) {
        velCorr -= 2.0 * _nyquist;
      }
      while (velCorr < -_nyquist) {
        velCorr += 2.0 * _nyquist;
      }
      _gateData[ii]->fieldsF.vel_corrected = velCorr;
    }

  } // ii

}

//////////////////////////////////////
// initialize noise computations
  
int Beam::_noiseInit()
  
{

  if (_noise == NULL) {
    _noise = new NoiseLocator();
  }

  // initialize noise location
  
  _noise->setNGatesKernel(_params.noise_ngates_kernel);

  if (_params.noise_method == Params::NOISE_RUNNING_MEDIAN) {
    _noise->setComputeRunningMedian(_params.noise_ngates_for_running_median);
  } else {
    _noise->setComputeRayMedian(_params.noise_min_ngates_for_ray_median);
  }

  if (_params.set_equal_noise_bias_in_all_channels) {
    _noise->setEqualBiasInAllChannels(true);
  }

  // interest maps for for noise
  
  vector<InterestMap::ImPoint> pts;
  if (_convertInterestParamsToVector
      ("phase_change_error_for_noise",
       _params._phase_change_error_for_noise_interest_map,
       _params.phase_change_error_for_noise_interest_map_n,
       pts)) {
    return -1;
  }
  _noise->setInterestMapPhaseChangeErrorForNoise
    (pts, _params.phase_change_error_for_noise_interest_weight);

  if (_convertInterestParamsToVector
      ("dbm_sdev_for_noise",
       _params._dbm_sdev_for_noise_interest_map,
       _params.dbm_sdev_for_noise_interest_map_n,
       pts)) {
    return -1;
  }
  _noise->setInterestMapDbmSdevForNoise
    (pts, _params.dbm_sdev_for_noise_interest_weight);

  if (_convertInterestParamsToVector
      ("ncp_mean_sdev_for_noise",
       _params._ncp_mean_for_noise_interest_map,
       _params.ncp_mean_for_noise_interest_map_n,
       pts)) {
    return -1;
  }
  _noise->setInterestMapNcpMeanForNoise
    (pts, _params.ncp_mean_for_noise_interest_weight);

  _noise->setInterestThresholdForNoise
    (_params.interest_threshold_for_noise);

  // interest maps for for signal
  
  if (_convertInterestParamsToVector
      ("phase_change_error_for_signal",
       _params._phase_change_error_for_signal_interest_map,
       _params.phase_change_error_for_signal_interest_map_n,
       pts)) {
    return -1;
  }
  _noise->setInterestMapPhaseChangeErrorForSignal
    (pts, _params.phase_change_error_for_signal_interest_weight);

  if (_convertInterestParamsToVector
      ("dbm_sdev_for_signal",
       _params._dbm_sdev_for_signal_interest_map,
       _params.dbm_sdev_for_signal_interest_map_n,
       pts)) {
    return -1;
  }
  _noise->setInterestMapDbmSdevForSignal
    (pts, _params.dbm_sdev_for_signal_interest_weight);

  _noise->setInterestThresholdForSignal
    (_params.interest_threshold_for_signal);

  if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
    _noise->printParams(cerr);
  }

  return 0;

}

////////////////////////////////////////////////////////////////////////
// Convert interest map points to vector
//
// Returns 0 on success, -1 on failure

int Beam::_convertInterestParamsToVector(const string &label,
                                         const Params::interest_map_point_t *map,
                                         int nPoints,
                                         vector<InterestMap::ImPoint> &pts)

{
  
  pts.clear();
  
  double prevVal = -1.0e99;
  for (int ii = 0; ii < nPoints; ii++) {
    if (map[ii].value <= prevVal) {
      pthread_mutex_lock(&_debugPrintMutex);
      cerr << "ERROR - Beam::_convertInterestParamsToVector" << endl;
      cerr << "  Map label: " << label << endl;
      cerr << "  Map values must increase monotonically" << endl;
      pthread_mutex_unlock(&_debugPrintMutex);
      return -1;
    }
    InterestMap::ImPoint pt(map[ii].value, map[ii].interest);
    pts.push_back(pt);
    prevVal = map[ii].value;
  } // ii
  
  return 0;

}

////////////////////////////////////////////////////////////////
// Check for antenna transiion
//
// set transition for beam if both the start and end pulses are
// in transition

void Beam::_checkAntennaTransition(const vector<const IwrfTsPulse *> &pulses)

{

  const IwrfTsPulse *midPulse = pulses[_nSamplesHalf];

  _antennaTransition = false;
  size_t nPulses = pulses.size();
  switch (_params.transition_method) {
  case Params::TRANSITION_FLAG_AT_CENTER:
    if (midPulse->get_antenna_transition()) {
      _antennaTransition = true;
    }
    break;
  case Params::TRANSITION_FLAG_AT_BOTH_ENDS:
    if (pulses[0]->get_antenna_transition() &&
	pulses[nPulses-1]->get_antenna_transition()) {
      _antennaTransition = true;
    }
    break;
  case Params::TRANSITION_FLAG_AT_EITHER_END:
    if (pulses[0]->get_antenna_transition() ||
	pulses[nPulses-1]->get_antenna_transition()) {
      _antennaTransition = true;
    }
    break;
  default:
    _antennaTransition = false;
  }

  if (_antennaTransition) {
    return;
  }

  // check for transition from fixed angle error

  if (_params.check_transition_from_fixed_angle_error) {

    if (_scanMode == IWRF_SCAN_MODE_RHI ||
        _scanMode == IWRF_SCAN_MODE_EL_SURV ||
        _scanMode == IWRF_SCAN_MODE_MANRHI ||
        _scanMode == IWRF_SCAN_MODE_SUNSCAN_RHI) {

      // RHI

      double error = fabs(_targetAz - _az);
      if (error > 180) {
        error = fabs(error - 360.0);
      }
      if (error > _params.max_fixed_angle_error_rhi) {
        _antennaTransition = true;
      }

    } else {

      // PPI

      double error = fabs(_targetEl - _el);
      if (error > _params.max_fixed_angle_error_ppi) {
        _antennaTransition = true;
      }

    }


  }
  
}

/////////////////////////////////////////////////////////////////
// Allocate or re-allocate gate data

void Beam::_allocGateData(int nGates)

{
  int nNeeded = nGates - (int) _gateData.size();
  if (nNeeded > 0) {
    for (int ii = 0; ii < nNeeded; ii++) {
      GateData *gate = new GateData();
      _gateData.push_back(gate);
    }
  }
  for (size_t ii = 0; ii < _gateData.size(); ii++) {
    _gateData[ii]->allocArrays(_nSamples, _applyFiltering, _isStagPrt, _applySz1);
  }
  _momFields = _momFields_.alloc(_gateData.size());
  _momFieldsF = _momFieldsF_.alloc(_gateData.size());
  _censorFlags = _censorFlags_.alloc(_gateData.size());
}

/////////////////////////////////////////////////////////////////
// Free gate data

void Beam::_freeGateData()

{
  for (int ii = 0; ii < (int) _gateData.size(); ii++) {
    delete _gateData[ii];
  }
  _gateData.clear();
}

/////////////////////////////////////////////////////////////////
// Initialize field data at each gate

void Beam::_initFieldData()

{
  for (int ii = 0; ii < (int) _gateData.size(); ii++) {
    _gateData[ii]->initFields();
    _censorFlags[ii] = false;
  }
}

/////////////////////////////////////////////////////////////////
// Load gate IQ data.
// Apply window as appropriate.
// How this is loaded depends on the polarization mode.
//
// Assumptions:
// 1. For alternating mode, first pulse in sequence is H.
// 2. For non-switching dual receivers,
//    H is channel 0 and V channel 1.
// 3. For single pol mode, data is in channel 0.

void Beam::_loadGateIq(const fl32 **iqChan0,
                       const fl32 **iqChan1)
  
{

  switch (_xmitRcvMode) {
    
    case IWRF_ALT_HV_CO_ONLY: {
      
      // assumes first pulse is H xmit

      int nBytesComplex = _nSamplesHalf * sizeof(RadarComplex_t);
      for (int igate = 0, ipos = 0; igate < _nGates; igate++, ipos += 2) {
        GateData *gate = _gateData[igate];
        // original data
        RadarComplex_t *iqhc = gate->iqhcOrig;
        RadarComplex_t *iqvc = gate->iqvcOrig;
        for (int isamp = 0, jsamp = 0; isamp < _nSamplesHalf;
             isamp++, iqhc++, iqvc++) {
          iqhc->re = iqChan0[jsamp][ipos];
          iqhc->im = iqChan0[jsamp][ipos + 1];
          jsamp++;
          iqvc->re = iqChan0[jsamp][ipos];
          iqvc->im = iqChan0[jsamp][ipos + 1];
          jsamp++;
        }
        // windowed data
        memcpy(gate->iqhc, gate->iqhcOrig, nBytesComplex);
        memcpy(gate->iqvc, gate->iqvcOrig, nBytesComplex);
        RadarMoments::applyWindow(gate->iqhc, _windowHalf, _nSamplesHalf);
        RadarMoments::applyWindow(gate->iqvc, _windowHalf, _nSamplesHalf);
      }
    
    } break;
        
    case IWRF_ALT_HV_CO_CROSS: {

      // assumes first pulse is H xmit
      
      int nBytesComplex = _nSamplesHalf * sizeof(RadarComplex_t);
      for (int igate = 0, ipos = 0; igate < _nGates; igate++, ipos += 2) {
        GateData *gate = _gateData[igate];
        // original data
        RadarComplex_t *iqhc = gate->iqhcOrig;
        RadarComplex_t *iqvc = gate->iqvcOrig;
        RadarComplex_t *iqhx = gate->iqhxOrig;
        RadarComplex_t *iqvx = gate->iqvxOrig;
        for (int isamp = 0, jsamp = 0; isamp < _nSamplesHalf;
             isamp++, iqhc++, iqvc++, iqhx++, iqvx++) {
          iqhc->re = iqChan0[jsamp][ipos];
          iqhc->im = iqChan0[jsamp][ipos + 1];
          if (iqChan1) {
            iqvx->re = iqChan1[jsamp][ipos];
            iqvx->im = iqChan1[jsamp][ipos + 1];
          }
          jsamp++;
          iqvc->re = iqChan0[jsamp][ipos];
          iqvc->im = iqChan0[jsamp][ipos + 1];
          if (iqChan1) {
            iqhx->re = iqChan1[jsamp][ipos];
            iqhx->im = iqChan1[jsamp][ipos + 1];
          }
          jsamp++;
        }
        // windowed data
        memcpy(gate->iqhc, gate->iqhcOrig, nBytesComplex);
        memcpy(gate->iqvc, gate->iqvcOrig, nBytesComplex);
        memcpy(gate->iqhx, gate->iqhxOrig, nBytesComplex);
        memcpy(gate->iqvx, gate->iqvxOrig, nBytesComplex);
        RadarMoments::applyWindow(gate->iqhc, _windowHalf, _nSamplesHalf);
        RadarMoments::applyWindow(gate->iqvc, _windowHalf, _nSamplesHalf);
        RadarMoments::applyWindow(gate->iqhx, _windowHalf, _nSamplesHalf);
        RadarMoments::applyWindow(gate->iqvx, _windowHalf, _nSamplesHalf);
      }

    } break;

    case IWRF_ALT_HV_FIXED_HV: {

      // not switching
      int nBytesComplex = _nSamplesHalf * sizeof(RadarComplex_t);
      for (int igate = 0, ipos = 0; igate < _nGates; igate++, ipos += 2) {
        GateData *gate = _gateData[igate];
        // original data
        RadarComplex_t *iqhc = gate->iqhcOrig;
        RadarComplex_t *iqvc = gate->iqvcOrig;
        RadarComplex_t *iqhx = gate->iqhxOrig;
        RadarComplex_t *iqvx = gate->iqvxOrig;
        for (int isamp = 0, jsamp = 0; isamp < _nSamplesHalf;
             isamp++, iqhc++, iqvc++, iqhx++, iqvx++) {
          iqhc->re = iqChan0[jsamp][ipos];
          iqhc->im = iqChan0[jsamp][ipos + 1];
          if (iqChan1) {
            iqhx->re = iqChan1[jsamp][ipos];
            iqhx->im = iqChan1[jsamp][ipos + 1];
          }
          jsamp++;
          iqvx->re = iqChan0[jsamp][ipos];
          iqvx->im = iqChan0[jsamp][ipos + 1];
          if (iqChan1) {
            iqvc->re = iqChan1[jsamp][ipos];
            iqvc->im = iqChan1[jsamp][ipos + 1];
          }
          jsamp++;
        }
        // windowed data
        memcpy(gate->iqhc, gate->iqhcOrig, nBytesComplex);
        memcpy(gate->iqvc, gate->iqvcOrig, nBytesComplex);
        memcpy(gate->iqhx, gate->iqhxOrig, nBytesComplex);
        memcpy(gate->iqvx, gate->iqvxOrig, nBytesComplex);
        RadarMoments::applyWindow(gate->iqhc, _windowHalf, _nSamplesHalf);
        RadarMoments::applyWindow(gate->iqvc, _windowHalf, _nSamplesHalf);
        RadarMoments::applyWindow(gate->iqhx, _windowHalf, _nSamplesHalf);
        RadarMoments::applyWindow(gate->iqvx, _windowHalf, _nSamplesHalf);
      }

    } break;

    case IWRF_SIM_HV_FIXED_HV: {

      int nBytesComplex = _nSamples * sizeof(RadarComplex_t);
      for (int igate = 0, ipos = 0; igate < _nGates; igate++, ipos += 2) {
        GateData *gate = _gateData[igate];
        // original data
        RadarComplex_t *iqhc = gate->iqhcOrig;
        RadarComplex_t *iqvc = gate->iqvcOrig;
        for (int isamp = 0; isamp < _nSamples; isamp++, iqhc++, iqvc++) {
          iqhc->re = iqChan0[isamp][ipos];
          iqhc->im = iqChan0[isamp][ipos + 1];
          if (iqChan1) {
            iqvc->re = iqChan1[isamp][ipos];
            iqvc->im = iqChan1[isamp][ipos + 1];
          }
        }
        // windowed data
        memcpy(gate->iqhc, gate->iqhcOrig, nBytesComplex);
        memcpy(gate->iqvc, gate->iqvcOrig, nBytesComplex);
        RadarMoments::applyWindow(gate->iqhc, _window, _nSamples);
        RadarMoments::applyWindow(gate->iqvc, _window, _nSamples);
      }
      
    } break;

    case IWRF_SIM_HV_SWITCHED_HV: {

      int nBytesComplex = _nSamples * sizeof(RadarComplex_t);
      for (int igate = 0, ipos = 0; igate < _nGates; igate++, ipos += 2) {
        GateData *gate = _gateData[igate];
        // original data
        RadarComplex_t *iqhc = gate->iqhcOrig;
        RadarComplex_t *iqvc = gate->iqvcOrig;
        for (int isamp = 0; isamp < _nSamples; isamp++, iqhc++, iqvc++) {
          if (isamp % 2 == 0) {
            iqhc->re = iqChan0[isamp][ipos];
            iqhc->im = iqChan0[isamp][ipos + 1];
            if (iqChan1) {
              iqvc->re = iqChan1[isamp][ipos];
              iqvc->im = iqChan1[isamp][ipos + 1];
            }
          } else {
            iqvc->re = iqChan0[isamp][ipos];
            iqvc->im = iqChan0[isamp][ipos + 1];
            if (iqChan1) {
              iqhc->re = iqChan1[isamp][ipos];
              iqhc->im = iqChan1[isamp][ipos + 1];
            }
          }
        }
        // windowed data
        memcpy(gate->iqhc, gate->iqhcOrig, nBytesComplex);
        memcpy(gate->iqvc, gate->iqvcOrig, nBytesComplex);
        RadarMoments::applyWindow(gate->iqhc, _window, _nSamples);
        RadarMoments::applyWindow(gate->iqvc, _window, _nSamples);
      }
      
    } break;

    case IWRF_H_ONLY_FIXED_HV: {

      int nBytesComplex = _nSamples * sizeof(RadarComplex_t);
      for (int igate = 0, ipos = 0; igate < _nGates; igate++, ipos += 2) {
        GateData *gate = _gateData[igate];
        // original data
        RadarComplex_t *iqhc = gate->iqhcOrig;
        RadarComplex_t *iqvx = gate->iqvxOrig;
        for (int isamp = 0; isamp < _nSamples; isamp++, iqhc++, iqvx++) {
          iqhc->re = iqChan0[isamp][ipos];
          iqhc->im = iqChan0[isamp][ipos + 1];
          if (iqChan1) {
            iqvx->re = iqChan1[isamp][ipos];
            iqvx->im = iqChan1[isamp][ipos + 1];
          }
        }
        // windowed data
        memcpy(gate->iqhc, gate->iqhcOrig, nBytesComplex);
        memcpy(gate->iqvx, gate->iqvxOrig, nBytesComplex);
        RadarMoments::applyWindow(gate->iqhc, _window, _nSamples);
        RadarMoments::applyWindow(gate->iqvx, _window, _nSamples);
      }
    
    } break;
    
    case IWRF_V_ONLY_FIXED_HV: {

      int nBytesComplex = _nSamples * sizeof(RadarComplex_t);
      for (int igate = 0, ipos = 0; igate < _nGates; igate++, ipos += 2) {
        GateData *gate = _gateData[igate];
        // original data
        RadarComplex_t *iqhx = gate->iqhxOrig;
        RadarComplex_t *iqvc = gate->iqvcOrig;
        for (int isamp = 0; isamp < _nSamples; isamp++, iqhx++, iqvc++) {
          iqhx->re = iqChan0[isamp][ipos];
          iqhx->im = iqChan0[isamp][ipos + 1];
          if (iqChan1) {
            iqvc->re = iqChan1[isamp][ipos];
            iqvc->im = iqChan1[isamp][ipos + 1];
          }
        }
        // windowed data
        memcpy(gate->iqhx, gate->iqhxOrig, nBytesComplex);
        memcpy(gate->iqvc, gate->iqvcOrig, nBytesComplex);
        RadarMoments::applyWindow(gate->iqhx, _window, _nSamples);
        RadarMoments::applyWindow(gate->iqvc, _window, _nSamples);
      }
    
    } break;

    case IWRF_SINGLE_POL:
    default: {
    
      int nBytesComplex = _nSamples * sizeof(RadarComplex_t);
      for (int igate = 0, ipos = 0; igate < _nGates; igate++, ipos += 2) {
        GateData *gate = _gateData[igate];
        // original data
        RadarComplex_t *iqhc = gate->iqhcOrig;
        for (int isamp = 0; isamp < _nSamples; isamp++, iqhc++) {
          iqhc->re = iqChan0[isamp][ipos];
          iqhc->im = iqChan0[isamp][ipos + 1];
        }
        // windowed data
        memcpy(gate->iqhc, gate->iqhcOrig, nBytesComplex);
        RadarMoments::applyWindow(gate->iqhc, _window, _nSamples);
      }

    } break;
    
  } // switch;

}

/////////////////////////////////////////////////////////////////
// Load gate IQ data for staggered PRT data
// Apply window as appropriate.
// How this is loaded depends on the polarization mode.
//
// Assumptions:
// 1. First pulse in sequence is short PRT.
// 2. For non-switching dual receivers,
//    H is channel 0 and V channel 1.
// 3. For single pol mode, data is in channel 0.

void Beam::_loadGateIqStagPrt(const fl32 **iqChan0,
                              const fl32 **iqChan1)
  
{
  
  // Note - First pulse is a short-PRT pulse

  switch (_xmitRcvMode) {
    
    case IWRF_SIM_HV_FIXED_HV: {

      int nBytesComplex = _nSamplesHalf * sizeof(RadarComplex_t);

      // short prt data
      
      for (int igate = 0, ipos = 0; igate < _nGatesPrtShort;
           igate++, ipos += 2) {
        
        GateData *gate = _gateData[igate];
        
        // original data - full series
        
        RadarComplex_t *iqhcOrig = gate->iqhcOrig;
        RadarComplex_t *iqvcOrig = gate->iqvcOrig;
        for (int isamp = 0; isamp < _nSamples; isamp++, iqhcOrig++, iqvcOrig++) {
          iqhcOrig->re = iqChan0[isamp][ipos];
          iqhcOrig->im = iqChan0[isamp][ipos+1];
          iqvcOrig->re = iqChan1[isamp][ipos];
          iqvcOrig->im = iqChan1[isamp][ipos+1];
        }
       
        // short PRT from input sequence, which starts with short
        
        RadarComplex_t *iqhcShort = gate->iqhcPrtShortOrig;
        RadarComplex_t *iqvcShort = gate->iqvcPrtShortOrig;
        for (int isamp = 0, jsamp = 0; isamp < _nSamplesHalf;
             isamp++, iqhcShort++, iqvcShort++) {
          iqhcShort->re = iqChan0[jsamp][ipos];
          iqhcShort->im = iqChan0[jsamp][ipos+1];
          iqvcShort->re = iqChan1[jsamp][ipos];
          iqvcShort->im = iqChan1[jsamp][ipos+1];
          jsamp++;
          jsamp++;
        }
        
        // windowed data

        memcpy(gate->iqhc, gate->iqhcOrig, _nSamples * sizeof(RadarComplex_t));
        memcpy(gate->iqvc, gate->iqvcOrig, _nSamples * sizeof(RadarComplex_t));
        RadarMoments::applyWindow(gate->iqhc, _window, _nSamples);
        RadarMoments::applyWindow(gate->iqvc, _window, _nSamples);

        memcpy(gate->iqhcPrtShort, gate->iqhcPrtShortOrig, nBytesComplex);
        RadarMoments::applyWindow(gate->iqhcPrtShort, _windowHalf, _nSamplesHalf);
        memcpy(gate->iqvcPrtShort, gate->iqvcPrtShortOrig, nBytesComplex);
        RadarMoments::applyWindow(gate->iqvcPrtShort, _windowHalf, _nSamplesHalf);
        
      } // igate < _ngatesPrtShort

      // long prt data

      for (int igate = 0, ipos = 0; igate < _nGatesPrtLong; igate++, ipos += 2) {

        GateData *gate = _gateData[igate];

        // long PRT from input sequence, which starts with short

        RadarComplex_t *iqhcLong = gate->iqhcPrtLongOrig;
        RadarComplex_t *iqvcLong = gate->iqvcPrtLongOrig;
        for (int isamp = 0, jsamp = 0; isamp < _nSamplesHalf;
             isamp++, iqhcLong++, iqvcLong++) {
          jsamp++;
          iqhcLong->re = iqChan0[jsamp][ipos];
          iqhcLong->im = iqChan0[jsamp][ipos+1];
          iqvcLong->re = iqChan1[jsamp][ipos];
          iqvcLong->im = iqChan1[jsamp][ipos+1];
          jsamp++;
        }
        
        // windowed data
        
        memcpy(gate->iqhcPrtLong, gate->iqhcPrtLongOrig, nBytesComplex);
        RadarMoments::applyWindow(gate->iqhcPrtLong, _windowHalf, _nSamplesHalf);
        memcpy(gate->iqvcPrtLong, gate->iqvcPrtLongOrig, nBytesComplex);
        RadarMoments::applyWindow(gate->iqvcPrtLong, _windowHalf, _nSamplesHalf);

      } // igate < _ngatesPrtLong

    } break;
    
    case IWRF_SIM_HV_SWITCHED_HV: {
      
      int nBytesComplex = _nSamplesHalf * sizeof(RadarComplex_t);

      // short prt data
      
      for (int igate = 0, ipos = 0; igate < _nGatesPrtShort; igate++, ipos += 2) {
        
        GateData *gate = _gateData[igate];
        
        // original data - full series
        
        RadarComplex_t *iqhcOrig = gate->iqhcOrig;
        RadarComplex_t *iqvcOrig = gate->iqvcOrig;
        for (int isamp = 0; isamp < _nSamples; isamp++, iqhcOrig++, iqvcOrig++) {
          if (isamp % 2 == 0) {
            iqhcOrig->re = iqChan0[isamp][ipos];
            iqhcOrig->im = iqChan0[isamp][ipos+1];
            if (iqChan1) {
              iqvcOrig->re = iqChan1[isamp][ipos];
              iqvcOrig->im = iqChan1[isamp][ipos+1];
            }
          } else {
            iqvcOrig->re = iqChan0[isamp][ipos];
            iqvcOrig->im = iqChan0[isamp][ipos+1];
            if (iqChan1) {
              iqhcOrig->re = iqChan1[isamp][ipos];
              iqhcOrig->im = iqChan1[isamp][ipos+1];
            }
          }
        }
        
        // short PRT from in sequence, starting with short
        
        RadarComplex_t *iqhcShort = gate->iqhcPrtShortOrig;
        RadarComplex_t *iqvcShort = gate->iqvcPrtShortOrig;
        for (int isamp = 0, jsamp = 0; isamp < _nSamplesHalf;
             isamp++, iqhcShort++, iqvcShort++) {
          if (isamp % 2 == 0) {
            iqhcShort->re = iqChan0[jsamp][ipos];
            iqhcShort->im = iqChan0[jsamp][ipos+1];
            if (iqChan1) {
              iqvcShort->re = iqChan1[jsamp][ipos];
              iqvcShort->im = iqChan1[jsamp][ipos+1];
            }
          } else {
            iqvcShort->re = iqChan0[jsamp][ipos];
            iqvcShort->im = iqChan0[jsamp][ipos+1];
            if (iqChan1) {
              iqhcShort->re = iqChan1[jsamp][ipos];
              iqhcShort->im = iqChan1[jsamp][ipos+1];
            }
          }
          jsamp++;
          jsamp++;
        }
        
        // windowed data

        memcpy(gate->iqhc, gate->iqhcOrig, _nSamples * sizeof(RadarComplex_t));
        memcpy(gate->iqvc, gate->iqvcOrig, _nSamples * sizeof(RadarComplex_t));
        RadarMoments::applyWindow(gate->iqhc, _window, _nSamples);
        RadarMoments::applyWindow(gate->iqvc, _window, _nSamples);

        memcpy(gate->iqhcPrtShort, gate->iqhcPrtShortOrig, nBytesComplex);
        RadarMoments::applyWindow(gate->iqhcPrtShort, _windowHalf, _nSamplesHalf);
        memcpy(gate->iqvcPrtShort, gate->iqvcPrtShortOrig, nBytesComplex);
        RadarMoments::applyWindow(gate->iqvcPrtShort, _windowHalf, _nSamplesHalf);
        
      } // igate < _ngatesPrtShort

      // long prt data

      for (int igate = 0, ipos = 0; igate < _nGatesPrtLong; igate++, ipos += 2) {

        GateData *gate = _gateData[igate];

        // long PRT from sequence - starts with short

        RadarComplex_t *iqhcLong = gate->iqhcPrtLongOrig;
        RadarComplex_t *iqvcLong = gate->iqvcPrtLongOrig;
        for (int isamp = 0, jsamp = 0; isamp < _nSamplesHalf;
             isamp++, iqhcLong++, iqvcLong++) {
          jsamp++;
          if (isamp % 2 == 0) {
            iqhcLong->re = iqChan0[jsamp][ipos];
            iqhcLong->im = iqChan0[jsamp][ipos+1];
            if (iqChan1) {
              iqvcLong->re = iqChan1[jsamp][ipos];
              iqvcLong->im = iqChan1[jsamp][ipos+1];
            }
          } else {
            iqvcLong->re = iqChan0[jsamp][ipos];
            iqvcLong->im = iqChan0[jsamp][ipos+1];
            if (iqChan1) {
              iqhcLong->re = iqChan1[jsamp][ipos];
              iqhcLong->im = iqChan1[jsamp][ipos+1];
            }
          }
          jsamp++;
        }
        
        // windowed data
        
        memcpy(gate->iqhcPrtLong, gate->iqhcPrtLongOrig, nBytesComplex);
        RadarMoments::applyWindow(gate->iqhcPrtLong, _windowHalf, _nSamplesHalf);
        memcpy(gate->iqvcPrtLong, gate->iqvcPrtLongOrig, nBytesComplex);
        RadarMoments::applyWindow(gate->iqvcPrtLong, _windowHalf, _nSamplesHalf);

      } // igate < _ngatesPrtLong

    } break;
    
    case IWRF_H_ONLY_FIXED_HV: {
      
      int nBytesComplex = _nSamplesHalf * sizeof(RadarComplex_t);

      // short prt data
      
      for (int igate = 0, ipos = 0; igate < _nGatesPrtShort; igate++, ipos += 2) {
        
        GateData *gate = _gateData[igate];
        
        // original data - full series
        
        RadarComplex_t *iqhcOrig = gate->iqhcOrig;
        RadarComplex_t *iqvxOrig = gate->iqvxOrig;
        for (int isamp = 0; isamp < _nSamples; isamp++, iqhcOrig++, iqvxOrig++) {
          iqhcOrig->re = iqChan0[isamp][ipos];
          iqhcOrig->im = iqChan0[isamp][ipos+1];
          iqvxOrig->re = iqChan1[isamp][ipos];
          iqvxOrig->im = iqChan1[isamp][ipos+1];
        }
        
        // short PRT from in sequence, starting with short
        
        RadarComplex_t *iqhcShort = gate->iqhcPrtShortOrig;
        RadarComplex_t *iqvxShort = gate->iqvxPrtShortOrig;
        for (int isamp = 0, jsamp = 0; isamp < _nSamplesHalf;
             isamp++, iqhcShort++, iqvxShort++) {
          iqhcShort->re = iqChan0[jsamp][ipos];
          iqhcShort->im = iqChan0[jsamp][ipos+1];
          iqvxShort->re = iqChan1[jsamp][ipos];
          iqvxShort->im = iqChan1[jsamp][ipos+1];
          jsamp++;
          jsamp++;
        }
        
        // windowed data

        memcpy(gate->iqhc, gate->iqhcOrig, _nSamples * sizeof(RadarComplex_t));
        memcpy(gate->iqvx, gate->iqvxOrig, _nSamples * sizeof(RadarComplex_t));
        RadarMoments::applyWindow(gate->iqhc, _window, _nSamples);
        RadarMoments::applyWindow(gate->iqvx, _window, _nSamples);

        memcpy(gate->iqhcPrtShort, gate->iqhcPrtShortOrig, nBytesComplex);
        RadarMoments::applyWindow(gate->iqhcPrtShort, _windowHalf, _nSamplesHalf);
        memcpy(gate->iqvxPrtShort, gate->iqvxPrtShortOrig, nBytesComplex);
        RadarMoments::applyWindow(gate->iqvxPrtShort, _windowHalf, _nSamplesHalf);
        
      } // igate < _ngatesPrtShort

      // long prt data
      
      for (int igate = 0, ipos = 0; igate < _nGatesPrtLong; igate++, ipos += 2) {

        GateData *gate = _gateData[igate];

        // long PRT from sequence - starts with short

        RadarComplex_t *iqhcLong = gate->iqhcPrtLongOrig;
        RadarComplex_t *iqvxLong = gate->iqvxPrtLongOrig;
        for (int isamp = 0, jsamp = 0; isamp < _nSamplesHalf;
             isamp++, iqhcLong++, iqvxLong++) {
          jsamp++;
          iqhcLong->re = iqChan0[jsamp][ipos];
          iqhcLong->im = iqChan0[jsamp][ipos+1];
          iqvxLong->re = iqChan1[jsamp][ipos];
          iqvxLong->im = iqChan1[jsamp][ipos+1];
          jsamp++;
        }
        
        // windowed data
        
        memcpy(gate->iqhcPrtLong, gate->iqhcPrtLongOrig, nBytesComplex);
        RadarMoments::applyWindow(gate->iqhcPrtLong, _windowHalf, _nSamplesHalf);
        memcpy(gate->iqvxPrtLong, gate->iqvxPrtLongOrig, nBytesComplex);
        RadarMoments::applyWindow(gate->iqvxPrtLong, _windowHalf, _nSamplesHalf);

      } // igate < _ngatesPrtLong

    } break;
    
    case IWRF_V_ONLY_FIXED_HV: {
      
      int nBytesComplex = _nSamplesHalf * sizeof(RadarComplex_t);

      // short prt data
      
      for (int igate = 0, ipos = 0; igate < _nGatesPrtShort; igate++, ipos += 2) {
        
        GateData *gate = _gateData[igate];
        
        // original data - full series
        
        RadarComplex_t *iqhxOrig = gate->iqhxOrig;
        RadarComplex_t *iqvcOrig = gate->iqvcOrig;
        for (int isamp = 0; isamp < _nSamples; isamp++, iqhxOrig++, iqvcOrig++) {
          iqhxOrig->re = iqChan0[isamp][ipos];
          iqhxOrig->im = iqChan0[isamp][ipos+1];
          iqvcOrig->re = iqChan1[isamp][ipos];
          iqvcOrig->im = iqChan1[isamp][ipos+1];
        }
        
        // short PRT from in sequence, starting with short
        
        RadarComplex_t *iqhxShort = gate->iqhxPrtShortOrig;
        RadarComplex_t *iqvcShort = gate->iqvcPrtShortOrig;
        for (int isamp = 0, jsamp = 0; isamp < _nSamplesHalf;
             isamp++, iqhxShort++, iqvcShort++) {
          iqhxShort->re = iqChan0[jsamp][ipos];
          iqhxShort->im = iqChan0[jsamp][ipos+1];
          iqvcShort->re = iqChan1[jsamp][ipos];
          iqvcShort->im = iqChan1[jsamp][ipos+1];
          jsamp++;
          jsamp++;
        }
        
        // windowed data

        memcpy(gate->iqhx, gate->iqhxOrig, _nSamples * sizeof(RadarComplex_t));
        memcpy(gate->iqvc, gate->iqvcOrig, _nSamples * sizeof(RadarComplex_t));
        RadarMoments::applyWindow(gate->iqhx, _window, _nSamples);
        RadarMoments::applyWindow(gate->iqvc, _window, _nSamples);

        memcpy(gate->iqhxPrtShort, gate->iqhxPrtShortOrig, nBytesComplex);
        RadarMoments::applyWindow(gate->iqhxPrtShort, _windowHalf, _nSamplesHalf);
        memcpy(gate->iqvcPrtShort, gate->iqvcPrtShortOrig, nBytesComplex);
        RadarMoments::applyWindow(gate->iqvcPrtShort, _windowHalf, _nSamplesHalf);
        
      } // igate < _ngatesPrtShort

      // long prt data
      
      for (int igate = 0, ipos = 0; igate < _nGatesPrtLong; igate++, ipos += 2) {

        GateData *gate = _gateData[igate];

        // long PRT from sequence - starts with short

        RadarComplex_t *iqhxLong = gate->iqhxPrtLongOrig;
        RadarComplex_t *iqvcLong = gate->iqvcPrtLongOrig;
        for (int isamp = 0, jsamp = 0; isamp < _nSamplesHalf;
             isamp++, iqhxLong++, iqvcLong++) {
          jsamp++;
          iqhxLong->re = iqChan0[jsamp][ipos];
          iqhxLong->im = iqChan0[jsamp][ipos+1];
          iqvcLong->re = iqChan1[jsamp][ipos];
          iqvcLong->im = iqChan1[jsamp][ipos+1];
          jsamp++;
        }
        
        // windowed data
        
        memcpy(gate->iqhxPrtLong, gate->iqhxPrtLongOrig, nBytesComplex);
        RadarMoments::applyWindow(gate->iqhxPrtLong, _windowHalf, _nSamplesHalf);
        memcpy(gate->iqvcPrtLong, gate->iqvcPrtLongOrig, nBytesComplex);
        RadarMoments::applyWindow(gate->iqvcPrtLong, _windowHalf, _nSamplesHalf);

      } // igate < _ngatesPrtLong

    } break;
    
    case IWRF_SINGLE_POL:
    default: {

      // full series - uses short number of gates
      
      for (int igate = 0, ipos = 0; igate < _nGatesPrtShort; igate++, ipos += 2) {
        GateData *gate = _gateData[igate];
        RadarComplex_t *iqhcOrig = gate->iqhcOrig;
        for (int isamp = 0; isamp < _nSamples; isamp++, iqhcOrig++) {
          iqhcOrig->re = iqChan0[isamp][ipos];
          iqhcOrig->im = iqChan0[isamp][ipos+1];
        }
        RadarMoments::applyWindow(gate->iqhcOrig, _window,
                                  gate->iqhc, _nSamples);
      }  // igate

      // short prt data - uses short number of gates

      for (int igate = 0, ipos = 0; igate < _nGatesPrtShort; igate++, ipos += 2) {
        GateData *gate = _gateData[igate];
        RadarComplex_t *iqhcShort = gate->iqhcPrtShortOrig;
        for (int isamp = 0, jsamp = 0; isamp < _nSamplesHalf; isamp++, iqhcShort++) {
          iqhcShort->re = iqChan0[jsamp][ipos];
          iqhcShort->im = iqChan0[jsamp][ipos+1];
          jsamp++;
          jsamp++;
        }
        RadarMoments::applyWindow(gate->iqhcPrtShortOrig, _windowHalf,
                                  gate->iqhcPrtShort, _nSamplesHalf);
        
      } // igate

      // long PRT from sequence - uses long number of gates
      
      for (int igate = 0, ipos = 0; igate < _nGatesPrtLong; igate++, ipos += 2) {
        GateData *gate = _gateData[igate];
        RadarComplex_t *iqhcLong = gate->iqhcPrtLongOrig;
        for (int isamp = 0, jsamp = 0; isamp < _nSamplesHalf; isamp++, iqhcLong++) {
          jsamp++;
          iqhcLong->re = iqChan0[jsamp][ipos];
          iqhcLong->im = iqChan0[jsamp][ipos+1];
          jsamp++;
        }
        RadarMoments::applyWindow(gate->iqhcPrtLongOrig, _windowHalf,
                                  gate->iqhcPrtLong, _nSamplesHalf);
      } // igate

    } break; // SP
    
  } // switch;

}

///////////////////////////////////
// initialize staggered PRT mode

void Beam::_initStagPrt(int nGatesPrtShort,
                             int nGatesPrtLong,
                             double prtShort,
                             double prtLong)

{
  
  _prt = prtShort;
  _prtShort = prtShort;
  _prtLong = prtLong;
  _nGatesPrtShort = nGatesPrtShort;
  _nGatesPrtLong = nGatesPrtLong;
  _nGatesStagPrt = MAX(_nGatesPrtShort, _nGatesPrtLong);
  
  double wavelengthMeters = _opsInfo.get_radar_wavelength_cm() / 100.0;
  
  double prtRatio = _prtShort / _prtLong;
  int ratio60 = (int) (prtRatio * 60.0 + 0.5);
  if (ratio60 == 40) {
    // 2/3
    _stagM = 2;
    _stagN = 3;
  } else if (ratio60 == 45) {
    // 3/4
    _stagM = 3;
    _stagN = 4;
  } else if (ratio60 == 48) {
    // 4/5
    _stagM = 4;
    _stagN = 5;
  } else {
    // assume 2/3
    cerr << "WARNING - Iq2Dsr::Beam::_initStagPrt" << endl;
    cerr << "  No support for prtRatio: " << prtRatio << endl;
    cerr << "  Assuming 2/3 stagger" << endl;
    _stagM = 2;
    _stagN = 3;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "===>> staggered PRT, ratio: "
         << _stagM << "/" << _stagN << " <<===" << endl;
  }
  
  _nyquistPrtShort = ((wavelengthMeters / _prtShort) / 4.0);
  _nyquistPrtLong = ((wavelengthMeters / _prtLong) / 4.0);
  _nyquist = _nyquistPrtShort * _stagM;

}


/////////////////////////////////////////////////
// copy gate data to output fields

void Beam::_copyDataToOutputFields()
  
{

  int maxGates = _nGates;
  if (maxGates > (int) _gateData.size()) {
    maxGates = (int) _gateData.size();
  }
  
  for (int ii = 0; ii < maxGates; ii++) {
    _fields[ii] = _gateData[ii]->fields;
    _fieldsF[ii] = _gateData[ii]->fieldsF;
    if (_applySz1) {
      _fields[ii + _nGates] = _gateData[ii]->secondTrip;
      _fieldsF[ii + _nGates] = _gateData[ii]->secondTripF;
    }
  }
  
}

/////////////////////////////////////////////////
// check that the baseDbz1km values are set

int Beam::_checkCalib()
  
{
  
  int iret = 0;
  
  if (_params.correct_receiver_gains_for_temperature) {
    if (_correctCalibGainsForTemp()) {
      iret = -1;
    }
  }

  // compute base dbz if needed
  
  if (_calib.isMissing(_calib.getBaseDbz1kmHc())) {
    double noise = _calib.getNoiseDbmHc();
    double gain = _calib.getReceiverGainDbHc();
    double constant = _calib.getRadarConstH();
    if (!_calib.isMissing(noise) && !_calib.isMissing(gain) &&
        !_calib.isMissing(constant)) {
      _calib.setBaseDbz1kmHc(noise - gain - constant);
    }
  }
  
  if (_calib.isMissing(_calib.getBaseDbz1kmVc())) {
    double noise = _calib.getNoiseDbmVc();
    double gain = _calib.getReceiverGainDbVc();
    double constant = _calib.getRadarConstV();
    if (!_calib.isMissing(noise) && !_calib.isMissing(gain) &&
        !_calib.isMissing(constant)) {
      _calib.setBaseDbz1kmVc(noise - gain - constant);
    }
  }
  
  if (_calib.isMissing(_calib.getBaseDbz1kmHc())) {
    double noise = _calib.getNoiseDbmHx();
    double gain = _calib.getReceiverGainDbHx();
    double constant = _calib.getRadarConstH();
    if (!_calib.isMissing(noise) && !_calib.isMissing(gain) &&
        !_calib.isMissing(constant)) {
      _calib.setBaseDbz1kmHx(noise - gain - constant);
    }
  }
  
  if (_calib.isMissing(_calib.getBaseDbz1kmVx())) {
    double noise = _calib.getNoiseDbmVx();
    double gain = _calib.getReceiverGainDbVx();
    double constant = _calib.getRadarConstV();
    if (!_calib.isMissing(noise) && !_calib.isMissing(gain) &&
        !_calib.isMissing(constant)) {
      _calib.setBaseDbz1kmVx(noise - gain - constant);
    }
  }

  return iret;
  
}

/////////////////////////////////////////////////
// Correct calibration gains for temperature

int Beam::_correctCalibGainsForTemp()
  
{

  // get tag list
    
  vector<string> tags;
  TaStr::tokenize(_params.temperature_tags_in_status_xml, "<>", tags);
  if (tags.size() == 0) {
    // no tags
    cerr << "WARNING - no tags found in params.temperature_tags_in_status_xml: "
         << _params.temperature_tags_in_status_xml << endl;
    return -1;
  }
  
  // read through the outer tags in status XML
  
  string buf(_statusXml);
  for (size_t jj = 0; jj < tags.size(); jj++) {
    string val;
    if (TaXml::readString(buf, tags[jj], val)) {
      cerr << "WARNING - bad tags found in status xml, expecting: "
           << _params.temperature_tags_in_status_xml << endl;
      return -1;
    }
    buf = val;
  }

  // read temperature
  
  double temp = -9999.0;
  if (TaXml::readDouble(buf, temp)) {
    cerr << "WARNING - bad temp found in status xml, buf: " << buf << endl;
    return -1;
  }
  
  double gainCorrectionHc = 0.0;
  double gainCorrectionVc = 0.0;
  double gainCorrectionHx = 0.0;
  double gainCorrectionVx = 0.0;

  for (int ii = 0; ii < _params.rx_gain_correction_n; ii++) {
    const Params::rx_gain_correction_t &corr = 
      _params._rx_gain_correction[ii];
    switch (corr.rx_channel) {
      case Params::RX_CHANNEL_HC:
        gainCorrectionHc = corr.intercept + temp * corr.slope;
        break;
      case Params::RX_CHANNEL_VC:
        gainCorrectionVc = corr.intercept + temp * corr.slope;
        break;
      case Params::RX_CHANNEL_HX:
        gainCorrectionHx = corr.intercept + temp * corr.slope;
        break;
      case Params::RX_CHANNEL_VX:
        gainCorrectionVx = corr.intercept + temp * corr.slope;
        break;
    } // switch
  } // ii

  if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
    cerr << "======= CALIBRATION BEFORE TEMP CORRECTION =============" << endl;
    _calib.print(cerr);
    cerr << "======= END CALIBRATION BEFORE TEMP CORRECTION =========" << endl;
  }
    
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "correctCalibGainsForTemp(): got temp from status XML" << endl;
    cerr << "  temp: " << temp << endl;
    cerr << "  gainCorrectionHc: " << gainCorrectionHc << endl;
    cerr << "  gainCorrectionVc: " << gainCorrectionVc << endl;
    cerr << "  gainCorrectionHx: " << gainCorrectionHx << endl;
    cerr << "  gainCorrectionVx: " << gainCorrectionVx << endl;
  }

  // augment status xml in ops info

  string tempCorrXml;
  tempCorrXml += TaXml::writeStartTag("TempBasedRxGainCorrection", 0);
  tempCorrXml += TaXml::writeDouble("temp", 1, temp);
  tempCorrXml += TaXml::writeDouble("gainCorrectionHc", 1, gainCorrectionHc);
  tempCorrXml += TaXml::writeDouble("gainCorrectionVc", 1, gainCorrectionVc);
  tempCorrXml += TaXml::writeDouble("gainCorrectionHx", 1, gainCorrectionHx);
  tempCorrXml += TaXml::writeDouble("gainCorrectionVx", 1, gainCorrectionVx);
  tempCorrXml += TaXml::writeEndTag("TempBasedRxGainCorrection", 0);
  _statusXml += tempCorrXml;
  
  // compute base dbz if needed
  
  if (!_calib.isMissing(_calib.getReceiverGainDbHc())) {
    double rconst = _calib.getRadarConstH();
    double noise = _calib.getNoiseDbmHc();
    double noiseFixed = noise + gainCorrectionHc;
    double gain = _calib.getReceiverGainDbHc();
    double gainFixed = gain + gainCorrectionHc;
    double baseDbz1km = noiseFixed - gainFixed + rconst;
    if (!_calib.isMissing(noise) && !_calib.isMissing(rconst)) {
      _calib.setBaseDbz1kmHc(baseDbz1km);
    }
    _calib.setReceiverGainDbHc(gainFixed);
    _calib.setNoiseDbmHc(noiseFixed);
  }
  
  if (!_calib.isMissing(_calib.getReceiverGainDbVc())) {
    double rconst = _calib.getRadarConstV();
    double noise = _calib.getNoiseDbmVc();
    double noiseFixed = noise + gainCorrectionVc;
    double gain = _calib.getReceiverGainDbVc();
    double gainFixed = gain + gainCorrectionVc;
    double baseDbz1km = noiseFixed - gainFixed + rconst;
    if (!_calib.isMissing(noise) && !_calib.isMissing(rconst)) {
      _calib.setBaseDbz1kmVc(baseDbz1km);
    }
    _calib.setReceiverGainDbVc(gainFixed);
    _calib.setNoiseDbmVc(noiseFixed);
  }
  
  if (!_calib.isMissing(_calib.getReceiverGainDbHx())) {
    double rconst = _calib.getRadarConstH();
    double noise = _calib.getNoiseDbmHx();
    double noiseFixed = noise + gainCorrectionHx;
    double gain = _calib.getReceiverGainDbHx();
    double gainFixed = gain + gainCorrectionHx;
    double baseDbz1km = noiseFixed - gainFixed + rconst;
    if (!_calib.isMissing(noise) && !_calib.isMissing(rconst)) {
      _calib.setBaseDbz1kmHx(baseDbz1km);
    }
    _calib.setReceiverGainDbHx(gainFixed);
    _calib.setNoiseDbmHx(noiseFixed);
  }
  
  if (!_calib.isMissing(_calib.getReceiverGainDbVx())) {
    double rconst = _calib.getRadarConstV();
    double noise = _calib.getNoiseDbmVx();
    double noiseFixed = noise + gainCorrectionVx;
    double gain = _calib.getReceiverGainDbVx();
    double gainFixed = gain + gainCorrectionVx;
    double baseDbz1km = noiseFixed - gainFixed + rconst;
    if (!_calib.isMissing(noise) && !_calib.isMissing(rconst)) {
      _calib.setBaseDbz1kmVx(baseDbz1km);
    }
    _calib.setReceiverGainDbVx(gainFixed);
    _calib.setNoiseDbmVx(noiseFixed);
  }
  
  if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
    cerr << "####### CALIBRATION AFTER TEMP CORRECTION #############" << endl;
    _calib.print(cerr);
    cerr << "####### END CALIBRATION AFTER TEMP CORRECTION #########" << endl;
  }
    
  return 0;
  
}

/////////////////////////////////////////////////////////////////
// Apply time-domain filter to IQ data

void Beam::_applyTimeDomainFilter(const RadarComplex_t *iq,
					RadarComplex_t *filtered) const

{

  // filter coefficients

  double aa = 0.654424;
  double bb = 0.474320;
  double cc = 1.614429;
  double dd = 0.742456;
  double ee = 1.989848;
  double ff = 1.874940;
  double gg = 0.951312;
  double hh = 1.976096;
  double pp = 1.0;
  double qq = 1.0;

  // initialize

  double kk = aa / (1.0 - bb);
  RadarComplex_t uu1, vv1, vv2, ww1, ww2;

  uu1.re = kk * iq[0].re;
  uu1.im = kk * iq[0].im;

  vv1.re = 0.0;
  vv1.im = 0.0;
  vv2.re = 0.0;
  vv2.im = 0.0;
  
  ww1.re = 0.0;
  ww1.im = 0.0;
  ww2.re = 0.0;
  ww2.im = 0.0;
  
  RadarComplex_t uu, vv, ww;

  for (int ii = -2; ii < _nSamples; ii++) {

    int jj = ii;
    if (jj < 0) {
      jj = 0;
    }

    uu.re = aa * iq[jj].re + bb * uu1.re;
    uu.im = aa * iq[jj].im + bb * uu1.im;

    vv.re = uu.re - uu1.re + cc * vv1.re - dd * vv2.re;
    vv.im = uu.im - uu1.im + cc * vv1.im - dd * vv2.im;

    ww.re = vv.re - ee * vv1.re + pp * vv2.re + ff * ww1.re - gg * ww2.re;
    ww.im = vv.im - ee * vv1.im + pp * vv2.im + ff * ww1.im - gg * ww2.im;

    filtered[jj].re = ww.re - hh * ww1.re + qq * ww2.re;
    filtered[jj].im = ww.im - hh * ww1.im + qq * ww2.im;

  } // ii

}

////////////////////////////////////////
// apply a median filter to CPA

void Beam::_applyMedianFilterToCPA(int nGates)

{
  
  if (_params.cpa_median_filter_len < 3) {
    return;
  }
  
  TaArray<double> cpa_;
  double *cpa = cpa_.alloc(nGates);
  for (int ii = 0; ii < nGates; ii++) {
    cpa[ii] = _gateData[ii]->fields.cpa;
  }
  FilterUtils::applyMedianFilter(cpa, nGates,
				 _params.cpa_median_filter_len);
  for (int ii = 0; ii < nGates; ii++) {
    _gateData[ii]->fields.cpa = cpa[ii];
  }

}
  
////////////////////////////////////////
// apply a median filter to ZDR

void Beam::_applyMedianFilterToZDR(int nGates)

{
  
  if (_params.zdr_median_filter_len < 3) {
    return;
  }
  
  TaArray<double> zdr_;
  double *zdr = zdr_.alloc(nGates);
  for (int ii = 0; ii < nGates; ii++) {
    zdr[ii] = _gateData[ii]->fields.zdr;
  }
  FilterUtils::applyMedianFilter(zdr, nGates,
				 _params.zdr_median_filter_len);
  for (int ii = 0; ii < nGates; ii++) {
    _gateData[ii]->fields.zdr = zdr[ii];
  }

  if (_mmgr.applyClutterFilter()) {
    for (int ii = 0; ii < nGates; ii++) {
      zdr[ii] = _gateData[ii]->fieldsF.zdr;
    }
    FilterUtils::applyMedianFilter(zdr, nGates,
                                   _params.zdr_median_filter_len);
    for (int ii = 0; ii < nGates; ii++) {
      _gateData[ii]->fieldsF.zdr = zdr[ii];
    }
  }

}
  
////////////////////////////////////////
// apply a median filter to RHOHV

void Beam::_applyMedianFilterToRHOHV(int nGates)

{
  
  if (_params.rhohv_median_filter_len < 3) {
    return;
  }
  
  TaArray<double> rhohv_;
  double *rhohv = rhohv_.alloc(nGates);
  for (int ii = 0; ii < nGates; ii++) {
    rhohv[ii] = _gateData[ii]->fields.rhohv;
  }
  FilterUtils::applyMedianFilter(rhohv, nGates,
				 _params.rhohv_median_filter_len);
  for (int ii = 0; ii < nGates; ii++) {
    _gateData[ii]->fields.rhohv = rhohv[ii];
  }

  if (_mmgr.applyClutterFilter()) {
    for (int ii = 0; ii < nGates; ii++) {
      rhohv[ii] = _gateData[ii]->fieldsF.rhohv;
    }
    FilterUtils::applyMedianFilter(rhohv, nGates,
                                   _params.rhohv_median_filter_len);
    for (int ii = 0; ii < nGates; ii++) {
      _gateData[ii]->fieldsF.rhohv = rhohv[ii];
    }
  }

}
  
////////////////////////////////////////
// clean up staggered PRT velocity

void Beam::_cleanUpStagVel()

{

  if (_params.staggered_prt_median_filter_len < 3) {
    return;
  }

  TaArray<double> smoothedVel_;
  double *smoothedVel = smoothedVel_.alloc(_nGatesPrtLong);
  for (int ii = 0; ii < _nGatesPrtLong; ii++) {
    smoothedVel[ii] = _gateData[ii]->fields.vel;
  }
  FilterUtils::applyMedianFilter(smoothedVel, _nGatesPrtLong,
				 _params.staggered_prt_median_filter_len);
  for (int ii = 0; ii < _nGatesPrtLong; ii++) {
    MomentsFields &fld = _gateData[ii]->fields;
    if (fabs(fld.vel - smoothedVel[ii]) > _nyquistPrtShort / 2) {
      fld.vel = smoothedVel[ii];
    }
  }

}

///////////////////////////////////////////////////////////
// Perform clutter filtering

void Beam::_performClutterFiltering()

{

  // compute CMD

  _cmd->setRangeGeometry(_startRangeKm, _gateSpacingKm);
  _cmd->compute(_nGates, _dualPol);
  _fixAltClutVelocity();

  // copy the unfiltered fields to the filtered fields
  
  for (int igate = 0; igate < _nGates; igate++) {
    _gateData[igate]->fieldsF = _gateData[igate]->fields;
  }
  
  // filter clutter from moments
  
  _filterMoments();

}

///////////////////////////////////////////////////////////
// Perform clutter filtering for SZ864

void Beam::_performClutterFilteringSz()

{

  // set field pointers to first trip
  
  for (int ii = 0; ii < _nGates; ii++) {
    _gateData[ii]->setFieldsToNormalTrip();
  }
  
  // compute CMD
  
  _cmd->setRangeGeometry(_startRangeKm, _gateSpacingKm);
  _cmd->compute(_nGates, _dualPol);
  _fixAltClutVelocity();

  // copy the unfiltered fields to the filtered fields
  
  for (int igate = 0; igate < _nGates; igate++) {
    _gateData[igate]->fieldsF = _gateData[igate]->fields;
  }
  
  // set field pointers to second trip
  
  for (int ii = 0; ii < _nGates; ii++) {
    _gateData[ii]->setFieldsToSecondTrip();
  }

  // compute CMD
  
  _cmd->compute(_nGates, _dualPol);
  _fixAltClutVelocity();

  // copy the unfiltered fields to the filtered fields
  
  for (int igate = 0; igate < _nGates; igate++) {
    _gateData[igate]->secondTripF = _gateData[igate]->secondTrip;
  }
  
  // reset field pointers to first trip
  
  for (int ii = 0; ii < _nGates; ii++) {
    _gateData[ii]->setFieldsToNormalTrip();
  }
      
  // perform filtering
  
  _performClutterFiltering();

}

///////////////////////////////////////////////////////////
// Fix velocity for alternating mode in clutter
//
// In alternating dual-pol mode, the velocity computed in clutter
// swings wildly in value from 0 to the nyquist and back.
// To handle this, we use an alternative velocity computation based
// on the H and V time series independently.
//
// So, we use this alternative form of velocity for gates at
// which CMD identifies clutter

void Beam::_fixAltClutVelocity()

{

  if (!_params.use_h_only_for_alt_mode_clutter_vel) {
    return;
  }

  if (_xmitRcvMode != IWRF_ALT_HV_CO_ONLY &&
      _xmitRcvMode != IWRF_ALT_HV_CO_CROSS) {
    return;
  }

  double quarterNyquist = _nyquist / 4.0;

  for (int ii = 0; ii < _nGates; ii++) {

    MomentsFields &fld = _gateData[ii]->flds;
    if (fld.cmd_flag == 1) {
      double vel_h_only = fld.vel_h_only;
      if (vel_h_only != MomentsFields::missingDouble &&
	  fabs(vel_h_only) < quarterNyquist) {
        fld.vel = vel_h_only;
      }
    }
    
  } // ii

}

//////////////////////////////////////////////////////////////
// compute clutter power
// returns missing if either dbz value is missing

double Beam::_computeClutPower(const MomentsFields &unfiltered,
                               const MomentsFields &filtered)
  
{
  
  if (unfiltered.dbz != MomentsFields::missingDouble &&
      filtered.dbz != MomentsFields::missingDouble) {
    double clut = unfiltered.dbz - filtered.dbz;
    if (clut == 0.0) {
      return MomentsFields::missingDouble;
    }
    return clut;
  } else {
    return MomentsFields::missingDouble;
  }

}
	
////////////////////////////////////////
// print selected moments values
//
// A debugging routine to print out selected
// moments values to stdout

void Beam::_printSelectedMoments()

{

  int nSamples = _nSamples;
  if (_xmitRcvMode == IWRF_ALT_HV_CO_ONLY ||
      _xmitRcvMode == IWRF_ALT_HV_CO_CROSS ||
      _xmitRcvMode == IWRF_ALT_HV_FIXED_HV) {
    nSamples /= 2;
  }

  double *vonHann = RadarMoments::createWindowVonhann(nSamples);

  for (int ii = 0; ii < _nGates; ii++) {
    
    RadarComplex_t *iqhc = _gateData[ii]->iqhcOrig;
    double snr = _gateData[ii]->fields.snr;

    TaArray<RadarComplex_t> windowed_;
    RadarComplex_t *windowed = windowed_.alloc(nSamples);
    memcpy(windowed, iqhc, nSamples * sizeof(RadarComplex_t));
    RadarMoments::applyWindow(windowed, vonHann, nSamples);

    double cpa = RadarMoments::computeCpa(iqhc, nSamples);
    double cpaw = RadarMoments::computeCpa(windowed, nSamples);
      
    double pr = RadarMoments::computePowerRatio(iqhc, nSamples);
    double prw = RadarMoments::computePowerRatio(windowed, nSamples);

    //    if (cpaw > 0.99 && prw < 0.58) {

    if (snr > 30) {
      cout << cpa << " "
           << cpaw << " "
           << sqrt(pr) << " "
           << sqrt(prw) << " "
           << snr << endl;
    }

    //       char header[1024];
    //       sprintf(header, "IQ time series, nSamples: %d, CPAW: %g, PRW: %g",
    //               nSamples, cpaw, prw);
    //       RadarComplex::printVector(cout, nSamples, iqhc, true, header, false);
    //     }

  }

#ifdef NOTNOW
  for (int ii = 0; ii < _nGates; ii++) {
    
    double cpa = _gateData[ii]->fields.cpa;
    RadarComplex_t *iqhc = _gateData[ii]->iqhc;

    double pr = RadarMoments::computePowerRatio(iqhc, nSamples);

    if (cpa > 0.98 && pr < 0.6) {

      //       TaArray<RadarComplex_t> spec_;
      //       RadarComplex_t *spec = spec_.alloc(nSamples);
      //       RadarFft fft(nSamples);
      //       fft.fwd(iqhc, spec);
      
      cout << "cpa, pr: " << cpa << ", " << pr << endl;
      char header[1024];
      sprintf(header, "IQ time series, nSamples: %d, CPA: %g, PR: %g",
              nSamples, cpa, pr);
      // RadarComplex::printIq(cout, nSamples, iqhc, true, header);
      // RadarComplex::printVector(cout, nSamples, spec, true, header, false);
      RadarComplex::printVector(cout, nSamples, iqhc, true, header, false);
    }

  }
#endif

  delete[] vonHann;

#ifdef NOTNOW
  for (int ii = 0; ii < _nGates; ii++) {
    double cmd = _gateData[ii]->fields.cmd;
    double cpa = _gateData[ii]->fields.cpa;
    double pr = _gateData[ii]->fields.pratio;
    double vel = _gateData[ii]->fields.vel;
    double snr = _gateData[ii]->fields.snr;
    double clutPower = _gateData[ii]->fields.clut;
    double specSnr = _gateData[ii]->fields.spectral_snr;
    double specNoise = _gateData[ii]->fields.spectral_noise;
    double cwr = _gateData[ii]->fields.clut_2_wx_ratio;
    double widthf = _gateData[ii]->fieldsF.width;
    if (clutPower > -100 && cwr > -1000) {
      cout << cpa << " "
           << pr << " "
           << vel << " "
           << widthf << " "
           << cmd << " "
           << clutPower << " "
           << specSnr << " "
           << specNoise << " "
           << cwr << endl;
    }
  }
#endif

}

/////////////////////////////////////////////////////////////////
// Compute phase differences between this pulse and previous ones
// to be able to cohere to multiple trips
//
// Before this method is called, this pulse should be added to
// the queue.

void Beam::_computePhaseDiffs
  (const vector<const IwrfTsPulse *> &pulseQueue, int maxTrips)
  
{
  
  _phaseDiffs.clear();
  
  // phase diffs for maxTrips previous beams
  
  int qSize = (int) pulseQueue.size();
  
  for (int ii = 0; ii < maxTrips; ii++) { // ii is (trip - 1)
    if (ii == 0) {
      _phaseDiffs.push_back(0.0);
    } else {
      if (ii <= qSize) {
	double sum = _phaseDiffs[ii-1] + pulseQueue[ii-1]->getPhaseDiff0();
	while (sum > 360.0) {
	  sum -= 360.0;
	}
	_phaseDiffs.push_back(sum);
      } else {
	// actual phase diff not available
	// use previous trip's value
	_phaseDiffs.push_back(_phaseDiffs[ii-1]);
      }
    }
  }

}

///////////////////////////////
// set the noise fields

void Beam::_setNoiseFields()
  
{
  
  const vector<bool> &noiseFlag = _noise->getNoiseFlag();
  const vector<bool> &signalFlag = _noise->getSignalFlag();
  const vector<double> &accumPhaseChange = _noise->getAccumPhaseChange();
  const vector<double> &phaseChangeError = _noise->getPhaseChangeError();
  const vector<double> &dbmSdev = _noise->getDbmSdev();
  const vector<double> &ncpMean = _noise->getNcpMean();

  if ((int) noiseFlag.size() < _nGates) {
    return;
  }

  for (int igate = 0; igate < _nGates; igate++) {
    GateData *gate = _gateData[igate];
    MomentsFields &fields = gate->fields;
    fields.noise_flag = noiseFlag[igate];
    fields.signal_flag = signalFlag[igate];
    fields.accum_phase_change = accumPhaseChange[igate];
    fields.phase_change_error = phaseChangeError[igate];
    fields.dbm_sdev = dbmSdev[igate];
    fields.ncp_mean = ncpMean[igate];
  }
  
}

///////////////////////////////
// censor gates based on noise

void Beam::_censorByNoiseFlag()

{

  // load up censor flag from noise flag

  for (int igate = 0; igate < _nGates; igate++) {
    GateData *gate = _gateData[igate];
    MomentsFields &fields = gate->fields;
    if (fields.noise_flag) {
      _censorFlags[igate] = true;
    } else {
      _censorFlags[igate] = false;
    }
  }

  // fill in gaps in censor flag

  _fillInCensoring();

  // censor the fields
  
  for (int igate = 0; igate < _nGates; igate++) {
    if (_censorFlags[igate]) {
      GateData *gate = _gateData[igate];
      _censorFields(gate->fields);
      _censorFields(gate->fieldsF);
    }
  }

}

////////////////////////////////////////////////////////////////////
// censor based on snr and ncp thresholds

void Beam::_censorBySnrNcp()

{

  // load up censor flag from snr and ncp

  double snrThreshold = _params.censoring_snr_threshold;
  double ncpThreshold = _params.censoring_ncp_threshold;

  for (int igate = 0; igate < _nGates; igate++) {
    GateData *gate = _gateData[igate];
    MomentsFields &fields = gate->fields;
    if (fields.snr < snrThreshold &&
        fields.ncp < ncpThreshold) {
      _censorFlags[igate] = true;
    } else {
      _censorFlags[igate] = false;
    }
  }

  // fill in gaps in censor flag

  _fillInCensoring();

  // censor the fields
  
  for (int igate = 0; igate < _nGates; igate++) {
    if (_censorFlags[igate]) {
      GateData *gate = _gateData[igate];
      _censorFields(gate->fields);
      _censorFields(gate->fieldsF);
    }
  }

}

///////////////////////////////////////////////////////////
// censor specified fields in a ray

void Beam::_censorFields(MomentsFields &mfield)

{

  mfield.ncp = MomentsFields::missingDouble;
  mfield.snr = MomentsFields::missingDouble;
  mfield.dbm = MomentsFields::missingDouble;
  mfield.dbz = MomentsFields::missingDouble;
  mfield.dbz_no_atmos_atten = MomentsFields::missingDouble;
  
  mfield.vel = MomentsFields::missingDouble;
  mfield.vel_alt = MomentsFields::missingDouble;
  mfield.vel_hv = MomentsFields::missingDouble;
  mfield.vel_h_only = MomentsFields::missingDouble;
  mfield.vel_v_only = MomentsFields::missingDouble;
  mfield.width = MomentsFields::missingDouble;
  mfield.width_h_only = MomentsFields::missingDouble;
  mfield.width_v_only = MomentsFields::missingDouble;
  
  mfield.zdrm = MomentsFields::missingDouble;
  mfield.zdr = MomentsFields::missingDouble;
  mfield.ldrhm = MomentsFields::missingDouble;
  mfield.ldrh = MomentsFields::missingDouble;
  mfield.ldrvm = MomentsFields::missingDouble;
  mfield.ldrv = MomentsFields::missingDouble;
  
  mfield.rhohv = MomentsFields::missingDouble;
  mfield.rhohv_nnc = MomentsFields::missingDouble;
  mfield.phidp = MomentsFields::missingDouble;
  mfield.kdp = MomentsFields::missingDouble;
  mfield.psob = MomentsFields::missingDouble;
  
  mfield.snrhc = MomentsFields::missingDouble;
  mfield.snrhx = MomentsFields::missingDouble;
  mfield.snrvc = MomentsFields::missingDouble;
  mfield.snrvx = MomentsFields::missingDouble;
  
  mfield.dbmhc = MomentsFields::missingDouble;
  mfield.dbmhx = MomentsFields::missingDouble;
  mfield.dbmvc = MomentsFields::missingDouble;
  mfield.dbmvx = MomentsFields::missingDouble;
  
  mfield.dbmhc_ns = MomentsFields::missingDouble;
  mfield.dbmhx_ns = MomentsFields::missingDouble;
  mfield.dbmvc_ns = MomentsFields::missingDouble;
  mfield.dbmvx_ns = MomentsFields::missingDouble;
  
  mfield.dbzhc = MomentsFields::missingDouble;
  mfield.dbzhx = MomentsFields::missingDouble;
  mfield.dbzvc = MomentsFields::missingDouble;
  mfield.dbzvx = MomentsFields::missingDouble;

}

/////////////////////////////////////////////////////////////
// apply in-fill filter to censor flag

void Beam::_fillInCensoring()
  
{
  
  int *countSet = new int[_nGates];
  int *countNot = new int[_nGates];

  // compute the running count of gates which have the flag set and
  // those which do not

  // Go forward through the gates, counting up the number of gates set
  // or not set and assigning that number to the arrays as we go.

  int nSet = 0;
  int nNot = 0;
  for (int igate = 0; igate < _nGates; igate++) {
    if (_censorFlags[igate]) {
      nSet++;
      nNot = 0;
    } else {
      nSet = 0;
      nNot++;
    }
    countSet[igate] = nSet;
    countNot[igate] = nNot;
  }

  // Go in reverse through the gates, taking the max non-zero
  // values and copying them across the set or not-set regions.
  // This makes all the counts equal in the gaps and set areas.

  for (int igate = _nGates - 2; igate >= 0; igate--) {
    if (countSet[igate] != 0 &&
        countSet[igate] < countSet[igate+1]) {
      countSet[igate] = countSet[igate+1];
    }
    if (countNot[igate] != 0 &&
        countNot[igate] < countNot[igate+1]) {
      countNot[igate] = countNot[igate+1];
    }
  }

  // fill in gaps

  for (int igate = 1; igate < _nGates - 1; igate++) {

    // is the gap small enough?

    nNot = countNot[igate];
    if (nNot > 0 && nNot <= 3) {

      // is it surrounded by regions at least as large as the gap?

      int minGateCheck = igate - nNot;
      if (minGateCheck < 0) {
        minGateCheck = 0;
      }
      int maxGateCheck = igate + nNot;
      if (maxGateCheck > _nGates - 1) {
        maxGateCheck = _nGates - 1;
      }

      int nAdjacentBelow = 0;
      for (int jgate = igate - 1; jgate >= minGateCheck; jgate--) {
        nSet = countSet[jgate];
        if (nSet != 0) {
          nAdjacentBelow = nSet;
          break;
        }
      } // jgate

      int nAdjacentAbove = 0;
      for (int jgate = igate + 1; jgate <= maxGateCheck; jgate++) {
        nSet = countSet[jgate];
        if (nSet != 0) {
          nAdjacentAbove = nSet;
          break;
        }
      } // jgate

      int minAdjacent = nAdjacentBelow;
      minAdjacent = MIN(minAdjacent, nAdjacentAbove);
      
      if (minAdjacent >= nNot) {
        _censorFlags[igate] = true;
      }
    }
  } // igate

  delete[] countSet;
  delete[] countNot;

}




///////////////////////////////////////////////////////////
// Cohere trip1 time series to trip 2
  
void Beam::_cohereTrip1ToTrip2
  (RadarComplex_t *iq,
   const vector<IwrfTsPulse::burst_phase_t> burstPhases)
  
{
  



}

//////////////////////////////////////////////////////////////////
/// Get azimuth corrected for possible offset

double Beam::_getCorrectedAz(double az)
  
{
  
  if (_params.apply_azimuth_offset) {
    az += _params.azimuth_offset;
  }

  while (az > 360.0) {
    az -= 360.0;
  }

  while (az < 0.0) {
    az += 360.0;
  }

  return az;

}

//////////////////////////////////////////////////////////////////
/// Apply an elevation offset to all rays in the volume
/// This applies to the rays currently in the volume, not to
/// any future reads

double Beam::_getCorrectedEl(double el)

{

  if (_params.apply_elevation_offset) {
    el += _params.elevation_offset;
  }

  while (el > 180.0) {
    el -= 360.0;
  }

  while (el < -180.0) {
    el += 360.0;
  }

  return el;

}
