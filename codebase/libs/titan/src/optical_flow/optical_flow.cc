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

#include "optical_flow.hh"
#include "array_utils.hh"

#include <stdexcept>
#include <limits>
#include <cmath>
#include <iostream>
#include <alloca.h>
#include <stdexcept>

using namespace titan;

optical_flow::optical_flow(
      size_t dim_x
    , size_t dim_y
    , fl32 scale
    , int levels
    , int window_size
    , int iterations
    , int polygon_neighbourhood
    , double polygon_sigma)
  : dims_{dim_y, dim_x}
  , scale_(scale)
  , win_size_(window_size)
  , iterations_(iterations)
  , poly_n_(polygon_neighbourhood)
  , poly_sigma_(polygon_sigma)
{
  constexpr int min_size = 32;

  // limit the levels to a sensible value
  int min_side = std::min(dims_[0], dims_[1]);
  scale = 1.0;
  for (int i = 0; i < levels; ++i, scale *= scale_)
  {
    if (min_side * scale < min_size)
    {
      levels = i;
      break;
    }
  }

  // calculate the per level constants
  info_.resize(levels);
  scale = 1.0;
  for (int i = 0; i < levels; ++i, scale *= scale_)
  {
    // record the scale at this level
    info_[i].scale = scale;

    // determine kernel size and sigma for gaussian blur
    info_[i].sigma = (1.0 / scale - 1.0) * 0.5;
    info_[i].ksize = std::max(std::lround(info_[i].sigma * 5.0) | 1L, 3L);

    // determine width and height at this level
    info_[i].dims[0] = std::lround(dims_[0] * scale);
    info_[i].dims[1] = std::lround(dims_[1] * scale);

#if 0
    printf("lvl %d scale %f sigma %f ksize %d size %dx%d\n"
           , i, scale, info_[i].sigma, info_[i].ksize, info_[i].dims[1], info_[i].dims[0]);
#endif
  }

  // setup the constants for the polynomial expansion function
  poly_exp_setup();
}

optical_flow::optical_flow(optical_flow&& rhs) noexcept
  : dims_{rhs.dims_[0], rhs.dims_[1]}
  , scale_(rhs.scale_)
  , win_size_(rhs.win_size_)
  , iterations_(rhs.iterations_)
  , poly_n_(rhs.poly_n_)
  , poly_sigma_(rhs.poly_sigma_)
  , info_(std::move(rhs.info_))
  , kbuf_(std::move(rhs.kbuf_))
  , ig11_(rhs.ig11_)
  , ig03_(rhs.ig03_)
  , ig33_(rhs.ig33_)
  , ig55_(rhs.ig55_)
{

}

auto optical_flow::operator=(optical_flow&& rhs) noexcept -> optical_flow&
{
  dims_[0] = rhs.dims_[0];
  dims_[1] = rhs.dims_[1];
  scale_ = rhs.scale_;
  win_size_ = rhs.win_size_;
  iterations_ = rhs.iterations_;
  poly_n_ = rhs.poly_n_;
  poly_sigma_ = rhs.poly_sigma_;
  info_ = std::move(rhs.info_);
  kbuf_ = std::move(rhs.kbuf_);
  ig11_ = rhs.ig11_;
  ig03_ = rhs.ig03_;
  ig33_ = rhs.ig33_;
  ig55_ = rhs.ig55_;
  return *this;
}

/** this function uses the ... method */
auto optical_flow::determine_velocities(
      const array2<fl32>& lag1
    , const array2<fl32>& lag0
    , array2<fl32>& velocity_u
    , array2<fl32>& velocity_v
    , bool use_initial_flow
    , fl32 background
    , fl32 threshold
    , fl32 gain
    , bool fill_gaps
    , size_t spacing
    , fl32 min_frac_bins_for_avg
    , double idw_low_res_pwr
    , double idw_high_res_pwr
    ) const -> void
{
  auto& flow0_u = velocity_u;
  auto& flow0_v = velocity_v;

  // sanity checks
  if (   lag0.rows() != dims_[0] || lag0.cols() != dims_[1]
      || lag1.rows() != dims_[0] || lag1.cols() != dims_[1]
      || flow0_u.rows() != dims_[0] || flow0_u.cols() != dims_[1]
      || flow0_v.rows() != dims_[0] || flow0_v.cols() != dims_[1])
    throw std::runtime_error("determine velocities: field/flow size mismatch");

  array2<fl32> lag1_copy;
  array2<fl32> lag0_copy;

  // do we need to make copies of the inputs?
  if (!isnan(background))
  {
    lag1_copy = array2<fl32>{dims_};
    lag0_copy = array2<fl32>{dims_};

    array_utils::remove_nans(lag1_copy, lag1, background);
    array_utils::remove_nans(lag0_copy, lag0, background);

    // need to apply a signal threshold?
    if (!isnan(threshold))
    {
      array_utils::threshold_min(lag1_copy, lag1_copy, threshold, background);
      array_utils::threshold_min(lag0_copy, lag0_copy, threshold, background);
    }

    if (!isnan(gain))
    {
      array_utils::multiply(lag1_copy, lag1_copy, gain);
      array_utils::multiply(lag0_copy, lag0_copy, gain);
    }
  }

  // do the actual tracking
  // track backwards from lag0 to lag1, so that vectors are located according to the lag0
  // data.  this means we then have to negate all the vectors afterwards
  track_fields(
        lag0_copy.size() ? lag0_copy : lag0
      , lag1_copy.size() ? lag1_copy : lag1
      , velocity_u
      , velocity_v
      , use_initial_flow);

  array_utils::multiply(velocity_u, velocity_u, -1.0);
  array_utils::multiply(velocity_v, velocity_v, -1.0);

  // interpolate over nan() areas if needed
  if (fill_gaps)
  {
    const auto d0 = lag0.data();
          auto o0 = velocity_u.data();
    for (size_t i = 0; i < lag0_copy.size(); ++i)
      if (!(d0[i] > threshold))
        o0[i] = NAN;

    // now do the gap filling interpolation
    interpolate_gaps(
          velocity_u
        , velocity_v
        , spacing
        , min_frac_bins_for_avg
        , idw_low_res_pwr
        , idw_high_res_pwr);
  }
}

auto optical_flow::track_fields(
      const array2<fl32>& lag1
    , const array2<fl32>& lag0
    , array2<fl32>& velocity_u
    , array2<fl32>& velocity_v
    , bool use_initial_flow
    ) const -> void
{
  auto& flow0_u = velocity_u;
  auto& flow0_v = velocity_v;

  // field buffers
  array2<fl32> img0(dims_);
  array2<fl32> img1(dims_);

  // matrix buffers
  array2<vec5d> r0(dims_);
  array2<vec5d> r1(dims_);
  array2<vec5d> m(dims_);

  // temporary flow buffers
  array2<fl32> f0_u(info_[1].dims);
  array2<fl32> f0_v(info_[1].dims);
  array2<fl32> f1_u(info_[1].dims);
  array2<fl32> f1_v(info_[1].dims);

  // flow pointers
  auto flow_u = &f0_u;
  auto flow_v = &f0_v;
  auto prev_flow_u = &f1_u;
  auto prev_flow_v = &f1_v;

#if 0
  // TEMP TEMP - dump levels to a file!
  rainfields::cdf::forecast output("blur.nc", lag0.cols(), lag0.rows(), info_.size());
#endif

  // process from the coarsest level upwards
  bool first = true;
  for (size_t lvl = info_.size(); lvl-- != 0; )
  {
    const auto& info = info_[lvl];

#if 0
    printf("level %d size %dx%d\n", lvl, info.dims[1], info.dims[0]);
#endif

    // hack the size of wrapper objects needed for this level
    img1.hack_size(info.dims);
    r0.hack_size(info.dims);
    r1.hack_size(info.dims);
    m.hack_size(info.dims);

    // on the final iteration, write directly to our output vectors
    if (lvl > 0)
    {
      flow_u->hack_size(info.dims);
      flow_v->hack_size(info.dims);
    }
    else
    {
      flow_u = &flow0_u;
      flow_v = &flow0_v;
    }

    // fill our flow with the appropriate initial state
    if (first)
    {
      // top level - use the passed in flow as initial state?
      if (use_initial_flow)
      {
        // yes - resize down from the input flow
        array_utils::interpolate(*flow_u, flow0_u);
        array_utils::interpolate(*flow_v, flow0_v);

        // scale the vectors to match the level size
        array_utils::multiply(*flow_u, *flow_u, info.scale);
        array_utils::multiply(*flow_v, *flow_v, info.scale);
      }
      else
      {
        // no - use zero velocities
        array_utils::zero(*flow_u);
        array_utils::zero(*flow_v);
      }
      first = false;
    }
    else
    {
      // not the top level, resize up from previous level's data
      array_utils::interpolate(*flow_u, *prev_flow_u);
      array_utils::interpolate(*flow_v, *prev_flow_v);

      // scale the vectors to match the level size (ie: multiply by inverse scale)
      array_utils::multiply(*flow_u, *flow_u, 1.0 / scale_);
      array_utils::multiply(*flow_v, *flow_v, 1.0 / scale_);
    }
  
    // process lag1/lag0 for this level
    // BUG BUG - OpenCV still blurs with sigma 0 (which CV sees as a special value
    //           meaning automatically calculate sigma - 0.8 for our case) at level 0!
    if (lvl > 0)
    {
      array_utils::copy(img0, lag1);
      gaussian_blur(img0, img0, info.ksize, info.sigma);
      array_utils::interpolate(img1, img0);
      poly_exp(img1, r0);

      array_utils::copy(img0, lag0);
      gaussian_blur(img0, img0, info.ksize, info.sigma);
      array_utils::interpolate(img1, img0);
      poly_exp(img1, r1);
    }
    else
    {
      poly_exp(lag1, r0);
      poly_exp(lag0, r1);
    }

    // update the matricies
    update_matrices(r0, r1, *flow_u, *flow_v, m, 0, flow_u->rows());

    // perform our blur/update iterations
    for (int i = 0; i < iterations_; ++i) {
      blur_iteration(r0, r1, *flow_u, *flow_v, m, i < iterations_ - 1);
    }

    // swap the current and previous flow for the next level
    std::swap(flow_u, prev_flow_u);
    std::swap(flow_v, prev_flow_v);
  }
}

auto optical_flow::poly_exp_setup() -> void
{
  // setup constant part of polyexp calls
  kbuf_.resize(poly_n_ * 6 + 3);

  fl32* g = &kbuf_[poly_n_];
  fl32* xg = g + poly_n_ * 2 + 1;
  fl32* xxg = xg + poly_n_ * 2 + 1;

  if (poly_sigma_ < std::numeric_limits<float>::epsilon())
    poly_sigma_ = poly_n_ * 0.3;

  double s = 0.0;
  for (int x = -poly_n_; x <= poly_n_; ++x)
  {
    g[x] = std::exp(-x * x / (2.0 * poly_sigma_ * poly_sigma_));
    s += g[x];
  }

  s = 1.0 / s;
  for (int x = -poly_n_; x <= poly_n_; ++x)
  {
    g[x] = g[x] * s;
    xg[x] = x * g[x];
    xxg[x] = x * x * g[x];
  }

  double gmat[6][6];
  memset(&gmat, 0, 6 * 6 * sizeof(double));

  for(int y = -poly_n_; y <= poly_n_; y++ )
  {
    for(int x = -poly_n_; x <= poly_n_; x++ )
    {
      gmat[0][0] += g[y] * g[x];
      gmat[1][1] += g[y] * g[x] * x * x;
      gmat[3][3] += g[y] * g[x] * x * x * x * x;
      gmat[5][5] += g[y] * g[x] * x * x * y * y;
    }
  }

  //gmat[0][0] = 1.0;
  gmat[2][2] = gmat[0][3] = gmat[0][4] = gmat[3][0] = gmat[4][0] = gmat[1][1];
  gmat[4][4] = gmat[3][3];
  gmat[3][4] = gmat[4][3] = gmat[5][5];

  // invG:
  // [ x        e  e    ]
  // [    y             ]
  // [       y          ]
  // [ e        z       ]
  // [ e           z    ]
  // [                u ]
  double ginv[6][6];
  // colesky matrix inversion of gmat (destructive to gmat)
  {
    memset(&ginv, 0, 6 * 6 * sizeof(double));
    ginv[0][0] = ginv[1][1] = ginv[2][2] = ginv[3][3] = ginv[4][4] = ginv[5][5] = 1.0;

    double s;
    for (int i = 0; i < 6; ++i)
    {
      for (int j = 0; j < i; ++j)
      {
        s = gmat[i][j];
        for (int k = 0; k < j; ++k)
          s -= gmat[i][k] * gmat[j][k];
        gmat[i][j] = s * gmat[j][j];
      }
      s = gmat[i][i];
      for (int k = 0; k < i; ++k)
      {
        double t = gmat[i][k];
        s -= t * t;
      }
      if (s < std::numeric_limits<double>::epsilon())
        throw std::runtime_error("optical_flow: unable to invert 'G' matrix");
      gmat[i][i] = 1.0 / std::sqrt(s);
    }

    for (int i = 0; i < 6; ++i)
    {
      for (int j = 0; j < 6; ++j)
      {
        s = ginv[i][j];
        for (int k = 0; k < i; ++k)
          s -= gmat[i][k] * ginv[k][j];
        ginv[i][j] = s * gmat[i][i];
      }
    }

    for (int i = 6; i-- != 0; )
    {
      for (int j = 0; j < 6; ++j)
      {
        s = ginv[i][j];
        for (int k = 6; k-- > i; )
          s -= gmat[k][i] * ginv[k][j];
        ginv[i][j] = s * gmat[i][i];
      }
    }
  }

  // store the values we care about
  ig11_ = ginv[1][1];
  ig03_ = ginv[0][3];
  ig33_ = ginv[3][3];
  ig55_ = ginv[5][5];
}

auto optical_flow::poly_exp(const array2<fl32>& src, array2<vec5d>& dst) const -> void
{
  // allocate some buffers
  // TODO - can this be allocated outside here to reduce repeat allocs?
  std::unique_ptr<fl32[]> rbuf(new fl32[(src.cols() + poly_n_ * 2) * 3]);

  // get the pointers into our buffers
  // don't store these in the class, because it would mean writing our own copy constructor
  const fl32* g = &kbuf_[poly_n_];
  const fl32* xg = g + poly_n_ * 2 + 1;
  const fl32* xxg = xg + poly_n_ * 2 + 1;
        fl32* row = &rbuf[poly_n_ * 3];

  const int rows = src.rows();
  const int cols = src.cols();

  for (int y = 0; y < rows; ++y)
  {
    fl32 g0 = g[0], g1, g2;
    const fl32* srow0 = src[y];
    const fl32* srow1 = NULL;
    vec5d* drow = dst[y];

    // vertical part of convolution
    for (int x = 0; x < cols; ++x)
    {
      row[x*3] = srow0[x] * g0;
      row[x*3+1] = row[x*3+2] = 0.0;
    }

    for (int k = 1; k <= poly_n_; ++k)
    {
      g0 = g[k]; g1 = xg[k]; g2 = xxg[k];
      srow0 = src[std::max(y - k, 0)];
      srow1 = src[std::min(y + k, rows - 1)];

      for (int x = 0; x < cols; ++x)
      {
        fl32 p = srow0[x] + srow1[x];
        fl32 t0 = row[x*3] + g0 * p;
        fl32 t1 = row[x*3+1] + g1 * (srow1[x] - srow0[x]);
        fl32 t2 = row[x*3+2] + g2 * p;

        row[x*3] = t0;
        row[x*3+1] = t1;
        row[x*3+2] = t2;
      }
    }

    // horizontal part of convolution
    for (int x = 0; x < poly_n_ * 3; ++x)
    {
      row[-1-x] = row[2-x];
      row[cols*3+x] = row[cols*3+x-3];
    }

    for (int x = 0; x < cols; ++x)
    {
      g0 = g[0];
      // r1 ~ 1, r2 ~ x, r3 ~ y, r4 ~ x^2, r5 ~ y^2, r6 ~ xy
      double b1 = row[x*3] * g0, b2 = 0, b3 = row[x*3+1] * g0,
             b4 = 0, b5 = row[x*3+2] * g0, b6 = 0;

      for (int k = 1; k <= poly_n_; ++k)
      {
        double tg = row[(x+k)*3] + row[(x-k)*3];
        g0 = g[k];
        b1 += tg*g0;
        b4 += tg*xxg[k];
        b2 += (row[(x+k)*3] - row[(x-k)*3]) * xg[k];
        b3 += (row[(x+k)*3+1] + row[(x-k)*3+1]) * g0;
        b6 += (row[(x+k)*3+1] - row[(x-k)*3+1]) * xg[k];
        b5 += (row[(x+k)*3+2] + row[(x-k)*3+2]) * g0;
      }

      // do not store r1
      drow[x][1] = b2 * ig11_;
      drow[x][0] = b3 * ig11_;
      drow[x][3] = b1 * ig03_ + b4 * ig33_;
      drow[x][2] = b1 * ig03_ + b5 * ig33_;
      drow[x][4] = b6 * ig55_;
    }
  }

  // TODO - why is this here?!?! from OpenCV implementation
  // all this does is realign row with rbuf, but it's never used...
  row -= poly_n_*3;
}

auto optical_flow::update_matrices(
      const array2<vec5d>& r0
    , const array2<vec5d>& r1
    , const array2<fl32>& flow_u
    , const array2<fl32>& flow_v
    , array2<vec5d>& mat
    , int row_from
    , int row_to) const -> void
{
  const int bsize = 5;
  static const fl32 border[bsize] = {0.14, 0.14, 0.4472, 0.4472, 0.4472};

  const int frows = flow_u.rows();
  const int fcols = flow_u.cols();

  for (int y = row_from; y < row_to; ++y)
  {
    auto flow_u_y = flow_u[y];
    auto flow_v_y = flow_v[y];
    auto r0_y = r0[y];
    auto mat_y = mat[y];

    for (int x = 0; x < fcols; ++x)
    {
      const vec5d& r0_yx = r0_y[x];

      fl32 dx = flow_u_y[x], dy = flow_v_y[x];
      fl32 fx = x + dx, fy = y + dy;

      int x1 = std::floor(fx), y1 = std::floor(fy);
      fl32 r2, r3, r4, r5, r6;

      fx -= x1; fy -= y1;

      if(   (unsigned) x1 < (unsigned) (fcols - 1) 
         && (unsigned) y1 < (unsigned) (frows - 1))
      {
        fl32 a00 = (1.0 - fx) * (1.0 - fy), a01 = fx * (1.0 - fy),
             a10 = (1.0 - fx) * fy, a11 = fx * fy;

        const vec5d& r1_00 = r1[y1][x1];
        const vec5d& r1_01 = r1[y1][x1+1];
        const vec5d& r1_10 = r1[y1+1][x1];
        const vec5d& r1_11 = r1[y1+1][x1+1];

        r2 = a00 * r1_00[0] + a01 * r1_01[0] + a10 * r1_10[0] + a11 * r1_11[0];
        r3 = a00 * r1_00[1] + a01 * r1_01[1] + a10 * r1_10[1] + a11 * r1_11[1];
        r4 = a00 * r1_00[2] + a01 * r1_01[2] + a10 * r1_10[2] + a11 * r1_11[2];
        r5 = a00 * r1_00[3] + a01 * r1_01[3] + a10 * r1_10[3] + a11 * r1_11[3];
        r6 = a00 * r1_00[4] + a01 * r1_01[4] + a10 * r1_10[4] + a11 * r1_11[4];

        r4 = (r0_yx[2] + r4) * 0.5;
        r5 = (r0_yx[3] + r5) * 0.5;
        r6 = (r0_yx[4] + r6) * 0.25;
      }
      else
      {
        r2 = r3 = 0.0;
        r4 = r0_yx[2];
        r5 = r0_yx[3];
        r6 = r0_yx[4] * 0.5;
      }

      r2 = (r0_yx[0] - r2) * 0.5;
      r3 = (r0_yx[1] - r3) * 0.5;

      r2 += r4 * dy + r6 * dx;
      r3 += r6 * dy + r5 * dx;

      if(   (unsigned) (x - bsize) >= (unsigned) (fcols - bsize * 2)
         || (unsigned) (y - bsize) >= (unsigned) (frows - bsize * 2))
      {
        fl32 scale = 
            (x < bsize ? border[x] : 1.0) 
          * (x >= fcols - bsize ? border[fcols - x - 1] : 1.0)
          * (y < bsize ? border[y] : 1.0)
          * (y >= frows - bsize ? border[frows - y - 1] : 1.0);

        r2 *= scale; r3 *= scale; r4 *= scale;
        r5 *= scale; r6 *= scale;
      }

      vec5d& mat_yx = mat_y[x];
      mat_yx[0] = r4*r4 + r6*r6; // G(1,1)
      mat_yx[1] = (r4 + r5)*r6;  // G(1,2)=G(2,1)
      mat_yx[2] = r5*r5 + r6*r6; // G(2,2)
      mat_yx[3] = r4*r2 + r6*r3; // h(1)
      mat_yx[4] = r6*r2 + r5*r3; // h(2)
    }
  }
}

auto optical_flow::blur_iteration(
      const array2<vec5d>& r0
    , const array2<vec5d>& r1
    , array2<fl32>& flow_u
    , array2<fl32>& flow_v
    , array2<vec5d>& mat
    , bool update_mats) const -> void
{
  const int frows = flow_u.rows();
  const int fcols = flow_u.cols();

  int m = win_size_ / 2;
  int min_update_stripe = std::max((1 << 10) / fcols, win_size_);
  double scale = 1.0 / (win_size_ * win_size_);

  // allocate a buffer
  // TODO - move this outside and unify with poly_exp's temp buffer since
  //        we only need one of them at a time
  std::unique_ptr<vec5d[]> vbuf(new vec5d[fcols + m * 2 + 2]);
  vec5d* vsum = &vbuf[m+1];

  // init vsum
  const vec5d* srow0 = mat[0];
  for (int x = 0; x < fcols; ++x)
  {
    vsum[x][0] = srow0[x][0] * (m + 2);
    vsum[x][1] = srow0[x][1] * (m + 2);
    vsum[x][2] = srow0[x][2] * (m + 2);
    vsum[x][3] = srow0[x][3] * (m + 2);
    vsum[x][4] = srow0[x][4] * (m + 2);
  }

  for (int y = 1; y < m; ++y)
  {
    srow0 = mat[std::min(y, frows - 1)];
    for (int x = 0; x < fcols; ++x)
    {
      vsum[x][0] += srow0[x][0];
      vsum[x][1] += srow0[x][1];
      vsum[x][2] += srow0[x][2];
      vsum[x][3] += srow0[x][3];
      vsum[x][4] += srow0[x][4];
    }
  }

  // compute blur(G)*flow=blur(h)
  for (int y = 0, y0 = 0; y < frows; ++y)
  {
    auto flow_u_y = flow_u[y];
    auto flow_v_y = flow_v[y];

                 srow0 = mat[std::max(y - m - 1, 0)];
    const vec5d* srow1 = mat[std::min(y + m, frows - 1)];

    // vertical blur
    for (int x = 0; x < fcols; ++x)
    {
      vsum[x][0] += srow1[x][0] - srow0[x][0];
      vsum[x][1] += srow1[x][1] - srow0[x][1];
      vsum[x][2] += srow1[x][2] - srow0[x][2];
      vsum[x][3] += srow1[x][3] - srow0[x][3];
      vsum[x][4] += srow1[x][4] - srow0[x][4];
    }

    // update borders
    for (int x = 0; x < m + 1; ++x)
    {
      // left border
      vsum[-x-1][4] = vsum[-x][4];
      vsum[-x-1][3] = vsum[-x][3];
      vsum[-x-1][2] = vsum[-x][2];
      vsum[-x-1][1] = vsum[-x][1];
      vsum[-x-1][0] = vsum[-x][0];

      // right border
      vsum[fcols+x][0] = vsum[fcols+x-1][0];
      vsum[fcols+x][1] = vsum[fcols+x-1][1];
      vsum[fcols+x][2] = vsum[fcols+x-1][2];
      vsum[fcols+x][3] = vsum[fcols+x-1][3];
      vsum[fcols+x][4] = vsum[fcols+x-1][4];
    }

    // init g** and h*
    double g11 = vsum[0][0] * (m + 2);
    double g12 = vsum[0][1] * (m + 2);
    double g22 = vsum[0][2] * (m + 2);
    double h1  = vsum[0][3] * (m + 2);
    double h2  = vsum[0][4] * (m + 2);

    for (int x = 1; x < m; ++x)
    {
      g11 += vsum[x][0];
      g12 += vsum[x][1];
      g22 += vsum[x][2];
      h1  += vsum[x][3];
      h2  += vsum[x][4];
    }

    // horizontal blur
    for (int x = 0; x < fcols; ++x)
    {
      g11 += vsum[x+m][0] - vsum[x-m-1][0];
      g12 += vsum[x+m][1] - vsum[x-m-1][1];
      g22 += vsum[x+m][2] - vsum[x-m-1][2];
      h1  += vsum[x+m][3] - vsum[x-m-1][3];
      h2  += vsum[x+m][4] - vsum[x-m-1][4];

      double g11_ = g11 * scale;
      double g12_ = g12 * scale;
      double g22_ = g22 * scale;
      double h1_  = h1 * scale;
      double h2_  = h2 * scale;

      double idet = 1.0 / (g11_ * g22_ - g12_ * g12_ + 1e-3);

      flow_u_y[x] = (g11_ *h2_ - g12_ * h1_) * idet;
      flow_v_y[x] = (g22_ *h1_ - g12_ * h2_) * idet;
    }

    int y1 = y == frows - 1 ? frows : y - win_size_;
    if (update_mats && (y1 == frows || y1 >= y0 + min_update_stripe))
    {
      update_matrices(r0, r1, flow_u, flow_v, mat, y0, y1);
      y0 = y1;
    }
  }
}

auto optical_flow::interpolate_gaps(
      array2<fl32>& velocity_u
    , array2<fl32>& velocity_v
    , const size_t spacing
    , const fl32 min_frac_bins_for_avg
    , const double idw_low_res_pwr
    , const double idw_high_res_pwr
    ) const -> void
{
  const int min_bins_for_avg = (spacing * spacing) * min_frac_bins_for_avg;
  const size_t half_spacing = spacing / 2;

  // stage 1 - build the low resolution grid based on cell averages
  size_t low_size_y = (velocity_u.rows() + spacing - 1) / spacing; // round up for partial cells
  size_t low_size_x = (velocity_u.cols() + spacing - 1) / spacing;
  array2<fl32> grid_u(low_size_y, low_size_x);
  array2<fl32> grid_v(low_size_y, low_size_x);
  for (size_t y = 0; y < grid_u.rows(); ++y)
  {
    for (size_t x = 0; x < grid_u.cols(); ++x)
    {
      int count = 0;
      fl32 sum_u = 0.0, sum_v = 0.0;
      size_t ymax = std::min((y + 1) * spacing, velocity_u.rows());
      size_t xmax = std::min((x + 1) * spacing, velocity_u.cols());
      for (size_t yy = y * spacing; yy < ymax; ++yy)
      {
        for (size_t xx = x * spacing; xx < xmax; ++xx)
        {
          auto val_u = velocity_u[yy][xx];
          auto val_v = velocity_v[yy][xx];
          if (!isnan(val_u))
          {
            ++count;
            sum_u += val_u;
            sum_v += val_v;
          }
        }
      }
      if (count < min_bins_for_avg)
      {
        grid_u[y][x] = NAN;
        grid_v[y][x] = NAN;
      }
      else
      {
        grid_u[y][x] = sum_u / count;
        grid_v[y][x] = sum_v / count;
      }
    }
  }

  // stage 2 - fill in missing low resolution cells using inverse distance weighting
  auto pwr = idw_low_res_pwr * 0.5; // to avoid std::pow(std::sqrt(...))
  size_t max_x = ((grid_u.cols() - 1) * spacing + half_spacing < velocity_u.cols() ? grid_u.cols() : grid_u.cols() - 1);
  size_t max_y = ((grid_u.rows() - 1) * spacing + half_spacing < velocity_u.rows() ? grid_u.rows() : grid_u.rows() - 1);
  for (size_t y = 0; y < max_y; ++y)
  {
    size_t yo = y * spacing + half_spacing;
    for (size_t x = 0; x < max_x; ++x)
    {
      auto& out_u = velocity_u[yo][x * spacing + half_spacing];
      auto& out_v = velocity_v[yo][x * spacing + half_spacing];

      if (isnan(out_u))
      {
        double ac_weight = 0.0;
        double ac_val_u = 0.0;
        double ac_val_v = 0.0;

        // search in rings around the point until we get at least 8 neighbours
        // TODO - is this fl32ly doing what it says!?!?!! this really IDVs the whole low res grid!
        for (size_t yy = 0; yy < grid_u.rows(); ++yy)
        {
          for (size_t xx = 0; xx < grid_u.cols(); ++xx)
          {
            auto vv_u = grid_u[yy][xx];
            auto vv_v = grid_v[yy][xx];
            if (!isnan(vv_u))
            {
              double weight = 1.0 / std::pow((double(xx) - x) * (double(xx) - x) + (double(yy) - y) * (double(yy) - y), pwr);
              ac_val_u += vv_u * weight;
              ac_val_v += vv_v * weight;
              ac_weight += weight;
            }
          }
        }
        if (ac_weight > 0.0)
        {
          out_u = ac_val_u / ac_weight;
          out_v = ac_val_v / ac_weight;
        }
        else
        {
          out_u = 0.0;
          out_v = 0.0;
        }
      }
    }
  }

  // stage 4 - perform the high resolution interpolation
  pwr = idw_high_res_pwr * 0.5; // to avoid std::pow(std::sqrt(...))
  array2<fl32> ref_u(velocity_u);
  array2<fl32> ref_v(velocity_v);
  for (size_t y = 0; y < velocity_u.rows(); ++y)
  {
    for (size_t x = 0; x < velocity_u.cols(); ++x)
    {
      if (!isnan(ref_u[y][x]))
        continue;

      double ac_weight = 0.0;
      double ac_val_u = 0.0;
      double ac_val_v = 0.0;
      size_t min_x = std::max(x, spacing) - spacing;
      size_t min_y = std::max(y, spacing) - spacing;
      size_t max_x = std::min(x + spacing, velocity_u.cols());
      size_t max_y = std::min(y + spacing, velocity_u.rows());
      for (auto yy = min_y; yy < max_y; ++yy)
      {
        for (auto xx = min_x; xx < max_x; ++xx)
        {
          const auto& vv_u = ref_u[yy][xx];
          const auto& vv_v = ref_v[yy][xx];
          if (!isnan(vv_u))
          {
            double weight = 1.0 / std::pow((double(xx) - x) * (double(xx) - x) + (double(yy) - y) * (double(yy) - y), pwr);
            ac_val_u += vv_u * weight;
            ac_val_v += vv_v * weight;
            ac_weight += weight;
          }
        }
      }
      velocity_u[y][x] = ac_val_u / ac_weight;
      velocity_v[y][x] = ac_val_v / ac_weight;
    }
  }
}

//////////////////////////////////
// gaussian blur filter

void optical_flow::gaussian_blur(
        array2<fl32>& output
        , const array2<fl32>& input
        , int kernel_size
        , fl32 sigma) const
{
  // sanity checks
  if (output.size() != input.size())
    throw std::logic_error("array size mismatch");

  // auto calculate the kernel size from sigma or vice versa (if desired)
  if (kernel_size == 0 && sigma > 0.0)
    kernel_size = static_cast<int>(round(sigma * 4 * 2 + 1)) | 1;
  else if (sigma <= 0.0 && kernel_size > 0)
    sigma = ((kernel_size - 1) * 0.5 - 1.0) * 0.3 + 0.8;

  // sanity checks
  if (kernel_size < 0 || kernel_size % 2 != 1)
    throw std::invalid_argument("gaussian_blur: invalid kernel size (must be 0 or odd)");

  // build the kernel
  fl32* kernel = static_cast<fl32*>(alloca(kernel_size * sizeof(fl32)));
  {
    fl32 scale_2x = -0.5 / (sigma * sigma);
    fl32 sum = 0.0;
    for (int i = 0; i < kernel_size; ++i)
    {
      fl32 x = i - (kernel_size - 1) * 0.5;
      kernel[i] = std::exp(scale_2x * x * x);
      sum += kernel[i];
    }
    sum = 1.0 / sum;
    for (int i = 0; i < kernel_size; ++i)
      kernel[i] *= sum;
  }

  // as our kernel is symetrical, we can do 2 passes of the 1D kernel
  run_kernel_x(output, input, kernel, kernel_size);
  run_kernel_y(output, output, kernel, kernel_size);
}

void optical_flow::run_kernel_x(
        array2<fl32>& output
        , const array2<fl32>& input
        , const fl32 kernel[]
        , int kernel_size) const
{
  // get signed versions of our dims (eliminates many casts)
  const int dims_x = output.cols();
  const int dims_y = output.rows();

  // note: to enable safe in-place operation (ie: src == dst), a buffer is used that delays
  //       output of calculated values back into the image until the kernel has fully passed
  //       over the value in question.  Output is delayed by 'side' iterations.
  // note: the use of negative array indexes in this function is intentional (not a bug)
  if (dims_x < kernel_size)
    throw std::runtime_error("filter field smaller than kernel - unimplemented feature");

  auto inp = input.data();
  auto out = output.data();
  auto buf = static_cast<fl32*>(alloca(kernel_size * sizeof(fl32)));

  int side = kernel_size / 2;
  for (int y = 0; y < dims_y; ++y)
  {
    // left border (replicate left most pixel)
    for (int x = 0; x < side; ++x)
    {
      fl32 val = 0.0;
      for (int i = 0; i < side - x; ++i)
        val += kernel[i] * inp[-x];
      for (int i = side - x; i < kernel_size; ++i)
        val += kernel[i] * inp[i - side];
      ++inp;

      // just fill our buffer
      buf[x % side] = val;
    }

    // middle section of row
    for (int x = side; x < dims_x - side; ++x)
    {
      fl32 val = 0.0;
      for (int i = 0; i < kernel_size; ++i)
        val += kernel[i] * inp[i - side];
      ++inp;

      // output buffered value, and fill more space in it
      *out = buf[x % side];
      buf[x % side] = val;
      ++out;
    }

    // right border (replicate right most pixel)
    for (int x = dims_x - side; x < dims_x; ++x)
    {
      fl32 val = 0.0;
      for (int i = 0; i < kernel_size - ((side + 1) - (dims_x - x)); ++i)
        val += kernel[i] * inp[i - side];
      for (int i = kernel_size - ((side + 1) - (dims_x - x)); i < kernel_size; ++i)
        val += kernel[i] * inp[dims_x - x - 1];
      ++inp;

      // output buffered value and fill more space
      *out = buf[x % side];
      buf[x % side] = val;
      ++out;
    }

    // flush remaining buffered values
    for (int x = dims_x; x < dims_x + side; ++x)
    {
      *out = buf[x % side];
      ++out;
    }
  }
}

void optical_flow::run_kernel_y(
        array2<fl32>& output
        , const array2<fl32>& input
        , const fl32 kernel[]
        , int kernel_size) const
{
  // get signed versions of our dims (eliminates many casts)
  const int dims_x = output.cols();
  const int dims_y = output.rows();

  // note: to enable safe in-place operation (ie: src == dst), a buffer is used that delays
  //       output of calculated values back into the image until the kernel has fully passed
  //       over the value in question.  Output is delayed by 'side' iterations.
  // note: the use of negative array indexes in this function is intentional (not a bug)
  if (dims_y < kernel_size)
    throw std::runtime_error("filter field smaller than kernel - unimplemented feature");

  int side = kernel_size / 2;
  for (int x = 0; x < dims_x; ++x)
  {
    auto inp = &input.data()[x];
    auto out = &output.data()[x];
    auto buf = static_cast<fl32*>(alloca(kernel_size * sizeof(fl32)));

    // top border (replicate top most pixel)
    for (int y = 0; y < side; ++y)
    {
      fl32 val = 0.0;
      for (int i = 0; i < side - y; ++i)
        val += kernel[i] * inp[-y * dims_x];
      for (int i = side - y; i < kernel_size; ++i)
        val += kernel[i] * inp[(i - side) * dims_x];
      inp += dims_x;

      // just fill our buffer
      buf[y % side] = val;
    }

    // middle section of column
    for (int y = side; y < dims_y - side; ++y)
    {
      fl32 val = 0.0;
      for (int i = 0; i < kernel_size; ++i)
        val += kernel[i] * inp[(i - side) * dims_x];
      inp += dims_x;

      // output a buffered value and record the currently calculated one
      *out = buf[y % side];
      buf[y % side] = val;
      out += dims_x;
    }

    // bottom border (replicate bottom most pixel)
    for (int y = dims_y - side; y < dims_y; ++y)
    {
      fl32 val = 0.0;
      for (int i = 0; i < kernel_size - ((side + 1) - (dims_y - y)); ++i)
        val += kernel[i] * inp[(i - side) * dims_x];
      for (int i = kernel_size - ((side + 1) - (dims_y - y)); i < kernel_size; ++i)
        val += kernel[i] * inp[(dims_y - y - 1) * dims_x];
      inp += dims_x;

      // output a buffered value and record current calculation
      *out = buf[y % side];
      buf[y % side] = val;
      out += dims_x;
    }

    // flush remaining buffered values
    for (int y = dims_y; y < dims_y + side; ++y)
    {
      *out = buf[y % side];
      out += dims_x;
    }
  }
}

fl32 optical_flow::pixel_mask(
        const array2<fl32>& img
        , fl32 src_x
        , fl32 src_y
        , fl32 background)
{
  // integer coordinates of the nw source bin
  // note: must use floor (not cast) to cope with negative indexes
  int src_i = std::floor(src_y);
  int src_j = std::floor(src_x);
  
  // determine fractional contributions of each pixel under the source mask
  fl32 frac_i = 1.0 + src_i - src_y;
  fl32 frac_j = 1.0 + src_j - src_x;
  
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
        +     frac_i       * (1.0 - frac_j) * get_pixel(img, src_i, src_j + 1, background)
        + (1.0 - frac_i) *     frac_j       * get_pixel(img, src_i + 1, src_j, background)
        + (1.0 - frac_i) * (1.0 - frac_j) * get_pixel(img, src_i + 1, src_j + 1, background);
    }
  }
  else
  {
    // normal case - use all 4 pixels
    return
      frac_i       *     frac_j       * replace_nan(img[src_i][src_j], background)
      +     frac_i       * (1.0 - frac_j) * replace_nan(img[src_i][src_j + 1], background)
      + (1.0 - frac_i) *     frac_j       * replace_nan(img[src_i + 1][src_j], background)
      + (1.0 - frac_i) * (1.0 - frac_j) * replace_nan(img[src_i + 1][src_j + 1], background);
  }
}

void optical_flow::advect_field(
        const array2<fl32>& flow_u
        , const array2<fl32>& flow_v
        , fl32 background
        , const array2<fl32>& lag1
        , array2<fl32>& lag0)
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
