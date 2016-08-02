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
// PhaseCoding.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2014
//
///////////////////////////////////////////////////////////////

#ifndef PhaseCoding_hh
#define PhaseCoding_hh

#include <string>
#include <vector>
#include <radar/RadarFft.hh>
#include <radar/GateData.hh>
#include <radar/IwrfTsPulse.hh>

using namespace std;

////////////////////////
// This class

class PhaseCoding {
  
public:

  // constructor
  
  PhaseCoding(bool debug);
  
  // destructor
  
  ~PhaseCoding();

  // number of samples

  void setNSamples(int n) { _nSamples = n; }
  int getNSamples() const { return _nSamples; }
  
  // wavelength and noise levels
  
  void setWavelength(double wavelength);
  void setNoiseValueDbm(double dbm);

  // censoring thresholds

  void setSnrThreshold(double db);
  void setNcpThreshold(double val);

  // finding replicas

  void setOutOfTripPowerRatioThreshold(double db);
  void setOutOfTripPowerNReplicas(int n);

  // compute power from IQ

  double computePower(const RadarComplex_t *IQ) const;
    
  // compute power from magnitudes

  double computePower(const double *mag) const;
    
  // adjust the power in IQ data to a given value
  
  void adjustPower(RadarComplex_t *IQ, double adjustedPower) const;

  // check if we should threshold based on SNR of total power
  // Side-effect: passes back total power

  bool checkSnrThreshold(const RadarComplex_t *IQ, double &totalPower) const;
    
  // cohere measured iq to a given trip
  
  void cohereToTrip1(const RadarComplex_t *iq,
                     vector<IwrfTsPulse::burst_phase_t> &codes,
                     RadarComplex_t *trip1) const;

  void cohereToTrip2(const RadarComplex_t *iq,
                     vector<IwrfTsPulse::burst_phase_t> &codes,
                     RadarComplex_t *trip2) const;

  void cohereToTrip3(const RadarComplex_t *iq,
                     vector<IwrfTsPulse::burst_phase_t> &codes,
                     RadarComplex_t *trip3) const;

  void cohereToTrip4(const RadarComplex_t *iq,
                     vector<IwrfTsPulse::burst_phase_t> &codes,
                     RadarComplex_t *trip4) const;

  // cohere from given trip back to measured phase

  void revertFromTrip1(const RadarComplex_t *trip1,
                       vector<IwrfTsPulse::burst_phase_t> &codes,
                       RadarComplex_t *measured) const;
  
  void revertFromTrip2(const RadarComplex_t *trip2,
                       vector<IwrfTsPulse::burst_phase_t> &codes,
                       RadarComplex_t *measured) const;
  
  void revertFromTrip3(const RadarComplex_t *trip3,
                       vector<IwrfTsPulse::burst_phase_t> &codes,
                       RadarComplex_t *measured) const;
  
  void revertFromTrip4(const RadarComplex_t *trip4,
                       vector<IwrfTsPulse::burst_phase_t> &codes,
                       RadarComplex_t *measured) const;

  // apply notch to time series
  
  void applyNotch(const RadarFft &fft,
                  RadarComplex_t *iq,
                  double notchWidth);

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

  // censoring thresholds

  double _snrThreshold;
  double _ncpThreshold;

  // finding replicas

  double _outOfTripPowerRatioThreshold;
  int _outOfTripPowerNReplicas;

  int _nSamples;
  static const double _missingDbl;
  static const double _smallValue;
  
  // censoring flags
  
  static const int _censorOnSnr = 1;
  static const int _censorOnPowerRatio = 2;
  static const int _censorOnReplicas = 4;

  // 3/4 notch

  static const int _notchWidth75 = 48;
  static const int _powerWidth75 = 16;
  static const double _fracPower75;

  // 1/2 notch
  
  static const int _notchWidth50 = 32;
  static const int _powerWidth50 = 32;
  static const double _fracPower50;

  // functions

  double _computeR1(const RadarComplex_t *IQ) const;
  
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

