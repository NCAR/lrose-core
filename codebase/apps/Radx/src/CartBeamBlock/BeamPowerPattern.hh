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

#ifndef BEAM_POWER_PATTERN_HH
#define BEAM_POWER_PATTERN_HH

#include <vector>
#include <string>

#include <euclid/EuclidAngle.hh>

/**
 * @class BeamPowerPattern
 *
 * @brief 2D beam power pattern for CartBeamBlock.
 *
 * This class combines two conceptual steps:
 *
 *  1. Define the intrinsic antenna beam power pattern from the horizontal
 *     and vertical 3 dB beam widths.
 *
 *  2. Discretize that pattern onto a rectangular angular grid centered on
 *     the beam axis, optionally convolving in azimuth with a dwell-width
 *     boxcar to account for antenna motion during the dwell.
 *
 * The resulting 2D array is always normalized so that the total power sums
 * to 1.0.
 *
 * Rows correspond to elevation offsets from beam center.
 * Columns correspond to azimuth offsets from beam center.
 *
 * This class is intended for planar beam-power weighting in CartBeamBlock.
 * It is not a spherical/geodetic geometry class.
 */

class BeamPowerPattern
{

public:

  //=======================================================================
  // Construction / destruction
  //=======================================================================

  BeamPowerPattern();

  BeamPowerPattern(const euclid::EuclidAngle &beamWidthEl,
                   const euclid::EuclidAngle &beamWidthAz,
                   size_t nEl,
                   size_t nAz,
                   const euclid::EuclidAngle &patternHeight,
                   const euclid::EuclidAngle &patternWidth,
                   bool convolveDwellInAz = false,
                   const euclid::EuclidAngle &dwellWidth =
                   euclid::EuclidAngle::fromDegrees(1.0));
  
  ~BeamPowerPattern();

  //=======================================================================
  // Public methods
  //=======================================================================

  /**
   * Initialize the beam power pattern.
   *
   * beamWidthAz and beamWidthEl are the intrinsic 3 dB beam widths.
   *
   * patternHeight and patternWidth set the total angular extent of the
   * computed 2D pattern. For example, 6 degrees means +/-3 degrees around
   * beam center.
   *
   * If convolveDwellInAz is true, the intrinsic beam pattern is convolved
   * in azimuth with a boxcar of width dwellWidth. This is intended to model
   * antenna motion during a PPI-style dwell. In many applications for
   * blockage estimation this refinement may not be necessary.
   */
  void set(const euclid::EuclidAngle &beamWidthEl,
           const euclid::EuclidAngle &beamWidthAz,
           size_t nEl,
           size_t nAz,
           const euclid::EuclidAngle &patternHeight,
           const euclid::EuclidAngle &patternWidth,
           bool convolveDwellInAz = false,
           const euclid::EuclidAngle &dwellWidth =
             euclid::EuclidAngle::fromDegrees(1.0));

  /**
   * Convert the pattern to cumulative vertical power.
   *
   * After this call, power(y,x) no longer represents power at a single
   * angular cell. Instead it represents the fraction of total beam power
   * at that cell and below. This is useful for computing partial beam
   * blockage from terrain height relative to the beam.
   */
  void makeVerticalIntegration();

  /// Set all stored power values to zero.
  void clear();

  //=======================================================================
  // Accessors
  //=======================================================================
  
  size_t getNEl() const { return _nEl; }
  size_t getNAz() const { return _nAz; }

  euclid::EuclidAngle getBeamWidthEl() const { return _beamWidthEl; }
  euclid::EuclidAngle getBeamWidthAz() const { return _beamWidthAz; }
  
  euclid::EuclidAngle getDeltaEl() const { return _deltaEl; }
  euclid::EuclidAngle getDeltaAz() const { return _deltaAz; }
  
  euclid::EuclidAngle getHeight() const { return _patternHeight; }
  euclid::EuclidAngle getWidth() const { return _patternWidth; }
  
  const euclid::EuclidAngle &getElevationOffset(size_t iel) const
  {
    return _elevationOffsets[iel];
  }

  const euclid::EuclidAngle &getAzimuthOffset(size_t iaz) const
  {
    return _azimuthOffsets[iaz];
  }

  double getPower(size_t iel, size_t iaz) const
  {
    return _power[_index(iel, iaz)];
  }

  void setPower(size_t iel, size_t iaz, double value)
  {
    _power[_index(iel, iaz)] = value;
  }

  const double *getPowerData() const { return _power.data(); }
  double *getPowerData() { return _power.data(); }
  
  bool isValid() const
  {
    return (_nEl > 0 && _nAz > 0 &&
            _power.size() == _nEl * _nAz);
  }

  std::string getSummary() const;

private:

  //=======================================================================
  // Private methods
  //=======================================================================

  size_t _index(size_t iel, size_t iaz) const
  {
    return iel * _nAz + iaz;
  }

  void _computeOffsets();
  void _computeIntrinsicPattern();
  void _convolveDwellInAz(const euclid::EuclidAngle &dwellWidth);
  void _normalize();
  double _computeIntrinsicPower(const euclid::EuclidAngle &elOffset,
                                const euclid::EuclidAngle &azOffset) const;

  //=======================================================================
  // Private data members
  //=======================================================================

  euclid::EuclidAngle _beamWidthEl;
  euclid::EuclidAngle _beamWidthAz;

  size_t _nEl;
  size_t _nAz;

  euclid::EuclidAngle _deltaEl;
  euclid::EuclidAngle _deltaAz;
  
  euclid::EuclidAngle _patternHeight;
  euclid::EuclidAngle _patternWidth;

  std::vector<euclid::EuclidAngle> _elevationOffsets;
  std::vector<euclid::EuclidAngle> _azimuthOffsets;

  std::vector<double> _power;

  // Precomputed coefficients for intrinsic Gaussian beam pattern.

  double _fourLnTwo;
  double _invBeamWidthAzSq;
  double _invBeamWidthElSq;

};

#endif
