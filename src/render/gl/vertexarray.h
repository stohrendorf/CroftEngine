#pragma once

#include "bindableresource.h"
#include "elementarraybuffer.h"
#include "program.h"
#include "structuredarraybuffer.h"

#include <tuple>
#include <utility>
#include <vector>

namespace render
{
namespace gl
{
namespace detail
{
template<std::size_t I = 0, typename... Ts>
inline typename std::enable_if<I == sizeof...(Ts), void>::type bindAll(std::tuple<Ts...>&, const Program& program)
{
}

template<std::size_t I = 0, typename... Ts>
inline typename std::enable_if<(I < sizeof...(Ts)), void>::type bindAll(std::tuple<Ts...>& t, const Program& program)
{
    std::get<I>(t)->bind(program);
    bindAll<I + 1, Ts...>(t, program);
}
} // namespace detail

template<typename IndexT, typename VertexT0, typename... VertexTs>
class VertexArray : public BindableResource
{
public:
    explicit VertexArray(std::vector<gsl::not_null<std::shared_ptr<ElementArrayBuffer<IndexT>>>> indexBuffers,
                         std::tuple<gsl::not_null<std::shared_ptr<StructuredArrayBuffer<VertexT0>>>,
                                    gsl::not_null<std::shared_ptr<StructuredArrayBuffer<VertexTs>>>...> vertexBuffers,
                         const Program& program,
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
        detail::bindAll(m_vertexBuffers, program);
        unbind();
    }

    explicit VertexArray(const gsl::not_null<std::shared_ptr<ElementArrayBuffer<IndexT>>>& indexBuffer,
                         const gsl::not_null<std::shared_ptr<StructuredArrayBuffer<VertexT0>>>& vertexBuffer,
                         const Program& program,
                         const std::string& label = {})
        : VertexArray{std::vector<gsl::not_null<std::shared_ptr<ElementArrayBuffer<IndexT>>>>{indexBuffer},
                      std::tuple<gsl::not_null<std::shared_ptr<StructuredArrayBuffer<VertexT0>>>>{vertexBuffer},
                      program,
                      label}
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

private:
    std::vector<gsl::not_null<std::shared_ptr<ElementArrayBuffer<IndexT>>>> m_indexBuffers;

    std::tuple<gsl::not_null<std::shared_ptr<StructuredArrayBuffer<VertexT0>>>,
               gsl::not_null<std::shared_ptr<StructuredArrayBuffer<VertexTs>>>...>
        m_vertexBuffers;
};
} // namespace gl
} // namespace render
