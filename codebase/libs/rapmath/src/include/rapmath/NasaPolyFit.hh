///////////////////////////////////////////////////////////////////////
// This class is based on NASA code at:
//
//  github.com/nasa/polyfit
//
// originally written by Ianik Plante.
//
// The mods were created with the assistance of ChatGpt.
//
// Mike Dixon, EOL, NCAR, Boulder, Colorado
// June 2026
//
///////////////////////////////////////////////////////////////////////

// ********************************************************************
// * Code PolyFit                                                     *
// * Written by Ianik Plante                                          *
// *                                                                  *
// * KBR                                                              *
// * 2400 NASA Parkway, Houston, TX 77058                             *
// * Ianik.Plante-1@nasa.gov                                          *
// *                                                                  *
// * This code is used to fit a series of n points with a polynomial  *
// * of degree k, and calculation of error bars on the coefficients.  *
// * If error is provided on the y values, it is possible to use a    *
// * weighted fit as an option. Another option provided is to fix the *
// * intercept value, i.e. the first parameter.                       *
// *                                                                  *
// * This code has been written partially using data from publicly    *
// * available sources.                                               *
// *                                                                  *  
// * The code works to the best of the author's knowledge, but some   *   
// * bugs may be present. This code is provided as-is, with no        *
// * warranty of any kind. By using this code you agree that the      * 
// * author, the company KBR or NASA are not responsible for possible *
// * problems related to the usage of this code.                      * 
// *                                                                  *   
// * The program has been reviewed and approved by export control for * 
// * public release. However some export restriction exists. Please   *    
// * respect applicable laws.                                         *
// *                                                                  *   
// ********************************************************************


#ifndef NASAPOLYFIT_HH
#define NASAPOLYFIT_HH

/**********************************************************************
 * PolyFit.hh
 *
 * Modern C++ polynomial least-squares fitting utility.
 *
 * Usage example:
 *
 *   #include <rapmath/NasaPolyFit.hh>
 *       nasapolyfit::fitPolynomial(xPtr, yPtr, wPtr, nPoints, order);
 *
 * Notes:
 *
 *   - weights are direct least-squares weights.
 *   - weight <= 0 means the point is ignored.
 *   - if weights represent standard deviation sigma, use 1 / sigma^2.
 *   - no explicit matrix inverse is formed.
 *   - coefficients are returned in increasing power order:
 *       coeffs[0] + coeffs[1] * x + coeffs[2] * x^2 + ...
 *
 **********************************************************************/

#include <string>
#include <vector>

namespace nasapolyfit {

struct FitResult {
  bool success = false;
  std::string message;

  std::vector<double> coeffs;

  double rss = 0.0;       // residual sum of squares
  double tss = 0.0;       // total sum of squares
  double rSquared = 0.0;
  double rmse = 0.0;

  int nUsed = 0;
  int order = 0;
  int degreesOfFreedom = 0;
};

// Unweighted fit using std::vector inputs.
FitResult fitPolynomial(const std::vector<double>& x,
                        const std::vector<double>& y,
                        int order);

// Weighted fit using std::vector inputs. The weights are direct weights.
FitResult fitPolynomial(const std::vector<double>& x,
                        const std::vector<double>& y,
                        const std::vector<double>& weights,
                        int order);

// Pointer-array interface. If weights is nullptr, an unweighted fit is used.
FitResult fitPolynomial(const double* x,
                        const double* y,
                        const double* weights,
                        int nPoints,
                        int order);

// Evaluate c[0] + c[1] * x + c[2] * x^2 + ... using Horner's method.
double evaluatePolynomial(const std::vector<double>& coeffs, double x);

// Evaluate derivative: c[1] + 2*c[2]*x + 3*c[3]*x^2 + ...
double evaluatePolynomialDerivative(const std::vector<double>& coeffs, double x);

}  // namespace nasapolyfit

#endif  // NASAPOLYFIT_HH
