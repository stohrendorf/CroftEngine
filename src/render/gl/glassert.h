#pragma once

#include "glew.h"

#include <boost/assert.hpp>
#include <boost/log/trivial.hpp>

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

    BOOST_LOG_TRIVIAL( error ) << "OpenGL error " << error << ": " << gluErrorString( error );
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
