#include "meshes.h"

#include "render/textureanimator.h"
#include "primitives.h"

#include <gsl.h>

#include <boost/lexical_cast.hpp>
#include <boost/range/adaptors.hpp>

namespace loader
{
    namespace
    {
        irr::u16 addVertex(irr::scene::SMeshBuffer& meshBuffer, uint16_t vertexIndex, const UVCoordinates* uvCoordinates, const std::vector<core::TRCoordinates>& vertices, const std::vector<core::TRCoordinates>& normals)
        {
            irr::video::S3DVertex iv;
            iv.Color.set(0xffffffff);
            BOOST_ASSERT(vertexIndex < vertices.size());
            iv.Pos = vertices[vertexIndex].toIrrlicht();
            if( !normals.empty() )
                iv.Normal = normals[vertexIndex].toIrrlicht();
            else
                iv.Normal.set(0, 0, 1);
            if( uvCoordinates != nullptr )
            {
                iv.TCoords.X = uvCoordinates->xpixel / 255.0f;
                iv.TCoords.Y = uvCoordinates->ypixel / 255.0f;
            }
            irr::s32 ivIdx = meshBuffer.Vertices.linear_search(iv);
            if( ivIdx < 0 )
            {
                ivIdx = meshBuffer.Vertices.size();
                meshBuffer.Vertices.push_back(iv);
            }
            BOOST_ASSERT(ivIdx >= 0);
            BOOST_ASSERT(static_cast<irr::u32>(ivIdx) < meshBuffer.Vertices.size());
            meshBuffer.Indices.push_back(gsl::narrow<irr::u16>(ivIdx));
            return gsl::narrow_cast<irr::u16>(ivIdx);
        }
    }

    irr::scene::SMesh* Mesh::createMesh(irr::scene::ISceneManager* mgr,
                                        int dumpIdx,
                                        const std::vector<TextureLayoutProxy>& textureProxies,
                                        const std::map<TextureLayoutProxy::TextureKey, irr::video::SMaterial>& materials,
                                        const std::vector<irr::video::SMaterial>& colorMaterials,
                                        render::TextureAnimator& animator) const
    {
        BOOST_ASSERT(colorMaterials.size() == 256);

        // texture => mesh buffer
        std::map<TextureLayoutProxy::TextureKey, irr::scene::SMeshBuffer*> texBuffers;
        for( const QuadFace& quad : textured_rectangles )
        {
            const TextureLayoutProxy& proxy = textureProxies.at(quad.proxyId);
            if( texBuffers.find(proxy.textureKey) == texBuffers.end() )
                texBuffers[proxy.textureKey] = new irr::scene::SMeshBuffer();
            auto buf = texBuffers[proxy.textureKey];

            animator.registerVertex(quad.proxyId, buf, 0, addVertex(*buf, quad.vertices[0], &proxy.uvCoordinates[0], vertices, normals));
            animator.registerVertex(quad.proxyId, buf, 1, addVertex(*buf, quad.vertices[1], &proxy.uvCoordinates[1], vertices, normals));
            animator.registerVertex(quad.proxyId, buf, 2, addVertex(*buf, quad.vertices[2], &proxy.uvCoordinates[2], vertices, normals));
            animator.registerVertex(quad.proxyId, buf, 0, addVertex(*buf, quad.vertices[0], &proxy.uvCoordinates[0], vertices, normals));
            animator.registerVertex(quad.proxyId, buf, 2, addVertex(*buf, quad.vertices[2], &proxy.uvCoordinates[2], vertices, normals));
            animator.registerVertex(quad.proxyId, buf, 3, addVertex(*buf, quad.vertices[3], &proxy.uvCoordinates[3], vertices, normals));
        }
        for( const QuadFace& quad : colored_rectangles )
        {
            TextureLayoutProxy::TextureKey tk;
            tk.blendingMode = BlendingMode::Solid;
            tk.flags = 0;
            tk.tileAndFlag = 0;
            tk.colorId = quad.proxyId & 0xff;

            if( texBuffers.find(tk) == texBuffers.end() )
                texBuffers[tk] = new irr::scene::SMeshBuffer();
            auto buf = texBuffers[tk];

            addVertex(*buf, quad.vertices[0], nullptr, vertices, normals);
            addVertex(*buf, quad.vertices[1], nullptr, vertices, normals);
            addVertex(*buf, quad.vertices[2], nullptr, vertices, normals);
            addVertex(*buf, quad.vertices[0], nullptr, vertices, normals);
            addVertex(*buf, quad.vertices[2], nullptr, vertices, normals);
            addVertex(*buf, quad.vertices[3], nullptr, vertices, normals);
        }
        for( const Triangle& poly : textured_triangles )
        {
            const TextureLayoutProxy& proxy = textureProxies.at(poly.proxyId);
            if( texBuffers.find(proxy.textureKey) == texBuffers.end() )
                texBuffers[proxy.textureKey] = new irr::scene::SMeshBuffer();
            auto buf = texBuffers[proxy.textureKey];

            for( int i = 0; i < 3; ++i )
                animator.registerVertex(poly.proxyId, buf, i, addVertex(*buf, poly.vertices[i], &proxy.uvCoordinates[i], vertices, normals));
        }

        for( irr::scene::SMeshBuffer* buffer : texBuffers | boost::adaptors::map_values )
            buffer->recalculateBoundingBox();

        for( const Triangle& poly : colored_triangles )
        {
            TextureLayoutProxy::TextureKey tk;
            tk.blendingMode = BlendingMode::Solid;
            tk.flags = 0;
            tk.tileAndFlag = 0;
            tk.colorId = poly.proxyId & 0xff;

            if( texBuffers.find(tk) == texBuffers.end() )
                texBuffers[tk] = new irr::scene::SMeshBuffer();
            auto buf = texBuffers[tk];

            for( int i = 0; i < 3; ++i )
                addVertex(*buf, poly.vertices[i], nullptr, vertices, normals);
        }

        irr::scene::SMesh* result = new irr::scene::SMesh();
        for( auto& buffer : texBuffers )
        {
            auto it = materials.find(buffer.first);
            if( it != materials.end() )
            {
                buffer.second->Material = it->second;
            }
            else if( buffer.first.colorId >= 0 && buffer.first.colorId <= 255 )
            {
                buffer.second->Material = colorMaterials[buffer.first.colorId];
            }
            else
            {
                BOOST_LOG_TRIVIAL(error) << "Invalid mesh material";
            }

            result->addMeshBuffer(buffer.second);
        }

        result->recalculateBoundingBox();

        if( dumpIdx >= 0 )
        {
            mgr->getFileSystem()->changeWorkingDirectoryTo("dump");
            irr::scene::IMeshWriter* meshWriter = mgr->createMeshWriter(irr::scene::EMWT_COLLADA);
            irr::io::path outputName;
            outputName = "object_";
            outputName += boost::lexical_cast<std::string>(dumpIdx).c_str();
            outputName += ".dae";
            gsl::not_null<irr::io::IWriteFile*> file = mgr->getFileSystem()->createAndWriteFile(outputName);

            meshWriter->writeMesh(file, result);
            file->drop();
            meshWriter->drop();

            mgr->getFileSystem()->changeWorkingDirectoryTo("..");
        }

        return result;
    }
}
