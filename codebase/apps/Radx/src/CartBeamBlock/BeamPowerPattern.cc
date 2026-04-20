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
//
// BeamPowerPattern
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2026
//
///////////////////////////////////////////////////////////////
//
// BeamPowerPattern was derived from the beam_power and
// cross_section classes in the BOM RainFields code base.
// With help from ChatGpt
//
///////////////////////////////////////////////////////////////

#include "BeamPowerPattern.hh"

#include <cmath>
#include <cstdio>
#include <sstream>
#include <stdexcept>
#include <algorithm>

//=======================================================================
// Construction / destruction
//=======================================================================

BeamPowerPattern::BeamPowerPattern()
  : _nPatternEl(31),
    _nPatternAz(31),
    _beamWidthAz(euclid::EuclidAngle::fromDegrees(1.0)),
    _beamWidthEl(euclid::EuclidAngle::fromDegrees(1.0)),
    _patternHeight(euclid::EuclidAngle::fromDegrees(3.0)),
    _patternWidth(euclid::EuclidAngle::fromDegrees(3.0)),
    _fourLnTwo(4.0 * std::log(2.0)),
    _invBeamWidthAzSq(1.0),
    _invBeamWidthElSq(1.0)
{
}

BeamPowerPattern::BeamPowerPattern(const euclid::EuclidAngle &beamWidthAz,
                                   const euclid::EuclidAngle &beamWidthEl,
                                   size_t nPatternEl,
                                   size_t nPatternAz,
                                   const euclid::EuclidAngle &patternHeight,
                                   const euclid::EuclidAngle &patternWidth,
                                   bool convolveDwellInAz,
                                   const euclid::EuclidAngle &dwellWidth)
  : BeamPowerPattern()
{
  set(beamWidthAz, beamWidthEl,
      nPatternEl, nPatternAz,
      patternHeight, patternWidth,
      convolveDwellInAz, dwellWidth);
}

BeamPowerPattern::~BeamPowerPattern()
{
}

//=======================================================================
// Public methods
//=======================================================================

void BeamPowerPattern::set(const euclid::EuclidAngle &beamWidthAz,
                           const euclid::EuclidAngle &beamWidthEl,
                           size_t nPatternEl,
                           size_t nPatternAz,
                           const euclid::EuclidAngle &patternHeight,
                           const euclid::EuclidAngle &patternWidth,
                           bool convolveDwellInAz,
                           const euclid::EuclidAngle &dwellWidth)
{
  if (nPatternEl < 1 || nPatternAz < 1) {
    throw std::runtime_error("BeamPowerPattern::set: invalid array dimensions");
  }

  if (!beamWidthAz.isFinite() || !beamWidthEl.isFinite() ||
      !patternHeight.isFinite() || !patternWidth.isFinite() ||
      !dwellWidth.isFinite()) {
    throw std::runtime_error("BeamPowerPattern::set: non-finite angle supplied");
  }

  if (beamWidthAz.radians() <= 0.0 || beamWidthEl.radians() <= 0.0) {
    throw std::runtime_error("BeamPowerPattern::set: beam widths must be positive");
  }

  if (patternHeight.radians() <= 0.0 || patternWidth.radians() <= 0.0) {
    throw std::runtime_error("BeamPowerPattern::set: pattern extents must be positive");
  }

  _nPatternEl = nPatternEl;
  _nPatternAz = nPatternAz;

  _beamWidthAz = beamWidthAz;
  _beamWidthEl = beamWidthEl;

  _patternHeight = patternHeight;
  _patternWidth = patternWidth;

  _elevationOffsets.resize(_nPatternEl);
  _azimuthOffsets.resize(_nPatternAz);
  _power.resize(_nPatternEl * _nPatternAz);

  _fourLnTwo = 4.0 * std::log(2.0);
  _invBeamWidthAzSq = 1.0 / (_beamWidthAz.radians() * _beamWidthAz.radians());
  _invBeamWidthElSq = 1.0 / (_beamWidthEl.radians() * _beamWidthEl.radians());

  _computeOffsets();
  _computeIntrinsicPattern();

  if (convolveDwellInAz) {
    _convolveDwellInAz(dwellWidth);
  }

  _normalize();
}

void BeamPowerPattern::makeVerticalIntegration()
{
  if (!isValid()) {
    return;
  }

  for (size_t iel = 1; iel < _nPatternEl; ++iel) {
    for (size_t iaz = 0; iaz < _nPatternAz; ++iaz) {
      _power[_index(iel, iaz)] += _power[_index(iel - 1, iaz)];
    }
  }
}

void BeamPowerPattern::clear()
{
  _nPatternEl = 0;
  _nPatternAz = 0;
  _elevationOffsets.clear();
  _azimuthOffsets.clear();
  _power.clear();
}

std::string BeamPowerPattern::getSummary() const
{
  std::ostringstream out;
  out << "BeamPowerPattern:"
      << " nPatternEl=" << _nPatternEl
      << " nPatternAz=" << _nPatternAz
      << " beamWidthAzDeg=" << _beamWidthAz.degrees()
      << " beamWidthElDeg=" << _beamWidthEl.degrees()
      << " patternHeightDeg=" << _patternHeight.degrees()
      << " patternWidthDeg=" << _patternWidth.degrees();
  return out.str();
}

//=======================================================================
// Private methods
//=======================================================================

void BeamPowerPattern::_computeOffsets()
{
  // Compute elevation offsets for row centers.
  // Rows span the requested total angular height, centered on zero.

  const double rowOffset0 = 0.5 * (1.0 - static_cast<double>(_nPatternEl));
  const euclid::EuclidAngle deltaEl =
    _patternHeight / static_cast<double>(_nPatternEl);

  for (size_t iel = 0; iel < _nPatternEl; ++iel) {
    _elevationOffsets[iel] =
      (rowOffset0 + static_cast<double>(iel)) * deltaEl;
  }

  // Compute azimuth offsets for column centers.
  // Columns span the requested total angular width, centered on zero.

  const double colOffset0 = 0.5 * (1.0 - static_cast<double>(_nPatternAz));
  const euclid::EuclidAngle deltaAz =
    _patternWidth / static_cast<double>(_nPatternAz);

  for (size_t iaz = 0; iaz < _nPatternAz; ++iaz) {
    _azimuthOffsets[iaz] =
      (colOffset0 + static_cast<double>(iaz)) * deltaAz;
  }
}

double BeamPowerPattern::_computeIntrinsicPower(
    const euclid::EuclidAngle &azOffset,
    const euclid::EuclidAngle &elOffset) const
{
  // Gaussian beam model for a parabolic dish.
  //
  // beamWidthAz and beamWidthEl are the 3 dB beam widths. Therefore
  // at half a beamwidth from center the power drops to 0.5.

  const double azRad = azOffset.radians();
  const double elRad = elOffset.radians();

  const double exponent =
    -_fourLnTwo *
    (azRad * azRad * _invBeamWidthAzSq +
     elRad * elRad * _invBeamWidthElSq);

  return std::exp(exponent);
}

void BeamPowerPattern::_computeIntrinsicPattern()
{
  for (size_t iel = 0; iel < _nPatternEl; ++iel) {
    for (size_t iaz = 0; iaz < _nPatternAz; ++iaz) {
      _power[_index(iel, iaz)] =
        _computeIntrinsicPower(_azimuthOffsets[iaz],
                               _elevationOffsets[iel]);
    }
  }
}

void BeamPowerPattern::_convolveDwellInAz(
    const euclid::EuclidAngle &dwellWidth)
{
  // Convolve the intrinsic beam pattern in azimuth with a rectangular
  // kernel representing antenna motion during the dwell.
  //
  // This is a scan-acquisition refinement, not part of the intrinsic
  // beam physics. In CartBeamBlock it may often be reasonable to leave
  // this turned off.

  if (dwellWidth.radians() <= 0.0) {
    return;
  }

  if (_nPatternAz < 2) {
    return;
  }

  const euclid::EuclidAngle deltaAz =
    _patternWidth / static_cast<double>(_nPatternAz);

  const int start =
    static_cast<int>(std::lround(
      dwellWidth.radians() / (-2.0 * deltaAz.radians())));

  const int end =
    static_cast<int>(std::lround(
      dwellWidth.radians() / (2.0 * deltaAz.radians()))) + 1;

  std::vector<double> intrinsic = _power;
  std::fill(_power.begin(), _power.end(), 0.0);

  for (int ishift = start; ishift < end; ++ishift) {

    const size_t srcX0 = (ishift < 0) ? static_cast<size_t>(-ishift) : 0;
    const size_t srcX1 = (ishift > 0) ? (_nPatternAz - static_cast<size_t>(ishift))
                                      : _nPatternAz;

    for (size_t iel = 0; iel < _nPatternEl; ++iel) {
      for (size_t iaz = srcX0; iaz < srcX1; ++iaz) {
        const size_t dstCol =
          static_cast<size_t>(static_cast<int>(iaz) + ishift);
        _power[_index(iel, dstCol)] += intrinsic[_index(iel, iaz)];
      }
    }

  }
}

void BeamPowerPattern::_normalize()
{
  double total = 0.0;

  for (size_t ii = 0; ii < _power.size(); ++ii) {
    total += _power[ii];
  }

  if (total <= 0.0) {
    throw std::runtime_error("BeamPowerPattern::_normalize: non-positive total power");
  }

  for (size_t ii = 0; ii < _power.size(); ++ii) {
    _power[ii] /= total;
  }
}
