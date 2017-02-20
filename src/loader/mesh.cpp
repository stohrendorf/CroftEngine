#include "mesh.h"

#include "render/textureanimator.h"
#include "color.h"


namespace loader
{
#pragma pack(push,1)
    struct Mesh::ModelBuilder::RenderVertex
    {
        glm::vec2 texcoord;
        glm::vec3 position;
        glm::vec4 color;


        static gameplay::VertexFormat getFormat()
        {
            static const gameplay::VertexFormat::Element elems[4] = {
                {gameplay::VertexFormat::TEXCOORD, 2},
                {gameplay::VertexFormat::POSITION, 3},
                {gameplay::VertexFormat::COLOR, 4}
            };
            return gameplay::VertexFormat{elems, 3};
        }
    };


    struct Mesh::ModelBuilder::RenderVertexWithNormal
    {
        glm::vec2 texcoord;
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec4 color;


        static gameplay::VertexFormat getFormat()
        {
            static const gameplay::VertexFormat::Element elems[5] = {
                {gameplay::VertexFormat::TEXCOORD, 2},
                {gameplay::VertexFormat::POSITION, 3},
                {gameplay::VertexFormat::NORMAL, 3},
                {gameplay::VertexFormat::COLOR, 4}
            };
            return gameplay::VertexFormat{elems, 4};
        }
    };
#pragma pack(pop)

    gameplay::VertexFormat Mesh::ModelBuilder::getFormat(bool withNormals)
    {
        return withNormals ? RenderVertexWithNormal::getFormat() : RenderVertex::getFormat();
    }


    Mesh::ModelBuilder::ModelBuilder(bool withNormals,
                                     bool dynamic,
                                     const std::vector<TextureLayoutProxy>& textureProxies,
                                     const std::map<TextureLayoutProxy::TextureKey, std::shared_ptr<gameplay::Material>>& materials,
                                     const std::shared_ptr<gameplay::Material>& colorMaterial,
                                     const Palette& palette,
                                     render::TextureAnimator& animator)
        : m_hasNormals{withNormals}
        , m_textureProxies{textureProxies}
        , m_materials{materials}
        , m_colorMaterial{colorMaterial}
        , m_palette{palette}
        , m_animator{animator}
        , m_mesh{std::make_shared<gameplay::Mesh>(getFormat(withNormals), 0, dynamic)}
    {
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
                    iv.texcoord = proxy.uvCoordinates[i].toGl();
                    if(quad.vertices[i] < mesh.vertexDarknesses.size())
                        iv.color = glm::vec4(1 - mesh.vertexDarknesses[quad.vertices[i]] / 8192.0f);
                    else
                        iv.color = glm::vec4(1.0f);
                    append(iv);
                }

                for(auto j : { 0,1,2,0,2,3 })
                {
                    m_animator.registerVertex(quad.proxyId, m_mesh, j, firstVertex + j);
                    m_parts[partId].indices.emplace_back(firstVertex + j);
                }
            }
            for( const QuadFace& quad : mesh.colored_rectangles )
            {
                const TextureLayoutProxy& proxy = m_textureProxies.at(quad.proxyId);
                auto partId = getPartForColor(quad.proxyId);

                const auto firstVertex = m_vertexCount;
                for( int i = 0; i < 4; ++i )
                {
                    RenderVertex iv;
                    iv.position = mesh.vertices[quad.vertices[i]].toRenderSystem();
                    iv.texcoord = proxy.uvCoordinates[i].toGl();
                    if(quad.vertices[i] < mesh.vertexDarknesses.size())
                        iv.color = glm::vec4(1 - mesh.vertexDarknesses[quad.vertices[i]] / 8192.0f);
                    else
                        iv.color = glm::vec4(1.0f);
                    append(iv);
                }

                for(auto j : { 0,1,2,0,2,3 })
                {
                    m_parts[partId].indices.emplace_back(firstVertex + j);
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
                    iv.texcoord = proxy.uvCoordinates[i].toGl();
                    if(tri.vertices[i] < mesh.vertexDarknesses.size())
                        iv.color = glm::vec4(1 - mesh.vertexDarknesses[tri.vertices[i]] / 8192.0f);
                    else
                        iv.color = glm::vec4(1.0f);
                    m_parts[partId].indices.emplace_back(m_vertexCount);
                    append(iv);
                }

                m_animator.registerVertex(tri.proxyId, m_mesh, 0, firstVertex + 0);
                m_animator.registerVertex(tri.proxyId, m_mesh, 1, firstVertex + 1);
                m_animator.registerVertex(tri.proxyId, m_mesh, 2, firstVertex + 2);
            }
            for( const Triangle& tri : mesh.colored_triangles )
            {
                const TextureLayoutProxy& proxy = m_textureProxies.at(tri.proxyId);
                auto partId = getPartForColor(tri.proxyId);

                for( int i = 0; i < 3; ++i )
                {
                    RenderVertex iv;
                    iv.position = mesh.vertices[tri.vertices[i]].toRenderSystem();
                    iv.texcoord = proxy.uvCoordinates[i].toGl();
                    if(tri.vertices[i] < mesh.vertexDarknesses.size())
                        iv.color = glm::vec4(1 - mesh.vertexDarknesses[tri.vertices[i]] / 8192.0f);
                    else
                        iv.color = glm::vec4(1.0f);
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
                    iv.texcoord = proxy.uvCoordinates[i].toGl();
                    iv.normal = mesh.normals[quad.vertices[i]].toRenderSystem();
                    iv.color = glm::vec4(1.0f);
                    append(iv);
                }

                for(auto j : { 0,1,2,0,2,3 })
                {
                    m_animator.registerVertex(quad.proxyId, m_mesh, j, firstVertex + j);
                    m_parts[partId].indices.emplace_back(firstVertex + j);
                }
            }
            for( const QuadFace& quad : mesh.colored_rectangles )
            {
                const TextureLayoutProxy& proxy = m_textureProxies.at(quad.proxyId);
                auto partId = getPartForColor(quad.proxyId);

                const auto firstVertex = m_vertexCount;
                for( int i = 0; i < 4; ++i )
                {
                    RenderVertexWithNormal iv;
                    iv.position = mesh.vertices[quad.vertices[i]].toRenderSystem();
                    iv.texcoord = proxy.uvCoordinates[i].toGl();
                    iv.normal = mesh.normals[quad.vertices[i]].toRenderSystem();
                    iv.color = glm::vec4(1.0f);
                    append(iv);
                }
                for(auto j : { 0,1,2,0,2,3 })
                {
                    m_parts[partId].indices.emplace_back(firstVertex + j);
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
                    iv.texcoord = proxy.uvCoordinates[i].toGl();
                    iv.normal = mesh.normals[tri.vertices[i]].toRenderSystem();
                    iv.color = glm::vec4(1.0f);
                    m_parts[partId].indices.emplace_back(m_vertexCount);
                    append(iv);
                }

                m_animator.registerVertex(tri.proxyId, m_mesh, 0, firstVertex + 0);
                m_animator.registerVertex(tri.proxyId, m_mesh, 1, firstVertex + 1);
                m_animator.registerVertex(tri.proxyId, m_mesh, 2, firstVertex + 2);
            }
            for( const Triangle& tri : mesh.colored_triangles )
            {
                const TextureLayoutProxy& proxy = m_textureProxies.at(tri.proxyId);
                auto partId = getPartForColor(tri.proxyId);

                for( int i = 0; i < 3; ++i )
                {
                    RenderVertexWithNormal iv;
                    iv.position = mesh.vertices[tri.vertices[i]].toRenderSystem();
                    iv.texcoord = proxy.uvCoordinates[i].toGl();
                    iv.normal = mesh.normals[tri.vertices[i]].toRenderSystem();
                    iv.color = glm::vec4(1.0f);
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
            for( auto idx : localPart.indices )
            {
                BOOST_ASSERT(idx < m_vertexCount);
            }
#endif
            auto part = m_mesh->addPart(gameplay::PrimitiveType::TRIANGLES, gameplay::IndexFormat::INDEX16, localPart.indices.size(), true);
            part->setIndexData(localPart.indices.data(), 0, 0);
            part->setMaterial(localPart.material);
            if( localPart.color )
            {
                auto color = *localPart.color;
                part->registerMaterialParameterSetter([color](gameplay::Material& material)
                    {
                        material.getParameter("u_diffuseColor")->set(color);
                    });
            }
        }

        auto model = std::make_shared<gameplay::Model>();
        model->addMesh(m_mesh);
        return model;
    }


    std::shared_ptr<gameplay::Model> Mesh::createModel(const std::vector<TextureLayoutProxy>& textureProxies,
                                                       const std::map<TextureLayoutProxy::TextureKey, std::shared_ptr<gameplay::Material>>& materials,
                                                       const std::shared_ptr<gameplay::Material>& colorMaterial,
                                                       const Palette& palette,
                                                       render::TextureAnimator& animator) const
    {
        ModelBuilder mb{
            !normals.empty(),
            true,
            textureProxies,
            materials,
            colorMaterial,
            palette,
            animator
        };

        mb.append(*this);


        return mb.finalize();
    }
}
