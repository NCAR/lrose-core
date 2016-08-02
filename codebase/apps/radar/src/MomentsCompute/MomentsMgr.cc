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
using namespace std;

const double MomentsMgr::_missingDbl = -9999.0;

#ifndef RAD_TO_DEG
#define RAD_TO_DEG 57.29577951308092
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

	_noiseFixedHc = pow(10.0, _params.hc_receiver.noise_dBm / 10.0);
	_noiseFixedHx = pow(10.0, _params.hx_receiver.noise_dBm / 10.0);
	_noiseFixedVc = pow(10.0, _params.vc_receiver.noise_dBm / 10.0);
	_noiseFixedVx = pow(10.0, _params.vx_receiver.noise_dBm / 10.0);

	_dbz0Hc = _params.hc_receiver.dbz0;
	_dbz0Hx = _params.hx_receiver.dbz0;
	_dbz0Vc = _params.vc_receiver.dbz0;
	_dbz0Vx = _params.vx_receiver.dbz0;

	// set window type

	if (_momentsParams.window == Params::WINDOW_NONE) {
		_fftWindow = Moments::WINDOW_NONE;
	} else if (_momentsParams.window == Params::WINDOW_HANNING) {
		_fftWindow = Moments::WINDOW_HANNING;
	} else if (_momentsParams.window == Params::WINDOW_BLACKMAN) {
		_fftWindow = Moments::WINDOW_BLACKMAN;
	}

	// initialize moments objects

	_moments.setWavelength(_params.radar.wavelength_cm / 100.0);
	_momentsHalf.setWavelength(_params.radar.wavelength_cm / 100.0);

        _moments.setNoiseValueDbm(_params.hc_receiver.noise_dBm);
        _momentsHalf.setNoiseValueDbm(_params.hc_receiver.noise_dBm);
  
}

//////////////////////////////////////////////////////////////////
// destructor

MomentsMgr::~MomentsMgr()

{

	if (_rangeCorr) {
		delete[] _rangeCorr;
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
							   Complex_t **IQC,
							   Fields *fields)

{

	_checkRangeCorrection(nGates);

	double range = _startRange;
	for (int igate = 0; igate < nGates; igate++, range += _gateSpacing) {

		Complex_t *iq = IQC[igate];

		double power = _missingDbl;
		double vel = _missingDbl;
		double width = _missingDbl;

                if (_momentsParams.algorithm == Params::ALG_FFT) {
                  _moments.computeByFft(iq, _fftWindow, prt,
                                        power, vel, width);
                } else if (_momentsParams.algorithm == Params::ALG_PP) {
                  _moments.computeByPp(iq, prt,
                                       power, vel, width);
                } else if (_momentsParams.algorithm == Params::ALG_ABP) {
                  _moments.computeByAbp(iq, prt,
                                        power, vel, width);
                }

		double dbm = _missingDbl;
		if (power != _missingDbl) {
			dbm = 10.0 * log10(power);
			fields[igate].dbm = dbm;
			if (power <= _noiseFixedHc) {
				power = _noiseFixedHc + 1.0e-20;
			}
			double snr =
				10.0 * log10((power - _noiseFixedHc) / _noiseFixedHc);
			fields[igate].snr = snr;
			fields[igate].dbz = snr + _params.hc_receiver.dbz0 + _rangeCorr[igate];
		}

		fields[igate].vel = vel;
		fields[igate].width = width;

	} // igate

}

/////////////////////////////////////////////////////
// compute moments in fast alternating dual pol mode
//
// IQHC and IQVC data is assumed to be copolar
//
// IQHC[nGates][nPulses]
// IQVC[nGates][nPulses]

void MomentsMgr::computeDualFastAlt(double beamTime,
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

	double range = _startRange;
	for (int igate = 0; igate < nGates; igate++, range += _gateSpacing) {

		Complex_t *iqh = IQHC[igate];
		Complex_t *iqv = IQVC[igate];

		// compute power, vel and width for each channel

		double power_h = _missingDbl, vel_h = _missingDbl, width_h = _missingDbl;
		double power_v = _missingDbl, vel_v = _missingDbl, width_v = _missingDbl;

                if (_momentsParams.algorithm == Params::ALG_FFT) {
                  _momentsHalf.computeByFft(iqh, _fftWindow, prt,
                                            power_h, vel_h, width_h);
                  _momentsHalf.computeByFft(iqv, _fftWindow, prt,
                                            power_v, vel_v, width_v);
                } else if (_momentsParams.algorithm == Params::ALG_PP) {
                  _momentsHalf.computeByPp(iqh, prt, power_h, vel_h, width_h);
                  _momentsHalf.computeByPp(iqv, prt, power_v, vel_v, width_v);
                } else if (_momentsParams.algorithm == Params::ALG_ABP) {
                  _momentsHalf.computeByAbp(iqh, prt, power_h, vel_h, width_h);
                  _momentsHalf.computeByAbp(iqv, prt, power_v, vel_v, width_v);
                }

		// compute snr

		double snr_h = _missingDbl;
		double dbm_h = 10.0 * log10(power_h);
		if (power_h <= _noiseFixedHc) {
			power_h = _noiseFixedHc + 1.0e-20;
		}

		fields[igate].dbmhc = dbm_h;

		snr_h = 10.0 * log10((power_h - _noiseFixedHc) / _noiseFixedHc);

		double snr_v = _missingDbl;
		double dbm_v = 10.0 * log10(power_v);
		if (power_v <= _noiseFixedVc) {
			power_v = _noiseFixedVc + 1.0e-20;
		}

		fields[igate].dbmvc = dbm_v;

		snr_v = 10.0 * log10((power_v - _noiseFixedVc) / _noiseFixedVc);

		// average power and SNR

		double dbmMean = (dbm_h + dbm_v) / 2.0;
		double snrMean = (snr_h + snr_v) / 2.0;
		fields[igate].dbm = dbmMean;
		fields[igate].snr = snrMean;

		double dbz_h = snr_h + _dbz0Hc + _rangeCorr[igate];
		double dbz_v = snr_v + _dbz0Vc + _rangeCorr[igate];
		fields[igate].dbz = (dbz_h + dbz_v) / 2.0;
		fields[igate].dbzhc = dbz_h;
		fields[igate].dbzvc = dbz_v;

		// width from half-spectra

		fields[igate].width = (width_h + width_v) / 2.0;

		// zdr

		fields[igate].zdr = dbz_h - dbz_v + _params.zdr_correction;
		fields[igate].zdrm = snr_h - snr_v;

		// phidp and velocity

		Complex_t Rhhvv1 = _meanConjugateProduct(iqv, iqh + 1,
			_nSamplesHalf - 1);
		Complex_t Rvvhh1 = _meanConjugateProduct(iqh + 1, iqv + 1,
			_nSamplesHalf - 1);

		double argRhhvv1 = _computeArg(Rhhvv1);
		double argRvvhh1 = _computeArg(Rvvhh1);

		double phiDpRad = (argRhhvv1 - argRvvhh1) / 2.0;
		fields[igate].phidp = phiDpRad * RAD_TO_DEG;

		double argVelhhvv = phiDpRad - argRhhvv1;
		double argVelvvhh = - phiDpRad - argRvvhh1;
		double meanArgVel = (argVelhhvv + argVelvvhh) / 2.0;
		double meanVel = (meanArgVel / M_PI) * nyquist;
		fields[igate].vel = meanVel* -1.0;

		// rhohv

		double Phh = _meanPower(iqh + 1, _nSamplesHalf - 1);
		double Pvv = _meanPower(iqv + 1, _nSamplesHalf - 1);
		double rhohhvv1 = _computeMag(Rhhvv1) / sqrt(Phh * Pvv);

		Complex_t Rhhhh2 = _meanConjugateProduct(iqh, iqh + 1, _nSamplesHalf - 1);
		Complex_t Rvvvv2 = _meanConjugateProduct(iqv, iqv + 1, _nSamplesHalf - 1);
		double rhohh2 = _computeMag(Rhhhh2) / Phh;

		double rhohhvv0 = rhohhvv1 / pow(rhohh2, 0.25);
		if (rhohhvv0 > 1.0) {
			rhohhvv0 = 1.0;
		}
		fields[igate].rhohv = rhohhvv0;

	} // igate

	// compute kdp

	// Kdp disabled until a better algorithm is implemented
	//	_computeKdp(nGates, fields);

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

	double range = _startRange;
	for (int igate = 0; igate < nGates; igate++, range += _gateSpacing) {

		// Moments from HC

		Complex_t *iqhc = IQHC[igate];

		double power_hc = _missingDbl;
		double vel_hc = _missingDbl;
		double width_hc = _missingDbl;

                if (_momentsParams.algorithm == Params::ALG_FFT) {
                  _moments.computeByFft(iqhc, _fftWindow, prt,
                                        power_hc, vel_hc, width_hc);
                } else if (_momentsParams.algorithm == Params::ALG_PP) {
                  _moments.computeByPp(iqhc, prt,
                                       power_hc, vel_hc, width_hc);
                } else if (_momentsParams.algorithm == Params::ALG_ABP) {
                  _moments.computeByAbp(iqhc, prt,
                                        power_hc, vel_hc, width_hc);
                }

		double dbz_hc = _missingDbl;
		if (power_hc != _missingDbl) {
			double dbm_hc = 10.0 * log10(power_hc);
			fields[igate].dbm = dbm_hc;
			fields[igate].dbmhc = dbm_hc;
			if (power_hc <= _noiseFixedHc) {
				power_hc = _noiseFixedHc + 1.0e-20;
			}
			double snr_hc =
				10.0 * log10((power_hc - _noiseFixedHc) / _noiseFixedHc);
			fields[igate].snr = snr_hc;
			dbz_hc =
				snr_hc + _params.hc_receiver.dbz0 + _rangeCorr[igate];
			fields[igate].dbz = dbz_hc;
			fields[igate].dbzhc = dbz_hc;
		}

		fields[igate].vel = vel_hc;
		fields[igate].width = width_hc;

		// LDR from HC - VX

		Complex_t *iqvx = IQVX[igate];
		double power_vx = _moments.computePower(iqvx);

		if (power_vx != _missingDbl) {
			if (power_vx <= _noiseFixedVx) {
				power_vx = _noiseFixedVx + 1.0e-20;
			}
			double dbm_vx = 10.0 * log10(power_vx);
			fields[igate].dbmvx = dbm_vx;
			double snr_vx =
				10.0 * log10((power_vx - _noiseFixedVx) / _noiseFixedVx);
			double dbz_vx =
				snr_vx + _params.vx_receiver.dbz0 + _rangeCorr[igate];
			fields[igate].dbzvx = dbz_vx;
			double ldr = dbz_vx - dbz_hc + _params.ldr_correction;
			fields[igate].ldrh = ldr;
		}
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
		_rangeCorr[i] = _params.dbz_calib_correction +
			20.0 * log10(range_km) +
			range_km * _params.atmos_attenuation;
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

//void MomentsMgr::_computeKdp(int nGates,
//							 Fields *fields) const
//
//{

//	int nGatesForSlope = 15;
//	int nGatesHalf = nGatesForSlope / 2;
//
//	if (nGatesForSlope > nGates) {
//		return;
//	}
//
//	for (int ii = nGatesHalf; ii < nGates - nGatesHalf; ii++) {
//
//		if (fields[ii].phidp != _missingDbl) {
//			double slope =
//				_computePhidpSlope(ii, nGatesForSlope, nGatesHalf, fields);
//			if (slope != _missingDbl) {
//				fields[ii].kdp = slope / 2.0; // deg/km
//			}
//		}
//
//	}
//
//}

//////////////////////////////////////////////////
// compute PhiDp slope
//
// returns _missingDbl if not enough data

//double MomentsMgr::_computePhidpSlope(int index,
//									  int nGatesForSlope,
//									  int nGatesHalf,
//									  const Fields *fields) const
//
//{
//
//	double *xx = new double[nGatesForSlope];
//	double *yy = new double[nGatesForSlope];
//	double *wt = new double[nGatesForSlope];
//	int count = 0;
//
//	double pdpThisGate = fields[index].phidp;
//	double range = 0.0;
//
//	for (int ii = index - nGatesHalf; ii <= index + nGatesHalf;
//		ii++, range += _gateSpacing) {
//			double snrDb = _missingDbl;
//			double snr = fields[ii].snr;
//			double pdp = fields[ii].phidp;
//			if (snr != _missingDbl) {
//				snrDb = pow(10.0, snr);
//			}
//			if (pdp != _missingDbl && snrDb != _missingDbl && snrDb > 5) {
//				double diff = pdp - pdpThisGate;
//				if (diff > 180.0) {
//					pdp -= 360.0;
//				} else if (diff < -180) {
//					pdp += 360.0;
//				}
//				xx[count] = range;
//				yy[count] = pdp;
//				wt[count] = pow(10.0, snr);
//				count++;
//			}
//		}
//
//		if (count < nGatesHalf) {
//			delete[] xx;
//			delete[] yy;
//			delete[] wt;
//			return _missingDbl;
//		}
//
//		// apply median filter to phidp
//
//		double *yyMed = new double[count];
//		yyMed[0] = yy[0];
//		yyMed[count - 1] = yy[count - 1];
//		for (int ii = 1; ii < count - 1; ii++) {
//			double yy0 = yy[ii - 1];
//			double yy1 = yy[ii];
//			double yy2 = yy[ii + 1];
//			if (yy0 > yy1) {
//				if (yy1 > yy2) {
//					yyMed[ii] = yy1;
//				} else {
//					if (yy0 > yy2) {
//						yyMed[ii] = yy2;
//					} else {
//						yyMed[ii] = yy0;
//					}
//				}
//			} else {
//				if (yy0 > yy2) {
//					yyMed[ii] = yy0;
//				} else {
//					if (yy1 > yy2) {
//						yyMed[ii] = yy2;
//					} else {
//						yyMed[ii] = yy1;
//					}
//				}
//			}
//		}
//		delete[] yyMed;
//
//		// sum up terms
//
//		double sumx = 0.0;
//		double sumy = 0.0;
//		double sumx2 = 0.0;
//		double sumy2 = 0.0;
//		double sumxy = 0.0;
//		double sumwt = 0.0;
//
//		for (int ii = 0; ii < count; ii++) {
//			double xxx = xx[ii];
//			double yyy = yy[ii];
//			double weight= wt[ii];
//			sumx += xxx * weight;
//			sumx2 += xxx * xxx * weight;
//			sumy += yyy * weight;
//			sumy2 += yyy * yyy * weight;
//			sumxy += xxx * yyy * weight;
//			sumwt += weight;
//		}
//
//		// compute y-on-x slope
//
//		double num = sumwt * sumxy - sumx * sumy;
//		double denom = sumwt * sumx2 - sumx * sumx;
//		double slope_y_on_x;
//
//		if (denom != 0.0) {
//			slope_y_on_x = num / denom;
//		} else {
//			slope_y_on_x = 0.0;
//		}
//
//		// get x-on-y slope
//
//		denom = sumwt * sumy2 - sumy * sumy;
//		double slope_x_on_y;
//
//		if (denom != 0.0) {
//			slope_x_on_y = num / denom;
//		} else {
//			slope_x_on_y = 0.0;
//		}
//
//		// average slopes
//
//		double slope = 0.0;
//		if (slope_y_on_x != 0.0 && slope_x_on_y != 0.0) {
//			slope = (slope_y_on_x + 1.0 / slope_x_on_y) / 2.0;
//		} else if (slope_y_on_x != 0.0) {
//			slope = slope_y_on_x;
//		} else if (slope_x_on_y != 0.0) {
//			slope = 1.0 / slope_x_on_y;
//		}
//
//		delete[] xx;
//		delete[] yy;
//		delete[] wt;
//
//		return slope;
//
//}
