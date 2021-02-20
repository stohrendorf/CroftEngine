#include "world.h"

#include "audioengine.h"
#include "core/pybindmodule.h"
#include "engine.h"
#include "engine/i18n.h"
#include "loader/file/level/level.h"
#include "loader/file/rendermeshdata.h"
#include "loader/trx/trx.h"
#include "objects/aiagent.h"
#include "objects/block.h"
#include "objects/laraobject.h"
#include "objects/modelobject.h"
#include "objects/pickupobject.h"
#include "objects/tallblock.h"
#include "presenter.h"
#include "render/scene/camera.h"
#include "render/scene/materialmanager.h"
#include "render/scene/renderer.h"
#include "render/scene/scene.h"
#include "render/scene/screenoverlay.h"
#include "render/textureanimator.h"
#include "render/textureatlas.h"
#include "serialization/array.h"
#include "serialization/bitset.h"
#include "serialization/map.h"
#include "serialization/not_null.h"
#include "serialization/objectreference.h"
#include "serialization/optional.h"
#include "serialization/quantity.h"
#include "serialization/serialization.h"
#include "serialization/vector.h"
#include "serialization/yamldocument.h"
#include "tracks_tr1.h"
#include "ui/label.h"
#include "ui/ui.h"

#include <boost/format.hpp>
#include <gl/texture2darray.h>
#include <glm/gtx/norm.hpp>
#include <numeric>
#include <utility>

namespace engine
{
namespace
{
struct UVRect
{
  explicit UVRect(const std::array<loader::file::UVCoordinates, 4>& cos)
  {
    xy0.x = xy0.y = loader::file::UVCoordinates::Component{std::numeric_limits<uint16_t>::max()};
    xy1.x = xy1.y = loader::file::UVCoordinates::Component{std::numeric_limits<uint16_t>::min()};
    for(const auto& co : cos)
    {
      if(co.x.get() == 0 && co.y.get() == 0)
        continue;

      xy0.x = std::min(xy0.x, co.x);
      xy0.y = std::min(xy0.y, co.y);
      xy1.x = std::max(xy1.x, co.x);
      xy1.y = std::max(xy1.y, co.y);
    }
  }

  UVRect(const loader::file::UVCoordinates& t0, const loader::file::UVCoordinates& t1)
  {
    xy0.x = std::min(t0.x, t1.x);
    xy0.y = std::min(t0.y, t1.y);
    xy1.x = std::max(t0.x, t1.x);
    xy1.y = std::max(t0.y, t1.y);
  }

  constexpr bool operator==(const UVRect& rhs) const noexcept
  {
    return xy0 == rhs.xy0 && xy1 == rhs.xy1;
  }

  constexpr bool operator<(const UVRect& rhs) const noexcept
  {
    if(xy0.x != rhs.xy0.x)
      return xy0.x < rhs.xy0.x;
    if(xy0.y != rhs.xy1.y)
      return xy0.y < rhs.xy0.y;
    if(xy1.x != rhs.xy1.x)
      return xy1.x < rhs.xy1.x;
    return xy1.y < rhs.xy1.y;
  }

  loader::file::UVCoordinates xy0{};
  loader::file::UVCoordinates xy1{};
};

void activateCommand(objects::Object& object,
                     const floordata::ActivationState& activationRequest,
                     floordata::SequenceCondition condition)
{
  if(object.m_state.activationState.isOneshot())
    return;

  object.m_state.timer = activationRequest.getTimeout();

  if(condition == floordata::SequenceCondition::ItemActivated)
    object.m_state.activationState ^= activationRequest.getActivationSet();
  else if(condition == floordata::SequenceCondition::LaraOnGroundInverted)
    object.m_state.activationState &= ~activationRequest.getActivationSet();
  else
    object.m_state.activationState |= activationRequest.getActivationSet();

  if(!object.m_state.activationState.isFullyActivated())
    return;

  if(activationRequest.isOneshot())
    object.m_state.activationState.setOneshot(true);

  if(object.m_isActive)
    return;

  if(object.m_state.triggerState == objects::TriggerState::Inactive
     || object.m_state.triggerState == objects::TriggerState::Invisible
     || dynamic_cast<objects::AIAgent*>(&object) == nullptr)
  {
    object.m_state.triggerState = objects::TriggerState::Active;
    object.m_state.touch_bits = 0;
    object.activate();
  }
}

bool flipMapCommand(floordata::ActivationState& state,
                    const floordata::ActivationState& request,
                    floordata::SequenceCondition condition,
                    bool roomsAreSwapped)
{
  if(state.isOneshot())
    return false;

  if(condition == floordata::SequenceCondition::ItemActivated)
  {
    state ^= request.getActivationSet();
  }
  else
  {
    state |= request.getActivationSet();
  }

  if(state.isFullyActivated())
  {
    if(request.isOneshot())
      state.setOneshot(true);

    if(!roomsAreSwapped)
      return true;
  }
  else if(roomsAreSwapped)
  {
    return true;
  }

  return false;
}

std::optional<bool> evaluateCondition(floordata::SequenceCondition condition,
                                      const floordata::ActivationState& request,
                                      const ObjectManager& objectManager,
                                      const floordata::FloorDataValue*& floorData,
                                      bool& switchIsOn)
{
  switch(condition)
  {
  case floordata::SequenceCondition::LaraIsHere: return true;
  case floordata::SequenceCondition::LaraOnGround:
  case floordata::SequenceCondition::LaraOnGroundInverted:
    return objectManager.getLara().m_state.position.position.Y == objectManager.getLara().m_state.floor;
  case floordata::SequenceCondition::ItemActivated:
  {
    auto swtch = objectManager.getObject(floordata::Command{*floorData++}.parameter);
    Expects(swtch != nullptr);
    if(!swtch->triggerSwitch(request.getTimeout()))
      return std::nullopt;

    switchIsOn = (swtch->m_state.current_anim_state == 1_as);
    return true;
  }
  case floordata::SequenceCondition::KeyUsed:
  {
    auto key = objectManager.getObject(floordata::Command{*floorData++}.parameter);
    Expects(key != nullptr);
    if(key->triggerKey())
      return true;
    else
      return std::nullopt;
  }
  case floordata::SequenceCondition::ItemPickedUp:
  {
    auto item = objectManager.getObject(floordata::Command{*floorData++}.parameter);
    Expects(item != nullptr);
    if(item->triggerPickUp())
      return true;
    else
      return std::nullopt;
  }
  case floordata::SequenceCondition::LaraInCombatMode:
    return objectManager.getLara().getHandStatus() == objects::HandStatus::Combat;
  case floordata::SequenceCondition::ItemIsHere:
  case floordata::SequenceCondition::Dummy: return std::nullopt;
  default: return true;
  }
}
} // namespace

std::tuple<int8_t, int8_t> getFloorSlantInfo(gsl::not_null<const loader::file::Sector*> sector,
                                             const core::TRVec& position)
{
  while(sector->roomBelow != nullptr)
  {
    sector = sector->roomBelow->getSectorByAbsolutePosition(position);
  }

  static const auto zero = std::make_tuple(int8_t{0}, int8_t{0});

  if(position.Y + core::QuarterSectorSize * 2 < sector->floorHeight)
    return zero;
  if(sector->floorData == nullptr)
    return zero;
  if(floordata::FloorDataChunk{*sector->floorData}.type != floordata::FloorDataChunkType::FloorSlant)
    return zero;

  const auto fd = sector->floorData[1];
  return std::make_tuple(gsl::narrow_cast<int8_t>(fd.get() & 0xffu), gsl::narrow_cast<int8_t>(fd.get() >> 8u));
}

void World::swapAllRooms()
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

bool World::isValid(const loader::file::AnimFrame* frame) const
{
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  return reinterpret_cast<const short*>(frame) >= m_level->m_poseFrames.data()
         // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
         && reinterpret_cast<const short*>(frame) < m_level->m_poseFrames.data() + m_level->m_poseFrames.size();
}

const std::unique_ptr<loader::file::SpriteSequence>& World::findSpriteSequenceForType(core::TypeId type) const
{
  return m_level->findSpriteSequenceForType(type);
}

const loader::file::StaticMesh* World::findStaticMeshById(core::StaticMeshId meshId) const
{
  return m_level->findStaticMeshById(meshId);
}

const std::vector<loader::file::Room>& World::getRooms() const
{
  return m_level->m_rooms;
}

std::vector<loader::file::Room>& World::getRooms()
{
  return m_level->m_rooms;
}

const std::vector<loader::file::Box>& World::getBoxes() const
{
  return m_level->m_boxes;
}

void World::loadSceneData()
{
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
        const auto& mesh = (model->mesh_base_index + i).from(m_meshesDirect);
        model->bones.emplace_back(mesh->meshData,
                                  mesh->collisionCenter,
                                  mesh->collisionRadius,
                                  i == 0 || model->boneTree.empty() ? std::nullopt
                                                                    : std::make_optional(model->boneTree[i - 1]));
      }
    }
  }

  for(auto& staticMesh : m_level->m_staticMeshes)
  {
    loader::file::RenderMeshDataCompositor compositor;
    compositor.append(*m_meshesDirect.at(staticMesh.mesh)->meshData);
    staticMesh.renderMesh = compositor.toMesh(*getPresenter().getMaterialManager(), false, {});
  }

  for(size_t i = 0; i < m_level->m_rooms.size(); ++i)
  {
    m_level->m_rooms[i].createSceneNode(i, *m_level, *m_textureAnimator, *getPresenter().getMaterialManager());
    getPresenter().getRenderer().getScene()->addNode(m_level->m_rooms[i].node);
  }

  m_objectManager.createObjects(*this, m_level->m_items);
  if(m_objectManager.getLaraPtr() == nullptr)
  {
    m_cameraController = std::make_unique<CameraController>(this, getPresenter().getRenderer().getCamera(), true);

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
    m_cameraController = std::make_unique<CameraController>(this, getPresenter().getRenderer().getCamera());
  }

  m_positionalEmitters.reserve(m_level->m_soundSources.size());
  for(loader::file::SoundSource& src : m_level->m_soundSources)
  {
    m_positionalEmitters.emplace_back(src.position.toRenderSystem(), getPresenter().getSoundEngine().get());
    auto voice = m_audioEngine->playSoundEffect(src.sound_effect_id, &m_positionalEmitters.back());
    Expects(voice != nullptr);
    voice->setLooping(true);
  }
}

void World::useAlternativeLaraAppearance(const bool withHead)
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

void World::dinoStompEffect(objects::Object& object)
{
  const auto d = object.m_state.position.position.toRenderSystem() - m_cameraController->getPosition();
  const auto absD = glm::abs(d);

  static constexpr auto MaxD = 16 * core::SectorSize.get<float>();
  if(absD.x > MaxD || absD.y > MaxD || absD.z > MaxD)
    return;

  const auto x = (100_len).cast<float>() * (1 - glm::length2(d) / util::square(MaxD));
  m_cameraController->setBounce(x.cast<core::Length>());
}

void World::laraNormalEffect()
{
  m_objectManager.getLara().setCurrentAnimState(loader::file::LaraStateId::Stop);
  m_objectManager.getLara().setRequiredAnimState(loader::file::LaraStateId::Unknown12);
  m_objectManager.getLara().getSkeleton()->setAnim(
    &m_level->m_animations[static_cast<int>(loader::file::AnimationId::STAY_SOLID)], 185_frame);
  m_cameraController->setMode(CameraMode::Chase);
  getPresenter().getRenderer().getCamera()->setFieldOfView(glm::radians(80.0f));
}

void World::laraBubblesEffect(objects::Object& object)
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

void World::finishLevelEffect()
{
  m_levelFinished = true;
}

void World::earthquakeEffect()
{
  switch(m_effectTimer.get())
  {
  case 0:
    m_audioEngine->playSoundEffect(TR1SoundEffect::Explosion1, nullptr);
    m_cameraController->setBounce(-250_len);
    break;
  case 3: m_audioEngine->playSoundEffect(TR1SoundEffect::RollingBall, nullptr); break;
  case 35: m_audioEngine->playSoundEffect(TR1SoundEffect::Explosion1, nullptr); break;
  case 20:
  case 50:
  case 70: m_audioEngine->playSoundEffect(TR1SoundEffect::TRexFootstep, nullptr); break;
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

void World::floodEffect()
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
    m_audioEngine->playSoundEffect(TR1SoundEffect::WaterFlow3, pos.toRenderSystem());
  }
  else
  {
    m_activeEffect.reset();
  }
  m_effectTimer += 1_frame;
}

void World::chandelierEffect()
{
  m_audioEngine->playSoundEffect(TR1SoundEffect::GlassyFlow, nullptr);
  m_activeEffect.reset();
}

void World::raisingBlockEffect()
{
  m_effectTimer += 1_frame;
  if(m_effectTimer == 5_frame)
  {
    m_audioEngine->playSoundEffect(TR1SoundEffect::Clank, nullptr);
    m_activeEffect.reset();
  }
}

void World::stairsToSlopeEffect()
{
  if(m_effectTimer > 120_frame)
  {
    m_activeEffect.reset();
  }
  else
  {
    if(m_effectTimer == 0_frame)
    {
      m_audioEngine->playSoundEffect(TR1SoundEffect::HeavyDoorSlam, nullptr);
    }
    auto pos = m_cameraController->getLookAt().position;
    pos.Y += 100_spd * m_effectTimer;
    m_audioEngine->playSoundEffect(TR1SoundEffect::FlowingAir, pos.toRenderSystem());
  }
  m_effectTimer += 1_frame;
}

void World::sandEffect()
{
  if(m_effectTimer <= 120_frame)
  {
    m_audioEngine->playSoundEffect(TR1SoundEffect::LowHum, nullptr);
  }
  else
  {
    m_activeEffect.reset();
  }
  m_effectTimer += 1_frame;
}

void World::explosionEffect()
{
  m_audioEngine->playSoundEffect(TR1SoundEffect::LowPitchedSettling, nullptr);
  m_cameraController->setBounce(-75_len);
  m_activeEffect.reset();
}

// ReSharper disable once CppMemberFunctionMayBeConst
void World::laraHandsFreeEffect()
{
  m_objectManager.getLara().setHandStatus(objects::HandStatus::None);
}

void World::flipMapEffect()
{
  swapAllRooms();
}

void World::chainBlockEffect()
{
  if(m_effectTimer == 0_frame)
  {
    m_audioEngine->playSoundEffect(TR1SoundEffect::SecretFound, nullptr);
  }
  m_effectTimer += 1_frame;
  if(m_effectTimer == 55_frame)
  {
    m_audioEngine->playSoundEffect(TR1SoundEffect::LaraFallIntoWater, nullptr);
    m_activeEffect.reset();
  }
}

void World::flickerEffect()
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

void World::swapWithAlternate(loader::file::Room& orig, loader::file::Room& alternate)
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
    else if(const auto tmp2 = std::dynamic_pointer_cast<objects::TallBlock>(object.get()))
    {
      loader::file::Room::patchHeightsForBlock(*tmp2, core::SectorSize * 2);
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
    else if(const auto tmp2 = std::dynamic_pointer_cast<objects::TallBlock>(object.get()))
    {
      loader::file::Room::patchHeightsForBlock(*tmp2, -core::SectorSize * 2);
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

std::shared_ptr<objects::PickupObject> World::createPickup(const core::TypeId type,
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
    this, "pickup", room, item, &sprite, getPresenter().getMaterialManager()->getSprite());

  m_objectManager.registerDynamicObject(object);
  addChild(room->node, object->getNode());

  return object;
}

void World::doGlobalEffect()
{
  if(m_activeEffect.has_value())
    runEffect(*m_activeEffect, nullptr);

  m_audioEngine->setUnderwater(m_cameraController->getCurrentRoom()->isWaterRoom());
}

const loader::file::Animation& World::getAnimation(loader::file::AnimationId id) const
{
  return m_level->m_animations.at(static_cast<int>(id));
}

const std::vector<loader::file::CinematicFrame>& World::getCinematicFrames() const
{
  return m_level->m_cinematicFrames;
}

const std::vector<loader::file::Camera>& World::getCameras() const
{
  return m_level->m_cameras;
}

const std::vector<int16_t>& World::getAnimCommands() const
{
  return m_level->m_animCommands;
}

void World::update(const bool godMode)
{
  m_objectManager.update(*this, godMode);

  static constexpr auto UVAnimTime = 10_frame;

  m_uvAnimTime += 1_frame;
  if(m_uvAnimTime >= UVAnimTime)
  {
    m_textureAnimator->updateCoordinates(m_level->m_textureTiles);
    m_uvAnimTime -= UVAnimTime;
  }

  m_pickupWidgets.erase(std::remove_if(m_pickupWidgets.begin(),
                                       m_pickupWidgets.end(),
                                       [](const ui::PickupWidget& w) { return w.expired(); }),
                        m_pickupWidgets.end());
  for(auto& w : m_pickupWidgets)
    w.nextFrame();
}

void World::runEffect(const size_t id, objects::Object* object)
{
  switch(id)
  {
  case 0: Expects(object != nullptr); return turn180Effect(*object);
  case 1: Expects(object != nullptr); return dinoStompEffect(*object);
  case 2: return laraNormalEffect();
  case 3: Expects(object != nullptr); return laraBubblesEffect(*object);
  case 4: return finishLevelEffect();
  case 5: return earthquakeEffect();
  case 6: return floodEffect();
  case 7: return chandelierEffect();
  case 8: return raisingBlockEffect();
  case 9: return stairsToSlopeEffect();
  case 10: return sandEffect();
  case 11: return explosionEffect();
  case 12: return laraHandsFreeEffect();
  case 13: return flipMapEffect();
  case 14:
    Expects(object != nullptr);
    if(const auto m = dynamic_cast<objects::ModelObject*>(object))
      return unholsterRightGunEffect(*m);
    break;
  case 15: return chainBlockEffect();
  case 16: return flickerEffect();
  default: BOOST_LOG_TRIVIAL(warning) << "Unhandled effect: " << id;
  }
}

const std::vector<int16_t>& World::getPoseFrames() const
{
  return m_level->m_poseFrames;
}

const std::vector<loader::file::Animation>& World::getAnimations() const
{
  return m_level->m_animations;
}

const std::vector<uint16_t>& World::getOverlaps() const
{
  return m_level->m_overlaps;
}

const std::unique_ptr<loader::file::SkeletalModelType>& World::findAnimatedModelForType(core::TypeId type) const
{
  return m_level->findAnimatedModelForType(type);
}

const engine::floordata::FloorData& World::getFloorData() const
{
  return m_level->m_floorData;
}

gsl::not_null<std::shared_ptr<loader::file::RenderMeshData>> World::getRenderMesh(const size_t idx) const
{
  return m_level->m_meshes.at(idx).meshData;
}

const std::vector<loader::file::Mesh>& World::getMeshes() const
{
  return m_level->m_meshes;
}

std::array<gl::SRGBA8, 256> World::getPalette() const
{
  std::array<gl::SRGBA8, 256> result;
  for(size_t i = 0; i < 256; ++i)
    result[i] = m_level->m_palette->colors[i].toTextureColor();
  return result;
}

const loader::file::Palette& World::getRawPalette() const
{
  return *m_level->m_palette;
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void World::turn180Effect(objects::Object& object)
{
  object.m_state.rotation.Y += 180_deg;
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void World::unholsterRightGunEffect(const objects::ModelObject& object)
{
  const auto& src = *findAnimatedModelForType(TR1ItemId::LaraPistolsAnim);
  BOOST_ASSERT(src.bones.size() == object.getSkeleton()->getBoneCount());
  object.getSkeleton()->setMeshPart(10, src.bones[10].mesh);
  object.getSkeleton()->rebuildMesh();
}

void World::handleCommandSequence(const floordata::FloorDataValue* floorData, const bool fromHeavy)
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
    auto evalResult
      = evaluateCondition(chunkHeader.sequenceCondition, activationRequest, m_objectManager, floorData, switchIsOn);
    if(!evalResult.has_value())
      return;

    conditionFulfilled = evalResult.value();
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
      activateCommand(*object, activationRequest, chunkHeader.sequenceCondition);
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
      swapRooms = flipMapCommand(m_mapFlipActivationStates.at(command.parameter),
                                 activationRequest,
                                 chunkHeader.sequenceCondition,
                                 m_roomsAreSwapped);
      break;
    case floordata::CommandOpcode::FlipOn:
      BOOST_ASSERT(command.parameter < m_mapFlipActivationStates.size());
      if(!m_roomsAreSwapped && m_mapFlipActivationStates[command.parameter].isFullyActivated())
        swapRooms = true;
      break;
    case floordata::CommandOpcode::FlipOff:
      BOOST_ASSERT(command.parameter < m_mapFlipActivationStates.size());
      if(m_roomsAreSwapped && m_mapFlipActivationStates[command.parameter].isFullyActivated())
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

core::TypeId World::find(const loader::file::SkeletalModelType* model) const
{
  auto it = std::find_if(m_level->m_animatedModels.begin(),
                         m_level->m_animatedModels.end(),
                         [&model](const auto& item) { return item.second.get() == model; });
  if(it != m_level->m_animatedModels.end())
    return it->first;

  BOOST_THROW_EXCEPTION(std::runtime_error("Cannot find model"));
}

core::TypeId World::find(const loader::file::Sprite* sprite) const
{
  auto it = std::find_if(
    m_level->m_spriteSequences.begin(), m_level->m_spriteSequences.end(), [&sprite](const auto& sequence) {
      return !sequence.second->sprites.empty() && &sequence.second->sprites[0] == sprite;
    });
  if(it != m_level->m_spriteSequences.end())
    return it->first;

  BOOST_THROW_EXCEPTION(std::runtime_error("Cannot find sprite"));
}

void World::serialize(const serialization::Serializer<World>& ser)
{
  if(ser.loading)
  {
    getPresenter().getRenderer().getScene()->clear();
    for(auto& room : m_level->m_rooms)
    {
      room.resetScenery();
      getPresenter().getRenderer().getScene()->addNode(room.node);
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
      S_NV("mapFlipActivationStates", m_mapFlipActivationStates),
      S_NV("cameras", serialization::FrozenVector{m_level->m_cameras}),
      S_NV("activeEffect", m_activeEffect),
      S_NV("effectTimer", m_effectTimer),
      S_NV("cameraController", *m_cameraController),
      S_NV("secretsFound", m_secretsFoundBitmask),
      S_NV("roomsAreSwapped", m_roomsAreSwapped),
      S_NV("roomOrder", m_roomOrder),
      S_NV("rooms", serialization::FrozenVector{m_level->m_rooms}),
      S_NV("boxes", serialization::FrozenVector{m_level->m_boxes}));

  if(ser.loading)
    m_level->updateRoomBasedCaches();
}

void World::gameLoop(const std::string& levelName, bool godMode)
{
  ui::Ui ui{getPresenter().getMaterialManager()->getScreenSpriteTextured(),
            getPresenter().getMaterialManager()->getScreenSpriteColorRect(),
            getPalette()};

  if(!levelName.empty())
    getPresenter().drawLevelName(ui, levelName);

  update(godMode);

  const auto waterEntryPortals = m_cameraController->update();
  doGlobalEffect();
  getPresenter().drawBars(ui, *m_level->m_palette, getObjectManager());
  if(getObjectManager().getLara().getHandStatus() == engine::objects::HandStatus::Combat
     && getObjectManager().getLara().gunType != WeaponId::Pistols)
  {
    size_t n = 0;
    std::string suffix;
    switch(getObjectManager().getLara().gunType)
    {
    case WeaponId::Shotgun:
      n = m_inventory.getAmmo(WeaponId::Shotgun)->ammo / 6;
      suffix = " A";
      break;
    case WeaponId::Magnums:
      n = m_inventory.getAmmo(WeaponId::Magnums)->ammo;
      suffix = " B";
      break;
    case WeaponId::Uzis:
      n = m_inventory.getAmmo(WeaponId::Uzis)->ammo;
      suffix = " C";
      break;
    }
    auto text = ui::Label{{-17, 22}, ui::makeAmmoString(std::to_string(n) + suffix)};
    text.alignX = ui::Label::Alignment::Right;
    text.draw(ui, getPresenter().getTrFont(), getPresenter().getViewport());
  }

  drawPickupWidgets(ui);
  getPresenter().renderWorld(ui, getObjectManager(), getRooms(), getCameraController(), waterEntryPortals);
}

bool World::cinematicLoop()
{
  update(false);

  const auto waterEntryPortals
    = m_cameraController->updateCinematic(m_level->m_cinematicFrames[m_cameraController->m_cinematicFrame], false);
  doGlobalEffect();

  ui::Ui ui{getPresenter().getMaterialManager()->getScreenSpriteTextured(),
            getPresenter().getMaterialManager()->getScreenSpriteColorRect(),
            getPalette()};
  getPresenter().renderWorld(ui, getObjectManager(), getRooms(), getCameraController(), waterEntryPortals);
  if(++m_cameraController->m_cinematicFrame >= m_level->m_cinematicFrames.size())
    return false;
  return true;
}

void World::load(const std::filesystem::path& filename)
{
  getPresenter().drawLoadingScreen(m_engine.i18n(I18n::Loading));
  BOOST_LOG_TRIVIAL(info) << "Load";
  serialization::YAMLDocument<true> doc{m_engine.getSavegamePath() / filename};
  SavegameMeta meta{};
  doc.load("meta", meta, meta);
  if(meta.filename != std::filesystem::relative(m_level->getFilename(), m_engine.getRootPath()))
  {
    BOOST_LOG_TRIVIAL(error) << "Savegame mismatch. File is for " << meta.filename << ", but current level is "
                             << m_level->getFilename();
    return;
  }
  doc.load("data", *this, *this);
  m_level->updateRoomBasedCaches();
}

void World::save(const std::filesystem::path& filename)
{
  getPresenter().drawLoadingScreen(m_engine.i18n(I18n::Saving));
  BOOST_LOG_TRIVIAL(info) << "Save";
  serialization::YAMLDocument<false> doc{m_engine.getSavegamePath() / filename};
  SavegameMeta meta{std::filesystem::relative(m_level->getFilename(), m_engine.getRootPath()).string(), m_title};
  doc.save("meta", meta, meta);
  doc.save("data", *this, *this);
  doc.write();
}

std::map<size_t, SavegameMeta> World::getSavedGames() const
{
  std::map<size_t, SavegameMeta> result;
  for(size_t i = 0; i < 100; ++i)
  {
    const auto path = m_engine.getSavegamePath() / makeSavegameFilename(i);
    if(!std::filesystem::is_regular_file(path))
      continue;

    serialization::YAMLDocument<true> doc{path};
    SavegameMeta meta{};
    doc.load("meta", meta, meta);
    result.emplace(i, std::move(meta));
  }
  return result;
}

bool World::hasSavedGames() const
{
  for(size_t i = 0; i < 100; ++i)
  {
    const auto path = m_engine.getSavegamePath() / makeSavegameFilename(i);
    if(!std::filesystem::is_regular_file(path))
      continue;

    return true;
  }
  return false;
}

namespace
{
void remapRange(loader::file::UVCoordinates& co,
                const glm::vec2& rangeAMin,
                const glm::vec2& rangeAMax,
                const glm::vec2& rangeBMin,
                const glm::vec2& rangeBMax)
{
  auto value = co.toGl();
  value -= rangeAMin;
  value /= rangeAMax - rangeAMin;
  value *= rangeBMax - rangeBMin;
  value += rangeBMin;
  BOOST_ASSERT(value.x >= 0 && value.x <= 1);
  BOOST_ASSERT(value.y >= 0 && value.y <= 1);
  co.set(value);
}

void remap(loader::file::TextureTile& tile,
           size_t atlas,
           const glm::vec2& replacementUvPos,
           const glm::vec2& replacementUvMax)
{
  tile.textureKey.tileAndFlag &= ~loader::file::TextureIndexMask;
  tile.textureKey.tileAndFlag |= atlas;

  const auto [tileUvMin, tileUvMax] = tile.getMinMaxUv();
  const auto tileUvSize = tileUvMax - tileUvMin;
  if(tileUvSize.x == 0 || tileUvSize.y == 0)
    return;

  for(auto& uvComponent : tile.uvCoordinates)
  {
    if(uvComponent.x.get() == 0 && uvComponent.y.get() == 0)
      continue;

    remapRange(uvComponent, tileUvMin, tileUvMax, replacementUvPos, replacementUvMax);
  }
}
void remap(loader::file::Sprite& sprite,
           size_t atlas,
           const glm::vec2& replacementUvPos,
           const glm::vec2& replacementUvMax)
{
  sprite.texture_id = core::TextureId{atlas};

  // re-map uv coordinates
  const auto a = sprite.uv0.toNearestGl(256);
  const auto b = sprite.uv1.toNearestGl(256);

  remapRange(sprite.uv0, a, b, replacementUvPos, replacementUvMax);
  remapRange(sprite.uv1, a, b, replacementUvPos, replacementUvMax);
}
} // namespace

World::World(Engine& engine,
             std::unique_ptr<loader::file::level::Level>&& level,
             std::string title,
             const std::optional<TR1TrackId>& track,
             bool useAlternativeLara,
             std::unordered_map<TR1ItemId, size_t> initialInventory,
             std::unordered_map<std::string, std::unordered_map<TR1ItemId, std::string>> itemTitles)
    : m_engine{engine}
    , m_audioEngine{std::make_unique<AudioEngine>(
        *this, engine.getRootPath() / "data" / "tr1" / "audio", engine.getPresenter().getSoundEngine())}
    , m_level{std::move(level)}
    , m_title{std::move(title)}
    , m_itemTitles{std::move(itemTitles)}
{
  {
    if(const auto tbl = core::get<pybind11::dict>(
         core::get<pybind11::dict>(pybind11::globals(), "cheats").value_or(pybind11::dict{}), "inventory"))
    {
      for(const auto& [type, qty] : *tbl)
        initialInventory[type.cast<TR1ItemId>()] += qty.cast<size_t>();
    }

    getPresenter().drawLoadingScreen(m_engine.i18n(I18n::BuildingTextures));
    for(auto& texture : m_level->m_textures)
    {
      texture.toImage();
    }

    BOOST_LOG_TRIVIAL(info) << "Building texture atlases";

    std::unordered_set<loader::file::TextureTile*> doneTiles;
    std::unordered_set<loader::file::Sprite*> doneSprites;

    render::MultiTextureAtlas atlases{2048};
    const auto atlasUvScale = 256.0f / gsl::narrow_cast<float>(atlases.getSize());
    bool hasGlidosPack = false;
    if(const auto& glidos = m_engine.getGlidos())
    {
      hasGlidosPack = true;

      for(size_t texIdx = 0; texIdx < m_level->m_textures.size(); ++texIdx)
      {
        const auto& texture = m_level->m_textures[texIdx];
        const auto mappings = glidos->getMappingsForTexture(texture.md5);

        for(const auto& [tile, path] : mappings.tiles)
        {
          std::unique_ptr<gl::CImgWrapper> replacementImg;
          if(path.empty() || !std::filesystem::is_regular_file(path))
          {
            replacementImg = std::make_unique<gl::CImgWrapper>(
              // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
              reinterpret_cast<const uint8_t*>(texture.image->getRawData()),
              256,
              256,
              true);
            replacementImg->crop(tile.getX0(), tile.getY0(), tile.getX1(), tile.getY1());
          }
          else
          {
            replacementImg = std::make_unique<gl::CImgWrapper>(path);
          }

          auto [page, replacementPos] = atlases.put(*replacementImg);
          const auto replacementUvPos = glm::vec2{replacementPos} / gsl::narrow_cast<float>(atlases.getSize());
          const auto replacementUvMax = replacementUvPos
                                        + glm::vec2{replacementImg->width() - 1, replacementImg->height() - 1}
                                            / gsl::narrow_cast<float>(atlases.getSize());

          bool remapped = false;
          for(auto& srcTile : m_level->m_textureTiles)
          {
            if(doneTiles.count(&srcTile) != 0)
              continue;

            if((srcTile.textureKey.tileAndFlag & loader::file::TextureIndexMask) != texIdx)
              continue;

            const auto [min, max] = srcTile.getMinMaxPx(256);
            if(!tile.contains(min.x, min.y) || !tile.contains(max.x, max.y))
              continue;

            doneTiles.emplace(&srcTile);
            remapped = true;
            remap(srcTile, page, replacementUvPos, replacementUvMax);
          }

          for(auto& sprite : m_level->m_sprites)
          {
            if(doneSprites.count(&sprite) != 0)
              continue;

            if(sprite.texture_id.get() != texIdx)
              continue;

            const auto a = sprite.uv0.toPx(256);
            const auto b = sprite.uv1.toPx(256);
            if(!tile.contains(a.x, a.y) || !tile.contains(b.x, b.y))
              continue;

            doneSprites.emplace(&sprite);
            remapped = true;
            remap(sprite, page, replacementUvPos, replacementUvMax);
          }

          if(!remapped)
          {
            BOOST_LOG_TRIVIAL(error) << "Failed to re-map texture tile " << tile;
          }
        }
      }

      BOOST_LOG_TRIVIAL(debug) << "Re-mapped " << doneTiles.size() << " tiles and " << doneSprites.size() << " sprites";
    }

    struct SourceTile final
    {
      int textureId;
      std::pair<glm::ivec2, glm::ivec2> px;

      bool operator<(const SourceTile& rhs) const noexcept
      {
        if(textureId != rhs.textureId)
          return textureId < rhs.textureId;

        if(px.first.x != rhs.px.first.x)
          return px.first.x < rhs.px.first.x;
        if(px.first.y != rhs.px.first.y)
          return px.first.y < rhs.px.first.y;
        if(px.second.x != rhs.px.second.x)
          return px.second.x < rhs.px.second.x;
        return px.second.y < rhs.px.second.y;
      }

      bool operator==(const SourceTile& rhs) const noexcept
      {
        return textureId == rhs.textureId && px == rhs.px;
      }
    };

    std::map<SourceTile, std::pair<size_t, glm::ivec2>> replaced;

    std::vector<loader::file::TextureTile*> tilesOrderedBySize;
    for(auto& tile : m_level->m_textureTiles)
      tilesOrderedBySize.emplace_back(&tile);

    std::sort(tilesOrderedBySize.begin(),
              tilesOrderedBySize.end(),
              [](loader::file::TextureTile* a, loader::file::TextureTile* b) {
                const auto aDims = a->getMinMaxUv();
                const auto aSize = aDims.second - aDims.first;
                const auto aArea = glm::abs(aSize.x * aSize.y);
                const auto bDims = b->getMinMaxUv();
                const auto bSize = bDims.second - bDims.first;
                const auto bArea = glm::abs(bSize.x * bSize.y);
                return aArea > bArea;
              });

    for(auto* tile : tilesOrderedBySize)
    {
      if(!doneTiles.emplace(tile).second)
        continue;
      auto textureId = tile->textureKey.tileAndFlag & loader::file::TextureIndexMask;
      const auto srcPxDims = tile->getMinMaxPx(256);
      const SourceTile srcTile{textureId, srcPxDims};
      auto it = replaced.find(srcTile);
      std::pair<size_t, glm::ivec2> replacementPos;
      if(it == replaced.end())
      {
        const auto& texture = m_level->m_textures.at(textureId);
        auto replacementImg = std::make_unique<gl::CImgWrapper>(
          // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
          reinterpret_cast<const uint8_t*>(texture.image->getRawData()),
          256,
          256,
          true);
        replacementImg->crop(srcPxDims.first.x, srcPxDims.first.y, srcPxDims.second.x, srcPxDims.second.y);

        replacementPos = atlases.put(*replacementImg);
        replaced[srcTile] = replacementPos;
      }
      else
      {
        replacementPos = it->second;
      }

      const auto srcUvDims = tile->getMinMaxUv();
      const auto replacementUvPos = glm::vec2{replacementPos.second} / gsl::narrow_cast<float>(atlases.getSize());
      remap(*tile,
            replacementPos.first,
            replacementUvPos,
            replacementUvPos + (srcUvDims.second - srcUvDims.first) * atlasUvScale);
    }

    std::vector<loader::file::Sprite*> spritesOrderedBySize;
    for(auto& sprite : m_level->m_sprites)
      spritesOrderedBySize.emplace_back(&sprite);

    std::sort(
      spritesOrderedBySize.begin(), spritesOrderedBySize.end(), [](loader::file::Sprite* a, loader::file::Sprite* b) {
        const auto aSize = a->uv1.toGl() - a->uv1.toGl();
        const auto aArea = glm::abs(aSize.x * aSize.y);
        const auto bSize = b->uv1.toGl() - b->uv1.toGl();
        const auto bArea = glm::abs(bSize.x * bSize.y);
        return aArea > bArea;
      });

    for(auto* sprite : spritesOrderedBySize)
    {
      if(!doneSprites.emplace(sprite).second)
        continue;

      std::pair minMaxPx{sprite->uv0.toPx(256), sprite->uv1.toPx(256)};

      const SourceTile srcTile{sprite->texture_id.get(), minMaxPx};
      auto it = replaced.find(srcTile);
      std::pair<size_t, glm::ivec2> replacementPos;

      if(it == replaced.end())
      {
        const auto& texture = m_level->m_textures.at(sprite->texture_id.get());
        auto replacementImg = std::make_unique<gl::CImgWrapper>(
          // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
          reinterpret_cast<const uint8_t*>(texture.image->getRawData()),
          256,
          256,
          true);
        replacementImg->crop(minMaxPx.first.x, minMaxPx.first.y, minMaxPx.second.x, minMaxPx.second.y);

        replacementPos = atlases.put(*replacementImg);
        replaced[srcTile] = replacementPos;
      }
      else
      {
        replacementPos = it->second;
      }
      const auto replacementUvPos = glm::vec2{replacementPos.second} / gsl::narrow_cast<float>(atlases.getSize());
      std::pair minMaxUv{sprite->uv0.toGl(), sprite->uv1.toGl()};
      remap(*sprite,
            replacementPos.first,
            replacementUvPos,
            replacementUvPos + (minMaxUv.second - minMaxUv.first) * atlasUvScale);
      sprite->texture_id = replacementPos.first;
    }

    Expects(doneTiles.size() == m_level->m_textureTiles.size());
    Expects(doneSprites.size() == m_level->m_sprites.size());

    const int textureLevels = static_cast<int>(std::log2(atlases.getSize()) + 1) / 2;
    auto images = atlases.takeImages();
    m_allTextures = std::make_unique<gl::Texture2DArray<gl::SRGBA8>>(
      glm::ivec3{atlases.getSize(), atlases.getSize(), gsl::narrow<int>(images.size())}, textureLevels, "all-textures");
    m_allTextures->set(gl::api::TextureMinFilter::NearestMipmapLinear);
    if(hasGlidosPack)
      m_allTextures->set(gl::api::TextureMagFilter::Linear);
    else
      m_allTextures->set(gl::api::TextureMagFilter::Nearest);
    m_allTextures->set(gl::api::TextureParameterName::TextureWrapS, gl::api::TextureWrapMode::ClampToEdge);
    m_allTextures->set(gl::api::TextureParameterName::TextureWrapT, gl::api::TextureWrapMode::ClampToEdge);
    getPresenter().getMaterialManager()->setGeometryTextures(m_allTextures);

    for(size_t i = 0; i < images.size(); ++i)
      m_allTextures->assign(images[i]->pixels<gl::SRGBA8>().data(), gsl::narrow_cast<int>(i), 0);
    createMipmaps(images, textureLevels);

    m_textureAnimator = std::make_unique<render::TextureAnimator>(m_level->m_animatedTextures);

    m_audioEngine->init(m_level->m_soundEffectProperties, m_level->m_soundEffects);
    while(m_roomOrder.size() < m_level->m_rooms.size())
      m_roomOrder.emplace_back(m_roomOrder.size());

    BOOST_LOG_TRIVIAL(info) << "Loading samples...";

    for(const auto offset : m_level->m_sampleIndices)
    {
      Expects(offset < m_level->m_samplesData.size());
      m_audioEngine->addWav(&m_level->m_samplesData[offset]);
    }

    getPresenter().drawLoadingScreen(util::unescape(m_title));
    loadSceneData();

    if(useAlternativeLara)
    {
      useAlternativeLaraAppearance();
    }

    if(m_objectManager.getLaraPtr() != nullptr)
    {
      for(const auto& [item, qty] : initialInventory)
      {
        if(m_level->findAnimatedModelForType(item) != nullptr)
          m_inventory.put(item, qty);
      }
    }
  }

  getPresenter().getSoundEngine()->setListener(m_cameraController.get());
  getPresenter().setTrFont(std::make_unique<ui::TRFont>(*m_level->m_spriteSequences.at(TR1ItemId::FontGraphics)));
  if(track.has_value())
    m_audioEngine->playStopCdTrack(track.value(), false);
}

World::~World() = default;

void World::createMipmaps(const std::vector<std::shared_ptr<gl::CImgWrapper>>& images, size_t nMips)
{
  std::map<int, std::set<UVRect>> tilesByTexture;
  BOOST_LOG_TRIVIAL(debug) << m_level->m_textureTiles.size() << " total texture tiles";
  for(const auto& tile : m_level->m_textureTiles)
  {
    tilesByTexture[tile.textureKey.tileAndFlag & loader::file::TextureIndexMask].emplace(tile.uvCoordinates);
  }
  for(const auto& sprite : m_level->m_sprites)
  {
    tilesByTexture[sprite.texture_id.get()].emplace(sprite.uv0, sprite.uv1);
  }

  size_t totalTiles = std::accumulate(
    tilesByTexture.begin(), tilesByTexture.end(), std::size_t{0}, [](size_t n, const auto& textureAndTiles) {
      return n + textureAndTiles.second.size();
    });
  BOOST_LOG_TRIVIAL(debug) << totalTiles << " unique texture tiles";

  size_t processedTiles = 0;
  for(const auto& [texture, tiles] : tilesByTexture)
  {
    auto src = *images.at(texture);
    Expects(src.width() == src.height());

    BOOST_LOG_TRIVIAL(debug) << "Mipmapping texture " << texture;

    auto dstSize = src.width() / 2;
    auto margin = render::MultiTextureAtlas::BoundaryMargin / 2;
    for(int mipmapLevel = 1; static_cast<size_t>(mipmapLevel) < nMips; dstSize /= 2, margin /= 2, ++mipmapLevel)
    {
      getPresenter().drawLoadingScreen(
        m_engine.i18n(I18n::CreatingMipmaps, processedTiles * 100 / (totalTiles * (nMips - 1))));
      processedTiles += tiles.size();

      BOOST_LOG_TRIVIAL(debug) << "Mipmap level " << mipmapLevel << " (size " << dstSize << ", " << tiles.size()
                               << " tiles)";
      src.resizePow2Mipmap(1);
      m_allTextures->assign(src.pixels<gl::SRGBA8>().data(), texture, mipmapLevel);
    }
  }
}

void World::drawPickupWidgets(ui::Ui& ui)
{
  auto& img = *getPresenter().getScreenOverlay().getImage();
  auto x = img.getWidth() * 9 / 10;
  auto y = img.getHeight() * 9 / 10;
  auto widthPerWidget = img.getWidth() / 10 * 4 / 3;
  for(const auto& widget : m_pickupWidgets)
  {
    if(widget.expired())
      continue;

    widget.draw(ui, x, y);
    x -= widthPerWidget;
  }
}

const Presenter& World::getPresenter() const
{
  return m_engine.getPresenter();
}

Presenter& World::getPresenter()
{
  return m_engine.getPresenter();
}

std::optional<std::string> World::getItemTitle(TR1ItemId id) const
{
  if(auto langIt = m_itemTitles.find(m_engine.getLanguage()); langIt != m_itemTitles.end())
  {
    if(auto itemIt = langIt->second.find(id); itemIt != langIt->second.end())
    {
      return itemIt->second;
    }
  }
  if(auto langIt = m_itemTitles.find("en"); langIt != m_itemTitles.end())
  {
    if(auto itemIt = langIt->second.find(id); itemIt != langIt->second.end())
    {
      return itemIt->second;
    }
  }

  return std::nullopt;
}

void World::load(size_t slot)
{
  load(makeSavegameFilename(slot));
}

void World::save(size_t slot)
{
  save(makeSavegameFilename(slot));
}

void SavegameMeta::serialize(const serialization::Serializer<SavegameMeta>& ser)
{
  ser(S_NV("filename", filename), S_NV("title", title));
}
} // namespace engine
