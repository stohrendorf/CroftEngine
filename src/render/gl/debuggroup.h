#pragma once

#include "glassert.h"

#include "gsl-lite.hpp"

namespace render
{
namespace gl
{
class DebugGroup final
{
public:
    explicit DebugGroup(const std::string& message, const ::gl::GLuint id = 0)
    {
        GL_ASSERT( glPushDebugGroup( ::gl::GL_DEBUG_SOURCE_APPLICATION,
                                     id,
                                     gsl::narrow<::gl::GLsizei>( message.length() ),
                                     message.c_str() ) );
    }

    DebugGroup(const DebugGroup&) = delete;

    DebugGroup(DebugGroup&&) noexcept = delete;

    DebugGroup& operator=(const DebugGroup&) = delete;

    DebugGroup& operator=(DebugGroup&&) = delete;

    ~DebugGroup()
    {
        GL_ASSERT( glPopDebugGroup() );
    }
};
}
}
