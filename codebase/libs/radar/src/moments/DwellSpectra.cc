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
// DwellSpectra.cc
//
// DwellSpectra object
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2023
//
///////////////////////////////////////////////////////////////
//
// DwellSpectra object holds time series for a dwell, and
// computes spectra, SpectralCmd etc.
//
////////////////////////////////////////////////////////////////

#include <cassert>
#include <iostream>
#include <algorithm>
#include <toolsa/toolsa_macros.h>
#include <toolsa/DateTime.hh>
#include <toolsa/sincos.h>
#include <radar/FilterUtils.hh>
#include <radar/ClutFilter.hh>
#include <radar/DwellSpectra.hh>
using namespace std;

const double DwellSpectra::_missingDbl = MomentsFields::missingDouble;
pthread_mutex_t DwellSpectra::_fftMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t DwellSpectra::_debugPrintMutex = PTHREAD_MUTEX_INITIALIZER;

////////////////////////////////////////////////////
// Constructor

DwellSpectra::DwellSpectra()
        
{

  _nSamples = 0;
  _nGates = 0;
  
  _timeSecs = 0;
  _nanoSecs = 0;
  _dtime = 0.0;

  _el = 0.0;
  _az = 0.0;

  _antennaRate = 0.0;

  _startRangeKm = 0.0;
  _gateSpacingKm = 0.0;

  _xmitRcvMode = IWRF_SINGLE_POL;
  
  _prt = 0.0;
  _nyquist = 0.0;
  _pulseWidthUs = 0.0;
  _wavelengthM = 0.0;

  _tdbzKernelNGates = 5;
  _tdbzKernelNSamples = 3;
  _sdevZdrKernelNGates = 5;
  _sdevZdrKernelNSamples = 3;
  _sdevPhidpKernelNGates = 5;
  _sdevPhidpKernelNSamples = 3;

  _interestMapSnr = NULL;
  _interestMapTdbz = NULL;
  _interestMapSdevZdr = NULL;
  _interestMapSdevPhidp = NULL;

  _createDefaultInterestMaps();

  _windowType = RadarMoments::WINDOW_RECT;
  _clutterFilterType = RadarMoments::CLUTTER_FILTER_NONE;
  _regrOrder = -1;
  _regrClutWidthFactor = 1.0;
  _regrCnrExponent = 0.66667;
  _regrNotchInterpMethod = RadarMoments::INTERP_METHOD_GAUSSIAN;
  
  resetFlags();

}

//////////////////////////////////////////////////////////////////
// destructor

DwellSpectra::~DwellSpectra()

{

  _freeArrays();

}

//////////////////////////////////////////////////////////////////
// set dimensions

void DwellSpectra::setDimensions(size_t nGates, size_t nSamples)

{

  resetFlags();

  if (nGates == _nGates && nSamples == _nSamples) {
    // nothing to do
    return;
  }

  // allocate the arrays
  
  _allocArrays(nGates, nSamples);

  // save dims
  
  _nGates = nGates;
  _nSamples = nSamples;
  
  _fft.init(nSamples);

}

/////////////////////////////////////////////////////////////////
// Allocate or re-allocate arrays

void DwellSpectra::_allocArrays(size_t nGates, size_t nSamples)

{
  
  // free up existing arrays

  _freeArrays();

  // allocate arrays

  _window1D.alloc(nSamples);
  for (size_t ii = 0; ii < nSamples; ii++) {
    _window1D.dat()[ii] = 1.0;
  }
  
  _specNoiseHc1D.alloc(nGates);
  _meanCmd1D.alloc(nGates);
  _fractionCmd1D.alloc(nGates);

  _iqHc2D.alloc(nGates, nSamples);
  _iqVc2D.alloc(nGates, nSamples);
  _iqHx2D.alloc(nGates, nSamples);
  _iqVx2D.alloc(nGates, nSamples);
  
  _iqHcFilt2D.alloc(nGates, nSamples);
  _iqVcFilt2D.alloc(nGates, nSamples);

  _specCompHc2D.alloc(nGates, nSamples);
  _specCompVc2D.alloc(nGates, nSamples);
  _specCompHx2D.alloc(nGates, nSamples);
  _specCompVx2D.alloc(nGates, nSamples);

  _specCompHcFilt2D.alloc(nGates, nSamples);
  _specCompVcFilt2D.alloc(nGates, nSamples);

  _specPowerHc2D.alloc(nGates, nSamples);
  _specPowerVc2D.alloc(nGates, nSamples);
  _specPowerHx2D.alloc(nGates, nSamples);
  _specPowerVx2D.alloc(nGates, nSamples);

  _specDbmHc2D.alloc(nGates, nSamples);
  _specDbmVc2D.alloc(nGates, nSamples);
  _specDbmHx2D.alloc(nGates, nSamples);
  _specDbmVx2D.alloc(nGates, nSamples);

  _specDbz2D.alloc(nGates, nSamples);
  _specSnr2D.alloc(nGates, nSamples);
  _specZdr2D.alloc(nGates, nSamples);
  _specPhidp2D.alloc(nGates, nSamples);
  _specRhohv2D.alloc(nGates, nSamples);

  _specTdbz2D.alloc(nGates, nSamples);
  _specSdevZdr2D.alloc(nGates, nSamples);
  _specSdevPhidp2D.alloc(nGates, nSamples);

  _specSnrInterest2D.alloc(nGates, nSamples);
  _specTdbzInterest2D.alloc(nGates, nSamples);
  _specSdevZdrInterest2D.alloc(nGates, nSamples);
  _specSdevPhidpInterest2D.alloc(nGates, nSamples);

  _specCmd2D.alloc(nGates, nSamples);

}

//////////////////////////////////////////////////////////////////
// free up arrays

void DwellSpectra::_freeArrays()

{

  _window1D.free();
  _specNoiseHc1D.free();
  _meanCmd1D.free();
  _fractionCmd1D.free();
  
  _iqHc2D.free();
  _iqVc2D.free();
  _iqHx2D.free();
  _iqVx2D.free();

  _iqHcFilt2D.free();
  _iqVcFilt2D.free();

  _specCompHc2D.free();
  _specCompVc2D.free();
  _specCompHx2D.free();
  _specCompVx2D.free();

  _specCompHcFilt2D.free();
  _specCompVcFilt2D.free();

  _specPowerHc2D.free();
  _specPowerVc2D.free();
  _specPowerHx2D.free();
  _specPowerVx2D.free();

  _specDbmHc2D.free();
  _specDbmVc2D.free();
  _specDbmHx2D.free();
  _specDbmVx2D.free();

  _specDbz2D.free();
  _specSnr2D.free();
  _specZdr2D.free();
  _specPhidp2D.free();
  _specRhohv2D.free();

  _specTdbz2D.free();
  _specSdevZdr2D.free();
  _specSdevPhidp2D.free();

  _specSnrInterest2D.free();
  _specTdbzInterest2D.free();
  _specSdevZdrInterest2D.free();
  _specSdevPhidpInterest2D.free();

  _specCmd2D.free();

}
  
//////////////////////////////////////////////////////////////////
// reset the availability flags

void DwellSpectra::resetFlags()

{

  _hcAvail = false;
  _vcAvail = false;
  _hxAvail = false;
  _vxAvail = false;

}

///////////////////////////////////////////////////////////////
// set IQ arrays

void DwellSpectra::setIqHc(const RadarComplex_t *iqHc,
                           size_t gateNum, size_t nSamples)
  
{
  assert(gateNum < _nGates);
  assert(nSamples == _nSamples);
  memcpy(_iqHc2D.dat2D()[gateNum], iqHc, nSamples * sizeof(RadarComplex_t));
  _hcAvail = true;
}
  
void DwellSpectra::setIqVc(const RadarComplex_t *iqVc,
                           size_t gateNum, size_t nSamples)
  
{
  assert(gateNum < _nGates);
  assert(nSamples == _nSamples);
  memcpy(_iqVc2D.dat2D()[gateNum], iqVc, nSamples * sizeof(RadarComplex_t));
  _vcAvail = true;
}
  
void DwellSpectra::setIqHx(const RadarComplex_t *iqHx,
                           size_t gateNum, size_t nSamples)
  
{
  assert(gateNum < _nGates);
  assert(nSamples == _nSamples);
  memcpy(_iqHx2D.dat2D()[gateNum], iqHx, nSamples * sizeof(RadarComplex_t));
  _hxAvail = true;
}
  
void DwellSpectra::setIqVx(const RadarComplex_t *iqVx,
                           size_t gateNum, size_t nSamples)
  
{
  assert(gateNum < _nGates);
  assert(nSamples == _nSamples);
  memcpy(_iqVx2D.dat2D()[gateNum], iqVx, nSamples * sizeof(RadarComplex_t));
  _vxAvail = true;
}
  
////////////////////////////////////////////////////
// Compute spectra

void DwellSpectra::computePowerSpectra()
  
{

  // initialize
  
  RadarMoments::initWindow(_windowType, _nSamples, _window1D.dat());
  
  RadarMoments moments;
  moments.setNSamples(_nSamples);
  moments.init(_prt, _wavelengthM, _startRangeKm, _gateSpacingKm);
  moments.setCalib(_calib);
  moments.setClutterWidthMps(0.75);
  moments.setClutterInitNotchWidthMps(3.0);
  moments.setAntennaRate(_antennaRate);
  
  if (_hcAvail) {
    _computePowerSpectra(_iqHc2D,
                         pow(10.0, _calib.getNoiseDbmHc() / 10.0),
                         moments,
                         _specCompHc2D, _specPowerHc2D, _specDbmHc2D);
  }
      
  if (_vcAvail) {
    _computePowerSpectra(_iqVc2D,
                         pow(10.0, _calib.getNoiseDbmVc() / 10.0),
                         moments,
                         _specCompVc2D, _specPowerVc2D, _specDbmVc2D);
  }
      
  if (_hxAvail) {
    _computePowerSpectra(_iqHx2D,
                         pow(10.0, _calib.getNoiseDbmHx() / 10.0),
                         moments,
                         _specCompHx2D, _specPowerHx2D, _specDbmHx2D);
  }
  
  if (_vxAvail) {
    _computePowerSpectra(_iqVx2D,
                         pow(10.0, _calib.getNoiseDbmVx() / 10.0),
                         moments,
                         _specCompVx2D, _specPowerVx2D, _specDbmVx2D);
  }

}

void DwellSpectra::_computePowerSpectra(TaArray2D<RadarComplex_t> &iq2D,
                                        double calibNoise,
                                        RadarMoments &moments,
                                        TaArray2D<RadarComplex_t> &specComp2D,
                                        TaArray2D<double> &specPower2D,
                                        TaArray2D<double> &specDbm2D)
  
{

  // allocate arrays and variables
  
  TaArray<RadarComplex_t> iqWindowed_;
  RadarComplex_t *iqWindowed = iqWindowed_.alloc(_nSamples);
  
  TaArray<RadarComplex_t> iqFilt_;
  RadarComplex_t *iqFilt = iqFilt_.alloc(_nSamples);

  double filterRatio, spectralNoise, spectralSnr;
  ClutFilter clutFilt;

  bool orderAuto = true;
  if (_regrOrder > 2) {
    orderAuto = false;
  }
  ForsytheRegrFilter regrF;
  regrF.setup(_nSamples, orderAuto, _regrOrder,
              _regrClutWidthFactor, _regrCnrExponent,
              _wavelengthM);
      
  for (size_t igate = 0; igate < _nGates; igate++) {

    // filter the time series
    
    memcpy(iqWindowed, iq2D.dat2D()[igate], _nSamples * sizeof(RadarComplex_t));
    RadarMoments::applyWindow(iqWindowed, _window1D.dat(), _nSamples);
    
    // filter as appropriate
    
    if (_clutterFilterType == RadarMoments::CLUTTER_FILTER_ADAPTIVE) {

      // adaptive spectral filter

      moments.applyAdaptiveFilter(_nSamples, _prt, clutFilt, _fft,
                                  iqWindowed, calibNoise, _nyquist,
                                  iqFilt, NULL,
                                  filterRatio, spectralNoise, spectralSnr);
      
    } else if (_clutterFilterType == RadarMoments::CLUTTER_FILTER_REGRESSION) {
      
      // regression filter
      
      moments.setUseRegressionFilter(_regrNotchInterpMethod, -120.0);
      
      moments.applyRegressionFilter(_nSamples, _prt, _fft, regrF,
                                    iqWindowed, calibNoise,
                                    iqFilt, NULL,
                                    filterRatio, spectralNoise, spectralSnr);
      
    } else if (_clutterFilterType == RadarMoments::CLUTTER_FILTER_NOTCH) {
      
      // simple notch filter
      
      moments.applyNotchFilter(_nSamples, _prt, _fft,
                               iqWindowed, pow(10.0, calibNoise / 10.0),
                               iqFilt,
                               filterRatio, spectralNoise, spectralSnr);
      
    } else {
      
      // no filtering
      
      memcpy(iqFilt, iqWindowed, _nSamples * sizeof(RadarComplex_t));

    }

    // spectra
    
    RadarComplex_t *specComp1D = specComp2D.dat2D()[igate];
    double *specPower1D = specPower2D.dat2D()[igate];
    double *specDbm1D = specDbm2D.dat2D()[igate];
    
    _fft.fwd(iqFilt, specComp1D);
    _fft.shift(specComp1D);
    
    _computePowerSpectrum(specComp1D, specPower1D, specDbm1D);
    
  } // igate

}

void DwellSpectra::_computePowerSpectrum(RadarComplex_t *specComp1D,
                                         double *specPower1D,
                                         double *specDbm1D)

{
  
  for (size_t isample = 0; isample < _nSamples; isample++) {

    double pwr = RadarComplex::power(specComp1D[isample]);
    double dbm = 10.0 * log10(pwr);
    if (pwr <= 1.0e-12) {
      dbm = -120.0;
    }

    specPower1D[isample] = pwr;
    specDbm1D[isample] = dbm;

  } // isample

}

////////////////////////////////////////////////////
// Compute dbz spectra

void DwellSpectra::computeDbzSpectra()
  
{
  
  if (!_hcAvail) {
    return;
  }

  // compute spectral noise, using min of any gate
  
  _specNoiseDwellHc = computeDwellSpectralNoise(_specPowerHc2D, _specNoiseHc1D);
  
  double rangeKm = _startRangeKm;
  for (size_t igate = 0; igate < _nGates; igate++, rangeKm += _gateSpacingKm) {
    
    double rangeCorr = 20.0 * log10(rangeKm);
    
    double *specPowerHc1D = _specPowerHc2D.dat2D()[igate];
    double *specSnr1D = _specSnr2D.dat2D()[igate];
    double *specDbz1D = _specDbz2D.dat2D()[igate];

    for (size_t isample = 0; isample < _nSamples; isample++) {
      
      double powerHc = specPowerHc1D[isample];
      double powerHcNs = powerHc - _specNoiseDwellHc;

      if (powerHcNs > 0.0) {
        
        double snr = powerHcNs / _specNoiseDwellHc;
        double snrDb = 10.0 * log10(snr);
        double dbz =  snrDb + _calib.getBaseDbz1kmHc() + rangeCorr;
        specSnr1D[isample] = snrDb;
        specDbz1D[isample] = dbz;
        
      } else {

        // censored, set to low val
        
        specSnr1D[isample] = -50.0;
        specDbz1D[isample] = -50.0;
        
      }
      
    } // isample

  } // igate

}

////////////////////////////////////////////////////
// Compute zdr spectra

void DwellSpectra::computeZdrSpectra()
  
{

  if (!_hcAvail || !_vcAvail) {
    return;
  }
  
  for (size_t igate = 0; igate < _nGates; igate++) {
    
    double *specDbmHc1D = _specDbmHc2D.dat2D()[igate];
    double *specDbmVc1D = _specDbmVc2D.dat2D()[igate];
    double *specZdr1D = _specZdr2D.dat2D()[igate];
    
    for (size_t isample = 0; isample < _nSamples; isample++) {
      double zdr = specDbmHc1D[isample] - specDbmVc1D[isample];
      specZdr1D[isample] = zdr;
    } // isample

  } // igate

}

////////////////////////////////////////////////////
// Compute phidp spectra

void DwellSpectra::computePhidpSpectra()
  
{

  if (!_hcAvail || !_vcAvail) {
    return;
  }
  
  for (size_t igate = 0; igate < _nGates; igate++) {
    
    RadarComplex_t *specCompHc1D = _specCompHc2D.dat2D()[igate];
    RadarComplex_t *specCompVc1D = _specCompVc2D.dat2D()[igate];
    // double *specPowerHc1D = _specPowerHc2D.dat2D()[igate];
    // double *specPowerVc1D = _specPowerVc2D.dat2D()[igate];
    double *specPhidp1D = _specPhidp2D.dat2D()[igate];
    // double *specRhohv1D = _specRhohv2D.dat2D()[igate];
    
    for (size_t isample = 0; isample < _nSamples; isample++) {
      
      RadarComplex_t phaseDiff = RadarComplex::conjugateProduct(specCompHc1D[isample],
                                                                specCompVc1D[isample]);
      specPhidp1D[isample] = RadarComplex::argDeg(phaseDiff);
      // specRhohv1D[isample] =
      // RadarComplex::mag(phaseDiff) / sqrt(specPowerHc1D[isample] * specPowerVc1D[isample]);

    } // isample

  } // igate
  
  // compute the phidp folding range - 90 or 180 deg?
  
  _computePhidpFoldingRange();

}

////////////////////////////////////////////////////
// Compute rhohv spectra

void DwellSpectra::computeRhohvSpectra()
  
{

  if (!_hcAvail || !_vcAvail) {
    return;
  }
  
  for (size_t igate = 0; igate < _nGates; igate++) {
    
    RadarComplex_t *specCompHc1D = _specCompHc2D.dat2D()[igate];
    RadarComplex_t *specCompVc1D = _specCompVc2D.dat2D()[igate];
    // double *specPowerHc1D = _specPowerHc2D.dat2D()[igate];
    // double *specPowerVc1D = _specPowerVc2D.dat2D()[igate];
    double *specRhohv1D = _specRhohv2D.dat2D()[igate];
    
    for (size_t isample = 0; isample < _nSamples; isample++) {

      RadarComplex_t sumDiffs(0.0, 0.0);
      double sumPowerH = 0.0;
      double sumPowerV = 0.0;
      
      for (int kk = -1; kk <= 1; kk++) {
        
        int jsample = (isample + kk + _nSamples) % _nSamples;
        
        RadarComplex_t phaseDiff = RadarComplex::conjugateProduct(specCompHc1D[jsample],
                                                                  specCompVc1D[jsample]);

        sumDiffs = RadarComplex::complexSum(sumDiffs, phaseDiff);

        sumPowerH += RadarComplex::power(specCompHc1D[jsample]);
        sumPowerV += RadarComplex::power(specCompVc1D[jsample]);
        
      } // kk
        
      specRhohv1D[isample] = 
        RadarComplex::mag(sumDiffs) / sqrt(sumPowerH * sumPowerV);
      
    } // isample

  } // igate
  
}

////////////////////////////////////////////////////
// Compute texture of reflectivity

void DwellSpectra::computeTdbz()
  
{

  if (!_hcAvail) {
    return;
  }
  
  // compute 2D sdev of spectral dbz
  
  double **dbz2D = _specDbz2D.dat2D();
  double **tdbz2D = _specTdbz2D.dat2D();
  size_t nSamplesSdev = _nSamples;
  size_t nGatesSdev = _nGates;
  if (_tdbzKernelNGates < _nGates) {
    nGatesSdev = _tdbzKernelNGates;
    nSamplesSdev = _tdbzKernelNSamples;
  }
  size_t nSamplesSdevHalf = nSamplesSdev / 2;
  size_t nGatesSdevHalf = nGatesSdev / 2;
  
  for (size_t igate = 0; igate < _nGates; igate++) {
    for (size_t isample = 0; isample < _nSamples; isample++) {
      
      // compute index limits for computing sdev

      int sampleStart = isample - nSamplesSdevHalf;
      sampleStart = max(0, sampleStart);
      int sampleEnd = sampleStart + nSamplesSdev;
      sampleEnd = min((int) _nSamples, sampleEnd);
      sampleStart = sampleEnd - nSamplesSdev;

      int gateStart = igate - nGatesSdevHalf;
      gateStart = max(0, gateStart);
      int gateEnd = gateStart + nGatesSdev;
      gateEnd = min((int) _nGates, gateEnd);
      gateStart = gateEnd - nGatesSdev;
      
      // load up dbz^2 values for kernel region
      
      vector<double> tdbzKernel;
      for (int jgate = gateStart; jgate < gateEnd; jgate++) {
        for (int jsample = sampleStart; jsample < sampleEnd; jsample++) {
          double dbz = dbz2D[jgate][jsample];
          if (dbz < 0.0) {
            dbz = 0.0;
          }
          tdbzKernel.push_back(dbz * dbz);
        } // jsample
      } // jgate

      // compute sdev of tdbz
      
      double sdev = _computeSdev(tdbzKernel);
      double tdbz = sqrt(sdev);
      tdbz2D[igate][isample] = tdbz;
      
    } // isample
  } // igate

}

////////////////////////////////////////////////////
// Compute 2D standard deviation of zdr

void DwellSpectra::computeSdevZdr()
  
{

  if (!_hcAvail || !_vcAvail) {
    return;
  }
  
  // compute 2D sdev of spectral zdr
  
  double **zdr2D = _specZdr2D.dat2D();
  double **sdev2D = _specSdevZdr2D.dat2D();
  size_t nSamplesSdev = _nSamples;
  size_t nGatesSdev = _nGates;
  if (_sdevZdrKernelNGates < _nGates) {
    nGatesSdev = _sdevZdrKernelNGates;
    nSamplesSdev = _sdevZdrKernelNSamples;
  }
  size_t nSamplesSdevHalf = nSamplesSdev / 2;
  size_t nGatesSdevHalf = nGatesSdev / 2;
  
  for (size_t igate = 0; igate < _nGates; igate++) {
    for (size_t isample = 0; isample < _nSamples; isample++) {
      
      // compute index limits for computing sdev

      int sampleStart = isample - nSamplesSdevHalf;
      sampleStart = max(0, sampleStart);
      int sampleEnd = sampleStart + nSamplesSdev;
      sampleEnd = min((int) _nSamples, sampleEnd);
      sampleStart = sampleEnd - nSamplesSdev;

      int gateStart = igate - nGatesSdevHalf;
      gateStart = max(0, gateStart);
      int gateEnd = gateStart + nGatesSdev;
      gateEnd = min((int) _nGates, gateEnd);
      gateStart = gateEnd - nGatesSdev;
      
      // load up zdr values for kernel region

      vector<double> zdrKernel;
      for (int jgate = gateStart; jgate < gateEnd; jgate++) {
        for (int jsample = sampleStart; jsample < sampleEnd; jsample++) {
          zdrKernel.push_back(zdr2D[jgate][jsample]);
        } // jsample
      } // jgate

      // compute sdev of zdr
      
      double zdrSdev = _computeSdev(zdrKernel);
      sdev2D[igate][isample] = zdrSdev;
      
    } // isample
  } // igate

}

////////////////////////////////////////////////////
// Compute 2D standard deviation of phidp

void DwellSpectra::computeSdevPhidp()
  
{

  if (!_hcAvail || !_vcAvail) {
    return;
  }
  
  // compute 2D sdev of spectral phidp
  
  double **phidp2D = _specPhidp2D.dat2D();
  double **sdev2D = _specSdevPhidp2D.dat2D();
  size_t nSamplesSdev = _nSamples;
  size_t nGatesSdev = _nGates;
  if (_sdevPhidpKernelNGates < _nGates) {
    nGatesSdev = _sdevPhidpKernelNGates;
    nSamplesSdev = _sdevPhidpKernelNSamples;
  }
  size_t nSamplesSdevHalf = nSamplesSdev / 2;
  size_t nGatesSdevHalf = nGatesSdev / 2;
  
  for (size_t igate = 0; igate < _nGates; igate++) {
    for (size_t isample = 0; isample < _nSamples; isample++) {
      
      // compute index limits for computing sdev

      int sampleStart = isample - nSamplesSdevHalf;
      sampleStart = max(0, sampleStart);
      int sampleEnd = sampleStart + nSamplesSdev;
      sampleEnd = min((int) _nSamples, sampleEnd);
      sampleStart = sampleEnd - nSamplesSdev;

      int gateStart = igate - nGatesSdevHalf;
      gateStart = max(0, gateStart);
      int gateEnd = gateStart + nGatesSdev;
      gateEnd = min((int) _nGates, gateEnd);
      gateStart = gateEnd - nGatesSdev;
      
      // load up phidp values for kernel region
      
      vector<double> phidpKernel;
      for (int jgate = gateStart; jgate < gateEnd; jgate++) {
        for (int jsample = sampleStart; jsample < sampleEnd; jsample++) {
          phidpKernel.push_back(phidp2D[jgate][jsample]);
        } // jsample
      } // jgate

      // compute sdev of phidp
      
      double phidpSdev = _computeSdevPhidp(phidpKernel);
      sdev2D[igate][isample] = phidpSdev;
      
    } // isample
  } // igate

}

/////////////////////////////////////////////////
// compute SDEV

double DwellSpectra::_computeSdev(const vector<double> &val)
  
{
  
  double nVal = 0.0;
  double sumVal = 0.0;
  double sumValSq = 0.0;
  
  for (size_t ii = 0; ii < val.size(); ii++) {
    double vv = val[ii];
    sumVal += vv;
    sumValSq += (vv * vv);
    nVal++;
  }
    
  double meanVal = sumVal / nVal;
  double sdev = 0.001;
  if (nVal > 2) {
    double term1 = sumValSq / nVal;
    double term2 = meanVal * meanVal;
    if (term1 >= term2) {
      sdev = sqrt(term1 - term2);
    }
  }

  return sdev;

}

/////////////////////////////////////////////////
// compute SDEV for PHIDP
// takes account of folding

double DwellSpectra::_computeSdevPhidp(const vector<double> &phidp)
   
{
  
  // compute mean phidp

  double count = 0.0;
  double sumxx = 0.0;
  double sumyy = 0.0;
  for (size_t ii = 0; ii < phidp.size(); ii++) {
    double xx, yy;
    ta_sincos(phidp[ii] * DEG_TO_RAD, &yy, &xx);
    sumxx += xx;
    sumyy += yy;
    count++;
  }
  double meanxx = sumxx / count;
  double meanyy = sumyy / count;
  double phidpMean = atan2(meanyy, meanxx) * RAD_TO_DEG;
  if (_phidpFoldsAt90) {
    phidpMean *= 0.5;
  }
  
  // compute standard deviation centered on the mean value
  
  count = 0.0;
  double sum = 0.0;
  double sumSq = 0.0;
  for (size_t ii = 0; ii < phidp.size(); ii++) {
    double diff = phidp[ii] - phidpMean;
    // constrain diff
    while (diff < -_phidpFoldVal) {
      diff += 2 * _phidpFoldVal;
    }
    while (diff > _phidpFoldVal) {
      diff -= 2 * _phidpFoldVal;
    }
    // sum up
    count++;
    sum += diff;
    sumSq += diff * diff;
  }

  double meanDiff = sum / count;
  double term1 = sumSq / count;
  double term2 = meanDiff * meanDiff;
  double sdev = 0.001;
  if (term1 >= term2) {
    sdev = sqrt(term1 - term2);
  }

  return sdev;
  
}

/////////////////////////////////////////////
// compute the folding values and range
// by inspecting the phidp values

void DwellSpectra::_computePhidpFoldingRange()
  
{
  
  // check if fold is at 90 or 180
  
  double phidpMin = 9999;
  double phidpMax = -9999;
  
  for (size_t igate = 0; igate < _nGates; igate++) {
    for (size_t isample = 0; isample < _nSamples; isample++) {
      double phidp = _specPhidp2D.dat2D()[igate][isample];
      phidpMin = min(phidpMin, phidp);
      phidpMax = max(phidpMax, phidp);
    } // isample
  } // igate
  
  _phidpFoldsAt90 = false;
  _phidpFoldVal = 180.0;
  if (phidpMin > -90 && phidpMax < 90) {
    _phidpFoldVal = 90.0;
    _phidpFoldsAt90 = true;
  }
  _phidpFoldRange = _phidpFoldVal * 2.0;
  
}

/////////////////////////////////////////////////////////////////
// Compute spectral noise for entire dwell for specified variable
// this is the min noise at any gate

double DwellSpectra::computeDwellSpectralNoise(const TaArray2D<double> &specPower2D,
                                               TaArray<double> &specNoise1D)
  
{

  // get spectral noise at each gate

  double minNoise = 1000.0;
  vector<double> noiseDbm;
  for (size_t igate = 0; igate < _nGates; igate++) {
    double *specPower = specPower2D.dat2D()[igate];
    double noise = computeSpectralNoise(specPower, _nSamples);
    if (noise < minNoise) {
      minNoise = noise;
    }
    specNoise1D.dat()[igate] = noise;
    noiseDbm.push_back(10.0 * log10(noise));
  }
  
  // compute mean and standard deviation
  
  double meanDbm, sdevDbm;
  _computeMeanSdev(noiseDbm, meanDbm, sdevDbm);

  // load array with low end of noise
  
  // double cutoffDbm = meanDbm - sdevDbm;
  double cutoffDbm = meanDbm;
  vector<double> lowDbm;
  for (size_t ii = 0; ii < noiseDbm.size(); ii++) {
    if (noiseDbm[ii] < cutoffDbm) {
      lowDbm.push_back(noiseDbm[ii]);
    }
  }

  if (lowDbm.size() < 1) {
    return minNoise;
  }
  
  // sort array

  sort(lowDbm.begin(), lowDbm.end());
  
  // return median of low Dbm

  double medianLowDbm = lowDbm[lowDbm.size() / 2];

  return pow(10.0, medianLowDbm / 10.0);

}

/////////////////////////////////////////////////////
// Compute noise from a power spectrum
//
// Divide spectrum into runs and compute the mean power
// for each run, incrementing by one index at a time.
//
// The noise power is estimated as the mimumum of the section
// powers.

double DwellSpectra::computeSpectralNoise(const double *powerSpec,
                                          size_t nSamples)
  
{

  // for short spectra use the entire spectrum

  if (nSamples < 8) {
    double sum = 0.0;
    for (size_t ii = 0; ii < nSamples; ii++) {
      sum += powerSpec[ii];
    }
    double noisePower = sum / nSamples;
    return noisePower;
  }

  // compute the size of a section

  int nRuns = nSamples / 8;
  if (nRuns < 8) {
    nRuns = 8;
  }
  int nPtsRun = nSamples / nRuns;
  
  // compute the sum for the first run
  
  double sum = 0.0;
  for (int ii = 0; ii < nPtsRun; ii++) {
    sum += powerSpec[ii];
  }
  double minSum = sum;
  
  // now move through the spectrum one point at a time, computing
  // the sum for that run by removing the first point of the previous
  // run and adding the next point to the end.
  // keep track of the minimum sum

  for (size_t ii = nPtsRun; ii < nSamples; ii++) {
    sum -= powerSpec[ii - nPtsRun];
    sum += powerSpec[ii];
    if (sum < minSum) {
      minSum = sum;
    }
  }

  // compute noise from minimum sum

  double noisePower = minSum / nPtsRun;
  
  return noisePower;

}

////////////////////////////////////////////////////
// Compute spectral CMD

void DwellSpectra::computeSpectralCmd()
  
{

  // check we have data
  
  if (!_hcAvail || !_vcAvail) {
    return;
  }

  // compute the features we need
  
  // computePowerSpectra();
  // computeDbzSpectra();
  // computeZdrSpectra();
  // computePhidpRhohvSpectra();
  // computeTdbz();
  // computeSdevZdr();
  // computeSdevPhidp();
 
  // accumulate the interest
  
  double **snr = _specSnr2D.dat2D();
  double **tdbz = _specTdbz2D.dat2D();
  double **zdrSdev = _specSdevZdr2D.dat2D();
  double **phidpSdev = _specSdevPhidp2D.dat2D();

  double **snrInt = _specSnrInterest2D.dat2D();
  double **tdbzInt = _specTdbzInterest2D.dat2D();
  double **zdrSdevInt = _specSdevZdrInterest2D.dat2D();
  double **phidpSdevInt = _specSdevPhidpInterest2D.dat2D();

  for (size_t igate = 0; igate < _nGates; igate++) {
    for (size_t isample = 0; isample < _nSamples; isample++) {
      
      snrInt[igate][isample] =
        _interestMapSnr->getInterest(snr[igate][isample]);
      
      tdbzInt[igate][isample] =
        _interestMapTdbz->getInterest(tdbz[igate][isample]);
      
      zdrSdevInt[igate][isample] =
        _interestMapSdevZdr->getInterest(zdrSdev[igate][isample]);
      
      phidpSdevInt[igate][isample] =
        _interestMapSdevPhidp->getInterest(phidpSdev[igate][isample]);

    } // isample
  } // igate

  // compute sum weights

  double weightSnr = _interestMapSnr->getWeight();
  double weightTdbz = _interestMapTdbz->getWeight();
  double weightSdevZdr = _interestMapSdevZdr->getWeight();
  double weightSdevPhidp = _interestMapSdevPhidp->getWeight();

  double sumWeights = 0.0;
  sumWeights += weightSnr;
  sumWeights += weightTdbz;
  sumWeights += weightSdevZdr;
  sumWeights += weightSdevPhidp;
  
  // compute cmd, and filter
  
  double **cmd = _specCmd2D.dat2D();
  for (size_t igate = 0; igate < _nGates; igate++) {
    for (size_t isample = 0; isample < _nSamples; isample++) {

      double sumInterest = 0.0;

      sumInterest += snrInt[igate][isample] * weightSnr;
      sumInterest += tdbzInt[igate][isample] * weightTdbz;
      sumInterest += zdrSdevInt[igate][isample] * weightSdevZdr;
      sumInterest += phidpSdevInt[igate][isample] * weightSdevPhidp;

      cmd[igate][isample] = sumInterest / sumWeights;
        
    } // isample
  } // igate

  // compute cmd mean and fraction exceeding threshold
  
  double *meanCmd = getMeanCmd1D();
  double *fractionCmd = getFractionCmd1D();
  
  for (size_t igate = 0; igate < _nGates; igate++) {

    double sumCmd = 0.0;
    double sumFrac = 0.0;
    
    for (size_t isample = 0; isample < _nSamples; isample++) {
      double val = cmd[igate][isample];
      sumCmd += val;
      if (val >= _cmdThresholdDetect) {
        sumFrac++;
      }
    } // isample

    double mean = sumCmd / (double) _nSamples;
    double frac = sumFrac / (double) _nSamples;

    meanCmd[igate] = mean;
    fractionCmd[igate] = frac;

  } // igate
  
}

////////////////////////////////////////////////////
// Filter IQ Hc and Vc based on CMD

void DwellSpectra::filterIqUsingCmd()
  
{

  // copy the unfiltered data to filtered arrays
  
  _specCompHcFilt2D = _specCompHc2D;
  _specCompVcFilt2D = _specCompVc2D;

  // clear (zero out) the filtered spectra for those
  // specrtal points that have a CMD value
  // exceeding the threshold
  
  double **specCmd = _specCmd2D.dat2D();
  RadarComplex_t **specHcFilt2D = _specCompHcFilt2D.dat2D();
  RadarComplex_t **specVcFilt2D = _specCompVcFilt2D.dat2D();

  for (size_t igate = 0; igate < _nGates; igate++) {
    for (size_t isample = 0; isample < _nSamples; isample++) {
      double cmdVal = specCmd[igate][isample];
      if (cmdVal > _cmdThresholdMoments) {

        // notch out the spectral point
        specHcFilt2D[igate][isample].clear();
        specVcFilt2D[igate][isample].clear();

        // set the power to the noise
        
        // RadarComplex_t &specHc = specHcFilt2D[igate][isample];
        // double powerHc = RadarComplex::power(specHc);
        // double powerRatioHc = _specNoiseDwellHc / powerHc;
        // double magRatioHc = sqrt(powerRatioHc);
        // specHc.re *= magRatioHc;
        // specHc.im *= magRatioHc;
        
        // RadarComplex_t &specVc = specVcFilt2D[igate][isample];
        // double powerVc = RadarComplex::power(specVc);
        // double powerRatioVc = _specNoiseDwellVc / powerVc;
        // double magRatioVc = sqrt(powerRatioVc);
        // specVc.re *= magRatioVc;
        // specVc.im *= magRatioVc;

      }
    } // isample
  } // igate

  // invert the filtered spectra into the time series
  
  RadarComplex_t **iqHcFilt2D = _iqHcFilt2D.dat2D();
  RadarComplex_t **iqVcFilt2D = _iqVcFilt2D.dat2D();

  for (size_t igate = 0; igate < _nGates; igate++) {
    
    RadarComplex_t *specHcFilt1D = specHcFilt2D[igate];
    RadarComplex_t *specVcFilt1D = specVcFilt2D[igate];
    
    RadarComplex_t *iqHcFilt1D = iqHcFilt2D[igate];
    RadarComplex_t *iqVcFilt1D = iqVcFilt2D[igate];
    
    _fft.unshift(specHcFilt1D);
    _fft.unshift(specVcFilt1D);

    _fft.inv(specHcFilt1D, iqHcFilt1D);
    _fft.inv(specVcFilt1D, iqVcFilt1D);
    
  } // igate
  
}

///////////////////////////////////////////////////////////////
// Create the default interest maps and weights

void DwellSpectra::_createDefaultInterestMaps()
  
{

  vector<InterestMap::ImPoint> pts;
  
  pts.clear();
  pts.push_back(InterestMap::ImPoint(0.0, 0.0001));
  pts.push_back(InterestMap::ImPoint(20.0, 1.0));
  setInterestMapSnr(pts, 1.0);
  
  pts.clear();
  pts.push_back(InterestMap::ImPoint(30.0, 0.0001));
  pts.push_back(InterestMap::ImPoint(40.0, 1.0));
  setInterestMapTdbz(pts, 1.0);
  
  pts.clear();
  pts.push_back(InterestMap::ImPoint(3.0, 0.0001));
  pts.push_back(InterestMap::ImPoint(4.0, 1.0));
  setInterestMapSdevZdr(pts, 1.0);

  pts.clear();
  pts.push_back(InterestMap::ImPoint(28.0, 0.0001));
  pts.push_back(InterestMap::ImPoint(32.0, 1.0));
  setInterestMapSdevPhidp(pts, 1.0);

  setCmdThresholdMoments(0.7);
  setCmdThresholdDetect(0.9);

}

/////////////////////////////////////////////////////////
// set interest map and weight for snr

void DwellSpectra::setInterestMapSnr
  (const vector<InterestMap::ImPoint> &pts,
   double weight)
  
{
  if (_interestMapSnr) {
    delete _interestMapSnr;
  }
  _interestMapSnr = new InterestMap("Snr", pts, weight);
}

/////////////////////////////////////////////////////////
// set interest map and weight for dbz texture

void DwellSpectra::setInterestMapTdbz
  (const vector<InterestMap::ImPoint> &pts,
   double weight)
  
{
  if (_interestMapTdbz) {
    delete _interestMapTdbz;
  }
  _interestMapTdbz = new InterestMap("Tdbz", pts, weight);
}

/////////////////////////////////////////////////////////
// set interest map and weight for sdev of ZDR

void DwellSpectra::setInterestMapSdevZdr
  (const vector<InterestMap::ImPoint> &pts,
   double weight)
  
{
  if (_interestMapSdevZdr) {
    delete _interestMapSdevZdr;
  }
  _interestMapSdevZdr = new InterestMap("SdevZdr", pts, weight);
}

/////////////////////////////////////////////////////////
// set interest map and weight for sdev of PHIDP

void DwellSpectra::setInterestMapSdevPhidp
  (const vector<InterestMap::ImPoint> &pts,
   double weight)
  
{
  if (_interestMapSdevPhidp) {
    delete _interestMapSdevPhidp;
  }
  _interestMapSdevPhidp = new InterestMap("SdevPhidp", pts, weight);
}

///////////////////////////////////////////////////////////////
// Compute mean and standard deviation

void DwellSpectra::_computeMeanSdev(vector<double> &xx,
                                    double &mean,
                                    double &sdev)
  
{

  double sumx = 0.0;
  double sumx2 = 0.0;
  double nn = 0.0;
  for (size_t ii = 0; ii < xx.size(); ii++) {
    double xval = xx[ii];
    sumx += xval;
    sumx2 += xval * xval;
    nn++;
  }

  mean = sumx / nn;

  double var = (sumx2 - (sumx * sumx) / nn) / (nn - 1.0);
  if (var >= 0.0) {
    sdev = sqrt(var);
  } else {
    sdev = 0.0;
  }

}

