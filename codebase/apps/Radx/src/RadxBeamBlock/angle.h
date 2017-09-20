// %=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%
// ** Rainfields Utilities Library (rainutil)
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

#ifndef RAINUTIL_ANGLE_H
#define RAINUTIL_ANGLE_H

#include "real.h"
#include "string_utils.h"

#include <cmath>
#include <istream>
#include <limits>
#include <ostream>

namespace rainfields
{
  class angle
  {
  public:
    using value_type = double;

  public:
    angle() noexcept = default;

    constexpr angle(const angle& rhs) noexcept = default;
    constexpr angle(angle&& rhs) noexcept = default;

    auto operator=(const angle& rhs) noexcept -> angle& = default;
    auto operator=(angle&& rhs) noexcept -> angle& = default;

    ~angle() noexcept = default;

    constexpr auto operator==(angle rhs) const -> bool  { return rads_ == rhs.rads_; }
    constexpr auto operator!=(angle rhs) const -> bool  { return rads_ != rhs.rads_; }
    constexpr auto operator<(angle rhs) const -> bool   { return rads_ < rhs.rads_; }
    constexpr auto operator<=(angle rhs) const -> bool  { return rads_ <= rhs.rads_; }
    constexpr auto operator>(angle rhs) const -> bool   { return rads_ > rhs.rads_; }
    constexpr auto operator>=(angle rhs) const -> bool  { return rads_ >= rhs.rads_; }

    auto operator+=(angle rhs) -> angle&                { rads_ += rhs.rads_; return *this; }
    auto operator-=(angle rhs) -> angle&                { rads_ -= rhs.rads_; return *this; }
    auto operator*=(double rhs) -> angle&               { rads_ *= rhs; return *this; }
    auto operator/=(double rhs) -> angle&               { rads_ /= rhs; return *this; }

    constexpr auto degrees() const -> double            { return rads_ * constants<double>::to_degrees; }
              auto set_degrees(double val) -> void      { rads_ = val * constants<double>::to_radians; }
    constexpr auto radians() const -> double            { return rads_; }
              auto set_radians(double val) -> void      { rads_ = val; }
              auto dms(int& deg, int& min, double& sec) const -> void;
              auto set_dms(int d, int m, double s) -> void;
    constexpr auto abs() const -> angle                 { return angle(std::abs(rads_)); }
              auto normalize() const -> angle           { return angle(rads_ - constants<double>::two_pi * std::floor(rads_ / constants<double>::two_pi)); }

  public:
    friend constexpr auto operator"" _deg(unsigned long long val) -> angle;
    friend constexpr auto operator"" _deg(long double val) -> angle;
    friend constexpr auto operator"" _rad(unsigned long long val) -> angle;
    friend constexpr auto operator"" _rad(long double val) -> angle;

    friend auto operator>>(std::istream& lhs, angle& rhs) -> std::istream&;
    friend auto operator<<(std::ostream& lhs, const angle& rhs) -> std::ostream&;

    friend constexpr auto operator-(angle lhs) -> angle;

    friend constexpr auto operator+(angle lhs, angle rhs) -> angle;
    friend constexpr auto operator-(angle lhs, angle rhs) -> angle;
    friend constexpr auto operator*(double lhs, angle rhs) -> angle;
    friend constexpr auto operator*(angle lhs, double rhs) -> angle;
    friend constexpr auto operator/(angle lhs, angle rhs) -> double;
    friend constexpr auto operator/(angle lhs, double rhs) -> angle;

    friend auto cos(angle x) -> double;
    friend auto sin(angle x) -> double;
    friend auto tan(angle x) -> double;
    friend auto acos(double x) -> angle;
    friend auto asin(double x) -> angle;
    friend auto atan(double x) -> angle;
    friend auto atan2(double y, double x) -> angle;

  private:
    explicit constexpr angle(double rads) : rads_(rads) { }

  private:
    double rads_;
  };

  // user defined literals (use 'val * 1_deg' to convert a built-in type to an angle)
  inline constexpr auto operator"" _deg(unsigned long long val) -> angle
  {
    return angle(val * constants<double>::to_radians);
  }
  inline constexpr auto operator"" _deg(long double val) -> angle
  {
    return angle(val * constants<double>::to_radians);
  }
  inline constexpr auto operator"" _rad(unsigned long long val) -> angle
  {
    return angle(val);
  }
  inline constexpr auto operator"" _rad(long double val) -> angle
  {
    return angle(val);
  }

  // stream insertion and extration
  inline auto operator>>(std::istream& lhs, angle& rhs) -> std::istream&
  {
    lhs >> rhs.rads_;
    rhs.rads_ *= constants<double>::to_radians;
    return lhs;
  }
  inline auto operator<<(std::ostream& lhs, const angle& rhs) -> std::ostream&
  {
    return lhs << rhs.degrees();
  }

  // unary operators
  inline constexpr auto operator-(angle lhs) -> angle             { return angle(-lhs.rads_); }

  // binary operators
  inline constexpr auto operator+(angle lhs, angle rhs) -> angle { return angle(lhs.rads_ + rhs.rads_); }
  inline constexpr auto operator-(angle lhs, angle rhs) -> angle { return angle(lhs.rads_ - rhs.rads_); }
  inline constexpr auto operator*(double lhs, angle rhs) -> angle { return angle(lhs * rhs.rads_); }
  inline constexpr auto operator*(angle lhs, double rhs) -> angle { return angle(lhs.rads_ * rhs); }
  inline constexpr auto operator/(angle lhs, angle rhs) -> double { return lhs.rads_ / rhs.rads_; }
  inline constexpr auto operator/(angle lhs, double rhs) -> angle { return angle(lhs.rads_ / rhs); }

  // overload the standard trig functions to work directly with angle
  inline auto cos(angle x) -> double                  { return std::cos(x.rads_); }
  inline auto sin(angle x) -> double                  { return std::sin(x.rads_); }
  inline auto tan(angle x) -> double                  { return std::tan(x.rads_); }
  inline auto acos(double x) -> angle                 { return angle(std::acos(x)); }
  inline auto asin(double x) -> angle                 { return angle(std::asin(x)); }
  inline auto atan(double x) -> angle                 { return angle(std::atan(x)); }
  inline auto atan2(double y, double x) -> angle      { return angle(std::atan2(y, x)); }

  // overload the nan functions from real.h
  template <>
  inline constexpr auto nan<angle>() -> angle
  {
    return angle(nan<double>() * 1_rad);
  }

  template <>
  inline constexpr auto nan<angle>(std::uint16_t payload) -> angle
  {
    return angle(nan<double>(payload) * 1_rad);
  }

  template <>
  inline constexpr auto nan_payload<angle>(angle val) -> std::uint16_t
  {
    return nan_payload(val.radians());
  }

  template <>
  inline constexpr auto is_nan<angle>(angle val) -> bool
  {
    return std::isnan(val.radians());
  }

  // overload the from_string function from string_utils.h
  template <>
  auto from_string<angle>(const char* str) -> angle;
}

#endif
