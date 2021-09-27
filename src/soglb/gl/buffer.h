#pragma once

#include "bindableresource.h" // IWYU pragma: export
#include "typetraits.h"

#include <gsl/gsl-lite.hpp>
#include <string_view>

namespace gl
{
// NOLINTNEXTLINE(bugprone-reserved-identifier)
template<typename T, api::BufferTarget _Target>
class Buffer : public BindableResource
{
public:
  static constexpr api::BufferTarget Target = _Target;

  explicit Buffer(const std::string_view& label)
      : BindableResource{api::createBuffers,
                         [](const uint32_t handle) { bindBuffer(Target, handle); },
                         api::deleteBuffers,
                         api::ObjectIdentifier::Buffer,
                         label}
  {
  }

  [[nodiscard]] gsl::span<T> map(const api::BufferAccess access = api::BufferAccess::ReadOnly)
  {
    const void* data = GL_ASSERT_FN(api::mapNamedBuffer(getHandle(), access));
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    return gsl::span{static_cast<T*>(const_cast<void*>(data)),
                     gsl::narrow_cast<typename gsl::span<T>::index_type>(m_size)};
  }

  void unmap()
  {
    GL_ASSERT(api::unmapNamedBuffer(getHandle()));
  }

  void setData(const T& data, const api::BufferUsage usage)
  {
    setData(gsl::span{&data, 1}, usage);
  }

  void setData(const gsl::span<const T>& data, const api::BufferUsage usage)
  {
    if(m_size == data.size() && m_usage == usage)
    {
      GL_ASSERT(api::namedBufferSubData(getHandle(), 0, data.size_bytes(), data.data()));
    }
    else
    {
      m_usage = usage;
      m_size = gsl::narrow<api::core::SizeType>(data.size());
      GL_ASSERT(api::namedBufferData(getHandle(), data.size_bytes(), data.data(), usage));
    }
  }

  void setSubData(const gsl::span<const T>& data, const api::core::SizeType start)
  {
    Expects(m_size >= 0);

    GL_ASSERT(
      api::namedBufferSubData(getHandle(), gsl::narrow<std::intptr_t>(sizeof(T) * start), data.size_byts(), data));
  }

  [[nodiscard]] auto size() const noexcept
  {
    return m_size;
  }

private:
  api::core::SizeType m_size = 0;
  api::BufferUsage m_usage{static_cast<api::BufferUsage>(0)};
};

template<typename T>
using ShaderStorageBuffer = Buffer<T, api::BufferTarget::ShaderStorageBuffer>;

template<typename T>
using UniformBuffer = Buffer<T, api::BufferTarget::UniformBuffer>;

template<typename T>
using ArrayBuffer = Buffer<T, api::BufferTarget::ArrayBuffer>;

template<typename T>
class ElementArrayBuffer final : public Buffer<T, api::BufferTarget::ElementArrayBuffer>
{
public:
  using Buffer<T, api::BufferTarget::ElementArrayBuffer>::size;

  explicit ElementArrayBuffer(const std::string_view& label)
      : Buffer<T, api::BufferTarget::ElementArrayBuffer>{label}
  {
  }

  void drawElements(api::PrimitiveType primitiveType) const
  {
    GL_ASSERT(api::drawElements(primitiveType, size(), DrawElementsType<T>, nullptr));
  }

  void drawElements(api::PrimitiveType primitiveType, api::core::SizeType instances) const
  {
    GL_ASSERT(api::drawElementsInstance(primitiveType, size(), DrawElementsType<T>, instances));
  }
};
} // namespace gl
