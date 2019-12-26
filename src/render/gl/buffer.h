#pragma once

#include "bindableresource.h"
#include "typetraits.h"

#include <gsl-lite.hpp>

namespace render::gl
{
template<typename T, ::gl::BufferTargetARB _Target>
class Buffer : public BindableResource
{
public:
  static constexpr ::gl::BufferTargetARB Target = _Target;

  explicit Buffer(const std::string& label = {})
      : BindableResource{::gl::genBuffers,
                         [](const uint32_t handle) { bindBuffer(Target, handle); },
                         ::gl::deleteBuffers,
                         ::gl::ObjectIdentifier::Buffer,
                         label}
  {
  }

  [[nodiscard]] T* map(const ::gl::BufferAccessARB access = ::gl::BufferAccessARB::ReadOnly)
  {
    bind();
    const void* data = GL_ASSERT_FN(::gl::mapBuffer(Target, access));
    return static_cast<T*>(const_cast<void*>(data));
  }

  void unmap()
  {
    GL_ASSERT(::gl::unmapBuffer(Target));
  }

  void setData(const T& data, const ::gl::BufferUsageARB usage)
  {
    setData(&data, 1, usage);
  }

  void setData(const std::vector<T>& data, const ::gl::BufferUsageARB usage)
  {
    if(data.empty())
      return;

    setData(gsl::not_null<const T*>(data.data()), gsl::narrow<::gl::core::SizeType>(data.size()), usage);
  }

  void setData(const gsl::not_null<const T*>& data, const ::gl::core::SizeType size, const ::gl::BufferUsageARB usage)
  {
    Expects(size >= 0);
    bind();

    if(size != 0)
      m_size = size;

    GL_ASSERT(::gl::bufferData(Target, gsl::narrow<std::size_t>(sizeof(T) * m_size), data.get(), usage));
  }

  void setSubData(const gsl::not_null<const T*>& data, const ::gl::core::SizeType start, ::gl::core::SizeType count)
  {
    Expects(size >= 0);
    Expects(count >= 0);
    bind();

    if(count == 0)
    {
      count = m_size - start;
      Expects(count >= 0);
    }

    GL_ASSERT(::gl::bufferSubData(
      Target, gsl::narrow<std::intptr_t>(sizeof(T) * start), gsl::narrow<std::size_t>(sizeof(T) * start), data));
  }

  [[nodiscard]] auto size() const noexcept
  {
    return m_size;
  }

private:
  ::gl::core::SizeType m_size = 0;
};

template<typename T>
using ShaderStorageBuffer = Buffer<T, ::gl::BufferTargetARB::ShaderStorageBuffer>;

template<typename T>
using UniformBuffer = Buffer<T, ::gl::BufferTargetARB::UniformBuffer>;

template<typename T>
using ArrayBuffer = Buffer<T, ::gl::BufferTargetARB::ArrayBuffer>;

template<typename T>
class ElementArrayBuffer : public Buffer<T, ::gl::BufferTargetARB::ElementArrayBuffer>
{
public:
  void drawElements(::gl::PrimitiveType primitiveType) const
  {
    GL_ASSERT(::gl::drawElements(primitiveType, size(), gl::TypeTraits<T>::DrawElementsType, nullptr));
  }
};
} // namespace render::gl
