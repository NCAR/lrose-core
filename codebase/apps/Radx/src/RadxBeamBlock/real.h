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

#ifndef RAINUTIL_REAL_H
#define RAINUTIL_REAL_H

#include <cstdint>
#include <cmath>
#include <limits>
#include <type_traits>

namespace rainfields
{
  /// type parameterized constants
  template <typename T>
  struct constants
  {
    /// value of pi
    static constexpr T pi = M_PI;
    /// value of 2 * pi
    static constexpr T two_pi = 2.0 * pi;
    /// value of pi / 180 used to convert degrees to radians
    static constexpr T to_radians = M_PI / 180.0;
    /// value of 180 / pi used to convert radians to degrees
    static constexpr T to_degrees = 180.0 / M_PI;
  };
  /// these are needed to satisfy ODR requirements
  template <typename T> constexpr T constants<T>::pi;
  template <typename T> constexpr T constants<T>::two_pi;
  template <typename T> constexpr T constants<T>::to_radians;
  template <typename T> constexpr T constants<T>::to_degrees;

  namespace detail {
    /* this union is based on those in ieee754.h, however a number of things
     * are different:
     * - it is templated to reduce duplication of nan/is_nan style functions
     * - a constructor is added (with consistent argument order regarless of 
     *   endian-ness) to allow use within a constexpr context
     * - we split out 16 bits of the mantissa as a custom payload.  16 bits 
     *   are used because this is the maximum sized integer that will fit
     *   within the space of the 32 bit precision type.  it also avoids
     *   complicated bit shifting when needing to use mantissa0 and mantissa1
     *   in the larger types.
     */
    template <typename T, int S = sizeof(T), int digits = std::numeric_limits<T>::digits>
    union ieee754_nan;

    // 32 bit floating point types
    // 23 bit mantissa assumption
    template <typename T>
    union ieee754_nan<T, 4, 24>
    {
      static_assert(std::numeric_limits<T>::is_iec559, "floating point type is not ieee754");

      struct splitter
      {
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        unsigned int negative:1;
        unsigned int exponent:8;
        unsigned int quiet_nan:1;
        unsigned int payload:16;
        unsigned int mantissa:6;
        constexpr splitter(std::uint16_t payload)
          : negative(0), exponent(255), quiet_nan(1), payload(payload), mantissa(0)
        { }
#else
        unsigned int mantissa:6;
        unsigned int payload:16;
        unsigned int quiet_nan:1;
        unsigned int exponent:8;
        unsigned int negative:1;
        constexpr splitter(std::uint16_t payload)
          : mantissa(0), payload(payload), quiet_nan(1), exponent(255), negative(0)
        { }
#endif
      };

      splitter  split;
      T         value;

      constexpr ieee754_nan(std::uint16_t payload)
        : split(payload)
      { }
      constexpr ieee754_nan(T val)
        : value(val)
      { }
    };

    // 64 bit floating point types
    // 52 bit mantissa assumption
    template <typename T>
    union ieee754_nan<T, 8, 53>
    {
      static_assert(std::numeric_limits<T>::is_iec559, "floating point type is not ieee754");

      struct splitter
      {
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        unsigned int negative:1;
        unsigned int exponent:11;
        unsigned int quiet_nan:1;
        unsigned int payload:16;
        unsigned int mantissa0:3;
        unsigned int mantissa1:32;
        constexpr splitter(std::uint16_t payload)
          : negative(0), exponent(2047), quiet_nan(1), payload(payload), mantissa0(0), mantissa1(0)
        { }
#else
#if __FLOAT_WORD_ORDER__ == __ORDER_BIG_ENDIAN__
        unsigned int mantissa0:3;
        unsigned int payload:16;
        unsigned int quiet_nan:1;
        unsigned int exponent:11;
        unsigned int negative:1;
        unsigned int mantissa1:32;
        constexpr splitter(std::uint16_t payload)
          : mantissa0(0), payload(payload), quiet_nan(1), exponent(2047), negative(0), mantissa1(0)
        { }
#else
        unsigned int mantissa1:32;
        unsigned int mantissa0:3;
        unsigned int payload:16;
        unsigned int quiet_nan:1;
        unsigned int exponent:11;
        unsigned int negative:1;
        constexpr splitter(std::uint16_t payload)
          : mantissa1(0), mantissa0(0), payload(payload), quiet_nan(1), exponent(2047), negative(0)
        { }
#endif
#endif
      };

      splitter  split;
      T         value;

      constexpr ieee754_nan(std::uint16_t payload)
        : split(payload)
      { }
      constexpr ieee754_nan(T val)
        : value(val)
      { }
    };

    // 80 bit extended precision floating point types (long double)
    template <typename T>
    union ieee754_nan<T, 16, 64>
    {
      static_assert(std::numeric_limits<T>::is_iec559, "floating point type is not ieee754");

      struct splitter
      {
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        unsigned int negative:1;
        unsigned int exponent:15;
        unsigned int empty:16;
        unsigned int one:1;
        unsigned int quiet_nan:1;
        unsigned int payload:16;
        unsigned int mantissa0:14;
        unsigned int mantissa1:32;
        constexpr splitter(std::uint16_t payload)
          : negative(0), exponent(32767), empty(0), one(1), quiet_nan(1), payload(payload), mantissa0(0), mantissa1(0)
        { }
#else
#if __FLOAT_WORD_ORDER__ == __ORDER_BIG_ENDIAN__
        unsigned int exponent:15;
        unsigned int negative:1;
        unsigned int empty:16;
        unsigned int mantissa0:14;
        unsigned int payload:16;
        unsigned int quiet_nan:1;
        unsigned int one:1;
        unsigned int mantissa1:32;
        constexpr splitter(std::uint16_t payload)
          : exponent(32767), negative(0), empty(0), mantissa0(0), payload(payload), quiet_nan(1), one(1), mantissa1(0)
        { }
#else
        unsigned int mantissa1:32;
        unsigned int mantissa0:14;
        unsigned int payload:16;
        unsigned int quiet_nan:1;
        unsigned int one:1;
        unsigned int exponent:15;
        unsigned int negative:1;
        unsigned int empty:16;
        constexpr splitter(std::uint16_t payload)
          : mantissa1(0), mantissa0(0), payload(payload), quiet_nan(1), one(1), exponent(32767), negative(0), empty(0)
        { }
#endif
#endif
      };

      splitter  split;
      T         value;

      constexpr ieee754_nan(std::uint16_t payload)
        : split(payload)
      { }
      constexpr ieee754_nan(T val)
        : value(val)
      { }
    };

    // future work - 128 bit (quad precision) layouts if needed
  }

  // general arithmetic 'nan'
  template <typename T>
  inline constexpr auto nan() -> T
  {
    static_assert(std::is_floating_point<T>::value, "nan() instanciated on non floating point type");
    return std::numeric_limits<T>::quiet_NaN();
  }

  template <typename T>
  inline constexpr auto nan(std::uint16_t payload) -> T
  {
    static_assert(std::is_floating_point<T>::value, "nan() instanciated on non floating point type");
    return detail::ieee754_nan<T>{payload}.value;
  }

  template <typename T>
  inline constexpr auto nan_payload(T val) -> std::uint16_t
  {
    static_assert(std::is_floating_point<T>::value, "nan_payload() instanciated on non floating point type");
    return detail::ieee754_nan<T>{val}.split.payload;
  }

  template <typename T>
  inline constexpr auto is_nan(T val) -> bool
  {
    static_assert(std::is_floating_point<T>::value, "is_nan() instanciated on non floating point type");
    return std::isnan(val);
  }

  /// Linear interpolation function template
  template <typename T, typename U>
  inline constexpr auto lerp(const T& lhs, const T& rhs, U fraction) -> T
  {
    return lhs + fraction * (rhs - lhs);
  }
}

// THIS IS TAKEN FROM ancilla/src/util/real.h but here to avoid name clash
// note: we are directly messing with the rainfields namespace here!
namespace rainfields {
  /// Standard floating point type used within ancilla
  /**
   * This type may be changed to double to enable default use of double precision
   * floats within all ancilla processing.
   *
   * NOTE: If changing this to single precision, the following functions will 
   *       require work.  They are known to perform poorly with single precision:
   *         beam_propagation::determin_required_elevation_angle()
   */
  typedef float real;
  //typedef double real;

  // User defined literals for real
  /**
   * The only problem with changing the above definition of 'real' is that literal constants
   * embedded within the source would now be of the wrong type.  This causes problems in cases
   * where the literal is embedded in an expression and may now cause a precision extension. eg:
   * real x = sin(my_real * 5.0):
   * In this case the double version of sin will be called even if real is defined as float, 
   * which results in a cast of my_real to double, and the result back down to float.
   *
   * To alleviate this problem, the user defined literal "_r" has been defined.  It should be used
   * as follows:
   * real x = sin(my_real * 5.0_r);
   * In this case, the literal (5.0) will automatically take the same type as 'real'.
   */
  inline constexpr real operator"" _r(long double val)      { return val; }

  /// Overload rainfields::nan<T>() to provide real as the default type
  inline constexpr auto nan() -> real                       { return nan<real>(); }

  /// Overload rainfields::nan<T>(payload) to provide real as the default type
  inline constexpr auto nan(std::uint16_t payload) -> real  { return nan<real>(payload); }
}

#endif
