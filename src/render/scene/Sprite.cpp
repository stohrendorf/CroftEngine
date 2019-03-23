#include "Sprite.h"

#include "names.h"
#include "Material.h"
#include "mesh.h"
#include "Node.h"
#include "render/gl/vertexarray.h"
#include "render/gl/indexbuffer.h"

namespace render
{
namespace scene
{
gsl::not_null<std::shared_ptr<Mesh>> Sprite::createMesh(const float x0,
                                                        const float y0,
                                                        const float x1,
                                                        const float y1,
                                                        const glm::vec2& t0,
                                                        const glm::vec2& t1,
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
            {{x0, y0, 0}, {t0.x, t0.y}},
            {{x1, y0, 0}, {t1.x, t0.y}},
            {{x1, y1, 0}, {t1.x, t1.y}},
            {{x0, y1, 0}, {t0.x, t1.y}}
    };

    gl::StructuredVertexBuffer::AttributeMapping layout{
            {VERTEX_ATTRIBUTE_POSITION_NAME,        gl::VertexAttribute{&SpriteVertex::pos}},
            {VERTEX_ATTRIBUTE_TEXCOORD_PREFIX_NAME, gl::VertexAttribute{&SpriteVertex::uv}},
            {VERTEX_ATTRIBUTE_COLOR_NAME,           gl::VertexAttribute{&SpriteVertex::color}}
    };
    auto vb = std::make_shared<render::gl::StructuredVertexBuffer>(layout, false);
    vb->assign<SpriteVertex>( &vertices[0], 4 );

    static const uint16_t indices[6] =
            {
                    0, 1, 2,
                    0, 2, 3
            };

    auto indexBuffer = std::make_shared<render::gl::IndexBuffer>();
    indexBuffer->setData( gsl::not_null<const uint16_t*>( &indices[0] ), 6, false );

    auto vao = std::make_shared<render::gl::VertexArray>(indexBuffer, vb, material->getShaderProgram()->getHandle());
    auto mesh = std::make_shared<Mesh>( vao );
    mesh->setMaterial( material );

    mesh->registerMaterialParameterSetter(
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
    m_mesh->draw(context);
}
}
}
