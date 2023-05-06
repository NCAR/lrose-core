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
// regression_test.hh
//
// testing the regression filter
//
////////////////////////////////////////////////////////////////////////
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2023
//
///////////////////////////////////////////////////////////////

#include <vector>
#include <complex>
using namespace std;

/////////////////////////////////////////////////////////////////////////
// compute the power from the central 3 points in the FFT

extern double
  compute3PtClutPower(int nSamples, const complex<double> *rawIq);

/////////////////////////////////////////////
// compute mean power of time series

extern double
  meanPower(int nSamples, const complex<double> *c1);

extern double
  meanPower(int nSamples, const double *c1);

////////////////////////////////////////
// load power from complex

extern void
  loadPower(int nSamples, const complex<double> *in, double *power);

/////////////////////////////////////////////////////////////////////////
// compute csr and cnr

extern void
  computePowerRatios(int nSamples, double calNoise, const complex<double> *iqRaw,
                     double &cnrDb, double &csrDb);

/////////////////////////////////////////////////////////////////////////
// compute polynomial order

extern int
  computePolyOrder(int nSamples,
                   double cnrDb,
                   double antRateDegPerSec,
                   double prtSecs,
                   double wavelengthM);

///////////////////////////////
// fit gaussian to spectrum

extern void
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

extern double
  computeSpectralNoise(int nSamples, const double *powerSpec);

//////////////////////////////////////////////////////////////////////
// Interpolate across the regression filter notch

extern void
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

extern void
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

extern double
  computePwrCorrectionRatio(bool applyDbForDbCorrection,
                            int nSamples,
                            double spectralSnr,
                            double rawPower,
                            double filteredPower,
                            double powerRemoved,
                            double calNoise);

  
