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
// MatrixRegrFilter.hh
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
// the variation around the polynomial.
//
// Uses Vandermonde matrices for polynomial fitting.
//
// See: Torres S and D.S.Zrnic, 1999: Ground clutter filtering with
// a regression filter. Jtech, 16, 1364 - 1372.
//
////////////////////////////////////////////////////////////////

#ifndef MatrixRegrFilter_HH
#define MatrixRegrFilter_HH

#include <radar/RadarComplex.hh>
#include <rapmath/ForsytheFit.hh>
#include <cstdio>
using namespace std;

////////////////////////
// This class

class MatrixRegrFilter {
  
public:

  // constructor

  MatrixRegrFilter();

  // copy constructor
  
  MatrixRegrFilter(const MatrixRegrFilter &rhs);

  // destructor
  
  ~MatrixRegrFilter();

  // assignment
  
  MatrixRegrFilter & operator=(const MatrixRegrFilter &rhs);

  // set number of samples in IQ data and order of
  // polynomial to use
  //
  // nSamples: number of samples in IQ time series
  // polyOrder: order of polynomial for regression
  // orderAuto: determine polynomial order from CNR

  void setup(size_t nSamples, size_t polyOrder = 5,
             bool orderAuto = false);

  // set up regression parameters - staggered PRT
  //
  // nSamples: number of samples in IQ time series
  // staggeredM, staggeredN - stagger ratio = M/N
  //   time series starts with short PRT
  // polyOrder: order of polynomial for regression
  // orderAuto: determine polynomial order from CNR
  //
  // If successful, _setupDone will be set to true.
  // If not successful, _setupDone will be set to false.
  // Failure occurs if it is not possible to compute the
  // SVD of vvA.
  
  void setupStaggered(size_t nSamples,
                      int staggeredM,
                      int staggeredN,
                      size_t polyOrder = 5,
                      bool orderAuto = false);

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
             RadarComplex_t *filteredIq);

  // Perform regression filtering on I,Q data
  // using Forsythe polynomials
  //
  // Inputs:
  //   rawIq: raw I,Q data
  //   cnr3Db: clutter-to-noise-ratio from center 3 spectral points
  //
  // Outputs:
  //   filteredIq: filtered I,Q data
  //
  // Side effect:
  //   polyfitIq is computed
  //
  // Note: assumes setup() has been successfully completed.

  void applyForsythe(const RadarComplex_t *rawIq,
                     double cnr3Db,
                     double antennaRateDegPerSec,
                     double prtSecs,
                     double wavelengthM,
                     RadarComplex_t *filteredIq);
  
  // Perform 3rd-order regression filtering on I,Q data
  // using Forsythe polynomials

  void applyForsythe3(const RadarComplex_t *rawIq,
                      RadarComplex_t *filteredIq);
  
  // compute the power from the central 3 points in the FFT
  
  double compute3PtClutPower(const RadarComplex_t *rawIq);
  
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

  inline size_t getNSamples() const { return _nSamples; }
  inline bool isStaggered() const { return _isStaggered; }
  inline int getStaggeredM() const { return _staggeredM; }
  inline int getStaggeredN() const { return _staggeredN; }

  inline double getX(size_t sampleNum) const {
    if (sampleNum < _nSamples) {
      return _xx[sampleNum];
    } else {
      return -9999;
    }
  }

  inline size_t getNPoly() const { return _polyOrder; }
  inline size_t getNPoly1() const { return _polyOrder1; }
  inline bool getOrderAuto() const { return _orderAuto; }
  inline int getPolyOrderInUse() const { return _polyOrderInUse; }
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

  static const size_t ORDER_ARRAY_MAX = 32;
  static const size_t NSAMPLES_ARRAY_MAX = 1024;
  // static const size_t ORDER_ARRAY_MAX = 2;
  // static const size_t NSAMPLES_ARRAY_MAX = 10;
  
  // data

  size_t _nSamples;
  size_t _polyOrder;    // polynomial order
  size_t _polyOrder1;   // polynomial order plus 1

  bool _isStaggered; // staggered-PRT version
  int _staggeredM;
  int _staggeredN;

  bool _orderAuto;     // determine the order from the clutter to signal ratio
  int _polyOrderInUse; // polynomial order used in auto selection

  bool _setupDone;

  double *_xx;    // x vector - observed
  vector<double> _xxVals;
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

  // for orthogonal polynomials

  ForsytheFit _forsythe;
  ForsytheFit _forsythe3;
  vector<vector<ForsytheFit *>> _forsytheArray;
  
  // methods

  void _init();
  MatrixRegrFilter &_copy(const MatrixRegrFilter &rhs);
  
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

