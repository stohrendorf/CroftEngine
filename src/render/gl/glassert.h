#pragma once

#include "glew.h"

#include <boost/assert.hpp>
#include <boost/log/trivial.hpp>
#include <boost/stacktrace.hpp>
#include <glbinding-aux/types_to_string.h>

namespace render
{
namespace gl
{
inline void checkGlError(const char* code)
{
#ifndef NDEBUG
    const auto error = ::gl::glGetError();
    if(error == ::gl::GL_NO_ERROR)
        return;

    BOOST_LOG_TRIVIAL(error) << "OpenGL error " << error << " after evaluation of '" << code << "'";
    BOOST_LOG_TRIVIAL(error) << "Stacktrace:\n" << boost::stacktrace::stacktrace();
    BOOST_ASSERT_MSG(false, code);
#endif
}

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

#define GL_ASSERT_NO_NS(gl_code)              \
    do                                        \
    {                                         \
        gl_code;                              \
        ::render::gl::checkGlError(#gl_code); \
    } while(false)

#define GL_ASSERT(gl_code) GL_ASSERT_NO_NS(::gl::gl_code)

#define GL_ASSERT_FN(code) ::render::gl::detail::glAssertFn([&]() { return ::gl::code; }, #code)
