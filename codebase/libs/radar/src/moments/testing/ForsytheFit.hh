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
// ForsytheFit.hh
//
// Regression fit to (x, y) data set using Forsythe polynomials
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2020
//
///////////////////////////////////////////////////////////////

#ifndef ForsytheFit_hh
#define ForsytheFit_hh

#include <vector>
using namespace std;

class ForsytheFit {
  
public:
  
  // constructor

  ForsytheFit();
  
  // destructor
  
  virtual ~ForsytheFit();

  // clear the data values

  void clear();

  // perform a fit, given (x, y) vals
  
  int performFit(size_t order,
                 const vector<double> &xVals,
                 const vector<double> &yVals);

  // get order
  
  int getOrder() const { return _order; }

  // get number of values
  
  size_t getNVals() const { return _nObs; }
  
  // get coefficients after fit
  
  const vector<double> getCoeffs() const { return _coeffs; }
  
  // get single y value, given the x value
  
  double getYEst(double xx);

  // get the full vector of estimated Y values
  
  const vector<double> &getYEstVector();

  // compute standard error of estimate for the fit
  
  double computeStdErrEst(double &rSquared);

protected:
private:

  size_t _order;        // polynomial order

  vector<double> _coeffs; // polynomial coefficients (0-based)
  
  vector<double> _xObs, _yObs; // observations
  size_t _nObs; // number of obs
  
  vector<double> _yEst; // regression estimate of y, size _nObs

  // arrays for the fitting procedure
  
  vector<double> _aa, _bb, _ff, _cc; // size _order + 2 (1-based)
  vector<double> _dd, _ee; // size _nObs (0-based)
  vector<vector<double> > _bbSave, _eeSave; // if prepareForFit is used
  vector<vector<double> > _xPowers; // polynomial powers of x [_nObs][_order+1]

  // private methods

  void _allocDataArrays();
  void _allocPolyArrays();

};

#endif

