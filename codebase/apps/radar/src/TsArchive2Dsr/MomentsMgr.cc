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
#include <toolsa/TaArray.hh>
#include <rapmath/umath.h>
#include <toolsa/mem.h>
#include "MomentsMgr.hh"
#include "Fft.hh"
#include "PhidpKdp.hh"
using namespace std;

const double MomentsMgr::_missingDbl = -9999.0;
const double MomentsMgr::_phidpPhaseLimit = -70;

////////////////////////////////////////////////////
// Constructor

MomentsMgr::MomentsMgr(const string &prog_name,
		       const Params &params,
                       const OpsInfo &opsInfo,
		       Params::moments_params_t &moments_params,
		       int n_samples,
		       int max_gates) :
        _progName(prog_name),
        _params(params),
        _moments_params(moments_params),
        _opsInfo(opsInfo),
        _lowerPrf(moments_params.lower_prf),
        _upperPrf(moments_params.upper_prf),
        _pulseWidth(moments_params.pulse_width),
        _maxGates(max_gates),
        _useFft(moments_params.algorithm == Params::ALG_FFT),
        _applySz(moments_params.apply_sz),
        _dualPol(moments_params.dual_pol),
        _momentsWindow(Moments::WINDOW_RECT),
        _nSamples(n_samples),
        _nSamplesHalf(n_samples / 2),
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
  
  if (_moments_params.window == Params::WINDOW_RECT) {
    _momentsWindow = Moments::WINDOW_RECT;
  } else if (_moments_params.window == Params::WINDOW_HANNING) {
    _momentsWindow = Moments::WINDOW_HANNING;
  } else if (_moments_params.window == Params::WINDOW_BLACKMAN) {
    _momentsWindow = Moments::WINDOW_BLACKMAN;
  }

  // initialize moments object

  _moments.setSignalToNoiseRatioThreshold
    (_params.moments_snr_threshold);
  _momentsHalf.setSignalToNoiseRatioThreshold
    (_params.moments_snr_threshold);
  
  if (_params.negate_phase_codes) {
    _moments.setSzNegatePhaseCodes();
  }
  if (_params.sz_window == Params::WINDOW_HANNING) {
    _moments.setSzWindow(Moments::WINDOW_HANNING);
  } else if (_params.sz_window == Params::WINDOW_BLACKMAN) {
    _moments.setSzWindow(Moments::WINDOW_BLACKMAN);
  }
  
  _moments.setSzStrongToWeakPowerRatioThreshold
    (_params.sz_strong_to_weak_power_ratio_threshold);
  _moments.setSzOutOfTripPowerRatioThreshold
    (_params.sz_out_of_trip_power_ratio_threshold);
  _moments.setSzOutOfTripPowerNReplicas
    (_params.sz_out_of_trip_power_n_replicas);

  if (_params.apply_db_for_db_correction) {

    _moments.setDbForDbRatio(_params.db_for_db_ratio);
    _moments.setDbForDbThreshold(_params.db_for_db_threshold);
    
    _momentsHalf.setDbForDbRatio(_params.db_for_db_ratio);
    _momentsHalf.setDbForDbThreshold(_params.db_for_db_threshold);
    
  } else {

    // do not apply correction, so set these accordingly

    _moments.setDbForDbRatio(0.01);
    _moments.setDbForDbThreshold(100.0);
    
    _momentsHalf.setDbForDbRatio(0.01);
    _momentsHalf.setDbForDbThreshold(100.0);

  }

  // compute refract if required

  _needRefractVars = false;

  if (_params.apply_cmd ||
      _params.output_fields.cpa ||
      _params.output_fields.aiq ||
      _params.output_fields.niq ||
      _params.output_fields.meani ||
      _params.output_fields.meanq) {
    _needRefractVars = true;
  }

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

void MomentsMgr::computeMoments(time_t beamTime,
				double el, double az, double prt,
				int nGatesPulse,
				Complex_t **IQ,
				int &combinedPrintCount,
				FILE *combinedSpectraFile,
                                Fields *fields,
				vector<GateSpectra *> gateSpectra)
  
{
  
  if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
    cerr << "-->> computing moments, el, az, nGatesPulse, time: "
  	 << el << ", " << az << ", " << nGatesPulse << ", "
  	 << DateTime::str(beamTime) << endl;
  }

  // initialize based on OpsInfo
  
  _init();
  
  double range = _startRange;
  for (int igate = 0; igate < nGatesPulse; igate++, range += _gateSpacing) {
    
    bool doWriteCombinedSpectra =
      _setMomentsDebugPrint(_moments, el, az, range);

    // set IQ pointer

    const Complex_t *iq = IQ[igate];
    
    // apply window as appropriate

    TaArray<Complex_t> iqWindowed_;
    Complex_t *iqWindowed = iqWindowed_.alloc(_nSamples);
    
    if (_moments_params.window == Params::WINDOW_HANNING) {
      _moments.applyHanningWindow(iq, iqWindowed);
    } else if (_moments_params.window == Params::WINDOW_BLACKMAN) {
      _moments.applyBlackmanWindow(iq, iqWindowed);
    } else {
      _moments.applyRectWindow(iq, iqWindowed);
    }

    // save gate spectra

    gateSpectra[igate]->setIq(iq);
    gateSpectra[igate]->setIqWindowed(iqWindowed);

    // initialize for moments
    
    double power_0 = _missingDbl;
    double vel_0 = _missingDbl, width_0 = _missingDbl;
    double noise_0 = _missingDbl;
    int flags_0 = 0;
    ClutProb clutProb_0;
    clutProb_0.setRatioInnerIndex(_params.clutter_ratio_inner_index);
    clutProb_0.setRatioOuterIndex(_params.clutter_ratio_outer_index);
    
    if (_useFft) {
      
      // FFT
      // specify rectangular window since window has already been applied
      
      _moments.computeByFft(iqWindowed, Moments::WINDOW_RECT,
                            prt, power_0, noise_0,
			    vel_0, width_0, flags_0, &clutProb_0);
      
    } else {
      
      // Pulse Pair
      
      _moments.computeByPp(iqWindowed, prt, power_0, vel_0, width_0, flags_0);
      if (_params.apply_cmd) {
        _moments.computeClutProb(iqWindowed, Moments::WINDOW_RECT,
                                 prt, clutProb_0);
      }

    } // (if (_useFft)
    
    double dbm_0 = _missingDbl;
    if (power_0 != _missingDbl) {
      dbm_0 = 10.0 * log10(power_0);
      fields[igate].dbm = dbm_0;
      if (power_0 <= _noiseFixedChan0) {
        power_0 = _noiseFixedChan0 + 1.0e-20;
      }
      double snr_0 =
        10.0 * log10((power_0 - _noiseFixedChan0) / _noiseFixedChan0);
      fields[igate].snr = snr_0;
      fields[igate].dbz = snr_0 + _dbz0Chan0 + _rangeCorr[igate];
    }
    
    fields[igate].vel = vel_0;
    fields[igate].width = width_0;
    fields[igate].flags = flags_0;

//     if (fields[igate].snr > 3) {
//       cerr << "cpa, pr, snr: "
//            << fields[igate].cpa << " "
//            << clutProb_0.getPowerRatio() << " "
//            << fields[igate].snr << endl;
//     }

    if (_params.apply_cmd) {
      double powerNarrow = clutProb_0.getPowerNarrow();
      double snrNarrow =
        10.0 * log10((powerNarrow - _noiseFixedChan0) / _noiseFixedChan0);
      fields[igate].cmd_dbz_narrow =
        snrNarrow + _dbz0Chan0 + _rangeCorr[igate];
      fields[igate].cmd_power_ratio = clutProb_0.getPowerRatio();
      fields[igate].cmd_ratio_narrow = clutProb_0.getRatioNarrow();
      fields[igate].cmd_pr_narrow = clutProb_0.getPrNarrow();
      fields[igate].cmd_ratio_wide = clutProb_0.getRatioWide();
      fields[igate].cmd_pr_wide = clutProb_0.getPrWide();
      fields[igate].cmd_clut2wx_sep = clutProb_0.getClutWxPeakSeparation();
      fields[igate].cmd_clut2wx_ratio = clutProb_0.getClutWxPeakRatio();
      fields[igate].cmd_clut_width = clutProb_0.getClutSpectrumWidth();
      fields[igate].cmd_wx2noise_ratio = clutProb_0.getWx2NoiseRatio();
    }
    
    // add spectrum to output file if required
    
    if (doWriteCombinedSpectra && (combinedSpectraFile != NULL)) {
      if (dbm_0 > _params.min_snr_for_combined_spectra_file) {
	combinedPrintCount++;
	_addSpectrumToFile(combinedSpectraFile, combinedPrintCount,
			   beamTime, el, az, igate,
			   dbm_0, vel_0, width_0, _nSamples, iq);
      }
    }
  
  } // igate

  if (_needRefractVars) {
    _computeRefract(nGatesPulse, gateSpectra, fields);
  }

  // #define PRINT_FOR_GNUPLOT
#ifdef PRINT_FOR_GNUPLOT
  for (int igate = 0; igate < nGatesPulse; igate++) {

    double snr = fields[igate].snr;
    //     double vel = fields[igate].vel;
    //     double width = fields[igate].width;
    double cpa = fields[igate].cpa;
    double sqrtPr = sqrt(fields[igate].cmd_power_ratio);
    
    if (snr < 50) {
      continue;
    }
    
#ifdef NOTNOW
    if (width > 0 && width < 0.6) {
      cout
        << " " << snr
        << " " << vel
        << " " << width
        << " " << cpa
        << " " << sqrtPr
        << endl;
    }
#endif
      
    if (cpa > 0.9 && sqrtPr > 0.8 ) {
      
      cerr << "CPA: " << cpa << endl;
      cerr << "sqrtPR: " << sqrtPr << endl;
      
      const Complex_t *iqWindowed = gateSpectra[igate]->getIqWindowed();
    
      Fft fft(_nSamples);
      TaArray<Complex_t> spectrum_;
      Complex_t *spectrum = spectrum_.alloc(_nSamples);
      fft.fwd(iqWindowed, spectrum);
      
      cerr << "=========== SPEC =====================" << endl;
      
      for (int ii = 0; ii < _nSamples; ii++) {
        int jj = ii;
        jj += _nSamples / 2;
        jj %= _nSamples;
        double re = spectrum[jj].re;
        double im = spectrum[jj].im;
        double power = re * re + im * im;
        double dbm = 10.0 * log10(power);
        cerr << setw(3) << ii << " "
             << setw(10) << dbm << endl;
      }
      
      
      exit(0);

    }

  } // igate
  
#endif
    
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
                                  Fields *fields,
				  vector<GateSpectra *> gateSpectra) 
  
{

  if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
    cerr << "-->> computing moments SZ, el, az, nGatesPulse, time: "
  	 << el << ", " << az << ", " << nGatesPulse << ", "
  	 << DateTime::str(beamTime) << endl;
  }
  
  // initialize based on OpsInfo
  
  _init();
  
  double range = _startRange;

  for (int igate = 0; igate < nGatesPulse; igate++, range += _gateSpacing) {
    
    Complex_t *iq = IQ[igate];
    
    bool doWriteCombinedSpectra =
      _setMomentsDebugPrint(_moments, el, az, range);
    
    double powerTotal_ = _missingDbl;
    double power_0 = _missingDbl, vel_0 = _missingDbl, width_0 = _missingDbl;
    double power_1 = _missingDbl, vel_1 = _missingDbl, width_2 = _missingDbl;
    double leak_0 = _missingDbl, leak_1 = _missingDbl;
    double power_ratio = 0.0;
    int flags_0 = 0, flags_1 = 0;
    int strong_trip_flag = 0;
    
    if (_useFft) {
      
      // FFT
      
      _moments.computeBySzFft(iq, delta12, prt, powerTotal_,
                              power_0, vel_0, width_0, flags_0,
                              power_1, vel_1, width_2, flags_1,
                              leak_0, leak_1,
                              strong_trip_flag, power_ratio,
                              *(gateSpectra[igate]));
    } else {
      
      // Pulse Pair
      
      _moments.computeBySzPp(iq, delta12, prt, powerTotal_,
                             power_0, vel_0, width_0, flags_0,
                             power_1, vel_1, width_2, flags_1,
                             strong_trip_flag, power_ratio);
      
    } // (if (_useFft)
    
    double dbm_0 = _missingDbl, dbm_1 = _missingDbl, dbmT = _missingDbl;
    double snr_0 = _missingDbl, snr_1 = _missingDbl;

    if (power_0 > 0.0) {
      dbm_0 = 10.0 * log10(power_0);
      if (power_0 <= _noiseFixedChan0) {
        power_0 = _noiseFixedChan0 + 1.0e-20;
      }
      snr_0 = 10.0 * log10((power_0 - _noiseFixedChan0) / _noiseFixedChan0);
    }

    if (power_1 > 0.0) {
      dbm_1 = 10.0 * log10(power_1);
      if (power_1 <= _noiseFixedChan0) {
        power_1 = _noiseFixedChan0 + 1.0e-20;
      }
      snr_1 = 10.0 * log10((power_1 - _noiseFixedChan0) / _noiseFixedChan0);
    }

    if (power_0 > _noiseFixedChan0 && power_1 > _noiseFixedChan0 ) {
      dbmT = 10.0 * log10((power_0 + power_1 - _noiseFixedChan0) /
                          _noiseFixedChan0);
    } else if (power_0 > 0) {
      dbmT = dbm_0;
    } else if (power_1 > 0) {
      dbmT = dbm_1;
    }
    
    if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
      cerr << setprecision(4);
      cerr << "  trip1 retrv db,v,w,f: "
	   << setw(10) << dbm_0 << setw(10) << vel_0
	   << setw(10) << width_0 << setw(10) << flags_0 << endl;
      if (_applySz) {
	cerr << "  trip2 retrv db,v,w,f: "
	     << setw(10) << dbm_1 << setw(10) << vel_1
	     << setw(10) << width_2 << setw(10) << flags_1 << endl;
      }
    }
    
    // trip 1
    
    if (snr_0 != _missingDbl && snr_0 > _params.moments_snr_threshold) {
      fields[igate].dbm = dbm_0;
      fields[igate].snr = snr_0;
      fields[igate].dbz = snr_0 + _dbz0Chan0 + _rangeCorr[igate];
      if (dbmT != _missingDbl) {
	fields[igate].sz_dbzt = dbmT + _dbz0Chan0 + _rangeCorr[igate];
      }
      fields[igate].vel = vel_0;
      fields[igate].width = width_0;
    }
    fields[igate].flags = flags_0;
    fields[igate].sz_leakage = _computeInterest(leak_0, 0.15, 0.25);
    if (strong_trip_flag == 1) {
      fields[igate].sz_trip_flag = 1;
    } else {
      fields[igate].sz_trip_flag = 2;
    }
    
    // trip 2
    
    int jgate = igate + nGatesPulse;
    if (snr_1 != _missingDbl && snr_1 > _params.moments_snr_threshold) {
      fields[jgate].dbm = dbm_1;
      fields[jgate].snr = snr_1;
      if (snr_1 != _missingDbl) {
	fields[jgate].dbz = snr_1 + _dbz0Chan0 + _rangeCorr[jgate];
      }
      fields[jgate].sz_dbzt = _missingDbl;
      fields[jgate].vel = vel_1;
      fields[jgate].width = width_2;
    }
    fields[jgate].flags = flags_1;
    fields[jgate].sz_leakage = _computeInterest(leak_1, 0.15, 0.25);
    if (strong_trip_flag == 2) {
      fields[jgate].sz_trip_flag = 1;
    } else {
      fields[jgate].sz_trip_flag = 2;
    }
    
    // add spectrum to output file if required
    
    if (doWriteCombinedSpectra && (combinedSpectraFile != NULL)) {
      if (dbm_0 > _params.min_snr_for_combined_spectra_file) {
	combinedPrintCount++;
	_addSpectrumToFile(combinedSpectraFile, combinedPrintCount,
			   beamTime, el, az, igate,
			   dbm_0, vel_0, width_0, _nSamples, iq);
      }
    }

  } // igate

}

/////////////////////////////////////////////////
// compute moments in alternating dual pol mode
//
// Set combinedSpectraFile to NULL to prevent printing.

void MomentsMgr::computeDualAlt(time_t beamTime,
                                double el, double az, double prt,
                                int nGatesPulse,
                                Complex_t **IQ,
                                Complex_t **IQH,
                                Complex_t **IQV,
                                int &combinedPrintCount,
                                FILE *combinedSpectraFile,
                                Fields *fields,
                                vector<GateSpectra *> gateSpectra)

{

  // initialize based on OpsInfo
  
  _init();
  
  double wavelengthMeters = _opsInfo.getWavelengthCm() / 100.0;
  double nyquist = ((wavelengthMeters / prt) / 4.0);
  
  if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
    cerr << "--->> computing dual pol moments, el, az, nGates, time: "
  	 << el << ", " << az << ", " << nGatesPulse << ", "
  	 << DateTime::str(beamTime) << endl;
  }
  
  double range = _startRange;
  for (int igate = 0; igate < nGatesPulse; igate++, range += _gateSpacing) {
    
    Complex_t *iqh = IQH[igate];
    Complex_t *iqv = IQV[igate];

    // apply window as appropriate
    
    TaArray<Complex_t> iqhWindowed_;
    Complex_t *iqhWindowed = iqhWindowed_.alloc(_nSamples);
    
    if (_moments_params.window == Params::WINDOW_HANNING) {
      _momentsHalf.applyHanningWindow(iqh, iqhWindowed);
    } else if (_moments_params.window == Params::WINDOW_BLACKMAN) {
      _momentsHalf.applyBlackmanWindow(iqh, iqhWindowed);
    } else {
      _momentsHalf.applyRectWindow(iqh, iqhWindowed);
    }

    TaArray<Complex_t> iqvWindowed_;
    Complex_t *iqvWindowed = iqvWindowed_.alloc(_nSamples);
    
    if (_moments_params.window == Params::WINDOW_HANNING) {
      _momentsHalf.applyHanningWindow(iqv, iqvWindowed);
    } else if (_moments_params.window == Params::WINDOW_BLACKMAN) {
      _momentsHalf.applyBlackmanWindow(iqv, iqvWindowed);
    } else {
      _momentsHalf.applyRectWindow(iqv, iqvWindowed);
    }

    // save gate spectra

    gateSpectra[igate]->setIqH(iqh);
    gateSpectra[igate]->setIqHWindowed(iqhWindowed);
    gateSpectra[igate]->setIqV(iqv);
    gateSpectra[igate]->setIqVWindowed(iqvWindowed);

    // compute power, vel and width for each channel
    
    double power_h = _missingDbl, vel_h = _missingDbl, width_h = _missingDbl;
    double power_v = _missingDbl, vel_v = _missingDbl, width_2 = _missingDbl;
    double noise_h = _missingDbl, noise_v = _missingDbl;
    ClutProb clutProb_h, clutProb_v;
    clutProb_h.setRatioInnerIndex(_params.clutter_ratio_inner_index);
    clutProb_h.setRatioOuterIndex(_params.clutter_ratio_outer_index);
    clutProb_v.setRatioInnerIndex(_params.clutter_ratio_inner_index);
    clutProb_v.setRatioOuterIndex(_params.clutter_ratio_outer_index);
    int flags_h = 0, flags_v = 0;

    if (_useFft) {
      // specify rectangular window since window has already been applied
      _momentsHalf.computeByFft(iqhWindowed, Moments::WINDOW_RECT,
                                prt, power_h, noise_h,
                                vel_h, width_h, flags_h, &clutProb_h);
      _momentsHalf.computeByFft(iqvWindowed, Moments::WINDOW_RECT,
                                prt, power_v, noise_v,
                                vel_v, width_2, flags_v, &clutProb_v);
    } else {
      _momentsHalf.computeByPp(iqhWindowed, prt,
                               power_h, vel_h, width_h, flags_h);
      _momentsHalf.computeByPp(iqvWindowed, prt,
                               power_v, vel_v, width_2, flags_v);
      if (_params.apply_cmd) {
        _momentsHalf.computeClutProb(iqhWindowed, Moments::WINDOW_RECT,
                                     prt, clutProb_h);
        _momentsHalf.computeClutProb(iqvWindowed, Moments::WINDOW_RECT,
                                     prt, clutProb_v);
      }
    }
    
    // adjust for saturation value

    power_h *= _opsInfo.getSaturationMult();
    power_v *= _opsInfo.getSaturationMult();
    
    // compute snr
    
    double snr_h = _missingDbl;
    double dbm_h = 10.0 * log10(power_h);
    if (power_h <= _noiseFixedChan0) {
      power_h = _noiseFixedChan0 + 1.0e-20;
    }
    snr_h = 10.0 * log10((power_h - _noiseFixedChan0) / _noiseFixedChan0);
    
    double snr_v = _missingDbl;
    double dbm_v = 10.0 * log10(power_v);
    if (power_v <= _noiseFixedChan0) {
      power_v = _noiseFixedChan0 + 1.0e-20;
    }
    snr_v = 10.0 * log10((power_v - _noiseFixedChan0) / _noiseFixedChan0);
    
    // check for SNR

    if (snr_h == _missingDbl || snr_v == _missingDbl) {
      // below SNR
      fields[igate].flags = flags_h | flags_v;
    }

    // average power and SNR
    
    double dbmMean = (dbm_h + dbm_v) / 2.0;
    double snrMean = (snr_h + snr_v) / 2.0;
    fields[igate].dbm = dbmMean;
    fields[igate].snr = snrMean;
    
    double dbzChan0 = snr_h + _dbz0Chan0 + _rangeCorr[igate];
    double dbzChan1 = snr_v + _dbz0Chan1 + _rangeCorr[igate];
    fields[igate].dbz = (dbzChan0 + dbzChan1) / 2.0;

    // width from half-spectra

    fields[igate].width = (width_h + width_2) / 2.0;
    
    // zdr
    
    // fields[igate].zdr = dbzChan0 - dbzChan1 + _params.zdr_correction;
    fields[igate].zdr = snr_h - snr_v;

//     if (igate < 5) {
//       cerr << "1111 igate, snr_h, snr_v, zdr: " << igate << ", "
//            << snr_h << ", " << snr_v << ", " << fields[igate].zdr << endl;
//     }

    // phidp and velocity
    
    Complex_t Rhhvv1 = _meanConjugateProduct(iqvWindowed, iqhWindowed + 1,
                                             _nSamplesHalf - 1);
    Complex_t Rvvhh1 = _meanConjugateProduct(iqhWindowed + 1, iqvWindowed + 1,
                                             _nSamplesHalf - 1);
    
    double argRhhvv1 = _arg(Rhhvv1);
    double argRvvhh1 = _arg(Rvvhh1);

    double phiDpRad = (argRhhvv1 - argRvvhh1) / 2.0;
    fields[igate].phidp = phiDpRad * RAD_TO_DEG;
    
    double argVelhhvv = phiDpRad - argRhhvv1;
    double argVelvvhh = - phiDpRad - argRvvhh1;
    double meanArgVel = (argVelhhvv + argVelvvhh) / 2.0;
    double meanVel = (meanArgVel / M_PI) * nyquist;
    fields[igate].vel = meanVel* -1.0;
    
    // rhohv

    double Phh = _meanPower(iqhWindowed + 1, _nSamplesHalf - 1);
    double Pvv = _meanPower(iqvWindowed + 1, _nSamplesHalf - 1);
    double rhohhvv1 = _mag(Rhhvv1) / sqrt(Phh * Pvv);

    Complex_t Rhhhh2 = _meanConjugateProduct(iqhWindowed, iqhWindowed + 1,
                                             _nSamplesHalf - 1);
    Complex_t Rvvvv2 = _meanConjugateProduct(iqvWindowed, iqvWindowed + 1,
                                             _nSamplesHalf - 1);
    double rhohh2 = _mag(Rhhhh2) / Phh;
    
    double rhohhvv0 = rhohhvv1 / pow(rhohh2, 0.25);
    if (rhohhvv0 > 1.0) {
      rhohhvv0 = 1.0;
    }
    fields[igate].rhohv = rhohhvv0;

    // clutter probability
    
    ClutProb clutProb;
    clutProb.setRatioInnerIndex(_params.clutter_ratio_inner_index);
    clutProb.setRatioOuterIndex(_params.clutter_ratio_outer_index);
    clutProb.combine(clutProb_h, clutProb_v);

    if (_params.apply_cmd) {
      fields[igate].cmd_ratio_narrow = clutProb.getRatioNarrow();
      fields[igate].cmd_ratio_wide = clutProb.getRatioWide();
      fields[igate].cmd_clut2wx_sep = clutProb.getClutWxPeakSeparation();
      fields[igate].cmd_clut2wx_ratio = clutProb.getClutWxPeakRatio();
    }
    
    // adjust phase on the H pulses by phidp
    
    Complex_t phiDpComplex;
    phiDpComplex.re = cos(phiDpRad * 2.0);
    phiDpComplex.im = sin(phiDpRad * 2.0);
    
    Complex_t *iq = IQ[igate];
    TaArray<Complex_t> iq2_;
    Complex_t *iq2 = iq2_.alloc(_nSamples);
    memcpy(iq2, iq, _nSamples * sizeof(Complex_t));
    for (int ii = 0; ii < _nSamples; ii += 2) {
      Complex_t adjusted = _complexProduct(iq[ii], phiDpComplex);
      iq2[ii] = adjusted;
    }
    gateSpectra[igate]->setIq(iq2);

  } // igate

  // compute kdp
  
  _computeKdp(nGatesPulse, fields);
  
  if (_needRefractVars) {
    _computeRefractDualPol(nGatesPulse, gateSpectra, fields);
  }

  // compute SDZDR as test field
  // _computePulseSdZdr(nGatesPulse, IQH, IQV, fields);
  // _computePulseSdPhidp(nGatesPulse, IQH, IQV, fields);

}

#ifdef USING_CHILL_CODE

/////////////////////////////////////////////////
// compute moments in alternating dual pol mode
//
// Set combinedSpectraFile to NULL to prevent printing.

void MomentsMgr::computeDualAlt(time_t beamTime,
                                double el, double az, double prt,
                                int nGatesPulse,
                                Complex_t **IQ,
                                Complex_t **IQH,
                                Complex_t **IQV,
                                int &combinedPrintCount,
                                FILE *combinedSpectraFile,
                                Fields *fields,
                                vector<GateSpectra *> gateSpectra)

{

  // initialize based on OpsInfo
  
  _init();
  
  double wavelengthMeters = _opsInfo.get_radar_wavelength_cm() / 100.0;
  double nyquist = ((wavelengthMeters / prt) / 4.0);

  // compute phidp offset, to prevent premature wrapping of the phase
  // vectors from which phidp and velocity are computed.

  Complex_t phidpOffset;
  if (_params.correct_for_system_phidp) {
    phidpOffset.re = cos((_params.system_phidp - _phidpPhaseLimit) * DEG_TO_RAD * -1.0);
    phidpOffset.im = sin((_params.system_phidp - _phidpPhaseLimit) * DEG_TO_RAD * -1.0);
  } else {
    phidpOffset.re = 1.0;
    phidpOffset.im = 0.0;
  }

  if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
    cerr << "--->> computing dual pol moments, el, az, nGates, time: "
  	 << el << ", " << az << ", " << nGatesPulse << ", "
  	 << DateTime::str(beamTime) << endl;
  }
  
  double range = _startRange;
  for (int igate = 0; igate < nGatesPulse; igate++, range += _gateSpacing) {
    
    Complex_t *iqh = IQH[igate];
    Complex_t *iqv = IQV[igate];

    // apply window as appropriate
    
    TaArray<Complex_t> iqhWindowed_;
    Complex_t *iqhWindowed = iqhWindowed_.alloc(_nSamples);
    
    if (_moments_params.window == Params::WINDOW_HANNING) {
      _momentsHalf.applyHanningWindow(iqh, iqhWindowed);
    } else if (_moments_params.window == Params::WINDOW_BLACKMAN) {
      _momentsHalf.applyBlackmanWindow(iqh, iqhWindowed);
    } else {
      _momentsHalf.applyRectWindow(iqh, iqhWindowed);
    }

    TaArray<Complex_t> iqvWindowed_;
    Complex_t *iqvWindowed = iqvWindowed_.alloc(_nSamples);
    
    if (_moments_params.window == Params::WINDOW_HANNING) {
      _momentsHalf.applyHanningWindow(iqv, iqvWindowed);
    } else if (_moments_params.window == Params::WINDOW_BLACKMAN) {
      _momentsHalf.applyBlackmanWindow(iqv, iqvWindowed);
    } else {
      _momentsHalf.applyRectWindow(iqv, iqvWindowed);
    }

    // save gate spectra

    gateSpectra[igate]->setIqH(iqh);
    gateSpectra[igate]->setIqHWindowed(iqhWindowed);
    gateSpectra[igate]->setIqV(iqv);
    gateSpectra[igate]->setIqVWindowed(iqvWindowed);

    // compute power, vel and width for each channel
    
    double power_h = _missingDbl, vel_h = _missingDbl, width_h = _missingDbl;
    double power_v = _missingDbl, vel_v = _missingDbl, width_2 = _missingDbl;
    double noise_h = _missingDbl, noise_v = _missingDbl;
    ClutProb clutProb_h, clutProb_v;
    clutProb_h.setRatioInnerIndex(_params.clutter_ratio_inner_index);
    clutProb_h.setRatioOuterIndex(_params.clutter_ratio_outer_index);
    clutProb_v.setRatioInnerIndex(_params.clutter_ratio_inner_index);
    clutProb_v.setRatioOuterIndex(_params.clutter_ratio_outer_index);
    int flags_h = 0, flags_v = 0;

    if (_useFft) {
      // specify rectangular window since window has already been applied
      _momentsHalf.computeByFft(iqhWindowed, Moments::WINDOW_RECT,
                                prt, power_h, noise_h,
                                vel_h, width_h, flags_h, &clutProb_h);
      _momentsHalf.computeByFft(iqvWindowed, Moments::WINDOW_RECT,
                                prt, power_v, noise_v,
                                vel_v, width_2, flags_v, &clutProb_v);
    } else {
      _momentsHalf.computeByPp(iqhWindowed, prt,
                               power_h, vel_h, width_h, flags_h);
      _momentsHalf.computeByPp(iqvWindowed, prt,
                               power_v, vel_v, width_2, flags_v);
      if (_params.apply_cmd) {
        _momentsHalf.computeClutProb(iqhWindowed, Moments::WINDOW_RECT,
                                     prt, clutProb_h);
        _momentsHalf.computeClutProb(iqvWindowed, Moments::WINDOW_RECT,
                                     prt, clutProb_v);
      }
    }
    
    // compute snr
    
    double snr_h = _missingDbl;
    double dbm_h = 10.0 * log10(power_h);
    if (power_h <= _noiseFixedChan0) {
      power_h = _noiseFixedChan0 + 1.0e-20;
    }
    snr_h = 10.0 * log10((power_h - _noiseFixedChan0) / _noiseFixedChan0);
    
    double snr_v = _missingDbl;
    double dbm_v = 10.0 * log10(power_v);
    if (power_v <= _noiseFixedChan0) {
      power_v = _noiseFixedChan0 + 1.0e-20;
    }
    snr_v = 10.0 * log10((power_v - _noiseFixedChan0) / _noiseFixedChan0);
    
    // check for SNR

    if (snr_h == _missingDbl || snr_v == _missingDbl) {
      // below SNR
      fields[igate].flags = flags_h | flags_v;
    }

    // average power and SNR
    
    double dbmMean = (dbm_h + dbm_v) / 2.0;
    double snrMean = (snr_h + snr_v) / 2.0;
    fields[igate].dbm = dbmMean;
    fields[igate].snr = snrMean;
    
    double dbzChan0 = snr_h + _dbz0Chan0 + _rangeCorr[igate];
    double dbzChan1 = snr_v + _dbz0Chan1 + _rangeCorr[igate];
    fields[igate].dbz = (dbzChan0 + dbzChan1) / 2.0;

    // width from half-spectra

    // fields[igate].width = (width_h + width_2) / 2.0;
    
    // zdr
    
    // fields[igate].zdr = dbzChan0 - dbzChan1 + _params.zdr_correction;
    fields[igate].zdr = snr_h - snr_v;

    ////////////////////////////
    // phidp, velocity and rhohv
    //
    // See A. Zahrai and D. Zrnic, "The 10 cm wavelength polarimetric weather radar
    // at NOAA'a National Severe Storms Lab. JTech, Vol 10, No 5, October 1993.

    // compute correlation V to H

    Complex_t Ra = _meanConjugateProduct(iqvWindowed, iqhWindowed,
                                         _nSamplesHalf - 1);

    // compute correlation H to V

    Complex_t Rb = _meanConjugateProduct(iqhWindowed + 1, iqvWindowed,
                                         _nSamplesHalf - 1);
    
    // Correct for system PhiDp offset, so that phidp will not wrap prematurely

    Complex_t RaPrime = _complexProduct(Ra, phidpOffset);
    Complex_t RbPrime = _conjugateProduct(Rb, phidpOffset);
    
    // compute angular difference between them, which is phidp
    
    Complex_t RaRbconj = _conjugateProduct(RaPrime, RbPrime);
    double phidpRad = _arg(RaRbconj) / 2.0;
    fields[igate].phidp = phidpRad * RAD_TO_DEG;

    // velocity phase is mean of phidp phases

    Complex_t velVect = _complexMean(RaPrime, RbPrime);
    double argVel = _arg(velVect);
    double meanVel = (argVel / M_PI) * nyquist;
    fields[igate].vel = meanVel * -1.0;

    //////////
    // rhohv

    // power HH and VV
    
    double Phh = _meanPower(iqhWindowed + 1, _nSamplesHalf - 1);
    double Pvv = _meanPower(iqvWindowed + 1, _nSamplesHalf - 1);

    double PhhCorr = Phh - _noiseFixedChan0;
    if (PhhCorr < 0) {
      PhhCorr = 1.0e-20;
    }
    double PvvCorr = Pvv - _noiseFixedChan0;
    if (PvvCorr < 0) {
      PvvCorr = 1.0e-20;
    }

    // lag-2 correlations for HH and VV

    Complex_t Rhhhh2 = _meanConjugateProduct(iqhWindowed, iqhWindowed + 1,
                                             _nSamplesHalf - 1);
    Complex_t Rvvvv2 = _meanConjugateProduct(iqvWindowed, iqvWindowed + 1,
                                             _nSamplesHalf - 1);

    // compute lag-2 rho

    Complex_t sumR2 = _complexSum(Rhhhh2, Rvvvv2);
    double magSumR2 = _mag(sumR2);
    double rho2 = magSumR2 / (Phh + Pvv);
    if (rho2 > 1.0) {
      rho2 = 1.0;
    } else if (rho2 < 0) {
      rho2 = 0;
    }

    // compute lag-1 rhohv

    double magRa = _mag(Ra);
    double magRb = _mag(Rb);
    double rhohv1 = (magRa + magRb) / (2.0 * sqrt(Phh * Pvv));
    
    // lag-0 rhohv is rhohv1 corrected by rho2

    double rhohv0 = rhohv1 / pow(rho2, 0.25);
    fields[igate].rhohv = rhohv0;

    // spectrum width

    // if (_params.pp_width_estimator == Params::R1_R2) {
      double argWidth = sqrt(-0.5 * log(rho2));
      double width = (argWidth / M_PI) * nyquist;
      fields[igate].width = width;
//     } else {
//       double argWidth = sqrt(-0.5 * log(rho2));
//       double width = (argWidth / M_PI) * nyquist;
//       fields[igate].width = width;
//     }

//       if (Phh > 1.0e-6) {

        double rho2X = magSumR2 / (PhhCorr + PvvCorr);
        if (rho2X > 1.0) {
          rho2X = 1.0;
        } else if (rho2X < 0) {
          rho2X = 0;
        }
        double argWidthX = sqrt(-0.5 * log(rho2X));
        double widthX = (argWidthX / M_PI) * nyquist;

//         cerr << "Phh, Pvv, _noiseFixedChan0, PhhCorr, PvvCorr: "
//              << Phh << ", " << Pvv << ", " << _noiseFixedChan0 << ", "
//              << PhhCorr << ", " << PvvCorr << endl;
//         cerr << "rho2, rho2X: " << rho2 << ", " << rho2X << endl;
//         cerr << "width, widthX: " << width << ", " << widthX << endl;

         fields[igate].width = widthX;

//       }

    // clutter probability
    
    ClutProb clutProb;
    clutProb.setRatioInnerIndex(_params.clutter_ratio_inner_index);
    clutProb.setRatioOuterIndex(_params.clutter_ratio_outer_index);
    clutProb.combine(clutProb_h, clutProb_v);

    if (_params.apply_cmd) {
      fields[igate].cmd_ratio_narrow = clutProb.getRatioNarrow();
      fields[igate].cmd_ratio_wide = clutProb.getRatioWide();
      fields[igate].cmd_clut2wx_sep = clutProb.getClutWxPeakSeparation();
      fields[igate].cmd_clut2wx_ratio = clutProb.getClutWxPeakRatio();
    }
    
    // adjust phase on the H pulses by phidp
    
    Complex_t phidpComplex;
    phidpComplex.re = cos(phidpRad * 2.0);
    phidpComplex.im = sin(phidpRad * 2.0);
    
    Complex_t *iq = IQ[igate];
    TaArray<Complex_t> iq2_;
    Complex_t *iq2 = iq2_.alloc(_nSamples);
    memcpy(iq2, iq, _nSamples * sizeof(Complex_t));
    for (int ii = 0; ii < _nSamples; ii += 2) {
      Complex_t adjusted = _complexProduct(iq[ii], phidpComplex);
      iq2[ii] = adjusted;
    }
    gateSpectra[igate]->setIq(iq2);

  } // igate

  // compute kdp
  
  _computeKdp(nGatesPulse, fields);
  
  if (_needRefractVars) {
    _computeRefractDualPol(nGatesPulse, gateSpectra, fields);
  }

  // compute SDZDR as test field
  // _computePulseSdZdr(nGatesPulse, IQH, IQV, fields);
  // _computePulseSdPhidp(nGatesPulse, IQH, IQV, fields);

}

#endif

///////////////////////////////////////////////////////////
// compute time-domain moments in alternating dual pol mode
//
// Set combinedSpectraFile to NULL to prevent printing.

void MomentsMgr::computeTdDualAlt(time_t beamTime,
                                  double el, double az, double prt,
                                  int nGatesPulse,
                                  Complex_t **IQHC,
                                  Complex_t **IQVC,
                                  Complex_t **IQHX,
                                  Complex_t **IQVX,
                                  int &combinedPrintCount,
                                  FILE *combinedSpectraFile,
                                  Fields *fields,
                                  vector<GateSpectra *> gateSpectra)

{

  // initialize based on OpsInfo
  
  _init();
  
  double wavelengthMeters = _opsInfo.getWavelengthCm() / 100.0;
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
  for (int igate = 0; igate < nGatesPulse; igate++, range += _gateSpacing) {
    
    // locate the IQ samples for this gate

    Complex_t *iqhc = IQHC[igate];
    Complex_t *iqvc = IQVC[igate];
    Complex_t *iqhx = IQHX[igate];
    Complex_t *iqvx = IQVX[igate];

    // apply window as appropriate
    
    TaArray<Complex_t> iqhcW_, iqvcW_;
    Complex_t *iqhcW = iqhcW_.alloc(_nSamples);
    Complex_t *iqvcW = iqvcW_.alloc(_nSamples);
    _applyWindow(iqhc, iqhcW, _momentsHalf);
    _applyWindow(iqvc, iqvcW, _momentsHalf);

    TaArray<Complex_t> iqhxW_, iqvxW_;
    Complex_t *iqhxW = iqhxW_.alloc(_nSamples);
    Complex_t *iqvxW = iqvxW_.alloc(_nSamples);
    _applyWindow(iqhx, iqhxW, _momentsHalf);
    _applyWindow(iqvx, iqvxW, _momentsHalf);

    // save gate samples

    gateSpectra[igate]->setIqH(iqhc);
    gateSpectra[igate]->setIqHWindowed(iqhcW);
    gateSpectra[igate]->setIqV(iqvc);
    gateSpectra[igate]->setIqVWindowed(iqvcW);

    // compute lagged covariances
    
    double lag0_hc = _meanPower(iqhcW, _nSamplesHalf);
    double lag0_vc = _meanPower(iqvcW, _nSamplesHalf);
    double lag0_hx = _meanPower(iqhxW, _nSamplesHalf);
    double lag0_vx = _meanPower(iqvxW, _nSamplesHalf);

    // compute noise-subtracted lag0

    double lag0_hc_ns = lag0_hc - _noiseFixedChan0;
    double lag0_vc_ns = lag0_vc - _noiseFixedChan0;
    double lag0_hx_ns = lag0_hx - _noiseFixedChan1;
    double lag0_vx_ns = lag0_vx - _noiseFixedChan1;

    if (lag0_hc_ns < 0) { lag0_hc_ns = 0.0; }
    if (lag0_vc_ns < 0) { lag0_vc_ns = 0.0; }
    if (lag0_hx_ns < 0) { lag0_hx_ns = 0.0; }
    if (lag0_vx_ns < 0) { lag0_vx_ns = 0.0; }

    // compute snr

    double snr_hc = lag0_hc_ns / _noiseFixedChan0;
    double snr_vc = lag0_vc_ns / _noiseFixedChan0;
//     double snr_hx = lag0_hx_ns / _noiseFixedChan1;
//     double snr_vx = lag0_vx_ns / _noiseFixedChan1;

    double snrMean = (snr_hc + snr_vc) / 2.0;
    double snrDb = _missingDbl;
    bool snrOK = false;
    if (snrMean > 0) {
      snrDb = 10.0 * log10(snrMean);
    }
    if (snrDb >= _params.moments_snr_threshold) {
      fields[igate].snr = snrDb;
      snrOK = true;
      fields[igate].flags = 0;
    } else {
      fields[igate].snr = _missingDbl;
      fields[igate].flags = 1;
    }
    
    // compute dbz and dbm

    if (snrOK) {

      double dbz_hc = 10.0 * log10(snr_hc) + _dbz0Chan0 + _rangeCorr[igate];
      double dbz_vc = 10.0 * log10(snr_vc) + _dbz0Chan1 + _rangeCorr[igate];
      fields[igate].dbz = (dbz_hc + dbz_vc) / 2.0;

      double dbmMean = (lag0_hc + lag0_vc) / 2.0;
      fields[igate].dbm = dbmMean;
     
    }

    // zdr
    
    fields[igate].zdr = snr_hc - snr_vc + _params.zdr_correction;


    // compute power, vel and width for each channel
    
//     double power_h = _missingDbl, vel_h = _missingDbl, width_h = _missingDbl;
//     double power_v = _missingDbl, vel_v = _missingDbl, width_2 = _missingDbl;
//     double noise_h = _missingDbl, noise_v = _missingDbl;
//     ClutProb clutProb_h, clutProb_v;
//     clutProb_h.setRatioInnerIndex(_params.clutter_ratio_inner_index);
//     clutProb_h.setRatioOuterIndex(_params.clutter_ratio_outer_index);
//     clutProb_v.setRatioInnerIndex(_params.clutter_ratio_inner_index);
//     clutProb_v.setRatioOuterIndex(_params.clutter_ratio_outer_index);
//     int flags_h = 0, flags_v = 0;

//     if (_useFft) {
//       // specify rectangular window since window has already been applied
//       _momentsHalf.computeByFft(iqhcW, Moments::WINDOW_RECT,
//                                 prt, power_h, noise_h,
//                                 vel_h, width_h, flags_h, &clutProb_h);
//       _momentsHalf.computeByFft(iqvcW, Moments::WINDOW_RECT,
//                                 prt, power_v, noise_v,
//                                 vel_v, width_2, flags_v, &clutProb_v);
//     } else {
//       _momentsHalf.computeByPp(iqhcW, prt,
//                                power_h, vel_h, width_h, flags_h);
//       _momentsHalf.computeByPp(iqvcW, prt,
//                                power_v, vel_v, width_2, flags_v);
//       if (_params.apply_cmd) {
//         _momentsHalf.computeClutProb(iqhcW, Moments::WINDOW_RECT,
//                                      prt, clutProb_h);
//         _momentsHalf.computeClutProb(iqvcW, Moments::WINDOW_RECT,
//                                      prt, clutProb_v);
//       }
//     }
    
    // compute snr
    
//     double snr_h = _missingDbl;
//     double dbm_h = 10.0 * log10(power_h);
//     if (power_h <= _noiseFixedChan0) {
//       power_h = _noiseFixedChan0 + 1.0e-20;
//     }
//     snr_h = 10.0 * log10((power_h - _noiseFixedChan0) / _noiseFixedChan0);
    
//     double snr_v = _missingDbl;
//     double dbm_v = 10.0 * log10(power_v);
//     if (power_v <= _noiseFixedChan0) {
//       power_v = _noiseFixedChan0 + 1.0e-20;
//     }
//     snr_v = 10.0 * log10((power_v - _noiseFixedChan0) / _noiseFixedChan0);
    
//     // check for SNR

//     if (snr_h == _missingDbl || snr_v == _missingDbl) {
//       // below SNR
//       fields[igate].flags = flags_h | flags_v;
//     }

    // average power and SNR
    
//     double dbmMean = (dbm_h + dbm_v) / 2.0;
//     // double snrMean = (snr_h + snr_v) / 2.0;
//     fields[igate].dbm = dbmMean;
    // fields[igate].snr = snrMean;
    
//     double dbzChan0 = snr_h + _dbz0Chan0 + _rangeCorr[igate];
//     double dbzChan1 = snr_v + _dbz0Chan1 + _rangeCorr[igate];
//     fields[igate].dbz = (dbzChan0 + dbzChan1) / 2.0;

    // width from half-spectra

    // fields[igate].width = (width_h + width_2) / 2.0;
    
    // zdr
    
    // fields[igate].zdr = dbzChan0 - dbzChan1 + _params.zdr_correction;
    // fields[igate].zdr = snr_h - snr_v;

    ////////////////////////////
    // phidp, velocity and rhohv
    //
    // See A. Zahrai and D. Zrnic, "The 10 cm wavelength polarimetric weather radar
    // at NOAA'a National Severe Storms Lab. JTech, Vol 10, No 5, October 1993.

    // compute correlation V to H

    Complex_t Ra = _meanConjugateProduct(iqvcW, iqhcW,
                                         _nSamplesHalf - 1);

    // compute correlation H to V

    Complex_t Rb = _meanConjugateProduct(iqhcW + 1, iqvcW,
                                         _nSamplesHalf - 1);
    
    // Correct for system PhiDp offset, so that phidp will not wrap prematurely

    Complex_t RaPrime = _complexProduct(Ra, phidpOffset);
    Complex_t RbPrime = _conjugateProduct(Rb, phidpOffset);
    
    // compute angular difference between them, which is phidp
    
    Complex_t RaRbconj = _conjugateProduct(RaPrime, RbPrime);
    double phidpRad = _arg(RaRbconj) / 2.0;
    fields[igate].phidp = phidpRad * RAD_TO_DEG;

    // velocity phase is mean of phidp phases

    Complex_t velVect = _complexMean(RaPrime, RbPrime);
    double argVel = _arg(velVect);
    double meanVel = (argVel / M_PI) * nyquist;
    fields[igate].vel = meanVel * -1.0;

    //////////
    // rhohv

    // power HH and VV
    
    double Phh = _meanPower(iqhcW + 1, _nSamplesHalf - 1);
    double Pvv = _meanPower(iqvcW + 1, _nSamplesHalf - 1);

    double PhhCorr = Phh - _noiseFixedChan0;
    if (PhhCorr < 0) {
      PhhCorr = 1.0e-20;
    }
    double PvvCorr = Pvv - _noiseFixedChan0;
    if (PvvCorr < 0) {
      PvvCorr = 1.0e-20;
    }

    // lag-2 correlations for HH and VV

    Complex_t Rhhhh2 = _meanConjugateProduct(iqhcW, iqhcW + 1,
                                             _nSamplesHalf - 1);
    Complex_t Rvvvv2 = _meanConjugateProduct(iqvcW, iqvcW + 1,
                                             _nSamplesHalf - 1);

    // compute lag-2 rho

    Complex_t sumR2 = _complexSum(Rhhhh2, Rvvvv2);
    double magSumR2 = _mag(sumR2);
    double rho2 = magSumR2 / (Phh + Pvv);
    if (rho2 > 1.0) {
      rho2 = 1.0;
    } else if (rho2 < 0) {
      rho2 = 0;
    }

    // compute lag-1 rhohv

    double magRa = _mag(Ra);
    double magRb = _mag(Rb);
    double rhohv1 = (magRa + magRb) / (2.0 * sqrt(Phh * Pvv));
    
    // lag-0 rhohv is rhohv1 corrected by rho2

    double rhohv0 = rhohv1 / pow(rho2, 0.25);
    fields[igate].rhohv = rhohv0;

    // spectrum width

    // if (_params.pp_width_estimator == Params::R1_R2) {
      double argWidth = sqrt(-0.5 * log(rho2));
      double width = (argWidth / M_PI) * nyquist;
      fields[igate].width = width;
//     } else {
//       double argWidth = sqrt(-0.5 * log(rho2));
//       double width = (argWidth / M_PI) * nyquist;
//       fields[igate].width = width;
//     }

//       if (Phh > 1.0e-6) {

        double rho2X = magSumR2 / (PhhCorr + PvvCorr);
        if (rho2X > 1.0) {
          rho2X = 1.0;
        } else if (rho2X < 0) {
          rho2X = 0;
        }
        double argWidthX = sqrt(-0.5 * log(rho2X));
        double widthX = (argWidthX / M_PI) * nyquist;

//         cerr << "Phh, Pvv, _noiseFixedChan0, PhhCorr, PvvCorr: "
//              << Phh << ", " << Pvv << ", " << _noiseFixedChan0 << ", "
//              << PhhCorr << ", " << PvvCorr << endl;
//         cerr << "rho2, rho2X: " << rho2 << ", " << rho2X << endl;
//         cerr << "width, widthX: " << width << ", " << widthX << endl;

         fields[igate].width = widthX;

//       }

    // clutter probability
    
//     ClutProb clutProb;
//     clutProb.setRatioInnerIndex(_params.clutter_ratio_inner_index);
//     clutProb.setRatioOuterIndex(_params.clutter_ratio_outer_index);
//     clutProb.combine(clutProb_h, clutProb_v);

//     if (_params.apply_cmd) {
//       fields[igate].cmd_ratio_narrow = clutProb.getRatioNarrow();
//       fields[igate].cmd_ratio_wide = clutProb.getRatioWide();
//       fields[igate].cmd_clut2wx_sep = clutProb.getClutWxPeakSeparation();
//       fields[igate].cmd_clut2wx_ratio = clutProb.getClutWxPeakRatio();
//     }
    
  } // igate

  // compute kdp
  
  _computeKdp(nGatesPulse, fields);
  
  if (_needRefractVars) {
    _computeRefractDualPol(nGatesPulse, gateSpectra, fields);
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
                               Fields *fields,
			       vector<GateSpectra *> gateSpectra)
  
{

  _init();
  
  // copy over unfiltered data first

  for (int igate = 0; igate < nGatesPulse; igate++) {
    fields[igate].dbzf = fields[igate].dbz;
    fields[igate].velf = fields[igate].vel;
    fields[igate].widthf = fields[igate].width;
  }
  
  for (int igate = 0; igate < nGatesPulse; igate++) {

    if (fields[igate].cmd_flag) {
      
      const Complex_t *iq = gateSpectra[igate]->getIq();
      double power_0 = _missingDbl;
      double vel_0 = _missingDbl;
      double width_0 = _missingDbl;
      double clut_0 = _missingDbl;
      double noise_0 = _missingDbl;

      if (_params.clutter_filter_window == Params::WINDOW_HANNING) {
        _moments.computeByFftFilterClutter(iq, Moments::WINDOW_HANNING, prt,
                                           power_0, vel_0, width_0,
                                           clut_0, noise_0);
      } else if (_params.clutter_filter_window == Params::WINDOW_BLACKMAN) {
        _moments.computeByFftFilterClutter(iq, Moments::WINDOW_BLACKMAN, prt,
                                           power_0, vel_0, width_0,
                                           clut_0, noise_0);
      } else {
        _moments.computeByFftFilterClutter(iq, Moments::WINDOW_RECT, prt,
                                           power_0, vel_0, width_0,
                                           clut_0, noise_0);
      }
      
      double snr_0 = 0.5;
      if (power_0 > _noiseFixedChan0) {
        snr_0 = 0.5 +
          10.0 * log10((power_0 - _noiseFixedChan0) / _noiseFixedChan0);
      }
      fields[igate].dbzf = snr_0 + _dbz0Chan0 + _rangeCorr[igate];
      fields[igate].velf = vel_0;
      fields[igate].widthf = width_0;
      fields[igate].clut = fields[igate].dbz - fields[igate].dbzf;

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
				 const Complex_t *delta12,
                                 Fields *fields,
				 vector<GateSpectra *> gateSpectra)
  
{
  
  _init();
  
  // copy over unfiltered data first

  for (int igate = 0; igate < nGatesPulse; igate++) {
    fields[igate].dbzf = fields[igate].dbz;
    fields[igate].velf = fields[igate].vel;
    fields[igate].widthf = fields[igate].width;
  }
  
  for (int trip1Gate = 0; trip1Gate < nGatesPulse; trip1Gate++) {

    int trip2Gate = trip1Gate + nGatesPulse;
    bool trip1IsStrong = (fields[trip1Gate].sz_trip_flag == 1);

    // determine the presence of clutter in strong and weak trip

    bool clutterInStrong = false;
    bool clutterInWeak = false;
    
    if (trip1IsStrong) {
      if (fields[trip1Gate].cmd_flag) {
	clutterInStrong = true;
      }
      if (fields[trip2Gate].cmd_flag) {
	clutterInWeak = true;
      }
    } else {
      // trip 1 is weak
      if (fields[trip1Gate].cmd_flag) {
	clutterInWeak = true;
      }
      if (fields[trip2Gate].cmd_flag) {
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
    if (powerStrong > _noiseFixedChan0) {
      snrStrong =
        10.0 * log10((powerStrong - _noiseFixedChan0) / _noiseFixedChan0);
    }
    if (powerWeak > _noiseFixedChan0) {
      snrStrong =
        10.0 * log10((powerWeak - _noiseFixedChan0) / _noiseFixedChan0);
    }

    if (trip1IsStrong) {
      
      if (snrStrong != _missingDbl) {
	fields[trip1Gate].dbzf =
          snrStrong + _dbz0Chan0 + _rangeCorr[trip1Gate];
	if (clutStrong > 0) {
	  fields[trip1Gate].clut = 10.0 * log10(clutStrong / powerStrong);
	}
      }
      if (fields[trip1Gate].velf == _missingDbl ||
          velStrong != _missingDbl) {
	fields[trip1Gate].velf = velStrong;
      }
      if (fields[trip1Gate].widthf == _missingDbl ||
          widthStrong != _missingDbl) {
	fields[trip1Gate].widthf = widthStrong;
      }

      if (snrWeak != _missingDbl) {
	fields[trip2Gate].dbzf = snrWeak + _dbz0Chan0 + _rangeCorr[trip2Gate];
	if (clutWeak > 0) {
	  fields[trip2Gate].clut = 10.0 * log10(clutWeak / powerWeak);
	}
      }

      if (fields[trip2Gate].velf == _missingDbl ||
          velWeak != _missingDbl) {
	fields[trip2Gate].velf = velWeak;
      }
      if (fields[trip2Gate].widthf == _missingDbl ||
          widthWeak != _missingDbl) {
	fields[trip2Gate].widthf = widthWeak;
      }

    } else {

      if (snrStrong != _missingDbl) {
	fields[trip2Gate].dbzf =
          snrStrong + _dbz0Chan0 + _rangeCorr[trip2Gate];
	if (clutStrong > 0) {
	  fields[trip2Gate].clut = 10.0 * log10(clutStrong / powerStrong);
	}
      }
      if (fields[trip2Gate].velf == _missingDbl ||
          velStrong != _missingDbl) {
	fields[trip2Gate].velf = velStrong;
      }
      if (fields[trip2Gate].widthf == _missingDbl ||
          widthStrong != _missingDbl) {
	fields[trip2Gate].widthf = widthStrong;
      }

      if (snrWeak != _missingDbl) {
	fields[trip1Gate].dbzf = snrWeak + _dbz0Chan0 + _rangeCorr[trip1Gate];
	if (clutWeak > 0) {
	  fields[trip1Gate].clut = 10.0 * log10(clutWeak / powerWeak);
	}
      }
      if (fields[trip1Gate].velf == _missingDbl ||
          velWeak != _missingDbl) {
	fields[trip1Gate].velf = velWeak;
      }
      if (fields[trip1Gate].widthf == _missingDbl ||
          widthWeak != _missingDbl) {
	fields[trip1Gate].widthf = widthWeak;
      }

    }
      
  } // trip1Gate

}

////////////////////////////////////////////////////////////////////////
// Filter clutter for Dual pol data
//
// compute moments, applying the clutter filter as appropriate.
//
// If cmd_flag is TRUE for a gate, the clutter filter is applied.
// If not, the unfiltered moments are copied over to the filtered arrays.

void MomentsMgr::filterClutterDualAlt(double prt,
				      int nGatesPulse,
				      Complex_t **IQ,
				      Complex_t **IQH,
				      Complex_t **IQV,
                                      Fields *fields,
				      vector<GateSpectra *> gateSpectra)
  
{

  _init();
  
  if (!_dualPol) {
    return;
  }

  double wavelengthMeters = _opsInfo.getWavelengthCm() / 100.0;
  double nyquist = ((wavelengthMeters / prt) / 4.0);
  double dbForDbRatio = _params.db_for_db_ratio;
  double dbForDbThreshold = _params.db_for_db_threshold;
  
  // copy over unfiltered data first

  for (int igate = 0; igate < nGatesPulse; igate++) {
    fields[igate].dbzf = fields[igate].dbz;
    fields[igate].velf = fields[igate].vel;
    fields[igate].widthf = fields[igate].width;
  }
  
  for (int igate = 0; igate < nGatesPulse; igate++) {
    
    if (!fields[igate].cmd_flag) {
      continue;
    }
      
    // filter IQ data for each channel
    
    Complex_t *iqh = IQH[igate];
    Complex_t *iqv = IQV[igate];
    
    TaArray<Complex_t> iqhFiltered_, iqvFiltered_;
    Complex_t *iqhFiltered = iqhFiltered_.alloc(_nSamplesHalf);
    Complex_t *iqvFiltered = iqvFiltered_.alloc(_nSamplesHalf);
    
    _momentsHalf.iqFilterClutter(iqh, _momentsWindow, prt, iqhFiltered);
    _momentsHalf.iqFilterClutter(iqv, _momentsWindow, prt, iqvFiltered);
    
    // form filtered time series
    
    TaArray<Complex_t> iqFiltered_;
    Complex_t *iqFiltered = iqFiltered_.alloc(_nSamples);
    for (int ii = 0, jj = 0; ii < _nSamplesHalf; ii++, jj += 2) {
      iqFiltered[jj] = iqhFiltered[ii];
      iqFiltered[jj + 1] = iqvFiltered[ii];
    }
    
    // compute moments from filtered time series
    
    double power_f = _missingDbl;
    double vel_f = _missingDbl, width_f = _missingDbl;
    double noise_f = _missingDbl;
    int flags_f = 0;
    
    if (_useFft) {
      _moments.computeByFft(iqFiltered, _momentsWindow, prt, power_f, noise_f,
                            vel_f, width_f, flags_f);
    } else {
      _moments.computeByPp(iqFiltered, prt, power_f, vel_f, width_f, flags_f);
    }

    if (power_f > _noiseFixedChan0) {
      double snr_f =
        10.0 * log10((power_f - _noiseFixedChan0) / _noiseFixedChan0);
      double dbz_f = snr_f + _dbz0Chan0 + _rangeCorr[igate];
      double clutDb = fields[igate].dbz - dbz_f;
      // db for db correction to clutter removed
      double clutDbCorrection = clutDb * dbForDbRatio;
      if (clutDb > dbForDbThreshold) {
        clutDbCorrection += (clutDb - dbForDbThreshold);
      }
      clutDb += clutDbCorrection;
      fields[igate].clut = clutDb;
      fields[igate].dbzf = fields[igate].dbz - clutDb;
    }
    fields[igate].velf = vel_f;
    fields[igate].widthf = width_f;

    // phidp and velocity
    
    Complex_t Rhhvv1 = _meanConjugateProduct(iqvFiltered, iqhFiltered + 1,
                                             _nSamplesHalf - 1);
    Complex_t Rvvhh1 = _meanConjugateProduct(iqhFiltered + 1, iqvFiltered + 1,
                                             _nSamplesHalf - 1);
    
    double argRhhvv1 = _arg(Rhhvv1);
    double argRvvhh1 = _arg(Rvvhh1);

    double phidpRad = (argRhhvv1 - argRvvhh1) / 2.0;
    
    double argVelhhvv = phidpRad - argRhhvv1;
    double argVelvvhh = - phidpRad - argRvvhh1;
    double meanArgVel = (argVelhhvv + argVelvvhh) / 2.0;
    double meanVel = (meanArgVel / M_PI) * nyquist;
    fields[igate].velf = meanVel* -1.0;

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

  if (_params.override_ts_cal) {
    _noiseFixedChan0 = pow(10.0, _params.hc_receiver.noise_db / 10.0);
    _noiseFixedChan1 = pow(10.0, _params.vc_receiver.noise_db / 10.0);
  } else {
    _noiseFixedChan0 = pow(10.0, _opsInfo.getNoiseDbm0() / 10.0);
    _noiseFixedChan1 = pow(10.0, _opsInfo.getNoiseDbm1() / 10.0);
  }

  // if noise sdev is negative, the dbz0 values for each channel
  // have been stored there

  if (_params.override_ts_cal) {
    _dbz0Chan0 = _params.hc_receiver.noise_db;
    _dbz0Chan1 = _params.vc_receiver.noise_db;
  } else {
    if (_opsInfo.getNoiseSdev0() < 0) {
      _dbz0Chan0 = _opsInfo.getNoiseSdev0();
      _dbz0Chan1 = _opsInfo.getNoiseSdev1();
    }
  }
  
  // set up moments object
  
  _moments.setWavelength(_opsInfo.getWavelengthCm() / 100.0);
  _moments.setNoiseValueDbm(_opsInfo.getNoiseDbm0());
  
  _momentsHalf.setWavelength(_opsInfo.getWavelengthCm() / 100.0);
  _momentsHalf.setNoiseValueDbm(_opsInfo.getNoiseDbm0());
  
}
     
////////////////////////////////////////
// Apply selected window to the IQ data

void MomentsMgr::_applyWindow(const Complex_t *iq,
                              Complex_t *iqW,
                              Moments &moments)
  
{
  
  if (_moments_params.window == Params::WINDOW_HANNING) {
    moments.applyHanningWindow(iq, iqW);
  } else if (_moments_params.window == Params::WINDOW_BLACKMAN) {
    moments.applyBlackmanWindow(iq, iqW);
  } else {
    moments.applyRectWindow(iq, iqW);
  }

}

///////////////////////////////////////
// compute range correction table

void MomentsMgr::_computeRangeCorrection()

{
  
  for (int i = 0; i < _maxGates; i++) {
    double range_km = _startRange + i * _gateSpacing;
    if (range_km <= 1.0) {
      _rangeCorr[i] = _params.dbz_calib_correction + range_km * _atmosAtten;
    } else {
      _rangeCorr[i] = _params.dbz_calib_correction + range_km * _atmosAtten +
        20.0 * log10(range_km);
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

double MomentsMgr::_velDiff2Angle(double vel_0, double vel_1, double nyquist) const

{
  
  double ang1 = (vel_0 / nyquist) * 180.0;
  double ang2 = (vel_1 / nyquist) * 180.0;
  double adiff = (ang1 - ang2) - _params.system_phidp;
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

////////////////////////////////////////////
// compute phase difference in complex space

Complex_t MomentsMgr::_complexDiff(const Complex_t &c1,
                                   const Complex_t &c2) const
  
{
  Complex_t diff;
  diff.re = (c1.re * c2.re + c1.im * c2.im);
  diff.im = (c1.im * c2.re - c1.re * c2.im);
  return diff;
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

//////////////////////////////////////
// compute phase difference in radians

double MomentsMgr::_argDiff(const Complex_t &c1,
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

double MomentsMgr::_arg(const Complex_t &cc) const
  
{
  double arg = 0.0;
  if (cc.im != 0.0 && cc.re != 0.0) {
    arg = atan2(cc.im, cc.re);
  }
  return arg;
  
}

//////////////////////////////////////
// compute magnitude

double MomentsMgr::_mag(const Complex_t &cc) const
  
{
  return sqrt(cc.re * cc.re + cc.im * cc.im);
}

//////////////////////////////////////
// compute power

double MomentsMgr::_power(const Complex_t &cc) const
  
{
  return cc.re * cc.re + cc.im * cc.im;
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

  // compute the data stride from one gate to the next

  int stride = ((char *) &(fields[1].dbz) - (char *) &(fields[0].dbz)) / sizeof(double);

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
  pac_get_phidp_sd(nGates, &fields[0].phidp, stride, phidpSd, 1, 10);

  // unpack phidp

  int proc_start_gate = KDP_PROC_START_OFFSET;
  TaArray<double> phidpU_;
  double *phidpU = phidpU_.alloc(nGates);
  double sysPhase = NAN;
  pac_unfold_phidp(nGates,
                   &fields[0].phidp, stride,
                   &fields[0].rhohv, stride,
                   phidpSd, 1,
                   &fields[0].snr, stride,
                   rangeKm, 1,
                   PAC_UNFOLD_AUTO, proc_start_gate,
                   sysPhase, 90.0,
                   phidpU, 1);

  TaArray<char> dataMask_;
  char *dataMask = dataMask_.alloc(nGates);

  pac_get_datamask(nGates,
                   &fields[0].dbz, stride,
                   phidpU, 1,
                   &fields[0].rhohv, stride,
                   &fields[0].snr, stride,
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
                 &fields[0].dbz, stride,
                 rangeKm, 1,
                 kdp, 1);
    
    // put KDP into fields objects

    for (int ii = 0; ii < nGates; ii++) {
      fields[ii].kdp = kdp[ii];
    }

  }

#ifdef NOTNOW  
   int nGatesForSlope = 25;
   int nGatesHalf = nGatesForSlope / 2;
   
   if (nGatesForSlope > nGates) {
     return;
   }
   
   for (int ii = nGatesHalf; ii < nGates - nGatesHalf; ii++) {
     
     if (fields[ii].phidp != _missingDbl) {
       double slope =
         _computePhidpSlope(ii, nGatesForSlope, nGatesHalf, fields);
       if (slope != _missingDbl) {
         fields[ii].kdp = slope / 2.0; // deg/km
       }
     }
     
   }

#endif

}

//////////////////////////////////////////////////
// compute PhiDp slope
//
// returns _missingDbl if not enough data

double MomentsMgr::_computePhidpSlope(int index,
                                      int nGatesForSlope,
                                      int nGatesHalf,
                                      const Fields *fields) const

{
  
  TaArray<double> xx_;
  double *xx = xx_.alloc(nGatesForSlope);
  TaArray<double> yy_;
  double *yy = yy_.alloc(nGatesForSlope);
  TaArray<double> wt_;
  double *wt = wt_.alloc(nGatesForSlope);

  int count = 0;
  
  double pdpThisGate = fields[index].phidp;
  double range = 0.0;
  
  for (int ii = index - nGatesHalf; ii <= index + nGatesHalf;
       ii++, range += _gateSpacing) {
    double snrDb = _missingDbl;
    double snr = fields[ii].snr;
    double pdp = fields[ii].phidp;
    if (snr != _missingDbl) {
      snrDb = pow(10.0, snr);
    }
    if (pdp != _missingDbl && snrDb != _missingDbl && snrDb > 5) {
      double diff = pdp - pdpThisGate;
      if (diff > 180.0) {
	pdp -= 360.0;
      } else if (diff < -180) {
	pdp += 360.0;
      }
      xx[count] = range;
      yy[count] = pdp;
      wt[count] = pow(10.0, snr);
      count++;
    }
  }

  if (count < nGatesHalf) {
    return _missingDbl;
  }

  // apply median filter to phidp

  TaArray<double> yyMed_;
  double *yyMed = yyMed_.alloc(count);
  yyMed[0] = yy[0];
  yyMed[count - 1] = yy[count - 1];
  for (int ii = 1; ii < count - 1; ii++) {
    double yy0 = yy[ii - 1];
    double yy1 = yy[ii];
    double yy2 = yy[ii + 1];
    if (yy0 > yy1) {
      if (yy1 > yy2) {
        yyMed[ii] = yy1;
      } else {
        if (yy0 > yy2) {
          yyMed[ii] = yy2;
        } else {
          yyMed[ii] = yy0;
        }
      }
    } else {
      if (yy0 > yy2) {
        yyMed[ii] = yy0;
      } else {
        if (yy1 > yy2) {
          yyMed[ii] = yy2;
        } else {
          yyMed[ii] = yy1;
        }
      }
    }
  }

  ///////////////////////////////////////
  // first try using principal components

  // alloc coords

  double **coords =
    (double **) umalloc2 (count, 2, sizeof(double));

  // load up coords

  for (int ii = 0; ii < count; ii++) {
    coords[ii][0] = xx[ii];
    coords[ii][1] = yyMed[ii];
  }

  // obtain the principal component transformation for the coord data
  // The technique is applicable here because the first principal
  // component will lie along the axis of maximum variance, which
  // is equivalent to fitting a line through the data points,
  // minimizing the sum of the sguared perpendicular distances
  // from the data to the line.
  
  double means[3];
  double eigenvalues[3];
  double **eigenvectors = (double **) umalloc2(3, 3, sizeof(double));
  double slope = _missingDbl;

  if (upct(2, count, coords,
           means, eigenvectors, eigenvalues) == 0) {
    double uu = eigenvectors[0][0];
    double vv = eigenvectors[1][0];
    slope = vv / uu;
  }

  ufree2((void **) coords);
  ufree2((void **) eigenvectors);

  if (slope != _missingDbl) {
    if (slope < -50 || slope > 50) {
      return _missingDbl;
    } else {
      return slope;
    }
  }

  //////////////////////////////////////////////////////////////
  // principal components did not work, try regression instead

  // sum up terms

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

  // compute y-on-x slope

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

/////////////////////////////////////////////////
// compute refractivity variables

void MomentsMgr::_computeRefractVars(const Complex_t *IQ,
                                     int nSamples,
                                     double &meani,
                                     double &meanq,
                                     double &aiq,
                                     double &niq,
                                     double &cpa,
                                     double dbz)
  
{

  double factor = DEG_PER_RAD;
  if (_params.change_aiq_sign) {
    factor *= -1.0;
  }

  double sumMag = 0.0;
  double sumI = 0.0, sumQ = 0.0;
  const Complex_t *iq = IQ;
  for (int i = 0; i < nSamples; i++, iq++) {
    double ii = iq->re;
    double qq = iq->im;
    sumI += ii;
    sumQ += qq;
    sumMag += sqrt(ii * ii + qq * qq);
#ifdef PHASOR_PRINT
    if (dbz > 60) {
      cout << sumI << " " << sumQ << endl;
    }
#endif
  }

  // compute mean I and Q
  
  double avI = sumI / nSamples;
  double avQ = sumQ / nSamples;
  double avMag = sumMag / nSamples;
  
  meani = 10.0 * log10(avI);
  meanq = 10.0 * log10(avQ);
    
  // compute aiq and niq
    
  aiq = factor * atan2(avQ, avI);
  double phasorLen = sqrt(avI * avI + avQ * avQ);
  niq = 10.0 * log10(phasorLen);

  // clutter phase alignment - cpa
  // cpa is niq normalized by the mag
  
  cpa = phasorLen / avMag;

}
    
/////////////////////////////////////////////////
// compute refractivity variables

void MomentsMgr::_computeRefract(int nGatesPulse,
                                 const vector<GateSpectra *> gateSpectra,
                                 Fields *fields)
  
{
  
  Fields *flds = fields;
  for (int igate = 0; igate < nGatesPulse; igate++, flds++) {
    
    // compute variables
    
    const Complex_t *iqWindowed = gateSpectra[igate]->getIqWindowed();
    double meani, meanq, aiq, niq, cpa;
    _computeRefractVars(iqWindowed, _nSamples,
                        meani, meanq, aiq, niq, cpa, flds->dbz);
    
    flds->meani = meani;
    flds->meanq = meanq;
    flds->aiq = aiq;
    flds->niq = niq;
    flds->cpa = cpa;

  } // igate
  
  // apply median filter to cpa
  
  _applyMedianFilterToCPA(nGatesPulse, fields);
  
}

/////////////////////////////////////////////////
// compute refractivity variables

void MomentsMgr::_computeRefractDualPol(int nGatesPulse,
                                        const vector<GateSpectra *>
                                        gateSpectra,
                                        Fields *fields)

{
  
  Fields *flds = fields;
  for (int igate = 0; igate < nGatesPulse; igate++, flds++) {
    
    // compute variables for H and V
    
    const Complex_t *iqh = gateSpectra[igate]->getIqHWindowed();
    double meaniH, meanqH, aiqH, niqH, cpaH;
    _computeRefractVars(iqh, _nSamplesHalf,
                        meaniH, meanqH, aiqH, niqH, cpaH,
                        flds->dbz);
    
    const Complex_t *iqv = gateSpectra[igate]->getIqVWindowed();
    double meaniV, meanqV, aiqV, niqV, cpaV;
    _computeRefractVars(iqv, _nSamplesHalf / 2,
                        meaniV, meanqV, aiqV, niqV, cpaV,
                        flds->dbz);
    
    flds->meani = (meaniH + meaniV) / 2.0;
    flds->meanq = (meanqH + meanqV) / 2.0;
    flds->aiq = (aiqH + aiqV) / 2.0;
    flds->niq = (niqH + niqV) / 2.0;
    flds->cpa = (cpaH + cpaV) / 2.0;
    
  } // igate
  
  // apply median filter to cpa
  
  _applyMedianFilterToCPA(nGatesPulse, fields);

}

///////////////////////////////////////
// compute sdzdr on time series pulses

void MomentsMgr::_computePulseSdZdr(int nGatesPulse,
                                    Complex_t **IQH,
                                    Complex_t **IQV,
                                    Fields *fields)

{
  
  for (int igate = 0; igate < nGatesPulse; igate++) {

    const Complex_t *iqh = IQH[igate];
    const Complex_t *iqv = IQV[igate];

    double sum = 0.0;
    double sum2 = 0.0;
    double nn = 0.0;

    for (int ii = 0; ii < _nSamplesHalf; ii++, iqh++, iqv++) {
      
      double ph = (iqh->re * iqh->re + iqh->im * iqh->im);
      double pv = (iqv->re * iqv->re + iqv->im * iqv->im);

      double snrh = 0;
      if (ph <= _noiseFixedChan0) {
        ph = _noiseFixedChan0 + 1.0e-20;
      }
      snrh = 10.0 * log10((ph - _noiseFixedChan0) / _noiseFixedChan0);

      double snrv = 0;
      if (pv <= _noiseFixedChan0) {
        pv = _noiseFixedChan0 + 1.0e-20;
      }
      snrv = 10.0 * log10((pv - _noiseFixedChan0) / _noiseFixedChan0);

      double zdr = snrh - snrv;
      sum += zdr;
      sum2 += zdr * zdr;
      nn++;

    } // ii

    if (nn > 2) {
      // double mean = sum / nn;
      double var = (sum2 - (sum * sum) / nn) / (nn - 1.0);
      double sdev = 0.0;
      if (var >= 0.0) {
        sdev = sqrt(var);
        fields[igate].test = sdev;
      }
    }

  } // igate

}

////////////////////////////////////////
// compute sdphidp on time series pulses

void MomentsMgr::_computePulseSdPhidp(int nGatesPulse,
                                      Complex_t **IQH,
                                      Complex_t **IQV,
                                      Fields *fields)

{
  
  for (int igate = 0; igate < nGatesPulse; igate++) {

    const Complex_t *iqh = IQH[igate];
    const Complex_t *iqv = IQV[igate];

    double sum = 0.0;
    double sum2 = 0.0;
    double nn = 0.0;

    for (int ii = 0; ii < _nSamplesHalf; ii++, iqh++, iqv++) {
      
      Complex_t conjProd = _conjugateProduct(*iqh, *iqv);
      double phaseDiff = _arg(conjProd) * RAD_TO_DEG;
      sum += phaseDiff;
      sum2 += phaseDiff * phaseDiff;
      nn++;
      
    } // ii 

    if (nn > 2) {
      // double mean = sum / nn;
      double var = (sum2 - (sum * sum) / nn) / (nn - 1.0);
      double sdev = 0.0;
      if (var >= 0.0) {
        sdev = sqrt(var);
        fields[igate].test = sdev;
      }
    } // if (nn > 2)

  } // igate

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

////////////////////////////////////////////
// compute folding which will occur for
// alternating velocity computed from every second
// pulse. The nyquist is halved because of the
// decreased prt

void MomentsMgr::_computeFoldingAlt(const double *snr,
                                    const double *vel,
                                    int *fold,
                                    int nGatesPulse,
                                    double nyquist) const

{
  
  // determine folding in alt velocity
  
  double halfNyq = nyquist / 2.0;
  fold[0] = 0;
  for (int igate = 1; igate < nGatesPulse - 1; igate++) {
    
    if (snr[igate] < 3) {

      fold[igate] = 0;

    } else {

      double vvm1 = vel[igate - 1];
      double vv0 = vel[igate];
      double vvp1 = vel[igate + 1];
      
      if (fold[igate - 1] == 0) {
        if (vvm1 > halfNyq && vv0 > halfNyq && vvp1 > halfNyq) {
          fold[igate] = 1;
        } else if (vvm1 < -halfNyq && vv0 < -halfNyq && vvp1 < -halfNyq) {
          fold[igate] = -1;
        } else {
          fold[igate] = 0;
        }
      } else if (fold[igate - 1] == 1) {
        if (vv0 > halfNyq || (vv0 > -1 && vv0 < 1)) {
          fold[igate] = 1;
        } else {
          fold[igate] = 0;
        }
      } else if (fold[igate - 1] == -1) {
        if (vv0 < -halfNyq || (vv0 > -1 && vv0 < 1)) {
          fold[igate] = -1;
        } else {
          fold[igate] = 0;
        }
      }

    } // if (snr[igate] < 6)
    
  } // igate

  // copy into the last gate

  fold[nGatesPulse - 1] = fold[nGatesPulse - 2];
  
}

////////////////////////////////////////
// apply a median filter to CPA

void MomentsMgr::_applyMedianFilterToCPA(int nGatesPulse,
                                         Fields *fields)

{
  
  if (_params.cpa_median_filter_len < 3) {
    return;
  }

  double *cpa = new double[nGatesPulse];
  for (int ii = 0; ii < nGatesPulse; ii++) {
    cpa[ii] = fields[ii].cpa;
  }
  _applyMedianFilter(cpa, nGatesPulse,
                     _params.cpa_median_filter_len);
  for (int ii = 0; ii < nGatesPulse; ii++) {
    fields[ii].cpa = cpa[ii];
  }
  delete[] cpa;

}
  
///////////////////////////////////
// apply a median filter to a field

void MomentsMgr::_applyMedianFilter(double *field,
                                    int fieldLen,
                                    int filterLen)

{
  
  // make sure filter len is odd

  int halfLen = filterLen / 2;
  int len = halfLen * 2 + 1;
  if (len < 3) {
    return;
  }

  double *buf = new double[len];
  double *copy = new double[fieldLen];
  memcpy(copy, field, fieldLen * sizeof(double));
  
  for (int ii = halfLen; ii < fieldLen - halfLen; ii++) {
    memcpy(buf, copy + ii - halfLen, len * sizeof(double));
    qsort(buf, len, sizeof(double), _doubleCompare);
    field[ii] = buf[halfLen];
  }

  delete[] buf;
  delete[] copy;

}

/////////////////////////////////////////////////////
// define function to be used for sorting

int MomentsMgr::_doubleCompare(const void *i, const void *j)
{
  double *f1 = (double *) i;
  double *f2 = (double *) j;
  if (*f1 < *f2) {
    return -1;
  } else if (*f1 > *f2) {
    return 1;
  } else {
    return 0;
  }
}

  
  
