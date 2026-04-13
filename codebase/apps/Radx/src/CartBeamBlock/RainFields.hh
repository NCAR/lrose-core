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
    angle(double val, bool isDegrees) {
      if (isDegrees) {
        set_degrees(val);
      } else {
        set_radians(val);
      }
    }

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
  inline auto is_nan<angle>(angle val) -> bool
  {
    return std::isnan(val.radians());
  }

  // overload the from_string function from string_utils.h
  template <>
  auto from_string<angle>(const char* str) -> angle;
}

#endif
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

#ifndef ARRAY_TEMPLATE_H
#define ARRAY_TEMPLATE_H

#include <cstring> // for memcpy
#include <iterator>
#include <memory>
#include <algorithm>

namespace rainfields {
  template <typename T>
  struct allocate_array
  {
    T* operator()(size_t count) const
    {
      return new T[count];
    }
  };

  template <typename T>
  void copy_array(const T* src, size_t count, T* dest)
  {
    std::copy(src, src + count, dest);
  }

  template <
      size_t Rank
    , typename T
    , class alloc = allocate_array<T>
    , class dealloc = std::default_delete<T[]>>
  class array;

  template <typename T, class alloc, class dealloc>
  class array<1, T, alloc, dealloc>
  {
  public:
    typedef size_t size_type;
    typedef T value_type;
    typedef T* iterator;
    typedef const T* const_iterator;
    typedef std::reverse_iterator<T*> reverse_iterator;
    typedef std::reverse_iterator<const T*> const_reverse_iterator;

  public:
    array() noexcept                                : size_(0) { }
    array(size_type size)                           : size_(size), data_(alloc{}(size_)) { }
    array(const size_type* dims)                    : size_(*dims), data_(alloc{}(size_)) { }

    /* we are forced to manually specify move constructors here because a bug in libstdc++
     * (for gcc 4.7.x) causes std::unique_ptr<T[]> fail the std::is_nothrow_move_constructible
     * type trait. */
    array(const array& rhs);
    array(array&& rhs) noexcept                     : size_(rhs.size_), data_(std::move(rhs.data_)) { }

    auto operator=(const array& rhs) -> array&;
    auto operator=(array&& rhs) noexcept -> array&
    {
      size_ = rhs.size_;
      data_ = std::move(rhs.data_);
      return *this;
    }
    
    /* we are forced to explicity provide a destructor because of a bug in libstdc++ for
     * gcc 4.7.x that causes std::unique_ptr<T[]> to have a noexcept(false) destructor! */
    virtual ~array() noexcept                       { }

    virtual auto rank() const -> size_t             { return 1; }

    auto size() const -> size_type                  { return size_; }
    auto dims() const -> const size_type*           { return &size_; }
    auto dim(size_type i) const -> size_type        { return size_; }
    auto operator[](size_type i) -> T&              { return data_[i]; }
    auto operator[](size_type i) const -> const T&  { return data_[i]; }

    auto begin() -> iterator                        { return data_.get(); }
    auto begin() const -> const_iterator            { return data_.get(); }
    auto end() -> iterator                          { return data_.get() + size_; }
    auto end() const -> const_iterator              { return data_.get() + size_; }
    auto rbegin() -> reverse_iterator               { return reverse_iterator(data_.get() + size_); }
    auto rbegin() const -> const_reverse_iterator   { return const_reverse_iterator(data_.get() + size_); }
    auto rend() -> reverse_iterator                 { return reverse_iterator(data_.get()); }
    auto rend() const -> const_reverse_iterator     { return const_reverse_iterator(data_.get()); }
    auto cbegin() const -> const_iterator           { return data_.get(); }
    auto cend() const -> const_iterator             { return data_.get() + size_; }
    auto crbegin() const -> const_reverse_iterator  { return const_reverse_iterator(data_.get() + size_); }
    auto crend() const -> const_reverse_iterator    { return const_reverse_iterator(data_.get()); }

    auto data() -> T*                               { return data_.get(); }
    auto data() const -> const T*                   { return data_.get(); }
    auto cdata() const -> const T*                  { return data_.get(); }

    auto resize(size_type size) -> void;

    /// Change array dimensions without resizing memory
    /**
     * This function exists for the sole purpose of being able to optimize cases where 
     * an array will need to be resized multiple times, but the maximum size required is
     * known in advance.  The array should be initially created with the maximum size, and
     * then this function called to fool the array into changing size without performing
     * any memory reallocation.
     *
     * \warning Calling this function with a size greater than the original memory allocation
     *          will not fail, and will silently cause memory corruption.  You have been
     *          warned!
     */
    auto hack_size(size_type val) -> void           { size_ = val; }

  protected:
    typedef std::unique_ptr<T[], dealloc> data_ptr;

  protected:
    size_type   size_;
    data_ptr    data_;
  };

  template <typename T, class alloc, class dealloc>
  array<1, T, alloc, dealloc>::array(const array& rhs)
    : size_(rhs.size_)
    , data_(alloc()(size_))
  {
    copy_array(rhs.data_.get(), size_, data_.get());
  }

  template <typename T, class alloc, class dealloc>
  auto array<1, T, alloc, dealloc>::resize(size_type size) -> void
  {
    if (size > size_)
      data_ = data_ptr{alloc{}(size)};
    size_ = size;
  }

  /* 
   * this function provides the strong exception safety guarantee only if either:
   * 1. T is a trivially copyable type
   * 2. T provides a noexcept copy constructor
   */
  template <typename T, class alloc, class dealloc>
  auto array<1, T, alloc, dealloc>::operator=(const array& rhs) -> array&
  {
    if (&rhs != this)
    {
      if (size_ != rhs.size_)
        data_.reset(alloc()(rhs.size_));

      size_ = rhs.size_;
      copy_array(rhs.data_.get(), size_, data_.get());
    }
    return *this;
  }

  template <typename T, class alloc, class dealloc>
  class array<2, T, alloc, dealloc> : public array<1, T, alloc, dealloc>
  {
  private:
    typedef array<1, T, alloc, dealloc> base;
    using base::data_;

  public:
    using typename base::value_type;
    using typename base::size_type;

  public:
    array() noexcept                               : dims_{0, 0} { }
    array(size_type y, size_type x)                : base(y * x), dims_{y, x} { }
    array(const size_type* dims)                   : base(dims[0] * dims[1]), dims_{dims[0], dims[1]} { }

    array(const array& rhs) = default;
    array(array&& rhs) noexcept = default;

    auto operator=(const array& rhs) -> array& = default;
    auto operator=(array&& rhs) noexcept -> array& = default;
    
    ~array() noexcept = default;

    auto rank() const -> size_t                     { return 2; }

    auto rows() const -> size_type                  { return dims_[0]; }
    auto cols() const -> size_type                  { return dims_[1]; }

    // these functions hide their equivalents from array
    // they are intentionally not virtual to allow array to be accessed as an array
    auto dims() const -> const size_type*           { return dims_; }
    auto dim(size_type i) const -> size_type        { return dims_[i]; }
    auto operator[](size_type y) -> T*              { return &data_[y * dims_[1]]; }
    auto operator[](size_type y) const -> const T*  { return &data_[y * dims_[1]]; }
    auto resize(size_type rows, size_type cols) -> void
    {
      base::resize(rows * cols);
      dims_[0] = rows;
      dims_[1] = cols;
    }
    auto hack_size(const size_type* dims) -> void
    {
      base::hack_size(dims[0] * dims[1]);
      dims_[0] = dims[0];
      dims_[1] = dims[1];
    }

  protected:
    size_type dims_[2];
  };

  // template aliases for the rank
  template <typename T, class alloc = allocate_array<T>, class dealloc = std::default_delete<T[]>>
  using array1 = array<1, T, alloc, dealloc>;
  template <typename T, class alloc = allocate_array<T>, class dealloc = std::default_delete<T[]>>
  using array2 = array<2, T, alloc, dealloc>;

}

#endif
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

#ifndef RAINUTIL_ARRAY_UTILS_H
#define RAINUTIL_ARRAY_UTILS_H

#include "array.h"
#include "real.h"
#include <stdexcept>

namespace rainfields {
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
  template <typename T>
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
      output.data()[i] = is_nan(input.data()[i]) ? val : input.data()[i];
    return output;
  }

  template <typename T>
  T& remove_nans(T& output, const T& input, const T& val)
  {
    if (   output.size() != input.size()
        || output.size() != val.size())
      throw std::logic_error("array size mismatch");
    for (typename T::size_type i = 0; i < output.size(); ++i)
      output.data()[i] = is_nan(input.data()[i]) ? val.data()[i] : input.data()[i];
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
          out[0] = (top[0] + top[1] + bot[0] + bot[1]) * 0.25f;
          top += 2;
          bot += 2;
          ++out;
        }
      }
    }
#if 0 // this isn't right - we should still interpolate i think...
    else if (   output.dims()[0] == input.dims()[0] * 2
             && output.dims()[1] == input.dims()[1] * 2)
    {
      // special case 3: perfect upscale (we are one level larger than input)
      const T* inp = input[0];
            T* top = output[0];
            T* bot = output[width_];
      for (typename array2<T>::size_type y = 0; y < height_; y += 2)
      {
        for (typename array2<T>::size_type x = 0; x < width_; x += 2)
        {
          top[0] = inp[0];
          top[1] = inp[0];
          bot[0] = inp[0];
          bot[1] = inp[0];
          top += 2;
          bot += 2;
          ++inp;
        }
        top = bot;
        bot += width_;
      }
    }
#endif
    else
    {
      // generic case: bi-linear interpolation (much slower)
      typedef typename std::make_signed<typename array2<T>::size_type>::type difference_type;
      float scale_x = static_cast<float>(input.dims()[1]) / output.dims()[1];
      float scale_y = static_cast<float>(input.dims()[0]) / output.dims()[0];
      for (typename array2<T>::size_type y = 0; y < output.dims()[0]; ++y)
      {
        T* out = output[y];
        for (typename array2<T>::size_type x = 0; x < output.dims()[1]; ++x)
        {
          // target x, y (the +0.5 is for rounding purposes)
          float tx = x * scale_x + 0.5;
          float ty = y * scale_y + 0.5;

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

#ifndef ANCILLA_MODELS_BEAM_POWER_H
#define ANCILLA_MODELS_BEAM_POWER_H

#include "angle.h"
#include "array.h"
#include "real.h"

namespace rainfields {
namespace ancilla {
  
  class beam_power
  {
  public:
    beam_power(angle beam_width_h, angle beam_width_v);

    auto beam_width_h() const -> angle                      { return beam_width_h_; }
    auto beam_width_v() const -> angle                      { return beam_width_v_; }

    auto calculate(angle theta_h, angle theta_v) const -> real;

  private:
    angle   beam_width_h_;
    angle   beam_width_v_;
    double  four_ln_two_;
    double  inv_h_sqr_;
    double  inv_v_sqr_;
  };

  /**
   * This class creates a rectangular array that represents a 2d cross-sectional view
   * of beam power.  The array is centered on the center (highest power) of the beam.
   *
   * The array is always normalized so that the total power in the array sums to 1.
   */
  class beam_power_cross_section
  {
  public:
    /**
     * \param beam    Model for the beam power
     * \param rows    Number of samples along beam vertically (rows in array)
     * \param cols    Number of samples along beam horizontally (columns in array)
     * \param height  Angular height of the array (6 = +/-3 from beam center)
     * \param width   Angular width of the array (6 = +/-3 from beam center)
     */
    beam_power_cross_section(
          const beam_power& beam
        , angle gate_width
        , size_t rows
        , size_t cols
        , angle height = 6.0_deg
        , angle width = 6.0_deg
        );

    /// Get the number of rows (elevation offsets) in array
    auto rows() const -> size_t                       { return data_.rows(); }

    /// Get the number of columns (azimuthal offsets) in array
    auto cols() const -> size_t                       { return data_.cols(); }

    /// Get the total angular height of the array
    auto height() const -> angle                      { return height_; }

    /// Get the total angular widht of the array
    auto width() const -> angle                       { return width_; }

    /// Transform the cross section to contain integrated 'power loss at height' values
    /**
     * After calling this function values within the array shall no longer represent beam
     * power at a particular offset from beam center.  Instead, the value returned at any
     * point represents the fraction of total beam power at that point and below.  This
     * can be used to determine fraction of signal loss due to terrain obstructions.
     */
    void make_vertical_integration();

    /// Get the vertical angular offset of cell centers from beam center at a particular row
    auto offset_elevation(size_t y) const -> angle    { return elevations_[y]; }

    /// Get the horizontal angular offset of cell centers from beam center at a particular column
    auto offset_azimuth(size_t x) const -> angle      { return azimuths_[x]; }

    /// Get the array value at a row, column
    auto power(size_t y, size_t x) const -> real      { return data_[y][x]; }

    /// Get direct access to the power array
    auto data() const -> const array2<real>&          { return data_; }

  private:
    angle         height_;
    angle         width_;
    array1<angle> elevations_;
    array1<angle> azimuths_;
    array2<real>  data_;
  };
}}

#endif
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

#ifndef DIGITAL_ELEVATION_H
#define DIGITAL_ELEVATION_H

#include "spheroid.h"
#include "angle.h"
#include "array.h"
#include "latlon.h"
#include "Params.hh"

#include <list>

namespace rainfields {
  namespace ancilla {
    
    class digital_elevation {

    public:
      class model_error;
  
    public:
  
      digital_elevation(const Params &params);
      virtual ~digital_elevation();
  
      /// Get the reference spheroid used for by this elevation model
      virtual auto reference_spheroid() -> const spheroid& = 0;
  
      /// Lookup the elevation above the reference spheroid at a given location
      virtual auto lookup(const latlon& loc) -> real = 0;
  
      /// Lookup the altitude at multiple locations
      virtual auto lookup(latlonalt* values, size_t count) -> void = 0;
  
      virtual auto testBOM(void) -> void = 0;
      virtual auto testFTG(void) -> void = 0;
  
    protected:
      const Params &_params;
  
    }; // digital_elevation

    class digital_elevation::model_error : public std::runtime_error {

    public:
      model_error(std::string description);
      model_error(std::string description, latlon location);
      
      auto description() const -> const std::string&  { return description_; }
      auto location() const -> const latlon&          { return location_; }
  
    private:
      std::string description_;
      latlon location_;

    };
    
    /// Shuttle Radar Topography Mission DEM (3 arc-second version)
    /**
     * Tiles freely available for downlow from USGS at 
     * http://dds.cr.usgs.gov/srtm/version2_1/SRTM3/
     */

    class srtm_tile
    {
    public:
      int lat, lon;
      int nlat, nlon;
      double dlat, dlon;
      array2<real> data;
    };

    class digital_elevation_srtm3 : public digital_elevation {

    public:
      digital_elevation_srtm3(const Params &params,
                              std::string path, size_t cache_size = 16);
  
      auto reference_spheroid() -> const spheroid&;
      auto lookup(const latlon& loc) -> real;
      auto lookup(latlonalt* values, size_t count) -> void;
  
      auto testBOM(void) -> void;
      auto testFTG(void) -> void;
      auto test(int lat0, int lon0, int lat1, int lon1, 
                int lat2, int lon2, int lat3, int lon3) -> void;
  
    private:
      static constexpr int    void_value = -32768;
  
    private:
      auto get_tile(int lat, int lon) -> const srtm_tile&;
  
    private:
      std::string     path_;
      size_t          cache_size_;
      spheroid        wgs84_;
      std::list<srtm_tile> tiles_;
  
    }; // digital_elevation_srtm3

    /// ESRI ASCII grid based DEM
    class digital_elevation_esri : public digital_elevation {

    public:
      digital_elevation_esri(
              const Params &params
              , const std::string& path
              , latlon sw
              , latlon ne
              , spheroid::standard reference_spheroid = spheroid::standard::wgs84);
      digital_elevation_esri(
              const Params &params
              , const std::string& path
              , latlon sw
              , latlon ne
              , spheroid reference_spheroid);
  
      auto reference_spheroid() -> const spheroid&;
      auto lookup(const latlon& loc) -> real;
      auto lookup(latlonalt* values, size_t count) -> void;
      auto testBOM(void) -> void {}
      auto testFTG(void) -> void {}
  
    private:
      spheroid      spheroid_;
      latlon        nw_;
      real          delta_deg_;
      array2<real>  data_;

    }; // digital_elevation_esri

  } // namespace ancilla

} // namespace rainfields

#endif
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

#ifndef ANCILLA_CORE_FIELD_H
#define ANCILLA_CORE_FIELD_H

#include "array.h"
#include "real.h"
#include <string>

namespace rainfields {
namespace ancilla {
  // nan with a special payload that indicates 'no data'
  template <typename T = real>
  inline constexpr auto nan_nodata() -> T
  {
    static_assert(
          std::is_floating_point<T>::value
        , "nan_nodata() instanciated on non floating point type");
    return nan<T>(25U);
  }

  template <typename T>
  constexpr auto is_nodata(T val) -> bool
  {
    static_assert(
          std::is_floating_point<T>::value
        , "is_nodata() instanciated on non floating point type");
    return is_nan(val) && nan_payload(val) == 25U;
  }

  // this version assumes you passed in a nan value (ie: you already checked is_nan())
  template <typename T>
  constexpr auto is_nan_nodata(T val) -> bool
  {
    static_assert(
          std::is_floating_point<T>::value
        , "is_nan_nodata() instanciated on non floating point type");
    return nan_payload(val) == 25U;
  }

  // nan with a special payload to indicate 'undetect'
  template <typename T = real>
  inline constexpr auto nan_undetect() -> T
  {
    static_assert(
          std::is_floating_point<T>::value
        , "nan_undetect() instanciated on non floating point type");
    return nan<T>(83U);
  }

  template <typename T>
  constexpr auto is_undetect(T val) -> bool
  {
    static_assert(
          std::is_floating_point<T>::value
        , "is_undetect() instanciated on non floating point type");
    return is_nan(val) && nan_payload(val) == 83U;
  }

  // this version assumes you passed in a nan value (ie: you already checked is_nan())
  template <typename T>
  constexpr auto is_nan_undetect(T val) -> bool
  {
    static_assert(
          std::is_floating_point<T>::value
        , "is_nan_undetect() instanciated on non floating point type");
    return nan_payload(val) == 83U;
  }

  class field
  {
  public:
    field() = default;
    field(std::string id);

    field(const field& rhs) = default;
    field(field&& rhs) noexcept = default;

    auto operator=(const field& rhs) -> field& = default;
    auto operator=(field&& rhs) noexcept -> field&;

    virtual ~field() noexcept = default;

    auto id() const -> const std::string&                   { return id_; }
    virtual auto set_id(const std::string& val) -> void;

  private:
    std::string id_;
  };

  class field1 : public field, public array1<real>
  {
  public:
    field1(std::string id, size_t size);
    field1(std::string id, const size_t dims[]);

    field1(const field1& rhs) = default;
    field1(field1&& rhs) noexcept = default;
    auto operator=(const field1& rhs) -> field1& = default;
    auto operator=(field1&& rhs) noexcept -> field1& = default;
    ~field1() noexcept = default;
  };

  class field2 : public field, public array2<real>
  {
  public:
    field2(std::string id, size_t y, size_t x);
    field2(std::string id, const size_t dims[]);

    field2(const field2& rhs) = default;
    field2(field2&& rhs) noexcept = default;
    auto operator=(const field2& rhs) -> field2& = default;
    auto operator=(field2&& rhs) noexcept -> field2& = default;
    ~field2() noexcept = default;
  };
}}

#endif
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

#ifndef RAINUTIL_LATLON_H
#define RAINUTIL_LATLON_H

#include "angle.h"
#include "string_utils.h"
#include <ostream>

namespace rainfields {

  struct latlon
  {
    angle lat, lon;

    latlon() noexcept  = default;
    latlon(angle lat, angle lon) noexcept : lat(lat), lon(lon) { }

    latlon(const latlon& rhs) noexcept = default;
    latlon(latlon&& rhs) noexcept = default;

    auto operator=(const latlon& rhs) noexcept -> latlon& = default;
    auto operator=(latlon&& rhs) noexcept -> latlon& = default;

    ~latlon() noexcept = default;
  };

  struct latlonalt : public latlon
  {
    double alt;

    latlonalt() noexcept  = default;
    latlonalt(angle lat, angle lon, double alt) noexcept : latlon{lat, lon}, alt{alt} { }
    latlonalt(latlon ll, double alt) noexcept : latlon{ll}, alt{alt} { }

    latlonalt(const latlonalt& rhs) noexcept = default;
    latlonalt(latlonalt&& rhs) noexcept = default;

    auto operator=(const latlonalt& rhs) noexcept -> latlonalt& = default;
    auto operator=(latlonalt&& rhs) noexcept -> latlonalt& = default;

    ~latlonalt() noexcept = default;
  };

  auto operator<<(std::ostream& lhs, const latlon& rhs) -> std::ostream&;
  auto operator<<(std::ostream& lhs, const latlonalt& rhs) -> std::ostream&;

  template <>
  auto from_string<latlon>(const char* str) -> latlon;
  template <>
  auto from_string<latlonalt>(const char* str) -> latlonalt;
}

#endif
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
  inline auto is_nan(T val) -> bool
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

#ifndef RAINUTIL_STRING_UTILS_H
#define RAINUTIL_STRING_UTILS_H

#include "traits.h"

#include <cstring>
#include <stdexcept>
#include <string>
#include <sstream>
#include <type_traits>
#include <vector>

namespace rainfields {

  class string_conversion_error : public std::runtime_error
  {
  public:
    string_conversion_error(std::string type);
    string_conversion_error(std::string type, std::string string);

    auto type() const -> const std::string&   { return type_; }
    auto string() const -> const std::string& { return string_; }
    auto from_string() const -> bool          { return from_string_; }

  private:
    std::string type_;
    std::string string_;
    bool        from_string_;
  };

  // user defined literal for std::string
  inline std::string operator"" _s(const char* str, size_t len) { return std::string(str, len); }

  bool is_numeric(const std::string& val);

  template <typename T>
  auto from_string(const char* str) -> typename std::enable_if<!std::is_enum<T>::value, T>::type;
  template <> auto from_string<bool>(const char* str) -> bool;
  template <> auto from_string<char>(const char* str) -> char;
  template <> auto from_string<signed char>(const char* str) -> signed char;
  template <> auto from_string<unsigned char>(const char* str) -> unsigned char;
  template <> auto from_string<short>(const char* str) -> short;
  template <> auto from_string<unsigned short>(const char* str) -> unsigned short;
  template <> auto from_string<int>(const char* str) -> int;
  template <> auto from_string<unsigned int>(const char* str) -> unsigned int;
  template <> auto from_string<long>(const char* str) -> long;
  template <> auto from_string<unsigned long>(const char* str) -> unsigned long;
  template <> auto from_string<long long>(const char* str) -> long long;
  template <> auto from_string<unsigned long long>(const char* str) -> unsigned long long;
  template <> auto from_string<float>(const char* str) -> float;
  template <> auto from_string<double>(const char* str) -> double;
  template <> auto from_string<long double>(const char* str) -> long double;
  template <> inline auto from_string<const char*>(const char* str) -> const char* { return str; }

  template <typename T>
  auto from_string(const char* str) -> typename std::enable_if<std::is_enum<T>::value, T>::type
  {
    static_assert(enum_traits<T>::name, "enum_traits not specialized for desired enum");
    static_assert(
          enum_traits<T>::count == (int) std::extent<decltype(enum_traits<T>::strings)>::value
        , "enum_traits count / label size mismatch");
    
    for (int i = 0; i < enum_traits<T>::count; ++i)
      if (strcmp(str, enum_traits<T>::strings[i]) == 0)
        return static_cast<T>(i);
    throw string_conversion_error(enum_traits<T>::name, str);
  }

  template <typename T>
  inline auto from_string(const std::string& str) -> T { return from_string<T>(str.c_str()); }

  auto to_string(bool val) -> std::string;
  auto to_string(char val) -> std::string;
  auto to_string(signed char val) -> std::string;
  auto to_string(unsigned char val) -> std::string;
  auto to_string(short val) -> std::string;
  auto to_string(unsigned short val) -> std::string;
  auto to_string(int val) -> std::string;
  auto to_string(unsigned int val) -> std::string;
  auto to_string(long val) -> std::string;
  auto to_string(unsigned long val) -> std::string;
  auto to_string(long long val) -> std::string;
  auto to_string(unsigned long long val) -> std::string;
  auto to_string(float val) -> std::string;
  auto to_string(double val) -> std::string;
  auto to_string(long double val) -> std::string;

  template <typename T>
  auto to_string(T val) -> typename std::enable_if<std::is_enum<T>::value, std::string>::type
  {
    static_assert(enum_traits<T>::name, "enum_traits not specialized for desired enum");
    static_assert(
          enum_traits<T>::count == (int) std::extent<decltype(enum_traits<T>::strings)>::value
        , "enum_traits count / label size mismatch");
    
    return enum_traits<T>::strings[static_cast<typename std::underlying_type<T>::type>(val)];
  }

  auto to_string(bool val, std::string& out) -> void;
  auto to_string(char val, std::string& out) -> void;
  auto to_string(signed char val, std::string& out) -> void;
  auto to_string(unsigned char val, std::string& out) -> void;
  auto to_string(short val, std::string& out) -> void;
  auto to_string(unsigned short val, std::string& out) -> void;
  auto to_string(int val, std::string& out) -> void;
  auto to_string(unsigned int val, std::string& out) -> void;
  auto to_string(long val, std::string& out) -> void;
  auto to_string(unsigned long val, std::string& out) -> void;
  auto to_string(long long val, std::string& out) -> void;
  auto to_string(unsigned long long val, std::string& out) -> void;
  auto to_string(float val, std::string& out) -> void;
  auto to_string(double val, std::string& out) -> void;
  auto to_string(long double val, std::string& out) -> void;

  template <typename T>
  auto to_string(T val, std::string& out) -> typename std::enable_if<std::is_enum<T>::value, std::string>::type
  {
    static_assert(enum_traits<T>::name, "enum_traits not specialized for desired enum");
    static_assert(
          enum_traits<T>::count == std::extent<decltype(enum_traits<T>::strings)>::value
        , "enum_traits count / label size mismatch");
    
    out = enum_traits<T>::strings[static_cast<typename std::underlying_type<T>::type>(val)];
  }

  /// Wrapper around a ostringstream to allow inline construction of a string using stream operators
  class msg
  {
  public:
    msg() = default;
    msg(const msg& rhs) = default;
    msg(msg&& rhs) = default;
    msg& operator=(const msg& rhs) = default;
    msg& operator=(msg&& rhs) = default;
    ~msg() = default;

    operator std::string() { return stream_.str(); }

    template <typename T>
    auto operator<<(const T& rhs) -> msg&
    {
      stream_ << rhs;
      return *this;
    }

    auto operator<<(std::ostream& (*rhs)(std::ostream& os)) -> msg&
    {
      stream_ << rhs;
      return *this;
    }

  private:
    std::ostringstream stream_;
  };

  /// Simple object to ease insertion of indentation to streams
  /** Use it as if it was an integer:
   *  {
   *    indent in;
   *    cout << in << "no indent here";
   *    cout << in + 4 << "this is indented by 4 spaces";
   *    indent_will_be_2_spaces_inside_here(obj, in + 2);
   *  }
   */
  class indent
  {
  public:
    constexpr indent() : spaces_(0) { }
    constexpr indent(int spaces) : spaces_(spaces) { }

    constexpr indent operator+(int spaces) const  { return {spaces_ + spaces}; }
    constexpr indent operator-(int spaces) const  { return {spaces_ - spaces}; }
    
    indent& operator+=(int spaces)                { spaces_ += spaces; return *this; }
    indent& operator-=(int spaces)                { spaces_ -= spaces; return *this; }

    constexpr int spaces() const                  { return spaces_; }

  private:
    int spaces_;
  };
  auto operator<<(std::ostream& os, const indent& in) -> std::ostream&;

  /// Tokenize a string into a vector of strings
  /**
   * \param str[in]     String to be tokenized
   * \param delims[in]  Character(s) used to delimit tokens within str
   * \return            Vector of extracted strings
   */
  auto tokenize(const std::string& str, const char* delims) -> std::vector<std::string>;
}

#endif
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

#ifndef RAINUTIL_TRACE_H
#define RAINUTIL_TRACE_H

#include "traits.h"

#include <mutex>
#include <ostream>
#include <sstream>

namespace rainfields {
namespace trace {

  /// trace severity levels
  enum class level
  {
      none
    , status
    , error
    , warning
    , log
    , debug
  };

  /// temporary object which manages the lifetime of a single chained trace output expression
  class session
  {
  public:
    session(level lvl);

    session(const session& rhs) = delete;
    session(session&& rhs) = default;
    auto operator=(const session& rhs) -> session& = delete;
    auto operator=(session&& rhs) -> session& = default;

    ~session()
    {
      if (echo_on_)
        *stream_ << std::endl;
    }

    operator std::ostream&() const
    {
      return *stream_;
    }

  private:
    std::unique_lock<std::recursive_mutex>  lock_;
    std::ostream* stream_;
    bool          echo_on_;

    // these will short-circuit the actual serialization when possible
    template <typename T>
    friend auto operator<<(const session& lhs, const T& rhs) -> const session&
    {
      if (lhs.echo_on_)
        *lhs.stream_ << rhs;
      return lhs;
    }
    friend auto operator<<(const session& lhs, std::ostream& (*rhs)(std::ostream&)) -> const session&
    {
      if (lhs.echo_on_)
        rhs(*lhs.stream_);
      return lhs;
    }
  };

  auto min_level() -> level;
  auto set_min_level(level min) -> void;

  auto target() -> std::ostream&;
  auto set_target(std::ostream& target) -> void;

  auto identify_threads() -> bool;
  auto set_identify_threads(bool identify) -> void;

  inline auto status() -> session        { return level::status; }
  inline auto error() -> session         { return level::error; }
  inline auto warning() -> session       { return level::warning; }
  inline auto log() -> session           { return level::log; }
  inline auto debug() -> session         { return level::debug; }
}}

// TODO - move this to a separate header?
namespace rainfields {
  /// print an exception including possibly nested ones
  auto format_exception(const std::exception& err, const char* prefix = "") -> std::string;
}

namespace rainfields {
  template <>
  struct enum_traits<trace::level>
  {
    static constexpr const char* name = "trace::level";
    static constexpr int count = 6;
    static constexpr const char* strings[] =
    {
        "none"
      , "status"
      , "error"
      , "warning"
      , "log"
      , "debug"
    };
  };
}

#endif
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

#ifndef RAINUTIL_TRAITS_H
#define RAINUTIL_TRAITS_H

#include <cstddef>
#include <type_traits>
#include <utility>

namespace rainfields
{
  /// get the value of the sizeof() operator for type T
  template <typename T>
  struct size_of : std::integral_constant<decltype(sizeof(T)), sizeof(T)>
  { };

  /// get type of first type in parameter pack
  template <typename F, typename... T>
  struct first_type{ typedef F type; };

  /// get the index of the first occurence of a type in a parameter pack
  template <typename S, typename F, typename... T>
  struct type_index : std::integral_constant<int, type_index<S, T...>::value + 1>
  { };
  template <typename S, typename... T>
  struct type_index<S, S, T...> : std::integral_constant<int, 0>
  { };

  /// trait used determine the minimum integral value of a trait applied to multiple types
  template <template<class> class Trait, typename F, typename... T>
  struct trait_min : std::integral_constant<decltype(Trait<F>::value), (Trait<F>::value < trait_min<Trait, T...>::value) ? Trait<F>::value : trait_min<Trait, T...>::value>
  { };
  template <template<class> class Trait, typename F>
  struct trait_min<Trait, F> : std::integral_constant<decltype(Trait<F>::value), Trait<F>::value>
  { };

  /// trait used determine the maximum integral value of a trait applied to multiple types
  template <template<class> class Trait, typename F, typename... T>
  struct trait_max : std::integral_constant<decltype(Trait<F>::value), (Trait<F>::value > trait_max<Trait, T...>::value) ? Trait<F>::value : trait_max<Trait, T...>::value>
  { };
  template <template<class> class Trait, typename F>
  struct trait_max<Trait, F> : std::integral_constant<decltype(Trait<F>::value), Trait<F>::value>
  { };

  /// true if given trait (Trait) is true when applied to every one of the supplied type (T...)
  template <template<class> class Trait, typename F, typename... T>
  struct trait_all : std::integral_constant<bool, Trait<F>::value && trait_all<Trait, T...>::value>
  { };
  template <template<class> class Trait, typename F>
  struct trait_all<Trait, F> : std::integral_constant<bool, Trait<F>::value>
  { };

  /// true if given trait (Trait) is false when applied to every one of the supplied types (T...)
  template <template<class> class Trait, typename F, typename... T>
  struct trait_none : std::integral_constant<bool, !Trait<F>::value && trait_none<Trait, T...>::value>
  { };
  template <template<class> class Trait, typename F>
  struct trait_none<Trait, F> : std::integral_constant<bool, !Trait<F>::value>
  { };

  /// true if given trait (Trait) is true when applied to any of the supplied types (T...)
  template <template<class> class Trait, typename F, typename... T>
  struct trait_any : std::integral_constant<bool, Trait<F>::value || trait_any<Trait, T...>::value>
  { };
  template <template<class> class Trait, typename F>
  struct trait_any<Trait, F> : std::integral_constant<bool, Trait<F>::value>
  { };

  /// trait used to serialize enumerations (requires specialization)
  template <typename T>
  struct enum_traits
  {
    static_assert(std::is_enum<T>::value, "attempt to instanciate enum_traits on non-enum type");

    static constexpr const char* name = nullptr;
    static constexpr int count = 0;
    static constexpr const char* strings[] = { "" };
  };

  namespace detail
  {
    // used by is_instance_of
    template <template <typename...> class F>
    struct conversion_tester
    {
      template <typename... Args>
      conversion_tester(const F<Args...>&);
    };

    template <typename T>
    struct void_type{ typedef void type; };
  }

  /// determine if a class is an instance of a template
  template <class C, template <typename...> class T>
  struct is_instance_of
  {
    static const bool value 
      = std::is_convertible<C, detail::conversion_tester<T>>::value;
  };

  /// determine the most 'inner' value_type typedef of a class
  template <class T, typename = void>
  struct inner_value_type
  {
    typedef T type;
  };
  template <class T>
  struct inner_value_type<T, typename detail::void_type<typename T::value_type>::type>
  {
    typedef typename inner_value_type<typename T::value_type>::type type;
  };

  /// get the 'rank' value of a class or 1 if a scalar type is used
#if 0
  template <typename T, bool = std::is_scalar<T>::value>
  struct rank_or_1
  {
    static constexpr size_t value = 1;
  };
  template <typename T>
  struct rank_or_1<T, false>
  {
    static constexpr size_t value = T::rank;
  };
#endif
  template <typename T, typename = void>
  struct rank_or_1
  {
    static constexpr size_t value = 1;
  };
  template <typename T>
  struct rank_or_1<T, decltype(T::rank)>
  {
    static constexpr size_t value = T::rank;
  };

  /// generic typelist implementation
  template <typename...>
  struct typelist;

  template <>
  struct typelist<>
  {
    typedef std::true_type empty;
  };

  template <typename First, typename... Rest>
  struct typelist<First, Rest...>
  {
    typedef std::false_type empty;

    struct first
    {
      typedef First type;
    };

    struct rest
    {
      typedef typelist<Rest...> type;
    };
  };

  /// determine whether a type is a member of a typelist
  template <typename List, typename T, bool empty = List::empty::value>
  struct typelist_contains : std::integral_constant<bool, std::is_same<typename List::first::type, T>::value ? true : typelist_contains<typename List::rest::type, T>::value>
  { };
  template <typename List, typename T>
  struct typelist_contains<List, T, true> : std::false_type
  { };

  /// function object used to call the subscript operator on an object
  struct subscript
  {
    template <typename T, typename K>
    inline auto operator()(T&& obj, K&& key) const -> decltype(std::forward<T>(obj)[std::forward<K>(key)])
    {
      return std::forward<T>(obj)[std::forward<K>(key)];
    }
  };

  /// generic enumerate with the specified number of members
  template <int count> struct generic_enum { };
  template <> struct generic_enum<1>  { enum type { _0 }; };
  template <> struct generic_enum<2>  { enum type { _0, _1 }; };
  template <> struct generic_enum<3>  { enum type { _0, _1, _2 }; };
  template <> struct generic_enum<4>  { enum type { _0, _1, _2, _3 }; };
  template <> struct generic_enum<5>  { enum type { _0, _1, _2, _3, _4 }; };
  template <> struct generic_enum<6>  { enum type { _0, _1, _2, _3, _4, _5 }; };
  template <> struct generic_enum<7>  { enum type { _0, _1, _2, _3, _4, _5, _6 }; };
  template <> struct generic_enum<8>  { enum type { _0, _1, _2, _3, _4, _5, _6, _7 }; };
  template <> struct generic_enum<9>  { enum type { _0, _1, _2, _3, _4, _5, _6, _7, _8 }; };
  template <> struct generic_enum<10> { enum type { _0, _1, _2, _3, _4, _5, _6, _7, _8, _9 }; };
}

#endif
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

#ifndef RAINUTIL_XML_H
#define RAINUTIL_XML_H

#include "string_utils.h"

#include <istream>
#include <iterator>
#include <exception>
#include <ostream>
#include <string>
#include <type_traits>
#include <unordered_set>
#include <vector>

namespace rainfields {
namespace xml {

  class node;
  class attribute;

  typedef std::unordered_set<std::string> string_store;

  class parse_error : public std::exception
  {
  public:
    virtual auto what() const noexcept -> const char*;

    auto description() const -> const char*         { return desc_; }
    auto line() const -> int                        { return line_; }
    auto col() const -> int                         { return col_; }

  private:
    parse_error(const char* desc, const char* where);

    auto determine_line_col(const char* src) -> void;

    const char*   desc_;
    const char*   where_;
    int           line_;
    int           col_;
    mutable char  what_[128];

    friend class attribute;
    friend class node;
    friend class document;
    friend auto skip_pi(const char*& src) -> void;
    friend auto skip_comment(const char*& src) -> void;
    friend auto skip_doctype(const char*& src) -> void;
    friend auto resolve_entities(std::string& src, const char* where_end) -> void;
  };

  class path_error : public std::exception
  {
  public:
    path_error(const char* name, bool is_attribute);

    virtual auto what() const noexcept -> const char*;

    auto name() const -> const std::string&         { return name_; }
    auto is_attribute() const -> bool               { return is_attribute_; }

  private:
    std::string         name_;
    bool                is_attribute_;
    mutable std::string description_;
  };

  class attribute_store : private std::vector<attribute>
  {
  private:
    typedef std::vector<attribute> base;

  public:
    typedef base::iterator iterator;
    typedef base::const_iterator const_iterator;
    typedef base::reverse_iterator reverse_iterator;
    typedef base::const_reverse_iterator const_reverse_iterator;

  public:
    attribute_store(const attribute_store& rhs) = delete;
    attribute_store(attribute_store&& rhs) noexcept;

    auto operator=(const attribute_store& rhs) -> attribute_store& = delete;
    auto operator=(attribute_store&& rhs) noexcept -> attribute_store&;

    ~attribute_store() noexcept = default;

    using base::begin;
    using base::end;
    using base::rbegin;
    using base::rend;
    using base::cbegin;
    using base::cend;
    using base::crbegin;
    using base::crend;

    using base::size;
    using base::empty;
    using base::reserve;

    using base::operator[];
    auto operator[](const char* name) -> attribute&;
    auto operator[](const char* name) const -> const attribute&;
    auto operator[](const std::string& name) -> attribute&;
    auto operator[](const std::string& name) const -> const attribute&;

    auto find(const char* name) -> iterator;
    auto find(const char* name) const -> const_iterator;
    auto find(const std::string& name) -> iterator;
    auto find(const std::string& name) const -> const_iterator;

    auto insert(const std::string& name) -> iterator;
    auto insert(std::string&& name) -> iterator;

    auto find_or_insert(const char* name) -> iterator;
    auto find_or_insert(const std::string& name) -> iterator;

    using base::erase;
    using base::clear;

  private:
    attribute_store(string_store* strings) : strings_(strings) { }

  private:
    string_store* strings_;

    friend class node;
    friend class node_store;
  };

  class node_store : private std::vector<node>
  {
  private:
    typedef std::vector<node> base;

  public:
    node_store(const node_store& rhs) = delete;
    node_store(node_store&& rhs) noexcept;

    auto operator=(const node_store& rhs) -> node_store& = delete;
    auto operator=(node_store&& rhs) noexcept -> node_store&;

    ~node_store() noexcept = default;

    using base::begin;
    using base::end;
    using base::rbegin;
    using base::rend;
    using base::cbegin;
    using base::cend;
    using base::crbegin;
    using base::crend;

    using base::size;
    using base::empty;
    using base::reserve;

    using base::operator[];
    using base::front;
    using base::back;

    using base::clear;

    auto operator[](const char* name) -> node&;
    auto operator[](const char* name) const -> const node&;
    auto operator[](const std::string& name) -> node&;
    auto operator[](const std::string& name) const -> const node&;

    auto find(const char* name) -> iterator;
    auto find(const char* name) const -> const_iterator;
    auto find(const std::string& name) -> iterator;
    auto find(const std::string& name) const -> const_iterator;

    auto push_back(const std::string& name) -> iterator;
    auto push_back(std::string&& name) -> iterator;
    using base::pop_back;

    auto insert(iterator position, const std::string& name) -> iterator;
    auto insert(iterator position, std::string&& name) -> iterator;
    using base::erase;

    auto find_or_insert(const char* name) -> iterator;
    auto find_or_insert(const std::string& name) -> iterator;

  private:
    node_store(node* owner) : owner_(owner) { }

  private:
    node* owner_;

    friend class node;
  };

  class attribute
  {
  public:
    attribute(const attribute& rhs) = delete;
    attribute(attribute&& rhs) noexcept = default;

    auto operator=(const attribute& rhs) -> attribute& = delete;
    auto operator=(attribute&& rhs) noexcept -> attribute& = default;

    ~attribute() noexcept = default;

    auto name() const -> const std::string&           { return *name_; }
    auto set_name(const std::string& val) -> void;

    auto get() const -> const std::string&            { return *value_; }
    auto set(const std::string& val) -> void;

    // allow conversion to all the primitive types (including traited enums)
    operator bool() const                             { return from_string<bool>(*value_); }
    operator char() const                             { return from_string<char>(*value_); }
    operator signed char() const                      { return from_string<signed char>(*value_); }
    operator unsigned char() const                    { return from_string<unsigned char>(*value_); }
    operator short() const                            { return from_string<short>(*value_); }
    operator unsigned short() const                   { return from_string<unsigned short>(*value_); }
    operator int() const                              { return from_string<int>(*value_); }
    operator unsigned int() const                     { return from_string<unsigned int>(*value_); }
    operator long() const                             { return from_string<long>(*value_); }
    operator unsigned long() const                    { return from_string<unsigned long>(*value_); }
    operator long long() const                        { return from_string<long long>(*value_); }
    operator unsigned long long() const               { return from_string<unsigned long long>(*value_); }
    operator float() const                            { return from_string<float>(*value_); }
    operator double() const                           { return from_string<double>(*value_); }
    operator long double() const                      { return from_string<long double>(*value_); }
    operator const std::string&() const               { return *value_; }
    template <typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type >
    operator T() const
    {
      return from_string<T>(*value_);
    }

    auto set(bool val) -> void                        { set(to_string(val)); }
    auto set(char val) -> void                        { set(to_string(val)); }
    auto set(signed char val) -> void                 { set(to_string(val)); }
    auto set(unsigned char val) -> void               { set(to_string(val)); }
    auto set(short val) -> void                       { set(to_string(val)); }
    auto set(unsigned short val) -> void              { set(to_string(val)); }
    auto set(int val) -> void                         { set(to_string(val)); }
    auto set(unsigned int val) -> void                { set(to_string(val)); }
    auto set(long val) -> void                        { set(to_string(val)); }
    auto set(unsigned long val) -> void               { set(to_string(val)); }
    auto set(long long val) -> void                   { set(to_string(val)); }
    auto set(unsigned long long val) -> void          { set(to_string(val)); }
    auto set(float val) -> void                       { set(to_string(val)); }
    auto set(double val) -> void                      { set(to_string(val)); }
    auto set(long double val) -> void                 { set(to_string(val)); }
    auto set(const char* val) -> void                 { set(std::string(val)); }
    template <typename T>
    auto set(T val) -> typename std::enable_if<std::is_enum<T>::value, void>::type
    {
      set(to_string(val));
    }

  private:
    // this allows us to use a constructor in a container's emplace methods while 
    // still ensuring that it uncallable (private) from the user's point of view
    struct private_constructor { };

  public:
    attribute(
          string_store* strings
        , const char*& src
        , std::string& tmp
        , private_constructor);
    attribute(
          string_store* strings
        , const attribute& rhs
        , private_constructor);
    attribute(
          string_store* strings
        , const std::string* name
        , private_constructor);

    auto write(std::ostream& out) const -> void;

  private:
    string_store*       strings_;
    const std::string*  name_;
    const std::string*  value_;

    friend class node;
    friend class attribute_store;
  };

  /* mixed mode elements are emulated using elements with an empty name */
  class node
  {
  public:
    enum class type
    {
        element_simple      //!< empty, or simple text contents element
      , element_compound    //!< element containing children (elements and/or text)
      , text_block          //!< text only block (child of a mixed element)
    };

  public:
    node(const node& rhs) = delete;
    node(node&& rhs) noexcept;

    auto operator=(const node& rhs) -> node& = delete;
    auto operator=(node&& rhs) noexcept -> node&;

    ~node() noexcept = default;

    auto write(std::ostream& out) const -> void;

    auto node_type() const -> enum type;

    auto name() const -> const std::string&                             { return *name_; }
    auto set_name(const std::string& val) -> void;

    // attribute access: only valid for 'element_simple' and 'element_compund' nodes
    auto attributes() -> attribute_store&                               { return attributes_; }
    auto attributes() const -> const attribute_store&                   { return attributes_; }
    auto operator()(const char* name) -> attribute&                     { return attributes_[name]; }
    auto operator()(const char* name) const -> const attribute&         { return attributes_[name]; }
    auto operator()(const std::string& name) -> attribute&              { return attributes_[name]; }
    auto operator()(const std::string& name) const -> const attribute&  { return attributes_[name]; }
    // optional attribute access:
    auto operator()(const char* name, bool def) const -> bool;
    auto operator()(const char* name, char def) const -> char;
    auto operator()(const char* name, signed char def) const -> signed char;
    auto operator()(const char* name, unsigned char def) const -> unsigned char;
    auto operator()(const char* name, short def) const -> short;
    auto operator()(const char* name, unsigned short def) const -> unsigned short;
    auto operator()(const char* name, int def) const -> int;
    auto operator()(const char* name, unsigned int def) const -> unsigned int;
    auto operator()(const char* name, long def) const -> long;
    auto operator()(const char* name, unsigned long def) const -> unsigned long;
    auto operator()(const char* name, long long def) const -> long long;
    auto operator()(const char* name, unsigned long long def) const -> unsigned long long;
    auto operator()(const char* name, float def) const -> float;
    auto operator()(const char* name, double def) const -> double;
    auto operator()(const char* name, long double def) const -> long double;
    auto operator()(const char* name, const char* def) const -> const char*;
    template <typename T>
    auto operator()(const char* name, T def) const -> typename std::enable_if<std::is_enum<T>::value, T>::type;

    // child access: only valid for 'element_compound' nodes
    auto children() -> node_store&                                      { return children_; }
    auto children() const -> const node_store&                          { return children_; }
    auto operator[](const char* name) -> node&                          { return children_[name]; }
    auto operator[](const char* name) const -> const node&              { return children_[name]; }
    auto operator[](const std::string& name) -> node&                   { return children_[name]; }
    auto operator[](const std::string& name) const -> const node&       { return children_[name]; }

    auto get() const -> const std::string&                              { return *value_; }
    auto set(const std::string& val) -> void;

    operator bool() const                                               { return from_string<bool>(*value_); }
    operator char() const                                               { return from_string<char>(*value_); }
    operator signed char() const                                        { return from_string<signed char>(*value_); }
    operator unsigned char() const                                      { return from_string<unsigned char>(*value_); }
    operator short() const                                              { return from_string<short>(*value_); }
    operator unsigned short() const                                     { return from_string<unsigned short>(*value_); }
    operator int() const                                                { return from_string<int>(*value_); }
    operator unsigned int() const                                       { return from_string<unsigned int>(*value_); }
    operator long() const                                               { return from_string<long>(*value_); }
    operator unsigned long() const                                      { return from_string<unsigned long>(*value_); }
    operator long long() const                                          { return from_string<long long>(*value_); }
    operator unsigned long long() const                                 { return from_string<unsigned long long>(*value_); }
    operator float() const                                              { return from_string<float>(*value_); }
    operator double() const                                             { return from_string<double>(*value_); }
    operator long double() const                                        { return from_string<long double>(*value_); }
    operator const std::string&() const                                 { return *value_; }
    template <typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type >
    operator T() const
    {
      return from_string<T>(*value_);
    }

    auto set(bool val) -> void                                          { set(to_string(val)); }
    auto set(char val) -> void                                          { set(to_string(val)); }
    auto set(signed char val) -> void                                   { set(to_string(val)); }
    auto set(unsigned char val) -> void                                 { set(to_string(val)); }
    auto set(short val) -> void                                         { set(to_string(val)); }
    auto set(unsigned short val) -> void                                { set(to_string(val)); }
    auto set(int val) -> void                                           { set(to_string(val)); }
    auto set(unsigned int val) -> void                                  { set(to_string(val)); }
    auto set(long val) -> void                                          { set(to_string(val)); }
    auto set(unsigned long val) -> void                                 { set(to_string(val)); }
    auto set(long long val) -> void                                     { set(to_string(val)); }
    auto set(unsigned long long val) -> void                            { set(to_string(val)); }
    auto set(float val) -> void                                         { set(to_string(val)); }
    auto set(double val) -> void                                        { set(to_string(val)); }
    auto set(long double val) -> void                                   { set(to_string(val)); }
    auto set(const char* val) -> void                                   { set(std::string(val)); }
    template <typename T>
    auto set(T val) -> typename std::enable_if<std::is_enum<T>::value, void>::type
    {
      set(to_string(val));
    }

  private:
    // this allows us to use a constructor in a container's emplace methods while 
    // still ensuring that it uncallable (private) from the user's point of view
    struct private_constructor { };

  public:
    node(private_constructor);
    node(
          string_store* strings
        , const std::string* value
        , private_constructor);
    node(
          string_store* strings
        , const char*& src
        , std::string& tmp
        , private_constructor);
    node(
          string_store* strings
        , const node& rhs
        , private_constructor);
    node(
          string_store* strings
        , const std::string* name
        , bool // dummy flag to distinguish constructor
        , private_constructor);

  private:
    attribute_store     attributes_;  // contains a pointer to the strings store
    const std::string*  name_;
    const std::string*  value_;       // only meaningful if a text node
    node_store          children_;

    friend class document;
    friend class node_store;
  };

  template <typename T>
  auto node::operator()(const char* name, T def) const -> typename std::enable_if<std::is_enum<T>::value, T>::type
  {
    auto i = attributes_.find(name);
    return i != attributes_.end() ? static_cast<T>(*i) : def;
  }

  class document
  {
  public:
    /// create an empty document with an empty root node
    document();
    /// parse a document from an input stream
    document(std::istream& source);
    /// parse a document from an input stream
    document(std::istream&& source);
    /// parse a document from a string
    document(const char* source);
    /// create a document by copying a node from another document as the root
    document(const xml::node& root);

    document(const document& rhs);
    document(document&& rhs) noexcept;

    auto operator=(const document& rhs) -> document&;
    auto operator=(document&& rhs) noexcept -> document&;

    ~document() noexcept = default;

    auto write(std::ostream& out) -> void;
    auto write(std::ostream&& out) -> void;

    auto root() -> node&             { return root_; }
    auto root() const -> const node& { return root_; }

  private:
    string_store  strings_;
    node          root_;
  };
}}

#endif
