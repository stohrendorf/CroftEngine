#pragma once

#include "util.h"


namespace gameplay
{
    namespace gl
    {
        class DebugGroup final
        {
        public:
            explicit DebugGroup(const std::string& message, GLuint id = 0)
            {
#ifndef NDEBUG
                glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, id, message.length(), message.c_str());
                checkGlError();
#endif
            }


            ~DebugGroup()
            {
#ifndef NDEBUG
                glPopDebugGroup();
                checkGlError();
#endif
            }
        };
    }
}
