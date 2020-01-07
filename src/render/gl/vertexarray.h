#pragma once

#include "buffer.h"
#include "program.h"
#include "vertexbuffer.h"

#include <tuple>
#include <utility>
#include <vector>

namespace render::gl
{
template<typename IndexT, typename VertexT0, typename... VertexTs>
class VertexArray : public BindableResource
{
private:
  template<typename... Ts, size_t... Is>
  void bindVertexAttributes(const std::tuple<Ts...>& t, const Program& p, const std::index_sequence<Is...>&)
  {
    (..., std::get<Is>(t)->bindVertexAttributes(getHandle(), p, Is));
  }

  void bindVertexAttributes(const Program& p)
  {
    bindVertexAttributes(m_vertexBuffers, p, std::make_index_sequence<1 + sizeof...(VertexTs)>());
  }

  template<typename... Ts, size_t... Is>
  void extractBindContextData(const std::tuple<Ts...>& t,
                              std::vector<::gl::core::Handle>& handles,
                              std::vector<int>& strides,
                              const std::index_sequence<Is...>&)
  {
    handles = std::vector<::gl::core::Handle>{std::get<Is>(t)->getHandle()...};
    strides = std::vector<int>{std::get<Is>(t)->getStride()...};
  }

  void extractBindContextData(std::vector<::gl::core::Handle>& handles, std::vector<int>& strides)
  {
    extractBindContextData(m_vertexBuffers, handles, strides, std::make_index_sequence<1 + sizeof...(VertexTs)>());
  }

public:
  template<typename T>
  using VertexBufferPtr = gsl::not_null<std::shared_ptr<VertexBuffer<T>>>;
  using VertexBuffers = std::tuple<VertexBufferPtr<VertexT0>, VertexBufferPtr<VertexTs>...>;

  using IndexBufferPtr = gsl::not_null<std::shared_ptr<ElementArrayBuffer<IndexT>>>;

  explicit VertexArray(IndexBufferPtr indexBuffer,
                       VertexBuffers vertexBuffers,
                       const std::vector<const Program*>& programs,
                       const std::string& label = {})
      : BindableResource{::gl::createVertexArrays,
                         ::gl::bindVertexArray,
                         ::gl::deleteVertexArrays,
                         ::gl::ObjectIdentifier::VertexArray,
                         label}
      , m_indexBuffer{std::move(indexBuffer)}
      , m_vertexBuffers{std::move(vertexBuffers)}
  {
    GL_ASSERT(::gl::vertexArrayElementBuffer(getHandle(), m_indexBuffer->getHandle()));
    std::vector<::gl::core::Handle> vbos;
    std::vector<int> strides;
    extractBindContextData(vbos, strides);
    Expects(vbos.size() == strides.size());
    Expects(vbos.size() == sizeof...(VertexTs) + 1);
    std::vector<intptr_t> offsets;
    offsets.resize(vbos.size(), 0);
    GL_ASSERT(::gl::vertexArrayVertexBuffers(
      getHandle(), 0, 1 + sizeof...(VertexTs), vbos.data(), offsets.data(), strides.data()));

    for(const auto& program : programs)
    {
      if(program != nullptr)
        bindVertexAttributes(*program);
    }
  }

  const auto& getIndexBuffer() const
  {
    return m_indexBuffer;
  }

  const auto& getVertexBuffers() const
  {
    return m_vertexBuffers;
  }

  void drawIndexBuffer(::gl::PrimitiveType primitiveType)
  {
    bind();
    m_indexBuffer->drawElements(primitiveType);
    unbind();
  }

private:
  IndexBufferPtr m_indexBuffer;
  VertexBuffers m_vertexBuffers;
};
} // namespace render::gl
