#pragma once

#include "arraybuffer.h"
#include "gsl-lite.hpp"
#include "program.h"
#include "typetraits.h"

#include <map>

namespace render::gl
{
template<typename T>
class StructureMember final
{
public:
  struct Trivial
  {
  };

  template<typename U>
  StructureMember(const U T::*member, const bool normalized = false)
      : m_type{TypeTraits<U>::VertexAttribPointerType}
      , m_pointer{&(static_cast<T*>(nullptr)->*member)}
      , m_size{TypeTraits<U>::ElementCount}
      , m_normalized{normalized}
  {
  }

  StructureMember(const Trivial&, const bool normalized = false)
      : m_type{TypeTraits<T>::VertexAttribPointerType}
      , m_pointer{nullptr}
      , m_size{TypeTraits<T>::ElementCount}
      , m_normalized{normalized}
  {
  }

  void bindVertexAttribute(const uint32_t index) const
  {
    GL_ASSERT(::gl::vertexAttribPointer(index, m_size, m_type, m_normalized, sizeof(T), m_pointer));
    GL_ASSERT(::gl::enableVertexAttribArray(index));
  }

  [[nodiscard]] std::uintptr_t getOffset() const noexcept
  {
    return reinterpret_cast<std::uintptr_t>(m_pointer);
  }

private:
  const ::gl::VertexAttribPointerType m_type;

  const void* const m_pointer;

  const int32_t m_size;

  const bool m_normalized;
};

template<typename T>
using StructureLayout = std::map<std::string, StructureMember<T>>;

template<typename T>
class StructuredArrayBuffer : public ArrayBuffer<T>
{
public:
  explicit StructuredArrayBuffer(const StructureLayout<T>& layout, const std::string& label = {})
      : ArrayBuffer<T>{label}
      , m_structureLayout{layout}
  {
    BOOST_ASSERT(!layout.empty());
  }

  void bindVertexAttributes(const Program& program) const
  {
    ArrayBuffer<T>::bind();

    for(const auto& input : program.getInputs())
    {
      auto it = m_structureLayout.find(input.getName());
      if(it == m_structureLayout.end())
        continue;

      it->second.bindVertexAttribute(input.getLocation());
    }
  }

  void setSubData(const gsl::not_null<const T*>& data, const size_t start, size_t count)
  {
    ArrayBuffer<T>::bind();

    if(count == 0)
    {
      count = ArrayBuffer<T>::size() - start;
    }

    GL_ASSERT(::gl::bufferSubData(::gl::BufferTargetARB::ArrayBuffer, start * sizeof(T), count * sizeof(T), data));
  }

  void setData(const gsl::not_null<const T*>& data, const size_t count, const ::gl::BufferUsageARB access)
  {
    ArrayBuffer<T>::bind();

    if(count != 0)
      ArrayBuffer<T>::m_size = gsl::narrow<::gl::core::SizeType>(count);

    GL_ASSERT(::gl::bufferData(::gl::BufferTargetARB::ArrayBuffer, sizeof(T) * ArrayBuffer<T>::size(), data, access));
  }

  void setDataRaw(const gsl::not_null<const T*>& data, const size_t count, const ::gl::BufferUsageARB access)
  {
    ArrayBuffer<T>::bind();

    if(count != 0)
      ArrayBuffer<T>::m_size = count;

    GL_ASSERT(::gl::bufferData(::gl::BufferTargetARB::ArrayBuffer, sizeof(T) * ArrayBuffer<T>::size(), data, access));
  }

  void setData(const std::vector<T>& data, const ::gl::BufferUsageARB access)
  {
    if(!data.empty())
      setData(data.data(), data.size(), access);
  }

  void setDataRaw(const std::vector<T>& data, const size_t count, const ::gl::BufferUsageARB access)
  {
    if(!data.empty())
      setDataRaw(data.data(), count, access);
  }

  [[nodiscard]] const StructureLayout<T>& getStructureLayout() const
  {
    return m_structureLayout;
  }

private:
  const StructureLayout<T> m_structureLayout;
};
} // namespace render
