#include "mesh.h"

#include "color.h"
#include "render/gl/vertexarray.h"
#include "render/scene/MaterialParameter.h"
#include "render/scene/mesh.h"
#include "render/scene/model.h"
#include "render/scene/names.h"
#include "render/textureanimator.h"
#include "util.h"

#include <utility>

namespace loader
{
namespace file
{
Mesh::ModelBuilder::ModelBuilder(
    const bool withNormals,
    const std::vector<TextureTile>& textureTiles,
    const std::map<TextureKey, gsl::not_null<std::shared_ptr<render::scene::Material>>>& materials,
    gsl::not_null<std::shared_ptr<render::scene::Material>> colorMaterial,
    const Palette& palette,
    std::string label)
    : m_hasNormals{withNormals}
    , m_textureTiles{textureTiles}
    , m_materials{materials}
    , m_colorMaterial{std::move(colorMaterial)}
    , m_palette{palette}
    , m_vb{std::make_shared<render::gl::StructuredArrayBuffer<RenderVertex>>(RenderVertex::getFormat(), label)}
    , m_label{std::move(label)}
{
}

Mesh::ModelBuilder::~ModelBuilder() = default;

void Mesh::ModelBuilder::append(const RenderVertex& v)
{
    m_vertices.emplace_back(v);
}

void Mesh::ModelBuilder::append(const Mesh& mesh)
{
    if(mesh.normals.empty() && m_hasNormals)
        BOOST_THROW_EXCEPTION(std::runtime_error("Trying to append a mesh with normals to a buffer without normals"));
    else if(!mesh.normals.empty() && !m_hasNormals)
        BOOST_THROW_EXCEPTION(std::runtime_error("Trying to append a mesh without normals to a buffer with normals"));

    for(const QuadFace& quad : mesh.textured_rectangles)
    {
        const TextureTile& tile = m_textureTiles.at(quad.tileId.get());
        const auto partId = getPartForTexture(tile);

        glm::vec3 defaultNormal{0.0f};
        if(m_hasNormals)
        {
            for(auto v : quad.vertices)
            {
                const auto n = v.from(mesh.normals).toRenderSystem();
                if(n != glm::vec3{0.0f})
                {
                    defaultNormal = n;
                    break;
                }
            }
        }

        const auto firstVertex = m_vertices.size();
        for(int i = 0; i < 4; ++i)
        {
            RenderVertex iv{};

            if(!m_hasNormals)
            {
                iv.color = glm::vec3(1 - quad.vertices[i].from(mesh.vertexDarknesses) / 8191.0f);
                if(i <= 2)
                {
                    static const int indices[3] = {0, 1, 2};
                    iv.normal = generateNormal(quad.vertices[indices[(i + 0) % 3]].from(mesh.vertices),
                                               quad.vertices[indices[(i + 1) % 3]].from(mesh.vertices),
                                               quad.vertices[indices[(i + 2) % 3]].from(mesh.vertices));
                }
                else
                {
                    static const int indices[3] = {0, 2, 3};
                    iv.normal = generateNormal(quad.vertices[indices[(i + 0) % 3]].from(mesh.vertices),
                                               quad.vertices[indices[(i + 1) % 3]].from(mesh.vertices),
                                               quad.vertices[indices[(i + 2) % 3]].from(mesh.vertices));
                }
            }
            else
            {
                iv.normal = quad.vertices[i].from(mesh.normals).toRenderSystem();
                if(iv.normal == glm::vec3{0.0f})
                    iv.normal = defaultNormal;
            }

            iv.position = quad.vertices[i].from(mesh.vertices).toRenderSystem();
            iv.uv = tile.uvCoordinates[i].toGl();
            append(iv);
        }

        for(auto i : {0, 1, 2, 0, 2, 3})
        {
            m_parts[partId].indices.emplace_back(gsl::narrow<MeshPart::IndexBuffer::value_type>(firstVertex + i));
        }
    }
    for(const QuadFace& quad : mesh.colored_rectangles)
    {
        const TextureTile& tile = m_textureTiles.at(quad.tileId.get());
        const auto partId = getPartForColor(quad.tileId);
        const auto color = *m_parts[partId].color;

        glm::vec3 defaultNormal{0.0f};
        if(m_hasNormals)
        {
            for(auto v : quad.vertices)
            {
                const auto n = v.from(mesh.normals).toRenderSystem();
                if(n != glm::vec3{0.0f})
                {
                    defaultNormal = n;
                    break;
                }
            }
        }

        const auto firstVertex = m_vertices.size();
        for(int i = 0; i < 4; ++i)
        {
            RenderVertex iv{};
            iv.position = quad.vertices[i].from(mesh.vertices).toRenderSystem();
            iv.color = color;
            if(!m_hasNormals)
            {
                iv.color *= 1 - quad.vertices[i].from(mesh.vertexDarknesses) / 8191.0f;
                if(i <= 2)
                {
                    static const int indices[3] = {0, 1, 2};
                    iv.normal = generateNormal(quad.vertices[indices[(i + 0) % 3]].from(mesh.vertices),
                                               quad.vertices[indices[(i + 1) % 3]].from(mesh.vertices),
                                               quad.vertices[indices[(i + 2) % 3]].from(mesh.vertices));
                }
                else
                {
                    static const int indices[3] = {0, 2, 3};
                    iv.normal = generateNormal(quad.vertices[indices[(i + 0) % 3]].from(mesh.vertices),
                                               quad.vertices[indices[(i + 1) % 3]].from(mesh.vertices),
                                               quad.vertices[indices[(i + 2) % 3]].from(mesh.vertices));
                }
            }
            else
            {
                iv.normal = quad.vertices[i].from(mesh.normals).toRenderSystem();
                if(iv.normal == glm::vec3{0.0f})
                    iv.normal = defaultNormal;
            }
            iv.uv = tile.uvCoordinates[i].toGl();
            append(iv);
        }
        for(auto i : {0, 1, 2, 0, 2, 3})
        {
            m_parts[partId].indices.emplace_back(gsl::narrow<MeshPart::IndexBuffer::value_type>(firstVertex + i));
        }
    }
    for(const Triangle& tri : mesh.textured_triangles)
    {
        const TextureTile& tile = m_textureTiles.at(tri.tileId.get());
        const auto partId = getPartForTexture(tile);

        glm::vec3 defaultNormal{0.0f};
        if(m_hasNormals)
        {
            for(auto v : tri.vertices)
            {
                const auto n = v.from(mesh.normals).toRenderSystem();
                if(n != glm::vec3{0.0f})
                {
                    defaultNormal = n;
                    break;
                }
            }
        }

        for(int i = 0; i < 3; ++i)
        {
            RenderVertex iv{};
            iv.position = tri.vertices[i].from(mesh.vertices).toRenderSystem();
            iv.uv = tile.uvCoordinates[i].toGl();
            if(!m_hasNormals)
            {
                iv.color = glm::vec3(1 - tri.vertices[i].from(mesh.vertexDarknesses) / 8191.0f);

                static const int indices[3] = {0, 1, 2};
                iv.normal = generateNormal(tri.vertices[indices[(i + 0) % 3]].from(mesh.vertices),
                                           tri.vertices[indices[(i + 1) % 3]].from(mesh.vertices),
                                           tri.vertices[indices[(i + 2) % 3]].from(mesh.vertices));
            }
            else
            {
                iv.normal = tri.vertices[i].from(mesh.normals).toRenderSystem();
                if(iv.normal == glm::vec3{0.0f})
                    iv.normal = defaultNormal;
            }
            m_parts[partId].indices.emplace_back(gsl::narrow<MeshPart::IndexBuffer::value_type>(m_vertices.size()));
            append(iv);
        }
    }
    for(const Triangle& tri : mesh.colored_triangles)
    {
        const TextureTile& tile = m_textureTiles.at(tri.tileId.get());
        const auto partId = getPartForColor(tri.tileId);
        const auto color = *m_parts[partId].color;

        glm::vec3 defaultNormal{0.0f};
        if(m_hasNormals)
        {
            for(auto v : tri.vertices)
            {
                const auto n = v.from(mesh.normals).toRenderSystem();
                if(n != glm::vec3{0.0f})
                {
                    defaultNormal = n;
                    break;
                }
            }
        }

        for(int i = 0; i < 3; ++i)
        {
            RenderVertex iv{};
            iv.position = tri.vertices[i].from(mesh.vertices).toRenderSystem();
            iv.color = color;
            if(!m_hasNormals)
            {
                iv.color *= glm::vec3(1 - tri.vertices[i].from(mesh.vertexDarknesses) / 8191.0f);

                static const int indices[3] = {0, 1, 2};
                iv.normal = generateNormal(tri.vertices[indices[(i + 0) % 3]].from(mesh.vertices),
                                           tri.vertices[indices[(i + 1) % 3]].from(mesh.vertices),
                                           tri.vertices[indices[(i + 2) % 3]].from(mesh.vertices));
            }
            else
            {
                iv.normal = tri.vertices[i].from(mesh.normals).toRenderSystem();
                if(iv.normal == glm::vec3{0.0f})
                    iv.normal = defaultNormal;
            }
            iv.uv = tile.uvCoordinates[i].toGl();
            m_parts[partId].indices.emplace_back(gsl::narrow<MeshPart::IndexBuffer::value_type>(m_vertices.size()));
            append(iv);
        }
    }
}

gsl::not_null<std::shared_ptr<render::scene::Model>> Mesh::ModelBuilder::finalize()
{
    m_vb->setData(m_vertices, ::gl::BufferUsageARB::StaticDraw);

    auto model = std::make_shared<render::scene::Model>();
    for(const MeshPart& localPart : m_parts)
    {
#ifndef NDEBUG
        for(auto idx : localPart.indices)
        {
            BOOST_ASSERT(idx < m_vertices.size());
        }
#endif

        auto indexBuffer = std::make_shared<render::gl::ElementArrayBuffer<uint16_t>>();
        indexBuffer->setData(localPart.indices, ::gl::BufferUsageARB::DynamicDraw);

        auto va = std::make_shared<render::gl::VertexArray<uint16_t, RenderVertex>>(
            indexBuffer, m_vb, localPart.material->getShaderProgram()->getHandle(), m_label);
        auto mesh
            = std::make_shared<render::scene::MeshImpl<uint16_t, RenderVertex>>(va, ::gl::PrimitiveType::Triangles);
        mesh->setMaterial(localPart.material);

        model->addMesh(mesh);
    }

    return model;
}

std::shared_ptr<render::scene::Model>
    Mesh::createModel(const std::vector<TextureTile>& textureTiles,
                      const std::map<TextureKey, gsl::not_null<std::shared_ptr<render::scene::Material>>>& materials,
                      const gsl::not_null<std::shared_ptr<render::scene::Material>>& colorMaterial,
                      const Palette& palette,
                      const std::string& label) const
{
    ModelBuilder mb{!normals.empty(), textureTiles, materials, colorMaterial, palette, label};

    mb.append(*this);

    return mb.finalize();
}
} // namespace file
} // namespace loader
