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

#ifndef ANCILLA_MODELS_BEAM_PROPAGATION_H
#define ANCILLA_MODELS_BEAM_PROPAGATION_H

#include "angle.h"
#include "real.h"
#include "vec2.h"

namespace rainfields {
namespace ancilla {

#if 0
  /* this is the effective radius previously in use at BoM.  it is supposed to be
   * 4/3 earth, however clearly it is not (closer to 7/6 earth).  for now, we are
   * returning to the theoretically supported 4/3 earth...
   */
  static constexpr real default_effective_earth_radius = 7362500.0;
#endif

  /// Model used to determine radar beam path
  /** 
   * Due to the large numbers involved, this class always uses double precision
   * for internal calculation.
   */
  class beam_propagation
  {
  public:
    /// default radius of earth (wgs84 major axis)
    static constexpr real default_radius = 6378137.000_r;

    /// default multiplier used to convert from earth radius to effective earth radius
    static constexpr real default_multiplier = 4.0 / 3.0_r;

  public:
    beam_propagation(
          angle elevation_angle
        , real site_altitude = 0.0_r
        , real earth_radius = default_radius
        , real effective_multiplier = default_multiplier);

    beam_propagation(const beam_propagation& rhs) = default;
    beam_propagation(beam_propagation&& rhs) = default;
    auto operator=(const beam_propagation& rhs) -> beam_propagation& = default;
    auto operator=(beam_propagation&& rhs) -> beam_propagation& = default;
    ~beam_propagation() = default;

    auto elevation_angle() const -> angle               { return elevation_angle_; }
    auto set_elevation_angle(angle val) -> void;

    auto site_altitude() const -> real                  { return site_altitude_; }
    auto set_site_altitude(real val) -> void;

    auto effective_multiplier() const -> real           { return multiplier_; }
    auto set_effective_multiplier(real val) -> void;

    auto earth_radius() const -> real                   { return earth_radius_; }
    auto set_earth_radius(real val) -> void;

    /// Determine ground range and altitude from slant range
    auto ground_range_altitude(real slant_range) const -> vec2<real>;

    /// Determine slant range from ground range
    auto slant_range(real ground_range) const -> real;

    /// Determine required elevation angle to hit given ground range and altitude
    auto required_elevation_angle(real ground_range, real altitude) const -> angle;

  private:
    angle   elevation_angle_;
    real    site_altitude_;
    real    earth_radius_;
    real    multiplier_;

    // cached for speed of repeated calls
    double  effective_radius_;
    double  cos_elev_;
    double  sin_elev_;
    double  eer_alt_;
  };
}}

#endif
