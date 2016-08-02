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
// Sz864.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Aug 2007
//
///////////////////////////////////////////////////////////////

#ifndef Sz864_hh
#define Sz864_hh

#include <string>
#include <vector>
#include <radar/RadarFft.hh>
#include <radar/GateData.hh>

using namespace std;

////////////////////////
// This class

class Sz864 {
  
public:

  typedef enum {
    WINDOW_RECT,
    WINDOW_VONHANN,
    WINDOW_BLACKMAN
  } window_t;

  // constructor
  
  Sz864(bool debug);
  
  // destructor
  
  ~Sz864();

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
  
  // separate the trips into iq time series

  void separateTrips(GateData &gateData,
                     const RadarComplex_t *delta12,
                     double prtSecs,
                     const RadarFft &fft);

  // compute power from IQ

  double computePower(const RadarComplex_t *IQ) const;
    
  // compute power from magnitudes

  double computePower(const double *mag) const;
    
  // adjust the power in IQ data to a given value
  
  void adjustPower(RadarComplex_t *IQ, double adjustedPower) const;

  // check if we should threshold based on signal-to-noise of total power
  // Side-effect: passes back total power

  bool checkSnThreshold(const RadarComplex_t *IQ, double &totalPower) const;
    
  // clutter filter IQ data

  void iqFilterClutter(const RadarComplex_t *IQ,
                       double prtSecs,
                       RadarComplex_t *iqFiltered) const;

  // cohere to a given trip

  void cohere2Trip(const RadarComplex_t *IQ,
		   const RadarComplex_t *beamCode,
		   int trip_num,
		   RadarComplex_t *iqTrip) const;

  // print spectra

  void printComplex(ostream &out,
                    const string &heading,
                    const RadarComplex_t *comp,
                    bool reCenter = false) const;
  
  void printVector(ostream &out,
                   const string &heading,
                   const RadarComplex_t *comp,
                   bool reCenter = false) const;

protected:
  
private:

  bool _debug;

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
  RadarComplex_t *_modCode12;
  RadarComplex_t *_modCode21;
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

  double *_vonhann;
  double *_blackman;

  // deconvolution matrices

  double *_deconvMatrix75;
  double *_deconvMatrix50;

  // functions

  void _initPhaseCodes();
  void _initDeconMatrix(int notchWidth, int powerWidth,
			double fracPower, double *deconvMatrix);
  
  void _initVonhann(double *window);
  void _initBlackman(double *window);

  void _applyWindow(const double *window,
		    const RadarComplex_t *in, RadarComplex_t *out) const;

  double _computeR1(const RadarComplex_t *IQ) const;
  
  void _cohereTrip1_to_Trip2(const RadarComplex_t *trip1,
			     const RadarComplex_t *delta12,
			     RadarComplex_t *trip2) const;
  
  void _cohereTrip2_to_Trip1(const RadarComplex_t *trip2,
			     const RadarComplex_t *delta12,
			     RadarComplex_t *trip1) const;

  void _addCode(const RadarComplex_t *in, const RadarComplex_t *code,
		RadarComplex_t *sum) const;
  
  void _addCode(const RadarComplex_t *in, const RadarComplex_t *code, int trip,
		RadarComplex_t *sum) const;
  
  void _subCode(const RadarComplex_t *in, const RadarComplex_t *code,
		RadarComplex_t *diff) const;

  void _subCode(const RadarComplex_t *in, const RadarComplex_t *code, int trip,
		RadarComplex_t *diff) const;
  
  void _conjugate(const RadarComplex_t *in, RadarComplex_t *conj) const;
  
  void _loadMag(const RadarComplex_t *in, double *mag) const;
  void _loadPower(const RadarComplex_t *in, double *mag) const;

  void _normalizeMag(const RadarComplex_t *in, double *norm_mag);

  void _computeSpectralNoise(const double *powerCentered,
			     double &noiseMean,
			     double &noiseSdev) const;
  
  int _computeNotchStart(int notchWidth,
			 double vel,
			 double prtSecs) const;

  int _adjustNotchForClutter(int clutNotchStart, int clutNotchEnd,
			     int notchWidth, int notchStart) const;
  
  void _applyNotch(int startIndex,
		   const RadarComplex_t *in,
		   int notchWidth,
		   int powerWidth,
		   double fracPower,
		   RadarComplex_t *notched) const;
  
  void _applyNotch75(int startIndex,
		     const RadarComplex_t *in,
		     RadarComplex_t *notched) const;

  void _applyNotch50(int startIndex,
		     const RadarComplex_t *in,
		     RadarComplex_t *notched) const;

  void _velWidthFromTd(const RadarComplex_t *IQ,
		       double prtSecs,
		       double &vel, double &width) const;
  
  void _velWidthFromFft(const double *magnitude,
			double prtSecs,
			double &vel, double &width,
			double &measuredNoise) const;
  
  bool _hasReplicas(double *magnitude, double &leakage) const;
  
  void _invertMatrix(double *data, int nn) const;
  
  static int _compareDoubles(const void *v1, const void *v2);

};

#endif

