#include "mesh.h"

#include "render/textureanimator.h"


namespace loader
{
    namespace
    {
#pragma pack(push,1)
        struct RenderVertex
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


        struct RenderVertexWithNormal
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

        struct RenderModel
        {
            struct MeshPart
            {
                using IndexBuffer = std::vector<uint16_t>;

                IndexBuffer indices;
                std::shared_ptr<gameplay::Material> material;
            };


            std::vector<MeshPart> m_parts;


            std::shared_ptr<gameplay::Model> toModel(const gsl::not_null<std::shared_ptr<gameplay::Mesh>>& mesh)
            {
                for( const MeshPart& localPart : m_parts )
                {
                    gameplay::MeshPart* part = mesh->addPart(gameplay::Mesh::PrimitiveType::TRIANGLES, gameplay::Mesh::IndexFormat::INDEX16, localPart.indices.size(), true);
                    part->setIndexData(localPart.indices.data(), 0, localPart.indices.size());
                }

                auto model = std::make_shared<gameplay::Model>(mesh);

                for( size_t i = 0; i < m_parts.size(); ++i )
                {
                    model->setMaterial(m_parts[i].material, i);
                }

                return model;
            }
        };
    }


    std::shared_ptr<gameplay::Model> Mesh::createModel(const std::vector<TextureLayoutProxy>& textureProxies,
                                                       const std::map<TextureLayoutProxy::TextureKey, std::shared_ptr<gameplay::Material>>& materials,
                                                       const std::vector<std::shared_ptr<gameplay::Material>>& colorMaterials,
                                                       render::TextureAnimator& animator) const
    {
        BOOST_ASSERT(colorMaterials.size() == 256);

        std::shared_ptr<gameplay::Mesh> mesh = nullptr;
        RenderModel renderModel;
        std::map<TextureLayoutProxy::TextureKey, size_t> texBuffers;

        if( normals.empty() )
        {
            std::vector<RenderVertex> vbuf;
            mesh = gameplay::Mesh::createMesh(RenderVertex::getFormat(), vbuf.size(), true);
            for( const QuadFace& quad : textured_rectangles )
            {
                const TextureLayoutProxy& proxy = textureProxies.at(quad.proxyId);

                if( texBuffers.find(proxy.textureKey) == texBuffers.end() )
                {
                    texBuffers[proxy.textureKey] = renderModel.m_parts.size();
                    renderModel.m_parts.emplace_back();
                    auto it = materials.find(proxy.textureKey);
                    Expects(it != materials.end());
                    renderModel.m_parts.back().material = it->second;
                }
                const auto partId = texBuffers[proxy.textureKey];

                const auto firstVertex = vbuf.size();
                for( int i = 0; i < 4; ++i )
                {
                    RenderVertex iv;
                    iv.position = vertices[quad.vertices[i]].toRenderSystem();
                    iv.texcoord0.x = proxy.uvCoordinates[i].xpixel / 255.0f;
                    iv.texcoord0.y = proxy.uvCoordinates[i].ypixel / 255.0f;
                    renderModel.m_parts[partId].indices.emplace_back(vbuf.size());
                    vbuf.push_back(iv);
                }

                animator.registerVertex(quad.proxyId, {mesh, partId}, 0, firstVertex + 0);
                animator.registerVertex(quad.proxyId, {mesh, partId}, 1, firstVertex + 1);
                animator.registerVertex(quad.proxyId, {mesh, partId}, 2, firstVertex + 2);
                animator.registerVertex(quad.proxyId, {mesh, partId}, 0, firstVertex + 0);
                animator.registerVertex(quad.proxyId, {mesh, partId}, 2, firstVertex + 2);
                animator.registerVertex(quad.proxyId, {mesh, partId}, 3, firstVertex + 3);
            }
            for( const QuadFace& quad : colored_rectangles )
            {
                const TextureLayoutProxy& proxy = textureProxies.at(quad.proxyId);

                TextureLayoutProxy::TextureKey tk;
                tk.blendingMode = BlendingMode::Solid;
                tk.flags = 0;
                tk.tileAndFlag = 0;
                tk.colorId = quad.proxyId & 0xff;

                if( texBuffers.find(tk) == texBuffers.end() )
                {
                    texBuffers[tk] = renderModel.m_parts.size();
                    renderModel.m_parts.emplace_back();
                    auto it = materials.find(tk);
                    Expects(it != materials.end());
                    renderModel.m_parts.back().material = it->second;
                }
                auto partId = texBuffers[tk];

                for( int i = 0; i < 4; ++i )
                {
                    RenderVertex iv;
                    iv.position = vertices[quad.vertices[i]].toRenderSystem();
                    iv.texcoord0.x = proxy.uvCoordinates[i].xpixel / 255.0f;
                    iv.texcoord0.y = proxy.uvCoordinates[i].ypixel / 255.0f;
                    renderModel.m_parts[partId].indices.emplace_back(vbuf.size());
                    vbuf.push_back(iv);
                }
            }
            for( const Triangle& tri : textured_triangles )
            {
                const TextureLayoutProxy& proxy = textureProxies.at(tri.proxyId);

                if( texBuffers.find(proxy.textureKey) == texBuffers.end() )
                {
                    texBuffers[proxy.textureKey] = renderModel.m_parts.size();
                    renderModel.m_parts.emplace_back();
                    auto it = materials.find(proxy.textureKey);
                    Expects(it != materials.end());
                    renderModel.m_parts.back().material = it->second;
                }
                const auto partId = texBuffers[proxy.textureKey];

                const auto firstVertex = vbuf.size();
                for( int i = 0; i < 3; ++i )
                {
                    RenderVertex iv;
                    iv.position = vertices[tri.vertices[i]].toRenderSystem();
                    iv.texcoord0.x = proxy.uvCoordinates[i].xpixel / 255.0f;
                    iv.texcoord0.y = proxy.uvCoordinates[i].ypixel / 255.0f;
                    renderModel.m_parts[partId].indices.emplace_back(vbuf.size());
                    vbuf.push_back(iv);
                }

                animator.registerVertex(tri.proxyId, {mesh, partId}, 0, firstVertex + 0);
                animator.registerVertex(tri.proxyId, {mesh, partId}, 1, firstVertex + 1);
                animator.registerVertex(tri.proxyId, {mesh, partId}, 2, firstVertex + 2);
            }
            for( const Triangle& tri : colored_triangles )
            {
                const TextureLayoutProxy& proxy = textureProxies.at(tri.proxyId);

                TextureLayoutProxy::TextureKey tk;
                tk.blendingMode = BlendingMode::Solid;
                tk.flags = 0;
                tk.tileAndFlag = 0;
                tk.colorId = tri.proxyId & 0xff;

                if( texBuffers.find(tk) == texBuffers.end() )
                {
                    texBuffers[tk] = renderModel.m_parts.size();
                    renderModel.m_parts.emplace_back();
                    auto it = materials.find(tk);
                    Expects(it != materials.end());
                    renderModel.m_parts.back().material = it->second;
                }
                auto partId = texBuffers[tk];

                for( int i = 0; i < 3; ++i )
                {
                    RenderVertex iv;
                    iv.position = vertices[tri.vertices[i]].toRenderSystem();
                    iv.texcoord0.x = proxy.uvCoordinates[i].xpixel / 255.0f;
                    iv.texcoord0.y = proxy.uvCoordinates[i].ypixel / 255.0f;
                    renderModel.m_parts[partId].indices.emplace_back(vbuf.size());
                    vbuf.push_back(iv);
                }
            }
            mesh->setVertexData(reinterpret_cast<float*>(vbuf.data()), 0, vbuf.size());
        }
        else
        {
            std::vector<RenderVertexWithNormal> vbuf;
            mesh = gameplay::Mesh::createMesh(RenderVertex::getFormat(), vbuf.size(), true);
            for( const QuadFace& quad : textured_rectangles )
            {
                const TextureLayoutProxy& proxy = textureProxies.at(quad.proxyId);

                if( texBuffers.find(proxy.textureKey) == texBuffers.end() )
                {
                    texBuffers[proxy.textureKey] = renderModel.m_parts.size();
                    renderModel.m_parts.emplace_back();
                    auto it = materials.find(proxy.textureKey);
                    Expects(it != materials.end());
                    renderModel.m_parts.back().material = it->second;
                }
                const auto partId = texBuffers[proxy.textureKey];

                const auto firstVertex = vbuf.size();
                for( int i = 0; i < 4; ++i )
                {
                    RenderVertexWithNormal iv;
                    iv.position = vertices[quad.vertices[i]].toRenderSystem();
                    iv.texcoord0.x = proxy.uvCoordinates[i].xpixel / 255.0f;
                    iv.texcoord0.y = proxy.uvCoordinates[i].ypixel / 255.0f;
                    iv.normal = normals[quad.vertices[i]].toRenderSystem();
                    renderModel.m_parts[partId].indices.emplace_back(vbuf.size());
                    vbuf.push_back(iv);
                }

                animator.registerVertex(quad.proxyId, {mesh, partId}, 0, firstVertex + 0);
                animator.registerVertex(quad.proxyId, {mesh, partId}, 1, firstVertex + 1);
                animator.registerVertex(quad.proxyId, {mesh, partId}, 2, firstVertex + 2);
                animator.registerVertex(quad.proxyId, {mesh, partId}, 0, firstVertex + 0);
                animator.registerVertex(quad.proxyId, {mesh, partId}, 2, firstVertex + 2);
                animator.registerVertex(quad.proxyId, {mesh, partId}, 3, firstVertex + 3);
            }
            for( const QuadFace& quad : colored_rectangles )
            {
                const TextureLayoutProxy& proxy = textureProxies.at(quad.proxyId);

                TextureLayoutProxy::TextureKey tk;
                tk.blendingMode = BlendingMode::Solid;
                tk.flags = 0;
                tk.tileAndFlag = 0;
                tk.colorId = quad.proxyId & 0xff;

                if( texBuffers.find(tk) == texBuffers.end() )
                {
                    texBuffers[tk] = renderModel.m_parts.size();
                    renderModel.m_parts.emplace_back();
                    auto it = materials.find(tk);
                    Expects(it != materials.end());
                    renderModel.m_parts.back().material = it->second;
                }
                auto partId = texBuffers[tk];

                for( int i = 0; i < 4; ++i )
                {
                    RenderVertexWithNormal iv;
                    iv.position = vertices[quad.vertices[i]].toRenderSystem();
                    iv.texcoord0.x = proxy.uvCoordinates[i].xpixel / 255.0f;
                    iv.texcoord0.y = proxy.uvCoordinates[i].ypixel / 255.0f;
                    iv.normal = normals[quad.vertices[i]].toRenderSystem();
                    renderModel.m_parts[partId].indices.emplace_back(vbuf.size());
                    vbuf.push_back(iv);
                }
            }
            for( const Triangle& tri : textured_triangles )
            {
                const TextureLayoutProxy& proxy = textureProxies.at(tri.proxyId);

                if( texBuffers.find(proxy.textureKey) == texBuffers.end() )
                {
                    texBuffers[proxy.textureKey] = renderModel.m_parts.size();
                    renderModel.m_parts.emplace_back();
                    auto it = materials.find(proxy.textureKey);
                    Expects(it != materials.end());
                    renderModel.m_parts.back().material = it->second;
                }
                const auto partId = texBuffers[proxy.textureKey];

                const auto firstVertex = vbuf.size();
                for( int i = 0; i < 3; ++i )
                {
                    RenderVertexWithNormal iv;
                    iv.position = vertices[tri.vertices[i]].toRenderSystem();
                    iv.texcoord0.x = proxy.uvCoordinates[i].xpixel / 255.0f;
                    iv.texcoord0.y = proxy.uvCoordinates[i].ypixel / 255.0f;
                    iv.normal = normals[tri.vertices[i]].toRenderSystem();
                    renderModel.m_parts[partId].indices.emplace_back(vbuf.size());
                    vbuf.push_back(iv);
                }

                animator.registerVertex(tri.proxyId, {mesh, partId}, 0, firstVertex + 0);
                animator.registerVertex(tri.proxyId, {mesh, partId}, 1, firstVertex + 1);
                animator.registerVertex(tri.proxyId, {mesh, partId}, 2, firstVertex + 2);
            }
            for( const Triangle& tri : colored_triangles )
            {
                const TextureLayoutProxy& proxy = textureProxies.at(tri.proxyId);

                TextureLayoutProxy::TextureKey tk;
                tk.blendingMode = BlendingMode::Solid;
                tk.flags = 0;
                tk.tileAndFlag = 0;
                tk.colorId = tri.proxyId & 0xff;

                if( texBuffers.find(tk) == texBuffers.end() )
                {
                    texBuffers[tk] = renderModel.m_parts.size();
                    renderModel.m_parts.emplace_back();
                    auto it = materials.find(tk);
                    Expects(it != materials.end());
                    renderModel.m_parts.back().material = it->second;
                }
                auto partId = texBuffers[tk];

                for( int i = 0; i < 3; ++i )
                {
                    RenderVertexWithNormal iv;
                    iv.position = vertices[tri.vertices[i]].toRenderSystem();
                    iv.texcoord0.x = proxy.uvCoordinates[i].xpixel / 255.0f;
                    iv.texcoord0.y = proxy.uvCoordinates[i].ypixel / 255.0f;
                    iv.normal = normals[tri.vertices[i]].toRenderSystem();
                    renderModel.m_parts[partId].indices.emplace_back(vbuf.size());
                    vbuf.push_back(iv);
                }
            }
            mesh->setVertexData(reinterpret_cast<float*>(vbuf.data()), 0, vbuf.size());
        }

        return renderModel.toModel(mesh);
    }
}
