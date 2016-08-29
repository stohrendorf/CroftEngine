#include "datatypes.h"

#include "level/level.h"
#include "render/textureanimator.h"
#include "util/vmath.h"

#include <boost/lexical_cast.hpp>
#include <boost/range/adaptors.hpp>

namespace loader
{

gameplay::Node* Room::createSceneNode(int dumpIdx,
                                                  const level::Level& level,
                                                  const std::map<TextureLayoutProxy::TextureKey, gameplay::Material*>& materials,
                                                  const std::vector<gameplay::Texture*>& textures,
                                                  const std::vector<gameplay::Mesh*>& staticMeshes,
                                                  render::TextureAnimator& animator)
{
    RenderModel model;

    // texture => mesh buffer
    std::map<TextureLayoutProxy::TextureKey, size_t> texBuffers;
    for(const QuadFace& quad : rectangles)
    {
        const TextureLayoutProxy& proxy = level.m_textureProxies.at(quad.proxyId);
        if(texBuffers.find(proxy.textureKey) == texBuffers.end())
        {
            model.m_parts.emplace_back();
            auto it = materials.find(proxy.textureKey);
            Expects(it != materials.end());
            model.m_parts.back().material = it->second;
            texBuffers[proxy.textureKey] = model.m_parts.size() - 1;
        }
        auto partId = texBuffers[proxy.textureKey];

        animator.registerVertex(quad.proxyId, partId, 0, model.addVertex(partId, quad.vertices[0], proxy.uvCoordinates[0], vertices));
        animator.registerVertex(quad.proxyId, partId, 1, model.addVertex(partId, quad.vertices[1], proxy.uvCoordinates[1], vertices));
        animator.registerVertex(quad.proxyId, partId, 2, model.addVertex(partId, quad.vertices[2], proxy.uvCoordinates[2], vertices));
        animator.registerVertex(quad.proxyId, partId, 0, model.addVertex(partId, quad.vertices[0], proxy.uvCoordinates[0], vertices));
        animator.registerVertex(quad.proxyId, partId, 2, model.addVertex(partId, quad.vertices[2], proxy.uvCoordinates[2], vertices));
        animator.registerVertex(quad.proxyId, partId, 3, model.addVertex(partId, quad.vertices[3], proxy.uvCoordinates[3], vertices));
    }
    for(const Triangle& poly : triangles)
    {
        const TextureLayoutProxy& proxy = level.m_textureProxies.at(poly.proxyId);
        if(texBuffers.find(proxy.textureKey) == texBuffers.end())
        {
            model.m_parts.emplace_back();
            auto it = materials.find(proxy.textureKey);
            Expects(it != materials.end());
            model.m_parts.back().material = it->second;
            texBuffers[proxy.textureKey] = model.m_parts.size() - 1;
        }
        auto partId = texBuffers[proxy.textureKey];

        for(int i = 0; i < 3; ++i)
            animator.registerVertex(poly.proxyId, partId, i, model.addVertex(partId, poly.vertices[i], proxy.uvCoordinates[i], vertices));
    }

    gameplay::Model* resModel = model.toModel(true);
    node = gameplay::Node::create();
    node->setDrawable(resModel);
    resModel->release();

    for(Light& light : lights)
    {
        const auto f = std::abs(light.specularIntensity) / 8191.0f;
        BOOST_ASSERT(f >= 0 && f <= 1);

        switch(light.getLightType())
        {
            case LightType::Shadow:
                BOOST_LOG_TRIVIAL(debug) << "Light: Shadow";
                light.node = gameplay::Light::createPoint(light.color.r / 255.0f*f, light.color.g / 255.0f*f, light.color.b / 255.0f*f, light.specularFade);
                break;
            case LightType::Null:
            case LightType::Point:
                BOOST_LOG_TRIVIAL(debug) << "Light: Null/Point";
                light.node = gameplay::Light::createPoint(light.color.r / 255.0f*f, light.color.g / 255.0f*f, light.color.b / 255.0f*f, light.specularFade);
                break;
            case LightType::Spotlight:
                BOOST_LOG_TRIVIAL(debug) << "Light: Spot";
                light.node = gameplay::Light::createSpot(light.color.r / 255.0f*f, light.color.g / 255.0f*f, light.color.b / 255.0f*f, light.specularFade, light.r_inner, light.r_outer);
                break;
            case LightType::Sun:
                BOOST_LOG_TRIVIAL(debug) << "Light: Sun";
                light.node = gameplay::Light::createDirectional(light.color.r / 255.0f*f, light.color.g / 255.0f*f, light.color.b / 255.0f*f);
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
    }

    for(const RoomStaticMesh& sm : this->staticMeshes)
    {
        auto idx = level.findStaticMeshIndexById(sm.meshId);
        BOOST_ASSERT(idx >= 0);
        BOOST_ASSERT(static_cast<size_t>(idx) < staticMeshes.size());
        auto subNode = gameplay::Node::create();
        subNode->setDrawable(gameplay::Model::create(staticMeshes[idx]));
        subNode->setRotation({ 0,1,0 }, util::auToRad(sm.rotation));
        subNode->setTranslation((sm.position - position).toRenderSystem());
        node->addChild(subNode);
    }
    node->setTranslation(position.toRenderSystem());

    node->setId(("Room:" + boost::lexical_cast<std::string>(dumpIdx)).c_str());

    for(const Sprite& sprite : sprites)
    {
        BOOST_ASSERT(sprite.vertex < vertices.size());
        BOOST_ASSERT(sprite.texture < level.m_spriteTextures.size());

        const SpriteTexture& tex = level.m_spriteTextures[sprite.texture];

        gameplay::Vector2 dim{ static_cast<float>(tex.right_side - tex.left_side + 1), static_cast<float>(tex.bottom_side - tex.top_side + 1) };
        BOOST_ASSERT(dim.x > 0);
        BOOST_ASSERT(dim.y > 0);

        irr::scene::IBillboardSceneNode* n = mgr->addBillboardSceneNode(resultNode, dim, (vertices[sprite.vertex].vertex - core::TRCoordinates{0, tex.bottom_side/2, 0}).toRenderSystem(), -1, 0, 0);
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

    // resultNode->addShadowVolumeSceneNode();

    return node;
}

gameplay::Texture* DWordTexture::toTexture() const
{
    gameplay::Image* img = gameplay::Image::create(256, 256, gameplay::Image::Format::RGBA, reinterpret_cast<uint8_t*>(img->getData()));
    gameplay::Texture* tex = gameplay::Texture::create(img, false);
    img->release();
    return tex;
}

gameplay::BoundingBox StaticMesh::getCollisionBox(const core::TRCoordinates& pos, core::Angle angle) const
{
    auto result = collision_box;

    const auto axis = core::axisFromAngle(angle, 45_deg);
    switch(*axis)
    {
        case core::Axis::PosZ:
            // nothing to do
            break;
        case core::Axis::PosX:
            std::swap(result.min.x, result.min.z);
            result.min.z *= -1;
            std::swap(result.max.x, result.max.z);
            result.max.z *= -1;
            break;
        case core::Axis::NegZ:
            result.min.x *= -1;
            result.min.z *= -1;
            result.max.x *= -1;
            result.max.z *= -1;
            break;
        case core::Axis::NegX:
            std::swap(result.min.x, result.min.z);
            result.min.x *= -1;
            std::swap(result.max.x, result.max.z);
            result.max.x *= -1;
            break;
    }

    result.min += {pos.X, pos.Y, pos.Z};
    result.max += {pos.X, pos.Y, pos.Z};
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
