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

#include "beam_propagation.h"

#include "angle.h"

using namespace rainfields;
using namespace rainfields::ancilla;

beam_propagation::beam_propagation(
      angle elevation_angle
    , real site_altitude
    , real earth_radius
    , real effective_multiplier)
  : elevation_angle_(elevation_angle)
  , site_altitude_(site_altitude)
  , earth_radius_(earth_radius)
  , multiplier_(effective_multiplier)
  , effective_radius_(multiplier_ * earth_radius_)
  , cos_elev_(std::cos(elevation_angle_.radians()))
  , sin_elev_(std::sin(elevation_angle_.radians()))
  , eer_alt_(effective_radius_ + site_altitude_)
{

}

auto beam_propagation::set_elevation_angle(angle val) -> void
{
  elevation_angle_ = val;
  cos_elev_ = cos(elevation_angle_);
  sin_elev_ = sin(elevation_angle_);
}

auto beam_propagation::set_site_altitude(real val) -> void
{
  site_altitude_ = val;
  eer_alt_ = effective_radius_ + site_altitude_;
}

auto beam_propagation::set_effective_multiplier(real val) -> void
{
  multiplier_ = val;
  effective_radius_ = multiplier_ * earth_radius_;
  eer_alt_ = effective_radius_ + site_altitude_;
}

auto beam_propagation::set_earth_radius(real val) -> void
{
  earth_radius_ = val;
  effective_radius_ = multiplier_ * earth_radius_;
  eer_alt_ = effective_radius_ + site_altitude_;
}

auto beam_propagation::ground_range_altitude(real slant_range) const -> vec2<real>
{
  // distance along plane tangental to earth at site location
  double h = slant_range * cos_elev_;
  // distance above plane tangental to earth at site location
  double v = (slant_range * sin_elev_) + eer_alt_;
  return vec2<real>(
      // x = ground range = curved distance along earth surface
      std::atan(h/v) * effective_radius_
      // y = altitude = distance above spherical earth surface
    , std::hypot(h, v) - effective_radius_
  );

#if 0
  /* Alternative version based on theory from text book:
   *    Dopper Radar and Weather Observations, Doviak & Zrnic
   * The two versions here were derived separately, however result in negligible
   * difference (< 6.5mm height, < 0.1mm range @ 100km with elev angle of 30 deg) */
  real hgt = sqrt(in.x * in.x + eff_sqr_ + 2.0 * in.x * eff_rad_ * sin_elev_) - eff_rad_ + ref_point_sph_.hgt;
  real rng = eff_rad_ * asin((in.x * cos_elev_)/(eff_rad_ + hgt));
#endif
}

auto beam_propagation::slant_range(real ground_range) const -> real
{
  // TODO - it would be nice to calculate the geometric height here too...

  // inverse of above formula
  if (ground_range <= 0.0_r)
    return 0.0_r;
  else
    return eer_alt_ / ((cos_elev_ / std::tan(ground_range / effective_radius_)) - sin_elev_);
}

auto beam_propagation::required_elevation_angle(real ground_range, real altitude) const -> angle
{
  /* Warning to modifiers:
   * It is crucial that the calculations performed by this function use double precision
   * math.  This is due mostly to the very small angle of theta and very large values
   * of tgt_alt and eer_alt.  DO NOT CHANGE IT TO SINGLE PRECISION!  */

  // angle between site location and target on great circle
  angle theta = (ground_range / effective_radius_) * 1.0_rad;

  // slant range (law of cosines)
  double tgt_alt = effective_radius_ + altitude;
  double eer_alt = eer_alt_;
  double slant_range
    = (tgt_alt * tgt_alt) 
    + (eer_alt * eer_alt) 
    - 2.0 * tgt_alt * eer_alt * cos(theta);
  slant_range = std::sqrt(slant_range);

  // elevation angle (law of sines)
  // find the angle opposite the shorter side (since asin always returns < 90 degrees)
  if (tgt_alt < eer_alt)
    return asin((tgt_alt * sin(theta)) / slant_range) - 90.0_deg;
  else
    return 90.0_deg - theta - asin((eer_alt * sin(theta)) / slant_range);
}

