#pragma once

#include <boost/current_function.hpp>
#include <gsl-lite.hpp>

namespace audio::detail
{
extern bool checkALError(gsl::czstring code, gsl::czstring func, int line);

template<typename F>
auto alAssertFn(F code, gsl::czstring codeStr, gsl::czstring func, int line) -> decltype(code())
{
  const auto result = code();
#ifndef NDEBUG
  checkALError(codeStr, func, line);
#endif
  return result;
}
} // namespace audio::detail

#define AL_ASSERT_FN(code) ::audio::detail::alAssertFn([&]() { return code; }, #code, BOOST_CURRENT_FUNCTION, __LINE__)

#ifndef NDEBUG
#  define AL_ASSERT(code)                                                     \
    do                                                                        \
    {                                                                         \
      code;                                                                   \
      ::audio::detail::checkALError(#code, BOOST_CURRENT_FUNCTION, __LINE__); \
    } while(false)
#else
#  define AL_ASSERT(code) \
    do                    \
    {                     \
      code;               \
    } while(false)
#endif
