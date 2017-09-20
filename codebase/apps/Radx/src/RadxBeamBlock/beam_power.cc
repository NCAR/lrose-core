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

#include "beam_power.h"

#include "array_utils.h"

#include <cmath>

using namespace rainfields::ancilla;

beam_power::beam_power(angle beam_width_h, angle beam_width_v)
  : beam_width_h_(beam_width_h)
  , beam_width_v_(beam_width_v)
  , four_ln_two_(-4.0 * std::log(2.0))
  , inv_h_sqr_(1.0 / (beam_width_h_.radians() * beam_width_h_.radians()))
  , inv_v_sqr_(1.0 / (beam_width_v_.radians() * beam_width_v_.radians()))
{

}

/* calculate the 2d power of the radar beam at theta_azi and theta_elev degrees 
 * off the beam centre.
 *
 * Probert & Jones 1962 Meteoroogical Radar Equation, QJR Met Soc 88, 485 - 495
 */
auto beam_power::calculate(angle theta_azi, angle theta_elev) const -> real
{
  const auto azi = theta_azi.radians();
  const auto ele = theta_elev.radians();
  return std::exp(four_ln_two_ * ((azi * azi * inv_h_sqr_) + (ele * ele * inv_v_sqr_)));
}

beam_power_cross_section::beam_power_cross_section(
      const beam_power& beam
    , angle gate_width
    , size_t rows
    , size_t cols
    , angle height
    , angle width)
  : height_(height)
  , width_(width)
  , elevations_(rows)
  , azimuths_(cols)
  , data_(rows, cols)
{
  real offset;

  // determine elevation centers
  offset = 0.5_r * (1.0_r - rows);
  angle delta_v  = height / rows;
  for (size_t i = 0; i < rows; ++i)
    elevations_[i] = (offset + i) * delta_v;

  // determine azimuth centers
  offset = 0.5_r * (1.0_r - cols);
  angle delta_h  = width / cols;
  for (size_t i = 0; i < cols; ++i)
    azimuths_[i] = (offset + i) * delta_h;

  // calculate cross sectional power array for a single pointing location
  array2<real> csec{rows, cols};
  for (size_t y = 0; y < rows; ++y)
  {
    for (size_t x = 0; x < cols; ++x)
    {
      csec[y][x] = beam.calculate(azimuths_[x], elevations_[y]);
    }
  }

  // convolve the pattern over the gate sweep arc
  int start = std::lround(gate_width / (-2.0_r * delta_h));
  int end = std::lround(gate_width / (2.0_r * delta_h)) + 1;
  array_utils::fill(data_, 0.0_r);
  for (auto i = start; i < end; ++i)
  {
    const size_t sx = i < 0 ? -i : 0;
    const size_t ex = i > 0 ? cols - i : cols;

    for (size_t y = 0; y < rows; ++y)
    {
      for (size_t x = sx; x < ex; ++x)
      {
        data_[y][x + i] += csec[y][x];
      }
    }
  }

  // normalize the array
  double total = 0.0;
  for (size_t i = 0; i < data_.size(); ++i)
    total += data_.data()[i];
  array_utils::divide(data_, data_, total);
}

void beam_power_cross_section::make_vertical_integration()
{
  for (size_t y = 1; y < data_.rows(); ++y)
  {
    auto below = data_[y-1];
    auto above = data_[y];
    for (size_t x = 0; x < data_.cols(); ++x)
      above[x] += below[x];
  }
}

