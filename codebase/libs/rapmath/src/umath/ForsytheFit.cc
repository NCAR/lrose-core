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
/********************************************************
 * Approximation of a discrete real function F(x) by     *
 * least squares                                         *
 * ----------------------------------------------------- *
 * Ref.: "Méthodes de calcul numérique, Tome 2 by Claude *
 *        Nowakowski, PSI Edition, 1984" [BIBLI 04].     *
 * ----------------------------------------------------- *
 * C++ version by J-P Moreau, Paris.                     *
 * (www.jpmoreau.fr)                                     *
 ********************************************************/
///////////////////////////////////////////////////////////////
// ForsytheFit.cc
//
// Regression fit to (x, y) data set using Forsythe polynomials
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2020
//
///////////////////////////////////////////////////////////////

#include <rapmath/ForsytheFit.hh>
#include <toolsa/mem.h>
#include <rapmath/usvd.h>
#include <iostream>
using namespace std;

// #define DEBUG_PRINT

////////////////////////////////////////////////////
// constructor

ForsytheFit::ForsytheFit()
        
{
  _init();
  clear();
  setOrder(3);
}

////////////////////////////////////////////////////
// destructor

ForsytheFit::~ForsytheFit()
  
{
  clear();
  _freePolyArrays();
}

//////////////////////////////////////////////////////////////////
// clear the data values
  
void ForsytheFit::clear()
{
  _freeDataArrays();
  _xObs.clear();
  _yObs.clear();
  _nObs = 0;
}

//////////////////////////////////////////////////////////////////
// set polynomial order

void ForsytheFit::setOrder(size_t order) 
{
  _order = order;
  _orderPlus1 = order + 1;
  _orderPlus2 = order + 2;
  _allocPolyArrays();
}

//////////////////////////////////////////////////////////////////
// add a data value

void ForsytheFit::addValue(double xx, double yy) 
{
  
  _xObs.push_back(xx);
  _yObs.push_back(yy);
  _nObs = _xObs.size();
  _nObsPlus1 = _nObs + 1;

}

//////////////////////////////////////////////////////////////////
// set the data values from vectors
    
void ForsytheFit::setValues(const vector<double> &xVals,
                            const vector<double> &yVals)

{
  
  if (xVals.size() != yVals.size()) {
    cerr << "ERROR - ForsytheFit::setValues()" << endl;
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
  _nObsPlus1 = _nObs + 1;

}

////////////////////////////////////////////////////
// get single y value, given the x value

double ForsytheFit::getYEst(double xx)
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

double ForsytheFit::getYEst(size_t index)
{
  if (index > _nObs - 1) {
    cerr << "ERROR - ForsytheFit::getYEst()" << endl;
    cerr << "  Index out of range: " << index << endl;
    cerr << "  Max index: " << _nObs - 1 << endl;
    return NAN;
  }
  return _yEst[index];
}

////////////////////////////////////////////////////
// get vector of estimated y values

vector<double> ForsytheFit::getYEst() const
{
  vector<double> yyEst;
  for (size_t ii = 0; ii < _nObs; ii++) {
    yyEst.push_back(_yEst[ii]);
  }
  return yyEst;
}

////////////////////////////////////////////////////
// raise to an integer power of log x

double ForsytheFit::_intPower(double xx, int kk) 
{
  if (xx == 0.0) {
    return 0.0;
  } else {
    return exp(kk * log(xx));
  }
}

////////////////////////////////////////////////////
// perform a fit

int ForsytheFit::performFit()
  
{

  if (_nObs < _orderPlus1) {
    return -1;
  }
  
  // allocate arrays
  
  _allocDataArrays();

  // do the fit

  int m = _order;
  int m1 = _order + 1;
  int m2 = _order + 2;

  for (int k = 1; k <= m2; k++) {
    _Xc[k] = 0.0;
    for (size_t i = 0; i < _nObs; i++) {
      _Xc[k] += _intPower(_xObs[i], k);
    }
  }

  double yc = 0.0;
  for (size_t i = 1; i < _nObs; i++) {
    yc += _yObs[i];
  }
  
  for (int k = 1; k <= m; k++) {

    _Yx[k]=0.0;

    for (size_t i = 1; i < _nObs; i++) {
      _Yx[k] += _yObs[i] * _intPower(_xObs[i], k);
    } // i

  } // k
    
  for (int i = 1; i <= m1; i++) {
    for (int j = 1; j <= m1; j++) {
      int ij = i + j - 2;
      if (i == 1 && j == 1) {
        _CC[1][1]= _nObs;
      } else {
        _CC[i][j] = _Xc[ij];
      }
    } // j
  } // i

  _BB[1] = yc;
  for (int i = 2; i <= m1; i++) {
    _BB[i] = _Yx[i-1];
  } // i
    
  for (int k = 1; k <= m; k++) {
    for (int i = k+1; i <= m1; i++) {
      _BB[i] -= _CC[i][k]/_CC[k][k]*_BB[k];
      for (int j = k+1; j <= m1; j++) {
        _CC[i][j] -= _CC[i][k] / _CC[k][k] * _CC[k][j];
      }
    }
  } // k

  _AA[m1] = _BB[m1] / _CC[m1][m1];
  for (int i = m; i > 0; i--)  {
    double s = 0.0;
    for (int k= i+1; k <= m1; k++) {
      s = s + _CC[i][k] * _AA[k];
    }
    _AA[i] = (_BB[i] - s) / _CC[i][i];
  } // i

  // printf("\n Polynomial approximation of degree %d (%ld points)\n", m, _nObs);
  // printf(" Coefficients of polynomial:\n");
  // for (int i = 1; i <= m1; i++) {
  //   printf("  A(%d) = %15.9f\n", i-1, _AA[i]);
  // }
  // printf("\n Approximated function:\n");
  // printf("        X           Y\n");
  // for (size_t i = 0; i < _nObs; i++) {
  //   double xx = _xObs[i];
  //   double p = 0;
  //   for (int k = 1; k <= m1; k++) {
  //     p = p * xx + _AA[m1+1-k];
  //   }
  //   printf(" %11.6f %11.6f\n", xx, p);
  // }
  // printf("\n\n");

  // save the coefficients
  
  _coeffs.clear();
  for (size_t ii = 0; ii < _orderPlus1; ii++) {
    _coeffs.push_back(_AA[ii+1]);
  }
  
  return 0;

}

/////////////////////////////
// initialization

void ForsytheFit::_init()

{

  _yEst = NULL;

  _AA = NULL;
  _BB = NULL;
  _CC = NULL;
  _Xc = NULL;
  _Yx = NULL;

}

/////////////////////////////////////////////////////
// allocate space

void ForsytheFit::_allocDataArrays()

{
  
  _freeDataArrays();
  _yEst = (double *) umalloc(_nObs * sizeof(double));
  
}

void ForsytheFit::_allocPolyArrays()

{
  
  _freePolyArrays();
  
  _CC = (double **) umalloc2(_orderPlus2, _orderPlus2, sizeof(double));

  _AA = (double *) umalloc(_orderPlus2 * sizeof(double));
  _BB = (double *) umalloc(_orderPlus2 * sizeof(double));
  _Xc = (double *) umalloc(_orderPlus2 * sizeof(double));
  _Yx = (double *) umalloc(_orderPlus2 * sizeof(double));

}

/////////////////////////////////////////////////////
// free allocated space

void ForsytheFit::_freeDataArrays()

{
  
  _freeVec(_yEst);
  
}

void ForsytheFit::_freePolyArrays()

{
  
  _freeVec(_AA);
  _freeVec(_BB);
  _freeVec(_Xc);
  _freeVec(_Yx);

  _freeMatrix(_CC);

}

void ForsytheFit::_freeVec(double* &vec)
{
  if (vec != NULL) {
    ufree(vec);
    vec = NULL;
  }
}

void ForsytheFit::_freeMatrix(double** &array)
{
  if (array != NULL) {
    ufree2((void **) array);
    array = NULL;
  }
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

void ForsytheFit::_matrixMult(double **aa,
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

void ForsytheFit::_matrixVectorMult(double **aa,
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

void ForsytheFit::_matrixPrint(string name,
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

void ForsytheFit::_vectorPrint(string name,
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
  
