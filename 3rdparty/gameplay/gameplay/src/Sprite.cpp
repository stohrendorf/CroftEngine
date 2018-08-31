#include "Sprite.h"

#include "gameplay.h"

namespace gameplay
{
gsl::not_null<std::shared_ptr<Mesh>> Sprite::createMesh(float left,
                                                        float bottom,
                                                        float width,
                                                        float height,
                                                        const glm::vec2& uvTopLeft,
                                                        const glm::vec2& uvBottomRight,
                                                        const gsl::not_null<std::shared_ptr<Material>>& material,
                                                        const Axis pole)
{
    struct SpriteVertex
    {
        glm::vec3 pos;

        glm::vec2 uv;

        glm::vec3 color{1.0f};
    };

    const SpriteVertex vertices[]{
            {{left,         bottom,          0}, {uvTopLeft.x,     uvBottomRight.y}},
            {{left + width, bottom,          0}, {uvBottomRight.x, uvBottomRight.y}},
            {{left + width, bottom + height, 0}, {uvBottomRight.x, uvTopLeft.y}},
            {{left,         bottom + height, 0}, {uvTopLeft.x,     uvTopLeft.y}}
    };

    gl::StructuredVertexBuffer::AttributeMapping attribs{
            {VERTEX_ATTRIBUTE_POSITION_NAME,        gl::VertexAttribute{&SpriteVertex::pos}},
            {VERTEX_ATTRIBUTE_TEXCOORD_PREFIX_NAME, gl::VertexAttribute{&SpriteVertex::uv}},
            {VERTEX_ATTRIBUTE_COLOR_NAME,           gl::VertexAttribute{&SpriteVertex::color}}
    };

    auto mesh = make_not_null_shared<Mesh>( attribs, false );
    mesh->getBuffer( 0 )->assign<SpriteVertex>( to_not_null( &vertices[0] ), 4 );

    static const uint16_t indices[6] =
            {
                    0, 1, 2,
                    0, 2, 3
            };

    gl::VertexArrayBuilder builder;

    auto indexBuffer = make_not_null_shared<gl::IndexBuffer>();
    indexBuffer->setData( to_not_null( &indices[0] ), 6, false );
    builder.attach( indexBuffer );
    builder.attach( mesh->getBuffers() );

    auto part = make_not_null_shared<MeshPart>(
            builder.build( material->getShaderProgram()->getHandle() ) );

    mesh->addPart( part );
    part->setMaterial( material );

    part->registerMaterialParameterSetter(
            [pole](const Node& node, Material& material) {
                auto m = node.getModelViewMatrix();
                // clear out rotation component
                for( int i : {0, 1, 2} )
                {
                    switch( pole )
                    {

                        case Axis::X:
                            if( i == 0 )
                                continue;
                            break;
                        case Axis::Y:
                            if( i == 1 )
                                continue;
                            break;
                        case Axis::Z:
                            if( i == 2 )
                                continue;
                            break;
                        default:
                            continue;
                    }

                    for( int j = 0; j < 3; ++j )
                        m[i][j] = i == j ? 1.0f : 0.0f;
                }

                material.getParameter( "u_modelViewMatrix" )->set( m );
            }
    );

    return mesh;
}

void Sprite::draw(RenderContext& context)
{
    context.pushState( m_renderState );
    for( const gsl::not_null<std::shared_ptr<MeshPart>>& part : m_mesh->getParts() )
    {
        part->draw( context );
    }
    context.popState();
}
}