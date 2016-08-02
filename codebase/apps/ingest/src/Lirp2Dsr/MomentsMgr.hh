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
#include "GateSpectra.hh"
using namespace std;

////////////////////////
// This class

class MomentsMgr {
  
public:
  
  // constructor
  
  MomentsMgr (const string &prog_name,
	      const Params &params,
	      Params::moments_params_t &moments_params,
	      int n_samples,
	      int max_gates);
  
  // destructor
  
  ~MomentsMgr();
  
  // compute moments, either normal or dual pol
  // Set combinedSpectraFile to NULL to prevent printing.
  
  void computeMoments(time_t beamTime,
		      double el, double az, double prt,
		      int nGatesPulse,
		      Complex_t **IQ,
		      int &combinedPrintCount,
		      FILE *combinedSpectraFile,
		      double *powerDbm, double *snr, double *dbz,
		      double *noiseDbm, double *snrn, double *dbzn,
		      double *vel, double *width, int *flags,
                      double *clutDbzNarrow,
                      double *clutRatioNarrow,
                      double *clutRatioWide,
                      double *clutWxPeakRatio,
                      double *clutWxPeakSep,
		      vector<GateSpectra *> gateSpectra) const;
  
  void computeMomentsSz(time_t beamTime,
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
			vector<GateSpectra *> gateSpectra) const;
  
  void computeMomentsDualPol(time_t beamTime,
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
			     vector<GateSpectra *> gateSpectra) const;
  
  void filterClutter(double prt,
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
		     double *widthf) const;
  
  void filterClutterSz(double prt,
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
		       double *widthf) const;
  
  void filterClutterDualPol(double prt,
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
			    double *widthf) const;

  // get methods
  
  double getLowerPrf() const { return _lowerPrf; }
  double getUpperPrf() const { return _upperPrf; }
  double getPulseWidth() const { return _pulseWidth; }

  double getStartRange() const { return _startRange; }
  double getGateSpacing() const { return _gateSpacing; }
  int getMaxGates() const { return _maxGates; }

  bool getUseFft() const { return _useFft; }
  bool getApplySz() const { return _applySz; }
  bool getUseCForSz() const { return _useCForSz; }
  bool getDualPol() const { return _dualPol; }

  Moments::window_t getFftWindow() const { return _fftWindow; }

  int getNSamples() const { return _nSamples; }

  const double *getRangeCorr() const { return _rangeCorr; }

  double getMinDbzAt1km() const { return _minDbzAt1km; }
  double getAtmosAtten() const { return _atmosAtten; }
  int getZvFlag() const { return _zvFlag; }

  const Moments &getMoments() const { return _moments; }

  double getMeasuredNoise() const {
    return _measuredNoise;
  }
  
  double getMeasuredNoiseDbm() const {
    return _measuredNoiseDbm;
  }
  
protected:
  
private:

  static const double _missingDbl = -9999.0;

  string _progName;
  const Params &_params;
  
  // PRF range
  
  double _lowerPrf;
  double _upperPrf;
  double _pulseWidth;
  
  // geometry
  
  double _startRange;
  double _gateSpacing;
  int _maxGates;

  // flags

  bool _useFft;
  bool _applySz;
  bool _dualPol;
  bool _useCForSz;
  Moments::window_t _fftWindow;

  // number of pulse samples

  int _nSamples;
  
  // range correction

  double *_rangeCorr;

  // calibration
  
  double _minDbzAt1km;
  double _atmosAtten;  // db/km
  double _noiseFixed;

  // measured noise

  double _noiseHistMin;
  double _noiseHistMax;
  double _noiseHistDelta;
  int _noiseHistSize;
  mutable double *_noiseHist;
  mutable double _noiseHistTotalCount;
  mutable int _noiseHistComputeCount;

  mutable double _measuredNoise;
  mutable double _measuredNoiseDbm;

  // measured dual pol phase difference

  mutable Complex_t _dualPolPhaseDiffSum;
  mutable double _dualPolPhaseDiffCount;
  mutable Complex_t _dualPolPhaseDiff;

  // Moments object

  Moments _moments;
  Moments _momentsHalf; // for dual pol computations

  // dual pol - deciding which pulse is horizontal from ZDR

  static bool _zvFlagReady;
  static int _zvFlag;
  static int _nZdr;
  static double _sumZdr;

  // functions

  void _computeRangeCorrection();

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

  void _computeKdp(int nGates, const double *phidp, const double *snr,
		   double *kdp) const;
  
  double _computeSlope(int index,
		       int nGatesForSlope,
		       int nGatesHalf,
		       const double *phidp,
		       const double *snr) const;

  double _computeInterest(double xx, double x0, double x1) const;

  void _addSpectrumToFile(FILE *specFile, int count, time_t beamTime,
			  double el, double az, int gateNum,
			  double snr, double vel, double width,
			  int nSamples, const Complex_t *iq) const;

  void _computeNoiseStats(double noise) const;

  bool _setMomentsDebugPrint(const Moments &moments, double el,
			     double az, double range) const;
  
  void _applyTimeDomainFilter(const Complex_t *iq,
			      Complex_t *filtered) const;

};

#endif

