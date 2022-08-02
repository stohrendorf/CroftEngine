#pragma once

#include "bindableresource.h" // IWYU pragma: export
#include "typetraits.h"

#include <gsl/gsl-lite.hpp>
#include <string_view>

namespace gl
{
// NOLINTNEXTLINE(bugprone-reserved-identifier)
template<typename T, api::BufferTarget _Target>
class Buffer : public BindableResource<api::ObjectIdentifier::Buffer>
{
public:
  static constexpr api::BufferTarget Target = _Target;

  explicit Buffer(const std::string_view& label)
      : BindableResource{api::createBuffers,
                         [](const uint32_t handle)
                         {
                           bindBuffer(Target, handle);
                         },
                         api::deleteBuffers,
                         label}
  {
  }

  [[nodiscard]] gsl::span<T> map(const api::core::Bitfield<api::MapBufferAccessMask>& access
                                 = api::MapBufferAccessMask::MapReadBit)
  {
    const void* data = GL_ASSERT_FN(api::mapNamedBufferRange(getHandle(), 0, m_size * sizeof(T), access));
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    return gsl::span{static_cast<T*>(const_cast<void*>(data)), m_size};
  }

  void flush()
  {
    GL_ASSERT(api::flushMappedNamedBufferRange(getHandle(), 0, m_size * sizeof(T)));
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
      m_size = data.size();
      GL_ASSERT(api::namedBufferData(getHandle(), data.size_bytes(), data.data(), usage));
    }
  }

  void setSubData(const gsl::span<const T>& data, const api::core::SizeType start)
  {
    GL_ASSERT(api::namedBufferSubData(
      getHandle(), gsl::narrow<std::intptr_t>(sizeof(T) * start), data.size_bytes(), data.data()));
  }

  [[nodiscard]] auto size() const noexcept
  {
    return m_size;
  }

private:
  size_t m_size = 0;
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
    GL_ASSERT(api::drawElements(primitiveType, gsl::narrow<api::core::SizeType>(size()), DrawElementsType<T>, nullptr));
  }

  void drawElements(api::PrimitiveType primitiveType, api::core::SizeType instanceCount) const
  {
    GL_ASSERT(api::drawElementsInstanced(
      primitiveType, gsl::narrow<api::core::SizeType>(size()), DrawElementsType<T>, nullptr, instanceCount));
  }
};
} // namespace gl
