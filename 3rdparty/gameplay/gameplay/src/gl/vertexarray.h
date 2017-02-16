#pragma once

#include "bindableresource.h"

namespace gameplay
{
namespace gl
{
class VertexArray : public BindableResource
{
public:
    explicit VertexArray()
        : BindableResource(glGenVertexArrays, glBindVertexArray, glDeleteVertexArrays)
    {
    }
};
}
}
