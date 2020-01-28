#pragma once

#include "buffer.h"
#include "program.h"
#include "typetraits.h"

#include <gsl-lite.hpp>
#include <map>

namespace gl
{
template<typename T>
class VertexBuffer;

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

  void bindVertexAttribute(api::core::Handle vertexArray, const ProgramInput& input, uint32_t bindingIndex) const;

private:
  const api::VertexAttribType m_type;
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

  void
    bindVertexAttributes(const api::core::Handle vertexArray, const Program& program, const uint32_t bindingIndex) const
  {
    for(const auto& input : program.getInputs())
    {
      auto it = m_format.find(input.getName());
      if(it == m_format.end())
        continue;

      it->second.bindVertexAttribute(vertexArray, input, bindingIndex);
    }
  }

  [[nodiscard]] const VertexFormat<T>& getFormat() const
  {
    return m_format;
  }

  [[nodiscard]] constexpr int getStride() const
  {
    return sizeof(T);
  }

private:
  const VertexFormat<T> m_format;
};

template<typename T>
void VertexAttribute<T>::bindVertexAttribute(const api::core::Handle vertexArray,
                                             const ProgramInput& input,
                                             const uint32_t bindingIndex) const
{
  GL_ASSERT(api::enableVertexArrayAttrib(vertexArray, input.getLocation()));
  GL_ASSERT(
    api::vertexArrayAttribFormat(vertexArray, input.getLocation(), m_size, m_type, m_normalized, m_relativeOffset));
  GL_ASSERT(api::vertexArrayAttribBinding(vertexArray, input.getLocation(), bindingIndex));
}
} // namespace gl
