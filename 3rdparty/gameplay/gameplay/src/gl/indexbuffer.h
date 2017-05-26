#pragma once

#include "bindableresource.h"


namespace gameplay
{
    namespace gl
    {
        class IndexBuffer : public BindableResource
        {
        public:
            explicit IndexBuffer(const std::string& label = {})
                : BindableResource{glGenBuffers,
                                   [](GLuint handle) { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle); },
                                   glDeleteBuffers,
                                   GL_BUFFER,
                                   label}
            {
            }


            // ReSharper disable once CppMemberFunctionMayBeConst
            const void* map()
            {
                bind();
                const void* data = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_READ_ONLY);
                checkGlError();
                return data;
            }


            static void unmap()
            {
                glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
                checkGlError();
            }
        };
    }
}
