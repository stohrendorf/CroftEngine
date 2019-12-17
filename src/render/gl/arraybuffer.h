#pragma once

#include "buffer.h"

namespace render::gl
{
template<typename T>
class ArrayBuffer : public Buffer
{
public:
  explicit ArrayBuffer(const std::string& label = {})
      : Buffer{::gl::BufferTargetARB::ArrayBuffer, label}
  {
  }

  // ReSharper disable once CppMemberFunctionMayBeConst
  [[nodiscard]] T* map(const ::gl::BufferAccessARB access = ::gl::BufferAccessARB::ReadOnly)
  {
    bind();
    return static_cast<T*>(GL_ASSERT_FN(::gl::mapBuffer(::gl::BufferTargetARB::ArrayBuffer, access)));
  }

  static void unmap()
  {
    GL_ASSERT(::gl::unmapBuffer(::gl::BufferTargetARB::ArrayBuffer));
  }

  auto size() const noexcept
  {
    return m_size;
  }

protected:
  ::gl::core::SizeType m_size = 0;
};
} // namespace render::gl
