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

#include "spheroid.h"

using namespace rainfields;
using namespace rainfields::ancilla;

constexpr const char* enum_traits<spheroid::standard>::strings[];

// the order of the spheroids specified here _must_ match common_sphereoid
// first value is major axis, second value is inverse flattening
static constexpr real std_sphere[][2] =
{
    { 6378165.000, 298.3         } // i65
  , { 6378160.000, 298.25        } // ans
  , { 6378293.645, 294.26        } // clark1858
  , { 6378137.000, 298.257222101 } // grs80
  , { 6378137.000, 298.257223563 } // wgs84
  , { 6378135.000, 298.26        } // wgs72
  , { 6378388.000, 297           } // international1924
  , { 6378160.000, 298.25        } // australian_national
};

spheroid::spheroid(standard cs)
  : a_(std_sphere[static_cast<int>(cs)][0])
  , inv_f_(std_sphere[static_cast<int>(cs)][1])
{
  derive_parameters();
}

spheroid::spheroid(real semi_major_axis, real inverse_flattening)
  : a_(semi_major_axis)
  , inv_f_(inverse_flattening)
{
  derive_parameters();
}

spheroid::spheroid(const xml::node& conf)
{
  auto i = conf.attributes().find("spheroid");
  if (i == conf.attributes().end())
  {
    a_ = conf("semi_major_axis");
    inv_f_ = conf("inverse_flattening");
  }
  else
  {
    standard cs = from_string<standard>(*i);
    a_ = std_sphere[static_cast<int>(cs)][0];
    inv_f_ = std_sphere[static_cast<int>(cs)][1];
  }
  derive_parameters();
}

auto spheroid::radius_physical(angle lat) const -> real
{
  real a_cos = a_ * cos(lat);
  real a2_cos = a_ * a_cos;
  real b_sin = b_ * sin(lat);
  real b2_sin = b_ * b_sin;
  return std::sqrt((a2_cos * a2_cos + b2_sin * b2_sin) / (a_cos * a_cos + b_sin * b_sin));
}

auto spheroid::radius_of_curvature_meridional(angle lat) const -> real
{
  real ab = a_ * b_;
  real a_cos = a_ * cos(lat);
  real b_sin = b_ * sin(lat);
  return (ab * ab) / std::pow(a_cos * a_cos + b_sin * b_sin, 1.5_r);
}

auto spheroid::radius_of_curvature_normal(angle lat) const -> real
{
  real a_cos = a_ * cos(lat);
  real b_sin = b_ * sin(lat);
  return (a_ * a_) / std::sqrt(a_cos * a_cos + b_sin * b_sin);
}

auto spheroid::radius_of_curvature(angle lat) const -> real
{
  real a_cos = a_ * cos(lat);
  real b_sin = b_ * sin(lat);
  return (a_ * a_ * b_) / (a_cos * a_cos + b_sin * b_sin);
}

auto spheroid::latlon_to_ecefxyz(latlonalt pos) const -> vec3<real>
{
  real sin_lat = sin(pos.lat);
  real cos_lat = cos(pos.lat);
  real nu = a_ / std::sqrt(1.0_r - e_sqr_ * sin_lat * sin_lat);
  real nu_plus_h = nu + pos.alt;

  return vec3<real>(
        nu_plus_h * cos_lat * cos(pos.lon)
      , nu_plus_h * cos_lat * sin(pos.lon)
      , ((1.0_r - e_sqr_) * nu + pos.alt) * sin_lat);
}

auto spheroid::ecefxyz_to_latlon(vec3<real> pos) const -> latlonalt
{
  real p = std::sqrt(pos.x * pos.x + pos.y * pos.y);
  angle theta = atan((pos.z * a_) / (p * b_));
  real sin3 = sin(theta); sin3 = sin3 * sin3 * sin3;
  real cos3 = cos(theta); cos3 = cos3 * cos3 * cos3;
  angle lat = atan((pos.z + ep2_ * b_ * sin3) / (p - e_sqr_ * a_ * cos3));
  real sinlat = sin(lat);

  return latlonalt(
        lat
      , atan2(pos.y, pos.x)
      , (p / cos(lat)) - (a_ / std::sqrt(1.0_r - e_sqr_ * sinlat * sinlat)));
}

auto spheroid::bearing_range_to_latlon(latlon pos, angle bearing, real range) const -> latlon
{
  // based on original open source script found at:
  // http://www.movable-type.co.uk/scripts/latlong-vincenty-direct.html
  // modified for use in ancilla

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  */
  /* Vincenty Direct Solution of Geodesics on the Ellipsoid (c) Chris Veness 2005-2012              */
  /*                                                                                                */
  /* from: Vincenty direct formula - T Vincenty, "Direct and Inverse Solutions of Geodesics on the  */
  /*       Ellipsoid with application of nested equations", Survey Review, vol XXII no 176, 1975    */
  /*       http://www.ngs.noaa.gov/PUBS_LIB/inverse.pdf                                             */
  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  */

  auto sin_alpha_1 = sin(bearing);
  auto cos_alpha_1 = cos(bearing);
  
  auto tanU1 = (1.0 - f_) * tan(pos.lat);
  auto cosU1 = 1.0 / std::sqrt((1.0 + tanU1 * tanU1));
  auto sinU1 = tanU1 * cosU1;

  auto sigma1 = atan2(tanU1, cos_alpha_1);
  auto sinAlpha = cosU1 * sin_alpha_1;
  auto cosSqAlpha = 1.0 - sinAlpha * sinAlpha;
  auto uSq = cosSqAlpha * ep2_;
  auto A = 1.0 + uSq / 16384.0 * (4096.0 + uSq * (-768.0 + uSq * (320.0 - 175.0 * uSq)));
  auto B = uSq / 1024.0 * (256.0 + uSq * (-128.0 + uSq * (74.0 - 47.0 * uSq)));
  
  auto sigma = (range / (b_ * A)) * 1_rad;
  auto sigmaP = constants<double>::two_pi * 1_rad;
  real cos2SigmaM, sinSigma, cosSigma;
  while (true)
  {
    cos2SigmaM = cos(2.0 * sigma1 + sigma);
    sinSigma = sin(sigma);
    cosSigma = cos(sigma);

    if ((sigma - sigmaP).abs() > 1e-12_rad)
      break;

    auto deltaSigma = 
      B * sinSigma * (cos2SigmaM + B / 4 * (cosSigma * (-1.0 + 2.0 * cos2SigmaM * cos2SigmaM)
      - B / 6.0 * cos2SigmaM * (-3.0 + 4.0 * sinSigma * sinSigma) 
      * (-3.0 + 4.0 * cos2SigmaM * cos2SigmaM)));
    sigmaP = sigma;
    sigma.set_radians(range / (b_ * A) + deltaSigma);
  }

  auto tmp = sinU1 * sinSigma - cosU1 * cosSigma * cos_alpha_1;
  auto lat2 = atan2(sinU1 * cosSigma + cosU1 * sinSigma * cos_alpha_1, (1-f_) * std::hypot(sinAlpha, tmp));
  auto lambda = atan2(sinSigma*sin_alpha_1, cosU1*cosSigma - sinU1*sinSigma*cos_alpha_1);
  auto C = f_/16*cosSqAlpha*(4+f_*(4-3*cosSqAlpha));
  auto L = lambda - (1-C) * f_ * sinAlpha *
      (sigma + 1_rad * C * sinSigma * (cos2SigmaM + C * cosSigma * (-1.0 + 2.0 * cos2SigmaM * cos2SigmaM)));
  auto lon2 = 1_rad * (std::fmod(pos.lon.radians() + L.radians() + (3 * constants<double>::pi), constants<double>::two_pi) - constants<double>::pi);  // normalise to -180...+180

  return { lat2, lon2 };
}

/* Vincenty inverse formula - T Vincenty, "Direct and Inverse Solutions of Geodesics on the
 * Ellipsoid with application of nested equations", Survey Review, vol XXII no 176, 1975
 * http://www.ngs.noaa.gov/PUBS_LIB/inverse.pdf
 *
 * Implementation based on formulas found from the "Vincenty's Formulae" Wikipedia article.
 */
auto spheroid::latlon_to_bearing_range(latlon pos1, latlon pos2) const -> std::pair<angle, real>
{
  // initialization
  const double u1 = std::atan((1.0 - f_) * std::tan((double) pos1.lat.radians()));
  const double u2 = std::atan((1.0 - f_) * std::tan((double) pos2.lat.radians()));
  const double l = pos2.lon.radians() - pos1.lon.radians();

  // dependent initialization
  const double cos_u1 = std::cos(u1);
  const double cos_u2 = std::cos(u2);
  const double sin_u1 = std::sin(u1);
  const double sin_u2 = std::sin(u2);
  const double cu1su2 = cos_u1 * sin_u2;
  const double cu2su1 = cos_u2 * sin_u1;
  const double su1su2 = sin_u1 * sin_u2;
  const double cu1cu2 = cos_u1 * cos_u2;

  // iteration until convergence of lambda
  double lambda = l;
  for (size_t iters = 0; iters < 100; ++iters)
  {
    double cos_lambda = std::cos(lambda);
    double sin_lambda = std::sin(lambda);
    double sin_sigma  = std::hypot(cos_u2 * sin_lambda, cu1su2 - cu2su1 * cos_lambda);

    // check for coincident points
    if (sin_sigma == 0.0)
      return { 0.0_rad, 0.0 };

    double cos_sigma  = su1su2 + cu1cu2 * cos_lambda;
    double sigma      = std::atan2(sin_sigma, cos_sigma);
    double sin_alpha  = (cu1cu2 * sin_lambda) / sin_sigma;
    double csq_alpha  = 1.0 - sin_alpha * sin_alpha;
    double cos_2sigm  = cos_sigma - ((2.0 * su1su2) / csq_alpha);

    // check for equatorial line
    if (is_nan(cos_2sigm))
      cos_2sigm = 0.0;

    double c          = (f_ / 16.0) * csq_alpha * (4.0 + f_ * (4.0 - 3.0 * csq_alpha));
    double lambda_new = l + (1.0 - c) * f_ * sin_alpha * (sigma + c * sin_sigma * 
                          (cos_2sigm + c * cos_sigma * (-1.0 + 2.0 * cos_2sigm)));

    // check for convergence to correct answer
    if (std::abs(lambda - lambda_new) < 1e-12)
    {
      double u_sqr  = csq_alpha * ep2_;
      double a      = 1.0 + (u_sqr / 16384.0) * (4096.0 + u_sqr * 
                        (-768.0 + u_sqr * (320.0 - 175.0 * u_sqr)));
      double b      = (u_sqr / 1024.0) * (256.0 + u_sqr * (-128.0 + u_sqr * (74 - 47.0 * u_sqr)));
      double dsigma = b * sin_sigma * (cos_2sigm + 0.25 * b * (cos_sigma * 
                        (-1.0 + 2.0 * cos_2sigm * cos_2sigm) - (1.0 / 6.0) * b * cos_2sigm * 
                        (-3.0 + 4.0 * sin_sigma * sin_sigma) * (-3.0 + 4.0 * cos_2sigm * cos_2sigm)));
      double s      = b_ * a * (sigma - dsigma);
      double az_fwd = std::atan2(
                          (cos_u2 * std::sin(lambda_new))
                        , (cu1su2 - cu2su1 * std::cos(lambda_new)));
      return { az_fwd * 1_rad, s };
    }
    lambda = lambda_new;
  }

  // failed to converge
  return { nan<angle>(), nan() };
}

auto spheroid::bearing_range_to_latlon_haversine(latlon pos, angle bearing, real range) const -> latlon
{
  auto ronrad = (range / radius_of_curvature(pos.lat)) * 1_rad;
  auto srad = sin(ronrad);
  auto crad = cos(ronrad);
  auto slat = sin(pos.lat);
  auto clat = cos(pos.lat);

  auto lat = asin(slat * crad + clat * srad * cos(bearing));
  auto lon = pos.lon + atan2(sin(bearing) * srad * clat, crad - slat * sin(lat));

  return { lat, lon };
}

auto spheroid::latlon_to_bearing_range_haversine(latlon pos1, latlon pos2) const -> std::pair<angle, real>
{
  auto radius = radius_of_curvature(pos1.lat);

  auto dlat = pos2.lat - pos1.lat;
  auto dlon = pos2.lon - pos1.lon;

  auto clat1 = cos(pos1.lat);
  auto clat2 = cos(pos2.lat);

  // distance
  auto sdlat2 = sin(dlat * 0.5_r);
  auto sdlon2 = sin(dlon * 0.5_r);
  auto a = sdlat2 * sdlat2 + sdlon2 * sdlon2 * clat1 * clat2;
  auto r = radius * (2.0_r * atan2(sqrt(a), sqrt(1.0_r - a)).radians());

  // bearing
  auto y = sin(dlon) * clat2;
  auto x = clat1 * sin(pos2.lat) - sin(pos1.lat) * clat2 * cos(dlon);
  auto b = atan2(y,x);
  
  return { b, r };
}

void spheroid::derive_parameters()
{
  f_      = 1.0_r / inv_f_;
  b_      = a_ * (1.0_r - f_);
  e_sqr_  = f_ * (2.0_r - f_);
  ep2_    = (a_ * a_ - b_ * b_) / (b_ * b_);
  e_      = std::sqrt(e_sqr_);
  avg_r_  = (2.0_r * a_ + b_) / 3.0_r;

  auth_r_ = e_sqr_ * e_sqr_ * e_sqr_ * 4.0_r / 7.0_r;
  auth_r_ += e_sqr_ * e_sqr_ * 3.0_r / 5.0_r;
  auth_r_ += 1.0_r;
  auth_r_ *= (1.0_r - e_sqr_);
  auth_r_ = a_ * std::sqrt(auth_r_);
}


