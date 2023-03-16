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
  //   clutterWidthMps: spectrum width for clutter model (m/s)
  //   initNotchWidthMps: width of first guess notch (m/s)
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
                              double clutterWidthMps,
                              double initNotchWidthMps,
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
                                 double clutterWidthMps,
                                 double initNotchWidthMps,
                                 double nyquist,
                                 int &notchWidth,
                                 bool &clutterFound,
                                 int &clutterPos,
                                 double &clutterPeak,
                                 int &weatherPos,
                                 double &weatherPeak,
                                 double &spectralNoise);

  // compute half notch using clutter model
  // we find the spectral points at which the clutter model
  // crosses 
  
  static int computeHalfNotchWidth(const double *power,
                                   int nSamples,
                                   double clutterWidthMps,
                                   double initNotchWidthMps,
                                   double nyquist);
  
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
  
  // Compute noise from a power spectrum
  //
  // Divide spectrum into runs and compute the mean power
  // for each run, incrementing by one index at a time.
  //
  // The noise power is estimated as the mimumum of the section
  // powers.
  
  static double computeSpectralNoise(const double *powerSpec,
                                     int nSamples);
  
  //////////////////////////////////////////////
  // Compute a gaussian clutter model, based
  // on an observed power spectrum.
  //
  // Powers are linear - i.e. not dBm.
  //
  // Assume:
  // (a) Spectrum is shifted so DC is centered.
  // (b) Clutter is centered - i.e. 0 vel.
  // (c) Clutter width is supplied.
  //
  // The model will match the peak of the spectrum at DC.
  // If the peak is not at DC, the model will be set to missing.
  // Missing power values are set to 1.0e-12 = -120 dBm. 
  //
  // The caller manages the memory for gaussianModel.
  //
  // Returns 0 if clutter is found, -1 otherwise.
  
  static int computeGaussianClutterModel(const double *powerSpectrum,
                                         int nSamples, 
                                         double widthMps,
                                         double nyquistMps,
                                         double *gaussianModel,
                                         bool force = false);
  
  /////////////////////////////////////////////////////////////////
  // Shift a spectrum, in place, so that DC is in the center.
  // Swaps left and right sides.
  // DC location location starts at index 0.
  // After the shift:
  //   if n is odd,  the DC location is at the center index
  //   if n is even, the DC location is at index n/2
  
  static void shift(RadarComplex_t *spectrum, int nSamples);
  static void shift(double *spectrum, int nSamples);

  /////////////////////////////////////////////////////////////////
  // Unshift a spectrum, in place, to undo a previous shift.
  // Swaps left and right sides.
  // After the shift, DC is at index 0.
  
  static void unshift(RadarComplex_t *spectrum, int nSamples);
  static void unshift(double *spectrum, int nSamples);
  
  /////////////////////////////////////////////
  // copy arrays
  
  static void copy(RadarComplex_t *dest,
                   const RadarComplex_t *src,
                   size_t nSamples);

  static void copy(double *dest,
                   const double *src,
                   size_t nSamples);
  
  static double MissingPower;

protected:
private:
  
};

#endif

