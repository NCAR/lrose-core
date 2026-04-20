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

////////////////////////////////////////////////////////////////////////////
// RainFields.cc is a concatenation of a number of source files originally
// made available in open source by the Australian BOM.
// The intention is to cut down on the number of source files.
////////////////////////////////////////////////////////////////////////////

#include "RainFields.hh"

#include <arpa/inet.h> // just for ntohs()
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <strings.h>
#include <sys/stat.h>
#include <thread>

using namespace rainfields;
using namespace rainfields::ancilla;

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

IMPL_FROMSTRING_I(char, strtol)
IMPL_FROMSTRING_I(signed char, strtol)
IMPL_FROMSTRING_I(unsigned char, strtoul)
IMPL_FROMSTRING_I(short, strtol)
IMPL_FROMSTRING_I(unsigned short, strtoul)
IMPL_FROMSTRING_I(int, strtol)
IMPL_FROMSTRING_I(unsigned int, strtoul)
IMPL_FROMSTRING_I(long, strtol)
IMPL_FROMSTRING_I(unsigned long, strtoul)
IMPL_FROMSTRING_I(long long, strtoll)
IMPL_FROMSTRING_I(unsigned long long, strtoull)
IMPL_FROMSTRING_F(float, strtod)
IMPL_FROMSTRING_F(double, strtod)
IMPL_FROMSTRING_F(long double, strtold)

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

IMPL_TOSTRING_1(char, 4 * sizeof(char), "%hhd")
IMPL_TOSTRING_1(signed char, 4 * sizeof(signed char), "%hhd")
IMPL_TOSTRING_1(unsigned char, 4 * sizeof(unsigned char), "%hhu")
IMPL_TOSTRING_1(short, 4 * sizeof(short), "%hd")
IMPL_TOSTRING_1(unsigned short, 4 * sizeof(unsigned short), "%hu")
IMPL_TOSTRING_1(int, 4 * sizeof(int), "%d")
IMPL_TOSTRING_1(unsigned int, 4 * sizeof(unsigned int), "%u")
IMPL_TOSTRING_1(long, 4 * sizeof(long), "%ld")
IMPL_TOSTRING_1(unsigned long, 4 * sizeof(unsigned long), "%lu")
IMPL_TOSTRING_1(long long, 4 * sizeof(long long), "%lld")
IMPL_TOSTRING_1(unsigned long long, 4 * sizeof(unsigned long long), "%llu")
IMPL_TOSTRING_1(float, std::numeric_limits<float>::max_exponent10 + 20, "%f")
IMPL_TOSTRING_1(double, std::numeric_limits<double>::max_exponent10 + 20, "%f")
IMPL_TOSTRING_1(long double, std::numeric_limits<long double>::max_exponent10 + 20, "%Lf")

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

IMPL_TOSTRING_2(char, 4 * sizeof(char), "%hhd")
IMPL_TOSTRING_2(signed char, 4 * sizeof(char), "%hhd")
IMPL_TOSTRING_2(unsigned char, 4 * sizeof(unsigned char), "%hhu")
IMPL_TOSTRING_2(short, 4 * sizeof(short), "%hd")
IMPL_TOSTRING_2(unsigned short, 4 * sizeof(unsigned short), "%hu")
IMPL_TOSTRING_2(int, 4 * sizeof(int), "%d")
IMPL_TOSTRING_2(unsigned int, 4 * sizeof(unsigned int), "%u")
IMPL_TOSTRING_2(long, 4 * sizeof(long), "%ld")
IMPL_TOSTRING_2(unsigned long, 4 * sizeof(unsigned long), "%lu")
IMPL_TOSTRING_2(long long, 4 * sizeof(long long), "%lld")
IMPL_TOSTRING_2(unsigned long long, 4 * sizeof(unsigned long long), "%llu")
IMPL_TOSTRING_2(float, std::numeric_limits<float>::max_exponent10 + 20, "%f")
IMPL_TOSTRING_2(double, std::numeric_limits<double>::max_exponent10 + 20, "%f")
IMPL_TOSTRING_2(long double, std::numeric_limits<long double>::max_exponent10 + 20, "%Lf")

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
  return ret;
}

////////////////////////////////////////////////////////////////////////////
///////////////////////// RAINFIELDS_TRACE_H ///////////////////////////////

using namespace rainfields::trace;

constexpr const char* rainfields::enum_traits<level>::strings[];

static constexpr const char* log_codes[] = { "    ", " ** ", " EE ", " WW ", "    ", " DD " };

namespace rainfields {
namespace trace {
  class tracebuf : public std::stringbuf
  {
  public:
    tracebuf(std::ostream* out, bool identify_threads);

    auto target() const -> std::ostream*        { return out_; }
    auto set_target(std::ostream* out) -> void  { out_ = out; }

    auto identify_thread() const -> bool        { return identify_thread_; }
    auto set_identify_thread(bool val) -> void  { identify_thread_ = val; }

    auto min_level() const -> level             { return min_level_; }
    auto set_min_level(level lvl) -> void       { min_level_ = lvl; }

    auto setup_session(level lvl) -> bool;
    auto echo_on() const -> bool                { return echo_on_; }

    virtual int sync();

  private:
    std::ostream* out_;             ///< downstream target stream
    bool          identify_thread_; ///< whether to print thread identifiers
    level         min_level_;       ///< minimum level messages to echo
    char          prefix_[64];      ///< prefix to write to each line
    size_t        prefix_size_;     ///< length of line prefix string
    bool          echo_on_;         ///< are we supressing output?
    bool          armed_;           ///< output prefix before next character?
  };
}}

tracebuf::tracebuf(std::ostream* out, bool identify_threads)
  : out_(out)
  , identify_thread_(identify_threads)
  , min_level_(level::log)
  , prefix_size_(0)
  , armed_(true)
{ }

auto tracebuf::setup_session(level lvl) -> bool
{
  // is this trace below the importance threshold?
  echo_on_ = static_cast<int>(lvl) <= static_cast<int>(min_level_);

  if (echo_on_)
  {
    // trace identifier
    char thread_id[32];
    thread_id[0] = '\0';
    if (identify_thread_)
    {
#if 0 // TODO
      auto id = std::this_thread::get_id();
      if (id != std::thread::id())
        *target_ << std::hex << id << std::dec << "\t";
      else
        *target_ << "-\t";
#endif
    }

    // generate the timestamp
    time_t now = time(NULL);
    struct tm tm_now;
    if (!gmtime_r(&now, &tm_now))
      throw std::runtime_error("gmtime_r failed");

    // generate the prefix
    prefix_size_ = snprintf(
          prefix_
        , sizeof(prefix_)
        , "%s%04d-%02d-%02d %02d:%02d:%02d%s"
        , thread_id
        , tm_now.tm_year + 1900
        , tm_now.tm_mon + 1
        , tm_now.tm_mday
        , tm_now.tm_hour
        , tm_now.tm_min
        , tm_now.tm_sec
        , log_codes[static_cast<int>(lvl)]);
  }

  return echo_on_;
}

int tracebuf::sync()
{
  // get pointers to the buffer
  auto start = pbase(), cur = pbase(), end = pptr();

  // reset the internal buffer
  /* do this here so that if out_->write throws, we still clear the buffer and don't
   * get stuck trying to output the same data over and over */
  setp(pbase(), epptr());

  // only output if the stream is unmuted
  if (!echo_on_)
    return 0;

  // output the leading prefix if needed
  if (armed_ && cur != end)
  {
    out_->write(prefix_, prefix_size_);
    armed_ = false;
  }

  // output all the buffered data inserting the prefix before each new line
  while (cur != end)
  {
    if (*cur == '\n')
    {
      // if we have more characters after this one, dump the prefix now
      if (cur + 1 < end)
      {
        // dump what we've accumulated so far
        out_->write(start, cur - start + 1);
        start = cur + 1;

        // insert the prefix
        out_->write(prefix_, prefix_size_);
      }
      // otherwise arm us to print it straight up next time
      else
        armed_ = true;
    }

    ++cur;
  }

  // dump the residual
  if (cur != start)
    out_->write(start, cur - start);

  // flush the output stream
  out_->flush();

  return 0;
}

// all these static values are protected by trace_mutex_
static std::recursive_mutex trace_mutex_;
static tracebuf             buffer_(&std::cout, false);
static std::ostream         stream_(&buffer_);

auto rainfields::trace::min_level() -> level
{
  std::lock_guard<std::recursive_mutex> lock(trace_mutex_);
  return buffer_.min_level();
}

auto rainfields::trace::set_min_level(level min) -> void
{
  std::lock_guard<std::recursive_mutex> lock(trace_mutex_);
  buffer_.set_min_level(min);
}

auto rainfields::trace::target() -> std::ostream&
{
  std::lock_guard<std::recursive_mutex> lock(trace_mutex_);
  return *buffer_.target();
}

auto rainfields::trace::set_target(std::ostream& target) -> void
{
  std::lock_guard<std::recursive_mutex> lock(trace_mutex_);
  buffer_.set_target(&target);
}

auto rainfields::trace::identify_threads() -> bool
{
  std::lock_guard<std::recursive_mutex> lock(trace_mutex_);
  return buffer_.identify_thread();
}

auto rainfields::trace::set_identify_threads(bool identify) -> void
{
  std::lock_guard<std::recursive_mutex> lock(trace_mutex_);
  buffer_.set_identify_thread(identify);
}

session::session(level lvl)
  : lock_(trace_mutex_)
  , stream_(&::stream_)
  , echo_on_(buffer_.setup_session(lvl))
{ }

static void format_nested_exception(std::ostream& ss, const std::exception& err, const char* prefix)
try
{
  std::rethrow_if_nested(err);
}
catch (const std::exception& sub)
{
  ss << '\n' << prefix << "  -> " << sub.what();
  format_nested_exception(ss, sub, prefix);
}
catch (...)
{
  ss << '\n' << prefix << "  -> <unknown exception>";
}

auto rainfields::format_exception(const std::exception& err, const char* prefix) -> std::string
{
  std::ostringstream ss;
  ss << err.what();
  format_nested_exception(ss, err, prefix);
  return ss.str();
}

/////////////////////////////////////////////////////////////////////////////
//////////////////////// ANCILLA_CORE_FIELD_H ///////////////////////////////

field::field(std::string id)
  : id_(std::move(id))
{

}

// should be = default - but can't due to non-conforming std::string in gcc
auto field::operator=(field&& rhs) noexcept -> field&
{
  id_ = std::move(rhs.id_);
  return *this;
}

auto field::set_id(const std::string& val) -> void
{
  id_ = val;
}

field1::field1(std::string id, size_t size)
  : field(std::move(id))
  , array1<real>(size)
{ }

field1::field1(std::string id, const size_t dims[])
  : field(std::move(id))
  , array1<real>(dims)
{ }

field2::field2(std::string id, size_t y, size_t x)
  : field(std::move(id))
  , array2<real>(y, x)
{ }

field2::field2(std::string id, const size_t dims[])
  : field(std::move(id))
  , array2<real>(dims)
{ }

////////////////////////////////////////////////////////////////////////////
///////////////////////// RAINFIELDS_ANGLE_H ///////////////////////////////

auto angle::dms(int& deg, int& min, double& sec) const -> void
{
  double decimal = std::abs(degrees());
  deg = decimal;
  decimal = (decimal - deg) * 60.0;
  min = decimal;
  decimal = (decimal - min) * 60.0;
  sec = decimal;

  // fix rare case of floating point stuff up
  if (sec >= 60.0)
  {
    min++;
    sec -= 60.0;
  }

  // ensure correct sign on degrees
  if (rads_ < 0.0)
    deg = -deg;
}

auto angle::set_dms(int d, int m, double s) -> void
{
  rads_ = (d + (m / 60.0) + (s / 3600.0)) * constants<double>::to_radians;
}

template<>
auto rainfields::from_string<angle>(const char* str) -> angle
{
  // for now assume it's just a number specified in degrees
  return from_string<double>(str) * 1._deg;
}

////////////////////////////////////////////////////////////////////////////
//////////////////////// RAINFIELDS_LATLON_H ///////////////////////////////

auto rainfields::operator<<(std::ostream& lhs, const latlon& rhs) -> std::ostream&
{
  int d, m;
  double s;
  char buf[64];

  rhs.lat.dms(d, m, s);
  snprintf(buf, sizeof(buf), "%03d%02d%02.lf%c", std::abs(d), m, s, d < 0 ? 'S' : 'N');
  lhs << buf;

  rhs.lon.dms(d, m, s);
  snprintf(buf, sizeof(buf), " %03d%02d%02.lf%c", std::abs(d), m, s, d < 0 ? 'W' : 'E');
  lhs << buf;

  return lhs;
}

auto rainfields::operator<<(std::ostream& lhs, const latlonalt& rhs) -> std::ostream&
{
  char buf[32];
  snprintf(buf, sizeof(buf), " %.0fm", rhs.alt);
  return lhs << static_cast<const latlon&>(rhs) << buf;
}

template <>
auto rainfields::from_string<latlon>(const char* str) -> latlon
{
  // TODO - support multiple formats here...
  double lat, lon;
  if (sscanf(str, "%lf %lf", &lat, &lon) != 2)
    throw string_conversion_error("latlon", str);
  return {lat * 1_deg, lon * 1_deg};
}

template <>
auto rainfields::from_string<latlonalt>(const char* str) -> latlonalt
{
  // TODO - support multiple formats here...
  double lat, lon, alt;
  if (sscanf(str, "%lf %lf %lf", &lat, &lon, &alt) != 3)
    throw string_conversion_error("latlonalt", str);
  return {lat * 1_deg, lon * 1_deg, alt};
}

////////////////////////////////////////////////////////////////////////////
/////////////////////////// RAINFIELDS_XML_H ///////////////////////////////

using namespace rainfields::xml;

namespace rainfields {
namespace xml {

  // this is thread safe because we only ever use it in comparisons
  static const std::string empty_string;

  auto skip_pi(const char*& src) -> void
  {
    // loop until we find a '?>'
    for (char c = *++src; c != '\0'; c = *++src)
    {
      if (c != '?')
        continue;

      c = *++src;
      if (c != '>')
        continue;

      ++src;
      return;
    }

    throw parse_error("unterminated processing instruction '<?'", src);
  }

  auto skip_comment(const char*& src) -> void
  {
    // check that we have the '--'
    if (*++src != '-')
      throw parse_error("invalid comment", src);
    if (*++src != '-')
      throw parse_error("invalid comment", src);

    // loop until we find a '--'
    for (char c = *++src; c != '\0'; c = *++src)
    {
      if (c != '-')
        continue;

      c = *++src;
      if (c != '-')
        continue;

      c = *++src;
      if (c != '>')
        throw parse_error("comment body contains '--'", src);

      ++src;
      return;
    }

    throw parse_error("unterminated comment", src);
  }

  /* this function is currently only able to skip simple DOCTYPE declarations.
   * if you need to skip declarations that contain internal subsets you are in
   * for some fun... */
  auto skip_doctype(const char*& src) -> void
  {
    ++src;
    if (   src[0] != 'D' || src[1] != 'O' || src[2] != 'C'
        || src[3] != 'T' || src[4] != 'Y' || src[5] != 'P' || src[6] != 'E')
      throw parse_error("invalid DOCTYPE", src);
    src += 7;

    char quote_char = '\0';
    for (char c = *++src; c != '\0'; c = *++src)
    {
      if (quote_char == '\0')
      {
        if (c == '>')
        {
          ++src;
          return;
        }
        if (c == '"' || c == '\'')
          quote_char = c;
      }
      else if (c == quote_char)
        quote_char = '\0';
    }

    throw parse_error("unterminated DOCTYPE", src);
  }

  /* where_end should point to the character after 'src' in the stream. it 
   * is only used to provide location details when an exception is thrown */
  auto resolve_entities(std::string& src, const char* where_end) -> void
  {
    // search backward through the string for an '&'
    for (auto i = src.rbegin(); i != src.rend(); ++i)
    {
      if (*i != '&')
        continue;
      
      // got an entity, now loop forward until the ';'
      std::string::iterator s = i.base() - 1;
      std::string::iterator e = s;
      while (e != src.end() && *e != ';')
        ++e;
      if (e == src.end())
        throw parse_error(
              "unterminated character entity"
            , where_end - (i - src.rbegin() + 1));

      // determine which entity we have found.  if we wanted to support 
      // custom entities, they would need to be detected here
      auto el = e - s;
      if (el == 3 && *(s + 1) == 'l' && *(s + 2) == 't')
        *e = '<';
      else if (el == 3 && *(s + 1) == 'g' && *(s + 2) == 't')
        *e = '>';
      else if (el == 4 && *(s + 1) == 'a' && *(s + 2) == 'm' && *(s + 3) == 'p')
        *e = '&';
      else if (el == 5 && *(s + 1) == 'a' && *(s + 2) == 'p' && *(s + 3) == 'o' && *(s + 4) == 's')
        *e = '\'';
      else if (el == 5 && *(s + 1) == 'q' && *(s + 2) == 'u' && *(s + 3) == 'o' && *(s + 4) == 't')
        *e = '"';
      else
        throw parse_error(
              "unterminated character entity"
            , where_end - (i - src.rbegin() + 1));

      // erase the entity
      src.erase((i.base() - 1), e);
    }
  }
}}

parse_error::parse_error(const char* desc, const char* where)
  : desc_(desc)
  , where_(where)
  , line_(0)
  , col_(0)
{

}

auto parse_error::what() const noexcept -> const char*
{
  snprintf(
        what_
      , std::extent<decltype(what_)>::value
      , "xml parse error: %s at line %d col %d"
      , desc_
      , line_
      , col_);
  return what_;
}

auto parse_error::determine_line_col(const char* src) -> void
{
  for (line_ = col_ = 1; src < where_; ++src)
  {
    if (*src == '\n')
    {
      ++line_;
      col_ = 1;
    }
    else
      ++col_;
  }
}

path_error::path_error(const char* name, bool is_attribute)
  : name_(name)
  , is_attribute_(is_attribute)
{

}

auto path_error::what() const noexcept -> const char*
{
  description_.assign("xml path error: requested child ");
  if (is_attribute_)
    description_.append("attribute '");
  else
    description_.append("node '");
  description_.append(name_);
  description_.append("' does not exist");
  return description_.c_str();
}

attribute_store::attribute_store(attribute_store&& rhs) noexcept
  : base(std::move(rhs))
  , strings_(rhs.strings_)
{

}

auto attribute_store::operator=(attribute_store&& rhs) noexcept -> attribute_store&
{
  base::operator=(std::move(rhs));
  strings_ = rhs.strings_;
  return *this;
}

auto attribute_store::operator[](const char* name) -> attribute&
{
  for (auto& i : *this)
    if (i.name() == name)
      return i;
  throw path_error(name, true);
}

auto attribute_store::operator[](const char* name) const -> const attribute&
{
  for (auto& i : *this)
    if (i.name() == name)
      return i;
  throw path_error(name, true);
}

auto attribute_store::operator[](const std::string& name) -> attribute&
{
  for (auto& i : *this)
    if (i.name() == name)
      return i;
  throw path_error(name.c_str(), true);
}

auto attribute_store::operator[](const std::string& name) const -> const attribute&
{
  for (auto& i : *this)
    if (i.name() == name)
      return i;
  throw path_error(name.c_str(), true);
}

auto attribute_store::find(const char* name) -> iterator
{
  auto i = begin();
  while (i != end() && i->name() != name)
    ++i;
  return i;
}

auto attribute_store::find(const char* name) const -> const_iterator
{
  auto i = begin();
  while (i != end() && i->name() != name)
    ++i;
  return i;
}

auto attribute_store::find(const std::string& name) -> iterator
{
  auto i = begin();
  while (i != end() && i->name() != name)
    ++i;
  return i;
}

auto attribute_store::find(const std::string& name) const -> const_iterator
{
  auto i = begin();
  while (i != end() && i->name() != name)
    ++i;
  return i;
}

auto attribute_store::insert(const std::string& name) -> iterator
{
  emplace_back(
        strings_
      , &*strings_->insert(name).first
      , attribute::private_constructor());
  return end() - 1;
}

auto attribute_store::insert(std::string&& name) -> iterator
{
  emplace_back(
        strings_
      , &*strings_->insert(std::move(name)).first
      , attribute::private_constructor());
  return end() - 1;
}

auto attribute_store::find_or_insert(const char* name) -> iterator
{
  auto i = find(name);
  return i != end() ? i : insert(name);
}

auto attribute_store::find_or_insert(const std::string& name) -> iterator
{
  auto i = find(name);
  return i != end() ? i : insert(name);
}

node_store::node_store(node_store&& rhs) noexcept
  : base(std::move(rhs))
  , owner_(rhs.owner_)
{

}

auto node_store::operator=(node_store&& rhs) noexcept -> node_store&
{
  base::operator=(std::move(rhs));
  owner_ = rhs.owner_;
  return *this;
}

auto node_store::operator[](const char* name) -> node&
{
  for (auto& i : *this)
    if (i.name() == name)
      return i;
  throw path_error(name, false);
}

auto node_store::operator[](const char* name) const -> const node&
{
  for (auto& i : *this)
    if (i.name() == name)
      return i;
  throw path_error(name, false);
}

auto node_store::operator[](const std::string& name) -> node&
{
  for (auto& i : *this)
    if (i.name() == name)
      return i;
  throw path_error(name.c_str(), false);
}

auto node_store::operator[](const std::string& name) const -> const node&
{
  for (auto& i : *this)
    if (i.name() == name)
      return i;
  throw path_error(name.c_str(), false);
}

auto node_store::find(const char* name) -> iterator
{
  auto i = begin();
  while (i != end() && i->name() != name)
    ++i;
  return i;
}

auto node_store::find(const char* name) const -> const_iterator
{
  auto i = begin();
  while (i != end() && i->name() != name)
    ++i;
  return i;
}

auto node_store::find(const std::string& name) -> iterator
{
  auto i = begin();
  while (i != end() && i->name() != name)
    ++i;
  return i;
}

auto node_store::find(const std::string& name) const -> const_iterator
{
  auto i = begin();
  while (i != end() && i->name() != name)
    ++i;
  return i;
}

auto node_store::push_back(const std::string& name) -> iterator
{
  if (!owner_->value_->empty())
    throw std::logic_error("xml logic error: adding child node to non-empty element_simple");
  emplace_back(
        owner_->attributes_.strings_
      , &*owner_->attributes_.strings_->insert(name).first
      , true
      , node::private_constructor());
  return end() - 1;
}

auto node_store::push_back(std::string&& name) -> iterator
{
  if (!owner_->value_->empty())
    throw std::logic_error("xml logic error: adding child node to non-empty element_simple");
  emplace_back(
        owner_->attributes_.strings_
      , &*owner_->attributes_.strings_->insert(std::move(name)).first
      , true
      , node::private_constructor());
  return end() - 1;
}

auto node_store::insert(iterator position, const std::string& name) -> iterator
{
  if (!owner_->value_->empty())
    throw std::logic_error("xml logic error: adding child node to non-empty element_simple");
  return emplace(
        position
      , owner_->attributes_.strings_
      , &*owner_->attributes_.strings_->insert(name).first
      , true
      , node::private_constructor());
}

auto node_store::insert(iterator position, std::string&& name) -> iterator
{
  if (!owner_->value_->empty())
    throw std::logic_error("xml logic error: adding child node to non-empty element_simple");
  return emplace(
        position
      , owner_->attributes_.strings_
      , &*owner_->attributes_.strings_->insert(std::move(name)).first
      , true
      , node::private_constructor());
}

auto node_store::find_or_insert(const char* name) -> iterator
{
  auto i = find(name);
  return i != end() ? i : insert(end(), name);
}

auto node_store::find_or_insert(const std::string& name) -> iterator
{
  auto i = find(name);
  return i != end() ? i : insert(end(), name);
}

// this constructor is used to parse an attribute from a string
attribute::attribute(
      string_store* strings
    , const char*& src
    , std::string& tmp
    , private_constructor)
  : strings_(strings)
{
  size_t len;
  char c = *src;

  // extract the name
  len = 0;
  while (c != '\0' && c != '=' && c != ' ' && c != '\t' && c != '\n')
    c = *++src, ++len;
  if (len == 0)
    throw parse_error("attribute missing name", src);
  tmp.assign(src - len, len);
  name_ = &*strings_->insert(tmp).first;

  // extract the =
  while (c != '\0' && (c == ' ' || c == '\t' || c == '\n'))
    c = *++src;
  if (c != '=')
    throw parse_error("attribute missing '='", src);
  c = *++src;

  // extract the leading "
  while (c != '\0' && (c == ' ' || c == '\t' || c == '\n'))
    c = *++src;
  if (c != '"')
    throw parse_error("attribute missing leading '\"'", src);
  c = *++src;

  // extract the value
  len = 0;
  while (c != '\0' && c != '"')
    c = *++src, ++len;
  tmp.assign(src - len, len);
  resolve_entities(tmp, src);
  value_ = &*strings_->insert(tmp).first;

  // extract the trailing "
  if (c != '"')
    throw parse_error("attribute missing '\"'", src);
  ++src;
}

// this constructor is used to copy an attribute between documents
attribute::attribute(string_store* strings, const attribute& rhs, private_constructor)
  : strings_(strings)
  , name_(&*strings_->insert(*rhs.name_).first)
  , value_(&*strings_->insert(*rhs.value_).first)
{

}

// this constructor is used by attribute_store when appending a new attribute
attribute::attribute(string_store* strings, const std::string* name, private_constructor)
  : strings_(strings)
  , name_(name)
  , value_(&empty_string)
{

}

auto attribute::write(std::ostream& out) const -> void
{
  out << *name_ << "=\"";
  for (auto c : *value_)
    if (c == '<')
      out << "&lt;";
    else if (c == '&')
      out << "&amp;";
    else if (c == '"')
      out << "&quot;";
    else
      out << c;
  out << "\" ";
}

auto attribute::set_name(const std::string& val) -> void
{
  name_ = &*strings_->insert(val).first;
}

auto attribute::set(const std::string& val) -> void
{
  value_ = &*strings_->insert(val).first;
}

// this constructor is used to initialize a text_block
node::node(
      string_store* strings
    , const std::string* value
    , private_constructor)
  : attributes_(strings)
  , name_(&empty_string)
  , value_(value)
  , children_(this)
{ }

// this constructor is used to parse an element from a string
node::node(
      string_store* strings
    , const char*& src
    , std::string& tmp
    , private_constructor)
  : attributes_(strings)
  , value_(&empty_string)
  , children_(this)
{
  size_t len;
  char c = *src;

  // extract the name
  len = 0;
  while (c != '\0' && c != ' ' && c != '>' && c != '/' && c != '\t' && c != '\n')
    c = *++src, ++len;
  if (len == 0)
    throw parse_error("element missing name", src);
  tmp.assign(src - len, len);
  name_ = &*attributes_.strings_->insert(tmp).first;

  // extract any attributes
  while (c != '\0')
  {
    if (c == ' ' || c == '\t' || c == '\n')
      c = *++src;
    else if (c == '/' || c == '>')
      break;
    else
    {
      attributes_.emplace_back(attributes_.strings_, src, tmp, attribute::private_constructor());
      c = *src;
    }
  }

  // is it an empty element?
  if (c == '/')
  {
    c = *++src;
    if (c != '>')
      throw parse_error("invalid empty element tag", src);
    ++src;
    return;
  }
  if (c != '>')
    throw parse_error("invalid start element tag", src);
  c = *++src;

  // extract children
  while (c != '\0')
  {
    if (c == ' ' || c == '\t' || c == '\n')
      c = *++src;
    else if (c == '<')
    {
      // end-tag, comment, processing instruction or child element?
      c = *++src;
      if (c == '/')
        break;
      if (c == '\0')
        throw parse_error("unexpected end of file", src);
      if (c == ' ' || c == '\t' || c == '\n')
        throw parse_error("unexpected whitespace in tag", src);
      if (c == '!')
        /* Note: CDATA will also take this path (but cause a parse error) */
        skip_comment(src);
      else if (c == '?')
        skip_pi(src);
      else
      {
        // if we get an element subsequent to initialzing as a text only element
        // then upgrade us to a mixed content element first
        if (value_ != &empty_string)
        {
          children_.emplace_back(attributes_.strings_, value_, private_constructor());
          value_ = &empty_string;
        }
        children_.emplace_back(attributes_.strings_, src, tmp, private_constructor());
      }
      c = *src;
    }
    else
    {
      // child text

      // if we are already a text node, we must have encountered a comment or 
      // processing instruction after text. upgrade us to a mixed content element
      if (value_ != &empty_string)
        children_.emplace_back(attributes_.strings_, value_, private_constructor());

      // extract until end of the text
      len = 0;
      while (c != '\0' && c != '<')
        c = *++src, ++len;

      // strip trailing whitespace
      size_t strip = 0;
      for (; strip < len; ++strip)
      {
        char cc = *(src - strip - 1);
        if (cc != ' ' && cc != '\t' && cc != '\n')
          break;
      }
      tmp.assign(src - len, len - strip);
      resolve_entities(tmp, src);
      value_ = &*attributes_.strings_->insert(tmp).first;

      // is this a mixed content element?
      if (!children_.empty())
      {
        children_.emplace_back(attributes_.strings_, value_, private_constructor());
        value_ = &empty_string;
      }
    }
  }
  if (c == '\0')
    throw parse_error("unexpected end of file", src);
  c = *++src;

  // extract end tag
  len = 0;
  while (c != '\0' && c != '>' && c != ' ' && c != '\t' && c != '\n')
    c = *++src, ++len;
  if (len == 0)
    throw parse_error("invalid element end tag", src);
  tmp.assign(src - len, len);
  if (tmp != *name_)
    throw parse_error("element end tag mismatch", src);

  // extract the trailing >
  while (c != '\0' && (c == ' ' || c == '\t' || c == '\n'))
    c = *++src;
  if (c != '>')
    throw parse_error("invalid element end tag", src);
  ++src;
}

// this constructor is used when copying a node to a new document
node::node(string_store* strings, const node& rhs, private_constructor)
  : attributes_(strings)
  , name_(&*attributes_.strings_->insert(*rhs.name_).first)
  , value_(&*attributes_.strings_->insert(*rhs.value_).first)
  , children_(this)
{
  attributes_.reserve(rhs.attributes_.size());
  for (auto& i : rhs.attributes_)
    attributes_.emplace_back(strings, i, attribute::private_constructor());

  children_.reserve(rhs.children_.size());
  for (auto& i : rhs.children_)
    children_.emplace_back(strings, i, private_constructor());
}

// this constructor is used to insert a new node
node::node(
      string_store* strings
    , const std::string* name
    , bool
    , private_constructor)
  : attributes_(strings)
  , name_(name)
  , value_(&empty_string)
  , children_(this)
{ }

node::node(node&& rhs) noexcept
  : attributes_(std::move(rhs.attributes_))
  , name_(rhs.name_)
  , value_(rhs.value_)
  , children_(std::move(rhs.children_))
{
  children_.owner_ = this;
}

auto node::operator=(node&& rhs) noexcept -> node&
{
  attributes_ = std::move(rhs.attributes_);
  name_ = rhs.name_;
  value_ = rhs.value_;
  children_ = std::move(rhs.children_);
  children_.owner_ = this;
  return *this;
}

auto node::write(std::ostream& out) const -> void
{
  if (!children_.empty())
  {
    out << "<" << *name_ << " ";
    for (auto& i : attributes_)
      i.write(out);
    out << ">";
    for (auto& i : children_)
      i.write(out);
    out << "</" << *name_ << ">";
  }
  else if (!name_->empty())
  {
    out << "<" << *name_ << " ";
    for (auto& i : attributes_)
      i.write(out);
    if (value_->empty())
      out << "/>";
    else
    {
      out << ">";
      for (auto c : *value_)
        if (c == '<')
          out << "&lt;";
        else if (c == '>')
          out << "&gt;";
        else if (c == '&')
          out << "&amp;";
        else
          out << c;
      out << "</" << *name_ << ">";
    }
  }
  else
  {
    for (auto c : *value_)
      if (c == '<')
        out << "&lt;";
      else if (c == '>')
        out << "&gt;";
      else if (c == '&')
        out << "&amp;";
      else
        out << c;
  }
}

auto node::node_type() const -> enum type
{
  if (!children_.empty())
    return type::element_compound;
  if (!name_->empty())
    return type::element_simple;
  return type::text_block;
}

auto node::set_name(const std::string& val) -> void
{
  name_ = &*attributes_.strings_->insert(val).first;
}

auto node::operator()(const char* name, bool def) const -> bool
{
  auto i = attributes_.find(name);
  return i != attributes_.end() ? *i : def;
}

auto node::operator()(const char* name, char def) const -> char
{
  auto i = attributes_.find(name);
  return i != attributes_.end() ? *i : def;
}

auto node::operator()(const char* name, signed char def) const -> signed char
{
  auto i = attributes_.find(name);
  return i != attributes_.end() ? *i : def;
}

auto node::operator()(const char* name, unsigned char def) const -> unsigned char
{
  auto i = attributes_.find(name);
  return i != attributes_.end() ? *i : def;
}

auto node::operator()(const char* name, short def) const -> short
{
  auto i = attributes_.find(name);
  return i != attributes_.end() ? *i : def;
}

auto node::operator()(const char* name, unsigned short def) const -> unsigned short
{
  auto i = attributes_.find(name);
  return i != attributes_.end() ? *i : def;
}

auto node::operator()(const char* name, int def) const -> int
{
  auto i = attributes_.find(name);
  return i != attributes_.end() ? *i : def;
}

auto node::operator()(const char* name, unsigned int def) const -> unsigned int
{
  auto i = attributes_.find(name);
  return i != attributes_.end() ? *i : def;
}

auto node::operator()(const char* name, long def) const -> long
{
  auto i = attributes_.find(name);
  return i != attributes_.end() ? *i : def;
}

auto node::operator()(const char* name, unsigned long def) const -> unsigned long
{
  auto i = attributes_.find(name);
  return i != attributes_.end() ? *i : def;
}

auto node::operator()(const char* name, long long def) const -> long long
{
  auto i = attributes_.find(name);
  return i != attributes_.end() ? *i : def;
}

auto node::operator()(const char* name, unsigned long long def) const -> unsigned long long
{
  auto i = attributes_.find(name);
  return i != attributes_.end() ? *i : def;
}

auto node::operator()(const char* name, float def) const -> float
{
  auto i = attributes_.find(name);
  return i != attributes_.end() ? *i : def;
}

auto node::operator()(const char* name, double def) const -> double
{
  auto i = attributes_.find(name);
  return i != attributes_.end() ? *i : def;
}

auto node::operator()(const char* name, long double def) const -> long double
{
  auto i = attributes_.find(name);
  return i != attributes_.end() ? *i : def;
}

auto node::operator()(const char* name, const char* def) const -> const char*
{
  auto i = attributes_.find(name);
  return i != attributes_.end() ? i->get().c_str() : def;
}

auto node::set(const std::string& val) -> void
{
  if (!children_.empty())
    throw std::logic_error("xml logic error: attempt to set simple contents in element_compound");
  value_ = &*attributes_.strings_->insert(val).first;
}

document::document()
  : root_(&strings_, &*strings_.insert("root").first, true, node::private_constructor())
{

}

document::document(std::istream& source)
  : document(std::string(
            std::istreambuf_iterator<char>(source)
          , std::istreambuf_iterator<char>()).c_str())
{

}

document::document(std::istream&& source)
  : document(std::string(
            std::istreambuf_iterator<char>(source)
          , std::istreambuf_iterator<char>()).c_str())
{

}

document::document(const char* source) try
  : root_(&strings_, &empty_string, node::private_constructor())
{
  const char* src = source;
  std::string tmp;
  char c = *src;

#if 0 // enable to force checking of <?xml ... ?> at start of document
  // skip the xml declaration
  if (src[0] != '<' || src[1] != '?' || src[2] != 'x' || src[3] != 'm' || src[4] != 'l')
    throw parse_error("missing xml declaration", src);
  src += 5;
  skip_pi(src);
#endif

  // skip comments, processing instructions and the document type declaration
  while (c != '\0')
  {
    if (c == ' ' || c == '\t' || c == '\n')
    {
      c = *++src;
      continue;
    }

    if (c != '<')
      throw parse_error("unexpected character", src);

    c = *++src;
    if (c == '?')
      skip_pi(src);
    else if (c == '!' && src[1] == '-')
      skip_comment(src);
    else if (c == '!')
      skip_doctype(src);
    else
    {
      root_ = node(&strings_, src, tmp, node::private_constructor());
      return;
    }
    c = *src;
  }

  throw parse_error("missing root element", src);
}
catch (parse_error& err)
{
  err.determine_line_col(source);
  throw;
}

document::document(const node& root)
  : root_(&strings_, root, node::private_constructor())
{
  
}

document::document(const document& rhs)
  : strings_(rhs.strings_)
  , root_(&strings_, rhs.root_, node::private_constructor())
{

}

document::document(document&& rhs) noexcept
  : root_(std::move(rhs.root_))
{
  // only a.swap(b) is guarenteed to _not_ perform any copy/move/assignment on the
  // items held by a container. this ensures that the string pointers held by the
  // nodes and attributes shall not be invalidated (c++ 23.2.1 item 8)
  strings_.swap(rhs.strings_);
}

auto document::operator=(document&& rhs) noexcept -> document&
{
  strings_.swap(rhs.strings_);
  root_ = std::move(rhs.root_);
  return *this;
}

auto document::write(std::ostream& out) -> void
{
  out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>";
  root_.write(out);
}

auto document::write(std::ostream&& out) -> void
{
  write(out);
}

////////////////////////////////////////////////////////////////////////////////////
//////////////////////// ANCILLA_MODELS_BEAM_POWER_H ///////////////////////////////

beam_power::beam_power(angle beam_width_h, angle beam_width_v)
  : beam_width_h_(beam_width_h)
  , beam_width_v_(beam_width_v)
  , four_ln_two_(-4.0 * std::log(2.0))
  , inv_h_sqr_(1.0 / (beam_width_h_.radians() * beam_width_h_.radians()))
  , inv_v_sqr_(1.0 / (beam_width_v_.radians() * beam_width_v_.radians()))
{

}

/* calculate the 2d power of the radar beam at theta_azi and theta_elev degrees 
 * off the beam centre.
 *
 * Probert & Jones 1962 Meteoroogical Radar Equation, QJR Met Soc 88, 485 - 495
 */
auto beam_power::calculate(angle theta_azi, angle theta_elev) const -> real
{
  const auto azi = theta_azi.radians();
  const auto ele = theta_elev.radians();
  return std::exp(four_ln_two_ * ((azi * azi * inv_h_sqr_) + (ele * ele * inv_v_sqr_)));
}

/**
 * This class creates a rectangular array that represents a 2d cross-sectional view
 * of beam power.  The array is centered on the center (highest power) of the beam.
 *
 * The array is always normalized so that the total power in the array sums to 1.
 */

beam_power_cross_section::beam_power_cross_section(const beam_power& beam,
                                                   size_t rows,
                                                   size_t cols,
                                                   angle height /* = 6.0_deg */,
                                                   angle width /* = 6.0_deg */,
                                                   bool convolve_dwell_in_az /* = false */,
                                                   angle dwell_width /* = 1.0_deg */)
        : height_(height)
        , width_(width)
        , elevations_(rows)
        , azimuths_(cols)
        , data_(rows, cols)
{

  real offset;
  
  // determine relative elevation centers

  offset = 0.5_r * (1.0_r - rows);
  angle delta_v  = height / rows;
  for (size_t i = 0; i < rows; ++i) {
    elevations_[i] = (offset + i) * delta_v;
  }
  
  // determine relative azimuth centers

  offset = 0.5_r * (1.0_r - cols);
  angle delta_h  = width / cols;
  for (size_t i = 0; i < cols; ++i) {
    azimuths_[i] = (offset + i) * delta_h;
  }
  
  // calculate cross sectional power array around the pointing angle

  array2<real> csec{rows, cols};
  for (size_t y = 0; y < rows; ++y) {
    for (size_t x = 0; x < cols; ++x) {
      csec[y][x] = beam.calculate(azimuths_[x], elevations_[y]);
    }
  }

  if (convolve_dwell_in_az) {
    
    // convolve the pattern over the gate sweep arc
    // Start with the intrinsic 2D antenna power pattern in angular space,
    // with rows spanning elevation offset and columns spanning azimuth offset.
    //
    // Then convolve that pattern in the horizontal (azimuth) direction with
    // a rectangular window representing the finite azimuthal sweep arc over
    // which the radar sample is accumulated.
    //
    // NOTE:
    //   - dwell_width here is an angular width in azimuth, not a range-gate depth.
    //   - this broadens the effective horizontal response of the beam.
    //   - this refinement is mainly applicable when the antenna is moving in
    //     azimuth during sampling (e.g. PPI). It is less appropriate for RHI,
    //     staring modes, or electronically steered antennas.
    
    int start = std::lround(dwell_width / (-2.0_r * delta_h));
    int end = std::lround(dwell_width / (2.0_r * delta_h)) + 1;
    array_utils::fill(data_, 0.0_r);
    for (auto i = start; i < end; ++i) {
      const size_t sx = i < 0 ? -i : 0;
      const size_t ex = i > 0 ? cols - i : cols;
      for (size_t y = 0; y < rows; ++y) {
        for (size_t x = sx; x < ex; ++x) {
          data_[y][x + i] += csec[y][x];
        }
      }
    }
    
  } else {

    // use the power distribution as is, no convolution

    data_ = csec;

  }

  // normalize the array
  double total = 0.0;
  for (size_t i = 0; i < data_.size(); ++i) {
    total += data_.data()[i];
  }
  array_utils::divide(data_, data_, total);
  
}

/**
 * After calling this function values within the array shall no longer represent beam
 * power at a particular offset from beam center.  Instead, the value returned at any
 * point represents the fraction of total beam power at that point and below.  This
 * can be used to determine fraction of signal loss due to terrain obstructions.
 */

void beam_power_cross_section::make_vertical_integration()
{
  for (size_t y = 1; y < data_.rows(); ++y)
  {
    auto below = data_[y-1];
    auto above = data_[y];
    for (size_t x = 0; x < data_.cols(); ++x)
      above[x] += below[x];
  }
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////// ANCILLA_MODELS_BEAM_PROPAGATION_H ///////////////////////////////

beam_propagation::beam_propagation(
      angle elevation_angle
    , real site_altitude
    , real earth_radius
    , real effective_multiplier)
  : elevation_angle_(elevation_angle)
  , site_altitude_(site_altitude)
  , earth_radius_(earth_radius)
  , multiplier_(effective_multiplier)
  , effective_radius_(multiplier_ * earth_radius_)
  , cos_elev_(std::cos(elevation_angle_.radians()))
  , sin_elev_(std::sin(elevation_angle_.radians()))
  , eer_alt_(effective_radius_ + site_altitude_)
{

}

auto beam_propagation::set_elevation_angle(angle val) -> void
{
  elevation_angle_ = val;
  cos_elev_ = cos(elevation_angle_);
  sin_elev_ = sin(elevation_angle_);
}

auto beam_propagation::set_site_altitude(real val) -> void
{
  site_altitude_ = val;
  eer_alt_ = effective_radius_ + site_altitude_;
}

auto beam_propagation::set_effective_multiplier(real val) -> void
{
  multiplier_ = val;
  effective_radius_ = multiplier_ * earth_radius_;
  eer_alt_ = effective_radius_ + site_altitude_;
}

auto beam_propagation::set_earth_radius(real val) -> void
{
  earth_radius_ = val;
  effective_radius_ = multiplier_ * earth_radius_;
  eer_alt_ = effective_radius_ + site_altitude_;
}

auto beam_propagation::ground_range_altitude(real slant_range) const -> vec2<real>
{
  // distance along plane tangental to earth at site location
  double h = slant_range * cos_elev_;
  // distance above plane tangental to earth at site location
  double v = (slant_range * sin_elev_) + eer_alt_;
  return vec2<real>(
      // x = ground range = curved distance along earth surface
      std::atan(h/v) * effective_radius_
      // y = altitude = distance above spherical earth surface
    , std::hypot(h, v) - effective_radius_
  );

#if 0
  /* Alternative version based on theory from text book:
   *    Dopper Radar and Weather Observations, Doviak & Zrnic
   * The two versions here were derived separately, however result in negligible
   * difference (< 6.5mm height, < 0.1mm range @ 100km with elev angle of 30 deg) */
  real hgt = sqrt(in.x * in.x + eff_sqr_ + 2.0 * in.x * eff_rad_ * sin_elev_) - eff_rad_ + ref_point_sph_.hgt;
  real rng = eff_rad_ * asin((in.x * cos_elev_)/(eff_rad_ + hgt));
#endif
}

auto beam_propagation::slant_range(real ground_range) const -> real
{
  // TODO - it would be nice to calculate the geometric height here too...

  // inverse of above formula
  if (ground_range <= 0.0_r)
    return 0.0_r;
  else
    return eer_alt_ / ((cos_elev_ / std::tan(ground_range / effective_radius_)) - sin_elev_);
}

auto beam_propagation::required_elevation_angle(real ground_range, real altitude) const -> angle
{
  /* Warning to modifiers:
   * It is crucial that the calculations performed by this function use double precision
   * math.  This is due mostly to the very small angle of theta and very large values
   * of tgt_alt and eer_alt.  DO NOT CHANGE IT TO SINGLE PRECISION!  */

  // angle between site location and target on great circle
  angle theta = (ground_range / effective_radius_) * 1.0_rad;

  // slant range (law of cosines)
  double tgt_alt = effective_radius_ + altitude;
  double eer_alt = eer_alt_;
  double slant_range
    = (tgt_alt * tgt_alt) 
    + (eer_alt * eer_alt) 
    - 2.0 * tgt_alt * eer_alt * cos(theta);
  slant_range = std::sqrt(slant_range);

  // elevation angle (law of sines)
  // find the angle opposite the shorter side (since asin always returns < 90 degrees)
  if (tgt_alt < eer_alt)
    return asin((tgt_alt * sin(theta)) / slant_range) - 90.0_deg;
  else
    return 90.0_deg - theta - asin((eer_alt * sin(theta)) / slant_range);
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////// ANCILLA_MODELS_SPHEROID_H ///////////////////////////////

constexpr const char* enum_traits<spheroid::standard>::strings[];

// the order of the spheroids specified here _must_ match common_sphereoid
// first value is major axis, second value is inverse flattening
static constexpr real std_sphere[][2] =
{
    { 6378165.000, 298.3         } // i65
  , { 6378160.000, 298.25        } // ans
  , { 6378293.645, 294.26        } // clark1858
  , { 6378137.000, 298.257222101 } // grs80
  , { 6378137.000, 298.257223563 } // wgs84
  , { 6378135.000, 298.26        } // wgs72
  , { 6378388.000, 297           } // international1924
  , { 6378160.000, 298.25        } // australian_national
};

spheroid::spheroid(standard cs)
  : a_(std_sphere[static_cast<int>(cs)][0])
  , inv_f_(std_sphere[static_cast<int>(cs)][1])
{
  derive_parameters();
}

spheroid::spheroid(real semi_major_axis, real inverse_flattening)
  : a_(semi_major_axis)
  , inv_f_(inverse_flattening)
{
  derive_parameters();
}

spheroid::spheroid(const xml::node& conf)
{
  auto i = conf.attributes().find("spheroid");
  if (i == conf.attributes().end())
  {
    a_ = conf("semi_major_axis");
    inv_f_ = conf("inverse_flattening");
  }
  else
  {
    standard cs = from_string<standard>(*i);
    a_ = std_sphere[static_cast<int>(cs)][0];
    inv_f_ = std_sphere[static_cast<int>(cs)][1];
  }
  derive_parameters();
}

auto spheroid::radius_physical(angle lat) const -> real
{
  real a_cos = a_ * cos(lat);
  real a2_cos = a_ * a_cos;
  real b_sin = b_ * sin(lat);
  real b2_sin = b_ * b_sin;
  return std::sqrt((a2_cos * a2_cos + b2_sin * b2_sin) / (a_cos * a_cos + b_sin * b_sin));
}

auto spheroid::radius_of_curvature_meridional(angle lat) const -> real
{
  real ab = a_ * b_;
  real a_cos = a_ * cos(lat);
  real b_sin = b_ * sin(lat);
  return (ab * ab) / std::pow(a_cos * a_cos + b_sin * b_sin, 1.5_r);
}

auto spheroid::radius_of_curvature_normal(angle lat) const -> real
{
  real a_cos = a_ * cos(lat);
  real b_sin = b_ * sin(lat);
  return (a_ * a_) / std::sqrt(a_cos * a_cos + b_sin * b_sin);
}

auto spheroid::radius_of_curvature(angle lat) const -> real
{
  real a_cos = a_ * cos(lat);
  real b_sin = b_ * sin(lat);
  return (a_ * a_ * b_) / (a_cos * a_cos + b_sin * b_sin);
}

auto spheroid::latlon_to_ecefxyz(latlonalt pos) const -> vec3<real>
{
  real sin_lat = sin(pos.lat);
  real cos_lat = cos(pos.lat);
  real nu = a_ / std::sqrt(1.0_r - e_sqr_ * sin_lat * sin_lat);
  real nu_plus_h = nu + pos.alt;

  return vec3<real>(
        nu_plus_h * cos_lat * cos(pos.lon)
      , nu_plus_h * cos_lat * sin(pos.lon)
      , ((1.0_r - e_sqr_) * nu + pos.alt) * sin_lat);
}

auto spheroid::ecefxyz_to_latlon(vec3<real> pos) const -> latlonalt
{
  real p = std::sqrt(pos.x * pos.x + pos.y * pos.y);
  angle theta = atan((pos.z * a_) / (p * b_));
  real sin3 = sin(theta); sin3 = sin3 * sin3 * sin3;
  real cos3 = cos(theta); cos3 = cos3 * cos3 * cos3;
  angle lat = atan((pos.z + ep2_ * b_ * sin3) / (p - e_sqr_ * a_ * cos3));
  real sinlat = sin(lat);

  return latlonalt(
        lat
      , atan2(pos.y, pos.x)
      , (p / cos(lat)) - (a_ / std::sqrt(1.0_r - e_sqr_ * sinlat * sinlat)));
}

auto spheroid::bearing_range_to_latlon(latlon pos, angle bearing, real range) const -> latlon
{
  // based on original open source script found at:
  // http://www.movable-type.co.uk/scripts/latlong-vincenty-direct.html
  // modified for use in ancilla

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  */
  /* Vincenty Direct Solution of Geodesics on the Ellipsoid (c) Chris Veness 2005-2012              */
  /*                                                                                                */
  /* from: Vincenty direct formula - T Vincenty, "Direct and Inverse Solutions of Geodesics on the  */
  /*       Ellipsoid with application of nested equations", Survey Review, vol XXII no 176, 1975    */
  /*       http://www.ngs.noaa.gov/PUBS_LIB/inverse.pdf                                             */
  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  */

  auto sin_alpha_1 = sin(bearing);
  auto cos_alpha_1 = cos(bearing);
  
  auto tanU1 = (1.0 - f_) * tan(pos.lat);
  auto cosU1 = 1.0 / std::sqrt((1.0 + tanU1 * tanU1));
  auto sinU1 = tanU1 * cosU1;

  auto sigma1 = atan2(tanU1, cos_alpha_1);
  auto sinAlpha = cosU1 * sin_alpha_1;
  auto cosSqAlpha = 1.0 - sinAlpha * sinAlpha;
  auto uSq = cosSqAlpha * ep2_;
  auto A = 1.0 + uSq / 16384.0 * (4096.0 + uSq * (-768.0 + uSq * (320.0 - 175.0 * uSq)));
  auto B = uSq / 1024.0 * (256.0 + uSq * (-128.0 + uSq * (74.0 - 47.0 * uSq)));
  
  auto sigma = (range / (b_ * A)) * 1_rad;
  auto sigmaP = constants<double>::two_pi * 1_rad;
  real cos2SigmaM, sinSigma, cosSigma;
  while (true)
  {
    cos2SigmaM = cos(2.0 * sigma1 + sigma);
    sinSigma = sin(sigma);
    cosSigma = cos(sigma);

    if ((sigma - sigmaP).abs() > 1e-12_rad)
      break;

    auto deltaSigma = 
      B * sinSigma * (cos2SigmaM + B / 4 * (cosSigma * (-1.0 + 2.0 * cos2SigmaM * cos2SigmaM)
      - B / 6.0 * cos2SigmaM * (-3.0 + 4.0 * sinSigma * sinSigma) 
      * (-3.0 + 4.0 * cos2SigmaM * cos2SigmaM)));
    sigmaP = sigma;
    sigma.set_radians(range / (b_ * A) + deltaSigma);
  }

  auto tmp = sinU1 * sinSigma - cosU1 * cosSigma * cos_alpha_1;
  auto lat2 = atan2(sinU1 * cosSigma + cosU1 * sinSigma * cos_alpha_1, (1-f_) * std::hypot(sinAlpha, tmp));
  auto lambda = atan2(sinSigma*sin_alpha_1, cosU1*cosSigma - sinU1*sinSigma*cos_alpha_1);
  auto C = f_/16*cosSqAlpha*(4+f_*(4-3*cosSqAlpha));
  auto L = lambda - (1-C) * f_ * sinAlpha *
      (sigma + 1_rad * C * sinSigma * (cos2SigmaM + C * cosSigma * (-1.0 + 2.0 * cos2SigmaM * cos2SigmaM)));
  auto lon2 = 1_rad * (std::fmod(pos.lon.radians() + L.radians() + (3 * constants<double>::pi), constants<double>::two_pi) - constants<double>::pi);  // normalise to -180...+180

  return { lat2, lon2 };
}

/* Vincenty inverse formula - T Vincenty, "Direct and Inverse Solutions of Geodesics on the
 * Ellipsoid with application of nested equations", Survey Review, vol XXII no 176, 1975
 * http://www.ngs.noaa.gov/PUBS_LIB/inverse.pdf
 *
 * Implementation based on formulas found from the "Vincenty's Formulae" Wikipedia article.
 */
auto spheroid::latlon_to_bearing_range(latlon pos1, latlon pos2) const -> std::pair<angle, real>
{
  // initialization
  const double u1 = std::atan((1.0 - f_) * std::tan((double) pos1.lat.radians()));
  const double u2 = std::atan((1.0 - f_) * std::tan((double) pos2.lat.radians()));
  const double l = pos2.lon.radians() - pos1.lon.radians();

  // dependent initialization
  const double cos_u1 = std::cos(u1);
  const double cos_u2 = std::cos(u2);
  const double sin_u1 = std::sin(u1);
  const double sin_u2 = std::sin(u2);
  const double cu1su2 = cos_u1 * sin_u2;
  const double cu2su1 = cos_u2 * sin_u1;
  const double su1su2 = sin_u1 * sin_u2;
  const double cu1cu2 = cos_u1 * cos_u2;

  // iteration until convergence of lambda
  double lambda = l;
  for (size_t iters = 0; iters < 100; ++iters)
  {
    double cos_lambda = std::cos(lambda);
    double sin_lambda = std::sin(lambda);
    double sin_sigma  = std::hypot(cos_u2 * sin_lambda, cu1su2 - cu2su1 * cos_lambda);

    // check for coincident points
    if (sin_sigma == 0.0)
      return { 0.0_rad, 0.0 };

    double cos_sigma  = su1su2 + cu1cu2 * cos_lambda;
    double sigma      = std::atan2(sin_sigma, cos_sigma);
    double sin_alpha  = (cu1cu2 * sin_lambda) / sin_sigma;
    double csq_alpha  = 1.0 - sin_alpha * sin_alpha;
    double cos_2sigm  = cos_sigma - ((2.0 * su1su2) / csq_alpha);

    // check for equatorial line
    if (is_nan(cos_2sigm))
      cos_2sigm = 0.0;

    double c          = (f_ / 16.0) * csq_alpha * (4.0 + f_ * (4.0 - 3.0 * csq_alpha));
    double lambda_new = l + (1.0 - c) * f_ * sin_alpha * (sigma + c * sin_sigma * 
                          (cos_2sigm + c * cos_sigma * (-1.0 + 2.0 * cos_2sigm)));

    // check for convergence to correct answer
    if (std::abs(lambda - lambda_new) < 1e-12)
    {
      double u_sqr  = csq_alpha * ep2_;
      double a      = 1.0 + (u_sqr / 16384.0) * (4096.0 + u_sqr * 
                        (-768.0 + u_sqr * (320.0 - 175.0 * u_sqr)));
      double b      = (u_sqr / 1024.0) * (256.0 + u_sqr * (-128.0 + u_sqr * (74 - 47.0 * u_sqr)));
      double dsigma = b * sin_sigma * (cos_2sigm + 0.25 * b * (cos_sigma * 
                        (-1.0 + 2.0 * cos_2sigm * cos_2sigm) - (1.0 / 6.0) * b * cos_2sigm * 
                        (-3.0 + 4.0 * sin_sigma * sin_sigma) * (-3.0 + 4.0 * cos_2sigm * cos_2sigm)));
      double s      = b_ * a * (sigma - dsigma);
      double az_fwd = std::atan2(
                          (cos_u2 * std::sin(lambda_new))
                        , (cu1su2 - cu2su1 * std::cos(lambda_new)));
      return { az_fwd * 1_rad, s };
    }
    lambda = lambda_new;
  }

  // failed to converge
  return { nan<angle>(), nan() };
}

auto spheroid::bearing_range_to_latlon_haversine(latlon pos, angle bearing, real range) const -> latlon
{
  auto ronrad = (range / radius_of_curvature(pos.lat)) * 1_rad;
  auto srad = sin(ronrad);
  auto crad = cos(ronrad);
  auto slat = sin(pos.lat);
  auto clat = cos(pos.lat);

  auto lat = asin(slat * crad + clat * srad * cos(bearing));
  auto lon = pos.lon + atan2(sin(bearing) * srad * clat, crad - slat * sin(lat));

  return { lat, lon };
}

auto spheroid::latlon_to_bearing_range_haversine(latlon pos1, latlon pos2) const -> std::pair<angle, real>
{
  auto radius = radius_of_curvature(pos1.lat);

  auto dlat = pos2.lat - pos1.lat;
  auto dlon = pos2.lon - pos1.lon;

  auto clat1 = cos(pos1.lat);
  auto clat2 = cos(pos2.lat);

  // distance
  auto sdlat2 = sin(dlat * 0.5_r);
  auto sdlon2 = sin(dlon * 0.5_r);
  auto a = sdlat2 * sdlat2 + sdlon2 * sdlon2 * clat1 * clat2;
  auto r = radius * (2.0_r * atan2(sqrt(a), sqrt(1.0_r - a)).radians());

  // bearing
  auto y = sin(dlon) * clat2;
  auto x = clat1 * sin(pos2.lat) - sin(pos1.lat) * clat2 * cos(dlon);
  auto b = atan2(y,x);
  
  return { b, r };
}

void spheroid::derive_parameters()
{
  f_      = 1.0_r / inv_f_;
  b_      = a_ * (1.0_r - f_);
  e_sqr_  = f_ * (2.0_r - f_);
  ep2_    = (a_ * a_ - b_ * b_) / (b_ * b_);
  e_      = std::sqrt(e_sqr_);
  avg_r_  = (2.0_r * a_ + b_) / 3.0_r;

  auth_r_ = e_sqr_ * e_sqr_ * e_sqr_ * 4.0_r / 7.0_r;
  auth_r_ += e_sqr_ * e_sqr_ * 3.0_r / 5.0_r;
  auth_r_ += 1.0_r;
  auth_r_ *= (1.0_r - e_sqr_);
  auth_r_ = a_ * std::sqrt(auth_r_);
}

////////////////////////////////////////////////////////////////////////////
//////////////////////// DIGITAL_ELEVATION_H ///////////////////////////////

digital_elevation::digital_elevation(bool debug) :
        _debug(debug)
{
}

digital_elevation::~digital_elevation()
{
  
}

digital_elevation::model_error::model_error(std::string description)
        : runtime_error(description)
        , description_(std::move(description))
        , location_(nan<angle>(), nan<angle>())
{
  
}

digital_elevation::model_error::model_error(std::string description, latlon location)
        : runtime_error(msg{} << description << std::endl << "  location: " << location)
        , description_(std::move(description))
        , location_(location)
{
  
}

digital_elevation_srtm3::digital_elevation_srtm3(bool debug,
                                                 std::string path, size_t cache_size)
        : digital_elevation(debug)
        , path_(std::move(path))
        , cache_size_(cache_size)
        , wgs84_(spheroid::standard::wgs84)
{
  if (path_.empty())
    path_.assign("./");
  else if (path_.back() != '/')
    path_.append("/");
}

auto digital_elevation_srtm3::reference_spheroid() -> const spheroid&
{
  return wgs84_;
}

auto digital_elevation_srtm3::lookup(const latlon& loc) -> real
{

  real lat = std::abs(loc.lat.degrees());
  real lon = std::abs(loc.lon.degrees());
  int ilat = lat, ilon = lon;

  // determine the tile to use
  int tilelat = loc.lat.radians() < 0.0 ? -ilat - 1 : ilat;
  int tilelon = loc.lon.radians() < 0.0 ? -ilon - 1 : ilon;
  
  const srtm_tile &tile = get_tile(tilelat, tilelon);
  if (tile.nlat == 0 || tile.nlon == 0) {
    return 0.0; // sea location
  }

  // determine the indices within the tile
  int x, y;
  if (tilelat >= 0)
  {
    y = std::lround((ilat + 1 - lat) / tile.dlat);
  }
  else
  {
    y = std::lround((lat - ilat) / tile.dlat);
  }
  if (tilelon >= 0)
  {
    x = std::lround((lon - ilon) / tile.dlon);
  }
  else
  {
    x = std::lround((ilon + 1 - lon) / tile.dlon);
  }
  
  return tile.data[y][x];
     
}

auto digital_elevation_srtm3::lookup(latlonalt* values, size_t count) -> void
{
  throw model_error{"unimplemented feature: model multi lookup"};
}

auto digital_elevation_srtm3::testFTG(void) ->void
{
  test(39, -104, 40, -104, 39, -105, 39, -104);
}

auto digital_elevation_srtm3::testBOM(void) ->void
{
  test(-37, 144, -38, 144, -38, 145, -38, 146);
}

auto digital_elevation_srtm3::test(int lat0, int lon0, int lat1, int lon1, 
				   int lat2, int lon2, int lat3, int lon3) ->void
{
  array2<real> t0 = get_tile(lat0, lon0).data;
  array2<real> t1 = get_tile(lat1, lon1).data;
  bool match01_00 = true;
  bool match01_01 = true;
  bool match01_10 = true;
  bool match01_11 = true;
  for (size_t i=0; i<1201; ++i)
  {
    if (t0[0][i] != t1[1200][i])
    {
      match01_01 = false;
    }
    if (t0[0][i] != t1[0][i])
    {
      match01_00 = false;
    }
    if (t0[1200][i] != t1[0][i])
    {
      match01_10 = false;
    }
    if (t0[1200][i] != t1[1200][i])
    {
      match01_11 = false;
    }
  }
  if (match01_01)
  {
    printf("[%d,%d] row[0] matches [%d,%d] row[1200]\n",
	   lat0, lon0, lat1, lon1);
  }
  if (match01_10)
  {
    printf("[%d,%d] row[1200] matches [%d,%d] row[0]\n",
	   lat0, lon0, lat1, lon1);
  }
  if (match01_00)
  {
    printf("[%d,%d] row[0] matches [%d,%d] row[0]\n",
	   lat0, lon0, lat1, lon1);
  }
  if (match01_11)
  {
    printf("[%d,%d] row[1200] matches [%d,%d] row[1200]\n",
	   lat0, lon0, lat1, lon1);
  }


  t0 = get_tile(lat2, lon2).data;
  t1 = get_tile(lat3, lon3).data;
  match01_00 = true;
  match01_01 = true;
  match01_10 = true;
  match01_11 = true;
  for (size_t i=0; i<1201; ++i)
  {
    if (t0[i][0] != t1[i][1200])
    {
      match01_01 = false;
    }
    if (t0[i][0] != t1[i][0])
    {
      match01_00 = false;
    }
    if (t0[i][1200] != t1[i][0])
    {
      match01_10 = false;
    }
    if (t0[i][1200] != t1[i][1200])
    {
      match01_11 = false;
    }
  }
  if (match01_01)
  {
    printf("[%d,%d] col[0] matches [%d,%d] col[1200]\n",
	   lat2, lon2, lat3, lon3);
  }
  if (match01_10)
  {
    printf("[%d,%d] col[1200] matches [%d,%d] col[0]\n",
	   lat2, lon2, lat3, lon3);
  }
  if (match01_00)
  {
    printf("[%d,%d] col[0] matches [%d,%d] col[0]\n",
	   lat2, lon2, lat3, lon3);
  }
  if (match01_11)
  {
    printf("[%d,%d] col[1200] matches [%d,%d] col[1200]\n",
	   lat2, lon2, lat3, lon3);
  }
}

auto digital_elevation_srtm3::get_tile(int lat, int lon) -> const srtm_tile&
{

  // do we have this tile cached?
  for (auto i = tiles_.begin(); i != tiles_.end(); ++i)
  {
    if (i->lat == lat && i->lon == lon)
    {
      // promote tile to front of list
      if (i != tiles_.begin())
        tiles_.splice(tiles_.begin(), tiles_, i);
      return *i;
    }
  }

  // allocate or reuse a tile
  if (tiles_.size() < cache_size_)
    tiles_.emplace_front();
  else
    tiles_.splice(tiles_.begin(), tiles_, --tiles_.end());

  auto& tile = tiles_.front();

  // compute tile file name and path
  char file_name[128];
  snprintf(
          file_name
          , sizeof(file_name)
          , "%c%02d%c%03d.hgt"
          , lat < 0 ? 'S' : 'N'
          , std::abs(lat)
          , lon < 0 ? 'W' : 'E'
          , std::abs(lon));
  std::string file_path = path_ + file_name;

  // check tile dimensions

  struct stat fileStat;
  if (stat(file_path.c_str(), &fileStat)) {
    // file does not exist, set tile to have size 0x0
    if (_debug) {
      std::cerr << "Missing tile path: " << file_path << std::endl;
      std::cerr << "  Probably a sea tile" << std::endl;
    }
    tile.lat = lat;
    tile.lon = lon;
    tile.nlat = 0;
    tile.nlon = 0;
    tile.dlat = 0;
    tile.dlon = 0;
    return tile;
  }

  int nBytes = fileStat.st_size;
  long nCells = nBytes / 2; // data is 2 byte ints
  int tileDim = sqrt(nCells);
  if (_debug) {
    std::cerr << "Tile path: " << file_path << std::endl;
    std::cerr << " lat, lon: " << lat << ", " << lon << std::endl;
    std::cerr << "      dim: " << tileDim << std::endl;
  }

  // set tile metadata
  // depends on tile dimension
  // 3-sec data has tiles 1201 x 1201
  // 1-sec data has tiles 3601 x 3601

  tile.lat = lat;
  tile.lon = lon;
  tile.nlat = tileDim;
  tile.nlon = tileDim;
  tile.dlat = 1.0 / (tileDim - 1.0);
  tile.dlon = 1.0 / (tileDim - 1.0);

  // allocate space for tile data

  tile.data.resize(tile.nlat, tile.nlon);

  // fill tile data

  std::ifstream file((path_ + file_name).c_str(),
                     std::ifstream::in | std::ifstream::binary);
  if (file) {
    
    std::unique_ptr<std::int16_t[]>
      buf{new std::int16_t[tile.nlat * tile.nlon]};
    
    file.read(reinterpret_cast<char*>(buf.get()),
              sizeof(int16_t) * tile.nlat * tile.nlon);
    if (!file)
      throw model_error{
          msg{} << "srtm3: tile read failed: " << path_ << file_name
        , {lat * 1_deg, lon * 1_deg}};

    // convert from big-endian into host order
    for (int i = 0; i < tile.nlat * tile.nlon; ++i) {
      buf[i] = ntohs(buf[i]);
    }

    // convert to real replacing void values with NaN
    for (int i = 0; i < tile.nlat * tile.nlon; ++i) {
      tile.data.data()[i] = buf[i] == void_value ? nan() : buf[i];
    }

  }

  return tile;

}

digital_elevation_esri::digital_elevation_esri(
        bool debug
        , const std::string& path
        , latlon sw
        , latlon ne
        , spheroid::standard reference_spheroid)
        : digital_elevation_esri(debug, path, sw, ne, spheroid{reference_spheroid})
{

}

digital_elevation_esri::digital_elevation_esri(
        bool debug
        , const std::string& path
        , latlon sw
        , latlon ne
        , spheroid reference_spheroid)
        : digital_elevation(debug)
        , spheroid_(std::move(reference_spheroid))
{
  std::ifstream file(path);
  if (!file)
    throw model_error{msg{} << "esri: failed to open dataset: " << path};

  std::string label;
  int cols, rows;
  real llx, lly;
  real nodata;
  
  file >> label >> cols;
  if (!file || label != "ncols")
    throw model_error{msg{} << "esri: dataset expected ncols: " << path};

  file >> label >> rows;
  if (!file || label != "nrows")
    throw model_error{msg{} << "esri: dataset expected nrows: " << path};

  file >> label >> llx;
  if (!file || label != "xllcorner")
    throw model_error{msg{} << "esri: dataset expected xllcorner: " << path};

  file >> label >> lly;
  if (!file || label != "yllcorner")
    throw model_error{msg{} << "esri: dataset expected yllcorner: " << path};

  file >> label >> delta_deg_;
  if (!file || label != "cellsize")
    throw model_error{msg{} << "esri: dataset expected cellsize: " << path};

  file >> label >> nodata;
  if (!file || label != "NODATA_value")
    throw model_error{msg{} << "esri: dataset expected NODATA_value: " << path};

  // determine the subset of points to load
  int miny = rows - (ne.lat.degrees() - lly) / delta_deg_;
  int maxy = miny + (ne.lat - sw.lat).degrees() / delta_deg_;
  int minx = (sw.lon.degrees() - llx) / delta_deg_;
  int maxx = minx + (ne.lon - sw.lon).degrees() / delta_deg_;

  // clamp subset to the actual data
  bool warn = false;
  if (miny < 0)
  {
    miny = 0;
    warn = true;
  }
  if (maxy >= rows)
  {
    maxy = rows;
    warn = true;
  }
  if (minx < 0)
  {
    minx = 0;
    warn = true;
  }
  if (maxx >= cols)
  {
    maxx = cols;
    warn = true;
  }
  if (warn)
    trace::log() << "esri: requested subset " << sw << " - " << ne << " exceeds dataset bounds";

  // allocate our data
  data_ = array2<real>(maxy - miny, maxx - minx);

  // record central position of 0,0 point of our array
  nw_.lat.set_degrees(lly + (rows - miny - 0.5_r) * delta_deg_);
  nw_.lon.set_degrees(llx + (minx + 0.5_r) * delta_deg_);

  // skip to the first row we want to load
  file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  for (int i = 0; i < miny; ++i)
    file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

  // read each row in the interesting region
  for (size_t y = 0; y < data_.rows(); ++y)
  {
    // skip to the first column we want to load
    for (int i = 0; i < minx; ++i)
      file.ignore(std::numeric_limits<std::streamsize>::max(), ' ');

    // read the data
    real* raw = data_[y];
    for (size_t x = 0; x < data_.cols(); ++x)
    {
      file >> raw[x];
      if (std::abs(raw[x] - nodata) < 0.0001_r)
        raw[x] = nan();
    }

    // skip until the start of the next row
    file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  }
}

auto digital_elevation_esri::reference_spheroid() -> const spheroid&
{
  return spheroid_;
}

auto digital_elevation_esri::lookup(const latlon& loc) -> real
{
  // determine the array coordinates
  auto y = std::lround((nw_.lat - loc.lat).degrees() / delta_deg_);
  auto x = std::lround((loc.lon - nw_.lon).degrees() / delta_deg_);

  if (   y < 0 || y >= (int) data_.rows()
      || x < 0 || x >= (int) data_.cols())
    return nan();

  return data_[y][x];
}

auto digital_elevation_esri::lookup(latlonalt* values, size_t count) -> void
{
  throw model_error{"unimplemented feature: model multi lookup"};
}


