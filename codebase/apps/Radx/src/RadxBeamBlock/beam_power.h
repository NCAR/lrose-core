// %=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%
// ** Ancilla Radar Quality Control System (ancilla)
// ** Copyright BOM (C) 2013
// ** Bureau of Meteorology, Commonwealth of Australia, 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from the BOM.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of the BOM nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// %=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%

#ifndef ANCILLA_MODELS_BEAM_POWER_H
#define ANCILLA_MODELS_BEAM_POWER_H

#include "angle.h"
#include "array.h"
#include "real.h"

namespace rainfields {
namespace ancilla {
  
  class beam_power
  {
  public:
    beam_power(angle beam_width_h, angle beam_width_v);

    auto beam_width_h() const -> angle                      { return beam_width_h_; }
    auto beam_width_v() const -> angle                      { return beam_width_v_; }

    auto calculate(angle theta_h, angle theta_v) const -> real;

  private:
    angle   beam_width_h_;
    angle   beam_width_v_;
    double  four_ln_two_;
    double  inv_h_sqr_;
    double  inv_v_sqr_;
  };

  /**
   * This class creates a rectangular array that represents a 2d cross-sectional view
   * of beam power.  The array is centered on the center (highest power) of the beam.
   *
   * The array is always normalized so that the total power in the array sums to 1.
   */
  class beam_power_cross_section
  {
  public:
    /**
     * \param beam    Model for the beam power
     * \param rows    Number of samples along beam vertically (rows in array)
     * \param cols    Number of samples along beam horizontally (columns in array)
     * \param height  Angular height of the array (6 = +/-3 from beam center)
     * \param width   Angular width of the array (6 = +/-3 from beam center)
     */
    beam_power_cross_section(
          const beam_power& beam
        , angle gate_width
        , size_t rows
        , size_t cols
        , angle height = 6.0_deg
        , angle width = 6.0_deg
        );

    /// Get the number of rows (elevation offsets) in array
    auto rows() const -> size_t                       { return data_.rows(); }

    /// Get the number of columns (azimuthal offsets) in array
    auto cols() const -> size_t                       { return data_.cols(); }

    /// Get the total angular height of the array
    auto height() const -> angle                      { return height_; }

    /// Get the total angular widht of the array
    auto width() const -> angle                       { return width_; }

    /// Transform the cross section to contain integrated 'power loss at height' values
    /**
     * After calling this function values within the array shall no longer represent beam
     * power at a particular offset from beam center.  Instead, the value returned at any
     * point represents the fraction of total beam power at that point and below.  This
     * can be used to determine fraction of signal loss due to terrain obstructions.
     */
    void make_vertical_integration();

    /// Get the vertical angular offset of cell centers from beam center at a particular row
    auto offset_elevation(size_t y) const -> angle    { return elevations_[y]; }

    /// Get the horizontal angular offset of cell centers from beam center at a particular column
    auto offset_azimuth(size_t x) const -> angle      { return azimuths_[x]; }

    /// Get the array value at a row, column
    auto power(size_t y, size_t x) const -> real      { return data_[y][x]; }

    /// Get direct access to the power array
    auto data() const -> const array2<real>&          { return data_; }

  private:
    angle         height_;
    angle         width_;
    array1<angle> elevations_;
    array1<angle> azimuths_;
    array2<real>  data_;
  };
}}

#endif
