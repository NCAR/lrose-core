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
// AparMoments.cc
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// August 2007
//
///////////////////////////////////////////////////////////////
//
// AparMoments computes moments at a gate
//
///////////////////////////////////////////////////////////////

#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>
#include <toolsa/mem.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/TaArray.hh>
#include <rapmath/umath.h>
#include <apardata/AparMoments.hh>
#include <apardata/AparTsPulse.hh>

const double AparMoments::_missing = AparMomFields::missingDouble;
const double AparMoments::_phidpPhaseLimitAlt = -70;
const double AparMoments::_phidpPhaseLimitSim = -160;
const double AparMoments::_minDetectableSnr = 0.01; // -20 dB

// coeffs for computing least squares fit for width

const double AparMoments::_c1 = -0.192307692307692;
const double AparMoments::_c2 = -0.076923076923077;
const double AparMoments::_c3 = 0.269230769230769;

////////////////////////////////////////////////////
// Default constructor

AparMoments::AparMoments() :
        _debug(false),
        _verbose(false)

{

  _maxGates = 0;
  _rangeCorr = NULL;
  _atmosAttenCorr = NULL;
  _init();
}

////////////////////////////////////////////////////
// Constructor with max gates specified

AparMoments::AparMoments(int max_gates,
                         bool debug,
                         bool verbose) :
        _debug(debug),
        _verbose(verbose)
  
{
  
  _maxGates = max_gates;
  _allocRangeCorr();
  _allocAtmosAttenCorr();
  _init();

}

//////////////////////////////////////////////////////////////////
// destructor

AparMoments::~AparMoments()

{

  if (_rangeCorr) {
    delete[] _rangeCorr;
  }

  if (_atmosAttenCorr) {
    delete[] _atmosAttenCorr;
  }

}

////////////////////////////////////////////////////
// construct members

void AparMoments::_init()
  
{
  
  // initialize geometry

  _nSamples = 64;
  _nSamplesHalf = _nSamples / 2;

  _minSnrDbForZdr = _missing;
  _minSnrDbForLdr = _missing;

  _correctForSystemPhidp = false;
  _changeAiqSign = false;
  _computeCpaUsingAlt = false;

  _wavelengthMeters = 10.0;
  _prt = 0.001;
  _nyquist = 25;
 
  _prtShort = 0.005;
  _prtLong = 0.0075;
  _staggeredM = 2;
  _staggeredN = 3;
  _nGatesPrtShort = 0;
  _nGatesPrtLong = 0;
  _nyquistPrtShort = 10;
  _nyquistPrtLong = 6.66;
  _nyquistShortPlusLong = 4.0;
  _nyquistStagNominal = 25.0;
  _LL = 1;
  MEM_zero(_PP_);
  _PP = _PP_;
  
  _velSign = 1.0;
  _velSignStaggered = 1.0;
  _phidpSign = 1.0;
  _widthMethod = WIDTH_METHOD_R0R1;

  _windowR1 = 1.0;
  _windowR2 = 1.0;
  _windowR3 = 1.0;
  _windowHalfR1 = 1.0;
  _windowHalfR2 = 1.0;
  _windowHalfR3 = 1.0;

  _startRangeKm = _missing;
  _gateSpacingKm = _missing;

  _rangeCorrInit = false;
  
  _calNoisePowerHc = _missing;
  _calNoisePowerHx = _missing;
  _calNoisePowerVc = _missing;
  _calNoisePowerVx = _missing;

  _noisePowerHc = _missing;
  _noisePowerHx = _missing;
  _noisePowerVc = _missing;
  _noisePowerVx = _missing;

  _baseDbz1kmHc = _missing;
  _baseDbz1kmHx = _missing;
  _baseDbz1kmVc = _missing;
  _baseDbz1kmVx = _missing;

  _receiverGainDbHc = _missing;
  _receiverGainDbHx = _missing;
  _receiverGainDbVc = _missing;
  _receiverGainDbVx = _missing;

  _dbzCorrection = 0;
  _zdrCorrectionDb = 0;
  _ldrCorrectionDbH = 0;
  _ldrCorrectionDbV = 0;
  _systemPhidpDeg = 0;

  _calibXmitPowerDbmH = _missing;
  _calibXmitPowerDbmV = _missing;

  _computeZdrUsingSnr = false;

  _adjustDbzForMeasXmitPower = false;
  _adjustZdrForMeasXmitPower = false;
  
  _measXmitPowerDbmH = _missing;
  _measXmitPowerDbmV = _missing;

  _phidpOffsetAlt.re = 1.0;
  _phidpOffsetAlt.im = 0.0;
  _phidpOffsetSim.re = 1.0;
  _phidpOffsetSim.im = 0.0;

}

////////////////////////////////////////////////////
// set max number of gates

void AparMoments::setMaxGates(int max_gates)
  
{
  
  if (max_gates <= _maxGates) {
    return;
  }
  if (_rangeCorr != NULL) {
    delete[] _rangeCorr;
  }
  if (_atmosAttenCorr != NULL) {
    delete[] _atmosAttenCorr;
  }
  _maxGates = max_gates;
  _allocRangeCorr();
  _allocAtmosAttenCorr();

}

/////////////////////////
// set number of samples

void AparMoments::setNSamples(int n)

{

  _nSamples = n;
  _nSamplesHalf = n / 2;

}
  
///////////////////////////////////////////////////////////
// initialize for covariances

void AparMoments::_initForCovar()

{

  _sum_lag0_hc.initToZero();
  _sum_lag0_vc.initToZero();
  _sum_lag0_hx.initToZero();
  _sum_lag0_vx.initToZero();

  _sum_lag0_vchx.initToZero();
  _sum_lag0_hcvx.initToZero();
  _sum_lag1_hc.initToZero();
  _sum_lag1_vc.initToZero();
  _sum_lag1_hcvc.initToZero();
  _sum_lag1_vchc.initToZero();
  _sum_lag1_vxhx.initToZero();
  _sum_lag2_hc.initToZero();
  _sum_lag2_vc.initToZero();
  _sum_lag3_hc.initToZero();
  _sum_lag3_vc.initToZero();
  _sumrvvhh0.initToZero();

  _sum_lag0_hc_short.initToZero();
  _sum_lag0_vc_short.initToZero();
  _sum_lag0_hc_long.initToZero();
  _sum_lag0_vc_long.initToZero();

  _sum_lag1_hc_long.initToZero();
  _sum_lag1_vc_long.initToZero();
  _sum_lag1_hc_short.initToZero();
  _sum_lag1_vc_short.initToZero();
  _sum_lag1_hc_short_to_long.initToZero();
  _sum_lag1_vc_short_to_long.initToZero();
  _sum_lag1_hc_long_to_short.initToZero();
  _sum_lag1_vc_long_to_short.initToZero();

  _sum_rvvhh0_long.initToZero();
  _sum_rvvhh0_short.initToZero();


}

///////////////////////////////////////////////////////////
// Compute moments, for pulses from a single channel
// store results in fields object

void AparMoments::computeMoments(vector<const AparTsPulse *> &pulses,
                                 RadarComplex_t *iq0,
                                 int gateNum,
                                 AparMomFields &fields)
  
{

  // initialize
  
  _initForCovar();
  fields.init();

  // set the field metadata

  _setFieldMetaData(fields);

  // sum up for covariances - lag 0

  for (size_t ii = 0; ii < pulses.size(); ii++) {
    const AparTsPulse *pulse0 = pulses[ii];
    bool isCopol = pulse0->getChanIsCopol(0);
    if (isCopol) {
      if (pulse0->isHoriz()) {
        _sum_lag0_hc.add(RadarComplex::power(iq0[ii]));
      } else {
        _sum_lag0_vc.add(RadarComplex::power(iq0[ii]));
      }
    } else {
      // cross pol - for LDR
      if (pulse0->isHoriz()) {
        _sum_lag0_vx.add(RadarComplex::power(iq0[ii]));
      } else {
        _sum_lag0_hx.add(RadarComplex::power(iq0[ii]));
      }
    }
  }

  // sum up for covariances - lag 1

  for (size_t ii = 1; ii < pulses.size(); ii++) {
    const AparTsPulse *pulse0 = pulses[ii-1];
    const AparTsPulse *pulse1 = pulses[ii];
    if (pulse0->getVisitNumInBeam() != pulse1->getVisitNumInBeam()) {
      // not the same visit, so do not use this pair
      continue;
    }
    RadarComplex_t diff = RadarComplex::conjugateProduct(iq0[ii], iq0[ii-1]);
    if (pulse0->isHoriz() && pulse1->isHoriz()) {
      _sum_lag1_hc.add(diff);
    } else if (!pulse0->isHoriz() && !pulse1->isHoriz()) {
      _sum_lag1_vc.add(diff);
    } else if (pulse0->isHoriz() && !pulse1->isHoriz()) {
      _sum_lag1_vchc.add(diff);
    } else if (!pulse0->isHoriz() && pulse1->isHoriz()) {
      _sum_lag1_hcvc.add(diff);
    }
  }

  // sum up for covariances - lag 2

  for (size_t ii = 2; ii < pulses.size(); ii++) {
    const AparTsPulse *pulse0 = pulses[ii-2];
    const AparTsPulse *pulse2 = pulses[ii];
    if (pulse0->getVisitNumInBeam() != pulse2->getVisitNumInBeam()) {
      // not the same visit, so do not use this pair
      continue;
    }
    RadarComplex_t diff = RadarComplex::conjugateProduct(iq0[ii], iq0[ii-2]);
    if (pulse0->isHoriz() && pulse2->isHoriz()) {
      _sum_lag2_hc.add(diff);
    } else if (!pulse0->isHoriz() && !pulse2->isHoriz()) {
      _sum_lag2_vc.add(diff);
    }
  }

  // sum up for covariances - lag 3

  for (size_t ii = 3; ii < pulses.size(); ii++) {
    const AparTsPulse *pulse0 = pulses[ii-3];
    const AparTsPulse *pulse3 = pulses[ii];
    if (pulse0->getVisitNumInBeam() != pulse3->getVisitNumInBeam()) {
      // not the same visit, so do not use this pair
      continue;
    }
    RadarComplex_t diff = RadarComplex::conjugateProduct(iq0[ii], iq0[ii-3]);
    if (pulse0->isHoriz() && pulse3->isHoriz()) {
      _sum_lag3_hc.add(diff);
    } else if (!pulse0->isHoriz() && !pulse3->isHoriz()) {
      _sum_lag3_vc.add(diff);
    }
  }
  
  // compute and store covariances

  fields.lag0_hc = _sum_lag0_hc.getMean();
  fields.lag0_vc = _sum_lag0_vc.getMean();
  fields.lag0_hx = _sum_lag0_hx.getMean();
  fields.lag0_vx = _sum_lag0_vx.getMean();
  
  fields.lag1_hc = _sum_lag1_hc.getMean();
  fields.lag1_vc = _sum_lag1_vc.getMean();
  fields.lag1_vchc = _sum_lag1_vchc.getMean();
  fields.lag1_hcvc = _sum_lag1_hcvc.getMean();
  
  fields.lag2_hc = _sum_lag2_hc.getMean();
  fields.lag2_vc = _sum_lag2_vc.getMean();
  
  fields.lag3_hc = _sum_lag3_hc.getMean();
  fields.lag3_vc = _sum_lag3_vc.getMean();

  // dbm

  double dbm_hc = AparMomFields::missingDouble;
  double dbm_vc = AparMomFields::missingDouble;
  double dbm_hx = AparMomFields::missingDouble;
  double dbm_vx = AparMomFields::missingDouble;

  if (_sum_lag0_hc.valid()) {
    fields.lag0_hc_db = 10.0 * log10(fields.lag0_hc);
    dbm_hc = fields.lag0_hc_db - _receiverGainDbHc;
    fields.dbmhc = dbm_hc;
  }
  if (_sum_lag0_vc.valid()) {
    fields.lag0_vc_db = 10.0 * log10(fields.lag0_vc);
    dbm_vc = fields.lag0_vc_db - _receiverGainDbVc;
    fields.dbmvc = dbm_vc;
  }
  if (_sum_lag0_hx.valid()) {
    fields.lag0_hx_db = 10.0 * log10(fields.lag0_hx);
    dbm_hx = fields.lag0_hx_db - _receiverGainDbHx;
    fields.dbmhx = dbm_hx;
  }
  if (_sum_lag0_vx.valid()) {
    fields.lag0_vx_db = 10.0 * log10(fields.lag0_vx);
    dbm_vx = fields.lag0_vx_db - _receiverGainDbVx;
    fields.dbmvx = dbm_vx;
  }

  if (_sum_lag0_hc.valid() && _sum_lag0_vc.valid()) {
    fields.dbm = (dbm_hc + dbm_vc) / 2.0;
  } else if (_sum_lag0_hc.valid()) {
    fields.dbm = dbm_hc;
  } else if (_sum_lag0_vc.valid()) {
    fields.dbm = dbm_vc;
  }
  
  // compute noise-subtracted lag0
  
  double lag0_hc_ns = fields.lag0_hc - _noisePowerHc;
  double lag0_vc_ns = fields.lag0_vc - _noisePowerVc;
  double lag0_hx_ns = fields.lag0_hx - _noisePowerHx;
  double lag0_vx_ns = fields.lag0_vx - _noisePowerVx;
  
  // check SNR
  
  bool snrHcOK = true;
  double min_valid_pwr_hc = _noisePowerHc * _minDetectableSnr;
  if (lag0_hc_ns < min_valid_pwr_hc) {
    snrHcOK = false;
  }
  bool snrVcOK = true;
  double min_valid_pwr_vc = _noisePowerVc * _minDetectableSnr;
  if (lag0_vc_ns < min_valid_pwr_vc) {
    snrVcOK = false;
  }
  
  bool snrHxOK = true;
  double min_valid_pwr_hx = _noisePowerHx * _minDetectableSnr;
  if (lag0_hx_ns < min_valid_pwr_hx) {
    snrHxOK = false;
  }

  bool snrVxOK = true;
  double min_valid_pwr_vx = _noisePowerVx * _minDetectableSnr;
  if (lag0_vx_ns < min_valid_pwr_vx) {
    snrVxOK = false;
  }
  
  // compute snr
  
  double snr_hc = lag0_hc_ns / _calNoisePowerHc;
  double snr_hx = lag0_hx_ns / _calNoisePowerHx;
  double snr_vc = lag0_vc_ns / _calNoisePowerVc;
  double snr_vx = lag0_vx_ns / _calNoisePowerVx;
  
  if (snrHcOK) {
    fields.dbmhc_ns = 10.0 * log10(lag0_hc_ns) - _receiverGainDbHc;
    fields.snrhc = 10.0 * log10(snr_hc);
  } else {
    fields.dbmhc_ns = _missing;
    fields.snrhc = _missing;
  }
  if (snrHxOK) {
    fields.dbmhx_ns = 10.0 * log10(lag0_hx_ns) - _receiverGainDbHx;
    fields.snrhx = 10.0 * log10(snr_hx);
  } else {
    fields.dbmhx_ns = _missing;
    fields.snrhx = _missing;
  }
  if (snrVcOK) {
    fields.dbmvc_ns = 10.0 * log10(lag0_vc_ns) - _receiverGainDbVc;
    fields.snrvc = 10.0 * log10(snr_vc);
  } else {
    fields.dbmvc_ns = _missing;
    fields.snrvc = _missing;
  }
  if (snrVxOK) {
    fields.dbmvx_ns = 10.0 * log10(lag0_vx_ns) - _receiverGainDbVx;
    fields.snrvx = 10.0 * log10(snr_vx);
  } else {
    fields.dbmvx_ns = _missing;
    fields.snrvx = _missing;
  }
  
  if (snrHcOK && snrVcOK) {
    double snrMean = (snr_hc + snr_vc) / 2.0;
    fields.snr = 10.0 * log10(snrMean);
  } else {
    fields.snr = _missing;
  }
  
  // dbz
  
  if (snrHcOK) {
    double dbz_hc_no_atten_corr =
      10.0 * log10(snr_hc) + _baseDbz1kmHc + _rangeCorr[gateNum] + _dbzCorrection;
    double dbz_hc = dbz_hc_no_atten_corr + _atmosAttenCorr[gateNum];
    fields.dbzhc = _adjustDbzForPwrH(dbz_hc);
    fields.dbz = fields.dbzhc;
    fields.dbz_no_atmos_atten = _adjustDbzForPwrH(dbz_hc_no_atten_corr);
  } else {
    fields.dbz = _missing;
    fields.dbzhc = _missing;
    fields.dbz_no_atmos_atten = _missing;
  }
  if (snrVcOK) {
    double dbz_vc_no_atten_corr = 
      10.0 * log10(snr_vc) + _baseDbz1kmVc + _rangeCorr[gateNum] + _dbzCorrection;
    double dbz_vc = dbz_vc_no_atten_corr + _atmosAttenCorr[gateNum];
    fields.dbzvc = _adjustDbzForPwrV(dbz_vc);
  } else {
    fields.dbzvc = _missing;
  }
    
  // zdr
    
  if (snrHcOK && (fields.snrhc > _minSnrDbForZdr) &&
      snrVcOK && (fields.snrvc > _minSnrDbForZdr)) {
    double zdrm = 0.0;
    if (_computeZdrUsingSnr) {
      zdrm = fields.snrhc - fields.snrvc;
    } else {
      zdrm = 10.0 * log10(lag0_hc_ns / lag0_vc_ns);
    }
    fields.zdrm = _adjustZdrForPwr(zdrm);
    fields.zdr = fields.zdrm + _zdrCorrectionDb;
  } else {
    fields.zdrm = _missing;
    fields.zdr = _missing;
  }
    
  // ldr

  if (snrHxOK) {
    double dbz_hx_no_atten_corr =
      10.0 * log10(snr_hx) + _baseDbz1kmHx + _rangeCorr[gateNum] + _dbzCorrection;
    double dbz_hx = dbz_hx_no_atten_corr + _atmosAttenCorr[gateNum];
    fields.dbzhx = _adjustDbzForPwrV(dbz_hx);
  } else {
    fields.dbzhx = _missing;
  }
    
  if (snrVxOK) {
    double dbz_vx_no_atten_corr = 
      10.0 * log10(snr_vx) + _baseDbz1kmVx + _rangeCorr[gateNum] + _dbzCorrection;
    double dbz_vx = dbz_vx_no_atten_corr + _atmosAttenCorr[gateNum];
    fields.dbzvx = _adjustDbzForPwrH(dbz_vx);
  } else {
    fields.dbzvx = _missing;
  }
    
  if (snrHcOK && (fields.snrhc > _minSnrDbForLdr) &&
      snrVxOK && (fields.snrvx > _minSnrDbForLdr)) {
    double ldrhm = 10.0 * log10(lag0_vx_ns / lag0_hc_ns);
    fields.ldrhm = ldrhm;
    fields.ldrh = ldrhm + _ldrCorrectionDbH;
  } else {
    fields.ldrhm = _missing;
    fields.ldrh = _missing;
  }

  if (snrVcOK && (fields.snrvc > _minSnrDbForLdr) &&
      snrHxOK && (fields.snrhx > _minSnrDbForLdr)) {
    double ldrvm = 10.0 * log10(lag0_hx_ns / lag0_vc_ns);
    fields.ldrvm = ldrvm;
    fields.ldrv = ldrvm + _ldrCorrectionDbV;
  } else {
    fields.ldrvm = _missing;
    fields.ldrv = _missing;
  }

  fields.ldr = _missing;
  fields.ldr_diff = _missing;
  fields.ldr_mean = _missing;
  fields.zdr_bias = _missing;

  if (fields.ldrh != _missing && fields.ldrv != _missing) {
    fields.ldr = (fields.ldrh + fields.ldrv) / 2.0;
    fields.ldr_diff = fields.ldrv - fields.ldrh;
    if (fields.zdr != _missing) {
      double ldrvPrime = fields.ldrv - fields.zdr;
      fields.ldr_mean = (fields.ldrh + ldrvPrime) / 2.0;
      fields.zdr_bias = fields.zdr - fields.ldr_diff;
    }
  } else if (fields.ldrh != _missing) {
    fields.ldr = fields.ldrh;
    fields.ldr_mean = fields.ldrh;
  } else if (fields.ldrv != _missing) {
    fields.ldr = fields.ldrv;
    fields.ldr_mean = fields.ldrv;
  }

  ////////////////////////////////////////////////////
  // phidp, velocity
  //
  // See A. Zahrai and D. Zrnic
  // "The 10 cm wavelength polarimetric weather radar
  // at NOAA'a National Severe Storms Lab. "
  // JTech, Vol 10, No 5, October 1993.
  
  if (_sum_lag1_vchc.valid() && _sum_lag1_hcvc.valid()) {
    
    double mag_lag1_vchc = RadarComplex::mag(fields.lag1_vchc);
    double mag_lag1_hcvc = RadarComplex::mag(fields.lag1_hcvc);
    double arg_lag1_vchc = RadarComplex::argRad(fields.lag1_vchc);
    double arg_lag1_hcvc = RadarComplex::argRad(fields.lag1_hcvc);
    
    fields.lag1_vchc_db = 20.0 * log10(mag_lag1_vchc);
    fields.lag1_vchc_phase = arg_lag1_vchc * RAD_TO_DEG;
    fields.lag1_hcvc_db = 20.0 * log10(mag_lag1_hcvc);
    fields.lag1_hcvc_phase = arg_lag1_hcvc * RAD_TO_DEG;
    
    // Correct for system PhiDp offset, so that phidp will not wrap prematurely
    
    RadarComplex_t phi_h = 
      RadarComplex::complexProduct(fields.lag1_vchc, _phidpOffsetAlt);
    RadarComplex_t phi_v = 
      RadarComplex::conjugateProduct(fields.lag1_hcvc, _phidpOffsetAlt);
    
    // compute angular difference between them, which is phidp
    
    RadarComplex_t phidp = RadarComplex::conjugateProduct(phi_h, phi_v);
    double phidpRad = RadarComplex::argRad(phidp) / 2.0;
    fields.phidp = phidpRad * RAD_TO_DEG * _velSign * _phidpSign;
    
    RadarComplex_t phidp0 = 
      RadarComplex::conjugateProduct(fields.lag1_vchc, fields.lag1_hcvc);
    double phidpRad0 = RadarComplex::argRad(phidp0) / 2.0;
    fields.phidp0 = phidpRad0 * RAD_TO_DEG * _velSign * _phidpSign;
    
    // velocity phase is mean of phidp phases
    
    RadarComplex_t velVect = RadarComplex::complexMean(phi_h, phi_v);
    double argVel = RadarComplex::argRad(velVect);
    double meanVel = (argVel / M_PI) * _nyquist;
    fields.vel = meanVel * _velSign * -1.0;
    fields.vel_alt = fields.vel;

  } else if (_sum_lag1_hc.valid()) {
    
    double lag1_hc_mag = RadarComplex::mag(fields.lag1_hc);
    double argVel = RadarComplex::argRad(fields.lag1_hc);
    
    fields.lag1_hc_db = 20.0 * log10(lag1_hc_mag);
    fields.lag1_hc_phase = argVel * RAD_TO_DEG;
    
    double vel = (argVel / M_PI) * _nyquist;
    fields.vel = vel * _velSign * -1.0;
    
    fields.phase_for_noise = fields.lag1_hc;

  } else if (_sum_lag1_vc.valid()) {
    
    double lag1_vc_mag = RadarComplex::mag(fields.lag1_vc);
    double argVel = RadarComplex::argRad(fields.lag1_vc);
    
    fields.lag1_vc_db = 20.0 * log10(lag1_vc_mag);
    fields.lag1_vc_phase = argVel * RAD_TO_DEG;
    
    double vel = (argVel / M_PI) * _nyquist;
    fields.vel = vel * _velSign * -1.0;
    
    fields.phase_for_noise = fields.lag1_vc;

  }
  
  // also compute velocity which can be used in clutter

  if (_sum_lag2_hc.valid()) {
    double argVelH = RadarComplex::argRad(fields.lag2_hc);
    fields.vel_h_only = (argVelH / M_PI) * (_nyquist / 2.0) * _velSign * -1.0;
  }
  
  if (_sum_lag2_vc.valid()) {
    double argVelV = RadarComplex::argRad(fields.lag2_vc);
    fields.vel_v_only = (argVelV / M_PI) * (_nyquist / 2.0) * _velSign * -1.0;
  }

  // compute alt mode velocity

  if (_sum_lag2_hc.valid() && _sum_lag2_vc.valid()) {
    RadarComplex_t lag2_sum = RadarComplex::complexSum(fields.lag2_hc, fields.lag2_vc);
    double arg_vel2 = RadarComplex::argRad(lag2_sum);
    double vel_hv = (arg_vel2 / M_PI) * (_nyquist / 2.0);
    fields.vel_hv = vel_hv * _velSign * -1.0;
    fields.phase_for_noise = lag2_sum;
  }

  /////////////////////////
  // width, rhohv
  // noise corrected
  
  if (snrHcOK && snrVcOK && _sum_lag2_hc.valid() && _sum_lag2_vc.valid()) {
    
    // compute lag-2 rho
    
    double mean_lag0_ns = (lag0_hc_ns + lag0_vc_ns) / 2.0;
    if (mean_lag0_ns <= 0) {
      mean_lag0_ns = 1.0e-20;
    }
    RadarComplex_t sumR2 = RadarComplex::complexSum(fields.lag2_hc, fields.lag2_vc);
    double meanMagR2 = RadarComplex::mag(sumR2) / 2.0;
    double rho2 = meanMagR2 / mean_lag0_ns;
    
    // spectrum width from H and V
    
    double argWidth = sqrt(-0.5 * log(rho2));
    double width = (argWidth / M_PI) * _nyquist;
    fields.width = _constrain(width, 0.01, _nyquist);
    
    // width using H and V separately
  
    double lag2_hc_mag = RadarComplex::mag(fields.lag2_hc);
    double arg_lag2Hc = RadarComplex::argRad(fields.lag2_hc);
    fields.lag2_hc_db = 20.0 * log10(lag2_hc_mag);
    fields.lag2_hc_phase = arg_lag2Hc * RAD_TO_DEG;
    
    double lag2_vc_mag = RadarComplex::mag(fields.lag2_vc);
    double arg_lag2Vc = RadarComplex::argRad(fields.lag2_vc);
    fields.lag2_vc_db = 20.0 * log10(lag2_vc_mag);
    fields.lag2_vc_phase = arg_lag2Vc * RAD_TO_DEG;
    
    double r0_h = lag0_hc_ns;
    double r1_h = lag2_hc_mag / _windowR1;
    double r0r1_h = _computeR0R1Width(r0_h, r1_h, _nyquist / 2.0);
    fields.width_h_only = _constrain(r0r1_h, 0.01, _nyquist);

    double r0_v = lag0_vc_ns;
    double r1_v = lag2_vc_mag / _windowR1;
    double r0r1_v = _computeR0R1Width(r0_v, r1_v, _nyquist / 2.0);
    fields.width_v_only = _constrain(r0r1_v, 0.01, _nyquist);

    // compute lag-1 rhohv
    
    if (_sum_lag1_vchc.valid() && _sum_lag1_hcvc.valid()) {
    
      double mag_lag1_vchc = RadarComplex::mag(fields.lag1_vchc);
      double mag_lag1_hcvc = RadarComplex::mag(fields.lag1_hcvc);
      
      double rhohv1 =
        (mag_lag1_vchc + mag_lag1_hcvc) / (2.0 * sqrt(lag0_hc_ns * lag0_vc_ns));
      
      // lag-0 rhohv is rhohv1 corrected by rho2
      
      double rhohv0 = rhohv1 / pow(rho2, 0.25);
      fields.rhohv = _constrain(rhohv0, 0.0, 1.0);

    }
    
  } else if (snrHcOK && _sum_lag1_hc.valid() &&
             _sum_lag2_hc.valid() && _sum_lag3_vc.valid()) {
    
    double lag1_hc_mag = RadarComplex::mag(fields.lag1_hc);
    fields.lag1_hc_db = 20.0 * log10(lag1_hc_mag);
    fields.lag1_hc_phase = RadarComplex::argDeg(fields.lag1_hc);
    
    double lag2_hc_mag = RadarComplex::mag(fields.lag2_hc);
    fields.lag2_hc_db = 20.0 * log10(lag2_hc_mag);
    fields.lag2_hc_phase = RadarComplex::argDeg(fields.lag2_hc);
    
    double lag3_hc_mag = RadarComplex::mag(fields.lag3_hc);
    fields.lag3_hc_db = 20.0 * log10(lag3_hc_mag);
    fields.lag3_hc_phase = RadarComplex::argDeg(fields.lag3_hc);
    
    double r0 = lag0_hc_ns;
    double r1 = lag1_hc_mag / _windowR1;
    double r2 = lag2_hc_mag / _windowR2;
    double r3 = lag3_hc_mag / _windowR3;
    
    double width = _computeHybridWidth(r0, r1, r2, r3, _nyquist);
    fields.width = _constrain(width, 0.01, _nyquist);

  } else if (snrVcOK && _sum_lag1_vc.valid() &&
             _sum_lag2_vc.valid() && _sum_lag3_vc.valid()) {
    
    double lag1_vc_mag = RadarComplex::mag(fields.lag1_vc);
    fields.lag1_vc_db = 20.0 * log10(lag1_vc_mag);
    fields.lag1_vc_phase = RadarComplex::argDeg(fields.lag1_vc);
    
    double lag2_vc_mag = RadarComplex::mag(fields.lag2_vc);
    fields.lag2_vc_db = 20.0 * log10(lag2_vc_mag);
    fields.lag2_vc_phase = RadarComplex::argDeg(fields.lag2_vc);
    
    double lag3_vc_mag = RadarComplex::mag(fields.lag3_vc);
    fields.lag3_vc_db = 20.0 * log10(lag3_vc_mag);
    fields.lag3_vc_phase = RadarComplex::argDeg(fields.lag3_vc);
    
    double r0 = lag0_vc_ns;
    double r1 = lag1_vc_mag / _windowR1;
    double r2 = lag2_vc_mag / _windowR2;
    double r3 = lag3_vc_mag / _windowR3;
    
    double width = _computeHybridWidth(r0, r1, r2, r3, _nyquist);
    fields.width = _constrain(width, 0.01, _nyquist);

  } // if (snrHcOK && snrVcOK)
  
  ////////////////////////////
  // not-noise-corrected rhohv
  
  if (_sum_lag0_hc.valid() && _sum_lag0_vc.valid() &&
      _sum_lag1_vchc.valid() && _sum_lag1_hcvc.valid() &&
      _sum_lag2_hc.valid() && _sum_lag2_vc.valid()) {
    
    // compute lag-2 rho
    
    double mean_lag0 = (fields.lag0_hc + fields.lag0_vc) / 2.0;
    RadarComplex_t sumR2 = RadarComplex::complexSum(fields.lag2_hc, fields.lag2_vc);
    double meanMagR2 = RadarComplex::mag(sumR2) / 2.0;
    double rho2 = meanMagR2 / mean_lag0;
    
    // compute lag-1 rhohv
    
    double mag_lag1_vchc = RadarComplex::mag(fields.lag1_vchc);
    double mag_lag1_hcvc = RadarComplex::mag(fields.lag1_hcvc);
    double rhohv1 =
      (mag_lag1_vchc + mag_lag1_hcvc) / (2.0 * sqrt(fields.lag0_hc * fields.lag0_vc));
    
    // lag-0 rhohv is rhohv1 corrected by rho2
    
    double rhohv0 = rhohv1 / pow(rho2, 0.25);
    fields.rhohv_nnc = _constrain(rhohv0, 0.0, 1.0);

  } // if (_sum_lag0_hc.valid() && _sum_lag0_vc.valid() ...
  
  /////////////////////////////
  // ncp - not noise corrected
  
  {

    if (_sum_lag0_hc.valid() && _sum_lag0_vc.valid() &&
        _sum_lag2_hc.valid() && _sum_lag2_vc.valid()) {
      // alternating mode
      double mean_lag0 = (fields.lag0_hc + fields.lag0_vc) / 2.0;
      RadarComplex_t sumR2 = RadarComplex::complexSum(fields.lag2_hc, fields.lag2_vc);
      double meanMagR2 = RadarComplex::mag(sumR2) / 2.0;
      double rho2 = meanMagR2 / mean_lag0;
      fields.ncp = _constrain(rho2, 0.0, 1.0);
      // ncp using H and V separately
      double lag2_hc_mag = RadarComplex::mag(fields.lag2_hc);
      fields.ncp_h_only = _constrain(lag2_hc_mag / fields.lag0_hc, 0.0, 1.0);
      double lag2_vc_mag = RadarComplex::mag(fields.lag2_vc);
      fields.ncp_v_only = _constrain(lag2_vc_mag / fields.lag0_vc, 0.0, 1.0);
      fields.ncp_h_minus_v = fields.ncp_h_only - fields.ncp_v_only;
    } else if (_sum_lag0_hc.valid() && _sum_lag1_hc.valid()) {
      // hc-only
      double lag1_hc_mag = RadarComplex::mag(fields.lag1_hc);
      double ncp = lag1_hc_mag / fields.lag0_hc;
      ncp = _constrain(ncp, 0.0, 1.0);
      fields.ncp = ncp;
      fields.ncp_h_only = ncp;
    } else if (_sum_lag0_vc.valid() && _sum_lag1_vc.valid()) {
      // vc-only
      double lag1_vc_mag = RadarComplex::mag(fields.lag1_vc);
      double ncp = lag1_vc_mag / fields.lag0_vc;
      ncp = _constrain(ncp, 0.0, 1.0);
      fields.ncp = ncp;
      fields.ncp_v_only = ncp;
    }
    
  } // if (snrHcOK && snrVcOK)
  
}

///////////////////////////////////////////////////////////
// set field metadata
  
void AparMoments::_setFieldMetaData(AparMomFields &fields)
  
{
  
  fields.prt = _prt;
  fields.num_pulses = _nSamples;
  fields.prt_short = _prtShort;
  fields.prt_long = _prtLong;

}

///////////////////////////////////////
// initialize based on params passed in

void AparMoments::init(double prt,
                       double wavelengthMeters,
                       double startRangeKm,
                       double gateSpacingKm)
  
{

  // basic parameters
  
  _wavelengthMeters = wavelengthMeters;
  _prt = prt;
  _nyquist = ((_wavelengthMeters / _prt) / 4.0);

  // compute range correction table
  
  _computeRangeCorrection(startRangeKm, gateSpacingKm);

}

///////////////////////////////////////
// initialize based on AparTsInfo

void AparMoments::init(double prt,
                       const AparTsInfo &opsInfo)
  
{
  
  init(prt,
       opsInfo.getRadarWavelengthCm() / 100.0,
       opsInfo.getProcStartRangeM() / 1000.0,
       opsInfo.getProcGateSpacingM() / 1000.0);

}

///////////////////////////////////////
// initialize staggered PRT mode

void AparMoments::initStagPrt(double prtShort,
                              double prtLong,
                              int staggeredM,
                              int staggeredN,
                              int nGatesPrtShort,
                              int nGatesPrtLong,
                              double wavelengthMeters,
                              double startRangeKm,
                              double gateSpacingKm)
  
{

  _wavelengthMeters = wavelengthMeters;

  // staggered PRT
  
  _prtShort = prtShort;
  _prtLong = prtLong;
  _staggeredM = staggeredM;
  _staggeredN = staggeredN;
  _nGatesPrtShort = nGatesPrtShort;
  _nGatesPrtLong = nGatesPrtLong;

  // basic parameters
  
  _nyquistPrtShort = ((_wavelengthMeters / _prtShort) / 4.0);
  _nyquistPrtLong = ((_wavelengthMeters / _prtLong) / 4.0);
  _nyquistShortPlusLong = ((_wavelengthMeters / (_prtShort + _prtLong)) / 4.0);
  _nyquistStagNominal = ((_wavelengthMeters / (_prtShort / _staggeredM)) / 4.0);
  _nyquist = _nyquistPrtShort * _staggeredM;

  // #define DEBUG_PRINT
#ifdef DEBUG_PRINT
   cerr << "11111 _prtShort: " <<  _prtShort << endl;
  cerr << "11111 _prtLong: " <<  _prtLong << endl;
  cerr << "11111 _nyquistPrtShort: " <<  _nyquistPrtShort << endl;
  cerr << "11111 _nyquistPrtLong: " <<  _nyquistPrtLong << endl;
  cerr << "11111 _nyquist: " <<  _nyquist << endl;
  cerr << "11111 _staggeredM: " <<  _staggeredM << endl;
  cerr << "11111 _staggeredN: " <<  _staggeredN << endl;
#endif
  
  // compute range correction table
  
  _computeRangeCorrection(startRangeKm, gateSpacingKm);

  // compute the P lookup table for dealiasing
  // See Torres et al, JTech, Sept 2004

  _LL = (_staggeredM + _staggeredN - 1) / 2;
  if (_LL > 5) {
    _LL = 2; // set to 2/3
  }
  _PP = _PP_ + _LL;

  int cc = 0;
  int pp = 0;
  _PP[0] = 0;
  for (int ll = 1; ll <= _LL; ll++) {
    if ((ll / 2 * 2) == ll) {
      // even - va1 transition
      cc -= _staggeredN;
      pp++;
    } else {
      // odd - va2 transition
      cc += _staggeredM;
    }
    _PP[cc] = pp;
    _PP[-cc] = -pp;
  }

#ifdef NOTNOW
  cerr << "LL: " << _LL << endl;
  cerr << "PP array: " << endl;
  for (int ll = -_LL; ll <= _LL; ll++) {
    cerr << "ll, PP: " << ll << ": " << _PP[ll] << endl;
  }
#endif

}

///////////////////////////////////////
// initialize staggered PRT mode
// bases on OpsInfo

void AparMoments::initStagPrt(double prtShort,
                              double prtLong,
                              int staggeredM,
                              int staggeredN,
                              int nGatesPrtShort,
                              int nGatesPrtLong,
                              const AparTsInfo &opsInfo)
  
{
  
  initStagPrt(prtShort,
              prtLong,
              staggeredM,
              staggeredN,
              nGatesPrtShort,
              nGatesPrtLong,
              opsInfo.getRadarWavelengthCm() / 100.0,
              opsInfo.getProcStartRangeM() / 1000.0,
              opsInfo.getProcGateSpacingM() / 1000.0);

}

/////////////////////////////////////////////////////////////
// set the calibration

void AparMoments::setCalib(const AparTsCalib &calib)
  
{
  
  // set noise values
  
  _calNoisePowerHc = pow(10.0, calib.getNoiseDbmHc() / 10.0);
  _calNoisePowerHx = pow(10.0, calib.getNoiseDbmHx() / 10.0);
  _calNoisePowerVc = pow(10.0, calib.getNoiseDbmVc() / 10.0);
  _calNoisePowerVx = pow(10.0, calib.getNoiseDbmVx() / 10.0);

  _noisePowerHc = _calNoisePowerHc;
  _noisePowerVc = _calNoisePowerVc;
  _noisePowerHx = _calNoisePowerHx;
  _noisePowerVx = _calNoisePowerVx;

  _baseDbz1kmHc = calib.getBaseDbz1kmHc();
  _baseDbz1kmHx = calib.getBaseDbz1kmHx();
  _baseDbz1kmVc = calib.getBaseDbz1kmVc();
  _baseDbz1kmVx = calib.getBaseDbz1kmVx();

  _receiverGainDbHc = calib.getReceiverGainDbHc();
  _receiverGainDbHx = calib.getReceiverGainDbHx();
  _receiverGainDbVc = calib.getReceiverGainDbVc();
  _receiverGainDbVx = calib.getReceiverGainDbVx();

  _dbzCorrection = calib.getDbzCorrection();
  _zdrCorrectionDb = calib.getZdrCorrectionDb();
  _ldrCorrectionDbH = calib.getLdrCorrectionDbH();
  _ldrCorrectionDbV = calib.getLdrCorrectionDbV();
  _systemPhidpDeg = calib.getSystemPhidpDeg();

  _calibXmitPowerDbmH = calib.getXmitPowerDbmH();
  _calibXmitPowerDbmV = calib.getXmitPowerDbmV();

  // compute phidp offset, to prevent premature wrapping of the phase
  // vectors from which phidp and velocity are computed.
  
  if (_correctForSystemPhidp) {
    double offsetAlt = _systemPhidpDeg - _phidpPhaseLimitAlt;
    _phidpOffsetAlt.re = cos(offsetAlt * DEG_TO_RAD * -1.0 * _velSign);
    _phidpOffsetAlt.im = sin(offsetAlt * DEG_TO_RAD * -1.0 * _velSign);
    double offsetSim = _systemPhidpDeg - _phidpPhaseLimitSim;
    _phidpOffsetSim.re = cos(offsetSim * DEG_TO_RAD * _velSign);
    _phidpOffsetSim.im = sin(offsetSim * DEG_TO_RAD * _velSign);
  } else {
    _phidpOffsetAlt.re = 1.0 * _velSign;
    _phidpOffsetAlt.im = 0.0;
    _phidpOffsetSim.re = 1.0 * _velSign;
    _phidpOffsetSim.im = 0.0;
  }

}

/////////////////////////////////////////////////////////////
// set the noise power to be used for moments computation
// note that noise is power at the DRX,
// before subtraction of receiver gain

void AparMoments::setNoiseDbmHc(double val)
{
  _noisePowerHc = pow(10.0, val / 10.0);
}

void AparMoments::setNoiseDbmVc(double val)
{
  _noisePowerVc = pow(10.0, val / 10.0);
}

void AparMoments::setNoiseDbmHx(double val)
{
  _noisePowerHx = pow(10.0, val / 10.0);
}

void AparMoments::setNoiseDbmVx(double val)
{
  _noisePowerVx = pow(10.0, val / 10.0);
}

///////////////////////////////////////////
// negate the velocity - change the sign
  
void AparMoments::setChangeVelocitySign(bool state)
{
  if (state) {
    _velSign = -1.0;
  } else {
    _velSign = 1.0;
  }
}

///////////////////////////////////////////
// negate the phidp - change the sign
  
void AparMoments::setChangePhidpSign(bool state)
{
  if (state) {
    _phidpSign = -1.0;
  } else {
    _phidpSign = 1.0;
  }
}

/////////////////////////////////////////////////////////
// negate the velocity - change the sign - staggered only
// this is used in conjunction with setChangeVelocitySign()
// so if both are set they cancel out for staggered.
  
void AparMoments::setChangeVelocitySignStaggered(bool state)
{
  if (state) {
    _velSignStaggered = -1.0;
  } else {
    _velSignStaggered = 1.0;
  }
}

///////////////////////////////////////
// set the window R values, used for width correction

void AparMoments::setWindowRValues(double windowR1,
                                   double windowR2,
                                   double windowR3,
                                   double windowHalfR1,
                                   double windowHalfR2,
                                   double windowHalfR3)

{

  _windowR1 = windowR1;
  _windowR2 = windowR2;
  _windowR3 = windowR3;
  _windowHalfR1 = windowHalfR1;
  _windowHalfR2 = windowHalfR2;
  _windowHalfR3 = windowHalfR3;
  
}
     
/////////////////////////////////////////////////
// compute sdev of magnitudes of a time series

double AparMoments::computeMagSdev(const RadarComplex_t *iq,
                                   int nSamples)
  
{
  
  double sumMag = 0.0;
  double sumMagSq = 0.0;
  const RadarComplex_t *iqp = iq;
  for (int i = 0; i < nSamples; i++, iqp++) {
    double ii = iqp->re;
    double qq = iqp->im;
    double mag = sqrt(ii * ii + qq * qq);
    sumMag += mag;
    sumMagSq += (mag * mag);
  }

  double nn = (double) nSamples;
  double var = (sumMagSq - (sumMag * sumMag) / nn) / (nn - 1.0);
  double sdev = 0.0;

  if (var >= 0.0) {
    sdev = sqrt(var);
  }

  return sdev;

}
    
///////////////////////////////////////////////////
// compute SQRT of ratio of DC power to total power

double AparMoments::computePowerRatio(const RadarComplex_t *iq,
                                      int nSamples)
  
{

  RadarComplex_t sumIq;
  sumIq.re = 0.0;
  sumIq.im = 0.0;

  double sumPower = 0.0;

  const RadarComplex_t *iqp = iq;
  for (int i = 0; i < nSamples; i++, iqp++) {
    double ii = iqp->re;
    double qq = iqp->im;
    double power = ii * ii + qq * qq;
    sumPower += power;
    sumIq.re += ii;
    sumIq.im += qq;
  }

  double magSum = RadarComplex::mag(sumIq);
  double pr = (magSum * magSum) / (sumPower * nSamples);

  return sqrt(pr);

}

double AparMoments::computePowerRatio(const RadarComplex_t *iqh,
                                      const RadarComplex_t *iqv,
                                      int nSamples)
  
{
  
  double prh = computePowerRatio(iqh, nSamples);
  double prv = computePowerRatio(iqv, nSamples);
  return ((prh + prv) / 2.0);

}
  
///////////////////////////////////////////////////////////
// Compute ncp for a given time series
  
double AparMoments::computeNcp(RadarComplex_t *iq)
  
{
  
  double lag0 = RadarComplex::meanPower(iq, _nSamples);
  RadarComplex_t lag1 =
    RadarComplex::meanConjugateProduct(iq + 1, iq, _nSamples - 1);
  double lag1_mag = RadarComplex::mag(lag1);
  double ncp = lag1_mag / lag0;
  ncp = _constrain(ncp, 0.0, 1.0);
  return ncp;
  
}

/////////////////////////////////////
// initialize rectangular window

void AparMoments::initWindowRect(int nSamples, double *window)
  
{
  
  for (int ii = 0; ii < nSamples; ii++) {
    window[ii] = 1.0;
  }

}
  
/////////////////////////////////////
// initialize vonHann window

void AparMoments::initWindowVonhann(int nSamples, double *window)

{

  for (int ii = 0; ii < nSamples; ii++) {
    double ang = 2.0 * M_PI * ((ii + 0.5) / (double) nSamples - 0.5);
    window[ii] = 0.5 * (1.0 + cos(ang));
  }

  // adjust window to keep power constant

  double sumsq = 0.0;
  for (int ii = 0; ii < nSamples; ii++) {
    sumsq += window[ii] * window[ii];
  }
  double rms = sqrt(sumsq / nSamples);
  for (int ii = 0; ii < nSamples; ii++) {
    window[ii] /= rms;
  }

}
  
/////////////////////////////////////
// initialize Blackman window

void AparMoments::initWindowBlackman(int nSamples, double *window)

{
  
  for (int ii = 0; ii < nSamples; ii++) {
    double pos =
      ((nSamples + 1.0) / 2.0 + (double) ii ) / nSamples;
    window[ii] = (0.42
                  + 0.5 * cos(2.0 * M_PI * pos)
                  + 0.08 * cos(4.0 * M_PI * pos));
  }
  
  // adjust window to keep power constant
  
  double sumsq = 0.0;
  for (int ii = 0; ii < nSamples; ii++) {
    sumsq += window[ii] * window[ii];
  }
  double rms = sqrt(sumsq / nSamples);
  for (int ii = 0; ii < nSamples; ii++) {
    window[ii] /= rms;
  }

}
  
/////////////////////////////////////
// initialize Blackman-Nuttall window

void AparMoments::initWindowBlackmanNuttall(int nSamples, double *window)

{
  
  double a0 = 0.3635819;
  double a1 = 0.4891775;
  double a2 = 0.1365995;
  double a3 = 0.0106411;

  for (int ii = 0; ii < nSamples; ii++) {
    double pos = (ii + 0.5) / nSamples;
    window[ii] = (a0
                  - (a1 * cos(2.0 * M_PI * pos))
                  + (a2 * cos(4.0 * M_PI * pos))
                  - (a3 * cos(6.0 * M_PI * pos)));
  }

  // adjust window to keep power constant
  
  double sumsq = 0.0;
  for (int ii = 0; ii < nSamples; ii++) {
    sumsq += window[ii] * window[ii];
  }
  double rms = sqrt(sumsq / nSamples);
  for (int ii = 0; ii < nSamples; ii++) {
    window[ii] /= rms;
  }

}
  
/////////////////////////////////////
// initialize Tukey window
// alpha can vary between 0 and 1
// alpha == 1 implies a VonHann window
// alpha == 0 implies a rectangular window

void AparMoments::initWindowTukey(double alpha, int nSamples, double *window)

{

  // sanity check

  if (alpha < 0.0) {
    alpha = 0.0;
  } else if (alpha > 1.0) {
    alpha = 1.0;
  }

  // compute limits between which function is 1.0

  double lowerLimit = (alpha * (nSamples - 1.0)) / 2.0;
  double upperLimit = (nSamples - 1.0) * (1.0 - alpha / 2.0);

  // compute window terms

  for (int ii = 0; ii < nSamples; ii++) {
    if (ii < lowerLimit) {
      double term1 = ((2.0 * ii) / (alpha * (nSamples - 1.0))) - 1.0;
      window[ii] = 0.5 * (1.0 + cos(M_PI * term1));
    } else if (ii > upperLimit) {
      double term1 = ((2.0 * ii) / (alpha * (nSamples - 1.0))) - (2.0 / alpha) + 1.0;
      window[ii] = 0.5 * (1.0 + cos(M_PI * term1));
    } else {
      window[ii] = 1.0;
    }
  }

  // adjust window to keep power constant
  
  double sumsq = 0.0;
  for (int ii = 0; ii < nSamples; ii++) {
    sumsq += window[ii] * window[ii];
  }
  double rms = sqrt(sumsq / nSamples);
  for (int ii = 0; ii < nSamples; ii++) {
    window[ii] /= rms;
  }

}
  
/////////////////////////////////////
// create rectangular window
// Allocates memory and returns window

double *AparMoments::createWindowRect(int nSamples)
  
{
  
  double *window = new double[nSamples];
  initWindowRect(nSamples, window);
  return window;

}
  
/////////////////////////////////////
// create vonHann window
// Allocates memory and returns window

double *AparMoments::createWindowVonhann(int nSamples)

{

  double *window = new double[nSamples];
  initWindowVonhann(nSamples, window);
  return window;

}
  
/////////////////////////////////////
// create Blackman window
// Allocates memory and returns window

double *AparMoments::createWindowBlackman(int nSamples)

{
  
  double *window = new double[nSamples];
  initWindowBlackman(nSamples, window);
  return window;
  
}
  
/////////////////////////////////////
// create Blackman window
// Allocates memory and returns window

double *AparMoments::createWindowBlackmanNuttall(int nSamples)

{
  
  double *window = new double[nSamples];
  initWindowBlackmanNuttall(nSamples, window);
  return window;
  
}
  
/////////////////////////////////////
// create Tukey window
// Allocates memory and returns window

double *AparMoments::createWindowTukey(double alpha, int nSamples)

{
  
  double *window = new double[nSamples];
  initWindowTukey(alpha, nSamples, window);
  return window;
  
}
  
///////////////////////////////////////
// apply window to IQ samples, in place

void AparMoments::applyWindow(RadarComplex_t *iq,
                              const double *window,
                              int nSamples)
  
{
  
  const double *ww = window;
  RadarComplex_t *iqp = iq;
  
  for (int ii = 0; ii < nSamples; ii++, ww++, iqp++) {
    iqp->re *= *ww;
    iqp->im *= *ww;
  }

}

///////////////////////////////////////////
// apply window to IQ samples, not in place

void AparMoments::applyWindow(const RadarComplex_t *iqOrig,
                              const double *window,
                              RadarComplex_t *iqWindowed,
                              int nSamples)
  
{
  
  const double *ww = window;
  const RadarComplex_t *iqp = iqOrig;
  RadarComplex_t *iqw = iqWindowed;
  
  for (int ii = 0; ii < nSamples; ii++, ww++, iqp++, iqw++) {
    iqw->re = iqp->re * *ww;
    iqw->im = iqp->im * *ww;
  }

}

///////////////////////////////////////////////////
// invert (undo) window to IQ samples, not in place

void AparMoments::invertWindow(const RadarComplex_t *iqWindowed,
                               const double *window,
                               RadarComplex_t *iqOrig,
                               int nSamples)
  
{
  
  const double *ww = window;
  const RadarComplex_t *iqw = iqWindowed;
  RadarComplex_t *iqp = iqOrig;
  
  for (int ii = 0; ii < nSamples; ii++, ww++, iqp++, iqw++) {
    iqp->re = iqw->re / *ww;
    iqp->im = iqw->im / *ww;
  }

}

////////////////////////////////////////////////
// compute serial correlation value for a window

double AparMoments::computeWindowCorrelation(int lag,
                                             double *window,
                                             int nSamples)
  
{
  
  double sum12 = 0.0;

  for (int ii = 0; ii < nSamples - lag; ii++) {
    double val1 = window[ii];
    double val2 = window[ii + lag];
    sum12 += val1 * val2;
  }
  
  double correlation = sum12 / (double) (nSamples - lag);
  
  return correlation;

}

///////////////////////////////////////
// compute range correction table

void AparMoments::_computeRangeCorrection(double startRangeKm,
                                          double gateSpacingKm)

{


  if (_startRangeKm == startRangeKm &&
      _gateSpacingKm == gateSpacingKm &&
      _rangeCorrInit) {
    return;
  }

  _startRangeKm = startRangeKm;
  _gateSpacingKm = gateSpacingKm;
  
  for (int ii = 0; ii < _maxGates; ii++) {
    double range_km = _startRangeKm + ii * _gateSpacingKm;
    if (range_km < 0.001) {
      _rangeCorr[ii] = 0.0;
    } else {
      _rangeCorr[ii] = 20.0 * log10(range_km);
    }
  }

  _rangeCorrInit = true;

}

/////////////////////////////////////////////////////
// load up the 2-way atmospheric attenuation table,
// given the elevation angle

void AparMoments::loadAtmosAttenCorrection(int nGates,
                                           double elevationDeg,
                                           const AtmosAtten &atmosAtten)
{ 

  if (nGates > _maxGates) {
    nGates = _maxGates;
  }

  for (int ii = 0; ii < nGates; ii++) {
    double rangeKm = _startRangeKm + ii * _gateSpacingKm;
    _atmosAttenCorr[ii] = atmosAtten.getAtten(elevationDeg, rangeKm);
  }

}

//////////////////////////////////////////////////
// Clutter phase alignment - single pol

double AparMoments::computeCpa(const RadarComplex_t *iq,
                               int nSamples)
  
{

  double sumMag = 0.0;
  double sumI = 0.0, sumQ = 0.0;
  const RadarComplex_t *iqp = iq;
  for (int i = 0; i < nSamples; i++, iqp++) {
    double ii = iqp->re;
    double qq = iqp->im;
    sumI += ii;
    sumQ += qq;
    sumMag += sqrt(ii * ii + qq * qq);
  }

  // compute mean I and Q
  
  double avI = sumI / nSamples;
  double avQ = sumQ / nSamples;
  double avMag = sumMag / nSamples;
  double phasorLen = sqrt(avI * avI + avQ * avQ);
  double cpa = phasorLen / avMag;
  return cpa;

}
    
// CPA - dual pol mode

double AparMoments::computeCpa(const RadarComplex_t *iqh,
                               const RadarComplex_t *iqv,
                               int nSamples)
  
{
  double cpa_h = computeCpa(iqh, nSamples);
  double cpa_v = computeCpa(iqv, nSamples);
  return (cpa_h + cpa_v) / 2.0;
}

//////////////////////////////////////////////////
// Clutter phase alignment - alternative form.
//
// Alternative formulation where we look for the
// minimum 5-pt running CPA and then compute the CPA
// values on each side of the minimum. The mean of
// these two values is returned.
//
// This formulation works well for time series in
// which the CPA value is high, then becomes low for
// a short period, and then returns to high values
// for the rest of the series.

double AparMoments::computeCpaAlt(const RadarComplex_t *iq,
                                  int nSamples)
  
{

  // check we have enough points for the running CPA

  if (nSamples < 8) {
    return computeCpa(iq, nSamples);
  }
  
  TaArray<RadarComplex_t> iqPhasor_;
  RadarComplex_t *iqPhasor = iqPhasor_.alloc(nSamples);
  TaArray<double> mag_;
  double *mag = mag_.alloc(nSamples);
  double sumRe = 0.0, sumIm = 0.0;
  for (int ii = 0; ii < nSamples; ii++) {
    sumRe += iq[ii].re;
    sumIm += iq[ii].im;
    iqPhasor[ii].re = sumRe;
    iqPhasor[ii].im = sumIm;
    mag[ii] = RadarComplex::mag(iq[ii]);
  }

  TaArray<double> runningCpa_;
  double *runningCpa = runningCpa_.alloc(nSamples);
  memset(runningCpa, 0, nSamples * sizeof(double));
 
  int nrun = 5;
  int nhalf = nrun/2;

  double sumMag = 0.0;
  for (int ii = 0; ii < nrun - 1; ii++) {
    sumMag += mag[ii];
  }

  double dI, dQ, dist;
  for (int ii = nhalf; ii < nSamples - nhalf; ii++) {
    sumMag += mag[ii+nhalf];
    dI = iqPhasor[ii-nhalf].re - iqPhasor[ii+nhalf].re;
    dQ = iqPhasor[ii-nhalf].im - iqPhasor[ii+nhalf].im;
    dist = sqrt(dI * dI + dQ * dQ);
    runningCpa[ii] = dist / sumMag;
    sumMag -= mag[ii-nhalf];
  }
  for (int ii = 0; ii < nhalf; ii++) {
    runningCpa[ii] = runningCpa[nhalf];
    runningCpa[nSamples-ii-1] = runningCpa[nSamples-nhalf-1];
  }

  // find minimum running cpa
  
  double minRunningCpa = 99.0;
  double maxRunningCpa = -99.0;
  int minIndex = 0;
  for (int ii = nhalf; ii < nSamples - nhalf; ii++) {
    if (runningCpa[ii] < minRunningCpa) {
      minRunningCpa = runningCpa[ii];
      minIndex = ii;
    }
    if (runningCpa[ii] > maxRunningCpa) {
      maxRunningCpa = runningCpa[ii];
    }
  }

  // compute CPA from either side of minimum
  
  sumMag = 0;
  for (int ii = 0; ii < nSamples; ii++) {
    sumMag += mag[ii];
  }

  // int end = minIndex - nhalf;

  double dI0 = iqPhasor[minIndex].re;
  double dQ0 = iqPhasor[minIndex].im;
  double dist0 = sqrt(dI0 * dI0 + dQ0 * dQ0);

  double dI1 = iqPhasor[nSamples-1].re - iqPhasor[minIndex].re;
  double dQ1 = iqPhasor[nSamples-1].im - iqPhasor[minIndex].im;
  double dist1 = sqrt(dI1 * dI1 + dQ1 * dQ1);

  double cpa = (dist0 + dist1) / sumMag;

  return cpa;

}
    
/////////////////////////////////////////////////////////////
// CPA alt - dual pol mode

double AparMoments::computeCpaAlt(const RadarComplex_t *iqh,
                                  const RadarComplex_t *iqv,
                                  int nSamples)
  
{
  double cpa_h = computeCpaAlt(iqh, nSamples);
  double cpa_v = computeCpaAlt(iqv, nSamples);
  return (cpa_h + cpa_v) / 2.0;
}

/////////////////////////////////////////////////
// compute width using R0R1 method
// from Greg Meymaris

double AparMoments::_computeR0R1Width(double r0,
                                      double r1,
                                      double nyquist) const
{ 
  
  double r0r1 = 0;
  if (r0 > r1) {
    r0r1 = sqrt(log(r0 / r1) * 2.0) / M_PI;
  }

  r0r1 = _constrain(r0r1, 0, 1);
  return r0r1 * nyquist;

}

/////////////////////////////////////////////////
// compute width using R1R2 method
// from Greg Meymaris

double AparMoments::_computeR1R2Width(double r1,
                                      double r2,
                                      double nyquist) const
{ 
  double r1r2 = 0;
  if (r1 > r2) {
    r1r2 = sqrt(log(r1 / r2) * 0.6667) / M_PI;
  }
  r1r2 = _constrain(r1r2, 0, 1);
  return r1r2 * nyquist;
  
}

/////////////////////////////////////////////////
// compute width using R1R3 method
// from Greg Meymaris

double AparMoments::_computeR1R3Width(double r1,
                                      double r3,
                                      double nyquist) const
{ 

  double r1r3 = 0;
  if (r1 > r3) {
    r1r3 = sqrt(log(r1 / r3) * 0.25) / M_PI;
  }
  _constrain(r1r3, 0, 1);
  return r1r3 * nyquist;

}

/////////////////////////////////////////////////
// compute width using R0R1R2 method
// from Greg Meymaris

double AparMoments::_computePplsWidth(double r0,
                                      double r1,
                                      double r2,
                                      double nyquist) const
{ 
  double r0r1r2 = 0;
  double qq = _c1 * log(r0) + _c2 * log(r1) + _c3 * log(r2);
  if (qq < 0) {
    r0r1r2 = sqrt(-2.0 * qq) / M_PI;
  }
  _constrain(r0r1r2, 0, 1);
  return r0r1r2 * nyquist;

}

/////////////////////////////////////////////////
// compute width using hybrid method
// from Greg Meymaris

double AparMoments::_computeHybridWidth(double r0,
                                        double r1,
                                        double r2,
                                        double r3,
                                        double nyquist) const
  
{
  
  static const int nps[] = {23,  24, 25,   30,   35,   40,   45,  50,   55,   58,   59,   70,   80,   100,  150,  200,  300};
  static const double high_cutoff[] = {-1, -1, .161, .163, .165, .168, .17, .171, .173, .174, .174, .176, .177, .179, .184, .185, .189};
  static const double low_cutoff[]  = {-1, -1,  -1,   -1,   -1,   -1,   -1,  -1,   -1,   -1,   .073, .074, .072, .073, .073, .074, .074};
  static const int cutoff_len = 17;

  // r0/r1 estimator

  double r0r1 = _computeR0R1Width(r0,r1,nyquist)/nyquist;
  if (_widthMethod == WIDTH_METHOD_R0R1) {
    return r0r1 * nyquist;
  }

  // r1/r2 estimator

  double r1r2 = _computeR1R2Width(r1,r2,nyquist)/nyquist;
  if (_widthMethod == WIDTH_METHOD_R1R2) {
    return r1r2 * nyquist;
  }

  // r1/r3 estimator

  double r1r3 = _computeR1R3Width(r1,r3,nyquist)/nyquist;

  // least-squared r0-r1-r2 estimator
  
  double r0r1r2 = _computePplsWidth(r0,r1,r2,nyquist)/nyquist;

  // compute hybrid estimators

  int table_ind = -1;
  for (int kk = 0; kk < cutoff_len; kk++){
    if (nps[kk]>_nSamples){
      table_ind = kk;
      break;
    }
  }
  if (table_ind == -1)
    table_ind = cutoff_len-1;

  // double hybridLow = (r1r3 + r0r1r2) / 2.0;
  double hybridHigh = (r0r1 + r0r1r2) / 2.0;

  double width = 0;
  if (hybridHigh > high_cutoff[table_ind]) {
    width = r0r1 * nyquist;
  } else if (r1r3 < low_cutoff[table_ind]) {
    width = r1r3 * nyquist;
  } else if (r1r2 != 0) {
    width = r1r2 * nyquist;
  } else {
    width = r0r1r2 * nyquist;
  }
  
#ifdef DEBUG_PRINT
  cerr << "-------------------" << endl;
  cerr << "nyquist: " << _nyquist << endl;
  cerr << "r0, r1, r2, r3: "
       << r0 << ", " << r1 << ", " << r2 << ", " << r3 << endl;
  cerr << "r0r1: " << r0r1 * nyquist << endl;
  cerr << "r1r3: " << r1r3 * nyquist << endl;
  cerr << "r0r1r2: " << r0r1r2 * nyquist << endl;
  cerr << "hybridLow: " << hybridLow * nyquist << endl;
  cerr << "hybridHigh: " << hybridHigh * nyquist << endl;
  cerr << "width: " << width << endl;
#endif

  _constrain(width, 0, nyquist);
  return width;
  
}

/////////////////////////////////////////////////////
// compute width from lags a and b for staggered prt
// using an rA/rB estimator

double AparMoments::_computeStagWidth(double rA,
                                      double rB,
                                      int lagA,
                                      int lagB,
                                      double nyquist) const

{
  
  double factor =
    nyquist / (M_PI * sqrt((lagB * lagB - lagA * lagA) / 2.0));
  
  double rArB = 0;
  if (rA > rB) {
    rArB = sqrt(log(rA / rB));
  }
  double width = rArB * factor;

  return width;

}

/////////////////////////////////////////////////////
// get the calibrated noise power given the channel

double AparMoments::getCalNoisePower(channel_t channel)
  
{

  switch (channel) {
    case CHANNEL_HC:
      return _calNoisePowerHc;
      break;
    case CHANNEL_VC:
      return _calNoisePowerVc;
      break;
    case CHANNEL_HX:
      return _calNoisePowerHx;
      break;
    case CHANNEL_VX:
      return _calNoisePowerVx;
      break;
    default:
      return _calNoisePowerHc;
  }

}

/////////////////////////////////////////////////////
// get the noise power in use given the channel

double AparMoments::getNoisePower(channel_t channel)
  
{

  switch (channel) {
    case CHANNEL_HC:
      return _noisePowerHc;
      break;
    case CHANNEL_VC:
      return _noisePowerVc;
      break;
    case CHANNEL_HX:
      return _noisePowerHx;
      break;
    case CHANNEL_VX:
      return _noisePowerVx;
      break;
    default:
      return _noisePowerHc;
  }

}

//////////////////////////////////////////
// get the receiver gain given the channel

double AparMoments::getReceiverGain(channel_t channel)
  
{

  switch (channel) {
    case CHANNEL_HC:
      return _receiverGainDbHc;
      break;
    case CHANNEL_VC:
      return _receiverGainDbVc;
      break;
    case CHANNEL_HX:
      return _receiverGainDbHx;
      break;
    case CHANNEL_VX:
      return _receiverGainDbVx;
      break;
    default:
      return _receiverGainDbHc;
  }

}

/////////////////////////////////////////////
// get the base dbz at 1km given the channel

double AparMoments::getBaseDbz1km(channel_t channel)
  
{

  switch (channel) {
    case CHANNEL_HC:
      return _baseDbz1kmHc;
      break;
    case CHANNEL_VC:
      return _baseDbz1kmVc;
      break;
    case CHANNEL_HX:
      return _baseDbz1kmHx;
      break;
    case CHANNEL_VX:
      return _baseDbz1kmVx;
      break;
    default:
      return _baseDbz1kmHc;
  }

}

/////////////////////////////////////////////
// compute percentile power value in spectrum

double AparMoments::computePowerPercentile(int nSamples, double *powerSpec,
                                           double percentile)

{

  // create a vector with the power values

  vector<double> powers;
  for (int ii = 0; ii < nSamples; ii++) {
    powers.push_back(powerSpec[ii]);
  }
  sort(powers.begin(), powers.end());

  int index = (int) (percentile * nSamples * 0.01 + 0.5);
  if (index < 0) {
    index = 0;
  } else if (index > nSamples - 1) {
    index = nSamples - 1;
  }

  return powers[index];

}
  
///////////////////////////////////////////////////////////
// De-trend a time series in preparation
// for windowing and FFT.
//
// The end-point linear trend is removed and the residual remains.
//
// This mitigates the raised noise floor problem which can
// occur when a window is applied to a time series with
// ends which are considerably different.
//
// The I and Q series are filtered indendently.
//
// A line is computed between the start and end median values, using
// 3 points for the median.
// The de-trended time series is computed as the redidual difference
// between the original values and the computed line.

void AparMoments::detrendTs(const RadarComplex_t *iq,
                            int nSamples,
                            RadarComplex_t *iqDeTrended)
  
{

  if (nSamples < 6) {
    return;
  }

  // compute the medians at each end
  
  RadarComplex_t medianStart, medianEnd;
  _compute3PtMedian(iq, medianStart);
  _compute3PtMedian(iq + nSamples - 3, medianEnd);
  
  double nIntervals = nSamples - 1.0;
  
  double startI = medianStart.re;
  double endI = medianEnd.re;
  double deltaI = (endI - startI) / nIntervals;

  double startQ = medianStart.im;
  double endQ = medianEnd.im;
  double deltaQ = (endQ - startQ) / nIntervals;

  for (int ii = 0, jj = nSamples - 1; ii < nSamples; ii++, jj--) {
    iqDeTrended[ii].re = iq[ii].re + jj * deltaI - endI;
    iqDeTrended[ii].im = iq[ii].im + jj * deltaQ - endQ;
  }

}

//////////////////////////////////////////////
// compute the 3-pt median for a time series
//
// Assumes iq has space for 3 values.

void AparMoments::_compute3PtMedian(const RadarComplex_t *iq,
                                    RadarComplex_t &median)
  
{

  vector<double> ivals;
  vector<double> qvals;
  
  for (int ii = 0; ii < 3; ii++) {
    ivals.push_back(iq[ii].re);
    qvals.push_back(iq[ii].im);
  }

  sort(ivals.begin(), ivals.end());
  sort(qvals.begin(), qvals.end());

  median.re = ivals[1];
  median.im = qvals[1];

}
  
////////////////////////////////////////////////////////////
// condition SNR - i.e. set a minumum reasonable SNR

double AparMoments::_conditionSnr(double snr)
  
{
  
  if (snr < _minDetectableSnr) {
    return _minDetectableSnr;
  } else {
    return snr;
  }
  
}
  
////////////////////////////////////////////////////////////
// adjust H dbz for measured transmit power as applicable
//
// The received power is adjusted downwards if the measured
// transmit power exceeds the calibrated power,
// and vice versa.

double AparMoments::_adjustDbzForPwrH(double dbz)
  
{

  if (!_adjustDbzForMeasXmitPower) {
    return dbz;
  }

  if (_measXmitPowerDbmH < -9990 || _calibXmitPowerDbmH < -9990) {
    return dbz;
  }
    
  double diff = _measXmitPowerDbmH - _calibXmitPowerDbmH;
  double adjDbz = dbz - diff;
  return adjDbz;

}
  
////////////////////////////////////////////////////////////
// adjust V dbz for measured transmit power as applicable
//
// The received power is adjusted downwards if the measured
// transmit power exceeds the calibrated power,
// and vice versa.

double AparMoments::_adjustDbzForPwrV(double dbz)
  
{

  if (!_adjustDbzForMeasXmitPower) {
    return dbz;
  }

  if (_measXmitPowerDbmV < -9990 || _calibXmitPowerDbmV < -9990) {
    return dbz;
  }
    
  double diff = _measXmitPowerDbmV - _calibXmitPowerDbmV;
  double adjDbz = dbz - diff;
  return adjDbz;

}
  
////////////////////////////////////////////////////////////
// adjust zdr for measured transmit power as applicable
//
// The received power is adjusted downwards if the measured
// transmit power exceeds the calibrated power,
// and vice versa.

double AparMoments::_adjustZdrForPwr(double zdr)
  
{

  if (!_adjustZdrForMeasXmitPower) {
    return zdr;
  }

  if (_measXmitPowerDbmH < -9990 || _calibXmitPowerDbmH < -9990 ||
      _measXmitPowerDbmV < -9990 || _calibXmitPowerDbmV < -9990) {
    return zdr;
  }
    
  double diffH = _measXmitPowerDbmH - _calibXmitPowerDbmH;
  double diffV = _measXmitPowerDbmV - _calibXmitPowerDbmV;

  double adjZdr = zdr - (diffH - diffV);
  return adjZdr;

}
  
///////////////////////////////////////////////////////////
// prepare for noise detection - single polarization
// hc channel
  
void AparMoments::singlePolHNoisePrep(double lag0_hc,
                                      RadarComplex_t lag1_hc,
                                      AparMomFields &fields)
  
{
  
  // lag0 power

  fields.lag0_hc_db = 10.0 * log10(lag0_hc);
  fields.dbm_for_noise = fields.lag0_hc_db;

  // phase for noise detection

  fields.phase_for_noise = lag1_hc;

  // ncp
  
  double lag1_hc_mag = RadarComplex::mag(lag1_hc);
  double ncp = lag1_hc_mag / lag0_hc;
  fields.ncp = _constrain(ncp, 0.0, 1.0);
  
}

///////////////////////////////////////////////////////////
// prepare for noise detection - single polarization
// vc channel
  
void AparMoments::singlePolVNoisePrep(double lag0_vc,
                                      RadarComplex_t lag1_vc,
                                      AparMomFields &fields)
  
{
  
  // lag0 power

  fields.lag0_vc_db = 10.0 * log10(lag0_vc);
  fields.dbm_for_noise = fields.lag0_vc_db;

  // phase for noise detection

  fields.phase_for_noise = lag1_vc;

  // ncp
  
  double lag1_vc_mag = RadarComplex::mag(lag1_vc);
  double ncp = lag1_vc_mag / lag0_vc;
  fields.ncp = _constrain(ncp, 0.0, 1.0);
  
}

///////////////////////////////////////////////////////////
// prepare for noise detection - DP_ALT_HV_CO_ONLY
// Transmit alternating, receive copolar only

void AparMoments::dpAltHvCoOnlyNoisePrep(double lag0_hc,
                                         double lag0_vc,
                                         RadarComplex_t lag2_hc,
                                         RadarComplex_t lag2_vc,
                                         AparMomFields &fields)
  
{
  
  // lag0
  
  fields.lag0_hc_db = 10.0 * log10(lag0_hc);
  fields.lag0_vc_db = 10.0 * log10(lag0_vc);
  fields.dbm_for_noise = (fields.lag0_hc_db + fields.lag0_vc_db) / 2.0;
  
  // phase for noise detection
  
  RadarComplex_t lag2_mean = RadarComplex::complexMean(lag2_hc, lag2_vc);
  fields.phase_for_noise = lag2_mean;

  // ncp - use H and V separately
  
  double lag2_hc_mag = RadarComplex::mag(lag2_hc);
  double lag2_vc_mag = RadarComplex::mag(lag2_vc);
  double ncp =
    ((lag2_hc_mag + lag2_vc_mag) / 2) / sqrt(lag0_hc * lag0_vc);
  fields.ncp = _constrain(ncp, 0.0, 1.0);

}

///////////////////////////////////////////////////////////
// prepare for noise detection - DP_ALT_HV_CO_CROSS
// Transmit alternating, receive co/cross

void AparMoments::dpAltHvCoCrossNoisePrep(double lag0_hc,
                                          double lag0_hx,
                                          double lag0_vc,
                                          double lag0_vx,
                                          RadarComplex_t lag2_hc,
                                          RadarComplex_t lag2_vc,
                                          AparMomFields &fields)
  
{
  
  // lag0

  fields.lag0_hc_db = 10.0 * log10(lag0_hc);
  fields.lag0_hx_db = 10.0 * log10(lag0_hx);
  fields.lag0_vc_db = 10.0 * log10(lag0_vc);
  fields.lag0_vx_db = 10.0 * log10(lag0_vx);
  fields.dbm_for_noise = (fields.lag0_hc_db + fields.lag0_vc_db) / 2.0;

  // phase for noise detection
  
  RadarComplex_t lag2_mean = RadarComplex::complexMean(lag2_hc, lag2_vc);
  fields.phase_for_noise = lag2_mean;

  // ncp - use H and V separately
  
  double lag2_hc_mag = RadarComplex::mag(lag2_hc);
  double lag2_vc_mag = RadarComplex::mag(lag2_vc);
  double ncp =
    ((lag2_hc_mag + lag2_vc_mag) / 2) / sqrt(lag0_hc * lag0_vc);
  fields.ncp = _constrain(ncp, 0.0, 1.0);

}

///////////////////////////////////////////////////////////
// prepare for noise detection - DP_SIM_HV
// Dual pol, transmit simultaneous, receive fixed channels

void AparMoments::dpSimHvNoisePrep(double lag0_hc,
                                   double lag0_vc,
                                   RadarComplex_t lag1_hc,
                                   RadarComplex_t lag1_vc,
                                   AparMomFields &fields)
  
{
  
  // lag0

  fields.lag0_hc_db = 10.0 * log10(lag0_hc);
  fields.lag0_vc_db = 10.0 * log10(lag0_vc);
  fields.dbm_for_noise = (fields.lag0_hc_db + fields.lag0_vc_db) / 2.0;

  // phase for noise detection
  
  RadarComplex_t lag1_mean = RadarComplex::complexMean(lag1_hc, lag1_vc);
  fields.phase_for_noise = lag1_mean;

  RadarComplex_t lag1_sum = RadarComplex::complexSum(lag1_hc, lag1_vc);
  double arg_vel = RadarComplex::argRad(lag1_sum);
  
  double vel = (arg_vel / M_PI) * _nyquist;
  fields.vel = vel * _velSign * -1.0;

  fields.phase_for_noise = lag1_sum;

  // ncp
  
  double ncp = RadarComplex::mag(lag1_sum) / (lag0_hc + lag0_vc);
  fields.ncp = _constrain(ncp, 0.0, 1.0);

}

///////////////////////////////////////////////////////////
// prepare for noise detection - DP_H_ONLY
// Dual pol, transmit H, receive co/cross

void AparMoments::dpHOnlyNoisePrep(double lag0_hc,
                                   double lag0_vx,
                                   RadarComplex_t lag1_hc,
                                   AparMomFields &fields)
  
{
  
  // lag0

  fields.lag0_hc_db = 10.0 * log10(lag0_hc);
  fields.lag0_vx_db = 10.0 * log10(lag0_vx);
  fields.dbm_for_noise = fields.lag0_hc_db;

  // phase for noise detection
  
  fields.phase_for_noise = lag1_hc;

  // ncp
  
  double lag1_hc_mag = RadarComplex::mag(lag1_hc);
  double ncp = lag1_hc_mag / lag0_hc;
  fields.ncp = _constrain(ncp, 0.0, 1.0);
  
}

///////////////////////////////////////////////////////////
// prepare for noise detection - DP_V_ONLY
// Dual pol, transmit V, receive co/cross

void AparMoments::dpVOnlyNoisePrep(double lag0_vc,
                                   double lag0_hx,
                                   RadarComplex_t lag1_vc,
                                   AparMomFields &fields)
  
{
  
  // lag0

  fields.lag0_vc_db = 10.0 * log10(lag0_vc);
  fields.lag0_hx_db = 10.0 * log10(lag0_hx);
  fields.dbm_for_noise = fields.lag0_vc_db;

  // phase for noise detection
  
  fields.phase_for_noise = lag1_vc;

  // ncp
  
  double lag1_vc_mag = RadarComplex::mag(lag1_vc);
  double ncp = lag1_vc_mag / lag0_vc;
  fields.ncp = _constrain(ncp, 0.0, 1.0);
  
}


///////////////////////////////////////////////////////////
// prepare for noise detection
// Single polarization Staggered-PRT

void AparMoments::singlePolHStagPrtNoisePrep(RadarComplex_t *iqhc,
                                             RadarComplex_t *iqhcShort,
                                             RadarComplex_t *iqhcLong,
                                             AparMomFields &fields)
  
{
  

  // lag0
  
  double lag0_hc = RadarComplex::meanPower(iqhcLong, _nSamplesHalf - 1);
  fields.lag0_hc_db = 10.0 * log10(lag0_hc);
  fields.dbm_for_noise = fields.lag0_hc_db;

  // phase for noise detection
  
  RadarComplex_t lag1_short_to_long =
    RadarComplex::meanConjugateProduct(iqhcShort, iqhcLong,
                                       _nSamplesHalf - 1);
  fields.phase_for_noise = lag1_short_to_long;

  // ncp
  
  double lag1_hc_mag = RadarComplex::mag(lag1_short_to_long);
  double ncp = lag1_hc_mag / lag0_hc;
  fields.ncp = _constrain(ncp, 0.0, 1.0);
  
}

///////////////////////////////////////////////////////////
// prepare for noise detection
// Dual pol, transmit simultaneous, receive fixed channels
// Staggered-PRT

void AparMoments::dpSimHvStagPrtNoisePrep(RadarComplex_t *iqhc,
                                          RadarComplex_t *iqvc,
                                          RadarComplex_t *iqhcShort,
                                          RadarComplex_t *iqvcShort,
                                          RadarComplex_t *iqhcLong,
                                          RadarComplex_t *iqvcLong,
                                          AparMomFields &fields)
  
{
  
  // lag0
  
  double lag0_hc_long = RadarComplex::meanPower(iqhcLong, _nSamplesHalf - 1);
  double lag0_vc_long = RadarComplex::meanPower(iqvcLong, _nSamplesHalf - 1);
  
  fields.lag0_hc_db = 10.0 * log10(lag0_hc_long);
  fields.lag0_vc_db = 10.0 * log10(lag0_vc_long);
  double lag0_mean_db = (fields.lag0_hc_db + fields.lag0_vc_db) / 2.0;
  fields.dbm_for_noise = lag0_mean_db;

  // phase for noise detection
  
  RadarComplex_t lag1_hc_short_to_long =
    RadarComplex::meanConjugateProduct(iqhcShort, iqhcLong, _nSamplesHalf - 1);
  RadarComplex_t lag1_vc_short_to_long =
    RadarComplex::meanConjugateProduct(iqvcShort, iqvcLong, _nSamplesHalf - 1);
  RadarComplex_t lag1_mean_short_to_long =
    RadarComplex::complexMean(lag1_hc_short_to_long, lag1_vc_short_to_long);
  fields.phase_for_noise = lag1_mean_short_to_long;

  // ncp
  
  double lag1_mag = RadarComplex::mag(lag1_mean_short_to_long);
  double lag0_mean = (lag0_hc_long + lag0_vc_long) / 2.0;
  double ncp = lag1_mag / lag0_mean;
  fields.ncp = _constrain(ncp, 0.0, 1.0);

}

///////////////////////////////////////////////////////////
// allocate range correction table

void AparMoments::_allocRangeCorr()

{

  _rangeCorr = new double[_maxGates];
  memset(_rangeCorr, 0, _maxGates * sizeof(double));
  _rangeCorrInit = false;

}

///////////////////////////////////////////////////////////
// allocate atmos atten corr table

void AparMoments::_allocAtmosAttenCorr()

{

  _atmosAttenCorr = new double[_maxGates];
  memset(_atmosAttenCorr, 0, _maxGates * sizeof(double));

}

