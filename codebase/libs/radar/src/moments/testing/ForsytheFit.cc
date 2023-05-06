/********************************************************************
 * Approximation of a discrete real function F(x) by least squares
 * ---------------------------------------------------------------
 * Refs:
 *
 * (a) Méthodes de calcul numérique, Tome 2 by Claude
 *     Nowakowski, PSI Edition, 1984" [BIBLI 04].
 *
 * (b) Generation and use of orthogonal polynomials for data-fitting
 *     with a digital computer.
 *     George E. Forsythe.
 *     J.Soc.Indust.Appl.Math, Vol 5, No 2, June 1957.
 *
 * (c) Basic Scientific Subroutines, Vol II.
 *     Fred Ruckdeschel. McGraw Hill, 1981.
 *
 ********************************************************/
/////////////////////////////////////////////////////////////
// ForsytheFit.cc
//
// Regression fit to (x, y) data set using Forsythe polynomials
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2020
//
///////////////////////////////////////////////////////////////

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
// NOTE on array indices
//   The obs arrays (_xObs, _yObs) and associated 
//     arrays (dd, ee) are 0-based.
//   The order arrays (aa, bb, ff, c2) are 1-based.
///////////////////////////////////////////////////////////////

#include <string>
#include <cmath>
#include <iostream>
#include <cstddef>
#include <complex>
#include "ForsytheFit.hh"
using namespace std;

////////////////////////////////////////////////////
// constructor

ForsytheFit::ForsytheFit()
        
{
  clear();
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
  _order = 0;
  _xObs.clear();
  _yObs.clear();
  _yEst.clear();
  _nObs = 0;
}

////////////////////////////////////////////////////
// perform a fit

int ForsytheFit::performFit(size_t order,
                            const vector<double> &xVals,
                            const vector<double> &yVals)
  
{

  _order = order;
  _allocPolyArrays();
  _xPowers.clear();
  
  _xObs = xVals;
  _yObs = yVals;
  _nObs = _xObs.size();
  if (_xObs.size() != _yObs.size()) {
    cerr << "ERROR - ForsytheFit::performFit" << endl;
    cerr << "  number of X and Y vals differ, they should be the same" << endl;
    cerr << "  n X vals: " << _xObs.size() << endl;
    cerr << "  n Y vals: " << _yObs.size() << endl;
    return -1;
  }

  // allocate arrays
  
  _allocDataArrays();

  // check conditions

  if (_nObs < _order + 2) {
    cerr << "ERROR - ForsytheFit::performFit()" << endl;
    cerr << "  Not enough observations to  fit order: " << _order << endl;
    cerr << "  Min n obs: " << _order << endl;
    return -1;
  }

  size_t order1 = _order + 1;
  
  // Initialize the order arrays - 1-based

  for (size_t ii = 1; ii <= order1; ++ii) {
    _aa[ii] = 0.0;
    _bb[ii] = 0.0;
    _cc[ii] = 0.0;
    _ff[ii] = 0.0;
  }
  
  double rootN = sqrt((double) _nObs);
  _bb[1] = 1.0 / rootN;
  
  for (size_t ii = 0; ii < _nObs; ++ii) {
    _ee[ii] = 1.0 / rootN;
  }
  
  double a1 = 0.0;
  for (size_t ii = 0; ii < _nObs; ++ii) {
    a1 += (_xObs[ii] * _ee[ii] * _ee[ii]);
  }

  double c1 = 0.0;
  for (size_t ii = 0; ii < _nObs; ++ii) {
    c1 += (_yObs[ii] * _ee[ii]);
  }
  _ff[1] = _bb[1] * c1;

  // Initialize the sample arrays - 0-based

  for (size_t ii = 0; ii < _nObs; ++ii) {
    _dd[ii] = 0.0;
  }

  // Main loop, increasing the order as we go

  double f1 = rootN;
  for (size_t iorder = 2; iorder <= order1; iorder++) {
    
    double f1Prev = f1;
    double a1Prev = a1;
    
    double f1Sq = 0.0;
    for (size_t ii = 0; ii < _nObs; ++ii) {
      double ddPrev = _dd[ii];
      _dd[ii] = _ee[ii];
      _ee[ii] = (_xObs[ii] - a1Prev) * _ee[ii] - f1Prev * ddPrev;
      f1Sq += _ee[ii] * _ee[ii];
    }
    f1 = sqrt(f1Sq);

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

    for (size_t jj = 0; jj < iorder; jj++) {
      size_t ll = iorder - jj;
      double bbSave = _bb[ll];
      double bbNew = 0.0;
      if (ll > 1) {
        bbNew = _bb[ll - 1];
      }
      bbNew = bbNew - a1Prev * _bb[ll] - f1Prev * _aa[ll];
      _bb[ll] = bbNew / f1;
      _aa[ll] = bbSave;
    } // jj

    for (size_t ii = 1; ii <= order1; ++ii) {
      _ff[ii] += _bb[ii] * c1;
      _cc[ii] = _ff[ii];
    }

  } // iorder

  // Load _coeffs arrays - this is 0 based instead of 1 based
  // so shift down by 1

  for (size_t ii = 1; ii <= order1; ++ii) {
    _coeffs[ii - 1] = _cc[ii];
  }

  return 0;

}

////////////////////////////////////////////////////
// get single y value, given the x value

double ForsytheFit::getYEst(double xx)
{
  if (_coeffs.size() < 1) {
    return 0.0;
  }
  double yy = _coeffs[0];
  double fac = xx;
  for (size_t ii = 1; ii < _coeffs.size(); ii++) {
    yy += _coeffs[ii] * fac;
    fac *= xx;
  }
  return yy;
}

////////////////////////////////////////////////////
// get full vector of estimated Y value

const vector<double> &ForsytheFit::getYEstVector()
{ 
  _yEst.clear();
  for (size_t ii = 0; ii < _nObs; ii++) {
    _yEst.push_back(getYEst(_xObs[ii]));
  }
  return _yEst;
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

/////////////////////////////////////////////////////
// allocate space

void ForsytheFit::_allocDataArrays()

{
  
  _dd.resize(_nObs);
  _ee.resize(_nObs);
  _yEst.clear();
  
}

void ForsytheFit::_allocPolyArrays()

{

  _aa.resize(_order + 2);
  _bb.resize(_order + 2);
  _cc.resize(_order + 2);
  _ff.resize(_order + 2);
  _coeffs.resize(_order + 1);
  
}

