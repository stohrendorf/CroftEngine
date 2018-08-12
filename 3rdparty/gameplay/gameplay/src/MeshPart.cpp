#include "MeshPart.h"

#include "Material.h"

namespace gameplay
{
MeshPart::MeshPart(const std::shared_ptr<gl::VertexArray>& vao)
        : m_vao{vao}
{
}

MeshPart::~MeshPart() = default;

void MeshPart::draw(RenderContext& context) const
{
    if( m_material == nullptr )
        return;

    BOOST_ASSERT( context.getCurrentNode() != nullptr );

    for( const auto& mps : m_materialParameterSetters )
        mps( *context.getCurrentNode(), *m_material );

    m_material->bind( *context.getCurrentNode() );

    m_vao->bind();

    for( const auto& buffer : m_vao->getIndexBuffers() )
    {
        buffer->draw( GL_TRIANGLES );
    }

    m_vao->unbind();
}
}
