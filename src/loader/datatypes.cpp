#include "datatypes.h"

#include "defaultanimdispatcher.h"
#include "level.h"
#include "util/vmath.h"

#include <boost/lexical_cast.hpp>
#include <boost/range/adaptors.hpp>

namespace loader
{

namespace
{
irr::video::S3DVertex& addVertex(irr::scene::SMeshBuffer& meshBuffer, uint16_t vertexIndex, const UVVertex* tex, const std::vector<TRCoordinates>& vertices, const std::vector<TRCoordinates>& normals)
{
    irr::video::S3DVertex iv;
    iv.Color.set(0xffffffff);
    BOOST_ASSERT(vertexIndex < vertices.size());
    iv.Pos = vertices[vertexIndex].toIrrlicht();
    if(!normals.empty())
        iv.Normal = normals[vertexIndex].toIrrlicht();
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
    iv.Pos = vertices[vertexIndex].vertex.toIrrlicht();
    iv.TCoords.X = tex.xpixel/255.0f;
    iv.TCoords.Y = tex.ypixel/255.0f;
    iv.Color = vertices[vertexIndex].color;
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
        
        addVertex(*buf, quad.vertices[0], &tex.vertices[0], vertices, normals);
        addVertex(*buf, quad.vertices[1], &tex.vertices[1], vertices, normals);
        addVertex(*buf, quad.vertices[2], &tex.vertices[2], vertices, normals);
        addVertex(*buf, quad.vertices[0], &tex.vertices[0], vertices, normals);
        addVertex(*buf, quad.vertices[2], &tex.vertices[2], vertices, normals);
        addVertex(*buf, quad.vertices[3], &tex.vertices[3], vertices, normals);
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
        
        addVertex(*buf, quad.vertices[0], nullptr, vertices, normals);
        addVertex(*buf, quad.vertices[1], nullptr, vertices, normals);
        addVertex(*buf, quad.vertices[2], nullptr, vertices, normals);
        addVertex(*buf, quad.vertices[0], nullptr, vertices, normals);
        addVertex(*buf, quad.vertices[2], nullptr, vertices, normals);
        addVertex(*buf, quad.vertices[3], nullptr, vertices, normals);
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

    for(irr::scene::SMeshBuffer* buffer : texBuffers|boost::adaptors::map_values)
        buffer->recalculateBoundingBox();
    
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
        mgr->getFileSystem()->changeWorkingDirectoryTo("dump");
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
        
        mgr->getFileSystem()->changeWorkingDirectoryTo("..");
    }
    
    return result;
}

irr::scene::IMeshSceneNode* Room::createSceneNode(irr::scene::ISceneManager* mgr,
                                                  int dumpIdx,
                                                  const Level& level,
                                                  const std::map<UVTexture::TextureKey, irr::video::SMaterial>& materials,
                                                  const std::vector<irr::video::ITexture*>& textures,
                                                  const std::vector<irr::scene::SMesh*>& staticMeshes)
{
    // texture => mesh buffer
    std::map<UVTexture::TextureKey, irr::scene::SMeshBuffer*> texBuffers;
    for(const QuadFace& quad : rectangles)
    {
        const UVTexture& tex = level.m_uvTextures.at(quad.uvTexture);
        if(texBuffers.find(tex.textureKey) == texBuffers.end())
            texBuffers[tex.textureKey] = new irr::scene::SMeshBuffer();
        auto buf = texBuffers[tex.textureKey];
        
        addVertex(*buf, quad.vertices[0], tex.vertices[0], vertices);
        addVertex(*buf, quad.vertices[1], tex.vertices[1], vertices);
        addVertex(*buf, quad.vertices[2], tex.vertices[2], vertices);
        addVertex(*buf, quad.vertices[0], tex.vertices[0], vertices);
        addVertex(*buf, quad.vertices[2], tex.vertices[2], vertices);
        addVertex(*buf, quad.vertices[3], tex.vertices[3], vertices);
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
    
    for(irr::scene::SMeshBuffer* buffer : texBuffers|boost::adaptors::map_values)
        buffer->recalculateBoundingBox();
    
    irr::scene::SMesh* result = new irr::scene::SMesh();
    for(auto& buffer : texBuffers)
    {
        auto it = materials.find(buffer.first);
        BOOST_ASSERT(it != materials.end());
        irr::video::SMaterial material = it->second;
        material.Lighting = false;
        material.DiffuseColor.set(0);
        material.AmbientColor.set(0);
        material.Shininess = 20;
        material.SpecularColor.set(0);
        material.EmissiveColor = lightColor.toSColor(0.25f);
        BOOST_LOG_TRIVIAL(debug) << "Intensity=" << intensity1;
        if(flags & TR_ROOM_FLAG_WATER)
        {
            material.FogEnable = true;
        }
        buffer.second->Material = material;
        result->addMeshBuffer(buffer.second);
        buffer.second->drop();
    }
    
    result->recalculateBoundingBox();
    
    irr::scene::IMeshSceneNode* resultNode = mgr->addMeshSceneNode(result);
    result->drop();
    // resultNode->setDebugDataVisible(irr::scene::EDS_FULL);
    // resultNode->setAutomaticCulling(irr::scene::EAC_OFF);
    for(Light& light : lights)
    {
        light.node = mgr->addLightSceneNode(resultNode);
        light.node->enableCastShadow(true);
        switch(light.getLightType())
        {
            case LightType::Shadow:
                BOOST_LOG_TRIVIAL(debug) << "Light: Shadow";
                light.node->setLightType(irr::video::ELT_POINT);
                break;
            case LightType::Null:
            case LightType::Point:
                BOOST_LOG_TRIVIAL(debug) << "Light: Null/Point";
                light.node->setLightType(irr::video::ELT_POINT);
                break;
            case LightType::Spotlight:
                BOOST_LOG_TRIVIAL(debug) << "Light: Spot";
                light.node->setLightType(irr::video::ELT_SPOT);
                break;
            case LightType::Sun:
                BOOST_LOG_TRIVIAL(debug) << "Light: Sun";
                light.node->setLightType(irr::video::ELT_DIRECTIONAL);
                break;
        }

        BOOST_LOG_TRIVIAL(debug) << "  - Position: " << light.position.X << "/" << light.position.Y << "/" << light.position.Z;
        BOOST_LOG_TRIVIAL(debug) << "  - Length: " << light.length;
        BOOST_LOG_TRIVIAL(debug) << "  - Color: " << light.color.a/255.0f << "/" << light.color.r/255.0f << "/" << light.color.g/255.0f << "/" << light.color.b/255.0f;
        BOOST_LOG_TRIVIAL(debug) << "  - Specular Fade: " << light.specularFade;
        BOOST_LOG_TRIVIAL(debug) << "  - Specular Intensity: " << light.specularIntensity;
        BOOST_LOG_TRIVIAL(debug) << "  - Inner: " << light.r_inner;
        BOOST_LOG_TRIVIAL(debug) << "  - Outer: " << light.r_outer;
        BOOST_LOG_TRIVIAL(debug) << "  - Intensity: " << light.intensity;

        irr::video::SLight& ld = light.node->getLightData();
        ld.InnerCone = light.r_inner;
        ld.OuterCone = light.r_outer;
        const auto f = light.specularIntensity / 4095.0f;
        ld.DiffuseColor.set(light.color.a/255.0f*f, light.color.r/255.0f*f, light.color.g/255.0f*f, light.color.b/255.0f*f);
        ld.SpecularColor = ld.DiffuseColor;
        ld.AmbientColor = ld.DiffuseColor;
        ld.Falloff = light.specularFade / 8192.0f;
        light.node->setPosition((light.position - position).toIrrlicht());
        light.node->setRotation(light.dir.toIrrlicht());
        light.node->setRadius(light.specularFade);
        ld.Attenuation.Z = ld.Attenuation.Y;
#ifndef NDEBUG
        light.node->setDebugDataVisible(irr::scene::EDS_FULL);
#endif
    }
    
    for(const RoomStaticMesh& sm : this->staticMeshes)
    {
        auto idx = level.findStaticMeshIndexByObjectId(sm.object_id);
        BOOST_ASSERT(idx >= 0);
        BOOST_ASSERT(static_cast<size_t>(idx) < staticMeshes.size());
        irr::scene::IMeshSceneNode* smNode = mgr->addMeshSceneNode(staticMeshes[idx]);
        smNode->setRotation({0,-util::auToDeg(sm.rotation),0});
        smNode->setPosition((sm.position - position).toIrrlicht());
        resultNode->addChild(smNode);
    }
    resultNode->setPosition(position.toIrrlicht());
    resultNode->updateAbsolutePosition();
    
    resultNode->setName(("Room:" + boost::lexical_cast<std::string>(dumpIdx)).c_str());
    
    for(const Sprite& sprite : sprites)
    {
        BOOST_ASSERT(sprite.vertex < vertices.size());
        BOOST_ASSERT(sprite.texture < level.m_spriteTextures.size());
        
        const SpriteTexture& tex = level.m_spriteTextures[sprite.texture];
        
        irr::core::vector2df dim{ static_cast<irr::f32>(tex.right_side - tex.left_side + 1), static_cast<irr::f32>(tex.bottom_side - tex.top_side + 1) };
        BOOST_ASSERT(dim.X > 0);
        BOOST_ASSERT(dim.Y > 0);
        
        irr::scene::IBillboardSceneNode* n = mgr->addBillboardSceneNode(resultNode, dim, (vertices[sprite.vertex].vertex - TRCoordinates{0, tex.bottom_side/2, 0}).toIrrlicht(), -1, 0, 0);
        n->setMaterialType(irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL);
        n->getMaterial(0).BlendOperation = irr::video::EBO_ADD;
        n->getMaterial(0).EmissiveColor.set(0);
        n->setMaterialFlag(irr::video::EMF_LIGHTING, true);
        n->setMaterialTexture( 0, textures[tex.texture] );
        {
            irr::video::SColor col;
            col.set(gsl::narrow<irr::u8>(lightColor.a * 255), gsl::narrow<irr::u8>(lightColor.r * 255), gsl::narrow<irr::u8>(lightColor.g * 255), gsl::narrow<irr::u8>(lightColor.b * 255));
            n->getMaterial(0).AmbientColor = col;
            n->getMaterial(0).DiffuseColor = col;
            n->getMaterial(0).SpecularColor = col;
            n->getMaterial(0).Lighting = true;
        }
        
        n->getMaterial(0).setTextureMatrix(0, tex.buildTextureMatrix());
    }
    
    if(dumpIdx >= 0)
    {
        mgr->getFileSystem()->changeWorkingDirectoryTo("dump");
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
        
        mgr->getFileSystem()->changeWorkingDirectoryTo("..");
    }
    
    // resultNode->addShadowVolumeSceneNode();
    node = resultNode;
    
    return resultNode;
}

irr::video::ITexture* DWordTexture::toTexture(irr::scene::ISceneManager* mgr, int texIdx)
{
    auto img = mgr->getVideoDriver()->createImageFromData(
                   irr::video::ECF_A8R8G8B8,
                   {256, 256},
                   &pixels[0][0]);
    irr::io::path p;
    p = "tex_";
    p += boost::lexical_cast<std::string>(texIdx).c_str();
    p += ".png";
    auto tex = mgr->getVideoDriver()->addTexture(p, img);

    mgr->getFileSystem()->changeWorkingDirectoryTo("dump");
    mgr->getVideoDriver()->writeImageToFile(img, p);
    mgr->getFileSystem()->changeWorkingDirectoryTo("..");
    
    img->drop();
    return tex;
}

AbstractTriggerHandler::AbstractTriggerHandler(const Item& item, const std::shared_ptr<DefaultAnimDispatcher>& dispatcher)
    : m_triggerMask{item.getTriggerMask()}
    , m_lock{item.getLockBit()}
    , m_event{item.getEventBit()}
    , m_dispatcher{dispatcher}
{
    BOOST_ASSERT(dispatcher != nullptr);
}

void AbstractTriggerHandler::prepare()
{
    if(m_triggerMask == 0x1F)
    {
        activate(nullptr, 0, TriggerOp::Or, false, 0);
        m_lock = true;
        m_event = true;
    }
}

TimerState AbstractTriggerHandler::updateTimer(irr::f32 frameTime)
{
    if(m_timer <= 0)
        return TimerState::Idle;

    m_timer -= frameTime;
    if(m_timer < 0)
        m_timer = 0;
    if(m_timer == 0)
        return TimerState::Stopped;
    
    return TimerState::Active;
}

void AbstractTriggerHandler::activate(AbstractTriggerHandler* activator, uint8_t mask, TriggerOp trigger_op, bool onlyOnce, irr::f32 timer)
{
    if(m_lock && m_triggerMask == 0x1F)
        return;
    
    // Apply trigger mask to entity mask.
    
    if(trigger_op == TriggerOp::XOr)
        m_triggerMask ^= mask;   // Switch cases
    else
        m_triggerMask |= mask;    // Other cases
    
    // Apply trigger lock to entity lock.
    
    m_lock |= onlyOnce;
    
    // Full entity mask (11111) is always a reason to activate an entity.
    // If mask is not full, entity won't activate - no exclusions.
    
    if(m_triggerMask == 0x1F && !m_event)
    {
        onActivate(activator);
        m_event = true;
    }
    else if(m_triggerMask != 0x1f && m_event)
    {
        onDeactivate(activator);
        m_event = false;
    }
    
    m_timer = timer;
}

void AbstractTriggerHandler::deactivate(AbstractTriggerHandler* activator, bool onlyOnce)
{
    if(m_lock)
        return;
    
    m_lock |= onlyOnce;
    
    // Execute entity deactivation function, only if activation was previously set.
    if(m_event)
    {
        onDeactivate(activator);
        
        // Activation mask and timer are forced to zero when entity is deactivated.
        // Activity lock is ignored, since it can't be raised by antitriggers.
        
        m_timer = 0;
        m_triggerMask = 0;
        m_event = false;
    }
}
irr::core::aabbox3di StaticMesh::getCollisionBox(const TRCoordinates & pos, int16_t angle) const
{
    irr::core::aabbox3di result{
        collision_box[0].X, collision_box[0].Y, collision_box[0].Z,
        collision_box[1].X, collision_box[1].Y, collision_box[1].Z
    };
    result.repair();

    const auto axis = util::axisFromAngle(angle, 0x2000);
    switch(*axis)
    {
        case util::Axis::PosZ:
            std::swap(result.MinEdge.X, result.MinEdge.Z);
            result.MinEdge.X *= -1;
            std::swap(result.MaxEdge.X, result.MaxEdge.Z);
            result.MaxEdge.X *= -1;
            break;
        case util::Axis::PosX:
            // nothing to do
            break;
        case util::Axis::NegZ:
            result.MinEdge.X *= -1;
            result.MaxEdge.X *= -1;
            break;
        case util::Axis::NegX:
            std::swap(result.MinEdge.X, result.MinEdge.Z);
            result.MinEdge.Z *= -1;
            std::swap(result.MaxEdge.X, result.MaxEdge.Z);
            result.MaxEdge.Z *= -1;
            break;
    }

    result.MinEdge += irr::core::vector3di(pos.X, pos.Y, pos.Z);
    result.MaxEdge += irr::core::vector3di(pos.X, pos.Y, pos.Z);
    return result;
}
}
