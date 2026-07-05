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
// KdpFilt.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2008
//
///////////////////////////////////////////////////////////////

#include <iomanip>
#include <cerrno>
#include <cassert>
#include <cmath>
#include <cstring>
#include <random>
#include <rapmath/NasaPolyFit.hh>
#include <toolsa/os_config.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/file_io.h>
#include <toolsa/sincos.h>
#include <toolsa/DateTime.hh>
#include <radar/KdpFilt.hh>
#include <radar/FilterUtils.hh>
#include <radar/DpolFilter.hh>
#include <radar/KdpFiltParams.hh>
#include <radar/RadarComplex.hh>
#include <radar/RadarFft.hh>
using namespace std;

const double KdpFilt::firCoeff_125[FIR_LEN_125+1] = {
  2.5107443e-003,2.6960328e-003,2.8834818e-003,3.0729344e-003,
  3.2642298e-003,3.4572038e-003,3.6516884e-003,3.8475124e-003,
  4.0445018e-003,4.2424792e-003,4.4412651e-003,4.6406770e-003,
  4.8405305e-003,5.0406391e-003,5.2408146e-003,5.4408670e-003,
  5.6406054e-003,5.8398373e-003,6.0383699e-003,6.2360093e-003,
  6.4325618e-003,6.6278331e-003,6.8216293e-003,7.0137569e-003,
  7.2040230e-003,7.3922356e-003,7.5782040e-003,7.7617385e-003,
  7.9426516e-003,8.1207572e-003,8.2958715e-003,8.4678134e-003,
  8.6364038e-003,8.8014671e-003,8.9628303e-003,9.1203239e-003,
  9.2737821e-003,9.4230427e-003,9.5679474e-003,9.7083424e-003,
  9.8440780e-003,9.9750093e-003,1.0100996e-002,1.0221904e-002,
  1.0337601e-002,1.0447965e-002,1.0552875e-002,1.0652219e-002,
  1.0745888e-002,1.0833782e-002,1.0915804e-002,1.0991867e-002,
  1.1061886e-002,1.1125785e-002,1.1183495e-002,1.1234953e-002,
  1.1280103e-002,1.1318895e-002,1.1351287e-002,1.1377243e-002,
  1.1396735e-002,1.1409742e-002,1.1416248e-002,1.1416248e-002,
  1.1409742e-002,1.1396735e-002,1.1377243e-002,1.1351287e-002,
  1.1318895e-002,1.1280103e-002,1.1234953e-002,1.1183495e-002,
  1.1125785e-002,1.1061886e-002,1.0991867e-002,1.0915804e-002,
  1.0833782e-002,1.0745888e-002,1.0652219e-002,1.0552875e-002,
  1.0447965e-002,1.0337601e-002,1.0221904e-002,1.0100996e-002,
  9.9750093e-003,9.8440780e-003,9.7083424e-003,9.5679474e-003,
  9.4230427e-003,9.2737821e-003,9.1203239e-003,8.9628303e-003,
  8.8014671e-003,8.6364038e-003,8.4678134e-003,8.2958715e-003,
  8.1207572e-003,7.9426516e-003,7.7617385e-003,7.5782040e-003,
  7.3922356e-003,7.2040230e-003,7.0137569e-003,6.8216293e-003,
  6.6278331e-003,6.4325618e-003,6.2360093e-003,6.0383699e-003,
  5.8398373e-003,5.6406054e-003,5.4408670e-003,5.2408146e-003,
  5.0406391e-003,4.8405305e-003,4.6406770e-003,4.4412651e-003,
  4.2424792e-003,4.0445018e-003,3.8475124e-003,3.6516884e-003,
  3.4572038e-003,3.2642298e-003,3.0729344e-003,2.8834818e-003,
  2.6960328e-003,2.5107443e-003};

const double KdpFilt::firCoeff_60[FIR_LEN_60+1] = {
  0.005192387815,0.006000584633,0.006826878703,
  0.007668199579,0.008521340618,0.009382975237,
  0.01024967409,0.01111792308,0.0119841421,
  0.01284470437,0.01369595621,0.01453423726,
  0.01535590085,0.01615733448,0.01693498027,
  0.01768535525,0.01840507134,0.01909085485,
  0.01973956552,0.02034821473,0.02091398302,
  0.02143423665,0.02190654305,0.02232868523,
  0.0226986749,0.02301476423,0.02327545631,
  0.02347951399,0.02362596732,0.02371411929,
  0.02374355002,0.02371411929,0.02362596732,
  0.02347951399,0.02327545631,0.02301476423,
  0.0226986749,0.02232868523,0.02190654305,
  0.02143423665,0.02091398302,0.02034821473,
  0.01973956552,0.01909085485,0.01840507134,
  0.01768535525,0.01693498027,0.01615733448,
  0.01535590085,0.01453423726,0.01369595621,
  0.01284470437,0.0119841421,0.01111792308,
  0.01024967409,0.009382975237,0.008521340618,
  0.007668199579,0.006826878703,0.006000584633,
  0.005192387815 };

const double KdpFilt::firCoeff_40[FIR_LEN_40+1] = {
  0.007806525986, 0.009628559511, 0.01150585082,
  0.01342243276, 0.01536143961, 0.01730530352,
  0.0192359639, 0.02113508696, 0.02298429218,
  0.02476538263, 0.02646057561, 0.02805273044,
  0.02952556994, 0.03086389233, 0.03205377043,
  0.03308273518, 0.03393994069, 0.03461630839,
  0.03510464806, 0.0353997539, 0.03549847428,
  0.0353997539, 0.03510464806, 0.03461630839,
  0.03393994069, 0.03308273518, 0.03205377043,
  0.03086389233, 0.02952556994, 0.02805273044,
  0.02646057561, 0.02476538263, 0.02298429218,
  0.02113508696, 0.0192359639, 0.01730530352,
  0.01536143961, 0.01342243276, 0.01150585082,
  0.009628559511, 0.007806525986 };

const double KdpFilt::firCoeff_30[FIR_LEN_30+1] = {
  0.01040850049,0.0136551033,0.01701931136,0.0204494327,
  0.0238905658,0.02728575662,0.03057723021,0.03370766631,
  0.03662148602,0.03926611662,0.04159320123,0.04355972181,
  0.04512900539,0.04627158699,0.04696590613,0.04719881804,
  0.04696590613,0.04627158699,0.04512900539,0.04355972181,
  0.04159320123,0.03926611662,0.03662148602,0.03370766631,
  0.03057723021,0.02728575662,0.0238905658,0.0204494327,
  0.01701931136,0.0136551033,0.01040850049};

const double KdpFilt::firCoeff_20[FIR_LEN_20+1] = {
  0.016976991942, 0.023294989742, 0.030244475217,
  0.037550056394, 0.044888313214, 0.051908191403,
  0.058254532798, 0.063592862330, 0.067633391375,
  0.070152221980, 0.071007947209, 0.070152221980,
  0.067633391375, 0.063592862330, 0.058254532798,
  0.051908191403, 0.044888313214, 0.037550056394,
  0.030244475217, 0.023294989742, 0.016976991942
};

const double KdpFilt::firCoeff_10[FIR_LEN_10+1] = {
  0.03064579383,0.0603038422,0.09022859603,0.1159074511,
  0.1332367851,0.1393550634,0.1332367851,0.1159074511,
  0.09022859603,0.0603038422,0.03064579383 };

// Constructor

KdpFilt::KdpFilt()
  
{

  // FIR filter defaults to length 20

  setFIRFilterLen(FIR_LENGTH_20);

  _nFiltIterUnfolded = 2;
  _nFiltIterCond = 4;

  _nGates = 0;
  setNGatesStats(9);

  _limitMaxRange = false;
  _maxRangeKm = 0.0;

  _wavelengthCm = 10.0;

  _startRangeKm = 0.0;
  _gateSpacingKm = 0.0;

  _elevDeg = -9999;
  _azDeg = -9999;

  _checkSnr = false;
  _snrThreshold = -6.0;
  _snrAvailable = false;

  _checkRhohv = false;
  _rhohvThreshold = 0.7;
  _rhohvAvailable = false;

  _checkZdrSdev = false;
  _zdrSdevMax = 2.0;
  _zdrAvailable = false;

  _phidpJitterMax = 30.0;
  _phidpSdevMax = 20.0;

  _minValidAbsKdp = 0.05;

  _useIterativeFiltering = false;
  _phidpDiffThreshold = 4.0;

  // initialize attenuation correction for Sband

  _dbzAttenCoeff = 0.017;
  _dbzAttenExpon = 0.84;
  _zdrAttenCoeff = 0.003;
  _zdrAttenExpon = 1.05;
  _doComputeAttenCorr = false;
  _attenCoeffsSpecified = false;

  // initialize computation of KDP from Z and ZDR

  _kdpZExpon = 1.0;
  _kdpZdrExpon = -2.05;
  _kdpZZdrCoeff = 3.32e-5;
  _kdpMinForSelfConsistency = 0.25;
  _kdpZZdrMedianLen = 5;

  // debugging

  _debug = false;
  _verbose = false;
  _writeRayFile = false;

}

// Destructor

KdpFilt::~KdpFilt()
  
{

}

/////////////////////////////////////
// set FIR filter length

void KdpFilt::setFIRFilterLen(fir_filter_len_t len)

{
  
  switch (len) {
    case FIR_LENGTH_125:
      _firLength = FIR_LEN_125 + 1;
      _firCoeff = firCoeff_125;
      break;
    case FIR_LENGTH_60:
      _firLength = FIR_LEN_60 + 1;
      _firCoeff = firCoeff_60;
      break;
    case FIR_LENGTH_40:
      _firLength = FIR_LEN_40 + 1;
      _firCoeff = firCoeff_40;
      break;
    case FIR_LENGTH_30:
      _firLength = FIR_LEN_30 + 1;
      _firCoeff = firCoeff_30;
      break;
    case FIR_LENGTH_20:
      _firLength = FIR_LEN_20 + 1;
      _firCoeff = firCoeff_20;
      break;
    case FIR_LENGTH_10:
    default:
      _firLength = FIR_LEN_10 + 1;
      _firCoeff = firCoeff_10;
  }

  _firLenHalf = _firLength / 2;

}

//////////////////////////////////////////
// set to write ray data to specified dir

void KdpFilt::setWriteRayFile(bool state /* = true */,
                              string dir /* = "" */)
  
{
  _writeRayFile = state;
  _rayFileDir = dir;
}
  
//////////////////////////////////////////
// Set flag to indicate we should compute corrections.
// Uses default coefficients.

void KdpFilt::setComputeAttenCorr(bool val)

{
  _doComputeAttenCorr = true;
}
  
//////////////////////////////////////////
// Set attenuation coefficients

void KdpFilt::setAttenCoeffs(double dbzCoeff, double dbzExpon,
                             double zdrCoeff, double zdrExpon)

{

  _dbzAttenCoeff = dbzCoeff;
  _dbzAttenExpon = dbzExpon;
  _zdrAttenCoeff = zdrCoeff;
  _zdrAttenExpon = zdrExpon;
  _doComputeAttenCorr = true;
  _attenCoeffsSpecified = true;

}
  
////////////////////////////////////////////
// Set processing options from params object

void KdpFilt::setFromParams(const KdpFiltParams &params)
{

  _params = params;

  // initialize KDP object

  if (params.KDP_fir_filter_len == KdpFiltParams::KDP_FIR_LEN_125) {
    setFIRFilterLen(KdpFilt::FIR_LENGTH_125);
  } else if (params.KDP_fir_filter_len == KdpFiltParams::KDP_FIR_LEN_60) {
    setFIRFilterLen(KdpFilt::FIR_LENGTH_60);
  } else if (params.KDP_fir_filter_len == KdpFiltParams::KDP_FIR_LEN_40) {
    setFIRFilterLen(KdpFilt::FIR_LENGTH_40);
  } else if (params.KDP_fir_filter_len == KdpFiltParams::KDP_FIR_LEN_30) {
    setFIRFilterLen(KdpFilt::FIR_LENGTH_30);
  } else if (params.KDP_fir_filter_len == KdpFiltParams::KDP_FIR_LEN_20) {
    setFIRFilterLen(KdpFilt::FIR_LENGTH_20);
  } else {
    setFIRFilterLen(KdpFilt::FIR_LENGTH_10);
  }
  setNGatesStats(params.KDP_ngates_for_stats);
  setMinValidAbsKdp(params.KDP_min_valid_abs_kdp);
  setNFiltIterUnfolded(params.KDP_n_filt_iterations_unfolded);
  setNFiltIterCond(params.KDP_n_filt_iterations_hubbert_bringi);
  if (params.KDP_psob_method == KdpFiltParams::HUBBERT_BRINGI_METHOD) {
    setUseIterativeFiltering(true);
    setPhidpDiffThreshold(params.KDP_phidp_difference_threshold_hubbert_bringi);
  }
  setPhidpSdevMax(params.KDP_phidp_sdev_max);
  setPhidpJitterMax(params.KDP_phidp_jitter_max);
  setMinValidAbsKdp(params.KDP_min_valid_abs_kdp);
  checkSnr(params.KDP_check_snr);
  setSnrThreshold(params.KDP_snr_threshold);
  checkRhohv(params.KDP_check_rhohv);
  setRhohvThreshold(params.KDP_rhohv_threshold);
  if (params.KDP_check_zdr_sdev) {
    checkZdrSdev(true);
  }
  setZdrSdevMax(params.KDP_zdr_sdev_max);
  setKdpMinForSelfConsistency(params.KDP_minimum_for_self_consistency);
  setMedianFilterLenForKdpZZdr(params.KDP_median_filter_len_for_ZZDR);

  if (params.KDP_debug) {
    setDebug(true);
  }
  if (params.KDP_verbose) {
    setDebug(true);
    setVerbose(true);
  }
  if (params.KDP_write_ray_files) {
    setWriteRayFile(true, params.KDP_ray_files_dir);
  }

  if (params.KDP_specify_coefficients_for_attenuation_correction) {
    setAttenCoeffs(params.KDP_dbz_attenuation_coefficient,
                   params.KDP_dbz_attenuation_exponent,
                   params.KDP_zdr_attenuation_coefficient,
                   params.KDP_zdr_attenuation_exponent);
  }
  setComputeAttenCorr(true);
  setPhidpFeatureLengthKm(params.phidp_feature_length_km);

}

////////////////////////////////////////////////////////////////////////
// Initialize the object arrays for later use.
// Do this if you need access to the arrays, but have not yet called
// compute(), and do not plan to do so.
// For example, you may want to output missing fields that you have
// not computed, but the memory needs to be there.

void KdpFilt::initializeArrays(int nGates)

{
  _nGates = nGates;
  _initArrays(NULL, NULL, NULL, NULL, NULL, _nGates);
}

/////////////////////////////////////
// compute KDP

int KdpFilt::compute(time_t timeSecs,
                     double timeFractionSecs,
                     double elevDeg,
                     double azDeg,
                     double wavelengthCm,
                     int nGates,
                     double startRangeKm,
                     double gateSpacingKm,
                     const double *snr,
                     const double *dbz,
                     const double *zdr,
                     const double *rhohv,
                     const double *phidp,
                     double missingValue)
  
{

  // set time

  _timeSecs = timeSecs;
  _timeFractionSecs = timeFractionSecs;

  // set beam location

  _elevDeg = elevDeg;
  _azDeg = azDeg;

  // set wavelength

  _wavelengthCm = wavelengthCm;

  // set attenuation coefficients from wavelenth if
  // not previously specified by caller
  // Ref: Bringi and Chandrasekar, Table 7.1, p494.

  if (!_attenCoeffsSpecified) {
    if (_wavelengthCm < 4) {
      // x band
      _dbzAttenCoeff = 0.233;
      _dbzAttenExpon = 1.02;
      _zdrAttenCoeff = 0.033;
      _zdrAttenExpon = 1.15;
    } else if (_wavelengthCm < 7) {
      // C band
      _dbzAttenCoeff = 0.073;
      _dbzAttenExpon = 0.99;
      _zdrAttenCoeff = 0.013;
      _zdrAttenExpon = 1.23;
    } else {
      // S band
      _dbzAttenCoeff = 0.017;
      _dbzAttenExpon = 0.84;
      _zdrAttenCoeff = 0.003;
      _zdrAttenExpon = 1.05;
    }
  }

  if (_verbose) {
    if (_doComputeAttenCorr) {
      cerr << "DEBUG - KdpFilt::compute" << endl;
      cerr << "  Performing attenuation correction from KDP" << endl;
      cerr << "    dbzAttenCoeff: " << _dbzAttenCoeff << endl;
      cerr << "    dbzAttenExpon: " << _dbzAttenExpon << endl;
      cerr << "    zdrAttenCoeff: " << _zdrAttenCoeff << endl;
      cerr << "    zdrAttenExpon: " << _zdrAttenExpon << endl;
    }
  }

  // set params for computing KDP from Z and ZDR

  _kdpZExpon = 1.0;
  _kdpZdrExpon = -2.05;
  _kdpZZdrCoeff = 3.32e-5 * (10.0 / _wavelengthCm);

  // set range details

  _startRangeKm = startRangeKm;
  _gateSpacingKm = gateSpacingKm;

  // set number of gates

  _nGates = nGates;

  // initialize the data arrays
  
  _missingValue = missingValue;
  _initArrays(snr, dbz, zdr, rhohv, phidp, _getNGatesMaxValid());
  
  // check if fold is at 90 or 180
  
  _computeFoldingRange();

  // filter with low-pass FFT
  
  _fftFilter();

  // unfold phidp
  
  if (_unfoldPhidp()) {
    // no good data in whole ray, fill with missing, return early
    for (int igate = 0; igate < _nGates; igate++) {
      if (_snr[igate] < _snrThreshold) {
        _kdp[igate] = _missingValue;
        _kdpZZdr[igate] = _missingValue;
        _kdpSC[igate] = _missingValue;
        _psob[igate] = _missingValue;
      } else {
        _kdp[igate] = 0;
        _kdpZZdr[igate] = 0;
        _kdpSC[igate] = 0;
        _psob[igate] = 0;
      }
    }
    return 0;
  }

  // compute filtered phidp,
  // and kdp from the filtered data

  _computeKdp();

  // compute phidp filtered with regression filter
  
  // _computePhidpWtRegrFilt();
  _computePhidpRegrFilt2();

  // compute phase shift on backscatter as the difference between
  // measured and filtered phidp

  for (int igate = 0; igate < _nGates; igate++) {
    if (_validForKdp[igate]) {
      double psob = _phidpFilt[igate] - _phidpCondFilt[igate];
      if (psob > 0) {
        _psob[igate] = psob;
      }
    }
  }
  
  // write ray file if requested

  if (_writeRayFile) {
    _writeRayDataToFile();
  }
    
  // set KDP and PSOB to 0
  // for small values of KDP, and non-good gates

  // for (int ii = 0; ii < _nGates; ii++) {
  //   if (!_validForKdp[ii] || fabs(_kdp[ii]) < _minValidAbsKdp) {
  //     if (_snr[ii] < _snrThreshold) {
  //       _kdp[ii] = _missingValue;
  //     } else { 
  //       _kdp[ii] = 0.0;
  //     }
  //   }
  // }

  // load up conditional KDP from estimated kdp and kdpZZdr

  _loadKdpSC();

  // compute attenuation corrections

  if (_doComputeAttenCorr) {
    _computeAttenCorrection();
  }

  return 0;

}
  
/////////////////////////////////////
// compute PHIDP statistics
//
// Computes sdev, jitter at each gate
//
// Use getPhidpSdev(), getPhidpJitter() for access to results

int KdpFilt::computePhidpStats(int nGates,
                               double startRangeKm,
                               double gateSpacingKm,
                               const double *phidp,
                               double missingValue)
  
{
  
  // set range details

  _startRangeKm = startRangeKm;
  _gateSpacingKm = gateSpacingKm;

  // set number of gates

  _nGates = nGates;

  // initialize the data arrays
  
  _missingValue = missingValue;
  _initArrays(NULL, NULL, NULL, NULL, phidp, _getNGatesMaxValid());
  
  // check if fold is at 90 or 180
  
  _computeFoldingRange();
  
  // initialize the gate states - the state at each gate is
  // dependent on the phidp values and the spatial relatioship
  // between them

  _gateStatesInit();
  
  // compute mean and standard deviation of phidp
  // and mean angular jitter at each gate

  for (int ii = _nGatesStatsHalf; 
       ii < _nGates - _nGatesStatsHalf; ii++) {
    _computePhidpStats(ii);
    _phidpJitter[ii] = _gateStates[ii].phidpJitter;
    _phidpMean[ii] = _gateStates[ii].phidpMean;
    _phidpMeanValid[ii] = _gateStates[ii].phidpMean;
    _phidpSdev[ii] = _gateStates[ii].phidpSdev;
  }
  
  return 0;

}
  
/////////////////////////////////////
// get max number of valid gates

int KdpFilt::_getNGatesMaxValid()
  
{
  
  int nValid = _nGates;

  if (_limitMaxRange) {
    nValid =
      (int) ((_maxRangeKm - _startRangeKm) / _gateSpacingKm + 0.5);
    if (nValid > _nGates) {
      nValid = _nGates;
    }
  }

  return nValid;

}

/////////////////////////////////////
// initialize arrays

void KdpFilt::_initArrays(const double *snr,
                          const double *dbz,
                          const double *zdr,
                          const double *rhohv,
                          const double *phidp,
                          int nGatesMaxValid)
  
{
  
  _arrayExtra = _firLength + 1;
  if (_firLength < _nGatesStats) {
    _arrayExtra = _nGatesStats + 1;
  }
  _arrayLen = _nGates + 2 * _arrayExtra;

  // allocate the arrays needed
  // copy input arrays, leaving extra space at the beginning
  // for negative indices and at the end for filtering as required

  _snr_.resize(_nGates); _snr = _snr_.data();
  _dbz_.resize(_nGates); _dbz = _dbz_.data();
  _dbzMax_.resize(_nGates); _dbzMax = _dbzMax_.data();
  _dbzMedian_.resize(_nGates); _dbzMedian = _dbzMedian_.data();
  _zdr_.resize(_nGates); _zdr = _zdr_.data();
  _zdrSdev_.resize(_nGates); _zdrSdev = _zdrSdev_.data();
  _zdrMedian_.resize(_nGates); _zdrMedian = _zdrMedian_.data();
  _rhohv_.resize(_nGates); _rhohv = _rhohv_.data();
  _phidp_.resize(_nGates); _phidp = _phidp_.data();
  _phidp180_.resize(_nGates); _phidp180 = _phidp180_.data();
  _phidp180Filt_.resize(_nGates); _phidp180Filt = _phidp180Filt_.data();
  _phidpFftFilt_.resize(_nGates); _phidpFftFilt = _phidpFftFilt_.data();
  _phidpMean_.resize(_nGates); _phidpMean = _phidpMean_.data();
  _phidpMeanValid_.resize(_nGates); _phidpMeanValid = _phidpMeanValid_.data();
  _phidpSdev_.resize(_nGates); _phidpSdev = _phidpSdev_.data();
  _phidpJitter_.resize(_nGates); _phidpJitter = _phidpJitter_.data();
  _phidpMeanUnfold_.resize(_nGates); _phidpMeanUnfold = _phidpMeanUnfold_.data();
  _phidpUnfold_.resize(_nGates); _phidpUnfold = _phidpUnfold_.data();
  _phidpFilt_.resize(_nGates); _phidpFilt = _phidpFilt_.data();
  _phidpCond_.resize(_nGates); _phidpCond = _phidpCond_.data();
  _phidpCondFilt_.resize(_nGates); _phidpCondFilt = _phidpCondFilt_.data();
  _phidpAccumFilt_.resize(_nGates); _phidpAccumFilt = _phidpAccumFilt_.data();
  _validForKdp_.resize(_nGates); _validForKdp = _validForKdp_.data();
  _validForUnfold_.resize(_nGates); _validForUnfold = _validForUnfold_.data();
  _kdp_.resize(_nGates); _kdp = _kdp_.data();
  _kdpZZdr_.resize(_nGates); _kdpZZdr = _kdpZZdr_.data();
  _kdpSC_.resize(_nGates); _kdpSC = _kdpSC_.data();
  _psob_.resize(_nGates); _psob = _psob_.data();
  _dbzAttenCorr_.resize(_nGates); _dbzAttenCorr = _dbzAttenCorr_.data();
  _zdrAttenCorr_.resize(_nGates); _zdrAttenCorr = _zdrAttenCorr_.data();
  _dbzCorrected_.resize(_nGates); _dbzCorrected = _dbzCorrected_.data();
  _zdrCorrected_.resize(_nGates); _zdrCorrected = _zdrCorrected_.data();
  _gateStates_.resize(_nGates); _gateStates = _gateStates_.data();
  _regrFilt_.resize(_nGates); _regrFilt = _regrFilt_.data();
  _xxVals_.resize(_nGates); _xxVals = _xxVals_.data();
  
  // copy data to working arrays

  if (snr != NULL) {
    memcpy(_snr, snr, _nGates * sizeof(double));
    _snrAvailable = true;
  } else {
    for (int ii = 0; ii < _nGates; ii++) {
      _snr[ii] = _missingValue;
    }
    _snrAvailable = false;
  }
  
  if (dbz != NULL) {
    memcpy(_dbz, dbz, _nGates * sizeof(double));
  } else {
    for (int ii = 0; ii < _nGates; ii++) {
      _dbz[ii] = _missingValue;
    }
  }
  memcpy(_dbzMedian, _dbz, _nGates * sizeof(double));
  FilterUtils::applyMedianFilter(_dbzMedian, _nGates,
                                 _kdpZZdrMedianLen, _missingValue);

  memcpy(_dbzCorrected, _dbz, _nGates * sizeof(double));

  if (zdr != NULL) {
    memcpy(_zdr, zdr, _nGates * sizeof(double));
    _zdrAvailable = true;
  } else {
    for (int ii = 0; ii < _nGates; ii++) {
      _zdr[ii] = _missingValue;
    }
    _zdrAvailable = false;
  }
  memcpy(_zdrMedian, _zdr, _nGates * sizeof(double));
  FilterUtils::applyMedianFilter(_zdrMedian, _nGates,
                                 _kdpZZdrMedianLen, _missingValue);
  memcpy(_zdrCorrected, _zdr, _nGates * sizeof(double));

  if (rhohv != NULL) {
    memcpy(_rhohv, rhohv, _nGates * sizeof(double));
    _rhohvAvailable = true;
  } else {
    for (int ii = 0; ii < _nGates; ii++) {
      _rhohv[ii] = _missingValue;
    }
    _rhohvAvailable = false;
  }

  if (phidp != NULL) {
    memcpy(_phidp, phidp, _nGates * sizeof(double));
  } else {
    for (int ii = 0; ii < _nGates; ii++) {
      _phidp[ii] = _missingValue;
    }
  }

  // beyond max range, set input values to missing

  for (int igate = nGatesMaxValid; igate < _nGates; igate++) {
    _snr[igate] = _missingValue;
    _dbz[igate] = _missingValue;
    _zdr[igate] = _missingValue;
    _phidp[igate] = _missingValue;
    _rhohv[igate] = _missingValue;
  }

  // compute max dbz for surrounding gates
  
  _computeDbzMax();
  
  // initialize computed arrays

  double xxDelta = 1.0 / (double) _nGates;
  for (int ii = 0; ii < _nGates; ii++) {
    _zdrSdev[ii] = _missingValue;
    _phidpMean[ii] = _missingValue;
    _phidp180[ii] = _missingValue;
    _phidp180Filt[ii] = _missingValue;
    _phidpFftFilt[ii] = _missingValue;
    _phidpMeanValid[ii] = _missingValue;
    _phidpJitter[ii] = _missingValue;
    _phidpSdev[ii] = _missingValue;
    _phidpMeanUnfold[ii] = _missingValue;
    _phidpUnfold[ii] = _missingValue;
    _phidpFilt[ii] = _missingValue;
    _phidpCond[ii] = _missingValue;
    _phidpCondFilt[ii] = _missingValue;
    _phidpAccumFilt[ii] = _missingValue;
    _validForKdp[ii] = false;
    _validForUnfold[ii] = false;
    if (_snr[ii] < _snrThreshold) {
      _kdp[ii] = _missingValue;
      _kdpZZdr[ii] = _missingValue;
      _kdpSC[ii] = _missingValue;
      _psob[ii] = _missingValue;
    } else {
      _kdp[ii] = 0;
      _kdpZZdr[ii] = 0;
      _kdpSC[ii] = 0;
      _psob[ii] = 0;
    }
    _dbzAttenCorr[ii] = 0;
    _zdrAttenCorr[ii] = 0;
    _regrFilt[ii] = _missingValue;
    _xxVals[ii] = -0.5 + ii * xxDelta;
  }
  
}

/////////////////////////////////////////////
// unfold phidp

int KdpFilt::_unfoldPhidp()

{
  
  // initialize the gate states - the state at each gate is
  // dependent on the phidp values and the spatial relatioship
  // between them

  _gateStatesInit();
  
  // compute mean and standard deviation of phidp
  // and mean angular jitter at each gate
  // also compute zdr sdev

  for (int ii = _nGatesStatsHalf; 
       ii < _nGates - _nGatesStatsHalf; ii++) {
    _computePhidpStats(ii);
    _computeZdrSdev(ii);
    _phidpJitter[ii] = _gateStates[ii].phidpJitter;
    _phidpMean[ii] = _gateStates[ii].phidpMean;
    _phidpMeanValid[ii] = _gateStates[ii].phidpMean;
    _phidpSdev[ii] = _gateStates[ii].phidpSdev;
  }
  
  // load up runs of valid phidp

  if (_findValidRuns()) {
    memcpy(_phidpUnfold, _phidp, _nGates * sizeof(double));
    return -1;
  }

  // create a mean field only for valid points
  // fill in gaps in mean phidp using values
  // from each end of the gap

  for (size_t irun = 0; irun < _gapRuns.size(); irun++) {
    int startGap = _gapRuns[irun].ibegin;
    int endGap = _gapRuns[irun].iend;
    int midGap = (startGap + endGap) / 2;
    // fill in first half of gap
    for (int jj = startGap; jj < midGap; jj++) {
      _phidpMeanValid[jj] = _phidpMeanValid[startGap-1];
      _gateStates[jj] = _gateStates[startGap-1];
    }
    // fill in last half of gap
    for (int jj = midGap; jj <= endGap; jj++) {
      _phidpMeanValid[jj] = _phidpMeanValid[endGap+1];
      _gateStates[jj] = _gateStates[endGap+1];
    }
  }

  // unfold the valid mean field

  int sumFold = 0;
  for (int ii = _firstValidGate; ii <= _lastValidGate; ii++) {
    int fold = 0;
    GateState &statePrev = _gateStates[ii-1];
    GateState &stateThis = _gateStates[ii];
    if (statePrev.meanxx < 0 && stateThis.meanxx < 0) {
      if (statePrev.meanyy < 0 && stateThis.meanyy > 0) {
        fold = -1;
      } else if (statePrev.meanyy > 0 && stateThis.meanyy < 0) {
        fold = 1;
      }
    }
    sumFold += fold;
    if (_phidpMeanValid[ii] == _missingValue) {
      _phidpMeanUnfold[ii] = _missingValue;
    } else {
      _phidpMeanUnfold[ii] = _phidpMeanValid[ii] + (sumFold * _foldRange);
    }

  } // ii

  // interpolate unfolded mean through the gaps

  for (size_t irun = 0; irun < _gapRuns.size(); irun++) {
    int startGap = _gapRuns[irun].ibegin;
    int endGap = _gapRuns[irun].iend;
    double valBefore = _phidpMeanUnfold[startGap-1];
    double valAfter = _phidpMeanUnfold[endGap+1];
    double range = valAfter - valBefore;
    double npts = endGap - startGap + 1;
    double delta = range / npts;
    double val = valBefore + delta;
    for (int jj = startGap; jj <= endGap; jj++, val += delta) {
      _phidpMeanUnfold[jj] = val;
    }
  }

  // data before the first valid gate and after the last valid gate

  for (int ii = 0; ii < _firstValidGate; ii++) {
    _phidpMeanUnfold[ii] = _phidpMeanUnfold[_firstValidGate];
  }

  for (int ii = _lastValidGate + 1; ii < _nGates; ii++) {
    _phidpMeanUnfold[ii] = _phidpMeanUnfold[_lastValidGate];
  }
  
  // set start and end phidp values for each run
  
  for (size_t irun = 0; irun < _validRuns.size(); irun++) {
    PhidpRun &run = _validRuns[irun];
    run.phidpBegin = _phidpMeanUnfold[run.ibegin];
    run.phidpEnd = _phidpMeanUnfold[run.iend];
  }

  // unfold the unfiltered phidp
  
  for (int ii = _firstValidGate; ii <= _lastValidGate; ii++) {
    if (!_validForUnfold[ii] || _phidp[ii] == _missingValue) {
      _phidpUnfold[ii] = _phidpMeanUnfold[ii];
    } else {
      double diff = _phidpMeanUnfold[ii] - _phidp[ii];
      int fold = (int) (fabs(diff / _foldRange) + 0.5);
      if (diff < 0) {
        fold *= -1;
      }
      _phidpUnfold[ii] = _phidp[ii] + fold * _foldRange;
    }
  }

  // before and after the data, set to the mean

  double sumAtStart = 0.0;
  for (int ii = 0; ii < _nGatesStats; ii++) {
    sumAtStart += _phidpMeanUnfold[ii + _firstValidGate];
  }
  double meanAtStart = sumAtStart / _nGatesStats; 

  double sumAtEnd = 0.0;
  for (int ii = 0; ii < _nGatesStats; ii++) {
    sumAtEnd += _phidpMeanUnfold[_lastValidGate - ii];
  }
  double meanAtEnd = sumAtEnd / _nGatesStats; 

  for (int ii = 0; ii < _firstValidGate; ii++) {
    _phidpUnfold[ii] = meanAtStart;
  }
  for (int ii = _lastValidGate + 1; ii < _nGates; ii++) {
    _phidpUnfold[ii] = meanAtEnd;
  }

  return 0;

}
    
/////////////////////////////////////////////
// compute KDP for region given gate limits

void KdpFilt::_computeKdp()

{

  // compute required array sizes, given that we need to
  // have space for the FIR filter on each side
  
  int arrayOffset = _firLength + 1;
  if (_nGatesStats > _firLength) {
    arrayOffset = _nGatesStats + 1;
  }
  int arrayLen = _nGates + 2 * arrayOffset;
  
  // allocate working arrays

  vector<double> work1_(arrayLen);
  double *work1 = work1_.data() + arrayOffset;
  
  vector<double> work2_(arrayLen);
  double *work2 = work2_.data() + arrayOffset;
  
  // vector<double> work1_, work2_;
  // work1_.resize(arrayLen);
  // double *work1 = work1_.data() + arrayOffset;
  // work1_.resize(arrayLen);
  // double *work1 = work1_.data() + arrayOffset;
  // double *work2 = work2_.alloc(arrayLen) + arrayOffset;

  // initialize working array work2
  
  _copyArray(work2, _phidpMeanUnfold);
  _padArray(work2);
  
  // apply FIR filter, computing work1 from work2, iterate
    
  for (int iloop = 0; iloop < _nFiltIterUnfolded; iloop++) {
    _applyFirFilter(work1, work2);
    _copyArray(work2, work1);
  } // iloop
  
  // save filtered phidp

  _copyArray(_phidpFilt, work2);
  _copyArray(_phidpCond, _phidpFilt);
  
  // compute conditioned phidp
  
  if (_useIterativeFiltering) {
    
    // use iterative filtering to remove phase shift on backscatter
    
    _copyArray(work2, _phidpCond);
    _padArray(work2);

    for (int iloop = 0; iloop < _nFiltIterCond; iloop++) {
      _applyFirFilter(work1, work2);
      _copyArrayCond(work2, work1, _phidpCond);
    } // iloop
    
    _copyArray(_phidpCondFilt, work2);
    
  } else {

    // compute phidp conditioned to remove phase shift on backscatter
    
    _computePhidpConditioned();
    
    // apply the FIR filter to the increasing phidp
    
    _copyArray(work2, _phidpCond);
    _padArray(work2);
    
    for (int iloop = 0; iloop < _nFiltIterCond; iloop++) {
      _applyFirFilter(work1, work2);
      _copyArray(work2, work1);
    } // iloop

    _copyArray(_phidpCondFilt, work1);

  }
  
  // compute KDP as slope between successive gates

  _loadKdp();

  // load up accumulated filtered phidp along range

  _loadPhidpAccumFilt(_phidpCondFilt, _phidpAccumFilt);

}

/////////////////////////////////////////////////
// compute phidp filtered with regression filter

void KdpFilt::_computePhidpRegrFilt()

{

  // compute regression order to be used

  int nGates = _getNGatesMaxValid();
  double maxRangeKm = nGates * _gateSpacingKm + _startRangeKm;
  int polyOrder = floor(maxRangeKm / _phidpFeatureLengthKm) + 2;
  if (polyOrder < 5) {
    polyOrder = 5;
  }
  
  // prepare for the fit
  
  ForsytheFit fit;
  fit.prepareForFit(polyOrder, _xxVals_);

  // perform the polynomial fit on unfolded phidp
  
  fit.performFit(_phidpUnfold_);
  vector<double> smoothed = fit.getYEstVector();
  for (int ii = 0; ii < nGates; ii++) {
    // _regrFilt[ii] = _phidpUnfold[ii] - smoothed[ii];
    _regrFilt[ii] = smoothed[ii];
  }
  
}

void KdpFilt::_computePhidpRegrFilt2()

{

  // initialize

  for (int ii = 0; ii < _nGates; ii++) {
    _regrFilt[ii] = _phidpMeanUnfold[ii];
  }

  // filter each valid run
  
  for (size_t irun = 0; irun < _validRuns.size(); irun++) {
    _computeRegrFiltValidRun(irun);
  }
  
}

void KdpFilt::_computePhidpRegrFilt3()

{

  // compute regression order to be used

  int nGates = _getNGatesMaxValid();
  double maxRangeKm = nGates * _gateSpacingKm + _startRangeKm;
  int polyOrder = floor(maxRangeKm / _phidpFeatureLengthKm) + 2;
  if (polyOrder < 5) {
    polyOrder = 5;
  }
  
  // find the entry in the forsythe array, if possible

  nasapolyfit::FitResult res = nasapolyfit::fitPolynomial(_xxVals_, _phidpUnfold_, polyOrder);
  for (int ii = 0; ii < nGates; ii++) {
    _regrFilt[ii] = nasapolyfit::evaluatePolynomial(res.coeffs, _xxVals_[ii]);
  }
  
}

/////////////////////////////////////////////////
// compute phidp filtered with regression filter
// weighted by rhohv

void KdpFilt::_computePhidpWtRegrFilt()

{

  // compute regression order to be used

  int nGates = _getNGatesMaxValid();
  double maxRangeKm = nGates * _gateSpacingKm + _startRangeKm;
  int polyOrder = floor(maxRangeKm / _phidpFeatureLengthKm) + 2;
  if (polyOrder < 5) {
    polyOrder = 5;
  }

  // compute weighted phidp and x data

  vector<double> xxValsWt(nGates);
  vector<double> phidpUnfoldWt(nGates);

  // double rho0 = 0.85;
  
  for (int ii = 0; ii < nGates; ii++) {

    // double rhohv = _rhohv[ii];
    // double wt = sqrt(rhohv);
    double wt = 1.0;
    
    // double wt = 1.0;
    // if (rhohv <= rho0) {
    //   wt = 0.0;
    // } else {
    //   wt = sqrt((rhohv - rho0) / (1.0 - rho0));
    // }

    xxValsWt[ii] = _xxVals[ii] * wt;
    phidpUnfoldWt[ii] = _phidpUnfold[ii] * wt; 
    
  } // ii
  
  // find the entry in the forsythe array, if possible
  
  ForsytheFit fit;
  fit.prepareForFit(polyOrder, xxValsWt);
    
  // perform the polynomial fit on unfolded phidp
  
  fit.performFit(phidpUnfoldWt);
  vector<double> smoothed = fit.getYEstVector();
  for (int ii = 0; ii < nGates; ii++) {
    // _regrFilt[ii] = _phidpUnfold[ii] - smoothed[ii];
    _regrFilt[ii] = smoothed[ii];
  }
  
}

///////////////////////////////////////////////////////
// run regression filter on a valid run

void KdpFilt::_computeRegrFiltValidRun(int runNum)

{

  const PhidpRun &run = _validRuns[runNum];
  
  // compute regression order to be used

  int nGates = run.iend - run.ibegin;
  double dRangeKm = nGates * _gateSpacingKm;
  int polyOrder = floor(dRangeKm / _phidpFeatureLengthKm) + 2;
  if (polyOrder < 5) {
    polyOrder = 5;
  }
  
  // prepare for the fit
  
  ForsytheFit fit;
  vector<double> xx;
  for (int ii = run.ibegin; ii <= run.iend; ii++) {
    xx.push_back(_xxVals_[ii]);
  }
  fit.prepareForFit(polyOrder, xx);
  
  // perform the polynomial fit on unfolded phidp
  
  vector<double> phidp;
  for (int ii = run.ibegin; ii <= run.iend; ii++) {
    phidp.push_back(_phidpMeanUnfold_[ii]);
  }
  fit.performFit(phidp);

  vector<double> smoothed = fit.getYEstVector();
  for (int ii = run.ibegin; ii <= run.iend; ii++) {
    _regrFilt[ii] = smoothed[ii - run.ibegin];
  }

}

/////////////////////////////////////////////
// load array ready for filter

void KdpFilt::_copyArray(double *out, const double *in)

{
  memcpy(out, in, _nGates * sizeof(double));
}

/////////////////////////////////////////////
// copy array conditionally

void KdpFilt::_copyArrayCond(double *out, const double *in,
                             const double *original)

{
  for (int ii = 0; ii < _nGates; ii++) {
    double diff = in[ii] - out[ii];
    if (fabs(diff) < _phidpDiffThreshold) {
      out[ii] = original[ii];
    } else {
      out[ii] = in[ii];
    }
  }
}

/////////////////////////////////////////////
// Pad array ready for filter

void KdpFilt::_padArray(double *array)

{
  for (int ii = -_firLength; ii < 0; ii++) {
    array[ii] = array[0];
  }
  for (int ii = _nGates; ii < _nGates + _firLength; ii++) {
    array[ii] = array[_nGates - 1];
  }
}

/////////////////////////////////////////////
// Load up KDP array

void KdpFilt::_loadKdp()

{

  // compute kdp
  
  for (int ii = 0; ii < _nGates; ii++) {

    // check SNR
    
    if (_snr[ii] < _snrThreshold) {
      _kdp[ii] = _missingValue;
      _kdpZZdr[ii] = _missingValue;
      _kdpSC[ii] = _missingValue;
      _psob[ii] = _missingValue;
      continue;
    }

    // get max DBZ for surrounding gates
    
    double maxDbz = _dbzMax[ii];
    
    // Use max dbz val to decide the number of gates over which to
    // compute kdp. The default value for adapLen is 4.
    
    int adapLen = 4;
    if (maxDbz < 20.0) {
      adapLen = 8;
    } else if (maxDbz < 35.0) {
      adapLen = 4;
    } else {
      adapLen = 2;
    }

    int i0 = ii - adapLen;
    if (i0 < 0) {
      i0 = 0;
    }
    int i1 = ii + adapLen;
    if (i1 > _nGates - 1) {
      i1 = _nGates - 1;
    }
    int len = i1 - i0;
    if (len < 1) {
      _kdp[ii] = 0;
    } else {
      double dphi = _phidpCondFilt[i1] - _phidpCondFilt[i0];
      _kdp[ii] = (dphi / (_gateSpacingKm * len)) / 2.0;
    }

    _kdpZZdr[ii] = _computeKdpFromZZdr(_dbzMedian[ii], _zdrMedian[ii]);

  } // ii

}

/////////////////////////////////////////////
// Load up filtered phidp accumulation array

void KdpFilt::_loadPhidpAccumFilt(const double *phidp, double *accum)

{

  double phidpStart = phidp[0];
  
  for (int ii = 0; ii < _nGates; ii++) {
    accum[ii] = phidp[ii] - phidpStart;
  } // ii

}

////////////////////////////////////////////////
// compute attenuation corrections based on KDP

void KdpFilt::_computeAttenCorrection()
  
{

  // accumulate corrections

  double sumDbzCorr = 0.0;
  double sumZdrCorr = 0.0;

  for (int ii = 0; ii < _nGates; ii++) {
    
    double kdp = _kdp[ii];
    if (kdp > 20) {
      kdp = 20;
    }
    
    double dbzCorr = 0.0;
    double zdrCorr = 0.0;
    
    if (_validForKdp[ii] && kdp != _missingValue && kdp > 0) {
      dbzCorr = _dbzAttenCoeff * pow(kdp, _dbzAttenExpon);
      zdrCorr = _zdrAttenCoeff * pow(kdp, _zdrAttenExpon);
    }

    sumDbzCorr += (dbzCorr * _gateSpacingKm);
    sumZdrCorr += (zdrCorr * _gateSpacingKm);

    _dbzAttenCorr[ii] = sumDbzCorr;
    _zdrAttenCorr[ii] = sumZdrCorr;

    if (_dbz[ii] > -9990) {
      _dbzCorrected[ii] = _dbz[ii] + sumDbzCorr;
    }
    if (_zdr[ii] > -9990) {
      _zdrCorrected[ii] = _zdr[ii] + sumZdrCorr;
    }

  } // ii

}

/////////////////////////////////////////////
// Apply FIR filter

void KdpFilt::_applyFirFilter(double *out, const double *in)

{

  for (int ii = -_firLenHalf; ii < _nGates + _firLenHalf; ii++) {
    double acc = 0.0;
    int kk = ii - _firLenHalf;
    for (int jj = 0; jj < _firLength; jj++, kk++) {
      acc = acc + _firCoeff[jj] * in[kk];
    }
    out[ii] = acc;
  } // ii

}
    
/////////////////////////////////////////////
// Get FIR filter gain

double KdpFilt::_getFirFilterGain()
  
{
  double sum = 0.0;
  for (int jj = 0; jj < _firLength; jj++) {
    sum += _firCoeff[jj];
  }
  return sum;
}
    
/////////////////////////////////////////////
// Compute DBZ max for surrounding gates

void KdpFilt::_computeDbzMax()

{
 
  for (int ii = 0; ii < _nGates; ii++) {
    double dmax = _dbz[ii];
    for (int kk = ii - _nGatesStatsHalf; kk <= ii + _nGatesStatsHalf; kk++) {
      if (kk >= 0 && kk < _nGates) {
        double dbz = _dbz[kk];
        if (dbz > dmax) {
          dmax = dbz;
        }
      }
    }
    _dbzMax[ii] = dmax;
  } // ii

}
    
////////////////////////////////////////////////////////////////////
// compute phidp conditioned to remove phase shift on backscatter

void KdpFilt::_computePhidpConditioned()

{

  bool debug = false;

  // loop through the valid runs

  for (size_t irun = 0; irun < _validRuns.size(); irun++) {

    PhidpRun &run = _validRuns[irun];
    run.phidpBegin = _phidpMeanUnfold[run.ibegin];
    run.phidpEnd = _phidpMeanUnfold[run.iend];

    // find the regions where phidp increases and then comes down again

    bool increasing = false;
    bool decreasing = false;
    double prevDiff = 0.0;
    
    int topIndex = -1;
    int botIndex = -1;
    
    vector<int> topIndices;
    vector<int> botIndices;
    
    for (int igate = run.ibegin + 1; igate <= run.iend; igate++) {
    
      double diff = _phidpFilt[igate] - _phidpFilt[igate-1];
      
      // look for increasing trend
      
      if (diff > 0 && prevDiff > 0) {
        if (!increasing) {
          botIndex = igate - 2;
          increasing = true;
          if (topIndex > 0) {
            topIndices.push_back(topIndex);
            botIndices.push_back(botIndex);
          }
        }
      } else {
        increasing = false;
      }
      
      // look for decreasing trend
      
      if (diff < 0 && prevDiff < 0) {
        if (!decreasing) {
          topIndex = igate - 2;
          decreasing = true;
        }
      } else {
        decreasing = false;
      }
      
      prevDiff = diff;
      
    } // igate
    
    // for each bot index, look back for the previous location at
    // the same value
    
    int prevBotIndex = 0;
    for (size_t ii = 0; ii < botIndices.size(); ii++) {
      
      if (debug)
        cerr << "----------------------------------------" << endl;
      
      // look back for the point where the phidp value
      // drops below the bottom value
      int botIndex = botIndices[ii];
      double botVal = _phidpFilt[botIndex];
      bool matchFound = false;
      
      if (debug)
        cerr << "DDDDDDD prevBotIndex, botIndex, botVal: "
             << prevBotIndex << ", " << botIndex << ", " << botVal << endl;
      
      for (int igate = topIndices[ii]; igate >= prevBotIndex; igate--) {
        double val = _phidpFilt[igate];
        if (val < botVal) {
          if (debug)
            cerr << "CCCCCCC val, igate: " << val << ", " << igate << endl;
          for (int jgate = igate + 1; jgate < botIndices[ii]; jgate++) {
            _phidpCond[jgate] = botVal;
          } // jgate
          matchFound = true;
          break;
        }
      } // igate
      
      if (debug)
        cerr << "FFFFFFF matchFound: " << matchFound << endl;
      
      if (!matchFound && prevBotIndex > 0) {
        // did not find a value below the bottom value
        // so move forward instead
        double prevBotVal = _phidpFilt[prevBotIndex];
        if (debug)
          cerr << "HHHHHHH prevBotIndex, prevBotVal: "
               << prevBotIndex << ", " << prevBotVal << endl;
        for (int igate = prevBotIndex + 1; igate <= botIndex; igate++) {
          double val = _phidpFilt[igate];
          if (val <= prevBotVal) {
            if (debug)
              cerr << "EEEEEE val, igate: " << val << ", " << igate << endl;
            for (int jgate = prevBotIndex + 1; jgate < igate; jgate++) {
              _phidpCond[jgate] = prevBotVal;
            } // jgate
            break;
          }
        } // igate
      }

      prevBotIndex = botIndex;
      
    } // ii

  } // irun

}
  
/////////////////////////////////////////////
// compute the folding values and range
// by inspecting the phidp values

void KdpFilt::_computeFoldingRange()

{

  // check if fold is at 90 or 180
  
  double phidpMin = 9999;
  double phidpMax = -9999;
  for (int igate = 0; igate < _nGates; igate++) {
    double phi = _phidp[igate];
    if (phi != _missingValue) {
      if (phi < phidpMin) phidpMin = phi;
      if (phi > phidpMax) phidpMax = phi;
    }
  }

  _foldsAt90 = false;
  _foldVal = 180.0;
  if (phidpMin > -90 && phidpMax < 90) {
    _foldVal = 90.0;
    _foldsAt90 = true;
  }
  _foldRange = _foldVal * 2.0;
  
  // if values range from (0 -> 360), normalize to (-180 -> 180)
  
  if (phidpMin >= 0 && phidpMax > 180) {
    for (int igate = 0; igate < _nGates; igate++) {
      if (_phidp[igate] != _missingValue) {
        _phidp[igate] -= 180.0;
      }
    }
  }

  // populate phidp array that folds at 180

  if (_foldsAt90) {
    for (int igate = 0; igate < _nGates; igate++) {
      if (_phidp[igate] != _missingValue) {
        _phidp180[igate] = _phidp[igate] * 2.0;
      }
    }
  } else {
    std::copy(_phidp, _phidp + _nGates, _phidp180);
  }

}

//////////////////////////////////////////////////////////////////////////
// load up runs of 'valid' phidp
//
// returns -1 if no valid runs found, 0 otherwise

int KdpFilt::_findValidRuns()
{
  
  // first pass - load up all runs

  vector<PhidpRun> allRuns;
  int runLen = 0;
  for (int igate = 0; igate < _nGates; igate++) {

    bool validGate = _isGateValid(igate);

    if (validGate) {
      runLen++;
    }

    // save runs longer than _nGatesStats

    if (!validGate) {
      if (runLen > _nGatesStats) {
        int iend = igate - 1;
        int ibegin = iend - runLen + 1;
        PhidpRun run(ibegin, iend);
        allRuns.push_back(run);
      }
      runLen = 0;
    } else if (igate == _nGates - 1) {
      // last gate in ray
      if (runLen > _nGatesStats) {
        int iend = igate;
        int ibegin = iend - runLen + 1;
        PhidpRun run(ibegin, iend);
        allRuns.push_back(run);
      }
    }

  } // igate
  
  // now combine runs with a gap between them
  // smaller than or equal to _nGatesStatsHalf

  vector<PhidpRun> combRuns;
  bool done = false;
  int count = 0;
  while (!done) {
    count++;
    done = true;
    combRuns.clear();
    if (allRuns.size() < 2) {
      combRuns = allRuns;
      break; // from while loop
    }
    for (size_t irun = 0; irun < allRuns.size() - 1; irun++) {
      PhidpRun thisRun = allRuns[irun];
      PhidpRun nextRun = allRuns[irun+1];
      int gapLen = nextRun.ibegin - thisRun.iend - 1;
      if (gapLen > _nGatesStatsHalf) {
        combRuns.push_back(thisRun);
        if (irun == allRuns.size() - 2) {
          combRuns.push_back(nextRun);
        }
      } else {
        // add combined run
        thisRun.iend = nextRun.iend;
        combRuns.push_back(thisRun);
        for (size_t jrun = irun + 2; jrun < allRuns.size(); jrun++) {
          // add remainder of runs
          combRuns.push_back(allRuns[jrun]);
        } // jrun
        done = false;
        // copy modified array back to allRuns ready for another try
        allRuns = combRuns;
        break; // from irun loop
      }
    } // irun
  } // while (!done)
  
  // find runs longer than 2 * _nGatesStats
  // trim each end by _nGatesStats/2
  // and add to valid runs array
  
  _validRuns.clear();
  for (size_t irun = 0; irun < combRuns.size(); irun++) {
    PhidpRun run = combRuns[irun];
    if (run.len() >= _nGatesStats * 2) {
      run.ibegin += _nGatesStatsHalf;
      run.iend -= _nGatesStatsHalf;
      _validRuns.push_back(run);
    }
  }

  if (_validRuns.size() < 1) {
    // no valid runs
    return -1;
  }

  // save the gaps

  _gapRuns.clear();
  for (size_t irun = 1; irun < _validRuns.size(); irun++) {
    PhidpRun gapRun;
    gapRun.ibegin = _validRuns[irun-1].iend + 1;
    gapRun.iend = _validRuns[irun].ibegin - 1;
    _gapRuns.push_back(gapRun);
  }

  // set global start and end gates
  
  _firstValidGate = _validRuns[0].ibegin + 2;
  _lastValidGate = _validRuns[_validRuns.size()-1].iend - 2;

  // set valid flags for valid runs
  
  for (size_t irun = 0; irun < _validRuns.size(); irun++) {
    const PhidpRun &validRun = _validRuns[irun];
    for (int igate = validRun.ibegin; igate <= validRun.iend; igate++) {
      _validForUnfold[igate] = true;
      _validForKdp[igate] = true;
    }
  }

  // if gap is smaller than the surrounding valid runs,
  // flag as OK for KDP

  for (size_t igap = 0; igap < _gapRuns.size(); igap++) {
    const PhidpRun &gap = _gapRuns[igap];
    const PhidpRun &prevValid = _validRuns[igap];
    const PhidpRun &nextValid = _validRuns[igap + 1];
    if (prevValid.len() > gap.len() && nextValid.len() > gap.len()) {
      for (int igate = gap.ibegin; igate <= gap.iend; igate++) {
        _validForKdp[igate] = true;
      }
    }
  }

  // cerr << "*********************** valid runs **************************" << endl;
  // for (size_t irun = 0; irun < _validRuns.size(); irun++) {
  //   _validRuns[irun].print(irun, cerr);
  // }
  // cerr << "*************************************************************" << endl;

  // cerr << "*********************** gap runs **************************" << endl;
  // for (size_t irun = 0; irun < _gapRuns.size(); irun++) {
  //   _gapRuns[irun].print(irun, cerr);
  // }
  // cerr << "*************************************************************" << endl;

  return 0;

}
  
//////////////////////////////////////////////////////////////////////////
// Check gate for validity
 
bool KdpFilt::_isGateValid(int igate)

{

  // check we have non-missing data

  if (_phidpMean[igate] == _missingValue) {
    return false;
  }

  // check SNR
  
  if (_checkSnr && _snrAvailable) {
    if ((_snr[igate] == _missingValue) || (_snr[igate] < _snrThreshold)) {
      return false;
    }
  }

  // check for clutter effects

  if (_phidpSdev[igate] > _phidpSdevMax) {
    return false;
  }
  if (_phidpJitter[igate] > _phidpJitterMax) {
    return false;
  }
  if (_checkZdrSdev) {
    if (_zdrSdev[igate] > _zdrSdevMax) {
      return false;
    }
  }
  if (_checkRhohv) {
    if ((_rhohv[igate] != _missingValue) && (_rhohv[igate] < _rhohvThreshold)) {
      return false;
    }
  }

  return true;

}

//////////////////////////////////////////////////////////////////////////
// Initialize the state at each gate
 
void KdpFilt::_gateStatesInit()

{

  // init (x,y) representation of phidp

  for (int ii = 0; ii < _nGates; ii++) {
    GateState &state = _gateStates[ii];
    state.init(_missingValue);
    if (_phidp[ii] != _missingValue) {
      state.missing = false;
      double phase = _phidp[ii];
      state.phidp = _phidp[ii];
      if (_foldsAt90) {
        phase *= 2.0;
      }
      double sinVal, cosVal;
      ta_sincos(phase * DEG_TO_RAD, &sinVal, &cosVal);
      state.xx = cosVal;
      state.yy = sinVal;
    }
  }

  // init dist between phidp at successive gates

  for (int ii = 1; ii < _nGates; ii++) {
    GateState &istate0 = _gateStates[ii-1];
    GateState &istate1 = _gateStates[ii];
    if (!istate0.missing && !istate1.missing) {
      double xx0 = istate0.xx;
      double yy0 = istate0.yy;
      double xx1 = istate1.xx;
      double yy1 = istate1.yy;
      double dx = xx1 - xx0;
      double dy = yy1 - yy0;
      double dist = sqrt(dx * dx + dy * dy);
      _gateStates[ii].distFromPrev = dist;
    }
  }
  
}
  
//////////////////////////////////////////////////////////////////////////
//  To calculate the mean phidp, standard deviation, and jitter
//  in phidp at a gate, using stats on the circle

void KdpFilt::_computePhidpStats(int igate)
  
{

  GateState &istate = _gateStates[igate];
  
  double count = 0.0;
  double sumxx = 0.0;
  double sumyy = 0.0;
  double sumDist = 0.0;
  double sumDistSq = 0.0;
  
  for (int jj = igate - _nGatesStatsHalf;
       jj <= igate + _nGatesStatsHalf; jj++) {
    if (jj < 0 || jj >= _nGates) {
      continue;
    }
    GateState &jstate = _gateStates[jj];
    if (jstate.missing) {
      continue;
    }
    double xx = jstate.xx;
    double yy = jstate.yy;
    double dist = jstate.distFromPrev;
    sumxx += xx;
    sumyy += yy;
    sumDist += dist;
    sumDistSq += dist * dist;
    count++;
  }
  
  if (count <= _nGatesStatsHalf) {
    return;
  }

  // mean phidp
  
  istate.meanxx = sumxx / count;
  istate.meanyy = sumyy / count;
  
  double phase = atan2(istate.meanyy, istate.meanxx) * RAD_TO_DEG;
  if (_foldsAt90) {
    phase *= 0.5;
  }
  istate.phidpMean = phase;
  
  // jitter
  
  double meanDist = sumDist / count;
  double meanAngChangePerGate = meanDist * RAD_TO_DEG;
  if (_foldsAt90) {
    meanAngChangePerGate *= 0.5;
  }
  istate.phidpJitter = meanAngChangePerGate;
  
  // sdev of distance moved, is a proxy for sdev of phidp
  
  if (count > 2) {
    double term1 = sumDistSq / count;
    double term2 = meanDist * meanDist;
    if (term1 >= term2) {
      double sdev = sqrt(term1 - term2) * RAD_TO_DEG;
      if (_foldsAt90) {
        sdev *= 0.5;
      }
      istate.phidpSdev = sdev;
    }
  }
  
}

//////////////////////////////////////////////////////////////////////////
//  To calculate the sdev of ZDR

void KdpFilt::_computeZdrSdev(int igate)
  
{

  double count = 0.0;
  double sum = 0.0;
  double sumSq = 0.0;
  
  for (int jj = igate - _nGatesStatsHalf;
       jj <= igate + _nGatesStatsHalf; jj++) {
    if (jj < 0 || jj >= _nGates) {
      continue;
    }
    double zdr = _zdr[jj];
    if (zdr != _missingValue) {
      sum += zdr;
      sumSq += zdr * zdr;
      count++;
    }
  } // jj
  
  if (count <= _nGatesStatsHalf) {
    // not enough data
    return;
  }

  if (count > 2) {
    double mean = sum / count;
    double term1 = sumSq / count;
    double term2 = mean * mean;
    if (term1 >= term2) {
      double sdev = sqrt(term1 - term2);
      _zdrSdev[igate] = sdev;
    }
  }
  
}

//////////////////////////////////////////////////////////////////////////
// Write the ray data to a text file

void KdpFilt::_writeRayDataToFile()
  
{

  // make sure output dir exists

  if (ta_makedir_recurse(_rayFileDir.c_str())) {
    int errNum = errno;
    cerr << "ERROR - KdpFilt::_writeRayDataToFile()" << endl;
    cerr << "  Cannot create dir: " << _rayFileDir << endl;
    cerr << "  " << strerror(errNum) << endl;
    return;
  }

  // create file name

  char filePath[MAX_PATH_LEN];
  DateTime rtime(_timeSecs);
  int msecs = (int) (_timeFractionSecs * 1000.0 + 0.5);
  sprintf(filePath,
          "%s%skdpray_%.4d%.2d%.2d-%.2d%.2d%.2d.%.3d_el-%06.2f_az-%06.2f_.txt",
          _rayFileDir.c_str(), PATH_DELIM,
          rtime.getYear(), rtime.getMonth(), rtime.getDay(),
          rtime.getHour(), rtime.getMin(), rtime.getSec(), msecs,
          _elevDeg, _azDeg);

  // open file

  FILE *out = fopen(filePath, "w");
  if (out == NULL) {
    int errNum = errno;
    cerr << "ERROR - KdpFilt::_writeRayDataToFile()" << endl;
    cerr << "  Cannot open file: " << filePath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return;
  }

  // write header line

  fprintf(out,
          "# gateNum validKdp validUnfold snr dbz zdr rhohv phidp "
          "phidpMean phidpMeanValid phidpJitter phidpSdev "
          "phidpMeanUnfold phidpUnfold phidpFilt phidpCond phidpCondFilt "
          "zdrSdev psob kdp "
          "dbzAtten zdrAtten dbzCorrected zdrCorrected "
          "regrFilt phidp180 phidp180Filt phidpFftFilt\n");

  // write data

  for (int igate = 0; igate < _nGates; igate++) {
    double dbzCorrected = 0;
    double zdrCorrected = 0;
    if (_dbz[igate] > -9990 && _dbzAttenCorr[igate] > -9990) {
      dbzCorrected = _dbz[igate] + _dbzAttenCorr[igate];
    } else {
      dbzCorrected = _dbz[igate];
    }
    if (_zdr[igate] > -9990 && _zdrAttenCorr[igate] > -9990) {
      zdrCorrected = _zdr[igate] + _zdrAttenCorr[igate];
    } else {
      zdrCorrected = _zdr[igate];
    }
    fprintf(out,
            "%3d %3d %3d "
            "%10.3f %10.3f %10.3f %10.3f %10.3f %10.3f %10.3f %10.3f "
            "%10.3f %10.3f %10.3f %10.3f %10.3f %10.3f %10.3f %10.3f "
            "%10.3f %10.3f %10.3f %10.3f %10.3f %10.3f %10.3f %10.3f %10.3f\n",
            igate,
            (_validForKdp[igate]?1:0),
            (_validForUnfold[igate]?1:0),
            _getPlotVal(_snr[igate], -10),
            _getPlotVal(_dbz[igate], -20),
            _getPlotVal(_zdr[igate], 0),
            _getPlotVal(_rhohv[igate], 0),
            _getPlotVal(_phidp[igate], 0),
            _getPlotVal(_phidpMean[igate], 0),
            _getPlotVal(_phidpMeanValid[igate], 0),
            _getPlotVal(_phidpJitter[igate], 0),
            _getPlotVal(_phidpSdev[igate], 0),
            _getPlotVal(_phidpMeanUnfold[igate], 0),
            _getPlotVal(_phidpUnfold[igate], 0),
            _getPlotVal(_phidpFilt[igate], 0),
            _getPlotVal(_phidpCond[igate], 0),
            _getPlotVal(_phidpCondFilt[igate], 0),
            _getPlotVal(_zdrSdev[igate], 0),
            _getPlotVal(_psob[igate], 0),
            _getPlotVal(_kdp[igate], 0),
            _getPlotVal(_dbzAttenCorr[igate], 0),
            _getPlotVal(_zdrAttenCorr[igate], 0),
            _getPlotVal(dbzCorrected, 0),
            _getPlotVal(zdrCorrected, 0),
            _getPlotVal(_regrFilt[igate], 0),
            _getPlotVal(_phidp180[igate], 0),
            _getPlotVal(_phidp180Filt[igate], 0),
            _getPlotVal(_phidpFftFilt[igate], 0)
            );
  }
  
  // close file

  fclose(out);

}

//////////////////////////////////////////////////////////////////////////
// Get a value suitable for plotting
// i.e. interpret missing data reasonably

double KdpFilt::_getPlotVal(double val, double valIfMissing)
  
{
  if (val < -9990) {
    return valIfMissing;
  } else {
    return val;
  }
}

////////////////////////////////////////////////////////////
// Compute estimated kdp from Z and ZDR using power law

double KdpFilt::_computeKdpFromZZdr(double dbz,
                                    double zdr)
  
{
  
  double zzLin = pow(10.0, dbz / 10.0);

  if (zdr < 0.1) {
    zdr = 0.1;
  }
  double zdrLin = pow(10.0, zdr / 10.0);
  
  double zTerm = pow(zzLin, _kdpZExpon);
  double zdrTerm = pow(zdrLin, _kdpZdrExpon);
  double kdpEst = zTerm * zdrTerm * _kdpZZdrCoeff;

  return kdpEst;
  
}

////////////////////////////////////////////////////////////
/// load up kdp conditioned using ZZDR self-consistency

void KdpFilt::_loadKdpSC()

{

  int startGate = 0;
  int endGate = 0;
  bool inRun = false;

  for (int igate = 0; igate < _nGates; igate++) {

    if (_kdp[igate] == _missingValue ||
        _kdp[igate] <= _kdpMinForSelfConsistency ||
        _kdpZZdr[igate] == _missingValue) {
      
      // non-positive KDP
    
      if (inRun) {

        // end of run so process it
      
        _loadKdpSCRun(startGate, endGate);

      }
        
      // start again
      
      startGate = igate + 1;
      endGate = igate + 1;
      inRun = false;
      
    } else {
      
      // have positive KDP, update run info
      
      if (!inRun) {
        startGate = igate;
      }
      endGate = igate;
      inRun = true;
      
    } // if (_kdp[igate] == _missingValue ...

  } // igate

  // check for active run
  
  if (inRun) {
    _loadKdpSCRun(startGate, endGate);
  }

}

////////////////////////////////////////////////////////////
/// load up kdp conditioned using ZZDR self-consistency
/// for a specific run

void KdpFilt::_loadKdpSCRun(int startGate, int endGate)

{

  if (endGate - startGate < 3) {
    // not enough gates for this to make sense
    return;
  }

  // integrate the KDP in the run

  double sumKdp = 0.0;
  double sumKdpZZdr = 0.0;
  
  for (int igate = startGate; igate <= endGate; igate++) {
    sumKdp += _kdp[igate];
    sumKdpZZdr += _kdpZZdr[igate];
  } // igate

  if (sumKdpZZdr < 0.5) {
    return;
  }

  // compute factor to normalize the ZZdr estimate
  // from the measured estimate

  double condFactor = sumKdp / sumKdpZZdr;

  // load the KDP conditioned by self-consistency

  for (int igate = startGate; igate <= endGate; igate++) {
    _kdpSC[igate] = _kdpZZdr[igate] * condFactor;
  }

}

////////////////////////////////////////////////////////////
/// filter phidp using FFT

void KdpFilt::_fftFilter()

{

  // fill missing gates with random values

  _fillPhidpMissingGates();

  // create complex array for phidp180

  vector<RadarComplex_t> phiComplex;
  phiComplex.resize(_nGates);
  for (int igate = 0; igate < _nGates; igate++) {
    RadarComplex::setFromDegrees(_phidp180[igate], phiComplex[igate]);
  }

  // perform forward FFT

  assert(_nGates > 0);
  assert(_gateSpacingKm > 0.0);
  assert(_phidp180 == _phidp180_.data());
  assert(_phidp180Filt == _phidp180Filt_.data());
  assert(_phidpFftFilt == _phidpFftFilt_.data());
  
  vector<RadarComplex_t> phiSpec;
  phiSpec.resize(_nGates);
  RadarFft fft;
  fft.init(_nGates);
  fft.fwd(phiComplex.data(), phiSpec.data());

  // determine cutoff
  
  const double f_cut = 1.0 / _phidpFeatureLengthKm;  // cycles/km
  // const double f_cut = 1.0 / 4.0;  // cycles/km

  // apply filter
  
  for (int kk = 0; kk < _nGates; ++kk) {
    // FFT bin interpreted as signed frequency index
    int kk_signed = (kk <= _nGates / 2) ? kk : kk - _nGates;
    double f = std::abs(kk_signed) / (_nGates * _gateSpacingKm);  // cycles/km
    if (f > f_cut) {
      phiSpec[kk].re = 0.0;
      phiSpec[kk].im = 0.0;
    }
  }
  
  // perform inverse FFT
  
  fft.inv(phiSpec.data(), phiComplex.data());

  // compute the filtered PHIDP

  for (int kk = 0; kk < _nGates; ++kk) {
    _phidp180Filt[kk] = RadarComplex::argDeg(phiComplex[kk]);
    if (_foldsAt90) {
      _phidpFftFilt[kk] = _phidp180Filt[kk] / 2.0;
    } else {
      _phidpFftFilt[kk] = _phidp180Filt[kk];
    }
  }

}

////////////////////////////////////////////////////////////
/// fill phidp missing gates with random values

void KdpFilt::_fillPhidpMissingGates()

{

  // Seed source
  std::random_device rd;
  
  // Mersenne Twister generator
  std::mt19937 gen(rd());
  
  // Uniform distribution [0, 1)
  std::uniform_real_distribution<double> dist(0.0, 1.0);
  
  for (int igate = 0; igate < _nGates; igate++) {
    if (_phidp[igate] == _missingValue) {
      _phidp180[igate] = (dist(gen) - 0.5) * 180.0;
    }
  }

}
