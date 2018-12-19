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
// ClutFilter.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2004
//
///////////////////////////////////////////////////////////////

#ifndef ClutFilter_HH
#define ClutFilter_HH

#include <string>
#include <radar/RadarComplex.hh>
using namespace std;

////////////////////////
// This class

class ClutFilter {
  
public:

  // constructor

  ClutFilter();

  // destructor
  
  ~ClutFilter();

  // Perform adaptive filtering on a power spectrum
  //
  // Inputs:
  //   rawPowerSpec: unfiltered power spectrum
  //   nSamples: number of samples
  //   maxClutterVel: max velocity of the clutter component
  //                  of the signal (m/s)
  //   initNotchWidth: width of first guess notch (m/s)
  //   nyquist: unambiguous vel (m/s)
  //   calibratedNoise: noise power at digitizer from calibration (mW)
  //   setNotchToNoise: if true, points within the notch will be
  //                       set to the calibrated noise
  //
  // Outputs:
  //
  //   clutterFound: true if clutter is identified in signal
  //   filteredPowerSpec: power spectrum after filtering
  //   notchStart: spectral position of start of final filtering notch
  //   notchEnd: spectral position of end of final filtering notch
  //   rawPower: mean power in unfiltered spectrum
  //   filteredPower: mean power in filtered spectrum
  //   powerRemoved: mean power removed by the filter (mW)
  //   spectralNoise: noise determined from the spectrum (mW)
  //   weatherPos: spectral location of weather peak
  //   clutterPos: spectral location of clutter peak

  static void performAdaptive(const double *rawPowerSpec, 
                              int nSamples,
                              double maxClutterVel,
                              double initNotchWidth,
                              double nyquist,
                              double calibratedNoise,
                              bool setNotchToNoise,
                              bool &clutterFound,
                              double *filteredPowerSpec,
                              int &notchStart,
                              int &notchEnd,
                              double &rawPower,
                              double &filteredPower,
                              double &powerRemoved,
                              double &spectralNoise,
                              int &weatherPos,
                              int &clutterPos);
  
  // Given a spectrum which has been filtered,
  // fill in the notch using a gaussian fit.
  // The phase information is preserved.
  //
  // Inputs:
  //   filtSpec: filtered spectrum, which has power reduced
  //             in the notch
  //   nSamples: number of samples
  //   maxNotchWidth: max width of filter notch in spectrum
  //
  // Outputs:
  //
  //   filledSpec: result of filling in spectrum
  //               with same phases as filtSpec
  
  static void fillNotchUsingGfit(const RadarComplex_t *filtSpec,
                                 int nSamples,
                                 int maxNotchWidth,
                                 RadarComplex_t *filledSpec);

  // perform notch filtering on a power spectrum
  //
  // Inputs:
  //   rawPowerSpec: unfiltered power spectrum
  //   nSamples: number of samples
  //   notchWidthMps: notch width (m/s)
  //   nyquist: unambiguous vel (m/s)
  //   notchPower: power top be set within the notch
  //
  // Outputs:
  //
  //   filteredPowerSpec: power spectrum after filtering
  //   notchStart: spectral position of start of final filtering notch
  //   notchEnd: spectral position of end of final filtering notch
  //   rawPower: mean power in unfiltered spectrum
  //   filteredPower: mean power in filtered spectrum
  //   powerRemoved: mean power removed by the filter (mW)
  
  static void performNotch(const double *rawPowerSpec, 
                           int nSamples,
                           double notchWidthMps,
                           double nyquist,
                           double notchPower,
                           double *filteredPowerSpec,
                           int &notchStart,
                           int &notchEnd,
			   double &rawPower,
			   double &filteredPower,
			   double &powerRemoved);

  // find weather and clutter
  // Divide spectrum into 8 parts, compute peaks and means
  // for each part. Check for bi-modal spectrum.

  static void locateWxAndClutter(const double *power,
                                 int nSamples,
                                 double max_clutter_vel,
                                 double init_notch_width,
                                 double nyquist,
                                 int &notchWidth,
                                 bool &clutterFound,
                                 int &clutterPos,
                                 double &clutterPeak,
                                 int &weatherPos,
                                 double &weatherPeak,
                                 double &spectralNoise);

  // estimate the spectral noise
  //
  // Divide spectrum into 8 parts, compute power in each part
  // estimate the spectral noise as the mean of the power
  // in the lowest 1/8th.
  
  static double estimateSpectralNoise(const double *powerSpec, int nSamples);
  
  // fit a gaussian to a spectrum
  //
  // Inputs:
  //   power: power spectrum
  //   nSamples
  //   weatherPos: approx spectral location of the weather signal
  //   spectralNoise: estimated noise for the spectrum
  //
  // Output:
  //   gaussian: gaussian fit, for each spectral point
  
  static void fitGaussian(const double *power,
                          int nSamples, 
                          int weatherPos,
                          double spectralNoise,
                          double *gaussian);
  
  // compute noise of a power spectrum
  // 
  // We compute the mean power for 3 regions of the spectrum:
  //   1. 1/8 at lower end plus 1/8 at upper end
  //   2. 1/4 at lower end
  //   3. 1/4 at uppoer end
  // We estimate the noise to be the least of these 3 values
  // because if there is a weather echo it will not affect both ends
  // of the spectrum unless the width is very high, in which case we
  // probably have a bad signal/noise ratio anyway.
  //
  // Inputs:
  //   powerSpec: power spectrum
  //   nSamples
  //
  // Outputs:
  //   noiseMean: mean of the noise in the relevant 1/4 of the spectrum
  //   noiseSdev: standard deviation of the same
  
  static void computeSpectralNoise(const double *powerSpec,
                                   int nSamples,
                                   double &noiseMean,
                                   double &noiseSdev);
  
  /////////////////////////////////////////////////////
  // Compute noise from a power spectrum
  //
  // Divide spectrum into sections and compute the mean power
  // for each section.
  //
  // The noise power is estimated as the mimumum of the section
  // powers.
  
  static double computeSpectralNoise(const double *powerSpec,
				     int nSamples);
  
protected:
private:
  
};

#endif

