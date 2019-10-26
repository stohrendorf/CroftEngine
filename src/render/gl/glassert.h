#pragma once

#include "glew.h"

#include <boost/assert.hpp>
#include <boost/log/trivial.hpp>
#include <boost/stacktrace.hpp>

namespace render
{
namespace gl
{
#ifndef NDEBUG
extern void checkGlError(const char* code);
#else
inline void checkGlError(const char* code)
{
}
#endif

namespace detail
{
template<typename F>
inline auto glAssertFn(F code, const char* codeStr) -> decltype(code())
{
  const auto result = code();
  checkGlError(codeStr);
  return result;
}
} // namespace detail
} // namespace gl
} // namespace render

#define GL_ASSERT(gl_code)                \
  do                                      \
  {                                       \
    gl_code;                              \
    ::render::gl::checkGlError(#gl_code); \
  } while(false)

#define GL_ASSERT_FN(code) ::render::gl::detail::glAssertFn([&]() { return code; }, #code)
