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
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2019
//
///////////////////////////////////////////////////////////////

#include <cassert>
#include <iostream>
#include <algorithm>
#include <toolsa/TaArray.hh>
#include <toolsa/toolsa_macros.h>
#include <toolsa/DateTime.hh>
#include <radar/FilterUtils.hh>
#include "Beam.hh"
using namespace std;

const double Beam::_missingDbl = MomentsFields::missingDouble;

////////////////////////////////////////////////////
// Constructor

Beam::Beam(const string &progName,
           const Params &params) :
        _progName(progName),
        _params(params)
        
{
  _init();
}

////////////////////////////////////////
// copy constructor

Beam::Beam(const Beam &rhs) :
        _params(rhs._params)
        
{
  _init();
  _copy(rhs);
}

/////////////////////////////
// Assignment
//

Beam &Beam::operator=(const Beam &rhs)
  
{
  return _copy(rhs);
}

////////////////////////////////////////
// copy method

Beam &Beam::_copy(const Beam &rhs)

{
    
  if (&rhs == this) {
    return *this;
  }

  // copy the meta data

  _progName = rhs._progName;

  _pulses = rhs._pulses;
  _isRhi = rhs._isRhi;

  _nSamples = rhs._nSamples;
  _nSamplesHalf = rhs._nSamplesHalf;
  _nSamplesAlloc = rhs._nSamplesAlloc;

  _nGates = rhs._nGates;
  _nGatesOut = rhs._nGatesOut;
  _nGatesOutAlloc = rhs._nGatesOutAlloc;

  _timeSecs = rhs._timeSecs;
  _nanoSecs = rhs._nanoSecs;
  _time = rhs._time;

  _el = rhs._el;
  _az = rhs._az;
  _targetAngle = rhs._targetAngle;

  _scanMode = rhs._scanMode;
  _xmitRcvMode = rhs._xmitRcvMode;
  _sweepNum = rhs._sweepNum;
  _volNum = rhs._volNum;

  _startRangeKm = rhs._startRangeKm;
  _gateSpacingKm = rhs._gateSpacingKm;

  _georef = rhs._georef;
  _georefActive = rhs._georefActive;

  _beamIsIndexed = rhs._beamIsIndexed;
  _angularResolution = rhs._angularResolution;
  _isAlternating = rhs._isAlternating;
  _dualPol = rhs._dualPol;
  _switchingReceiver = rhs._switchingReceiver;

  _prt = rhs._prt;
  _nyquist = rhs._nyquist;
  _pulseWidth = rhs._pulseWidth;
  _unambigRange = rhs._unambigRange;

  _isStagPrt = rhs._isStagPrt;
  _prtShort = rhs._prtShort;
  _prtLong = rhs._prtLong;
  _nGatesPrtShort = rhs._nGatesPrtShort;
  _nGatesPrtLong = rhs._nGatesPrtLong;
  _nGatesStagPrt = rhs._nGatesStagPrt;
  _nyquistPrtShort = rhs._nyquistPrtShort;
  _nyquistPrtLong = rhs._nyquistPrtLong;
  _stagM = rhs._stagM;
  _stagN = rhs._stagN;

  _measXmitPowerDbmH = rhs._measXmitPowerDbmH;
  _measXmitPowerDbmV = rhs._measXmitPowerDbmV;
  
  _opsInfo = rhs._opsInfo;
  _wavelengthM = rhs._wavelengthM;

  // calibration

  _calib = rhs._calib;

  // Moments computations

  if (_mom) {
    delete _mom;
  }
  _mom = new RadarMoments(_nGatesOut,
                          _params.debug >= Params::DEBUG_NORM,
                          _params.debug >= Params::DEBUG_VERBOSE);
  
  // KDP
  
  _kdpInit();

  _snrArray_ = rhs._snrArray_;
  _dbzArray_ = rhs._dbzArray_;
  _zdrArray_ = rhs._zdrArray_;
  _rhohvArray_ = rhs._rhohvArray_;
  _phidpArray_ = rhs._phidpArray_;
  
  _snrArray = _snrArray_.buf();
  _dbzArray = _dbzArray_.buf();
  _zdrArray = _zdrArray_.buf();
  _rhohvArray = _rhohvArray_.buf();
  _phidpArray = _phidpArray_.buf();

  // fields for moments

  _outFields_ = rhs._outFields_;
  _outFields = _outFields_.buf();

  _outFieldsF_ = rhs._outFieldsF_;
  _outFieldsF = _outFieldsF_.buf();

  _compFields_ = rhs._compFields_;
  _compFields = _compFields_.buf();

  _compFieldsF_ = rhs._compFieldsF_;
  _compFieldsF = _compFieldsF_.buf();

  // gate data

  _haveChan1 = rhs._haveChan1;
  _gateData = rhs._gateData;
  for (size_t ii = 0; ii < _gateData.size(); ii++) {
    _gateData[ii] = new GateData(*rhs._gateData[ii]);
  }

  // channel data

  _iqChan0_ = rhs._iqChan0_;
  _iqChan0 = _iqChan0_.buf();

  _iqChan1_ = rhs._iqChan1_;
  _iqChan1 = _iqChan1_.buf();

  // FFTs

  _fftInit();

  // regression filter

  _regrInit();

  // FFT windows

  _freeWindows();
  _fftWindowType = rhs._fftWindowType;
  _computeWindows();
  
  return *this;

}

////////////////////////////////////////
// initialize

void Beam::_init()
        
{

  _nSamples = 0;
  _nSamplesHalf = 0;
  _nSamplesAlloc = 0;

  _nGates = 0;
  _nGatesOut = 0;
  _nGatesOutAlloc = 0;

  _timeSecs = 0;
  _nanoSecs = 0;
  _time.set(DateTime::NEVER);

  _az = 0;
  _el = 0;
  _targetAngle = 0;

  _scanMode = IWRF_SCAN_MODE_NOT_SET;
  _xmitRcvMode = IWRF_SINGLE_POL;
  _sweepNum = 0;
  _volNum = 0;

  _startRangeKm = 0.0;
  _gateSpacingKm = 0.0;

  
  _beamIsIndexed = true;
  _angularResolution = 1.0;
  _isAlternating = false;
  _dualPol = false;

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
  _stagM = 1;
  _stagN = 1;

  _measXmitPowerDbmH = 0;
  _measXmitPowerDbmV = 0;

  _outFields = NULL;
  _outFieldsF = NULL;

  _mom = NULL;

  _fftWindowType = Params::FFT_WINDOW_RECT;

  _window = NULL;
  _windowHalf = NULL;

  _windowR1 = 0;
  _windowR2 = 0;
  _windowR3 = 0;
  _windowHalfR1 = 0;
  _windowHalfR2 = 0;
  _windowHalfR3 = 0;

  _fft = new RadarFft();
  _fftHalf = new RadarFft();
  _fftStag = new RadarFft();
  
  _regr = new ForsytheRegrFilter();
  _regrHalf = new ForsytheRegrFilter();
  _regrStag = new ForsytheRegrFilter();

  _haveChan1 = false;
  _iqChan0 = NULL;
  _iqChan1 = NULL;

}

////////////////////////////////////////////////////
// Initialize the pulse sequence

void Beam::setPulses(bool isRhi,
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
                     const deque<const IwrfTsPulse *> &pulses)
  
{

  _isRhi = isRhi;
  _nSamples = nSamples;
  _nSamplesHalf = _nSamples / 2;
  _nGates = nGates;
  _nGatesPrtShort = nGates;
  _nGatesPrtLong = nGatesPrtLong;
  _beamIsIndexed = beamIsIndexed;
  _angularResolution = angularResolution;
  _isAlternating = isAlternating;
  _isStagPrt = isStagPrt;
  _prt = prt;
  _prtShort = prt;
  _prtLong = prtLong;
  _opsInfo = opsInfo;
  _pulses = pulses;
  _georefActive = false;

  if (_isStagPrt) {
    _nGatesOut = _nGatesPrtShort;
  } else {
    _nGatesOut = _nGates;
  }

  // override OpsInfo time-series values as needed
  
  _overrideOpsInfo();
  _wavelengthM = _opsInfo.get_radar_wavelength_cm() / 100.0;

  // mid pulse for most properties

  const IwrfTsPulse *midPulse = _pulses[_nSamplesHalf];
  
  // scan mode and pulsing mode

  _scanMode = (iwrf_scan_mode) midPulse->get_scan_mode();
  _xmitRcvMode = (iwrf_xmit_rcv_mode_t) _opsInfo.get_proc_xmit_rcv_mode();
  switch(_xmitRcvMode) {
    case IWRF_ALT_HV_CO_CROSS:
    case IWRF_SIM_HV_SWITCHED_HV:
      _switchingReceiver = true;
      break;
    default:
      _switchingReceiver = false;
  }

  if (_xmitRcvMode == IWRF_SINGLE_POL ||
      _xmitRcvMode == IWRF_SINGLE_POL_V) {
    _dualPol = false;
  } else {
    _dualPol = true;
  }

  // sweep and vol num

  _sweepNum = _getSweepNum();
  _volNum = _getVolNum();

  // range geometry

  _startRangeKm = midPulse->get_start_range_km();
  _gateSpacingKm = midPulse->get_gate_spacing_km();

  // pulse width
  
  _pulseWidth = midPulse->getPulseWidthUs() / 1.0e6;

  // transmitter power

  _measXmitPowerDbmH = midPulse->getMeasXmitPowerDbmH();
  _measXmitPowerDbmV = midPulse->getMeasXmitPowerDbmV();

  // set time

  _timeSecs = (time_t) midPulse->getTime();
  _nanoSecs = midPulse->getNanoSecs();
  _time.set(_timeSecs, (double) _nanoSecs / 1.0e9);

  // select the georeference from the mid pulse

  if (midPulse->getGeorefActive()) {
    _georef = midPulse->getPlatformGeoref();
    _georefActive = true;
  }

  // set elevation / azimuth

  _el = midPulse->getEl();
  _az = midPulse->getAz();
  if (_isRhi) {
    _targetAngle = midPulse->getFixedAz();
  } else {
    _targetAngle = midPulse->getFixedEl();
  }

  // compute nyquist etc

  if (_isStagPrt) {
    _initStagPrt(_nGatesPrtShort, _nGatesPrtLong, _prtShort, _prtLong);
    _nGates = _nGatesStagPrt;
    _unambigRange = _startRangeKm + _nGatesStagPrt * _gateSpacingKm;
  } else {
    _nyquist = ((_wavelengthM / _prt) / 4.0);
    _nyquistPrtShort = _nyquist;
    _nyquistPrtLong = _nyquist;
    _unambigRange = IwrfCalib::LightSpeedMps * _prt / 2000.0;
  }

  // compute number of output gates
  
  if (_isStagPrt) {
    _nGatesOut = _nGatesPrtLong;
  } else {
    _nGatesOut = _nGates;
  }

  // ffts and regression filter

  _fftInit();
  _regrInit();
  
  // moments computations object

  if (_nGatesOut > _nGatesOutAlloc) {
    if (_mom) delete _mom;
    _mom = new RadarMoments(_nGatesOut,
                            _params.debug >= Params::DEBUG_NORM,
                            _params.debug >= Params::DEBUG_VERBOSE);
    _nGatesOutAlloc = _nGatesOut;
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

  _haveChan1 = true;
  for (int ii = 0; ii < _nSamples; ii++) {
    if (_pulses[ii]->getIq1() == NULL) {
      _haveChan1 = false;
      break;
    }
  }

  TaArray<const fl32 *> iqChan1_;
  const fl32* *iqChan1 = NULL;
  if (_haveChan1) {
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

  // set up data pointer arrays
  // channel 0

  _iqChan0 = _iqChan0_.alloc(_nSamples);
  for (int ii = 0; ii < _nSamples; ii++) {
    _iqChan0[ii] = pulses[ii]->getIq0();
  }
  
  // channel 1 - will be NULLs for single pol
  
  _haveChan1 = true;
  for (int ii = 0; ii < _nSamples; ii++) {
    if (pulses[ii]->getIq1() == NULL) {
      _haveChan1 = false;
      break;
    }
  }

  if (_haveChan1) {
    _iqChan1 = _iqChan1_.alloc(_nSamples);
    for (int ii = 0; ii < _nSamples; ii++) {
      _iqChan1[ii] = pulses[ii]->getIq1();
    }
  }

  // SZ phase coding

  // compute phase differences between this pulse and previous ones
  // to prepare for cohering to multiple trips
  
  _computePhaseDiffs(_pulses, 4);
  _txDelta12.resize(_nSamples);
  for (int i = 0; i < _nSamples; i++) {
    _txDelta12[i].re = cos(_pulses[i]->getPhaseDiff0() * DEG_TO_RAD);
    _txDelta12[i].im = -1.0 * sin(_pulses[i]->getPhaseDiff0() * DEG_TO_RAD);
  }

  // set up burst phase vector
  
  _burstPhases.clear();
  for (size_t ii = 0; ii < _pulses.size(); ii++) {
    _burstPhases.push_back(_pulses[ii]->getBurstPhases());
  }

}

//////////////////////////////////////////////////////////////////
// destructor

Beam::~Beam()

{

  if (_mom) {
    delete _mom;
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
  if (volNums[_nSamplesHalf] < 0) {
    return 0;
  }
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

  if (sweepNums[_nSamplesHalf] < 0) {
    return -1;
  }
  return sweepNums[_nSamplesHalf];

}


/////////////////////////////////////////////////
// compute moments
    
int Beam::computeMoments(Params::fft_window_t windowType
                         /* = Params::FFT_WINDOW_VONHANN */)
  
{

  // initialize the windows

  _fftWindowType = windowType;
  _computeWindows();
  
  // set calibration data on Moments object, ready for computations

  string errStr;
  if (_calib.readFromXmlFile(_params.cal_file_path, errStr)) {
    cerr << "ERROR reading cal file: " << _params.cal_file_path << endl;
    return -1;
  }
  _mom->setCalib(_calib);

  // compute the moments
  
  _computeMoments();

  // in staggered mode, apply median filter to the nyquist unfolding
  // interval and re-compute the velocity
  
  if (_isStagPrt) {
    _cleanUpStagVel();
  }

  // apply clutter filtering if required

  _performClutterFiltering();

  // post-processing

  bool kdpAvail = false;
  switch (_xmitRcvMode) {
    case IWRF_ALT_HV_CO_ONLY:
    case IWRF_ALT_HV_CO_CROSS:
    case IWRF_ALT_HV_FIXED_HV:
    case IWRF_SIM_HV_FIXED_HV:
    case IWRF_SIM_HV_SWITCHED_HV: {
      kdpAvail = true;
      break;
    }
    default: {
      kdpAvail = false;
    }
  }

  // compute kdp

  if (kdpAvail) {
    _kdpCompute(true);
  }
  
  // copy the results to the output beam Field vectors

  _copyDataToOutputFields();

  // load up time series for dwell spectra

  _loadDwellSpectra();

  return 0;

}

/////////////////////////////////////////////////
// set the calibration
    
void Beam::setCalib(const IwrfCalib &calib)

{

  _calib = calib;
  _checkCalib();

}

////////////////////////////////////////////////
// get scan mode

int Beam::getScanMode() const

{

  int scanMode = _scanMode;

  if (_scanMode == IWRF_SCAN_MODE_NOT_SET) {
    scanMode = _opsInfo.get_scan_mode();
  }

  if (scanMode < 0) {
    scanMode = IWRF_SCAN_MODE_AZ_SUR_360;
  }

  return scanMode;

}

////////////////////////////////////////////////
// get gate num for a given range

int Beam::getGateNum(double range) const

{
  int gateNum = (int) floor((range - _startRangeKm) / _gateSpacingKm + 0.5);
  if (gateNum < 0) {
    gateNum = 0;
  } else if (gateNum > _nGates) {
    gateNum = _nGates;
  }
  return gateNum;
}

////////////////////////////////////////////////
// get range for a given gate

double Beam::getRange(int gateNum) const
  
{
  double range = _startRangeKm + gateNum * _gateSpacingKm;
  return range;
}

////////////////////////////////////////////////
// get closest exact range to a give range

double Beam::getClosestRange(double range, int &gateNum) const
  
{
  gateNum = getGateNum(range);
  double closestRange = getRange(gateNum);
  return closestRange;
}

////////////////////////////////////////////////
// get maximum range

double Beam::getMaxRange() const

{
  double maxRange = _startRangeKm + _nGates * _gateSpacingKm;
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
      
    case IWRF_SINGLE_POL_V:
      _computeMomSpV();
      break;
      
    case IWRF_SINGLE_POL:
    default:
      if (_isStagPrt) {
        _computeMomSpStagPrt();
      } else {
        _computeMomSpH();
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
      
    case IWRF_SINGLE_POL_V:
      _filterSpV();
      break;
      
    case IWRF_SINGLE_POL:
    default:
      if (_isStagPrt) {
        _filterSpStagPrt();
      } else {
        _filterSpH();
      }
      
  }

}

///////////////////////////////////////////////////////////
// Compute moments - single pol H channel
// Single pol, data in hc

void Beam::_computeMomSpH()

{

  // copy gate fields to _compFields array

  for (int igate = 0; igate < _nGates; igate++) {
    _compFields[igate] = _gateData[igate]->fields;
  }

  // compute covariances
  
  for (int igate = 0; igate < _nGates; igate++) {
    GateData *gate = _gateData[igate];
    MomentsFields &fields = _compFields[igate];
    _mom->computeCovarSinglePolH(gate->iqhc, fields);
  }
  
  // compute main moments
  
  for (int igate = 0; igate < _nGates; igate++) {
    
    MomentsFields &fields = _compFields[igate];
      
    _mom->computeMomSinglePolH(fields.lag0_hc,
                               fields.lag1_hc,
                               fields.lag2_hc,
                               fields.lag3_hc,
                               igate,
                               fields);
    
  } // igate

  // copy back to gate data

  for (int igate = 0; igate < _nGates; igate++) {
    _gateData[igate]->fields = _compFields[igate];
  }

}

///////////////////////////////////////////////////////////
// Compute moments - single pol V channel
// Single pol, data in vc

void Beam::_computeMomSpV()

{

  // copy gate fields to _compFields array

  for (int igate = 0; igate < _nGates; igate++) {
    _compFields[igate] = _gateData[igate]->fields;
  }

  // compute covariances
  
  for (int igate = 0; igate < _nGates; igate++) {
    GateData *gate = _gateData[igate];
    MomentsFields &fields = _compFields[igate];
    _mom->computeCovarSinglePolV(gate->iqvc, fields);
  }
  
  // compute main moments
  
  for (int igate = 0; igate < _nGates; igate++) {
    
    MomentsFields &fields = _compFields[igate];
      
    _mom->computeMomSinglePolV(fields.lag0_vc,
                               fields.lag1_vc,
                               fields.lag2_vc,
                               fields.lag3_vc,
                               igate,
                               fields);

  } // igate

  // copy back to gate data
  
  for (int igate = 0; igate < _nGates; igate++) {
    _gateData[igate]->fields = _compFields[igate];
  }

}

///////////////////////////////////////////////////////////
// Compute moments - SP - staggered PRT mode
// Single pol, data in hc

void Beam::_computeMomSpStagPrt()
  
{

  // copy gate fields to _compFields array

  for (int igate = 0; igate < _nGates; igate++) {
    _compFields[igate] = _gateData[igate]->fields;
  }

  // moments computations
  
  for (int igate = 0; igate < _nGates; igate++) {
      
    GateData *gate = _gateData[igate];
    MomentsFields &fields = _compFields[igate];
      
    // compute main moments
      
    _mom->singlePolHStagPrt(gate->iqhcOrig,
                            gate->iqhcPrtShort,
                            gate->iqhcPrtLong,
                            igate, false, fields);
    
  } // igate

  // copy back to gate data

  for (int igate = 0; igate < _nGates; igate++) {
    _gateData[igate]->fields = _compFields[igate];
  }

}

///////////////////////////////////////////////////////////
// Compute moments DP_ALT_HV_CO_CROSS
// Transmit alternating, receive co/cross

void Beam::_computeMomDpAltHvCoCross()
{

  // copy gate fields to _compFields array

  for (int igate = 0; igate < _nGates; igate++) {
    _compFields[igate] = _gateData[igate]->fields;
  }

  // compute covariances
  
  for (int igate = 0; igate < _nGates; igate++) {
    GateData *gate = _gateData[igate];
    MomentsFields &fields = _compFields[igate];
    _mom->computeCovarDpAltHvCoCross(gate->iqhc, gate->iqvc,
                                     gate->iqhx, gate->iqvx, 
                                     fields);
  }
  
  for (int igate = 0; igate < _nGates; igate++) {
      
    MomentsFields &fields = _compFields[igate];
      
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

  } // igate

  // copy back to gate data

  for (int igate = 0; igate < _nGates; igate++) {
    _gateData[igate]->fields = _compFields[igate];
  }

}

///////////////////////////////////////////////////////////
// Compute moments DP_ALT_HV_CO_ONLY
// Transmit alternating, receive copolar only

void Beam::_computeMomDpAltHvCoOnly()
{

  // copy gate fields to _compFields array

  for (int igate = 0; igate < _nGates; igate++) {
    _compFields[igate] = _gateData[igate]->fields;
  }

  // compute covariances
  
  for (int igate = 0; igate < _nGates; igate++) {
    GateData *gate = _gateData[igate];
    MomentsFields &fields = _compFields[igate];
    _mom->computeCovarDpAltHvCoOnly(gate->iqhc, gate->iqvc, fields);
  }
  
  for (int igate = 0; igate < _nGates; igate++) {
      
    MomentsFields &fields = _compFields[igate];
      
    // compute moments for this gate
      
    _mom->computeMomDpAltHvCoOnly(fields.lag0_hc,
                                  fields.lag0_vc,
                                  fields.lag1_vchc,
                                  fields.lag1_hcvc,
                                  fields.lag2_hc,
                                  fields.lag2_vc,
                                  igate, 
                                  fields);
    
  } // igate

  // copy back to gate data

  for (int igate = 0; igate < _nGates; igate++) {
    _gateData[igate]->fields = _compFields[igate];
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

  // copy gate fields to _compFields array
  
  for (int igate = 0; igate < _nGates; igate++) {
    _compFields[igate] = _gateData[igate]->fields;
  }

  // compute covariances
  
  for (int igate = 0; igate < _nGates; igate++) {
    GateData *gate = _gateData[igate];
    MomentsFields &fields = _compFields[igate];
    _mom->computeCovarDpSimHv(gate->iqhc, gate->iqvc, fields);
  }
  
  for (int igate = 0; igate < _nGates; igate++) {
      
    MomentsFields &fields = _compFields[igate];
      
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
    
  } // igate

  // copy back to gate data

  for (int igate = 0; igate < _nGates; igate++) {
    _gateData[igate]->fields = _compFields[igate];
  }

}

///////////////////////////////////////////////////////////
// Compute moments DP_SIM_HV_FIXED_HV
// Simultaneous transmission, fixed receive

void Beam::_computeMomDpSimHvStagPrt()
{

  // copy gate fields to _compFields array
  
  for (int igate = 0; igate < _nGates; igate++) {
    _compFields[igate] = _gateData[igate]->fields;
  }

  for (int igate = 0; igate < _nGates; igate++) {
      
    GateData *gate = _gateData[igate];
    MomentsFields &fields = _compFields[igate];
      
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
    _gateData[igate]->fields = _compFields[igate];
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

  // copy gate fields to _compFields array

  for (int igate = 0; igate < _nGates; igate++) {
    _compFields[igate] = _gateData[igate]->fields;
  }

  // compute covariances
  
  for (int igate = 0; igate < _nGates; igate++) {
    GateData *gate = _gateData[igate];
    MomentsFields &fields = _compFields[igate];
    _mom->computeCovarDpHOnly(gate->iqhc, gate->iqvx, fields);
  }
  
  for (int igate = 0; igate < _nGates; igate++) {
      
    MomentsFields &fields = _compFields[igate];
      
    // compute moments for this gate
    
    _mom->computeMomDpHOnly(fields.lag0_hc, 
                            fields.lag0_vx,
                            fields.lag1_hc,
                            fields.lag2_hc,
                            fields.lag3_hc,
                            igate,
                            fields);

  } // igate

  // copy back to gate data

  for (int igate = 0; igate < _nGates; igate++) {
    _gateData[igate]->fields = _compFields[igate];
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

  // copy gate fields to _compFields array

  for (int igate = 0; igate < _nGates; igate++) {
    _compFields[igate] = _gateData[igate]->fields;
  }

  // compute covariances
  
  for (int igate = 0; igate < _nGates; igate++) {
    GateData *gate = _gateData[igate];
    MomentsFields &fields = _compFields[igate];
    _mom->computeCovarDpVOnly(gate->iqvc, gate->iqhx, fields);
  }
  
  for (int igate = 0; igate < _nGates; igate++) {
      
    MomentsFields &fields = _compFields[igate];
      
    // compute moments for this gate
      
    _mom->computeMomDpVOnly(fields.lag0_vc, 
                            fields.lag0_hx,
                            fields.lag1_vc,
                            fields.lag2_vc,
                            fields.lag3_vc,
                            igate,
                            fields);
    
  } // igate

  // copy back to gate data

  for (int igate = 0; igate < _nGates; igate++) {
    _gateData[igate]->fields = _compFields[igate];
  }

}

///////////////////////////////////////////////////////////
// Filter clutter SP, horizontal channel
// Single pol, data in hc

void Beam::_filterSpH()
{

  double calibNoise = _mom->getCalNoisePower(RadarMoments::CHANNEL_HC);

  for (int igate = 0; igate < _nGates; igate++) {
      
    GateData *gate = _gateData[igate];
    MomentsFields &fields = gate->fields;
    MomentsFields &fieldsF = gate->fieldsF;

    // filter the HC time series
    
    double spectralNoiseHc = 1.0e-13;
    double filterRatioHc = 1.0;
    double spectralSnrHc = 1.0;

    _mom->applyClutterFilter(_nSamples, _prt, *_fft, *_regr, _window,
                             gate->iqhcOrig, gate->iqhc,
                             calibNoise,
                             gate->iqhcF, gate->iqhcNotched,
                             filterRatioHc, spectralNoiseHc, spectralSnrHc);
    
    if (filterRatioHc > 1.0) {
      fields.clut_2_wx_ratio = 10.0 * log10(filterRatioHc - 1.0);
    } else {
      fields.clut_2_wx_ratio = MomentsFields::missingDouble;
    }
    fields.spectral_noise = 10.0 * log10(spectralNoiseHc);
    fields.spectral_snr = 10.0 * log10(spectralSnrHc);

    // compute filtered moments for this gate
    
    _mom->computeCovarSinglePolH(gate->iqhcF, fieldsF);
    
    _mom->computeMomSinglePolH(fieldsF.lag0_hc, fieldsF.lag1_hc,
                               fieldsF.lag2_hc, fieldsF.lag3_hc,
                               igate, fieldsF);
    
    // compute clutter power
    
    gate->fields.clut = _computeClutPower(fields, fieldsF);
    
  } // igate

}

///////////////////////////////////////////////////////////
// Filter clutter SP, vertical channel
// Single pol, data in vc

void Beam::_filterSpV()
{

  double calibNoise = _mom->getCalNoisePower(RadarMoments::CHANNEL_VC);

  for (int igate = 0; igate < _nGates; igate++) {
      
    GateData *gate = _gateData[igate];
    MomentsFields &fields = gate->fields;
    MomentsFields &fieldsF = gate->fieldsF;

    // check if we have clutter at this gate
    
    // if (!gate->fields.cmd_flag) {
    //   continue;
    // }
      
    // filter the VC time series
    
    double spectralNoiseVc = 1.0e-13;
    double filterRatioVc = 1.0;
    double spectralSnrVc = 1.0;

    _mom->applyClutterFilter(_nSamples, _prt, *_fft, *_regr, _window,
                             gate->iqvcOrig, gate->iqvc,
                             calibNoise,
                             gate->iqvcF, gate->iqvcNotched,
                             filterRatioVc, spectralNoiseVc, spectralSnrVc);
    
    if (filterRatioVc > 1.0) {
      fields.clut_2_wx_ratio = 10.0 * log10(filterRatioVc - 1.0);
    } else {
      fields.clut_2_wx_ratio = MomentsFields::missingDouble;
    }
    fields.spectral_noise = 10.0 * log10(spectralNoiseVc);
    fields.spectral_snr = 10.0 * log10(spectralSnrVc);
    
    // compute filtered moments for this gate
    
    _mom->computeCovarSinglePolV(gate->iqvcF, fieldsF);
    
    _mom->computeMomSinglePolV(fieldsF.lag0_vc, fieldsF.lag1_vc,
                               fieldsF.lag2_vc, fieldsF.lag3_vc,
                               igate, fieldsF);
    
    // compute clutter power
    
    gate->fields.clut = _computeClutPower(fields, fieldsF);
    
  } // igate

}

///////////////////////////////////////////////////////////
// Single Pol, staggered PRT filter

void Beam::_filterSpStagPrt()
  
{

  if (_params.ascope_clutter_filter_type == Params::CLUTTER_FILTER_ADAPTIVE) {
    _filterAdapSpStagPrt();
  } else {
    _filterRegrSpStagPrt();
  }

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
    
    // if (!fields.cmd_flag) {
    //   continue;
    // }
      
    // filter the short prt time series
    
    double spectralNoiseHc = 1.0e-13;
    double filterRatioHc = 1.0;
    double spectralSnrHc = 1.0;

    _mom->applyAdapFilterStagPrt(_nSamplesHalf, _prt, _prtLong, *_fftHalf,
                                 gate->iqhcPrtShort, gate->iqhcPrtLong,
                                 calibNoise,
                                 gate->iqhcPrtShortF, gate->iqhcPrtLongF,
                                 gate->iqhcPrtShortNotched, gate->iqhcPrtLongNotched,
                                 filterRatioHc, spectralNoiseHc, spectralSnrHc);
    
    if (filterRatioHc > 1.0) {
      fields.clut_2_wx_ratio = 10.0 * log10(filterRatioHc - 1.0);
    } else {
      fields.clut_2_wx_ratio = MomentsFields::missingDouble;
    }
    fields.spectral_noise = 10.0 * log10(spectralNoiseHc);
    fields.spectral_snr = 10.0 * log10(spectralSnrHc);
    
    // compute filtered moments for this gate
    
    _mom->singlePolHStagPrt(gate->iqhc, gate->iqhcPrtShortF, gate->iqhcPrtLongF,
                            igate, true, fieldsF);
    
    // compute clutter power
    
    fields.clut = _computeClutPower(fields, fieldsF);
    
  } // igate

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
    
    // if (!fields.cmd_flag) {
    //   continue;
    // }
      
    // filter the short prt time series
    
    double spectralNoiseHc = 1.0e-13;
    double filterRatioHc = 1.0;
    double spectralSnrHc = 1.0;

    // bool interpAcrossNotch = true;
    // memcpy(gate->iqhcF, gate->iqhcOrig, _nSamples * sizeof(RadarComplex_t));
    
    _mom->applyRegrFilterStagPrt(_nSamplesHalf, _prt, _prtLong, *_fftHalf, *_regrHalf,
                                 gate->iqhcPrtShort, gate->iqhcPrtLong,
                                 calibNoise,
                                 gate->iqhcPrtShortF, gate->iqhcPrtLongF,
                                 gate->iqhcPrtShortNotched, gate->iqhcPrtLongNotched,
                                 filterRatioHc, spectralNoiseHc, spectralSnrHc);
    
    if (filterRatioHc > 1.0) {
      fields.clut_2_wx_ratio = 10.0 * log10(filterRatioHc - 1.0);
    } else {
      fields.clut_2_wx_ratio = MomentsFields::missingDouble;
    }
    fields.spectral_noise = 10.0 * log10(spectralNoiseHc);
    fields.spectral_snr = 10.0 * log10(spectralSnrHc);
    
    // compute filtered moments for this gate
    
    _mom->singlePolHStagPrt(gate->iqhc,
                            gate->iqhcPrtShortF,
                            gate->iqhcPrtLongF,
                            igate, true, fieldsF);
    
    // compute clutter power
    
    fields.clut = _computeClutPower(fields, fieldsF);
    
  } // igate

}

///////////////////////////////////////////////////////////
// Filter clutter DP_ALT_HV_CO_CROSS
// Transmit alternating, receive co/cross

void Beam::_filterDpAltHvCoCross()
{

  // copy gate fields to _compFieldsF array

  for (int igate = 0; igate < _nGates; igate++) {
    _compFieldsF[igate] = _gateData[igate]->fieldsF;
  }

  double calibNoise = _mom->getCalNoisePower(RadarMoments::CHANNEL_HC);
  
  for (int igate = 0; igate < _nGates; igate++) {
      
    GateData *gate = _gateData[igate];
    MomentsFields &fields = gate->fields;
    MomentsFields &fieldsF = _compFieldsF[igate];
    
    if (_params.ascope_clutter_filter_type == Params::CLUTTER_FILTER_ADAPTIVE) {

      // filter the HC time series, save the filter ratio
      
      double spectralNoiseHc = 1.0e-13;
      double filterRatioHc = 1.0;
      double spectralSnrHc = 1.0;
      _mom->applyClutterFilter(_nSamplesHalf, _prt * 2.0,
                               *_fftHalf, *_regrHalf, _windowHalf,
                               gate->iqhcOrig, gate->iqhc,
                               calibNoise,
                               gate->iqhcF, gate->iqhcNotched,
                               filterRatioHc, spectralNoiseHc, spectralSnrHc,
                               false);
      
      if (filterRatioHc > 1.0) {
        fields.clut_2_wx_ratio = 10.0 * log10(filterRatioHc - 1.0);
      } else {
        fields.clut_2_wx_ratio = MomentsFields::missingDouble;
      }
      fields.spectral_noise = 10.0 * log10(spectralNoiseHc);
      fields.spectral_snr = 10.0 * log10(spectralSnrHc);
      
      // filter the other channels, using the same notch as Hc
      
      double filterRatioVc, spectralNoiseVc, spectralSnrVc;
      _mom->applyClutterFilter(_nSamplesHalf, _prt * 2.0,
                               *_fftHalf, *_regrHalf, _windowHalf,
                               gate->iqvcOrig, gate->iqvc,
                               calibNoise,
                               gate->iqvcF, gate->iqvcNotched,
                               filterRatioVc, spectralNoiseVc,
                               spectralSnrVc, true);
      
      double filterRatioHx, spectralNoiseHx, spectralSnrHx;
      _mom->applyClutterFilter(_nSamplesHalf, _prt * 2.0,
                               *_fftHalf, *_regrHalf, _windowHalf,
                               gate->iqhxOrig, gate->iqhx,
                               calibNoise,
                               gate->iqhxF, NULL,
                               filterRatioHx, spectralNoiseHx,
                               spectralSnrHx, true);
      
      double filterRatioVx, spectralNoiseVx, spectralSnrVx;
      _mom->applyClutterFilter(_nSamplesHalf, _prt * 2.0,
                               *_fftHalf, *_regrHalf, _windowHalf,
                               gate->iqvxOrig, gate->iqvx,
                               calibNoise,
                               gate->iqvxF, NULL,
                               filterRatioVx, spectralNoiseVx,
                               spectralSnrVx, true);
      
    } else {
      
      // regression - filter all of the channels individually - Hc, Vc, Hx, Vx
      
      double filterRatioHc, spectralNoiseHc, spectralSnrHc;
      _mom->applyClutterFilter(_nSamplesHalf, _prt * 2.0,
                               *_fftHalf, *_regrHalf, _windowHalf,
                               gate->iqhcOrig, gate->iqhc, calibNoise,
                               gate->iqhcF, gate->iqhcNotched,
                               filterRatioHc, spectralNoiseHc, spectralSnrHc);

      double filterRatioVc, spectralNoiseVc, spectralSnrVc;
      _mom->applyClutterFilter(_nSamplesHalf, _prt * 2.0,
                               *_fftHalf, *_regrHalf, _windowHalf,
                               gate->iqvcOrig, gate->iqvc, calibNoise,
                               gate->iqvcF, gate->iqvcNotched,
                               filterRatioVc, spectralNoiseVc, spectralSnrVc);

      double filterRatioHx, spectralNoiseHx, spectralSnrHx;
      _mom->applyClutterFilter(_nSamplesHalf, _prt * 2.0,
                               *_fftHalf, *_regrHalf, _windowHalf,
                               gate->iqhxOrig, gate->iqhx, calibNoise,
                               gate->iqhxF, NULL,
                               filterRatioHx, spectralNoiseHx, spectralSnrHx);

      double filterRatioVx, spectralNoiseVx, spectralSnrVx;
      _mom->applyClutterFilter(_nSamplesHalf, _prt * 2.0,
                               *_fftHalf, *_regrHalf, _windowHalf,
                               gate->iqvxOrig, gate->iqvx, calibNoise,
                               gate->iqvxF, NULL,
                               filterRatioVx, spectralNoiseVx, spectralSnrVx);
      
    }
    
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

    // compute notched moments for rhohv, phidp and zdr

    MomentsFields fieldsN;
    _mom->computeCovarDpAltHvCoCross(gate->iqhcF, gate->iqvcF,
                                     gate->iqhxF, gate->iqvxF, 
                                     fieldsN);
    _mom->computeMomDpAltHvCoCross(fieldsN.lag0_hc,
                                   fieldsN.lag0_hx,
                                   fieldsN.lag0_vc,
                                   fieldsN.lag0_vx,
                                   fieldsN.lag0_vchx,
                                   fieldsN.lag0_hcvx,
                                   fieldsN.lag1_vxhx,
                                   fieldsN.lag1_vchc,
                                   fieldsN.lag1_hcvc,
                                   fieldsN.lag2_hc,
                                   fieldsN.lag2_vc,
                                   igate, 
                                   fieldsN);

    fieldsF.test0 = fieldsN.zdr;
    fieldsF.test2 = fieldsN.phidp;
    fieldsF.test3 = fieldsN.rhohv;
    
    // compute clutter power
    
    fields.clut = _computeClutPower(fields, fieldsF);

  } // igate
  
  // copy back to gate data

  for (int igate = 0; igate < _nGates; igate++) {
    _gateData[igate]->fieldsF = _compFieldsF[igate];
  }

}

///////////////////////////////////////////////////////////
// Filter clutter DP_ALT_HV_CO_ONLY
// Transmit alternating, receive copolar only

void Beam::_filterDpAltHvCoOnly()
{

  // copy gate fields to _compFieldsF array

  for (int igate = 0; igate < _nGates; igate++) {
    _compFieldsF[igate] = _gateData[igate]->fieldsF;
  }

  double calibNoise = _mom->getCalNoisePower(RadarMoments::CHANNEL_HC);

  for (int igate = 0; igate < _nGates; igate++) {
      
    GateData *gate = _gateData[igate];
    MomentsFields &fields = gate->fields;
    MomentsFields &fieldsF = _compFieldsF[igate];
    
    // filter the HC time series, save the filter ratio
    
    double spectralNoiseHc = 1.0e-13;
    double filterRatioHc = 1.0;
    double spectralSnrHc = 1.0;
    _mom->applyClutterFilter(_nSamplesHalf, _prt * 2.0,
                             *_fftHalf, *_regrHalf, _windowHalf,
                             gate->iqhcOrig, gate->iqhc,
                             calibNoise,
                             gate->iqhcF, gate->iqhcNotched,
                             filterRatioHc, spectralNoiseHc, spectralSnrHc,
                             false);
    
    if (filterRatioHc > 1.0) {
      fields.clut_2_wx_ratio = 10.0 * log10(filterRatioHc - 1.0);
    } else {
      fields.clut_2_wx_ratio = MomentsFields::missingDouble;
    }
    fields.spectral_noise = 10.0 * log10(spectralNoiseHc);
    fields.spectral_snr = 10.0 * log10(spectralSnrHc);
    
    // testing cnr from 3-order regression filter

    fields.test3 = _mom->getRegrInterpRatioDb();
    fields.test4 = _regrHalf->getPolyOrder();
    fields.test5 = _mom->getRegrCnrDb();

    // filter the Vc channel using the same notch as Hc
    
    double filterRatioVc, spectralNoiseVc, spectralSnrVc;
    _mom->applyClutterFilter(_nSamplesHalf, _prt * 2.0,
                             *_fftHalf, *_regrHalf, _windowHalf,
                             gate->iqvcOrig, gate->iqvc,
                             calibNoise,
                             gate->iqvcF, gate->iqvcNotched,
                             filterRatioVc, spectralNoiseVc, spectralSnrVc,
                             true);
      
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
  
  // copy back to gate data

  for (int igate = 0; igate < _nGates; igate++) {
    _gateData[igate]->fieldsF = _compFieldsF[igate];
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
    
    // if (!fields.cmd_flag) {
    //   continue;
    // }
      
    // filter the HC time series
    
    double spectralNoiseHc = 1.0e-13;
    double filterRatioHc = 1.0;
    double spectralSnrHc = 1.0;
    _mom->applyClutterFilter(_nSamples, _prt, *_fft, *_regr, _window,
                             gate->iqhcOrig, gate->iqhc,
                             calibNoise,
                             gate->iqhcF, gate->iqhcNotched,
                             filterRatioHc, spectralNoiseHc, spectralSnrHc,
                             false);

    if (filterRatioHc > 1.0) {
      fields.clut_2_wx_ratio = 10.0 * log10(filterRatioHc - 1.0);
    } else {
      fields.clut_2_wx_ratio = MomentsFields::missingDouble;
    }
    fields.spectral_noise = 10.0 * log10(spectralNoiseHc);
    fields.spectral_snr = 10.0 * log10(spectralSnrHc);
    
    // filter Vc channel using the same notch as Hc
    
    double filterRatioVc, spectralNoiseVc, spectralSnrVc;
    _mom->applyClutterFilter(_nSamples, _prt, *_fft, *_regr, _window,
                             gate->iqvcOrig, gate->iqvc,
                             calibNoise,
                             gate->iqvcF, gate->iqvcNotched,
                             filterRatioVc, spectralNoiseVc, spectralSnrVc,
                             true);
      
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
    
    // if (!fields.cmd_flag) {
    //   continue;
    // }
      
    // filter the HC time series
    
    double spectralNoiseHc = 1.0e-13;
    double filterRatioHc = 1.0;
    double spectralSnrHc = 1.0;
    if (_params.ascope_clutter_filter_type == Params::CLUTTER_FILTER_ADAPTIVE) {
      _mom->applyAdapFilterStagPrt(_nSamplesHalf, _prt, _prtLong, *_fftHalf,
                                   gate->iqhcPrtShort, gate->iqhcPrtLong,
                                   calibNoise,
                                   gate->iqhcPrtShortF, gate->iqhcPrtLongF,
                                   gate->iqhcPrtShortNotched, gate->iqhcPrtLongNotched,
                                   filterRatioHc, spectralNoiseHc, spectralSnrHc,
                                   false);
    } else {
      _mom->applyRegrFilterStagPrt(_nSamplesHalf, _prt, _prtLong, *_fftHalf, *_regrHalf,
                                   gate->iqhcPrtShort, gate->iqhcPrtLong,
                                   calibNoise,
                                   gate->iqhcPrtShortF, gate->iqhcPrtLongF,
                                   gate->iqhcPrtShortNotched, gate->iqhcPrtLongNotched,
                                   filterRatioHc, spectralNoiseHc, spectralSnrHc);
    }
    
    if (filterRatioHc > 1.0) {
      fields.clut_2_wx_ratio = 10.0 * log10(filterRatioHc - 1.0);
    } else {
      fields.clut_2_wx_ratio = MomentsFields::missingDouble;
    }
    fields.spectral_noise = 10.0 * log10(spectralNoiseHc);
    fields.spectral_snr = 10.0 * log10(spectralSnrHc);
    
    // apply filter to Vc
    // for adaptive filter use the same notch as Hc
    
    double filterRatioVc, spectralNoiseVc, spectralSnrVc;
    if (_params.ascope_clutter_filter_type == Params::CLUTTER_FILTER_ADAPTIVE) {
      _mom->applyAdapFilterStagPrt(_nSamplesHalf, _prt, _prtLong, *_fftHalf,
                                   gate->iqvcPrtShort, gate->iqvcPrtLong,
                                   calibNoise,
                                   gate->iqvcPrtShortF, gate->iqvcPrtLongF,
                                   gate->iqvcPrtShortNotched, gate->iqvcPrtLongNotched,
                                   filterRatioVc, spectralNoiseVc, spectralSnrVc,
                                   true);
    } else {
      _mom->applyRegrFilterStagPrt(_nSamplesHalf, _prt, _prtLong, *_fftHalf, *_regrHalf,
                                   gate->iqvcPrtShort, gate->iqvcPrtLong,
                                   calibNoise,
                                   gate->iqvcPrtShortF, gate->iqvcPrtLongF,
                                   gate->iqvcPrtShortNotched, gate->iqvcPrtLongNotched,
                                   filterRatioVc, spectralNoiseVc, spectralSnrVc);
    }

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
    
    // if (!fields.cmd_flag) {
    //   continue;
    // }
      
    // filter the HC time series
    
    double spectralNoiseHc = 1.0e-13;
    double filterRatioHc = 1.0;
    double spectralSnrHc = 1.0;
    _mom->applyClutterFilter(_nSamples, _prt, *_fft, *_regr, _window,
                             gate->iqhcOrig, gate->iqhc,
                             calibNoise,
                             gate->iqhcF, gate->iqhcNotched,
                             filterRatioHc, spectralNoiseHc, spectralSnrHc,
                             false);

    if (filterRatioHc > 1.0) {
      fields.clut_2_wx_ratio = 10.0 * log10(filterRatioHc - 1.0);
    } else {
      fields.clut_2_wx_ratio = MomentsFields::missingDouble;
    }
    fields.spectral_noise = 10.0 * log10(spectralNoiseHc);
    fields.spectral_snr = 10.0 * log10(spectralSnrHc);
    
    // testing cnr from 3-order regression filter

    fields.test3 = _mom->getRegrInterpRatioDb();
    fields.test4 = _regr->getPolyOrder();
    fields.test5 = _mom->getRegrCnrDb();
    
    // apply the filter to the Vx channel
    // for the adaptive filter use the same notch as for Hc

    double spectralNoiseVx = 1.0e-13;
    double filterRatioVx = 1.0;
    double spectralSnrVx = 1.0;
    
    _mom->applyClutterFilter(_nSamples, _prt, *_fft, *_regr, _window,
                             gate->iqvxOrig, gate->iqvx,
                             calibNoise,
                             gate->iqvxF, gate->iqvxNotched,
                             filterRatioVx, spectralNoiseVx, spectralSnrVx,
                             true);

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
    
    // if (!fields.cmd_flag) {
    //   continue;
    // }
      
    // SHORT PRT filter the HC time series

    double spectralNoiseHc = 1.0e-13;
    double filterRatioHc = 1.0;
    double spectralSnrHc = 1.0;
    if (_params.ascope_clutter_filter_type == Params::CLUTTER_FILTER_ADAPTIVE) {
      _mom->applyAdapFilterStagPrt(_nSamplesHalf, _prt, _prtLong, *_fftHalf,
                                   gate->iqhcPrtShort, gate->iqhcPrtLong,
                                   calibNoise,
                                   gate->iqhcPrtShortF, gate->iqhcPrtLongF,
                                   gate->iqhcPrtShortNotched, gate->iqhcPrtLongNotched,
                                   filterRatioHc, spectralNoiseHc, spectralSnrHc,
                                   false);
    } else {
      _mom->applyRegrFilterStagPrt(_nSamplesHalf, _prt, _prtLong, *_fftHalf, *_regrHalf,
                                   gate->iqhcPrtShort, gate->iqhcPrtLong,
                                   calibNoise,
                                   gate->iqhcPrtShortF, gate->iqhcPrtLongF,
                                   gate->iqhcPrtShortNotched, gate->iqhcPrtLongNotched,
                                   filterRatioHc, spectralNoiseHc, spectralSnrHc);
    }
    
    if (filterRatioHc > 1.0) {
      fields.clut_2_wx_ratio = 10.0 * log10(filterRatioHc - 1.0);
    } else {
      fields.clut_2_wx_ratio = MomentsFields::missingDouble;
    }
    fields.spectral_noise = 10.0 * log10(spectralNoiseHc);
    fields.spectral_snr = 10.0 * log10(spectralSnrHc);
    
    // apply the filter to the Vx channel
    // for adaptive use the same notch as the Hc channel
    
    double spectralNoiseVx = 1.0e-13;
    double filterRatioVx = 1.0;
    double spectralSnrVx = 1.0;
    if (_params.ascope_clutter_filter_type == Params::CLUTTER_FILTER_ADAPTIVE) {
      _mom->applyAdapFilterStagPrt(_nSamplesHalf, _prt, _prtLong, *_fftHalf,
                                   gate->iqvxPrtShort, gate->iqvxPrtLong,
                                   calibNoise,
                                   gate->iqvxPrtShortF, gate->iqvxPrtLongF,
                                   gate->iqvxPrtShortNotched, gate->iqvxPrtLongNotched,
                                   filterRatioVx, spectralNoiseVx, spectralSnrVx,
                                   false);
    } else {
      _mom->applyRegrFilterStagPrt(_nSamplesHalf, _prt, _prtLong, *_fftHalf, *_regrHalf,
                                   gate->iqvxPrtShort, gate->iqvxPrtLong,
                                   calibNoise,
                                   gate->iqvxPrtShortF, gate->iqvxPrtLongF,
                                   gate->iqvxPrtShortNotched, gate->iqvxPrtLongNotched,
                                   filterRatioVx, spectralNoiseVx, spectralSnrVx);
    }
    
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
    
    // if (!fields.cmd_flag) {
    //   continue;
    // }
      
    // filter the HC time series
    
    double spectralNoiseVc = 1.0e-13;
    double filterRatioVc = 1.0;
    double spectralSnrVc = 1.0;
    _mom->applyClutterFilter(_nSamples, _prt, *_fft, *_regr, _window,
                             gate->iqvcOrig, gate->iqvc,
                             calibNoise,
                             gate->iqvcF, gate->iqvcNotched,
                             filterRatioVc, spectralNoiseVc, spectralSnrVc,
                             false);

    if (filterRatioVc > 1.0) {
      fields.clut_2_wx_ratio = 10.0 * log10(filterRatioVc - 1.0);
    } else {
      fields.clut_2_wx_ratio = MomentsFields::missingDouble;
    }
    fields.spectral_noise = 10.0 * log10(spectralNoiseVc);
    fields.spectral_snr = 10.0 * log10(spectralSnrVc);
    
    // apply the filter to the Hx channel
    // for adaptive use the same notch as the Vc channel
    
    double spectralNoiseHx = 1.0e-13;
    double filterRatioHx = 1.0;
    double spectralSnrHx = 1.0;
    _mom->applyClutterFilter(_nSamples, _prt, *_fft, *_regr, _window,
                             gate->iqhxOrig, gate->iqhx,
                             calibNoise,
                             gate->iqhxF, gate->iqhxNotched,
                             filterRatioHx, spectralNoiseHx, spectralSnrHx,
                             true);

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
    
    // if (!fields.cmd_flag) {
    //   continue;
    // }

    // SHORT PRT
    // filter the VC time series
    
    double spectralNoiseVc = 1.0e-13;
    double filterRatioVc = 1.0;
    double spectralSnrVc = 1.0;
    if (_params.ascope_clutter_filter_type == Params::CLUTTER_FILTER_ADAPTIVE) {
      _mom->applyAdapFilterStagPrt(_nSamplesHalf, _prt, _prtLong, *_fftHalf,
                                   gate->iqvcPrtShort, gate->iqvcPrtLong,
                                   calibNoise,
                                   gate->iqvcPrtShortF, gate->iqvcPrtLongF,
                                   gate->iqvcPrtShortNotched, gate->iqvcPrtLongNotched,
                                   filterRatioVc, spectralNoiseVc, spectralSnrVc,
                                   false);
    } else {
      _mom->applyRegrFilterStagPrt(_nSamplesHalf, _prt, _prtLong, *_fftHalf, *_regrHalf,
                                   gate->iqvcPrtShort, gate->iqvcPrtLong,
                                   calibNoise,
                                   gate->iqvcPrtShortF, gate->iqvcPrtLongF,
                                   gate->iqvcPrtShortNotched, gate->iqvcPrtLongNotched,
                                   filterRatioVc, spectralNoiseVc, spectralSnrVc);
    }
    
    if (filterRatioVc > 1.0) {
      fields.clut_2_wx_ratio = 10.0 * log10(filterRatioVc - 1.0);
    } else {
      fields.clut_2_wx_ratio = MomentsFields::missingDouble;
    }
    fields.spectral_noise = 10.0 * log10(spectralNoiseVc);
    fields.spectral_snr = 10.0 * log10(spectralSnrVc);
    
    // apply the filter to Hx channel
    // for the adaptive filter use the same notch
    
    double spectralNoiseHx = 1.0e-13;
    double filterRatioHx = 1.0;
    double spectralSnrHx = 1.0;
    if (_params.ascope_clutter_filter_type == Params::CLUTTER_FILTER_ADAPTIVE) {
      _mom->applyAdapFilterStagPrt(_nSamplesHalf, _prt, _prtLong, *_fftHalf,
                                   gate->iqhxPrtShort, gate->iqhxPrtLong,
                                   calibNoise,
                                   gate->iqhxPrtShortF, gate->iqhxPrtLongF,
                                   gate->iqhxPrtShortNotched, gate->iqhxPrtLongNotched,
                                   filterRatioHx, spectralNoiseHx, spectralSnrHx,
                                   true);
    } else {
      _mom->applyRegrFilterStagPrt(_nSamplesHalf, _prt, _prtLong, *_fftHalf, *_regrHalf,
                                   gate->iqhxPrtShort, gate->iqhxPrtLong,
                                   calibNoise,
                                   gate->iqhxPrtShortF, gate->iqhxPrtLongF,
                                   gate->iqhxPrtShortNotched, gate->iqhxPrtLongNotched,
                                   filterRatioHx, spectralNoiseHx, spectralSnrHx);
    }

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

  if (_params.debug >= Params::DEBUG_EXTRA) {
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

  if (_params.debug >= Params::DEBUG_EXTRA) {
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
    
  if (_params.debug >= Params::DEBUG_EXTRA) {
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

  if (_params.ascope_clutter_filter_type == Params::CLUTTER_FILTER_REGRESSION) {
    _window = RadarMoments::createWindowRect(_nSamples);
    _windowHalf = RadarMoments::createWindowRect(_nSamplesHalf);
  } else if (_fftWindowType == Params::FFT_WINDOW_RECT) {
    _window = RadarMoments::createWindowVonhann(_nSamples);
    _windowHalf = RadarMoments::createWindowVonhann(_nSamplesHalf);
  } else if (_fftWindowType == Params::FFT_WINDOW_VONHANN) {
    _window = RadarMoments::createWindowVonhann(_nSamples);
    _windowHalf = RadarMoments::createWindowVonhann(_nSamplesHalf);
  } else if (_fftWindowType == Params::FFT_WINDOW_BLACKMAN) {
    _window = RadarMoments::createWindowBlackman(_nSamples);
    _windowHalf = RadarMoments::createWindowBlackman(_nSamplesHalf);
  } else if (_fftWindowType == Params::FFT_WINDOW_BLACKMAN_NUTTALL) {
    _window = RadarMoments::createWindowBlackmanNuttall(_nSamples);
    _windowHalf = RadarMoments::createWindowBlackmanNuttall(_nSamplesHalf);
  } else if (_fftWindowType == Params::FFT_WINDOW_TUKEY_10) {
    _window = RadarMoments::createWindowTukey(0.1, _nSamples);
    _windowHalf = RadarMoments::createWindowTukey(0.1, _nSamplesHalf);
  } else if (_fftWindowType == Params::FFT_WINDOW_TUKEY_20) {
    _window = RadarMoments::createWindowTukey(0.2, _nSamples);
    _windowHalf = RadarMoments::createWindowTukey(0.2, _nSamplesHalf);
  } else if (_fftWindowType == Params::FFT_WINDOW_TUKEY_30) {
    _window = RadarMoments::createWindowTukey(0.3, _nSamples);
    _windowHalf = RadarMoments::createWindowTukey(0.3, _nSamplesHalf);
  } else if (_fftWindowType == Params::FFT_WINDOW_TUKEY_50) {
    _window = RadarMoments::createWindowTukey(0.5, _nSamples);
    _windowHalf = RadarMoments::createWindowTukey(0.5, _nSamplesHalf);
  }

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

  if (_params.ascope_clutter_filter_type == Params::CLUTTER_FILTER_REGRESSION) {
    _regr->setup(_nSamples,
                 _params.regression_filter_determine_order_from_cnr,
                 _params.regression_filter_specified_polynomial_order,
                 _params.regression_filter_clutter_width_factor,
                 _params.regression_filter_cnr_exponent,
                 _wavelengthM);
    _regrHalf->setup(_nSamplesHalf,
                     _params.regression_filter_determine_order_from_cnr,
                     _params.regression_filter_specified_polynomial_order,
                     _params.regression_filter_clutter_width_factor,
                     _params.regression_filter_cnr_exponent,
                     _wavelengthM);
    _regrStag->setupStaggered(_nSamples, _stagM, _stagN,
                              _params.regression_filter_determine_order_from_cnr,
                              _params.regression_filter_specified_polynomial_order,
                              _params.regression_filter_clutter_width_factor,
                              _params.regression_filter_cnr_exponent,
                              _wavelengthM);
    _mom->setUseRegressionFilter
      (RadarMoments::INTERP_METHOD_GAUSSIAN, _params.regression_filter_min_cnr_db);
  } else if (_params.use_simple_notch_clutter_filter) {
    _mom->setUseSimpleNotchFilter(_params.simple_notch_filter_width_mps);
  }

  _mom->setCorrectForSystemPhidp(false);
  
  _mom->setWindowRValues(_windowR1,
                         _windowR2,
                         _windowR3,
                         _windowHalfR1,
                         _windowHalfR2,
                         _windowHalfR3);

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

  _computeBeamAzRate();
  _computeBeamElRate();

  _mom->setAntennaRate(getAntennaRate());

}

//////////////////////////////////////
// initialize for KDP
  
void Beam::_kdpInit()

{

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

}

//////////////////////////////
// initialize FFTs
  
void Beam::_fftInit()

{

  // initialize the FFT objects on the threads
  // Note: the initialization is not thread safe so it 
  // must be protected by a mutex
  
  _fft->init(_nSamples);
  _fftHalf->init(_nSamplesHalf);
  int nStag =
    RadarMoments::computeNExpandedStagPrt(_nSamples, _stagM, _stagN);
  _fftStag->init(nStag);
  
}

////////////////////////////////
// initialize regression filter
  
void Beam::_regrInit()

{

  // initialize the regression filter objects
  
  _regr->setup(_nSamples,
               _params.regression_filter_determine_order_from_cnr,
               _params.regression_filter_specified_polynomial_order,
               _params.regression_filter_clutter_width_factor,
               _params.regression_filter_cnr_exponent,
               _wavelengthM);
  _regrHalf->setup(_nSamplesHalf,
                   _params.regression_filter_determine_order_from_cnr,
                   _params.regression_filter_specified_polynomial_order,
                   _params.regression_filter_clutter_width_factor,
                   _params.regression_filter_cnr_exponent,
                   _wavelengthM);
  _regrStag->setupStaggered(_nSamples, _stagM, _stagN,
                            _params.regression_filter_determine_order_from_cnr,
                            _params.regression_filter_specified_polynomial_order,
                            _params.regression_filter_clutter_width_factor,
                            _params.regression_filter_cnr_exponent,
                            _wavelengthM);
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
      GateData *gate = new GateData;
      _gateData.push_back(gate);
    }
  }

  for (size_t ii = 0; ii < _gateData.size(); ii++) {
    _gateData[ii]->allocArrays(_nSamples, true, _isStagPrt, true);
  }

  _outFields = _outFields_.alloc(nGates);
  _outFieldsF = _outFieldsF_.alloc(nGates);
  _compFields = _compFields_.alloc(nGates);
  _compFieldsF = _compFieldsF_.alloc(nGates);

  for (int ii = 0; ii < nGates; ii++) {
    _outFields[ii].init();
    _outFieldsF[ii].init();
    _compFields[ii].init();
    _compFieldsF[ii].init();
  }


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
    
    case IWRF_SINGLE_POL_V: {
    
      int nBytesComplex = _nSamples * sizeof(RadarComplex_t);
      for (int igate = 0, ipos = 0; igate < _nGates; igate++, ipos += 2) {
        GateData *gate = _gateData[igate];
        // original data
        RadarComplex_t *iqvc = gate->iqvcOrig;
        for (int isamp = 0; isamp < _nSamples; isamp++, iqvc++) {
          iqvc->re = iqChan0[isamp][ipos];
          iqvc->im = iqChan0[isamp][ipos + 1];
        }
        // windowed data
        memcpy(gate->iqvc, gate->iqvcOrig, nBytesComplex);
        RadarMoments::applyWindow(gate->iqvc, _window, _nSamples);
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
  
  _nyquistPrtShort = ((_wavelengthM / _prtShort) / 4.0);
  _nyquistPrtLong = ((_wavelengthM / _prtLong) / 4.0);
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
    _outFields[ii] = _gateData[ii]->fields;
    _outFieldsF[ii] = _gateData[ii]->fieldsF;
  }
  
}

/////////////////////////////////////////////////
// check that the baseDbz1km values are set

int Beam::_checkCalib()
  
{
  
  int iret = 0;

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

///////////////////////////////////////////////////////////
// Perform clutter filtering

void Beam::_performClutterFiltering()

{

  // copy the unfiltered fields to the filtered fields
  
  for (int igate = 0; igate < _nGates; igate++) {
    _gateData[igate]->fieldsF = _gateData[igate]->fields;
  }
  
  // filter clutter from moments
  
  _filterMoments();

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

////////////////////////////////////////////////////////////////
// compute azimuth rate for a beam

void Beam::_computeBeamAzRate()

{

  const IwrfTsPulse *pulseStart = _pulses[0];
  const IwrfTsPulse *pulseEnd = _pulses[_pulses.size() - 1];
  
  double azStart = pulseStart->getAz();
  double azEnd = pulseEnd->getAz();

  double deltaAz = azEnd - azStart;
  if (deltaAz > 180) {
    deltaAz -= 360;
  } else if (deltaAz < -180) {
    deltaAz += 360;
  }
  
  double timeStart = pulseStart->getFTime();
  double timeEnd = pulseEnd->getFTime();
  double deltaTime = timeEnd - timeStart;
  
  if (deltaTime <= 0) {
    _beamAzRate = 0.0;
  } else {
    _beamAzRate = deltaAz / deltaTime;
  }
  
}

////////////////////////////////////////////////////////////////
// compute elevation rate for a beam

void Beam::_computeBeamElRate()

{
  
  const IwrfTsPulse *pulseStart = _pulses[0];
  const IwrfTsPulse *pulseEnd = _pulses[_pulses.size() - 1];
  
  double elStart = pulseStart->getEl();
  double elEnd = pulseEnd->getEl();

  double deltaEl = elEnd - elStart;
  if (deltaEl > 180) {
    deltaEl -= 360;
  } else if (deltaEl < -180) {
    deltaEl += 360;
  }
  
  double timeStart = pulseStart->getFTime();
  double timeEnd = pulseEnd->getFTime();
  double deltaTime = timeEnd - timeStart;
  
  if (deltaTime <= 0) {
    _beamElRate = 0.0;
  } else {
    _beamElRate = deltaEl / deltaTime;
  }

}

///////////////////////////////////////////
// get antenna rate for scan type

double Beam::getAntennaRate()

{
  
  double rate = 0.0;
  if (_isRhi) {
    rate = _beamElRate;
  } else {
    rate = _beamAzRate;
  }
  return rate;
  
}

///////////////////////////////////////////
// get regression order

int Beam::getRegrOrder()

{
  return _regr->getPolyOrder();
}

/////////////////////////////////////////////////////////////////
// Compute phase differences between this pulse and previous ones
// to be able to cohere to multiple trips
//
// Before this method is called, this pulse should be added to
// the queue.

void Beam::_computePhaseDiffs
  (const deque<const IwrfTsPulse *> &pulseQueue, int maxTrips)
  
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

/////////////////////////////////////////////////////////////////
// load up time series in dwell spectra

void Beam::_loadDwellSpectra()
  
{

  // dimensions
  
  _spectra.setDimensions(_nGates, _nSamples);

  // metadata

  _spectra.setTime(_timeSecs, _nanoSecs);
  _spectra.setElevation(_el);
  _spectra.setAzimuth(_az);
  _spectra.setAntennaRate(getAntennaRate());

  _spectra.setRangeGeometry(_startRangeKm, _gateSpacingKm);
  _spectra.setXmitRcvMode(_xmitRcvMode);
  _spectra.setPrt(_prt);
  _spectra.setNyquist(_nyquist);
  _spectra.setPulseWidthUs(_pulseWidth);
  _spectra.setWavelengthM(_wavelengthM);

  // windowing

  _spectra.setWindow(_window, _nSamples);

  // set time series

  _spectra.prepareForData();

  for (size_t igate = 0; igate < _gateData.size(); igate++) {

    GateData *gd = _gateData[igate];

    if (gd->iqhcOrig != NULL) {
      _spectra.setIqHc(gd->iqhcOrig, igate, _nSamples);
    }
    
    if (gd->iqvcOrig != NULL) {
      _spectra.setIqVc(gd->iqvcOrig, igate, _nSamples);
    }
    
    if (gd->iqhxOrig != NULL) {
      _spectra.setIqHx(gd->iqhxOrig, igate, _nSamples);
    }
    
    if (gd->iqvxOrig != NULL) {
      _spectra.setIqVx(gd->iqvxOrig, igate, _nSamples);
    }
    
  } // igate

  // compute spectra

  _spectra.computePowerSpectra();
  _spectra.computeDbzSpectra();
  _spectra.computeZdrSpectra();
  _spectra.computePhidpRhohvSpectra();
  _spectra.computeTdbz();
  _spectra.computeZdrSdev();
  _spectra.computePhidpSdev();
  
  _spectra.computeSpectralCmd();

}

