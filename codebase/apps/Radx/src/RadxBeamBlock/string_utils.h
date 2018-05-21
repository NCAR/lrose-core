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
