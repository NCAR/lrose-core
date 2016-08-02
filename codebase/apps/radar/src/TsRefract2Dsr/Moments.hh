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
// March 2003
//
///////////////////////////////////////////////////////////////

#ifndef Moments_hh
#define Moments_hh

#include <string>
#include <vector>
#include "Fft.hh"

using namespace std;

////////////////////////
// This class

class Moments {
  
public:

  typedef enum {
    WINDOW_NONE,
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
  
  // censoring thresholds

  void setSignalToNoiseRatioThreshold(double db);

  // apply hanning window
  
  void applyHanningWindow(const Complex_t *in, Complex_t *out) const;
  
  // apply modified hanning window

  void applyBlackmanWindow(const Complex_t *in, Complex_t *out) const;

  // compute power from IQ

  double computePower(const Complex_t *IQ) const;
    
  // compute power from magnitudes

  double computePower(const double *mag) const;
    
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
		    int &flags) const;
  
protected:
  
private:

  bool _debugPrint;
  int _nSamples;

  // levels

  double _wavelengthMeters;
  double _noiseValueDbm;
  double _noiseValueMwatts;
  
  // censoring thresholds

  double _signalToNoiseRatioThreshold;
  
  static const double _missingDbl;
  static const double _smallValue;

  // censoring flags

  static const int _censorOnTotalPower = 1;

  // windows

  double *_hanning;
  double *_blackman;

  // FFT support

  Fft *_fft;

  // functions

  void _initHanning(double *window);
  void _initBlackman(double *window);

  void _applyWindow(const double *window,
		    const Complex_t *in, Complex_t *out) const;
  
  void _loadMag(const Complex_t *in, double *mag) const;
  void _loadPower(const Complex_t *in, double *mag) const;

  void _computeSpectralNoise(const double *powerCentered,
			     double &noiseMean,
			     double &noiseSdev) const;
  
  void _velWidthFromTd(const Complex_t *IQ,
		       double prtSecs,
		       double &vel, double &width) const;
  
  void _velWidthFromFft(const double *magnitude,
			double prtSecs,
			double &vel, double &width,
			double &measuredNoise) const;
  
};

#endif

