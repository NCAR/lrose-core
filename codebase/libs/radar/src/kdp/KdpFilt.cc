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
using namespace std;

// Constructor

KdpFilt::KdpFilt()
  
{

  _nGatesPad = 21;
  _setNGates(0);
  _setNGatesStats(9);

  _limitMaxRange = false;
  _maxRangeKm = 0.0;

  _wavelengthCm = 10.0;

  _startRangeKm = 0.0;
  _gateSpacingKm = 0.0;

  _elevDeg = -9999;
  _azDeg = -9999;

  // initialize attenuation correction for Sband

  _dbzAttenCoeff = 0.017;
  _dbzAttenExpon = 0.84;
  _zdrAttenCoeff = 0.003;
  _zdrAttenExpon = 1.05;
  _attenCoeffsSpecified = false;

  // initialize computation of KDP from Z and ZDR

  _kdpZExpon = 1.0;
  _kdpZdrExpon = -2.05;
  _kdpZZdrCoeff = 3.32e-5;
  _kdpZZdrMedianLen = 5;

  // debugging

  _writeRayFile = false;

}

// Destructor

KdpFilt::~KdpFilt()
  
{

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
// Set attenuation coefficients

void KdpFilt::setAttenCoeffs(double dbzCoeff, double dbzExpon,
                             double zdrCoeff, double zdrExpon)

{

  _dbzAttenCoeff = dbzCoeff;
  _dbzAttenExpon = dbzExpon;
  _zdrAttenCoeff = zdrCoeff;
  _zdrAttenExpon = zdrExpon;
  _attenCoeffsSpecified = true;

}
  
////////////////////////////////////////////
// Set processing options from params object

void KdpFilt::setParams(const KdpFiltParams &params)
{

  _params = params;

  if (params.KDP_specify_coefficients_for_attenuation_correction) {
    setAttenCoeffs(params.KDP_dbz_attenuation_coefficient,
                   params.KDP_dbz_attenuation_exponent,
                   params.KDP_zdr_attenuation_coefficient,
                   params.KDP_zdr_attenuation_exponent);
  }

  // initialize KDP object

  _setNGatesStats(_params.KDP_ngates_for_stats);

  if (_params.KDP_write_ray_files) {
    _params.KDP_compute_all_filters = pTRUE;
  }

  // set params for computing KDP from Z and ZDR

  _kdpZExpon = _params.KDP_self_con_Z_expon;
  _kdpZdrExpon = _params.KDP_self_con_ZDR_expon;
  _kdpZZdrCoeff = _params.KDP_self_con_Z_coeff_10cm * (10.0 / _wavelengthCm);
  _kdpZZdrMedianLen = _params.KDP_self_con_median_filter_len;

  // writing ray files

  setWriteRayFile(_params.KDP_write_ray_files,
                  _params.KDP_ray_files_dir);

}

////////////////////////////////////////////////////////////////////////
// Initialize the object arrays for later use.
// Do this if you need access to the arrays, but have not yet called
// compute(), and do not plan to do so.
// For example, you may want to output missing fields that you have
// not computed, but the memory needs to be there.

void KdpFilt::initializeArrays(int nGates)

{

  _setNGates(nGates);
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

  // set range details

  _startRangeKm = startRangeKm;
  _gateSpacingKm = gateSpacingKm;

  // set number of gates

  _setNGates(nGates);

  // initialize the data arrays
  
  _missingValue = missingValue;
  _initArrays(snr, dbz, zdr, rhohv, phidp, _getNGatesMaxValid());
  
  // check if fold is at 90 or 180
  
  _computeFoldingRange();

  // unfold phidp
  
  if (_unfoldPhidp()) {
    // no good data in whole ray, fill with missing, return early
    for (int igate = 0; igate < _nGates; igate++) {
      _kdp[igate] = _missingValue;
      _kdpSC[igate] = _missingValue;
      _phidpSC[igate] = _missingValue;
      _kdpZZdr[igate] = _missingValue;
      _delta[igate] = _missingValue;
      _deltaMean[igate] = _missingValue;
    }
    return 0;
  }

  // filter unfolded PHIDP
  
  _filterPhidp();
  
  // compute kdp from the filtered data
  
  _computeKdp();

  // load up conditional KDP from estimated kdp and kdpZZdr

  _loadKdpSC();

  // confine the results to the valid regions
  
  _censorNonValidKdp();

  // compute attenuation corrections

  _computeAttenCorrection();
  
  // write ray file if requested

  if (_writeRayFile) {
    _writeRayDataToFile();
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

  _setNGates(nGates);

  // initialize the data arrays
  
  _missingValue = missingValue;
  _initArrays(NULL, NULL, NULL, NULL, phidp, _getNGatesMaxValid());
  
  // check if fold is at 90 or 180
  
  _computeFoldingRange();
  
  // initialize the gate props - the state at each gate is
  // dependent on the phidp values and the spatial relatioship
  // between them

  _gatePropsInit();
  
  // compute mean and standard deviation of phidp
  // and mean angular jitter at each gate

  for (int ii = _nGatesStatsHalf; 
       ii < _nGates - _nGatesStatsHalf; ii++) {
    _computePhidpStats(ii);
    _phidpJitter[ii] = _gateProps[ii].phidpJitter;
    _phidpMean[ii] = _gateProps[ii].phidpMean;
    _phidpMeanFilled[ii] = _gateProps[ii].phidpMean;
    _phidpSdev[ii] = _gateProps[ii].phidpSdev;
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
  
  // allocate the arrays needed
  // copy input arrays, leaving extra space at the beginning
  // for negative indices and at the end for filtering as required

  _gateProps.resize(_nGates);

  _validRuns.clear();
  _gapRuns.clear();

  _validForKdp.resize(_nGates);

  _snr.resize(_nGates);

  _dbz.resize(_nGates);
  _dbzMedian.resize(_nGates);

  _rhohv.resize(_nGates);

  _zdr.resize(_nGates);
  _zdrSdev.resize(_nGates);
  _zdrMedian.resize(_nGates);

  _phidp.resize(_nGates);
  _phidpMean.resize(_nGates);
  _phidpMeanFilled.resize(_nGates);
  _phidpJitter.resize(_nGates);
  _phidpSdev.resize(_nGates);
  _phidpUnfold.resize(_nGates);
  _phidpUnfoldFilled.resize(_nGates);
  
  _phidpFilt.resize(_nGates);
  _phidpFiltTrend.resize(_nGates);
  _phidpFirFilt.resize(_nGates);
  _phidpQuadFilt.resize(_nGates);
  _kdpQuadFilt.resize(_nGates);
  _phidpFftFilt.resize(_nGates);
  _phidpRegrFilt.resize(_nGates);

  _kdp.resize(_nGates);
  _kdpZZdr.resize(_nGates);
  _kdpSC.resize(_nGates);
  _phidpSC.resize(_nGates);

  _delta.resize(_nGates);
  _deltaMean.resize(_nGates);

  _dbzAttenCorr.resize(_nGates);
  _zdrAttenCorr.resize(_nGates);
  _dbzCorrected.resize(_nGates);
  _zdrCorrected.resize(_nGates);

  _xxVals.resize(_nGatesPadded);
  _scBlock.resize(_nGates);
  
  // copy data to working arrays

  // SNR
  
  if (snr != NULL) {
    for (int ii = 0; ii < _nGates; ii++) {
      _snr[ii] = snr[ii];
    }
    _snrAvailable = true;
  } else {
    for (int ii = 0; ii < _nGates; ii++) {
      _snr[ii] = _missingValue;
    }
    _snrAvailable = false;
  }

  // DBZ
  
  if (dbz != NULL) {
    for (int ii = 0; ii < _nGates; ii++) {
      _dbz[ii] = dbz[ii];
    }
  } else {
    for (int ii = 0; ii < _nGates; ii++) {
      _dbz[ii] = _missingValue;
    }
  }
  for (int ii = 0; ii < _nGates; ii++) {
    _dbzMedian[ii] = _dbz[ii];
  }
  FilterUtils::applyMedianFilter(_dbzMedian.data(), _nGates,
                                 _kdpZZdrMedianLen, _missingValue);

  std::copy(_dbz.begin(), _dbz.end(), _dbzCorrected.begin());

  // RHOHV
  
  if (rhohv != NULL) {
    for (int ii = 0; ii < _nGates; ii++) {
      _rhohv[ii] = rhohv[ii];
    }
    _rhohvAvailable = true;
  } else {
    for (int ii = 0; ii < _nGates; ii++) {
      _rhohv[ii] = _missingValue;
    }
    _rhohvAvailable = false;
  }

  // ZDR
  
  if (zdr != NULL) {
    for (int ii = 0; ii < _nGates; ii++) {
      _zdr[ii] = zdr[ii];
    }
    _zdrAvailable = true;
  } else {
    for (int ii = 0; ii < _nGates; ii++) {
      _zdr[ii] = _missingValue;
    }
    _zdrAvailable = false;
  }
  std::copy(_zdr.begin(), _zdr.end(), _zdrMedian.begin());
  std::copy(_zdr.begin(), _zdr.end(), _zdrCorrected.begin());
  
  FilterUtils::applyMedianFilter(_zdrMedian.data(), _nGates,
                                 _kdpZZdrMedianLen, _missingValue);

  // PHIDP
  
  if (phidp != NULL) {
    for (int ii = 0; ii < _nGates; ii++) {
      _phidp[ii] = phidp[ii];
    }
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

  // initialize computed arrays

  for (int ii = 0; ii < _nGates; ii++) {

    _validForKdp[ii] = false;

    _zdrSdev[ii] = _missingValue;

    _phidpMean[ii] = _missingValue;
    _phidpMeanFilled[ii] = _missingValue;
    _phidpJitter[ii] = _missingValue;
    _phidpSdev[ii] = _missingValue;
    _phidpUnfold[ii] = _missingValue;
    _phidpUnfoldFilled[ii] = _missingValue;

    _phidpFilt[ii] = _missingValue;
    _phidpFiltTrend[ii] = _missingValue;
    _phidpFirFilt[ii] = _missingValue;
    _phidpQuadFilt[ii] = _missingValue;
    _kdpQuadFilt[ii] = _missingValue;
    _phidpRegrFilt[ii] = _missingValue;
    _phidpFftFilt[ii] = _missingValue;
    
    _kdp[ii] = _missingValue;
    _kdpZZdr[ii] = _missingValue;
    _kdpSC[ii] = _missingValue;
    _phidpSC[ii] = _missingValue;

    _delta[ii] = _missingValue;
    _deltaMean[ii] = _missingValue;

    _dbzAttenCorr[ii] = 0;
    _zdrAttenCorr[ii] = 0;

    _scBlock[ii] = 0;
    _deltaMean[ii] = 0.0;

  }
  
  double xxDelta = 1.0 / (double) _nGatesPadded;
  for (int ii = 0; ii < _nGatesPadded; ii++) {
    _xxVals[ii] = -0.5 + ii * xxDelta;
  }
  
  
}

/////////////////////////////////////////////
// unfold phidp

int KdpFilt::_unfoldPhidp()

{

  // adjust phidp array so that it folds at 180

  _adjustPhidpBeforeUnfolding(_phidp);

  // TESTING

#ifdef TESTING_FOLDING
  for (int igate = 0; igate < _nGates; igate++) {
    if (_phidp[igate] != _missingValue) {
      _phidp[igate] -= 80.0;
      if (_phidp[igate] < -180) {
        _phidp[igate] += 360.0;
      }
    }
  }
#endif

  // initialize the gate props - the state at each gate is
  // dependent on the phidp values and the spatial relatioship
  // between them

  _gatePropsInit();
  
  // compute mean and standard deviation of phidp
  // and mean angular jitter at each gate
  // also compute zdr sdev

  for (int ii = _nGatesStatsHalf; 
       ii < _nGates - _nGatesStatsHalf; ii++) {
    _computePhidpStats(ii);
    _computeZdrSdev(ii);
    _phidpJitter[ii] = _gateProps[ii].phidpJitter;
    _phidpSdev[ii] = _gateProps[ii].phidpSdev;
    _phidpMean[ii] = _gateProps[ii].phidpMean;
    _phidpMeanFilled[ii] = _gateProps[ii].phidpMean;
  }
  
  // load up runs of valid phidp
  // also identifies the gap runs

  if (_findValidRuns()) {
    _adjustPhidpAfterUnfolding(_phidp);
    _adjustPhidpAfterUnfolding(_phidpMean);
    _adjustPhidpAfterUnfolding(_phidpMeanFilled);
    std::copy(_phidp.begin(), _phidp.end(), _phidpUnfold.begin());
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
      _phidpMeanFilled[jj] = _phidpMeanFilled[startGap-1];
      _gateProps[jj] = _gateProps[startGap-1];
    }
    // fill in last half of gap
    for (int jj = midGap; jj <= endGap; jj++) {
      _phidpMeanFilled[jj] = _phidpMeanFilled[endGap+1];
      _gateProps[jj] = _gateProps[endGap+1];
    }
  }

  // unfold the valid mean field

  int sumFold = 0;
  for (int ii = _firstValidGate; ii <= _lastValidGate; ii++) {
    int fold = 0;
    GateProps &propsPrev = _gateProps[ii-1];
    GateProps &propsThis = _gateProps[ii];
    if (propsPrev.meanxx < 0 && propsThis.meanxx < 0) {
      if (propsPrev.meanyy < 0 && propsThis.meanyy > 0) {
        fold = -1;
      } else if (propsPrev.meanyy > 0 && propsThis.meanyy < 0) {
        fold = 1;
      }
    }
    sumFold += fold;
    if (_phidpMeanFilled[ii] == _missingValue) {
      _phidpUnfold[ii] = _missingValue;
    } else {
      _phidpUnfold[ii] = _phidpMeanFilled[ii] + (sumFold * _foldRange);
    }

  } // ii

  // interpolate unfolded mean through the gaps

  for (int igate = 0; igate < _nGates; igate++) {
    _phidpUnfoldFilled[igate] = _phidpUnfold[igate];
  }

  for (size_t irun = 0; irun < _gapRuns.size(); irun++) {
    int startGap = _gapRuns[irun].ibegin;
    int endGap = _gapRuns[irun].iend;
    double valBefore = _phidpUnfold[startGap-1];
    double valAfter = _phidpUnfold[endGap+1];
    double range = valAfter - valBefore;
    double npts = endGap - startGap + 1;
    double delta = range / npts;
    double val = valBefore + delta;
    for (int jj = startGap; jj <= endGap; jj++, val += delta) {
      _phidpUnfoldFilled[jj] = val;
    }
  }

  // data before the first valid gate and after the last valid gate

  for (int ii = 0; ii < _firstValidGate; ii++) {
    _phidpUnfoldFilled[ii] = _phidpUnfold[_firstValidGate];
  }

  for (int ii = _lastValidGate + 1; ii < _nGates; ii++) {
    _phidpUnfoldFilled[ii] = _phidpUnfold[_lastValidGate];
  }
  
  // before and after the data, set to the mean

  double sumAtStart = 0.0;
  for (int ii = 0; ii < _nGatesStats; ii++) {
    sumAtStart += _phidpUnfold[ii + _firstValidGate];
  }
  double meanAtStart = sumAtStart / _nGatesStats; 

  double sumAtEnd = 0.0;
  for (int ii = 0; ii < _nGatesStats; ii++) {
    sumAtEnd += _phidpUnfold[_lastValidGate - ii];
  }
  double meanAtEnd = sumAtEnd / _nGatesStats; 

  for (int ii = 0; ii < _firstValidGate; ii++) {
    _phidpUnfoldFilled[ii] = meanAtStart;
  }
  for (int ii = _lastValidGate + 1; ii < _nGates; ii++) {
    _phidpUnfoldFilled[ii] = meanAtEnd;
  }

  // adjust phidp arrays back to original range
  
  _adjustPhidpAfterUnfolding(_phidp);
  _adjustPhidpAfterUnfolding(_phidpMean);
  _adjustPhidpAfterUnfolding(_phidpMeanFilled);
  _adjustPhidpAfterUnfolding(_phidpUnfold);
  _adjustPhidpAfterUnfolding(_phidpUnfoldFilled);

  return 0;

}
    
/////////////////////////////////////////////
// filter the unfolded PHIDP

void KdpFilt::_filterPhidp()
  
{
  
  // apply FIR filter to unfolded phidp

  if (_params.phidp_filter_method == KdpFiltParams::FIR_FILTER ||
      _params.KDP_compute_all_filters) {
    _applyFirFilter();
  }
  
  // apply quadratic filter to phidp unfolded

  if (_params.phidp_filter_method == KdpFiltParams::QUADRATIC_FILTER ||
      _params.KDP_compute_all_filters) {
    _applyQuadFilter();
  }
  
  // apply fft filter to phidp unfolded

  if (_params.phidp_filter_method == KdpFiltParams::FFT_FILTER ||
      _params.KDP_compute_all_filters) {
    _applyFftFilter();
  }
  
  // compute phidp filtered with regression filter

  if (_params.phidp_filter_method == KdpFiltParams::REGRESSION_FILTER ||
      _params.KDP_compute_all_filters) {
    _applyRegrFilter();
  }

  // copy the relevant filtered data into _phidpFilt
  
  if (_params.phidp_filter_method == KdpFiltParams::FIR_FILTER) {
    std::copy(_phidpFirFilt.begin(), _phidpFirFilt.end(), _phidpFilt.begin());
  } else if (_params.phidp_filter_method == KdpFiltParams::QUADRATIC_FILTER) {
    std::copy(_phidpQuadFilt.begin(), _phidpQuadFilt.end(), _phidpFilt.begin());
  } else if (_params.phidp_filter_method == KdpFiltParams::FFT_FILTER) {
    std::copy(_phidpFftFilt.begin(), _phidpFftFilt.end(), _phidpFilt.begin());
  } else if (_params.phidp_filter_method == KdpFiltParams::REGRESSION_FILTER) {
    std::copy(_phidpRegrFilt.begin(), _phidpRegrFilt.end(), _phidpFilt.begin());
  }

}

/////////////////////////////////////////////
// Compute KDP

void KdpFilt::_computeKdp()

{

  // compute kdp
  
  for (int ii = 1; ii < _nGates - 1; ii++) {
    
    // check validity
    
    if (!_validForKdp[ii]) {
      _kdp[ii] = 0.0;
      _kdpZZdr[ii] = 0.0;
      _kdpSC[ii] = 0.0;
      _delta[ii] = 0.0;
      _deltaMean[ii] = 0.0;
      continue;
    }

    // compute the slope between each adjacent point
    
    int i0 = ii - 1;
    int i1 = ii + 1;
    int len = i1 - i0;
    if (_phidpFilt[i0] != _missingValue &&
        _phidpFilt[i1] != _missingValue) {
      double dphi = _phidpFilt[i1] - _phidpFilt[i0];
      _kdp[ii] = (dphi / (_gateSpacingKm * len)) / 2.0;
      // if (_foldsAt90) {
      //   _kdp[ii] /= 2.0;
      // }
      _kdpZZdr[ii] = _computeKdpFromZZdr(_dbzMedian[ii], _zdrMedian[ii]);
    } else {
      _kdp[ii] = _missingValue;
      _kdpZZdr[ii] = _missingValue;
    }

  } // ii

}

////////////////////////////////////////////////
// compute attenuation corrections based on KDP

void KdpFilt::_computeAttenCorrection()
  
{

  // sum up corrections

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
  // _foldRange = _foldVal * 2.0;
  _foldRange = 360.0;
  
  // if values range from (0 -> 360), normalize to (-180 -> 180)
  
  if (phidpMin >= 0 && phidpMax > 180) {
    for (int igate = 0; igate < _nGates; igate++) {
      if (_phidp[igate] != _missingValue) {
        _phidp[igate] -= 180.0;
      }
    }
  }

}

//////////////////////////////////////////////////////////////////////////
// adjust input phidp for folding range
// this multiplies phidp by 2 for alternating mode radars
// for which phidp folds at -90 and +90

void KdpFilt::_adjustPhidpBeforeUnfolding(vector<double> &phidp)
{

  // adjust phidp array so that it folds at 180
  
  if (_foldsAt90) {
    for (int igate = 0; igate < _nGates; igate++) {
      if (phidp[igate] != _missingValue) {
        phidp[igate] = phidp[igate] * 2.0;
      }
    }
  }

}

//////////////////////////////////////////////////////////////////////////
// adjust output unfolded values for folding range
// this multiplies phidp by 0.5 for alternating mode radars
// for which phidp folds at -90 and +90

void KdpFilt::_adjustPhidpAfterUnfolding(vector<double> &phidp)
{

  // adjust phidp array so that it folds at 180
  
  if (_foldsAt90) {
    for (int igate = 0; igate < _nGates; igate++) {
      if (phidp[igate] != _missingValue) {
        phidp[igate] = phidp[igate] * 0.5;
      }
    }
  }

}

//////////////////////////////////////////////////////////////////////////
// load up runs of 'valid' phidp
//
// returns -1 if no valid runs found, 0 otherwise

int KdpFilt::_findValidRuns()
{

  _validRuns.clear();
  _gapRuns.clear();
  
  // first pass - load up all runs

  vector<PhidpRun> allRuns;
  int runLen = 0;
  int minValidRunLen = (int) (_params.phidp_feature_length_km / _gateSpacingKm) * 2 + 1;
  for (int igate = 0; igate < _nGates; igate++) {

    bool validGate = _isGateValid(igate);

    if (validGate) {
      runLen++;
    }

    // save runs longer than _params.phidp_feature_length_km
    
    if (!validGate) {
      if (runLen >= minValidRunLen) {
        int iend = igate - 1;
        int ibegin = iend - runLen + 1;
        PhidpRun run(ibegin, iend);
        allRuns.push_back(run);
      }
      runLen = 0;
    } else if (igate == _nGates - 1) {
      // last gate in ray
      if (runLen >= minValidRunLen) {
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

  _validRuns = combRuns;

  if (_validRuns.size() < 1) {
    // no valid runs
    return -1;
  }

  // set valid flags
  
  for (size_t irun = 0; irun < _validRuns.size(); irun++) {
    const PhidpRun &validRun = _validRuns[irun];
    for (int igate = validRun.ibegin; igate <= validRun.iend; igate++) {
      _validForKdp[igate] = true;
    }
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

#ifdef NOTNOW

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
#endif

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
  
  if (_params.KDP_check_snr && _snrAvailable) {
    if ((_snr[igate] == _missingValue) || (_snr[igate] < _params.KDP_snr_threshold)) {
      return false;
    }
  }

  // check for clutter effects

  if (_phidpSdev[igate] > _params.KDP_phidp_sdev_max) {
    return false;
  }
  if (_phidpJitter[igate] > _params.KDP_phidp_jitter_max) {
    return false;
  }
  if (_params.KDP_check_zdr_sdev) {
    if (_zdrSdev[igate] > _params.KDP_zdr_sdev_max) {
      return false;
    }
  }
  if (_params.KDP_check_rhohv) {
    if ((_rhohv[igate] != _missingValue) && (_rhohv[igate] < _params.KDP_rhohv_threshold)) {
      return false;
    }
  }

  return true;

}

//////////////////////////////////////////////////////////////////////////
// Initialize the props at each gate
 
void KdpFilt::_gatePropsInit()

{

  // init (x,y) representation of phidp

  for (int ii = 0; ii < _nGates; ii++) {
    GateProps &props = _gateProps[ii];
    props.init(_missingValue);
    if (_phidp[ii] != _missingValue) {
      props.missing = false;
      double phase = _phidp[ii];
      props.phidp = _phidp[ii];
      double sinVal, cosVal;
      ta_sincos(phase * DEG_TO_RAD, &sinVal, &cosVal);
      props.xx = cosVal;
      props.yy = sinVal;
    }
  }

  // init dist between phidp at successive gates

  for (int ii = 1; ii < _nGates; ii++) {
    GateProps &iprops0 = _gateProps[ii-1];
    GateProps &iprops1 = _gateProps[ii];
    if (!iprops0.missing && !iprops1.missing) {
      double xx0 = iprops0.xx;
      double yy0 = iprops0.yy;
      double xx1 = iprops1.xx;
      double yy1 = iprops1.yy;
      double dx = xx1 - xx0;
      double dy = yy1 - yy0;
      double dist = sqrt(dx * dx + dy * dy);
      _gateProps[ii].distFromPrev = dist;
    }
  }
  
}
  
//////////////////////////////////////////////////////////////////////////
//  To calculate the mean phidp, standard deviation, and jitter
//  in phidp at a gate, using stats on the circle

void KdpFilt::_computePhidpStats(int igate)
  
{

  GateProps &iprops = _gateProps[igate];
  
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
    GateProps &jprops = _gateProps[jj];
    if (jprops.missing) {
      continue;
    }
    double xx = jprops.xx;
    double yy = jprops.yy;
    double dist = jprops.distFromPrev;
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
  
  iprops.meanxx = sumxx / count;
  iprops.meanyy = sumyy / count;
  
  double phaseMean = atan2(iprops.meanyy, iprops.meanxx) * RAD_TO_DEG;
  iprops.phidpMean = phaseMean;
  
  // jitter
  
  double meanDist = sumDist / count;
  double meanAngChangePerGate = meanDist * RAD_TO_DEG;
  iprops.phidpJitter = meanAngChangePerGate;
  
  // sdev of distance moved, is a proxy for sdev of phidp
  
  if (count > 2) {
    double term1 = sumDistSq / count;
    double term2 = meanDist * meanDist;
    if (term1 >= term2) {
      double sdev = sqrt(term1 - term2) * RAD_TO_DEG;
      iprops.phidpSdev = sdev;
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

////////////////////////////////////////////////////////////
// Compute estimated kdp from Z and ZDR using power law

double KdpFilt::_computeKdpFromZZdr(double dbz,
                                    double zdr)
  
{

  if (dbz == _missingValue ||
      zdr == _missingValue) {
    return 0.0;
  }

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

  // compute trend of filt
  
  _phidpFiltTrend[0] = 0.0;
  for (int kk = 1; kk < _nGates; ++kk) {
    _phidpFiltTrend[kk] = _phidpFilt[kk] - _phidpFilt[kk-1];
  }

  // copy KDP array to KDP SC
  
  std::copy(_kdp.begin(), _kdp.end(), _kdpSC.begin());

  // loop through the valid runs
  
  vector<PhidpRun> deltaRuns;
    
  for (size_t irun = 0; irun < _validRuns.size(); irun++) {
    
    const PhidpRun &validRun = _validRuns[irun];

    int ibegin = validRun.ibegin;
    int iend = ibegin;

    while (ibegin <= validRun.iend) {

      // look for block starting with a positive trend, going negative
      // and returning to a positive

      int index = ibegin;
      while (_phidpFiltTrend[index] >= 0.0 && index < validRun.iend) {
        index++;
      }
      while (_phidpFiltTrend[index] < 0.0 && index < validRun.iend) {
        index++;
      }
      iend = index;
      
      if (iend - ibegin > _nGatesStats) {
        _loadKdpSCRun(ibegin, iend);
      }

      deltaRuns.push_back(PhidpRun(ibegin, iend));

      ibegin = iend + 1;

    } // while (ibegin ...
      
  } // irun

  // moving mean on _kdpSC
  
  vector<double> meanKdpSC;
  _movingMean(_kdpSC, _nGatesStats, meanKdpSC);
  std::copy(meanKdpSC.begin(), meanKdpSC.end(), _kdpSC.begin());

  // compute _phidpSC by integrating _kdpSC, compute delta

  std::copy(_phidpFilt.begin(), _phidpFilt.end(), _phidpSC.begin());
  for (size_t irun = 0; irun < _validRuns.size(); irun++) {
    const PhidpRun &validRun = _validRuns[irun];
    for (int igate = validRun.ibegin + 1; igate <= validRun.iend; igate++) {
      double kdpSC = _kdpSC[igate - 1];
      double deltaPhi = kdpSC * 2 * _gateSpacingKm;
      // if (_foldsAt90) {
      //   deltaPhi *= 2;
      // }
      _phidpSC[igate] = RadarComplex::sumDeg(_phidpSC[igate - 1], deltaPhi);
      // compute phase shift on backscatter as the difference between
      // filtered value and SC phidp
      _delta[igate] = _phidpFilt[igate] - _phidpSC[igate];
    }
  }

  // set conditions on delta
  // compute mean delta for pos values only
  // threshold using _meanDeltaThreshold
  
  for (size_t ii = 0; ii < deltaRuns.size(); ii++) {
    const PhidpRun &run = deltaRuns[ii];
    double deltaSum = 0.0;
    double count = 0.0;
    for (int igate = run.ibegin; igate <= run.iend; igate++) {
      if (_delta[igate] >= 0.0) {
        deltaSum += _delta[igate];
        count++;
      }
    }
    double deltaMean = 0.0;
    if (count > 0) {
      deltaMean = deltaSum / count;
    }
    for (int igate = run.ibegin; igate <= run.iend; igate++) {
      _deltaMean[igate] = deltaMean;
      if (deltaMean < _params.KDP_self_con_mean_delta_threshold || _delta[igate] < 0) {
        _delta[igate] = 0;
      }
    }
  }

}

////////////////////////////////////////////////////////////
/// load up kdp conditioned using ZZDR self-consistency
/// for a specific run

void KdpFilt::_loadKdpSCRun(int startGate, int endGate)

{

  for (int igate = startGate; igate <= endGate; igate++) {
    _scBlock[startGate] = 0.0;
  }
  _scBlock[startGate] = 1.0;
  _scBlock[endGate] = 1.0;

  if (endGate - startGate < 3) {
    // not enough gates for this to make sense
    return;
  }

  // integrate KDP to get phidp over the run

  double sumPhidp = 0.0;
  double sumPhidpZZdr = 0.0;
  
  for (int igate = startGate; igate <= endGate; igate++) {
    sumPhidp += _kdp[igate] * _gateSpacingKm * 2;
    sumPhidpZZdr += _kdpZZdr[igate] * _gateSpacingKm * 2;
  } // igate

  if (sumPhidpZZdr < 1.0) {
    return;
  }
  // if (_foldsAt90) {
  //   sumPhidp *= 2.0;
  //   sumPhidpZZdr *= 2.0;
  // }
  
  // compute factor to normalize the ZZdr estimate
  // from the measured estimate
  
  double condFactor = sumPhidp / sumPhidpZZdr;

  // load the KDP conditioned by self-consistency

  for (int igate = startGate; igate <= endGate; igate++) {
    _kdpSC[igate] = _kdpZZdr[igate] * condFactor;
  }

}

////////////////////////////////////////////////////////////
/// filter phidp using FFT

void KdpFilt::_applyFftFilter()

{

  // create complex array for phidp
  // pad out to avoid ringing at extremities
  
  vector<RadarComplex_t> phiComplex_;
  phiComplex_.resize(_nGatesPadded);
  RadarComplex_t *phiComplex = phiComplex_.data() + _nGatesPad;
  for (int igate = 0; igate < _nGates; igate++) {
    RadarComplex::setFromDegrees(_phidpUnfoldFilled[igate], phiComplex[igate]);
  }
  
  // interpolate between end-points for the padded gates
  
  RadarComplex_t angleStart = phiComplex[0];
  RadarComplex_t angleEnd = phiComplex[_nGates - 1];
  vector<RadarComplex_t> interpVec;
  RadarComplex::interpAndLoadVec(angleStart, angleEnd, _nGatesPad * 2, interpVec);
  for (int ii = 0; ii < _nGatesPad; ii++) {
    phiComplex[-1 - ii] = interpVec[ii];
    phiComplex[_nGates + ii] = interpVec[_nGatesPad * 2 - 1 - ii];
  }

  // perform forward FFT
  
  vector<RadarComplex_t> phiSpec_;
  phiSpec_.resize(_nGatesPadded);
  _fft.init(_nGatesPadded);
  _fft.fwd(phiComplex_.data(), phiSpec_.data());
  
  // determine cutoff
  
  const double f_cut = 1.0 / _params.phidp_feature_length_km;  // cycles/km

  // apply filter
  
  for (int kk = 0; kk < _nGatesPadded; ++kk) {
    // FFT bin interpreted as signed frequency index
    int kk_signed = (kk <= _nGatesPadded / 2) ? kk : kk - _nGatesPadded;
    double f = std::abs(kk_signed) / (_nGatesPadded * _gateSpacingKm);  // cycles/km
    if (f > f_cut) {
      phiSpec_[kk].re = 0.0;
      phiSpec_[kk].im = 0.0;
    }
  }
  
  // perform inverse FFT
  
  _fft.inv(phiSpec_.data(), phiComplex_.data());

  // compute the filtered PHIDP

  for (int kk = 0; kk < _nGates; ++kk) {
    _phidpFftFilt[kk] = RadarComplex::argDeg(phiComplex[kk]);
  }

}

////////////////////////////////////////////////////////////
/// filter phidp using fir

void KdpFilt::_applyFirFilter()
  
{
  
  _firFilt.setFeatureLength(_params.phidp_feature_length_km,
                            _gateSpacingKm);
  _firFilt.applyFilter(_phidpUnfoldFilled, _phidpFirFilt,
                       _params.fir_n_iterations, _missingValue);

}
    
////////////////////////////////////////////////////////////
/// filter phidp using quadratic fit

void KdpFilt::_applyQuadFilter()

{

  // perform the quadratic fit

  int nFeatureHalf = ((int) (_params.phidp_feature_length_km / _gateSpacingKm) + 1) / 2;

  if (_quadFilt.compute(_phidpUnfoldFilled,
                        _gateSpacingKm,
                        nFeatureHalf,
                        _missingValue) == 0) {
    _phidpQuadFilt = _quadFilt.getPhidpFitDeg();
    _kdpQuadFilt = _quadFilt.getKdpDegPerKm();
  }

}

////////////////////////////////////////////////////////////
/// filter phidp using regression filter

void KdpFilt::_applyRegrFilter()

{

  for (size_t irun = 0; irun < _validRuns.size(); irun++) {
    _applyPhidpRegrFilt(irun);
  }

}
  
/////////////////////////////////////////////////
// compute phidp filtered with regression filter
// for specified valid run
// i.e. not global

void KdpFilt::_applyPhidpRegrFilt(int runNum)

{

  // perform regression for valid gates, plus a pad

  PhidpRun &run = _validRuns[runNum];
  int startGate = run.ibegin - _nGatesPad;
  if (startGate < 0) {
    startGate = 0;
  }
  int endGate = run.iend + _nGatesPad + 1;
  if (endGate > _nGates - 1) {
    endGate = _nGates - 1;
  }
  int nGatesFit = endGate - startGate;

  // compute regression order to be used

  double deltaRangeKm = nGatesFit * _gateSpacingKm;
  int polyOrder = floor(deltaRangeKm / _params.phidp_feature_length_km) * 3 + 1;
  if (polyOrder < 5) {
    polyOrder = 5;
  }
  
  // prepare for the fit
  
  ForsytheFit fit;
  vector<double> xx;
  double xxDelta = 1.0 / (double) nGatesFit;
  for (int ii = 0; ii < nGatesFit; ii++) {
    xx.push_back(-0.5 + ii * xxDelta);
  }
  fit.prepareForFit(polyOrder, xx);
  
  // perform the polynomial fit on unfolded phidp
  
  vector<double> phiRegr_;
  phiRegr_.resize(nGatesFit);
  double *phiRegr = phiRegr_.data();
  for (int igate = 0; igate < nGatesFit; igate++) {
    phiRegr[igate] = _phidpUnfoldFilled[igate + startGate];
  }
  
  fit.performFit(phiRegr_);
  vector<double> smoothed = fit.getYEstVector();
  for (int ii = 0; ii < nGatesFit; ii++) {
    _phidpRegrFilt[ii + startGate] = smoothed[ii];
  }
  
}

/////////////////////////////////////////////////
// compute phidp filtered with regression filter
// globally - i.e. full ray

void KdpFilt::_applyPhidpRegrFiltGlobal()

{

  // compute regression order to be used
  
  double deltaRangeKm = _nGatesPadded * _gateSpacingKm;
  int polyOrder = floor(deltaRangeKm / _params.phidp_feature_length_km) * 2 + 1;
  if (polyOrder < 5) {
    polyOrder = 5;
  }
  
  // prepare for the fit
  
  ForsytheFit fit;
  fit.prepareForFit(polyOrder, _xxVals);
  
  // perform the polynomial fit on unfolded phidp

  vector<double> phiRegr_;
  phiRegr_.resize(_nGatesPadded);
  double *phiRegr = phiRegr_.data() + _nGatesPad;
  for (int igate = 0; igate < _nGates; igate++) {
    phiRegr[igate] = _phidpUnfoldFilled[igate];
  }
  for (int igate = 0; igate < _nGatesPad; igate++) {
    phiRegr[-1 - igate] = phiRegr[0];
    phiRegr[_nGates + igate] = phiRegr[_nGates - 1];
  }
  
  fit.performFit(phiRegr_);
  vector<double> smoothed = fit.getYEstVector();
  for (int ii = 0; ii < _nGates; ii++) {
    if (_validForKdp[ii]) {
      _phidpRegrFilt[ii] = smoothed[ii + _nGatesPad];
      if (fabs(_phidpRegrFilt[ii]) > 720.0) {
        _phidpRegrFilt[ii] = _missingValue;
      }
    } else {
      _phidpRegrFilt[ii] = _missingValue;
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
      _phidp[igate] = (dist(gen) - 0.5) * 180.0;
    }
  }

}

////////////////////////////////////////////////////////////
/// censor non valid kdp results

void KdpFilt::_censorNonValidKdp()

{

  for (int kk = 0; kk < _nGates; ++kk) {
    if (!_validForKdp[kk]) {
      _phidpFilt[kk] = _missingValue;
      _phidpFiltTrend[kk] = _missingValue;
      _phidpSC[kk] = _missingValue;
      _kdp[kk] = _missingValue;
      _kdpSC[kk] = _missingValue;
      _delta[kk] = _missingValue;
      _deltaMean[kk] = _missingValue;
    }
    if (fabs(_kdp[kk]) < 0.1) {
      _kdp[kk] = 0;
    }
    if (fabs(_kdpSC[kk]) < 0.1) {
      _kdpSC[kk] = 0;
    }
  }

}
  
////////////////////////////////////////////////////////////
/// get quality based on rhohv

double KdpFilt::_rhohvQuality(double rhohv)
{

  constexpr double rho0 = 0.90;
  constexpr double rho1 = 0.98;
  constexpr double smallVal = 0.00001;
  

  if (!std::isfinite(rhohv) || rhohv == _missingValue || rhohv < rho0) {
    return smallVal;
  }
  if (rhohv >= rho1) return 1.0;
  
  const double x = (rhohv - rho0) / (rho1 - rho0);
  
  // Smoothstep: zero slope at both ends
  return x * x * (3.0 - 2.0 * x);
  
}

////////////////////////////////////////////////////////////
/// moving mean along a vector

void KdpFilt::_movingMean(const std::vector<double>& xx,
                          size_t filtLen,
                          std::vector<double>& filt)
  
{

  filt.resize(xx.size());
  
  if (filtLen % 2 == 0) {
    cerr << "WARNING - KdpFilt::_movingMean" << endl;
    cerr << "  filtLen should be odd, passed in: " << filtLen << endl;
    filtLen = (filtLen / 2) * 2;
    cerr << "  will use filtLen: " << filtLen << endl;
  }
  
  size_t half = filtLen / 2;
  
  // Running sum
  double sum = 0.0;
  for (size_t i = 0; i < filtLen; ++i)
    sum += xx[i];
  
  filt[half] = sum / filtLen;
  
  for (size_t i = half + 1; i < xx.size() - half; ++i) {
    sum += xx[i + half];
    sum -= xx[i - half - 1];
    filt[i] = sum / filtLen;
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
          "# gateNum validKdp "
          "snr dbz zdr rhohv phidp "
          "phidpMean phidpMeanFilled phidpJitter phidpSdev zdrSdev "
          "phidpUnfold phidpUnfoldFilled phidpFilt phidpFiltTrend phidpSC "
          "phidpFirFilt phidpQuadFilt kdpQuadFilt phidpFftFilt phidpRegrFilt "
          "delta deltaMean kdp kdpSC kdpZZdr "
          "scBlock \n");

  // write data

  for (int igate = 0; igate < _nGates; igate++) {
    fprintf(out,
            "%3d %3d "
            "%10.3f %10.3f %10.3f %10.3f %10.3f "
            "%10.3f %10.3f %10.3f %10.3f %10.3f "
            "%10.3f %10.3f %10.3f %10.3f %10.3f "
            "%10.3f %10.3f %10.3f %10.3f %10.3f "
            "%10.3f %10.3f %10.3f %10.3f %10.3f "
            "%10.3f\n",
            igate,
            (_validForKdp[igate]?1:0),
            _getPlotVal(_snr[igate], NAN),
            _getPlotVal(_dbz[igate], NAN),
            _getPlotVal(_zdr[igate], NAN),
            _getPlotVal(_rhohv[igate], NAN),
            _getPlotVal(_phidp[igate], NAN),
            _getPlotVal(_phidpMean[igate], NAN),
            _getPlotVal(_phidpMeanFilled[igate], NAN),
            _getPlotVal(_phidpJitter[igate], NAN),
            _getPlotVal(_phidpSdev[igate], NAN),
            _getPlotVal(_zdrSdev[igate], NAN),
            _getPlotVal(_phidpUnfold[igate], NAN),
            _getPlotVal(_phidpUnfoldFilled[igate], NAN),
            _getPlotVal(_phidpFilt[igate], NAN),
            _getPlotVal(_phidpFiltTrend[igate], NAN),
            _getPlotVal(_phidpSC[igate], NAN),
            _getPlotVal(_phidpFirFilt[igate], NAN),
            _getPlotVal(_phidpQuadFilt[igate], NAN),
            _getPlotVal(_kdpQuadFilt[igate], NAN),
            _getPlotVal(_phidpFftFilt[igate], NAN),
            _getPlotVal(_phidpRegrFilt[igate], NAN),
            _getPlotVal(_delta[igate], 0),
            _getPlotVal(_deltaMean[igate], 0),
            _getPlotVal(_kdp[igate], 0),
            _getPlotVal(_kdpSC[igate], 0),
            _getPlotVal(_kdpZZdr[igate], 0),
            _getPlotVal(_scBlock[igate], 0)
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

