#include "engine.h"

#include "audio/tracktype.h"
#include "core/pybindmodule.h"
#include "engine/ai/ai.h"
#include "engine/audioengine.h"
#include "floordata/floordata.h"
#include "hid/inputhandler.h"
#include "loader/file/level/level.h"
#include "loader/file/rendermeshdata.h"
#include "loader/file/texturecache.h"
#include "loader/trx/trx.h"
#include "menu/menudisplay.h"
#include "objects/aiagent.h"
#include "objects/block.h"
#include "objects/laraobject.h"
#include "objects/modelobject.h"
#include "objects/objectfactory.h"
#include "objects/pickupobject.h"
#include "objects/tallblock.h"
#include "presenter.h"
#include "render/renderpipeline.h"
#include "render/scene/csm.h"
#include "render/scene/materialmanager.h"
#include "render/scene/renderer.h"
#include "render/scene/rendervisitor.h"
#include "render/scene/scene.h"
#include "render/scene/screenoverlay.h"
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
#include "throttler.h"
#include "tracks_tr1.h"
#include "ui/label.h"

#include <boost/locale/generator.hpp>
#include <boost/locale/info.hpp>
#include <boost/range/adaptor/map.hpp>
#include <filesystem>
#include <gl/font.h>
#include <glm/gtx/norm.hpp>
#include <locale>
#include <numeric>
#include <pybind11/embed.h>

namespace engine
{
namespace
{
std::shared_ptr<pybind11::scoped_interpreter> createScriptEngine(const std::filesystem::path& rootPath)
{
  auto interpreter = std::make_shared<pybind11::scoped_interpreter>();
  pybind11::module::import("sys").attr("path").cast<pybind11::list>().append(
    std::filesystem::absolute(rootPath).string());
  return interpreter;
}
} // namespace

std::tuple<int8_t, int8_t> Engine::getFloorSlantInfo(gsl::not_null<const loader::file::Sector*> sector,
                                                     const core::TRVec& position)
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
  return std::make_tuple(gsl::narrow_cast<int8_t>(fd.get() & 0xffu), gsl::narrow_cast<int8_t>(fd.get() >> 8u));
}

void Engine::swapAllRooms()
{
  BOOST_LOG_TRIVIAL(info) << "Swapping rooms";
  Expects(m_level->m_rooms.size() == m_roomOrder.size());
  for(size_t i = 0; i < m_level->m_rooms.size(); ++i)
  {
    auto& room = m_level->m_rooms[i];
    if(room.alternateRoom.get() < 0)
      continue;

    BOOST_ASSERT(static_cast<size_t>(room.alternateRoom.get()) < m_level->m_rooms.size());
    std::swap(m_roomOrder[i], m_roomOrder.at(room.alternateRoom.get()));
    swapWithAlternate(room, m_level->m_rooms.at(room.alternateRoom.get()));
  }

  m_roomsAreSwapped = !m_roomsAreSwapped;
  m_level->updateRoomBasedCaches();
}

bool Engine::isValid(const loader::file::AnimFrame* frame) const
{
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  return reinterpret_cast<const short*>(frame) >= m_level->m_poseFrames.data()
         // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
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

void Engine::loadSceneData(const std::string& animatedTextureId)
{
  for(auto& sprite : m_level->m_sprites)
  {
    sprite.image = m_level->m_textures[sprite.texture_id.get()].image;
  }

  m_presenter->initTextures(*m_level, animatedTextureId);

  for(size_t i = 0; i < m_level->m_meshes.size(); ++i)
  {
    m_level->m_meshes[i].meshData = std::make_shared<loader::file::RenderMeshData>(
      m_level->m_meshes[i], m_level->m_textureTiles, *m_level->m_palette);
  }

  for(auto idx : m_level->m_meshIndices)
  {
    Expects(idx < m_level->m_meshes.size());
    m_meshesDirect.emplace_back(&m_level->m_meshes[idx]);
  }

  for(const std::unique_ptr<loader::file::SkeletalModelType>& model :
      m_level->m_animatedModels | boost::adaptors::map_values)
  {
    if(model->nMeshes > 0)
    {
      BOOST_ASSERT(model->boneTree.empty() || static_cast<size_t>(model->nMeshes) == model->boneTree.size() + 1);
      for(size_t i = 0; i < gsl::narrow_cast<size_t>(model->nMeshes); ++i)
      {
        const auto& mesh = m_meshesDirect.at(model->mesh_base_index + i);
        model->bones.emplace_back(mesh->meshData,
                                  mesh->center,
                                  mesh->collision_size,
                                  i == 0 || model->boneTree.empty() ? std::nullopt
                                                                    : std::make_optional(model->boneTree[i - 1]));
      }
    }
  }

  for(auto& staticMesh : m_level->m_staticMeshes)
  {
    loader::file::RenderMeshDataCompositor compositor;
    compositor.append(*m_meshesDirect.at(staticMesh.mesh)->meshData);
    staticMesh.renderMesh = compositor.toMesh(*m_presenter->getMaterialManager(), false, {});
  }

  for(size_t i = 0; i < m_level->m_rooms.size(); ++i)
  {
    m_level->m_rooms[i].createSceneNode(
      i, *m_level, m_presenter->getTextureAnimator(), *m_presenter->getMaterialManager());
    m_presenter->getRenderer().getScene()->addNode(m_level->m_rooms[i].node);
  }

  m_objectManager.createObjects(*this, m_level->m_items);
  if(m_objectManager.getLaraPtr() == nullptr)
  {
    m_cameraController = std::make_unique<CameraController>(this, m_presenter->getRenderer().getCamera(), true);

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
    m_cameraController = std::make_unique<CameraController>(this, m_presenter->getRenderer().getCamera());
  }

  m_positionalEmitters.reserve(m_level->m_soundSources.size());
  for(loader::file::SoundSource& src : m_level->m_soundSources)
  {
    m_positionalEmitters.emplace_back(src.position.toRenderSystem(), &m_presenter->getAudioEngine().getSoundEngine());
    auto handle = m_presenter->getAudioEngine().playSoundEffect(src.sound_effect_id, &m_positionalEmitters.back());
    Expects(handle != nullptr);
    handle->setLooping(true);
  }
}

// ReSharper disable once CppMemberFunctionMayBeConst
void Engine::useAlternativeLaraAppearance(const bool withHead)
{
  const auto& base = *findAnimatedModelForType(TR1ItemId::Lara);
  BOOST_ASSERT(base.bones.size() == m_objectManager.getLara().getSkeleton()->getBoneCount());

  const auto& alternate = *findAnimatedModelForType(TR1ItemId::AlternativeLara);
  BOOST_ASSERT(alternate.bones.size() == m_objectManager.getLara().getSkeleton()->getBoneCount());

  for(size_t i = 0; i < m_objectManager.getLara().getSkeleton()->getBoneCount(); ++i)
    m_objectManager.getLara().getSkeleton()->setMeshPart(i, alternate.bones[i].mesh);

  if(!withHead)
    m_objectManager.getLara().getSkeleton()->setMeshPart(14, base.bones[14].mesh);

  m_objectManager.getLara().getSkeleton()->rebuildMesh();
}

void Engine::dinoStompEffect(objects::Object& object)
{
  const auto d = object.m_state.position.position.toRenderSystem() - m_cameraController->getPosition();
  const auto absD = glm::abs(d);

  static constexpr auto MaxD = 16 * core::SectorSize.get<float>();
  if(absD.x > MaxD || absD.y > MaxD || absD.z > MaxD)
    return;

  const auto x = (100_len).cast<float>() * (1 - length2(d) / util::square(MaxD));
  m_cameraController->setBounce(x.cast<core::Length>());
}

// ReSharper disable once CppMemberFunctionMayBeStatic
// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void Engine::turn180Effect(objects::Object& object)
{
  object.m_state.rotation.Y += 180_deg;
}

void Engine::laraNormalEffect()
{
  m_objectManager.getLara().setCurrentAnimState(loader::file::LaraStateId::Stop);
  m_objectManager.getLara().setRequiredAnimState(loader::file::LaraStateId::Unknown12);
  m_objectManager.getLara().getSkeleton()->anim
    = &m_level->m_animations[static_cast<int>(loader::file::AnimationId::STAY_SOLID)];
  m_objectManager.getLara().getSkeleton()->frame_number = 185_frame;
  m_cameraController->setMode(CameraMode::Chase);
  m_presenter->getRenderer().getCamera()->setFieldOfView(glm::radians(80.0f));
}

void Engine::laraBubblesEffect(objects::Object& object)
{
  const auto modelNode = dynamic_cast<objects::ModelObject*>(&object);
  if(modelNode == nullptr)
    return;

  auto bubbleCount = util::rand15(12);
  if(bubbleCount == 0)
    return;

  object.playSoundEffect(TR1SoundEffect::LaraUnderwaterGurgle);

  const auto boneSpheres = modelNode->getSkeleton()->getBoneCollisionSpheres(
    object.m_state, *modelNode->getSkeleton()->getInterpolationInfo().getNearestFrame(), nullptr);

  const auto position
    = core::TRVec{glm::vec3{translate(boneSpheres.at(14).m, core::TRVec{0_len, 0_len, 50_len}.toRenderSystem())[3]}};

  while(bubbleCount-- > 0)
  {
    auto particle
      = std::make_shared<BubbleParticle>(core::RoomBoundPosition{object.m_state.position.room, position}, *this);
    setParent(particle, object.m_state.position.room->node);
    m_objectManager.registerParticle(particle);
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
    m_presenter->getAudioEngine().playSoundEffect(TR1SoundEffect::Explosion1, nullptr);
    m_cameraController->setBounce(-250_len);
    break;
  case 3: m_presenter->getAudioEngine().playSoundEffect(TR1SoundEffect::RollingBall, nullptr); break;
  case 35: m_presenter->getAudioEngine().playSoundEffect(TR1SoundEffect::Explosion1, nullptr); break;
  case 20:
  case 50:
  case 70: m_presenter->getAudioEngine().playSoundEffect(TR1SoundEffect::TRexFootstep, nullptr); break;
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
    auto pos = m_objectManager.getLara().m_state.position.position;
    core::Frame mul = 0_frame;
    if(m_effectTimer >= 30_frame)
    {
      mul = m_effectTimer - 30_frame;
    }
    else
    {
      mul = 30_frame - m_effectTimer;
    }
    pos.Y = 100_len * mul / 1_frame + m_cameraController->getLookAt().position.Y;
    m_presenter->getAudioEngine().playSoundEffect(TR1SoundEffect::WaterFlow3, pos.toRenderSystem());
  }
  else
  {
    m_activeEffect.reset();
  }
  m_effectTimer += 1_frame;
}

void Engine::chandelierEffect()
{
  m_presenter->getAudioEngine().playSoundEffect(TR1SoundEffect::GlassyFlow, nullptr);
  m_activeEffect.reset();
}

void Engine::raisingBlockEffect()
{
  m_effectTimer += 1_frame;
  if(m_effectTimer == 5_frame)
  {
    m_presenter->getAudioEngine().playSoundEffect(TR1SoundEffect::Clank, nullptr);
    m_activeEffect.reset();
  }
}

void Engine::stairsToSlopeEffect()
{
  if(m_effectTimer > 120_frame)
  {
    m_activeEffect.reset();
  }
  else
  {
    if(m_effectTimer == 0_frame)
    {
      m_presenter->getAudioEngine().playSoundEffect(TR1SoundEffect::HeavyDoorSlam, nullptr);
    }
    auto pos = m_cameraController->getLookAt().position;
    pos.Y += 100_spd * m_effectTimer;
    m_presenter->getAudioEngine().playSoundEffect(TR1SoundEffect::FlowingAir, pos.toRenderSystem());
  }
  m_effectTimer += 1_frame;
}

void Engine::sandEffect()
{
  if(m_effectTimer <= 120_frame)
  {
    m_presenter->getAudioEngine().playSoundEffect(TR1SoundEffect::LowHum, nullptr);
  }
  else
  {
    m_activeEffect.reset();
  }
  m_effectTimer += 1_frame;
}

void Engine::explosionEffect()
{
  m_presenter->getAudioEngine().playSoundEffect(TR1SoundEffect::LowPitchedSettling, nullptr);
  m_cameraController->setBounce(-75_len);
  m_activeEffect.reset();
}

// ReSharper disable once CppMemberFunctionMayBeConst
void Engine::laraHandsFreeEffect()
{
  m_objectManager.getLara().setHandStatus(objects::HandStatus::None);
}

void Engine::flipMapEffect()
{
  swapAllRooms();
}

// ReSharper disable once CppMemberFunctionMayBeConst
// NOLINTNEXTLINE(readability-make-member-function-const)
void Engine::unholsterRightGunEffect(const objects::ModelObject& object)
{
  const auto& src = *findAnimatedModelForType(TR1ItemId::LaraPistolsAnim);
  BOOST_ASSERT(src.bones.size() == object.getSkeleton()->getBoneCount());
  object.getSkeleton()->setMeshPart(10, src.bones[10].mesh);
  object.getSkeleton()->rebuildMesh();
}

void Engine::chainBlockEffect()
{
  if(m_effectTimer == 0_frame)
  {
    m_presenter->getAudioEngine().playSoundEffect(TR1SoundEffect::SecretFound, nullptr);
  }
  m_effectTimer += 1_frame;
  if(m_effectTimer == 55_frame)
  {
    m_presenter->getAudioEngine().playSoundEffect(TR1SoundEffect::LaraFallIntoWater, nullptr);
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

  for(const auto& object : m_objectManager.getObjects() | boost::adaptors::map_values)
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
  for(const auto& object : m_objectManager.getObjects() | boost::adaptors::map_values)
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

  for(const auto& object : m_objectManager.getDynamicObjects())
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

  auto object = std::make_shared<objects::PickupObject>(
    this, "pickup", room, item, &sprite, m_presenter->getMaterialManager()->getSprite());

  m_objectManager.registerDynamicObject(object);
  addChild(room->node, object->getNode());

  return object;
}

void Engine::doGlobalEffect()
{
  if(m_activeEffect.has_value())
    runEffect(*m_activeEffect, nullptr);

  m_presenter->getAudioEngine().setUnderwater(m_cameraController->getCurrentRoom()->isWaterRoom());
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
  m_objectManager.update(*this, godMode);
  m_presenter->animateUV(m_level->m_textureTiles);
}

Engine::Engine(const std::filesystem::path& rootPath, bool fullscreen, const glm::ivec2& resolution)
    : m_rootPath{rootPath}
    , m_presenter{std::make_shared<Presenter>(rootPath, fullscreen, resolution)}
    , m_scriptEngine{createScriptEngine(rootPath)}
{
  try
  {
    pybind11::eval_file((m_rootPath / "scripts/main.py").string());
  }
  catch(std::exception& e)
  {
    BOOST_LOG_TRIVIAL(fatal) << "Failed to load main.py: " << e.what();
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to load main.py"));
  }

  std::optional<std::string> glidosPack;
  if(const auto getGlidosPack = core::get<pybind11::handle>(pybind11::globals(), "getGlidosPack"))
  {
    const auto pack = getGlidosPack.value()();
    if(!pack.is_none())
      glidosPack = pack.cast<std::string>();
  }

  std::unique_ptr<loader::trx::Glidos> glidos;
  if(glidosPack && std::filesystem::is_directory(glidosPack.value()))
  {
    m_presenter->drawLoadingScreen("Loading Glidos texture pack");
    glidos = std::make_unique<loader::trx::Glidos>(m_rootPath / glidosPack.value(),
                                                   [this](const std::string& s) { m_presenter->drawLoadingScreen(s); });
  }

  levelInfo = pybind11::globals()["getLevelInfo"]();
  const bool isVideo = !core::get<std::string>(levelInfo, "video").value_or(std::string{}).empty();
  const auto cutsceneName = core::get<std::string>(levelInfo, "cutscene").value_or(std::string{});

  const auto baseName
    = cutsceneName.empty() ? core::get<std::string>(levelInfo, "baseName").value_or(std::string{}) : cutsceneName;
  Expects(isVideo || !baseName.empty());
  const bool useAlternativeLara = core::get<bool>(levelInfo, "useAlternativeLara").value_or(false);

  if(!isVideo)
  {
    std::map<TR1ItemId, size_t> initInv;

    if(const auto tbl = core::get<pybind11::dict>(levelInfo, "inventory"))
    {
      for(const auto& kv : *tbl)
        initInv[kv.first.cast<TR1ItemId>()] += kv.second.cast<size_t>();
    }

    if(const auto tbl = core::get<pybind11::dict>(
         core::get<pybind11::dict>(pybind11::globals(), "cheats").value_or(pybind11::dict{}), "inventory"))
    {
      for(const auto& kv : *tbl)
        initInv[kv.first.cast<TR1ItemId>()] += kv.second.cast<size_t>();
    }

    m_presenter->drawLoadingScreen("Preparing to load " + baseName);

    m_level = loader::file::level::Level::createLoader(m_rootPath / "data/tr1/data" / (baseName + ".PHD"),
                                                       loader::file::level::Game::Unknown);

    m_presenter->drawLoadingScreen("Loading " + baseName);

    m_level->loadFileData();
    while(m_roomOrder.size() < m_level->m_rooms.size())
      m_roomOrder.emplace_back(m_roomOrder.size());

    m_presenter->initAudio(*this, m_level.get(), m_rootPath / "data/tr1/audio");

    BOOST_LOG_TRIVIAL(info) << "Loading samples...";

    for(const auto offset : m_level->m_sampleIndices)
    {
      Expects(offset < m_level->m_samplesData.size());
      m_presenter->getAudioEngine().getSoundEngine().addWav(&m_level->m_samplesData[offset]);
    }

    for(size_t i = 0; i < m_level->m_textures.size(); ++i)
    {
      if(glidos != nullptr)
        m_presenter->drawLoadingScreen("Upgrading texture " + std::to_string(i + 1) + " of "
                                       + std::to_string(m_level->m_textures.size()));
      else
        m_presenter->drawLoadingScreen("Loading texture " + std::to_string(i + 1) + " of "
                                       + std::to_string(m_level->m_textures.size()));
      m_level->m_textures[i].toImage(
        glidos.get(),
        std::function<void(const std::string&)>([this](const std::string& s) { m_presenter->drawLoadingScreen(s); }));
    }

    m_presenter->drawLoadingScreen("Preparing the game");
    loadSceneData(baseName + "-animated");

    if(useAlternativeLara)
    {
      useAlternativeLaraAppearance();
    }

    if(m_objectManager.getLaraPtr() != nullptr)
    {
      for(const auto& item : initInv)
        m_inventory.put(m_objectManager.getLara(), item.first, item.second);
    }
  }
  else
  {
    m_presenter->initAudio(*this, nullptr, m_rootPath / "data/tr1/audio");
    m_cameraController = std::make_unique<CameraController>(this, m_presenter->getRenderer().getCamera(), true);
  }

  m_presenter->getAudioEngine().getSoundEngine().setListener(m_cameraController.get());

  if(!cutsceneName.empty() && !isVideo)
  {
    m_cameraController->setEyeRotation(0_deg, core::angleFromDegrees(levelInfo["cameraRot"].cast<float>()));
    auto pos = m_cameraController->getTRPosition().position;
    if(auto x = core::get<core::Length::type>(levelInfo, "cameraPosX"))
      pos.X = core::Length{x.value()};
    if(auto y = core::get<core::Length::type>(levelInfo, "cameraPosY"))
      pos.Y = core::Length{y.value()};
    if(auto z = core::get<core::Length::type>(levelInfo, "cameraPosZ"))
      pos.Z = core::Length{z.value()};

    m_cameraController->setPosition(pos);

    if(core::get<bool>(levelInfo, "flipRooms").value_or(false))
      swapAllRooms();

    if(core::get<bool>(levelInfo, "gunSwap").value_or(false))
    {
      const auto& laraPistol = findAnimatedModelForType(TR1ItemId::LaraPistolsAnim);
      Expects(laraPistol != nullptr);
      for(const auto& object : m_objectManager.getObjects() | boost::adaptors::map_values)
      {
        if(object->m_state.type != TR1ItemId::CutsceneActor1)
          continue;

        auto m = std::dynamic_pointer_cast<objects::ModelObject>(object.get());
        Expects(m != nullptr);
        m->getSkeleton()->setMeshPart(1, laraPistol->bones[1].mesh);
        m->getSkeleton()->setMeshPart(4, laraPistol->bones[4].mesh);
        m->getSkeleton()->rebuildMesh();
      }
    }
  }

  if(m_level != nullptr)
  {
    for(size_t i = 0; i < m_level->m_textures.size(); ++i)
      m_presenter->assignTextures(m_level->m_textures[i].image->getRawData(), gsl::narrow_cast<int>(i), 0);

    struct Rect
    {
      explicit Rect(const std::array<loader::file::UVCoordinates, 4>& cos)
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

    size_t totalTiles = std::accumulate(
      tilesByTexture.begin(), tilesByTexture.end(), std::size_t{0}, [](size_t n, const auto& textureAndTiles) {
        return n + textureAndTiles.second.size();
      });
    BOOST_LOG_TRIVIAL(debug) << totalTiles << " unique texture tiles";

    const auto cacheBaseDir = glidos != nullptr ? glidos->getBaseDir() : m_rootPath / "data" / "tr1" / "data";
    auto cache = loader::file::TextureCache{cacheBaseDir / "_edisonengine"};

    size_t processedTiles = 0;
    for(const auto& textureAndTiles : tilesByTexture)
    {
      m_presenter->drawLoadingScreen("Mipmapping (" + std::to_string(processedTiles * 100 / totalTiles) + "%)");
      processedTiles += textureAndTiles.second.size();

      const loader::file::DWordTexture& texture = m_level->m_textures.at(textureAndTiles.first);
      Expects(texture.image->getWidth() == texture.image->getHeight());

      BOOST_LOG_TRIVIAL(debug) << "Mipmapping texture " << textureAndTiles.first;

      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
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
          m_presenter->assignTextures(
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
            reinterpret_cast<const gl::SRGBA8*>(dst.data()),
            textureAndTiles.first,
            mipmapLevel);
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
          m_presenter->assignTextures(
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
            reinterpret_cast<const gl::SRGBA8*>(dst.data()),
            textureAndTiles.first,
            mipmapLevel);
        }
      }
    }
  }
}

void Engine::gameLoop(Throttler& throttler, const std::string& levelName, bool godMode)
{
  m_presenter->preFrame();

  if(!levelName.empty())
    m_presenter->drawLevelName(getPalette(), levelName);

  update(godMode);

  const auto waterEntryPortals = m_cameraController->update();
  doGlobalEffect();
  m_presenter->drawBars(getPalette(), getObjectManager());
  m_presenter->renderWorld(getObjectManager(), getRooms(), getCameraController(), waterEntryPortals);

  if(m_presenter->getInputHandler().getInputState().save.justChangedTo(true))
  {
    m_presenter->drawLoadingScreen("Saving...");

    BOOST_LOG_TRIVIAL(info) << "Save";

    serialization::Serializer::save("quicksave.yaml", *this);

    throttler.reset();
  }
  else if(m_presenter->getInputHandler().getInputState().load.justChangedTo(true))
  {
    m_presenter->drawLoadingScreen("Loading...");

    serialization::Serializer::load("quicksave.yaml", *this);
    m_level->updateRoomBasedCaches();

    throttler.reset();
  }
}

void Engine::cinematicLoop()
{
  m_presenter->preFrame();
  update(false);

  const auto waterEntryPortals
    = m_cameraController->updateCinematic(m_level->m_cinematicFrames[m_cameraController->m_cinematicFrame], false);
  doGlobalEffect();

  m_presenter->renderWorld(getObjectManager(), getRooms(), getCameraController(), waterEntryPortals);
}

void Engine::run()
{
  gl::Framebuffer::unbindAll();

  language = std::use_facet<boost::locale::info>(boost::locale::generator()("")).language();
  BOOST_LOG_TRIVIAL(info) << "Detected user's language is " << language;
  if(const std::optional<std::string> overrideLanguage
     = core::get<std::string>(pybind11::globals(), "language_override"))
  {
    language = overrideLanguage.value();
    BOOST_LOG_TRIVIAL(info) << "Language override is " << language;
  }

  if(const std::optional<std::string> video = core::get<std::string>(levelInfo, "video"))
  {
    m_presenter->playVideo(m_rootPath / "data/tr1/fmv" / video.value());
    return;
  }

  const auto levelName = loadLevel();
  const bool godMode
    = core::get<bool>(core::get<pybind11::dict>(pybind11::globals(), "cheats").value_or(pybind11::dict{}), "godMode")
        .value_or(false);
  const bool isCutscene = !core::get<std::string>(levelInfo, "cutscene").value_or(std::string{}).empty();

  std::shared_ptr<menu::MenuDisplay> menu;
  Throttler throttler;
  while(!m_presenter->shouldClose())
  {
    throttler.wait();

    if(menu != nullptr)
    {
      m_presenter->preFrame();
      menu->display(*m_presenter->getScreenOverlay().getImage(), *this);
      render::scene::RenderContext context{render::scene::RenderMode::Full, std::nullopt};
      m_presenter->getScreenOverlay().render(context);
      m_presenter->swapBuffers();
      if(menu->isDone)
        menu.reset();
      continue;
    }

    if(!isCutscene)
    {
      if(m_presenter->getInputHandler().getInputState().menu.justChangedTo(true))
      {
        menu = std::make_shared<menu::MenuDisplay>();
        menu->init(*this);
        continue;
      }

      gameLoop(throttler, levelName, godMode);
    }
    else
    {
      if(++m_cameraController->m_cinematicFrame >= m_level->m_cinematicFrames.size())
        break;
      cinematicLoop();
    }
  }
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
      if(m_objectManager.getLara().m_state.position.position.Y == m_objectManager.getLara().m_state.floor)
      {
        m_objectManager.getLara().burnIfAlive();
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
      conditionFulfilled
        = m_objectManager.getLara().m_state.position.position.Y == m_objectManager.getLara().m_state.floor;
    }
    break;
    case floordata::SequenceCondition::ItemActivated:
    {
      auto swtch = m_objectManager.getObject(floordata::Command{*floorData++}.parameter);
      Expects(swtch != nullptr);
      if(!swtch->triggerSwitch(activationRequest.getTimeout()))
        return;

      switchIsOn = (swtch->m_state.current_anim_state == 1_as);
      conditionFulfilled = true;
    }
    break;
    case floordata::SequenceCondition::KeyUsed:
    {
      auto key = m_objectManager.getObject(floordata::Command{*floorData++}.parameter);
      Expects(key != nullptr);
      if(key->triggerKey())
        conditionFulfilled = true;
      else
        return;
    }
    break;
    case floordata::SequenceCondition::ItemPickedUp:
    {
      auto item = m_objectManager.getObject(floordata::Command{*floorData++}.parameter);
      Expects(item != nullptr);
      if(item->triggerPickUp())
        conditionFulfilled = true;
      else
        return;
      break;
    }
    case floordata::SequenceCondition::LaraInCombatMode:
      conditionFulfilled = m_objectManager.getLara().getHandStatus() == objects::HandStatus::Combat;
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
      auto object = m_objectManager.getObject(command.parameter);
      Expects(object != nullptr);
      if(object->m_state.activationState.isOneshot())
        break;

      object->m_state.timer = activationRequest.getTimeout();

      if(chunkHeader.sequenceCondition == floordata::SequenceCondition::ItemActivated)
        object->m_state.activationState ^= activationRequest.getActivationSet();
      else if(chunkHeader.sequenceCondition == floordata::SequenceCondition::LaraOnGroundInverted)
        object->m_state.activationState &= ~activationRequest.getActivationSet();
      else
        object->m_state.activationState |= activationRequest.getActivationSet();

      if(!object->m_state.activationState.isFullyActivated())
        break;

      if(activationRequest.isOneshot())
        object->m_state.activationState.setOneshot(true);

      if(object->m_isActive)
        break;

      if(object->m_state.triggerState == objects::TriggerState::Inactive
         || object->m_state.triggerState == objects::TriggerState::Invisible
         || std::dynamic_pointer_cast<objects::AIAgent>(object) == nullptr)
      {
        object->m_state.triggerState = objects::TriggerState::Active;
        object->m_state.touch_bits = 0;
        object->activate();
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
    case floordata::CommandOpcode::LookAt:
      m_cameraController->setLookAtObject(m_objectManager.getObject(command.parameter));
      break;
    case floordata::CommandOpcode::UnderwaterCurrent:
    {
      const auto& sink = m_level->m_cameras.at(command.parameter);
      if(m_objectManager.getLara().m_underwaterRoute.required_box != &m_level->m_boxes[sink.box_index])
      {
        m_objectManager.getLara().m_underwaterRoute.required_box = &m_level->m_boxes[sink.box_index];
        m_objectManager.getLara().m_underwaterRoute.target = sink.position;
      }
      m_objectManager.getLara().m_underwaterCurrentStrength
        = 6_len * static_cast<core::Length::type>(sink.underwaterCurrentStrength);
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
      m_presenter->getAudioEngine().triggerCdTrack(
        static_cast<TR1TrackId>(command.parameter), activationRequest, chunkHeader.sequenceCondition);
      break;
    case floordata::CommandOpcode::Secret:
      BOOST_ASSERT(command.parameter < 16);
      if(!m_secretsFoundBitmask.test(command.parameter))
      {
        m_secretsFoundBitmask.set(command.parameter);
        m_presenter->getAudioEngine().playStopCdTrack(TR1TrackId::Secret, false);
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
  auto it = std::find_if(m_level->m_animatedModels.begin(),
                         m_level->m_animatedModels.end(),
                         [&model](const auto& item) { return item.second.get() == model; });
  if(it != m_level->m_animatedModels.end())
    return it->first;

  BOOST_THROW_EXCEPTION(std::runtime_error("Cannot find model"));
}

core::TypeId Engine::find(const loader::file::Sprite* sprite) const
{
  auto it = std::find_if(
    m_level->m_spriteSequences.begin(), m_level->m_spriteSequences.end(), [&sprite](const auto& sequence) {
      return !sequence.second->sprites.empty() && &sequence.second->sprites[0] == sprite;
    });
  if(it != m_level->m_spriteSequences.end())
    return it->first;

  BOOST_THROW_EXCEPTION(std::runtime_error("Cannot find sprite"));
}

void Engine::serialize(const serialization::Serializer& ser)
{
  if(ser.loading)
  {
    m_presenter->getRenderer().getScene()->clear();
    for(auto& room : m_level->m_rooms)
    {
      room.resetScenery();
      m_presenter->getRenderer().getScene()->addNode(room.node);
    }

    auto currentRoomOrder = m_roomOrder;
    ser(S_NV("roomOrder", m_roomOrder));
    Ensures(m_roomOrder.size() == currentRoomOrder.size());
    for(size_t i = 0; i < m_roomOrder.size(); ++i)
    {
      const auto currentIdx = currentRoomOrder[i];
      Expects(currentIdx < m_level->m_rooms.size());
      Expects(m_roomOrder[i] < m_level->m_rooms.size());
      if(currentIdx == m_roomOrder[i])
        continue;

      const auto otherIdx
        = std::distance(m_roomOrder.begin(), std::find(m_roomOrder.begin(), m_roomOrder.end(), currentIdx));

      std::swap(m_roomOrder[i], m_roomOrder[otherIdx]);
      swapWithAlternate(m_level->m_rooms[currentIdx], m_level->m_rooms[otherIdx]);
      Ensures(currentIdx == m_roomOrder[i]);
    }
  }

  ser(S_NV("objectManager", m_objectManager),
      S_NV("inventory", m_inventory),
      S_NV("mapFlipActivationStates", mapFlipActivationStates),
      S_NV("cameras", serialization::FrozenVector{m_level->m_cameras}),
      S_NV("activeEffect", m_activeEffect),
      S_NV("effectTimer", m_effectTimer),
      S_NV("cameraController", *m_cameraController),
      S_NV("secretsFound", m_secretsFoundBitmask),
      S_NV("roomsAreSwapped", m_roomsAreSwapped),
      S_NV("roomOrder", m_roomOrder));

  if(ser.loading)
    m_level->updateRoomBasedCaches();
}

const engine::floordata::FloorData& Engine::getFloorData() const
{
  return m_level->m_floorData;
}

gsl::not_null<std::shared_ptr<loader::file::RenderMeshData>> Engine::getRenderMesh(const size_t idx) const
{
  return m_level->m_meshes.at(idx).meshData;
}

const std::vector<loader::file::Mesh>& Engine::getMeshes() const
{
  return m_level->m_meshes;
}

const loader::file::Palette& Engine::getPalette() const
{
  return *m_level->m_palette;
}

std::string Engine::loadLevel()
{
  std::string levelName;
  if(const auto levelNames = core::get<pybind11::dict>(levelInfo, "name"))
  {
    levelName = core::get<std::string>(levelNames.value(), language.c_str()).value_or(std::string{});
    if(levelName.empty())
    {
      BOOST_LOG_TRIVIAL(warning) << "Missing level name, falling back to language en";
      levelName = levelNames.value()["en"].cast<std::string>();
    }
  }

  m_presenter->setTrFont(std::make_unique<ui::CachedFont>(*m_level->m_spriteSequences.at(TR1ItemId::FontGraphics)));

  if(const std::optional<TR1TrackId> trackToPlay = core::get<TR1TrackId>(levelInfo, "track"))
  {
    m_presenter->getAudioEngine().playStopCdTrack(trackToPlay.value(), false);
  }

  return levelName;
}

Engine::~Engine() = default;
} // namespace engine
