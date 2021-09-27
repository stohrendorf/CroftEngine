#pragma once

#include "buffer.h"
#include "program.h"
#include "soglb_fwd.h" // IWYU pragma: export

#include <gsl/gsl-lite.hpp>
#include <map>
#include <string_view>

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
  // cppcheck-suppress noExplicitConstructor
  // NOLINTNEXTLINE(google-explicit-constructor)
  VertexAttribute(const U T::*member, const bool normalized = false)
      : m_type{VertexAttribType<U>}
      , m_relativeOffset{static_cast<uint32_t>(
          reinterpret_cast<uintptr_t>( // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
            &(static_cast<T*>(nullptr)->*member)))}
      , m_size{ElementCount<U>}
      , m_normalized{normalized}
  {
  }

  // cppcheck-suppress noExplicitConstructor
  // NOLINTNEXTLINE(google-explicit-constructor)
  VertexAttribute(const Trivial&, const bool normalized = false)
      : m_type{VertexAttribType<T>}
      , m_relativeOffset{0}
      , m_size{ElementCount<T>}
      , m_normalized{normalized}
  {
  }

  void bindVertexAttribute(api::core::Handle vertexArray,
                           const ProgramInput& input,
                           uint32_t bindingIndex,
                           uint32_t divisor) const;

private:
  const api::VertexAttribType m_type;
  const uint32_t m_relativeOffset;
  const int32_t m_size;
  const bool m_normalized;
};

template<typename T>
using VertexLayout = std::map<std::string, VertexAttribute<T>>;

template<typename T>
class VertexBuffer final : public ArrayBuffer<T>
{
public:
  explicit VertexBuffer(VertexLayout<T> layout, const std::string_view& label, uint32_t divisor = 0)
      : ArrayBuffer<T>{label}
      , m_layout{std::move(layout)}
      , m_divisor{divisor}
  {
    BOOST_ASSERT(!m_layout.empty());
  }

  void
    bindVertexAttributes(const api::core::Handle vertexArray, const Program& program, const uint32_t bindingIndex) const
  {
    for(const auto& input : program.getInputs())
    {
      auto it = m_layout.find(input.getName());
      if(it == m_layout.end())
        continue;

      it->second.bindVertexAttribute(vertexArray, input, bindingIndex, m_divisor);
    }
  }

  [[nodiscard]] static constexpr int getStride()
  {
    return sizeof(T);
  }

private:
  const VertexLayout<T> m_layout;
  const uint32_t m_divisor;
};

template<typename T>
void VertexAttribute<T>::bindVertexAttribute(const api::core::Handle vertexArray,
                                             const ProgramInput& input,
                                             const uint32_t bindingIndex,
                                             const uint32_t divisor) const
{
  GL_ASSERT(api::enableVertexArrayAttrib(vertexArray, input.getLocation()));
  GL_ASSERT(
    api::vertexArrayAttribFormat(vertexArray, input.getLocation(), m_size, m_type, m_normalized, m_relativeOffset));
  GL_ASSERT(api::vertexArrayAttribBinding(vertexArray, input.getLocation(), bindingIndex));
  GL_ASSERT(api::vertexArrayBindingDivisor(vertexArray, bindingIndex, divisor));
}
} // namespace gl
