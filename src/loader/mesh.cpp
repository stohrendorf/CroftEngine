#include "mesh.h"

#include "render/textureanimator.h"


namespace loader
{
#pragma pack(push,1)
    struct Mesh::ModelBuilder::RenderVertex
    {
        glm::vec2 texcoord0;
        glm::vec3 position;
        glm::ivec4 blendIndices{ 0, 0, 0, 0 };
        glm::vec4 blendWeights{ 0, 0, 0, 0 };


        static gameplay::VertexFormat getFormat(bool withWeights)
        {
            static const gameplay::VertexFormat::Element elems[4] = {
                {gameplay::VertexFormat::TEXCOORD0, 2},
                {gameplay::VertexFormat::POSITION, 3},
                {gameplay::VertexFormat::BLENDINDICES, 4},
                {gameplay::VertexFormat::BLENDWEIGHTS, 4}
            };
            return gameplay::VertexFormat{elems, size_t(withWeights ? 4 : 2)};
        }
    };


    struct Mesh::ModelBuilder::RenderVertexWithNormal
    {
        glm::vec2 texcoord0;
        glm::vec3 position;
        glm::vec3 normal;
        glm::ivec4 blendIndices{ 0, 0, 0, 0 };
        glm::vec4 blendWeights{ 0, 0, 0, 0 };

        static gameplay::VertexFormat getFormat(bool withWeights)
        {
            static const gameplay::VertexFormat::Element elems[5] = {
                {gameplay::VertexFormat::TEXCOORD0, 2},
                {gameplay::VertexFormat::POSITION, 3},
                {gameplay::VertexFormat::NORMAL, 3},
                {gameplay::VertexFormat::BLENDINDICES, 4},
                {gameplay::VertexFormat::BLENDWEIGHTS, 4}
            };
            return gameplay::VertexFormat{elems, size_t(withWeights ? 5 : 3)};
        }
    };
#pragma pack(pop)

    gameplay::VertexFormat Mesh::ModelBuilder::getFormat(bool withNormals, bool withWeights)
    {
        return withNormals ? RenderVertexWithNormal::getFormat(withWeights) : RenderVertex::getFormat(withWeights);
    }

    Mesh::ModelBuilder::ModelBuilder(bool withNormals,
                                     bool dynamic,
                                     bool withWeights,
                                     const std::vector<TextureLayoutProxy>& textureProxies,
                                     const std::map<TextureLayoutProxy::TextureKey, std::shared_ptr<gameplay::Material>>& materials,
                                     const std::vector<std::shared_ptr<gameplay::Material>>& colorMaterials,
                                     render::TextureAnimator& animator)
        : m_hasNormals{withNormals}
        , m_withWeights{withWeights}
        , m_textureProxies{textureProxies}
        , m_materials{materials}
        , m_colorMaterials{colorMaterials}
        , m_animator{animator}
        , m_mesh{gameplay::Mesh::createMesh(getFormat(withNormals, withWeights), 0, dynamic)}
    {
        Expects(colorMaterials.size() == 256);
    }


    Mesh::ModelBuilder::~ModelBuilder() = default;


    void Mesh::ModelBuilder::append(const RenderVertex& v)
    {
        static_assert(sizeof(RenderVertex) % sizeof(float) == 0, "Invalid vertex structure");
        Expects(!m_hasNormals);
        Expects(sizeof(v) >= m_mesh->getVertexSize());
        Expects(m_mesh->getVertexSize() % sizeof(float) == 0);

        const float* data = reinterpret_cast<const float*>(&v);
        const auto n = m_mesh->getVertexSize() / sizeof(float);
        std::copy_n(data, n, std::back_inserter(m_vbuf));
        ++m_vertexCount;
    }


    void Mesh::ModelBuilder::append(const RenderVertexWithNormal& v)
    {
        static_assert(sizeof(RenderVertexWithNormal) % sizeof(float) == 0, "Invalid vertex structure");
        Expects(m_hasNormals);
        Expects(sizeof(v) >= m_mesh->getVertexSize());
        Expects(m_mesh->getVertexSize() % sizeof(float) == 0);

        const float* data = reinterpret_cast<const float*>(&v);
        const auto n = m_mesh->getVertexSize() / sizeof(float);
        std::copy_n(data, n, std::back_inserter(m_vbuf));
        ++m_vertexCount;
    }


    void Mesh::ModelBuilder::append(const Mesh& mesh, float blendWeight, float blendIndex)
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
                    iv.blendWeights = { blendWeight, 0, 0, 0 };
                    iv.blendIndices = { blendIndex, 0, 0, 0 };
                    append(iv);
                }

                m_animator.registerVertex(quad.proxyId, {m_mesh, partId}, 0, firstVertex + 0);
                m_parts[partId].indices.emplace_back(firstVertex + 0);
                m_animator.registerVertex(quad.proxyId, {m_mesh, partId}, 1, firstVertex + 1);
                m_parts[partId].indices.emplace_back(firstVertex + 1);
                m_animator.registerVertex(quad.proxyId, {m_mesh, partId}, 2, firstVertex + 2);
                m_parts[partId].indices.emplace_back(firstVertex + 2);
                m_animator.registerVertex(quad.proxyId, {m_mesh, partId}, 0, firstVertex + 0);
                m_parts[partId].indices.emplace_back(firstVertex + 0);
                m_animator.registerVertex(quad.proxyId, {m_mesh, partId}, 2, firstVertex + 2);
                m_parts[partId].indices.emplace_back(firstVertex + 2);
                m_animator.registerVertex(quad.proxyId, {m_mesh, partId}, 3, firstVertex + 3);
                m_parts[partId].indices.emplace_back(firstVertex + 3);
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
                    iv.blendWeights = { blendWeight, 0, 0, 0 };
                    iv.blendIndices = { blendIndex, 0, 0, 0 };
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
                    iv.blendWeights = { blendWeight, 0, 0, 0 };
                    iv.blendIndices = { blendIndex, 0, 0, 0 };
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
                    iv.blendWeights = { blendWeight, 0, 0, 0 };
                    iv.blendIndices = { blendIndex, 0, 0, 0 };
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
                    iv.blendWeights = { blendWeight, 0, 0, 0 };
                    iv.blendIndices = { blendIndex, 0, 0, 0 };
                    append(iv);
                }

                m_animator.registerVertex(quad.proxyId, {m_mesh, partId}, 0, firstVertex + 0);
                m_parts[partId].indices.emplace_back(firstVertex + 0);
                m_animator.registerVertex(quad.proxyId, {m_mesh, partId}, 1, firstVertex + 1);
                m_parts[partId].indices.emplace_back(firstVertex + 1);
                m_animator.registerVertex(quad.proxyId, {m_mesh, partId}, 2, firstVertex + 2);
                m_parts[partId].indices.emplace_back(firstVertex + 2);
                m_animator.registerVertex(quad.proxyId, {m_mesh, partId}, 0, firstVertex + 0);
                m_parts[partId].indices.emplace_back(firstVertex + 0);
                m_animator.registerVertex(quad.proxyId, {m_mesh, partId}, 2, firstVertex + 2);
                m_parts[partId].indices.emplace_back(firstVertex + 2);
                m_animator.registerVertex(quad.proxyId, {m_mesh, partId}, 3, firstVertex + 3);
                m_parts[partId].indices.emplace_back(firstVertex + 3);
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
                    iv.blendWeights = { blendWeight, 0, 0, 0 };
                    iv.blendIndices = { blendIndex, 0, 0, 0 };
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
                    iv.blendWeights = { blendWeight, 0, 0, 0 };
                    iv.blendIndices = { blendIndex, 0, 0, 0 };
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
                    iv.blendWeights = { blendWeight, 0, 0, 0 };
                    iv.blendIndices = { blendIndex, 0, 0, 0 };
                    m_parts[partId].indices.emplace_back(m_vertexCount);
                    append(iv);
                }
            }
        }
    }


    std::shared_ptr<gameplay::Model> Mesh::ModelBuilder::finalize()
    {
        Expects(m_vbuf.size() * sizeof(float) == m_vertexCount * m_mesh->getVertexSize())

        m_mesh->rebuild(m_vbuf.data(), m_vertexCount);

        for( const MeshPart& localPart : m_parts )
        {
            static_assert(sizeof(localPart.indices[0]) == sizeof(uint16_t), "Wrong index type");
#ifndef NDEBUG
            for(auto idx : localPart.indices)
                Expects(idx >= 0 && idx < m_vertexCount);
#endif
            gameplay::MeshPart* part = m_mesh->addPart(gameplay::Mesh::PrimitiveType::TRIANGLES, gameplay::Mesh::IndexFormat::INDEX16, localPart.indices.size(), true);
            part->setIndexData(localPart.indices.data(), 0, 0);
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
            false,
            textureProxies,
            materials,
            colorMaterials,
            animator
        };

        mb.append(*this);
        return mb.finalize();
    }
}
