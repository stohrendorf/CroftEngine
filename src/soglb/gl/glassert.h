#pragma once

#include "glew_init.h"

#include <gsl-lite.hpp>

namespace gl
{
#ifndef NDEBUG
extern void checkGlError(gsl::czstring code);
#else
inline void checkGlError(gsl::czstring /*code*/)
{
}
#endif

namespace detail
{
template<typename F>
inline auto glAssertFn(F code, gsl::czstring codeStr) -> decltype(code())
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
