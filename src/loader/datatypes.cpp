#include "datatypes.h"

#include "level/level.h"
#include "render/textureanimator.h"
#include "util/vmath.h"

#include <boost/lexical_cast.hpp>
#include <boost/range/adaptors.hpp>
#include <EffectHandler.h>

namespace loader
{

namespace
{
irr::u16 addVertex(irr::scene::SMeshBuffer& meshBuffer, uint16_t vertexIndex, const UVCoordinates& uvCoordinates, const std::vector<RoomVertex>& vertices)
{
    irr::video::S3DVertex iv;
    BOOST_ASSERT(vertexIndex < vertices.size());
    iv.Pos = vertices[vertexIndex].vertex.toIrrlicht();
    // TR5 only: iv.Normal = vertices[vertexIndex].normal.toIrrlicht();
    iv.Normal = { 1,0,0 };
    iv.TCoords.X = uvCoordinates.xpixel/255.0f;
    iv.TCoords.Y = uvCoordinates.ypixel/255.0f;
    iv.Color = vertices[vertexIndex].color;
    irr::s32 ivIdx = meshBuffer.Vertices.linear_search(iv);
    if(ivIdx < 0)
    {
        ivIdx = meshBuffer.Vertices.size();
        meshBuffer.Vertices.push_back(iv);
    }
    BOOST_ASSERT(ivIdx >= 0);
    BOOST_ASSERT(static_cast<irr::u32>(ivIdx) < meshBuffer.Vertices.size());
    meshBuffer.Indices.push_back(gsl::narrow<irr::u16>(ivIdx));
    return gsl::narrow_cast<irr::u16>(ivIdx);
}
} // anonymous namespace

irr::scene::IMeshSceneNode* Room::createSceneNode(irr::scene::ISceneManager* mgr,
                                                  int dumpIdx,
                                                  const level::Level& level,
                                                  const std::map<TextureLayoutProxy::TextureKey, irr::video::SMaterial>& materials,
                                                  const std::vector<irr::video::ITexture*>& textures,
                                                  const std::vector<irr::scene::SMesh*>& staticMeshes,
                                                  render::TextureAnimator& animator)
{
    // texture => mesh buffer
    std::map<TextureLayoutProxy::TextureKey, irr::scene::SMeshBuffer*> texBuffers;
    for(const QuadFace& quad : rectangles)
    {
        const TextureLayoutProxy& proxy = level.m_textureProxies.at(quad.proxyId);
        if(texBuffers.find(proxy.textureKey) == texBuffers.end())
            texBuffers[proxy.textureKey] = new irr::scene::SMeshBuffer();
        auto buf = texBuffers[proxy.textureKey];

        animator.registerVertex(quad.proxyId, buf, 0, addVertex(*buf, quad.vertices[0], proxy.uvCoordinates[0], vertices));
        animator.registerVertex(quad.proxyId, buf, 1, addVertex(*buf, quad.vertices[1], proxy.uvCoordinates[1], vertices));
        animator.registerVertex(quad.proxyId, buf, 2, addVertex(*buf, quad.vertices[2], proxy.uvCoordinates[2], vertices));
        animator.registerVertex(quad.proxyId, buf, 0, addVertex(*buf, quad.vertices[0], proxy.uvCoordinates[0], vertices));
        animator.registerVertex(quad.proxyId, buf, 2, addVertex(*buf, quad.vertices[2], proxy.uvCoordinates[2], vertices));
        animator.registerVertex(quad.proxyId, buf, 3, addVertex(*buf, quad.vertices[3], proxy.uvCoordinates[3], vertices));
    }
    for(const Triangle& poly : triangles)
    {
        const TextureLayoutProxy& proxy = level.m_textureProxies.at(poly.proxyId);
        if(texBuffers.find(proxy.textureKey) == texBuffers.end())
            texBuffers[proxy.textureKey] = new irr::scene::SMeshBuffer();
        auto buf = texBuffers[proxy.textureKey];

        for(int i = 0; i < 3; ++i)
            animator.registerVertex(poly.proxyId, buf, i, addVertex(*buf, poly.vertices[i], proxy.uvCoordinates[i], vertices));
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
        BOOST_LOG_TRIVIAL(debug) << "Intensity=" << darkness;
        if(isWaterRoom())
        {
            material.FogEnable = true;
        }
        buffer.second->Material = material;
        result->addMeshBuffer(buffer.second);
        buffer.second->drop();
    }

    result->recalculateBoundingBox();
    mgr->getMeshManipulator()->recalculateNormals(result);

    irr::scene::IMeshSceneNode* resultNode = mgr->addMeshSceneNode(result);
    result->drop();
    level.m_fx->addShadowToNode(resultNode);
    level.m_fx->addNodeToDepthPass(resultNode);
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
        const auto f = std::abs(light.specularIntensity) / 8191.0f;
        BOOST_ASSERT(f >= 0 && f <= 1);
        ld.DiffuseColor.set(light.color.a/255.0f*f, light.color.r/255.0f*f, light.color.g/255.0f*f, light.color.b/255.0f*f);
        ld.SpecularColor = ld.DiffuseColor;
        ld.AmbientColor = ld.DiffuseColor;
        //ld.Falloff = light.specularFade / 8192.0f;
        light.node->setPosition((light.position - position).toIrrlicht());
        light.node->setRotation(light.dir.toIrrlicht());
        light.node->setRadius(light.specularFade * 2);
        ld.Attenuation.Y = 1.0f / light.specularFade;
        //ld.Attenuation.Z = ld.Attenuation.X;
#ifndef NDEBUG
        //light.node->setDebugDataVisible(irr::scene::EDS_FULL);
#endif
    }

    for(const RoomStaticMesh& sm : this->staticMeshes)
    {
        auto idx = level.findStaticMeshIndexById(sm.meshId);
        BOOST_ASSERT(idx >= 0);
        BOOST_ASSERT(static_cast<size_t>(idx) < staticMeshes.size());
        irr::scene::IMeshSceneNode* smNode = mgr->addMeshSceneNode(staticMeshes[idx]);
        smNode->setRotation({0,util::auToDeg(sm.rotation),0});
        smNode->setPosition((sm.position - position).toIrrlicht());
        level.m_fx->addShadowToNode(smNode);
        resultNode->addChild(smNode);
        level.m_fx->addNodeToDepthPass(smNode);
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

        irr::scene::IBillboardSceneNode* n = mgr->addBillboardSceneNode(resultNode, dim, (vertices[sprite.vertex].vertex - core::TRCoordinates{0, tex.bottom_side/2, 0}).toIrrlicht(), -1, 0, 0);
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
        gsl::not_null<irr::io::IWriteFile*> file = mgr->getFileSystem()->createAndWriteFile(outputName);

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

irr::core::aabbox3di StaticMesh::getCollisionBox(const core::TRCoordinates & pos, core::Angle angle) const
{
    auto result = collision_box;

    const auto axis = core::axisFromAngle(angle, 45_deg);
    switch(*axis)
    {
        case core::Axis::PosZ:
            // nothing to do
            break;
        case core::Axis::PosX:
            std::swap(result.MinEdge.X, result.MinEdge.Z);
            result.MinEdge.Z *= -1;
            std::swap(result.MaxEdge.X, result.MaxEdge.Z);
            result.MaxEdge.Z *= -1;
            break;
        case core::Axis::NegZ:
            result.MinEdge.X *= -1;
            result.MinEdge.Z *= -1;
            result.MaxEdge.X *= -1;
            result.MaxEdge.Z *= -1;
            break;
        case core::Axis::NegX:
            std::swap(result.MinEdge.X, result.MinEdge.Z);
            result.MinEdge.X *= -1;
            std::swap(result.MaxEdge.X, result.MaxEdge.Z);
            result.MaxEdge.X *= -1;
            break;
    }

    result.MinEdge += irr::core::vector3di(pos.X, pos.Y, pos.Z);
    result.MaxEdge += irr::core::vector3di(pos.X, pos.Y, pos.Z);
    result.repair();
    return result;
}

void Room::patchHeightsForBlock(const engine::ItemController& ctrl, int height)
{
    core::RoomBoundPosition pos = ctrl.getRoomBoundPosition();
    //! @todo Ugly const_cast
    auto groundSector = const_cast<loader::Sector*>(ctrl.getLevel().findFloorSectorWithClampedPosition(pos).get());
    pos.position.Y += height - loader::SectorSize;
    const auto topSector = ctrl.getLevel().findFloorSectorWithClampedPosition(pos);

    const auto q = height / loader::QuarterSectorSize;
    if(groundSector->floorHeight == -127)
    {
        groundSector->floorHeight = topSector->ceilingHeight + q;
    }
    else
    {
        groundSector->floorHeight += q;
        if(groundSector->floorHeight == topSector->ceilingHeight)
            groundSector->floorHeight = -127;
    }

    if(groundSector->boxIndex == 0xffff)
        return;

    //! @todo Ugly const_cast
    loader::Box& box = const_cast<loader::Box&>(ctrl.getLevel().m_boxes[groundSector->boxIndex]);
    if((box.overlap_index & 0x8000) == 0)
        return;

    if(height >= 0)
        box.overlap_index &= ~0x4000;
    else
        box.overlap_index |= 0x4000;
}

}
