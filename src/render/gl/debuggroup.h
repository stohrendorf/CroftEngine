#pragma once

#include "glassert.h"
#include "gsl-lite.hpp"

namespace render::gl
{
class DebugGroup final
{
public:
  explicit DebugGroup(const std::string& message, const uint32_t id = 0)
  {
    GL_ASSERT(::gl::pushDebugGroup(::gl::DebugSource::DebugSourceApplication,
                                   id,
                                   gsl::narrow<::gl::core::SizeType>(message.length()),
                                   message.c_str()));
  }

  DebugGroup(const DebugGroup&) = delete;

  DebugGroup(DebugGroup&&) noexcept = delete;

  DebugGroup& operator=(const DebugGroup&) = delete;

  DebugGroup& operator=(DebugGroup&&) = delete;

  ~DebugGroup()
  {
    GL_ASSERT(::gl::popDebugGroup());
  }
};
} // namespace render::gl
