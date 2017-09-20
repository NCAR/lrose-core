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

#include "timestamp.h"

#include <stdexcept>

using namespace rainfields;

auto rainfields::operator<<(std::ostream& lhs, const timestamp& rhs) -> std::ostream&
{
  time_t time = rhs.as_time_t();
  char buf[32];
  struct tm tmm;
  if (   gmtime_r(&time, &tmm) == nullptr
      || strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S UTC", &tmm) <= 0)
    throw std::runtime_error("strftime failed");
  return lhs << buf;
}

auto rainfields::operator>>(std::istream& lhs, timestamp& rhs) -> std::istream&
{
  std::istream::sentry s(lhs);
  if (s)
  {
    try
    {
      auto rb = lhs.rdbuf();
      char buf[25];
      for (size_t i = 0; i < sizeof(buf); ++i)
      {
        buf[i] = rb->sgetc();

        // got the end of the token
        if (   buf[i] == std::char_traits<char>::eof()
            || std::isspace(buf[i]))
        {
          buf[i] = '\0';
          rhs = from_string<timestamp>(buf);
          return lhs;
        }

        rb->sbumpc();
      }

      // long token, failure, but read till next whitespace to make
      // it easier to recover the stream
      while (true)
      {
        auto c = rb->snextc();
        if (   c == std::char_traits<char>::eof()
            || std::isspace(c))
          break;
      }
    }
    catch (...)
    {
      lhs.setstate(std::ios_base::failbit);
    }
  }
  return lhs;
}

template <>
auto rainfields::from_string<timestamp>(const char* str) -> timestamp
{
  struct tm tms;
  char t, tz_sign;
  int tz_hh = 0, tz_mm = 0;

  // zero out our broken down time
  memset(&tms, 0, sizeof(struct tm));

  // try to parse as ISO 8601 with delimeters
  // NOTE: if no timezone is specified we assume UTC _not_ local time
  int ret = sscanf(
        str
      , "%04d-%02d-%02d%c%02d:%02d:%02d%c%02d%02d"
      , &tms.tm_year
      , &tms.tm_mon
      , &tms.tm_mday
      , &t
      , &tms.tm_hour
      , &tms.tm_min
      , &tms.tm_sec
      , &tz_sign
      , &tz_hh
      , &tz_mm);
  if (ret >= 3)
  {
    // adjust tm structure offsets
    tms.tm_year -= 1900;
    tms.tm_mon -= 1;

    // verify any 'T' or ' ' time field separator
    if (ret >= 4 && t != 't' && t != 'T' && t != ' ')
      throw string_conversion_error{"timestamp", str};

    // did we get a timezone?
    if (ret >= 8 && tz_sign != 'z' && tz_sign != 'Z')
    {
      // auto-detect daylight savings
      tms.tm_isdst = -1;

      // TODO cope with time zone
      //      currently impossible to do this in a thread-safe manner due to
      //      required modification of TZ environment variable
      throw std::runtime_error{"timezone parsing unimplemented, use UTC"};
    }

    // process as UTC
    // note: portability issue here - see man timegm
    return timestamp{timegm(&tms)};
  }
  // try to parse as a time_t directly
  else
  {
    // allow for date utility style '@' prefix to indicate epoch time
    auto start = str;
    if (start[0] == '@')
      ++start;

    // try to read as a long long
    char* end = nullptr;
    auto val = strtoll(start, &end, 10);
    if (*end == '\0' && *start != '\0')
      return timestamp{static_cast<time_t>(val)};
  }

  throw string_conversion_error{"timestamp", str};
}

