#include "mesh.h"

#include "names.h"
#include "Material.h"

#include <utility>

namespace render
{
namespace scene
{
gsl::not_null<std::shared_ptr<Mesh>> createQuadFullscreen(const float width,
                                                          const float height,
                                                          const gl::Program& program,
                                                          const bool invertY)
{
    struct Vertex
    {
        glm::vec2 pos;

        glm::vec2 uv;
    };

    const Vertex vertices[]{
            {{0.0f,  0.0f},   {0.0f, invertY ? 0.0f : 1.0f}},
            {{width, 0.0f},   {1.0f, invertY ? 0.0f : 1.0f}},
            {{width, height}, {1.0f, invertY ? 1.0f : 0.0f}},
            {{0.0f,  height}, {0.0f, invertY ? 1.0f : 0.0f}}
    };

    static const gl::StructuredVertexBuffer::AttributeMapping attribs{
            {VERTEX_ATTRIBUTE_POSITION_NAME,        gl::VertexAttribute{&Vertex::pos}},
            {VERTEX_ATTRIBUTE_TEXCOORD_PREFIX_NAME, gl::VertexAttribute{&Vertex::uv}}
    };

    auto vertexBuffer = std::make_shared<gl::StructuredVertexBuffer>( attribs, false );
    vertexBuffer->assign<Vertex>( &vertices[0], 4 );

    static const uint16_t indices[6] =
            {
                    0, 1, 2,
                    0, 2, 3
            };

    auto indexBuffer = std::make_shared<gl::IndexBuffer>();
    indexBuffer->setData( gsl::not_null<const uint16_t*>( &indices[0] ), 6, false );

    return std::make_shared<Mesh>( std::make_shared<gl::VertexArray>(indexBuffer, vertexBuffer, program) );
}

Mesh::Mesh(std::shared_ptr<gl::VertexArray> vao, ::gl::GLenum mode)
        : m_vao{std::move( vao )}
        , m_mode{mode}
{
}

Mesh::~Mesh() = default;

void Mesh::render(RenderContext& context)
{
    if( m_material == nullptr )
        return;

    BOOST_ASSERT( context.getCurrentNode() != nullptr );

    context.pushState( getRenderState() );

    for( const auto& mps : m_materialParameterSetters )
        mps( *context.getCurrentNode(), *m_material );

    context.pushState( m_material->getRenderState() );
    context.bindState();

    m_material->bind( *context.getCurrentNode() );

    m_vao->bind();

    for( const auto& buffer : m_vao->getIndexBuffers() )
    {
        buffer->draw( m_mode );
    }

    m_vao->unbind();
    context.popState();
    context.popState();
}
}
}
