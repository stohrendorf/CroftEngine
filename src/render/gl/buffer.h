#pragma once

#include "bindableresource.h"
#include "gsl-lite.hpp"
#include "typetraits.h"

namespace render::gl
{
class Buffer : public BindableResource
{
protected:
  explicit Buffer(const ::gl::BufferTargetARB type, const std::string& label = {})
      : BindableResource{::gl::genBuffers,
                         [type](const uint32_t handle) { bindBuffer(type, handle); },
                         ::gl::deleteBuffers,
                         ::gl::ObjectIdentifier::Buffer,
                         label}
  {
  }
};
} // namespace render::gl
