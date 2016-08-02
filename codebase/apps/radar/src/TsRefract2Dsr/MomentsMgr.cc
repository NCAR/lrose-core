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
// MomentsMgr.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2005
//
///////////////////////////////////////////////////////////////
//
// MomentsMgr manages the use of the Moments objects, handling 
// the specific parameters for each case.
//
///////////////////////////////////////////////////////////////

#include <iostream>
#include <iomanip>
#include <toolsa/toolsa_macros.h>
#include <toolsa/DateTime.hh>
#include "MomentsMgr.hh"
#include "OpsInfo.hh"
using namespace std;

const double MomentsMgr::_missingDbl = -9999.0;

////////////////////////////////////////////////////
// Constructor

MomentsMgr::MomentsMgr(const string &prog_name,
		       const Params &params,
                       const OpsInfo &opsInfo) :
        _progName(prog_name),
        _params(params),
        _opsInfo(opsInfo),
        _nSamples(_params.n_samples),
        _useFft(_params.moments_algorithm == Params::ALG_FFT),
        _rangeCorr(NULL),
        _atmosAtten(_params.atmos_attenuation),
        _moments(_nSamples),
        _momentsHalf(_nSamples / 2)
  
{

  // initialize geometry to missing values
  
  _startRange = _missingDbl;
  _gateSpacing = _missingDbl;
  _noiseFixedChan0 = _missingDbl;
  _dbz0Chan0 = _missingDbl;
  _noiseFixedChan1 = _missingDbl;
  _dbz0Chan1 = _missingDbl;

  // allocate range correction table

  _rangeCorr = new double[_maxGates];

  // set window type
  
  if (_params.fft_window == Params::WINDOW_NONE) {
    _fftWindow = Moments::WINDOW_NONE;
  } else if (_params.fft_window == Params::WINDOW_HANNING) {
    _fftWindow = Moments::WINDOW_HANNING;
  } else if (_params.fft_window == Params::WINDOW_BLACKMAN) {
    _fftWindow = Moments::WINDOW_BLACKMAN;
  }

  // initialize moments object

  _moments.setSignalToNoiseRatioThreshold
    (_params.signal_to_noise_ratio_threshold);
  _momentsHalf.setSignalToNoiseRatioThreshold
    (_params.signal_to_noise_ratio_threshold);
  
}

//////////////////////////////////////////////////////////////////
// destructor

MomentsMgr::~MomentsMgr()

{

  if (_rangeCorr) {
    delete[] _rangeCorr;
  }

}

/////////////////////////////////////////////////
// compute moments

void MomentsMgr::computeMoments(double prt, int nGatesPulse, Complex_t **IQ,
				double *powerDbm, double *snr, double *dbz,
				double *vel, double *width, int *flags)
  
{

  // initialize based on OpsInfo
  
  _init();
  
  // loop through the gates
  
  double range = _startRange;
  for (int igate = 0; igate < nGatesPulse; igate++, range += _gateSpacing) {
    
    Complex_t *iq = IQ[igate];
    
    double power_0 = _missingDbl;
    double vel_0 = _missingDbl, width_0 = _missingDbl;
    double noise_0 = _missingDbl;
    int flags_0 = 0;
    
    if (_useFft) {
      
      // FFT
      
      _moments.computeByFft(iq, _fftWindow, prt,
                            power_0, noise_0,
			    vel_0, width_0, flags_0);
      
    } else {
      
      // Pulse Pair
      
      _moments.computeByPp(iq, prt, power_0, vel_0, width_0, flags_0);
      
    } // (if (_useFft)
    
    if (power_0 > 0.0) {
      // adjust for saturation value
      power_0 *= _opsInfo.getSaturationMult();
    }

    double _dbm = _missingDbl;
    if (power_0 != _missingDbl) {
      _dbm = 10.0 * log10(power_0);
      powerDbm[igate] = _dbm + _params.dbm_correction;
      if (power_0 > _noiseFixedChan0) {
	double _snr =
          10.0 * log10((power_0 - _noiseFixedChan0) / _noiseFixedChan0);
	snr[igate] = _snr;
	dbz[igate] = _snr + _dbz0Chan0 + _rangeCorr[igate];
      }
    }
    
    vel[igate] = vel_0;
    width[igate] = width_0;
    flags[igate] = flags_0;

  } // igate

}

/////////////////////////////////////////////////
// compute aiq and niq values

void MomentsMgr::computeAiqNiq(int nGatesPulse, int nSamples,
                               Complex_t **IQ,
                               double *aiq, double *niq,
                               double *meanI, double *meanQ)

{

  double factor = DEG_PER_RAD;
  if (_params.change_aiq_sign) {
    factor *= -1.0;
  }

  for (int igate = 0; igate < nGatesPulse; igate++) {

    // sum up I and Q

    double sumI = 0.0, sumQ = 0.0;
    Complex_t *iq = IQ[igate];
    for (int i = 0; i < nSamples; i++, iq++) {
      sumI += iq->re;
      sumQ += iq->im;
    }

    // compute mean I and Q

    double avI = sumI / nSamples;
    double avQ = sumQ / nSamples;
    
    meanI[igate] = 10.0 * log10(avI);
    meanQ[igate] = 10.0 * log10(avQ);

    // compute aiq and niq
    
    aiq[igate] = factor * atan2(avQ, avI);
    niq[igate] = 10.0 * log10(sqrt(avI * avI + avQ * avQ));
    
  } // igate

}

/////////////////////////////////////////////////
// compute moments in alternating dual pol mode

void MomentsMgr::computeAlternating(double prt,
                                    int nGatesPulse,
                                    Complex_t **IQ,
                                    Complex_t **IQH,
                                    Complex_t **IQV,
                                    double *powerDbm,
                                    double *snr,
                                    double *dbz,
                                    double *vel,
                                    double *width,
                                    int *flags)
  
{

  // initialize based on OpsInfo
  
  _init();
  
  int nDual = _nSamples / 2;
  double wavelengthMeters = _opsInfo.getRadarWavelengthCm() / 100.0;
  double nyquist = ((wavelengthMeters / prt) / 4.0);
  
  double range = _startRange;
  for (int igate = 0; igate < nGatesPulse; igate++, range += _gateSpacing) {
    
    Complex_t *iqh = IQH[igate];
    Complex_t *iqv = IQV[igate];
    
    double power_0 = _missingDbl, vel_0 = _missingDbl, width_0 = _missingDbl;
    double power_1 = _missingDbl, vel_1 = _missingDbl, width_2 = _missingDbl;
    double noise_0 = _missingDbl, noise_1 = _missingDbl;
    int flags_0 = 0, flags_1 = 0;
    
    if (_useFft) {
      
      // Horizontal
      
      _momentsHalf.computeByFft(iqh, _fftWindow, prt, power_0, noise_0,
				vel_0, width_0, flags_0);
      
      // Vertical
      
      _momentsHalf.computeByFft(iqv, _fftWindow, prt, power_1, noise_1,
				vel_1, width_2, flags_1);

    } else {
      
      // Pulse Pair
      
      _momentsHalf.computeByPp(iqh, prt, power_0, vel_0, width_0, flags_0);
      _momentsHalf.computeByPp(iqv, prt, power_1, vel_1, width_2, flags_1);

    } // if (_useFft)
      
    if (power_0 > 0.0) {
      // adjust for saturation value
      power_0 *= _opsInfo.getSaturationMult();
    }

    if (power_1 > 0.0) {
      // adjust for saturation value
      power_1 *= _opsInfo.getSaturationMult();
    }

    double dbm_0 = _missingDbl, dbm_1 = _missingDbl;
    double snr_0 = _missingDbl, snr_1 = _missingDbl;
    if (power_0 != _missingDbl) {
      dbm_0 = 10.0 * log10(power_0);
      if (power_0 > _noiseFixedChan0) {
	snr_0 = 10.0 * log10((power_0 - _noiseFixedChan0) / _noiseFixedChan0);
      }
    }
    if (power_1 != _missingDbl) {
      dbm_1 = 10.0 * log10(power_1);
      if (power_1 > _noiseFixedChan1) {
	snr_1 = 10.0 * log10((power_1 - _noiseFixedChan1) / _noiseFixedChan1);
      }
    }
    
    // check for SNR

    if (snr_0 == _missingDbl || snr_1 == _missingDbl) {
      // below SNR
      flags[igate] = flags_0 | flags_1;
      // return;
    }
    
    // power and SNR
    
    double dbmMean = (dbm_0 + dbm_1) / 2.0;
    double snrMean = (snr_0 + snr_1) / 2.0;
    powerDbm[igate] = dbmMean + _params.dbm_correction;
    snr[igate] = snrMean;
    
    double dbzChan0 = snr_0 + _dbz0Chan0 + _rangeCorr[igate];
    double dbzChan1 = snr_1 + _dbz0Chan1 + _rangeCorr[igate];
    dbz[igate] = (dbzChan0 + dbzChan1) / 2.0;
    width[igate] = (width_0 + width_2) / 2.0;

    // velocity

    Complex_t Rhhvv1 = _meanConjugateProduct(iqv, iqh + 1, nDual - 1);
    Complex_t Rvvhh1 = _meanConjugateProduct(iqh + 1, iqv + 1, nDual - 1);

    double argRhhvv1 = _computeArg(Rhhvv1);
    double argRvvhh1 = _computeArg(Rvvhh1);

    double phiDpRad = (argRhhvv1 - argRvvhh1) / 2.0;
    
    double argVelhhvv = phiDpRad - argRhhvv1;
    double argVelvvhh = - phiDpRad - argRvvhh1;
    double meanArgVel = (argVelhhvv + argVelvvhh) / 2.0;
    double meanVel = (meanArgVel / M_PI) * nyquist;
    vel[igate] = meanVel* -1.0;

  } // igate
  
}
  
///////////////////////////////////////
// initialize based on OpsInfo

void MomentsMgr::_init()
  
{

  // compute range correction table
  
  if (_startRange != _opsInfo.getStartRange() ||
      _gateSpacing != _opsInfo.getGateSpacing() || 
      _dbz0Chan0 != _opsInfo.getNoiseDbzAt1km()) {
    _startRange = _opsInfo.getStartRange();
    _gateSpacing = _opsInfo.getGateSpacing();
    _dbz0Chan0 = _opsInfo.getNoiseDbzAt1km();
    _computeRangeCorrection();
  }

  // dbz cal info
  
  _dbz0Chan0 = _opsInfo.getDbz0();
  _dbz0Chan1 = _opsInfo.getDbz0();

  // set noise value

  _noiseFixedChan0 = pow(10.0, _opsInfo.getNoiseDbm0() / 10.0);
  _noiseFixedChan1 = pow(10.0, _opsInfo.getNoiseDbm1() / 10.0);

  // if noise sdev is negative, the dbz0 values for each channel
  // have been stored there

  if (_opsInfo.getNoiseSdev0() < 0) {
    _dbz0Chan0 = _opsInfo.getNoiseSdev0();
    _dbz0Chan1 = _opsInfo.getNoiseSdev1();
  }
  
  // set up moments object
  
  _moments.setWavelength(_opsInfo.getRadarWavelengthCm() / 100.0);
  _moments.setNoiseValueDbm(_opsInfo.getNoiseDbm0());
  _momentsHalf.setWavelength(_opsInfo.getRadarWavelengthCm() / 100.0);
  _momentsHalf.setNoiseValueDbm(_opsInfo.getNoiseDbm0());
  
}
     
///////////////////////////////////////
// compute range correction table

void MomentsMgr::_computeRangeCorrection()

{
  
  for (int i = 0; i < _maxGates; i++) {
    double range_km = _startRange + i * _gateSpacing;
    _rangeCorr[i] = _params.dbz_calib_correction +
      20.0 * log10(range_km) + range_km * _atmosAtten;
  }

}

/////////////////////////////////////////////
// compute mean conjugate product of series

Complex_t MomentsMgr::_meanConjugateProduct(const Complex_t *c1,
					    const Complex_t *c2,
					    int len) const
  
{
  
  double sumRe = 0.0;
  double sumIm = 0.0;

  for (int ipos = 0; ipos < len; ipos++, c1++, c2++) {
    sumRe += ((c1->re * c2->re) + (c1->im * c2->im));
    sumIm += ((c1->im * c2->re) - (c1->re * c2->im));
  }

  Complex_t product;
  product.re = sumRe / len;
  product.im = sumIm / len;

  return product;

}

//////////////////////////////////////
// compute angle in radians

double MomentsMgr::_computeArg(const Complex_t &cc) const
  
{
  double arg = 0.0;
  if (cc.im != 0.0 && cc.re != 0.0) {
    arg = atan2(cc.im, cc.re);
  }
  return arg;
  
}

