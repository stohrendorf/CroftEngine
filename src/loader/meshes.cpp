#include "meshes.h"

#include "render/textureanimator.h"
#include "primitives.h"

#include <gsl.h>

#include <boost/lexical_cast.hpp>
#include <boost/range/adaptors.hpp>

namespace loader
{
    gameplay::Model* Mesh::createMesh(const std::vector<TextureLayoutProxy>& textureProxies,
                                      const std::map<TextureLayoutProxy::TextureKey, gameplay::Material*>& materials,
                                      const std::vector<gameplay::Material*>& colorMaterials,
                                      render::TextureAnimator& animator) const
    {
        BOOST_ASSERT(colorMaterials.size() == 256);

        RenderModel<RenderVertex> nonNormalModel;
        RenderModel<RenderVertexWithNormal> normalModel;

        // texture => mesh buffer
        std::map<TextureLayoutProxy::TextureKey, size_t> texBuffers;
        for( const QuadFace& quad : textured_rectangles )
        {
            const TextureLayoutProxy& proxy = textureProxies.at(quad.proxyId);
            if(texBuffers.find(proxy.textureKey) == texBuffers.end())
            {
                normalModel.m_parts.emplace_back();
                auto it = materials.find(proxy.textureKey);
                Expects(it != materials.end());
                normalModel.m_parts.back().material = it->second;
                texBuffers[proxy.textureKey] = normalModel.m_parts.size() - 1;
            }
            auto partId = texBuffers[proxy.textureKey];

            animator.registerVertex(quad.proxyId, partId, 0, normalModel.addVertex(partId, quad.vertices[0], proxy.uvCoordinates[0], vertices, normals));
            animator.registerVertex(quad.proxyId, partId, 1, normalModel.addVertex(partId, quad.vertices[1], proxy.uvCoordinates[1], vertices, normals));
            animator.registerVertex(quad.proxyId, partId, 2, normalModel.addVertex(partId, quad.vertices[2], proxy.uvCoordinates[2], vertices, normals));
            animator.registerVertex(quad.proxyId, partId, 0, normalModel.addVertex(partId, quad.vertices[0], proxy.uvCoordinates[0], vertices, normals));
            animator.registerVertex(quad.proxyId, partId, 2, normalModel.addVertex(partId, quad.vertices[2], proxy.uvCoordinates[2], vertices, normals));
            animator.registerVertex(quad.proxyId, partId, 3, normalModel.addVertex(partId, quad.vertices[3], proxy.uvCoordinates[3], vertices, normals));
        }
        for( const QuadFace& quad : colored_rectangles )
        {
            TextureLayoutProxy::TextureKey tk;
            tk.blendingMode = BlendingMode::Solid;
            tk.flags = 0;
            tk.tileAndFlag = 0;
            tk.colorId = quad.proxyId & 0xff;

            if(texBuffers.find(tk) == texBuffers.end())
            {
                normalModel.m_parts.emplace_back();
                auto it = materials.find(tk);
                Expects(it != materials.end());
                normalModel.m_parts.back().material = it->second;
                texBuffers[tk] = normalModel.m_parts.size() - 1;
            }
            auto partId = texBuffers[tk];

            normalModel.addVertex(partId, quad.vertices[0], nullptr, vertices, normals);
            normalModel.addVertex(partId, quad.vertices[1], nullptr, vertices, normals);
            normalModel.addVertex(partId, quad.vertices[2], nullptr, vertices, normals);
            normalModel.addVertex(partId, quad.vertices[0], nullptr, vertices, normals);
            normalModel.addVertex(partId, quad.vertices[2], nullptr, vertices, normals);
            normalModel.addVertex(partId, quad.vertices[3], nullptr, vertices, normals);
        }
        for( const Triangle& poly : textured_triangles )
        {
            const TextureLayoutProxy& proxy = textureProxies.at(poly.proxyId);
            if(texBuffers.find(proxy.textureKey) == texBuffers.end())
            {
                normalModel.m_parts.emplace_back();
                auto it = materials.find(proxy.textureKey);
                Expects(it != materials.end());
                normalModel.m_parts.back().material = it->second;
                texBuffers[proxy.textureKey] = normalModel.m_parts.size() - 1;
            }
            auto partId = texBuffers[proxy.textureKey];

            for( int i = 0; i < 3; ++i )
                animator.registerVertex(poly.proxyId, partId, i, normalModel.addVertex(partId, poly.vertices[i], proxy.uvCoordinates[i], vertices, normals));
        }

        for( const Triangle& poly : colored_triangles )
        {
            TextureLayoutProxy::TextureKey tk;
            tk.blendingMode = BlendingMode::Solid;
            tk.flags = 0;
            tk.tileAndFlag = 0;
            tk.colorId = poly.proxyId & 0xff;

            if(texBuffers.find(tk) == texBuffers.end())
            {
                normalModel.m_parts.emplace_back();
                auto it = materials.find(tk);
                Expects(it != materials.end());
                normalModel.m_parts.back().material = it->second;
                texBuffers[tk] = normalModel.m_parts.size() - 1;
            }
            auto partId = texBuffers[tk];

            for( int i = 0; i < 3; ++i )
                normalModel.addVertex(partId, poly.vertices[i], nullptr, vertices, normals);
        }

        gameplay::Model* resModel = normalModel.toModel(true);

        return resModel;
    }
}
