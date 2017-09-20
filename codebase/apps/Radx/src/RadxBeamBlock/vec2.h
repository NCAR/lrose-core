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

#ifndef RAINUTIL_VEC2_H
#define RAINUTIL_VEC2_H

#include "angle.h"

#include <cmath>
#include <istream>
#include <ostream>

namespace rainfields {
  template <typename T>
  struct vec2
  {
    T x, y;

    typedef T value_type;
    static constexpr size_t rank = 2;

    vec2() = default;
    constexpr vec2(T x, T y) : x(x), y(y) { }

    constexpr vec2(const vec2& rhs) = default;
    constexpr vec2(vec2&& rhs) noexcept = default;

    auto operator=(const vec2& rhs) -> vec2& = default;
    auto operator=(vec2&& rhs) noexcept -> vec2& = default;

    ~vec2() noexcept = default;

    template <typename U>
    constexpr operator vec2<U>() const
    {
      return vec2<U>(static_cast<U>(x), static_cast<U>(y));
    }

    // TODO - disable these if T is floating_point
    constexpr auto operator==(const vec2& rhs) const -> bool        { return x == rhs.x && y == rhs.y; }
    constexpr auto operator!=(const vec2& rhs) const -> bool        { return x != rhs.x || y != rhs.y; }

    auto operator+=(const vec2& rhs) -> vec2&                       { x += rhs.x; y += rhs.y; return *this; }
    auto operator-=(const vec2& rhs) -> vec2&                       { x -= rhs.x; y -= rhs.y; return *this; }
    template <typename U>
    auto operator*=(const U& rhs) -> vec2&                          { x *= rhs; y *= rhs; return *this; }
    template <typename U>
    auto operator/=(const U& rhs) -> vec2&                          { x /= rhs; y /= rhs; return *this; }

    constexpr auto dot(const vec2& rhs) const -> T                  { return x * rhs.x + y * rhs.y; }
    constexpr auto cross(const vec2& rhs) const -> T                { return x * rhs.y - y * rhs.x; }
    constexpr auto length_sqr() const -> T                          { return x * x + y * y; }
              auto length() const -> decltype((std::sqrt(x*x + y*y))) { return std::sqrt(x*x + y*y); }
              auto angle() const -> angle                           { return atan2(y, x); }

    template <typename T1, typename T2>
    friend constexpr auto operator+(const vec2<T1>& lhs, const vec2<T2>& rhs) -> vec2<decltype(lhs.x + rhs.x)>;

    template <typename T1, typename T2>
    friend constexpr auto operator-(const vec2<T1>& lhs, const vec2<T2>& rhs) -> vec2<decltype(lhs.x - rhs.x)>;

    template <typename T1, typename U>
    friend constexpr auto operator*(const vec2<T1>& lhs, U rhs) -> vec2<decltype(lhs.x * rhs)>;

    template <typename T1, typename U>
    friend constexpr auto operator*(U lhs, const vec2<T1>& rhs) -> vec2<decltype(lhs * rhs.x)>;

    template <typename T1, typename U>
    friend constexpr auto operator/(const vec2<T1>& lhs, U rhs) -> vec2<decltype(lhs.x / rhs)>;

    template <typename T1>
    friend auto operator>>(std::istream& lhs, vec2<T1>& rhs) -> std::istream&;
    template <typename T1>
    friend auto operator<<(std::ostream& lhs, const vec2<T1>& rhs) -> std::ostream&;
  };

  template <typename T1, typename T2>
  inline constexpr auto operator+(const vec2<T1>& lhs, const vec2<T2>& rhs) -> vec2<decltype(lhs.x + rhs.x)>
  {
    return vec2<decltype(lhs.x + rhs.x)>(lhs.x + rhs.x, lhs.y + rhs.y);
  }

  template <typename T1, typename T2>
  inline constexpr auto operator-(const vec2<T1>& lhs, const vec2<T2>& rhs) -> vec2<decltype(lhs.x - rhs.x)>
  {
    return vec2<decltype(lhs.x - rhs.x)>(lhs.x - rhs.x, lhs.y - rhs.y);
  }

  template <typename T1, typename U>
  inline constexpr auto operator*(const vec2<T1>& lhs, U rhs) -> vec2<decltype(lhs.x * rhs)>
  {
    return vec2<decltype(lhs.x * rhs)>(lhs.x * rhs, lhs.y * rhs);
  }

  template <typename T1, typename U>
  inline constexpr auto operator*(U lhs, const vec2<T1>& rhs) -> vec2<decltype(lhs * rhs.x)>
  {
    return vec2<decltype(lhs * rhs.x)>(lhs * rhs.x, lhs * rhs.y);
  }

  template <typename T1, typename U>
  inline constexpr auto operator/(const vec2<T1>& lhs, U rhs) -> vec2<decltype(lhs.x / rhs)>
  {
    return vec2<decltype(lhs.x / rhs)>(lhs.x / rhs, lhs.y / rhs);
  }

  template <typename T1>
  inline auto operator>>(std::istream& lhs, vec2<T1>& rhs) -> std::istream&
  {
    return lhs >> rhs.x >> rhs.y;
  }

  template <typename T1>
  inline auto operator<<(std::ostream& lhs, const vec2<T1>& rhs) -> std::ostream&
  {
    return lhs << rhs.x << " " << rhs.y;
  }
}

#endif
