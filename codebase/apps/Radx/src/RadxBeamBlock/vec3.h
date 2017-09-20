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

#ifndef RAINUTIL_VEC3_H
#define RAINUTIL_VEC3_H

#include "angle.h"

#include <cmath>
#include <istream>
#include <ostream>

namespace rainfields {
  template <typename T>
  struct vec3
  {
    T x, y, z;

    typedef T value_type;
    static constexpr size_t rank = 3;

    vec3() = default;
    constexpr vec3(T x, T y, T z) : x(x), y(y), z(z) { }

    constexpr vec3(const vec3& rhs) = default;
    constexpr vec3(vec3&& rhs) noexcept = default;

    auto operator=(const vec3& rhs) -> vec3& = default;
    auto operator=(vec3&& rhs) noexcept -> vec3& = default;

    ~vec3() noexcept = default;

    template <typename U>
    constexpr operator vec3<U>() const
    {
      return vec3<U>(static_cast<U>(x), static_cast<U>(y), static_cast<U>(z));
    }

    // TODO - disable these if T is floating_point
    constexpr auto operator==(const vec3& rhs) const -> bool        { return x == rhs.x && y == rhs.y && z == rhs.z; }
    constexpr auto operator!=(const vec3& rhs) const -> bool        { return x != rhs.x || y != rhs.y || z != rhs.z; }

    auto operator+=(const vec3& rhs) -> vec3&                       { x += rhs.x; y += rhs.y; z += rhs.z; return *this; }
    auto operator-=(const vec3& rhs) -> vec3&                       { x -= rhs.x; y -= rhs.y; z -= rhs.z; return *this; }
    template <typename U>
    auto operator*=(const U& rhs) -> vec3&                          { x *= rhs; y *= rhs; z *= rhs; return *this; }
    template <typename U>
    auto operator/=(const U& rhs) -> vec3&                          { x /= rhs; y /= rhs; z /= rhs; return *this; }

    constexpr auto dot(const vec3& rhs) const -> T                  { return x * rhs.x + y * rhs.y + z * rhs.z; }
    constexpr auto cross(const vec3& rhs) const -> vec3             { return vec3(y * rhs.z - z * rhs.y, z * rhs.x - x * rhs.z, x * rhs.y - y * rhs.x); }
    constexpr auto length_sqr() const -> T                          { return x * x + y * y + z * z; }
              auto length() const -> decltype(std::sqrt(x*x + y*y)) { return std::sqrt(x * x + y * y + z * z); }
              auto angle(const vec3& rhs) const -> angle            { return acos(dot(rhs) / (length() * rhs.length())); }

    template <typename T1, typename T2>
    friend constexpr auto operator+(const vec3<T1>& lhs, const vec3<T2>& rhs) -> vec3<decltype(lhs.x + rhs.x)>;

    template <typename T1, typename T2>
    friend constexpr auto operator-(const vec3<T1>& lhs, const vec3<T2>& rhs) -> vec3<decltype(lhs.x - rhs.x)>;

    template <typename T1, typename U>
    friend constexpr auto operator*(const vec3<T1>& lhs, U rhs) -> vec3<decltype(lhs.x * rhs)>;

    template <typename T1, typename U>
    friend constexpr auto operator*(U lhs, const vec3<T1>& rhs) -> vec3<decltype(lhs * rhs.x)>;

    template <typename T1, typename U>
    friend constexpr auto operator/(const vec3<T1>& lhs, U rhs) -> vec3<decltype(lhs.x / rhs)>;

    template <typename T1>
    friend auto operator>>(std::istream& lhs, vec3<T1>& rhs) -> std::istream&;
    template <typename T1>
    friend auto operator<<(std::ostream& lhs, const vec3<T1>& rhs) -> std::ostream&;
  };

  template <typename T1, typename T2>
  inline constexpr auto operator+(const vec3<T1>& lhs, const vec3<T2>& rhs) -> vec3<decltype(lhs.x + rhs.x)>
  {
    return vec3<decltype(lhs.x + rhs.x)>(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z);
  }

  template <typename T1, typename T2>
  inline constexpr auto operator-(const vec3<T1>& lhs, const vec3<T2>& rhs) -> vec3<decltype(lhs.x - rhs.x)>
  {
    return vec3<decltype(lhs.x - rhs.x)>(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);
  }

  template <typename T1, typename U>
  inline constexpr auto operator*(const vec3<T1>& lhs, U rhs) -> vec3<decltype(lhs.x * rhs)>
  {
    return vec3<decltype(lhs.x * rhs)>(lhs.x * rhs, lhs.y * rhs, lhs.z * rhs);
  }

  template <typename T1, typename U>
  inline constexpr auto operator*(U lhs, const vec3<T1>& rhs) -> vec3<decltype(lhs * rhs.x)>
  {
    return vec3<decltype(lhs * rhs.x)>(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z);
  }

  template <typename T1, typename U>
  inline constexpr auto operator/(const vec3<T1>& lhs, U rhs) -> vec3<decltype(lhs.x / rhs)>
  {
    return vec3<decltype(lhs.x / rhs)>(lhs.x / rhs, lhs.y / rhs, lhs.z / rhs);
  }

  template <typename T1>
  inline auto operator>>(std::istream& lhs, vec3<T1>& rhs) -> std::istream&
  {
    return lhs >> rhs.x >> rhs.y >> rhs.z;
  }

  template <typename T1>
  inline auto operator<<(std::ostream& lhs, const vec3<T1>& rhs) -> std::ostream&
  {
    return lhs << rhs.x << ' ' << rhs.y << ' ' << rhs.z;
  }
}

#endif
