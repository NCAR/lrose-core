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

#ifndef OPTICAL_FLOW_HH
#define OPTICAL_FLOW_HH

#include "array.hh"
#include <dataport/port_types.h>
#include <cmath>
#include <vector>

using namespace titan;

/// Optical flow field tracker used to generate advection fields
class optical_flow
{
public:
  /// construct a new optical flow tracker for the given domain size
  /**
   * \param[in] dim_x                 X dimension of lag1/lag0 fields
   * \param[in] dim_y                 Y dimension of lag1/lag0 fields
   * \param[in] scale                 Scale factor for tracking resolution levels
   * \param[in] levels                Maximum numer of tracking resolution levels
   * \param[in] window_size           Blur operation kernel size
   * \param[in] iterations            Number of blur iterations at each resolution
   * \param[in] polygon_neighbourhood 
   * \param[in] polygon_sigma
   */
  optical_flow(
          size_t dim_x
          , size_t dim_y
          , fl32 scale = 0.5
          , int levels = 100
          , int window_size = 5 // experiment with this!
          , int iterations = 1 // and this
          , int polygon_neighbourhood = 5 // or 7
          , double polygon_sigma = 1.1 // 1.5 if set to 7 above
               );

  /// copy construction
  optical_flow(const optical_flow& rhs) = default;

  /// move construction
  optical_flow(optical_flow&& rhs) noexcept;

  /// copy assignment
  auto operator=(const optical_flow& rhs) -> optical_flow& = default;

  /// move assignment
  auto operator=(optical_flow&& rhs) noexcept -> optical_flow&;

  /// destruction
  ~optical_flow() noexcept = default;

  /// Determine the advection velocity field between two fields
  /**
   * The tracking algorithm does not play nice with NaNs.  If either of the input fields
   * (lag1/lag0) contain NaNs, then a non-NaN value must be supplied in the background
   * parameter.  The algorithm will then treat any NaNs occuring in the input fields as
   * if they had the value background.
   * 
   * \param[in]     lag1              The previous time step field
   * \param[in]     lag0              The current time step field
   * \param[in,out] velocity_u        Velocity field output (x component)
   * \param[in,out] velocity_v        Velocity field output (y component)
   * \param[in]     use_initial_flow  If true, use existing values in velocity field to seed algorithm
   * \param[in]     background        Value to use for NaNs encountered in lag1 and lag0
   * \param[in]     threshold         Only perform tracking on parts of field greater than this
   * \param[in]     gain              Internal gain used to tweak apparant field depth (ie: bring into 0-N range)
   * \param[in]     fill_gaps         If true, interpolate velocity over untracked regions
   * \param[in]     spacing           Sample spacing for interpolation algorithm
   * \param[in]     min_frac_bins_for_avg Fraction of points in sample square required for valid interpolation
   * \param[in]     idw_low_res_pwr   Power term in inverse distance weighting interpolation low-res pass
   * \param[in]     idw_high_res_pwr  Power term in inverse distance weighting interpolation high-res pass
   */
  auto determine_velocities(
          const array2<fl32>& lag1
          , const array2<fl32>& lag0
          , array2<fl32>& velocity_u
          , array2<fl32>& velocity_v
          , bool use_initial_flow
          , fl32 background = NAN
          , fl32 threshold = NAN
          , fl32 gain = NAN
          , bool fill_gaps = false
          , size_t spacing = 8
          , fl32 min_frac_bins_for_avg = 0.05
          , double idw_low_res_pwr = 4.0
          , double idw_high_res_pwr = 3.0
                            ) const -> void;

private:
  // per level constants
  struct level_info
  {
    fl32    scale;
    fl32    sigma;
    int     ksize;
    size_t  dims[2];
  };
  typedef double vec5d[5];

private:
  auto track_fields(
          const array2<fl32>& lag1
          , const array2<fl32>& lag0
          , array2<fl32>& velocity_u
          , array2<fl32>& velocity_v
          , bool use_initial_flow
                    ) const -> void;
  auto poly_exp_setup() -> void;
  auto poly_exp(const array2<fl32>& src, array2<vec5d>& dst) const -> void;
  auto update_matrices(
          const array2<vec5d>& r0
          , const array2<vec5d>& r1
          , const array2<fl32>& flow_u
          , const array2<fl32>& flow_v
          , array2<vec5d>& mat
          , int y0
          , int y1) const -> void;
  auto blur_iteration(
          const array2<vec5d>& r0
          , const array2<vec5d>& r1
          , array2<fl32>& flow_u
          , array2<fl32>& flow_v
          , array2<vec5d>& mat
          , bool update_mats) const -> void;

  auto interpolate_gaps(
          array2<fl32>& velocity_u
          , array2<fl32>& velocity_v
          , const size_t spacing
          , const fl32 min_frac_bins_for_avg
          , const double idw_low_res_pwr
          , const double idw_high_res_pwr
                        ) const -> void;

  void gaussian_blur(
          array2<fl32>& output
          , const array2<fl32>& input
          , int kernel_size
          , fl32 sigma) const;
      
  void run_kernel_x(
          array2<fl32>& output
          , const array2<fl32>& input
          , const fl32 kernel[]
          , int kernel_size) const;

  void run_kernel_y(
          array2<fl32>& output
          , const array2<fl32>& input
          , const fl32 kernel[]
          , int kernel_size) const;

private:
  // flow model input parameters
  size_t  dims_[2];
  fl32    scale_;
  int     win_size_;
  int     iterations_;
  int     poly_n_;
  double  poly_sigma_;

  // precalculated terms
  std::vector<level_info> info_;    // used in main loop

  // constant junk used in poly_exp
  std::vector<fl32>       kbuf_;    
  double                  ig11_;
  double                  ig03_;
  double                  ig33_;
  double                  ig55_;

  // helper functions used in pixel_mask()
  inline fl32 get_pixel(const array2<fl32>& img, int i, int j, fl32 background)
  {
    return 
      (   i < 0 || (size_t) i >= img.rows() 
          || j < 0 || (size_t) j >= img.cols()
          || isnan(img[i][j]))
      ? background 
      : img[i][j];
  }
  
  inline fl32 replace_nan(fl32 val, fl32 background)
  {
    return isnan(val) ? background : val;
  }

  fl32 pixel_mask(
          const array2<fl32>& img
          , fl32 src_x
          , fl32 src_y
          , fl32 background);
    
  void advect_field(
          const array2<fl32>& flow_u
          , const array2<fl32>& flow_v
          , fl32 background
          , const array2<fl32>& lag1
          , array2<fl32>& lag0);
    
};

#endif
