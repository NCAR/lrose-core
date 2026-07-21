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
// KdpQuadFit.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
// with help from ChatGpt 5.
//
// July 2026
//
///////////////////////////////////////////////////////////////
//
// Fit a local quadratic to PHIDP.
// Compute KDP as the slope at the center of the quadratic.
//
///////////////////////////////////////////////////////////////

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <limits>
#include <vector>

#include <radar/KdpQuadFit.hh>

// Unwrap a PHIDP vector whose values are in degrees.
// Missing values do not update the previous valid phase.

std::vector<double> KdpQuadFit::unwrapDegrees(const std::vector<double>& phaseDeg)
{
  std::vector<double> result(phaseDeg.size(), missingValue());
  
  bool havePrevious = false;
  double previousRaw = 0.0;
  double previousUnwrapped = 0.0;
  
  for (std::size_t ii = 0; ii < phaseDeg.size(); ++ii) {
    
    const double value = phaseDeg[ii];
    
    if (!std::isfinite(value)) {
      continue;
    }

    if (!havePrevious) {
      result[ii] = value;
      previousRaw = value;
      previousUnwrapped = value;
      havePrevious = true;
      continue;
    }

    double delta = value - previousRaw;

    // Map the gate-to-gate phase change into [-180, 180).
    delta = std::remainder(delta, 360.0);

    previousUnwrapped += delta;
    result[ii] = previousUnwrapped;
    previousRaw = value;
  }

  return result;
}

// Solve a 3-by-3 linear system using Gaussian elimination with pivoting.

bool KdpQuadFit::solve3x3(std::array<std::array<double, 3>, 3> matrix,
                          std::array<double, 3> rhs,
                          std::array<double, 3>& solution)
{
  constexpr double epsilon = 1.0e-12;

  for (int col = 0; col < 3; ++col) {

    int pivot = col;
    double pivotMagnitude = std::abs(matrix[col][col]);

    for (int row = col + 1; row < 3; ++row) {
      const double magnitude = std::abs(matrix[row][col]);
      if (magnitude > pivotMagnitude) {
        pivot = row;
        pivotMagnitude = magnitude;
      }
    }

    if (pivotMagnitude < epsilon) {
      return false;
    }

    if (pivot != col) {
      std::swap(matrix[pivot], matrix[col]);
      std::swap(rhs[pivot], rhs[col]);
    }

    const double diagonal = matrix[col][col];

    for (int jj = col; jj < 3; ++jj) {
      matrix[col][jj] /= diagonal;
    }
    rhs[col] /= diagonal;

    for (int row = 0; row < 3; ++row) {

      if (row == col) {
        continue;
      }

      const double factor = matrix[row][col];

      for (int jj = col; jj < 3; ++jj) {
        matrix[row][jj] -= factor * matrix[col][jj];
      }

      rhs[row] -= factor * rhs[col];
    }
  }

  solution = rhs;
  return true;
}

// Local weighted quadratic fit.
//
// x is measured in km relative to the center gate. Using centered x values
// improves numerical conditioning and makes coefficient[1] the derivative
// at the center gate.

KdpQuadFit::LocalFit KdpQuadFit::fitLocalQuadratic(const std::vector<double>& phidpUnwrapped,
                                                   const std::vector<double>* quality,
                                                   std::size_t center,
                                                   int halfWidth,
                                                   double gateSpacingKm)
{
  const std::size_t nGates = phidpUnwrapped.size();

  const std::size_t first =
    center > static_cast<std::size_t>(halfWidth)
    ? center - static_cast<std::size_t>(halfWidth)
    : 0;

  const std::size_t last =
    std::min(nGates - 1,
             center + static_cast<std::size_t>(halfWidth));

  // Weighted sums needed for the normal equations.
  double s0 = 0.0;
  double s1 = 0.0;
  double s2 = 0.0;
  double s3 = 0.0;
  double s4 = 0.0;

  double sy = 0.0;
  double sxy = 0.0;
  double sx2y = 0.0;

  int nValid = 0;

  for (std::size_t jj = first; jj <= last; ++jj) {

    const double y = phidpUnwrapped[jj];

    if (!std::isfinite(y)) {
      continue;
    }

    double weight = 1.0;

    if (quality != nullptr) {
      weight = (*quality)[jj];
      if (!std::isfinite(weight) || weight <= 0.0) {
        continue;
      }
    }

    const double gateOffset =
      static_cast<double>(
              static_cast<long long>(jj) -
              static_cast<long long>(center));

    const double x = gateOffset * gateSpacingKm;
    const double x2 = x * x;

    s0 += weight;
    s1 += weight * x;
    s2 += weight * x2;
    s3 += weight * x2 * x;
    s4 += weight * x2 * x2;

    sy += weight * y;
    sxy += weight * x * y;
    sx2y += weight * x2 * y;

    ++nValid;
  }

  LocalFit result;
  result.nValid = nValid;

  if (nValid < 5 || s0 <= 0.0) {
    return result;
  }

  const std::array<std::array<double, 3>, 3> normalMatrix = {{
      {{s0, s1, s2}},
      {{s1, s2, s3}},
      {{s2, s3, s4}}
    }};

  const std::array<double, 3> normalRhs = {{
      sy, sxy, sx2y
    }};

  std::array<double, 3> coefficients{};

  if (!solve3x3(normalMatrix, normalRhs, coefficients)) {
    return result;
  }

  double weightedSquaredError = 0.0;
  double weightSum = 0.0;

  for (std::size_t jj = first; jj <= last; ++jj) {

    const double y = phidpUnwrapped[jj];

    if (!std::isfinite(y)) {
      continue;
    }

    double weight = 1.0;

    if (quality != nullptr) {
      weight = (*quality)[jj];
      if (!std::isfinite(weight) || weight <= 0.0) {
        continue;
      }
    }

    const double gateOffset =
      static_cast<double>(
              static_cast<long long>(jj) -
              static_cast<long long>(center));

    const double x = gateOffset * gateSpacingKm;

    const double fitted =
      coefficients[0] +
      coefficients[1] * x +
      coefficients[2] * x * x;

    const double residual = y - fitted;

    weightedSquaredError += weight * residual * residual;
    weightSum += weight;
  }

  // Three fitted parameters: intercept, slope and curvature.
  const double degreesOfFreedom =
    std::max(1.0, weightSum - 3.0);

  result.valid = true;
  result.intercept = coefficients[0];
  result.slopeDegPerKm = coefficients[1];
  result.curvature = coefficients[2];
  result.residualStdDeg =
    std::sqrt(weightedSquaredError / degreesOfFreedom);

  return result;
}

KdpQuadFit::FitResult KdpQuadFit::computeQuadraticKdp(const std::vector<double>& phidpDeg,
                                                      double gateSpacingKm,
                                                      const std::vector<double>* quality /* = nullptr */,
                                                      int minHalfWidth /* = 3 */,
                                                      int maxHalfWidth /* = 20 */, 
                                                      int halfWidthIncrement /* = 2 */,
                                                      double targetResidualStdDeg /* = 3.0 */)
  
{
  
  const std::size_t nGates = phidpDeg.size();

  FitResult result;
  result.kdpDegPerKm.assign(nGates, missingValue());
  result.phidpFitDeg.assign(nGates, missingValue());
  result.residualStdDeg.assign(nGates, missingValue());
  result.windowHalfWidth.assign(nGates, -1);
  result.nValid.assign(nGates, 0);

  if (nGates == 0 ||
      gateSpacingKm <= 0.0 ||
      minHalfWidth < 1 ||
      maxHalfWidth < minHalfWidth ||
      halfWidthIncrement < 1) {
    return result;
  }

  if (quality != nullptr && quality->size() != nGates) {
    return result;
  }

  const std::vector<double> phidpUnwrapped =
    unwrapDegrees(phidpDeg);

  for (std::size_t center = 0; center < nGates; ++center) {

    if (!std::isfinite(phidpUnwrapped[center])) {
      continue;
    }

    LocalFit selectedFit;
    int selectedHalfWidth = -1;

    // Keep the best valid fit in case no candidate reaches the target
    // residual.
    LocalFit bestFit;
    int bestHalfWidth = -1;

    for (int halfWidth = minHalfWidth;
         halfWidth <= maxHalfWidth;
         halfWidth += halfWidthIncrement) {

      const LocalFit fit =
        fitLocalQuadratic(phidpUnwrapped,
                          quality,
                          center,
                          halfWidth,
                          gateSpacingKm);

      if (!fit.valid) {
        continue;
      }

      if (!bestFit.valid ||
          fit.residualStdDeg < bestFit.residualStdDeg) {
        bestFit = fit;
        bestHalfWidth = halfWidth;
      }

      // Choose the smallest window that gives an acceptable fit.
      if (fit.residualStdDeg <= targetResidualStdDeg) {
        selectedFit = fit;
        selectedHalfWidth = halfWidth;
        break;
      }
    }

    if (!selectedFit.valid) {
      selectedFit = bestFit;
      selectedHalfWidth = bestHalfWidth;
    }

    if (!selectedFit.valid) {
      continue;
    }

    // PHIDP is two-way differential phase, hence the factor 1/2.
    result.kdpDegPerKm[center] =
      0.5 * selectedFit.slopeDegPerKm;

    result.phidpFitDeg[center] =
      selectedFit.intercept;

    result.residualStdDeg[center] =
      selectedFit.residualStdDeg;

    result.windowHalfWidth[center] =
      selectedHalfWidth;

    result.nValid[center] =
      selectedFit.nValid;
  }

  return result;
}

