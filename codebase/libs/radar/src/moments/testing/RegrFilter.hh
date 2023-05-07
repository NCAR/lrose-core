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
////////////////////////////////////////////////////////////////////////
// RegrFilter.hh
//
// running the regression filter
//
////////////////////////////////////////////////////////////////////////
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2023
//
///////////////////////////////////////////////////////////////

#ifndef RegrFilter_hh
#define RegrFilter_hh

#include <vector>
#include <complex>
#include "FftManager.hh"
#include "PolyFit.hh"
using namespace std;

class RegrFilter {
  
public:
  
  // constructor

  RegrFilter();
  
  // destructor
  
  virtual ~RegrFilter();

  // get filter results after the filter has been applied
  
  double getCnrDb() const { return _cnrDb; } // clutter to noise ratio initial estimate
  double getCsrDb() const { return _csrDb; } // clutter to signal ratio initial estimate
  double getPolyOrder() const { return _polyOrder; } // polynomial order used

  int getNotchStart() const { return _notchStart; }
  int getNotchEnd() const { return _notchEnd; }

  double getFilterRatio() const { return _filterRatio; } // ratio of raw to unfiltered power
  double getSpectralNoise() const { return _spectralNoise; } // noise estimated from spectrum
  double getSpectralSnr() const { return _spectralSnr; } // SNR estimated from spectrum

  // apply polynomial regression clutter filter to IQ time series
  // NOTE: input raw IQ data should not be windowed.
  //
  // Inputs:
  //   nSamples
  //   antennaRateDegPerSec: antenna rate in deg / sec
  //   prtSecs: PRT in secs
  //   wavelengthM: wavelength in meters
  //   calNoise: calibration estimate of noise for this channel, linear units
  //   applyDbForDbCorrection: apply nexrad legacy power correction
  //   iqRaw: unfiltered time series, not windowed
  //
  //  Outputs (memory must be allocated by caller):
  //    iqFiltered: filtered time series, gaussian interpolation
  //    iqNotched: if non-NULL, notched time series, not interpolated
  //    iqPolyFit: if non-NULL, polynomial fit
  //
  //  After calling this routine, you can call:
  //    getCnrDb() to get the estimated clutter-to-noise ratio
  //    getCsrDb() to get the estimated clutter-to-signal ratio
  //    getPolyOrder() to get the polynomial order used
  
  void applyFilter(int nSamples,
                   double antennaRateDegPerSec,
                   double prtSecs,
                   double wavelengthM,
                   double calNoise,
                   bool applyDbForDbCorrection,
                   const complex<double> *iqRaw,
                   complex<double> *iqFiltered,
                   complex<double> *iqNotched = NULL,
                   complex<double> *iqPolyFit = NULL);
  
  /////////////////////////////////////////////////////////////////////////
  // compute the power from the central 3 points in the FFT
  
  static double
    compute3PtClutPower(int nSamples, const complex<double> *rawIq);
  
  /////////////////////////////////////////////
  // compute mean power of time series
  
  static double
    meanPower(int nSamples, const complex<double> *c1);
  
  static double
    meanPower(int nSamples, const double *c1);
  
  ////////////////////////////////////////
  // load power from complex
  
  static void
    loadPower(int nSamples, const complex<double> *in, double *power);
  
  /////////////////////////////////////////////////////////////////////////
  // compute csr and cnr
  
  static void
    computePowerRatios(int nSamples, double calNoise, const complex<double> *iqRaw,
                       double &cnrDb, double &csrDb);
  
  /////////////////////////////////////////////////////////////////////////
  // compute polynomial order
  
  static int
    computePolyOrder(int nSamples,
                     double cnrDb,
                     double antRateDegPerSec,
                     double prtSecs,
                     double wavelengthM);
  
  ///////////////////////////////
  // fit gaussian to spectrum
  
  static void
    fitGaussian(const double *power,
                int nSamples, 
                int weatherPos,
                double spectralNoise,
                double *gaussian);
  
  /////////////////////////////////////////////////////
  // Compute noise from a power spectrum
  //
  // Divide spectrum into runs and compute the mean power
  // for each run, incrementing by one index at a time.
  //
  // The noise power is estimated as the mimumum of the section
  // powers.
  
  static double
    computeSpectralNoise(int nSamples, const double *powerSpec);
  
  //////////////////////////////////////////////////////////////////////
  // Interpolate across the regression filter notch
  
  static void
    doInterpAcrossNotch(vector<double> &regrSpec,
                        int &notchStart,
                        int &notchEnd);
  
  /////////////////////////////////////////////////////
  // Perform regression filtering on I,Q data
  //
  // Inputs:
  //   nSamples
  //   rawIq: raw I,Q data
  //   cnr3Db: clutter-to-noise-ratio from center 3 spectral points
  //   antennaRateDegPerSec: antenna rate - higher rate widens clutter
  //   double prtSecs: PRT for the passed-in IQ values
  //
  // Outputs:
  //   filteredIq: filtered I,Q data
  
  static void
    applyRegrFilter(int nSamples,
                    const complex<double> *rawIq,
                    double cnr3Db,
                    double antennaRateDegPerSec,
                    double prtSecs,
                    complex<double> *filteredIq);
  
  /////////////////////////////////////////////////////////////////
  // Compute correction ratio to be applied to filtered time series
  // to account for noise added to the spectrum by the clutter peak
  //
  // The computations are carried out in dB space, because the
  // corrections are normally discussed in this manner.
  
  static double
    computeDbForDbCorrectionRatio(int nSamples,
                                  double spectralSnr,
                                  double rawPower,
                                  double filteredPower,
                                  double powerRemoved,
                                  double calNoise);

protected:
private:

  FftManager _fft;
  PolyFit _poly;
  
  double _cnrDb;
  double _csrDb;
  int _polyOrder;
  int _notchStart, _notchEnd;

  double _filterRatio;    // ratio of raw to unfiltered power
  double _spectralNoise;  // spectral noise estimated from the spectrum
  double _spectralSnr;    // ratio of spectral noise to noise power

};
  
#endif
