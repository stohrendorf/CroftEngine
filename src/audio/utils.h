#pragma once

#include <boost/current_function.hpp>
#include <gsl/gsl-lite.hpp>

struct ALCdevice;

namespace audio::detail
{
extern void checkALError(gsl::czstring code, gsl::czstring func, int line);
extern void checkALCError(ALCdevice* device, gsl::czstring code, gsl::czstring func, int line);

template<typename F>
auto alAssertFn(F code, gsl::czstring codeStr, gsl::czstring func, int line) -> decltype(code())
{
  const auto result = code();
#ifndef NDEBUG
  checkALError(codeStr, func, line);
#endif
  return result;
}

template<typename F>
auto alcAssertFn(ALCdevice* device, F code, gsl::czstring codeStr, gsl::czstring func, int line) -> decltype(code())
{
  const auto result = code();
#ifndef NDEBUG
  checkALCError(device, codeStr, func, line);
#endif
  return result;
}
} // namespace audio::detail

#define AL_ASSERT_FN(code) ::audio::detail::alAssertFn([&]() { return code; }, #code, BOOST_CURRENT_FUNCTION, __LINE__)
#define ALC_ASSERT_FN(device, code) \
  ::audio::detail::alcAssertFn(     \
    device, [&]() { return code; }, #code, BOOST_CURRENT_FUNCTION, __LINE__)

#ifndef NDEBUG
#  define AL_ASSERT(code)                                                     \
    do                                                                        \
    {                                                                         \
      code;                                                                   \
      ::audio::detail::checkALError(#code, BOOST_CURRENT_FUNCTION, __LINE__); \
    } while(false)
#  define ALC_ASSERT(device, code)                                                     \
    do                                                                                 \
    {                                                                                  \
      code;                                                                            \
      ::audio::detail::checkALCError(device, #code, BOOST_CURRENT_FUNCTION, __LINE__); \
    } while(false)
#else
#  define AL_ASSERT(code) \
    do                    \
    {                     \
      code;               \
    } while(false)
#  define ALC_ASSERT(device, code) \
    do                             \
    {                              \
      code;                        \
    } while(false)
#endif
