#include "engine.h"

#include "audio/tracktype.h"
#include "engine/ai/ai.h"
#include "floordata/floordata.h"
#include "hid/inputhandler.h"
#include "loader/file/level/level.h"
#include "loader/file/texturecache.h"
#include "loader/trx/trx.h"
#include "objects/aiagent.h"
#include "objects/block.h"
#include "objects/laraobject.h"
#include "objects/modelobject.h"
#include "objects/objectfactory.h"
#include "objects/pickupobject.h"
#include "objects/tallblock.h"
#include "render/renderpipeline.h"
#include "render/scene/csm.h"
#include "render/scene/rendervisitor.h"
#include "render/scene/scene.h"
#include "render/textureanimator.h"
#include "script/reflection.h"
#include "serialization/array.h"
#include "serialization/bitset.h"
#include "serialization/map.h"
#include "serialization/not_null.h"
#include "serialization/objectreference.h"
#include "serialization/optional.h"
#include "serialization/quantity.h"
#include "serialization/vector.h"
#include "tracks_tr1.h"
#include "ui/debug/debugview.h"
#include "ui/label.h"
#include "video/player.h"

#include <boost/locale/generator.hpp>
#include <boost/locale/info.hpp>
#include <boost/range/adaptor/map.hpp>
#include <filesystem>
#include <gl/font.h>
#include <glm/gtx/norm.hpp>
#include <locale>

constexpr int32_t CSMResolution = 1024;

namespace engine
{
namespace
{
constexpr int StatusLineFontSize = 40;
constexpr int DebugTextFontSize = 12;

sol::state createScriptEngine(const std::filesystem::path& rootPath)
{
  sol::state engine;
  engine.open_libraries(sol::lib::base, sol::lib::math, sol::lib::package);
  engine["package"]["path"] = (rootPath / "scripts" / "?.lua").string();
  engine["package"]["cpath"] = "";

  core::TRVec::registerUserType(engine);
  ai::CreatureInfo::registerUserType(engine);
  script::ObjectInfo::registerUserType(engine);
  script::TrackInfo::registerUserType(engine);

  engine.new_enum("ActivationState",
                  "INACTIVE",
                  objects::TriggerState::Inactive,
                  "ACTIVE",
                  objects::TriggerState::Active,
                  "DEACTIVATED",
                  objects::TriggerState::Deactivated,
                  "INVISIBLE",
                  objects::TriggerState::Invisible);

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

std::vector<loader::file::Room>& Engine::getRooms()
{
  return m_level->m_rooms;
}

const std::vector<loader::file::Box>& Engine::getBoxes() const
{
  return m_level->m_boxes;
}

gsl::not_null<std::shared_ptr<render::scene::Material>> Engine::createMaterial(bool water)
{
  return m_materialManager->createMaterial(m_allTextures, water);
}

std::shared_ptr<objects::LaraObject> Engine::createObjects()
{
  std::shared_ptr<objects::LaraObject> lara = nullptr;
  ObjectId id = -1;
  for(loader::file::Item& item : m_level->m_items)
  {
    ++id;

    auto object = objects::createObject(*this, item);
    if(item.type == TR1ItemId::Lara)
    {
      lara = std::dynamic_pointer_cast<objects::LaraObject>(object);
      Expects(lara != nullptr);
    }

    if(object != nullptr)
    {
      m_objects.emplace(std::make_pair(id, object));
    }
  }

  return lara;
}

void Engine::loadSceneData()
{
  for(auto& sprite : m_level->m_sprites)
  {
    sprite.image = m_level->m_textures[sprite.texture_id.get()].image;
  }

  m_textureAnimator = std::make_shared<render::TextureAnimator>(
    m_level->m_animatedTextures, m_level->m_textureTiles, m_level->m_textures);

  const int textureSize = m_level->m_textures[0].image->getWidth();
  const int textureLevels = std::log2(textureSize) + 1;

  m_allTextures = std::make_shared<gl::Texture2DArray<gl::SRGBA8>>(
    glm::ivec3{textureSize, textureSize, gsl::narrow<int>(m_level->m_textures.size())}, textureLevels, "all-textures");
  m_allTextures->set(gl::api::TextureMinFilter::NearestMipmapLinear);
  if(textureSize == 256)
  {
    m_allTextures->set(gl::api::TextureMagFilter::Nearest);
  }
  else
  {
    m_allTextures->set(gl::api::TextureMagFilter::Linear);
  }

  const auto materialFull = createMaterial(false);

  for(auto& mesh : m_level->m_meshes)
  {
    m_renderMeshes.emplace_back(mesh.toRenderMesh(m_level->m_textureTiles,
                                                  materialFull,
                                                  m_materialManager->getDepthOnly(),
                                                  m_materialManager->getCSMDepthOnly(),
                                                  *m_level->m_palette));
  }

  for(auto idx : m_level->m_meshIndices)
  {
    Expects(idx < m_renderMeshes.size());
    m_renderMeshesDirect.emplace_back(m_renderMeshes[idx]);
    m_meshesDirect.emplace_back(&m_level->m_meshes[idx]);
  }

  for(const std::unique_ptr<loader::file::SkeletalModelType>& model :
      m_level->m_animatedModels | boost::adaptors::map_values)
  {
    if(model->nMeshes > 0)
    {
      model->renderMeshes = make_span(&model->mesh_base_index.checkedFrom(m_renderMeshesDirect), model->nMeshes);
      model->meshes = make_span(&model->mesh_base_index.checkedFrom(m_meshesDirect), model->nMeshes);
    }
  }

  auto waterMaterialFull = createMaterial(true);

  for(size_t i = 0; i < m_level->m_rooms.size(); ++i)
  {
    m_level->m_rooms[i].createSceneNode(i,
                                        *m_level,
                                        materialFull,
                                        waterMaterialFull,
                                        m_materialManager->getDepthOnly(),
                                        m_renderMeshes,
                                        *m_textureAnimator,
                                        m_materialManager->getSprite(),
                                        m_materialManager->getPortal());
    m_renderer->getScene()->addNode(m_level->m_rooms[i].node);
  }

  m_lara = createObjects();
  if(m_lara == nullptr)
  {
    m_cameraController = std::make_unique<CameraController>(this, m_renderer->getCamera(), true);

    for(const auto& item : m_level->m_items)
    {
      if(item.type == TR1ItemId::CutsceneActor1)
      {
        m_cameraController->setPosition(item.position);
      }
    }
  }
  else
  {
    m_cameraController = std::make_unique<CameraController>(this, m_renderer->getCamera());
  }

  m_positionalEmitters.reserve(m_level->m_soundSources.size());
  for(loader::file::SoundSource& src : m_level->m_soundSources)
  {
    m_positionalEmitters.emplace_back(src.position.toRenderSystem(), &m_audioEngine->getSoundEngine());
    auto handle = m_audioEngine->playSound(src.sound_id, &m_positionalEmitters.back());
    Expects(handle != nullptr);
    handle->setLooping(true);
  }
}

std::shared_ptr<objects::Object> Engine::getObject(uint16_t id) const
{
  const auto it = m_objects.find(id);
  if(it == m_objects.end())
    return nullptr;

  return it->second.get();
}

void Engine::drawBars(const gsl::not_null<std::shared_ptr<gl::Image<gl::SRGBA8>>>& image)
{
  if(m_lara->isInWater())
  {
    const auto x0 = m_window->getViewport().x - 110;

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

  if(m_lara->getHandStatus() == objects::HandStatus::Combat || m_lara->m_state.health <= 0_hp)
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

// ReSharper disable once CppMemberFunctionMayBeConst
void Engine::useAlternativeLaraAppearance(const bool withHead)
{
  const auto& base = *findAnimatedModelForType(TR1ItemId::Lara);
  BOOST_ASSERT(gsl::narrow<size_t>(base.renderMeshes.size()) == m_lara->getNode()->getChildren().size());

  const auto& alternate = *findAnimatedModelForType(TR1ItemId::AlternativeLara);
  BOOST_ASSERT(gsl::narrow<size_t>(alternate.renderMeshes.size()) == m_lara->getNode()->getChildren().size());

  for(size_t i = 0; i < m_lara->getNode()->getChildren().size(); ++i)
    m_lara->getNode()->getChild(i)->setRenderable(alternate.renderMeshes[i].get());

  if(!withHead)
    m_lara->getNode()->getChild(14)->setRenderable(base.renderMeshes[14].get());
}

void Engine::dinoStompEffect(objects::Object& object)
{
  const auto d = object.m_state.position.position.toRenderSystem() - m_cameraController->getPosition();
  const auto absD = glm::abs(d);

  static constexpr auto MaxD = (16 * core::SectorSize).get_as<float>();
  if(absD.x > MaxD || absD.y > MaxD || absD.z > MaxD)
    return;

  const auto x = (100_len).retype_as<float>() * (1 - length2(d) / util::square(MaxD));
  m_cameraController->setBounce(x.retype_as<core::Length>());
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void Engine::turn180Effect(objects::Object& object)
{
  object.m_state.rotation.Y += 180_deg;
}

void Engine::laraNormalEffect()
{
  Expects(m_lara != nullptr);
  m_lara->setCurrentAnimState(loader::file::LaraStateId::Stop);
  m_lara->setRequiredAnimState(loader::file::LaraStateId::Unknown12);
  m_lara->getSkeleton()->anim = &m_level->m_animations[static_cast<int>(loader::file::AnimationId::STAY_SOLID)];
  m_lara->getSkeleton()->frame_number = 185_frame;
  m_cameraController->setMode(CameraMode::Chase);
  m_renderer->getCamera()->setFieldOfView(glm::radians(80.0f));
}

void Engine::laraBubblesEffect(objects::Object& object)
{
  const auto modelNode = dynamic_cast<objects::ModelObject*>(&object);
  if(modelNode == nullptr)
    return;

  auto bubbleCount = util::rand15(12);
  if(bubbleCount == 0)
    return;

  object.playSoundEffect(TR1SoundId::LaraUnderwaterGurgle);

  const auto boneSpheres = modelNode->getSkeleton()->getBoneCollisionSpheres(
    object.m_state, *modelNode->getSkeleton()->getInterpolationInfo().getNearestFrame(), nullptr);

  const auto position
    = core::TRVec{glm::vec3{translate(boneSpheres.at(14).m, core::TRVec{0_len, 0_len, 50_len}.toRenderSystem())[3]}};

  while(bubbleCount-- > 0)
  {
    auto particle
      = std::make_shared<BubbleParticle>(core::RoomBoundPosition{object.m_state.position.room, position}, *this);
    setParent(particle, object.m_state.position.room->node);
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
    m_cameraController->setBounce(-250_len);
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
    pos.Y = 100_len * mul / 1_frame + m_cameraController->getCenter().position.Y;
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
    auto pos = m_cameraController->getCenter().position;
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
  m_cameraController->setBounce(-75_len);
  m_activeEffect.reset();
}

// ReSharper disable once CppMemberFunctionMayBeConst
void Engine::laraHandsFreeEffect()
{
  m_lara->setHandStatus(objects::HandStatus::None);
}

void Engine::flipMapEffect()
{
  swapAllRooms();
}

// ReSharper disable once CppMemberFunctionMayBeConst
void Engine::unholsterRightGunEffect(objects::Object& object)
{
  const auto& src = *findAnimatedModelForType(TR1ItemId::LaraPistolsAnim);
  BOOST_ASSERT(gsl::narrow<size_t>(src.renderMeshes.size()) == object.getNode()->getChildren().size());
  object.getNode()->getChild(10)->setRenderable(src.renderMeshes[10].get());
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

  for(const auto& object : m_objects | boost::adaptors::map_values)
  {
    if(object->m_state.position.room != &orig)
      continue;

    if(const auto tmp = std::dynamic_pointer_cast<objects::Block>(object.get()))
    {
      loader::file::Room::patchHeightsForBlock(*tmp, core::SectorSize);
    }
    else if(const auto tmp = std::dynamic_pointer_cast<objects::TallBlock>(object.get()))
    {
      loader::file::Room::patchHeightsForBlock(*tmp, core::SectorSize * 2);
    }
  }

  // now swap the rooms and patch the alternate room ids
  std::swap(orig, alternate);
  orig.alternateRoom = std::exchange(alternate.alternateRoom, -1);

  // patch heights in the new room, and swap object ownerships.
  // note that this is exactly the same code as above,
  // except for the heights.
  for(const auto& object : m_objects | boost::adaptors::map_values)
  {
    if(object->m_state.position.room == &orig)
    {
      // although this seems contradictory, remember the nodes have been swapped above
      setParent(object->getNode(), orig.node);
    }
    else if(object->m_state.position.room == &alternate)
    {
      setParent(object->getNode(), alternate.node);
      continue;
    }
    else
    {
      continue;
    }

    if(const auto tmp = std::dynamic_pointer_cast<objects::Block>(object.get()))
    {
      loader::file::Room::patchHeightsForBlock(*tmp, -core::SectorSize);
    }
    else if(const auto tmp = std::dynamic_pointer_cast<objects::TallBlock>(object.get()))
    {
      loader::file::Room::patchHeightsForBlock(*tmp, -core::SectorSize * 2);
    }
  }

  for(const auto& object : m_dynamicObjects)
  {
    if(object->m_state.position.room == &orig)
    {
      setParent(object->getNode(), orig.node);
    }
    else if(object->m_state.position.room == &alternate)
    {
      setParent(object->getNode(), alternate.node);
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

std::shared_ptr<objects::PickupObject> Engine::createPickup(const core::TypeId type,
                                                            const gsl::not_null<const loader::file::Room*>& room,
                                                            const core::TRVec& position)
{
  loader::file::Item item;
  item.type = type;
  item.room = uint16_t(-1);
  item.position = position;
  item.rotation = 0_deg;
  item.shade = core::Shade{core::Shade::type{0}};
  item.activationState = 0;

  const auto& spriteSequence = findSpriteSequenceForType(type);
  Expects(spriteSequence != nullptr && !spriteSequence->sprites.empty());
  const loader::file::Sprite& sprite = spriteSequence->sprites[0];

  auto object
    = std::make_shared<objects::PickupObject>(this, "pickup", room, item, &sprite, m_materialManager->getSprite());

  m_dynamicObjects.emplace(object);
  addChild(room->node, object->getNode());

  return object;
}

void Engine::doGlobalEffect()
{
  if(m_activeEffect.has_value())
    runEffect(*m_activeEffect, nullptr);

  m_audioEngine->setUnderwater(m_cameraController->getCurrentRoom()->isWaterRoom());
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
  for(const auto& object : m_objects | boost::adaptors::map_values)
  {
    if(object.get() == m_lara) // Lara is special and needs to be updated last
      continue;

    if(object->m_isActive)
      object->update();

    object->getNode()->setVisible(object->m_state.triggerState != objects::TriggerState::Invisible);
  }

  for(const auto& object : m_dynamicObjects)
  {
    if(object->m_isActive)
      object->update();

    object->getNode()->setVisible(object->m_state.triggerState != objects::TriggerState::Invisible);
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

void Engine::drawDebugInfo(gl::Image<gl::SRGBA8>& img,
                           const gsl::not_null<std::shared_ptr<gl::Font>>& font,
                           const float fps)
{
  drawText(img, font, img.getWidth() - 40, img.getHeight() - 20, std::to_string(fps));
}

void Engine::drawText(gl::Image<gl::SRGBA8>& img,
                      const gsl::not_null<std::shared_ptr<gl::Font>>& font,
                      const int x,
                      const int y,
                      const std::string& txt,
                      const gl::SRGBA8& col)
{
  font->drawText(img, txt.c_str(), x, y, col, DebugTextFontSize);
}

Engine::Engine(const std::filesystem::path& rootPath, bool fullscreen, const glm::ivec2& resolution)
    : m_rootPath{rootPath}
    , m_scriptEngine{createScriptEngine(rootPath)}
    , m_window{std::make_unique<gl::Window>(fullscreen, resolution)}
    , m_renderer{std::make_unique<render::scene::Renderer>(
        std::make_shared<render::scene::Camera>(glm::radians(80.0f), m_window->getAspectRatio(), 20.0f, 20480.0f))}
    , splashImage{m_rootPath / "splash.png"}
    , abibasFont{std::make_shared<gl::Font>(m_rootPath / "abibas.ttf")}
    , m_inventory{*this}
{
  m_shaderManager = std::make_shared<render::scene::ShaderManager>(m_rootPath / "shaders");
  m_csm = std::make_shared<render::scene::CSM>(CSMResolution, *m_shaderManager);
  m_materialManager = std::make_unique<render::scene::MaterialManager>(m_shaderManager, m_csm, m_renderer);

  scaleSplashImage();

  screenOverlay = std::make_shared<render::scene::ScreenOverlay>(*m_shaderManager, m_window->getViewport());
  drawLoadingScreen("Booting");

  try
  {
    m_scriptEngine.safe_script_file((m_rootPath / "scripts/main.lua").string());
  }
  catch(sol::error& e)
  {
    BOOST_LOG_TRIVIAL(fatal) << "Failed to load main.lua: " << e.what();
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to load main.lua"));
  }

  const sol::optional<std::string> glidosPack = m_scriptEngine["getGlidosPack"]();

  std::unique_ptr<loader::trx::Glidos> glidos;
  if(glidosPack && std::filesystem::is_directory(glidosPack.value()))
  {
    drawLoadingScreen("Loading Glidos texture pack");
    glidos = std::make_unique<loader::trx::Glidos>(m_rootPath / glidosPack.value(),
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

    m_level = loader::file::level::Level::createLoader(m_rootPath / "data/tr1/data" / (baseName + ".PHD"),
                                                       loader::file::level::Game::Unknown);

    drawLoadingScreen("Loading " + baseName);

    m_level->loadFileData();

    m_audioEngine = std::make_unique<AudioEngine>(
      *this, m_rootPath / "data/tr1/audio", m_level->m_soundDetails, m_level->m_soundmap, m_level->m_sampleIndices);

    BOOST_LOG_TRIVIAL(info) << "Loading samples...";

    for(const auto offset : m_level->m_sampleIndices)
    {
      Expects(offset < m_level->m_samplesData.size());
      m_audioEngine->getSoundEngine().addWav(&m_level->m_samplesData[offset]);
    }

    for(size_t i = 0; i < m_level->m_textures.size(); ++i)
    {
      if(glidos != nullptr)
        drawLoadingScreen("Upgrading texture " + std::to_string(i + 1) + " of "
                          + std::to_string(m_level->m_textures.size()));
      else
        drawLoadingScreen("Loading texture " + std::to_string(i + 1) + " of "
                          + std::to_string(m_level->m_textures.size()));
      m_level->m_textures[i].toImage(
        glidos.get(), std::function<void(const std::string&)>([this](const std::string& s) { drawLoadingScreen(s); }));
    }

    drawLoadingScreen("Preparing the game");
    m_renderPipeline = std::make_shared<render::RenderPipeline>(*m_shaderManager, m_window->getViewport());
    loadSceneData();

    if(useAlternativeLara)
    {
      useAlternativeLaraAppearance();
    }

    for(const auto& item : initInv)
      m_inventory.put(item.first, item.second);
  }
  else
  {
    m_audioEngine = std::make_unique<AudioEngine>(*this, m_rootPath / "data/tr1/audio");
    m_cameraController = std::make_unique<CameraController>(this, m_renderer->getCamera(), true);
  }

  m_audioEngine->getSoundEngine().setListener(m_cameraController.get());

  if(!cutsceneName.empty() && !isVideo)
  {
    m_cameraController->setEyeRotation(0_deg, core::angleFromDegrees(levelInfo.get<float>("cameraRot")));
    auto pos = m_cameraController->getTRPosition().position;
    if(auto x = levelInfo["cameraPosX"])
      pos.X = core::Length{x.get<core::Length::type>()};
    if(auto y = levelInfo["cameraPosY"])
      pos.Y = core::Length{y.get<core::Length::type>()};
    if(auto z = levelInfo["cameraPosZ"])
      pos.Z = core::Length{z.get<core::Length::type>()};

    m_cameraController->setPosition(pos);

    if(levelInfo["flipRooms"].get_or(false))
      swapAllRooms();

    if(bool(levelInfo["gunSwap"]))
    {
      const auto& laraPistol = findAnimatedModelForType(TR1ItemId::LaraPistolsAnim);
      Expects(laraPistol != nullptr);
      for(const auto& object : m_objects | boost::adaptors::map_values)
      {
        if(object->m_state.type != TR1ItemId::CutsceneActor1)
          continue;

        object->getNode()->getChild(1)->setRenderable(laraPistol->renderMeshes[1].get());
        object->getNode()->getChild(4)->setRenderable(laraPistol->renderMeshes[4].get());
      }
    }
  }

  if(m_level != nullptr)
  {
    for(size_t i = 0; i < m_level->m_textures.size(); ++i)
      m_allTextures->assign(m_level->m_textures[i].image->getRawData(), gsl::narrow_cast<int>(i), 0);

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

      Rect(const glm::vec2& t0, const glm::vec2& t1)
      {
        x0 = std::min(t0.x, t1.x);
        y0 = std::min(t0.y, t1.y);
        x1 = std::max(t0.x, t1.x);
        y1 = std::max(t0.y, t1.y);
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
    for(const auto& sprite : m_level->m_sprites)
    {
      tilesByTexture[sprite.texture_id.get()].emplace(sprite.t0 * 256.0f, sprite.t1 * 256.0f);
    }

    size_t totalTiles = 0;
    for(const auto& textureAndTiles : tilesByTexture)
      totalTiles += textureAndTiles.second.size();
    BOOST_LOG_TRIVIAL(debug) << totalTiles << " unique texture tiles";

    const auto cacheBaseDir = glidos != nullptr ? glidos->getBaseDir() : m_rootPath / "data" / "tr1" / "data";
    auto cache = loader::file::TextureCache{cacheBaseDir / "_edisonengine"};

    size_t processedTiles = 0;
    for(const auto& textureAndTiles : tilesByTexture)
    {
      drawLoadingScreen("Mipmapping (" + std::to_string(processedTiles * 100 / totalTiles) + "%)");
      processedTiles += textureAndTiles.second.size();

      const loader::file::DWordTexture& texture = m_level->m_textures.at(textureAndTiles.first);
      Expects(texture.image->getWidth() == texture.image->getHeight());

      BOOST_LOG_TRIVIAL(debug) << "Mipmapping texture " << textureAndTiles.first;

      const gl::CImgWrapper src{reinterpret_cast<uint8_t*>(texture.image->getRawData()),
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
          m_allTextures->assign(reinterpret_cast<const gl::SRGBA8*>(dst.data()), textureAndTiles.first, mipmapLevel);
        }
        else
        {
          gl::CImgWrapper dst{dstSize, dstSize};
          for(const Rect& r : textureAndTiles.second)
          {
            // (scaled) source coordinates
            const auto x0 = r.x0 * texture.image->getWidth() / 256;
            const auto y0 = r.y0 * texture.image->getHeight() / 256;
            const auto x1 = (r.x1 + 1) * texture.image->getWidth() / 256;
            const auto y1 = (r.y1 + 1) * texture.image->getHeight() / 256;
            BOOST_ASSERT(x0 < x1);
            BOOST_ASSERT(y0 < y1);
            gl::CImgWrapper tmp = src.cropped(x0, y0, x1 - 1, y1 - 1);
            tmp.resizePow2Mipmap(mipmapLevel);
            // +1 for doing mathematically correct rounding
            dst.replace(
              (x0 * dstSize + 1) / texture.image->getWidth(), (y0 * dstSize + 1) / texture.image->getHeight(), tmp);
          }

          cache.savePng(texture.md5, mipmapLevel, dst);
          dst.interleave();
          m_allTextures->assign(reinterpret_cast<const gl::SRGBA8*>(dst.data()), textureAndTiles.first, mipmapLevel);
        }
      }
    }
  }
}

void Engine::run()
{
  ui::debug::DebugView debugView{0, nullptr};

  gl::Framebuffer::unbindAll();

  screenOverlay->init(*m_shaderManager, m_window->getViewport());

  if(const sol::optional<std::string> video = levelInfo["video"])
  {
    render::scene::RenderContext context{render::scene::RenderMode::Full, std::nullopt};
    render::scene::Node dummyNode{""};
    context.setCurrentNode(&dummyNode);
    video::play(m_rootPath / "data/tr1/fmv" / video.value(),
                m_audioEngine->getSoundEngine().getDevice(),
                screenOverlay->getImage(),
                [&]() {
                  if(m_window->updateWindowSize())
                  {
                    m_renderer->getCamera()->setAspectRatio(m_window->getAspectRatio());
                    screenOverlay->init(*m_shaderManager, m_window->getViewport());
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

  auto font = std::make_shared<gl::Font>(m_rootPath / "DroidSansMono.ttf");
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

    m_audioEngine->getSoundEngine().update();
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
      m_renderer->getCamera()->setAspectRatio(m_window->getAspectRatio());
      m_renderPipeline->resize(m_window->getViewport());
      screenOverlay->init(*m_shaderManager, m_window->getViewport());
    }

    std::unordered_set<const loader::file::Portal*> waterEntryPortals;
    if(!isCutscene)
    {
      waterEntryPortals = m_cameraController->update();
    }
    else
    {
      if(++m_cameraController->m_cinematicFrame >= m_level->m_cinematicFrames.size())
        break;

      waterEntryPortals
        = m_cameraController->updateCinematic(m_level->m_cinematicFrames[m_cameraController->m_cinematicFrame], false);
    }
    doGlobalEffect();

    if(m_lara != nullptr)
      drawBars(screenOverlay->getImage());

    m_renderPipeline->update(m_renderer->getCamera(), m_renderer->getGameTime());

    {
      gl::DebugGroup dbg{"csm-pass"};
      m_renderer->resetRenderState();
      m_csm->update(*m_renderer->getCamera());
      m_csm->applyViewport();

      for(size_t i = 0; i < render::scene::CSMBuffer::NSplits; ++i)
      {
        gl::DebugGroup dbg2{"csm-pass/" + std::to_string(i)};
        m_renderer->resetRenderState();

        m_csm->setActiveSplit(i);
        m_csm->getActiveFramebuffer()->bind();
        m_renderer->clear(gl::api::ClearBufferMask::DepthBufferBit, {0, 0, 0, 0}, 1);

        render::scene::RenderContext context{render::scene::RenderMode::CSMDepthOnly,
                                             m_csm->getActiveMatrix(glm::mat4{1.0f})};
        render::scene::RenderVisitor visitor{context};

        for(const auto& room : m_level->m_rooms)
        {
          if(!room.node->isVisible())
            continue;

          for(const auto& child : room.node->getChildren())
          {
            visitor.visit(*child);
          }
        }

        m_csm->finishSplitRender();
      }
    }

    {
      gl::DebugGroup dbg{"geometry-pass"};
      m_renderPipeline->bindGeometryFrameBuffer(m_window->getViewport());
      m_renderer->clear(
        gl::api::ClearBufferMask::ColorBufferBit | gl::api::ClearBufferMask::DepthBufferBit, {0, 0, 0, 0}, 1);

      {
        gl::DebugGroup dbg{"depth-prefill-pass"};
        m_renderer->resetRenderState();
        render::scene::RenderContext context{render::scene::RenderMode::DepthOnly,
                                             m_cameraController->getCamera()->getViewProjectionMatrix()};
        for(const auto& room : m_level->m_rooms)
        {
          if(!room.node->isVisible())
            continue;

          gl::DebugGroup debugGroup{room.node->getId()};
          context.setCurrentNode(room.node.get());
          room.node->getRenderable()->render(context);
        }
        if constexpr(render::RenderPipeline::FlushStages)
          GL_ASSERT(gl::api::finish());
      }

      m_renderer->resetRenderState();
      m_renderer->render();

      if constexpr(render::RenderPipeline::FlushStages)
        GL_ASSERT(gl::api::finish());
    }

    {
      gl::DebugGroup dbg{"portal-depth-pass"};
      m_renderer->resetRenderState();

      render::scene::RenderContext context{render::scene::RenderMode::DepthOnly,
                                           m_cameraController->getCamera()->getViewProjectionMatrix()};
      render::scene::Node dummyNode{""};
      context.setCurrentNode(&dummyNode);

      m_renderPipeline->bindPortalFrameBuffer();
      for(const auto& portal : waterEntryPortals)
      {
        portal->mesh->render(context);
      }
      if constexpr(render::RenderPipeline::FlushStages)
        GL_ASSERT(gl::api::finish());
    }

    render::scene::RenderContext context{render::scene::RenderMode::Full,
                                         m_cameraController->getCamera()->getViewProjectionMatrix()};
    render::scene::Node dummyNode{""};
    context.setCurrentNode(&dummyNode);

    m_renderPipeline->finalPass(m_cameraController->getCurrentRoom()->isWaterRoom());

    if(debugView.isVisible())
    {
      if(m_lara != nullptr)
      {
        debugView.update(*m_lara, m_objects, m_dynamicObjects);
      }
    }
    if(showDebugInfo)
    {
      drawDebugInfo(*screenOverlay->getImage(), font, m_renderer->getFrameRate());

      const auto drawObjectName
        = [this, &font](const std::shared_ptr<objects::Object>& object, const gl::SRGBA8& color) {
            const auto vertex = glm::vec3{m_renderer->getCamera()->getViewMatrix()
                                          * glm::vec4(object->getNode()->getTranslationWorld(), 1)};

            if(vertex.z > -m_renderer->getCamera()->getNearPlane())
            {
              return;
            }
            else if(vertex.z < -m_renderer->getCamera()->getFarPlane())
            {
              return;
            }

            glm::vec4 projVertex{vertex, 1};
            projVertex = m_renderer->getCamera()->getProjectionMatrix() * projVertex;
            projVertex /= projVertex.w;

            if(std::abs(projVertex.x) > 1 || std::abs(projVertex.y) > 1)
              return;

            projVertex.x = (projVertex.x / 2 + 0.5f) * m_window->getViewport().x;
            projVertex.y = (1 - (projVertex.y / 2 + 0.5f)) * m_window->getViewport().y;

            font->drawText(*screenOverlay->getImage(),
                           object->getNode()->getId().c_str(),
                           static_cast<int>(projVertex.x),
                           static_cast<int>(projVertex.y),
                           color,
                           DebugTextFontSize);
          };

      for(const auto& object : m_objects | boost::adaptors::map_values)
      {
        drawObjectName(object, gl::SRGBA8{255});
      }
      for(const auto& object : m_dynamicObjects)
      {
        drawObjectName(object, gl::SRGBA8{0, 255, 0, 255});
      }
    }

    {
      gl::DebugGroup dbg{"screen-overlay-pass"};
      m_renderer->resetRenderState();
      screenOverlay->render(context);
    }
    m_window->swapBuffers();

    if(m_inputHandler->getInputState().save.justPressed())
    {
      scaleSplashImage();
      drawLoadingScreen("Saving...");

      BOOST_LOG_TRIVIAL(info) << "Save";

      serialization::Serializer::save("quicksave.yaml", *this);

      nextFrameTime = std::chrono::high_resolution_clock::now() + frameDuration;
    }
    else if(m_inputHandler->getInputState().load.justPressed())
    {
      scaleSplashImage();
      drawLoadingScreen("Loading...");

      serialization::Serializer::load("quicksave.yaml", *this);
      m_level->updateRoomBasedCaches();

      nextFrameTime = std::chrono::high_resolution_clock::now() + frameDuration;
    }
  }

  debugView.stop();
}

void Engine::scaleSplashImage()
{
  // scale splash image so that its aspect ratio is preserved, but the boundaries match
  const float splashScale = std::max(gsl::narrow<float>(m_window->getViewport().x) / splashImage.width(),
                                     gsl::narrow<float>(m_window->getViewport().y) / splashImage.height());

  splashImageScaled = splashImage;
  splashImageScaled.resize(static_cast<int>(splashImageScaled.width() * splashScale),
                           static_cast<int>(splashImageScaled.height() * splashScale));

  // crop to boundaries
  const auto centerX = splashImageScaled.width() / 2;
  const auto centerY = splashImageScaled.height() / 2;
  splashImageScaled.crop(gsl::narrow<int>(centerX - m_window->getViewport().x / 2),
                         gsl::narrow<int>(centerY - m_window->getViewport().y / 2),
                         gsl::narrow<int>(centerX - m_window->getViewport().x / 2 + m_window->getViewport().x - 1),
                         gsl::narrow<int>(centerY - m_window->getViewport().y / 2 + m_window->getViewport().y - 1));

  Expects(static_cast<size_t>(splashImageScaled.width()) == m_window->getViewport().x);
  Expects(static_cast<size_t>(splashImageScaled.height()) == m_window->getViewport().y);

  splashImageScaled.interleave();
}

void Engine::drawLoadingScreen(const std::string& state)
{
  glfwPollEvents();
  if(m_window->updateWindowSize())
  {
    m_renderer->getCamera()->setAspectRatio(m_window->getAspectRatio());
    screenOverlay->init(*m_shaderManager, m_window->getViewport());
    scaleSplashImage();
  }
  screenOverlay->getImage()->assign(reinterpret_cast<const gl::SRGBA8*>(splashImageScaled.data()),
                                    m_window->getViewport().x * m_window->getViewport().y);
  abibasFont->drawText(*screenOverlay->getImage(),
                       state.c_str(),
                       40,
                       screenOverlay->getImage()->getHeight() - 100,
                       gl::SRGBA8{255, 255, 255, 192},
                       StatusLineFontSize);

  gl::Framebuffer::unbindAll();

  m_renderer->clear(
    gl::api::ClearBufferMask::ColorBufferBit | gl::api::ClearBufferMask::DepthBufferBit, {0, 0, 0, 0}, 1);
  render::scene::RenderContext context{render::scene::RenderMode::Full, std::nullopt};
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
      auto swtch = m_objects.at(floordata::Command{*floorData++}.parameter);
      if(!swtch->triggerSwitch(activationRequest.getTimeout()))
        return;

      switchIsOn = (swtch->m_state.current_anim_state == 1_as);
      conditionFulfilled = true;
    }
    break;
    case floordata::SequenceCondition::KeyUsed:
    {
      auto key = m_objects.at(floordata::Command{*floorData++}.parameter);
      if(key->triggerKey())
        conditionFulfilled = true;
      else
        return;
    }
    break;
    case floordata::SequenceCondition::ItemPickedUp:
      if(m_objects.at(floordata::Command{*floorData++}.parameter)->triggerPickUp())
        conditionFulfilled = true;
      else
        return;
      break;
    case floordata::SequenceCondition::LaraInCombatMode:
      conditionFulfilled = m_lara->getHandStatus() == objects::HandStatus::Combat;
      break;
    case floordata::SequenceCondition::ItemIsHere:
    case floordata::SequenceCondition::Dummy: return;
    default: conditionFulfilled = true; break;
    }
  }

  if(!conditionFulfilled)
    return;

  bool swapRooms = false;
  std::optional<size_t> flipEffect;
  while(true)
  {
    const floordata::Command command{*floorData++};
    switch(command.opcode)
    {
    case floordata::CommandOpcode::Activate:
    {
      auto& object = *m_objects.at(command.parameter);
      if(object.m_state.activationState.isOneshot())
        break;

      object.m_state.timer = activationRequest.getTimeout();

      if(chunkHeader.sequenceCondition == floordata::SequenceCondition::ItemActivated)
        object.m_state.activationState ^= activationRequest.getActivationSet();
      else if(chunkHeader.sequenceCondition == floordata::SequenceCondition::LaraOnGroundInverted)
        object.m_state.activationState &= ~activationRequest.getActivationSet();
      else
        object.m_state.activationState |= activationRequest.getActivationSet();

      if(!object.m_state.activationState.isFullyActivated())
        break;

      if(activationRequest.isOneshot())
        object.m_state.activationState.setOneshot(true);

      if(object.m_isActive)
        break;

      if(object.m_state.triggerState == objects::TriggerState::Inactive
         || object.m_state.triggerState == objects::TriggerState::Invisible
         || dynamic_cast<objects::AIAgent*>(&object) == nullptr)
      {
        object.m_state.triggerState = objects::TriggerState::Active;
        object.m_state.touch_bits = 0;
        object.activate();
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
    case floordata::CommandOpcode::LookAt: m_cameraController->setLookAtObject(getObject(command.parameter)); break;
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

  if(flipEffect.has_value())
    setGlobalEffect(*flipEffect);
}

core::TypeId Engine::find(const loader::file::SkeletalModelType* model) const
{
  for(const auto& item : m_level->m_animatedModels)
  {
    if(item.second.get() == model)
      return item.first;
  }

  BOOST_THROW_EXCEPTION(std::runtime_error("Cannot find model"));
}

core::TypeId Engine::find(const loader::file::Sprite* sprite) const
{
  for(const auto& sequence : m_level->m_spriteSequences)
  {
    if(!sequence.second->sprites.empty() && &sequence.second->sprites[0] == sprite)
      return sequence.first;
  }

  BOOST_THROW_EXCEPTION(std::runtime_error("Cannot find sprite"));
}

void Engine::serialize(const serialization::Serializer& ser)
{
  if(ser.loading)
  {
    m_renderer->getScene()->clear();
    for(auto& room : m_level->m_rooms)
    {
      room.resetScenery();
      m_renderer->getScene()->addNode(room.node);
    }
  }

  ser(S_NV("objectCounter", m_objectCounter),
      S_NV("objects", m_objects),
      S_NV("inventory", m_inventory),
      S_NV("mapFlipActivationStates", mapFlipActivationStates),
      S_NV("cameras", serialization::FrozenVector{m_level->m_cameras}),
      S_NV("activeEffect", m_activeEffect),
      S_NV("effectTimer", m_effectTimer),
      S_NV("cameraController", *m_cameraController),
      S_NV("secretsFound", m_secretsFoundBitmask),
      S_NV("lara", serialization::ObjectReference{m_lara}));
}

const engine::floordata::FloorData& Engine::getFloorData() const
{
  return m_level->m_floorData;
}

Engine::~Engine() = default;
} // namespace engine
