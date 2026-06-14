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

/**********************************************************************
 * PolyFit.cc
 *
 * Implementation of modern C++ polynomial least-squares fitting.
 *
 * Usage:
 *
 *   #include "PolyFit.hh"
 *
 *   std::vector<double> x = {...};
 *   std::vector<double> y = {...};
 *   std::vector<double> weights = {...};
 *
 *   polyfit::FitResult fit = polyfit::fitPolynomial(x, y, weights, 3);
 *
 *   if (fit.success) {
 *     const std::vector<double>& c = fit.coeffs;
 *     double y0 = polyfit::evaluatePolynomial(c, x0);
 *     double dydx = polyfit::evaluatePolynomialDerivative(c, x0);
 *   }
 *
 * Compile example:
 *
 *   g++ -std=c++17 -Wall -Wextra -pedantic -c PolyFit.cc
 *
 * Link with your application:
 *
 *   g++ -std=c++17 -Wall -Wextra -pedantic main.cc PolyFit.cc -o test_polyfit
 *
 **********************************************************************/

#include <rapmath/NasaPolyFit.hh>

#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <sstream>
#include <stdexcept>

namespace polyfit {
namespace {

using Matrix = std::vector<std::vector<double>>;

bool isFinite(double v) {
  return std::isfinite(v);
}

// Solve A*x = b using Gaussian elimination with partial pivoting.
// A is intentionally passed by value because the algorithm modifies it.
bool solveLinearSystem(Matrix A,
                       std::vector<double> b,
                       std::vector<double>& x,
                       std::string& message) {

  const int n = static_cast<int>(A.size());

  if (n <= 0 || static_cast<int>(b.size()) != n) {
    message = "Invalid linear system size";
    return false;
  }

  for (const auto& row : A) {
    if (static_cast<int>(row.size()) != n) {
      message = "Matrix is not square";
      return false;
    }
  }

  constexpr double singularTol = 1.0e-14;

  for (int col = 0; col < n; ++col) {

    int pivot = col;
    double maxAbs = std::abs(A[col][col]);

    for (int row = col + 1; row < n; ++row) {
      const double v = std::abs(A[row][col]);
      if (v > maxAbs) {
        maxAbs = v;
        pivot = row;
      }
    }

    if (maxAbs < singularTol) {
      std::ostringstream oss;
      oss << "Singular or ill-conditioned normal-equation matrix at column "
          << col;
      message = oss.str();
      return false;
    }

    if (pivot != col) {
      std::swap(A[pivot], A[col]);
      std::swap(b[pivot], b[col]);
    }

    const double diag = A[col][col];

    for (int row = col + 1; row < n; ++row) {
      const double factor = A[row][col] / diag;
      A[row][col] = 0.0;

      for (int j = col + 1; j < n; ++j) {
        A[row][j] -= factor * A[col][j];
      }
      b[row] -= factor * b[col];
    }
  }

  x.assign(n, 0.0);

  for (int row = n - 1; row >= 0; --row) {
    double sum = b[row];
    for (int col = row + 1; col < n; ++col) {
      sum -= A[row][col] * x[col];
    }

    if (std::abs(A[row][row]) < singularTol) {
      message = "Singular matrix during back substitution";
      return false;
    }

    x[row] = sum / A[row][row];
  }

  message.clear();
  return true;
}

FitResult fitPolynomialImpl(const double* x,
                            const double* y,
                            const double* weights,
                            int nPoints,
                            int order) {

  FitResult result;
  result.order = order;

  if (x == nullptr || y == nullptr) {
    result.message = "x and y pointers must not be null";
    return result;
  }

  if (nPoints <= 0) {
    result.message = "nPoints must be positive";
    return result;
  }

  if (order < 0) {
    result.message = "Polynomial order must be non-negative";
    return result;
  }

  const int nCoeffs = order + 1;

  // Count usable points.
  int nUsed = 0;
  for (int i = 0; i < nPoints; ++i) {
    const double w = (weights == nullptr) ? 1.0 : weights[i];
    if (isFinite(x[i]) && isFinite(y[i]) && isFinite(w) && w > 0.0) {
      ++nUsed;
    }
  }

  result.nUsed = nUsed;
  result.degreesOfFreedom = nUsed - nCoeffs;

  if (nUsed < nCoeffs) {
    std::ostringstream oss;
    oss << "Not enough valid points for polynomial order " << order
        << ": need at least " << nCoeffs
        << ", found " << nUsed;
    result.message = oss.str();
    return result;
  }

  Matrix normal(nCoeffs, std::vector<double>(nCoeffs, 0.0));
  std::vector<double> rhs(nCoeffs, 0.0);

  // Build weighted normal equations:
  //
  //   (X^T W X) c = X^T W y
  //
  // where W is diagonal and weights[i] is W_ii.
  for (int i = 0; i < nPoints; ++i) {

    const double w = (weights == nullptr) ? 1.0 : weights[i];

    if (!isFinite(x[i]) || !isFinite(y[i]) || !isFinite(w) || w <= 0.0) {
      continue;
    }

    std::vector<double> xp(2 * order + 1, 1.0);
    for (int p = 1; p <= 2 * order; ++p) {
      xp[p] = xp[p - 1] * x[i];
    }

    for (int row = 0; row < nCoeffs; ++row) {
      rhs[row] += w * y[i] * xp[row];
      for (int col = 0; col < nCoeffs; ++col) {
        normal[row][col] += w * xp[row + col];
      }
    }
  }

  std::vector<double> coeffs;
  std::string solveMessage;

  if (!solveLinearSystem(normal, rhs, coeffs, solveMessage)) {
    result.message = solveMessage;
    return result;
  }

  result.coeffs = coeffs;

  // Weighted residual statistics.
  double sumW = 0.0;
  double sumWY = 0.0;

  for (int i = 0; i < nPoints; ++i) {
    const double w = (weights == nullptr) ? 1.0 : weights[i];
    if (!isFinite(x[i]) || !isFinite(y[i]) || !isFinite(w) || w <= 0.0) {
      continue;
    }
    sumW += w;
    sumWY += w * y[i];
  }

  const double yMean = (sumW > 0.0) ? (sumWY / sumW) : 0.0;

  double rss = 0.0;
  double tss = 0.0;

  for (int i = 0; i < nPoints; ++i) {
    const double w = (weights == nullptr) ? 1.0 : weights[i];
    if (!isFinite(x[i]) || !isFinite(y[i]) || !isFinite(w) || w <= 0.0) {
      continue;
    }

    const double yFit = evaluatePolynomial(coeffs, x[i]);
    const double resid = y[i] - yFit;
    const double demeaned = y[i] - yMean;

    rss += w * resid * resid;
    tss += w * demeaned * demeaned;
  }

  result.rss = rss;
  result.tss = tss;
  result.rSquared = (tss > 0.0) ? (1.0 - rss / tss) : 1.0;

  if (result.degreesOfFreedom > 0) {
    result.rmse = std::sqrt(rss / static_cast<double>(result.degreesOfFreedom));
  } else {
    result.rmse = 0.0;
  }

  result.success = true;
  result.message = "OK";
  return result;
}

}  // namespace

FitResult fitPolynomial(const std::vector<double>& x,
                        const std::vector<double>& y,
                        int order) {

  if (x.size() != y.size()) {
    FitResult result;
    result.order = order;
    result.message = "x and y vectors must have the same size";
    return result;
  }

  return fitPolynomialImpl(x.data(), y.data(), nullptr,
                           static_cast<int>(x.size()), order);
}

FitResult fitPolynomial(const std::vector<double>& x,
                        const std::vector<double>& y,
                        const std::vector<double>& weights,
                        int order) {

  if (x.size() != y.size() || x.size() != weights.size()) {
    FitResult result;
    result.order = order;
    result.message = "x, y, and weights vectors must have the same size";
    return result;
  }

  return fitPolynomialImpl(x.data(), y.data(), weights.data(),
                           static_cast<int>(x.size()), order);
}

FitResult fitPolynomial(const double* x,
                        const double* y,
                        const double* weights,
                        int nPoints,
                        int order) {

  return fitPolynomialImpl(x, y, weights, nPoints, order);
}

double evaluatePolynomial(const std::vector<double>& coeffs, double x) {

  // Horner's method. Handles an empty coeff vector by returning 0.
  double y = 0.0;
  for (auto it = coeffs.rbegin(); it != coeffs.rend(); ++it) {
    y = y * x + *it;
  }
  return y;
}

double evaluatePolynomialDerivative(const std::vector<double>& coeffs, double x) {

  if (coeffs.size() <= 1) {
    return 0.0;
  }

  // Horner's method applied to derivative coefficients.
  double dydx = 0.0;
  for (int i = static_cast<int>(coeffs.size()) - 1; i >= 1; --i) {
    dydx = dydx * x + static_cast<double>(i) * coeffs[i];
  }

  return dydx;
}

}  // namespace polyfit
