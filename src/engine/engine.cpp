#include "engine.h"

#include "audio/tracktype.h"
#include "floordata/floordata.h"
#include "items/animating.h"
#include "items/atlanteanlava.h"
#include "items/barricade.h"
#include "items/bat.h"
#include "items/bear.h"
#include "items/block.h"
#include "items/boulder.h"
#include "items/bridgeflat.h"
#include "items/collapsiblefloor.h"
#include "items/crocodile.h"
#include "items/cutsceneactors.h"
#include "items/dart.h"
#include "items/dartgun.h"
#include "items/door.h"
#include "items/flameemitter.h"
#include "items/gorilla.h"
#include "items/keyhole.h"
#include "items/larson.h"
#include "items/lavaparticleemitter.h"
#include "items/lightningball.h"
#include "items/lion.h"
#include "items/mummy.h"
#include "items/mutant.h"
#include "items/mutantegg.h"
#include "items/pickupitem.h"
#include "items/pierre.h"
#include "items/puzzlehole.h"
#include "items/raptor.h"
#include "items/rat.h"
#include "items/scionpiece.h"
#include "items/slammingdoors.h"
#include "items/slopedbridge.h"
#include "items/stubitem.h"
#include "items/swingingblade.h"
#include "items/switch.h"
#include "items/swordofdamocles.h"
#include "items/tallblock.h"
#include "items/teethspikes.h"
#include "items/thorhammer.h"
#include "items/trapdoordown.h"
#include "items/trapdoorup.h"
#include "items/trex.h"
#include "items/underwaterswitch.h"
#include "items/waterfallmist.h"
#include "items/wolf.h"
#include "laranode.h"
#include "loader/file/level/level.h"
#include "loader/file/texturecache.h"
#include "loader/trx/trx.h"
#include "render/gl/font.h"
#include "render/renderpipeline.h"
#include "render/scene/scene.h"
#include "render/textureanimator.h"
#include "script/reflection.h"
#include "tracks_tr1.h"
#include "ui/label.h"
#include "video/player.h"

#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/locale/generator.hpp>
#include <boost/locale/info.hpp>
#include <boost/range/adaptor/map.hpp>
#include <glm/gtx/norm.hpp>
#include <locale>

namespace engine
{
namespace
{
sol::state createScriptEngine()
{
  sol::state engine;
  engine.open_libraries(sol::lib::base, sol::lib::math, sol::lib::package);
  engine["package"]["path"] = (boost::filesystem::path("scripts") / "?.lua").string();
  engine["package"]["cpath"] = "";

  engine.set_usertype(core::TRVec::userType());
  engine.set_usertype(ai::CreatureInfo::userType());
  engine.set_usertype(script::ObjectInfo::userType());
  engine.set_usertype(script::TrackInfo::userType());

  engine.new_enum("ActivationState",
                  "INACTIVE",
                  items::TriggerState::Inactive,
                  "ACTIVE",
                  items::TriggerState::Active,
                  "DEACTIVATED",
                  items::TriggerState::Deactivated,
                  "INVISIBLE",
                  items::TriggerState::Invisible);

  engine.new_enum(
    "Mood", "BORED", ai::Mood::Bored, "ATTACK", ai::Mood::Attack, "ESCAPE", ai::Mood::Escape, "STALK", ai::Mood::Stalk);

  engine.new_enum("TrackType",
                  "AMBIENT",
                  audio::TrackType::Ambient,
                  "INTERCEPTION",
                  audio::TrackType::Interception,
                  "AMBIENT_EFFECT",
                  audio::TrackType::AmbientEffect,
                  "LARA_TALK",
                  audio::TrackType::LaraTalk);

  {
    sol::table tbl = engine.create_table("TR1SoundId");
    for(const auto& entry : EnumUtil<TR1SoundId>::all())
      tbl[entry.second] = static_cast<std::underlying_type_t<TR1SoundId>>(entry.first);
  }

  {
    sol::table tbl = engine.create_table("TR1TrackId");
    for(const auto& entry : EnumUtil<TR1TrackId>::all())
      tbl[entry.second] = static_cast<std::underlying_type_t<TR1TrackId>>(entry.first);
  }

  return engine;
}
} // namespace

std::tuple<int8_t, int8_t> Engine::getFloorSlantInfo(gsl::not_null<const loader::file::Sector*> sector,
                                                     const core::TRVec& position) const
{
  while(sector->roomBelow != nullptr)
  {
    sector = sector->roomBelow->getSectorByAbsolutePosition(position);
  }

  static const auto zero = std::make_tuple(0, 0);

  if(position.Y + core::QuarterSectorSize * 2 < sector->floorHeight)
    return zero;
  if(sector->floorData == nullptr)
    return zero;
  if(floordata::FloorDataChunk{*sector->floorData}.type != floordata::FloorDataChunkType::FloorSlant)
    return zero;

  const auto fd = sector->floorData[1];
  return std::make_tuple(gsl::narrow_cast<int8_t>(fd.get() & 0xff), gsl::narrow_cast<int8_t>(fd.get() >> 8));
}

void Engine::swapAllRooms()
{
  BOOST_LOG_TRIVIAL(info) << "Swapping rooms";
  for(auto& room : m_level->m_rooms)
  {
    if(room.alternateRoom.get() < 0)
      continue;

    BOOST_ASSERT(static_cast<size_t>(room.alternateRoom.get()) < m_level->m_rooms.size());
    swapWithAlternate(room, m_level->m_rooms.at(room.alternateRoom.get()));
  }

  m_roomsAreSwapped = !m_roomsAreSwapped;
  m_level->updateRoomBasedCaches();
}

bool Engine::isValid(const loader::file::AnimFrame* frame) const
{
  return reinterpret_cast<const short*>(frame) >= m_level->m_poseFrames.data()
         && reinterpret_cast<const short*>(frame) < m_level->m_poseFrames.data() + m_level->m_poseFrames.size();
}

const std::unique_ptr<loader::file::SpriteSequence>& Engine::findSpriteSequenceForType(core::TypeId type) const
{
  return m_level->findSpriteSequenceForType(type);
}

const loader::file::StaticMesh* Engine::findStaticMeshById(core::StaticMeshId meshId) const
{
  return m_level->findStaticMeshById(meshId);
}

const std::vector<loader::file::Room>& Engine::getRooms() const
{
  return m_level->m_rooms;
}

const std::vector<loader::file::Box>& Engine::getBoxes() const
{
  return m_level->m_boxes;
}

std::map<loader::file::TextureKey, gsl::not_null<std::shared_ptr<render::scene::Material>>>
  Engine::createMaterials(const gsl::not_null<std::shared_ptr<render::scene::ShaderProgram>>& shader)
{
  const auto texMask = gameToEngine(m_level->m_gameVersion) == loader::file::level::Engine::TR4
                         ? loader::file::TextureIndexMaskTr4
                         : loader::file::TextureIndexMask;
  std::map<loader::file::TextureKey, gsl::not_null<std::shared_ptr<render::scene::Material>>> materials;
  for(loader::file::TextureTile& tile : m_level->m_textureTiles)
  {
    const auto& key = tile.textureKey;
    if(materials.find(key) != materials.end())
      continue;

    materials.emplace(key, tile.createMaterial(m_level->m_textures[key.tileAndFlag & texMask].texture, shader));
  }
  return materials;
}

std::shared_ptr<LaraNode> Engine::createItems()
{
  m_lightningShader = render::scene::ShaderProgram::createFromFile("shaders/lightning.vert", "shaders/lightning.frag");

  std::shared_ptr<LaraNode> lara = nullptr;
  int id = -1;
  for(loader::file::Item& item : m_level->m_items)
  {
    ++id;

    const auto* room = &m_level->m_rooms.at(item.room.get());

    if(const auto& model = findAnimatedModelForType(item.type))
    {
      std::shared_ptr<items::ItemNode> modelNode;

      if(item.type == TR1ItemId::Lara)
      {
        lara = std::make_shared<LaraNode>(this, room, item, *model);
        modelNode = lara;
      }
      else if(auto objectInfo = m_scriptEngine["getObjectInfo"].call(-1))
      {
        BOOST_LOG_TRIVIAL(info) << "Instantiating scripted type " << toString(item.type.get_as<TR1ItemId>()) << "/id "
                                << id;

        modelNode = std::make_shared<items::ScriptedItem>(this, room, item, *model, objectInfo);
        for(gsl::index boneIndex = 0; boneIndex < model->models.size(); ++boneIndex)
        {
          auto node = std::make_shared<render::scene::Node>(modelNode->getNode()->getId()
                                                            + "/bone:" + std::to_string(boneIndex));
          node->setDrawable(model->models[boneIndex].get());
          addChild(modelNode->getNode(), node);
        }

        BOOST_ASSERT(modelNode->getNode()->getChildren().size() == gsl::narrow<size_t>(model->meshes.size()));
      }
      else if(item.type == TR1ItemId::Wolf)
      {
        modelNode = std::make_shared<items::Wolf>(this, room, item, *model);
      }
      else if(item.type == TR1ItemId::Bear)
      {
        modelNode = std::make_shared<items::Bear>(this, room, item, *model);
      }
      else if(item.type == TR1ItemId::Bat)
      {
        modelNode = std::make_shared<items::Bat>(this, room, item, *model);
      }
      else if(item.type == TR1ItemId::FallingBlock)
      {
        modelNode = std::make_shared<items::CollapsibleFloor>(this, room, item, *model);
      }
      else if(item.type == TR1ItemId::SwingingBlade)
      {
        modelNode = std::make_shared<items::SwingingBlade>(this, room, item, *model);
      }
      else if(item.type == TR1ItemId::RollingBall)
      {
        modelNode = std::make_shared<items::RollingBall>(this, room, item, *model);
      }
      else if(item.type == TR1ItemId::Dart)
      {
        modelNode = std::make_shared<items::Dart>(this, room, item, *model);
      }
      else if(item.type == TR1ItemId::DartEmitter)
      {
        modelNode = std::make_shared<items::DartGun>(this, room, item, *model);
      }
      else if(item.type == TR1ItemId::LiftingDoor)
      {
        modelNode = std::make_shared<items::TrapDoorUp>(this, room, item, *model);
      }
      else if(item.type >= TR1ItemId::PushableBlock1 && item.type <= TR1ItemId::PushableBlock4)
      {
        modelNode = std::make_shared<items::Block>(this, room, item, *model);
      }
      else if(item.type == TR1ItemId::MovingBlock)
      {
        modelNode = std::make_shared<items::TallBlock>(this, room, item, *model);
      }
      else if(item.type == TR1ItemId::WallSwitch)
      {
        modelNode = std::make_shared<items::Switch>(this, room, item, *model);
      }
      else if(item.type == TR1ItemId::UnderwaterSwitch)
      {
        modelNode = std::make_shared<items::UnderwaterSwitch>(this, room, item, *model);
      }
      else if(item.type >= TR1ItemId::Door1 && item.type <= TR1ItemId::Door8)
      {
        modelNode = std::make_shared<items::Door>(this, room, item, *model);
      }
      else if(item.type >= TR1ItemId::Trapdoor1 && item.type <= TR1ItemId::Trapdoor2)
      {
        modelNode = std::make_shared<items::TrapDoorDown>(this, room, item, *model);
      }
      else if(item.type == TR1ItemId::BridgeFlat)
      {
        modelNode = std::make_shared<items::BridgeFlat>(this, room, item, *model);
      }
      else if(item.type == TR1ItemId::BridgeTilt1)
      {
        modelNode = std::make_shared<items::BridgeSlope1>(this, room, item, *model);
      }
      else if(item.type == TR1ItemId::BridgeTilt2)
      {
        modelNode = std::make_shared<items::BridgeSlope2>(this, room, item, *model);
      }
      else if(item.type >= TR1ItemId::Keyhole1 && item.type <= TR1ItemId::Keyhole4)
      {
        modelNode = std::make_shared<items::KeyHole>(this, room, item, *model);
      }
      else if(item.type >= TR1ItemId::PuzzleHole1 && item.type <= TR1ItemId::PuzzleHole4)
      {
        modelNode = std::make_shared<items::PuzzleHole>(this, room, item, *model);
      }
      else if(item.type >= TR1ItemId::Animating1 && item.type <= TR1ItemId::Animating3)
      {
        modelNode = std::make_shared<items::Animating>(this, room, item, *model);
      }
      else if(item.type == TR1ItemId::TeethSpikes)
      {
        modelNode = std::make_shared<items::TeethSpikes>(this, room, item, *model);
      }
      else if(item.type == TR1ItemId::Raptor)
      {
        modelNode = std::make_shared<items::Raptor>(this, room, item, *model);
      }
      else if(item.type == TR1ItemId::SwordOfDamocles || item.type == TR1ItemId::FallingCeiling)
      {
        modelNode = std::make_shared<items::SwordOfDamocles>(this, room, item, *model);
      }
      else if(item.type == TR1ItemId::CutsceneActor1)
      {
        modelNode = std::make_shared<items::CutsceneActor1>(this, room, item, *model);
      }
      else if(item.type == TR1ItemId::CutsceneActor2)
      {
        modelNode = std::make_shared<items::CutsceneActor2>(this, room, item, *model);
      }
      else if(item.type == TR1ItemId::CutsceneActor3)
      {
        modelNode = std::make_shared<items::CutsceneActor3>(this, room, item, *model);
      }
      else if(item.type == TR1ItemId::CutsceneActor4)
      {
        modelNode = std::make_shared<items::CutsceneActor4>(this, room, item, *model);
      }
      else if(item.type == TR1ItemId::WaterfallMist)
      {
        modelNode = std::make_shared<items::WaterfallMist>(this, room, item, *model);
      }
      else if(item.type == TR1ItemId::TRex)
      {
        modelNode = std::make_shared<items::TRex>(this, room, item, *model);
      }
      else if(item.type == TR1ItemId::Mummy)
      {
        modelNode = std::make_shared<items::Mummy>(this, room, item, *model);
      }
      else if(item.type == TR1ItemId::Larson)
      {
        modelNode = std::make_shared<items::Larson>(this, room, item, *model);
      }
      else if(item.type == TR1ItemId::CrocodileOnLand || item.type == TR1ItemId::CrocodileInWater)
      {
        modelNode = std::make_shared<items::Crocodile>(this, room, item, *model);
      }
      else if(item.type == TR1ItemId::LionMale || item.type == TR1ItemId::LionFemale || item.type == TR1ItemId::Panther)
      {
        modelNode = std::make_shared<items::Lion>(this, room, item, *model);
      }
      else if(item.type == TR1ItemId::Barricade)
      {
        modelNode = std::make_shared<items::Barricade>(this, room, item, *model);
      }
      else if(item.type == TR1ItemId::Gorilla)
      {
        modelNode = std::make_shared<items::Gorilla>(this, room, item, *model);
      }
      else if(item.type == TR1ItemId::Pierre)
      {
        modelNode = std::make_shared<items::Pierre>(this, room, item, *model);
      }
      else if(item.type == TR1ItemId::ThorHammerBlock)
      {
        modelNode = std::make_shared<items::ThorHammerBlock>(this, room, item, *model);
      }
      else if(item.type == TR1ItemId::ThorHammerHandle)
      {
        modelNode = std::make_shared<items::ThorHammerHandle>(this, room, item, *model);
      }
      else if(item.type == TR1ItemId::FlameEmitter)
      {
        modelNode = std::make_shared<items::FlameEmitter>(this, room, item, *model);
      }
      else if(item.type == TR1ItemId::ThorLightningBall)
      {
        modelNode = std::make_shared<items::LightningBall>(this, room, item, *model, m_lightningShader);
      }
      else if(item.type == TR1ItemId::RatInWater || item.type == TR1ItemId::RatOnLand)
      {
        modelNode = std::make_shared<items::Rat>(this, room, item, *model);
      }
      else if(item.type == TR1ItemId::SlammingDoors)
      {
        modelNode = std::make_shared<items::SlammingDoors>(this, room, item, *model);
      }
      else if(item.type == TR1ItemId::FlyingMutant)
      {
        modelNode = std::make_shared<items::FlyingMutant>(this, room, item, *model);
      }
      else if(item.type == TR1ItemId::WalkingMutant1 || item.type == TR1ItemId::WalkingMutant2)
      {
        // Special handling, these are just "mutations" of the flying mutants (no wings).
        modelNode = std::make_shared<items::WalkingMutant>(
          this, room, item, *findAnimatedModelForType(TR1ItemId::FlyingMutant));
      }
      else if(item.type == TR1ItemId::MutantEggSmall || item.type == TR1ItemId::MutantEggBig)
      {
        modelNode = std::make_shared<items::MutantEgg>(this, room, item, *model);
      }
      else if(item.type == TR1ItemId::CentaurMutant)
      {
        modelNode = std::make_shared<items::CentaurMutant>(this, room, item, *model);
      }
      else if(item.type == TR1ItemId::TorsoBoss)
      {
        modelNode = std::make_shared<items::TorsoBoss>(this, room, item, *model);
      }
      else if(item.type == TR1ItemId::LavaParticleEmitter)
      {
        modelNode = std::make_shared<items::LavaParticleEmitter>(this, room, item, *model);
      }
      else if(item.type == TR1ItemId::FlowingAtlanteanLava)
      {
        modelNode = std::make_shared<items::AtlanteanLava>(this, room, item, *model);
      }
      else if(item.type == TR1ItemId::ScionPiece3)
      {
        modelNode = std::make_shared<items::ScionPiece3>(this, room, item, *model);
      }
      else if(item.type == TR1ItemId::ScionPiece4)
      {
        modelNode = std::make_shared<items::ScionPiece4>(this, room, item, *model);
      }
      else if(item.type == TR1ItemId::ScionHolder)
      {
        modelNode = std::make_shared<items::ScionHolder>(this, room, item, *model);
      }
      else
      {
        BOOST_LOG_TRIVIAL(warning) << "Unimplemented item " << toString(item.type.get_as<TR1ItemId>());

        modelNode = std::make_shared<items::StubItem>(this, room, item, *model);
        if(item.type == TR1ItemId::MidasGoldTouch || item.type == TR1ItemId::CameraTarget
           || item.type == TR1ItemId::LavaParticleEmitter || item.type == TR1ItemId::FlameEmitter
           || item.type == TR1ItemId::Earthquake)
        {
          modelNode->getNode()->setDrawable(nullptr);
          modelNode->getNode()->removeAllChildren();
        }
      }

      m_itemNodes.emplace(std::make_pair(id, modelNode));
      addChild(room->node, modelNode->getNode());

      modelNode->applyTransform();
      modelNode->updateLighting();

      continue;
    }

    if(const auto& spriteSequence = findSpriteSequenceForType(item.type))
    {
      BOOST_ASSERT(!findAnimatedModelForType(item.type));
      BOOST_ASSERT(!spriteSequence->sprites.empty());

      const loader::file::Sprite& sprite = spriteSequence->sprites[0];
      std::shared_ptr<items::ItemNode> node;

      if(item.type == TR1ItemId::ScionPiece1)
      {
        node = std::make_shared<items::ScionPiece>(this,
                                                   std::string("sprite(type:") + toString(item.type.get_as<TR1ItemId>())
                                                     + ")",
                                                   room,
                                                   item,
                                                   sprite,
                                                   m_spriteMaterial);
      }
      else if(item.type == TR1ItemId::Item141 || item.type == TR1ItemId::Item142 || item.type == TR1ItemId::Key1Sprite
              || item.type == TR1ItemId::Key2Sprite || item.type == TR1ItemId::Key3Sprite
              || item.type == TR1ItemId::Key4Sprite || item.type == TR1ItemId::Puzzle1Sprite
              || item.type == TR1ItemId::Puzzle2Sprite || item.type == TR1ItemId::Puzzle3Sprite
              || item.type == TR1ItemId::Puzzle4Sprite || item.type == TR1ItemId::PistolsSprite
              || item.type == TR1ItemId::ShotgunSprite || item.type == TR1ItemId::MagnumsSprite
              || item.type == TR1ItemId::UzisSprite || item.type == TR1ItemId::PistolAmmoSprite
              || item.type == TR1ItemId::ShotgunAmmoSprite || item.type == TR1ItemId::MagnumAmmoSprite
              || item.type == TR1ItemId::UziAmmoSprite || item.type == TR1ItemId::ExplosiveSprite
              || item.type == TR1ItemId::SmallMedipackSprite || item.type == TR1ItemId::LargeMedipackSprite
              || item.type == TR1ItemId::ScionPiece2 || item.type == TR1ItemId::LeadBarSprite)
      {
        node = std::make_shared<items::PickupItem>(this,
                                                   std::string("sprite(type:") + toString(item.type.get_as<TR1ItemId>())
                                                     + ")",
                                                   room,
                                                   item,
                                                   sprite,
                                                   m_spriteMaterial);
      }
      else
      {
        BOOST_LOG_TRIVIAL(warning) << "Unimplemented item " << toString(item.type.get_as<TR1ItemId>());
        node = std::make_shared<items::SpriteItemNode>(this,
                                                       std::string("sprite(type:")
                                                         + toString(item.type.get_as<TR1ItemId>()) + ")",
                                                       room,
                                                       item,
                                                       true,
                                                       sprite,
                                                       m_spriteMaterial);
      }

      m_itemNodes.emplace(std::make_pair(id, node));
      continue;
    }

    BOOST_LOG_TRIVIAL(error) << "Failed to find an appropriate animated model for item " << id << "/type "
                             << int(item.type.get());
  }

  return lara;
}

void Engine::loadSceneData(bool linearTextureInterpolation)
{
  for(auto& sprite : m_level->m_sprites)
  {
    sprite.texture = m_level->m_textures.at(sprite.texture_id.get()).texture;
    sprite.image = m_level->m_textures[sprite.texture_id.get()].image;
  }

  m_textureAnimator = std::make_shared<render::TextureAnimator>(
    m_level->m_animatedTextures, m_level->m_textureTiles, m_level->m_textures, linearTextureInterpolation);

  const auto texturedShader
    = render::scene::ShaderProgram::createFromFile("shaders/textured_2.vert", "shaders/textured_2.frag");
  const auto materials = createMaterials(texturedShader);

  const auto colorMaterial
    = std::make_shared<render::scene::Material>("shaders/colored_2.vert", "shaders/colored_2.frag");
  colorMaterial->getUniform("u_modelMatrix")->bindModelMatrix();
  colorMaterial->getUniform("u_modelViewMatrix")->bindModelViewMatrix();
  colorMaterial->getUniform("u_camProjection")->bindProjectionMatrix();

  BOOST_ASSERT(m_spriteMaterial == nullptr);
  m_spriteMaterial = std::make_shared<render::scene::Material>("shaders/textured_2.vert", "shaders/textured_2.frag");
  m_spriteMaterial->getRenderState().setCullFace(false);

  m_spriteMaterial->getUniform("u_modelMatrix")->bindModelMatrix();
  m_spriteMaterial->getUniform("u_camProjection")->bindProjectionMatrix();

  BOOST_ASSERT(m_portalMaterial == nullptr);
  m_portalMaterial = std::make_shared<render::scene::Material>("shaders/portal.vert", "shaders/portal.frag");
  m_portalMaterial->getRenderState().setCullFace(false);

  m_portalMaterial->getUniform("u_mvp")->bind([camera = m_renderer->getScene()->getActiveCamera()](
                                                const render::scene::Node& node, render::gl::ProgramUniform& uniform) {
    uniform.set(camera->getViewProjectionMatrix()); // portals are already in world space
  });

  for(auto& mesh : m_level->m_meshes)
  {
    m_models.emplace_back(mesh.createModel(m_level->m_textureTiles, materials, colorMaterial, *m_level->m_palette));
  }

  for(auto idx : m_level->m_meshIndices)
  {
    Expects(idx < m_models.size());
    m_modelsDirect.emplace_back(m_models[idx]);
    m_meshesDirect.emplace_back(&m_level->m_meshes[idx]);
  }

  for(const std::unique_ptr<loader::file::SkeletalModelType>& model :
      m_level->m_animatedModels | boost::adaptors::map_values)
  {
    if(model->nMeshes > 0)
    {
      model->models = make_span(&model->mesh_base_index.checkedFrom(m_modelsDirect), model->nMeshes);
      model->meshes = make_span(&model->mesh_base_index.checkedFrom(m_meshesDirect), model->nMeshes);
    }
  }

  const auto waterTexturedShader
    = render::scene::ShaderProgram::createFromFile("shaders/textured_2.vert", "shaders/textured_2.frag", {"WATER"});
  auto waterMaterials = createMaterials(waterTexturedShader);
  for(const auto& m : waterMaterials | boost::adaptors::map_values)
  {
    m->getUniform("u_time")->bind([this](const render::scene::Node& /*node*/, render::gl::ProgramUniform& uniform) {
      const auto now = std::chrono::time_point_cast<std::chrono::milliseconds>(m_renderer->getGameTime());
      uniform.set(gsl::narrow_cast<float>(now.time_since_epoch().count()));
    });
  }

  for(size_t i = 0; i < m_level->m_rooms.size(); ++i)
  {
    m_level->m_rooms[i].createSceneNode(
      i, *m_level, materials, waterMaterials, m_models, *m_textureAnimator, m_spriteMaterial, m_portalMaterial);
    m_renderer->getScene()->addNode(m_level->m_rooms[i].node);
  }

  m_lara = createItems();
  if(m_lara == nullptr)
  {
    m_cameraController = std::make_unique<CameraController>(this, m_renderer->getScene()->getActiveCamera(), true);

    for(const auto& item : m_level->m_items)
    {
      if(item.type == TR1ItemId::CutsceneActor1)
      {
        getCameraController().setPosition(item.position);
      }
    }
  }
  else
  {
    m_cameraController = std::make_unique<CameraController>(this, m_renderer->getScene()->getActiveCamera());
  }

  m_positionalEmitters.reserve(m_level->m_soundSources.size());
  for(loader::file::SoundSource& src : m_level->m_soundSources)
  {
    m_positionalEmitters.emplace_back(src.position.toRenderSystem(), &m_audioEngine->m_soundEngine);
    auto handle = m_audioEngine->playSound(src.sound_id, &m_positionalEmitters.back());
    Expects(handle != nullptr);
    handle->setLooping(true);
  }
}

std::shared_ptr<items::ItemNode> Engine::getItem(const uint16_t id) const
{
  const auto it = m_itemNodes.find(id);
  if(it == m_itemNodes.end())
    return nullptr;

  return it->second.get();
}

void Engine::drawBars(const gsl::not_null<std::shared_ptr<render::gl::Image<render::gl::SRGBA8>>>& image)
{
  if(m_lara->isInWater())
  {
    const auto x0 = gsl::narrow<int32_t>(m_window->getViewport().width - 110);

    for(int i = 7; i <= 13; ++i)
      image->line(x0 - 1, i, x0 + 101, i, m_level->m_palette->colors[0].toTextureColor());
    image->line(x0 - 2, 14, x0 + 102, 14, m_level->m_palette->colors[17].toTextureColor());
    image->line(x0 + 102, 6, x0 + 102, 14, m_level->m_palette->colors[17].toTextureColor());
    image->line(x0 + 102, 6, x0 + 102, 14, m_level->m_palette->colors[19].toTextureColor());
    image->line(x0 - 2, 6, x0 - 2, 14, m_level->m_palette->colors[19].toTextureColor());

    const int p = util::clamp(m_lara->getAir() * 100 / core::LaraAir, 0, 100);
    if(p > 0)
    {
      image->line(x0, 8, x0 + p, 8, m_level->m_palette->colors[32].toTextureColor());
      image->line(x0, 9, x0 + p, 9, m_level->m_palette->colors[41].toTextureColor());
      image->line(x0, 10, x0 + p, 10, m_level->m_palette->colors[32].toTextureColor());
      image->line(x0, 11, x0 + p, 11, m_level->m_palette->colors[19].toTextureColor());
      image->line(x0, 12, x0 + p, 12, m_level->m_palette->colors[21].toTextureColor());
    }
  }

  if(m_lara->getHandStatus() == HandStatus::Combat || m_lara->m_state.health <= 0_hp)
    m_healthBarTimeout = 40_frame;

  if(std::exchange(m_drawnHealth, m_lara->m_state.health) != m_lara->m_state.health)
    m_healthBarTimeout = 40_frame;

  m_healthBarTimeout -= 1_frame;
  if(m_healthBarTimeout <= -40_frame)
    return;

  uint8_t alpha = 255;
  if(m_healthBarTimeout < 0_frame)
  {
    alpha = util::clamp(255 - std::abs(255 * m_healthBarTimeout / 40_frame), 0, 255);
  }

  const int x0 = 8;
  for(int i = 7; i <= 13; ++i)
    image->line(x0 - 1, i, x0 + 101, i, m_level->m_palette->colors[0].toTextureColor(alpha), true);
  image->line(x0 - 2, 14, x0 + 102, 14, m_level->m_palette->colors[17].toTextureColor(alpha), true);
  image->line(x0 + 102, 6, x0 + 102, 14, m_level->m_palette->colors[17].toTextureColor(alpha), true);
  image->line(x0 + 102, 6, x0 + 102, 14, m_level->m_palette->colors[19].toTextureColor(alpha), true);
  image->line(x0 - 2, 6, x0 - 2, 14, m_level->m_palette->colors[19].toTextureColor(alpha), true);

  const int p = util::clamp(m_lara->m_state.health * 100 / core::LaraHealth, 0, 100);
  if(p > 0)
  {
    image->line(x0, 8, x0 + p, 8, m_level->m_palette->colors[8].toTextureColor(alpha), true);
    image->line(x0, 9, x0 + p, 9, m_level->m_palette->colors[11].toTextureColor(alpha), true);
    image->line(x0, 10, x0 + p, 10, m_level->m_palette->colors[8].toTextureColor(alpha), true);
    image->line(x0, 11, x0 + p, 11, m_level->m_palette->colors[6].toTextureColor(alpha), true);
    image->line(x0, 12, x0 + p, 12, m_level->m_palette->colors[24].toTextureColor(alpha), true);
  }
}

void Engine::useAlternativeLaraAppearance(const bool withHead)
{
  const auto& base = *findAnimatedModelForType(TR1ItemId::Lara);
  BOOST_ASSERT(gsl::narrow<size_t>(base.models.size()) == m_lara->getNode()->getChildren().size());

  const auto& alternate = *findAnimatedModelForType(TR1ItemId::AlternativeLara);
  BOOST_ASSERT(gsl::narrow<size_t>(alternate.models.size()) == m_lara->getNode()->getChildren().size());

  for(size_t i = 0; i < m_lara->getNode()->getChildren().size(); ++i)
    m_lara->getNode()->getChild(i)->setDrawable(alternate.models[i].get());

  if(!withHead)
    m_lara->getNode()->getChild(14)->setDrawable(base.models[14].get());
}

void Engine::dinoStompEffect(items::ItemNode& node)
{
  const auto d = node.m_state.position.position.toRenderSystem() - getCameraController().getPosition();
  const auto absD = glm::abs(d);

  static constexpr auto MaxD = (16 * core::SectorSize).get_as<float>();
  if(absD.x > MaxD || absD.y > MaxD || absD.z > MaxD)
    return;

  const auto x = (100_len).retype_as<float>() * (1 - glm::length2(d) / util::square(MaxD));
  getCameraController().setBounce(x.retype_as<core::Length>());
}

void Engine::turn180Effect(items::ItemNode& node)
{
  node.m_state.rotation.Y += 180_deg;
}

void Engine::laraNormalEffect()
{
  Expects(m_lara != nullptr);
  m_lara->setCurrentAnimState(LaraStateId::Stop);
  m_lara->setRequiredAnimState(LaraStateId::Unknown12);
  m_lara->m_state.anim = &m_level->m_animations[static_cast<int>(AnimationId::STAY_SOLID)];
  m_lara->m_state.frame_number = 185_frame;
  getCameraController().setMode(CameraMode::Chase);
  getCameraController().getCamera()->setFieldOfView(glm::radians(80.0f));
}

void Engine::laraBubblesEffect(items::ItemNode& node)
{
  const auto modelNode = dynamic_cast<items::ModelItemNode*>(&node);
  if(modelNode == nullptr)
    return;

  auto bubbleCount = util::rand15(12);
  if(bubbleCount == 0)
    return;

  node.playSoundEffect(TR1SoundId::LaraUnderwaterGurgle);

  const auto itemSpheres = modelNode->getSkeleton()->getBoneCollisionSpheres(
    node.m_state, *modelNode->getSkeleton()->getInterpolationInfo(modelNode->m_state).getNearestFrame(), nullptr);

  const auto position
    = core::TRVec{glm::vec3{translate(itemSpheres.at(14).m, core::TRVec{0_len, 0_len, 50_len}.toRenderSystem())[3]}};

  while(bubbleCount-- > 0)
  {
    auto particle
      = std::make_shared<BubbleParticle>(core::RoomBoundPosition{node.m_state.position.room, position}, *this);
    setParent(particle, node.m_state.position.room->node);
    m_particles.emplace_back(particle);
  }
}

void Engine::finishLevelEffect()
{
  m_levelFinished = true;
}

void Engine::earthquakeEffect()
{
  switch(m_effectTimer.get())
  {
  case 0:
    m_audioEngine->playSound(TR1SoundId::Explosion1, nullptr);
    getCameraController().setBounce(-250_len);
    break;
  case 3: m_audioEngine->playSound(TR1SoundId::RollingBall, nullptr); break;
  case 35: m_audioEngine->playSound(TR1SoundId::Explosion1, nullptr); break;
  case 20:
  case 50:
  case 70: m_audioEngine->playSound(TR1SoundId::TRexFootstep, nullptr); break;
  default:
    // silence compiler
    break;
  }

  m_effectTimer += 1_frame;
  if(m_effectTimer == 105_frame)
  {
    m_activeEffect.reset();
  }
}

void Engine::floodEffect()
{
  if(m_effectTimer <= 120_frame)
  {
    auto pos = m_lara->m_state.position.position;
    core::Frame mul = 0_frame;
    if(m_effectTimer >= 30_frame)
    {
      mul = m_effectTimer - 30_frame;
    }
    else
    {
      mul = 30_frame - m_effectTimer;
    }
    pos.Y = 100_len * mul / 1_frame + getCameraController().getCenter().position.Y;
    m_audioEngine->playSound(TR1SoundId::WaterFlow3, pos.toRenderSystem());
  }
  else
  {
    m_activeEffect.reset();
  }
  m_effectTimer += 1_frame;
}

void Engine::chandelierEffect()
{
  m_audioEngine->playSound(TR1SoundId::GlassyFlow, nullptr);
  m_activeEffect.reset();
}

void Engine::raisingBlockEffect()
{
  m_effectTimer += 1_frame;
  if(m_effectTimer == 5_frame)
  {
    m_audioEngine->playSound(TR1SoundId::Clank, nullptr);
    m_activeEffect.reset();
  }
}

void Engine::stairsToSlopeEffect()
{
  if(m_effectTimer <= 120_frame)
  {
    if(m_effectTimer == 0_frame)
    {
      m_audioEngine->playSound(TR1SoundId::HeavyDoorSlam, nullptr);
    }
    auto pos = getCameraController().getCenter().position;
    pos.Y += 100_spd * m_effectTimer;
    m_audioEngine->playSound(TR1SoundId::FlowingAir, pos.toRenderSystem());
  }
  else
  {
    m_activeEffect.reset();
  }
  m_effectTimer += 1_frame;
}

void Engine::sandEffect()
{
  if(m_effectTimer <= 120_frame)
  {
    m_audioEngine->playSound(TR1SoundId::LowHum, nullptr);
  }
  else
  {
    m_activeEffect.reset();
  }
  m_effectTimer += 1_frame;
}

void Engine::explosionEffect()
{
  m_audioEngine->playSound(TR1SoundId::LowPitchedSettling, nullptr);
  getCameraController().setBounce(-75_len);
  m_activeEffect.reset();
}

void Engine::laraHandsFreeEffect()
{
  m_lara->setHandStatus(HandStatus::None);
}

void Engine::flipMapEffect()
{
  swapAllRooms();
}

void Engine::unholsterRightGunEffect(items::ItemNode& node)
{
  const auto& src = *findAnimatedModelForType(TR1ItemId::LaraPistolsAnim);
  BOOST_ASSERT(gsl::narrow<size_t>(src.models.size()) == node.getNode()->getChildren().size());
  node.getNode()->getChild(10)->setDrawable(src.models[10].get());
}

void Engine::chainBlockEffect()
{
  if(m_effectTimer == 0_frame)
  {
    m_audioEngine->playSound(TR1SoundId::SecretFound, nullptr);
  }
  m_effectTimer += 1_frame;
  if(m_effectTimer == 55_frame)
  {
    m_audioEngine->playSound(TR1SoundId::LaraFallIntoWater, nullptr);
    m_activeEffect.reset();
  }
}

void Engine::flickerEffect()
{
  if(m_effectTimer == 90_frame || m_effectTimer == 92_frame || m_effectTimer == 105_frame || m_effectTimer == 107_frame)
  {
    swapAllRooms();
  }
  else if(m_effectTimer > 125_frame)
  {
    swapAllRooms();
    m_activeEffect.reset();
  }
  m_effectTimer += 1_frame;
}

void Engine::swapWithAlternate(loader::file::Room& orig, loader::file::Room& alternate)
{
  // find any blocks in the original room and un-patch the floor heights

  for(const auto& item : m_itemNodes | boost::adaptors::map_values)
  {
    if(item->m_state.position.room != &orig)
      continue;

    if(const auto tmp = std::dynamic_pointer_cast<items::Block>(item.get()))
    {
      loader::file::Room::patchHeightsForBlock(*tmp, core::SectorSize);
    }
    else if(const auto tmp = std::dynamic_pointer_cast<items::TallBlock>(item.get()))
    {
      loader::file::Room::patchHeightsForBlock(*tmp, core::SectorSize * 2);
    }
  }

  // now swap the rooms and patch the alternate room ids
  std::swap(orig, alternate);
  orig.alternateRoom = std::exchange(alternate.alternateRoom, -1);

  // patch heights in the new room, and swap item ownerships.
  // note that this is exactly the same code as above,
  // except for the heights.
  for(const auto& item : m_itemNodes | boost::adaptors::map_values)
  {
    if(item->m_state.position.room == &orig)
    {
      // although this seems contradictory, remember the nodes have been swapped above
      setParent(item->getNode(), orig.node);
    }
    else if(item->m_state.position.room == &alternate)
    {
      setParent(item->getNode(), alternate.node);
      continue;
    }
    else
    {
      continue;
    }

    if(const auto tmp = std::dynamic_pointer_cast<items::Block>(item.get()))
    {
      loader::file::Room::patchHeightsForBlock(*tmp, -core::SectorSize);
    }
    else if(const auto tmp = std::dynamic_pointer_cast<items::TallBlock>(item.get()))
    {
      loader::file::Room::patchHeightsForBlock(*tmp, -core::SectorSize * 2);
    }
  }

  for(const auto& item : m_dynamicItems)
  {
    if(item->m_state.position.room == &orig)
    {
      setParent(item->getNode(), orig.node);
    }
    else if(item->m_state.position.room == &alternate)
    {
      setParent(item->getNode(), alternate.node);
    }
  }
}

void Engine::animateUV()
{
  static constexpr auto UVAnimTime = 10;

  ++m_uvAnimTime;
  if(m_uvAnimTime >= UVAnimTime)
  {
    m_textureAnimator->updateCoordinates(m_level->m_textureTiles);
    m_uvAnimTime -= UVAnimTime;
  }
}

YAML::Node Engine::save() const
{
  YAML::Node result;

  YAML::Node inventory;

  const auto addInventory = [&](const TR1ItemId id) { inventory[toString(id)] = m_inventory.count(id); };

  addInventory(TR1ItemId::Pistols);
  addInventory(TR1ItemId::Magnums);
  addInventory(TR1ItemId::MagnumAmmo);
  addInventory(TR1ItemId::Uzis);
  addInventory(TR1ItemId::UziAmmo);
  addInventory(TR1ItemId::Shotgun);
  addInventory(TR1ItemId::ShotgunAmmo);
  addInventory(TR1ItemId::SmallMedipack);
  addInventory(TR1ItemId::LargeMedipack);
  addInventory(TR1ItemId::ScionPiece1);
  addInventory(TR1ItemId::Item141);
  addInventory(TR1ItemId::Item142);
  addInventory(TR1ItemId::Puzzle1);
  addInventory(TR1ItemId::Puzzle2);
  addInventory(TR1ItemId::Puzzle3);
  addInventory(TR1ItemId::Puzzle4);
  addInventory(TR1ItemId::Key1);
  addInventory(TR1ItemId::Key2);
  addInventory(TR1ItemId::Key3);
  addInventory(TR1ItemId::Key4);
  addInventory(TR1ItemId::LeadBar);

  result["inventory"] = inventory;

  YAML::Node flipStatus;
  for(const auto& state : mapFlipActivationStates)
  {
    flipStatus.push_back(state.save());
  }
  result["flipStatus"] = flipStatus;

  YAML::Node cameraFlags;
  for(const auto& camera : m_level->m_cameras)
  {
    cameraFlags.push_back(camera.flags);
  }
  result["cameraFlags"] = cameraFlags;

  if(m_activeEffect.is_initialized())
    result["flipEffect"] = *m_activeEffect;
  result["flipEffectTimer"] = m_effectTimer;

  for(const auto& item : m_itemNodes)
  {
    result["items"][item.first] = item.second->save();
  }

  result["camera"] = m_cameraController->save();

  result["secretsFound"] = m_secretsFoundBitmask.to_ulong();

  return result;
}

void Engine::load(const YAML::Node& node)
{
  m_inventory.clear();

  for(const auto& entry : node["inventory"])
    m_inventory.put(EnumUtil<TR1ItemId>::fromString(entry.first.as<std::string>()), entry.second.as<size_t>());

  for(size_t i = 0; i < mapFlipActivationStates.size(); ++i)
  {
    mapFlipActivationStates[i].load(node["flipStatus"][i]);
  }

  if(!node["cameraFlags"].IsSequence() || node["cameraFlags"].size() != m_level->m_cameras.size())
    BOOST_THROW_EXCEPTION(std::domain_error("Camera flag sequence is invalid"));

  for(size_t i = 0; i < m_level->m_cameras.size(); ++i)
    m_level->m_cameras[i].flags = node["cameraFlags"][i].as<uint16_t>();

  if(!node["flipEffect"].IsDefined())
    m_activeEffect.reset();
  else
    m_activeEffect = node["flipEffect"].as<size_t>();

  m_effectTimer = node["flipEffectTimer"].as<core::Frame>();

  for(const auto& item : m_itemNodes)
  {
    item.second->load(node["items"][item.first]);
  }

  getCameraController().load(node["camera"]);

  m_secretsFoundBitmask = node["secretsFound"].as<uint16_t>();
}

std::shared_ptr<items::PickupItem> Engine::createPickup(const core::TypeId type,
                                                        const gsl::not_null<const loader::file::Room*>& room,
                                                        const core::TRVec& position)
{
  loader::file::Item item;
  item.type = type;
  item.room = uint16_t(-1);
  item.position = position;
  item.rotation = 0_deg;
  item.darkness = 0;
  item.activationState = 0;

  const auto& spriteSequence = findSpriteSequenceForType(type);
  Expects(spriteSequence != nullptr && !spriteSequence->sprites.empty());
  const loader::file::Sprite& sprite = spriteSequence->sprites[0];

  auto node = std::make_shared<items::PickupItem>(this, "pickup", room, item, sprite, m_spriteMaterial);

  m_dynamicItems.emplace(node);
  addChild(room->node, node->getNode());

  return node;
}

void Engine::doGlobalEffect()
{
  if(m_activeEffect.is_initialized())
    runEffect(*m_activeEffect, nullptr);

  m_audioEngine->setUnderwater(getCameraController().getCurrentRoom()->isWaterRoom());
}

const loader::file::Animation& Engine::getAnimation(loader::file::AnimationId id) const
{
  return m_level->m_animations.at(static_cast<int>(id));
}

const std::vector<loader::file::CinematicFrame>& Engine::getCinematicFrames() const
{
  return m_level->m_cinematicFrames;
}

const std::vector<loader::file::Camera>& Engine::getCameras() const
{
  return m_level->m_cameras;
}

const std::vector<int16_t>& Engine::getAnimCommands() const
{
  return m_level->m_animCommands;
}

void Engine::update(const bool godMode)
{
  for(const auto& item : m_itemNodes | boost::adaptors::map_values)
  {
    if(item.get() == m_lara) // Lara is special and needs to be updated last
      continue;

    if(item->m_isActive)
      item->update();

    item->getNode()->setVisible(item->m_state.triggerState != items::TriggerState::Invisible);
  }

  for(const auto& item : m_dynamicItems)
  {
    if(item->m_isActive)
      item->update();

    item->getNode()->setVisible(item->m_state.triggerState != items::TriggerState::Invisible);
  }

  auto currentParticles = std::move(m_particles);
  for(const auto& particle : currentParticles)
  {
    if(particle->update(*this))
    {
      setParent(particle, particle->pos.room->node);
      particle->updateLight();
      m_particles.emplace_back(particle);
    }
    else
    {
      setParent(particle, nullptr);
    }
  }

  if(m_lara != nullptr)
  {
    if(godMode)
      m_lara->m_state.health = core::LaraHealth;
    m_lara->update();
  }

  applyScheduledDeletions();
  animateUV();
}

void Engine::drawDebugInfo(const gsl::not_null<std::shared_ptr<render::gl::Font>>& font, const float fps)
{
  drawText(font, font->getTarget()->getWidth() - 40, font->getTarget()->getHeight() - 20, std::to_string(fps));

  if(m_lara != nullptr)
  {
    // position/rotation
    drawText(font, 10, 40, m_lara->m_state.position.room->node->getId());

    drawText(font, 300, 20, std::to_string(std::lround(toDegrees(m_lara->m_state.rotation.Y))) + " deg");
    drawText(font, 300, 40, "x=" + m_lara->m_state.position.position.X.toString());
    drawText(font, 300, 60, "y=" + m_lara->m_state.position.position.Y.toString());
    drawText(font, 300, 80, "z=" + m_lara->m_state.position.position.Z.toString());

    // physics
    drawText(font, 300, 100, "grav " + m_lara->m_state.fallspeed.toString());
    drawText(font, 300, 120, "fwd  " + m_lara->m_state.speed.toString());

    // animation
    drawText(font, 10, 60, std::string("current/anim    ") + toString(m_lara->getCurrentAnimState()));
    drawText(font, 10, 100, std::string("target          ") + toString(m_lara->getGoalAnimState()));
    drawText(font, 10, 120, std::string("frame           ") + m_lara->m_state.frame_number.toString());
  }

  // triggers
  {
    int y = 180;
    for(const auto& item : m_itemNodes | boost::adaptors::map_values)
    {
      if(!item->m_isActive)
        continue;

      drawText(font, 10, y, item->getNode()->getId());
      switch(item->m_state.triggerState)
      {
      case items::TriggerState::Inactive: drawText(font, 180, y, "inactive"); break;
      case items::TriggerState::Active: drawText(font, 180, y, "active"); break;
      case items::TriggerState::Deactivated: drawText(font, 180, y, "deactivated"); break;
      case items::TriggerState::Invisible: drawText(font, 180, y, "invisible"); break;
      }
      drawText(font, 260, y, item->m_state.timer.toString());
      y += 20;
    }
    for(const auto& item : m_dynamicItems)
    {
      if(!item->m_isActive)
        continue;

      drawText(font, 10, y, item->getNode()->getId());
      switch(item->m_state.triggerState)
      {
      case items::TriggerState::Inactive: drawText(font, 180, y, "inactive"); break;
      case items::TriggerState::Active: drawText(font, 180, y, "active"); break;
      case items::TriggerState::Deactivated: drawText(font, 180, y, "deactivated"); break;
      case items::TriggerState::Invisible: drawText(font, 180, y, "invisible"); break;
      }
      drawText(font, 260, y, item->m_state.timer.toString());
      y += 20;
    }
  }

  if(m_lara == nullptr)
    return;

#ifndef NDEBUG
  // collision
  drawText(font,
           400,
           20,
           boost::lexical_cast<std::string>("AxisColl: ") + toString(m_lara->lastUsedCollisionInfo.collisionType));
  drawText(font,
           400,
           40,
           boost::lexical_cast<std::string>("Current floor:   ")
             + m_lara->lastUsedCollisionInfo.mid.floorSpace.y.toString());
  drawText(font,
           400,
           60,
           boost::lexical_cast<std::string>("Current ceiling: ")
             + m_lara->lastUsedCollisionInfo.mid.ceilingSpace.y.toString());
  drawText(font,
           400,
           80,
           boost::lexical_cast<std::string>("Front floor:     ")
             + m_lara->lastUsedCollisionInfo.front.floorSpace.y.toString());
  drawText(font,
           400,
           100,
           boost::lexical_cast<std::string>("Front ceiling:   ")
             + m_lara->lastUsedCollisionInfo.front.ceilingSpace.y.toString());
  drawText(font,
           400,
           120,
           boost::lexical_cast<std::string>("Front/L floor:   ")
             + m_lara->lastUsedCollisionInfo.frontLeft.floorSpace.y.toString());
  drawText(font,
           400,
           140,
           boost::lexical_cast<std::string>("Front/L ceiling: ")
             + m_lara->lastUsedCollisionInfo.frontLeft.ceilingSpace.y.toString());
  drawText(font,
           400,
           160,
           boost::lexical_cast<std::string>("Front/R floor:   ")
             + m_lara->lastUsedCollisionInfo.frontRight.floorSpace.y.toString());
  drawText(font,
           400,
           180,
           boost::lexical_cast<std::string>("Front/R ceiling: ")
             + m_lara->lastUsedCollisionInfo.frontRight.ceilingSpace.y.toString());
  drawText(font,
           400,
           200,
           boost::lexical_cast<std::string>("Need bottom:     ")
             + m_lara->lastUsedCollisionInfo.badPositiveDistance.toString());
  drawText(font,
           400,
           220,
           boost::lexical_cast<std::string>("Need top:        ")
             + m_lara->lastUsedCollisionInfo.badNegativeDistance.toString());
  drawText(font,
           400,
           240,
           boost::lexical_cast<std::string>("Need ceiling:    ")
             + m_lara->lastUsedCollisionInfo.badCeilingDistance.toString());
#endif

  // weapons
  drawText(font, 400, 280, std::string("L.aiming    ") + (m_lara->leftArm.aiming ? "true" : "false"));
  drawText(font,
           400,
           300,
           std::string("L.aim       X=") + std::to_string(toDegrees(m_lara->leftArm.aimRotation.X))
             + ", Y=" + std::to_string(toDegrees(m_lara->leftArm.aimRotation.Y)));
  drawText(font, 400, 320, std::string("R.aiming    ") + (m_lara->rightArm.aiming ? "true" : "false"));
  drawText(font,
           400,
           340,
           std::string("R.aim       X=") + std::to_string(toDegrees(m_lara->rightArm.aimRotation.X))
             + ", Y=" + std::to_string(toDegrees(m_lara->rightArm.aimRotation.Y)));
}

void Engine::drawText(const gsl::not_null<std::shared_ptr<render::gl::Font>>& font,
                      const int x,
                      const int y,
                      const std::string& txt,
                      const render::gl::SRGBA8& col)
{
  font->drawText(txt, x, y, col.channels[0], col.channels[1], col.channels[2], col.channels[3]);
}

Engine::Engine(bool fullscreen, const render::scene::Dimension2<int>& resolution)
    : m_renderer{std::make_unique<render::scene::Renderer>()}
    , m_window{std::make_unique<render::scene::Window>(fullscreen, resolution)}
    , splashImage{boost::filesystem::path{"splash.png"}}
    , abibasFont{std::make_shared<render::gl::Font>("abibas.ttf", 48)}
    , m_scriptEngine{createScriptEngine()}
    , m_inventory{*this}
{
  m_renderer->getScene()->setActiveCamera(
    std::make_shared<render::scene::Camera>(glm::radians(80.0f), m_window->getAspectRatio(), 10.0f, 20480.0f));

  scaleSplashImage();

  screenOverlay = std::make_shared<render::scene::ScreenOverlay>(m_window->getViewport());

  abibasFont->setTarget(screenOverlay->getImage());

  drawLoadingScreen("Booting");

  try
  {
    m_scriptEngine.safe_script_file("scripts/main.lua");
  }
  catch(sol::error& e)
  {
    BOOST_LOG_TRIVIAL(fatal) << "Failed to load main.lua: " << e.what();
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to load main.lua"));
  }

  const sol::optional<std::string> glidosPack = m_scriptEngine["getGlidosPack"]();

  std::unique_ptr<loader::trx::Glidos> glidos;
  if(glidosPack && boost::filesystem::is_directory(glidosPack.value()))
  {
    drawLoadingScreen("Loading Glidos texture pack");
    glidos = std::make_unique<loader::trx::Glidos>(glidosPack.value(),
                                                   [this](const std::string& s) { drawLoadingScreen(s); });
  }

  levelInfo = m_scriptEngine["getLevelInfo"]();
  const bool isVideo = !levelInfo.get<std::string>("video").empty();
  const auto cutsceneName = levelInfo.get<std::string>("cutscene");

  const auto baseName = cutsceneName.empty() ? levelInfo.get<std::string>("baseName") : cutsceneName;
  Expects(isVideo || !baseName.empty());
  const bool useAlternativeLara = levelInfo.get_or("useAlternativeLara", false);

  m_inputHandler = std::make_unique<hid::InputHandler>(m_window->getWindow());

  if(!isVideo)
  {
    std::map<TR1ItemId, size_t> initInv;

    if(sol::optional<sol::table> tbl = levelInfo["inventory"])
    {
      for(const auto& kv : *tbl)
        initInv[EnumUtil<TR1ItemId>::fromString(kv.first.as<std::string>())] += kv.second.as<size_t>();
    }

    if(sol::optional<sol::table> tbl = m_scriptEngine["cheats"]["inventory"])
    {
      for(const auto& kv : *tbl)
        initInv[EnumUtil<TR1ItemId>::fromString(kv.first.as<std::string>())] += kv.second.as<size_t>();
    }

    drawLoadingScreen("Preparing to load " + baseName);

    m_level = loader::file::level::Level::createLoader("data/tr1/data/" + baseName + ".PHD",
                                                       loader::file::level::Game::Unknown);

    drawLoadingScreen("Loading " + baseName);

    m_level->loadFileData();

    m_audioEngine
      = std::make_unique<AudioEngine>(*this, m_level->m_soundDetails, m_level->m_soundmap, m_level->m_sampleIndices);

    BOOST_LOG_TRIVIAL(info) << "Loading samples...";

    for(const auto offset : m_level->m_sampleIndices)
    {
      Expects(offset < m_level->m_samplesData.size());
      m_audioEngine->m_soundEngine.addWav(&m_level->m_samplesData[offset]);
    }

    for(size_t i = 0; i < m_level->m_textures.size(); ++i)
    {
      if(glidos != nullptr)
        drawLoadingScreen("Upgrading texture " + std::to_string(i + 1) + " of "
                          + std::to_string(m_level->m_textures.size()));
      else
        drawLoadingScreen("Loading texture " + std::to_string(i + 1) + " of "
                          + std::to_string(m_level->m_textures.size()));
      m_level->m_textures[i].toTexture(
        glidos.get(), std::function<void(const std::string&)>([this](const std::string& s) { drawLoadingScreen(s); }));
    }

    drawLoadingScreen("Preparing the game");
    loadSceneData(glidos != nullptr);

    if(useAlternativeLara)
    {
      useAlternativeLaraAppearance();
    }

    for(const auto& item : initInv)
      m_inventory.put(item.first, item.second);
  }
  else
  {
    m_audioEngine = std::make_unique<AudioEngine>(*this);
    m_cameraController = std::make_unique<CameraController>(this, m_renderer->getScene()->getActiveCamera(), true);
  }

  m_audioEngine->m_soundEngine.setListener(m_cameraController.get());

  if(!cutsceneName.empty() && !isVideo)
  {
    m_cameraController->setEyeRotation(0_deg, core::angleFromDegrees(levelInfo.get<float>("cameraRot")));
    auto pos = getCameraController().getTRPosition().position;
    if(auto x = levelInfo["cameraPosX"])
      pos.X = core::Length{x.get<core::Length::type>()};
    if(auto y = levelInfo["cameraPosY"])
      pos.Y = core::Length{y.get<core::Length::type>()};
    if(auto z = levelInfo["cameraPosZ"])
      pos.Z = core::Length{z.get<core::Length::type>()};

    getCameraController().setPosition(pos);

    if(levelInfo["flipRooms"].get_or(false))
      swapAllRooms();

    if(bool(levelInfo["gunSwap"]))
    {
      const auto& laraPistol = findAnimatedModelForType(TR1ItemId::LaraPistolsAnim);
      Expects(laraPistol != nullptr);
      for(const auto& item : m_itemNodes | boost::adaptors::map_values)
      {
        if(item->m_state.type != TR1ItemId::CutsceneActor1)
          continue;

        item->getNode()->getChild(1)->setDrawable(laraPistol->models[1].get());
        item->getNode()->getChild(4)->setDrawable(laraPistol->models[4].get());
      }
    }
  }

  if(m_level != nullptr)
  {
    struct Rect
    {
      Rect(const std::array<loader::file::UVCoordinates, 4>& cos)
      {
        for(const auto& co : cos)
        {
          x0 = std::min(x0, co.xpixel);
          y0 = std::min(y0, co.ypixel);
          x1 = std::max(x1, co.xpixel);
          y1 = std::max(y1, co.ypixel);
        }
      }

      constexpr bool operator==(const Rect& rhs) const noexcept
      {
        return x0 == rhs.x0 && y0 == rhs.y0 && x1 == rhs.x1 && y1 == rhs.y1;
      }

      constexpr bool operator<(const Rect& rhs) const noexcept
      {
        if(x0 != rhs.x0)
          return x0 < rhs.x0;
        if(y0 != rhs.y0)
          return y0 < rhs.y0;
        if(x1 != rhs.x1)
          return x1 < rhs.x1;
        return y1 < rhs.y1;
      }

      uint8_t x0 = std::numeric_limits<uint8_t>::max(), y0 = std::numeric_limits<uint8_t>::max();
      uint8_t x1 = std::numeric_limits<uint8_t>::min(), y1 = std::numeric_limits<uint8_t>::min();
    };

    std::map<int, std::set<Rect>> tilesByTexture;
    BOOST_LOG_TRIVIAL(debug) << m_level->m_textureTiles.size() << " total texture tiles";
    for(const auto& tile : m_level->m_textureTiles)
    {
      tilesByTexture[tile.textureKey.tileAndFlag & loader::file::TextureIndexMask].emplace(tile.uvCoordinates);
    }

    size_t totalTiles = 0;
    for(const auto& textureAndTiles : tilesByTexture)
      totalTiles += textureAndTiles.second.size();
    BOOST_LOG_TRIVIAL(debug) << totalTiles << " unique texture tiles";

    const auto cacheBaseDir = glidos != nullptr ? glidos->getBaseDir() : boost::filesystem::path("data/tr1/data");
    auto cache = loader::file::TextureCache{cacheBaseDir / "_edisonengine"};

    size_t processedTiles = 0;
    for(const auto& textureAndTiles : tilesByTexture)
    {
      drawLoadingScreen("Mipmapping (" + std::to_string(processedTiles * 100 / totalTiles) + "%)");
      processedTiles += textureAndTiles.second.size();

      const loader::file::DWordTexture& texture = m_level->m_textures.at(textureAndTiles.first);
      Expects(texture.image->getWidth() == texture.image->getHeight());

      BOOST_LOG_TRIVIAL(debug) << "Mipmapping texture " << textureAndTiles.first;

      const util::CImgWrapper src{reinterpret_cast<uint8_t*>(texture.image->getRawData()),
                                  texture.image->getWidth(),
                                  texture.image->getHeight(),
                                  true};
      int mipmapLevel = 1;
      for(auto dstSize = texture.image->getWidth() / 2; dstSize > 1; dstSize /= 2, ++mipmapLevel)
      {
        BOOST_LOG_TRIVIAL(debug) << "Mipmap level " << mipmapLevel << " (size " << dstSize / 2 << ", "
                                 << textureAndTiles.second.size() << " tiles)";
        if(cache.exists(texture.md5, mipmapLevel))
        {
          auto dst = cache.loadPng(texture.md5, mipmapLevel);
          dst.interleave();
          texture.texture->image(reinterpret_cast<const render::gl::SRGBA8*>(dst.data()), mipmapLevel);
        }
        else
        {
          util::CImgWrapper dst{dstSize, dstSize};
          for(const Rect& r : textureAndTiles.second)
          {
            // (scaled) source coordinates
            const auto x0 = r.x0 * texture.image->getWidth() / 256;
            const auto y0 = r.y0 * texture.image->getHeight() / 256;
            const auto x1 = (r.x1 + 1) * texture.image->getWidth() / 256;
            const auto y1 = (r.y1 + 1) * texture.image->getHeight() / 256;
            BOOST_ASSERT(x0 < x1);
            BOOST_ASSERT(y0 < y1);
            util::CImgWrapper tmp = src.cropped(x0, y0, x1 - 1, y1 - 1);
            tmp.resizePow2Mipmap(mipmapLevel);
            // +1 for doing mathematically correct rounding
            dst.replace(
              (x0 * dstSize + 1) / texture.image->getWidth(), (y0 * dstSize + 1) / texture.image->getHeight(), tmp);
          }

          cache.savePng(texture.md5, mipmapLevel, dst);
          dst.interleave();
          texture.texture->image(reinterpret_cast<const render::gl::SRGBA8*>(dst.data()), mipmapLevel);
        }
      }
    }
  }

  m_renderPipeline = std::make_shared<render::RenderPipeline>(m_window->getViewport());
}

void Engine::run()
{
  render::scene::RenderContext context{};
  render::scene::Node dummyNode{""};
  context.setCurrentNode(&dummyNode);

  render::gl::Framebuffer::unbindAll();

  screenOverlay->init(m_window->getViewport());

  if(const sol::optional<std::string> video = levelInfo["video"])
  {
    video::play(
      "data/tr1/fmv/" + video.value(), m_audioEngine->m_soundEngine.getDevice(), screenOverlay->getImage(), [&]() {
        if(m_window->updateWindowSize())
        {
          m_renderer->getScene()->getActiveCamera()->setAspectRatio(m_window->getAspectRatio());
          screenOverlay->init(m_window->getViewport());
        }

        screenOverlay->render(context);
        m_window->swapBuffers();
        m_inputHandler->update();
        return !m_window->windowShouldClose();
      });
    return;
  }

  static const auto frameDuration
    = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::seconds(1)) / core::FrameRate.get();

  std::string language = std::use_facet<boost::locale::info>(boost::locale::generator()("")).language();
  BOOST_LOG_TRIVIAL(info) << "Detected user's language is " << language;
  if(const sol::optional<std::string> overrideLanguage = getScriptEngine()["language_override"])
  {
    language = overrideLanguage.get();
    BOOST_LOG_TRIVIAL(info) << "Language override is " << language;
  }

  std::string levelName;
  if(const auto levelNames = levelInfo["name"])
  {
    levelName = levelNames[language];
    if(levelName.empty())
    {
      BOOST_LOG_TRIVIAL(warning) << "Missing level name, falling back to language en";
      levelName = levelNames["en"];
    }
  }

  bool showDebugInfo = false;

  auto font = std::make_shared<render::gl::Font>("DroidSansMono.ttf", 12);
  font->setTarget(screenOverlay->getImage());

  auto trFont = ui::CachedFont(*m_level->m_spriteSequences.at(TR1ItemId::FontGraphics));

  auto nextFrameTime = std::chrono::high_resolution_clock::now() + frameDuration;

  const bool isCutscene = !levelInfo.get<std::string>("cutscene").empty();

  if(const sol::optional<TR1TrackId> trackToPlay = levelInfo["track"])
  {
    m_audioEngine->playStopCdTrack(trackToPlay.value(), false);
  }

  while(!m_window->windowShouldClose())
  {
    screenOverlay->getImage()->fill({0, 0, 0, 0});

    if(!levelName.empty())
    {
      ui::Label tmp{0, -50, levelName};
      tmp.alignX = ui::Label::Alignment::Center;
      tmp.alignY = ui::Label::Alignment::Bottom;
      tmp.outline = true;
      tmp.addBackground(0, 0, 0, 0);
      tmp.draw(trFont, *screenOverlay->getImage(), *m_level->m_palette);
    }

    m_audioEngine->m_soundEngine.update();
    m_inputHandler->update();

    if(m_inputHandler->getInputState().debug.justPressed())
    {
      showDebugInfo = !showDebugInfo;
    }

    {
      // frame rate throttling
      // TODO this assumes that the frame rate capacity (the processing power so to speak)
      // is faster than 30 FPS.
      std::this_thread::sleep_until(nextFrameTime);
      nextFrameTime += frameDuration;
    }

    update(bool(m_scriptEngine["cheats"]["godMode"]));

    if(m_window->updateWindowSize())
    {
      m_renderer->getScene()->getActiveCamera()->setAspectRatio(m_window->getAspectRatio());
      m_renderPipeline->resize(m_window->getViewport());
      screenOverlay->init(m_window->getViewport());
      font->setTarget(screenOverlay->getImage());
    }

    std::unordered_set<const loader::file::Portal*> waterEntryPortals;
    if(!isCutscene)
    {
      waterEntryPortals = getCameraController().update();
    }
    else
    {
      if(++getCameraController().m_cinematicFrame >= m_level->m_cinematicFrames.size())
        break;

      waterEntryPortals = m_cameraController->updateCinematic(
        m_level->m_cinematicFrames[getCameraController().m_cinematicFrame], false);
    }
    doGlobalEffect();

    if(m_lara != nullptr)
      drawBars(screenOverlay->getImage());

    m_renderPipeline->update(*getCameraController().getCamera(), m_renderer->getGameTime());

    {
      render::gl::DebugGroup dbg{"geometry-pass"};
      m_renderPipeline->bindGeometryFrameBuffer();
      m_renderer->render();
    }

    render::scene::RenderContext context{};
    render::scene::Node dummyNode{""};
    context.setCurrentNode(&dummyNode);

    {
      render::gl::DebugGroup dbg{"portal-depth-pass"};
      m_renderPipeline->bindPortalFrameBuffer();
      const auto now = std::chrono::time_point_cast<std::chrono::milliseconds>(m_renderer->getGameTime());
      m_portalMaterial->getUniform("u_time")->set(gsl::narrow_cast<float>(now.time_since_epoch().count()));
      for(const auto& portal : waterEntryPortals)
      {
        portal->mesh->render(context);
      }
    }

    m_renderPipeline->finalPass(getCameraController().getCurrentRoom()->isWaterRoom());

    if(showDebugInfo)
    {
      drawDebugInfo(font, m_renderer->getFrameRate());

      const auto drawItemName
        = [this, &font](const std::shared_ptr<items::ItemNode>& node, const render::gl::SRGBA8& color) {
            const auto vertex = glm::vec3{m_renderer->getScene()->getActiveCamera()->getViewMatrix()
                                          * glm::vec4(node->getNode()->getTranslationWorld(), 1)};

            if(vertex.z > -m_renderer->getScene()->getActiveCamera()->getNearPlane())
            {
              return;
            }
            else if(vertex.z < -m_renderer->getScene()->getActiveCamera()->getFarPlane())
            {
              return;
            }

            glm::vec4 projVertex{vertex, 1};
            projVertex = m_renderer->getScene()->getActiveCamera()->getProjectionMatrix() * projVertex;
            projVertex /= projVertex.w;

            if(std::abs(projVertex.x) > 1 || std::abs(projVertex.y) > 1)
              return;

            projVertex.x = (projVertex.x / 2 + 0.5f) * m_window->getViewport().width;
            projVertex.y = (1 - (projVertex.y / 2 + 0.5f)) * m_window->getViewport().height;

            font->drawText(node->getNode()->getId().c_str(), projVertex.x, projVertex.y, color);
          };

      for(const auto& node : m_itemNodes | boost::adaptors::map_values)
      {
        drawItemName(node, render::gl::SRGBA8{255});
      }
      for(const auto& node : m_dynamicItems)
      {
        drawItemName(node, render::gl::SRGBA8{0, 255, 0, 255});
      }
    }

    {
      render::gl::DebugGroup dbg{"screen-overlay-pass"};
      screenOverlay->render(context);
    }
    m_window->swapBuffers();

    if(m_inputHandler->getInputState().save.justPressed())
    {
      scaleSplashImage();
      abibasFont->setTarget(screenOverlay->getImage());
      drawLoadingScreen("Saving...");

      BOOST_LOG_TRIVIAL(info) << "Save";
      std::ofstream file{"quicksave.yaml", std::ios::out | std::ios::trunc};
      Expects(file.is_open());
      file << save();

      nextFrameTime = std::chrono::high_resolution_clock::now() + frameDuration;
    }
    else if(m_inputHandler->getInputState().load.justPressed())
    {
      scaleSplashImage();
      abibasFont->setTarget(screenOverlay->getImage());
      drawLoadingScreen("Loading...");

      BOOST_LOG_TRIVIAL(info) << "Load";
      load(YAML::LoadFile("quicksave.yaml"));

      nextFrameTime = std::chrono::high_resolution_clock::now() + frameDuration;
    }
  }
}

void Engine::scaleSplashImage()
{
  // scale splash image so that its aspect ratio is preserved, but the boundaries match
  const float splashScale = std::max(gsl::narrow<float>(m_window->getViewport().width) / splashImage.width(),
                                     gsl::narrow<float>(m_window->getViewport().height) / splashImage.height());

  splashImageScaled = splashImage;
  splashImageScaled.resize(splashImageScaled.width() * splashScale, splashImageScaled.height() * splashScale);

  // crop to boundaries
  const auto centerX = splashImageScaled.width() / 2;
  const auto centerY = splashImageScaled.height() / 2;
  splashImageScaled.crop(
    gsl::narrow<int>(centerX - m_window->getViewport().width / 2),
    gsl::narrow<int>(centerY - m_window->getViewport().height / 2),
    gsl::narrow<int>(centerX - m_window->getViewport().width / 2 + m_window->getViewport().width - 1),
    gsl::narrow<int>(centerY - m_window->getViewport().height / 2 + m_window->getViewport().height - 1));

  Expects(splashImageScaled.width() == m_window->getViewport().width);
  Expects(splashImageScaled.height() == m_window->getViewport().height);

  splashImageScaled.interleave();
}

void Engine::drawLoadingScreen(const std::string& state)
{
  glfwPollEvents();
  if(m_window->updateWindowSize())
  {
    m_renderer->getScene()->getActiveCamera()->setAspectRatio(m_window->getAspectRatio());
    screenOverlay->init(m_window->getViewport());
    abibasFont->setTarget(screenOverlay->getImage());

    scaleSplashImage();
  }
  screenOverlay->getImage()->assign(reinterpret_cast<const render::gl::SRGBA8*>(splashImageScaled.data()),
                                    m_window->getViewport().width * m_window->getViewport().height);
  abibasFont->drawText(state, 40, gsl::narrow<int>(m_window->getViewport().height - 100), 255, 255, 255, 192);

  render::gl::Framebuffer::unbindAll();

  m_renderer->clear(::gl::ClearBufferMask::ColorBufferBit | ::gl::ClearBufferMask::DepthBufferBit, {0, 0, 0, 0}, 1);
  render::scene::RenderContext context{};
  render::scene::Node dummyNode{""};
  context.setCurrentNode(&dummyNode);
  screenOverlay->render(context);
  m_window->swapBuffers();
}

const std::vector<int16_t>& Engine::getPoseFrames() const
{
  return m_level->m_poseFrames;
}

const std::vector<loader::file::Animation>& Engine::getAnimations() const
{
  return m_level->m_animations;
}

const std::vector<uint16_t>& Engine::getOverlaps() const
{
  return m_level->m_overlaps;
}

const std::unique_ptr<loader::file::SkeletalModelType>& Engine::findAnimatedModelForType(core::TypeId type) const
{
  return m_level->findAnimatedModelForType(type);
}

void Engine::handleCommandSequence(const floordata::FloorDataValue* floorData, const bool fromHeavy)
{
  if(floorData == nullptr)
    return;

  floordata::FloorDataChunk chunkHeader{*floorData};

  if(chunkHeader.type == floordata::FloorDataChunkType::Death)
  {
    if(!fromHeavy)
    {
      if(m_lara->m_state.position.position.Y == m_lara->m_state.floor)
      {
        m_lara->burnIfAlive();
      }
    }

    if(chunkHeader.isLast)
      return;

    ++floorData;
  }

  chunkHeader = floordata::FloorDataChunk{*floorData++};
  BOOST_ASSERT(chunkHeader.type == floordata::FloorDataChunkType::CommandSequence);
  const floordata::ActivationState activationRequest{*floorData++};

  m_cameraController->handleCommandSequence(floorData);

  bool conditionFulfilled, switchIsOn = false;
  if(fromHeavy)
  {
    conditionFulfilled = chunkHeader.sequenceCondition == floordata::SequenceCondition::ItemIsHere;
  }
  else
  {
    switch(chunkHeader.sequenceCondition)
    {
    case floordata::SequenceCondition::LaraIsHere: conditionFulfilled = true; break;
    case floordata::SequenceCondition::LaraOnGround:
    case floordata::SequenceCondition::LaraOnGroundInverted:
    {
      conditionFulfilled = m_lara->m_state.position.position.Y == m_lara->m_state.floor;
    }
    break;
    case floordata::SequenceCondition::ItemActivated:
    {
      auto swtch = m_itemNodes.at(floordata::Command{*floorData++}.parameter);
      if(!swtch->triggerSwitch(activationRequest.getTimeout()))
        return;

      switchIsOn = (swtch->m_state.current_anim_state == 1_as);
      conditionFulfilled = true;
    }
    break;
    case floordata::SequenceCondition::KeyUsed:
    {
      auto key = m_itemNodes.at(floordata::Command{*floorData++}.parameter);
      if(key->triggerKey())
        conditionFulfilled = true;
      else
        return;
    }
    break;
    case floordata::SequenceCondition::ItemPickedUp:
      if(m_itemNodes.at(floordata::Command{*floorData++}.parameter)->triggerPickUp())
        conditionFulfilled = true;
      else
        return;
      break;
    case floordata::SequenceCondition::LaraInCombatMode:
      conditionFulfilled = m_lara->getHandStatus() == HandStatus::Combat;
      break;
    case floordata::SequenceCondition::ItemIsHere:
    case floordata::SequenceCondition::Dummy: return;
    default: conditionFulfilled = true; break;
    }
  }

  if(!conditionFulfilled)
    return;

  bool swapRooms = false;
  boost::optional<size_t> flipEffect;
  while(true)
  {
    const floordata::Command command{*floorData++};
    switch(command.opcode)
    {
    case floordata::CommandOpcode::Activate:
    {
      auto& item = *m_itemNodes.at(command.parameter);
      if(item.m_state.activationState.isOneshot())
        break;

      item.m_state.timer = activationRequest.getTimeout();

      if(chunkHeader.sequenceCondition == floordata::SequenceCondition::ItemActivated)
        item.m_state.activationState ^= activationRequest.getActivationSet();
      else if(chunkHeader.sequenceCondition == floordata::SequenceCondition::LaraOnGroundInverted)
        item.m_state.activationState &= ~activationRequest.getActivationSet();
      else
        item.m_state.activationState |= activationRequest.getActivationSet();

      if(!item.m_state.activationState.isFullyActivated())
        break;

      if(activationRequest.isOneshot())
        item.m_state.activationState.setOneshot(true);

      if(item.m_isActive)
        break;

      if(item.m_state.triggerState == items::TriggerState::Inactive
         || item.m_state.triggerState == items::TriggerState::Invisible
         || dynamic_cast<items::AIAgent*>(&item) == nullptr)
      {
        item.m_state.triggerState = items::TriggerState::Active;
        item.m_state.touch_bits = 0;
        item.activate();
        break;
      }
    }
    break;
    case floordata::CommandOpcode::SwitchCamera:
    {
      const floordata::CameraParameters camParams{*floorData++};
      m_cameraController->setCamOverride(camParams,
                                         command.parameter,
                                         chunkHeader.sequenceCondition,
                                         fromHeavy,
                                         activationRequest.getTimeout(),
                                         switchIsOn);
      command.isLast = camParams.isLast;
    }
    break;
    case floordata::CommandOpcode::LookAt: m_cameraController->setLookAtItem(getItem(command.parameter)); break;
    case floordata::CommandOpcode::UnderwaterCurrent:
    {
      const auto& sink = m_level->m_cameras.at(command.parameter);
      if(m_lara->m_underwaterRoute.required_box != &m_level->m_boxes[sink.box_index])
      {
        m_lara->m_underwaterRoute.required_box = &m_level->m_boxes[sink.box_index];
        m_lara->m_underwaterRoute.target = sink.position;
      }
      m_lara->m_underwaterCurrentStrength = 6_len * static_cast<core::Length::type>(sink.underwaterCurrentStrength);
    }
    break;
    case floordata::CommandOpcode::FlipMap:
      BOOST_ASSERT(command.parameter < mapFlipActivationStates.size());
      if(!mapFlipActivationStates[command.parameter].isOneshot())
      {
        if(chunkHeader.sequenceCondition == floordata::SequenceCondition::ItemActivated)
        {
          mapFlipActivationStates[command.parameter] ^= activationRequest.getActivationSet();
        }
        else
        {
          mapFlipActivationStates[command.parameter] |= activationRequest.getActivationSet();
        }

        if(mapFlipActivationStates[command.parameter].isFullyActivated())
        {
          if(activationRequest.isOneshot())
            mapFlipActivationStates[command.parameter].setOneshot(true);

          if(!m_roomsAreSwapped)
            swapRooms = true;
        }
        else if(m_roomsAreSwapped)
        {
          swapRooms = true;
        }
      }
      break;
    case floordata::CommandOpcode::FlipOn:
      BOOST_ASSERT(command.parameter < mapFlipActivationStates.size());
      if(!m_roomsAreSwapped && mapFlipActivationStates[command.parameter].isFullyActivated())
        swapRooms = true;
      break;
    case floordata::CommandOpcode::FlipOff:
      BOOST_ASSERT(command.parameter < mapFlipActivationStates.size());
      if(m_roomsAreSwapped && mapFlipActivationStates[command.parameter].isFullyActivated())
        swapRooms = true;
      break;
    case floordata::CommandOpcode::FlipEffect: flipEffect = command.parameter; break;
    case floordata::CommandOpcode::EndLevel: finishLevel(); break;
    case floordata::CommandOpcode::PlayTrack:
      m_audioEngine->triggerCdTrack(
        static_cast<TR1TrackId>(command.parameter), activationRequest, chunkHeader.sequenceCondition);
      break;
    case floordata::CommandOpcode::Secret:
      BOOST_ASSERT(command.parameter < 16);
      if(!m_secretsFoundBitmask.test(command.parameter))
      {
        m_secretsFoundBitmask.set(command.parameter);
        m_audioEngine->playStopCdTrack(TR1TrackId::Secret, false);
      }
      break;
    default: break;
    }

    if(command.isLast)
      break;
  }

  if(!swapRooms)
    return;

  swapAllRooms();

  if(flipEffect.is_initialized())
    setGlobalEffect(*flipEffect);
}

Engine::~Engine() = default;
} // namespace engine
