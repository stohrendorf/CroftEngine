#pragma once

#include "bindableresource.h"
#include "elementarraybuffer.h"
#include "program.h"
#include "structuredarraybuffer.h"

#include <tuple>
#include <utility>
#include <vector>

namespace render::gl
{
namespace detail
{
template<typename... Ts, size_t... Is>
inline void
  bindVertexAttributes(const std::tuple<Ts...>& t, const Program& p, const std::index_sequence<Is...>& indices)
{
  (..., std::get<Is>(t)->bindVertexAttributes(p));
}

template<typename... Ts>
inline void bindVertexAttributes(const std::tuple<Ts...>& t, const Program& p)
{
  bindVertexAttributes(t, p, std::make_index_sequence<sizeof...(Ts)>());
}
} // namespace detail

template<typename IndexT, typename VertexT0, typename... VertexTs>
class VertexArray : public BindableResource
{
public:
  template<typename T>
  using VertexBufferPtr = gsl::not_null<std::shared_ptr<StructuredArrayBuffer<T>>>;
  using VertexBuffers = std::tuple<VertexBufferPtr<VertexT0>, VertexBufferPtr<VertexTs>...>;

  using IndexBufferPtr = gsl::not_null<std::shared_ptr<ElementArrayBuffer<IndexT>>>;
  using IndexBuffers = std::vector<IndexBufferPtr>;

  explicit VertexArray(IndexBuffers indexBuffers,
                       VertexBuffers vertexBuffers,
                       const std::vector<const Program*>& programs,
                       const std::string& label = {})
      : BindableResource{::gl::genVertexArrays,
                         ::gl::bindVertexArray,
                         ::gl::deleteVertexArrays,
                         ::gl::ObjectIdentifier::VertexArray,
                         label}
      , m_indexBuffers{std::move(indexBuffers)}
      , m_vertexBuffers{std::move(vertexBuffers)}
  {
    bind();
    for(const auto& buffer : m_indexBuffers)
      buffer->bind();
    for(const auto& program : programs)
      if(program != nullptr)
        detail::bindVertexAttributes(m_vertexBuffers, *program);
    unbind();
  }

  explicit VertexArray(const IndexBufferPtr& indexBuffer,
                       const VertexBufferPtr<VertexT0>& vertexBuffer,
                       const std::vector<const Program*>& programs,
                       const std::string& label = {})
      : VertexArray{IndexBuffers{indexBuffer}, VertexBuffers{vertexBuffer}, programs, label}
  {
  }

  const auto& getIndexBuffers() const
  {
    return m_indexBuffers;
  }

  const auto& getVertexBuffers() const
  {
    return m_vertexBuffers;
  }

  void drawIndexBuffers(::gl::PrimitiveType primitiveType)
  {
    bind();

    for(const auto& buffer : m_indexBuffers)
    {
      buffer->drawElements(primitiveType);
    }

    unbind();
  }

private:
  IndexBuffers m_indexBuffers;
  VertexBuffers m_vertexBuffers;
};
} // namespace render::gl
