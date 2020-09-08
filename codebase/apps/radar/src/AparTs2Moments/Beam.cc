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
// Aug 2019
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

const double Beam::_missingDbl = AparMomFields::missingDouble;
int Beam::_nWarnings = 0;
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

  _scanMode = static_cast<int>(apar_ts_scan_mode_t::PPI);
  _sweepNum = 0;
  _volNum = 0;
  _endOfSweepFlag = false;
  _endOfVolFlag = false;

  _scanType = SCAN_TYPE_PPI;

  _startRangeKm = 0.0;
  _gateSpacingKm = 0.0;

  _isAlternating = false;
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

  _fields = NULL;

  _mom = NULL;

  _needKdp = false;

}

////////////////////////////////////////////////////
// Initialize before use

void Beam::init(int nSamples,
                int nGates,
                int nGatesPrtLong,
                double elevation,
                double azimuth,
                scan_type_t scanType,
                bool isAlternating,
                bool isStagPrt,
                double prt,
                double prtLong,
                bool endOfSweepFlag,
                bool endOfVolFlag,
                const AtmosAtten &atmosAtten,
                const AparTsInfo &opsInfo,
                const vector<const AparTsPulse *> &pulses)
  
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "================ new beam ========================" << endl;
    cerr << "  nSamples: " << nSamples << endl;
    cerr << "  pulses.size: " << pulses.size() << endl;
    cerr << "  nGates: " << nGates << endl;
    cerr << "  nGatesPrtLong: " << nGatesPrtLong << endl;
    cerr << "  elevation: " << elevation << endl;
    cerr << "  azimuth: " << azimuth << endl;
    cerr << "  scanType: " << scanType << endl;
    cerr << "  isAlternating: " << isAlternating << endl;
    cerr << "  isStagPrt: " << isStagPrt << endl;
    cerr << "  prt: " << prt << endl;
    cerr << "  prtLong: " << prtLong << endl;
    cerr << "  endOfSweepFlag: " << endOfSweepFlag << endl;
    cerr << "  endOfVolFlag: " << endOfVolFlag << endl;
    cerr << "=================================================" << endl;
  }
    
  _nSamples = nSamples;
  _nGates = nGates;
  _nGatesPrtShort = nGates;
  _nGatesPrtLong = nGatesPrtLong;
  _el = elevation;
  _az = azimuth;
  _scanType = scanType;
  _isAlternating = isAlternating;
  _isStagPrt = isStagPrt;
  _prt = prt;
  _prtShort = prt;
  _prtLong = prtLong;
  _endOfSweepFlag = endOfSweepFlag;
  _endOfVolFlag = endOfVolFlag;
  _atmosAtten = &atmosAtten;
  _opsInfo = opsInfo;
  _pulses = pulses;
  _georefActive = false;

  _targetEl = _el;
  _targetAz = _az;

  // for each pulse, increase client count by 1,
  // so we can keep track of how many threads are using this pulse

  for (size_t ii = 0; ii < _pulses.size(); ii++) {
    _pulses[ii]->addClient();
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
  
  _snrArray_.free();
  _dbzArray_.free();
  _phidpArray_.free();
  
  _freeGateData();

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
    double wavelengthMeters = _opsInfo.getRadarWavelengthCm() / 100.0;
    _nyquist = ((wavelengthMeters / _prt) / 4.0);
  }

  // if (_xmitRcvMode == APAR_TS_SINGLE_POL ||
  //     _xmitRcvMode == APAR_TS_SINGLE_POL_V) {
  //   _dualPol = false;
  // } else {
  //   _dualPol = true;
  // }

  // _switchingReceiver = _mmgr.isSwitchingReceiver();
  // 111111111111111111111111 FIX THIS
  _switchingReceiver = true;

  // range geometry

  _startRangeKm = _opsInfo.getProcStartRangeKm();
  _gateSpacingKm = _opsInfo.getProcGateSpacingKm();

  // override OpsInfo time-series values as needed
  
  _overrideOpsInfo();

  // pulse width
  
  const AparTsPulse *midPulse = _pulses[_nSamplesHalf];
  _pulseWidth = midPulse->getPulseWidthUs() / 1.0e6;

  // set time

  _timeSecs = (time_t) midPulse->getTime();
  _nanoSecs = midPulse->getNanoSecs();
  _dtime = midPulse->getFTime();

  // select the georeference from the mid pulse

  if (midPulse->getGeorefActive()) {
    _georef = midPulse->getPlatformGeoref();
    _georefActive = true;
  }

  // scan details

  _scanMode = static_cast<int>(midPulse->getScanMode());
  _sweepNum = _getSweepNum();
  _volNum = _getVolNum();

  // compute number of output gates
  
  if (_isStagPrt) {
    _nGatesOut = _nGatesPrtLong;
  } else {
    _nGatesOut = _nGates;
  }

  // alloc fields at each gate

  if (_nGatesOut > _nGatesOutAlloc) {

    if (_fields) delete[] _fields;
    if (_mom) delete _mom;

    _fields = new AparMomFields[_nGatesOut];
    
    _mom = new AparMoments(_nGatesOut,
                           _params.debug >= Params::DEBUG_NORM,
                           _params.debug >= Params::DEBUG_VERBOSE);

    _nGatesOutAlloc = _nGatesOut;

  }

  // initialize fields

  for (int ii = 0; ii < _nGatesOut; ii++) {
    _fields[ii].init();
  }

  if (_params.compute_zdr_using_snr) {
    _mom->setComputeZdrUsingSnr(true);
  }

  if (_params.spectrum_width_method == Params::WIDTH_METHOD_R0R1) {
    _mom->setSpectrumWidthMethod(AparMoments::spectrum_width_method_t::R0R1);
  } else if (_params.spectrum_width_method == Params::WIDTH_METHOD_R1R2) {
    _mom->setSpectrumWidthMethod(AparMoments::spectrum_width_method_t::R1R2);
  } else if (_params.spectrum_width_method == Params::WIDTH_METHOD_HYBRID) {
    _mom->setSpectrumWidthMethod(AparMoments::spectrum_width_method_t::HYBRID);
  }

  if (_params.threshold_zdr_using_snr) {
    _mom->setMinSnrDbForZdr(_params.min_snr_db_for_zdr);
  }
  if (_params.threshold_ldr_using_snr) {
    _mom->setMinSnrDbForLdr(_params.min_snr_db_for_ldr);
  }

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
  } else {
    _allocGateData(_nGates);
    _initFieldData();
  }
  _loadGateIq(iqChan0, iqChan1);

}

////////////////////////////////////////////////////
// Get the volume number
// compute the median volume number for the beam

int Beam::_getVolNum()

{

  vector<int> volNums;
  for (size_t ii = 0; ii < _pulses.size(); ii++) {
    volNums.push_back(_pulses[ii]->getVolumeNum());
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
    sweepNums.push_back(_pulses[ii]->getSweepNum());
  }
  sort(sweepNums.begin(), sweepNums.end());

  if (_nSamplesHalf > (int) sweepNums.size() - 1) {
    return -1;
  }

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
  
  // copy gate fields to _momFields array
  
  for (int igate = 0; igate < _nGates; igate++) {
    _momFields[igate] = _gateData[igate]->fields;
  }

  // compute moments gate by gate
  
  for (int igate = 0; igate < _nGates; igate++) {

    AparGateData *gate = _gateData[igate];
    AparMomFields &fields = _momFields[igate];

    _mom->computeMoments(_pulses, gate->iq0, igate, fields);

  }
  
  // compute the alternating velocity

  _altVel.computeVelAlt(_nGates, _momFields, _nyquist);

  // copy back to gate data

  for (int igate = 0; igate < _nGates; igate++) {
    _gateData[igate]->fields = _momFields[igate];
  }

  // applyMedianFilter as appropriate
  
  _applyMedianFilterToZDR(_nGates);
  _applyMedianFilterToRHOHV(_nGates);
  
  // copy the results to the output beam Field vectors

  _copyDataToOutputFields();

  // free up pulses for use by other threads

  _releasePulses();

}

/////////////////////////////////////////////////
// set the calibration
    
void Beam::setCalib(const AparTsCalib &calib)

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
    scanMode = _opsInfo.getScanMode();
  }

  if (scanMode <= 0) {
    scanMode = DS_RADAR_SURVEILLANCE_MODE;
  }

  return scanMode;

}

////////////////////////////////////////////////
// get maximum range

double Beam::getMaxRange() const

{

  double maxRange = _opsInfo.getProcStartRangeKm() +
    _nGatesOut * _opsInfo.getProcGateSpacingKm();
  
  return maxRange;

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

//////////////////////////////////////
// initialize moments computations object
  
void Beam::_initMomentsObject()
  
{
  
  _mom->setNSamples(_nSamples);

  if (_params.correct_for_system_phidp) {
    _mom->setCorrectForSystemPhidp(true);
  } else {
    _mom->setCorrectForSystemPhidp(false);
  }

  if (_params.change_velocity_sign) {
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
      _needKdp = true;
      break;
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

}

////////////////////////////////////////////////
// compute kdp from phidp, using Bringi's method

void Beam::_kdpCompute()
  
{

  // make sure arrays are allocated
  
  _snrArray = _snrArray_.alloc(_nGates);
  _dbzArray = _dbzArray_.alloc(_nGates);
  _zdrArray = _zdrArray_.alloc(_nGates);
  _rhohvArray = _rhohvArray_.alloc(_nGates);
  _phidpArray = _phidpArray_.alloc(_nGates);
  
  // copy input data from Fields into arrays
  
  for (int ii = 0; ii < _nGates; ii++) {
    _snrArray[ii] = _gateData[ii]->fields.snr;
    _dbzArray[ii] = _gateData[ii]->fields.dbz;
    _zdrArray[ii] = _gateData[ii]->fields.zdr;
    _rhohvArray[ii] = _gateData[ii]->fields.rhohv;
    _phidpArray[ii] = _gateData[ii]->fields.phidp;
  }

  // write KDP ray files?
  
  if (_params.KDP_write_ray_files) {
    string rayDir = _params.KDP_ray_files_dir;
    _kdp.setWriteRayFile(true, rayDir);
  }

  // compute KDP
  
  _kdp.compute(_timeSecs,
               _nanoSecs / 1.0e9,
               _el,
               _az,
               _opsInfo.getRadarWavelengthCm(),
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

/////////////////////////////////////////////////////////////////
// Allocate or re-allocate gate data

void Beam::_allocGateData(int nGates)

{
  int nNeeded = nGates - (int) _gateData.size();
  if (nNeeded > 0) {
    for (int ii = 0; ii < nNeeded; ii++) {
      AparGateData *gate = new AparGateData;
      _gateData.push_back(gate);
    }
  }
  for (size_t ii = 0; ii < _gateData.size(); ii++) {
    _gateData[ii]->allocArrays(_nSamples, _isStagPrt);
  }
  _momFields = _momFields_.alloc(_gateData.size());
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
  
  // load iq0 and iq1

  for (int igate = 0, ipos = 0; igate < _nGates; igate++, ipos += 2) {
    AparGateData *gate = _gateData[igate];
    RadarComplex_t *iq0 = gate->iq0;
    RadarComplex_t *iq1 = gate->iq1;
    for (int isamp = 0; isamp < _nSamples; isamp++, iq0++, iq1++) {
      iq0->re = iqChan0[isamp][ipos];
      iq0->im = iqChan0[isamp][ipos + 1];
      if (iqChan1) {
        iq1->re = iqChan1[isamp][ipos];
        iq1->im = iqChan1[isamp][ipos + 1];
      }
    } // isamp
  } // igate
  
  return;

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
  
  double wavelengthMeters = _opsInfo.getRadarWavelengthCm() / 100.0;
  
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
    cerr << "WARNING - AparTs2Moments::Beam::_initStagPrt" << endl;
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
    AparMomFields &fld = _gateData[ii]->fields;
    if (fabs(fld.vel - smoothedVel[ii]) > _nyquistPrtShort / 2) {
      fld.vel = smoothedVel[ii];
    }
  }

}

