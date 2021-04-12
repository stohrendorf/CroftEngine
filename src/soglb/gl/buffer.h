#pragma once

#include "bindableresource.h"
#include "typetraits.h"

namespace gl
{
// NOLINTNEXTLINE(bugprone-reserved-identifier)
template<typename T, api::BufferTargetARB _Target>
class Buffer : public BindableResource
{
public:
  static constexpr api::BufferTargetARB Target = _Target;

  explicit Buffer(const std::string& label = {})
      : BindableResource{api::createBuffers,
                         [](const uint32_t handle) { bindBuffer(Target, handle); },
                         api::deleteBuffers,
                         api::ObjectIdentifier::Buffer,
                         label}
  {
  }

  [[nodiscard]] T* map(const api::BufferAccessARB access = api::BufferAccessARB::ReadOnly)
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

  void setData(const T& data, const api::BufferUsageARB usage)
  {
    setData(&data, 1, usage);
  }

  void setData(const std::vector<T>& data, const api::BufferUsageARB usage)
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

  void setData(const gsl::not_null<const T*>& data, const api::core::SizeType size, const api::BufferUsageARB usage)
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
  api::BufferUsageARB m_usage{static_cast<api::BufferUsageARB>(0)};
};

template<typename T>
using ShaderStorageBuffer = Buffer<T, api::BufferTargetARB::ShaderStorageBuffer>;

template<typename T>
using UniformBuffer = Buffer<T, api::BufferTargetARB::UniformBuffer>;

template<typename T>
using ArrayBuffer = Buffer<T, api::BufferTargetARB::ArrayBuffer>;

template<typename T>
class ElementArrayBuffer final : public Buffer<T, api::BufferTargetARB::ElementArrayBuffer>
{
public:
  void drawElements(api::PrimitiveType primitiveType) const
  {
    GL_ASSERT(api::drawElements(
      primitiveType, Buffer<T, api::BufferTargetARB::ElementArrayBuffer>::size(), DrawElementsType<T>, nullptr));
  }
};
} // namespace gl
