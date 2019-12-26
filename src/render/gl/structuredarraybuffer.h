#pragma once

#include "buffer.h"
#include "program.h"
#include "typetraits.h"

#include <gsl-lite.hpp>
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

  [[nodiscard]] const StructureLayout<T>& getStructureLayout() const
  {
    return m_structureLayout;
  }

private:
  const StructureLayout<T> m_structureLayout;
};
} // namespace render::gl
