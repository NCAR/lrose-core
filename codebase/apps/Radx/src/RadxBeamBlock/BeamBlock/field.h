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

#include "metadata.h"
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

  class field : public metadata
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

  protected:
    auto meta_proxy(size_t i) const -> const proxy*;

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
