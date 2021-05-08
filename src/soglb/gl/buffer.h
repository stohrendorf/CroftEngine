#pragma once

#include "bindableresource.h"
#include "typetraits.h"

namespace gl
{
// NOLINTNEXTLINE(bugprone-reserved-identifier)
template<typename T, api::BufferTarget _Target>
class Buffer : public BindableResource
{
public:
  static constexpr api::BufferTarget Target = _Target;

  explicit Buffer(const std::string& label = {})
      : BindableResource{api::createBuffers,
                         [](const uint32_t handle) { bindBuffer(Target, handle); },
                         api::deleteBuffers,
                         api::ObjectIdentifier::Buffer,
                         label}
  {
  }

  [[nodiscard]] T* map(const api::BufferAccess access = api::BufferAccess::ReadOnly)
  {
    const void* data = GL_ASSERT_FN(api::mapNamedBuffer(getHandle(), access));
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    return static_cast<T*>(const_cast<void*>(data));
  }

  // ReSharper disable once CppMemberFunctionMayBeConst
  void unmap()
  {
    GL_ASSERT(api::unmapNamedBuffer(getHandle()));
  }

  void setData(const T& data, const api::BufferUsage usage)
  {
    setData(&data, 1, usage);
  }

  void setData(const std::vector<T>& data, const api::BufferUsage usage)
  {
    if(data.empty())
    {
      if(size() != 0)
      {
        static const T tmp{};
        setData(&tmp, 0, usage);
      }
      return;
    }

    setData(gsl::not_null{data.data()}, gsl::narrow<api::core::SizeType>(data.size()), usage);
  }

  void setData(const gsl::not_null<const T*>& data, const api::core::SizeType size, const api::BufferUsage usage)
  {
    Expects(size >= 0);

    if(size == 0)
      return;

    if(m_size == size && m_usage == usage)
    {
      GL_ASSERT(api::namedBufferSubData(getHandle(), 0, gsl::narrow<std::size_t>(sizeof(T) * m_size), data));
    }
    else
    {
      m_usage = usage;
      m_size = size;
      GL_ASSERT(api::namedBufferData(getHandle(), gsl::narrow<std::size_t>(sizeof(T) * m_size), data, usage));
    }
  }

  void setSubData(const gsl::not_null<const T*>& data, const api::core::SizeType start, api::core::SizeType count)
  {
    Expects(m_size >= 0);
    Expects(count >= 0);

    if(count == 0)
    {
      count = m_size - start;
      Expects(count >= 0);
    }

    GL_ASSERT(api::namedBufferSubData(
      getHandle(), gsl::narrow<std::intptr_t>(sizeof(T) * start), gsl::narrow<std::size_t>(sizeof(T) * count), data));
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
  explicit ElementArrayBuffer(const std::string& label = {})
      : Buffer<T, api::BufferTarget::ElementArrayBuffer>{label}
  {
  }

  void drawElements(api::PrimitiveType primitiveType) const
  {
    GL_ASSERT(api::drawElements(
      primitiveType, Buffer<T, api::BufferTarget::ElementArrayBuffer>::size(), DrawElementsType<T>, nullptr));
  }

  void drawElements(api::PrimitiveType primitiveType, api::core::SizeType instances) const
  {
    GL_ASSERT(api::drawElementsInstance(
      primitiveType, Buffer<T, api::BufferTarget::ElementArrayBuffer>::size(), DrawElementsType<T>, instances));
  }
};
} // namespace gl
