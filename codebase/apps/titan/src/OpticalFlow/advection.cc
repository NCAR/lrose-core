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

#include "advection.h"
#include <stdexcept>

using namespace ancilla;

namespace {
  // helper functions used in pixel_mask()
  inline real get_pixel(const array2<real>& img, int i, int j, real background)
  {
    return 
        (   i < 0 || (size_t) i >= img.rows() 
         || j < 0 || (size_t) j >= img.cols()
         || is_nan(img[i][j]))
      ? background 
      : img[i][j];
  }

  inline real replace_nan(real val, real background)
  {
    return is_nan(val) ? background : val;
  }

  real pixel_mask(
        const array2<real>& img
      , real src_x
      , real src_y
      , real background)
  {
    // integer coordinates of the nw source bin
    // note: must use floor (not cast) to cope with negative indexes
    int src_i = std::floor(src_y);
    int src_j = std::floor(src_x);

    // determine fractional contributions of each pixel under the source mask
    real frac_i = 1.0_r + src_i - src_y;
    real frac_j = 1.0_r + src_j - src_x;

    // if any of the source pixel mask is outside domain do the slow case
    if (   src_i < 0 || src_i >= (int) img.rows() - 1
        || src_j < 0 || src_j >= (int) img.cols() - 1)
    {
      // if _all_ of the source pixels are outside, just set to border value
      if (   src_i < -1 || src_i >= (int) img.rows()
          || src_j < -1 || src_j >= (int) img.cols())
      {
        return background;
      }
      else
      {
        // okay, at least one good pixel.  use it
        return
                frac_i       *     frac_j       * get_pixel(img, src_i, src_j, background)
          +     frac_i       * (1.0_r - frac_j) * get_pixel(img, src_i, src_j + 1, background)
          + (1.0_r - frac_i) *     frac_j       * get_pixel(img, src_i + 1, src_j, background)
          + (1.0_r - frac_i) * (1.0_r - frac_j) * get_pixel(img, src_i + 1, src_j + 1, background);
      }
    }
    else
    {
      // normal case - use all 4 pixels
      return
              frac_i       *     frac_j       * replace_nan(img[src_i][src_j], background)
        +     frac_i       * (1.0_r - frac_j) * replace_nan(img[src_i][src_j + 1], background)
        + (1.0_r - frac_i) *     frac_j       * replace_nan(img[src_i + 1][src_j], background)
        + (1.0_r - frac_i) * (1.0_r - frac_j) * replace_nan(img[src_i + 1][src_j + 1], background);
    }
  }
}

void ancilla::advection::advect_field(
      const array2<real>& flow_u
    , const array2<real>& flow_v
    , real background
    , const array2<real>& lag1
    , array2<real>& lag0)
{
  // sanity checks
  if (&lag1 == &lag0)
    throw std::invalid_argument("advect_field inplace advection not supported");
  if (   flow_v.rows() != flow_u.rows() || flow_v.cols() != flow_u.cols()
      || lag0.rows() != flow_u.rows() || lag0.cols() != flow_u.cols()
      || lag1.rows() != flow_u.rows() || lag1.cols() != flow_u.cols())
    throw std::invalid_argument("advect_field velocity/field size mismatch");

  // advect every pixel in the output image
  for (size_t y = 0; y < flow_u.rows(); ++y)
  {
    const auto vu = flow_u[y];
    const auto vv = flow_u[y];
    auto odat = lag0[y];

    for (size_t x = 0; x < flow_u.cols(); ++x)
    {
      odat[x] = pixel_mask(lag1, x - vu[x], y - vv[x], background);
    }
  }
}

