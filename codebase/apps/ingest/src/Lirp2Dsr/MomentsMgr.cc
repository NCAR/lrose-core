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
#include "sz864.h"
#include "MomentsMgr.hh"
using namespace std;

bool MomentsMgr::_zvFlagReady = false;
int MomentsMgr::_zvFlag = 0;
int MomentsMgr::_nZdr = 0;
double MomentsMgr::_sumZdr = 0.0;

////////////////////////////////////////////////////
// Constructor

MomentsMgr::MomentsMgr(const string &prog_name,
		       const Params &params,
		       Params::moments_params_t &moments_params,
		       int n_samples,
		       int max_gates) :
  _progName(prog_name),
  _params(params),
  _lowerPrf(moments_params.lower_prf),
  _upperPrf(moments_params.upper_prf),
  _pulseWidth(moments_params.pulse_width),
  _startRange(moments_params.start_range),
  _gateSpacing(moments_params.gate_spacing),
  _maxGates(max_gates),
  _useFft(moments_params.algorithm == Params::ALG_FFT),
  _applySz(moments_params.apply_sz),
  _dualPol(moments_params.dual_pol),
  _useCForSz(_params.use_c_for_sz),
  _fftWindow(Moments::WINDOW_NONE),
  _nSamples(n_samples),
  _rangeCorr(NULL),
  _minDbzAt1km(_params.radar.min_dbz_at_1km),
  _atmosAtten(_params.atmos_attenuation),
  _moments(_nSamples),
  _momentsHalf(_nSamples / 2)
  
{

  _rangeCorr = new double[_maxGates];
  _computeRangeCorrection();

  if (moments_params.window == Params::WINDOW_NONE) {
    _fftWindow = Moments::WINDOW_NONE;
  } else if (moments_params.window == Params::WINDOW_HANNING) {
    _fftWindow = Moments::WINDOW_HANNING;
  } else if (moments_params.window == Params::WINDOW_BLACKMAN) {
    _fftWindow = Moments::WINDOW_BLACKMAN;
  }

  // noise measurements

  _noiseHistMin = -120.0;
  _noiseHistMax = -60.0;
  _noiseHistDelta = 0.01;
  _noiseHistSize = (int) ((_noiseHistMax - _noiseHistMin) / _noiseHistDelta + 10);
  _noiseHist = new double[_noiseHistSize];
  memset(_noiseHist, 0, _noiseHistSize * sizeof(double));
  _noiseHistTotalCount = 0.0;
  _noiseHistComputeCount = 0;
  _noiseFixed = pow(10.0, _params.radar.noise_value / 10.0);
  _measuredNoiseDbm = _params.radar.noise_value;
  _measuredNoise = _noiseFixed;

  // dual pol phase difference

  double angDiff = _params.dual_pol_h_v_phase_diff * DEG_TO_RAD;
  _dualPolPhaseDiff.re = cos(angDiff);
  _dualPolPhaseDiff.im = sin(angDiff);
  _dualPolPhaseDiffSum.re = _dualPolPhaseDiff.re * 100.0;
  _dualPolPhaseDiffSum.im = _dualPolPhaseDiff.im * 100.0;
  _dualPolPhaseDiffCount = 100.0;

  // initialize moments object

  _moments.setWavelength(_params.radar.wavelength / 100.0);
  _momentsHalf.setWavelength(_params.radar.wavelength / 100.0);
  
  _moments.setNoiseValueDbm(_params.radar.noise_value);
  _momentsHalf.setNoiseValueDbm(_params.radar.noise_value);
  
  _moments.setSignalToNoiseRatioThreshold
    (_params.signal_to_noise_ratio_threshold);
  _momentsHalf.setSignalToNoiseRatioThreshold
    (_params.signal_to_noise_ratio_threshold);
  
  if (_params.negate_phase_codes) {
    _moments.setSzNegatePhaseCodes();
  }
  if (_params.sz_window == Params::HANNING) {
    _moments.setSzWindow(Moments::WINDOW_HANNING);
  } else if (_params.sz_window == Params::BLACKMAN) {
    _moments.setSzWindow(Moments::WINDOW_BLACKMAN);
  }
  
  _moments.setSzStrongToWeakPowerRatioThreshold
    (_params.sz_strong_to_weak_power_ratio_threshold);
  _moments.setSzOutOfTripPowerRatioThreshold
    (_params.sz_out_of_trip_power_ratio_threshold);
  _moments.setSzOutOfTripPowerNReplicas
    (_params.sz_out_of_trip_power_n_replicas);

}

//////////////////////////////////////////////////////////////////
// destructor

MomentsMgr::~MomentsMgr()

{

  if (_rangeCorr) {
    delete[] _rangeCorr;
  }

  if (_noiseHist) {
    delete[] _noiseHist;
  }

}

/////////////////////////////////////////////////
// compute moments

void MomentsMgr::computeMoments(time_t beamTime,
				double el, double az, double prt,
				int nGatesPulse,
				Complex_t **IQ,
				int &combinedPrintCount,
				FILE *combinedSpectraFile,
				double *powerDbm, double *snr, double *dbz,
				double *noiseDbm, double *snrn, double *dbzn,
				double *vel, double *width,
				int *flags,
                                double *clutDbzNarrow,
                                double *clutRatioNarrow,
                                double *clutRatioWide,
                                double *clutWxPeakRatio,
                                double *clutWxPeakSep,
				vector<GateSpectra *> gateSpectra) const
  
{
  
  if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
    cerr << "-->> computing moments, el, az, nGatesPulse, time: "
  	 << el << ", " << az << ", " << nGatesPulse << ", "
  	 << DateTime::str(beamTime) << endl;
  }

  double range = _startRange;
  for (int igate = 0; igate < nGatesPulse; igate++, range += _gateSpacing) {
    
    if (_params.print_summary) {
      break;
    }
    
    bool doWriteCombinedSpectra =
      _setMomentsDebugPrint(_moments, el, az, range);
    
    Complex_t *iq = IQ[igate];
    
    double power1 = _missingDbl;
    double vel1 = _missingDbl, width1 = _missingDbl;
    double noise1 = _missingDbl;
    int flags1 = 0;
    ClutProb clutProb1;

    if (_useFft) {
      
      // FFT
      
      _moments.computeByFft(iq, _fftWindow, prt, power1, noise1,
			    vel1, width1, flags1, &clutProb1);
      _computeNoiseStats(power1);
      
    } else {
      
      // Pulse Pair
      
      _moments.computeByPp(iq, prt, power1, vel1, width1, flags1);
      _computeNoiseStats(power1);

    } // (if (_useFft)

    double dbm = _missingDbl;
    if (power1 != _missingDbl) {
      dbm = 10.0 * log10(power1);
      powerDbm[igate] = dbm;
      if (power1 > _noiseFixed) {
	double snr1 = 10.0 * log10((power1 - _noiseFixed) / _noiseFixed);
	snr[igate] = snr1;
	dbz[igate] = snr1 + _rangeCorr[igate];
      }
      if (power1 > noise1) {
	double snrn1 = 10.0 * log10((power1 - noise1) / noise1);
	snrn[igate] = snrn1;
	dbzn[igate] = snrn1 + _rangeCorr[igate];
      }
    }
    
    if (noise1 != _missingDbl) {
      noiseDbm[igate] =
 	10.0 * log10((noise1 - _measuredNoise) / _measuredNoise);
    }

    vel[igate] = vel1;
    width[igate] = width1;
    flags[igate] = flags1;

    if (_useFft) {
      
      if (clutDbzNarrow != NULL) {
        double powerNarrow = clutProb1.getPowerNarrow();
	double snrNarrow =
          10.0 * log10((powerNarrow - _noiseFixed) / _noiseFixed);
        clutDbzNarrow[igate] = snrNarrow + _rangeCorr[igate];
      }
      if (clutRatioNarrow != NULL) {
        clutRatioNarrow[igate] = clutProb1.getRatioNarrow();
      }
      if (clutRatioWide != NULL) {
        clutRatioWide[igate] = clutProb1.getRatioWide();
      }
      if (clutWxPeakRatio != NULL) {
        clutWxPeakRatio[igate] = clutProb1.getClutWxPeakRatio();
      }
      if (clutWxPeakSep != NULL) {
        clutWxPeakSep[igate] = clutProb1.getClutWxPeakSeparation();
      }
      
    }

    // add spectrum to output file if required
    
    if (doWriteCombinedSpectra && (combinedSpectraFile != NULL)) {
      if (dbm > _params.min_snr_for_combined_spectra_file) {
	combinedPrintCount++;
	_addSpectrumToFile(combinedSpectraFile, combinedPrintCount,
			   beamTime, el, az, igate,
			   dbm, vel1, width1, _nSamples, iq);
      }
    }
  
  } // igate

}

/////////////////////////////////////////////////
// compute moments using SZ8/64

void MomentsMgr::computeMomentsSz(time_t beamTime,
				  double el, double az, double prt,
				  int nGatesPulse,
				  Complex_t **IQ,
				  const Complex_t *delta12,
				  int &combinedPrintCount,
				  FILE *combinedSpectraFile,
				  double *powerDbm, double *snr,
				  double *dbz, double *dbzt,
				  double *vel, double *width,
				  double *leakage,
				  int *flags, int *tripFlag,
				  vector<GateSpectra *> gateSpectra) const
  
{

  if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
    cerr << "-->> computing moments SZ, el, az, nGatesPulse, time: "
  	 << el << ", " << az << ", " << nGatesPulse << ", "
  	 << DateTime::str(beamTime) << endl;
  }
  
  double range = _startRange;

  for (int igate = 0; igate < nGatesPulse; igate++, range += _gateSpacing) {
    
    if (_params.print_summary) {
      break;
    }
    
    Complex_t *iq = IQ[igate];
    
    bool doWriteCombinedSpectra =
      _setMomentsDebugPrint(_moments, el, az, range);
    
    double powerTotal_ = _missingDbl;
    double power1 = _missingDbl, vel1 = _missingDbl, width1 = _missingDbl;
    double power2 = _missingDbl, vel2 = _missingDbl, width2 = _missingDbl;
    double leak1 = _missingDbl, leak2 = _missingDbl;
    double power_ratio = 0.0;
    int flags1 = 0, flags2 = 0;
    int strong_trip_flag = 0;
    
    if (_useFft) {
      
      // FFT
      
      if (_useCForSz) {
	szComputeMomentsFft(iq, delta12, prt,
			    &power1, &vel1, &width1, &flags1,
			    &power2, &vel2, &width2, &flags2);
      } else {
	_moments.computeBySzFft(iq, delta12, prt, powerTotal_,
				power1, vel1, width1, flags1,
				power2, vel2, width2, flags2,
				leak1, leak2,
				strong_trip_flag, power_ratio,
				*(gateSpectra[igate]));
      }
      
    } else {
      
      // Pulse Pair
      
      if (_useCForSz) {
	szComputeMomentsPp(iq, delta12, prt,
			   &power1, &vel1, &width1, &flags1,
			   &power2, &vel2, &width2, &flags2);
      } else {
	_moments.computeBySzPp(iq, delta12, prt, powerTotal_,
			       power1, vel1, width1, flags1,
			       power2, vel2, width2, flags2,
			       strong_trip_flag, power_ratio);
	
      }
      
    } // (if (_useFft)
    
    double dbm1 = _missingDbl, dbm2 = _missingDbl, dbmT = _missingDbl;
    double snr1 = _missingDbl, snr2 = _missingDbl;

    if (power1 > 0.0) {
      dbm1 = 10.0 * log10(power1);
      if (power1 > _noiseFixed) {
	snr1 = 10.0 * log10((power1 - _noiseFixed) / _noiseFixed);
      }
    }

    if (power2 > 0.0) {
      dbm2 = 10.0 * log10(power2);
      if (power2 > _noiseFixed) {
	snr2 = 10.0 * log10((power2 - _noiseFixed) / _noiseFixed);
      }
    }

    if (power1 > _noiseFixed && power2 > _noiseFixed ) {
      dbmT = 10.0 * log10((power1 + power2 - _noiseFixed) / _noiseFixed);
    } else if (power1 > 0) {
      dbmT = dbm1;
    } else if (power2 > 0) {
      dbmT = dbm2;
    }
    
    if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
      cerr << setprecision(4);
      cerr << "  trip1 retrv db,v,w,f: "
	   << setw(10) << dbm1 << setw(10) << vel1
	   << setw(10) << width1 << setw(10) << flags1 << endl;
      if (_applySz) {
	cerr << "  trip2 retrv db,v,w,f: "
	     << setw(10) << dbm2 << setw(10) << vel2
	     << setw(10) << width2 << setw(10) << flags2 << endl;
      }
    }
    
    // trip 1
    
    if (snr1 != _missingDbl && snr1 > _params.signal_to_noise_ratio_threshold) {
      powerDbm[igate] = dbm1;
      snr[igate] = snr1;
      dbz[igate] = snr1 + _rangeCorr[igate];
      if (dbmT != _missingDbl) {
	dbzt[igate] = dbmT + _rangeCorr[igate];
      }
      vel[igate] = vel1;
      width[igate] = width1;
    }
    flags[igate] = flags1;
    leakage[igate] = _computeInterest(leak1, 0.15, 0.25);
    if (strong_trip_flag == 1) {
      tripFlag[igate] = 1;
    } else {
      tripFlag[igate] = 2;
    }
    
    // trip 2
    
    int jgate = igate + nGatesPulse;
    if (snr2 != _missingDbl && snr2 > _params.signal_to_noise_ratio_threshold) {
      powerDbm[jgate] = dbm2;
      snr[jgate] = snr2;
      if (snr2 != _missingDbl) {
	dbz[jgate] = snr2 + _rangeCorr[jgate];
      }
      dbzt[jgate] = _missingDbl;
      vel[jgate] = vel2;
      width[jgate] = width2;
    }
    flags[jgate] = flags2;
    leakage[jgate] = _computeInterest(leak2, 0.15, 0.25);
    if (strong_trip_flag == 2) {
      tripFlag[jgate] = 1;
    } else {
      tripFlag[jgate] = 2;
    }
    
    // add spectrum to output file if required
    
    if (doWriteCombinedSpectra && (combinedSpectraFile != NULL)) {
      if (dbm1 > _params.min_snr_for_combined_spectra_file) {
	combinedPrintCount++;
	_addSpectrumToFile(combinedSpectraFile, combinedPrintCount,
			   beamTime, el, az, igate,
			   dbm1, vel1, width1, _nSamples, iq);
      }
    }

  } // igate

}

/////////////////////////////////////////////////
// compute moments in dual pol mode
//
// Set combinedSpectraFile to NULL to prevent printing.

void MomentsMgr::computeMomentsDualPol(time_t beamTime,
				       double el, double az, double prt,
				       int nGatesPulse,
				       Complex_t **IQ,
				       Complex_t **IQH,
				       Complex_t **IQV,
				       int &combinedPrintCount,
				       FILE *combinedSpectraFile,
				       double *powerDbm, double *snr,
				       double *dbz, double *vel,
				       double *width, int *flags,
				       double *zdr, double *rhohv,
				       double *phidp, double *kdp,
                                       double *clutRatioNarrow,
                                       double *clutRatioWide,
                                       double *clutWxPeakRatio,
                                       double *clutWxPeakSep,
				       vector<GateSpectra *> gateSpectra) const
  
{

  int nDual = _nSamples / 2;
  double wavelengthMeters = _params.radar.wavelength / 100.0;
  double nyquist = ((wavelengthMeters / prt) / 4.0);
  
  if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
    cerr << "--->> computing dual pol moments, el, az, nGates, time: "
  	 << el << ", " << az << ", " << nGatesPulse << ", "
  	 << DateTime::str(beamTime) << endl;
  }
  
  double range = _startRange;
  for (int igate = 0; igate < nGatesPulse; igate++, range += _gateSpacing) {
    
    if (_params.print_summary) {
      break;
    }
    
    // bool doWriteCombinedSpectra =
    _setMomentsDebugPrint(_momentsHalf, el, az, range);

    Complex_t *iqh = IQH[igate];
    Complex_t *iqv = IQV[igate];
    
    double power1 = _missingDbl, vel1 = _missingDbl, width1 = _missingDbl;
    double power2 = _missingDbl, vel2 = _missingDbl, width2 = _missingDbl;
    double noise1 = _missingDbl, noise2 = _missingDbl;
    ClutProb clutProb1, clutProb2;
    int flags1 = 0, flags2 = 0;
    
    if (_useFft) {

      // FFT

      // Horizontal
      
      _momentsHalf.computeByFft(iqh, _fftWindow, prt, power1, noise1,
				vel1, width1, flags1, &clutProb1);
      _computeNoiseStats(power1);
      
      // Vertical
      
      _momentsHalf.computeByFft(iqv, _fftWindow, prt, power2, noise2,
				vel2, width2, flags2, &clutProb2);
      _computeNoiseStats(power2);
      
    } else {
      
      // Pulse Pair
      
      _momentsHalf.computeByPp(iqh, prt, power1, vel1, width1, flags1);
      _momentsHalf.computeByPp(iqv, prt, power2, vel2, width2, flags2);
      _computeNoiseStats(power1);
      _computeNoiseStats(power2);
      
    } // if (_useFft)
      
    double dbm1 = _missingDbl, dbm2 = _missingDbl;
    double snr1 = _missingDbl, snr2 = _missingDbl;
    if (power1 != _missingDbl) {
      dbm1 = 10.0 * log10(power1);
      if (power1 > _noiseFixed) {
	snr1 = 10.0 * log10((power1 - _noiseFixed) / _noiseFixed);
      }
    }
    if (power2 != _missingDbl) {
      dbm2 = 10.0 * log10(power2);
      if (power2 > _noiseFixed) {
	snr2 = 10.0 * log10((power2 - _noiseFixed) / _noiseFixed);
      }
    }
    
    // check for SNR

    if (snr1 == _missingDbl || snr2 == _missingDbl) {
      // below SNR
      flags[igate] = flags1 | flags2;
      // return;
    }
    
    // power and SNR
    
    double dbmMean = (dbm1 + dbm2) / 2.0;
    double snrMean = (snr1 + snr2) / 2.0;
    powerDbm[igate] = dbmMean;
    snr[igate] = snrMean;
    dbz[igate] = snrMean + _rangeCorr[igate];

    // ZV flag checking
    
    if (_zvFlagReady) {
      zdr[igate] = dbm1 - dbm2;
    } else {
      zdr[igate] = _missingDbl;
      if (igate < 100 && dbz[igate] > -20 && dbz[igate] < 20) {
	_nZdr++;
	_sumZdr += zdr[igate];
	if (_nZdr >= 1000) {
	  double meanZdr = _sumZdr / _nZdr;
	  if (meanZdr < 0) {
	    _zvFlag = !_zvFlag;
	  }
	  _zvFlagReady = true;
	  _nZdr = 0;
	  _sumZdr = 0.0;
	}
      }
    }

    // velocity and width

    vel[igate] = _computeMeanVelocity(vel1, vel2, nyquist / 2.0) / 2.0;
    width[igate] = (width1 + width2) / 2.0;
    
    // phidp

    Complex_t Rhhvv1 = _meanConjugateProduct(iqv, iqh + 1, nDual - 1);
    Complex_t Rvvhh1 = _meanConjugateProduct(iqh + 1, iqv + 1, nDual - 1);
    Complex_t psidp = _conjugateProduct(Rhhvv1, Rvvhh1);
    psidp.re /= 2.0;
    psidp.im /= 2.0;
    double psidpArg = _computeArg(psidp);
    phidp[igate] = psidpArg * RAD_TO_DEG;

    //  Complex_t cv = _complexProduct(Rhhvv1, Rvvhh1);
    //  double vArg = _computeArg(cv);
    //  double vv = (vArg / M_PI) * nyquist * 0.5;
    //  vel[igate] = vv;

    // rhohv

    double Phh = _meanPower(iqh + 1, nDual - 1);
    double Pvv = _meanPower(iqv + 1, nDual - 1);
    double rhohhvv1 = _computeMag(Rhhvv1) / sqrt(Phh * Pvv);

    Complex_t Rhhhh2 = _meanConjugateProduct(iqh, iqh + 1, nDual - 1);
    Complex_t Rvvvv2 = _meanConjugateProduct(iqv, iqv + 1, nDual - 1);
    double rhohh2 = _computeMag(Rhhhh2) / Phh;
    // double rhovv2 = _computeMag(Rvvvv2) / Pvv;
    // double rho2 = (rhohh2 + rhovv2) / 2.0;

    double rhohhvv0 = rhohhvv1 / pow(rhohh2, 0.25);
    if (rhohhvv0 > 1.0) {
      rhohhvv0 = 1.0;
    }
    rhohv[igate] = rhohhvv0;

    // clutter probability
    
    ClutProb clutProb;
    clutProb.combine(clutProb1, clutProb2);
    
    if (clutRatioNarrow != NULL) {
      clutRatioNarrow[igate] = clutProb.getRatioNarrow();
    }
    if (clutRatioWide != NULL) {
      clutRatioWide[igate] = clutProb.getRatioWide();
    }
    if (clutWxPeakRatio != NULL) {
      clutWxPeakRatio[igate] = clutProb.getClutWxPeakRatio();
    }
    if (clutWxPeakSep != NULL) {
      clutWxPeakSep[igate] = clutProb.getClutWxPeakSeparation();
    }

  } // igate

  // compute kdp

  _computeKdp(nGatesPulse, phidp, snr, kdp);

  // For gate 2 with SNR > 20
  // check for angular difference between horiz and vertical pulses
  
  if (snr[2] > 60) {
    
    Complex_t *iqh = IQH[2];
    Complex_t *iqv = IQV[2];
    Complex_t sumDiff;
    sumDiff.re = 0.0;
    sumDiff.im = 0.0;
    double count = 0.0;

    for (int ii = 0; ii < nDual; ii++, iqh++, iqv++) {
      Complex_t diff = _computeComplexDiff(*iqv, *iqh);
      sumDiff.re += diff.re;
      sumDiff.im += diff.im;
      count++;
    }

    //     cerr << "--------->> sumDiff: "
    //          << atan2(sumDiff.im, sumDiff.re) * RAD_TO_DEG << endl;

    if (!_zvFlagReady && sumDiff.im * _dualPolPhaseDiff.im < 0.0) {
      // hv sequence is not correct yet, because angle does not match
      // the theoretical one
      sumDiff.im *= -1.0;
    }
    
    _dualPolPhaseDiffSum.re += sumDiff.re;
    _dualPolPhaseDiffSum.im += sumDiff.im;
    _dualPolPhaseDiffCount += count;

    if (_dualPolPhaseDiffCount > 0) {
      _dualPolPhaseDiff.re =
        (_dualPolPhaseDiffSum.re / _dualPolPhaseDiffCount);
      _dualPolPhaseDiff.im =
        (_dualPolPhaseDiffSum.im / _dualPolPhaseDiffCount);
    }

    if (_dualPolPhaseDiffCount > 1.0e6) {
      _dualPolPhaseDiffSum.re /= 2.0;
      _dualPolPhaseDiffSum.im /= 2.0;
      _dualPolPhaseDiffCount /= 2.0;
    }
    
  }

  // adjust phase on the V pulses
  
  for (int igate = 0; igate < nGatesPulse; igate++) {
    Complex_t *iq = IQ[igate];
    for (int ii = 1; ii < _nSamples; ii+= 2) {
      Complex_t adjusted = _conjugateProduct(iq[ii], _dualPolPhaseDiff);
      iq[ii] = adjusted;
    }
    double power1 = _missingDbl;
    double vel1 = _missingDbl, width1 = _missingDbl;
    double noise1 = _missingDbl;
    int flags1 = 0;
    _moments.computeByFft(iq, _fftWindow, prt, power1, noise1, vel1, width1, flags1);
    // vel[igate] = vel1;
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "--------------------------> _dualPolPhaseDiff: "
	 << atan2(_dualPolPhaseDiff.im, _dualPolPhaseDiff.re) * RAD_TO_DEG << endl;
  }
  
}

////////////////////////////////////////////////////////////////////////
// compute moments, applying the clutter filter
// as appropriate.
//
// If hasClutter is TRUE for a gate, the clutter filter is applied.
// If not, the unfiltered moments are copied over to the filtered arrays.

void MomentsMgr::filterClutter(double prt,
			       int nGatesPulse,
			       Complex_t **IQ,
			       const bool *hasClutter,
			       const double *dbz,
			       const double *vel,
			       const double *width,
			       vector<GateSpectra *> gateSpectra,
			       double *clut,
			       double *dbzf,
			       double *velf,
			       double *widthf) const
  
{

  if (_dualPol || !_useFft) {
    return;
  }

  // copy over unfiltered data first

  memcpy(dbzf, dbz, nGatesPulse * sizeof(double));
  memcpy(velf, vel, nGatesPulse * sizeof(double));
  memcpy(widthf, width, nGatesPulse * sizeof(double));
  
  for (int igate = 0; igate < nGatesPulse; igate++) {

    if (hasClutter[igate]) {
      
      Complex_t *iq = IQ[igate];
      double power1 = _missingDbl;
      double snr1 = _missingDbl;
      double vel1 = _missingDbl;
      double width1 = _missingDbl;
      double clut1 = _missingDbl;
      double noise1 = _missingDbl;
      
      _moments.computeByFftFilterClutter(iq, _fftWindow, prt,
  					 power1, snr1, vel1, width1,
                                         clut1, noise1);
      
      // apply time-domain filter to IQ data, to compute the measured noise
      // without clutter influence

      Complex_t tdFiltered[_nSamples];
      _applyTimeDomainFilter(iq, tdFiltered);

      double power2 = _missingDbl;
      double vel2 = _missingDbl;
      double width2 = _missingDbl;
      double noise2 = _missingDbl;
      int flags2 = 0;
      _moments.computeByFft(tdFiltered, _fftWindow, prt,
  			    power2, noise2, vel2, width2, flags2);

      // reduce the filtered power by the difference between noise1 and noise2,
      // which represents the rise in noise floor from windowing

      // cerr << "noise1, noise2: " << noise1 << ", " << noise2 << endl;
      // double noiseFromWindowing = (noise1 - noise2);

      double noiseFromWindowing = 0;
      double correctedPower = power1 - noiseFromWindowing;
      // cerr << "power1, noiseFromWindowing, correctedPower: " << power1
      // << ", " << noiseFromWindowing << ", " << correctedPower << endl;
      double dbm1 = _missingDbl;
      if (correctedPower > _noiseFixed) {
	// dbm1 = 10.0 * log10(snr1);
	dbm1 = 10.0 * log10((correctedPower - _noiseFixed) / _noiseFixed);
	clut[igate] = 10.0 * log10(clut1 / correctedPower);
      }
      
      if (dbm1 == _missingDbl) {
	dbzf[igate] = _missingDbl;
      } else {
	dbzf[igate] = dbm1 + _rangeCorr[igate];
      }
      velf[igate] = vel1;
      widthf[igate] = width1;

    }
    
  } // igate

}

////////////////////////////////////////////////////////////////////////
// Filter clutter for SZ data
//
// Compute moments, applying the clutter filter
// as appropriate.
//
// If hasClutter is TRUE for a gate, the clutter filter is applied.
// If not, the unfiltered moments are copied over to the filtered arrays.

void MomentsMgr::filterClutterSz(double prt,
				 int nGatesPulse,
				 vector<GateSpectra *> gateSpectra,
				 const Complex_t *delta12,
				 const bool *hasClutter,
				 const double *dbz,
				 const double *vel,
				 const double *width,
				 const int *tripFlag,
				 double *clut,
				 double *dbzf,
				 double *velf,
				 double *widthf) const
  
{
  
  // copy over unfiltered data first

  memcpy(dbzf, dbz, 2 * nGatesPulse * sizeof(double));
  memcpy(velf, vel, 2 * nGatesPulse * sizeof(double));
  memcpy(widthf, width, 2 * nGatesPulse * sizeof(double));
  
  for (int trip1Gate = 0; trip1Gate < nGatesPulse; trip1Gate++) {
    int trip2Gate = trip1Gate + nGatesPulse;

    bool trip1IsStrong = (tripFlag[trip1Gate] == 1);

    // determine the presence of clutter in strong and weak trip

    bool clutterInStrong = false;
    bool clutterInWeak = false;
    
    if (trip1IsStrong) {
      if (hasClutter[trip1Gate]) {
	clutterInStrong = true;
      }
      if (hasClutter[trip2Gate]) {
	clutterInWeak = true;
      }
    } else {
      // trip 1 is weak
      if (hasClutter[trip1Gate]) {
	clutterInWeak = true;
      }
      if (hasClutter[trip2Gate]) {
	clutterInStrong = true;
      }
    }

    if (!clutterInStrong && !clutterInWeak) {
      // no filtering necessary
      continue;
    }

    double powerStrong = _missingDbl, powerWeak = _missingDbl;
    double velStrong = _missingDbl, velWeak = _missingDbl;
    double widthStrong = _missingDbl, widthWeak = _missingDbl;
    double clutStrong = _missingDbl, clutWeak = _missingDbl;
    
    _moments.filterClutterSz(*(gateSpectra[trip1Gate]), delta12, prt,
			     clutterInStrong, clutterInWeak,
			     powerStrong, velStrong, widthStrong, clutStrong,
			     powerWeak, velWeak, widthWeak, clutWeak);

    double snrStrong = _missingDbl, snrWeak = _missingDbl;
    if (powerStrong > _noiseFixed) {
      snrStrong = 10.0 * log10((powerStrong - _noiseFixed) / _noiseFixed);
    }
    if (powerWeak > _noiseFixed) {
      snrStrong = 10.0 * log10((powerWeak - _noiseFixed) / _noiseFixed);
    }

    if (trip1IsStrong) {
      
      if (snrStrong != _missingDbl) {
	dbzf[trip1Gate] = snrStrong + _rangeCorr[trip1Gate];
	if (clutStrong > 0) {
	  clut[trip1Gate] = 10.0 * log10(clutStrong / powerStrong);
	}
      }
      if (velf[trip1Gate] == _missingDbl || velStrong != _missingDbl) {
	velf[trip1Gate] = velStrong;
      }
      if (widthf[trip1Gate] == _missingDbl || widthStrong != _missingDbl) {
	widthf[trip1Gate] = widthStrong;
      }

      if (snrWeak != _missingDbl) {
	dbzf[trip2Gate] = snrWeak + _rangeCorr[trip2Gate];
	if (clutWeak > 0) {
	  clut[trip2Gate] = 10.0 * log10(clutWeak / powerWeak);
	}
      }

      if (velf[trip2Gate] == _missingDbl || velWeak != _missingDbl) {
	velf[trip2Gate] = velWeak;
      }
      if (widthf[trip2Gate] == _missingDbl || widthWeak != _missingDbl) {
	widthf[trip2Gate] = widthWeak;
      }

    } else {

      if (snrStrong != _missingDbl) {
	dbzf[trip2Gate] = snrStrong + _rangeCorr[trip2Gate];
	if (clutStrong > 0) {
	  clut[trip2Gate] = 10.0 * log10(clutStrong / powerStrong);
	}
      }
      if (velf[trip2Gate] == _missingDbl || velStrong != _missingDbl) {
	velf[trip2Gate] = velStrong;
      }
      if (widthf[trip2Gate] == _missingDbl || widthStrong != _missingDbl) {
	widthf[trip2Gate] = widthStrong;
      }

      if (snrWeak != _missingDbl) {
	dbzf[trip1Gate] = snrWeak + _rangeCorr[trip1Gate];
	if (clutWeak > 0) {
	  clut[trip1Gate] = 10.0 * log10(clutWeak / powerWeak);
	}
      }
      if (velf[trip1Gate] == _missingDbl || velWeak != _missingDbl) {
	velf[trip1Gate] = velWeak;
      }
      if (widthf[trip1Gate] == _missingDbl || widthWeak != _missingDbl) {
	widthf[trip1Gate] = widthWeak;
      }

    }
      
  } // trip1Gate

}

////////////////////////////////////////////////////////////////////////
// Filter clutter for Dual pol data
//
// compute moments, applying the clutter filter as appropriate.
//
// If hasClutter is TRUE for a gate, the clutter filter is applied.
// If not, the unfiltered moments are copied over to the filtered arrays.

void MomentsMgr::filterClutterDualPol(double prt,
				      int nGatesPulse,
				      Complex_t **IQ,
				      Complex_t **IQH,
				      Complex_t **IQV,
				      const bool *hasClutter,
				      const double *dbz,
				      const double *vel,
				      const double *width,
				      vector<GateSpectra *> gateSpectra,
				      double *clut,
				      double *dbzf,
				      double *velf,
				      double *widthf) const
  
{

  if (!_dualPol || !_useFft) {
    return;
  }

  // int nDual = _nSamples / 2;
  double wavelengthMeters = _params.radar.wavelength / 100.0;
  double nyquist = ((wavelengthMeters / prt) / 4.0);
  
  // copy over unfiltered data first

  memcpy(dbzf, dbz, nGatesPulse * sizeof(double));
  memcpy(velf, vel, nGatesPulse * sizeof(double));
  memcpy(widthf, width, nGatesPulse * sizeof(double));

  for (int igate = 0; igate < nGatesPulse; igate++) {
    
    if (hasClutter[igate]) {
      
      Complex_t *iqh = IQH[igate];
      Complex_t *iqv = IQV[igate];
      
      double power1 = _missingDbl, vel1 = _missingDbl, width1 = _missingDbl;
      double power2 = _missingDbl, vel2 = _missingDbl, width2 = _missingDbl;
      double snr1 = _missingDbl, snr2 = _missingDbl;
      double clut1 = _missingDbl, clut2 = _missingDbl;
      double noise1 = _missingDbl, noise2 = _missingDbl;
      
      // Horizontal
      
      _momentsHalf.computeByFftFilterClutter(iqh, _fftWindow, prt, power1, snr1,
					     vel1, width1, clut1, noise1);
      
      // Vertical
      
      _momentsHalf.computeByFftFilterClutter(iqv, _fftWindow, prt, power2, snr2,
					     vel2, width2, clut2, noise2);
      
      if (power1 != _missingDbl && power2 != _missingDbl) {
	double meanPower = (power1 + power2) / 2.0;
	double meanNoise = (noise1 + noise2) / 2.0;
	double noise = meanNoise;
	// double noise = _noiseFixed;
	if (meanPower > noise) {
	  double snrMean = 10.0 * log10((meanPower - noise) / noise);
	  dbzf[igate] = snrMean + _rangeCorr[igate];
	}
	if (clut1 != _missingDbl && clut2 != _missingDbl) {
	  double meanClut = (clut1 + clut2) / 2.0;
	  double clutDb = 10.0 * log10(meanClut / meanPower);
	  clut[igate] = clutDb;
          // cerr << "meanPower, meanClut, clutDb, dbz: "
          // << 10.0 * log10(meanPower) << ", " << 10.0 * log10(meanClut)
          // << ", " << clutDb << ", " << dbz[igate] << endl;
	}
      }

      if (vel1 != _missingDbl && vel2 != _missingDbl) {
	double meanVel = _computeMeanVelocity(vel1, vel2, nyquist / 2.0) / 2.0;
	velf[igate] = meanVel;
      }

      if (width1 != _missingDbl && width2 != _missingDbl) {
	double meanWidth = (width1 + width2) / 2.0;
	widthf[igate] = meanWidth;
      }

    } // if (hasClutter ...

  } // igate

}

///////////////////////////////////////
// compute range correction table

void MomentsMgr::_computeRangeCorrection()

{
  
  for (int i = 0; i < _maxGates; i++) {
    double range_km = _startRange + i * _gateSpacing;
    _rangeCorr[i] = _minDbzAt1km +
      20.0 * log10(range_km) + range_km * _atmosAtten;
  }

}

///////////////////////////////////////
// compute mean velocity

double MomentsMgr::_computeMeanVelocity(double vel1,
					double vel2,
					double nyquist) const

{

  double diff = fabs(vel1 - vel2);
  if (diff < nyquist) {
    return (vel1 + vel2) / 2.0;
  } else {
    if (vel1 > vel2) {
      vel2 += 2.0 * nyquist;
    } else {
      vel1 += 2.0 * nyquist;
    }
    double vel = (vel1 + vel2) / 2.0;
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

double MomentsMgr::_velDiff2Angle(double vel1, double vel2, double nyquist) const

{
  
  double ang1 = (vel1 / nyquist) * 180.0;
  double ang2 = (vel2 / nyquist) * 180.0;
  double adiff = (ang1 - ang2) - _params.H2V_phase_differential;
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
			     const double *phidp,
			     const double *snr,
			     double *kdp) const
  
{

  int nGatesForSlope = 7;
  int nGatesHalf = nGatesForSlope / 2;
  // double dRange = (nGatesForSlope - 1) * _gateSpacing;

  if (nGatesForSlope > nGates) {
    return;
  }

  for (int ii = nGatesHalf; ii < nGates - nGatesHalf; ii++) {

    if (phidp[ii] != _missingDbl) {
      double slope = _computeSlope(ii, nGatesForSlope, nGatesHalf, phidp, snr);
      if (slope != _missingDbl) {
	kdp[ii] = slope * DEG_TO_RAD;
      }
    }

    // double dphidp = phidp[ii + nGatesHalf] - phidp[ii - nGatesHalf];
    // double slope = (dphidp / (dRange * 2.0)) * DEG_TO_RAD;
    // if (slope < 0) {
    //   slope = 0;
    // }
    // kdp[ii] = slope;

  }

}

//////////////////////////////////////////////////
// compute PhiDp slope
//
// returns _missingDbl if not enough data

double MomentsMgr::_computeSlope(int index,
				 int nGatesForSlope,
				 int nGatesHalf,
				 const double *phidp,
				 const double *snr) const

{
  
  double xx[nGatesForSlope];
  double yy[nGatesForSlope];
  double wt[nGatesForSlope];
  int count = 0;
  
  double pdpThisGate = phidp[index];
  double range = 0.0;

  for (int ii = index - nGatesHalf; ii <= index + nGatesHalf;
       ii++, range += _gateSpacing) {
    double pdp = phidp[ii];
    if (pdp != _missingDbl) {
      double diff = pdp - pdpThisGate;
      if (diff > 180.0) {
	pdp -= 360.0;
      } else if (diff < -180) {
	pdp += 360.0;
      }
      xx[count] = range;
      yy[count] = pdp;
      wt[count] = pow(10.0, snr[ii]);
      count++;
    }
  }

  if (count < nGatesHalf) {
    return _missingDbl;
  }

  double sumx = 0.0;
  double sumy = 0.0;
  double sumx2 = 0.0;
  double sumy2 = 0.0;
  double sumxy = 0.0;
  double sumwt = 0.0;
  
  for (int ii = 0; ii < count; ii++) {
    double xxx = xx[ii];
    double yyy = yy[ii];
    double weight= wt[ii];
    sumx += xxx * weight;
    sumx2 += xxx * xxx * weight;
    sumy += yyy * weight;
    sumy2 += yyy * yyy * weight;
    sumxy += xxx * yyy * weight;
    sumwt += weight;
  }

  // get y-on-x slope
  
  double num = sumwt * sumxy - sumx * sumy;
  double denom = sumwt * sumx2 - sumx * sumx;
  double slope_y_on_x;
  
  if (denom != 0.0) {
    slope_y_on_x = num / denom;
  } else {
    slope_y_on_x = 0.0;
  }
  
  // get x-on-y slope

  denom = sumwt * sumy2 - sumy * sumy;
  double slope_x_on_y;
  
  if (denom != 0.0) {
    slope_x_on_y = num / denom;
  } else {
    slope_x_on_y = 0.0;
  }

  // average slopes

  double slope;
  
  if (slope_y_on_x != 0.0 && slope_x_on_y != 0.0) {
    slope = (slope_y_on_x + 1.0 / slope_x_on_y) / 2.0;
  } else if (slope_y_on_x != 0.0) {
    slope = slope_y_on_x;
  } else if (slope_x_on_y != 0.0) {
    slope = 1.0 / slope_x_on_y;
  } else {
    slope = 0.0;
  }

  // if (fabs(slope) < 20) {
  //   cerr << "slope_y_on_x, slope_x_on_y, slope: "
  //        << slope_y_on_x << " "
  //  	  << slope_x_on_y << " "
  //  	  << slope << endl;
  // }

  return 1.0 / slope_x_on_y;

}

////////////////////////////////////////////////////////////////////////
// Add spectrum to combined spectra output file

void MomentsMgr::_addSpectrumToFile(FILE *specFile, int count, time_t beamTime,
                                    double el, double az, int gateNum,
                                    double snr, double vel, double width,
                                    int nSamples, const Complex_t *iq) const

{

  date_time_t btime;
  btime.unix_time = beamTime;
  uconvert_from_utime(&btime);
  
  fprintf(specFile,
	  "%d %d %d %d %d %d %d %g %g %d %g %g %g %d",
	  count,
	  btime.year, btime.month, btime.day,
	  btime.hour, btime.min, btime.sec,
	  el, az, gateNum,
	  snr, vel, width, nSamples);

  for (int ii = 0; ii < nSamples; ii++) {
    fprintf(specFile, " %g %g", iq[ii].re, iq[ii].im);
  }
  
  fprintf(specFile, "\n");

}

////////////////////////////////////////////////////////////////////////
// Compute interest value

double MomentsMgr::_computeInterest(double xx,
				  double x0, double x1) const
  
{

  if (xx <= x0) {
    return 0.01;
  }
  
  if (xx >= x1) {
    return 0.99;
  }
  
  double xbar = (x0 + x1) / 2.0;
  
  if (xx <= xbar) {
    double yy = (xx - x0) / (x1 - x0);
    double yy2 = yy * yy;
    double interest = 2.0 * yy2;
    return interest;
  } else {
    double yy = (x1 - xx) / (x1 - x0);
    double yy2 = yy * yy;
    double interest = 1.0 - 2.0 * yy2;
    return interest;
  }

}

////////////////////////////////////////////////////////////////////////
// Gather noise stats

void MomentsMgr::_computeNoiseStats(double noise) const

{

  if (!_params.compute_noise) {
    return;
  }

  if (noise == _missingDbl) {
    return;
  }

  double noiseDbm = 10.0 * log10(noise);

  if (noiseDbm < _noiseHistMin || noiseDbm > _noiseHistMax) {
    return;
  }

  int index = (int) ((noiseDbm - _noiseHistMin) / _noiseHistDelta + 0.5);
  _noiseHist[index]++;
  _noiseHistTotalCount++;
  _noiseHistComputeCount++;

  if (_noiseHistComputeCount > 50000) {

    // compute the 10th noise percentile, every so often

    double count10 = _noiseHistTotalCount / 10;
    double sum = 0.0;

    for (int ii = 0; ii < _noiseHistSize; ii++) {
      sum += _noiseHist[ii];
      if (sum > count10) {
	_measuredNoiseDbm = _noiseHistMin + ii * _noiseHistDelta;
	_measuredNoise = pow(10.0, _measuredNoiseDbm / 10.0);
	break;
      }
    }
    
    // reset counter

    _noiseHistComputeCount = 0;

    // reset histogram when count gets large

    if (_noiseHistTotalCount > 1.0e12) {
      memset(_noiseHist, 0, _noiseHistSize * sizeof(double));
      _noiseHistTotalCount = 0.0;
    }

  } // if (_noiseHistComputeCount > 10000)

}

/////////////////////////////////////////////////////////////////
// Set debug prints

bool MomentsMgr::_setMomentsDebugPrint(const Moments &moments,
				       double el,
				       double az,
				       double range) const

{

  if (_params.debug < Params::DEBUG_EXTRA_VERBOSE &&
      !_params.do_selected_print) {
    return false;
  }

  moments.setDebugEl(el);
  moments.setDebugAz(az);
  moments.setDebugRange(range);
  moments.setDebugWriteSpectra(false);

  if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
    moments.setDebugPrint(true);
  } else {
    moments.setDebugPrint(false);
  }

  bool doWriteCombinedSpectra = false;
  if (_params.do_selected_print) {
    if (el >= _params.selected_region.min_el &&
	el <= _params.selected_region.max_el &&
	az >= _params.selected_region.min_az &&
	az <= _params.selected_region.max_az &&
	range >= _params.selected_region.min_range &&
	range <= _params.selected_region.max_range) {
      if (_params.write_individual_spectra_files) {
	moments.setDebugPrint(true);
	moments.setDebugWriteSpectra(true, _params.spectra_dir);
      }
      if (_params.write_combined_spectra_file) {
	doWriteCombinedSpectra = true;
      }
    }
  }

  return doWriteCombinedSpectra;

}

/////////////////////////////////////////////////////////////////
// Apply time-domain filter to IQ data

void MomentsMgr::_applyTimeDomainFilter(const Complex_t *iq,
					Complex_t *filtered) const

{

  // filter coefficients

  double aa = 0.654424;
  double bb = 0.474320;
  double cc = 1.614429;
  double dd = 0.742456;
  double ee = 1.989848;
  double ff = 1.874940;
  double gg = 0.951312;
  double hh = 1.976096;
  double pp = 1.0;
  double qq = 1.0;

  // initialize

  double kk = aa / (1.0 - bb);
  Complex_t uu1, vv1, vv2, ww1, ww2;

  uu1.re = kk * iq[0].re;
  uu1.im = kk * iq[0].im;

  vv1.re = 0.0;
  vv1.im = 0.0;
  vv2.re = 0.0;
  vv2.im = 0.0;
  
  ww1.re = 0.0;
  ww1.im = 0.0;
  ww2.re = 0.0;
  ww2.im = 0.0;
  
  Complex_t uu, vv, ww;

  for (int ii = -2; ii < _nSamples; ii++) {

    int jj = ii;
    if (jj < 0) {
      jj = 0;
    }

    uu.re = aa * iq[jj].re + bb * uu1.re;
    uu.im = aa * iq[jj].im + bb * uu1.im;

    vv.re = uu.re - uu1.re + cc * vv1.re - dd * vv2.re;
    vv.im = uu.im - uu1.im + cc * vv1.im - dd * vv2.im;

    ww.re = vv.re - ee * vv1.re + pp * vv2.re + ff * ww1.re - gg * ww2.re;
    ww.im = vv.im - ee * vv1.im + pp * vv2.im + ff * ww1.im - gg * ww2.im;

    filtered[jj].re = ww.re - hh * ww1.re + qq * ww2.re;
    filtered[jj].im = ww.im - hh * ww1.im + qq * ww2.im;

  } // ii

}

