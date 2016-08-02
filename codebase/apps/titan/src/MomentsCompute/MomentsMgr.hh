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
/////////////////////////////////////////////////////////////
// MomentsMgr.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2005
//
///////////////////////////////////////////////////////////////
//
// MomentsMgr manages the use of the Moments objects, handling 
// the specific parameters for each case.
//
///////////////////////////////////////////////////////////////

#ifndef MomentsMgr_hh
#define MomentsMgr_hh

#include <string>
#include <vector>
#include <cstdio>
#include "Params.hh"
#include "Pulse.hh"
#include "Moments.hh"
#include "Fields.hh"
#include "Fft.hh"
#include "ClutFilter.hh"
using namespace std;

////////////////////////
// This class

class MomentsMgr {

public:

	// constructor

	MomentsMgr (const Params &params,
		Params::moments_params_t &moments_params);

	// destructor

	~MomentsMgr();

	// compute moments - single pol
	// IQC data is for a single-channel copolar
	//   IQC[nGates][nPulses]

	void computeSingle(double beamTime,
		double el,
		double az,
		double prt,
		int nGates,
		Complex_t **IQC,
		Fields *fields);

        void computeDualCp2Sband(double beamTime,
                                 double el,
                                 double az,
                                 double prt,
                                 int nGates,
                                 Complex_t **IQHC,
                                 Complex_t **IQVC,
                                 Fields *fields);

	void computeDualCp2Xband(double beamTime,
		double el,
		double az,
		double prt,
		int nGates,
		Complex_t **IQHC,
		Complex_t **IQVX,
		Fields *fields);

	// get methods

	int getNSamples() const { return _nSamples; }
	Params::moments_mode_t getMode() const { return _momentsParams.mode; }
	double getStartRange() const { return _startRange; }
	double getGateSpacing() const { return _gateSpacing; }  
	const double *getRangeCorr() const { return _rangeCorr; }

	Moments::window_t getFftWindow() const { return _fftWindow; }

	const Moments &getMoments() const { return _moments; }

protected:

private:

	static const double _missingDbl;

	const Params &_params;
	const Params::moments_params_t _momentsParams;

	// fft window

	Moments::window_t _fftWindow;
 
	// number of pulse samples

	int _nSamples;
	int _nSamplesHalf;

        // FFT support

        Fft *_fft;
        Fft *_fftHalf;

	// Moments objects

	Moments _moments;
	Moments _momentsHalf; // for alternating mode dual pol

        // clutter filter

        ClutFilter _clutFilter;

	// range correction

	int _nGates;
	double _startRange;
	double _gateSpacing;
	double *_rangeCorr;

	// calibration

	double _noiseFixedHc;
	double _noiseFixedHx;
	double _noiseFixedVc;
	double _noiseFixedVx;

	double _dbz0Hc;
	double _dbz0Hx;
	double _dbz0Vc;
	double _dbz0Vx;

        // PHIDP corrections

       static const double _phidpPhaseLimit;

	// functions

	void _checkRangeCorrection(int nGates);

	double _computeMeanVelocity(double vel1, double vel2, double nyquist) const;

	double _velDiff2Angle(double vel1, double vel2, double nyquist) const;

	Complex_t _complexSum(Complex_t c1, Complex_t c2) const;

	Complex_t _complexMean(Complex_t c1, Complex_t c2) const;

	Complex_t _complexProduct(Complex_t c1, Complex_t c2) const;

	Complex_t _conjugateProduct(Complex_t c1, Complex_t c2) const;

	Complex_t _meanComplexProduct(const Complex_t *c1,
		const Complex_t *c2,
		int len) const;

	Complex_t _meanConjugateProduct(const Complex_t *c1,
		const Complex_t *c2,
		int len) const;

	double _velFromComplex(Complex_t cc, double nyquist) const;

	double _velFromArg(double arg, double nyquist) const;

	Complex_t _computeComplexDiff(const Complex_t &c1,
		const Complex_t &c2) const;

	double _computeArgDiff(const Complex_t &c1,
		const Complex_t &c2) const;

	double _computeArg(const Complex_t &cc) const;

	double _computeMag(const Complex_t &cc) const;

	double _computePower(const Complex_t &cc) const;

	double _meanPower(const Complex_t *c1, int len) const;

	// Kdp calculations disabled until a better algorithm is 
	// implemented.
	//  void _computeKdp(int nGates, Fields *fields) const;

	//  double _computePhidpSlope(int index,
	//                            int nGatesForSlope,
	//                            int nGatesHalf,
	//                            const Fields *fields) const;

	double _computeInterest(double xx, double x0, double x1) const;

	void _computeFoldingAlt(const double *snr,
		const double *vel,
		int *fold,
		int nGatesPulse,
		double nyquist) const;

	static int _doubleCompare(const void *i, const void *j);

        void _computeKdp(int nGates, Fields *fields);
  
        void _applyClutterFilter(int nSamples,
                                 Fft &fft,
                                 Moments &moments,
                                 double nyquist,
                                 const Complex_t *iq,
                                 Complex_t *iqFiltered);

};

#endif

