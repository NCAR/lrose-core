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
// Moments.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2006
//
///////////////////////////////////////////////////////////////

#ifndef Moments_hh
#define Moments_hh

#include <string>
#include <vector>
#include "Fft.hh"
#include "GateSpectra.hh"
#include "ClutProb.hh"

using namespace std;

////////////////////////
// This class

class Moments {
  
public:

  typedef enum {
    WINDOW_RECT,
    WINDOW_HANNING,
    WINDOW_BLACKMAN
  } window_t;

  // constructor
  
  Moments(int n_samples = 64);
  
  // destructor
  
  ~Moments();

  // run 

  int Run();

  // get number of samples

  int getNSamples() { return _nSamples; }
  
  // set methods

  // wavelength and noise levels
  
  void setWavelength(double wavelength);
  void setNoiseValueDbm(double dbm);

  // SZ decoding

  void setSzNegatePhaseCodes(bool status = true);
  void setSzWindow(window_t window);

  // censoring thresholds

  void setSignalToNoiseRatioThreshold(double db);
  void setSzStrongToWeakPowerRatioThreshold(double db);
  void setSzOutOfTripPowerRatioThreshold(double db);
  void setSzOutOfTripPowerNReplicas(int n);
  
  // clutter corrections

  void setDbForDbRatio(double ratio);
  void setDbForDbThreshold(double threshold);

  // debugging prints and spectra files

  void setDebugPrint(bool status = true) const;
  void setDebugWriteSpectra(bool status = true,
			    const string &dir = "/tmp") const;
  void setDebugEl(double el) const;
  void setDebugAz(double az) const;
  void setDebugRange(double range) const;

  // apply rectangular window
  
  void applyRectWindow(const Complex_t *in, Complex_t *out) const;
  
  // apply hanning window
  
  void applyHanningWindow(const Complex_t *in, Complex_t *out) const;
  
  // apply modified hanning window

  void applyBlackmanWindow(const Complex_t *in, Complex_t *out) const;

  // compute power from IQ

  double computePower(const Complex_t *IQ) const;
    
  // compute power from magnitudes

  double computePower(const double *mag) const;
    
  // compute power coefficient of variation
  
  double computePowerCvar(const Complex_t *IQ) const;
  
  // check if we should threshold based on signal-to-noise of total power
  // Side-effect: passes back total power

  bool checkSnThreshold(const Complex_t *IQ, double &totalPower) const;
    
  // compute time-domain moments using ABP pulse-pair method
  
  void computeByAbp(const Complex_t *IQ,
		    double prtSecs,
		    double &power, double &vel,
		    double &width) const;
  
  // compute moments using pulse-pair
  
  void computeByPp(const Complex_t *IQ,
		   double prtSecs,
		   double &power, double &vel,
		   double &width, int &flags) const;
  
  // compute moments using fft spectra

  void computeByFft(const Complex_t *IQ,
		    window_t windowType,
		    double prtSecs,
		    double &power, double &noise,
		    double &vel, double &width,
		    int &flags,
		    ClutProb *clutProb = NULL) const;
  
  // compute clutter probability - not windowed

  void computeClutProb(const Complex_t *IQ,
                       window_t windowType,
                       double prtSecs,
                       ClutProb &clutProb) const;
  
  // compute ratio of DC power to total power
  
  double computePowerRatio(const Complex_t *IQ) const;
  
  // compute moments using fft spectra and remove clutter
  
  void computeByFftFilterClutter(const Complex_t *IQ,
				 window_t windowType,
				 double prtSecs,
				 double &power,
				 double &vel, double &width,
				 double &clutterPower,
				 double &noise) const;
  
  // compute moments using SZ 8/64 algorithm using PP
  
  void computeBySzPp(const Complex_t *IQ,
		     const Complex_t *delta12,
		     double prtSecs,
		     double &totalPower,
		     double &power1, double &vel1,
		     double &width1, int &flags1,
		     double &power2, double &vel2,
		     double &width2, int &flags2,
		     int &strongTripFlag,
		     double &powerRatio) const;
  
  // compute moments using SZ 8/64 algorithm using FFT
  // beamCode should run from [-4 to 63].
  
  void computeBySzFft(const Complex_t *IQ,
		      const Complex_t *delta12,
		      double prtSecs,
		      double &totalPower,
		      double &power1, double &vel1, 
		      double &width1, int &flags1,
		      double &power2, double &vel2,
		      double &width2, int &flags2,
		      double &leak1, double &leak2,
		      int &strongTripFlag,
		      double &powerRatio,
		      GateSpectra &gateSpec) const;

  // filter clutter SZ mode

  void filterClutterSz(const GateSpectra &gateSpec,
		       const Complex_t *delta12,
		       double prtSecs,
		       bool clutterInStrong,
		       bool clutterInWeak,
		       double &powerStrong, double &velStrong, 
		       double &widthStrong, double &clutStrong,
		       double &powerWeak, double &velWeak,
		       double &widthWeak, double &clutWeak) const;

  // clutter filter IQ data

  void iqFilterClutter(const Complex_t *IQ,
                       window_t windowType,
                       double prtSecs,
                       Complex_t *iqFiltered) const;

  // cohere to a given trip

  void cohere2Trip(const Complex_t *IQ,
		   const Complex_t *beamCode,
		   int trip_num,
		   Complex_t *iqTrip) const;

  // print spectra

  void printComplex(ostream &out,
                    const string &heading,
                    const Complex_t *comp,
                    bool reCenter = false) const;
  
  void printVector(ostream &out,
                   const string &heading,
                   const Complex_t *comp,
                   bool reCenter = false) const;

protected:
  
private:

  // levels

  double _wavelengthMeters;
  double _noiseValueDbm;
  double _noiseValueMwatts;

  // SZ
  
  window_t _szWindow;
  bool _szNegatePhaseCodes;

  // censoring thresholds

  double _signalToNoiseRatioThreshold;
  double _szStrongToWeakPowerRatioThreshold;
  double _szOutOfTripPowerRatioThreshold;
  int _szOutOfTripPowerNReplicas;
  
  static const int _phaseCodeN = 8;
  int _nSamples;
  Complex_t *_modCode12;
  Complex_t *_modCode21;
  static const double _missingDbl;
  static const double _smallValue;

  // censoring flags

  static const int _censorOnSnr = 1;
  static const int _censorOnPowerRatio = 2;
  static const int _censorOnReplicas = 4;

  // 3/4 notch

  static const int _szNotchWidth75 = 48;
  static const int _szPowerWidth75 = 16;
  static const double _szFracPower75;

  // 1/2 notch
  
  static const int _szNotchWidth50 = 32;
  static const int _szPowerWidth50 = 32;
  static const double _szFracPower50;

  // clutter filter

  static const double _maxClutterVel;
  static const double _initNotchWidth;
  static const double _maxClutterVelSz;
  static const double _initNotchWidthSz;
  double _dbForDbRatio;
  double _dbForDbThreshold;

  // windows

  double *_hanning;
  double *_blackman;

  // deconvolution matrices

  double *_deconvMatrix75;
  double *_deconvMatrix50;

  // FFT support

  Fft *_fft;

  // selected debug printing, performed for limited ranges of
  // elevations, azimuths or ranges
  
  mutable bool _debugPrint;
  mutable bool _debugWriteSpectraFiles;
  mutable double _debugEl, _debugAz, _debugRange;
  mutable string _debugSpectraDir;

  // functions

  void _initPhaseCodes();
  void _initDeconMatrix(int notchWidth, int powerWidth,
			double fracPower, double *deconvMatrix);
  
  void _initHanning(double *window);
  void _initBlackman(double *window);

  void _applyWindow(const double *window,
		    const Complex_t *in, Complex_t *out) const;

  double _computeR1(const Complex_t *IQ) const;
  
  void _cohereTrip1_to_Trip2(const Complex_t *trip1,
			     const Complex_t *delta12,
			     Complex_t *trip2) const;
  
  void _cohereTrip2_to_Trip1(const Complex_t *trip2,
			     const Complex_t *delta12,
			     Complex_t *trip1) const;

  void _addCode(const Complex_t *in, const Complex_t *code,
		Complex_t *sum) const;
  
  void _addCode(const Complex_t *in, const Complex_t *code, int trip,
		Complex_t *sum) const;
  
  void _subCode(const Complex_t *in, const Complex_t *code,
		Complex_t *diff) const;

  void _subCode(const Complex_t *in, const Complex_t *code, int trip,
		Complex_t *diff) const;
  
  void _conjugate(const Complex_t *in, Complex_t *conj) const;
  
  void _loadMag(const Complex_t *in, double *mag) const;
  void _loadPower(const Complex_t *in, double *mag) const;

  void _normalizeMag(const Complex_t *in, double *norm_mag);

  void _computeSpectralNoise(const double *powerCentered,
			     double &noiseMean,
			     double &noiseSdev) const;
  
  int _computeNotchStart(int notchWidth,
			 double vel,
			 double prtSecs) const;

  int _adjustNotchForClutter(int clutNotchStart, int clutNotchEnd,
			     int notchWidth, int notchStart) const;
  
  void _applyNotch(int startIndex,
		   const Complex_t *in,
		   int notchWidth,
		   int powerWidth,
		   double fracPower,
		   Complex_t *notched) const;
  
  void _applyNotch75(int startIndex,
		     const Complex_t *in,
		     Complex_t *notched) const;

  void _applyNotch50(int startIndex,
		     const Complex_t *in,
		     Complex_t *notched) const;

  void _velWidthFromTd(const Complex_t *IQ,
		       double prtSecs,
		       double &vel, double &width) const;
  
  void _velWidthFromFft(const double *magnitude,
			double prtSecs,
			double &vel, double &width,
			double &measuredNoise) const;
  
  bool _hasReplicas(double *magnitude, double &leakage) const;
  
  void _invertMatrix(double *data, int nn) const;
  
  void _writeComplex2File(const string &heading,
			  const Complex_t *comp,
                          bool reCenter = false) const;
  
  void _writeMag2File(const string &heading,
		      const double *mag,
                      bool reCenter = false) const;
  
  static int _compareDoubles(const void *v1, const void *v2);

};

#endif

