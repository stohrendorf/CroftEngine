#pragma once

#include "bindableresource.h"


namespace gameplay
{
    namespace gl
    {
        class VertexArray : public BindableResource
        {
        public:
            explicit VertexArray(const std::string& label = {})
                : BindableResource{glGenVertexArrays, glBindVertexArray, glDeleteVertexArrays, GL_VERTEX_ARRAY, label}
            {
            }
        };
    }
}
