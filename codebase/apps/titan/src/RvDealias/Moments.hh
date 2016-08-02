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
    WINDOW_MOD_HANNING
  } sz_window_t;

  // constructor
  
  Moments ();
  
  // destructor
  
  ~Moments();

  // run 

  int Run();

  // get number of samples

  int getNSamples() { return _nSamples; }
  
  // set methods

  // debugging

  void setDebug(bool status = true);
  void setVerbose(bool status = true);

  // selected prints and spectra files

  void setSelectedPrint(bool status = true);
  void setWriteSpectra(bool status = true,
		       const string &dir = "");
  void setEl(double el);
  void setAz(double az);
  void setRange(double range);

  // wavelength and noise levels
  
  void setWavelength(double wavelength);
  void setNoiseValueDbm(double dbm);

  // SZ decoding

  void setSzNegatePhaseCodes(bool status = true);
  void setSzWindow(sz_window_t window);

  // censoring thresholds

  void setSignalToNoiseRatioThreshold(double db);
  void setSzStrongToWeakPowerRatioThreshold(double db);
  void setSzOutOfTripPowerRatioThreshold(double db);
  void setSzOutOfTripPowerNReplicas(int n);

  // apply hanning window
  
  void applyHanningWindow(const Complex_t *in, Complex_t *out);
  
  // apply modified hanning window

  void applyModHanningWindow(const Complex_t *in, Complex_t *out);

  // compute total power

  double computeTotalPower(const Complex_t *IQ);
    
  // check if we should threshold based on signal-to-noise of total power
  // Side-effect: passes back total power

  bool checkSnThreshold(const Complex_t *IQ, double &totalPower);
    
  // compute time-domain moments using ABP pulse-pair method
  
  void computeByAbp(const Complex_t *IQ,
		    double prtSecs,
		    double &power, double &vel,
		    double &width);
  
  // compute moments using pulse-pair
  
  void computeByPp(const Complex_t *IQ,
		   double prtSecs,
		   double &power, double &vel,
		   double &width, int &flags);
  
  // compute moments using spectra

  void computeByFft(const Complex_t *IQ,
		    double prtSecs,
		    double &power, double &vel,
		    double &width, int &flags);
  
  // compute moments using SZ 8/64 algorithm using PP
  
  void computeBySzPp(const Complex_t *IQ,
		     const Complex_t *delta12,
		     double prtSecs,
		     double &power1, double &vel1,
		     double &width1, int &flags1,
		     double &power2, double &vel2,
		     double &width2, int &flags2);
  
  // compute moments using SZ 8/64 algorithm using FFT
  // beamCode should run from [-4 to 63].
  
  void computeBySzFft(const Complex_t *IQ,
		      const Complex_t *delta12,
		      double prtSecs,
		      double &power1, double &vel1,
		      double &width1, int &flags1,
		      double &power2, double &vel2,
		      double &width2, int &flags2);

  // cohere to a given trip

  void cohere2Trip(const Complex_t *IQ,
		   const Complex_t *beamCode,
		   int trip_num,
		   Complex_t *iqTrip);
  
protected:
  
private:

  // debugging

  bool _isDebug;
  bool _isVerbose;

  // selected debug printing, performed for limited ranges of
  // elevations, azimuths or ranges

  bool _selectedPrint;
  bool _writeSpectraFiles;
  double _el, _az, _range;
  string _spectraDir;

  // levels

  double _wavelengthMeters;
  double _noiseValueDbm;
  double _noiseValueMwatts;

  // SZ

  bool _szNegatePhaseCodes;
  sz_window_t _szWindow;

  // censoring thresholds

  double _signalToNoiseRatioThreshold;
  double _szStrongToWeakPowerRatioThreshold;
  double _szOutOfTripPowerRatioThreshold;
  int _szOutOfTripPowerNReplicas;
  
  static const int _phaseCodeN = 8;
  static const int _nSamples = 64;
  Complex_t _modCode12[_nSamples];
  Complex_t _modCode21[_nSamples];
  static const double _missingDbl = -9999.0;
  static const double _smallValue = 1.0e-9;

  // censoring flags

  static const int _censorOnTotalPower = 1;
  static const int _censorOnPowerRatio = 2;
  static const int _censorOnReplicas = 4;

  // 3/4 notch

  static const int _szNotchWidth75 = 48;
  static const int _szPowerWidth75 = 16;
  static const double _szFracPower75 = 0.25;

  // 1/2 notch
  
  static const int _szNotchWidth50 = 32;
  static const int _szPowerWidth50 = 32;
  static const double _szFracPower50 = 0.5;

  // windows

  double _hanning[_nSamples];
  double _modHanning[_nSamples];

  // deconvolution matrices

  double _deconvMatrix75[_nSamples * _nSamples];
  double _deconvMatrix50[_nSamples * _nSamples];

  Fft *_fft;

  // functions

  void _initPhaseCodes();
  void _initDeconMatrix(int notchWidth, int powerWidth,
			double fracPower, double *deconvMatrix);
  
  void _initHanning(double *window);
  void _initModHanning(double *window);

  void _applyWindow(const double *window,
		    const Complex_t *in, Complex_t *out);

  double _computeMeanPower(const Complex_t *IQ);
  
  double _computeR1(const Complex_t *IQ);
  
  void _cohereTrip1_to_Trip2(const Complex_t *trip1,
			     const Complex_t *delta12,
			     Complex_t *trip2);
  
  void _cohereTrip2_to_Trip1(const Complex_t *trip2,
			     const Complex_t *delta12,
			     Complex_t *trip1);

  void _addCode(const Complex_t *in, const Complex_t *code,
		Complex_t *sum);
  
  void _addCode(const Complex_t *in, const Complex_t *code, int trip,
		Complex_t *sum);
  
  void _subCode(const Complex_t *in, const Complex_t *code,
		Complex_t *diff);

  void _subCode(const Complex_t *in, const Complex_t *code, int trip,
		Complex_t *diff);
  
  void _conjugate(const Complex_t *in, Complex_t *conj);
  
  void _computeMag(const Complex_t *in, double *mag);

  void _normalizeMag(const Complex_t *in, double *norm_mag);

  void _computeSpectralNoise(const double *powerCentered,
			     double &noiseMean,
			     double &noiseSdev);
  
  int _computeNotchStart(int notchWidth,
			 double vel,
			 double prtSecs);

  void _applyNotch(int startIndex,
		   Complex_t *in,
		   int notchWidth,
		   int powerWidth,
		   double fracPower,
		   Complex_t *notched);
  
  void _applyNotch75(int startIndex,
		     Complex_t *in,
		     Complex_t *notched);

  void _applyNotch50(int startIndex,
		     Complex_t *in,
		     Complex_t *notched);

  void _velWidthFromTd(const Complex_t *IQ,
		       double prtSecs,
		       double &vel, double &width);
  
  void _velWidthFromFft(const double *magnitude,
			double prtSecs,
			double &vel, double &width);
  
  bool _hasReplicas(double *magnitude);
  
  void _invertMatrix(double *data, int nn);
  
  void _printComplex(ostream &out,
		     const string &heading,
		     const Complex_t *comp);
  
  void _printVector(ostream &out,
		    const string &heading,
		    const Complex_t *comp);

  void _writeComplex2File(const string &heading,
			  const Complex_t *comp);
  
  void _writeMag2File(const string &heading,
		      const double *mag);
  
  static int _compareDoubles(const void *v1, const void *v2);

};

#endif

