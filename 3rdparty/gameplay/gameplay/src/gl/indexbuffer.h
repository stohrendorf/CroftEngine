#pragma once

#include "bindableresource.h"


namespace gameplay
{
    namespace gl
    {
        class IndexBuffer : public BindableResource
        {
        public:
            explicit IndexBuffer()
                : BindableResource(glGenBuffers, [](GLuint handle)
                                               {
                                                   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle);
                                               }, glDeleteBuffers)
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


            // ReSharper disable once CppMemberFunctionMayBeStatic
            // ReSharper disable once CppMemberFunctionMayBeConst
            void unmap()
            {
                glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
                checkGlError();
            }
        };
    }
}
