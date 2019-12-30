#pragma once

#include "buffer.h"
#include "program.h"
#include "typetraits.h"

#include <gsl-lite.hpp>
#include <map>

namespace render::gl
{
template<typename T>
class VertexAttribute final
{
public:
  struct Trivial
  {
  };

  template<typename U>
  VertexAttribute(const U T::*member, const bool normalized = false)
      : m_type{TypeTraits<U>::VertexAttribType}
      , m_relativeOffset{static_cast<uint32_t>(reinterpret_cast<uintptr_t>(&(static_cast<T*>(nullptr)->*member)))}
      , m_size{TypeTraits<U>::ElementCount}
      , m_normalized{normalized}
  {
  }

  VertexAttribute(const Trivial&, const bool normalized = false)
      : m_type{TypeTraits<T>::VertexAttribType}
      , m_relativeOffset{0}
      , m_size{TypeTraits<T>::ElementCount}
      , m_normalized{normalized}
  {
  }

  void bindVertexAttribute(const ProgramInput& input, const uint32_t bindingIndex) const
  {
    GL_ASSERT(::gl::enableVertexAttribArray(input.getLocation()));
    GL_ASSERT(::gl::vertexAttribFormat(input.getLocation(), m_size, m_type, m_normalized, m_relativeOffset));
    GL_ASSERT(::gl::vertexAttribBinding(input.getLocation(), bindingIndex));
  }

private:
  const ::gl::VertexAttribType m_type;
  const uint32_t m_relativeOffset;
  const int32_t m_size;
  const bool m_normalized;
};

template<typename T>
using VertexFormat = std::map<std::string, VertexAttribute<T>>;

template<typename T>
class VertexBuffer : public ArrayBuffer<T>
{
public:
  explicit VertexBuffer(const VertexFormat<T>& format, const std::string& label = {})
      : ArrayBuffer<T>{label}
      , m_format{format}
  {
    BOOST_ASSERT(!m_format.empty());
  }

  void bindVertexAttributes(const Program& program, const uint32_t bindingIndex) const
  {
    GL_ASSERT(::gl::bindVertexBuffer(bindingIndex, getHandle(), 0, sizeof(T)));

    for(const auto& input : program.getInputs())
    {
      auto it = m_format.find(input.getName());
      if(it == m_format.end())
        continue;

      it->second.bindVertexAttribute(input, bindingIndex);
    }
  }

  [[nodiscard]] const VertexFormat<T>& getFormat() const
  {
    return m_format;
  }

private:
  const VertexFormat<T> m_format;
};
} // namespace render::gl
