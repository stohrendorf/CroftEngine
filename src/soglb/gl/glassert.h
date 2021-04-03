#pragma once

#include <gsl/gsl-lite.hpp>

// #define NO_GL_ASSERT

namespace gl
{
#if !defined(NDEBUG) && !defined(NO_GL_ASSERT)
extern void checkGlError(gsl::czstring code);
#else
inline void checkGlError(gsl::czstring /*code*/)
{
}
#endif

namespace detail
{
template<typename F>
auto glAssertFn(F code, const gsl::czstring codeStr) -> decltype(code())
{
  const auto result = code();
  checkGlError(codeStr);
  return result;
}
} // namespace detail
} // namespace render::gl

#define GL_ASSERT(gl_code)        \
  do                              \
  {                               \
    gl_code;                      \
    ::gl::checkGlError(#gl_code); \
  } while(false)

#define GL_ASSERT_FN(code) ::gl::detail::glAssertFn([&]() { return code; }, #code)
