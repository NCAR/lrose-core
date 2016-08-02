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
// MomentsMgr.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2005
//
///////////////////////////////////////////////////////////////
//
// MomentsMgr manages the use of the Moments objects, handling 
// the specific parameters for each case.
//
///////////////////////////////////////////////////////////////

#ifndef MomentsMgr_hh
#define MomentsMgr_hh

#include <string>
#include <vector>
#include <cstdio>
#include "Params.hh"
#include "Pulse.hh"
#include "Moments.hh"
using namespace std;

class OpsInfo;

////////////////////////
// This class

class MomentsMgr {
  
public:
  
  // constructor
  
  MomentsMgr (const string &prog_name,
	      const Params &params,
              const OpsInfo &opsInfo);
  
  // destructor
  
  ~MomentsMgr();
  
  // compute moments
  
  void computeMoments(double prt, int nGatesPulse, Complex_t **IQ,
		      double *powerDbm, double *snr, double *dbz,
		      double *vel, double *width, int *flags);
  
  // compute refract variables
  
  void computeAiqNiq(int nGatesPulse, int nSamples,
                     Complex_t **IQ,
                     double *aiq, double *niq,
                     double *mean_i, double *mean_q);

  void computeAlternating(double prt, int nGatesPulse,
                          Complex_t **IQ, Complex_t **IQH, Complex_t **IQV,
                          double *powerDbm, double *snr, double *dbz,
                          double *vel, double *width, int *flags);
  
  // get methods
  
  int getMaxGates() const { return _maxGates; }
  bool getUseFft() const { return _useFft; }
  Moments::window_t getFftWindow() const { return _fftWindow; }
  int getNSamples() const { return _nSamples; }

  const double *getRangeCorr() const { return _rangeCorr; }

  double getDbz0Chan0() const { return _dbz0Chan0; }
  double getDbz0Chan1() const { return _dbz0Chan1; }
  double getAtmosAtten() const { return _atmosAtten; }

  const Moments &getMoments() const { return _moments; }

protected:
  
private:

  static const double _missingDbl;
  static const int _maxGates = 4096;
  
  string _progName;
  const Params &_params;
  const OpsInfo &_opsInfo;
  
  // number of pulse samples

  int _nSamples;
  
  // fft processing

  bool _useFft;
  Moments::window_t _fftWindow;

  // range correction

  mutable double *_rangeCorr;
  mutable double _startRange;
  mutable double _gateSpacing;

  // calibration
  
  double _atmosAtten;  // db/km
  double _noiseFixedChan0;
  double _noiseFixedChan1;
  double _dbz0Chan0;
  double _dbz0Chan1;

  // Moments object

  Moments _moments;
  Moments _momentsHalf; // for alternating mode dual pol

  // functions
  
  void _init();

  void _computeRangeCorrection();

  Complex_t _meanConjugateProduct(const Complex_t *c1,
                                  const Complex_t *c2,
                                  int len) const;

  double _computeArg(const Complex_t &cc) const;
 
};

#endif

