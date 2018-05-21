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
// RadarMoments.cc
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// August 2007
//
///////////////////////////////////////////////////////////////
//
// RadarMoments computes moments at a gate
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
#include <radar/ClutFilter.hh>
#include <radar/RadarMoments.hh>

const double RadarMoments::_missing = MomentsFields::missingDouble;
const double RadarMoments::_phidpPhaseLimitAlt = -70;
const double RadarMoments::_phidpPhaseLimitSim = -160;
const double RadarMoments::_minDetectableSnr = 0.01; // -20 dB

// coeffs for computing least squares fit for width

const double RadarMoments::_c1 = -0.192307692307692;
const double RadarMoments::_c2 = -0.076923076923077;
const double RadarMoments::_c3 = 0.269230769230769;

// C version of cpa_alt

#include "cpa.c"
  
////////////////////////////////////////////////////
// Default constructor

RadarMoments::RadarMoments() :
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

RadarMoments::RadarMoments(int max_gates,
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

RadarMoments::~RadarMoments()

{

  if (_rangeCorr) {
    delete[] _rangeCorr;
  }

  if (_atmosAttenCorr) {
    delete[] _atmosAttenCorr;
  }

  if (_sz) {
    delete _sz;
  }

}

////////////////////////////////////////////////////
// construct members

void RadarMoments::_init()
  
{
  
  // initialize geometry

  _nSamples = 64;
  _nSamplesHalf = _nSamples / 2;

  _applySpectralResidueCorrection = false;
  _minSnrDbForResidueCorrection = 80.0;
  _applyDbForDbCorrection = false;
  _dbForFbRatio = 0.0;
  _dbForDbThreshold = 0.0;

  _minSnrDbForZdr = _missing;
  _minSnrDbForLdr = _missing;

  _correctForSystemPhidp = false;
  _changeAiqSign = false;
  _computeCpaUsingAlt = false;

  _useRegressionFilter = false;
  _regrInterpAcrossNotch = true;

  _useSimpleNotchFilter = false;
  _notchWidthMps = 3.0;

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

  _estNoisePowerHc = _missing;
  _estNoisePowerHx = _missing;
  _estNoisePowerVc = _missing;
  _estNoisePowerVx = _missing;

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

  _applySz = false;
  _sz = NULL;

  _tssNotchWidth = 3;
  
}

////////////////////////////////////////////////////
// set max number of gates

void RadarMoments::setMaxGates(int max_gates)
  
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

void RadarMoments::setNSamples(int n)

{

  _nSamples = n;
  _nSamplesHalf = n / 2;

}
  
/////////////////////////////////////////////////////
// set SZ
// This will cause SZ864 processing to be applied

void RadarMoments::setSz(double snr_threshold,
                         bool negate_phase_codes,
                         double strong_to_weak_power_ratio_threshold,
                         double out_of_trip_power_ratio_threshold,
                         int out_of_trip_power_n_replicas)

{

  _applySz = true;
  _sz = new Sz864(_verbose);

  _sz->setSignalToNoiseRatioThreshold(snr_threshold);
    
  if (negate_phase_codes) {
    _sz->setSzNegatePhaseCodes();
  }
  
  _sz->setSzStrongToWeakPowerRatioThreshold
    (strong_to_weak_power_ratio_threshold);

  _sz->setSzOutOfTripPowerRatioThreshold
    (out_of_trip_power_ratio_threshold);

  _sz->setSzOutOfTripPowerNReplicas
    (out_of_trip_power_n_replicas);
  
}

/////////////////////////////////////////////////////
// Set dB for dB correction in clutter processing.
// This will turn on the dB for dB correction.

void RadarMoments::setDbForDb(double db_for_db_ratio,
                              double db_for_db_threshold)
  
{
  
  _applyDbForDbCorrection = true;
  _dbForFbRatio = db_for_db_ratio;
  _dbForDbThreshold = db_for_db_threshold;
  
}

///////////////////////////////////////////////////////////
// Compute covariances
// Single polarization
// Assumes the data is in the horizontal channel

void RadarMoments::computeCovarSinglePol(RadarComplex_t *iqhc,
                                         MomentsFields &fields)
  
{
  
  // covariances

  fields.lag0_hc = RadarComplex::meanPower(iqhc, _nSamples);

  fields.lag1_hc =
    RadarComplex::meanConjugateProduct(iqhc + 1, iqhc, _nSamples - 1);
  
  fields.lag2_hc =
    RadarComplex::meanConjugateProduct(iqhc + 2, iqhc, _nSamples - 2);

  fields.lag3_hc =
    RadarComplex::meanConjugateProduct(iqhc + 3, iqhc, _nSamples - 3);
  
  // refractivity
  
  computeRefract(iqhc, _nSamples, fields.aiq_hc, fields.niq_hc, _changeAiqSign);
  
  // CPA
  
  if (_computeCpaUsingAlt) {
    fields.cpa = computeCpaAlt(iqhc, _nSamples);
  } else {
    fields.cpa = computeCpa(iqhc, _nSamples);
  }

}

///////////////////////////////////////////////////////////
// Compute covariances
// DP_ALT_HV_CO_ONLY
// Transmit alternating, receive copolar only
// IQ data passed in

void RadarMoments::computeCovarDpAltHvCoOnly(RadarComplex_t *iqhc,
                                             RadarComplex_t *iqvc,
                                             MomentsFields &fields)
  
{
  
  // covariances

  fields.lag0_hc = RadarComplex::meanPower(iqhc, _nSamplesHalf - 1);
  fields.lag0_vc = RadarComplex::meanPower(iqvc, _nSamplesHalf - 1);
  
  fields.lag1_vchc =
    RadarComplex::meanConjugateProduct(iqvc, iqhc, _nSamplesHalf - 1);
  
  fields.lag1_hcvc =
    RadarComplex::meanConjugateProduct(iqhc + 1, iqvc, _nSamplesHalf - 1);
  
  fields.lag2_hc =
    RadarComplex::meanConjugateProduct(iqhc + 1, iqhc, _nSamplesHalf - 1);
  
  fields.lag2_vc =
    RadarComplex::meanConjugateProduct(iqvc + 1, iqvc, _nSamplesHalf - 1);
  
  // refractivity
  
  computeRefract(iqhc, _nSamplesHalf, fields.aiq_hc, fields.niq_hc, _changeAiqSign);
  computeRefract(iqvc, _nSamplesHalf, fields.aiq_vc, fields.niq_vc, _changeAiqSign);
  
  // CPA
  
  if (_computeCpaUsingAlt) {
    fields.cpa = computeCpaAlt(iqhc, iqvc, _nSamplesHalf);
  } else {
    fields.cpa = computeCpa(iqhc, iqvc, _nSamplesHalf);
  }
  
  // sdev of VV time series
  
  fields.sdev_vv = computeMagSdev(iqvc, _nSamplesHalf);
  
}

///////////////////////////////////////////////////////////
// Compute covariances
// DP_ALT_HV_CO_CROSS
// Transmit alternating, receive co/cross

void RadarMoments::computeCovarDpAltHvCoCross(RadarComplex_t *iqhc,
                                              RadarComplex_t *iqvc,
                                              RadarComplex_t *iqhx,
                                              RadarComplex_t *iqvx,
                                              MomentsFields &fields)
  
{
  
  // covariances
  
  fields.lag0_hc = RadarComplex::meanPower(iqhc, _nSamplesHalf - 1);
  fields.lag0_hx = RadarComplex::meanPower(iqhx, _nSamplesHalf - 1);
  fields.lag0_vc = RadarComplex::meanPower(iqvc, _nSamplesHalf - 1);
  fields.lag0_vx = RadarComplex::meanPower(iqvx, _nSamplesHalf - 1);

  // compute lag1 co-polar correlation V to H
  
  fields.lag1_vchc =
    RadarComplex::meanConjugateProduct(iqvc, iqhc, _nSamplesHalf - 1);
  
  // compute lag1 co-polar correlation H to V
  
  fields.lag1_hcvc =
    RadarComplex::meanConjugateProduct(iqhc + 1, iqvc, _nSamplesHalf - 1);

  // compute lag0 cross-polar correlation Hc to Vx
  
  fields.lag0_hcvx =
    RadarComplex::meanConjugateProduct(iqhc, iqvx, _nSamplesHalf - 1);
  
  // compute lag0 cross-polar correlation Vc to Hx
  
  fields.lag0_vchx =
    RadarComplex::meanConjugateProduct(iqvc, iqhx, _nSamplesHalf - 1);
  
  // compute lag0 cross-polar correlation Vx to Hx
  
  fields.lag1_vxhx =
    RadarComplex::meanConjugateProduct(iqvx, iqhx, _nSamplesHalf - 1);
  
  // lag-2 correlations for HH and VV
  
  fields.lag2_hc =
    RadarComplex::meanConjugateProduct(iqhc + 1, iqhc, _nSamplesHalf - 1);
  
  fields.lag2_vc =
    RadarComplex::meanConjugateProduct(iqvc + 1, iqvc, _nSamplesHalf - 1);
  
  // refractivity
  
  computeRefract(iqhc, _nSamplesHalf, 
                 fields.aiq_hc, fields.niq_hc, _changeAiqSign);
  computeRefract(iqvc, _nSamplesHalf,
                 fields.aiq_vc, fields.niq_vc, _changeAiqSign);
  
  // CPA
  
  if (_computeCpaUsingAlt) {
    fields.cpa = computeCpaAlt(iqhc, iqvc, _nSamplesHalf);
  } else {
    fields.cpa = computeCpa(iqhc, iqvc, _nSamplesHalf);
  }
  
  // CPR
  
  double cprPowerDb, cprPhaseDeg;
  computeCpr(iqhc, iqvx, _nSamplesHalf, cprPowerDb, cprPhaseDeg);
  fields.cpr_mag = cprPowerDb;
  fields.cpr_phase = cprPhaseDeg;
  fields.cpr_ldr = pow(10.0, (fields.cpr_mag - fields.ldrh) / 10.0);
  
  // sdev of VV time series

  fields.sdev_vv = computeMagSdev(iqvc, _nSamplesHalf);

}

///////////////////////////////////////////////////////////
// Compute covariances
// DP_SIM_HV
// Dual pol, transmit simultaneous, receive fixed channels 

void RadarMoments::computeCovarDpSimHv(RadarComplex_t *iqhc,
                                       RadarComplex_t *iqvc,
                                       MomentsFields &fields)
  
{
  
  // compute covariances
  
  fields.lag0_hc = RadarComplex::meanPower(iqhc, _nSamples - 1);
  fields.lag0_vc = RadarComplex::meanPower(iqvc, _nSamples - 1);

  fields.rvvhh0 =
    RadarComplex::meanConjugateProduct(iqvc, iqhc, _nSamples - 1);

  fields.lag1_hc =
    RadarComplex::meanConjugateProduct(iqhc + 1, iqhc, _nSamples - 1);

  fields.lag1_vc =
    RadarComplex::meanConjugateProduct(iqvc + 1, iqvc, _nSamples - 1);
  
  fields.lag2_hc =
    RadarComplex::meanConjugateProduct(iqhc + 2, iqhc, _nSamples - 2);

  fields.lag2_vc =
    RadarComplex::meanConjugateProduct(iqvc + 2, iqvc, _nSamples - 2);

  fields.lag3_hc =
    RadarComplex::meanConjugateProduct(iqhc + 3, iqhc, _nSamples - 3);

  fields.lag3_vc =
    RadarComplex::meanConjugateProduct(iqvc + 3, iqvc, _nSamples - 3);

  // refractivity
  
  computeRefract(iqhc, _nSamples, fields.aiq_hc, fields.niq_hc, _changeAiqSign);
  computeRefract(iqvc, _nSamples, fields.aiq_vc, fields.niq_vc, _changeAiqSign);
  
  // CPA
  
  if (_computeCpaUsingAlt) {
    fields.cpa = computeCpaAlt(iqhc, iqvc, _nSamples);
  } else {
    fields.cpa = computeCpa(iqhc, iqvc, _nSamples);
  }
  
  // sdev of VV time series
  
  fields.sdev_vv = computeMagSdev(iqvc, _nSamples);

}

///////////////////////////////////////////////////////////
// Compute covariances
// Dual pol, transmit H, receive co/cross

void RadarMoments::computeCovarDpHOnly(RadarComplex_t *iqhc,
                                       RadarComplex_t *iqvx,
                                       MomentsFields &fields)
  
{
  
  // compute covariances
  
  fields.lag0_hc = RadarComplex::meanPower(iqhc, _nSamples - 1);
  fields.lag0_vx = RadarComplex::meanPower(iqvx, _nSamples - 1);
  
  fields.lag1_hc =
    RadarComplex::meanConjugateProduct(iqhc + 1, iqhc, _nSamples - 1);
  
  fields.lag2_hc =
    RadarComplex::meanConjugateProduct(iqhc + 2, iqhc, _nSamples - 2);
  
  fields.lag3_hc =
    RadarComplex::meanConjugateProduct(iqhc + 3, iqhc, _nSamples - 3);
  
  // refractivity
  
  computeRefract(iqhc, _nSamples, fields.aiq_hc, fields.niq_hc, _changeAiqSign);
  
  // CPA
  
  if (_computeCpaUsingAlt) {
    fields.cpa = computeCpaAlt(iqhc, _nSamples);
  } else {
    fields.cpa = computeCpa(iqhc, _nSamples);
  }
  
}

///////////////////////////////////////////////////////////
// Compute covariances
// Dual pol, transmit V, receive co/cross

void RadarMoments::computeCovarDpVOnly(RadarComplex_t *iqvc,
                                       RadarComplex_t *iqhx,
                                       MomentsFields &fields)
  
{
  
  // compute covariances
  
  fields.lag0_vc = RadarComplex::meanPower(iqvc, _nSamples - 1);
  fields.lag0_hx = RadarComplex::meanPower(iqhx, _nSamples - 1);

  fields.lag1_vc =
    RadarComplex::meanConjugateProduct(iqvc + 1, iqvc, _nSamples - 1);

  fields.lag2_vc =
    RadarComplex::meanConjugateProduct(iqvc + 2, iqvc, _nSamples - 2);

  fields.lag3_vc =
    RadarComplex::meanConjugateProduct(iqvc + 3, iqvc, _nSamples - 3);

  // refractivity
  
  computeRefract(iqvc, _nSamples, fields.aiq_vc, fields.niq_vc, _changeAiqSign);
  
  // CPA
  
  if (_computeCpaUsingAlt) {
    fields.cpa = computeCpaAlt(iqvc, _nSamples);
  } else {
    fields.cpa = computeCpa(iqvc, _nSamples);
  }
  
}

///////////////////////////////////////////////////////////
// Single polarization
// Assumes the data is in the horizontal channel
// IQ passed in

void RadarMoments::singlePol(RadarComplex_t *iqhc,
                             int gateNum,
                             bool isFiltered,
                             MomentsFields &fields)
  
{
  
  // initialize field meta data

  _setFieldMetaData(fields);

  // compute lag covariances
  
  double lag0_hc = RadarComplex::meanPower(iqhc, _nSamples);

  RadarComplex_t lag1_hc =
    RadarComplex::meanConjugateProduct(iqhc + 1, iqhc, _nSamples - 1);
  
  RadarComplex_t lag2_hc =
    RadarComplex::meanConjugateProduct(iqhc + 2, iqhc, _nSamples - 2);

  RadarComplex_t lag3_hc =
    RadarComplex::meanConjugateProduct(iqhc + 3, iqhc, _nSamples - 3);
  
  // compute moments from covariances

  computeMomSinglePol(lag0_hc, lag1_hc, lag2_hc, lag3_hc,
                      gateNum, fields);

  if (!isFiltered) {
    
    // refractivity
    
    computeRefract(iqhc, _nSamples, fields.aiq_hc, fields.niq_hc, _changeAiqSign);

    // CPA

    if (_computeCpaUsingAlt) {
      fields.cpa = computeCpaAlt(iqhc, _nSamples);
    } else {
      fields.cpa = computeCpa(iqhc, _nSamples);
    }

  }
  
}

///////////////////////////////////////////////////////////
// Single polarization
// Assumes the data is in the horizontal channel
// covariances passed in
  
void RadarMoments::computeMomSinglePol(double lag0_hc,
                                       RadarComplex_t lag1_hc,
                                       RadarComplex_t lag2_hc,
                                       RadarComplex_t lag3_hc,
                                       int gateNum,
                                       MomentsFields &fields)
  
{

  _setFieldMetaData(fields);

  // lag0
  
  fields.lag0_hc_db = 10.0 * log10(lag0_hc);

  // compute dbm
  
  double dbm_hc = 10.0 * log10(lag0_hc) - _receiverGainDbHc;
  fields.dbmhc = dbm_hc;
  fields.dbm = dbm_hc;

  // compute noise-subtracted lag0
  
  double lag0_hc_ns = lag0_hc - _estNoisePowerHc;
  
  bool snrHcOK = true;
  double min_valid_pwr_hc = _estNoisePowerHc * _minDetectableSnr;
  if (lag0_hc_ns < min_valid_pwr_hc) {
    snrHcOK = false;
    fields.censoring_flag = 1;
  }
  
  // snr & dbz
  
  if (snrHcOK) {
    
    double snr_hc = lag0_hc_ns / _calNoisePowerHc;
    double snrDb = 10.0 * log10(snr_hc);

    fields.dbmhc_ns = 10.0 * log10(lag0_hc_ns) - _receiverGainDbHc;
    fields.snrhc = snrDb;
    fields.snr = snrDb;

    double dbz_hc_no_atten_corr = 
      10.0 * log10(snr_hc) + _baseDbz1kmHc + _rangeCorr[gateNum] + _dbzCorrection;
    double dbz_hc = dbz_hc_no_atten_corr + _atmosAttenCorr[gateNum];
    fields.dbzhc = _adjustDbzForPwrH(dbz_hc);
    fields.dbz = fields.dbzhc;
    fields.dbz_no_atmos_atten = _adjustDbzForPwrH(dbz_hc_no_atten_corr);
    
  } else {

    fields.dbmhc_ns = _missing;
    fields.snrhc = _missing;
    fields.snr = _missing;
    fields.dbz = _missing;
    fields.dbzhc = _missing;
    fields.dbz_no_atmos_atten = _missing;

  }
  
  // velocity

  double lag1_hc_mag = RadarComplex::mag(lag1_hc);
  double argVel = RadarComplex::argRad(lag1_hc);

  fields.lag1_hc_db = 20.0 * log10(lag1_hc_mag);
  fields.lag1_hc_phase = argVel * RAD_TO_DEG;
  
  double vel = (argVel / M_PI) * _nyquist;
  fields.vel = vel * _velSign * -1.0;

  fields.phase_for_noise = lag1_hc;

  // ncp
  
  double ncp = lag1_hc_mag / lag0_hc;
  ncp = _constrain(ncp, 0.0, 1.0);
  fields.ncp = ncp;
  
  // width
  
  double lag2_hc_mag = RadarComplex::mag(lag2_hc);
  fields.lag2_hc_db = 20.0 * log10(lag2_hc_mag);
  fields.lag2_hc_phase = RadarComplex::argDeg(lag2_hc);
  
  double lag3_hc_mag = RadarComplex::mag(lag3_hc);
  fields.lag3_hc_db = 20.0 * log10(lag3_hc_mag);
  fields.lag3_hc_phase = RadarComplex::argDeg(lag3_hc);

  double r1 = lag1_hc_mag / _windowR1;
  double r2 = lag2_hc_mag / _windowR2;
  double r3 = lag3_hc_mag / _windowR3;
  
  double r1r2 = _computeR1R2Width(r1,r2,_nyquist);
  fields.width_r1r2 = _constrain(r1r2, 0.01, _nyquist);
  double r1r3 = _computeR1R3Width(r1,r3,_nyquist);
  fields.width_r1r3 = _constrain(r1r3, 0.01, _nyquist);

  // default to R1R2

  fields.width = fields.width_r1r2;

  if (snrHcOK) {

    double r0 = lag0_hc_ns;
    
    double r0r1 = _computeR0R1Width(r0,r1,_nyquist);
    fields.width_r0r1 = _constrain(r0r1, 0.01, _nyquist);
    double r0r1r2 = _computePplsWidth(r0,r1,r2,_nyquist);
    fields.width_ppls = _constrain(r0r1r2, 0.01, _nyquist);
    
    if (_widthMethod == WIDTH_METHOD_R0R1) {
      fields.width = fields.width_r0r1;
    }

    if (_widthMethod == WIDTH_METHOD_HYBRID) {
      double width = _computeHybridWidth(r0, r1, r2, r3, _nyquist);
      fields.width = _constrain(width, 0.01, _nyquist);
    }

  }

}

///////////////////////////////////////////////////////////
// Compute reflectivity only
  
double RadarMoments::computeDbz(const RadarComplex_t *iq,
                                int nSamples,
                                double noisePower,
                                double baseDbz1km,
                                int gateNum)
  
{

  // compute lag0 power

  double lag0 = RadarComplex::meanPower(iq, nSamples);
  
  // compute noise-subtracted lag0
  
  double lag0_ns = lag0 - noisePower;
  if (lag0_ns < 0) {
    return _missing;
  }
  
  // compute reflectivity from snr and range correction

  double snr = lag0_ns / noisePower;
  double dbz = (10.0 * log10(snr) + baseDbz1km + 
                _rangeCorr[gateNum] + _atmosAttenCorr[gateNum]);
  dbz += _dbzCorrection;
  return dbz;

}

///////////////////////////////////////////////////////////
// DP_ALT_HV_CO_ONLY
// Transmit alternating, receive copolar only
// IQ data passed in

void RadarMoments::dpAltHvCoOnly(RadarComplex_t *iqhc,
                                 RadarComplex_t *iqvc,
                                 int gateNum,
                                 bool isFiltered,
                                 MomentsFields &fields)
  
{
  
  // initialize field meta data
  
  _setFieldMetaData(fields);
  
  // compute covariances
  
  fields.lag0_hc = RadarComplex::meanPower(iqhc, _nSamplesHalf - 1);
  fields.lag0_vc = RadarComplex::meanPower(iqvc, _nSamplesHalf - 1);
  
  fields.lag1_vchc =
    RadarComplex::meanConjugateProduct(iqvc, iqhc, _nSamplesHalf - 1);
  
  fields.lag1_hcvc =
    RadarComplex::meanConjugateProduct(iqhc + 1, iqvc, _nSamplesHalf - 1);
  
  fields.lag2_hc =
    RadarComplex::meanConjugateProduct(iqhc + 1, iqhc, _nSamplesHalf - 1);
  
  fields.lag2_vc =
    RadarComplex::meanConjugateProduct(iqvc + 1, iqvc, _nSamplesHalf - 1);

  _setFieldMetaData(fields);
  
  // compute moments from covariances
  
  computeMomDpAltHvCoOnly(fields.lag0_hc, fields.lag0_vc,
                          fields.lag1_vchc, fields.lag1_hcvc,
                          fields.lag2_hc, fields.lag2_vc,
                          gateNum, fields);
  
  if (!isFiltered) {
    
    // refractivity
    
    computeRefract(iqhc, _nSamplesHalf, fields.aiq_hc, fields.niq_hc, _changeAiqSign);
    computeRefract(iqvc, _nSamplesHalf, fields.aiq_vc, fields.niq_vc, _changeAiqSign);
      
    // CPA
      
    if (_computeCpaUsingAlt) {
      fields.cpa = computeCpaAlt(iqhc, iqvc, _nSamplesHalf);
    } else {
      fields.cpa = computeCpa(iqhc, iqvc, _nSamplesHalf);
    }
      
  }
  
  // sdev of VV time series
  
  fields.sdev_vv = computeMagSdev(iqvc, _nSamplesHalf);
  
}

///////////////////////////////////////////////////////////
// DP_ALT_HV_CO_ONLY
// Transmit alternating, receive copolar only
// covariances passed in

void RadarMoments::computeMomDpAltHvCoOnly(double lag0_hc,
                                           double lag0_vc,
                                           RadarComplex_t lag1_vchc,
                                           RadarComplex_t lag1_hcvc,
                                           RadarComplex_t lag2_hc,
                                           RadarComplex_t lag2_vc,
                                           int gateNum,
                                           MomentsFields &fields)
  
{
  
  _setFieldMetaData(fields);

  // lag0

  fields.lag0_hc_db = 10.0 * log10(lag0_hc);
  fields.lag0_vc_db = 10.0 * log10(lag0_vc);

  // compute dbm
  
  double dbm_hc = 10.0 * log10(lag0_hc) - _receiverGainDbHc;
  double dbm_vc = 10.0 * log10(lag0_vc) - _receiverGainDbVc;
  
  fields.dbmhc = dbm_hc;
  fields.dbmvc = dbm_vc;
  fields.dbm = (dbm_hc + dbm_vc) / 2.0;

  // compute noise-subtracted lag0
  
  double lag0_hc_ns = lag0_hc - _estNoisePowerHc;
  double lag0_vc_ns = lag0_vc - _estNoisePowerVc;
  
  // check SNR
  
  bool snrHcOK = true;
  double min_valid_pwr_hc = _estNoisePowerHc * _minDetectableSnr;
  if (lag0_hc_ns < min_valid_pwr_hc) {
    snrHcOK = false;
    fields.censoring_flag = 1;
  }
  bool snrVcOK = true;
  double min_valid_pwr_vc = _estNoisePowerVc * _minDetectableSnr;
  if (lag0_vc_ns < min_valid_pwr_vc) {
    snrVcOK = false;
    fields.censoring_flag = 1;
  }
  
  // compute snr
  
  double snr_hc = lag0_hc_ns / _calNoisePowerHc;
  double snr_vc = lag0_vc_ns / _calNoisePowerVc;
  
  if (snrHcOK) {
    fields.dbmhc_ns = 10.0 * log10(lag0_hc_ns) - _receiverGainDbHc;
    fields.snrhc = 10.0 * log10(snr_hc);
  } else {
    fields.dbmhc_ns = _missing;
    fields.snrhc = _missing;
  }
  if (snrVcOK) {
    fields.dbmvc_ns = 10.0 * log10(lag0_vc_ns) - _receiverGainDbVc;
    fields.snrvc = 10.0 * log10(snr_vc);
  } else {
    fields.dbmvc_ns = _missing;
    fields.snrvc = _missing;
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
    
  ////////////////////////////////////////////////////
  // phidp, velocity
  //
  // See A. Zahrai and D. Zrnic
  // "The 10 cm wavelength polarimetric weather radar
  // at NOAA'a National Severe Storms Lab. "
  // JTech, Vol 10, No 5, October 1993.
  
  double mag_lag1_vchc = RadarComplex::mag(lag1_vchc);
  double mag_lag1_hcvc = RadarComplex::mag(lag1_hcvc);
  double arg_lag1_vchc = RadarComplex::argRad(lag1_vchc);
  double arg_lag1_hcvc = RadarComplex::argRad(lag1_hcvc);

  fields.lag1_vchc_db = 20.0 * log10(mag_lag1_vchc);
  fields.lag1_vchc_phase = arg_lag1_vchc * RAD_TO_DEG;
  fields.lag1_hcvc_db = 20.0 * log10(mag_lag1_hcvc);
  fields.lag1_hcvc_phase = arg_lag1_hcvc * RAD_TO_DEG;

  // compute system phidp

  RadarComplex_t phidp0 = RadarComplex::conjugateProduct(lag1_vchc, lag1_hcvc);
  double phidpRad0 = RadarComplex::argRad(phidp0) / 2.0;
  fields.phidp0 = phidpRad0 * RAD_TO_DEG * _velSign * _phidpSign;

  // Compute phi H and V, correcting for system PhiDp offset
  // so that phidp will not wrap prematurely
  
  RadarComplex_t phi_h =
    RadarComplex::complexProduct(lag1_vchc, _phidpOffsetAlt);
  RadarComplex_t phi_v =
    RadarComplex::conjugateProduct(lag1_hcvc, _phidpOffsetAlt);

  RadarComplex_t prod = RadarComplex::conjugateProduct(phi_v, phi_h);
  double phi = -0.5 * RadarComplex::argRad(prod);
  fields.phidp = phi * RAD_TO_DEG * _velSign * _phidpSign;

  double psi_h = RadarComplex::argRad(phi_h) - phi;
  double psi_v = RadarComplex::argRad(phi_v) + phi;
  if (psi_h < -M_PI) psi_h += 2.0 * M_PI; 
  if (psi_h >  M_PI) psi_h -= 2.0 * M_PI; 
  if (psi_v < -M_PI) psi_v += 2.0 * M_PI; 
  if (psi_v >  M_PI) psi_v -= 2.0 * M_PI; 

  // compute angular difference between them, which is phidp
  
  // RadarComplex_t phidp = RadarComplex::conjugateProduct(phi_h, phi_v);
  // double phidpRad = RadarComplex::argRad(phidp) / 2.0;
  // fields.phidp = phidpRad * RAD_TO_DEG * _velSign * _phidpSign;

  // velocity phase is mean of phidp phases

  // RadarComplex_t velVect = RadarComplex::complexMean(phi_h, phi_v);
  RadarComplex_t velVect = RadarComplex::complexMean(psi_h, psi_v);
  double argVel = RadarComplex::argRad(velVect);
  double meanVel = (argVel / M_PI) * _nyquist;
  fields.vel = meanVel * _velSign * -1.0;
  fields.vel_alt = fields.vel;
  
  // also compute velocity which can be used in clutter

  double argVelH = RadarComplex::argRad(lag2_hc);
  fields.vel_h_only = (argVelH / M_PI) * (_nyquist / 2.0) * _velSign * -1.0;
  
  double argVelV = RadarComplex::argRad(lag2_vc);
  fields.vel_v_only = (argVelV / M_PI) * (_nyquist / 2.0) * _velSign * -1.0;

  // compute alt mode velocity

  RadarComplex_t lag2_sum = RadarComplex::complexSum(lag2_hc, lag2_vc);
  double arg_vel2 = RadarComplex::argRad(lag2_sum);
  
  double vel_hv = (arg_vel2 / M_PI) * (_nyquist / 2.0);
  fields.vel_hv = vel_hv * _velSign * -1.0;

  fields.phase_for_noise = lag2_sum;

  /////////////////////////
  // width, rhohv
  // noise corrected
  
  if (snrHcOK && snrVcOK) {
    
    // compute lag-2 rho and ncp
    
    double mean_lag0_ns = (lag0_hc_ns + lag0_vc_ns) / 2.0;
    if (mean_lag0_ns <= 0) {
      mean_lag0_ns = 1.0e-20;
    }
    RadarComplex_t sumR2 = RadarComplex::complexSum(lag2_hc, lag2_vc);
    double meanMagR2 = RadarComplex::mag(sumR2) / 2.0;
    double rho2 = meanMagR2 / mean_lag0_ns;
    
    // spectrum width from H and V
    
    double argWidth = sqrt(-0.5 * log(rho2));
    double width = (argWidth / M_PI) * _nyquist;
    fields.width = _constrain(width, 0.01, _nyquist);
    
    // width using H and V separately
  
    double lag2_hc_mag = RadarComplex::mag(lag2_hc);
    double arg_lag2Hc = RadarComplex::argRad(lag2_hc);
    fields.lag2_hc_db = 20.0 * log10(lag2_hc_mag);
    fields.lag2_hc_phase = arg_lag2Hc * RAD_TO_DEG;
    
    double lag2_vc_mag = RadarComplex::mag(lag2_vc);
    double arg_lag2Vc = RadarComplex::argRad(lag2_vc);
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
    
    double rhohv1 =
      (mag_lag1_vchc + mag_lag1_hcvc) / (2.0 * sqrt(lag0_hc_ns * lag0_vc_ns));
    
    // lag-0 rhohv is rhohv1 corrected by rho2
    
    double rhohv0 = rhohv1 / pow(rho2, 0.25);
    fields.rhohv = _constrain(rhohv0, 0.0, 1.0);
    
  } // if (snrHcOK && snrVcOK)
  
  /////////////////////////////
  // ncp - not noise corrected
  
  {

    double mean_lag0 = (lag0_hc + lag0_vc) / 2.0;
    RadarComplex_t sumR2 = RadarComplex::complexSum(lag2_hc, lag2_vc);
    double meanMagR2 = RadarComplex::mag(sumR2) / 2.0;
    double rho2 = meanMagR2 / mean_lag0;
    fields.ncp = _constrain(rho2, 0.0, 1.0);
    
    // ncp using H and V separately
    
    double lag2_hc_mag = RadarComplex::mag(lag2_hc);
    fields.ncp_h_only = _constrain(lag2_hc_mag / lag0_hc, 0.0, 1.0);
    
    double lag2_vc_mag = RadarComplex::mag(lag2_vc);
    fields.ncp_v_only = _constrain(lag2_vc_mag / lag0_vc, 0.0, 1.0);

    fields.ncp_h_minus_v = fields.ncp_h_only - fields.ncp_v_only;

  } // if (snrHcOK && snrVcOK)
  
  ////////////////////////////
  // not-noise-corrected rhohv
  
  {

    // compute lag-2 rho
    
    double mean_lag0 = (lag0_hc + lag0_vc) / 2.0;
    RadarComplex_t sumR2 = RadarComplex::complexSum(lag2_hc, lag2_vc);
    double meanMagR2 = RadarComplex::mag(sumR2) / 2.0;
    double rho2 = meanMagR2 / mean_lag0;
    // rho2 = _constrain(rho2, 0.0, 1.0);
    
    // compute lag-1 rhohv
    
    double rhohv1 =
      (mag_lag1_vchc + mag_lag1_hcvc) / (2.0 * sqrt(lag0_hc * lag0_vc));
    
    // lag-0 rhohv is rhohv1 corrected by rho2
    
    double rhohv0 = rhohv1 / pow(rho2, 0.25);
    fields.rhohv_nnc = _constrain(rhohv0, 0.0, 1.0);

  } // if (lag0_hc_ns > 0 && lag0_vc_ns > 0)
  
}

///////////////////////////////////////////////////////////
// DP_ALT_HV_CO_CROSS
// Transmit alternating, receive co/cross
// IQ passed in

void RadarMoments::dpAltHvCoCross(RadarComplex_t *iqhc,
                                  RadarComplex_t *iqvc,
                                  RadarComplex_t *iqhx,
                                  RadarComplex_t *iqvx,
                                  int gateNum,
                                  bool isFiltered,
                                  MomentsFields &fields)
  
{
  
  // initialize field meta data

  _setFieldMetaData(fields);

  // compute covariances
  
  double lag0_hc = RadarComplex::meanPower(iqhc, _nSamplesHalf - 1);
  double lag0_hx = RadarComplex::meanPower(iqhx, _nSamplesHalf - 1);
  double lag0_vc = RadarComplex::meanPower(iqvc, _nSamplesHalf - 1);
  double lag0_vx = RadarComplex::meanPower(iqvx, _nSamplesHalf - 1);

  // compute lag1 co-polar correlation V to H
  
  RadarComplex_t lag1_vchc =
    RadarComplex::meanConjugateProduct(iqvc, iqhc, _nSamplesHalf - 1);
  
  // compute lag1 co-polar correlation H to V
  
  RadarComplex_t lag1_hcvc =
    RadarComplex::meanConjugateProduct(iqhc + 1, iqvc, _nSamplesHalf - 1);

  // compute lag0 cross-polar correlation Vc to Hx
  
  RadarComplex_t lag0_vchx =
    RadarComplex::meanConjugateProduct(iqvc, iqhx, _nSamplesHalf - 1);
  
  // compute lag0 cross-polar correlation Hc to Vx
  
  RadarComplex_t lag0_hcvx =
    RadarComplex::meanConjugateProduct(iqhc, iqvx, _nSamplesHalf - 1);
  
  // compute lag0 cross-polar correlation Hx to Vx
  
  RadarComplex_t lag1_vxhx =
    RadarComplex::meanConjugateProduct(iqvx, iqhx, _nSamplesHalf - 1);

  // lag-2 correlations for HH and VV
  
  RadarComplex_t lag2_hc =
    RadarComplex::meanConjugateProduct(iqhc + 1, iqhc, _nSamplesHalf - 1);
  
  RadarComplex_t lag2_vc =
    RadarComplex::meanConjugateProduct(iqvc + 1, iqvc, _nSamplesHalf - 1);
  
  // compute moments from covariances
  
  computeMomDpAltHvCoCross(lag0_hc, lag0_hx,
                           lag0_vc, lag0_vx,
                           lag0_vchx, lag0_hcvx, lag1_vxhx,
                           lag1_vchc, lag1_hcvc,
                           lag2_hc, lag2_vc,
                           gateNum, fields);
  
  if (!isFiltered) {

    // refractivity

    computeRefract(iqhc, _nSamplesHalf, fields.aiq_hc, fields.niq_hc, _changeAiqSign);
    computeRefract(iqvc, _nSamplesHalf, fields.aiq_vc, fields.niq_vc, _changeAiqSign);
    
    // CPA

    if (_computeCpaUsingAlt) {
      fields.cpa = computeCpaAlt(iqhc, iqvc, _nSamplesHalf);
    } else {
      fields.cpa = computeCpa(iqhc, iqvc, _nSamplesHalf);
    }

    // CPR

    double cprPowerDb, cprPhaseDeg;
    computeCpr(iqhc, iqvx, _nSamplesHalf, cprPowerDb, cprPhaseDeg);
    fields.cpr_mag = cprPowerDb;
    fields.cpr_phase = cprPhaseDeg;
    fields.cpr_ldr = pow(10.0, (fields.cpr_mag - fields.ldrh) / 10.0);

  }
  
  // sdev of VV time series

  fields.sdev_vv = computeMagSdev(iqvc, _nSamplesHalf);

}

///////////////////////////////////////////////////////////
// DP_ALT_HV_CO_CROSS
// Transmit alternating, receive co/cross
// covariances passed in

void RadarMoments::computeMomDpAltHvCoCross(double lag0_hc,
                                            double lag0_hx,
                                            double lag0_vc,
                                            double lag0_vx,
                                            RadarComplex_t lag0_vchx,
                                            RadarComplex_t lag0_hcvx,
                                            RadarComplex_t lag1_vxhx,
                                            RadarComplex_t lag1_vchc,
                                            RadarComplex_t lag1_hcvc,
                                            RadarComplex_t lag2_hc,
                                            RadarComplex_t lag2_vc,
                                            int gateNum,
                                            MomentsFields &fields)
  
{
  
  _setFieldMetaData(fields);

  // lag0

  fields.lag0_hc_db = 10.0 * log10(lag0_hc);
  fields.lag0_hx_db = 10.0 * log10(lag0_hx);
  fields.lag0_vc_db = 10.0 * log10(lag0_vc);
  fields.lag0_vx_db = 10.0 * log10(lag0_vx);

  // compute dbm
  
  double dbm_hc = 10.0 * log10(lag0_hc) - _receiverGainDbHc;
  double dbm_hx = 10.0 * log10(lag0_hx) - _receiverGainDbHx;
  double dbm_vc = 10.0 * log10(lag0_vc) - _receiverGainDbVc;
  double dbm_vx = 10.0 * log10(lag0_vx) - _receiverGainDbVx;
  
  fields.dbmhc = dbm_hc;
  fields.dbmhx = dbm_hx;
  fields.dbmvc = dbm_vc;
  fields.dbmvx = dbm_vx;
  fields.dbm = (dbm_hc + dbm_vc) / 2.0;

  // compute noise-subtracted lag0
  
  double lag0_hc_ns = lag0_hc - _estNoisePowerHc;
  double lag0_hx_ns = lag0_hx - _estNoisePowerHx;
  double lag0_vc_ns = lag0_vc - _estNoisePowerVc;
  double lag0_vx_ns = lag0_vx - _estNoisePowerVx;
  
  // check SNR
  
  bool snrHcOK = true;
  double min_valid_pwr_hc = _estNoisePowerHc * _minDetectableSnr;
  if (lag0_hc_ns < min_valid_pwr_hc) {
    snrHcOK = false;
    fields.censoring_flag = 1;
  }

  bool snrVcOK = true;
  double min_valid_pwr_vc = _estNoisePowerVc * _minDetectableSnr;
  if (lag0_vc_ns < min_valid_pwr_vc) {
    snrVcOK = false;
    fields.censoring_flag = 1;
  }
  
  bool snrHxOK = true;
  double min_valid_pwr_hx = _estNoisePowerHx * _minDetectableSnr;
  if (lag0_hx_ns < min_valid_pwr_hx) {
    snrHxOK = false;
    fields.censoring_flag = 1;
  }

  bool snrVxOK = true;
  double min_valid_pwr_vx = _estNoisePowerVx * _minDetectableSnr;
  if (lag0_vx_ns < min_valid_pwr_vx) {
    snrVxOK = false;
    fields.censoring_flag = 1;
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

  //////////////////////////////////////////////////
  // phidp, velocity
  //
  // See A. Zahrai and D. Zrnic
  // "The 10 cm wavelength polarimetric weather radar
  // at NOAA'a National Severe Storms Lab. "
  // JTech, Vol 10, No 5, October 1993.
  
  double mag_lag1_vchc = RadarComplex::mag(lag1_vchc);
  double mag_lag1_hcvc = RadarComplex::mag(lag1_hcvc);
  double arg_lag1_vchc = RadarComplex::argRad(lag1_vchc);
  double arg_lag1_hcvc = RadarComplex::argRad(lag1_hcvc);

  fields.lag1_vchc_db = 20.0 * log10(mag_lag1_vchc);
  fields.lag1_vchc_phase = arg_lag1_vchc * RAD_TO_DEG;
  fields.lag1_hcvc_db = 20.0 * log10(mag_lag1_hcvc);
  fields.lag1_hcvc_phase = arg_lag1_hcvc * RAD_TO_DEG;
  
  double mag_lag0_vchx = RadarComplex::mag(lag0_vchx);
  double mag_lag0_hcvx = RadarComplex::mag(lag0_hcvx);
  double mag_lag1_vxhx = RadarComplex::mag(lag1_vxhx);

  double arg_lag0_vchx = RadarComplex::argRad(lag0_vchx);
  double arg_lag0_hcvx = RadarComplex::argRad(lag0_hcvx);
  double arg_lag1_vxhx = RadarComplex::argRad(lag1_vxhx);

  fields.lag0_vchx_db = 20.0 * log10(mag_lag0_vchx);
  fields.lag0_vchx_phase = arg_lag0_vchx * RAD_TO_DEG;

  fields.lag0_hcvx_db = 20.0 * log10(mag_lag0_hcvx);
  fields.lag0_hcvx_phase = arg_lag0_hcvx * RAD_TO_DEG;
  
  fields.lag1_vxhx_db = 20.0 * log10(mag_lag1_vxhx);
  fields.lag1_vxhx_phase = arg_lag1_vxhx * RAD_TO_DEG;
  
  // compute system phidp

  RadarComplex_t phidp0 = RadarComplex::conjugateProduct(lag1_vchc, lag1_hcvc);
  double phidpRad0 = RadarComplex::argRad(phidp0) / 2.0;
  fields.phidp0 = phidpRad0 * RAD_TO_DEG * _velSign * _phidpSign;

  // Compute phi H and V, correcting for system PhiDp offset
  // so that phidp will not wrap prematurely
  
  RadarComplex_t phi_h =
    RadarComplex::complexProduct(lag1_vchc, _phidpOffsetAlt);
  RadarComplex_t phi_v =
    RadarComplex::conjugateProduct(lag1_hcvc, _phidpOffsetAlt);

  RadarComplex_t prod = RadarComplex::conjugateProduct(phi_v, phi_h);
  double phi = -0.5 * RadarComplex::argRad(prod);
  fields.phidp = phi * RAD_TO_DEG * _velSign * _phidpSign;

  double psi_h = RadarComplex::argRad(phi_h) - phi;
  double psi_v = RadarComplex::argRad(phi_v) + phi;
  if (psi_h < -M_PI) psi_h += 2.0 * M_PI; 
  if (psi_h >  M_PI) psi_h -= 2.0 * M_PI; 
  if (psi_v < -M_PI) psi_v += 2.0 * M_PI; 
  if (psi_v >  M_PI) psi_v -= 2.0 * M_PI; 

  // compute angular difference between them, which is phidp
  
  // RadarComplex_t phidp = RadarComplex::conjugateProduct(phi_h, phi_v);
  // double phidpRad = RadarComplex::argRad(phidp) / 2.0;
  // fields.phidp = phidpRad * RAD_TO_DEG * _velSign * _phidpSign;
  
  // velocity phase is mean of phidp phases
  
  // RadarComplex_t velVect = RadarComplex::complexMean(phi_h, phi_v);
  RadarComplex_t velVect = RadarComplex::complexMean(psi_h, psi_v);
  double argVel = RadarComplex::argRad(velVect);
  double meanVel = (argVel / M_PI) * _nyquist;
  fields.vel = meanVel * _velSign * -1.0;
  fields.vel_alt = fields.vel;

  // also compute velocity which can be used in clutter
  
  double argVelH = RadarComplex::argRad(lag2_hc);
  fields.vel_h_only = (argVelH / M_PI) * (_nyquist / 2.0) * _velSign * -1.0;
  
  double argVelV = RadarComplex::argRad(lag2_vc);
  fields.vel_v_only = (argVelV / M_PI) * (_nyquist / 2.0) * _velSign * -1.0;
  
  // compute alt mode velocity

  RadarComplex_t lag2_sum = RadarComplex::complexSum(lag2_hc, lag2_vc);
  double arg_vel2 = RadarComplex::argRad(lag2_sum);
  
  double vel_hv = (arg_vel2 / M_PI) * (_nyquist / 2.0);
  fields.vel_hv = vel_hv * _velSign * -1.0;

  fields.phase_for_noise = lag2_sum;

  /////////////////////////
  // width, rhohv
  // noise corrected
  
  if (snrHcOK && snrVcOK) {
    
    // compute lag-2 rho and ncp
    
    double mean_lag0_ns = (lag0_hc_ns + lag0_vc_ns) / 2.0;
    if (mean_lag0_ns <= 0) {
      mean_lag0_ns = 1.0e-20;
    }
    RadarComplex_t sumR2 = RadarComplex::complexSum(lag2_hc, lag2_vc);
    double meanMagR2 = RadarComplex::mag(sumR2) / 2.0;
    double rho2 = meanMagR2 / mean_lag0_ns;
    
    // spectrum width from H and V
    
    double argWidth = sqrt(-0.5 * log(rho2));
    double width = (argWidth / M_PI) * _nyquist;
    fields.width = _constrain(width, 0.01, _nyquist);
    
    // width using H and V separately
  
    double lag2_hc_mag = RadarComplex::mag(lag2_hc);
    double arg_lag2Hc = RadarComplex::argRad(lag2_hc);
    fields.lag2_hc_db = 20.0 * log10(lag2_hc_mag);
    fields.lag2_hc_phase = arg_lag2Hc * RAD_TO_DEG;
    
    double lag2_vc_mag = RadarComplex::mag(lag2_vc);
    double arg_lag2Vc = RadarComplex::argRad(lag2_vc);
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
    
    double rhohv1 =
      (mag_lag1_vchc + mag_lag1_hcvc) / (2.0 * sqrt(lag0_hc_ns * lag0_vc_ns));
    
    // lag-0 rhohv is rhohv1 corrected by rho2
    
    double rhohv0 = rhohv1 / pow(rho2, 0.25);
    fields.rhohv = _constrain(rhohv0, 0.0, 1.0);

  } // if (snrHcOK && snrVcOK)
  
  // RHO CO-CROSS
  
  double rho_vchx =
    RadarComplex::mag(lag0_vchx) / sqrt(lag0_vc * lag0_hx);
  double rho_hcvx =
    RadarComplex::mag(lag0_hcvx) / sqrt(lag0_hc * lag0_vx);
  double rho_vxhx =
    RadarComplex::mag(lag1_vxhx) / sqrt(lag0_vx * lag0_hx);

  RadarComplex_t diff = RadarComplex::conjugateProduct(lag0_vchx, lag0_hcvx);
  RadarComplex_t corrected =
    RadarComplex::complexProduct(diff, _phidpOffsetAlt);
  double rho_phidp = RadarComplex::argDeg(corrected);
  
  fields.rho_vchx = _constrain(rho_vchx, 0.0, 1.0);
  fields.rho_hcvx = _constrain(rho_hcvx, 0.0, 1.0);
  fields.rho_vxhx = _constrain(rho_vxhx, 0.0, 1.0);
  fields.rho_phidp = _constrain(rho_phidp, 0.0, 1.0);
  
  /////////////////////////////
  // ncp - not noise corrected
  
  {

    double mean_lag0 = (lag0_hc + lag0_vc) / 2.0;
    RadarComplex_t sumR2 = RadarComplex::complexSum(lag2_hc, lag2_vc);
    double meanMagR2 = RadarComplex::mag(sumR2) / 2.0;
    double rho2 = meanMagR2 / mean_lag0;
    fields.ncp = _constrain(rho2, 0.0, 1.0);
    
    // ncp using H and V separately
    
    double lag2_hc_mag = RadarComplex::mag(lag2_hc);
    fields.ncp_h_only = _constrain(lag2_hc_mag / lag0_hc, 0.0, 1.0);
    
    double lag2_vc_mag = RadarComplex::mag(lag2_vc);
    fields.ncp_v_only = _constrain(lag2_vc_mag / lag0_vc, 0.0, 1.0);

    fields.ncp_h_minus_v = fields.ncp_h_only - fields.ncp_v_only;

  } // if (snrHcOK && snrVcOK)
  
  ////////////////////////////
  // not-noise-corrected rhohv
  
  {

    // compute lag-2 rho
    
    double mean_lag0 = (lag0_hc + lag0_vc) / 2.0;
    RadarComplex_t sumR2 = RadarComplex::complexSum(lag2_hc, lag2_vc);
    double meanMagR2 = RadarComplex::mag(sumR2) / 2.0;
    double rho2 = meanMagR2 / mean_lag0;
    // rho2 = _constrain(rho2, 0.0, 1.0);
    
    // compute lag-1 rhohv
    
    double rhohv1 =
      (mag_lag1_vchc + mag_lag1_hcvc) / (2.0 * sqrt(lag0_hc * lag0_vc));
    
    // lag-0 rhohv is rhohv1 corrected by rho2
    
    double rhohv0 = rhohv1 / pow(rho2, 0.25);
    fields.rhohv_nnc = _constrain(rhohv0, 0.0, 1.0);

  } // if (lag0_hc_ns > 0 && lag0_vc_ns > 0)
  
}

///////////////////////////////////////////////////////////
// DP_SIM_HV
// Dual pol, transmit simultaneous, receive fixed channels 
// IQ passed in

void RadarMoments::dpSimHv(RadarComplex_t *iqhc,
                           RadarComplex_t *iqvc,
                           int gateNum,
                           bool isFiltered,
                           MomentsFields &fields)
  
{
  
  // initialize field meta data

  _setFieldMetaData(fields);

  // compute covariances
  
  double lag0_hc = RadarComplex::meanPower(iqhc, _nSamples - 1);
  double lag0_vc = RadarComplex::meanPower(iqvc, _nSamples - 1);

  RadarComplex_t Rvvhh0 =
    RadarComplex::meanConjugateProduct(iqvc, iqhc, _nSamples - 1);

  RadarComplex_t lag1_hc =
    RadarComplex::meanConjugateProduct(iqhc + 1, iqhc, _nSamples - 1);

  RadarComplex_t lag1_vc =
    RadarComplex::meanConjugateProduct(iqvc + 1, iqvc, _nSamples - 1);
  
  RadarComplex_t lag2_hc =
    RadarComplex::meanConjugateProduct(iqhc + 2, iqhc, _nSamples - 2);

  RadarComplex_t lag2_vc =
    RadarComplex::meanConjugateProduct(iqvc + 2, iqvc, _nSamples - 2);

  RadarComplex_t lag3_hc =
    RadarComplex::meanConjugateProduct(iqhc + 3, iqhc, _nSamples - 3);

  RadarComplex_t lag3_vc =
    RadarComplex::meanConjugateProduct(iqvc + 3, iqvc, _nSamples - 3);

  // compute moments from covariances
  
  computeMomDpSimHv(lag0_hc, lag0_vc, Rvvhh0,
                    lag1_hc, lag1_vc, lag2_hc, lag2_vc, lag3_hc, lag3_vc,
                    gateNum, fields);
  
  if (!isFiltered) {
    
    // refractivity
    
    computeRefract(iqhc, _nSamples, fields.aiq_hc, fields.niq_hc, _changeAiqSign);
    computeRefract(iqvc, _nSamples, fields.aiq_vc, fields.niq_vc, _changeAiqSign);
    
    // CPA
    
    if (_computeCpaUsingAlt) {
      fields.cpa = computeCpaAlt(iqhc, iqvc, _nSamples);
    } else {
      fields.cpa = computeCpa(iqhc, iqvc, _nSamples);
    }
    
  }
  
  // sdev of VV time series

  fields.sdev_vv = computeMagSdev(iqvc, _nSamples);

}

///////////////////////////////////////////////////////////
// DP_SIM_HV
// Dual pol, transmit simultaneous, receive fixed channels
// covariances passed in

void RadarMoments::computeMomDpSimHv(double lag0_hc,
                                     double lag0_vc,
                                     RadarComplex_t Rvvhh0,
                                     RadarComplex_t lag1_hc,
                                     RadarComplex_t lag1_vc,
                                     RadarComplex_t lag2_hc,
                                     RadarComplex_t lag2_vc,
                                     RadarComplex_t lag3_hc,
                                     RadarComplex_t lag3_vc,
                                     int gateNum,
                                     MomentsFields &fields)
  
{
  
  _setFieldMetaData(fields);

  // lag0

  fields.lag0_hc_db = 10.0 * log10(lag0_hc);
  fields.lag0_vc_db = 10.0 * log10(lag0_vc);

  // compute dbm
  
  double dbm_hc = 10.0 * log10(lag0_hc) - _receiverGainDbHc;
  double dbm_vc = 10.0 * log10(lag0_vc) - _receiverGainDbVc;
  
  fields.dbmhc = dbm_hc;
  fields.dbmvc = dbm_vc;
  fields.dbm = (dbm_hc + dbm_vc) / 2.0;

  // compute noise-subtracted lag0
  
  double lag0_hc_ns = lag0_hc - _estNoisePowerHc;
  double lag0_vc_ns = lag0_vc - _estNoisePowerVc;
  
  // check SNR
  
  bool snrHcOK = true;
  double min_valid_pwr_hc = _estNoisePowerHc * _minDetectableSnr;
  if (lag0_hc_ns < min_valid_pwr_hc) {
    snrHcOK = false;
    fields.censoring_flag = 1;
  }

  bool snrVcOK = true;
  double min_valid_pwr_vc = _estNoisePowerVc * _minDetectableSnr;
  if (lag0_vc_ns < min_valid_pwr_vc) {
    snrVcOK = false;
    fields.censoring_flag = 1;
  }
  
  // compute snr
  
  double snr_hc = lag0_hc_ns / _calNoisePowerHc;
  double snr_vc = lag0_vc_ns / _calNoisePowerVc;
  
  if (snrHcOK) {
    fields.dbmhc_ns = 10.0 * log10(lag0_hc_ns) - _receiverGainDbHc;
    fields.snrhc = 10.0 * log10(snr_hc);
  } else {
    fields.snrhc = _missing;
  }
  if (snrVcOK) {
    fields.dbmvc_ns = 10.0 * log10(lag0_vc_ns) - _receiverGainDbVc;
    fields.snrvc = 10.0 * log10(snr_vc);
  } else {
    fields.snrvc = _missing;
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

  ////////////////////////////
  // phidp, rhohv
  
  RadarComplex_t phidp =
    RadarComplex::conjugateProduct(Rvvhh0, _phidpOffsetSim);
  double phidpRad = RadarComplex::argRad(phidp);
  fields.phidp = phidpRad * RAD_TO_DEG * _velSign * _phidpSign;

  double phidpRad0 = RadarComplex::argRad(Rvvhh0);
  fields.phidp0 = phidpRad0 * RAD_TO_DEG * _velSign * _phidpSign;

  double Rvvhh0Mag = RadarComplex::mag(Rvvhh0);
  if (snrHcOK && snrVcOK) {
    // noise corrected
    double rhohv = Rvvhh0Mag / sqrt(lag0_hc_ns * lag0_vc_ns);
    fields.rhohv = _constrain(rhohv, 0.0, 1.0);
  }
  {
    // not noise corrected
    double rhohv_nnc = Rvvhh0Mag / sqrt(lag0_hc * lag0_vc);
    fields.rhohv_nnc = _constrain(rhohv_nnc, 0.0, 1.0);
  }

  fields.rvvhh0_db = 20.0 * log10(Rvvhh0Mag);
  fields.rvvhh0_phase = RadarComplex::argDeg(Rvvhh0);

  // velocity
  
  double lag1_hc_mag = RadarComplex::mag(lag1_hc);
  double arg_vel_hc = RadarComplex::argRad(lag1_hc);

  double lag1_vc_mag = RadarComplex::mag(lag1_vc);
  double arg_vel_vc = RadarComplex::argRad(lag1_vc);

  fields.lag1_hc_db = 20.0 * log10(lag1_hc_mag);
  fields.lag1_hc_phase = arg_vel_hc * RAD_TO_DEG;

  fields.lag1_vc_db = 20.0 * log10(lag1_vc_mag);
  fields.lag1_vc_phase = arg_vel_vc * RAD_TO_DEG;
  
  RadarComplex_t lag1_sum = RadarComplex::complexSum(lag1_hc, lag1_vc);
  double arg_vel = RadarComplex::argRad(lag1_sum);
  
  double vel = (arg_vel / M_PI) * _nyquist;
  fields.vel = vel * _velSign * -1.0;

  double vel_h_only = (arg_vel_hc / M_PI) * _nyquist;
  fields.vel_h_only = vel_h_only * _velSign * -1.0;

  double vel_v_only = (arg_vel_vc / M_PI) * _nyquist;
  fields.vel_v_only = vel_v_only * _velSign * -1.0;

  fields.phase_for_noise = lag1_sum;

  // ncp
  
  double ncp = RadarComplex::mag(lag1_sum) / (lag0_hc + lag0_vc);
  ncp = _constrain(ncp, 0.0, 1.0);
  fields.ncp = ncp;

  ///////////////////////////////////////////////////////
  // width

  double lag2_hc_mag = RadarComplex::mag(lag2_hc);
  double lag2_vc_mag = RadarComplex::mag(lag2_vc);

  fields.lag2_hc_db = 20.0 * log10(lag2_hc_mag);
  fields.lag2_hc_phase = RadarComplex::argDeg(lag2_hc);
  fields.lag2_vc_db = 20.0 * log10(lag2_vc_mag);
  fields.lag2_vc_phase = RadarComplex::argDeg(lag2_vc);
  
  double lag3_hc_mag = RadarComplex::mag(lag3_hc);
  double lag3_vc_mag = RadarComplex::mag(lag3_vc);
  
  fields.lag3_hc_db = 20.0 * log10(lag3_hc_mag);
  fields.lag3_hc_phase = RadarComplex::argDeg(lag3_hc);
  fields.lag3_vc_db = 20.0 * log10(lag3_vc_mag);
  fields.lag3_vc_phase = RadarComplex::argDeg(lag3_vc);
  
  double r1_hc = lag1_hc_mag / _windowR1;
  double r2_hc = lag2_hc_mag / _windowR2;
  double r3_hc = lag3_hc_mag / _windowR3;
  
  double r1_vc = lag1_vc_mag / _windowR1;
  double r2_vc = lag2_vc_mag / _windowR2;
  double r3_vc = lag3_vc_mag / _windowR3;

  double width_r1r2_hc = _computeR1R2Width(r1_hc, r2_hc, _nyquist);
  double width_r1r2_vc = _computeR1R2Width(r1_vc, r2_vc, _nyquist);
  double width_r1r2 = (width_r1r2_hc + width_r1r2_vc) / 2.0;
  fields.width_r1r2 = _constrain(width_r1r2, 0.01, _nyquist);

  double width_r1r3_hc = _computeR1R3Width(r1_hc, r3_hc, _nyquist);
  double width_r1r3_vc = _computeR1R3Width(r1_vc, r3_vc, _nyquist);
  double width_r1r3 = (width_r1r3_hc + width_r1r3_vc) / 2.0;
  fields.width_r1r3 = _constrain(width_r1r3, 0.01, _nyquist);

  // default to R1R2

  fields.width = fields.width_r1r2;

  if (snrHcOK && snrVcOK) {

    double r0_hc = lag0_hc_ns;
    double r0_vc = lag0_vc_ns;

    double width_r0r1_hc = _computeR0R1Width(r0_hc, r1_hc, _nyquist);
    double width_r0r1_vc = _computeR0R1Width(r0_vc, r1_vc, _nyquist);
    double width_r0r1 = (width_r0r1_hc + width_r0r1_vc) / 2.0;
    fields.width_r0r1 = _constrain(width_r0r1, 0.01, _nyquist);

    if (_widthMethod == WIDTH_METHOD_R0R1) {
      fields.width = fields.width_r0r1;
    }
    
    if (_widthMethod == WIDTH_METHOD_HYBRID) {
      double width_hc = _computeHybridWidth(r0_hc, r1_hc, r2_hc, r3_hc, _nyquist);
      double width_vc = _computeHybridWidth(r0_vc, r1_vc, r2_vc, r3_vc, _nyquist);
      fields.width = _constrain((width_hc + width_vc) / 2.0, 0.01, _nyquist);
    }

  }

}

///////////////////////////////////////////////////////////
// Dual pol, transmit H, receive co/cross
// IQ passed in

void RadarMoments::dpHOnly(RadarComplex_t *iqhc,
                           RadarComplex_t *iqvx,
                           int gateNum,
                           bool isFiltered,
                           MomentsFields &fields)
  
{
  
  // initialize field meta data

  _setFieldMetaData(fields);

  // compute covariances
  
  double lag0_hc = RadarComplex::meanPower(iqhc, _nSamples - 1);
  double lag0_vx = RadarComplex::meanPower(iqvx, _nSamples - 1);
  
  RadarComplex_t lag1_hc =
    RadarComplex::meanConjugateProduct(iqhc + 1, iqhc, _nSamples - 1);
  
  RadarComplex_t lag2_hc =
    RadarComplex::meanConjugateProduct(iqhc + 2, iqhc, _nSamples - 2);
  
  RadarComplex_t lag3_hc =
    RadarComplex::meanConjugateProduct(iqhc + 3, iqhc, _nSamples - 3);
  
  // compute moments from covariances

  computeMomDpHOnly(lag0_hc, lag0_vx,
                    lag1_hc, lag2_hc, lag3_hc,
                    gateNum, fields);
  
  if (!isFiltered) {

    // refractivity
    
    computeRefract(iqhc, _nSamples, fields.aiq_hc, fields.niq_hc, _changeAiqSign);

    // CPA
    
    if (_computeCpaUsingAlt) {
      fields.cpa = computeCpaAlt(iqhc, _nSamples);
    } else {
      fields.cpa = computeCpa(iqhc, _nSamples);
    }

  }
  
}

///////////////////////////////////////////////////////////
// Dual pol, transmit H, receive co/cross
// covariances passed in

void RadarMoments::computeMomDpHOnly(double lag0_hc,
                                     double lag0_vx,
                                     RadarComplex_t lag1_hc,
                                     RadarComplex_t lag2_hc,
                                     RadarComplex_t lag3_hc,
                                     int gateNum,
                                     MomentsFields &fields)
  
{
  
  _setFieldMetaData(fields);

  // lag0

  fields.lag0_hc_db = 10.0 * log10(lag0_hc);
  fields.lag0_vx_db = 10.0 * log10(lag0_vx);

  // compute dbm
  
  double dbm_hc = 10.0 * log10(lag0_hc) - _receiverGainDbHc;
  double dbm_vx = 10.0 * log10(lag0_vx) - _receiverGainDbVx;
  
  fields.dbmhc = dbm_hc;
  fields.dbmvx = dbm_vx;
  fields.dbm = dbm_hc;

  // compute noise-subtracted lag0
  
  double lag0_hc_ns = lag0_hc - _estNoisePowerHc;
  double lag0_vx_ns = lag0_vx - _estNoisePowerVx;
  
  bool snrHcOK = true;
  double min_valid_pwr_hc = _estNoisePowerHc * _minDetectableSnr;
  if (lag0_hc_ns < min_valid_pwr_hc) {
    snrHcOK = false;
    fields.censoring_flag = 1;
  }

  bool snrVxOK = true;
  double min_valid_pwr_vx = _estNoisePowerVx * _minDetectableSnr;
  if (lag0_vx_ns < min_valid_pwr_vx) {
    snrVxOK = false;
    fields.censoring_flag = 1;
  }
  
  // compute snr
  
  double snr_hc = lag0_hc_ns / _calNoisePowerHc;
  double snr_vx = lag0_vx_ns / _calNoisePowerVx;

  if (snrHcOK) {
    double snrhc = 10.0 * log10(snr_hc);
    fields.dbmhc_ns = 10.0 * log10(lag0_hc_ns) - _receiverGainDbHc;
    fields.snrhc = snrhc;
    fields.snr = snrhc;
  } else {
    fields.dbmhc_ns = _missing;
    fields.snrhc = _missing;
    fields.snr = _missing;
  }
  if (snrVxOK) {
    fields.dbmvx_ns = 10.0 * log10(lag0_vx_ns) - _receiverGainDbVx;
    fields.snrvx = 10.0 * log10(snr_vx);
  } else {
    fields.dbmvx_ns = _missing;
    fields.snrvx = _missing;
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
  if (snrVxOK) {
    double dbz_vx_no_atten_corr = 
      10.0 * log10(snr_vx) + _baseDbz1kmVx + _rangeCorr[gateNum] + _dbzCorrection;
    double dbz_vx = dbz_vx_no_atten_corr + _atmosAttenCorr[gateNum];
    fields.dbzvx = _adjustDbzForPwrH(dbz_vx);
  } else {
    fields.dbzvx = _missing;
  }

  // measured zdr as power_hc minus power_vx
  
  if (snrHcOK && snrVxOK) {
    double zdrm = 10.0 * log10(lag0_hc_ns / lag0_vx_ns);
    fields.zdrm = zdrm;
  } else {
    fields.zdrm = _missing;
  }
  fields.zdr = _missing;

  // ldr
  
  if (snrHcOK && (fields.snrhc > _minSnrDbForLdr) &&
      snrVxOK && (fields.snrvx > _minSnrDbForLdr)) {
    double ldrhm = 10.0 * log10(lag0_vx_ns / lag0_hc_ns);
    fields.ldrhm = ldrhm;
    fields.ldrh = ldrhm + _ldrCorrectionDbH;
    fields.ldr = fields.ldrh;
  } else {
    fields.ldrhm = _missing;
    fields.ldrh = _missing;
    fields.ldr = _missing;
  }
  fields.ldr_diff = _missing;
  fields.ldr_mean = _missing;

  // velocity

  double lag1_hc_mag = RadarComplex::mag(lag1_hc);
  double argVel = RadarComplex::argRad(lag1_hc);

  fields.lag1_hc_db = 20.0 * log10(lag1_hc_mag);
  fields.lag1_hc_phase = argVel * RAD_TO_DEG;
  
  double vel = (argVel / M_PI) * _nyquist;
  fields.vel = vel * _velSign * -1.0;

  fields.phase_for_noise = lag1_hc;

  // ncp
  
  double ncp = lag1_hc_mag / lag0_hc;
  ncp = _constrain(ncp, 0.0, 1.0);
  fields.ncp = ncp;
  
  // width
  
  double lag2_hc_mag = RadarComplex::mag(lag2_hc);
  fields.lag2_hc_db = 20.0 * log10(lag2_hc_mag);
  fields.lag2_hc_phase = RadarComplex::argDeg(lag2_hc);
  
  double lag3_hc_mag = RadarComplex::mag(lag3_hc);
  fields.lag3_hc_db = 20.0 * log10(lag3_hc_mag);
  fields.lag3_hc_phase = RadarComplex::argDeg(lag3_hc);

  if (snrHcOK) {

    double r0 = lag0_hc_ns;
    double r1 = lag1_hc_mag / _windowR1;
    double r2 = lag2_hc_mag / _windowR2;
    double r3 = lag3_hc_mag / _windowR3;
    
    double width = _computeHybridWidth(r0, r1, r2, r3, _nyquist);
    fields.width = _constrain(width, 0.01, _nyquist);

  }

}

///////////////////////////////////////////////////////////
// Dual pol, transmit V, receive co/cross
// IQ passed in

void RadarMoments::dpVOnly(RadarComplex_t *iqvc,
                           RadarComplex_t *iqhx,
                           int gateNum,
                           bool isFiltered,
                           MomentsFields &fields)
  
{
  
  // initialize field meta data

  _setFieldMetaData(fields);

  // compute covariances
  
  double lag0_vc = RadarComplex::meanPower(iqvc, _nSamples - 1);
  double lag0_hx = RadarComplex::meanPower(iqhx, _nSamples - 1);

  RadarComplex_t lag1_vc =
    RadarComplex::meanConjugateProduct(iqvc + 1, iqvc, _nSamples - 1);

  RadarComplex_t lag2_vc =
    RadarComplex::meanConjugateProduct(iqvc + 2, iqvc, _nSamples - 2);

  RadarComplex_t lag3_vc =
    RadarComplex::meanConjugateProduct(iqvc + 3, iqvc, _nSamples - 3);

  // compute moments from covariances
  
  computeMomDpVOnly(lag0_vc, lag0_hx,
                    lag1_vc, lag2_vc, lag3_vc,
                    gateNum, fields);
  
  if (!isFiltered) {

    // refractivity
    
    computeRefract(iqvc, _nSamples, fields.aiq_vc, fields.niq_vc, _changeAiqSign);

    // CPA
    
    if (_computeCpaUsingAlt) {
      fields.cpa = computeCpaAlt(iqvc, _nSamples);
    } else {
      fields.cpa = computeCpa(iqvc, _nSamples);
    }

  }
  
}

///////////////////////////////////////////////////////////
// Dual pol, transmit V, receive co/cross
// covariances passed in

void RadarMoments::computeMomDpVOnly(double lag0_vc,
                                     double lag0_hx,
                                     RadarComplex_t lag1_vc,
                                     RadarComplex_t lag2_vc,
                                     RadarComplex_t lag3_vc,
                                     int gateNum,
                                     MomentsFields &fields)
  
{
  
  _setFieldMetaData(fields);

  // lag0

  fields.lag0_vc_db = 10.0 * log10(lag0_vc);
  fields.lag0_hx_db = 10.0 * log10(lag0_hx);

  // compute dbm
  
  double dbm_vc = 10.0 * log10(lag0_vc) - _receiverGainDbVc;
  double dbm_hx = 10.0 * log10(lag0_hx) - _receiverGainDbHx;
  
  fields.dbmvc = dbm_vc;
  fields.dbmhx = dbm_hx;
  fields.dbm = dbm_vc;

  // compute noise-subtracted lag0
  
  double lag0_vc_ns = lag0_vc - _estNoisePowerVc;
  double lag0_hx_ns = lag0_hx - _estNoisePowerHx;
  
  bool snrVcOK = true;
  double min_valid_pwr_vc = _estNoisePowerVc * _minDetectableSnr;
  if (lag0_vc_ns < min_valid_pwr_vc) {
    snrVcOK = false;
    fields.censoring_flag = 1;
  }
  
  bool snrHxOK = true;
  double min_valid_pwr_hx = _estNoisePowerHx * _minDetectableSnr;
  if (lag0_hx_ns < min_valid_pwr_hx) {
    snrHxOK = false;
    fields.censoring_flag = 1;
  }

  // snr
  
  double snr_vc = lag0_vc_ns / _calNoisePowerVc;
  double snr_hx = lag0_hx_ns / _calNoisePowerHx;
  
  if (snrVcOK) {
    fields.dbmvc_ns = 10.0 * log10(lag0_vc_ns) - _receiverGainDbVc;
    double snrvc = 10.0 * log10(snr_vc);
    fields.snrvc = snrvc;
    fields.snr = snrvc;
  } else {
    fields.snrvc = _missing;
    fields.snr = _missing;
  }
  if (snrHxOK) {
    fields.dbmhx_ns = 10.0 * log10(lag0_hx_ns) - _receiverGainDbHx;
    fields.snrhx = 10.0 * log10(snr_hx);
  } else {
    fields.snrhx = _missing;
  }
  
  // dbz
  
  if (snrVcOK) {
    double dbz_vc_no_atten_corr = 
      10.0 * log10(snr_vc) + _baseDbz1kmVc + _rangeCorr[gateNum] + _dbzCorrection;
    double dbz_vc = dbz_vc_no_atten_corr + _atmosAttenCorr[gateNum];
    fields.dbzvc = _adjustDbzForPwrV(dbz_vc);
    fields.dbz = fields.dbzvc;
    fields.dbz_no_atmos_atten = _adjustDbzForPwrV(dbz_vc_no_atten_corr);
  } else {
    fields.dbz = _missing;
    fields.dbzvc = _missing;
    fields.dbz_no_atmos_atten = _missing;
  }

  if (snrHxOK) {
    double dbz_hx_no_atten_corr =
      10.0 * log10(snr_hx) + _baseDbz1kmHx + _rangeCorr[gateNum] + _dbzCorrection;
    double dbz_hx = dbz_hx_no_atten_corr + _atmosAttenCorr[gateNum];
    fields.dbzhx = _adjustDbzForPwrV(dbz_hx);
  } else {
    fields.dbzhx = _missing;
  }

  // measured zdr as power_hx minus power_vc

  if (snrHxOK && snrVcOK) {
    double zdrm = 10.0 * log10(lag0_hx_ns / lag0_vc_ns);
    fields.zdrm = zdrm;
  } else {
    fields.zdrm = _missing;
  }
  fields.zdr = _missing;

  // ldrv

  if (snrVcOK && (fields.snrvc > _minSnrDbForLdr) &&
      snrHxOK && (fields.snrhx > _minSnrDbForLdr)) {
    double ldrvm = 10.0 * log10(lag0_hx_ns / lag0_vc_ns);
    fields.ldrvm = ldrvm;
    fields.ldrv = ldrvm + _ldrCorrectionDbV;
    fields.ldr = fields.ldrv;
  } else {
    fields.ldrvm = _missing;
    fields.ldrv = _missing;
    fields.ldr = _missing;
  }

  // velocity
  
  double lag1_vc_mag = RadarComplex::mag(lag1_vc);
  double argVel = RadarComplex::argRad(lag1_vc);

  fields.lag1_vc_db = 20.0 * log10(lag1_vc_mag);
  fields.lag1_vc_phase = argVel * RAD_TO_DEG;
  
  double vel = (argVel / M_PI) * _nyquist;
  fields.vel = vel * _velSign * -1.0;

  fields.phase_for_noise = lag1_vc;

  // ncp
  
  double ncp = lag1_vc_mag / lag0_vc;
  ncp = _constrain(ncp, 0.0, 1.0);
  fields.ncp = ncp;
  
  // width
  
  double lag2_vc_mag = RadarComplex::mag(lag2_vc);
  fields.lag2_vc_db = 20.0 * log10(lag2_vc_mag);
  fields.lag2_vc_phase = RadarComplex::argDeg(lag2_vc);
  
  double lag3_vc_mag = RadarComplex::mag(lag3_vc);
  fields.lag3_vc_db = 20.0 * log10(lag3_vc_mag);
  fields.lag3_vc_phase = RadarComplex::argDeg(lag3_vc);

  if (snrVcOK) {

    double r0 = lag0_vc_ns;
    double r1 = lag1_vc_mag / _windowR1;
    double r2 = lag2_vc_mag / _windowR2;
    double r3 = lag3_vc_mag / _windowR3;
    
    double width = _computeHybridWidth(r0, r1, r2, r3, _nyquist);
    fields.width = _constrain(width, 0.01, _nyquist);

  }

}

///////////////////////////////////////////////////////////
// Single polarization Staggered-PRT
// Assumes data is in horizontal channel

void RadarMoments::singlePolStagPrt(RadarComplex_t *iqhc,
                                    RadarComplex_t *iqhcShort,
                                    RadarComplex_t *iqhcLong,
                                    int gateNum,
                                    bool isFiltered,
                                    MomentsFields &fields)
  
{
  
  // initialize field meta data

  _setFieldMetaData(fields);
  
  if (gateNum >= _nGatesPrtLong) {
    // no data beyond this gate
    return;
  }
  
  // compute covariances
  
  double lag0_hc_long = RadarComplex::meanPower(iqhcLong, _nSamplesHalf - 1);
  double lag0_hc_short = RadarComplex::meanPower(iqhcShort, _nSamplesHalf - 1);
  
  RadarComplex_t lag1_hc_long =
    RadarComplex::meanConjugateProduct(iqhcLong + 1, iqhcLong, _nSamplesHalf - 1);
  RadarComplex_t lag1_hc_short =
    RadarComplex::meanConjugateProduct(iqhcShort + 1, iqhcShort, _nSamplesHalf - 1);
  
  RadarComplex_t lag1_hc_short_to_long =
    RadarComplex::meanConjugateProduct(iqhcShort, iqhcLong, _nSamplesHalf - 1);

  RadarComplex_t lag1_hc_long_to_short =
    RadarComplex::meanConjugateProduct(iqhcLong, iqhcShort + 1, _nSamplesHalf - 1);
  
  singlePolStagPrt(lag0_hc_long,
                   lag0_hc_short,
                   lag1_hc_long,
                   lag1_hc_short,
                   lag1_hc_short_to_long,
                   lag1_hc_long_to_short,
                   gateNum,
                   isFiltered,
                   fields);
  
  if (!isFiltered) {
    // CPA and refractivity
    computeRefract(iqhc, _nSamples, fields.aiq_hc, fields.niq_hc, _changeAiqSign);
    // CPA
    if (_computeCpaUsingAlt) {
      fields.cpa = computeCpaAlt(iqhc, _nSamples);
    } else {
      fields.cpa = computeCpa(iqhc, _nSamples);
    }
  }
  
}

///////////////////////////////////////////////////////////
// Single polarization Staggered-PRT
// Assumes data is in horizontal channel
// covariances passed in

void RadarMoments::singlePolStagPrt(double lag0_hc_long,
                                    double lag0_hc_short,
                                    RadarComplex_t &lag1_hc_long,
                                    RadarComplex_t &lag1_hc_short,
                                    RadarComplex_t &lag1_hc_short_to_long,
                                    RadarComplex_t &lag1_hc_long_to_short,
                                    int gateNum,
                                    bool isFiltered,
                                    MomentsFields &fields)
  
{
  
  // initialize field meta data
  
  _setFieldMetaData(fields);

  // return now?

  if (gateNum >= _nGatesPrtLong) {
    // no data beyond this gate
    return;
  }

  // copy covariances into field object

  fields.lag0_hc_long = lag0_hc_long;
  fields.lag0_hc_short = lag0_hc_short;
  
  fields.lag1_hc_long = lag1_hc_long;
  fields.lag1_hc_short = lag1_hc_short;
  
  fields.lag1_hc_short_to_long = lag1_hc_short_to_long;
  fields.lag1_hc_long_to_short = lag1_hc_long_to_short;
  
  fields.phase_for_noise = lag1_hc_short_to_long;
  
  // compute power-related fields

  if (lag0_hc_long < lag0_hc_short) {
    singlePolStagPrtPower(lag0_hc_long, gateNum, isFiltered, fields);
  } else {
    singlePolStagPrtPower(lag0_hc_short, gateNum, isFiltered, fields);
  }
  bool snrHcOK = (fields.snrhc != _missing);
  
  if (gateNum >= _nGatesPrtShort) {
    // only power beyond short PRT region
    // no phase-based computations
    return;
  }

  // set db and phase

  fields.lag0_hc_short_db = 10.0 * log10(lag0_hc_short);
  fields.lag0_hc_long_db = 10.0 * log10(lag0_hc_long);

  double lag1_hc_short_mag = RadarComplex::mag(lag1_hc_short);
  fields.lag1_hc_short_db = 20.0 * log10(lag1_hc_short_mag);
  fields.lag1_hc_short_phase = RadarComplex::argDeg(lag1_hc_short);
  
  double lag1_hc_long_mag = RadarComplex::mag(lag1_hc_long);
  fields.lag1_hc_long_db = 20.0 * log10(lag1_hc_long_mag);
  fields.lag1_hc_long_phase = RadarComplex::argDeg(lag1_hc_long);
  
  double lag1_hc_short_to_long_mag = RadarComplex::mag(lag1_hc_short_to_long);
  fields.lag1_hc_short_to_long_db = 20.0 * log10(lag1_hc_short_to_long_mag);
  fields.lag1_hc_short_to_long_phase = RadarComplex::argDeg(lag1_hc_short_to_long);
  
  double lag1_hc_long_to_short_mag = RadarComplex::mag(lag1_hc_long_to_short);
  fields.lag1_hc_long_to_short_db = 20.0 * log10(lag1_hc_long_to_short_mag);
  fields.lag1_hc_long_to_short_phase = RadarComplex::argDeg(lag1_hc_long_to_short);
  
  // ncp
  
  fields.ncp_prt_short = lag1_hc_short_mag / lag0_hc_short;
  fields.ncp_prt_long = lag1_hc_long_mag / lag0_hc_long;
  double ncpSL = lag1_hc_short_mag / lag0_hc_long;
  double ncpLS = lag1_hc_long_mag / lag0_hc_short;
  fields.ncp_trip_flag = MAX(ncpSL, ncpLS);

  fields.ncp_prt_short = _constrain(fields.ncp_prt_short, 0.0, 1.0);
  fields.ncp_prt_long = _constrain(fields.ncp_prt_long, 0.0, 1.0);
  fields.ncp = fields.ncp_prt_short;
  
  // compute velocity short PRT data
  
  double argVelShort =  fields.lag1_hc_short_to_long_phase * DEG_TO_RAD;
  double velShort = (argVelShort / M_PI) * _nyquistPrtShort;
  fields.vel_prt_short = velShort * _velSign * _velSignStaggered * -1.0;

  // compute velocity long PRT data
  
  double argVelLong = fields.lag1_hc_long_to_short_phase * DEG_TO_RAD;
  double velLong = (argVelLong / M_PI) * _nyquistPrtLong;
  fields.vel_prt_long = velLong * _velSign * _velSignStaggered * -1.0;
  
  // compute the unfolded velocity

  double vel_diff = fields.vel_prt_short - fields.vel_prt_long;
  double nyquistDiff = _nyquistPrtShort - _nyquistPrtLong;
  double nyquistIntervalShort = (vel_diff / nyquistDiff) / 2.0;
  int ll = (int) floor(nyquistIntervalShort + 0.5);
  if (ll < -_LL) {
    ll = -_LL;
  } else if (ll > _LL) {
    ll = _LL;
  }
  double unfoldedVel = fields.vel_prt_short + _PP[ll] * _nyquistPrtShort * 2;
  fields.vel = unfoldedVel;
  fields.vel_unfold_interval = _PP[ll];
  fields.vel_diff = vel_diff;

  // width

  if (snrHcOK) {

    // compute spectrum width using R(0)/R(m) - default
    
    double R0 = lag0_hc_short - _estNoisePowerHc;
    double Rm = lag1_hc_long_to_short_mag;
    double width_R0Rm = _computeStagWidth(R0, Rm, 0, _staggeredM, 1.0);
    fields.width = _constrain(width_R0Rm * _nyquistStagNominal,
                              0.01, _nyquistPrtShort);
    
    // widths from long and short prt sequences
    
    double r0_short = lag0_hc_short - _estNoisePowerHc;
    double r1_short = lag1_hc_short_mag;
    double r0r1_short = _computeR0R1Width(r0_short, r1_short, _nyquistShortPlusLong);
    fields.width_prt_short = _constrain(r0r1_short, 0.01, _nyquistShortPlusLong);
    
    double r0_long = lag0_hc_long - _estNoisePowerHc;
    double r1_long = lag1_hc_long_mag;
    double r0r1_long = _computeR0R1Width(r0_long, r1_long, _nyquistShortPlusLong);
    fields.width_prt_long = _constrain(r0r1_long, 0.01, _nyquistShortPlusLong);
    
    if (_widthMethod != WIDTH_METHOD_R0R1) {
      
      // use hybrid width estimator
      
      // spectrum width using R(m)/R(n)
      
      double Rn = lag1_hc_short_to_long_mag;
      double width_RmRn = _computeStagWidth(Rm, Rn, _staggeredM, _staggeredN, 1.0);
      
      // spectrum width using R(m)/R(m+n)
      
      double Rmplusn = lag1_hc_short_mag;
      double width_RmRmpn =
        _computeStagWidth(Rm, Rmplusn, _staggeredM, _staggeredM + _staggeredN, 1.0);
      
      // hybrid
      
      double hybrid = width_RmRn;
      if (width_R0Rm > 0.1) {
        hybrid = width_R0Rm;
      } else if (width_RmRmpn < 0.05) {
        hybrid = width_RmRmpn;
      }
      
      fields.width = _constrain(hybrid * _nyquist, 0.01, _nyquist);
      
    } // if (_widthMethod != WIDTH_METHOD_R01)

  } // if (snrHcOK)
    
}

///////////////////////////////////////////////////////////
// Single polarization staggered PRT power
// Assumes the data is in the horizontal channel

void RadarMoments::singlePolStagPrtPower(double lag0_hc,
                                         int gateNum,
                                         bool isFiltered,
                                         MomentsFields &fields)
  
{

  _setFieldMetaData(fields);

  // lag0
  
  fields.lag0_hc_db = 10.0 * log10(lag0_hc);

  // compute dbm
  
  double dbm_hc = 10.0 * log10(lag0_hc) - _receiverGainDbHc;
  fields.dbmhc = dbm_hc;
  fields.dbm = dbm_hc;

  // compute noise-subtracted lag0
  
  double lag0_hc_ns = lag0_hc - _estNoisePowerHc;
  
  bool snrHcOK = true;
  double min_valid_pwr_hc = _estNoisePowerHc * _minDetectableSnr;
  if (lag0_hc_ns < min_valid_pwr_hc) {
    snrHcOK = false;
    fields.censoring_flag = 1;
  }

  // snr & dbz
  
  if (snrHcOK) {

    double snr_hc = lag0_hc_ns / _calNoisePowerHc;
    double snrDb = 10.0 * log10(snr_hc);
    fields.dbmhc_ns = 10.0 * log10(lag0_hc_ns) - _receiverGainDbHc;
    fields.snrhc = snrDb;
    fields.snr = snrDb;

    double dbz_hc_no_atten_corr =
      10.0 * log10(snr_hc) + _baseDbz1kmHc + _rangeCorr[gateNum] + _dbzCorrection;
    double dbz_hc = dbz_hc_no_atten_corr + _atmosAttenCorr[gateNum];
    fields.dbzhc = _adjustDbzForPwrH(dbz_hc);
    fields.dbz = fields.dbzhc;
    fields.dbz_no_atmos_atten = _adjustDbzForPwrH(dbz_hc_no_atten_corr);
     
  } else {

    fields.snrhc = _missing;
    fields.snr = _missing;
    fields.dbz = _missing;
    fields.dbzhc = _missing;
    fields.dbz_no_atmos_atten = _missing;

  }

}

///////////////////////////////////////////////////////////
// Dual pol, transmit simultaneous, receive fixed channels
// Staggered-PRT

void RadarMoments::dpSimHvStagPrt(RadarComplex_t *iqhc,
                                  RadarComplex_t *iqvc,
                                  RadarComplex_t *iqhcShort,
                                  RadarComplex_t *iqvcShort,
                                  RadarComplex_t *iqhcLong,
                                  RadarComplex_t *iqvcLong,
                                  int gateNum,
                                  bool isFiltered,
                                  MomentsFields &fields)
  
{
  
  // initialize field meta data

  _setFieldMetaData(fields);

  if (gateNum >= _nGatesPrtLong) {
    // no data beyond this gate
    return;
  }

  double lag0_hc_long = RadarComplex::meanPower(iqhcLong, _nSamplesHalf - 1);
  double lag0_vc_long = RadarComplex::meanPower(iqvcLong, _nSamplesHalf - 1);

  double lag0_hc_short = RadarComplex::meanPower(iqhcShort, _nSamplesHalf - 1);
  double lag0_vc_short = RadarComplex::meanPower(iqvcShort, _nSamplesHalf - 1);
  
  RadarComplex_t lag1_hc_long =
    RadarComplex::meanConjugateProduct(iqhcLong + 1, iqhcLong, _nSamplesHalf - 1);
  RadarComplex_t lag1_vc_long =
    RadarComplex::meanConjugateProduct(iqvcLong + 1, iqvcLong, _nSamplesHalf - 1);

  RadarComplex_t lag1_hc_short =
    RadarComplex::meanConjugateProduct(iqhcShort + 1, iqhcShort, _nSamplesHalf - 1);
  RadarComplex_t lag1_vc_short =
    RadarComplex::meanConjugateProduct(iqvcShort + 1, iqvcShort, _nSamplesHalf - 1);

  RadarComplex_t lag1_hc_short_to_long =
    RadarComplex::meanConjugateProduct(iqhcShort, iqhcLong, _nSamplesHalf - 1);
  RadarComplex_t lag1_vc_short_to_long =
    RadarComplex::meanConjugateProduct(iqvcShort, iqvcLong, _nSamplesHalf - 1);

  RadarComplex_t lag1_hc_long_to_short =
    RadarComplex::meanConjugateProduct(iqhcLong, iqhcShort + 1, _nSamplesHalf - 1);
  RadarComplex_t lag1_vc_long_to_short =
    RadarComplex::meanConjugateProduct(iqvcLong, iqvcShort + 1, _nSamplesHalf - 1);
  
  RadarComplex_t rvvhh0_long =
    RadarComplex::meanConjugateProduct(iqvcLong, iqhcLong, _nSamplesHalf - 1);
  RadarComplex_t rvvhh0_short =
    RadarComplex::meanConjugateProduct(iqvcShort, iqhcShort, _nSamplesHalf - 1);

  dpSimHvStagPrt(lag0_hc_long,
                 lag0_vc_long,
                 lag0_hc_short,
                 lag0_vc_short,
                 lag1_hc_long,
                 lag1_vc_long,
                 lag1_hc_short,
                 lag1_vc_short,
                 lag1_hc_short_to_long,
                 lag1_vc_short_to_long,
                 lag1_hc_long_to_short,
                 lag1_vc_long_to_short,
                 rvvhh0_long,
                 rvvhh0_short,
                 gateNum,
                 isFiltered,
                 fields);
  
  if (!isFiltered) {
    // CPA and refractivity
    computeRefract(iqhc, _nSamples, fields.aiq_hc, fields.niq_hc, _changeAiqSign);
    computeRefract(iqvc, _nSamples, fields.aiq_vc, fields.niq_vc, _changeAiqSign);
    // CPA
    if (_computeCpaUsingAlt) {
      fields.cpa = computeCpaAlt(iqhc, iqvc, _nSamples);
    } else {
      fields.cpa = computeCpa(iqhc, iqvc, _nSamples);
    }
  }
  
}

///////////////////////////////////////////////////////////
// Dual pol, transmit simultaneous, receive fixed channels
// Staggered-PRT
// Covariances passed in

void RadarMoments::dpSimHvStagPrt(double lag0_hc_long,
                                  double lag0_vc_long,
                                  double lag0_hc_short,
                                  double lag0_vc_short,
                                  RadarComplex_t &lag1_hc_long,
                                  RadarComplex_t &lag1_vc_long,
                                  RadarComplex_t &lag1_hc_short,
                                  RadarComplex_t &lag1_vc_short,
                                  RadarComplex_t &lag1_hc_short_to_long,
                                  RadarComplex_t &lag1_vc_short_to_long,
                                  RadarComplex_t &lag1_hc_long_to_short,
                                  RadarComplex_t &lag1_vc_long_to_short,
                                  RadarComplex_t &rvvhh0_long,
                                  RadarComplex_t &rvvhh0_short,
                                  int gateNum,
                                  bool isFiltered,
                                  MomentsFields &fields)
  
{
  
  // initialize field meta data

  _setFieldMetaData(fields);

  if (gateNum >= _nGatesPrtLong) {
    // no data beyond this gate
    return;
  }

  // copy covariances into field object

  fields.lag0_hc_long = lag0_hc_long;
  fields.lag0_vc_long = lag0_vc_long;

  fields.lag0_hc_short = lag0_hc_short;
  fields.lag0_vc_short = lag0_vc_short;
  
  fields.lag1_hc_long = lag1_hc_long;
  fields.lag1_vc_long = lag1_vc_long;

  fields.lag1_hc_short = lag1_hc_short;
  fields.lag1_vc_short = lag1_vc_short;
  
  fields.lag1_hc_short_to_long = lag1_hc_short_to_long;
  fields.lag1_vc_short_to_long = lag1_vc_short_to_long;

  fields.lag1_hc_long_to_short = lag1_hc_long_to_short;
  fields.lag1_vc_long_to_short = lag1_vc_long_to_short;

  fields.rvvhh0_long = rvvhh0_long;
  fields.rvvhh0_short = rvvhh0_short;
  
  fields.phase_for_noise = lag1_hc_short_to_long;
  
  /////////////////////////////////////////////////////////////////////////////
  // power-related fields

  // dpSimHvStagPrtPower(lag0_hc_long, lag0_vc_long, gateNum,
  //                     isFiltered, fields);

  if (lag0_hc_long < lag0_hc_short && lag0_vc_long < lag0_vc_short) {
    dpSimHvStagPrtPower(lag0_hc_long, lag0_vc_long, gateNum,
                        isFiltered, fields);
  } else {
    dpSimHvStagPrtPower(lag0_hc_short, lag0_vc_short, gateNum,
                        isFiltered, fields);
  }
  bool snrHcOK = (fields.snrhc != _missing);
  bool snrVcOK = (fields.snrvc != _missing);
  
  if (gateNum >= _nGatesPrtShort) {
    // only power beyond short PRT region, no phase-based fields
    return;
  }

  // set db and phase
  
  fields.lag0_hc_short_db = 10.0 * log10(lag0_hc_short);
  fields.lag0_hc_long_db = 10.0 * log10(lag0_hc_long);

  fields.lag0_vc_short_db = 10.0 * log10(lag0_vc_short);
  fields.lag0_vc_long_db = 10.0 * log10(lag0_vc_long);

  double lag1_hc_short_mag = RadarComplex::mag(lag1_hc_short);
  fields.lag1_hc_short_db = 20.0 * log10(lag1_hc_short_mag);
  fields.lag1_hc_short_phase = RadarComplex::argDeg(lag1_hc_short);
  
  double lag1_vc_short_mag = RadarComplex::mag(lag1_vc_short);
  fields.lag1_vc_short_db = 20.0 * log10(lag1_vc_short_mag);
  fields.lag1_vc_short_phase = RadarComplex::argDeg(lag1_vc_short);
  
  double lag1_hc_long_mag = RadarComplex::mag(lag1_hc_long);
  fields.lag1_hc_long_db = 20.0 * log10(lag1_hc_long_mag);
  fields.lag1_hc_long_phase = RadarComplex::argDeg(lag1_hc_long);
  
  double lag1_vc_long_mag = RadarComplex::mag(lag1_vc_long);
  fields.lag1_vc_long_db = 20.0 * log10(lag1_vc_long_mag);
  fields.lag1_vc_long_phase = RadarComplex::argDeg(lag1_vc_long);
  
  double lag1_hc_short_to_long_mag = RadarComplex::mag(lag1_hc_short_to_long);
  fields.lag1_hc_short_to_long_db = 20.0 * log10(lag1_hc_short_to_long_mag);
  fields.lag1_hc_short_to_long_phase = RadarComplex::argDeg(lag1_hc_short_to_long);
  
  double lag1_vc_short_to_long_mag = RadarComplex::mag(lag1_vc_short_to_long);
  fields.lag1_vc_short_to_long_db = 20.0 * log10(lag1_vc_short_to_long_mag);
  fields.lag1_vc_short_to_long_phase = RadarComplex::argDeg(lag1_vc_short_to_long);
  
  double lag1_hc_long_to_short_mag = RadarComplex::mag(lag1_hc_long_to_short);
  fields.lag1_hc_long_to_short_db = 20.0 * log10(lag1_hc_long_to_short_mag);
  fields.lag1_hc_long_to_short_phase = RadarComplex::argDeg(lag1_hc_long_to_short);
  
  double lag1_vc_long_to_short_mag = RadarComplex::mag(lag1_vc_long_to_short);
  fields.lag1_vc_long_to_short_db = 20.0 * log10(lag1_vc_long_to_short_mag);
  fields.lag1_vc_long_to_short_phase = RadarComplex::argDeg(lag1_vc_long_to_short);
  
  double rvvhh0_long_mag = RadarComplex::mag(rvvhh0_long);
  fields.rvvhh0_long_db = 20.0 * log10(rvvhh0_long_mag);
  fields.rvvhh0_long_phase = RadarComplex::argDeg(rvvhh0_long);

  double rvvhh0_short_mag = RadarComplex::mag(rvvhh0_short);
  fields.rvvhh0_short_db = 20.0 * log10(rvvhh0_short_mag);
  fields.rvvhh0_short_phase = RadarComplex::argDeg(rvvhh0_short);

  ////////////////////////////////////////////////////////////////////////////////
  // ncp
  
  RadarComplex_t lag1_sum_short =
    RadarComplex::complexSum(lag1_hc_short, lag1_vc_short);
  
  RadarComplex_t lag1_sum_long =
    RadarComplex::complexSum(lag1_hc_long, lag1_vc_long);

  double lag1_mag_short = RadarComplex::mag(lag1_sum_short);
  double lag1_mag_long = RadarComplex::mag(lag1_sum_long);

  double lag0_sum_short = (lag0_hc_short + lag0_vc_short);
  double lag0_sum_long = (lag0_hc_long + lag0_vc_long);

  double ncpShort = lag1_mag_short / lag0_sum_short;
  double ncpLong = lag1_mag_long / lag0_sum_long;

  double ncpSL = lag1_mag_short / lag0_sum_long;
  double ncpLS = lag1_mag_long / lag0_sum_short;

  fields.ncp_prt_short = ncpShort;
  fields.ncp_prt_long = ncpLong;
  fields.ncp_trip_flag = MAX(ncpSL, ncpLS);

  fields.ncp_prt_short = _constrain(fields.ncp_prt_short, 0.0, 1.0);
  fields.ncp_prt_long = _constrain(fields.ncp_prt_long, 0.0, 1.0);
  fields.ncp = fields.ncp_prt_short;
  
  ////////////////////////////////////////////////////////////////////////////////
  // velocity

  // compute velocity short PRT data
  
  RadarComplex_t lag1_sum_short_to_long =
    RadarComplex::complexSum(lag1_hc_short_to_long, lag1_vc_short_to_long);
  double argVelShort = RadarComplex::argRad(lag1_sum_short_to_long);
  double velShort = (argVelShort / M_PI) * _nyquistPrtShort;
  fields.vel_prt_short = velShort * _velSign * _velSignStaggered * -1.0;

  fields.phase_for_noise = lag1_sum_short_to_long;
  
  // compute velocity long PRT data
  
  RadarComplex_t lag1_sum_long_to_short =
    RadarComplex::complexSum(lag1_hc_long_to_short, lag1_vc_long_to_short);
  double argVelLong = RadarComplex::argRad(lag1_sum_long_to_short);
  double velLong = (argVelLong / M_PI) * _nyquistPrtLong;
  fields.vel_prt_long = velLong * _velSign * _velSignStaggered * -1.0;

  // compute the unfolded velocity

  double vel_diff = fields.vel_prt_short - fields.vel_prt_long;
  double nyquistDiff = _nyquistPrtShort - _nyquistPrtLong;
  double nyquistIntervalShort = (vel_diff / nyquistDiff) / 2.0;
  int ll = (int) floor(nyquistIntervalShort + 0.5);
  if (ll < -_LL) {
    ll = -_LL;
  } else if (ll > _LL) {
    ll = _LL;
  }
  double unfoldedVel = fields.vel_prt_short + _PP[ll] * _nyquistPrtShort * 2;
  fields.vel = unfoldedVel;
  fields.vel_unfold_interval = _PP[ll];
  fields.vel_diff = vel_diff;

  ////////////////////////////////////////////////////////////////////////////////
  // spectrum width

  if (snrHcOK && snrVcOK) {
    
    // compute spectrum width using R(0)/R(m) - the default
    
    double R0_hc = lag0_hc_short - _estNoisePowerHc;
    double R0_vc = lag0_vc_short - _estNoisePowerVc;
    
    double Rm_hc = lag1_hc_long_to_short_mag;
    double Rm_vc = lag1_vc_long_to_short_mag;
    
    double width_R0Rm_hc = _computeStagWidth(R0_hc, Rm_hc, 0, _staggeredM, 1.0);
    double width_R0Rm_vc = _computeStagWidth(R0_vc, Rm_vc, 0, _staggeredM, 1.0);
    
    double width_R0Rm = (width_R0Rm_hc + width_R0Rm_vc) / 2.0;
    
    fields.width = _constrain(width_R0Rm * _nyquistStagNominal,
                              0.01, _nyquistPrtShort);
     
    // widths from long and short prt sequences
    
    double r0_short_hc = lag0_hc_short - _estNoisePowerHc;
    double r1_short_hc = lag1_hc_short_mag;
    double r0r1_short_hc = 
      _computeR0R1Width(r0_short_hc, r1_short_hc, _nyquistShortPlusLong);

    double r0_short_vc = lag0_vc_short - _estNoisePowerVc;
    double r1_short_vc = lag1_vc_short_mag;
    double r0r1_short_vc = 
      _computeR0R1Width(r0_short_vc, r1_short_vc, _nyquistShortPlusLong);
    
    double r0r1_short = (r0r1_short_hc + r0r1_short_vc) / 2.0;
    fields.width_prt_short = r0r1_short;

    double r0_long_hc = lag0_hc_long - _estNoisePowerHc;
    double r1_long_hc = lag1_hc_long_mag;
    double r0r1_long_hc = 
      _computeR0R1Width(r0_long_hc, r1_long_hc, _nyquistShortPlusLong);

    double r0_long_vc = lag0_vc_long - _estNoisePowerVc;
    double r1_long_vc = lag1_vc_long_mag;
    double r0r1_long_vc = 
      _computeR0R1Width(r0_long_vc, r1_long_vc, _nyquistShortPlusLong);
    
    double r0r1_long = (r0r1_long_hc + r0r1_long_vc) / 2.0;
    fields.width_prt_long = r0r1_long;
    
    if (_widthMethod != WIDTH_METHOD_R0R1) {
      
      // use hybrid width estimator
      
      // spectrum width using R(m)/R(m+n)
      
      double Rmplusn_hc = RadarComplex::mag(lag1_hc_short);
      double width_RmRmpn_hc =
        _computeStagWidth(Rm_hc, Rmplusn_hc,
                          _staggeredM, _staggeredM + _staggeredN, 1.0);
      
      double Rmplusn_vc = RadarComplex::mag(lag1_vc_short);
      double width_RmRmpn_vc =
        _computeStagWidth(Rm_vc, Rmplusn_vc,
                          _staggeredM, _staggeredM + _staggeredN, 1.0);
      
      double width_RmRmpn = (width_RmRmpn_hc + width_RmRmpn_vc) / 2.0;
      
      // spectrum width using R(m)/R(n)
      
      double Rn_hc = RadarComplex::mag(lag1_hc_long_to_short);
      double width_RmRn_hc =
        _computeStagWidth(Rm_hc, Rn_hc, _staggeredM, _staggeredN, 1.0);
      
      double Rn_vc = RadarComplex::mag(lag1_vc_long_to_short);
      double width_RmRn_vc =
        _computeStagWidth(Rm_vc, Rn_vc, _staggeredM, _staggeredN, 1.0);
      
      double width_RmRn = (width_RmRn_hc + width_RmRn_vc) / 2.0;
      
      // hybrid
      
      double hybrid = width_RmRn;
      if (width_R0Rm > 0.1) {
        hybrid = width_R0Rm;
      } else if (width_RmRmpn < 0.05) {
        hybrid = width_RmRmpn;
      }
      
      fields.width = _constrain(hybrid * _nyquist, 0.01, _nyquist);
      
    } // if (_widthMethod != WIDTH_METHOD_R01) 

  } // if (snrHcOK && snrVcOK)

  //////////////////////////////////////////////////////////////////////////////
  // phidp and rhohv

  double lag0_hc = (lag0_hc_short + lag0_hc_long) / 2.0;
  double lag0_vc = (lag0_vc_short + lag0_vc_long) / 2.0;

  double lag0_hc_ns = lag0_hc - _estNoisePowerHc;
  double lag0_vc_ns = lag0_vc - _estNoisePowerVc;

  RadarComplex_t Rvvhh0((rvvhh0_long.re + rvvhh0_short.re) / 2.0,
                        (rvvhh0_long.im + rvvhh0_short.im) / 2.0);

  RadarComplex_t phidp = RadarComplex::conjugateProduct(Rvvhh0, _phidpOffsetSim);
  double phidpRad = RadarComplex::argRad(phidp);
  fields.phidp = phidpRad * RAD_TO_DEG * _velSign * _phidpSign;
  
  double phidpRad0 = RadarComplex::argRad(Rvvhh0);
  fields.phidp0 = phidpRad0 * RAD_TO_DEG * _velSign * _phidpSign;
  
  double Rvvhh0Mag = RadarComplex::mag(Rvvhh0);
  if (lag0_hc_ns > 0 && lag0_vc_ns > 0) {
    // noise corrected
    double rhohv = Rvvhh0Mag / sqrt(lag0_hc_ns * lag0_vc_ns);
    fields.rhohv = _constrain(rhohv, 0.0, 1.0);
  }
  {
    // not noise corrected
    double rhohv_nnc = Rvvhh0Mag / sqrt(lag0_hc * lag0_vc);
    fields.rhohv_nnc = _constrain(rhohv_nnc, 0.0, 1.0);
  }

}

///////////////////////////////////////////////////////////
// Dual-pol sim HV staggered PRT power

void RadarMoments::dpSimHvStagPrtPower(double lag0_hc,
                                       double lag0_vc,
                                       int gateNum,
                                       bool isFiltered,
                                       MomentsFields &fields)
  
{

  // lag0

  fields.lag0_hc_db = 10.0 * log10(lag0_hc);
  fields.lag0_vc_db = 10.0 * log10(lag0_vc);

  // compute dbm
  
  double dbm_hc = 10.0 * log10(lag0_hc) - _receiverGainDbHc;
  double dbm_vc = 10.0 * log10(lag0_vc) - _receiverGainDbVc;
  
  fields.dbmhc = dbm_hc;
  fields.dbmvc = dbm_vc;
  fields.dbm = (dbm_hc + dbm_vc) / 2.0;

  // compute noise-subtracted lag0
  
  double lag0_hc_ns = lag0_hc - _estNoisePowerHc;
  double lag0_vc_ns = lag0_vc - _estNoisePowerVc;
  
  // check SNR
  
  bool snrHcOK = true;
  double min_valid_pwr_hc = _estNoisePowerHc * _minDetectableSnr;
  if (lag0_hc_ns < min_valid_pwr_hc) {
    snrHcOK = false;
    fields.censoring_flag = 1;
  }

  bool snrVcOK = true;
  double min_valid_pwr_vc = _estNoisePowerVc * _minDetectableSnr;
  if (lag0_vc_ns < min_valid_pwr_vc) {
    snrVcOK = false;
    fields.censoring_flag = 1;
  }
  
  // compute snr
  
  double snr_hc = lag0_hc_ns / _calNoisePowerHc;
  double snr_vc = lag0_vc_ns / _calNoisePowerVc;
  
  if (snrHcOK) {
    fields.dbmhc_ns = 10.0 * log10(lag0_hc_ns) - _receiverGainDbHc;
    fields.snrhc = 10.0 * log10(snr_hc);
  } else {
    fields.snrhc = _missing;
  }
  if (snrVcOK) {
    fields.dbmvc_ns = 10.0 * log10(lag0_vc_ns) - _receiverGainDbVc;
    fields.snrvc = 10.0 * log10(snr_vc);
  } else {
    fields.snrvc = _missing;
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

}

///////////////////////////////////////////////////////////
// Dual pol, H-only transmit, receive fixed channels
// Staggered-PRT

void RadarMoments::dpHOnlyStagPrt(RadarComplex_t *iqhc,
                                  RadarComplex_t *iqvx,
                                  RadarComplex_t *iqhcShort,
                                  RadarComplex_t *iqvxShort,
                                  RadarComplex_t *iqhcLong,
                                  RadarComplex_t *iqvxLong,
                                  int gateNum,
                                  bool isFiltered,
                                  MomentsFields &fields)
  
{

  // initialize field meta data

  _setFieldMetaData(fields);

  if (gateNum >= _nGatesPrtLong) {
    // no data beyond this gate
    return;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // power-related fields
  
  double lag0_hc_long = RadarComplex::meanPower(iqhcLong, _nSamplesHalf - 1);
  double lag0_vx_long = RadarComplex::meanPower(iqvxLong, _nSamplesHalf - 1);
  
  dpHOnlyStagPrtPower(lag0_hc_long, lag0_vx_long, gateNum, isFiltered, fields);
  bool snrHcOK = (fields.snrhc != _missing);
  
  if (gateNum >= _nGatesPrtShort) {
    // only power beyond short PRT region, no phase-based fields
    return;
  }

  /////////////////////////////////////////////////////////////////////////////
  // ncp
  
  double lag0_hc_short = RadarComplex::meanPower(iqhcShort, _nSamplesHalf - 1);
  RadarComplex_t lag1_hc_short =
    RadarComplex::meanConjugateProduct(iqhcShort, iqhcShort + 1, _nSamplesHalf - 1);
  double ncp = RadarComplex::mag(lag1_hc_short) / lag0_hc_short;
  ncp = _constrain(ncp, 0.0, 1.0);
  fields.ncp = ncp;

  ////////////////////////////////////////////////////////////////////////////////
  // CPA and refract

  if (!isFiltered) {
    
    // refractivity
    
    computeRefract(iqhc, _nSamples, fields.aiq_hc, fields.niq_hc, _changeAiqSign);

    // CPA
    
    if (_computeCpaUsingAlt) {
      fields.cpa = computeCpaAlt(iqhc, _nSamples);
    } else {
      fields.cpa = computeCpa(iqhc, _nSamples);
    }

  }

  ////////////////////////////////////////////////////////////////////////////////
  // velocity

  // compute velocity short PRT data
  
  RadarComplex_t lag1_hc_short_to_long =
    RadarComplex::meanConjugateProduct(iqhcShort, iqhcLong, _nSamplesHalf - 1);
  double argVelShort = RadarComplex::argRad(lag1_hc_short_to_long);
  double velShort = (argVelShort / M_PI) * _nyquistPrtShort;
  fields.vel_prt_short = velShort * _velSign * _velSignStaggered * -1.0;

  fields.phase_for_noise = lag1_hc_short_to_long;

  // compute velocity long PRT data
  
  RadarComplex_t lag1_hc_long_to_short =
    RadarComplex::meanConjugateProduct(iqhcLong, iqhcShort + 1, _nSamplesHalf - 1);
  double argVelLong = RadarComplex::argRad(lag1_hc_long_to_short);
  double velLong = (argVelLong / M_PI) * _nyquistPrtLong;
  fields.vel_prt_long = velLong * _velSign * _velSignStaggered * -1.0;

  // compute the unfolded velocity

  double vel_diff = fields.vel_prt_short - fields.vel_prt_long;
  double nyquistDiff = _nyquistPrtShort - _nyquistPrtLong;
  double nyquistIntervalShort = (vel_diff / nyquistDiff) / 2.0;
  int ll = (int) floor(nyquistIntervalShort + 0.5);
  if (ll < -_LL) {
    ll = -_LL;
  } else if (ll > _LL) {
    ll = _LL;
  }
  double unfoldedVel = fields.vel_prt_short + _PP[ll] * _nyquistPrtShort * 2;
  fields.vel = unfoldedVel;
  fields.vel_unfold_interval = _PP[ll];
  fields.vel_diff = vel_diff;

  ////////////////////////////////////////////////////////////////////////////////
  // spectrum width

  if (snrHcOK) {
    
    // compute spectrum width using R(0)/R(m)
    
    double R0_hc = lag0_hc_short - _estNoisePowerHc;
    double lag1_hc_long_to_short_mag = RadarComplex::mag(lag1_hc_long_to_short);
    double Rm_hc = lag1_hc_long_to_short_mag;
    double width_R0Rm_hc = _computeStagWidth(R0_hc, Rm_hc, 0, _staggeredM, 1.0);
    
    fields.width = _constrain(width_R0Rm_hc * _nyquistStagNominal,
                              0.01, _nyquistPrtShort);
    
    // widths from long and short prt sequences

    RadarComplex_t lag1_hc_long =
      RadarComplex::meanConjugateProduct(iqhcLong + 1, iqhcLong, _nSamplesHalf - 1);
    RadarComplex_t lag1_hc_short =
      RadarComplex::meanConjugateProduct(iqhcShort + 1, iqhcShort, _nSamplesHalf - 1);

    double lag1_hc_long_mag = RadarComplex::mag(lag1_hc_long);
    double lag1_hc_short_mag = RadarComplex::mag(lag1_hc_short);
    
    double r0_short = lag0_hc_short - _estNoisePowerHc;
    double r1_short = lag1_hc_short_mag;
    double r0r1_short = _computeR0R1Width(r0_short, r1_short, _nyquistShortPlusLong);
    fields.width_prt_short = _constrain(r0r1_short, 0.01, _nyquistShortPlusLong);
    
    double r0_long = lag0_hc_long - _estNoisePowerHc;
    double r1_long = lag1_hc_long_mag;
    double r0r1_long = _computeR0R1Width(r0_long, r1_long, _nyquistShortPlusLong);
    fields.width_prt_long = _constrain(r0r1_long, 0.01, _nyquistShortPlusLong);
    
    if (_widthMethod != WIDTH_METHOD_R0R1) {
      
      // computing hybrid width estimator
      
      // spectrum width using R(m)/R(m+n)
      
      double Rmplusn_hc = RadarComplex::mag(lag1_hc_short);
      double width_RmRmpn_hc =
        _computeStagWidth(Rm_hc, Rmplusn_hc,
                          _staggeredM, _staggeredM + _staggeredN, 1.0);
      
      // spectrum width using R(m)/R(n)
      
      double Rn_hc = RadarComplex::mag(lag1_hc_long_to_short);
      double width_RmRn_hc =
        _computeStagWidth(Rm_hc, Rn_hc, _staggeredM, _staggeredN, 1.0);
      
      // hybrid
      
      double hybrid = width_RmRn_hc;
      if (width_R0Rm_hc > 0.1) {
        hybrid = width_R0Rm_hc;
      } else if (width_RmRmpn_hc < 0.05) {
        hybrid = width_RmRmpn_hc;
      }
      
      fields.width = _constrain(hybrid * _nyquist, 0.01, _nyquist);
      
    } // if (_widthMethod != WIDTH_METHOD_R0R1) 

  } // if (snrHcOK)

}

///////////////////////////////////////////////////////////
// Dual-pol H-only transmit staggered PRT power

void RadarMoments::dpHOnlyStagPrtPower(double lag0_hc,
                                       double lag0_vx,
                                       int gateNum,
                                       bool isFiltered,
                                       MomentsFields &fields)
  
{

  _setFieldMetaData(fields);

  // lag0

  fields.lag0_hc_db = 10.0 * log10(lag0_hc);
  fields.lag0_vx_db = 10.0 * log10(lag0_vx);
  
  // compute dbm
  
  double dbm_hc = 10.0 * log10(lag0_hc) - _receiverGainDbHc;
  double dbm_vx = 10.0 * log10(lag0_vx) - _receiverGainDbVx;
  
  fields.dbmhc = dbm_hc;
  fields.dbmvx = dbm_vx;
  fields.dbm = dbm_hc;

  // compute noise-subtracted lag0
  
  double lag0_hc_ns = lag0_hc - _estNoisePowerHc;
  double lag0_vx_ns = lag0_vx - _estNoisePowerVx;
  
  // check SNR
  
  bool snrHcOK = true;
  double min_valid_pwr_hc = _estNoisePowerHc * _minDetectableSnr;
  if (lag0_hc_ns < min_valid_pwr_hc) {
    snrHcOK = false;
    fields.censoring_flag = 1;
  }

  bool snrVxOK = true;
  double min_valid_pwr_vx = _estNoisePowerVx * _minDetectableSnr;
  if (lag0_vx_ns < min_valid_pwr_vx) {
    snrVxOK = false;
    fields.censoring_flag = 1;
  }
  
  // compute snr
  
  double snr_hc = lag0_hc_ns / _calNoisePowerHc;
  double snr_vx = lag0_vx_ns / _calNoisePowerVx;
  
  if (snrHcOK) {
    fields.dbmhc_ns = 10.0 * log10(lag0_hc_ns) - _receiverGainDbHc;
    fields.snrhc = 10.0 * log10(snr_hc);
    fields.snr = fields.snrhc;
  } else {
    fields.dbmhc_ns = _missing;
    fields.snr = _missing;
    fields.snrhc = _missing;
  }
  if (snrVxOK) {
    fields.dbmvx_ns = 10.0 * log10(lag0_vx_ns) - _receiverGainDbVx;
    fields.snrvx = 10.0 * log10(snr_vx);
  } else {
    fields.dbmvx_ns = _missing;
    fields.snrvx = _missing;
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
    
  if (snrVxOK) {
    double dbz_vx_no_atten_corr = 
      10.0 * log10(snr_vx) + _baseDbz1kmVx + _rangeCorr[gateNum] + _dbzCorrection;
    double dbz_vx = dbz_vx_no_atten_corr + _atmosAttenCorr[gateNum];
    fields.dbzvx = _adjustDbzForPwrH(dbz_vx);
  } else {
    fields.dbzvx = _missing;
  }
    
  // measured zdr as hc minus vx

  if (snrHcOK && snrVxOK) {
    double zdrm = 10.0 * log10(lag0_hc_ns / lag0_vx_ns);
    fields.zdrm = zdrm;
  } else {
    fields.zdrm = _missing;
  }
  fields.zdr = _missing;

  // ldr

  if (snrHcOK && (fields.snrhc > _minSnrDbForLdr) &&
      snrVxOK && (fields.snrvx > _minSnrDbForLdr)) {
    double ldrhm = 10.0 * log10(lag0_vx_ns / lag0_hc_ns);
    fields.ldrhm = ldrhm;
    fields.ldrh = ldrhm + _ldrCorrectionDbH;
    fields.ldr = fields.ldrh;
  } else {
    fields.ldrhm = _missing;
    fields.ldrh = _missing;
    fields.ldr = _missing;
  }
  fields.ldr_diff = _missing;
  fields.ldr_mean = _missing;

}

///////////////////////////////////////////////////////////
// Dual pol, V-only transmit, receive fixed channels
// Staggered-PRT

void RadarMoments::dpVOnlyStagPrt(RadarComplex_t *iqvc,
                                  RadarComplex_t *iqhx,
                                  RadarComplex_t *iqvcShort,
                                  RadarComplex_t *iqhxShort,
                                  RadarComplex_t *iqvcLong,
                                  RadarComplex_t *iqhxLong,
                                  int gateNum,
                                  bool isFiltered,
                                  MomentsFields &fields)
  
{
  
  // initialize field meta data

  _setFieldMetaData(fields);

  if (gateNum >= _nGatesPrtLong) {
    // no data beyond this gate
    return;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // power-related fields
  
  double lag0_vc_long = RadarComplex::meanPower(iqvcLong, _nSamplesHalf - 1);
  double lag0_hx_long = RadarComplex::meanPower(iqhxLong, _nSamplesHalf - 1);
  
  dpVOnlyStagPrtPower(lag0_vc_long, lag0_hx_long, gateNum, isFiltered, fields);
  bool snrVcOK = (fields.snrvc != _missing);
  
  if (gateNum >= _nGatesPrtShort) {
    // only power beyond short PRT region, no phase-based fields
    return;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // ncp
  
  double lag0_vc_short = RadarComplex::meanPower(iqvcShort, _nSamplesHalf - 1);
  RadarComplex_t lag1_vc_short =  RadarComplex::meanConjugateProduct
    (iqvcShort, iqvcShort + 1, _nSamplesHalf - 1);
  double ncp = RadarComplex::mag(lag1_vc_short) / lag0_vc_short;
  ncp = _constrain(ncp, 0.0, 1.0);
  fields.ncp = ncp;

  ////////////////////////////////////////////////////////////////////////////////
  // CPA and refract

  if (!isFiltered) {
    
    // refractivity
    
    computeRefract(iqvc, _nSamples, fields.aiq_vc, fields.niq_vc, _changeAiqSign);

    // CPA
    
    if (_computeCpaUsingAlt) {
      fields.cpa = computeCpaAlt(iqvc, _nSamples);
    } else {
      fields.cpa = computeCpa(iqvc, _nSamples);
    }

  }

  ////////////////////////////////////////////////////////////////////////////////
  // velocity

  // compute velocity short PRT data
  
  RadarComplex_t lag1_vc_short_to_long =
    RadarComplex::meanConjugateProduct(iqvcShort, iqvcLong, _nSamplesHalf - 1);
  double argVelShort = RadarComplex::argRad(lag1_vc_short_to_long);
  double velShort = (argVelShort / M_PI) * _nyquistPrtShort;
  fields.vel_prt_short = velShort * _velSign * _velSignStaggered * -1.0;

  fields.phase_for_noise = lag1_vc_short_to_long;

  // compute velocity long PRT data
  
  RadarComplex_t lag1_vc_long_to_short =
    RadarComplex::meanConjugateProduct(iqvcLong, iqvcShort + 1, _nSamplesHalf - 1);
  double argVelLong = RadarComplex::argRad(lag1_vc_long_to_short);
  double velLong = (argVelLong / M_PI) * _nyquistPrtLong;
  fields.vel_prt_short = velLong * _velSign * _velSignStaggered * -1.0;

  // compute the unfolded velocity

  double vel_diff = fields.vel_prt_short - fields.vel_prt_long;
  double nyquistDiff = _nyquistPrtShort - _nyquistPrtLong;
  double nyquistIntervalShort = (vel_diff / nyquistDiff) / 2.0;
  int ll = (int) floor(nyquistIntervalShort + 0.5);
  if (ll < -_LL) {
    ll = -_LL;
  } else if (ll > _LL) {
    ll = _LL;
  }
  double unfoldedVel = fields.vel_prt_short + _PP[ll] * _nyquistPrtShort * 2;
  fields.vel = unfoldedVel;
  fields.vel_unfold_interval = _PP[ll];
  fields.vel_diff = vel_diff;

  ////////////////////////////////////////////////////////////////////////////////
  // spectrum width

  if (snrVcOK) {
    
    // compute spectrum width using R(0)/R(m)
    
    double R0_vc = lag0_vc_short - _estNoisePowerVc;
    double lag1_vc_long_to_short_mag = RadarComplex::mag(lag1_vc_long_to_short);
    double Rm_vc = lag1_vc_long_to_short_mag;
    double width_R0Rm_vc = _computeStagWidth(R0_vc, Rm_vc, 0, _staggeredM, 1.0);
    
    fields.width = _constrain(width_R0Rm_vc * _nyquistStagNominal,
                              0.01, _nyquistPrtShort);
    
    // widths from long and short prt sequences

    RadarComplex_t lag1_vc_long =
      RadarComplex::meanConjugateProduct(iqvcLong + 1, iqvcLong, _nSamplesHalf - 1);
    RadarComplex_t lag1_vc_short =
      RadarComplex::meanConjugateProduct(iqvcShort + 1, iqvcShort, _nSamplesHalf - 1);

    double lag1_vc_long_mag = RadarComplex::mag(lag1_vc_long);
    double lag1_vc_short_mag = RadarComplex::mag(lag1_vc_short);
    
    double r0_short = lag0_vc_short - _estNoisePowerVc;
    double r1_short = lag1_vc_short_mag;
    double r0r1_short = _computeR0R1Width(r0_short, r1_short, _nyquistShortPlusLong);
    fields.width_prt_short = _constrain(r0r1_short, 0.01, _nyquistShortPlusLong);
    
    double r0_long = lag0_vc_long - _estNoisePowerVc;
    double r1_long = lag1_vc_long_mag;
    double r0r1_long = _computeR0R1Width(r0_long, r1_long, _nyquistShortPlusLong);
    fields.width_prt_long = _constrain(r0r1_long, 0.01, _nyquistShortPlusLong);
    
    if (_widthMethod != WIDTH_METHOD_R0R1) {
      
      // computing hybrid width estimator
      
      // spectrum width using R(m)/R(m+n)
      
      double Rmplusn_vc = RadarComplex::mag(lag1_vc_short);
      double width_RmRmpn_vc =
        _computeStagWidth(Rm_vc, Rmplusn_vc,
                          _staggeredM, _staggeredM + _staggeredN, 1.0);
      
      // spectrum width using R(m)/R(n)
      
      double Rn_vc = RadarComplex::mag(lag1_vc_long_to_short);
      double width_RmRn_vc =
        _computeStagWidth(Rm_vc, Rn_vc, _staggeredM, _staggeredN, 1.0);
      
      // hybrid
      
      double hybrid = width_RmRn_vc;
      if (width_R0Rm_vc > 0.1) {
        hybrid = width_R0Rm_vc;
      } else if (width_RmRmpn_vc < 0.05) {
        hybrid = width_RmRmpn_vc;
      }
      
      fields.width = _constrain(hybrid * _nyquist, 0.01, _nyquist);
      
    } // if (_widthMethod != WIDTH_METHOD_R0R1) 

  } // if (snrVcOK)

}

///////////////////////////////////////////////////////////
// Dual-pol V-only transmit staggered PRT power

void RadarMoments::dpVOnlyStagPrtPower(double lag0_vc,
                                       double lag0_hx,
                                       int gateNum,
                                       bool isFiltered,
                                       MomentsFields &fields)
  
{

  // lag0

  fields.lag0_vc_db = 10.0 * log10(lag0_vc);
  fields.lag0_hx_db = 10.0 * log10(lag0_hx);
  
  // compute dbm
  
  double dbm_vc = 10.0 * log10(lag0_vc) - _receiverGainDbVc;
  double dbm_hx = 10.0 * log10(lag0_hx) - _receiverGainDbHx;
  
  fields.dbmvc = dbm_vc;
  fields.dbmhx = dbm_hx;
  fields.dbm = dbm_vc;

  // compute noise-subtracted lag0
  
  double lag0_vc_ns = lag0_vc - _estNoisePowerVc;
  double lag0_hx_ns = lag0_hx - _estNoisePowerHx;
  
  // check SNR
  
  bool snrVcOK = true;
  double min_valid_pwr_vc = _estNoisePowerVc * _minDetectableSnr;
  if (lag0_vc_ns < min_valid_pwr_vc) {
    snrVcOK = false;
    fields.censoring_flag = 1;
  }
  
  bool snrHxOK = true;
  double min_valid_pwr_hx = _estNoisePowerHx * _minDetectableSnr;
  if (lag0_hx_ns < min_valid_pwr_hx) {
    snrHxOK = false;
    fields.censoring_flag = 1;
  }

  // compute snr
  
  double snr_vc = lag0_vc_ns / _calNoisePowerVc;
  double snr_hx = lag0_hx_ns / _calNoisePowerHx;
  
  if (snrVcOK) {
    fields.dbmvc_ns = 10.0 * log10(lag0_vc_ns) - _receiverGainDbVc;
    fields.snrvc = 10.0 * log10(snr_vc);
    fields.snr = fields.snrvc;
  } else {
    fields.snrvc = _missing;
    fields.snr = _missing;
  }
  if (snrHxOK) {
    fields.dbmhx_ns = 10.0 * log10(lag0_hx_ns) - _receiverGainDbHx;
    fields.snrhx = 10.0 * log10(snr_hx);
  } else {
    fields.snrhx = _missing;
  }
  
  // dbz
  
  if (snrVcOK) {
    double dbz_vc_no_atten_corr = 
      10.0 * log10(snr_vc) + _baseDbz1kmVc + _rangeCorr[gateNum] + _dbzCorrection;
    double dbz_vc = dbz_vc_no_atten_corr + _atmosAttenCorr[gateNum];
    fields.dbzvc = _adjustDbzForPwrV(dbz_vc);
    fields.dbz = fields.dbzvc;
    fields.dbz_no_atmos_atten = _adjustDbzForPwrV(dbz_vc_no_atten_corr);
  } else {
    fields.dbz = _missing;
    fields.dbzvc = _missing;
    fields.dbz_no_atmos_atten = _missing;
  }
    
  if (snrHxOK) {
    double dbz_hx_no_atten_corr =
      10.0 * log10(snr_hx) + _baseDbz1kmHx + _rangeCorr[gateNum] + _dbzCorrection;
    double dbz_hx = dbz_hx_no_atten_corr + _atmosAttenCorr[gateNum];
    fields.dbzhx = _adjustDbzForPwrV(dbz_hx);
  } else {
    fields.dbzhx = _missing;
  }
    
  // measured zdr as hx minus vc

  if (snrHxOK && snrVcOK) {
    double zdrm = 10.0 * log10(lag0_hx_ns / lag0_vc_ns);
    fields.zdrm = zdrm;
  } else {
    fields.zdrm = _missing;
  }
  fields.zdr = _missing;

  // ldr

  if (snrVcOK && (fields.snrvc > _minSnrDbForLdr) &&
      snrHxOK && (fields.snrhx > _minSnrDbForLdr)) {
    double ldrvm = 10.0 * log10(lag0_hx_ns / lag0_vc_ns);
    fields.ldrvm = ldrvm;
    fields.ldrv = ldrvm + _ldrCorrectionDbV;
    fields.ldr = fields.ldrv;
  } else {
    fields.ldrvm = _missing;
    fields.ldrv = _missing;
    fields.ldr = _missing;
  }

}

///////////////////////////////////////////////////////////
// Single polarization, range unfolding using SZ864

void RadarMoments::singlePolSz864(GateData &gateData,
                                  RadarComplex_t *delta12,
                                  int gateNum,
                                  int ngatesPulse,
                                  const RadarFft &fft)
  
{

  if (!_applySz) {
    cerr << "ERROR - RadarMoments::spSz864" << endl;
    cerr << "SZ not supported" << endl;
    return;
  }

  // separate the trips

  _sz->separateTrips(gateData, delta12, _prt, fft);
  gateData.fields.sz_leakage = gateData.szLeakage;

  if (gateData.trip1IsStrong) {
    gateData.fields.sz_trip_flag = 1;
    gateData.secondTrip.sz_trip_flag = 2;
  } else {
    gateData.fields.sz_trip_flag = 2;
    gateData.secondTrip.sz_trip_flag = 1;
  }
    
  // compute strong trip moments, checking censoring flag first
  
  if (!gateData.censorStrong) {
    
    if (gateData.trip1IsStrong) {
      singlePol(gateData.iqStrong, gateNum, false, gateData.fields);
    } else {
      singlePol(gateData.iqStrong, gateNum + ngatesPulse, false, gateData.secondTrip);
    }
    
  }
    
  // compute weak trip moments, checking censoring flag first
  
  if (!gateData.censorWeak) {
    
    if (gateData.trip1IsStrong) {
      singlePol(gateData.iqWeak, gateNum + ngatesPulse, false, gateData.secondTrip);
    } else {
      singlePol(gateData.iqWeak, gateNum, false, gateData.fields);
    }
    
  }

}

///////////////////////////////////////////////////////////
// Single polarization, SZ864, Filtered

void RadarMoments::singlePolSz864Filtered(GateData &gateData,
                                          int gateNum,
                                          int ngatesPulse)
  
{
  
  // compute strong trip moments for filtered data
  
  if (gateData.clutterInStrong) {
    if (gateData.trip1IsStrong) {
      singlePol(gateData.iqStrongF, gateNum, true, gateData.fieldsF);
    } else {
      singlePol(gateData.iqStrongF, gateNum + ngatesPulse,
                true, gateData.secondTripF);
    }
  }
    
  // compute weak trip moments for filtered data
  
  if (gateData.clutterInWeak) {
    if (gateData.trip1IsStrong) {
      singlePol(gateData.iqWeakF, gateNum + ngatesPulse, true, gateData.secondTripF);
    } else {
      singlePol(gateData.iqWeakF, gateNum, true, gateData.fieldsF);
    }
  }
    
}

/////////////////////////////////////////////////////
// apply clutter filter to IQ time series
//
// Inputs:
//   nSamples
//   fft: object to be used for FFT computations
//   regr: object to be used for regression filtering
//   window: window used to create iqWindowed
//   iqOrig: unfltered time series, not windowed
//   iqWindowed: unfiltered time series, windowed using VONHANN or BLACKMAN
//   specWindowed: if not NULL, contains the spectrum of iqWindowed
//   calibratedNoise: noise level at digitizer, from cal
//
//  Outputs:
//    iqFiltered: filtered time series
//    filterRatio: ratio of raw to unfiltered power, before applying correction
//    spectralNoise: spectral noise estimated from the spectrum
//    spectralSnr: ratio of spectral noise to noise power
//    specRatio: ratio of filtered to unfiltered in spectrum, if non-NULL

void RadarMoments::applyClutterFilter(int nSamples,
                                      const RadarFft &fft,
                                      const RegressionFilter &regr,
                                      const double *window, // window in use
                                      const RadarComplex_t *iqOrig, // non-windowed
                                      const RadarComplex_t *iqWindowed, // windowed
                                      const RadarComplex_t *specWindowed,
                                      double calibratedNoise,
                                      RadarComplex_t *iqFiltered,
                                      double &filterRatio,
                                      double &spectralNoise,
                                      double &spectralSnr,
                                      double *specRatio /* = NULL*/)
  
{

  if (_useRegressionFilter) {
    
    applyRegressionFilter(nSamples, fft, regr, window,
                          iqOrig, calibratedNoise,
                          _regrInterpAcrossNotch,
                          iqFiltered, filterRatio,
                          spectralNoise, spectralSnr, specRatio);
    
  } else if (_useSimpleNotchFilter) {
    
    applyNotchFilter(nSamples, fft,
                     iqWindowed, specWindowed,
                     calibratedNoise,
                     iqFiltered, filterRatio,
                     spectralNoise, spectralSnr, specRatio);
    
  } else {
    
    applyAdaptiveFilter(nSamples, fft,
                        iqWindowed, specWindowed,
                        calibratedNoise,
                        iqFiltered, filterRatio,
                        spectralNoise, spectralSnr, specRatio);

  }
    
}

/////////////////////////////////////////////////////
// apply adaptive clutter filter to IQ time series
//
// Inputs:
//   nSamples
//   fft: object to be used for FFT computations
//   iqWindowed: unfiltered time series, pre-windowed using VONHANN or BLACKMAN
//   specWindowed: if not NULL, contains the spectrum of iqWindowed
//   calibratedNoise: noise level at digitizer, from cal
//
//  Outputs:
//    iqFiltered: filtered time series
//    filterRatio: ratio of raw to unfiltered power, before applying correction
//    spectralNoise: spectral noise estimated from the spectrum
//    spectralSnr: ratio of spectral noise to noise power
//    specRatio: ratio of filtered to unfiltered in spectrum, if non-NULL

void RadarMoments::applyAdaptiveFilter(int nSamples,
                                       const RadarFft &fft,
                                       const RadarComplex_t *iqWindowed,
                                       const RadarComplex_t *specWindowed,
                                       double calibratedNoise,
                                       RadarComplex_t *iqFiltered,
                                       double &filterRatio,
                                       double &spectralNoise,
                                       double &spectralSnr,
                                       double *specRatio /* = NULL*/)
  
{

  // If specWindowed is not NULL, it contains the spectrum of iqWindowed.
  // If it is NULL, we need to take the forward fft to compute the
  // raw complex power spectrum
  
  TaArray<RadarComplex_t> powerSpecC_;
  RadarComplex_t *powerSpecC = powerSpecC_.alloc(nSamples);
  if (specWindowed == NULL) {
    fft.fwd(iqWindowed, powerSpecC);
  } else {
    memcpy(powerSpecC, specWindowed, nSamples * sizeof(RadarComplex_t));
  }

  // load the raw power spectrum
  
  TaArray<double> powerSpec_;
  double *powerSpec = powerSpec_.alloc(nSamples);
  RadarComplex::loadPower(powerSpecC, powerSpec, nSamples);
    
  // allocate space for the filtered power spectrum
  
  TaArray<double> powerSpecF_;
  double *powerSpecF = powerSpecF_.alloc(nSamples);
  
  double rawPower = 0.0;
  double filteredPower = 0.0;
  double powerRemoved = 0.0;

  double maxClutterVel = 1.0;
  double initNotchWidth = 1.5;
  bool clutterFound = false;
  int notchStart = 0;
  int notchEnd = 0;
  
  ClutFilter::performAdaptive(powerSpec, nSamples, maxClutterVel,
                              initNotchWidth, _nyquist, calibratedNoise,
                              false, clutterFound, powerSpecF,
                              notchStart, notchEnd,
                              rawPower, filteredPower,
                              powerRemoved, spectralNoise);
  
  spectralSnr = (spectralNoise - calibratedNoise) / calibratedNoise;
  filterRatio = rawPower / filteredPower;
  
  if (powerRemoved > 0) {

    double correctionRatio =
      _computePwrCorrectionRatio(nSamples, spectralSnr,
 				 rawPower, filteredPower,
                                 powerRemoved, calibratedNoise);

    // correct the filtered powers for clutter residue
    
    for (int ii = 0; ii < nSamples; ii++) {
      powerSpecF[ii] *= correctionRatio;
    }
    
  }
  
  // adjust the input spectrum by the filter ratio
  // constrain ratios to be 1 or less

  for (int ii = 0; ii < nSamples; ii++) {
    double magRatio = sqrt(powerSpecF[ii] / powerSpec[ii]);
    if (magRatio > 1.0) {
      magRatio = 1.0;
    }
    powerSpecC[ii].re *= magRatio;
    powerSpecC[ii].im *= magRatio;
    if (specRatio != NULL) {
      specRatio[ii] = magRatio;
    }
  }

  // invert the fft

  fft.inv(powerSpecC, iqFiltered);
 
}

/////////////////////////////////////////////////////
// apply notch filter to IQ time series
//
// Inputs:
//   nSamples
//   fft: object to be used for FFT computations
//   iqWindowed: unfiltered time series, pre-windowed using VONHANN or BLACKMAN
//   specWindowed: if not NULL, contains the spectrum of iqWindowed
//   calibratedNoise: noise level at digitizer, from cal
//
//  Outputs:
//    iqFiltered: filtered time series
//    filterRatio: ratio of raw to unfiltered power, before applying correction
//    spectralNoise: spectral noise estimated from the spectrum
//    spectralSnr: ratio of spectral noise to noise power
//    specRatio: ratio of filtered to unfiltered in spectrum, if non-NULL

void RadarMoments::applyNotchFilter(int nSamples,
                                    const RadarFft &fft,
                                    const RadarComplex_t *iqWindowed,
                                    const RadarComplex_t *specWindowed,
                                    double calibratedNoise,
                                    RadarComplex_t *iqFiltered,
                                    double &filterRatio,
                                    double &spectralNoise,
                                    double &spectralSnr,
                                    double *specRatio /* = NULL*/)
  
{

  // If specWindowed is not NULL, it contains the spectrum of iqWindowed.
  // If it is NULL, we need to take the forward fft to compute the
  // raw complex power spectrum
  
  TaArray<RadarComplex_t> powerSpecC_;
  RadarComplex_t *powerSpecC = powerSpecC_.alloc(nSamples);
  if (specWindowed == NULL) {
    fft.fwd(iqWindowed, powerSpecC);
  } else {
    memcpy(powerSpecC, specWindowed, nSamples * sizeof(RadarComplex_t));
  }

  // load the raw power spectrum
  
  TaArray<double> powerSpec_;
  double *powerSpec = powerSpec_.alloc(nSamples);
  RadarComplex::loadPower(powerSpecC, powerSpec, nSamples);
    
  // allocate space for the filtered power spectrum
  
  TaArray<double> powerSpecF_;
  double *powerSpecF = powerSpecF_.alloc(nSamples);
  
  double rawPower = 0.0;
  double filteredPower = 0.0;
  double powerRemoved = 0.0;

  int notchStart = 0;
  int notchEnd = 0;
  
  ClutFilter::performNotch(powerSpec, nSamples,
                           _notchWidthMps, _nyquist,
                           calibratedNoise, powerSpecF,
                           notchStart, notchEnd,
                           rawPower, filteredPower, powerRemoved);
  
  spectralNoise = calibratedNoise;
  spectralSnr = 1.0;
  filterRatio = rawPower / filteredPower;
  
  if (powerRemoved > 0) {

    double correctionRatio =
      _computePwrCorrectionRatio(nSamples, spectralSnr,
 				 rawPower, filteredPower,
                                 powerRemoved, calibratedNoise);

    // correct the filtered powers for clutter residue
    
    for (int ii = 0; ii < nSamples; ii++) {
      powerSpecF[ii] *= correctionRatio;
    }
    
  }
  
  // adjust the input spectrum by the filter ratio
  // constrain ratios to be 1 or less

  for (int ii = 0; ii < nSamples; ii++) {
    double magRatio = sqrt(powerSpecF[ii] / powerSpec[ii]);
    if (magRatio > 1.0) {
      magRatio = 1.0;
    }
    powerSpecC[ii].re *= magRatio;
    powerSpecC[ii].im *= magRatio;
    if (specRatio != NULL) {
      specRatio[ii] = magRatio;
    }
  }

  // invert the fft

  fft.inv(powerSpecC, iqFiltered);
 
}

/////////////////////////////////////////////////////////////////
// apply polynomial regression clutter filter to IQ time series
//
// NOTE: IQ data should not be windowed.
//
// Inputs:
//   nSamples
//   fft: object to be used for filling in notch
//   regr: object to be used for polynomial computations
//   window: coefficients for window that is actively in use
//   iqOrig: unfiltered time series, not windowed
//   channel: which channel - used to determine calibrated noise
//
//  Outputs:
//    iqFiltered: filtered time series
//    filterRatio: ratio of raw to unfiltered power
//    spectralNoise: spectral noise estimated from the spectrum
//    spectralSnr: ratio of spectral noise to noise power
//    specRatio: if non-NULL, contains ratio of filtered to unfiltered spectrum

void RadarMoments::applyRegressionFilter
  (int nSamples,
   const RadarFft &fft,
   const RegressionFilter &regr,
   const double *window,
   const RadarComplex_t *iqOrig, // non-windowed
   double calibratedNoise,
   bool interpAcrossNotch,
   RadarComplex_t *iqFiltered,
   double &filterRatio,
   double &spectralNoise,
   double &spectralSnr,
   double *specRatio /* = NULL*/)
  
{

  
  // apply regression filter
  
  TaArray<RadarComplex_t> iqRegr_;
  RadarComplex_t *iqRegr = iqRegr_.alloc(nSamples);
  regr.apply(iqOrig, iqRegr);

  // adjust for residual etc, interpolating as needed

  _adjustRegressionFilter(nSamples, fft, window,
                          iqOrig, iqRegr,
                          calibratedNoise, interpAcrossNotch,
                          iqFiltered, filterRatio, spectralNoise,
                          spectralSnr, specRatio);
  
  // memcpy(iqFiltered, iqRegr, nSamples * sizeof(RadarComplex_t));

  double rawPower = RadarComplex::meanPower(iqOrig, nSamples);
  double filteredPower = RadarComplex::meanPower(iqFiltered, nSamples);
  filterRatio = rawPower / filteredPower;

}

/////////////////////////////////////////////////////////////////
// apply adaptive clutter filter to staggered PRT
// IQ time series
//
// The following is assumed:
//
//   1. nSamplesHalf refers to short and long prt sequences.
//      nSamples = nSamplesHalf * 2
//   2. The combined sequence starts with short PRT.
//   3. Memory has been allocated as follows:
//        iqOrigShort[nSamplesHalf]
//        iqOrigLong[nSamplesHalf]
//        iqFiltShort[nSamplesHalf]
//        iqFiltLong[nSamplesHalf]
//        spectralRatioShort[nSamplesHalf]
//        spectralRatioLong[nSamplesHalf]
//   4. Input and output data is windowed appropriately for FFTs.
//
// The short and long sequences are filtered separately.
// The notch is not filled in.
//
// Inputs:
//   fftHalf: object to be used for FFT computations
//   iqOrigShort: unfiltered short-prt time series
//   iqOrigLong: unfiltered long-prt time series
//   channel: which channel - used to determine calibrated noise
//   interpAcrossNotch: whether to fill in notch
//
//  Outputs:
//    iqFiltShort: filtered short-prt time series
//    iqFiltLong: filtered long-prt time series
//    filterRatio: ratio of raw to unfiltered power, before applying correction
//    spectralNoise: spectral noise estimated from the spectrum
//    spectralSnr: ratio of spectral noise to noise power
//    specRatioShort: filtered/unfiltered ratio, short PRT, if non-NULL
//    specRatioLong: filtered/unfiltered ratio, long PRT, if non-NULL
  
void RadarMoments::applyAdapFilterStagPrt(int nSamplesHalf,
                                          const RadarFft &fftHalf,
                                          const RadarComplex_t *iqShort,
                                          const RadarComplex_t *iqLong,
                                          double calibratedNoise,
                                          RadarComplex_t *iqFiltShort,
                                          RadarComplex_t *iqFiltLong,
                                          double &filterRatio,
                                          double &spectralNoise,
                                          double &spectralSnr,
                                          double *spectralRatioShort /* = NULL */,
                                          double *spectralRatioLong /* = NULL */)
  
{
  
  double filterNyquist = _nyquist / (_staggeredM + _staggeredN);
  
  // filter the short prt time series
  
  double filterRatioShort = 1.0;
  double spectralNoiseShort = 1.0e-13;
  double spectralSnrShort = 1.0;
  _adapFiltHalfTseries(_nSamplesHalf, fftHalf, iqShort, calibratedNoise,
                       filterNyquist, true, iqFiltShort,
                       filterRatioShort,
                       spectralNoiseShort,
                       spectralSnrShort,
                       spectralRatioShort);
  
  // filter the long prt time series
  
  double filterRatioLong = 1.0;
  double spectralNoiseLong = 1.0e-13;
  double spectralSnrLong = 1.0;
  _adapFiltHalfTseries(_nSamplesHalf, fftHalf, iqLong, calibratedNoise,
                       filterNyquist, true, iqFiltLong,
                       filterRatioLong,
                       spectralNoiseLong,
                       spectralSnrLong,
                       spectralRatioLong);
  
  filterRatio = (filterRatioShort + filterRatioLong) / 2.0;
  spectralNoise = (spectralNoiseShort + spectralNoiseLong) / 2.0;
  spectralSnr = (spectralSnrShort + spectralSnrLong) / 2.0;
  
}

////////////////////////////////////////////////////////////
// apply clutter filter to partial staggered PRT time series
// This is applied to horizontal and vertical separately.
//
// Inputs:
//   nSamples
//   fft: object to be used for FFT computations
//   iq: unfiltered time series
//   channel: HC, VC, HX, VX
//   adjustForPowerResidue: adjust filtered spectrum for power residue
//
//  Outputs:
//    iqFiltered: filtered time series
//    filterRatio: ratio of raw to unfiltered power, before applying correction
//    spectralNoise: spectral noise estimated from the spectrum
//    spectralSnr: ratio of spectral noise to noise power
//    specRatio: ratio of filtered to unfiltered in spectrum, if non-NULL
   
void RadarMoments::_adapFiltHalfTseries(int nSamplesHalf,
                                        const RadarFft &fftHalf,
                                        const RadarComplex_t *iq,
                                        double calibratedNoise,
                                        double nyquist,
                                        bool adjustForPowerResidue,
                                        RadarComplex_t *iqFiltered,
                                        double &filterRatio,
                                        double &spectralNoise,
                                        double &spectralSnr,
                                        double *specRatio /* = NULL*/)
  
{

  // take the forward fft to compute the complex power spectrum
  
  TaArray<RadarComplex_t> powerSpecC_;
  RadarComplex_t *powerSpecC = powerSpecC_.alloc(nSamplesHalf);
  fftHalf.fwd(iq, powerSpecC);

  // allocate space for the filtered power spectrum
  
  TaArray<double> powerSpecF_;
  double *powerSpecF = powerSpecF_.alloc(nSamplesHalf);
  
  // load the power spectrum
  
  TaArray<double> powerSpec_;
  double *powerSpec = powerSpec_.alloc(nSamplesHalf);
  RadarComplex::loadPower(powerSpecC, powerSpec, nSamplesHalf);
    
  double rawPower = 0.0;
  double filteredPower = 0.0;
  double powerRemoved = 0.0;
  
  if (_useSimpleNotchFilter) {

    int notchStart = 0;
    int notchEnd = 0;
    
    ClutFilter::performNotch(powerSpec, nSamplesHalf,
			     _notchWidthMps, nyquist,
                             calibratedNoise, powerSpecF,
			     notchStart, notchEnd,
			     rawPower, filteredPower, powerRemoved);
    
    spectralNoise = calibratedNoise;
    spectralSnr = 1.0;
    filterRatio = rawPower / filteredPower;

  } else {

    double maxClutterVel = 1.0;
    double initNotchWidth = 1.5;
    bool clutterFound = false;
    int notchStart = 0;
    int notchEnd = 0;
    
    ClutFilter::performAdaptive(powerSpec, nSamplesHalf, maxClutterVel,
				initNotchWidth, nyquist, calibratedNoise,
                                true, clutterFound, powerSpecF,
				notchStart, notchEnd,
				rawPower, filteredPower,
				powerRemoved, spectralNoise);
    
    spectralSnr = spectralNoise / calibratedNoise;
    filterRatio = rawPower / filteredPower;
    
  } // if (_useSimpleNotchFilter)

  if (adjustForPowerResidue && powerRemoved > 0) {
    
    double correctionRatio =
      _computePwrCorrectionRatio(nSamplesHalf, spectralSnr,
 				 rawPower, filteredPower,
                                 powerRemoved, calibratedNoise);

    // correct the filtered powers for clutter residue
    
    for (int ii = 0; ii < nSamplesHalf; ii++) {
      powerSpecF[ii] *= correctionRatio;
    }
    
  }
  
  // adjust the input spectrum by the filter ratio
  // constrain ratios to be 1 or less

  for (int ii = 0; ii < nSamplesHalf; ii++) {
    double magRatio = sqrt(powerSpecF[ii] / powerSpec[ii]);
    if (magRatio > 1.0) {
      magRatio = 1.0;
    }
    powerSpecC[ii].re *= magRatio;
    powerSpecC[ii].im *= magRatio;
    if (specRatio != NULL) {
      specRatio[ii] = magRatio;
    }
  }

  // invert the fft

  fftHalf.inv(powerSpecC, iqFiltered);
 
}

/////////////////////////////////////////////////////////////////
// apply polynomial regression clutter filter to IQ time series
//
// NOTE: IQ data should not be windowed.
//
// Inputs:
//   nSamples
//   fftHalf: fft object for short and long half time series, length nSamples/2
//   regr: object to be used for polynomial computations
//   iqOrig: unfiltered time series, not windowed
//   channel: which channel - used to determine calibrated noise
//
//  Outputs:
//    iqFiltered: filtered time series
//    filterRatio: ratio of raw to unfiltered power, before applying correction
//    spectralNoise: spectral noise estimated from the spectrum
//    spectralSnr: ratio of spectral noise to noise power
//    specRatio: if non-NULL, contains ratio of filtered to unfiltered spectrum
//
//  Memory allocation by calling routine:
//    regr - initialized to size nSamples
//    iqOrig[nSamples]
//    iqFiltered[nSamples]
//    specRatio[nSamples] - if non-NULL

void RadarMoments::applyRegrFilterStagPrt(int nSamples,
                                          const RadarFft &fftHalf,
                                          const RegressionFilter &regr,
                                          const RadarComplex_t *iqOrig,
                                          double calibratedNoise,
                                          bool interpAcrossNotch,
                                          RadarComplex_t *iqFiltered,
                                          double &filterRatio,
                                          double &spectralNoise,
                                          double &spectralSnr,
                                          double *specRatio /* = NULL*/)
  
{
  
  // apply the regression filter to the non-windowed (original) time series

  TaArray<RadarComplex_t> iqRegr_;
  RadarComplex_t *iqRegr = iqRegr_.alloc(nSamples);
  regr.apply(iqOrig, iqRegr);

  double powerOrig = RadarComplex::meanPower(iqOrig, nSamples);
  double powerRegr = RadarComplex::meanPower(iqRegr, nSamples);
  double powerClut = powerOrig - powerRegr;
  double csr = powerClut / powerRegr;

  // check for low CSR, is less than 10dB do not filter

  if (csr < .1) {
    memcpy(iqFiltered, iqOrig, nSamples * sizeof(RadarComplex_t));
    return;
  }
  
  // if no notch interpolation is required, return now

  if (!interpAcrossNotch || nSamples < 16) {
    memcpy(iqFiltered, iqRegr, nSamples * sizeof(RadarComplex_t));
    return;
  }

  // separate the filtered time series into two series,
  // for short and long PRT respectively

  int nSamplesHalf = nSamples / 2;
  TaArray<RadarComplex_t> filtShort_, filtLong_;
  RadarComplex_t *filtShort = filtShort_.alloc(nSamplesHalf);
  RadarComplex_t *filtLong = filtLong_.alloc(_nSamplesHalf);
  RadarMoments::separateStagIq(nSamples, iqFiltered, filtShort, filtLong);
  
  // compute the spectra of these 2 half series
  
  TaArray<RadarComplex_t> filtShortSpec_;
  RadarComplex_t *filtShortSpec = filtShortSpec_.alloc(nSamplesHalf);
  fftHalf.fwd(filtShort, filtShortSpec);

  TaArray<RadarComplex_t> filtLongSpec_;
  RadarComplex_t *filtLongSpec = filtLongSpec_.alloc(_nSamplesHalf);
  fftHalf.fwd(filtLong, filtLongSpec);

#ifdef JUNK

  // perform gaussian infill for powers, keeping phases unchanged

  TaArray<RadarComplex_t> filledShortSpec_;
  RadarComplex_t *filledShortSpec = filledShortSpec_.alloc(nSamplesHalf);

  TaArray<RadarComplex_t> filledLongSpec_;
  RadarComplex_t *filledLongSpec = filledLongSpec_.alloc(nSamplesHalf);

  int maxNotchWidth = 7;
  ClutFilter::fillNotchUsingGfit(filtShortSpec, nSamplesHalf,
                                 maxNotchWidth, filledShortSpec);
  
  ClutFilter::fillNotchUsingGfit(filtLongSpec, nSamplesHalf,
                                 maxNotchWidth, filledLongSpec);

  // for the long-PRT half spectrum, interpolate the phase difference
  // between the short and long
  
  int notchWidthHalf = maxNotchWidth / 2;
  if (notchWidthHalf > _nSamplesHalf - 1) notchWidthHalf = _nSamplesHalf - 1;
  int startIndex =  -notchWidthHalf;
  int endIndex = notchWidthHalf;

  RadarComplex_t diffStart =
    RadarComplex::conjugateProduct(filledLongSpec[startIndex],
                                   filledShortSpec[startIndex]);
  double diffPhaseStart = RadarComplex::argRad(diffStart);
  
  RadarComplex_t diffEnd =
    RadarComplex::conjugateProduct(filledLongSpec[endIndex],
                                   filledShortSpec[endIndex]);
  double diffPhaseEnd = RadarComplex::argRad(diffEnd);
  double deltaDiffPhase = RadarComplex::diffRad(diffPhaseEnd, diffPhaseStart);
  
  double count = 0.0;
  for (int ii = startIndex; ii <= endIndex; ii++, count++) {
    double interpFraction = count / (double) maxNotchWidth;
    int jj = (ii + _nSamplesHalf) % _nSamplesHalf;
    double interpDiffPhase = diffPhaseStart + interpFraction * deltaDiffPhase;
    double phaseShort = RadarComplex::argRad(filledShortSpec[jj]);
    double phaseLong = RadarComplex::sumRad(phaseShort, interpDiffPhase);
    double interpMag = RadarComplex::mag(filledLongSpec[jj]);
    filledLongSpec[jj].re = interpMag * cos(phaseLong);
    filledLongSpec[jj].im = interpMag * sin(phaseLong);
  }
 
  // invert from FFT space back into time series

  fftHalf.inv(filledShortSpec, filtShort);
  fftHalf.inv(filledLongSpec, filtLong);

#endif

  // for the short-PRT half spectrum, interpolate the power
  // across the filter notch, keeping the phase constant

  int notchWidth = 7;
  int notchWidthHalf = notchWidth / 2;
  int startIndex = nSamplesHalf - notchWidthHalf;
  int endIndex = notchWidthHalf;
  int nCenter = nSamplesHalf / 2;
  
  double powerStart = RadarComplex::power(filtShortSpec[startIndex]);
  double powerEnd = RadarComplex::power(filtShortSpec[endIndex]);
  double deltaPower = powerEnd - powerStart;
  
  for (int ii = nCenter - notchWidthHalf + 1;
       ii < nCenter + notchWidthHalf; ii++) {

    int jj = (ii + nSamplesHalf / 2) % nSamplesHalf;
    int kk = ii - (nCenter - notchWidthHalf);
    double interpFraction = (double) kk / (double) notchWidth;
    double interpPower = powerStart + interpFraction * deltaPower;
    double origPower = RadarComplex::power(filtShortSpec[jj]);
    double powerRatio = interpPower / origPower;
    double magRatio = sqrt(powerRatio);
    filtShortSpec[jj].re *= magRatio;
    filtShortSpec[jj].im *= magRatio;

  }

  // for the long-PRT half spectrum, interpolate the power
  // across the filter notch, and interpolate the phase difference
  // between the short and long

  powerStart = RadarComplex::power(filtLongSpec[startIndex]);
  powerEnd = RadarComplex::power(filtLongSpec[endIndex]);
  deltaPower = powerEnd - powerStart;
  
  RadarComplex_t diffStart =
    RadarComplex::conjugateProduct(filtLongSpec[startIndex],
                                   filtShortSpec[startIndex]);
  double diffPhaseStart = RadarComplex::argRad(diffStart);

  RadarComplex_t diffEnd =
    RadarComplex::conjugateProduct(filtLongSpec[endIndex],
                                   filtShortSpec[endIndex]);
  double diffPhaseEnd = RadarComplex::argRad(diffEnd);
  
  double deltaDiffPhase = RadarComplex::diffRad(diffPhaseEnd, diffPhaseStart);
  
  for (int ii = nCenter - notchWidthHalf + 1;
       ii < nCenter + notchWidthHalf; ii++) {
    
    int jj = (ii + nSamplesHalf / 2) % nSamplesHalf;
    int kk = ii - (nCenter - notchWidthHalf);
    double interpFraction = (double) kk / (double) notchWidth;
    
    // power

    double interpPower = powerStart + interpFraction * deltaPower;
    double origPower = RadarComplex::power(filtLongSpec[jj]);
    double powerRatio = interpPower / origPower;
    double magRatio = sqrt(powerRatio);
    filtLongSpec[jj].re *= magRatio;
    filtLongSpec[jj].im *= magRatio;
    
    // phase
    
    double interpDiffPhase = diffPhaseStart + interpFraction * deltaDiffPhase;
    double phaseShort = RadarComplex::argRad(filtShortSpec[jj]);
    double phaseLong = RadarComplex::sumRad(phaseShort, interpDiffPhase);
    double interpMag = RadarComplex::mag(filtLongSpec[jj]);
    filtLongSpec[jj].re = interpMag * cos(phaseLong);
    filtLongSpec[jj].im = interpMag * sin(phaseLong);
    
  }

  // invert from FFT space back into time series

  fftHalf.inv(filtShortSpec, filtShort);
  fftHalf.inv(filtLongSpec, filtLong);

  // recombine short and long PRT series into full series

  RadarMoments::combineStagIq(nSamples, filtShort, filtLong, iqFiltered);

}

/////////////////////////////////////////////////////////////////
// apply polynomial regression clutter filter to IQ time series
//
// NOTE: IQ data should not be windowed.
//
// Inputs:
//   nSamples
//   nExpanded = (nSamples / 2) * (m + n)
//   fftExp: fft object for expanded time series, length nExpanded
//   regr: object to be used for polynomial computations
//   iqOrig: unfiltered time series, not windowed
//   channel: which channel - used to determine calibrated noise
//
//  Outputs:
//    iqFiltered: filtered time series
//    filterRatio: ratio of raw to unfiltered power, before applying correction
//    spectralNoise: spectral noise estimated from the spectrum
//    spectralSnr: ratio of spectral noise to noise power
//    specRatio: if non-NULL, contains ratio of filtered to unfiltered spectrum
//
//  Memory allocation by calling routine:
//    regr - initialized to size nSamples
//    iqOrig[nSamples]
//    iqFiltered[nSamples]
//    specRatio[nSamples] - if non-NULL

void RadarMoments::applyRegrFilterStagPrt(int nSamples,
                                          int nExpanded,
                                          const RadarFft &fftExp,
                                          const RegressionFilter &regr,
                                          const RadarComplex_t *iqOrig,
                                          double calibratedNoise,
                                          bool interpAcrossNotch,
                                          RadarComplex_t *iqFiltered,
                                          double &filterRatio,
                                          double &spectralNoise,
                                          double &spectralSnr,
                                          double *specRatio /* = NULL*/)
  
{

  // apply the regression filter to the non-windowed (original) time series

  TaArray<RadarComplex_t> iqRegr_;
  RadarComplex_t *iqRegr = iqRegr_.alloc(nSamples);
  regr.apply(iqOrig, iqRegr);

  double powerOrig = RadarComplex::meanPower(iqOrig, nSamples);
  double powerRegr = RadarComplex::meanPower(iqRegr, nSamples);
  double powerClut = powerOrig - powerRegr;
  double csr = powerClut / powerRegr;

  // check for low CSR, is less than 10dB do not filter

  if (csr < .1) {
    memcpy(iqFiltered, iqOrig, nSamples * sizeof(RadarComplex_t));
    return;
  }
  
  // memcpy(iqFiltered, iqRegr, nSamples * sizeof(RadarComplex_t));
  // return;

  // if no notch interpolation is required, return now

  if (!interpAcrossNotch) {
    memcpy(iqFiltered, iqRegr, nSamples * sizeof(RadarComplex_t));
    return;
  }
  
  // expand the orig and filtered time series into pseudo-constant-prt series
  
  TaArray<RadarComplex_t> iqOrigExp_;
  RadarComplex_t *iqOrigExp = iqOrigExp_.alloc(nExpanded);
  expandStagIq(nSamples, nExpanded, _staggeredM, _staggeredN,
               iqOrig, iqOrigExp);
  
  TaArray<RadarComplex_t> iqRegrExp_;
  RadarComplex_t *iqRegrExp = iqRegrExp_.alloc(nExpanded);
  expandStagIq(nSamples, nExpanded, _staggeredM, _staggeredN,
               iqRegr, iqRegrExp);
  
  // take the forward fft to compute the complex spectra, orig and filtered
  
  TaArray<RadarComplex_t> origSpecC_;
  RadarComplex_t *origSpecC = origSpecC_.alloc(nExpanded);
  fftExp.fwd(iqOrigExp, origSpecC);
  
  TaArray<RadarComplex_t> regrSpecC_;
  RadarComplex_t *regrSpecC = regrSpecC_.alloc(nExpanded);
  fftExp.fwd(iqRegrExp, regrSpecC);

  // compute the real power spectra, original and filtered
  
  TaArray<double> origSpec_;
  double *origSpec = origSpec_.alloc(nExpanded);
  RadarComplex::loadPower(origSpecC, origSpec, nExpanded);

  TaArray<double> regrSpec_;
  double *regrSpec = regrSpec_.alloc(nExpanded);
  RadarComplex::loadPower(regrSpecC, regrSpec, nExpanded);
  
  // interpolate across the notches of the filtered spectrum
  
  _interpAcrossStagNotches(nSamples, nExpanded,
                           _staggeredM, _staggeredN, regrSpec);
  
  // adjust the spectrum by the filter ratio
  // constrain ratios to be 1 or less

  for (int ii = 0; ii < nExpanded; ii++) {
    double magRatio = sqrt(regrSpec[ii] / origSpec[ii]);
    if (magRatio > 1.0) {
      magRatio = 1.0;
    }
    regrSpecC[ii].re = origSpecC[ii].re * magRatio;
    regrSpecC[ii].im = origSpecC[ii].im * magRatio;
    if (specRatio != NULL) {
      specRatio[ii] = magRatio;
    }
  }

  // invert the fft
  
  fftExp.inv(regrSpecC, iqRegrExp);
  
  // condense the expanded time series

  condenseStagIq(nSamples, nExpanded, _staggeredM, _staggeredN,
                 iqRegrExp, iqFiltered);

}

//////////////////////////////////////////////////////////////////////
// adjust regression-filtered time series for notch and residual power
//
// Not used in current code

void RadarMoments::_adjustRegressionFilter
  (int nSamples,
   const RadarFft &fft,
   const double *window, // window to use
   const RadarComplex_t *iqOrig, // non-windowed
   const RadarComplex_t *iqRegr, // regr-filtered
   double calibratedNoise,
   bool interpAcrossNotch,
   RadarComplex_t *iqFiltered,
   double &filterRatio,
   double &spectralNoise,
   double &spectralSnr,
   double *specRatio)
  
{

  // apply the window to the regression-filtered times series

  TaArray<RadarComplex_t> regrWindowed_;
  RadarComplex_t *regrWindowed = regrWindowed_.alloc(nSamples);
  applyWindow(iqRegr, window, regrWindowed, nSamples);
  
  // take the forward fft to compute the complex spectrum of filtered series
  
  TaArray<RadarComplex_t> regrSpecC_;
  RadarComplex_t *regrSpecC = regrSpecC_.alloc(nSamples);
  fft.fwd(regrWindowed, regrSpecC);
  
  // compute the real filtered spectrum
  
  TaArray<double> regrSpec_;
  double *regrSpec = regrSpec_.alloc(nSamples);
  RadarComplex::loadPower(regrSpecC, regrSpec, nSamples);

  // interpolate across the notch

  if (interpAcrossNotch) {
    _interpAcrossNotch(nSamples, regrSpec);
  }

  // compute powers and filter ratio
  
  double rawPower = RadarComplex::meanPower(iqOrig, nSamples);
  double filteredPower = RadarComplex::meanPower(iqRegr, nSamples);
  double powerRemoved = rawPower - filteredPower;
  filterRatio = rawPower / filteredPower;

  // compute spectral noise value
  
  double spectalNoiseSdev;
  ClutFilter::computeSpectralNoise(regrSpec, nSamples,
                                   spectralNoise, spectalNoiseSdev);
  
  // compute SNR based on the spectral noise

  spectralSnr = spectralNoise / calibratedNoise;

  if (powerRemoved > 0) {
    double correctionRatio =
      _computePwrCorrectionRatio(nSamples, spectralSnr,
 				 rawPower, filteredPower,
                                 powerRemoved, calibratedNoise);
    // correct the filtered powers for clutter residue
    for (int ii = 0; ii < nSamples; ii++) {
      regrSpec[ii] *= correctionRatio;
    }
  }
  
  // window the input iq data
  
  TaArray<RadarComplex_t> iqWindowed_;
  RadarComplex_t *iqWindowed = iqWindowed_.alloc(nSamples);
  applyWindow(iqOrig, window, iqWindowed, nSamples);

  // take the forward fft to compute the raw complex power spectrum
  
  TaArray<RadarComplex_t> powerSpecC_;
  RadarComplex_t *powerSpecC = powerSpecC_.alloc(nSamples);
  fft.fwd(iqWindowed, powerSpecC);
  
  // load the raw power spectrum
  
  TaArray<double> powerSpec_;
  double *powerSpec = powerSpec_.alloc(nSamples);
  RadarComplex::loadPower(powerSpecC, powerSpec, nSamples);
  
  // adjust the input spectrum by the filter ratio
  // constrain ratios to be 1 or less

  for (int ii = 0; ii < nSamples; ii++) {
    double magRatio = sqrt(regrSpec[ii] / powerSpec[ii]);
    if (magRatio > 1.0) {
      magRatio = 1.0;
    }
    powerSpecC[ii].re *= magRatio;
    powerSpecC[ii].im *= magRatio;
    if (specRatio != NULL) {
      specRatio[ii] = magRatio;
    }
  }

  // invert the fft
  
  fft.inv(powerSpecC, regrWindowed);

  // invert the window

  invertWindow(regrWindowed, window, iqFiltered, nSamples);
 
}

//////////////////////////////////////////////////////////////////////
// fill in notch in regression-filtered spectrum

void RadarMoments::_fillNotchRegrFilter
  (int nSamples,
   const RadarFft &fft,
   const RadarComplex_t *iqOrig, // original data
   const RadarComplex_t *iqRegr, // regr-filtered
   RadarComplex_t *iqFiltered,
   double *specRatio)
  
{
  
  // compute the complex spectra
  
  TaArray<RadarComplex_t> origSpecC_;
  RadarComplex_t *origSpecC = origSpecC_.alloc(nSamples);
  fft.fwd(iqOrig, origSpecC);
  
  TaArray<RadarComplex_t> regrSpecC_;
  RadarComplex_t *regrSpecC = regrSpecC_.alloc(nSamples);
  fft.fwd(iqRegr, regrSpecC);
  
  // compute the real spectra
  
  TaArray<double> origSpec_;
  double *origSpec = origSpec_.alloc(nSamples);
  RadarComplex::loadPower(origSpecC, origSpec, nSamples);
  
  TaArray<double> regrSpec_;
  double *regrSpec = regrSpec_.alloc(nSamples);
  RadarComplex::loadPower(regrSpecC, regrSpec, nSamples);
  
  // interpolate across the notch in the filtered spectrum
  
  _interpAcrossNotch(nSamples, regrSpec);

  // adjust the input spectrum by the filter ratio
  // this preserves the phase information
  // constrain ratios to be 1 or less

  for (int ii = 0; ii < nSamples; ii++) {
    double magRatio = sqrt(regrSpec[ii] / origSpec[ii]);
    if (magRatio > 1.0) {
      magRatio = 1.0;
    }
    regrSpecC[ii].re = origSpecC[ii].re * magRatio;
    regrSpecC[ii].im = origSpecC[ii].im * magRatio;
    if (specRatio != NULL) {
      specRatio[ii] = magRatio;
    }
  }

  // invert to get the filtered time series
  
  fft.inv(regrSpecC, iqFiltered);

}

//////////////////////////////////////////////////////////////////////
// Compute the spectral snr
//
// Side effects: the following are computed:
//  specWindowed (from forward fft)
//  spectralNoise
//  spectralSnr

void RadarMoments::computeSpectralSnr
  (int nSamples,
   const RadarFft &fft,
   const RadarComplex_t *iqWindowed,   // windowed time series
   RadarComplex_t *specWindowed, // windowed spectrum
   double calibratedNoise,
   double &spectralNoise,
   double &spectralSnr)
  
{
  
  // compute complex spectrum using forward fft
  
  fft.fwd(iqWindowed, specWindowed);

  // compute real power spectrum
  
  TaArray<double> realSpec_;
  double *realSpec = realSpec_.alloc(nSamples);
  RadarComplex::loadPower(specWindowed, realSpec, nSamples);
    
  // compute spectral noise

  spectralNoise = ClutFilter::computeSpectralNoise(realSpec, nSamples);
  
  // compute snr
  
  spectralSnr = (spectralNoise - calibratedNoise) / calibratedNoise;

}
  
////////////////////////////////////////////////////////
// Separate a staggered time series into two
// interleaved sequences, short PRT and long PRT.
//
// The following is assumed:
//
//   1. nSamples refers to the combined length.
//   2. nSamples is even.
//   3. The sequence starts with short PRT.
//   4. Memory has been allocated as follows:
//        iq[nSamples]
//        iqShort[nSamples/2]
//        iqLong[nSamples/2]

void RadarMoments::separateStagIq(int nSamples,
                                  const RadarComplex_t *iq,
                                  RadarComplex_t *iqShort,
                                  RadarComplex_t *iqLong)
  
{

  for (int ii = 0, jj = 0; ii < nSamples / 2; ii++) {
    iqShort[ii] = iq[jj];
    jj++;
    iqLong[ii] = iq[jj];
    jj++;
  }

}

////////////////////////////////////////////////////////
// Combine short and long time series sequences into
// a single sequences, starting with short PRT.
//
// The following is assumed:
//
//   1. nSamples refers to the combined length.
//   2. nSamples is even.
//   3. The sequence starts with short PRT.
//   4. Memory has been allocated as follows:
//        iq[nSamples]
//        iqShort[nSamples/2]
//        iqLong[nSamples/2]

void RadarMoments::combineStagIq(int nSamples,
                                 const RadarComplex_t *iqShort,
                                 const RadarComplex_t *iqLong,
                                 RadarComplex_t *iq)
  
{
  
  for (int ii = 0, jj = 0; ii < nSamples / 2; ii++) {
    iq[jj] = iqShort[ii];
    jj++;
    iq[jj] = iqLong[ii];
    jj++;
  }

}

////////////////////////////////////////////////////////
// Expand a staggered time series into a series
// with 0's inserted, to create a constant-prt
// time series
//
// The following is assumed:
//
//   1. nSamples refers to the original length.
//   2. nExpanded is the expanded length.
//      nExpanded = (nSamples / 2) * (m + n)
//   3. The sequence starts with short PRT.
//   4. Memory has been allocated as follows:
//        iq[nSamples]
//        iqExpanded[nExpanded]

void RadarMoments::expandStagIq(int nSamples,
                                int nExpanded,
                                int staggeredM,
                                int staggeredN,
                                const RadarComplex_t *iq,
                                RadarComplex_t *iqExpanded)
  
{
  
  // load up IQ, with 0's inserted between the observed
  // pulses to make a pseudo-constant-prt series
  
  int kk = 0;
  memset(iqExpanded, 0, nExpanded * sizeof(RadarComplex_t));
  for (int ii = 0; ii < nSamples; ii++) {
    iqExpanded[kk] = iq[ii];
    if (ii % 2 == 0) {
      kk += staggeredM;
    } else {
      kk += staggeredN;
    }
  }

}
  
////////////////////////////////////////////////////////
// Condense and expanded staggered time series to
// match the times of the the original series
// by removing the extra pulses
//
// The following is assumed:
//
//   1. nSamples refers to the original length.
//   2. nExpanded is the expanded length.
//      nExpanded = (nSamples / 2) * (m + n)
//   3. The sequence starts with short PRT.
//   4. Memory has been allocated as follows:
//        iqExpanded[nExpanded]
//        iqCondensed[nSamples]

void RadarMoments::condenseStagIq(int nSamples,
                                  int nExpanded,
                                  int staggeredM,
                                  int staggeredN,
                                  const RadarComplex_t *iqExpanded,
                                  RadarComplex_t *iqCondensed)
  
{
  
  int kk = 0;
  for (int ii = 0; ii < nSamples; ii++) {
    iqCondensed[ii] = iqExpanded[kk];
    if (ii % 2 == 0) {
      kk += staggeredM;
    } else {
      kk += staggeredN;
    }
  }

}
  
/////////////////////////////////////////////////////
// Applies previously computed filter ratios, in the spectral
// domain, to a time series

void RadarMoments::applyFilterRatio(int nSamples,
                                    const RadarFft &fft,
                                    const RadarComplex_t *iq,
                                    const double *specRatio,
                                    RadarComplex_t *iqFiltered)
  
{
  
  // take the forward fft

  TaArray<RadarComplex_t> spec_;
  RadarComplex_t *spec = spec_.alloc(nSamples);
  fft.fwd(iq, spec);

  // adjust the spectrum by the filter ratio

  for (int ii = 0; ii < nSamples; ii++) {
    double ratio = specRatio[ii];
    spec[ii].re *= ratio;
    spec[ii].im *= ratio;
  }

  // invert the fft

  fft.inv(spec, iqFiltered);
 
}

/////////////////////////////////////////////////////
// apply clutter filter for SZ 864

void RadarMoments::applyClutterFilterSz(int nSamples,
                                        const RadarFft &fft,
                                        GateData &gateData,
                                        double calibratedNoise,
					double &filterRatio,
                                        double &spectralNoise,
					double &spectralSnr)
  
{

  // determine the presence of clutter in strong and weak trip
  
  gateData.clutterInStrong = false;
  gateData.clutterInWeak = false;
  
  if (gateData.trip1IsStrong) {
    if (gateData.fields.cmd_flag && !gateData.censorStrong) {
      gateData.clutterInStrong = true;
    }
    if (gateData.secondTrip.cmd_flag && !gateData.censorWeak) {
      gateData.clutterInWeak = true;
    }
  } else {
    // trip 1 is weak
    if (gateData.fields.cmd_flag && !gateData.censorWeak) {
      gateData.clutterInWeak = true;
    }
    if (gateData.secondTrip.cmd_flag && !gateData.censorStrong) {
      gateData.clutterInStrong = true;
    }
  }

  if (!gateData.clutterInStrong && !gateData.clutterInWeak) {
    // no filtering necessary
    return;
  }

  if (gateData.clutterInStrong) {
    
    applyAdaptiveFilter(nSamples, fft,
                        gateData.iqStrong, NULL,
                        calibratedNoise,
                        gateData.iqStrongF,
                        filterRatio,
                        spectralNoise,
                        spectralSnr);
    
  }
    
  if (gateData.clutterInWeak) {
    
    applyAdaptiveFilter(nSamples, fft,
                        gateData.iqWeak, NULL,
                        calibratedNoise,
                        gateData.iqWeakF,
                        filterRatio,
                        spectralNoise,
                        spectralSnr);
    
  }
  
}
  
///////////////////////////////////////
// initialize based on params passed in

void RadarMoments::init(double prt,
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
// initialize based on IwrfTsInfo

void RadarMoments::init(double prt,
                        const IwrfTsInfo &opsInfo)
  
{
  
  init(prt,
       opsInfo.get_radar_wavelength_cm() / 100.0,
       opsInfo.get_proc_start_range_m() / 1000.0,
       opsInfo.get_proc_gate_spacing_m() / 1000.0);

}

///////////////////////////////////////
// initialize staggered PRT mode

void RadarMoments::initStagPrt(double prtShort,
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

void RadarMoments::initStagPrt(double prtShort,
                               double prtLong,
                               int staggeredM,
                               int staggeredN,
                               int nGatesPrtShort,
                               int nGatesPrtLong,
                               const IwrfTsInfo &opsInfo)
  
{
  
  initStagPrt(prtShort,
              prtLong,
              staggeredM,
              staggeredN,
              nGatesPrtShort,
              nGatesPrtLong,
              opsInfo.get_radar_wavelength_cm() / 100.0,
              opsInfo.get_proc_start_range_m() / 1000.0,
              opsInfo.get_proc_gate_spacing_m() / 1000.0);

}

////////////////////////////////////////////////////
// compute the size of the expanded time series for
// staggered prt
//
// Note: assumes that nSamples is even.

int RadarMoments::computeNExpandedStagPrt(int nSamples,
                                          int staggeredM,
                                          int staggeredN)

{
  return ((nSamples / 2) * (staggeredM + staggeredN));
}

int RadarMoments::computeNExpandedStagPrt(int nSamples)

{
  return ((nSamples / 2) * (_staggeredM + _staggeredN));
}

///////////////////////////////////////
// set calibration

void RadarMoments::setCalib(const DsRadarCalib &calib)
  
{
  
  // set noise values
  
  _calNoisePowerHc = pow(10.0, calib.getNoiseDbmHc() / 10.0);
  _calNoisePowerHx = pow(10.0, calib.getNoiseDbmHx() / 10.0);
  _calNoisePowerVc = pow(10.0, calib.getNoiseDbmVc() / 10.0);
  _calNoisePowerVx = pow(10.0, calib.getNoiseDbmVx() / 10.0);

  _estNoisePowerHc = _calNoisePowerHc;
  _estNoisePowerVc = _calNoisePowerVc;
  _estNoisePowerHx = _calNoisePowerHx;
  _estNoisePowerVx = _calNoisePowerVx;

  _baseDbz1kmHc = calib.getBaseDbz1kmHc();
  _baseDbz1kmHx = calib.getBaseDbz1kmHx();
  _baseDbz1kmVc = calib.getBaseDbz1kmVc();
  _baseDbz1kmVx = calib.getBaseDbz1kmVx();

  _receiverGainDbHc = calib.getReceiverGainDbHc();
  _receiverGainDbHx = calib.getReceiverGainDbHx();
  _receiverGainDbVc = calib.getReceiverGainDbVc();
  _receiverGainDbVx = calib.getReceiverGainDbVx();

  _dbzCorrection= calib.getDbzCorrection();
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

void RadarMoments::setCalib(const IwrfCalib &calib)
  
{
  
  // set noise values
  
  _calNoisePowerHc = pow(10.0, calib.getNoiseDbmHc() / 10.0);
  _calNoisePowerHx = pow(10.0, calib.getNoiseDbmHx() / 10.0);
  _calNoisePowerVc = pow(10.0, calib.getNoiseDbmVc() / 10.0);
  _calNoisePowerVx = pow(10.0, calib.getNoiseDbmVx() / 10.0);

  _estNoisePowerHc = _calNoisePowerHc;
  _estNoisePowerVc = _calNoisePowerVc;
  _estNoisePowerHx = _calNoisePowerHx;
  _estNoisePowerVx = _calNoisePowerVx;

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
// set the estimated noise power based on analysis of the data
// note that noise is power at the DRX,
// before subtraction of receiver gain

void RadarMoments::setEstimatedNoiseDbmHc(double val)
{
  _estNoisePowerHc = pow(10.0, val / 10.0);
}

void RadarMoments::setEstimatedNoiseDbmVc(double val)
{
  _estNoisePowerVc = pow(10.0, val / 10.0);
}

void RadarMoments::setEstimatedNoiseDbmHx(double val)
{
  _estNoisePowerHx = pow(10.0, val / 10.0);
}

void RadarMoments::setEstimatedNoiseDbmVx(double val)
{
  _estNoisePowerVx = pow(10.0, val / 10.0);
}

///////////////////////////////////////////
// negate the velocity - change the sign
  
void RadarMoments::setChangeVelocitySign(bool state)
{
  if (state) {
    _velSign = -1.0;
  } else {
    _velSign = 1.0;
  }
}

///////////////////////////////////////////
// negate the phidp - change the sign
  
void RadarMoments::setChangePhidpSign(bool state)
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
  
void RadarMoments::setChangeVelocitySignStaggered(bool state)
{
  if (state) {
    _velSignStaggered = -1.0;
  } else {
    _velSignStaggered = 1.0;
  }
}

///////////////////////////////////////
// set the window R values, used for width correction

void RadarMoments::setWindowRValues(double windowR1,
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

double RadarMoments::computeMagSdev(const RadarComplex_t *iq,
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

double RadarMoments::computePowerRatio(const RadarComplex_t *iq,
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

double RadarMoments::computePowerRatio(const RadarComplex_t *iqh,
                                       const RadarComplex_t *iqv,
                                       int nSamples)
  
{
  
  double prh = computePowerRatio(iqh, nSamples);
  double prv = computePowerRatio(iqv, nSamples);
  return ((prh + prv) / 2.0);

}
  
////////////////////////////////////////////////////
// compute mean pulse-to-pulse magnitude variation
// in dB space

double RadarMoments::computeMvar(const RadarComplex_t *iq,
                                 int nSamples,
                                 double prt)
  
{
  
  // compute the power of each pulse, in dBm

  TaArray<double> dbm_;
  double *dbm = dbm_.alloc(nSamples);
  for (int i = 0; i < nSamples; i++) {
    double ii = iq[i].re;
    double qq = iq[i].im;
    double power = ii * ii + qq * qq;
    dbm[i] = 10.0 * log10(power);
  }

  // sum up absolute pulse to pulse variations

  double sumDiff = 0.0;
  for (int i = 0; i < nSamples - 1; i++) {
    double diff = fabs(dbm[i+1] - dbm[i]) / 2.0; // magnitude in dB
    sumDiff += diff;
  }

  // normalize with respect to prt of 0.001

  sumDiff /= sqrt(prt / 0.001);
  double mvar = sumDiff / (nSamples - 1);

  return mvar;

}

// mvar for H/V simultaneous
  
double RadarMoments::computeMvarSim(const RadarComplex_t *iqh,
                                    const RadarComplex_t *iqv,
                                    int nSamples,
                                    double prt)
  
{

  double mvarH = computeMvar(iqh, nSamples, prt);
  double mvarV = computeMvar(iqv, nSamples, prt);
  return (mvarH + mvarV) / 2.0;

}

// mvar for H/V alternating
  
double RadarMoments::computeMvarAlt(const RadarComplex_t *iqh,
                                    const RadarComplex_t *iqv,
                                    int nSamplesHalf,
                                    double prt)
  
{

  // compute the power of each pulse, in dBm

  TaArray<double> dbm_;
  double *dbm = dbm_.alloc(nSamplesHalf);
  int count = 0;
  for (int i = 0; i < nSamplesHalf; i++, iqh++, iqv++) {
    // power for h pulse
    double ii = iqh->re;
    double qq = iqh->im;
    double power = ii * ii + qq * qq;
    dbm[count] = 10.0 * log10(power);
    count++;
    // power for v pulse
    ii = iqv->re;
    qq = iqv->im;
    power = ii * ii + qq * qq;
    dbm[count] = 10.0 * log10(power);
    count++;
  }
  
  // sum up absolute pulse to pulse variations
  
  int nSamples = nSamplesHalf * 2;
  double sumDiff = 0.0;
  for (int i = 0; i < nSamples - 1; i++) {
    double diff = fabs(dbm[i+1] - dbm[i]) / 2.0; // magnitude in dB
    sumDiff += diff;
  }
  
  // normalize with respect to prt of 0.001

  sumDiff /= sqrt(prt / 0.001);

  double mvar = sumDiff / (nSamples - 1.0);
  return mvar;

}

//////////////////////////////////////////////////////
// compute time-series power trend between
// max-power and min-power halves of the series,
// in dB
  
double RadarMoments::computeTpt(const RadarComplex_t *iq,
                                int nSamples)
  
{
  
  if (nSamples < 8) {
    return _missing;
  }
  
  // divide spectrum into 8 parts,
  // compute sum of power in each part
  
  int nEighth = ((nSamples - 1) / 8) + 1;
  if (nEighth < 1) {
    nEighth = 1;
  }
  double partSumPower[8];
  double partCount[8];
  for (int ii = 0; ii < 8; ii++) {
    int jjStart = ((ii * nSamples) / 8);
    partSumPower[ii] = 0.0;
    partCount[ii] = 0.0;
    for (int jj = jjStart; jj < jjStart + nEighth; jj++) {
      int kk = (jj + nSamples) % nSamples;
      double ival = iq[kk].re;
      double qval = iq[kk].im;
      double power = ival * ival + qval * qval;
      partSumPower[ii] += power;
      partCount[ii]++;
    }
  }
  
  // compute mean power in each part
  
  double partMeanPower[8];
  for (int ii = 0; ii < 8; ii++) {
    partMeanPower[ii] = partSumPower[ii] / partCount[ii];
  }

  // compute the min and max power for 3 consecutive eighths
  // compute trend difference between the min and max
  
  double maxDb = -9999;
  double minDb = 9999;
  int nn = 3;
  for (int ii = 0; ii < 8 - nn; ii++) {
    double sum = 0.0;
    for (int jj = 0; jj < nn; jj++) {
      sum += partMeanPower[ii + jj];
    } // jj
    double meanDb = -9999;
    if (sum > 0) {
      meanDb = 10.0 * log10(sum / nn);
      if (meanDb > maxDb) {
        maxDb = meanDb;
      }
      if (meanDb < minDb) {
        minDb = meanDb;
      }
    }
  }
  
  if (maxDb < -9000 || minDb > 9000) {
    return _missing;
  } else {
    double trendDb = maxDb - minDb;
    return trendDb;
  }

}

////////////////////////////////////////////////
// compute number of times I and Q data cross 0
// Returns number of crossings

int RadarMoments::computeIqz(const RadarComplex_t *iq,
                             int nSamples)
  
{
  
  int iCrosses = 0;
  int qCrosses = 0;

  const RadarComplex_t *iq0 = iq;
  const RadarComplex_t *iq1 = iq + 1;
  for (int ii = 1; ii < nSamples; ii++, iq0++, iq1++) {
    if (iq0->re * iq1->re <= 0) {
      iCrosses++;
    }
  }
  
  iq0 = iq;
  iq1 = iq + 1;
  for (int ii = 1; ii < nSamples; ii++, iq0++, iq1++) {
    if (iq0->im * iq1->im <= 0) {
      qCrosses++;
    }
  }
  
  return iCrosses + qCrosses;

}


////////////////////////////////////////////////
// compute the Cumulative Phase Difference
// This is the maximum absolute change in cumulative
// phase for the time series

double RadarMoments::computeCpd(const RadarComplex_t *iq,
                                int nSamples)
  
{
  
  double cumulativeDiff = 0.0;
  double minDiff = 1.0e99;
  double maxDiff = -1.0e99;

  const RadarComplex_t *iq0 = iq;
  const RadarComplex_t *iq1 = iq + 1;
  
  for (int ii = 1; ii < nSamples; ii++, iq0++, iq1++) {
    RadarComplex_t complexDiff = RadarComplex::conjugateProduct(*iq0, *iq1);
    double phaseDiff = RadarComplex::argDeg(complexDiff);
    cumulativeDiff += phaseDiff;
    if (cumulativeDiff < minDiff) {
      minDiff = cumulativeDiff;
    }
    if (cumulativeDiff > maxDiff) {
      maxDiff = cumulativeDiff;
    }
  }
  
  return (maxDiff - minDiff);

}

////////////////////////////////////////////////
// compute the off-zero SNR, in dB.
// i.e. SNR away from 0 DC.
// This is computed using a relatively wide notch, to make sure
// no clutter power is included in the result.
// NotchWidth is specified in m/s.

double RadarMoments::computeOzSnr(const RadarComplex_t *iq,
                                  const double *window,
                                  int nSamples,
                                  const RadarFft &fft,
                                  double notchWidthMps,
                                  double noisePower)
  
{
  
  // take the forward fft to compute the raw complex power spectrum
  
  TaArray<RadarComplex_t> spec_;
  RadarComplex_t *spec = spec_.alloc(nSamples);
  if (window == NULL) {
    fft.fwd(iq, spec);
  } else {
    TaArray<RadarComplex_t> iqw_;
    RadarComplex_t *iqw = iqw_.alloc(nSamples);
    applyWindow(iq, window, iqw, nSamples);
    fft.fwd(iqw, spec);
  }

  // compute the notch width in spectrum counts
  
  int halfWidth =
    (int) (((notchWidthMps / 2.0) / _nyquist) * nSamples + 0.5);

  // compute the mean power in the outer parts of the 
  // spectrum, away from DC

  double sumPower = 0.0;
  double count = 0.0;
  for (int ii = halfWidth; ii < nSamples - halfWidth; ii++) {
    double re = spec[ii].re;
    double im = spec[ii].im;
    double power = re * re + im * im;
    sumPower += power;
    count++;
  }

  if (count < 1) {
    return _minDetectableSnr;
  }
  
  double meanPower = sumPower / count;
    
  // compute noise-subtracted power
  
  double power_ns = meanPower - noisePower;
  if (power_ns < 0) {
    return _minDetectableSnr;
  }

  // compute SNR
  
  double snr = power_ns / noisePower;
  double snrDb = 10.0 * log10(snr);
  return snrDb;

}

///////////////////////////////////////////////////////////
// Compute ncp for a given time series
  
double RadarMoments::computeNcp(RadarComplex_t *iq)
  
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

void RadarMoments::initWindowRect(int nSamples, double *window)
  
{
  
  for (int ii = 0; ii < nSamples; ii++) {
    window[ii] = 1.0;
  }

}
  
/////////////////////////////////////
// initialize vonHann window

void RadarMoments::initWindowVonhann(int nSamples, double *window)

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

void RadarMoments::initWindowBlackman(int nSamples, double *window)

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

void RadarMoments::initWindowBlackmanNuttall(int nSamples, double *window)

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
// create rectangular window
// Allocates memory and returns window

double *RadarMoments::createWindowRect(int nSamples)
  
{
  
  double *window = new double[nSamples];
  initWindowRect(nSamples, window);
  return window;

}
  
/////////////////////////////////////
// create vonHann window
// Allocates memory and returns window

double *RadarMoments::createWindowVonhann(int nSamples)

{

  double *window = new double[nSamples];
  initWindowVonhann(nSamples, window);
  return window;

}
  
/////////////////////////////////////
// create Blackman window
// Allocates memory and returns window

double *RadarMoments::createWindowBlackman(int nSamples)

{
  
  double *window = new double[nSamples];
  initWindowBlackman(nSamples, window);
  return window;
  
}
  
/////////////////////////////////////
// create Blackman window
// Allocates memory and returns window

double *RadarMoments::createWindowBlackmanNuttall(int nSamples)

{
  
  double *window = new double[nSamples];
  initWindowBlackmanNuttall(nSamples, window);
  return window;
  
}
  
///////////////////////////////////////
// apply window to IQ samples, in place

void RadarMoments::applyWindow(RadarComplex_t *iq,
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

void RadarMoments::applyWindow(const RadarComplex_t *iqOrig,
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

void RadarMoments::invertWindow(const RadarComplex_t *iqWindowed,
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

double RadarMoments::computeWindowCorrelation(int lag,
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

void RadarMoments::_computeRangeCorrection(double startRangeKm,
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

void RadarMoments::loadAtmosAttenCorrection(int nGates,
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

double RadarMoments::computeCpa(const RadarComplex_t *iq,
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

double RadarMoments::computeCpa(const RadarComplex_t *iqh,
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

double RadarMoments::computeCpaAlt(const RadarComplex_t *iq,
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

double RadarMoments::computeCpaAlt(const RadarComplex_t *iqh,
                                   const RadarComplex_t *iqv,
                                   int nSamples)
  
{
  double cpa_h = computeCpaAlt(iqh, nSamples);
  double cpa_v = computeCpaAlt(iqv, nSamples);
  return (cpa_h + cpa_v) / 2.0;
}

/////////////////////////////////////////////////
// compute refractivity

void RadarMoments::computeRefract(const RadarComplex_t *iq,
                                  int nSamples,
                                  double &aiq,
                                  double &niq,
                                  bool changeAiqSign /* = false */)
  
{

  double factor = DEG_PER_RAD;
  if (changeAiqSign) {
    factor *= -1.0;
  }

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
  
  double avI = sumI / nSamples;
  double avQ = sumQ / nSamples;

  aiq = factor * atan2(avQ, avI);
  double phasorLen = sqrt(avI * avI + avQ * avQ);
  niq = 10.0 * log10(phasorLen);

}
    
/////////////////////////////////////////////////
// compute CPR - cross-polar ratio

void RadarMoments::computeCpr(const RadarComplex_t *iqhc,
                              const RadarComplex_t *iqvx,
                              int nSamples,
                              double &cprPowerDb,
                              double &cprPhaseDeg)
  
{
  
  // compute mean of VX / HC

  RadarComplex_t meanQuotient =
    RadarComplex::meanComplexQuotient(iqvx, iqhc, nSamples);

  cprPowerDb = 20.0 * log10(RadarComplex::mag(meanQuotient));
  cprPhaseDeg = RadarComplex::argDeg(meanQuotient);

}
    
/////////////////////////////////////////////////
// compute width using R0R1 method
// from Greg Meymaris

double RadarMoments::_computeR0R1Width(double r0,
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

double RadarMoments::_computeR1R2Width(double r1,
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

double RadarMoments::_computeR1R3Width(double r1,
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

double RadarMoments::_computePplsWidth(double r0,
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

double RadarMoments::_computeHybridWidth(double r0,
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

double RadarMoments::_computeStagWidth(double rA,
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

  // 22222222222222222222 _constrain(width, 0, nyquist);
  return width;

}

/////////////////////////////////////////////////////////////////
// Compute correction ratio to be applied to filtered time series
// to account for noise added to the spectrum by the clutter peak
//
// The computations are carried out in dB space, because the
// corrections are normally discussed in this manner.

double RadarMoments::_computePwrCorrectionRatio(int nSamples,
						double spectralSnr,
						double rawPower,
						double filteredPower,
						double powerRemoved,
                                                double calibratedNoise)
  
{

  // check if we need to apply residue correction at all

  if (!_applySpectralResidueCorrection) {
    return 1.0;
  }

  // check the SNR

  double snr = (rawPower - calibratedNoise) / calibratedNoise;
  if (snr <= 0) {
    return 1.0;
  }
  double snrDb = 10.0 * log10(snr);
  if (snrDb < _minSnrDbForResidueCorrection) {
    return 1.0;
  }

  // Optionally, apply the legacy NEXRAD clutter residue correction
    
  if (_applyDbForDbCorrection) {
    
    double dbForDbRatio = _dbForFbRatio;
    double dbForDbThreshold = _dbForDbThreshold;
    
    double powerTotalDb = 10.0 * log10(rawPower);
    double powerLeft = rawPower - powerRemoved;
    if (powerLeft < 1.0e-12) {
      powerLeft = 1.0e-12;
    }
    double powerLeftDb = 10.0 * log10(powerLeft);
    double diffDb = powerTotalDb - powerLeftDb;
    double powerRemovedDbCorrection = diffDb * dbForDbRatio;
    if (diffDb > dbForDbThreshold) {
      powerRemovedDbCorrection += (diffDb - dbForDbThreshold);
    }
    double correctionRatio = 1.0 / pow(10.0, powerRemovedDbCorrection / 10.0);
    return correctionRatio;
    
  }
  
  // The correction ratio will be some fraction of the clutter residue ratio
  // depending on the power removed by the filter
  // For clutter ratios of less than 10 dB, no extra power is removed.
  // For clutter ratios of above 20 dB, all of the residue is removed.
  // Between these 2 limits, we interpolate linearly in dB space

  double clut2WxRatio = (rawPower - filteredPower) / filteredPower;
  double clut2WxRatioDb = 10.0 * log10(clut2WxRatio);
  double residueCorrectionRatioDb = 10.0 * log10(1.0 / spectralSnr);

  // compute fraction of clut residue to apply
  // the correction starts of as 0 dB at the lower bound,
  // and increases to a maximum at the upper bound

  double lowerBound = 6.0;
  double upperBound = 12.0;

  double fractionApplied = 0.0;
  if (clut2WxRatioDb < lowerBound) {
    fractionApplied = 0.0;
  } else if (clut2WxRatioDb > upperBound) {
    fractionApplied = 1.0;
  } else {
    double dx = clut2WxRatioDb - lowerBound;
    fractionApplied = dx / (upperBound - lowerBound);
  }

  double correctionAppliedDb = residueCorrectionRatioDb  * fractionApplied;
  double correctionRatio = pow(10.0, correctionAppliedDb / 10.0);

  return correctionRatio;

}

  
///////////////////////////////////////////////////
// compute the smoothness in the time series power

double RadarMoments::computeTss(const RadarComplex_t *iqWindowed,
                                int nSamples,
                                const RadarFft &fft)
  
{

  // compute magnitude time series, and sum of power
  
  TaArray<double> pwrTs_;
  double *pwrTs = pwrTs_.alloc(nSamples);
  
  double sumPowerSq = 0.0;
  for (int ii = 0; ii < nSamples; ii++) {
    double II = iqWindowed[ii].re;
    double QQ = iqWindowed[ii].im;
    double pwr = II * II + QQ * QQ;
    sumPowerSq += pwr * pwr;
    pwrTs[ii] = pwr;
  }
  
  // compute sum of power in notch, by computing fourier coefficients in notch
  
  const vector<vector<double> > &cosArray = fft.getCosArray();
  const vector<vector<double> > &sinArray = fft.getSinArray();

  double sqrtN = sqrt((double) nSamples);
  double sumPowerSqNotch = 0.0;
  for (int kk = 0; kk < _tssNotchWidth; kk++) {
    double sumRe = 0.0;
    double sumIm = 0.0;
    for (int jj = 0; jj < nSamples; jj++) {
      double pwr = pwrTs[jj];
      sumRe += pwr * cosArray[kk][jj];
      sumIm += pwr * sinArray[kk][jj];
    }
    double re = sumRe / sqrtN;
    double im = sumIm / sqrtN;
    double pwrSq = re * re + im * im;
    if (kk == 0) {
      sumPowerSqNotch += pwrSq; // DC
    } else {
      sumPowerSqNotch += pwrSq * 2.0; // spectrum is symmetrical around DC
    }
  }

  // compute smoothness as power in notch over power not in notch
  
  double smoothness = -999;
  if (sumPowerSqNotch > 0) {
    smoothness =
      10.0 * log10(sumPowerSqNotch / (sumPowerSq - sumPowerSqNotch));
  }
  
  return smoothness;

}

/////////////////////////////////////////////////////
// get the calibrated noise power given the channel

double RadarMoments::getCalNoisePower(channel_t channel)
  
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
// get the estimated noise power given the channel

double RadarMoments::getEstNoisePower(channel_t channel)
  
{

  switch (channel) {
    case CHANNEL_HC:
      return _estNoisePowerHc;
      break;
    case CHANNEL_VC:
      return _estNoisePowerVc;
      break;
    case CHANNEL_HX:
      return _estNoisePowerHx;
      break;
    case CHANNEL_VX:
      return _estNoisePowerVx;
      break;
    default:
      return _estNoisePowerHc;
  }

}

//////////////////////////////////////////
// get the receiver gain given the channel

double RadarMoments::getReceiverGain(channel_t channel)
  
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

double RadarMoments::getBaseDbz1km(channel_t channel)
  
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

double RadarMoments::computePowerPercentile(int nSamples, double *powerSpec,
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
  
//////////////////////////////////////////////////////
// interpolate across the notch for regression filter
//
// Memory allocation:
//  regrSpec[nSamples]

void RadarMoments::_interpAcrossNotch(int nSamples, double *regrSpec)

{

  // compute number of points for interpolation

  int mm = int (nSamples / 32.0 + 0.5);
  if (mm < 1) {
    return;
  }
  
  int nn = mm * 2 + 1;

  double startVal = regrSpec[nSamples - 1 - mm];
  double endVal = regrSpec[mm];
  double deltaVal = (endVal - startVal) / (nn - 1.0);

  for (int kk = 0; kk < nn; kk++) {
    int jj = (kk + nSamples - 1 - mm) % nSamples;
    regrSpec[jj] = startVal + kk * deltaVal;
  }
  
}

////////////////////////////////////////////////////////
// interpolate across the notched for regression filter
// in staggered mode
//
// Memory allocation:
//  regrSpec[nStaggered]

void RadarMoments::_interpAcrossStagNotches(int nSamples,
                                            int nStaggered,
                                            int staggeredM,
                                            int staggeredN,
                                            double *regrSpec)
  
{

  // compute number of points for interpolation

  // int mm = int (nSamples / 64.0 + 0.5);
  int mm = int (nSamples / 32.0 + 0.5);
  if (mm < 1) {
    return;
  }
  int nn = mm * 2 + 1;

  // loop through the replica locations

  for (int ii = 0; ii < staggeredM + staggeredN; ii++) {
    
    int loc = ii * nSamples / 2;

    int istart = loc - mm;
    int iend = loc + mm;
    
    int jstart = (istart + nStaggered) % nStaggered;
    int jend = (iend + nStaggered) % nStaggered;

    double startVal = regrSpec[jstart];
    double endVal = regrSpec[jend];
    double deltaVal = (endVal - startVal) / (nn - 1.0);
    
    for (int kk = 0; kk < nn; kk++) {
      int jj = (istart + kk + nStaggered) % nStaggered;
      regrSpec[jj] = startVal + kk * deltaVal;
    }

  }
  
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

void RadarMoments::detrendTs(const RadarComplex_t *iq,
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

void RadarMoments::_compute3PtMedian(const RadarComplex_t *iq,
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

double RadarMoments::_conditionSnr(double snr)
  
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

double RadarMoments::_adjustDbzForPwrH(double dbz)
  
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

double RadarMoments::_adjustDbzForPwrV(double dbz)
  
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

double RadarMoments::_adjustZdrForPwr(double zdr)
  
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
// set field metadata
  
void RadarMoments::_setFieldMetaData(MomentsFields &fields)
  
{
  
  fields.prt = _prt;
  fields.num_pulses = _nSamples;
  fields.prt_short = _prtShort;
  fields.prt_long = _prtLong;

}

///////////////////////////////////////////////////////////
// prepare for noise detection - single polarization
// Assumes the data is in the hc channel
  
void RadarMoments::singlePolNoisePrep(double lag0_hc,
                                      RadarComplex_t lag1_hc,
                                      MomentsFields &fields)
  
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
// prepare for noise detection - DP_ALT_HV_CO_ONLY
// Transmit alternating, receive copolar only

void RadarMoments::dpAltHvCoOnlyNoisePrep(double lag0_hc,
                                          double lag0_vc,
                                          RadarComplex_t lag2_hc,
                                          RadarComplex_t lag2_vc,
                                          MomentsFields &fields)
  
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

void RadarMoments::dpAltHvCoCrossNoisePrep(double lag0_hc,
                                           double lag0_hx,
                                           double lag0_vc,
                                           double lag0_vx,
                                           RadarComplex_t lag2_hc,
                                           RadarComplex_t lag2_vc,
                                           MomentsFields &fields)
  
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

void RadarMoments::dpSimHvNoisePrep(double lag0_hc,
                                    double lag0_vc,
                                    RadarComplex_t lag1_hc,
                                    RadarComplex_t lag1_vc,
                                    MomentsFields &fields)
  
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

void RadarMoments::dpHOnlyNoisePrep(double lag0_hc,
                                    double lag0_vx,
                                    RadarComplex_t lag1_hc,
                                    MomentsFields &fields)
  
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

void RadarMoments::dpVOnlyNoisePrep(double lag0_vc,
                                    double lag0_hx,
                                    RadarComplex_t lag1_vc,
                                    MomentsFields &fields)
  
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

void RadarMoments::singlePolStagPrtNoisePrep(RadarComplex_t *iqhc,
                                             RadarComplex_t *iqhcShort,
                                             RadarComplex_t *iqhcLong,
                                             MomentsFields &fields)
  
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

void RadarMoments::dpSimHvStagPrtNoisePrep(RadarComplex_t *iqhc,
                                           RadarComplex_t *iqvc,
                                           RadarComplex_t *iqhcShort,
                                           RadarComplex_t *iqvcShort,
                                           RadarComplex_t *iqhcLong,
                                           RadarComplex_t *iqvcLong,
                                           MomentsFields &fields)
  
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

void RadarMoments::_allocRangeCorr()

{

  _rangeCorr = new double[_maxGates];
  memset(_rangeCorr, 0, _maxGates * sizeof(double));
  _rangeCorrInit = false;

}

///////////////////////////////////////////////////////////
// allocate atmos atten corr table

void RadarMoments::_allocAtmosAttenCorr()

{

  _atmosAttenCorr = new double[_maxGates];
  memset(_atmosAttenCorr, 0, _maxGates * sizeof(double));

}

///////////////////////////////////////////////////////////
// compute ripple from mitch switch
//
// The Mitch switch presents a different face to the transmit pulse
// every alternate H and V pulse. This can lead to a difference in
// power or phase for each alternate pulse, which in turn can lead
// to spikes in the spectrum at 0.25 of the main PRF (or 0.5 of the
// H PRF).

void RadarMoments::computeMitchSwitchRipple(RadarComplex_t *iqhc,
                                            RadarComplex_t *iqvc,
                                            RadarComplex_t *iqhx,
                                            RadarComplex_t *iqvx,
                                            RadarComplex_t &rippleHc,
                                            RadarComplex_t &rippleVc,
                                            RadarComplex_t &rippleHx,
                                            RadarComplex_t &rippleVx,
                                            double &snrHc,
                                            double &snrVc,
                                            double &snrHx,
                                            double &snrVx)

{
                                      
}

