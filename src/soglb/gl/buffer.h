#pragma once

#include "bindableresource.h" // IWYU pragma: export
#include "typetraits.h"

#include <gsl/gsl-lite.hpp>
#include <string_view>

namespace gl
{
// NOLINTNEXTLINE(bugprone-reserved-identifier)
template<typename T, api::BufferTarget Target>
class Buffer;

// NOLINTNEXTLINE(bugprone-reserved-identifier)
template<typename T, api::BufferTarget _Target>
struct MappedBuffer final
{
  friend class Buffer<T, _Target>;

public:
  ~MappedBuffer()
  {
    if(!m_span.empty())
      GL_ASSERT(api::unmapNamedBuffer(m_buffer.getHandle()));
  }

  [[nodiscard]] auto size() const
  {
    return m_span.size();
  }

  [[nodiscard]] auto begin() const
  {
    return m_span.begin();
  }

  [[nodiscard]] auto end() const
  {
    return m_span.end();
  }

  [[nodiscard]] auto& operator[](size_t idx)
  {
    return m_span[idx];
  }

  [[nodiscard]] const auto& operator[](size_t idx) const
  {
    return m_span[idx];
  }

  void flush()
  {
    if(!m_span.empty())
      GL_ASSERT(api::flushMappedNamedBufferRange(m_buffer.getHandle(), 0, size() * sizeof(T)));
  }

private:
  explicit MappedBuffer(Buffer<T, _Target>& buffer, T* ptr, size_t size)
      : m_buffer{buffer}
      , m_span{ptr, size}
  {
  }

  explicit MappedBuffer(Buffer<T, _Target>& buffer)
      : m_buffer{buffer}
      , m_span{}
  {
  }

  Buffer<T, _Target>& m_buffer;
  gsl::span<T> m_span;
};

// NOLINTNEXTLINE(bugprone-reserved-identifier)
template<typename T, api::BufferTarget _Target>
class Buffer : public BindableResource<api::ObjectIdentifier::Buffer>
{
public:
  static constexpr api::BufferTarget Target = _Target;

  explicit Buffer(const std::string_view& label, api::BufferUsage usage, const gsl::span<const T>& data)
      : BindableResource{api::createBuffers,
                         [](const uint32_t handle)
                         {
                           bindBuffer(Target, handle);
                         },
                         api::deleteBuffers,
                         label}
      , m_size{data.size()}
  {
    GL_ASSERT(api::namedBufferData(getHandle(), data.size_bytes(), data.data(), usage));
  }

  explicit Buffer(const std::string_view& label, api::BufferUsage usage, size_t size)
      : BindableResource{api::createBuffers,
                         [](const uint32_t handle)
                         {
                           bindBuffer(Target, handle);
                         },
                         api::deleteBuffers,
                         label}
      , m_size{size}
  {
    GL_ASSERT(api::namedBufferData(getHandle(), sizeof(T) * size, nullptr, usage));
  }

  explicit Buffer(const std::string_view& label, api::BufferUsage usage, const T& data)
      : Buffer{label, usage, gsl::span{&data, 1}}
  {
  }

  [[nodiscard]] MappedBuffer<T, _Target> map(const api::core::Bitfield<api::MapBufferAccessMask>& access
                                             = api::MapBufferAccessMask::MapReadBit)
  {
    if(m_size == 0)
      return MappedBuffer{*this};

    const void* data = GL_ASSERT_FN(api::mapNamedBufferRange(getHandle(), 0, m_size * sizeof(T), access));
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    return MappedBuffer{*this, static_cast<T*>(const_cast<void*>(data)), m_size};
  }

  void setSubData(const gsl::span<const T>& data, const api::core::SizeType start)
  {
    GL_ASSERT(api::namedBufferSubData(
      getHandle(), gsl::narrow<std::intptr_t>(sizeof(T) * start), data.size_bytes(), data.data()));
  }

  void setSubData(const T& data, const api::core::SizeType start)
  {
    setSubData(gsl::span{&data, 1}, start);
  }

  [[nodiscard]] auto size() const noexcept
  {
    return m_size;
  }

private:
  const size_t m_size;
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

  explicit ElementArrayBuffer(const std::string_view& label, api::BufferUsage usage, const gsl::span<const T>& data)
      : Buffer<T, api::BufferTarget::ElementArrayBuffer>{label, usage, data}
  {
  }

  explicit ElementArrayBuffer(const std::string_view& label, api::BufferUsage usage, const T& data)
      : Buffer<T, api::BufferTarget::ElementArrayBuffer>{label, usage, data}
  {
  }

  void drawElements(api::PrimitiveType primitiveType) const
  {
    if(size() > 0)
    {
      GL_ASSERT(
        api::drawElements(primitiveType, gsl::narrow<api::core::SizeType>(size()), DrawElementsType<T>, nullptr));
    }
  }

  void drawElements(api::PrimitiveType primitiveType, api::core::SizeType instanceCount) const
  {
    if(size() > 0)
    {
      GL_ASSERT(api::drawElementsInstanced(
        primitiveType, gsl::narrow<api::core::SizeType>(size()), DrawElementsType<T>, nullptr, instanceCount));
    }
  }
};
} // namespace gl
