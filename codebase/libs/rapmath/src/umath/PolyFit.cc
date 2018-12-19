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
// PolyFit.cc
//
// Fit a polynomial to a data set
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2018
//
///////////////////////////////////////////////////////////////

#include <rapmath/PolyFit.hh>
#include <toolsa/mem.h>
#include <rapmath/usvd.h>
#include <iostream>
using namespace std;

// #define DEBUG_PRINT

////////////////////////////////////////////////////
// constructor

PolyFit::PolyFit()
        
{
  _init();
  setOrder(3);
}

////////////////////////////////////////////////////
// destructor

PolyFit::~PolyFit()
  
{
  clear();
  _freePolyArrays();
}

//////////////////////////////////////////////////////////////////
// clear the data values
  
void PolyFit::clear()
{
  _freeDataArrays();
  _xObs.clear();
  _yObs.clear();
  _nObs = 0;
}

//////////////////////////////////////////////////////////////////
// set polynomial order

void PolyFit::setOrder(size_t order) 
{
  _order = order;
  _orderPlus1 = order + 1;
  _allocPolyArrays();
}

//////////////////////////////////////////////////////////////////
// add a data value

void PolyFit::addValue(double xx, double yy) 
{
  
  _xObs.push_back(xx);
  _yObs.push_back(xx);
  _nObs = _xObs.size();

}

//////////////////////////////////////////////////////////////////
// set the data values from vectors
    
void PolyFit::setValues(const vector<double> &xVals,
                        const vector<double> &yVals)

{
  
  if (xVals.size() != yVals.size()) {
    cerr << "ERROR - PolyFit::setValues()" << endl;
    cerr << "  x and y value vector lengths do not match" << endl;
    cerr << "  xVals.size(): " << xVals.size() << endl;
    cerr << "  yVals.size(): " << yVals.size() << endl;
    clear();
    return;
  }
    
  if (xVals.size() < 1) {
    clear();
    return;
  }

  _xObs = xVals;
  _yObs = yVals;
  _nObs = _xObs.size();

}

////////////////////////////////////////////////////
// get single y value, given the x value

double PolyFit::getYEst(double xx)
{
  if (_coeffs.size() < 1) {
    return 0.0;
  }
  double yy = _coeffs[0];
  for (size_t ii = 1; ii < _coeffs.size(); ii++) {
    yy += _coeffs[ii] * pow(xx, (double) ii);
  }
  return yy;
}

////////////////////////////////////////////////////
// get single y value, given the index

double PolyFit::getYEst(size_t index)
{
  if (index > _nObs - 1) {
    cerr << "ERROR - PolyFit::getYEst()" << endl;
    cerr << "  Index out of range: " << index << endl;
    cerr << "  Max index: " << _nObs - 1 << endl;
    return NAN;
  }
  return _yEst[index];
}

////////////////////////////////////////////////////
// get vector of estimated y values

vector<double> PolyFit::getYEst() const
{
  vector<double> yyEst;
  for (size_t ii = 0; ii < _nObs; ii++) {
    yyEst.push_back(_yEst[ii]);
  }
  return yyEst;
}

////////////////////////////////////////////////////
// perform a fit

int PolyFit::performFit()
  
{
  
  if (_nObs < _orderPlus1) {
    return -1;
  }
  
  // allocate arrays
  
  _allocDataArrays();
  
  // perform polynomial fit
  
  _doFit();
  
  // save the coefficients
  
  _coeffs.clear();
  for (size_t ii = 0; ii < _orderPlus1; ii++) {
    _coeffs.push_back(_pp[ii]);
  }
  
  return 0;

}

/////////////////////////////
// initialization

void PolyFit::_init()

{

  _yEst = NULL;

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

}

/////////////////////////////////////////////////////
// Perform polynomial fit from observed data
// in histogram
//
// Input: _xObs, yVals
//
// Result:
//   polynomial coefficients in _pp

void PolyFit::_doFit()
  
{
  
  // compute CC matrix for later use

  _computeCc();
  
  for (size_t ii = 0; ii < _orderPlus1; ii++) {
    double sum = 0;
    for (size_t jj = 0; jj < _nObs; jj++) {
      sum += _cc[ii][jj] * _yObs[jj];
    }
    _pp[ii] = sum;
  }

  // compute the standard error of estimates of y
  
  _matrixVectorMult(_vv, _pp, _nObs, _orderPlus1, _yEst);
  
  double sumSq = 0.0;
  for (size_t ii = 0; ii < _nObs; ii++) {
    double error = _yEst[ii] - _yObs[ii];
    sumSq += error * error;
  }
  _stdErrEst = sqrt(sumSq / (double) _nObs);


#ifdef DEBUG_PRINT
  for (size_t ii = 0; ii < _orderPlus1; ii++) {
    cerr << "ii, pp: " << ii << ", " << _pp[ii] << endl;
  }
  for (size_t ii = 0; ii < _nObs; ii++) {
    cerr << "ii, yyObserved, yyEst: " << ii
         << ", " << _yObs[ii] << ", " << _yEst[ii] << endl;
  }
  cerr << "===>> sdtErrEst: " << _stdErrEst << endl;
#endif

}

/////////////////////////////////////////////////////
// allocate space

void PolyFit::_allocDataArrays()

{
  
  _freeDataArrays();
  
  _yEst = (double *) umalloc(_nObs * sizeof(double));
  
  _vv = (double **) umalloc2(_nObs, _orderPlus1, sizeof(double));
  _vvT = (double **) umalloc2(_orderPlus1, _nObs, sizeof(double));
  _cc = (double **) umalloc2(_orderPlus1, _nObs, sizeof(double));

}

void PolyFit::_allocPolyArrays()

{
  
  _freePolyArrays();
  
  _ssVec = (double *) umalloc(_orderPlus1 * sizeof(double));
  _pp = (double *) umalloc(_orderPlus1 * sizeof(double));

  _vvA = (double **) umalloc2(_orderPlus1, _orderPlus1, sizeof(double));
  _vvB = (double **) umalloc2(_orderPlus1, _orderPlus1, sizeof(double));

  _uu = (double **) umalloc2(_orderPlus1, _orderPlus1, sizeof(double));
  _uuT = (double **) umalloc2(_orderPlus1, _orderPlus1, sizeof(double));
  
  _ss = (double **) umalloc2(_orderPlus1, _orderPlus1, sizeof(double));
  _ssInv = (double **) umalloc2(_orderPlus1, _orderPlus1, sizeof(double));

  _ww = (double **) umalloc2(_orderPlus1, _orderPlus1, sizeof(double));
  _wwT = (double **) umalloc2(_orderPlus1, _orderPlus1, sizeof(double));

  _multa = (double **) umalloc2(_orderPlus1, _orderPlus1, sizeof(double));
  _multb = (double **) umalloc2(_orderPlus1, _orderPlus1, sizeof(double));

}

/////////////////////////////////////////////////////
// free allocated space

void PolyFit::_freeDataArrays()

{
  
  _freeVec(_yEst);

  _freeMatrix(_vv);
  _freeMatrix(_vvT);
  _freeMatrix(_cc);

}

void PolyFit::_freePolyArrays()

{
  
  _freeVec(_ssVec);
  _freeVec(_pp);

  _freeMatrix(_vvA);
  _freeMatrix(_vvB);
  _freeMatrix(_uu);
  _freeMatrix(_uuT);
  _freeMatrix(_ss);
  _freeMatrix(_ssInv);
  _freeMatrix(_ww);
  _freeMatrix(_wwT);
  _freeMatrix(_multa);
  _freeMatrix(_multb);

}

void PolyFit::_freeVec(double* &vec)
{
  if (vec != NULL) {
    ufree(vec);
    vec = NULL;
  }
}

void PolyFit::_freeMatrix(double** &array)
{
  if (array != NULL) {
    ufree2((void **) array);
    array = NULL;
  }
}

/////////////////////////
// compute the CC matrix

void PolyFit::_computeCc()

{

  // compute vandermonde and transpose
  // this loads _vv, _vvT and _vvA
  
  _computeVandermonde();
  
  // compute SVD of vvA
  
  int iret = usvd(_vvA, _orderPlus1, _orderPlus1, _uu, _ww, _ssVec);
  if (iret) {
    cerr << "ERROR - PolyFit::_computeCc()" << endl;
    cerr << "  SVD returns error: " << iret << endl;
    cerr << "  Cannot compute SVD on Vandermonde matrix * transpose" << endl;
    return;
  }

  // fill out diagonal matrix and its inverse
  
  for (size_t ii = 0; ii < _orderPlus1; ii++) {
    for (size_t jj = 0; jj < _orderPlus1; jj++) {
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

  for (size_t ii = 0; ii < _orderPlus1; ii++) {
    for (size_t jj = 0; jj < _orderPlus1; jj++) {
      _uuT[ii][jj] = _uu[jj][ii];
      _wwT[ii][jj] = _ww[jj][ii];
    }
  }

  // check

  _matrixMult(_uu, _ss, _orderPlus1, _orderPlus1, _orderPlus1, _multa);
  _matrixMult(_multa, _wwT, _orderPlus1, _orderPlus1, _orderPlus1, _vvB);

  // compute cc
  
  _matrixMult(_ww, _ssInv, _orderPlus1, _orderPlus1, _orderPlus1, _multa);
  _matrixMult(_multa, _uuT, _orderPlus1, _orderPlus1, _orderPlus1, _multb);
  _matrixMult(_multb, _vvT, _orderPlus1, _orderPlus1, _nObs, _cc);

#ifdef DEBUG_PRINT
  _matrixPrint("_vvB", _vvB, _orderPlus1, _orderPlus1, stderr);
  _matrixPrint("_uu", _uu, _orderPlus1, _orderPlus1, stderr);
  _matrixPrint("_uuT", _uuT, _orderPlus1, _orderPlus1, stderr);
  _matrixPrint("_ww", _ww, _orderPlus1, _orderPlus1, stderr);
  _matrixPrint("_wwT", _wwT, _orderPlus1, _orderPlus1, stderr);
  _matrixPrint("_ss", _ss, _orderPlus1, _orderPlus1, stderr);
  _matrixPrint("_ssInv", _ssInv, _orderPlus1, _orderPlus1, stderr);
  _matrixPrint("_cc", _cc, _orderPlus1, _nObs, stderr);
#endif
  
}

//////////////////////////////////////////////  
// compute Vandermonde matrix and transpose

void PolyFit::_computeVandermonde()

{

  // compute vandermonde and transpose

  for (size_t ii = 0; ii < _nObs; ii++) {
    double xx = _xObs[ii];
    for (size_t jj = 0; jj < _orderPlus1; jj++) {
      double vv = pow(xx, (double) jj);
      _vv[ii][jj] = vv;
      _vvT[jj][ii] = vv;
    }
  }

  // compute vvA = vvT * vv

  _matrixMult(_vvT, _vv, _orderPlus1, _nObs, _orderPlus1, _vvA);

  // debug print

#ifdef DEBUG_PRINT
  _matrixPrint("_vv", _vv, _nObs, _orderPlus1, stderr);
  _matrixPrint("_vvT", _vvT, _orderPlus1, _nObs, stderr);
  _matrixPrint("_vvA", _vvA, _orderPlus1, _orderPlus1, stderr);
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

void PolyFit::_matrixMult(double **aa,
                          double **bb,
                          size_t nRowsAa,
                          size_t nColsAa,
                          size_t nColsBb,
                          double **xx) const
  
{
  
  for (size_t ii = 0; ii < nRowsAa; ii++) {
    for (size_t jj = 0; jj < nColsBb; jj++) {
      double sum = 0.0;
      for (size_t kk = 0; kk < nColsAa; kk++) {
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

void PolyFit::_matrixVectorMult(double **aa,
                                double *bb,
                                size_t nRowsAa,
                                size_t nColsAa,
                                double *xx) const
  
{
  
  for (size_t ii = 0; ii < nRowsAa; ii++) {
    double sum = 0.0;
    for (size_t kk = 0; kk < nColsAa; kk++) {
      sum += aa[ii][kk] * bb[kk];
    }
    xx[ii] = sum;
  }

}
  
//////////////////////////////////////////////  
// print matrix
//

void PolyFit::_matrixPrint(string name,
                           double **aa,
                           size_t nRowsAa,
                           size_t nColsAa,
                           FILE *out) const
  
{
  
  fprintf(out, "=========== %10s ===========\n", name.c_str());
  for (size_t ii = 0; ii < nRowsAa; ii++) {
    fprintf(out, "row %3d: ", (int) ii);
    for (size_t jj = 0; jj < nColsAa; jj++) {
      fprintf(out, " %8.2g", aa[ii][jj]);
    }
    fprintf(out, "\n");
  }
  fprintf(out, "==================================\n");

}
  
//////////////////////////////////////////////  
// print vector
//

void PolyFit::_vectorPrint(string name,
                           double *aa,
                           size_t sizeAa,
                           FILE *out) const
  
{
  
  fprintf(out, "=========== %10s ===========\n", name.c_str());
  for (size_t ii = 0; ii < sizeAa; ii++) {
    fprintf(out, " %8.2g", aa[ii]);
  }
  fprintf(out, "\n");
  fprintf(out, "==================================\n");

}
  
