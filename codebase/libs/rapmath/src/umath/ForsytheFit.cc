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
  _allocPolyArrays();
}

//////////////////////////////////////////////////////////////////
// add a data value

void ForsytheFit::addValue(double xx, double yy) 
{
  
  _xObs.push_back(xx);
  _yObs.push_back(yy);
  _nObs = _xObs.size();

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

  // allocate arrays
  
  _allocDataArrays();

  // check conditions

  if (_nObs < 2 * (_order + 2)) {
    cerr << "ERROR - ForsytheFit::performFit()" << endl;
    cerr << "  Not enough observations to  fit order: " << _order << endl;
    cerr << "  Min n obs: " << 2 * (_order + 2) << endl;
    return -1;
  }

  int fitOrder;
  double sdev;

  _doFit(_order, 0, _nObs, fitOrder, _xObs.data(), _yObs.data(), 
         _coeffs.data(), sdev);  

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
  _yEst = (double *) ucalloc(_nObs, sizeof(double));
  _Xc = (double *) ucalloc(_nObs + 3, sizeof(double));
  _Yx = (double *) ucalloc(_nObs + 3, sizeof(double));
  
}

void ForsytheFit::_allocPolyArrays()

{
  
  _freePolyArrays();
  
  _AA = (double *) ucalloc(_order + 3, sizeof(double));
  _BB = (double *) ucalloc(_order + 3, sizeof(double));
  _CC = (double **) ucalloc2(_order + 3, _order + 3, sizeof(double));

}

/////////////////////////////////////////////////////
// free allocated space

void ForsytheFit::_freeDataArrays()

{
  
  _freeVec(_yEst);
  _freeVec(_Xc);
  _freeVec(_Yx);
  
}

void ForsytheFit::_freePolyArrays()

{
  
  _freeVec(_AA);
  _freeVec(_BB);
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
  
///////////////////////////////////////////////////////////////////////
//         LEAST SQUARES POLYNOMIAL FITTING PROCEDURE            
// ------------------------------------------------------------- 
// This program least squares fits a polynomial to input data.   
// forsythe orthogonal polynomials are used in the fitting.      
// The number of data points is n.                               
// The data is input to the subroutine in x[i], y[i] pairs.      
// The coefficients are returned in c[i],                        
// the smoothed data is returned in v[i],                        
// the order of the fit is specified by m.                       
// The standard deviation of the fit is returned in d.           
// There are two options available by use of the parameter e:    
//  1. if e = 0, the fit is to order m,                          
//  2. if e > 0, the order of fit increases towards m, but will  
//     stop if the relative standard deviation does not decrease 
//     by more than e between successive fits.                   
// The order of the fit then obtained is l.                      
///////////////////////////////////////////////////////////////////////

int ForsytheFit::_doFit(int mm, double ee, int nn, int &ll,
                        double *xx, double *yy,
                        double *coeffs, double &dd)
{
  // System generated locals 
    int i__1;
    double d__1;

    // Builtin functions

    double sqrt(double);

    // Local variables

    static double a[1024], b[1024], d__[1024], e[1024], f[1024];
    static int i__;
    static double v[1024], w, a1, a2, b1, c2[1024], b2, c1, d1, f1, f2;
    static int l2, n1;
    static double v1, v2;
    static double vv;

    //     Labels: 10,15,20,30,50
    // Parameter adjustments
    --coeffs;
    --yy;
    --xx;

    // Function Body
    n1 = mm + 1;
    ll = 0;
    v1 = 1e7;
    //     Initialize the arrays
    i__1 = n1;
    for (i__ = 1; i__ <= i__1; ++i__) {
	a[i__ - 1] = 0.;
	b[i__ - 1] = 0.;
	f[i__ - 1] = 0.;
    }
    i__1 = nn;
    for (i__ = 1; i__ <= i__1; ++i__) {
	v[i__ - 1] = 0.;
	d__[i__ - 1] = 0.;
    }
    d1 = sqrt((double) (nn));
    w = d1;
    i__1 = nn;
    for (i__ = 1; i__ <= i__1; ++i__) {
	e[i__ - 1] = 1. / w;
    }
    f1 = d1;
    a1 = 0.;
    i__1 = nn;
    for (i__ = 1; i__ <= i__1; ++i__) {
	a1 += xx[i__] * e[i__ - 1] * e[i__ - 1];
    }
    c1 = 0.;
    i__1 = nn;
    for (i__ = 1; i__ <= i__1; ++i__) {
	c1 += yy[i__] * e[i__ - 1];
    }
    b[0] = 1. / f1;
    f[0] = b[0] * c1;
    i__1 = nn;
    for (i__ = 1; i__ <= i__1; ++i__) {
	v[i__ - 1] += e[i__ - 1] * c1;
    }
    mm = 1;
    //     Save latest results
L10:
    i__1 = ll;
    for (i__ = 1; i__ <= i__1; ++i__) {
	c2[i__ - 1] = coeffs[i__];
    }
    l2 = ll;
    v2 = v1;
    f2 = f1;
    a2 = a1;
    f1 = 0.;
    i__1 = nn;
    for (i__ = 1; i__ <= i__1; ++i__) {
	b1 = e[i__ - 1];
	e[i__ - 1] = (xx[i__] - a2) * e[i__ - 1] - f2 * d__[i__ - 1];
	d__[i__ - 1] = b1;
	f1 += e[i__ - 1] * e[i__ - 1];
    }
    f1 = sqrt(f1);
    i__1 = nn;
    for (i__ = 1; i__ <= i__1; ++i__) {
	e[i__ - 1] /= f1;
    }
    a1 = 0.;
    i__1 = nn;
    for (i__ = 1; i__ <= i__1; ++i__) {
	c1 += e[i__ - 1] * yy[i__];
    }
    ++(mm);
    i__ = 0;
L15:
    ll = mm - i__;
    b2 = b[ll - 1];
    d1 = 0.;
    if (ll > 1) {
	d1 = b[ll - 2];
    }
    d1 = d1 - a2 * b[ll - 1] - f2 * a[ll - 1];
    b[ll - 1] = d1 / f1;
    a[ll - 1] = b2;
    ++i__;
    if (i__ != mm) {
	goto L15;
    }
    i__1 = nn;
    for (i__ = 1; i__ <= i__1; ++i__) {
	v[i__ - 1] += e[i__ - 1] * c1;
    }
    i__1 = n1;
    for (i__ = 1; i__ <= i__1; ++i__) {
	f[i__ - 1] += b[i__ - 1] * c1;
	coeffs[i__] = f[i__ - 1];
    }
    vv = 0.f;
    i__1 = nn;
    for (i__ = 1; i__ <= i__1; ++i__) {
	vv += (v[i__ - 1] - yy[i__]) * (v[i__ - 1] - yy[i__]);
    }
    //     Note the division is by the number of degrees of freedom
    vv = sqrt(vv / (double) (nn - ll - 1));
    ll = mm;
    if (ee == 0.) {
	goto L20;
    }
    //     Test for minimal improvement
    if ((d__1 = v1 - vv, abs(d__1)) / vv < ee) {
	goto L50;
    }
    //     if error is larger, quit
    if (ee * vv > ee * v1) {
	goto L50;
    }
    v1 = vv;
L20:
    if (mm == n1) {
	goto L30;
    }
    goto L10;
    //     Shift the c[i] down, so c(0) is the constant term
L30:
    i__1 = ll;
    for (i__ = 1; i__ <= i__1; ++i__) {
	coeffs[i__ - 1] = coeffs[i__];
    }
    coeffs[ll] = 0.;
    //     l is the order of the polynomial fitted
    --(ll);
    dd = vv;
    return 0;
    //     Aborted sequence, recover last values
L50:
    ll = l2;
    vv = v2;
    i__1 = ll;
    for (i__ = 1; i__ <= i__1; ++i__) {
	coeffs[i__] = c2[i__ - 1];
    }
    goto L30;
}

#ifdef JUNK

////////////////////////////////////////////////////
// perform a fit

int ForsytheFit::performFit()
  
{

  // allocate arrays
  
  _allocDataArrays();

  // check conditions

  if (_nObs < 2 * (_order + 2)) {
    cerr << "ERROR - ForsytheFit::performFit()" << endl;
    cerr << "  Not enough observations to  fit order: " << _order << endl;
    cerr << "  Min n obs: " << 2 * (_order + 2) << endl;
    return -1;
  }
  
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
  for (size_t i = 0; i < _nObs; i++) {
    yc += _yObs[i];
  }
  
  for (int k = 1; k <= m; k++) {
    _Yx[k]=0.0;
    for (size_t i = 0; i < _nObs; i++) {
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
  for (size_t ii = 0; ii < _order + 1; ii++) {
    _coeffs.push_back(_AA[ii+1]);
  }
  
  return 0;

}

#endif
