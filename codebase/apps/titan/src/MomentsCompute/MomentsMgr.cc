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

#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>
#include <iomanip>
#include <cmath>
#include "MomentsMgr.hh"
#include "Fft.hh"
#include "PhidpKdp.hh"
#include "TaArray.hh"
using namespace std;

const double MomentsMgr::_missingDbl = -9999.0;
const double MomentsMgr::_phidpPhaseLimit = -70;

#ifndef RAD_TO_DEG
#define RAD_TO_DEG 57.29577951308092
#endif

#ifndef DEG_TO_RAD
#define DEG_TO_RAD 0.01745329251994372
#endif

////////////////////////////////////////////////////
// Constructor

MomentsMgr::MomentsMgr(const Params &params,
					   Params::moments_params_t &moments_params) :
_params(params),
_momentsParams(moments_params),
_nSamples(moments_params.n_samples),
_nSamplesHalf(_nSamples / 2),
_moments(_nSamples),
_momentsHalf(_nSamplesHalf)

{

	// initialize

	_nGates = 0;
	_startRange = _momentsParams.start_range;
	_gateSpacing = _momentsParams.gate_spacing;
	_rangeCorr = NULL;
        _fft = NULL;
        _fftHalf = NULL;

	_noiseFixedHc = pow(10.0, _params.hc_receiver.noise_dBm / 10.0);
	_noiseFixedHx = pow(10.0, _params.hx_receiver.noise_dBm / 10.0);
	_noiseFixedVc = pow(10.0, _params.vc_receiver.noise_dBm / 10.0);
	_noiseFixedVx = pow(10.0, _params.vx_receiver.noise_dBm / 10.0);

	_dbz0Hc = _params.hc_receiver.noise_dBm -
          _params.hc_receiver.gain - _params.hc_receiver.radar_constant;
	_dbz0Hx = _params.hx_receiver.noise_dBm -
          _params.hx_receiver.gain - _params.hx_receiver.radar_constant;
	_dbz0Vc = _params.vc_receiver.noise_dBm -
          _params.vc_receiver.gain - _params.vc_receiver.radar_constant;
	_dbz0Vx = _params.vx_receiver.noise_dBm -
          _params.vx_receiver.gain - _params.vx_receiver.radar_constant;

	// set window type

	if (_momentsParams.window == Params::WINDOW_NONE) {
          _fftWindow = Moments::WINDOW_NONE;
 	} else if (_momentsParams.window == Params::WINDOW_VONHANN) {
          _fftWindow = Moments::WINDOW_VONHANN;
	} else if (_momentsParams.window == Params::WINDOW_BLACKMAN) {
          _fftWindow = Moments::WINDOW_BLACKMAN;
	}

	// initialize moments objects

	_moments.setWavelength(_params.radar.wavelength_cm / 100.0);
	_momentsHalf.setWavelength(_params.radar.wavelength_cm / 100.0);

        _moments.setNoiseValueDbm(_params.hc_receiver.noise_dBm);
        _momentsHalf.setNoiseValueDbm(_params.hc_receiver.noise_dBm);
  
        // set up FFT
  
        _fft = new Fft(_nSamples);
        _fftHalf = new Fft(_nSamplesHalf);
 
}

//////////////////////////////////////////////////////////////////
// destructor

MomentsMgr::~MomentsMgr()

{

	if (_rangeCorr) {
		delete[] _rangeCorr;
	}

        if (_fft) {
          delete _fft;
        }
        if (_fftHalf) {
          delete _fftHalf;
        }

}

/////////////////////////////////////////////////////////////
// compute moments - single pol
//
// IQC data is for a single-channel copolar
//
// IQC[nGates][nPulses]

void MomentsMgr::computeSingle(double beamTime,
                               double el,
                               double az,
                               double prt,
                               int nGates,
                               Complex_t **IQHC,
                               Fields *fields)

{

  _checkRangeCorrection(nGates);
  
  double wavelengthMeters = _params.radar.wavelength_cm / 100.0;
  double nyquist = ((wavelengthMeters / prt) / 4.0);
  
  double range = _startRange;
  for (int igate = 0; igate < nGates; igate++, range += _gateSpacing) {
    
    // locate the IQ samples for this gate
    
    Complex_t *iqHc = IQHC[igate];

    // apply clutter filter if needed
    // otherwise copy through
    
    TaArray<Complex_t> iqhc_;
    Complex_t *iqhc = iqhc_.alloc(_nSamples);
    
    if (_momentsParams.apply_clutter_filter) {
      
      _applyClutterFilter(_nSamples, *_fft, _moments,
                          nyquist, iqHc, iqhc);
      
    } else {

      memcpy(iqhc, iqHc, _nSamples * sizeof(Complex_t));

    }
  
    // compute lag 0 covariances
    // use N-1 so all covariances have the same number of points
    
    double lag0_hc_hc = Complex::meanPower(iqhc, _nSamples);

    // compute noise-subtracted lag0
    
    double lag0_hc_hc_ns = lag0_hc_hc - _noiseFixedHc;
    if (lag0_hc_hc_ns < 0) { lag0_hc_hc_ns = lag0_hc_hc; }
    
    // compute dbm

    fields[igate].dbmhc =
      10.0 * log10(lag0_hc_hc) - _params.hc_receiver.gain;
    fields[igate].dbm = fields[igate].dbmhc;
    
    // compute snr
    
    double snr_hc = lag0_hc_hc_ns / _noiseFixedHc;

    double snrDb = 10.0 * log10(snr_hc);
    fields[igate].snr = snrDb;
    fields[igate].snrhc = snrDb;
    
    // compute dbz and dbm
    
    double dbz_hc = 10.0 * log10(snr_hc) + _dbz0Hc + _rangeCorr[igate];
    fields[igate].dbz = dbz_hc;

    // velocity

    Complex_t lag1_hc_hc =
      Complex::meanConjugateProduct(iqhc, iqhc + 1, _nSamples - 1);
    
    double argVel = Complex::argRad(lag1_hc_hc);
    double vel = (argVel / M_PI) * nyquist;
    fields[igate].vel = vel * -1.0;

    // ncp
    
    double magLag1Hc = Complex::mag(lag1_hc_hc);
    double ncp = magLag1Hc / lag0_hc_hc;
    if (ncp < 1.0e-6) {
      ncp = 1.0e-6;
    } else if (ncp > 1) {
      ncp = 1;
    }
    
    // width

    double variance = _missingDbl;
    if (snrDb > 10) {
      variance = -2.0 * log(ncp);
    } else {
      Complex_t lag1_hc_prime =
        Complex::meanConjugateProduct(iqhc, iqhc + 1, _nSamples - 2);
      Complex_t lag2_hc_prime =
        Complex::meanConjugateProduct(iqhc, iqhc + 2, _nSamples - 2);
      variance = (2.0 / 3.0) *
        (Complex::mag(lag1_hc_prime) / Complex::mag(lag2_hc_prime));
    }
    double width = 0.0;
    if (variance >= 0) {
      width = (nyquist / M_PI) * sqrt(variance);
    }
    fields[igate].width = width;

  } // igate

}

/////////////////////////////////////////////////////
// compute moments for CP2 SBAND
//
// IQHC and IQVC data is assumed to be copolar
//
// IQHC[nGates][nPulses]
// IQVC[nGates][nPulses]

void MomentsMgr::computeDualCp2Sband(double beamTime,
                                     double el,
                                     double az,
                                     double prt,
                                     int nGates,
                                     Complex_t **IQHC,
                                     Complex_t **IQVC,
                                     Fields *fields)
  
{

  _checkRangeCorrection(nGates);

  double wavelengthMeters = _params.radar.wavelength_cm / 100.0;
  double nyquist = ((wavelengthMeters / prt) / 4.0);

  // compute phidp offset, to prevent premature wrapping of the phase
  // vectors from which phidp and velocity are computed.
  
  Complex_t phidpOffset;
  if (_params.correct_for_system_phidp) {
    double offset = _params.system_phidp - _phidpPhaseLimit;
    phidpOffset.re = cos(offset * DEG_TO_RAD * -1.0);
    phidpOffset.im = sin(offset * DEG_TO_RAD * -1.0);
  } else {
    phidpOffset.re = 1.0;
    phidpOffset.im = 0.0;
  }
  
  double range = _startRange;
  for (int igate = 0; igate < nGates; igate++, range += _gateSpacing) {

    // locate the IQ samples for this gate

    Complex_t *iqHc = IQHC[igate];
    Complex_t *iqVc = IQVC[igate];

    // apply clutter filter if needed
    // otherwise copy through
    
    TaArray<Complex_t> iqhc_, iqvc_;
    Complex_t *iqhc = iqhc_.alloc(_nSamplesHalf);
    Complex_t *iqvc = iqvc_.alloc(_nSamplesHalf);

    if (_momentsParams.apply_clutter_filter) {
      
      _applyClutterFilter(_nSamplesHalf, *_fftHalf, _momentsHalf,
                          nyquist, iqHc, iqhc);
      _applyClutterFilter(_nSamplesHalf, *_fftHalf, _momentsHalf,
                          nyquist, iqVc, iqvc);

    } else {

      memcpy(iqhc, iqHc, _nSamplesHalf * sizeof(Complex_t));
      memcpy(iqvc, iqVc, _nSamplesHalf * sizeof(Complex_t));

    }
  
    // compute lag 0 covariances
    // use N-1 so all covariances have the same number of points
    
    double lag0_hc_hc = Complex::meanPower(iqhc, _nSamplesHalf - 1);
    double lag0_vc_vc = Complex::meanPower(iqvc, _nSamplesHalf - 1);

    // compute noise-subtracted lag0
    
    double lag0_hc_hc_ns = lag0_hc_hc - _noiseFixedHc;
    double lag0_vc_vc_ns = lag0_vc_vc - _noiseFixedVc;

    if (lag0_hc_hc_ns < 0) { lag0_hc_hc_ns = lag0_hc_hc; }
    if (lag0_vc_vc_ns < 0) { lag0_vc_vc_ns = lag0_vc_vc; }

    // compute dbm

    fields[igate].dbmhc =
      10.0 * log10(lag0_hc_hc) - _params.hc_receiver.gain;
    fields[igate].dbmvc =
      10.0 * log10(lag0_vc_vc) - _params.vc_receiver.gain;
    fields[igate].dbm = (fields[igate].dbmhc + fields[igate].dbmvc) / 2.0;
    
    // compute snr

    double snr_hc = lag0_hc_hc_ns / _noiseFixedHc;
    double snr_vc = lag0_vc_vc_ns / _noiseFixedVc;

    fields[igate].snrhc = 10.0 * log10(snr_hc);
    fields[igate].snrvc = 10.0 * log10(snr_vc);

    double snrMean = (snr_hc + snr_vc) / 2.0;
    double snrDb = 10.0 * log10(snrMean);
    fields[igate].snr = snrDb;
    
    // compute dbz and dbm

    double dbz_hc = 10.0 * log10(snr_hc) + _dbz0Hc + _rangeCorr[igate];
    double dbz_vc = 10.0 * log10(snr_vc) + _dbz0Vc + _rangeCorr[igate];
    fields[igate].dbz = (dbz_hc + dbz_vc) / 2.0;

    // zdr
    
    double zdrm = 10.0 * log10(snr_hc / snr_vc);
    double zdrc = zdrm + _params.zdr_correction;
    fields[igate].zdr = zdrc;
    fields[igate].zdrm = zdrm;

    ////////////////////////////
    // phidp, velocity and rhohv
    //
    // See A. Zahrai and D. Zrnic
    // "The 10 cm wavelength polarimetric weather radar
    // at NOAA'a National Severe Storms Lab. "
    // JTech, Vol 10, No 5, October 1993.

    // compute correlation V to H
    
    Complex_t Ra =
      Complex::meanConjugateProduct(iqvc, iqhc, _nSamplesHalf - 1);

    // compute correlation H to V
    
    Complex_t Rb =
      Complex::meanConjugateProduct(iqhc + 1, iqvc, _nSamplesHalf - 1);
    
    // Correct for system PhiDp offset, so that phidp will not wrap prematurely

    Complex_t RaPrime = Complex::complexProduct(Ra, phidpOffset);
    Complex_t RbPrime = Complex::conjugateProduct(Rb, phidpOffset);
    
    // compute angular difference between them, which is phidp
    
    Complex_t RaRbconj = Complex::conjugateProduct(RaPrime, RbPrime);
    double phidpRad = Complex::argRad(RaRbconj) / 2.0;
    fields[igate].phidp = phidpRad * RAD_TO_DEG;

    // velocity phase is mean of phidp phases

    Complex_t velVect = Complex::complexMean(RaPrime, RbPrime);
    double argVel = Complex::argRad(velVect);
    double meanVel = (argVel / M_PI) * nyquist;
    fields[igate].vel = meanVel * -1.0;

    //////////
    // rhohv
 
    // lag-2 correlations for HH and VV
    
    Complex_t lag2_hc_hc =
      Complex::meanConjugateProduct(iqhc, iqhc + 1, _nSamplesHalf - 1);
    Complex_t lag2_vc_vc =
      Complex::meanConjugateProduct(iqvc, iqvc + 1, _nSamplesHalf - 1);

    // compute lag-2 rho

    Complex_t sumR2 = Complex::complexSum(lag2_hc_hc, lag2_vc_vc);
    double magSumR2 = Complex::mag(sumR2);
    double rho2 = magSumR2 / (lag0_hc_hc_ns + lag0_vc_vc_ns);
    if (rho2 > 1.0) {
      rho2 = 1.0;
    } else if (rho2 < 0) {
      rho2 = 0;
    }

    // compute lag-1 rhohv

    double magRa = Complex::mag(Ra);
    double magRb = Complex::mag(Rb);
    double rhohv1 = (magRa + magRb) / (2.0 * sqrt(lag0_hc_hc * lag0_vc_vc));
    
    // lag-0 rhohv is rhohv1 corrected by rho2
    
    double rhohv0 = rhohv1 / pow(rho2, 0.25);
    fields[igate].rhohv = rhohv0;

    // spectrum width
    
    double argWidth = sqrt(-0.5 * log(rho2));
    double width = (argWidth / M_PI) * nyquist;
    fields[igate].width = width;

  } // igate

  // compute kdp
  
  _computeKdp(nGates, fields);
  
}

/////////////////////////////////////////////////
// compute moments for CP2 XBand
//
// H xmit, H rcv (HC) and V rcv (VX)
//
// IQHC[nGates][nPulses]
// IQVX[nGates][nPulses]

void MomentsMgr::computeDualCp2Xband(double beamTime,
                                     double el,
                                     double az,
                                     double prt,
                                     int nGates,
                                     Complex_t **IQHC,
                                     Complex_t **IQVX,
                                     Fields *fields)

{

  _checkRangeCorrection(nGates);

  double wavelengthMeters = _params.radar.wavelength_cm / 100.0;
  double nyquist = ((wavelengthMeters / prt) / 4.0);

  double range = _startRange;
  for (int igate = 0; igate < nGates; igate++, range += _gateSpacing) {
    
    // locate the IQ samples for this gate

    Complex_t *iqHc = IQHC[igate];
    Complex_t *iqVx = IQVX[igate];

    // apply clutter filter if needed
    // otherwise copy through
    
    TaArray<Complex_t> iqhc_, iqvx_;
    Complex_t *iqhc = iqhc_.alloc(_nSamples);
    Complex_t *iqvx = iqvx_.alloc(_nSamples);

    if (_momentsParams.apply_clutter_filter) {
      
      _applyClutterFilter(_nSamples, *_fft, _moments,
                          nyquist, iqHc, iqhc);
      _applyClutterFilter(_nSamples, *_fft, _moments,
                          nyquist, iqVx, iqvx);
      
    } else {

      memcpy(iqhc, iqHc, _nSamples * sizeof(Complex_t));
      memcpy(iqvx, iqVx, _nSamples * sizeof(Complex_t));

    }
  
    // compute lag 0 covariances
    // use N-1 so all covariances have the same number of points
    
    double lag0_hc_hc = Complex::meanPower(iqhc, _nSamples);
    double lag0_vx_vx = Complex::meanPower(iqvx, _nSamples);

    // compute noise-subtracted lag0
    
    double lag0_hc_hc_ns = lag0_hc_hc - _noiseFixedHc;
    double lag0_vx_vx_ns = lag0_vx_vx - _noiseFixedVx;

    if (lag0_hc_hc_ns < 0) { lag0_hc_hc_ns = lag0_hc_hc; }
    if (lag0_vx_vx_ns < 0) { lag0_vx_vx_ns = lag0_vx_vx; }
    
    // compute dbm

    fields[igate].dbmhc =
      10.0 * log10(lag0_hc_hc) - _params.hc_receiver.gain;
    fields[igate].dbm = fields[igate].dbmhc;
    
    fields[igate].dbmvx =
      10.0 * log10(lag0_vx_vx) - _params.vx_receiver.gain;

    // compute snr
    
    double snr_hc = lag0_hc_hc_ns / _noiseFixedHc;
    double snr_vx = lag0_vx_vx_ns / _noiseFixedVx;

    double snrDb = 10.0 * log10(snr_hc);
    fields[igate].snr = snrDb;
    fields[igate].snrhc = snrDb;
    fields[igate].snrvx = 10.0 * log10(snr_vx);
    
    // compute dbz and dbm
    
    double dbz_hc = 10.0 * log10(snr_hc) + _dbz0Hc + _rangeCorr[igate];
    fields[igate].dbz = dbz_hc;

    // velocity

    Complex_t lag1_hc_hc =
      Complex::meanConjugateProduct(iqhc, iqhc + 1, _nSamples - 1);

    double argVel = Complex::argRad(lag1_hc_hc);
    double vel = (argVel / M_PI) * nyquist;
    fields[igate].vel = vel * -1.0;

    // ncp

    double magLag1Hc = Complex::mag(lag1_hc_hc);
    double ncp = magLag1Hc / lag0_hc_hc;
    if (ncp < 1.0e-6) {
      ncp = 1.0e-6;
    } else if (ncp > 1) {
      ncp = 1;
    }
    
    // width

    double variance = _missingDbl;
    if (snrDb > 10) {
      variance = -2.0 * log(ncp);
    } else {
      Complex_t lag1_hc_prime =
        Complex::meanConjugateProduct(iqhc, iqhc + 1, _nSamples - 2);
      Complex_t lag2_hc_prime =
        Complex::meanConjugateProduct(iqhc, iqhc + 2, _nSamples - 2);
      variance = (2.0 / 3.0) *
        (Complex::mag(lag1_hc_prime) / Complex::mag(lag2_hc_prime));
    }
    double width = 0.0;
    if (variance >= 0) {
      width = (nyquist / M_PI) * sqrt(variance);
    }
    fields[igate].width = width;

    // ldr

    double ldr = 10.0 * log10(snr_vx / snr_hc);
    fields[igate].ldrh = ldr;
    fields[igate].ldrv = ldr;

  } // igate

}

/////////////////////////////////////////////
// check range correction table is up to date

void MomentsMgr::_checkRangeCorrection(int nGates)

{

	if (_nGates >= nGates) {
		return;
	}

	_nGates = nGates;

	if (_rangeCorr) {
		delete[] _rangeCorr;
	}
	_rangeCorr = new double[_nGates];
        
	for (int i = 0; i < _nGates; i++) {
		double range_km = _startRange + i * _gateSpacing;
                if (range_km > 0) {
                  _rangeCorr[i] = _params.dbz_calib_correction +
                    20.0 * log10(range_km) +
                    range_km * _params.atmos_attenuation;
                } else {
                  _rangeCorr[i] = _params.dbz_calib_correction +
                    range_km * _params.atmos_attenuation;
                }
	}

}

///////////////////////////////////////
// compute mean velocity

double MomentsMgr::_computeMeanVelocity(double vel_0,
										double vel_1,
										double nyquist) const

{

	double diff = fabs(vel_0 - vel_1);
	if (diff < nyquist) {
		return (vel_0 + vel_1) / 2.0;
	} else {
		if (vel_0 > vel_1) {
			vel_1 += 2.0 * nyquist;
		} else {
			vel_0 += 2.0 * nyquist;
		}
		double vel = (vel_0 + vel_1) / 2.0;
		if (vel > nyquist) {
			vel -= 2.0 * nyquist;
		} else if (vel < -nyquist) {
			vel += 2.0 * nyquist;
		}
		return vel;
	}

}

/////////////////////////////////////////
// compute velocity difference as an angle

double MomentsMgr::_velDiff2Angle(double vel_0, double vel_1,
								  double nyquist) const

{

	double ang1 = (vel_0 / nyquist) * 180.0;
	double ang2 = (vel_1 / nyquist) * 180.0;
	double adiff = (ang1 - ang2);
	if (adiff > 180.0) {
		adiff -= 360.0;
	} else if (adiff < -180.0) {
		adiff += 360.0;
	}
	return adiff;
}

/////////////////////////////////////////
// sum complex values

Complex_t MomentsMgr::_complexSum(Complex_t c1, Complex_t c2) const

{
	Complex_t sum;
	sum.re = c1.re + c2.re;
	sum.im = c1.im + c2.im;
	return sum;
}

/////////////////////////////////////////
// mean of complex values

Complex_t MomentsMgr::_complexMean(Complex_t c1, Complex_t c2) const

{
	Complex_t mean;
	mean.re = (c1.re + c2.re) / 2.0;
	mean.im = (c1.im + c2.im) / 2.0;
	return mean;
}

/////////////////////////////////////////
// multiply complex values

Complex_t MomentsMgr::_complexProduct(Complex_t c1, Complex_t c2) const

{
	Complex_t product;
	product.re = (c1.re * c2.re) - (c1.im * c2.im);
	product.im = (c1.im * c2.re) + (c1.re * c2.im);
	return product;
}

/////////////////////////////////////////
// compute conjugate product

Complex_t MomentsMgr::_conjugateProduct(Complex_t c1,
										Complex_t c2) const

{
	Complex_t product;
	product.re = (c1.re * c2.re) + (c1.im * c2.im);
	product.im = (c1.im * c2.re) - (c1.re * c2.im);
	return product;
}

/////////////////////////////////////////////
// compute mean complex product of series

Complex_t MomentsMgr::_meanComplexProduct(const Complex_t *c1,
										  const Complex_t *c2,
										  int len) const

{

	double sumRe = 0.0;
	double sumIm = 0.0;

	for (int ipos = 0; ipos < len; ipos++, c1++, c2++) {
		sumRe += ((c1->re * c2->re) - (c1->im * c2->im));
		sumIm += ((c1->im * c2->re) + (c1->re * c2->im));
	}

	Complex_t product;
	product.re = sumRe / len;
	product.im = sumIm / len;

	return product;

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

/////////////////////////////////////////
// get velocity from angle of complex val

double MomentsMgr::_velFromComplex(Complex_t cc, double nyquist) const

{
	double arg = 0.0;
	if (cc.re != 0.0 || cc.im != 0.0) {
		arg = atan2(cc.im, cc.re);
	}
	double vel = (arg / M_PI) * nyquist;
	return vel;
}

/////////////////////////////////////////
// get velocity from arg

double MomentsMgr::_velFromArg(double arg, double nyquist) const

{
	return (arg / M_PI) * nyquist;
}

////////////////////////////////////////////
// compute phase difference in complex space

Complex_t MomentsMgr::_computeComplexDiff(const Complex_t &c1,
										  const Complex_t &c2) const

{
	Complex_t diff;
	diff.re = (c1.re * c2.re + c1.im * c2.im);
	diff.im = (c1.im * c2.re - c1.re * c2.im);
	return diff;
}

//////////////////////////////////////
// compute phase difference in radians

double MomentsMgr::_computeArgDiff(const Complex_t &c1,
								   const Complex_t &c2) const

{

	Complex_t diff;

	diff.re = (c1.re * c2.re + c1.im * c2.im);
	diff.im = (c1.im * c2.re - c1.re * c2.im);

	double angDiff = 0.0;
	if (diff.im != 0.0 && diff.re != 0.0) {
		angDiff = atan2(diff.im, diff.re);
	}
	return angDiff;

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

//////////////////////////////////////
// compute magnitude

double MomentsMgr::_computeMag(const Complex_t &cc) const

{
	return sqrt(cc.re * cc.re + cc.im * cc.im);
}

//////////////////////////////////////
// compute power

double MomentsMgr::_computePower(const Complex_t &cc) const

{
	return cc.re * cc.re + cc.im * cc.im;
}

/////////////////////////////////////////////
// compute mean power of series

double MomentsMgr::_meanPower(const Complex_t *c1, int len) const

{
	double sum = 0.0;
	for (int ipos = 0; ipos < len; ipos++, c1++) {
		sum += ((c1->re * c1->re) + (c1->im * c1->im));
	}
	return sum / len;
}

/////////////////////////////////////////////
// compute kdp from phidp

void MomentsMgr::_computeKdp(int nGates,
                             Fields *fields)
  
{

  // KDP processing from CHILL

  if (nGates < KDP_PROC_START_OFFSET * 2) {
    return;
  }

  // copy from Fields into array

  TaArray<double> snr_, dbz_, phidp_, rhohv_;
  double *snr = snr_.alloc(nGates);
  double *dbz = dbz_.alloc(nGates);
  double *phidp = phidp_.alloc(nGates);
  double *rhohv = rhohv_.alloc(nGates);
  for (int ii = 0; ii < nGates; ii++) {
    snr[ii] = fields[ii].snr;
    dbz[ii] = fields[ii].dbz;
    phidp[ii] = fields[ii].phidp;
    rhohv[ii] = fields[ii].rhohv;
  }

  // set up arrays for phidp, rhohv
  
  TaArray<double> rangeKm_;
  double *rangeKm = rangeKm_.alloc(nGates);
  double range = _startRange;
  for (int ii = 0; ii < nGates; ii++, range += _gateSpacing) {
    rangeKm[ii] = range;
  }

  // compute SD of phidp

  TaArray<double> phidpSd_;
  double *phidpSd = phidpSd_.alloc(nGates);
  pac_get_phidp_sd(nGates, phidp, 1, phidpSd, 1, 10);

  // unpack phidp

  int proc_start_gate = KDP_PROC_START_OFFSET;
  TaArray<double> phidpU_;
  double *phidpU = phidpU_.alloc(nGates);
  memcpy(phidpU, phidp, nGates * sizeof(double));
  double sysPhase = NAN;
  pac_unfold_phidp(nGates,
                   phidp, 1,
                   rhohv, 1,
                   phidpSd, 1,
                   snr, 1,
                   rangeKm, 1,
                   PAC_UNFOLD_AUTO, proc_start_gate,
                   sysPhase, 90.0,
                   phidpU, 1);

  TaArray<char> dataMask_;
  char *dataMask = dataMask_.alloc(nGates);

  pac_get_datamask(nGates,
                   dbz, 1,
                   phidpU, 1,
                   rhohv, 1,
                   snr, 1,
                   rangeKm, 1,
                   25,
                   dataMask, 1);
  
  // smooth_phidp returns 0 if there is valid data

  TaArray<double> phidpF_, phidpI_;
  double *phidpF = phidpF_.alloc(nGates);
  double *phidpI = phidpI_.alloc(nGates);

  if (0 == pac_smooth_phidp(nGates,
                            phidpU, 1,
                            rangeKm, 1,
                            dataMask, 1,
                            phidpI, 1,
                            phidpF, 1)) {

    TaArray<double> kdp_;
    double *kdp = kdp_.alloc(nGates);

    pac_calc_kdp(nGates,
                 phidpF, 1,
                 dbz, 1,
                 rangeKm, 1,
                 kdp, 1);
    
    // put KDP into fields objects

    for (int ii = 0; ii < nGates; ii++) {
      fields[ii].kdp = kdp[ii];
    }

  }

}

/////////////////////////////////////////////////////
// apply clutter filter
//
// IQHC and IQVC data is assumed to be copolar
//
// IQHC[nGates][nPulses]
// IQVC[nGates][nPulses]

void MomentsMgr::_applyClutterFilter(int nSamples,
                                     Fft &fft,
                                     Moments &moments,
                                     double nyquist,
                                     const Complex_t *iq,
                                     Complex_t *iqFiltered)
  
{
  
  // apply the window
  
  if (_fftWindow == Moments::WINDOW_VONHANN) {
    moments.applyVonhannWindow(iq, iqFiltered);
  } else if (_fftWindow == Moments::WINDOW_BLACKMAN) {
    moments.applyBlackmanWindow(iq, iqFiltered);
  } else {
    moments.applyRectWindow(iq, iqFiltered);
  }

  // take the forward fft

  TaArray<Complex_t> spec_;
  Complex_t *spec = spec_.alloc(nSamples);
  fft.fwd(iqFiltered, spec);

  // compute the magnitude spectrum

  TaArray<double> specMag_;
  double *specMag = specMag_.alloc(nSamples);
  Moments::loadMag(nSamples, spec, specMag);

  // compute filtered magnitude
  
  TaArray<double> filtMag_;
  double *filtMag = filtMag_.alloc(nSamples);


  double maxClutterVel = 1.0;
  double initNotchWidth = 1.5;
  bool clutterFound = false;
  int notchStart = 0;
  int notchEnd = 0;
  double powerRemoved = 0.0;
  double filteredVel = 0.0;
  double filteredWidth = 0.0;

  _clutFilter.run(specMag, nSamples,
                  maxClutterVel, initNotchWidth, nyquist,
                  filtMag, clutterFound, notchStart, notchEnd,
                  powerRemoved, filteredVel, filteredWidth);

  // compute clutter power correction, to reduce the effect of
  // the raised noise floor
  
  double pwr = _meanPower(iq, nSamples);
  double powerRemovedCorrection = 1.0;
  double dbForDbRatio = 0.1;
  double dbForDbThreshold = 50.0;

  if (powerRemoved > 0) {
    double powerTotalDb = 10.0 * log10(pwr + powerRemoved);
    double powerLeftDb = 10.0 * log10(pwr);
    double diffDb = powerTotalDb - powerLeftDb;
    double powerRemovedDbCorrection = diffDb * dbForDbRatio;
    if (diffDb > dbForDbThreshold) {
      powerRemovedDbCorrection += (diffDb - dbForDbThreshold);
    }
    powerRemovedCorrection = pow(10.0, powerRemovedDbCorrection / 10.0);
  }

  // correct the filtered magnitudes

  for (int ii = 0; ii < nSamples; ii++) {
    filtMag[ii] /= powerRemovedCorrection;
  }

  // adjust the spectrum by the filter ratio

  for (int ii = 0; ii < nSamples; ii++) {
    double ratio = filtMag[ii] / specMag[ii];
    spec[ii].re *= ratio;
    spec[ii].im *= ratio;
  }

  // invert the fft

  fft.inv(spec, iqFiltered);
 
}

