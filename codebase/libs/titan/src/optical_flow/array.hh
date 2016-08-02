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

#ifndef OPTFLOW_ARRAY_HH
#define OPTFLOW_ARRAY_HH

//#include "traits.h"

#include <cstring> // for memcpy
#include <iterator>
#include <memory>

namespace titan {
  template <typename T>
  struct allocate_array
  {
    T* operator()(size_t count) const
    {
      return new T[count];
    }
  };

  // TODO - should use is_trivially_copyable<T> when it becomes available
  template <typename T>
  typename std::enable_if<!std::has_trivial_copy_constructor<T>::value, void>::type
  copy_array(const T* src, size_t count, T* dest)
  {
    std::copy(src, src + count, dest);
  }

  template <typename T>
  typename std::enable_if<std::has_trivial_copy_constructor<T>::value, void>::type
  copy_array(const T* src, size_t count, T* dest)
  {
    std::memcpy(dest, src, sizeof(T) * count);
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
