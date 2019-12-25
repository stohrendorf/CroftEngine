#pragma once

#include "buffer.h"
#include "typetraits.h"

#include <gsl-lite.hpp>
#include <vector>

namespace render::gl
{
template<typename T>
class ShaderStorageBuffer final : public Buffer
{
public:
  explicit ShaderStorageBuffer(const std::string& label = {})
      : Buffer{::gl::BufferTargetARB::ShaderStorageBuffer, label}
  {
  }

  [[nodiscard]] T* map(const ::gl::BufferAccessARB access = ::gl::BufferAccessARB::ReadOnly)
  {
    bind();
    const void* data = GL_ASSERT_FN(::gl::mapBuffer(::gl::BufferTargetARB::ShaderStorageBuffer, access));
    return static_cast<const T*>(data);
  }

  static void unmap()
  {
    GL_ASSERT(::gl::unmapBuffer(::gl::BufferTargetARB::ShaderStorageBuffer));
  }

  void setData(const T& data, const ::gl::BufferUsageARB usage)
  {
    bind();

    GL_ASSERT(::gl::bufferData(::gl::BufferTargetARB::ShaderStorageBuffer, sizeof(T), &data, usage));
  }

  void setData(const std::vector<T>& data, const ::gl::BufferUsageARB usage)
  {
    bind();

    GL_ASSERT(
      ::gl::bufferData(::gl::BufferTargetARB::ShaderStorageBuffer, sizeof(T) * data.size(), data.data(), usage));
  }
};
} // namespace render::gl
