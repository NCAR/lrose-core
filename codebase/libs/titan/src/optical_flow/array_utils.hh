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

#ifndef OPTFLOW_ARRAY_UTILS_HH
#define OPTFLOW_ARRAY_UTILS_HH

#include "array.hh"
#include <dataport/port_types.h>
#include <stdexcept>

namespace titan {
namespace array_utils {

  /// Copy the contents of one array to another
  template <typename T>
  T& copy(T& output, const T& input)
  {
    if (output.size() != input.size())
      throw std::logic_error("array size mismatch");
    copy_array(input.data(), output.size(), output.data());
    return output;
  }

  /// Zero the contents of an array
  template <typename T, typename = typename std::enable_if<std::has_trivial_copy_constructor<typename T::value_type>::value>::type>
  T& zero(T& output)
  {
    std::memset(output.data(), 0, sizeof(typename T::value_type) * output.size());
    return output;
  }

  /// Fill an array with a constant value
  template <typename T>
  T& fill(T& output, const typename T::value_type val)
  {
    for (typename T::size_type i = 0; i < output.size(); ++i)
      output.data()[i] = val;
    return output;
  }

  /// Replace NaN values in an array with a constant value
  template <typename T>
  T& remove_nans(T& output, const T& input, const typename T::value_type val)
  {
    if (output.size() != input.size())
      throw std::logic_error("array size mismatch");
    for (typename T::size_type i = 0; i < output.size(); ++i)
      output.data()[i] = isnan(input.data()[i]) ? val : input.data()[i];
    return output;
  }

  template <typename T>
  T& remove_nans(T& output, const T& input, const T& val)
  {
    if (   output.size() != input.size()
        || output.size() != val.size())
      throw std::logic_error("array size mismatch");
    for (typename T::size_type i = 0; i < output.size(); ++i)
      output.data()[i] = isnan(input.data()[i]) ? val.data()[i] : input.data()[i];
    return output;
  }

  /// Addition involving the values of an array{1|2}<T>
  template <typename T>
  T& add(T& output, const T& lhs, const T& rhs)
  {
    if (   output.size() != lhs.size()
        || output.size() != rhs.size())
      throw std::logic_error("array size mismatch");
    for (typename T::size_type i = 0; i < output.size(); ++i)
      output.data()[i] = lhs.data()[i] + rhs.data()[i];
    return output;
  }

  template <typename T, typename U>
  T& add(T& output, const U& lhs, const T& rhs)
  {
    if (output.size() != rhs.size())
      throw std::logic_error("array size mismatch");
    for (typename T::size_type i = 0; i < output.size(); ++i)
      output.data()[i] = lhs + rhs.data()[i];
    return output;
  }

  template <typename T, typename U>
  T& add(T& output, const T& lhs, const U& rhs)
  {
    if (output.size() != lhs.size())
      throw std::logic_error("array size mismatch");
    for (typename T::size_type i = 0; i < output.size(); ++i)
      output.data()[i] = lhs.data()[i] + rhs;
    return output;
  }

  /// Subtraction involving the values of an array{1|2}<T>
  template <typename T>
  T& subtract(T& output, const T& lhs, const T& rhs)
  {
    if (   output.size() != lhs.size()
        || output.size() != rhs.size())
      throw std::logic_error("array size mismatch");
    for (typename T::size_type i = 0; i < output.size(); ++i)
      output.data()[i] = lhs.data()[i] - rhs.data()[i];
    return output;
  }

  template <typename T, typename U>
  T& subtract(T& output, const U& lhs, const T& rhs)
  {
    if (output.size() != rhs.size())
      throw std::logic_error("array size mismatch");
    for (typename T::size_type i = 0; i < output.size(); ++i)
      output.data()[i] = lhs - rhs.data()[i];
    return output;
  }

  template <typename T, typename U>
  T& subtract(T& output, const T& lhs, const U& rhs)
  {
    if (output.size() != lhs.size())
      throw std::logic_error("array size mismatch");
    for (typename T::size_type i = 0; i < output.size(); ++i)
      output.data()[i] = lhs.data()[i] - rhs;
    return output;
  }

  /// Multiplication involving the values of an array{1|2}<T>
  template <typename T>
  T& multiply(T& output, const T& lhs, const T& rhs)
  {
    if (   output.size() != lhs.size()
        || output.size() != rhs.size())
      throw std::logic_error("array size mismatch");
    for (typename T::size_type i = 0; i < output.size(); ++i)
      output.data()[i] = lhs.data()[i] * rhs.data()[i];
    return output;
  }

  template <typename T, typename U>
  T& multiply(T& output, const U& lhs, const T& rhs)
  {
    if (output.size() != rhs.size())
      throw std::logic_error("array size mismatch");
    for (typename T::size_type i = 0; i < output.size(); ++i)
      output.data()[i] = lhs * rhs.data()[i];
    return output;
  }

  template <typename T, typename U>
  T& multiply(T& output, const T& lhs, const U& rhs)
  {
    if (output.size() != lhs.size())
      throw std::logic_error("array size mismatch");
    for (typename T::size_type i = 0; i < output.size(); ++i)
      output.data()[i] = lhs.data()[i] * rhs;
    return output;
  }

  /// Division involving the values of an array{1|2}<T>
  template <typename T>
  T& divide(T& output, const T& lhs, const T& rhs)
  {
    if (   output.size() != lhs.size()
        || output.size() != rhs.size())
      throw std::logic_error("array size mismatch");
    for (typename T::size_type i = 0; i < output.size(); ++i)
      output.data()[i] = lhs.data()[i] / rhs.data()[i];
    return output;
  }

  template <typename T, typename U>
  T& divide(T& output, const U& lhs, const T& rhs)
  {
    if (output.size() != rhs.size())
      throw std::logic_error("array size mismatch");
    for (typename T::size_type i = 0; i < output.size(); ++i)
      output.data()[i] = lhs / rhs.data()[i];
    return output;
  }

  template <typename T, typename U>
  T& divide(T& output, const T& lhs, const U& rhs)
  {
    if (output.size() != lhs.size())
      throw std::logic_error("array size mismatch");
    for (typename T::size_type i = 0; i < output.size(); ++i)
      output.data()[i] = lhs.data()[i] / rhs;
    return output;
  }

  /// Multiply then add (a*b + c) involving the values of an array{1|2}<T>
  // TODO - use fma() to perform this operation if and _only_ if the
  //        FP_FAST_FMAx symbols are defined.  currently our processors
  //        don't have dedicated fma instructions so it's a low priority
  //        until we switch to Haswell (Intel) or similar.
  template <typename T>
  T& multiply_add(T& output, const T& mult_lhs, const T& mult_rhs, const T& add)
  {
    if (   output.size() != mult_lhs.size()
        || output.size() != mult_rhs.size()
        || output.size() != add.size())
      throw std::logic_error("array size mismatch");
    for (typename T::size_type i = 0; i < output.size(); ++i)
      output.data()[i] = mult_lhs.data()[i] * mult_rhs.data()[i] + add.data()[i];
    return output;
  }

  template <typename T, typename U>
  T& multiply_add(T& output, const U& mult_lhs, const T& mult_rhs, const T& add)
  {
    if (   output.size() != mult_rhs.size()
        || output.size() != add.size())
      throw std::logic_error("array size mismatch");
    for (typename T::size_type i = 0; i < output.size(); ++i)
      output.data()[i] = mult_lhs * mult_rhs.data()[i] + add.data()[i];
    return output;
  }

  template <typename T, typename U>
  T& multiply_add(T& output, const T& mult_lhs, const U& mult_rhs, const T& add)
  {
    if (   output.size() != mult_lhs.size()
        || output.size() != add.size())
      throw std::logic_error("array size mismatch");
    for (typename T::size_type i = 0; i < output.size(); ++i)
      output.data()[i] = mult_lhs.data()[i] * mult_rhs + add.data()[i];
    return output;
  }

  template <typename T, typename U>
  T& multiply_add(T& output, const T& mult_lhs, const T& mult_rhs, const U& add)
  {
    if (   output.size() != mult_lhs.size()
        || output.size() != mult_rhs.size())
      throw std::logic_error("array size mismatch");
    for (typename T::size_type i = 0; i < output.size(); ++i)
      output.data()[i] = mult_lhs.data()[i] * mult_rhs.data()[i] + add;
    return output;
  }

  template <typename T, typename U>
  T& multiply_add(T& output, const U& mult_lhs, const T& mult_rhs, const U& add)
  {
    if (output.size() != mult_rhs.size())
      throw std::logic_error("array size mismatch");
    for (typename T::size_type i = 0; i < output.size(); ++i)
      output.data()[i] = mult_lhs * mult_rhs.data()[i] + add;
    return output;
  }

  template <typename T, typename U>
  T& multiply_add(T& output, const T& mult_lhs, const U& mult_rhs, const U& add)
  {
    if (output.size() != mult_lhs.size())
      throw std::logic_error("array size mismatch");
    for (typename T::size_type i = 0; i < output.size(); ++i)
      output.data()[i] = mult_lhs.data()[i] * mult_rhs + add;
    return output;
  }

  /// Replace values in an array below a threshold with a constant
  template <typename T, typename U>
  T& threshold_min(T& output, const T& input, const U& threshold, const typename T::value_type val)
  {
    if (output.size() != input.size())
      throw std::logic_error("array size mismatch");
    for (typename T::size_type i = 0; i < output.size(); ++i)
      output.data()[i] = input.data()[i] < threshold ? val : input.data()[i];
    return output;
  }

  /// Replace values in an array above a threshold with a constant
  template <typename T, typename U>
  T& threshold_max(T& output, const T& input, const U& threshold, const typename T::value_type val)
  {
    if (output.size() != input.size())
      throw std::logic_error("array size mismatch");
    for (typename T::size_type i = 0; i < output.size(); ++i)
      output.data()[i] = input.data()[i] > threshold ? val : input.data()[i];
    return output;
  }

  /// Take the log10 of every value in the array
  template <typename T>
  T& log10(T& output, const T& in)
  {
    if (output.size() != in.size())
      throw std::logic_error("array size mismatch");
    for (typename T::size_type i = 0; i < output.size(); ++i)
      output.data()[i] = std::log10(in.data()[i]);
    return output;
  }

  /// Interpolate data from one array onto another of a different size (bi-linear)
  template <typename T>
  array2<T>& interpolate(array2<T>& output, const array2<T>& input)
  {
    if (   output.dims()[0] == input.dims()[0]
        && output.dims()[1] == input.dims()[1])
    {
      // special case 1: no interpolation required
      copy(output, input);
    }
    else if (   output.dims()[0] * 2 == input.dims()[0]
             && output.dims()[1] * 2 == input.dims()[1])
    {
      // special case 2: perfect downscale (we are one level smaller than input)
      for (typename array2<T>::size_type y = 0; y < output.dims()[0]; ++y)
      {
        const T* top = input[y * 2];
        const T* bot = input[y * 2 + 1];
              T* out = output[y];
        for (typename array2<T>::size_type x = 0; x < output.dims()[1]; ++x)
        {
          out[0] = (top[0] + top[1] + bot[0] + bot[1]) * 0.25;
          top += 2;
          bot += 2;
          ++out;
        }
      }
    }
    else
    {
      // generic case: bi-linear interpolation (much slower)
      typedef typename std::make_signed<typename array2<T>::size_type>::type difference_type;
      fl32 scale_x = static_cast<fl32>(input.dims()[1]) / output.dims()[1];
      fl32 scale_y = static_cast<fl32>(input.dims()[0]) / output.dims()[0];
      for (typename array2<T>::size_type y = 0; y < output.dims()[0]; ++y)
      {
        T* out = output[y];
        for (typename array2<T>::size_type x = 0; x < output.dims()[1]; ++x)
        {
          // target x, y (the +0.5 is for rounding purposes)
          fl32 tx = x * scale_x + 0.5;
          fl32 ty = y * scale_y + 0.5;

          // left, right, top and bottom sample boundaries
          auto x2 = static_cast<difference_type>(tx);
          auto x1 = x2 - 1;
          auto y2 = static_cast<difference_type>(ty);
          auto y1 = y2 - 1;

          // convert target coordinates to interpolation fractions
          // the +0.5 previously added means that these are correct
          tx -= x2;
          ty -= y2;

          // clamp samples to borders
          if (x1 < 0)
            x1 = 0;
          if (x2 >= static_cast<difference_type>(input.dims()[1]))
            x2 = input.dims()[1] - 1;
          if (y1 < 0)
            y1 = 0;
          if (y2 >= static_cast<difference_type>(input.dims()[0]))
            y2 = input.dims()[0] - 1;

          // interpolate in x direction first
          T r1 = tx * input[y1][x1] + (1 - tx) * input[y1][x2];
          T r2 = tx * input[y2][x1] + (1 - tx) * input[y2][x2];

          // now interpolate in y direction
          out[x] = ty * r1 + (1 - ty) * r2;
        }
      }
    }
    return output;
  }
}}

#endif
