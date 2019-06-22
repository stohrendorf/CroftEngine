#pragma once

#include "bindableresource.h"
#include "gsl-lite.hpp"
#include "typetraits.h"

#include <vector>

namespace render
{
namespace gl
{
class Buffer : public BindableResource
{
protected:
    explicit Buffer(const ::gl::BufferTargetARB type, const std::string& label = {})
        : BindableResource{::gl::genBuffers,
                           [type](const uint32_t handle) { ::gl::bindBuffer(type, handle); },
                           ::gl::deleteBuffers,
                           ::gl::ObjectIdentifier::Buffer,
                           label}
    {
    }
};
} // namespace gl
} // namespace render
