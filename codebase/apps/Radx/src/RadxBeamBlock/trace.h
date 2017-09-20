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
