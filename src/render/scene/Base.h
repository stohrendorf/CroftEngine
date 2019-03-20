#pragma once

#include <boost/assert.hpp>

#define VERTEX_ATTRIBUTE_POSITION_NAME              "a_position"
#define VERTEX_ATTRIBUTE_NORMAL_NAME                "a_normal"
#define VERTEX_ATTRIBUTE_COLOR_NAME                 "a_color"
#define VERTEX_ATTRIBUTE_TEXCOORD_PREFIX_NAME       "a_texCoord"

#ifdef NDEBUG
#define GL_ASSERT( gl_code ) do { gl_code; } while(false)
#else
#define GL_ASSERT(gl_code) \
    do { \
        gl_code; \
        BOOST_ASSERT(glGetError() == GL_NO_ERROR); \
    } while(false)
#endif
