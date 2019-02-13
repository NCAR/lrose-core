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
///////////////////////////////////////////////////////////////
// RegressionFilter.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
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

#include <iostream>
#include <cstring>
#include <toolsa/toolsa_macros.h>
#include <toolsa/mem.h>
#include <toolsa/TaArray.hh>
#include <radar/RegressionFilter.hh>
#include <rapmath/usvd.h>
using namespace std;

// Constructor

RegressionFilter::RegressionFilter()

{
  _init();
}

/////////////////////////////
// Copy constructor
//

RegressionFilter::RegressionFilter(const RegressionFilter &rhs)

{
  if (this != &rhs) {
    _free();
    _copy(rhs);
  }
}

// destructor

RegressionFilter::~RegressionFilter()

{
  _free();
}

/////////////////////////////
// Assignment
//

RegressionFilter &RegressionFilter::operator=(const RegressionFilter &rhs)

{
  return _copy(rhs);
}

/////////////////////////////
// initialization

void RegressionFilter::_init()

{

  _setupDone = false;

  _nSamples = 0;
  _nPoly = 5;
  _nPoly1 = _nPoly + 1;

  _isStaggered = false;
  _staggeredM = 0;
  _staggeredN = 0;

  _xx = NULL;
  _yyEst = NULL;

  _vv = NULL;
  _vvT = NULL;
  _vvA = NULL;
  _vvB = NULL;
  _uu = NULL;
  _uuT = NULL;
  _ss = NULL;
  _ssInv = NULL;
  _ssVec = NULL;
  _ww = NULL;
  _wwT = NULL;

  _pp = NULL;
  _cc = NULL;
  _multa = NULL;
  _multb = NULL;

  _polyfitIq = NULL;

}

/////////////////////////////
// copy
//

RegressionFilter &RegressionFilter::_copy(const RegressionFilter &rhs)

{

  if (&rhs == this) {
    return *this;
  }

  _init();

  // copy simple members

  _nSamples = rhs._nSamples;
  _nPoly = rhs._nPoly;
  _nPoly1 = rhs._nPoly1;
  _isStaggered = rhs._isStaggered;
  _staggeredM = rhs._staggeredM;
  _staggeredN = rhs._staggeredN;
  _setupDone = rhs._setupDone;
  _stdErrEst = rhs._stdErrEst;

  // allocate arrays

  _alloc();

  // copy the arrays

  memcpy(_xx, rhs._xx, _nSamples * sizeof(double));
  memcpy(_yyEst, rhs._yyEst, _nSamples * sizeof(double));
  memcpy(_ssVec, rhs._ssVec, _nPoly1 * sizeof(double));
  memcpy(_pp, rhs._pp, _nPoly1 * sizeof(double));
  memcpy(_polyfitIq, rhs._polyfitIq, _nSamples * sizeof(RadarComplex_t));

  memcpy(*_vv, *rhs._vv, _nSamples * _nPoly1 * sizeof(double));
  memcpy(*_vvT, *rhs._vvT, _nPoly1 * _nSamples * sizeof(double));
  memcpy(*_cc, *rhs._cc, _nPoly1 * _nSamples * sizeof(double));

  memcpy(*_vvA, *rhs._vvA, _nPoly1 * _nPoly1 * sizeof(double));
  memcpy(*_vvB, *rhs._vvB, _nPoly1 * _nPoly1 * sizeof(double));
  memcpy(*_uu, *rhs._uu, _nPoly1 * _nPoly1 * sizeof(double));
  memcpy(*_uuT, *rhs._uuT, _nPoly1 * _nPoly1 * sizeof(double));
  memcpy(*_ss, *rhs._ss, _nPoly1 * _nPoly1 * sizeof(double));
  memcpy(*_ssInv, *rhs._ssInv, _nPoly1 * _nPoly1 * sizeof(double));
  memcpy(*_ww, *rhs._ww, _nPoly1 * _nPoly1 * sizeof(double));
  memcpy(*_wwT, *rhs._wwT, _nPoly1 * _nPoly1 * sizeof(double));
  memcpy(*_multa, *rhs._multa, _nPoly1 * _nPoly1 * sizeof(double));
  memcpy(*_multb, *rhs._multb, _nPoly1 * _nPoly1 * sizeof(double));

  return *this;

}

/////////////////////////////////////////////////////
// set up regression parameters - fixed PRT
//
// nSamples: number of samples in IQ time series
// nPoly: order of polynomial for regression
//
// If successful, _setupDone will be set to true.
// If not successful, _setupDone will be set to false.
// Failure occurs if it is not possible to compute the
// SVD of vvA.

void RegressionFilter::setup(int nSamples,
                             int nPoly /* = 5*/)

{

  if (_setupDone && _nSamples == nSamples && _nPoly == nPoly) {
    return;
  }

  _nSamples = nSamples;
  _nPoly = nPoly;
  _nPoly1 = _nPoly + 1;

  _isStaggered = false;
  _staggeredM = 0;
  _staggeredN = 0;

  // allocate arrays

  _alloc();

  // load x vector

  double xDelta = 1.0 / _nSamples;
  double xx = -0.5;
  for (int ii = 0; ii < _nSamples; ii++) {
    _xx[ii] = xx;
    xx += xDelta;
  }

  // compute CC matrix for later use

  _computeCc();
  
  _setupDone = true;

}

/////////////////////////////////////////////////////
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

void RegressionFilter::setupStaggered(int nSamples,
                                      int staggeredM,
                                      int staggeredN,
                                      int nPoly /* = 5*/)

{

  if (_setupDone &&
      _isStaggered &&
      _nSamples == nSamples &&
      _nPoly == nPoly &&
      _staggeredM == staggeredM &&
      _staggeredN == staggeredN) {
    return;
  }

  _isStaggered = true;
  _nSamples = nSamples;
  _nPoly = nPoly;
  _nPoly1 = _nPoly + 1;
  _staggeredM = staggeredM;
  _staggeredN = staggeredN;

  // allocate arrays

  _alloc();

  // load x vector

  int nStaggered = (_nSamples / 2) * (_staggeredM + _staggeredN);
  double xDelta = 1.0 / nStaggered;
  double xx = -0.5;
  for (int ii = 0; ii < _nSamples; ii++) {
    _xx[ii] = xx;
    if (ii % 2 == 0) {
      xx += xDelta * _staggeredM;
    } else {
      xx += xDelta * _staggeredN;
    }
  }

  // compute CC matrix for later use

  _computeCc();
  
  _setupDone = true;

}

/////////////////////////////////////////////////////
// Perform regression filtering on I,Q data
//
// Inputs:
//   rawIq: raw I,Q data
//
// Outputs:
//   filteredIq: filtered I,Q data
//
// Side effect:
//   polyfitIq is computed
//
// Note: assumes setup() has been successfully completed.

void RegressionFilter::apply(const RadarComplex_t *rawIq,
                             RadarComplex_t *filteredIq) const

{

  if (_nSamples == 0) {
    cerr << "ERROR - RegressionFilter::apply" << endl;
    cerr << "  Number of samples has not been set" << endl;
    cerr << "  Call setup() before apply()" << endl;
    return;
  }

  // copy IQ data

  TaArray<double> rawI_, rawQ_;
  double *rawI = rawI_.alloc(_nSamples);
  double *rawQ = rawQ_.alloc(_nSamples);

  for (int ii = 0; ii < _nSamples; ii++) {
    rawI[ii] = rawIq[ii].re;
    rawQ[ii] = rawIq[ii].im;
  }

  // poly fit to I

  polyFit(rawI);

  // compute the estimated I polynomial values
  
  _matrixVectorMult(_vv, _pp, _nSamples, _nPoly1, _yyEst);

  // load residuals into filtered Iq
  
  for (int ii = 0; ii < _nSamples; ii++) {
    filteredIq[ii].re = rawI[ii] - _yyEst[ii];
    _polyfitIq[ii].re = _yyEst[ii];
  }
  
  // poly fit to Q

  polyFit(rawQ);

  // compute the estimated Q polynomial values
  
  _matrixVectorMult(_vv, _pp, _nSamples, _nPoly1, _yyEst);

  // load residuals into filtered Iq

  for (int ii = 0; ii < _nSamples; ii++) {
    filteredIq[ii].im = rawQ[ii] - _yyEst[ii];
    _polyfitIq[ii].im = _yyEst[ii];
  }
  
}

/////////////////////////////////////////////////////
// Perform polynomial fit from observed data
//
// Input: yy - observed data
//
// Result:
//   polynomial coefficients in _pp
//
// Note: assumes setup() has been successfully completed.

void RegressionFilter::polyFit(const double *yy) const

{

  if (!_setupDone) {
    cerr << "ERROR - RegressionFilter::polyFit" << endl;
    cerr << "  Setup not successful, cannot perform fit" << endl;
    return;
  }

  for (int ii = 0; ii < _nPoly1; ii++) {
    double sum = 0;
    for (int jj = 0; jj < _nSamples; jj++) {
      sum += _cc[ii][jj] * yy[jj];
    }
    _pp[ii] = sum;
  }

  // compute the standard error of estimates of y
  
  _matrixVectorMult(_vv, _pp, _nSamples, _nPoly1, _yyEst);

  double sumSq = 0.0;
  for (int ii = 0; ii < _nSamples; ii++) {
    double error = _yyEst[ii] - yy[ii];
    sumSq += error * error;
  }
  _stdErrEst = sqrt(sumSq / (double) _nSamples);

#ifdef DEBUG_PRINT
  for (int ii = 0; ii < _nPoly1; ii++) {
    cerr << "ii, pp: " << ii << ", " << _pp[ii][0] << endl;
  }
  for (int ii = 0; ii < _nSamples; ii++) {
    cerr << "ii, yyObserved, yyEst: " << ii
         << ", " << yy[ii] << ", " << _yyEst[ii][0] << endl;
  }
  cerr << "===>> sdtErrEst: " << _stdErrEst << endl;
#endif

}

/////////////////////////////////////////////////////
// allocate space

void RegressionFilter::_alloc()

{
  
  _free();

  _xx = (double *) umalloc(_nSamples * sizeof(double));
  _yyEst = (double *) umalloc(_nSamples * sizeof(double));

  _vv = (double **) umalloc2(_nSamples, _nPoly1, sizeof(double));
  _vvT = (double **) umalloc2(_nPoly1, _nSamples, sizeof(double));
  _vvA = (double **) umalloc2(_nPoly1, _nPoly1, sizeof(double));
  _vvB = (double **) umalloc2(_nPoly1, _nPoly1, sizeof(double));

  _uu = (double **) umalloc2(_nPoly1, _nPoly1, sizeof(double));
  _uuT = (double **) umalloc2(_nPoly1, _nPoly1, sizeof(double));

  _ss = (double **) umalloc2(_nPoly1, _nPoly1, sizeof(double));
  _ssInv = (double **) umalloc2(_nPoly1, _nPoly1, sizeof(double));
  _ssVec = (double *) umalloc(_nPoly1 * sizeof(double));

  _ww = (double **) umalloc2(_nPoly1, _nPoly1, sizeof(double));
  _wwT = (double **) umalloc2(_nPoly1, _nPoly1, sizeof(double));

  _pp = (double *) umalloc(_nPoly1 * sizeof(double));
  _cc = (double **) umalloc2(_nPoly1, _nSamples, sizeof(double));
  _multa = (double **) umalloc2(_nPoly1, _nPoly1, sizeof(double));
  _multb = (double **) umalloc2(_nPoly1, _nPoly1, sizeof(double));

  _polyfitIq = (RadarComplex_t *) umalloc(_nSamples * sizeof(RadarComplex_t));
  
}

/////////////////////////////////////////////////////
// free allocated space

void RegressionFilter::_free()

{
  
  _freeVec(_xx);
  _freeVec(_yyEst);
  _freeVec(_ssVec);
  _freeVec(_pp);
  _freeVec(_polyfitIq);

  _freeMatrix(_vv);
  _freeMatrix(_vvT);
  _freeMatrix(_vvA);
  _freeMatrix(_vvB);
  _freeMatrix(_uu);
  _freeMatrix(_uuT);
  _freeMatrix(_ss);
  _freeMatrix(_ssInv);
  _freeMatrix(_ww);
  _freeMatrix(_wwT);
  _freeMatrix(_cc);
  _freeMatrix(_multa);
  _freeMatrix(_multb);

}

void RegressionFilter::_freeVec(double* &vec)
{
  if (vec != NULL) {
    ufree(vec);
    vec = NULL;
  }
}

void RegressionFilter::_freeVec(RadarComplex_t* &vec)
{
  if (vec != NULL) {
    ufree(vec);
    vec = NULL;
  }
}

void RegressionFilter::_freeMatrix(double** &array)
{
  if (array != NULL) {
    ufree2((void **) array);
    array = NULL;
  }
}

/////////////////////////
// compute the CC matrix

void RegressionFilter::_computeCc()

{

  // compute vandermonde and transpose
  // this loads _vv, _vvT and _vvA
  
  _computeVandermonde();
  
  // compute SVD of vvA
  
  int iret = usvd(_vvA, _nPoly1, _nPoly1, _uu, _ww, _ssVec);
  if (iret) {
    cerr << "ERROR - RegressionFilter::_computeCc()" << endl;
    cerr << "  SVD returns error: " << iret << endl;
    cerr << "  Cannot compute SVD on Vandermonde matrix * transpose" << endl;
    return;
  }

  // fill out diagonal matrix and its inverse
  
  for (int ii = 0; ii < _nPoly1; ii++) {
    for (int jj = 0; jj < _nPoly1; jj++) {
      if (ii == jj) {
        _ss[ii][jj] = _ssVec[ii];
        _ssInv[ii][jj] = 1.0 / _ssVec[ii];
      } else {
        _ss[ii][jj] = 0.0;
        _ssInv[ii][jj] = 0.0;
      } 
    }
  }

  // compute transpose of _uu and _ww

  for (int ii = 0; ii < _nPoly1; ii++) {
    for (int jj = 0; jj < _nPoly1; jj++) {
      _uuT[ii][jj] = _uu[jj][ii];
      _wwT[ii][jj] = _ww[jj][ii];
    }
  }

  // check

  _matrixMult(_uu, _ss, _nPoly1, _nPoly1, _nPoly1, _multa);
  _matrixMult(_multa, _wwT, _nPoly1, _nPoly1, _nPoly1, _vvB);

  // compute cc
  
  _matrixMult(_ww, _ssInv, _nPoly1, _nPoly1, _nPoly1, _multa);
  _matrixMult(_multa, _uuT, _nPoly1, _nPoly1, _nPoly1, _multb);
  _matrixMult(_multb, _vvT, _nPoly1, _nPoly1, _nSamples, _cc);

  // #define DEBUG_PRINT
#ifdef DEBUG_PRINT
  _matrixPrint("_vvB", _vvB, _nPoly1, _nPoly1, stderr);
  _matrixPrint("_uu", _uu, _nPoly1, _nPoly1, stderr);
  _matrixPrint("_uuT", _uuT, _nPoly1, _nPoly1, stderr);
  _matrixPrint("_ww", _ww, _nPoly1, _nPoly1, stderr);
  _matrixPrint("_wwT", _wwT, _nPoly1, _nPoly1, stderr);
  _matrixPrint("_ss", _ss, _nPoly1, _nPoly1, stderr);
  _matrixPrint("_ssInv", _ssInv, _nPoly1, _nPoly1, stderr);
  _matrixPrint("_cc", _cc, _nPoly1, _nSamples, stderr);
#endif
  
}

//////////////////////////////////////////////  
// compute Vandermonde matrix and transpose

void RegressionFilter::_computeVandermonde()

{

  // compute vandermonde and transpose

  for (int ii = 0; ii < _nSamples; ii++) {
    double xx = _xx[ii];
    for (int jj = 0; jj < _nPoly1; jj++) {
      double vv = pow(xx, (double) jj);
      _vv[ii][jj] = vv;
      _vvT[jj][ii] = vv;
    }
  }

  // compute vvA = vvT * vv

  _matrixMult(_vvT, _vv, _nPoly1, _nSamples, _nPoly1, _vvA);

  // debug print

#ifdef DEBUG_PRINT
  _matrixPrint("_vv", _vv, _nSamples, _nPoly1, stderr);
  _matrixPrint("_vvT", _vvT, _nPoly1, _nSamples, stderr);
  _matrixPrint("_vvA", _vvA, _nPoly1, _nPoly1, stderr);
#endif

}
  
//////////////////////////////////////////////  
// multiply two matrices
//
// xx = aa * bb
//
// Notes: nRowsBb = nColsAa
//        aa[nRowsAa][nColsAa]
//        bb[nColsAa][nColsBb]
//        xx[nRowsAa][nColsBb]

void RegressionFilter::_matrixMult(double **aa,
                                   double **bb,
                                   int nRowsAa,
                                   int nColsAa,
                                   int nColsBb,
                                   double **xx) const
  
{
  
  for (int ii = 0; ii < nRowsAa; ii++) {
    for (int jj = 0; jj < nColsBb; jj++) {
      double sum = 0.0;
      for (int kk = 0; kk < nColsAa; kk++) {
        sum += aa[ii][kk] * bb[kk][jj];
      }
      xx[ii][jj] = sum;
    }
  }

}
  
//////////////////////////////////////////////  
// multiply matrix and vector
//
// xx = aa * bb
//
//   aa[nRowsAa][nColsAa]
//   bb[nColsAa]
//   xx[nRowsAa]

void RegressionFilter::_matrixVectorMult(double **aa,
                                         double *bb,
                                         int nRowsAa,
                                         int nColsAa,
                                         double *xx) const
  
{
  
  for (int ii = 0; ii < nRowsAa; ii++) {
    double sum = 0.0;
    for (int kk = 0; kk < nColsAa; kk++) {
      sum += aa[ii][kk] * bb[kk];
    }
    xx[ii] = sum;
  }

}
  
//////////////////////////////////////////////  
// print matrix
//

void RegressionFilter::_matrixPrint(string name,
                                    double **aa,
                                    int nRowsAa,
                                    int nColsAa,
                                    FILE *out) const
  
{

  fprintf(out, "=========== %10s ===========\n", name.c_str());
  for (int ii = 0; ii < nRowsAa; ii++) {
    fprintf(out, "row %3d: ", ii);
    for (int jj = 0; jj < nColsAa; jj++) {
      fprintf(out, " %8.2g", aa[ii][jj]);
    }
    fprintf(out, "\n");
  }
  fprintf(out, "==================================\n");

}
  
//////////////////////////////////////////////  
// print vector
//

void RegressionFilter::_vectorPrint(string name,
                                    double *aa,
                                    int sizeAa,
                                    FILE *out) const
  
{
  
  fprintf(out, "=========== %10s ===========\n", name.c_str());
  for (int ii = 0; ii < sizeAa; ii++) {
    fprintf(out, " %8.2g", aa[ii]);
  }
  fprintf(out, "\n");
  fprintf(out, "==================================\n");

}
  
