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

#ifndef ClutFilter_H
#define ClutFilter_H

#include <string>
#include <vector>
#include <cstdio>
#include "Args.hh"
#include "Params.hh"
#include "Complex.hh"
#include "MeasuredSpec.hh"
#include "Fft.hh"

using namespace std;

////////////////////////
// This class

class ClutFilter {
  
public:

  typedef struct {
    double dbm;
    double power;
    double vel;
    double width;
  } moments_t;

  // constructor

  ClutFilter (int argc, char **argv);

  // destructor
  
  ~ClutFilter();

  // run 

  int Run();

  // data members

  bool isOK;

  // debug prints

  int writeComplexSpectra(const string &name,
			  const Complex_t *spec,
			  int nPoints,
                          bool vel = true,
			  bool swap = true);
  
  int writeRealSpectra(const string &name,
		       const double *spec,
		       int nPoints,
                       bool vel = true,
		       bool swap = true);

  int writeRealSpectraSq(const string &name,
			 const double *spec,
			 int nPoints,
                         bool vel = true,
			 bool swap = true);

protected:
  
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;
  Fft *_fft;
  Fft *_fftLong;
  
  double _lambda;
  double _prtSecs;
  double _nyquist;
  double _rNoise;

  int _nSamples;
  static const int _nSamplesLong = 1024;

  string _resultsFilePath;
  FILE *_results;

  double *_hanning;
  double *_blackman;

  vector<MeasuredSpec *> _weatherSpectra;
  vector<MeasuredSpec *> _clutterSpectra;

  int _processModelledSpectra();

  int _processReconstructedSpectra();

  void _processCase(int weatherIndex,
		    int clutterIndex,
		    const Complex_t *weatherSpec,
		    const Complex_t *clutterSpec);

  void _modelSpectra(double weatherDbm,
		     double weatherVel,
		     double weatherWidth,
		     double cwrDbm,
		     double clutterVel,
		     double clutterWidth,
		     Complex_t *weatherSpec,
		     Complex_t *clutterSpec);
  
  int _retrieveSpectra(int weatherIndex,
		       int clutterIndex,
		       Complex_t *weatherSpec,
		       Complex_t *clutterSpec);
  
  void _createGaussian(double power,
		       double vel,
		       double width,
		       Complex_t *spec);

  void _createGaussianWindowed(double power,
			       double vel,
			       double width,
			       Complex_t *spec);

  double _computePower(const Complex_t *IQ);
  
  void _momentsByFft(const Complex_t *spec, double prtSecs,
		     double &power, double &vel, double &width);
  
  double _computeSpectralNoise(const double *magCentered);

  void _initHanning(double *window);

  void _initBlackman(double *window);

  void _applyWindow(const double *window,
		    const Complex_t *in, Complex_t *out);

  int _readFileSpectra(const char *path,
		       double maxAbsVel,
		       double maxWidt,
		       vector<MeasuredSpec *> &spectra);
  
};

#endif

