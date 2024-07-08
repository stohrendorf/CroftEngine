#pragma once

#include <AL/alc.h>
#include <boost/current_function.hpp>
#include <gsl/gsl-lite.hpp>

namespace audio::detail
{
extern void checkALError(gsl::czstring code, gsl::czstring func, int line);
extern void checkALCError(ALCdevice* device, gsl::czstring code, gsl::czstring func, int line);

template<typename F>
// NOLINTNEXTLINE(*-easily-swappable-parameters)
auto alAssertFn(F code, gsl::czstring codeStr, gsl::czstring func, int line) -> decltype(code())
{
  const auto result = code();
#ifndef NDEBUG
  checkALError(codeStr, func, line);
#else
  (void)codeStr;
  (void)func;
  (void)line;
#endif
  return result;
}

template<typename F>
// NOLINTNEXTLINE(*-easily-swappable-parameters)
auto alcAssertFn(ALCdevice* device, F code, gsl::czstring codeStr, gsl::czstring func, int line) -> decltype(code())
{
  const auto result = code();
#ifndef NDEBUG
  checkALCError(device, codeStr, func, line);
#else
  (void)device;
  (void)codeStr;
  (void)func;
  (void)line;
#endif
  return result;
}
} // namespace audio::detail

#define AL_ASSERT_FN(code)     \
  ::audio::detail::alAssertFn( \
    [&]()                      \
    {                          \
      return code;             \
    },                         \
    #code,                     \
    BOOST_CURRENT_FUNCTION,    \
    __LINE__)

#ifndef NDEBUG
#  define AL_ASSERT(code)                                                     \
    /* NOLINTNEXTLINE(*-avoid-do-while) */                                    \
    do                                                                        \
    {                                                                         \
      code;                                                                   \
      ::audio::detail::checkALError(#code, BOOST_CURRENT_FUNCTION, __LINE__); \
    } while(false)
#  define ALC_ASSERT(device, code)                                                     \
    /* NOLINTNEXTLINE(*-avoid-do-while) */                                             \
    do                                                                                 \
    {                                                                                  \
      code;                                                                            \
      ::audio::detail::checkALCError(device, #code, BOOST_CURRENT_FUNCTION, __LINE__); \
    } while(false)
#else
#  define AL_ASSERT(code)                  \
    /* NOLINTNEXTLINE(*-avoid-do-while) */ \
    do                                     \
    {                                      \
      code;                                \
    } while(false)
#  define ALC_ASSERT(device, code)         \
    /* NOLINTNEXTLINE(*-avoid-do-while) */ \
    do                                     \
    {                                      \
      code;                                \
    } while(false)
#endif
