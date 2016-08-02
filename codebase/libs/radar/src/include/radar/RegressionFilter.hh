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
// RegressionFilter.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2009
//
///////////////////////////////////////////////////////////////
//
// Filter clutter by performing a polynomial regression fit to 
// the time series and remove the smoothly-varing values, leaving
// the variation around the polynomial
//
// See: Torres S and D.S.Zrnic, 1999: Ground clutter filtering with
// a regression filter. Jtech, 16, 1364 - 1372.
//
////////////////////////////////////////////////////////////////

#ifndef RegressionFilter_HH
#define RegressionFilter_HH

#include <radar/RadarComplex.hh>
#include <cstdio>
using namespace std;

////////////////////////
// This class

class RegressionFilter {
  
public:

  // constructor

  RegressionFilter();

  // copy constructor
  
  RegressionFilter(const RegressionFilter &rhs);

  // destructor
  
  ~RegressionFilter();

  // assignment
  
  RegressionFilter & operator=(const RegressionFilter &rhs);

  // set number of samples in IQ data and order of
  // polynomial to use
  //
  // nSamples: number of samples in IQ time series
  // nPoly: order of polynomial for regression

  void setup(int nSamples, int nPoly = 5);

  // set up regression parameters - staggered PRT
  //
  // nSamples: number of samples in IQ time series
  // staggeredM, staggeredN - stagger ratio = M/N
  //   time series starts with short PRT
  // nPoly: order of polynomial for regression
  //
  // If successful, _setupDone will be set to true.
  // If not successful, _setupDone will be set to false.
  // Failure occurs if it is not possible to compute the
  // SVD of vvA.
  
  void setupStaggered(int nSamples,
                      int staggeredM,
                      int staggeredN,
                      int nPoly /* = 5*/);

  // Apply regression filtering on I,Q data
  //
  // Inputs:
  //   rawIq: raw I,Q data
  //
  // Outputs:
  //   filteredIq: filtered I,Q data
  //
  // Side effect:
  //   polyfitIq is computed - see getPolyfitIq()
  //
  // Note: call setup first
  
  void apply(const RadarComplex_t *rawIq,
             RadarComplex_t *filteredIq) const;
  
  // Perform polynomial fit from observed data
  //
  // Input: yy - observed data
  //
  // Result:
  //   polynomial coefficients in _pp
  //
  // Note: assumes setup() has been successfully completed.

  void polyFit(const double *yy) const;

  // get methods

  inline int getNSamples() const { return _nSamples; }
  inline bool isStaggered() const { return _isStaggered; }
  inline int getStaggeredM() const { return _staggeredM; }
  inline int getStaggeredN() const { return _staggeredN; }

  inline double getX(int sampleNum) const {
    if (sampleNum < _nSamples) {
      return _xx[sampleNum];
    } else {
      return -9999;
    }
  }

  inline int getNPoly() const { return _nPoly; }
  inline int getNPoly1() const { return _nPoly1; }
  inline bool getSetupDone() const { return _setupDone; }
  inline double* getX() const { return _xx; }
  inline double** getVv() const { return _vv; }
  inline double** getVvT() const { return _vvT; }
  inline double** getVvA() const { return _vvA; }
  inline double** getUu() const { return _uu; }
  inline double** getUuT() const { return _uuT; }
  inline double** getSs() const { return _ss; }
  inline double** getSsInv() const { return _ssInv; }
  inline double** getWw() const { return _ww; }
  inline double** getWwT() const { return _wwT; }
  inline double* getPp() const { return _pp; }
  inline double** getCc() const { return _cc; }
  inline double* getYEst() const { return _yyEst; }
  inline double getStdErrEst() const { return _stdErrEst; }

  inline const RadarComplex_t* getPolyfitIq() const {
    return _polyfitIq;
  }

protected:
private:

  // data

  int _nSamples;
  int _nPoly;    // polynomial order
  int _nPoly1;   // polynomial order plus 1

  bool _isStaggered;
  int _staggeredM;
  int _staggeredN;

  bool _setupDone;

  double *_xx;    // x vector - observed
  double *_yyEst; // regression estimate of y

  double **_vv;   // Vandermonde matrix
  double **_vvT;  // vv transpose
  double **_vvA;  // vvT * vv
  double **_vvB;  // vvT * vv - check
  double **_uu;   // from SVD of vvA: [uu, ss, wwT] = SVD(vvA)
  double **_uuT;  // uu transpose
  double *_ssVec; // from SVD of vvA - diagonal elements
  double **_ss;   // from SVD of vvA - diagonal matrix
  double **_ssInv; // from SVD of vvA - diagonal matrix
  double **_ww;   // from SVD of vvA
  double **_wwT;  // ww transpose
  
  double *_pp;   // regression coefficients - polynomial fit
  double **_cc;  // ww * diag * uuT * vvT
  double **_multa; // temporary matrix for intermediate results
  double **_multb; // temporary matrix for intermediate results

  mutable double _stdErrEst;
  
  RadarComplex_t *_polyfitIq;

  // methods

  void _init();
  RegressionFilter &_copy(const RegressionFilter &rhs);
  
  void _alloc();
  void _free();
  void _freeVec(double* &vec);
  void _freeVec(RadarComplex_t* &vec);
  void _freeMatrix(double** &array);

  void _computeCc();

  void _computeVandermonde();

  void _matrixMult(double **aa,
                   double **bb,
                   int nRowsAa,
                   int nColsAa,
                   int nColsBb,
                   double **xx) const;

  void _matrixVectorMult(double **aa,
                         double *bb,
                         int nRowsAa,
                         int nColsAa,
                         double *xx) const;

  void _matrixPrint(string name,
                    double **aa,
                    int nRowsAa,
                    int nColsAa,
                    FILE *out) const;
  
  void _vectorPrint(string name,
                    double *aa,
                    int sizeAa,
                    FILE *out) const;

};

#endif

