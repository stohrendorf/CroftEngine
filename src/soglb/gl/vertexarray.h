#pragma once

#include "bindableresource.h" // IWYU pragma: export
#include "renderstate.h"
#include "soglb_fwd.h"

#include <gslu.h>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

namespace gl
{
template<typename IndexT, typename VertexT0, typename... VertexTs>
class VertexArray final : public BindableResource<api::ObjectIdentifier::VertexArray>
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
                              std::vector<api::core::Handle>& handles,
                              std::vector<int>& strides,
                              const std::index_sequence<Is...>&)
  {
    handles = std::vector{std::get<Is>(t)->getHandle()...};
    strides = std::vector{std::get<Is>(t)->getStride()...};
  }

  void extractBindContextData(std::vector<api::core::Handle>& handles, std::vector<int>& strides)
  {
    extractBindContextData(m_vertexBuffers, handles, strides, std::make_index_sequence<1 + sizeof...(VertexTs)>());
  }

public:
  template<typename T>
  using VertexBufferPtr = gslu::nn_shared<VertexBuffer<T>>;
  using VertexBuffers = std::tuple<VertexBufferPtr<VertexT0>, VertexBufferPtr<VertexTs>...>;

  using IndexBufferPtr = gslu::nn_shared<ElementArrayBuffer<IndexT>>;

  explicit VertexArray(IndexBufferPtr indexBuffer,
                       VertexBuffers vertexBuffers,
                       const std::vector<const Program*>& programs,
                       const std::string_view& label)
      : BindableResource{api::createVertexArrays, api::bindVertexArray, api::deleteVertexArrays, label}
      , m_indexBuffer{std::move(indexBuffer)}
      , m_vertexBuffers{std::move(vertexBuffers)}
  {
    GL_ASSERT(api::vertexArrayElementBuffer(getHandle(), m_indexBuffer->getHandle()));
    std::vector<api::core::Handle> vbos;
    std::vector<int> strides;
    extractBindContextData(vbos, strides);
    gsl_Assert(vbos.size() == strides.size());
    gsl_Assert(vbos.size() == sizeof...(VertexTs) + 1);
    std::vector<intptr_t> offsets;
    offsets.resize(vbos.size(), 0);
    GL_ASSERT(api::vertexArrayVertexBuffers(
      getHandle(), 0, 1 + sizeof...(VertexTs), vbos.data(), offsets.data(), strides.data()));

    for(const auto& program : programs)
    {
      if(program != nullptr)
        bindVertexAttributes(*program);
    }
  }

  void drawElements(api::PrimitiveType primitiveType)
  {
    RenderState::applyWantedState();
    bind();
    m_indexBuffer->drawElements(primitiveType);
    unbind();
  }

  void drawElements(api::PrimitiveType primitiveType, api::core::SizeType instanceCount)
  {
    RenderState::applyWantedState();
    bind();
    m_indexBuffer->drawElements(primitiveType, instanceCount);
    unbind();
  }

  [[nodiscard]] bool empty() const
  {
    return m_indexBuffer->empty();
  }

private:
  IndexBufferPtr m_indexBuffer;
  VertexBuffers m_vertexBuffers;
};
} // namespace gl
