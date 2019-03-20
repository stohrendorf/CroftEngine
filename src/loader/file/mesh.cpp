#include "mesh.h"

#include "render/textureanimator.h"
#include "color.h"
#include "render/scene/Base.h"
#include "render/scene/MeshPart.h"
#include "render/scene/MaterialParameter.h"
#include "render/scene/Model.h"
#include "render/gl/vertexarray.h"

#include <utility>

namespace loader
{
namespace file
{
#pragma pack(push, 1)


struct Mesh::ModelBuilder::RenderVertex
{
    glm::vec3 position;
    glm::vec4 color;
    glm::vec2 uv;

    static const render::gl::StructuredVertexBuffer::AttributeMapping& getFormat()
    {
        static const render::gl::StructuredVertexBuffer::AttributeMapping attribs{
                {VERTEX_ATTRIBUTE_POSITION_NAME,        render::gl::VertexAttribute{&RenderVertex::position}},
                {VERTEX_ATTRIBUTE_COLOR_NAME,           render::gl::VertexAttribute{&RenderVertex::color}},
                {VERTEX_ATTRIBUTE_TEXCOORD_PREFIX_NAME, render::gl::VertexAttribute{&RenderVertex::uv}}
        };

        return attribs;
    }
};


struct Mesh::ModelBuilder::RenderVertexWithNormal
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec4 color;
    glm::vec2 uv;

    static const render::gl::StructuredVertexBuffer::AttributeMapping& getFormat()
    {
        static const render::gl::StructuredVertexBuffer::AttributeMapping attribs{
                {VERTEX_ATTRIBUTE_POSITION_NAME,        render::gl::VertexAttribute{
                        &RenderVertexWithNormal::position}},
                {VERTEX_ATTRIBUTE_NORMAL_NAME,          render::gl::VertexAttribute{&RenderVertexWithNormal::normal}},
                {VERTEX_ATTRIBUTE_COLOR_NAME,           render::gl::VertexAttribute{&RenderVertexWithNormal::color}},
                {VERTEX_ATTRIBUTE_TEXCOORD_PREFIX_NAME, render::gl::VertexAttribute{&RenderVertexWithNormal::uv}}
        };

        return attribs;
    }
};


#pragma pack(pop)

const render::gl::StructuredVertexBuffer::AttributeMapping& Mesh::ModelBuilder::getFormat(const bool withNormals)
{
    return withNormals ? RenderVertexWithNormal::getFormat() : RenderVertex::getFormat();
}

Mesh::ModelBuilder::ModelBuilder(
        const bool withNormals,
        bool dynamic,
        const std::vector<TextureLayoutProxy>& textureProxies,
        const std::map<TextureKey, gsl::not_null<std::shared_ptr<render::scene::Material>>>& materials,
        gsl::not_null<std::shared_ptr<render::scene::Material>> colorMaterial,
        const Palette& palette,
        render::TextureAnimator& animator,
        const std::string& label)
        : m_hasNormals{withNormals}
        , m_textureProxies{textureProxies}
        , m_materials{materials}
        , m_colorMaterial{std::move( colorMaterial )}
        , m_palette{palette}
        , m_animator{animator}
        , m_mesh{std::make_shared<render::scene::Mesh>( getFormat( withNormals ), dynamic, label )}
{
}

Mesh::ModelBuilder::~ModelBuilder() = default;

void Mesh::ModelBuilder::append(const RenderVertex& v)
{
    static_assert( sizeof( RenderVertex ) % sizeof( float ) == 0, "Invalid vertex structure" );
    Expects( !m_hasNormals );
    Expects( sizeof( v ) == m_mesh->getBuffers()[0]->getVertexSize() );

    const auto* data = reinterpret_cast<const float*>(&v);
    const auto n = m_mesh->getBuffers()[0]->getVertexSize() / sizeof( float );
    std::copy_n( data, n, std::back_inserter( m_vbuf ) );
    ++m_vertexCount;
}

void Mesh::ModelBuilder::append(const RenderVertexWithNormal& v)
{
    static_assert( sizeof( RenderVertexWithNormal ) % sizeof( float ) == 0, "Invalid vertex structure" );
    Expects( m_hasNormals );
    Expects( sizeof( v ) == m_mesh->getBuffers()[0]->getVertexSize() );
    Expects( m_mesh->getBuffers()[0]->getVertexSize() % sizeof( float ) == 0 );

    const auto* data = reinterpret_cast<const float*>(&v);
    const auto n = m_mesh->getBuffers()[0]->getVertexSize() / sizeof( float );
    std::copy_n( data, n, std::back_inserter( m_vbuf ) );
    ++m_vertexCount;
}

void Mesh::ModelBuilder::append(const Mesh& mesh)
{
    if( mesh.normals.empty() && m_hasNormals )
        BOOST_THROW_EXCEPTION(
                std::runtime_error( "Trying to append a mesh with normals to a buffer without normals" ) );
    else if( !mesh.normals.empty() && !m_hasNormals )
        BOOST_THROW_EXCEPTION(
                std::runtime_error( "Trying to append a mesh without normals to a buffer with normals" ) );

    if( !m_hasNormals )
    {
        for( const QuadFace& quad : mesh.textured_rectangles )
        {
            const TextureLayoutProxy& proxy = m_textureProxies.at( quad.proxyId.get() );
            const auto partId = getPartForTexture( proxy );

            const auto firstVertex = m_vertexCount;
            for( int i = 0; i < 4; ++i )
            {
                RenderVertex iv;
                iv.position = quad.vertices[i].from( mesh.vertices ).toRenderSystem();
                if( quad.vertices[i].index < mesh.vertexDarknesses.size() )
                    iv.color = glm::vec4( 1 - quad.vertices[i].from( mesh.vertexDarknesses ) / 8191.0f );
                else
                    iv.color = glm::vec4( 1.0f );
                iv.uv = proxy.uvCoordinates[i].toGl();
                append( iv );
            }

            for( auto j : {0, 1, 2, 0, 2, 3} )
            {
                m_animator.registerVertex( quad.proxyId, m_mesh, j, firstVertex + j );
                m_parts[partId].indices
                               .emplace_back( gsl::narrow<MeshPart::IndexBuffer::value_type>( firstVertex + j ) );
            }
        }
        for( const QuadFace& quad : mesh.colored_rectangles )
        {
            const TextureLayoutProxy& proxy = m_textureProxies.at( quad.proxyId.get() );
            const auto partId = getPartForColor( quad.proxyId );

            const auto firstVertex = m_vertexCount;
            for( int i = 0; i < 4; ++i )
            {
                RenderVertex iv;
                iv.position = quad.vertices[i].from( mesh.vertices ).toRenderSystem();
                if( quad.vertices[i].index < mesh.vertexDarknesses.size() )
                    iv.color = glm::vec4( 1 - quad.vertices[i].from( mesh.vertexDarknesses ) / 8191.0f );
                else
                    iv.color = glm::vec4( 1.0f );
                iv.uv = proxy.uvCoordinates[i].toGl();
                append( iv );
            }

            for( auto j : {0, 1, 2, 0, 2, 3} )
            {
                m_parts[partId].indices
                               .emplace_back( gsl::narrow<MeshPart::IndexBuffer::value_type>( firstVertex + j ) );
            }
        }
        for( const Triangle& tri : mesh.textured_triangles )
        {
            const TextureLayoutProxy& proxy = m_textureProxies.at( tri.proxyId.get() );
            const auto partId = getPartForTexture( proxy );

            const auto firstVertex = m_vertexCount;
            for( int i = 0; i < 3; ++i )
            {
                RenderVertex iv;
                iv.position = tri.vertices[i].from( mesh.vertices ).toRenderSystem();
                if( tri.vertices[i].index < mesh.vertexDarknesses.size() )
                    iv.color = glm::vec4( 1 - tri.vertices[i].from( mesh.vertexDarknesses ) / 8191.0f );
                else
                    iv.color = glm::vec4( 1.0f );
                iv.uv = proxy.uvCoordinates[i].toGl();
                m_parts[partId].indices.emplace_back( gsl::narrow<MeshPart::IndexBuffer::value_type>( m_vertexCount ) );
                append( iv );
            }

            m_animator.registerVertex( tri.proxyId, m_mesh, 0, firstVertex + 0 );
            m_animator.registerVertex( tri.proxyId, m_mesh, 1, firstVertex + 1 );
            m_animator.registerVertex( tri.proxyId, m_mesh, 2, firstVertex + 2 );
        }
        for( const Triangle& tri : mesh.colored_triangles )
        {
            const TextureLayoutProxy& proxy = m_textureProxies.at( tri.proxyId.get() );
            const auto partId = getPartForColor( tri.proxyId );

            for( int i = 0; i < 3; ++i )
            {
                RenderVertex iv;
                iv.position = tri.vertices[i].from( mesh.vertices ).toRenderSystem();
                if( tri.vertices[i].index < mesh.vertexDarknesses.size() )
                    iv.color = glm::vec4( 1 - tri.vertices[i].from( mesh.vertexDarknesses ) / 8191.0f );
                else
                    iv.color = glm::vec4( 1.0f );
                iv.uv = proxy.uvCoordinates[i].toGl();
                m_parts[partId].indices.emplace_back( gsl::narrow<MeshPart::IndexBuffer::value_type>( m_vertexCount ) );
                append( iv );
            }
        }
    }
    else
    {
        for( const QuadFace& quad : mesh.textured_rectangles )
        {
            const TextureLayoutProxy& proxy = m_textureProxies.at( quad.proxyId.get() );
            const auto partId = getPartForTexture( proxy );

            const auto firstVertex = m_vertexCount;
            for( int i = 0; i < 4; ++i )
            {
                RenderVertexWithNormal iv;
                iv.position = quad.vertices[i].from( mesh.vertices ).toRenderSystem();
                iv.normal = quad.vertices[i].from( mesh.normals ).toRenderSystem();
                iv.color = glm::vec4( 1.0f );
                iv.uv = proxy.uvCoordinates[i].toGl();
                append( iv );
            }

            for( auto j : {0, 1, 2, 0, 2, 3} )
            {
                m_animator.registerVertex( quad.proxyId, m_mesh, j, firstVertex + j );
                m_parts[partId].indices
                               .emplace_back( gsl::narrow<MeshPart::IndexBuffer::value_type>( firstVertex + j ) );
            }
        }
        for( const QuadFace& quad : mesh.colored_rectangles )
        {
            const TextureLayoutProxy& proxy = m_textureProxies.at( quad.proxyId.get() );
            const auto partId = getPartForColor( quad.proxyId );

            const auto firstVertex = m_vertexCount;
            for( int i = 0; i < 4; ++i )
            {
                RenderVertexWithNormal iv;
                iv.position = quad.vertices[i].from( mesh.vertices ).toRenderSystem();
                iv.normal = quad.vertices[i].from( mesh.normals ).toRenderSystem();
                iv.color = glm::vec4( 1.0f );
                iv.uv = proxy.uvCoordinates[i].toGl();
                append( iv );
            }
            for( auto j : {0, 1, 2, 0, 2, 3} )
            {
                m_parts[partId].indices
                               .emplace_back( gsl::narrow<MeshPart::IndexBuffer::value_type>( firstVertex + j ) );
            }
        }
        for( const Triangle& tri : mesh.textured_triangles )
        {
            const TextureLayoutProxy& proxy = m_textureProxies.at( tri.proxyId.get() );
            const auto partId = getPartForTexture( proxy );

            const auto firstVertex = m_vertexCount;
            for( int i = 0; i < 3; ++i )
            {
                RenderVertexWithNormal iv;
                iv.position = tri.vertices[i].from( mesh.vertices ).toRenderSystem();
                iv.normal = tri.vertices[i].from( mesh.normals ).toRenderSystem();
                iv.color = glm::vec4( 1.0f );
                iv.uv = proxy.uvCoordinates[i].toGl();
                m_parts[partId].indices.emplace_back( gsl::narrow<MeshPart::IndexBuffer::value_type>( m_vertexCount ) );
                append( iv );
            }

            m_animator.registerVertex( tri.proxyId, m_mesh, 0, firstVertex + 0 );
            m_animator.registerVertex( tri.proxyId, m_mesh, 1, firstVertex + 1 );
            m_animator.registerVertex( tri.proxyId, m_mesh, 2, firstVertex + 2 );
        }
        for( const Triangle& tri : mesh.colored_triangles )
        {
            const TextureLayoutProxy& proxy = m_textureProxies.at( tri.proxyId.get() );
            const auto partId = getPartForColor( tri.proxyId );

            for( int i = 0; i < 3; ++i )
            {
                RenderVertexWithNormal iv;
                iv.position = tri.vertices[i].from( mesh.vertices ).toRenderSystem();
                iv.normal = tri.vertices[i].from( mesh.normals ).toRenderSystem();
                iv.color = glm::vec4( 1.0f );
                iv.uv = proxy.uvCoordinates[i].toGl();
                m_parts[partId].indices.emplace_back( gsl::narrow<MeshPart::IndexBuffer::value_type>( m_vertexCount ) );
                append( iv );
            }
        }
    }
}

gsl::not_null<std::shared_ptr<render::scene::Model>> Mesh::ModelBuilder::finalize()
{
    Expects( m_vbuf.size() * sizeof( m_vbuf[0] ) == m_vertexCount * m_mesh->getBuffers()[0]->getVertexSize() );

    m_mesh->getBuffers()[0]->assignRaw( m_vbuf, m_vertexCount );

    for( const MeshPart& localPart : m_parts )
    {
        static_assert( sizeof( localPart.indices[0] ) == sizeof( uint16_t ),
                       "Wrong index type" );
#ifndef NDEBUG
        for( auto idx : localPart.indices )
        {
            BOOST_ASSERT( idx < m_vertexCount );
        }
#endif
        render::gl::VertexArrayBuilder builder;

        auto indexBuffer = std::make_shared<render::gl::IndexBuffer>();
        indexBuffer->setData( localPart.indices, true );
        builder.attach( indexBuffer );

        builder.attach( m_mesh->getBuffers() );

        auto part = std::make_shared<render::scene::MeshPart>(
                builder.build( localPart.material->getShaderProgram()->getHandle() ) );
        m_mesh->addPart( part );
        part->setMaterial( localPart.material );
        if( localPart.color.is_initialized() )
        {
            part->registerMaterialParameterSetter(
                    [color = *localPart.color](const render::scene::Node& /*node*/, render::scene::Material& material) {
                        material.getParameter( "u_diffuseColor" )->set( color );
                    } );
        }
    }

    auto model = std::make_shared<render::scene::Model>();
    model->addMesh( m_mesh );
    return model;
}

std::shared_ptr<render::scene::Model> Mesh::createModel(
        const std::vector<TextureLayoutProxy>& textureProxies,
        const std::map<TextureKey, gsl::not_null<std::shared_ptr<render::scene::Material>>>& materials,
        const gsl::not_null<std::shared_ptr<render::scene::Material>>& colorMaterial,
        const Palette& palette,
        render::TextureAnimator& animator,
        const std::string& label) const
{
    ModelBuilder mb{
            !normals.empty(),
            false,
            textureProxies,
            materials,
            colorMaterial,
            palette,
            animator,
            label
    };

    mb.append( *this );

    return mb.finalize();
}
}
}
