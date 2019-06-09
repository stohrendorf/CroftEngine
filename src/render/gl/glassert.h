#pragma once

#include "glew.h"

#include <boost/assert.hpp>
#include <boost/log/trivial.hpp>
#include <boost/stacktrace.hpp>

namespace render
{
namespace gl
{
inline void checkGlError()
{
#ifndef NDEBUG
    const auto error = glGetError();
    if( error == GL_NO_ERROR )
        return;

    BOOST_LOG_TRIVIAL( error ) << "OpenGL error " << error << ": " << (const char*)gluErrorString( error );
    BOOST_LOG_TRIVIAL( error ) << "Stacktrace:\n" << boost::stacktrace::stacktrace();
    BOOST_ASSERT_MSG( false, "OpenGL error check failed" );
#endif
}
}
}

#define GL_ASSERT(gl_code) \
    do { \
        gl_code; \
        ::render::gl::checkGlError(); \
    } while(false)
