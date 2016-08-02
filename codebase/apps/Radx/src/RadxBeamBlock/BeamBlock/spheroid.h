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

#ifndef ANCILLA_MODELS_SPHEROID_H
#define ANCILLA_MODELS_SPHEROID_H

#include "latlon.h"
#include "real.h"
#include "traits.h"
#include "vec3.h"
#include "xml.h"

namespace rainfields {
namespace ancilla {
  /// representation of spheroids commonly used in map projections
  class spheroid
  {
  public:
    /// common standardized spheroids
    enum class standard
    {
        i65
      , ans                   // australian national spheriod
      , clarke1858
      , grs80
      , wgs84
      , wgs72
      , international1924
      , australian_national
    };

  public:
    spheroid(standard cs);
    spheroid(real semi_major_axis, real invserse_flattening);
    spheroid(const xml::node& conf);

    spheroid(const spheroid& rhs) = default;
    spheroid(spheroid&& rhs) = default;
    auto operator=(const spheroid& rhs) -> spheroid& = default;
    auto operator=(spheroid&& rhs) -> spheroid& = default;
    ~spheroid() = default;

    auto semi_major_axis() const -> real        { return a_; }
    auto semi_minor_axis() const -> real        { return b_; }
    auto flattening() const -> real             { return f_; }
    auto inverse_flattening() const -> real     { return inv_f_; }
    auto eccentricity() const -> real           { return e_; }
    auto eccentricity_squared() const -> real   { return e_sqr_; }
    auto mean_radius() const -> real            { return avg_r_; }
    auto authalic_radius() const -> real        { return auth_r_; }

    /// distance from center of spheroid to point at given latitude
    auto radius_physical(angle lat) const -> real;

    /// radius of curvature in north/south direction at given latitude
    auto radius_of_curvature_meridional(angle lat) const -> real;

    /// radius of curvature in direction normal to meridian at given latitude (east/west @ equator)
    auto radius_of_curvature_normal(angle lat) const -> real;

    /// mean radius of curvature at given latitude (ie: rad of curve averaged in all directions)
    auto radius_of_curvature(angle lat) const -> real;

    /// convert latitude and longitude into earth-centered, earth-fixed cartesian coordinates
    auto latlon_to_ecefxyz(latlonalt pos) const -> vec3<real>;

    /// convert earth-centered, earth-fixed cartesian coordinates into latitude & longitude
    auto ecefxyz_to_latlon(vec3<real> pos) const -> latlonalt;

    /// calculate position based on bearing and range from a reference point
    /**
     * This calculation uses Vincenty's formula, and while being extremely accurate, is
     * also relatively expensive.
     */
    auto bearing_range_to_latlon(latlon pos, angle bearing, real range) const -> latlon;

    /// calculate bearing and initial range based on two reference points
    /**
     * The bearing returned is the bearing seen at pos1 looking toward pos2.
     *
     * This calculation uses Vincenty's formula, and while being extremely accurate, is
     * also relatively expensive.
     */
    auto latlon_to_bearing_range(latlon pos1, latlon pos2) const -> std::pair<angle, real>;

    /// calculate position based on bearing and range from a reference point
    /**
     * This version of the function uses the haversine formula, which while being less
     * accurate than the vincenty calculation is also significantly faster and never fails
     * to converge.
     */
    auto bearing_range_to_latlon_haversine(latlon pos, angle bearing, real range) const -> latlon;

    /// calculate initial bearing and range based on two reference points
    /**
     * The bearing returned is the bearing seen at pos1 looking toward pos2.
     *
     * This version of the function uses the haversine formula, which while being less
     * accurate than the vincenty calculation is also significantly faster and never fails
     * to converge.
     */
    auto latlon_to_bearing_range_haversine(latlon pos1, latlon pos2) const -> std::pair<angle, real>;

  private:
    void derive_parameters();

  private:
    // model parameters
    real a_;
    real inv_f_;

    // derived parameters
    real f_;
    real b_;
    real e_;
    real e_sqr_;
    real ep2_;
    real avg_r_;
    real auth_r_;
  };
}}

namespace rainfields {
  template <>
  struct enum_traits<ancilla::spheroid::standard>
  {
    static constexpr const char* name = "spheroid::standard";
    static constexpr int count = 8;
    static constexpr const char* strings[] = 
    {
        "i65"
      , "ans"
      , "clarke1858"
      , "grs80"
      , "wgs84"
      , "wgs72"
      , "international1924"
      , "australian_national"
    };
  };
}

#endif
