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

#include "string_utils.h"

#include <limits>
#include <cstdlib>
#include <strings.h>

using namespace rainfields;

string_conversion_error::string_conversion_error(std::string type)
  : std::runtime_error(msg{} << "to_string error (" << type << ")")
  , type_(std::move(type))
  , from_string_(false)
{

}

string_conversion_error::string_conversion_error(std::string type, std::string string)
  : std::runtime_error(msg{} << "from_string error (" << type << ", \"" << string << "\")")
  , type_(std::move(type))
  , string_(std::move(string))
  , from_string_(true)
{

}

bool rainfields::is_numeric(const std::string& val)
{
  return 
       !val.empty() 
    && (   isdigit(val[0])
        || (   val.size() > 1
            && val[0] == '-'
            && isdigit(val[1]))
        || (   val.size() == 3
            && strcasecmp(val.c_str(), "nan") == 0));
}

template <>
auto rainfields::from_string<bool>(const char* str) -> bool
{
  if (strcasecmp(str, "true") == 0)
    return true;
  if (strcasecmp(str, "false") == 0)
    return false;
  throw string_conversion_error("bool", str);
}

#define IMPL_FROMSTRING_I(TYPE, CONV) \
  template <> \
  auto rainfields::from_string<TYPE>(const char* str) -> TYPE \
  { \
    char *end; \
    errno = 0; \
    const TYPE ret = static_cast<TYPE>(CONV(str, &end, 0)); \
    if (str == end || errno) \
      throw string_conversion_error(#TYPE, str); \
    return ret; \
  }
    
#define IMPL_FROMSTRING_F(TYPE, CONV) \
  template <> \
  auto rainfields::from_string<TYPE>(const char* str) -> TYPE \
  { \
    char *end; \
    errno = 0; \
    const TYPE ret = static_cast<TYPE>(CONV(str, &end)); \
    if (str == end || errno) \
      throw string_conversion_error(#TYPE, str); \
    return ret; \
  }

IMPL_FROMSTRING_I(char, strtol);
IMPL_FROMSTRING_I(signed char, strtol);
IMPL_FROMSTRING_I(unsigned char, strtoul);
IMPL_FROMSTRING_I(short, strtol);
IMPL_FROMSTRING_I(unsigned short, strtoul);
IMPL_FROMSTRING_I(int, strtol);
IMPL_FROMSTRING_I(unsigned int, strtoul);
IMPL_FROMSTRING_I(long, strtol);
IMPL_FROMSTRING_I(unsigned long, strtoul);
IMPL_FROMSTRING_I(long long, strtoll);
IMPL_FROMSTRING_I(unsigned long long, strtoull);
IMPL_FROMSTRING_F(float, strtod);
IMPL_FROMSTRING_F(double, strtod);
IMPL_FROMSTRING_F(long double, strtold);

auto rainfields::to_string(bool val) -> std::string
{
  return val ? "true" : "false";
}

#define IMPL_TOSTRING_1(TYPE, BYTES, FMT) \
  auto rainfields::to_string(TYPE val) -> std::string \
  { \
    char buf[BYTES]; \
    const int len = snprintf(buf, BYTES, FMT, val); \
    if (len >= static_cast<int>(BYTES)) \
      throw string_conversion_error(#TYPE); \
    return std::string(buf, buf + len); \
  }

IMPL_TOSTRING_1(char, 4 * sizeof(char), "%hhd");
IMPL_TOSTRING_1(signed char, 4 * sizeof(signed char), "%hhd");
IMPL_TOSTRING_1(unsigned char, 4 * sizeof(unsigned char), "%hhu");
IMPL_TOSTRING_1(short, 4 * sizeof(short), "%hd");
IMPL_TOSTRING_1(unsigned short, 4 * sizeof(unsigned short), "%hu");
IMPL_TOSTRING_1(int, 4 * sizeof(int), "%d");
IMPL_TOSTRING_1(unsigned int, 4 * sizeof(unsigned int), "%u");
IMPL_TOSTRING_1(long, 4 * sizeof(long), "%ld");
IMPL_TOSTRING_1(unsigned long, 4 * sizeof(unsigned long), "%lu");
IMPL_TOSTRING_1(long long, 4 * sizeof(long long), "%lld");
IMPL_TOSTRING_1(unsigned long long, 4 * sizeof(unsigned long long), "%llu");
IMPL_TOSTRING_1(float, std::numeric_limits<float>::max_exponent10 + 20, "%f");
IMPL_TOSTRING_1(double, std::numeric_limits<double>::max_exponent10 + 20, "%f");
IMPL_TOSTRING_1(long double, std::numeric_limits<long double>::max_exponent10 + 20, "%Lf");

auto rainfields::to_string(bool val, std::string& out) -> void
{
  if (val)
    out = "true";
  else
    out = "false";
}

#define IMPL_TOSTRING_2(TYPE, BYTES, FMT) \
  auto rainfields::to_string(TYPE val, std::string& out) -> void \
  { \
    char buf[BYTES]; \
    const int len = snprintf(buf, BYTES, FMT, val); \
    if (len >= static_cast<int>(BYTES)) \
      throw string_conversion_error(#TYPE); \
    out.assign(buf, len); \
  }

IMPL_TOSTRING_2(char, 4 * sizeof(char), "%hhd");
IMPL_TOSTRING_2(signed char, 4 * sizeof(char), "%hhd");
IMPL_TOSTRING_2(unsigned char, 4 * sizeof(unsigned char), "%hhu");
IMPL_TOSTRING_2(short, 4 * sizeof(short), "%hd");
IMPL_TOSTRING_2(unsigned short, 4 * sizeof(unsigned short), "%hu");
IMPL_TOSTRING_2(int, 4 * sizeof(int), "%d");
IMPL_TOSTRING_2(unsigned int, 4 * sizeof(unsigned int), "%u");
IMPL_TOSTRING_2(long, 4 * sizeof(long), "%ld");
IMPL_TOSTRING_2(unsigned long, 4 * sizeof(unsigned long), "%lu");
IMPL_TOSTRING_2(long long, 4 * sizeof(long long), "%lld");
IMPL_TOSTRING_2(unsigned long long, 4 * sizeof(unsigned long long), "%llu");
IMPL_TOSTRING_2(float, std::numeric_limits<float>::max_exponent10 + 20, "%f");
IMPL_TOSTRING_2(double, std::numeric_limits<double>::max_exponent10 + 20, "%f");
IMPL_TOSTRING_2(long double, std::numeric_limits<long double>::max_exponent10 + 20, "%Lf");

auto rainfields::operator<<(std::ostream& os, const indent& in) -> std::ostream&
{
  for (int i = 0; i < in.spaces(); ++i)
    os.put(' ');
  return os;
}

auto rainfields::tokenize(const std::string& str, const char* delims) -> std::vector<std::string>
{
  std::vector<std::string> ret;
  size_t pos = 0;
  while ((pos = str.find_first_not_of(delims, pos)) != std::string::npos)
  {
    size_t end = str.find_first_of(delims, pos + 1);
    size_t len = (end == std::string::npos) ? end : end - pos;
    ret.emplace_back(str, pos, len);
  }
  return std::move(ret);
}


