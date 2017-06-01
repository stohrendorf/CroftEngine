#pragma once

#define WIN32_LEAN_AND_MEAN
#define GLEW_STATIC
#include <GL/glew.h>

#include <boost/assert.hpp>
#include <boost/log/trivial.hpp>


namespace gameplay
{
    namespace gl
    {
        inline void checkGlError()
        {
#ifndef NDEBUG
            const auto error = glGetError();
            if( error == GL_NO_ERROR )
                return;

            BOOST_LOG_TRIVIAL(error) << "OpenGL error " << error << ": " << gluErrorString(error);
            BOOST_ASSERT_MSG(false, "OpenGL error check failed");
#endif
        }
    }
}
