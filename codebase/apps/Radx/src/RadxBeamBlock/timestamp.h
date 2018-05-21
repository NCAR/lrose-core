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

#ifndef RAINUTIL_TIMESTAMP_H
#define RAINUTIL_TIMESTAMP_H

#include "string_utils.h"
#include <chrono>
#include <ostream>
#include <cstdlib> // needed for int returning std::abs overloads

namespace rainfields
{
  class timestamp
  {
  public:
    using clock = std::chrono::system_clock;

  public:
    /// initialize timestamp with system clock epoch
    constexpr timestamp() noexcept
    { }
    /// initialize timestamp with given time point
    constexpr timestamp(clock::time_point time) noexcept
      : time_(time)
    { }
    /// initialize timestamp with given unix time
    explicit timestamp(time_t time) noexcept
      : time_(clock::from_time_t(time))
    { }

    timestamp(const timestamp& rhs) noexcept = default;
    timestamp(timestamp&& rhs) noexcept = default;
    
    auto operator=(const timestamp& rhs) noexcept -> timestamp& = default;
    auto operator=(timestamp&& rhs) noexcept -> timestamp& = default;

    ~timestamp() noexcept = default;

    constexpr operator clock::time_point() const noexcept
    {
      return time_;
    }

    explicit operator time_t() const noexcept
    {
      return clock::to_time_t(time_);
    }

    constexpr auto time() const noexcept -> clock::time_point
    {
      return time_;
    }

    auto as_time_t() const noexcept -> time_t 
    {
      return clock::to_time_t(time_);
    }

    static constexpr auto min() noexcept -> timestamp
    {
      return timestamp{clock::time_point::min()};
    }

    static constexpr auto max() noexcept -> timestamp
    {
      return timestamp{clock::time_point::max()};
    }

  private:
    clock::time_point time_;
  };

  constexpr bool operator==(const timestamp& lhs, const timestamp& rhs)
  {
    return lhs.time() == rhs.time();
  }
  constexpr bool operator!=(const timestamp& lhs, const timestamp& rhs)
  {
    return lhs.time() != rhs.time();
  }
  constexpr bool operator<(const timestamp& lhs, const timestamp& rhs)
  {
    return lhs.time() < rhs.time();
  }
  constexpr bool operator>(const timestamp& lhs, const timestamp& rhs)
  {
    return lhs.time() > rhs.time();
  }
  constexpr bool operator<=(const timestamp& lhs, const timestamp& rhs)
  {
    return lhs.time() <= rhs.time();
  }
  constexpr bool operator>=(const timestamp& lhs, const timestamp& rhs)
  {
    return lhs.time() >= rhs.time();
  }
  constexpr auto operator-(const timestamp& lhs, const timestamp& rhs) -> decltype(lhs.time() - rhs.time())
  {
    return lhs.time() - rhs.time();
  }

  auto operator<<(std::ostream& lhs, const timestamp& rhs) -> std::ostream&;
  auto operator>>(std::istream& lhs, timestamp& rhs) -> std::istream&;

  template <>
  auto from_string<timestamp>(const char* str) -> timestamp;

  // this function is to make up for a lack of std::abs support in std::chrono::duration
  template <class Rep, class Period = std::ratio<1>>
  auto abs(const std::chrono::duration<Rep, Period>& d) -> std::chrono::duration<Rep, Period>
  {
    return std::chrono::duration<Rep, Period>(std::abs(d.count()));
  }
}

#endif
