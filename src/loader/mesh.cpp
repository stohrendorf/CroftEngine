#include "mesh.h"

#include "render/textureanimator.h"


namespace loader
{
#pragma pack(push,1)
    struct Mesh::ModelBuilder::RenderVertex
    {
        gameplay::Vector2 texcoord0;
        gameplay::Vector3 position;


        static const gameplay::VertexFormat& getFormat()
        {
            static const gameplay::VertexFormat::Element elems[2] = {
                {gameplay::VertexFormat::TEXCOORD0, 2},
                {gameplay::VertexFormat::POSITION, 3}
            };
            static const gameplay::VertexFormat fmt{elems, 2};

            return fmt;
        }
    };


    struct Mesh::ModelBuilder::RenderVertexWithNormal
    {
        gameplay::Vector2 texcoord0;
        gameplay::Vector3 position;
        gameplay::Vector3 normal;


        static const gameplay::VertexFormat& getFormat()
        {
            static const gameplay::VertexFormat::Element elems[3] = {
                {gameplay::VertexFormat::TEXCOORD0, 2},
                {gameplay::VertexFormat::POSITION, 3},
                {gameplay::VertexFormat::NORMAL, 3}
            };
            static const gameplay::VertexFormat fmt{elems, 3};

            return fmt;
        }
    };
#pragma pack(pop)


    Mesh::ModelBuilder::ModelBuilder(bool withNormals,
                                     bool dynamic,
                                     const std::vector<TextureLayoutProxy>& textureProxies,
                                     const std::map<TextureLayoutProxy::TextureKey, std::shared_ptr<gameplay::Material>>& materials,
                                     const std::vector<std::shared_ptr<gameplay::Material>>& colorMaterials,
                                     render::TextureAnimator& animator)
        : m_hasNormals{withNormals}
        , m_textureProxies{textureProxies}
        , m_materials{materials}
        , m_colorMaterials{colorMaterials}
        , m_animator{animator}
        , m_mesh{gameplay::Mesh::createMesh(withNormals ? RenderVertexWithNormal::getFormat() : RenderVertex::getFormat(), 0, dynamic)}
    {
        Expects(colorMaterials.size() == 256);
    }


    Mesh::ModelBuilder::~ModelBuilder() = default;


    void Mesh::ModelBuilder::append(const RenderVertex& v)
    {
        static_assert(sizeof(RenderVertex) % sizeof(float) == 0, "Invalid vertex structure");
        const float* data = reinterpret_cast<const float*>(&v);
        std::copy_n(data, sizeof(RenderVertex) / sizeof(float), std::back_inserter(m_vbuf));
        ++m_vertexCount;
    }


    void Mesh::ModelBuilder::append(const RenderVertexWithNormal& v)
    {
        static_assert(sizeof(RenderVertexWithNormal) % sizeof(float) == 0, "Invalid vertex structure");
        const float* data = reinterpret_cast<const float*>(&v);
        std::copy_n(data, sizeof(RenderVertexWithNormal) / sizeof(float), std::back_inserter(m_vbuf));
        ++m_vertexCount;
    }


    void Mesh::ModelBuilder::append(const Mesh& mesh)
    {
        if( mesh.normals.empty() && m_hasNormals )
        BOOST_THROW_EXCEPTION(std::runtime_error("Trying to append a mesh with normals to a buffer without normals"));
        else if( !mesh.normals.empty() && !m_hasNormals )
        BOOST_THROW_EXCEPTION(std::runtime_error("Trying to append a mesh without normals to a buffer with normals"));

        if( !m_hasNormals )
        {
            for( const QuadFace& quad : mesh.textured_rectangles )
            {
                const TextureLayoutProxy& proxy = m_textureProxies.at(quad.proxyId);
                const auto partId = getPartForTexture(proxy);

                const auto firstVertex = m_vertexCount;
                for( int i = 0; i < 4; ++i )
                {
                    RenderVertex iv;
                    iv.position = mesh.vertices[quad.vertices[i]].toRenderSystem();
                    iv.texcoord0.x = proxy.uvCoordinates[i].xpixel / 255.0f;
                    iv.texcoord0.y = proxy.uvCoordinates[i].ypixel / 255.0f;
                    m_parts[partId].indices.emplace_back(m_vertexCount);
                    append(iv);
                }

                m_animator.registerVertex(quad.proxyId, {m_mesh, partId}, 0, firstVertex + 0);
                m_animator.registerVertex(quad.proxyId, {m_mesh, partId}, 1, firstVertex + 1);
                m_animator.registerVertex(quad.proxyId, {m_mesh, partId}, 2, firstVertex + 2);
                m_animator.registerVertex(quad.proxyId, {m_mesh, partId}, 0, firstVertex + 0);
                m_animator.registerVertex(quad.proxyId, {m_mesh, partId}, 2, firstVertex + 2);
                m_animator.registerVertex(quad.proxyId, {m_mesh, partId}, 3, firstVertex + 3);
            }
            for( const QuadFace& quad : mesh.colored_rectangles )
            {
                const TextureLayoutProxy& proxy = m_textureProxies.at(quad.proxyId);
                auto partId = getPartForColor(quad.proxyId);

                for( int i = 0; i < 4; ++i )
                {
                    RenderVertex iv;
                    iv.position = mesh.vertices[quad.vertices[i]].toRenderSystem();
                    iv.texcoord0.x = proxy.uvCoordinates[i].xpixel / 255.0f;
                    iv.texcoord0.y = proxy.uvCoordinates[i].ypixel / 255.0f;
                    m_parts[partId].indices.emplace_back(m_vertexCount);
                    append(iv);
                }
            }
            for( const Triangle& tri : mesh.textured_triangles )
            {
                const TextureLayoutProxy& proxy = m_textureProxies.at(tri.proxyId);
                const auto partId = getPartForTexture(proxy);

                const auto firstVertex = m_vertexCount;
                for( int i = 0; i < 3; ++i )
                {
                    RenderVertex iv;
                    iv.position = mesh.vertices[tri.vertices[i]].toRenderSystem();
                    iv.texcoord0.x = proxy.uvCoordinates[i].xpixel / 255.0f;
                    iv.texcoord0.y = proxy.uvCoordinates[i].ypixel / 255.0f;
                    m_parts[partId].indices.emplace_back(m_vertexCount);
                    append(iv);
                }

                m_animator.registerVertex(tri.proxyId, {m_mesh, partId}, 0, firstVertex + 0);
                m_animator.registerVertex(tri.proxyId, {m_mesh, partId}, 1, firstVertex + 1);
                m_animator.registerVertex(tri.proxyId, {m_mesh, partId}, 2, firstVertex + 2);
            }
            for( const Triangle& tri : mesh.colored_triangles )
            {
                const TextureLayoutProxy& proxy = m_textureProxies.at(tri.proxyId);
                auto partId = getPartForColor(tri.proxyId);

                for( int i = 0; i < 3; ++i )
                {
                    RenderVertex iv;
                    iv.position = mesh.vertices[tri.vertices[i]].toRenderSystem();
                    iv.texcoord0.x = proxy.uvCoordinates[i].xpixel / 255.0f;
                    iv.texcoord0.y = proxy.uvCoordinates[i].ypixel / 255.0f;
                    m_parts[partId].indices.emplace_back(m_vertexCount);
                    append(iv);
                }
            }
        }
        else
        {
            for( const QuadFace& quad : mesh.textured_rectangles )
            {
                const TextureLayoutProxy& proxy = m_textureProxies.at(quad.proxyId);
                const auto partId = getPartForTexture(proxy);

                const auto firstVertex = m_vertexCount;
                for( int i = 0; i < 4; ++i )
                {
                    RenderVertexWithNormal iv;
                    iv.position = mesh.vertices[quad.vertices[i]].toRenderSystem();
                    iv.texcoord0.x = proxy.uvCoordinates[i].xpixel / 255.0f;
                    iv.texcoord0.y = proxy.uvCoordinates[i].ypixel / 255.0f;
                    iv.normal = mesh.normals[quad.vertices[i]].toRenderSystem();
                    m_parts[partId].indices.emplace_back(m_vertexCount);
                    append(iv);
                }

                m_animator.registerVertex(quad.proxyId, {m_mesh, partId}, 0, firstVertex + 0);
                m_animator.registerVertex(quad.proxyId, {m_mesh, partId}, 1, firstVertex + 1);
                m_animator.registerVertex(quad.proxyId, {m_mesh, partId}, 2, firstVertex + 2);
                m_animator.registerVertex(quad.proxyId, {m_mesh, partId}, 0, firstVertex + 0);
                m_animator.registerVertex(quad.proxyId, {m_mesh, partId}, 2, firstVertex + 2);
                m_animator.registerVertex(quad.proxyId, {m_mesh, partId}, 3, firstVertex + 3);
            }
            for( const QuadFace& quad : mesh.colored_rectangles )
            {
                const TextureLayoutProxy& proxy = m_textureProxies.at(quad.proxyId);
                auto partId = getPartForColor(quad.proxyId);

                for( int i = 0; i < 4; ++i )
                {
                    RenderVertexWithNormal iv;
                    iv.position = mesh.vertices[quad.vertices[i]].toRenderSystem();
                    iv.texcoord0.x = proxy.uvCoordinates[i].xpixel / 255.0f;
                    iv.texcoord0.y = proxy.uvCoordinates[i].ypixel / 255.0f;
                    iv.normal = mesh.normals[quad.vertices[i]].toRenderSystem();
                    m_parts[partId].indices.emplace_back(m_vertexCount);
                    append(iv);
                }
            }
            for( const Triangle& tri : mesh.textured_triangles )
            {
                const TextureLayoutProxy& proxy = m_textureProxies.at(tri.proxyId);
                const auto partId = getPartForTexture(proxy);

                const auto firstVertex = m_vertexCount;
                for( int i = 0; i < 3; ++i )
                {
                    RenderVertexWithNormal iv;
                    iv.position = mesh.vertices[tri.vertices[i]].toRenderSystem();
                    iv.texcoord0.x = proxy.uvCoordinates[i].xpixel / 255.0f;
                    iv.texcoord0.y = proxy.uvCoordinates[i].ypixel / 255.0f;
                    iv.normal = mesh.normals[tri.vertices[i]].toRenderSystem();
                    m_parts[partId].indices.emplace_back(m_vertexCount);
                    append(iv);
                }

                m_animator.registerVertex(tri.proxyId, {m_mesh, partId}, 0, firstVertex + 0);
                m_animator.registerVertex(tri.proxyId, {m_mesh, partId}, 1, firstVertex + 1);
                m_animator.registerVertex(tri.proxyId, {m_mesh, partId}, 2, firstVertex + 2);
            }
            for( const Triangle& tri : mesh.colored_triangles )
            {
                const TextureLayoutProxy& proxy = m_textureProxies.at(tri.proxyId);
                auto partId = getPartForColor(tri.proxyId);

                for( int i = 0; i < 3; ++i )
                {
                    RenderVertexWithNormal iv;
                    iv.position = mesh.vertices[tri.vertices[i]].toRenderSystem();
                    iv.texcoord0.x = proxy.uvCoordinates[i].xpixel / 255.0f;
                    iv.texcoord0.y = proxy.uvCoordinates[i].ypixel / 255.0f;
                    iv.normal = mesh.normals[tri.vertices[i]].toRenderSystem();
                    m_parts[partId].indices.emplace_back(m_vertexCount);
                    append(iv);
                }
            }
        }
    }


    std::shared_ptr<gameplay::Model> Mesh::ModelBuilder::finalize()
    {
        const gameplay::VertexFormat& format = m_hasNormals ? RenderVertexWithNormal::getFormat() : RenderVertex::getFormat();
        Expects(m_vbuf.size() * sizeof(float) == m_vertexCount * format.getVertexSize())

        m_mesh->setVertexData(m_vbuf.data(), 0, m_vertexCount);

        for( const MeshPart& localPart : m_parts )
        {
            static_assert(sizeof(localPart.indices[0]) == sizeof(uint16_t), "Wrong index type");
            gameplay::MeshPart* part = m_mesh->addPart(gameplay::Mesh::PrimitiveType::TRIANGLES, gameplay::Mesh::IndexFormat::INDEX16, localPart.indices.size(), true);
            part->setIndexData(localPart.indices.data(), 0, localPart.indices.size());
        }

        auto model = std::make_shared<gameplay::Model>(m_mesh);

        for( size_t i = 0; i < m_parts.size(); ++i )
        {
            model->setMaterial(m_parts[i].material, i);
        }

        return model;
    }


    std::shared_ptr<gameplay::Model> Mesh::createModel(const std::vector<TextureLayoutProxy>& textureProxies,
                                                       const std::map<TextureLayoutProxy::TextureKey, std::shared_ptr<gameplay::Material>>& materials,
                                                       const std::vector<std::shared_ptr<gameplay::Material>>& colorMaterials,
                                                       render::TextureAnimator& animator) const
    {
        ModelBuilder mb{
            !normals.empty(),
            true,
            textureProxies,
            materials,
            colorMaterials,
            animator
        };

        mb.append(*this);
        return mb.finalize();
    }
}
