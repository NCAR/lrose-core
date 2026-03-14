// safe_snprintf.hh

#ifndef SAFE_SNPRINTF_HH
#define SAFE_SNPRINTF_HH

#include <cstdio>
#include <cstddef>
#include <utility>

// Safe only for real char arrays with compile-time known bound.
// This intentionally does NOT accept char*.
template <std::size_t N, typename... Args>
inline int safe_snprintf(char (&dst)[N], const char* fmt, Args&&... args)
{
  int iret = std::snprintf(dst, N, fmt, std::forward<Args>(args)...);
  if (iret < 0) {
    std::fprintf(stderr,
                 "WARNING - safe_snprintf() formatting error, format: %s\n",
                 fmt);
  } else if (static_cast<std::size_t>(iret) >= N) {
    std::fprintf(stderr,
                 "WARNING - safe_snprintf() overflow, output: %s\n",
                 dst);
  }
  return iret;
}

#endif
