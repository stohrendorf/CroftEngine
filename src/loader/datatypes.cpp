#include "datatypes.h"

#include "util/vmath.h"
#include "level.h"

#include <boost/lexical_cast.hpp>
#include <iostream>

namespace loader
{

namespace
{
irr::video::S3DVertex& addVertex(irr::scene::SMeshBuffer& meshBuffer, uint16_t vertexIndex, const UVVertex* tex, const std::vector<Vertex>& vertices, const std::vector<Vertex>& normals)
{
    irr::video::S3DVertex iv;
    iv.Color.set(0xffffffff);
    BOOST_ASSERT(vertexIndex < vertices.size());
    iv.Pos = vertices[vertexIndex];
    if(!normals.empty())
        iv.Normal = -normals[vertexIndex];
    else
        iv.Normal.set(0,0,1);
    if(tex != nullptr)
    {
        iv.TCoords.X = tex->xpixel/255.0f;
        iv.TCoords.Y = tex->ypixel/255.0f;
    }
    irr::s32 ivIdx = meshBuffer.Vertices.linear_search(iv);
    if(ivIdx < 0)
    {
        ivIdx = meshBuffer.Vertices.size();
        meshBuffer.Vertices.push_back(iv);
    }
    BOOST_ASSERT(ivIdx >= 0);
    BOOST_ASSERT(static_cast<irr::u32>(ivIdx) < meshBuffer.Vertices.size());
    meshBuffer.Indices.push_back(ivIdx);
    return meshBuffer.Vertices[ivIdx];
}

irr::video::S3DVertex& addVertex(irr::scene::SMeshBuffer& meshBuffer, uint16_t vertexIndex, const UVVertex& tex, const std::vector<RoomVertex>& vertices)
{
    irr::video::S3DVertex iv;
    BOOST_ASSERT(vertexIndex < vertices.size());
    iv.Pos = vertices[vertexIndex].vertex;
    iv.TCoords.X = (tex.xpixel+tex.xcoordinate)/255.0f;
    iv.TCoords.Y = (tex.ypixel+tex.ycoordinate)/255.0f;
    iv.Color = vertices[vertexIndex].color;
    irr::s32 ivIdx = meshBuffer.Vertices.linear_search(iv);
    if(ivIdx < 0)
    {
        ivIdx = meshBuffer.Vertices.size();
        meshBuffer.Vertices.push_back(iv);
    }
    BOOST_ASSERT(ivIdx >= 0);
    BOOST_ASSERT(ivIdx < meshBuffer.Vertices.size());
    meshBuffer.Indices.push_back(ivIdx);
    return meshBuffer.Vertices[ivIdx];
}
} // anonymous namespace

irr::scene::SMesh* Mesh::createMesh(irr::scene::ISceneManager* mgr,
                                    int dumpIdx,
                                    const std::vector<UVTexture>& uvTextures,
                                    const std::map<UVTexture::TextureKey, irr::video::SMaterial>& materials,
                                    const std::vector<irr::video::SMaterial>& colorMaterials) const
{
    BOOST_ASSERT(colorMaterials.size() == 256);

    // texture => mesh buffer
    std::map<UVTexture::TextureKey, irr::scene::SMeshBuffer*> texBuffers;
    for(const QuadFace& quad : textured_rectangles)
    {
        const UVTexture& tex = uvTextures.at(quad.uvTexture);
        if(texBuffers.find(tex.textureKey) == texBuffers.end())
            texBuffers[tex.textureKey] = new irr::scene::SMeshBuffer();
        auto buf = texBuffers[tex.textureKey];
        
        addVertex(*buf, quad.uvCoordinates[0], &tex.vertices[0], vertices, normals);
        addVertex(*buf, quad.uvCoordinates[1], &tex.vertices[1], vertices, normals);
        addVertex(*buf, quad.uvCoordinates[2], &tex.vertices[2], vertices, normals);
        addVertex(*buf, quad.uvCoordinates[0], &tex.vertices[0], vertices, normals);
        addVertex(*buf, quad.uvCoordinates[2], &tex.vertices[2], vertices, normals);
        addVertex(*buf, quad.uvCoordinates[3], &tex.vertices[3], vertices, normals);
    }
    for(const QuadFace& quad : colored_rectangles)
    {
        UVTexture::TextureKey tk;
        tk.blendingMode = BlendingMode::Solid;
        tk.flags = 0;
        tk.tileAndFlag = 0;
        tk.colorId = quad.uvTexture&0xff;
        
        if(texBuffers.find(tk) == texBuffers.end())
            texBuffers[tk] = new irr::scene::SMeshBuffer();
        auto buf = texBuffers[tk];
        
        addVertex(*buf, quad.uvCoordinates[0], nullptr, vertices, normals);
        addVertex(*buf, quad.uvCoordinates[1], nullptr, vertices, normals);
        addVertex(*buf, quad.uvCoordinates[2], nullptr, vertices, normals);
        addVertex(*buf, quad.uvCoordinates[0], nullptr, vertices, normals);
        addVertex(*buf, quad.uvCoordinates[2], nullptr, vertices, normals);
        addVertex(*buf, quad.uvCoordinates[3], nullptr, vertices, normals);
    }
    for(const Triangle& poly : textured_triangles)
    {
        const UVTexture& tex = uvTextures.at(poly.uvTexture);
        if(texBuffers.find(tex.textureKey) == texBuffers.end())
            texBuffers[tex.textureKey] = new irr::scene::SMeshBuffer();
        auto buf = texBuffers[tex.textureKey];

        for(int i=0; i<3; ++i)
            addVertex(*buf, poly.vertices[i], &tex.vertices[i], vertices, normals);
    }

    for(const Triangle& poly : colored_triangles)
    {
        UVTexture::TextureKey tk;
        tk.blendingMode = BlendingMode::Solid;
        tk.flags = 0;
        tk.tileAndFlag = 0;
        tk.colorId = poly.uvTexture&0xff;

        if(texBuffers.find(tk) == texBuffers.end())
            texBuffers[tk] = new irr::scene::SMeshBuffer();
        auto buf = texBuffers[tk];

        for(int i=0; i<3; ++i)
            addVertex(*buf, poly.vertices[i], nullptr, vertices, normals);
    }
    
    irr::scene::SMesh* result = new irr::scene::SMesh();
    for(auto& buffer : texBuffers)
    {
        auto it = materials.find(buffer.first);
        if(it != materials.end())
        {
            buffer.second->Material = it->second;
        }
        else if(buffer.first.colorId >= 0 && buffer.first.colorId <= 255)
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
    
    if(dumpIdx >= 0)
    {
        chdir("dump");
        irr::scene::IMeshWriter* meshWriter = mgr->createMeshWriter(irr::scene::EMWT_COLLADA);
        irr::io::path outputName;
        outputName = "object_";
        outputName += boost::lexical_cast<std::string>(dumpIdx).c_str();
        outputName += ".dae";
        irr::io::IWriteFile* file = mgr->getFileSystem()->createAndWriteFile(outputName);
        BOOST_ASSERT(file != nullptr);

        meshWriter->writeMesh( file, result );
        file->drop();
        meshWriter->drop();
        
        chdir("..");
    }
    
    return result;
}

irr::scene::IMeshSceneNode* Room::createSceneNode(irr::scene::ISceneManager* mgr,
                                                  int dumpIdx,
                                                  const Level& level,
                                                  const std::map<UVTexture::TextureKey, irr::video::SMaterial>& materials,
                                                  const std::vector<irr::video::ITexture*>& textures,
                                                  const std::vector<irr::scene::SMesh*>& staticMeshes) const
{
    // texture => mesh buffer
    std::map<UVTexture::TextureKey, irr::scene::SMeshBuffer*> texBuffers;
    for(const QuadFace& quad : rectangles)
    {
        const UVTexture& tex = level.m_uvTextures.at(quad.uvTexture);
        if(texBuffers.find(tex.textureKey) == texBuffers.end())
            texBuffers[tex.textureKey] = new irr::scene::SMeshBuffer();
        auto buf = texBuffers[tex.textureKey];
        
        addVertex(*buf, quad.uvCoordinates[0], tex.vertices[0], vertices);
        addVertex(*buf, quad.uvCoordinates[1], tex.vertices[1], vertices);
        addVertex(*buf, quad.uvCoordinates[2], tex.vertices[2], vertices);
        addVertex(*buf, quad.uvCoordinates[0], tex.vertices[0], vertices);
        addVertex(*buf, quad.uvCoordinates[2], tex.vertices[2], vertices);
        addVertex(*buf, quad.uvCoordinates[3], tex.vertices[3], vertices);
    }
    for(const Triangle& poly : triangles)
    {
        const UVTexture& tex = level.m_uvTextures.at(poly.uvTexture);
        if(texBuffers.find(tex.textureKey) == texBuffers.end())
            texBuffers[tex.textureKey] = new irr::scene::SMeshBuffer();
        auto buf = texBuffers[tex.textureKey];

        for(int i=0; i<3; ++i)
            addVertex(*buf, poly.vertices[i], tex.vertices[i], vertices);
    }
    
    irr::scene::SMesh* result = new irr::scene::SMesh();
    for(auto& buffer : texBuffers)
    {
        auto it = materials.find(buffer.first);
        BOOST_ASSERT(it != materials.end());
        irr::video::SMaterial material = it->second;
        material.Lighting = true;
        if(flags & TR_ROOM_FLAG_WATER)
        {
            material.FogEnable = true;
        }
        buffer.second->Material = material;
        result->addMeshBuffer(buffer.second);
    }
    
    result->recalculateBoundingBox();
    
    irr::scene::IMeshSceneNode* resultNode = mgr->addMeshSceneNode(result);
    // resultNode->setDebugDataVisible(irr::scene::EDS_FULL);
    resultNode->setAutomaticCulling(irr::scene::EAC_OFF);
    if(flags & TR_ROOM_FLAG_WATER)
    {
    }
    for(const Light& light : lights)
    {
        irr::scene::ILightSceneNode* ln = mgr->addLightSceneNode(resultNode);
        switch(light.getLightType())
        {
            case LightType::Shadow:
                ln->enableCastShadow(true);
                // fall-through
            case LightType::Null:
            case LightType::Point:
                ln->setLightType(irr::video::ELT_POINT);
                break;
            case LightType::Spotlight:
                ln->setLightType(irr::video::ELT_SPOT);
                break;
            case LightType::Sun:
                ln->setLightType(irr::video::ELT_DIRECTIONAL);
                break;
        }

        ln->setPosition(light.position - offset);
        ln->setRotation(light.dir);
        ln->setRadius(light.length);
        
        irr::video::SLight ld;
        ld.InnerCone = light.r_inner;
        ld.InnerCone = light.r_outer;
        ld.DiffuseColor.set(light.color.a/255.0f, light.color.r/255.0f, light.color.g/255.0f, light.color.b/255.0f);
        ld.SpecularColor = ld.DiffuseColor;
        ld.AmbientColor = ld.DiffuseColor;
        ld.Falloff = light.fade1;
        ln->setLightData(ld);
    }
    
    for(const RoomStaticMesh& sm : static_meshes)
    {
        auto idx = level.findStaticMeshIndexByObjectId(sm.object_id);
        BOOST_ASSERT(idx >= 0);
        BOOST_ASSERT(idx < staticMeshes.size());
        irr::scene::IMeshSceneNode* smNode = mgr->addMeshSceneNode(staticMeshes[idx]);
        smNode->setRotation({0,sm.rotation,0});
        smNode->setPosition(sm.position - offset);
        resultNode->addChild(smNode);
    }
    resultNode->setPosition(offset);
    
    resultNode->setName(("Room:" + boost::lexical_cast<std::string>(dumpIdx)).c_str());
    
    for(const Sprite& sprite : sprites)
    {
        BOOST_ASSERT(sprite.vertex < vertices.size());
        BOOST_ASSERT(sprite.texture < level.m_spriteTextures.size());
        
        const SpriteTexture& tex = level.m_spriteTextures[sprite.texture];
        
        irr::core::vector2df dim(tex.right_side-tex.left_side+1, tex.bottom_side-tex.top_side+1);
        BOOST_ASSERT(dim.X > 0);
        BOOST_ASSERT(dim.Y > 0);
        
        irr::scene::IBillboardSceneNode* n = mgr->addBillboardSceneNode(resultNode, dim, vertices[sprite.vertex].vertex+irr::core::vector3df{0,tex.bottom_side/2,0}, -1, 0, 0);
        n->setMaterialType(irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL);
        n->setMaterialFlag(irr::video::EMF_BLEND_OPERATION, irr::video::EBO_ADD);
        n->setMaterialTexture( 0, textures[tex.texture] );
        
        n->getMaterial(0).setTextureMatrix(0, tex.buildTextureMatrix());
    }
    
    if(dumpIdx >= 0)
    {
        chdir("dump");
        irr::scene::IMeshWriter* meshWriter = mgr->createMeshWriter(irr::scene::EMWT_COLLADA);
        irr::io::path outputName;
        outputName = "room_";
        outputName += boost::lexical_cast<std::string>(dumpIdx).c_str();
        outputName += ".dae";
        irr::io::IWriteFile* file = mgr->getFileSystem()->createAndWriteFile(outputName);
        BOOST_ASSERT(file != nullptr);

        meshWriter->writeMesh( file, result );
        file->drop();
        meshWriter->drop();
        
        chdir("..");
    }
    
    return resultNode;
}

irr::video::ITexture* DWordTexture::toTexture(irr::video::IVideoDriver* drv, int texIdx)
{
    auto img = drv->createImageFromData(
                   irr::video::ECF_A8R8G8B8,
                   {256, 256},
                   &pixels[0][0]);
    irr::io::path p;
    p = "tex_";
    p += boost::lexical_cast<std::string>(texIdx).c_str();
    p += ".png";
    auto tex = drv->addTexture(p, img);

    chdir("dump");
    drv->writeImageToFile(img, p);
    chdir("..");
    
    img->drop();
    return tex;
}

}
