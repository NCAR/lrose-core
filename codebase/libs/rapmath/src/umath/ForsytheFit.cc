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
/********************************************************************
 * Approximation of a discrete real function F(x) by least squares
 * ---------------------------------------------------------------
 * Refs:
 *
 * (a) Generation and use of orthogonal polynomials for data-fitting
 *     with a digital computer.
 *     George E. Forsythe.
 *     J.Soc.Indust.Appl.Math, Vol 5, No 2, June 1957.
 *
 * (b) Basic Scientific Subroutines, Vol II.
 *     Fred Ruckdeschel. McGraw Hill, 1981.
 *
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
#include <rapmath/stats.h>
#include <toolsa/mem.h>
#include <iostream>
using namespace std;

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
}

//////////////////////////////////////////////////////////////////
// clear the data values
  
void ForsytheFit::clear()
{
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
// compute standard error of estimate for the fit

double ForsytheFit::computeStdErrEst(double &rSquared)
{

  // compute mean

  double sumy = 0.0;
  for (size_t ii = 0; ii < _nObs; ii++) {
    sumy += _yObs[ii];
  }
  double meany = sumy / _nObs;

  // compute sum of residuals
  
  double sum_dy_squared = 0.0;
  double sum_of_residuals = 0.0;
  for (size_t ii = 0; ii < _nObs; ii++) {
    double xx = _xObs[ii];
    double yy = _yObs[ii];
    double dy = yy - meany;
    double yyEst = getYEst(xx);
    double error = yyEst - yy;
    sum_of_residuals += error * error;
    sum_dy_squared += dy * dy;
  }

  // compute standard error of estimate and r-squared
    
  double stdErrEst = sqrt(sum_of_residuals / (_nObs - 3.0));
  rSquared = (sum_dy_squared - sum_of_residuals) / sum_dy_squared;
  return stdErrEst;

}


#ifdef __cplusplus
extern "C" {
#endif

extern int ls_poly_(int *order, double *ee, int *nn, int *fitOrder,
                    double *xx, double *yy, double *coeffs, double *sdev);  

extern int ls_poly2_(int *order, double *ee, int *nn, int *fitOrder,
                     double *xx, double *yy, double *coeffs, double *sdev);  

#ifdef __cplusplus
}
#endif

////////////////////////////////////////////////////
// perform a fit

int ForsytheFit::performFit()
  
{

  // allocate arrays
  
  _allocDataArrays();

  // check conditions

  if (_nObs < _order + 2) {
    cerr << "ERROR - ForsytheFit::performFit()" << endl;
    cerr << "  Not enough observations to  fit order: " << _order << endl;
    cerr << "  Min n obs: " << _order << endl;
    return -1;
  }

  // perform the fit

  _doFit();

  return 0;

}

////////////////////////////////////////////////////
// perform a fit using Fortran routine

int ForsytheFit::performFitFortran()
  
{

  // allocate arrays
  
  _allocDataArrays();

  // check conditions

  if (_nObs < _order) {
    cerr << "ERROR - ForsytheFit::performFitFortran()" << endl;
    cerr << "  Not enough observations to  fit order: " << _order << endl;
    cerr << "  Min n obs: " << _order << endl;
    return -1;
  }

  int fitOrder;
  double sdev;
  int order = _order;
  double ee = 0.0;
  int nn = _nObs;

  ls_poly_(&order, &ee, &nn, &fitOrder, _xObs.data(), _yObs.data(), _coeffs.data(), &sdev);  
  
  return 0;

}

/////////////////////////////
// initialization

void ForsytheFit::_init()

{

}

/////////////////////////////////////////////////////
// allocate space

void ForsytheFit::_allocDataArrays()

{
  
  _yEst.resize(_nObs);
  _dd.resize(_nObs);
  _ee.resize(_nObs);
  _vv.resize(_nObs);
  
}

void ForsytheFit::_allocPolyArrays()

{

  _aa.resize(_order + 2);
  _bb.resize(_order + 2);
  _cc.resize(_order + 2);
  _c2.resize(_order + 2);
  _ff.resize(_order + 2);

  _coeffs.resize(_order + 2);

}

///////////////////////////////////////////////////////////////////////
// Fit a polynomial to the observations, using
// forsythe orthogonal polynomials.
///////////////////////////////////////////////////////////////////////

int ForsytheFit::_doFit()
{
  
  // NOTE on array indices
  // the obs arrays (_xObs, yy) and associated arrays (vv, cc, ee) are 0-based.
  // the order arrays (aa, bb, ff, c2) are 1-based
  
  size_t mm1 = _order + 1;
  
  // Initialize the order arrays - 1-based

  for (size_t ii = 1; ii <= mm1; ++ii) {
    _aa[ii] = 0.0;
    _bb[ii] = 0.0;
    _cc[ii] = 0.0;
    _ff[ii] = 0.0;
  }

  double d1 = sqrt((double) _nObs);
  
  for (size_t ii = 0; ii < _nObs; ++ii) {
    _ee[ii] = 1.0 / d1;
  }
  
  double f1 = d1;
  double a1 = 0.0;
  for (size_t ii = 0; ii < _nObs; ++ii) {
    a1 += (_xObs[ii] * _ee[ii] * _ee[ii]);
  }

  double c1 = 0.0;
  for (size_t ii = 0; ii < _nObs; ++ii) {
    c1 += (_yObs[ii] * _ee[ii]);
  }

  _bb[1] = 1.0 / f1;
  _ff[1] = _bb[1] * c1;

  // Initialize the sample arrays - 0-based

  for (size_t ii = 0; ii < _nObs; ++ii) {
    _vv[ii] = 0.0;
    _dd[ii] = 0.0;
  }
  
  for (size_t ii = 0; ii < _nObs; ++ii) {
    _vv[ii] += (_ee[ii] * c1);
  }

  // Main loop, increasing the order as we go

  size_t ll = 0;
  size_t iorder = 1;
  while (iorder < mm1) {
    
    // Save latest results

    for (size_t ii = 1; ii <= ll; ++ii) {
      _c2[ii] = _cc[ii];
    }

    double f2 = f1;
    double a2 = a1;

    f1 = 0.0;
    for (size_t ii = 0; ii < _nObs; ++ii) {
      double b1 = _ee[ii];
      _ee[ii] = (_xObs[ii] - a2) * _ee[ii] - f2 * _dd[ii];
      _dd[ii] = b1;
      f1 += _ee[ii] * _ee[ii];
    }

    f1 = sqrt(f1);

    for (size_t ii = 0; ii < _nObs; ++ii) {
      _ee[ii] /= f1;
    }
    a1 = 0.0;
    for (size_t ii = 0; ii < _nObs; ++ii) {
      a1 += (_xObs[ii] * _ee[ii] * _ee[ii]);
    }

    c1 = 0.0;
    for (size_t ii = 0; ii < _nObs; ++ii) {
      c1 += _ee[ii] * _yObs[ii];
    }

    iorder++;

    size_t jj = 0;
    while (jj < iorder) {
      ll = iorder - jj;
      double b2 = _bb[ll];
      d1 = 0.0;
      if (ll > 1) {
        d1 = _bb[ll - 1];
      }
      d1 = d1 - a2 * _bb[ll] - f2 * _aa[ll];
      _bb[ll] = d1 / f1;
      _aa[ll] = b2;
      ++jj;
    }

    for (size_t ii = 0; ii < _nObs; ++ii) {
      _vv[ii] += _ee[ii] * c1;
    }
    for (size_t ii = 1; ii <= mm1; ++ii) {
      _ff[ii] += _bb[ii] * c1;
      _cc[ii] = _ff[ii];
    }

    ll = iorder;
    
  } // while (iorder < mm1)

  // Load _coeffs arrays - this is 0 based instead of 1 based
  // so shift down by 1

  for (size_t ii = 1; ii <= mm1; ++ii) {
    _coeffs[ii - 1] = _cc[ii];
  }
  _coeffs[mm1] = 0.0;

  return 0;

}

