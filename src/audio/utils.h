#pragma once

// ReSharper disable once CppUnusedIncludeDirective
#include <boost/current_function.hpp>

namespace audio
{
namespace detail
{
extern bool checkALError(const char* code, const char* func, int line);

template<typename F>
inline auto alAssertFn(F code, const char* codeStr, const char* func, int line) -> decltype( code() )
{
    const auto result = code();
#ifndef NDEBUG
    checkALError( codeStr, func, line );
#endif
    return result;
}
}
}

#define AL_ASSERT_FN(code) \
    ::audio::detail::alAssertFn([&](){return code;}, #code, BOOST_CURRENT_FUNCTION, __LINE__)

#ifndef NDEBUG
#define AL_ASSERT(code) \
    do { \
      code; \
      ::audio::detail::checkALError(#code, BOOST_CURRENT_FUNCTION, __LINE__); \
    } while(false)
#else
#define AL_ASSERT(code) \
    do { \
      code; \
    } while(false)
#endif
