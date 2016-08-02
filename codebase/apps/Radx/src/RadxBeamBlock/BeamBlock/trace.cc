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

#include "trace.h"

#include <iostream>
#include <thread>

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

