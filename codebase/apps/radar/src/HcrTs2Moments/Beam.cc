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
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
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
#include <Spdb/DsSpdb.hh>
#include "Beam.hh"
#include "EgmCorrection.hh"
using namespace std;

const double Beam::_missingDbl = MomentsFields::missingDouble;
int Beam::_nWarnings = 0;
pthread_mutex_t Beam::_fftMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t Beam::_pulseUnpackMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t Beam::_debugPrintMutex = PTHREAD_MUTEX_INITIALIZER;

////////////////////////////////////////////////////
// Constructor

Beam::Beam(const string &progName,
	   const Params &params) :
        _progName(progName),
        _params(params)
        
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
  _scanType = SCAN_TYPE_PPI;

  _startRangeKm = 0.0;
  _gateSpacingKm = 0.0;

  _xmitRcvMode = IWRF_SINGLE_POL;

  _prt = 0;
  _nyquist = 0;
  _pulseWidth = 0;

  _fields = NULL;

  _mom = NULL;
  _noise = NULL;

  _window = NULL;
  _windowHalf = NULL;
  _windowVonHann = NULL;

  _windowR1 = 0;
  _windowR2 = 0;
  _windowR3 = 0;
  _windowHalfR1 = 0;
  _windowHalfR2 = 0;
  _windowHalfR3 = 0;

  _fft = new RadarFft;
  _fftHalf = new RadarFft;

}

////////////////////////////////////////////////////
// Initialize before use

void Beam::init(int nSamples,
                int nGates,
                double prt,
                iwrf_xmit_rcv_mode_t xmitRcvMode,
                const IwrfTsInfo &opsInfo,
                const vector<shared_ptr<IwrfTsPulse>> &pulses)
  
{

  // initialize
  
  _nSamples = nSamples;
  _nSamplesHalf = _nSamples / 2;
  _nGates = nGates;
  _prt = prt;
  _xmitRcvMode = xmitRcvMode;
  _opsInfo = opsInfo;
  _wavelengthM = _opsInfo.get_radar_wavelength_cm() / 100.0;
  _pulses = pulses;
  _georefActive = false;

  // set time

  shared_ptr<IwrfTsPulse> midPulse = _pulses[_nSamplesHalf];
  _timeSecs = (time_t) midPulse->getTime();
  _nanoSecs = midPulse->getNanoSecs();
  _dtime = midPulse->getFTime();

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

  if (_fields) {
    delete[] _fields;
  }

  if (_mom) {
    delete _mom;
  }
  
  if (_noise) {
    delete _noise;
  }
  
  if (_fft) {
    delete _fft;
    _fft = NULL;
  }
  if (_fftHalf) {
    delete _fftHalf;
    _fftHalf = NULL;
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

  if (_windowVonHann) {
    delete[] _windowVonHann;
    _windowVonHann = NULL;
  }

}
  
////////////////////////////////////////////////////
// Prepare for moments computations

void Beam::_prepareForComputeMoments()

{

  // initialize
  
  _nSamplesHalf = _nSamples / 2;

  double wavelengthMeters = _opsInfo.get_radar_wavelength_cm() / 100.0;
  _nyquist = ((wavelengthMeters / _prt) / 4.0);

  // range geometry

  _startRangeKm = _opsInfo.get_proc_start_range_km();
  _gateSpacingKm = _opsInfo.get_proc_gate_spacing_km();

  // unpack the pulses as needed

  pthread_mutex_lock(&_pulseUnpackMutex);
  for (size_t ii = 0; ii < _pulses.size(); ii++) {
    _pulses[ii]->convertToFL32();
  }
  pthread_mutex_unlock(&_pulseUnpackMutex);

  // override OpsInfo time-series values as needed
  
  _overrideOpsInfo();

  // pulse width
  
  shared_ptr<IwrfTsPulse> midPulse = _pulses[_nSamplesHalf];
  _pulseWidth = midPulse->getPulseWidthUs() / 1.0e6;

  // set time

  _timeSecs = (time_t) midPulse->getTime();
  _nanoSecs = midPulse->getNanoSecs();
  _dtime = midPulse->getFTime();

  // select the georeference from the mid pulse

  if (midPulse->getGeorefActive()) {
    _georef = midPulse->getPlatformGeoref();
    _georefActive = true;
    if (_params.correct_altitude_for_egm) {
      _correctAltitudeForEgm();
    }
    if (_params.georef_fixed_location_mode) {
      _setGeorefForFixedMode();
    }
  }

  // set elevation / azimuth

  if (_scanType == SCAN_TYPE_VERT) {
    _el = _conditionEl(_meanPointingAngle);
    _az = _conditionAz(midPulse->getAz());
  } else if (_scanType == SCAN_TYPE_RHI) {
    _el = _conditionEl(_meanPointingAngle);
    _az = _conditionAz(midPulse->getAz());
  } else {
    _az = _conditionAz(_meanPointingAngle);
    _el = _conditionEl(midPulse->getEl());
  }
  if (midPulse->getFixedEl() > -990) {
    _targetEl = _conditionEl(midPulse->getFixedEl());
  } else {
    _targetEl = _meanPointingAngle;
  }
  if (midPulse->getFixedAz() > -990) {
    _targetAz = _conditionAz(midPulse->getFixedAz());
  } else {
    _targetAz = _meanPointingAngle;
  }

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
  
  // compute number of output gates
  
  _nGatesOut = _nGates;

  // ffts and regression filter

  // initialize the FFT objects on the threads
  // Note: the initialization is not thread safe so it 
  // must be protected by a mutex

  pthread_mutex_lock(&_fftMutex);

  _fft->init(_nSamples);
  _fftHalf->init(_nSamplesHalf);
  
  pthread_mutex_unlock(&_fftMutex);

  // alloc fields at each gate

  if (_nGatesOut > _nGatesOutAlloc) {

    // delete old moments objects
    
    if (_fields) delete[] _fields;
    if (_mom) delete _mom;

    // create new moments object
    
    _fields = new MomentsFields[_nGatesOut];

    _mom = new RadarMoments(_nGatesOut);

    _nGatesOutAlloc = _nGatesOut;

  }

  // initialize fields

  for (int ii = 0; ii < _nGatesOut; ii++) {
    _fields[ii].init();
  }

  // initialize moments computations objects
  
  _initMomentsObject(_mom);
  
  _mom->init(_prt, _opsInfo);
  
  // compute windows for FFTs

  _computeWindows();

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
  
  _allocGateData(_nGates);
  _initFieldData();
  if (_params.swap_receiver_channels) {
    _loadGateIq(iqChan1, iqChan0);
  } else {
    _loadGateIq(iqChan0, iqChan1);
  }

  // free up pulses for use by other threads

  _pulses.clear();

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
    
void Beam::computeMoments()
  
{

  _prepareForComputeMoments();

  // set calibration data on Moments object, ready for computations
  
  _mom->setCalib(_calib);

  // compute the moments
  
  _computeMoments();

  // compute corrected velocity
  
  if (_params.compute_velocity_corrected_for_platform_motion) {
    _computeVelocityCorrectedForMotion();
  }
  if (_params.compute_width_corrected_for_platform_motion) {
    _computeWidthCorrectedForMotion();
  }

  // applyMedianFilter as appropriate
  
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

void Beam::appendStatusXml(const string &extraXml)

{

  _statusXml += extraXml;

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

double Beam::getMaxRangeKm() const

{
  double maxRange = _opsInfo.get_proc_start_range_km() +
    _nGatesOut * _opsInfo.get_proc_gate_spacing_km();
  return maxRange;
}

////////////////////////////////////////////////
// get unambiguous range

double Beam::getUnambigRangeKm() const

{
  if (_mom == NULL) {
    return _missingDbl;
  }
  return _mom->getUnambigRangeKm();
}

///////////////////////////////////////////////////////////
// Compute moments at each gate

void Beam::_computeMoments()

{

  switch (_xmitRcvMode) {

    case IWRF_H_ONLY_FIXED_HV:
      _computeMomDpHOnly();
      break;
      
    case IWRF_V_ONLY_FIXED_HV:
    default:
      _computeMomDpVOnly();
      break;
      
  }

}

///////////////////////////////////////////////////////////
// Compute moments DP_H_ONLY_FIXED_HV
// H transmission, fixed dual receive

void Beam::_computeMomDpHOnly()
{

  // copy gate fields to _momFields array

  for (int igate = 0; igate < _nGates; igate++) {
    _momFields[igate] = _gateData[igate]->fields;
  }

  // compute covariances and prepare for noise comps
  
  for (int igate = 0; igate < _nGates; igate++) {
    GateData *gate = _gateData[igate];
    MomentsFields &fields = _momFields[igate];
    _mom->computeCovarDpHOnly(gate->iqhc, gate->iqvx, fields);
    _mom->dpHOnlyNoisePrep(igate,
                           fields.lag0_hc,
                           fields.lag0_vx,
                           fields.lag1_hc,
                           fields);
  }
  
  // identify noise regions, and compute the mean noise
  // mean noise values are stored in moments
  
  _noise->computeNoiseDpHOnly(_momFields);
  _noise->addToMoments(_momFields);
  
  // override noise for moments computations
  
  if (_params.use_estimated_noise_for_noise_subtraction) {
    if (_noise->getNoiseBiasDbHc() < _params.max_valid_noise_bias_db) {
      _mom->setEstimatedNoiseDbmHc(_noise->getMedianNoiseDbmHc());
      _mom->setEstimatedNoiseDbmVx(_noise->getMedianNoiseDbmVx());
    }
  }
    
  for (int igate = 0; igate < _nGates; igate++) {
      
    MomentsFields &fields = _momFields[igate];
      
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
    _gateData[igate]->fields = _momFields[igate];
  }

}

///////////////////////////////////////////////////////////
// Compute moments DP_V_ONLY_FIXED_HV
// V transmission, fixed dual receive

void Beam::_computeMomDpVOnly()
{

  // copy gate fields to _momFields array

  for (int igate = 0; igate < _nGates; igate++) {
    _momFields[igate] = _gateData[igate]->fields;
  }

  // compute covariances and prepare for noise comps
  
  for (int igate = 0; igate < _nGates; igate++) {
    GateData *gate = _gateData[igate];
    MomentsFields &fields = _momFields[igate];
    _mom->computeCovarDpVOnly(gate->iqvc, gate->iqhx, fields);
    _mom->dpVOnlyNoisePrep(igate,
                           fields.lag0_vc,
                           fields.lag0_hx,
                           fields.lag1_vc,
                           fields);
  }
  
  // identify noise regions, and compute the mean noise
  // mean noise values are stored in moments
  
  _noise->computeNoiseDpVOnly(_momFields);
  _noise->addToMoments(_momFields);
  
  // override noise for moments computations
  
  if (_params.use_estimated_noise_for_noise_subtraction) {
    if (_noise->getNoiseBiasDbVc() < _params.max_valid_noise_bias_db) {
      _mom->setEstimatedNoiseDbmVc(_noise->getMedianNoiseDbmVc());
      _mom->setEstimatedNoiseDbmHx(_noise->getMedianNoiseDbmHx());
    }
  }
    
  for (int igate = 0; igate < _nGates; igate++) {
      
    MomentsFields &fields = _momFields[igate];
      
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
    _gateData[igate]->fields = _momFields[igate];
  }

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
  if (_params.override_radar_location || _params.georef_fixed_location_mode) {
    _opsInfo.overrideRadarLocation(_params.radar_altitude_meters,
				   _params.radar_latitude_deg,
				   _params.radar_longitude_deg);
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

  if (_params.window_type == Params::WINDOW_RECT) {
    _window = RadarMoments::createWindowRect(_nSamples);
    _windowHalf = RadarMoments::createWindowRect(_nSamplesHalf);
  } else if (_params.window_type == Params::WINDOW_VONHANN) {
    _window = RadarMoments::createWindowVonhann(_nSamples);
    _windowHalf = RadarMoments::createWindowVonhann(_nSamplesHalf);
  } else if (_params.window_type == Params::WINDOW_BLACKMAN) {
    _window = RadarMoments::createWindowBlackman(_nSamples);
    _windowHalf = RadarMoments::createWindowBlackman(_nSamplesHalf);
  } else if (_params.window_type == Params::WINDOW_BLACKMAN_NUTTALL) {
    _window = RadarMoments::createWindowBlackmanNuttall(_nSamples);
    _windowHalf = RadarMoments::createWindowBlackmanNuttall(_nSamplesHalf);
  }

  _windowVonHann = RadarMoments::createWindowVonhann(_nSamples);

  // compute window R values, used for corrections in Spectrum width

  _computeWindowRValues();

}


//////////////////////////////////////
// initialize moments computations object
  
void Beam::_initMomentsObject(RadarMoments *mom)
  
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    mom->setDebug(true);
  }

  if (_params.spectrum_width_method == Params::WIDTH_METHOD_R0R1) {
    mom->setSpectrumWidthMethod(RadarMoments::WIDTH_METHOD_R0R1);
  } else if (_params.spectrum_width_method == Params::WIDTH_METHOD_R1R2) {
    mom->setSpectrumWidthMethod(RadarMoments::WIDTH_METHOD_R1R2);
  } else if (_params.spectrum_width_method == Params::WIDTH_METHOD_HYBRID) {
    mom->setSpectrumWidthMethod(RadarMoments::WIDTH_METHOD_HYBRID);
  }

  if (_params.threshold_ldr_using_snr) {
    mom->setMinSnrDbForLdr(_params.min_snr_db_for_ldr);
  }

  mom->setNSamples(_nSamples);

  if (_params.change_velocity_sign) {
    mom->setChangeVelocitySign(true);
  } else {
    mom->setChangeVelocitySign(false);
  }
  
  mom->setWindowRValues(_windowR1,
                        _windowR2,
                        _windowR3,
                        _windowHalfR1,
                        _windowHalfR2,
                        _windowHalfR3);

}

///////////////////////////////////////////////////////////
// compute velocity corrected for platform motion
//
// NOTES from Ulrike's Matlab code
//  
// % Compute y_t following equation 9 Lee et al (1994)
// y_subt=-cosd(data.rotation+data.roll).*cosd(data.drift).*cosd(data.tilt).*sind(data.pitch)...
//     +sind(data.drift).*sind(data.rotation+data.roll).*cosd(data.tilt)...
//     +cosd(data.pitch).*cosd(data.drift).*sind(data.tilt);
//
// % Compute z following equation 9 Lee et al (1994)
// z=cosd(data.pitch).*cosd(data.tilt).*cosd(data.rotation+data.roll)+sind(data.pitch).*sind(data.tilt);
//
// % compute tau_t following equation 11 Lee et al (1994)
// tau_subt=asind(y_subt);
//
// % Compute phi following equation 17 Lee et al (1994)
// phi=asind(z);
//
// % Compute platform motion based on Eq 27 from Lee et al (1994)
// ground_speed=sqrt(data.eastward_velocity.^2 + data.northward_velocity.^2);
// % Use this equation when starting from VEL_RAW
// %vr_platform=-ground_speed.*sin(tau_subt)-vertical_velocity.*sin(phi);
// % Use this equation when starting from VEL
// vr_platform=-ground_speed.*sind(tau_subt).*sind(phi);
//
// velAngCorr=data.VEL+vr_platform;

void Beam::_computeVelocityCorrectedForMotion()

{

  // no good if no georeference available

  if (!_georefActive) {
    _copyVelToCorrectedVel();
    return;
  }

  // pre-compute sin / cosine

  double cosEl, sinEl;
  ta_sincos(_el * DEG_TO_RAD, &sinEl, &cosEl);

  double cosPitch, sinPitch;
  ta_sincos(_georef.pitch_deg * DEG_TO_RAD, &sinPitch, &cosPitch);
  
  double cosRoll, sinRoll;
  ta_sincos(_georef.roll_deg * DEG_TO_RAD, &sinRoll, &cosRoll);
  
  double cosTilt, sinTilt;
  ta_sincos(_georef.tilt_deg * DEG_TO_RAD, &sinTilt, &cosTilt);
  
  double cosDrift, sinDrift;
  ta_sincos(_georef.drift_angle_deg * DEG_TO_RAD, &sinDrift, &cosDrift);
  
  double cosRotRoll, sinRotRoll;
  double rotPlusRoll = _georef.rotation_angle_deg + _georef.roll_deg;
  ta_sincos(rotPlusRoll * DEG_TO_RAD, &sinRotRoll, &cosRotRoll);

  // compute the vel correction from horiz platform motion, including drift

  // Compute y_t following equation 9 Lee et al (1994)

  double y_subt = ((-cosRotRoll * cosDrift * cosTilt * sinPitch) +
                   (sinDrift * sinRotRoll * cosTilt) +
                   (cosPitch * cosDrift * sinTilt));

  // Compute z following equation 9 Lee et al (1994)

  double zz = cosPitch * cosTilt * cosRotRoll + sinPitch * sinTilt;

  // compute tau_t following equation 11 Lee et al (1994) (radians)

  // double tau_subt = asin(y_subt);

  // Compute phi following equation 17 Lee et al (1994) (radians)

  // double phi = asin(zz);

  // Compute ground speed based on Eq 27 from Lee et al (1994)

  double ewVel = _georef.ew_velocity_mps;
  double nsVel = _georef.ns_velocity_mps;
  double ground_speed = sqrt(ewVel * ewVel + nsVel * nsVel);

  // compute the vert vel correction

  double vertCorr = 0.0;
  if (_georef.vert_velocity_mps > -9990) {
    vertCorr = _georef.vert_velocity_mps * zz;
  }

  // compute the horiz vel correction

  double horizCorr = ground_speed * y_subt;

  // check

  if (vertCorr == 0.0 && horizCorr == 0.0) {
    // no change needed
    _copyVelToCorrectedVel();
    return;
  }

  for (int ii = 0; ii < _nGates; ii++) {
    
    double vel = _gateData[ii]->fields.vel;
    if (vel != _missingDbl) {
      double velCorrVert = _correctForNyquist(vel + vertCorr);
      double velCorr = _correctForNyquist(vel + vertCorr + horizCorr);
      _gateData[ii]->fields.vel_corr_vert = velCorrVert;
      _gateData[ii]->fields.vel_corr_motion = velCorr;
    }

    double velF = _gateData[ii]->fieldsF.vel;
    if (velF != _missingDbl) {
      double velFCorrVert = _correctForNyquist(velF + vertCorr);
      double velFCorr = _correctForNyquist(velF + vertCorr + horizCorr);
      _gateData[ii]->fieldsF.vel_corr_vert = velFCorrVert;
      _gateData[ii]->fieldsF.vel_corr_motion = velFCorr;
    }
    
  } // ii

}

void Beam::_copyVelToCorrectedVel()

{

  for (int ii = 0; ii < _nGates; ii++) {

    double vel = _gateData[ii]->fields.vel;
    _gateData[ii]->fields.vel_corr_vert = vel;
    _gateData[ii]->fields.vel_corr_motion = vel;

    double velF = _gateData[ii]->fieldsF.vel;
    _gateData[ii]->fieldsF.vel_corr_vert = velF;
    _gateData[ii]->fieldsF.vel_corr_motion = velF;

  } // ii

}

///////////////////////////////////////////////////////////
// compute velocity corrected for vertical platform motion
  
void Beam::_computeVelocityCorrectedForVertMotion()

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
  
  if (vertCorr == 0.0) {
    // no change needed
    return;
  }
  
  for (int ii = 0; ii < _nGates; ii++) {
    
    double vel = _gateData[ii]->fields.vel;
    if (vel != _missingDbl) {
      double velCorr = _correctForNyquist(vel + vertCorr);
      _gateData[ii]->fields.vel_corr_vert = velCorr;
    }

    double velF = _gateData[ii]->fieldsF.vel;
    if (velF != _missingDbl) {
      double velFCorr = _correctForNyquist(velF + vertCorr);
      _gateData[ii]->fieldsF.vel_corr_vert = velFCorr;
    }

  } // ii

}

/////////////////////////////////////////////////
// correct velocity for nyquist
  
double Beam::_correctForNyquist(double vel)

{
  while (vel > _nyquist) {
    vel -= 2.0 * _nyquist;
  }
  while (vel < -_nyquist) {
    vel += 2.0 * _nyquist;
  }
  return vel;
}

/////////////////////////////////////////////////
// compute width corrected for platform motion
  
void Beam::_computeWidthCorrectedForMotion()

{

  // no good if no georeference available

  if (!_georefActive) {
    return;
  }

  // get the aircraft speed

  double ewVel = _georef.ew_velocity_mps;
  double nsVel = _georef.ns_velocity_mps;
  double speed = sqrt(ewVel * ewVel + nsVel * nsVel);

  // compute the delta correction
  
  double sinElev = sin(_el * DEG_TO_RAD);
  double delta =
    fabs(0.3 * speed * sinElev *
         (_params.width_correction_beamwidth_deg * DEG_TO_RAD));
  
  // compute the corrected width for each gate
  
  for (int ii = 0; ii < _nGates; ii++) {
    
    double width = _gateData[ii]->fields.width;
    if (width != _missingDbl) {
      double xx = width * width - delta * delta;
      if (xx < 0.01) {
        xx = 0.01;
      }
      _gateData[ii]->fields.width_corr_motion = sqrt(xx);
    }

    double widthF = _gateData[ii]->fieldsF.width;
    if (widthF != _missingDbl) {
      double xx = widthF * widthF - delta * delta;
      if (xx < 0.01) {
        xx = 0.01;
      }
      _gateData[ii]->fieldsF.width_corr_motion = sqrt(xx);
    }

  } // ii

}

//////////////////////////////////////
// initialize noise computations
  
int Beam::_noiseInit()
  
{

  if (_noise == NULL) {
    _noise = new NoiseLocator;
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
    _gateData[ii]->allocArrays(_nSamples, false, false, false);
  }
  _momFields = _momFields_.alloc(_gateData.size());
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

      int nBytesHalf = _nSamplesHalf * sizeof(RadarComplex_t);
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
        memcpy(gate->iqhc, gate->iqhcOrig, nBytesHalf);
        memcpy(gate->iqvc, gate->iqvcOrig, nBytesHalf);
        RadarMoments::applyWindow(gate->iqhc, _windowHalf, _nSamplesHalf);
        RadarMoments::applyWindow(gate->iqvc, _windowHalf, _nSamplesHalf);
      }
    
    } break;
        
    case IWRF_ALT_HV_CO_CROSS: {

      // assumes first pulse is H xmit
      
      int nBytesHalf = _nSamplesHalf * sizeof(RadarComplex_t);
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
        memcpy(gate->iqhc, gate->iqhcOrig, nBytesHalf);
        memcpy(gate->iqvc, gate->iqvcOrig, nBytesHalf);
        memcpy(gate->iqhx, gate->iqhxOrig, nBytesHalf);
        memcpy(gate->iqvx, gate->iqvxOrig, nBytesHalf);
        RadarMoments::applyWindow(gate->iqhc, _windowHalf, _nSamplesHalf);
        RadarMoments::applyWindow(gate->iqvc, _windowHalf, _nSamplesHalf);
        RadarMoments::applyWindow(gate->iqhx, _windowHalf, _nSamplesHalf);
        RadarMoments::applyWindow(gate->iqvx, _windowHalf, _nSamplesHalf);
      }

    } break;

    case IWRF_ALT_HV_FIXED_HV: {

      // not switching
      int nBytesHalf = _nSamplesHalf * sizeof(RadarComplex_t);
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
        memcpy(gate->iqhc, gate->iqhcOrig, nBytesHalf);
        memcpy(gate->iqvc, gate->iqvcOrig, nBytesHalf);
        memcpy(gate->iqhx, gate->iqhxOrig, nBytesHalf);
        memcpy(gate->iqvx, gate->iqvxOrig, nBytesHalf);
        RadarMoments::applyWindow(gate->iqhc, _windowHalf, _nSamplesHalf);
        RadarMoments::applyWindow(gate->iqvc, _windowHalf, _nSamplesHalf);
        RadarMoments::applyWindow(gate->iqhx, _windowHalf, _nSamplesHalf);
        RadarMoments::applyWindow(gate->iqvx, _windowHalf, _nSamplesHalf);
      }

    } break;

    case IWRF_SIM_HV_FIXED_HV: {

      int nBytesFull = _nSamples * sizeof(RadarComplex_t);
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
        memcpy(gate->iqhc, gate->iqhcOrig, nBytesFull);
        memcpy(gate->iqvc, gate->iqvcOrig, nBytesFull);
        RadarMoments::applyWindow(gate->iqhc, _window, _nSamples);
        RadarMoments::applyWindow(gate->iqvc, _window, _nSamples);
      }
      
    } break;

    case IWRF_SIM_HV_SWITCHED_HV: {

      int nBytesFull = _nSamples * sizeof(RadarComplex_t);
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
        memcpy(gate->iqhc, gate->iqhcOrig, nBytesFull);
        memcpy(gate->iqvc, gate->iqvcOrig, nBytesFull);
        RadarMoments::applyWindow(gate->iqhc, _window, _nSamples);
        RadarMoments::applyWindow(gate->iqvc, _window, _nSamples);
      }
      
    } break;

    case IWRF_H_ONLY_FIXED_HV: {

      int nBytesFull = _nSamples * sizeof(RadarComplex_t);
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
        memcpy(gate->iqhc, gate->iqhcOrig, nBytesFull);
        memcpy(gate->iqvx, gate->iqvxOrig, nBytesFull);
        RadarMoments::applyWindow(gate->iqhc, _window, _nSamples);
        RadarMoments::applyWindow(gate->iqvx, _window, _nSamples);
      }
    
    } break;
    
    case IWRF_V_ONLY_FIXED_HV: {

      int nBytesFull = _nSamples * sizeof(RadarComplex_t);
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
        memcpy(gate->iqhx, gate->iqhxOrig, nBytesFull);
        memcpy(gate->iqvc, gate->iqvcOrig, nBytesFull);
        RadarMoments::applyWindow(gate->iqhx, _window, _nSamples);
        RadarMoments::applyWindow(gate->iqvc, _window, _nSamples);
      }
    
    } break;

    case IWRF_SINGLE_POL:
    default: {
    
      int nBytesFull = _nSamples * sizeof(RadarComplex_t);
      for (int igate = 0, ipos = 0; igate < _nGates; igate++, ipos += 2) {
        GateData *gate = _gateData[igate];
        // original data
        RadarComplex_t *iqhc = gate->iqhcOrig;
        for (int isamp = 0; isamp < _nSamples; isamp++, iqhc++) {
          iqhc->re = iqChan0[isamp][ipos];
          iqhc->im = iqChan0[isamp][ipos + 1];
        }
        // windowed data
        memcpy(gate->iqhc, gate->iqhcOrig, nBytesFull);
        RadarMoments::applyWindow(gate->iqhc, _window, _nSamples);
      }

    } break;
    
    case IWRF_SINGLE_POL_V: {
    
      int nBytesFull = _nSamples * sizeof(RadarComplex_t);
      for (int igate = 0, ipos = 0; igate < _nGates; igate++, ipos += 2) {
        GateData *gate = _gateData[igate];
        // original data
        RadarComplex_t *iqvc = gate->iqvcOrig;
        for (int isamp = 0; isamp < _nSamples; isamp++, iqvc++) {
          iqvc->re = iqChan0[isamp][ipos];
          iqvc->im = iqChan0[isamp][ipos + 1];
        }
        // windowed data
        memcpy(gate->iqvc, gate->iqvcOrig, nBytesFull);
        RadarMoments::applyWindow(gate->iqvc, _window, _nSamples);
      }

    } break;
    
  } // switch;

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
  }
  
}

/////////////////////////////////////////////////
// check that the baseDbz1km values are set

int Beam::_checkCalib()
  
{
  
  int iret = 0;

  if (_params.correct_hcr_v_rx_gain_for_temperature) {
    if (_correctHcrVRxGainForTemp()) {
      iret = -1;
    }
  } else if (_params.correct_rx_gains_for_temperature) {
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
// reads the temperature from the XML status block,
// and then computes the temperature correction.

int Beam::_correctCalibGainsForTemp()
  
{

  // get temps for each channel, and gain correction
  
  double gainCorrectionHc = 0.0;
  double gainCorrectionVc = 0.0;
  double gainCorrectionHx = 0.0;
  double gainCorrectionVx = 0.0;

  double tempHc = NAN;
  double tempVc = NAN;
  double tempHx = NAN;
  double tempVx = NAN;
  
  for (int ii = 0; ii < _params.rx_temp_gain_corrections_n; ii++) {

    const Params::rx_temp_gain_correction_t &corr = 
      _params._rx_temp_gain_corrections[ii];

    switch (corr.rx_channel) {
      
      case Params::RX_CHANNEL_HC: {
        tempHc = _getTempFromTagList(corr.temp_tag_list_in_status_xml);
        if (!std::isnan(tempHc)) {
          gainCorrectionHc = corr.intercept + tempHc * corr.slope;
          if (_params.debug >= Params::DEBUG_VERBOSE) {
            cerr << "DEBUG: _correctCalibGainsForTemp()" << endl;
            cerr << "  Got tempHc from status XML: " << tempHc << endl;
            cerr << "  computed gainCorrectionHc: " << gainCorrectionHc << endl;
          }
        }
        break;
      }
        
      case Params::RX_CHANNEL_VC: {
        tempVc = _getTempFromTagList(corr.temp_tag_list_in_status_xml);
        if (!std::isnan(tempVc)) {
          gainCorrectionVc = corr.intercept + tempVc * corr.slope;
          if (_params.debug >= Params::DEBUG_VERBOSE) {
            cerr << "DEBUG: _correctCalibGainsForTemp()" << endl;
            cerr << "  Got tempVc from status XML: " << tempVc << endl;
            cerr << "  computed gainCorrectionVc: " << gainCorrectionVc << endl;
          }
        }
        break;
      }

      case Params::RX_CHANNEL_HX: {
        tempHx = _getTempFromTagList(corr.temp_tag_list_in_status_xml);
        if (!std::isnan(tempHx)) {
          gainCorrectionHx = corr.intercept + tempHx * corr.slope;
          if (_params.debug >= Params::DEBUG_VERBOSE) {
            cerr << "DEBUG: _correctCalibGainsForTemp()" << endl;
            cerr << "  Got tempHx from status XML: " << tempHx << endl;
            cerr << "  computed gainCorrectionHx: " << gainCorrectionHx << endl;
          }
        }
        break;
      }

      case Params::RX_CHANNEL_VX: {
        tempVx = _getTempFromTagList(corr.temp_tag_list_in_status_xml);
        if (!std::isnan(tempVx)) {
          gainCorrectionVx = corr.intercept + tempVx * corr.slope;
          if (_params.debug >= Params::DEBUG_VERBOSE) {
            cerr << "DEBUG: _correctCalibGainsForTemp()" << endl;
            cerr << "  Got tempVx from status XML: " << tempVx << endl;
            cerr << "  computed gainCorrectionVx: " << gainCorrectionVx << endl;
          }
        }
        break;
      }

    } // switch

  } // ii

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "+++++++ CALIBRATION BEFORE TEMP CORRECTION +++++++++++++" << endl;
    _calib.print(cerr);
    cerr << "+++++++ END CALIBRATION BEFORE TEMP CORRECTION +++++++++" << endl;
  }
    
  // augment status xml in ops info

  string tempCorrXml;
  tempCorrXml += TaXml::writeStartTag("TempBasedRxGainCorrection", 0);
  tempCorrXml += TaXml::writeDouble("tempHc", 1, tempHc);
  tempCorrXml += TaXml::writeDouble("gainCorrectionHc", 1, gainCorrectionHc);
  tempCorrXml += TaXml::writeDouble("tempVc", 1, tempVc);
  tempCorrXml += TaXml::writeDouble("gainCorrectionVc", 1, gainCorrectionVc);
  tempCorrXml += TaXml::writeDouble("tempHx", 1, tempHx);
  tempCorrXml += TaXml::writeDouble("gainCorrectionHx", 1, gainCorrectionHx);
  tempCorrXml += TaXml::writeDouble("tempVx", 1, tempVx);
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
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "+++++++ CALIBRATION AFTER TEMP CORRECTION +++++++++++++" << endl;
    _calib.print(cerr);
    cerr << "+++++++ END CALIBRATION AFTER TEMP CORRECTION +++++++++" << endl;
  }
  
  return 0;
  
}

////////////////////////////////////////////////////////
// Correct HCR V RX calibration gains for temperature
// the reads in the correction from SPDB

int Beam::_correctHcrVRxGainForTemp()
  
{
  
  // get gain correction data from SPDB

  DsSpdb spdb;
  if (spdb.getClosest(_params.hcr_delta_gain_spdb_url,
                      _timeSecs,
                      _params.hcr_delta_gain_search_margin_secs)) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "WARNING - Beam::_correctHcrVRxGainForTemp()" << endl;
      cerr << "  Cannot get delta gain from URL: "
           << _params.hcr_delta_gain_spdb_url << endl;
      cerr << "  Search time: " << DateTime::strm(_timeSecs) << endl;
      cerr << "  Search margin (secs): "
           << _params.hcr_delta_gain_search_margin_secs << endl;
      cerr << spdb.getErrStr() << endl;
    }
    return -1;
  }
  
  // got chunks
  
  const vector<Spdb::chunk_t> &chunks = spdb.getChunks();
  if (chunks.size() < 1) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "ERROR - Beam::_correctHcrVRxGainForTemp()" << endl;
      cerr << "  No suitable gain data from URL: "
           << _params.hcr_delta_gain_spdb_url << endl;
      cerr << "  Search time: " << DateTime::strm(_timeSecs) << endl;
      cerr << "  Search margin (secs): "
           << _params.hcr_delta_gain_search_margin_secs << endl;
    }
    return -1;
  }
  const Spdb::chunk_t &chunk = chunks[0];

  // get xml string with gain results
  
  string deltaGainXml((char *) chunk.data, chunk.len - 1);

  // find delta gain in xml
  
  double deltaGainVc =
    _getDeltaGainFromXml(deltaGainXml, _params.hcr_v_rx_delta_gain_tag_list);
  
  if (std::isnan(deltaGainVc)) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "WARNING - Beam::_correctHcrVRxGainForTemp()" << endl;
      cerr << "  Cannot find deltaGain in XML: " << deltaGainXml << endl;
    }
    return -1;
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "++++ CALIBRATION BEFORE HCR GAIN CORRECTION +++++++++++++" << endl;
    _calib.print(cerr);
    cerr << "++++ END CALIBRATION BEFORE HCR GAIN TEMP CORRECTION ++++" << endl;
    cerr << "Delta gain XML - created by HcrTempRxGain app" << endl;
    cerr << deltaGainXml << endl;
    cerr << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
  }
  
  // augment status xml in ops info

  _statusXml += deltaGainXml;
  
  // compute base dbz if needed
  
  if (!_calib.isMissing(_calib.getReceiverGainDbVc())) {
    double rconst = _calib.getRadarConstV();
    double noise = _calib.getNoiseDbmVc();
    double noiseFixed = noise + deltaGainVc;
    double gain = _calib.getReceiverGainDbVc();
    double gainFixed = gain + deltaGainVc;
    double baseDbz1km = noiseFixed - gainFixed + rconst;
    if (!_calib.isMissing(noise) && !_calib.isMissing(rconst)) {
      _calib.setBaseDbz1kmVc(baseDbz1km);
    }
    _calib.setReceiverGainDbVc(gainFixed);
    _calib.setNoiseDbmVc(noiseFixed);
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "++++ CALIBRATION AFTER HCR GAIN CORRECTION +++++++++++++" << endl;
    _calib.print(cerr);
    cerr << "++++ END CALIBRATION AFTER HCR GAIN TEMP CORRECTION ++++" << endl;
  }
    
  return 0;
  
}

/////////////////////////////////////////////////////////////////
// get temperature from status xml, given the tag list
// returns temp on success, NAN on failure

double Beam::_getTempFromTagList(const string &tagList) const
  
{

  // get tags in list
    
  vector<string> tags;
  TaStr::tokenize(tagList, "<>", tags);
  if (tags.size() == 0) {
    // no tags
    cerr << "WARNING - Beam::_getTempFromTagList()" << endl;
    cerr << "  Temp tag not found: " << tagList << endl;
    return NAN;
  }
  
  // read through the outer tags in status XML
  
  string buf(_statusXml);
  for (size_t jj = 0; jj < tags.size(); jj++) {
    string val;
    if (TaXml::readString(buf, tags[jj], val)) {
      cerr << "WARNING - Beam::_getTempFromTagList()" << endl;
      cerr << "  Bad tags found in status xml, expecting: "
           << tagList << endl;
      return NAN;
    }
    buf = val;
  }

  // read temperature
  
  double temp = NAN;
  if (TaXml::readDouble(buf, temp)) {
    cerr << "WARNING - Beam::_getTempFromTagList()" << endl;
    cerr << "  Bad temp found in status xml, buf: " << buf << endl;
    return NAN;
  }
  
  return temp;

}
  
/////////////////////////////////////////////////////////////////
// get delta gain from XML string, given the tag list
// returns delta gain, NAN on failure

double Beam::_getDeltaGainFromXml(const string &xml,
                                  const string &tagList) const
  
{
  
  // get tags in list
  
  vector<string> tags;
  TaStr::tokenize(tagList, "<>", tags);
  if (tags.size() == 0) {
    // no tags
    cerr << "WARNING - Beam::_getDeltaGainFromXml()" << endl;
    cerr << "  deltaGain tag not found: " << tagList << endl;
    return NAN;
  }
  
  // read through the outer tags in status XML
  
  string buf(xml);
  for (size_t jj = 0; jj < tags.size(); jj++) {
    string val;
    if (TaXml::readString(buf, tags[jj], val)) {
      cerr << "WARNING - Beam::_getDeltaGainFromXml()" << endl;
      cerr << "  Bad tags found in status xml, expecting: "
           << tagList << endl;
      return NAN;
    }
    buf = val;
  }

  // read delta gain
  
  double deltaGain = NAN;
  if (TaXml::readDouble(buf, deltaGain)) {
    cerr << "WARNING - Beam::_getDeltaGainFromXml()" << endl;
    cerr << "  Bad deltaGain found in status xml, buf: " << buf << endl;
    return NAN;
  }
  
  return deltaGain;

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

///////////////////////////////
// set the noise fields

void Beam::_setNoiseFields()
  
{
  
  const vector<bool> &noiseFlag = _noise->getNoiseFlag();
  const vector<double> &noiseInterest = _noise->getNoiseInterest();
  const vector<bool> &signalFlag = _noise->getSignalFlag();
  const vector<double> &signalInterest = _noise->getSignalInterest();
  const vector<double> &accumPhaseChange = _noise->getAccumPhaseChange();
  const vector<double> &phaseChangeError = _noise->getPhaseChangeError();
  const vector<double> &dbmSdev = _noise->getDbmSdev();
  const vector<double> &ncpMean = _noise->getNcpMean();
  
  size_t nGatesNoise = _nGates;

  if (noiseFlag.size() < nGatesNoise) {
    return;
  }

  for (size_t igate = 0; igate < nGatesNoise; igate++) {
    GateData *gate = _gateData[igate];
    MomentsFields &fields = gate->fields;
    fields.noise_flag = noiseFlag[igate];
    fields.signal_flag = signalFlag[igate];
    fields.noise_accum_phase_change = accumPhaseChange[igate];
    fields.noise_phase_change_error = phaseChangeError[igate];
    fields.noise_dbm_sdev = dbmSdev[igate];
    fields.noise_ncp_mean = ncpMean[igate];
    fields.noise_interest = noiseInterest[igate];
    fields.signal_interest = signalInterest[igate];
  }
  
}

///////////////////////////////
// censor gates based on noise

void Beam::_censorByNoiseFlag()

{

  size_t nGatesCensor = _nGates;
  
  for (size_t igate = 0; igate < nGatesCensor; igate++) {
    GateData *gate = _gateData[igate];
    MomentsFields &fields = gate->fields;
    if (fields.noise_flag) {
      _censorFlags[igate] = true;
    } else {
      _censorFlags[igate] = false;
    }
  }

  // fill in gaps in censor flag

  _fillInCensoring(nGatesCensor);

  // censor the fields
  
  for (size_t igate = 0; igate < nGatesCensor; igate++) {
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

  size_t nGatesCensor = _nGates;
  
  // load up censor flag from snr and ncp

  double snrThreshold = _params.censoring_snr_threshold;
  double ncpThreshold = _params.censoring_ncp_threshold;

  for (size_t igate = 0; igate < nGatesCensor; igate++) {
    GateData *gate = _gateData[igate];
    MomentsFields &fields = gate->fields;
    if (fields.snr < snrThreshold &&
        fields.ncp < ncpThreshold) {
      _censorFlags[igate] = true;
    } else {
      _censorFlags[igate] = false;
    }
  }

  // check that uncensored runs meet the minimum length
  // those which do not are censored
  
  int minValidRun = _params.censoring_min_valid_run;
  if (minValidRun > 1) {
    int runLength = 0;
    bool doCheck = false;
    for (int igate = 0; igate < (int) nGatesCensor; igate++) {
      if (_censorFlags[igate] == false) {
        doCheck = false;
        runLength++;
      } else {
        doCheck = true;
      }
      // last gate?
      if (igate == (int) nGatesCensor - 1) doCheck = true;
      // check run length
      if (doCheck) {
        if (runLength < minValidRun) {
          // clear the run which is too short
          for (int jgate = igate - runLength; jgate < igate; jgate++) {
            _censorFlags[jgate] = true;
          } // jgate
        }
        runLength = 0;
      } // if (doCheck ...
    } // igate
  }

  // fill in gaps in censor flag

  _fillInCensoring(nGatesCensor);

  // censor the fields
  
  for (size_t igate = 0; igate < nGatesCensor; igate++) {
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

void Beam::_fillInCensoring(size_t nGates)
  
{
  
  int *countSet = new int[nGates];
  int *countNot = new int[nGates];

  // compute the running count of gates which have the flag set and
  // those which do not

  // Go forward through the gates, counting up the number of gates set
  // or not set and assigning that number to the arrays as we go.

  int nSet = 0;
  int nNot = 0;
  for (size_t igate = 0; igate < nGates; igate++) {
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
  
  for (ssize_t igate = nGates - 2; igate >= 0; igate--) {
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

  for (ssize_t igate = 1; igate < (ssize_t) nGates - 1; igate++) {

    // is the gap small enough?

    nNot = countNot[igate];
    if (nNot > 0 && nNot <= 3) {

      // is it surrounded by regions at least as large as the gap?

      int minGateCheck = igate - nNot;
      if (minGateCheck < 0) {
        minGateCheck = 0;
      }
      int maxGateCheck = igate + nNot;
      if (maxGateCheck > (int) nGates - 1) {
        maxGateCheck = nGates - 1;
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

//////////////////////////////////////////////////////////////////
/// Condition az between 0 and 360

double Beam::_conditionAz(double az)
  
{
  
  while (az > 360.0) {
    az -= 360.0;
  }

  while (az < 0.0) {
    az += 360.0;
  }

  return az;

}

//////////////////////////////////////////////////////////////////
/// Condition the elevation between -180 and 180

double Beam::_conditionEl(double el)

{

  while (el > 180.0) {
    el -= 360.0;
  }

  while (el < -180.0) {
    el += 360.0;
  }

  return el;

}

//////////////////////////////////////////////////////////////////
/// Correct georef altitude for EGM
/// See:
///   https://earth-info.nga.mil/GandG/wgs84/gravitymod/egm2008/egm08_wgs84.html

void Beam::_correctAltitudeForEgm()

{

  EgmCorrection &egm = EgmCorrection::inst();
  double geoidM = egm.getGeoidM(_georef.latitude, _georef.longitude);
  double altCorrKm = (geoidM * -1.0) / 1000.0;
  _georef.altitude_msl_km += altCorrKm;

}

//////////////////////////////////////////////////////////////////
/// Set the georef information for fixed mode

void Beam::_setGeorefForFixedMode()

{

  _georef.latitude = _params.radar_latitude_deg;
  _georef.longitude = _params.radar_longitude_deg;
  _georef.altitude_msl_km = _params.radar_altitude_meters / 1000.0;

  _georef.ew_velocity_mps = 0.0;
  _georef.ns_velocity_mps = 0.0;
  _georef.vert_velocity_mps = 0.0;
  _georef.heading_deg = 0.0;
  _georef.track_deg = 0.0;
  _georef.drift_angle_deg = 0.0;
  _georef.ew_horiz_wind_mps = 0.0;
  _georef.ns_horiz_wind_mps = 0.0;
  _georef.vert_wind_mps = 0.0;

}

  
