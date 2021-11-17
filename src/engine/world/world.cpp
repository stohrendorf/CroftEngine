#include "world.h"

#include "animation.h"
#include "atlastile.h"
#include "audio/soundengine.h"
#include "audio/voice.h"
#include "box.h"
#include "camerasink.h"
#include "cinematicframe.h"
#include "core/containeroffset.h"
#include "core/i18n.h"
#include "core/interval.h"
#include "core/magic.h"
#include "engine/ai/pathfinder.h"
#include "engine/audioengine.h"
#include "engine/audiosettings.h"
#include "engine/cameracontroller.h"
#include "engine/displaysettings.h"
#include "engine/engine.h"
#include "engine/engineconfig.h"
#include "engine/floordata/floordata.h"
#include "engine/inventory.h"
#include "engine/location.h"
#include "engine/objects/aiagent.h"
#include "engine/objects/block.h" // IWYU pragma: keep
#include "engine/objects/laraobject.h"
#include "engine/objects/modelobject.h"
#include "engine/objects/objectstate.h"
#include "engine/objects/pickupobject.h"
#include "engine/objects/tallblock.h" // IWYU pragma: keep
#include "engine/particle.h"
#include "engine/player.h"
#include "engine/presenter.h"
#include "engine/skeletalmodelnode.h"
#include "engine/soundeffects_tr1.h"
#include "engine/tracks_tr1.h"
#include "engine/weapontype.h"
#include "loader/file/animation.h"
#include "loader/file/animationid.h"
#include "loader/file/audio.h"
#include "loader/file/color.h"
#include "loader/file/datatypes.h"
#include "loader/file/larastateid.h"
#include "loader/file/level/level.h"
#include "loader/file/mesh.h"
#include "loader/file/meshes.h"
#include "loader/file/texture.h"
#include "mesh.h"
#include "qs/qs.h"
#include "render/rendersettings.h"
#include "render/scene/camera.h"
#include "render/scene/materialmanager.h"
#include "render/scene/mesh.h"
#include "render/scene/node.h"
#include "render/scene/renderer.h"
#include "render/scene/sprite.h"
#include "render/textureanimator.h"
#include "render/textureatlas.h"
#include "rendermeshdata.h"
#include "room.h"
#include "sector.h"
#include "serialization/array.h"
#include "serialization/bitset.h"
#include "serialization/optional.h"
#include "serialization/quantity.h"
#include "serialization/serialization.h"
#include "serialization/vector.h"
#include "serialization/yamldocument.h"
#include "skeletalmodeltype.h"
#include "sprite.h"
#include "staticmesh.h"
#include "staticsoundeffect.h"
#include "texturing.h"
#include "transition.h"
#include "ui/core.h"
#include "ui/text.h"
#include "ui/ui.h"
#include "util/fsutil.h"
#include "util/helpers.h"

#include <algorithm>
#include <boost/log/trivial.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/throw_exception.hpp>
#include <cstdint>
#include <exception>
#include <gl/glad_init.h>
#include <gl/pixel.h>
#include <gl/sampler.h>
#include <gl/texture2darray.h>
#include <gl/texturehandle.h>
#include <glm/gtx/norm.hpp>
#include <glm/vec2.hpp>
#include <gslu.h>
#include <iterator>
#include <set>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace engine::world
{
namespace
{
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
    object.m_state.touch_bits.reset();
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

bool evaluateCondition(floordata::SequenceCondition condition,
                       const floordata::ActivationState& request,
                       const ObjectManager& objectManager,
                       const floordata::FloorDataValue*& floorData,
                       bool& switchIsOn)
{
  switch(condition)
  {
  case floordata::SequenceCondition::LaraIsHere:
    return true;
  case floordata::SequenceCondition::LaraOnGround:
  case floordata::SequenceCondition::LaraOnGroundInverted:
    return objectManager.getLara().m_state.location.position.Y == objectManager.getLara().m_state.floor;
  case floordata::SequenceCondition::ItemActivated:
  {
    auto swtch = objectManager.getObject(floordata::Command{*floorData++}.parameter);
    Expects(swtch != nullptr);
    if(!swtch->triggerSwitch(request.getTimeout()))
      return false;

    switchIsOn = (swtch->m_state.current_anim_state == 1_as);
    return true;
  }
  case floordata::SequenceCondition::KeyUsed:
  {
    auto key = objectManager.getObject(floordata::Command{*floorData++}.parameter);
    Expects(key != nullptr);
    return key->triggerKey();
  }
  case floordata::SequenceCondition::ItemPickedUp:
  {
    auto item = objectManager.getObject(floordata::Command{*floorData++}.parameter);
    Expects(item != nullptr);
    return item->triggerPickUp();
  }
  case floordata::SequenceCondition::LaraInCombatMode:
    return objectManager.getLara().getHandStatus() == objects::HandStatus::Combat;
  case floordata::SequenceCondition::ItemIsHere:
  case floordata::SequenceCondition::Dummy:
    return false;
  default:
    return true;
  }
}
} // namespace

void World::swapAllRooms()
{
  BOOST_LOG_TRIVIAL(info) << "Swapping rooms";
  for(auto& room : m_rooms)
  {
    if(room.alternateRoom == nullptr)
      continue;

    swapWithAlternate(room, *room.alternateRoom);
  }

  m_roomsAreSwapped = !m_roomsAreSwapped;
  connectSectors();
  updateStaticSoundEffects();
}

bool World::isValid(const loader::file::AnimFrame* frame) const
{
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  return reinterpret_cast<const short*>(frame) >= m_poseFrames.data()
         // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
         && reinterpret_cast<const short*>(frame) < m_poseFrames.data() + m_poseFrames.size();
}

const std::unique_ptr<SpriteSequence>& World::findSpriteSequenceForType(const core::TypeId& type) const
{
  const auto it = m_spriteSequences.find(type);
  if(it != m_spriteSequences.end())
    return it->second;

  static const std::unique_ptr<SpriteSequence> none;
  return none;
}

const StaticMesh* World::findStaticMeshById(const core::StaticMeshId& meshId) const
{
  auto it = m_staticMeshes.find(meshId);
  if(it != m_staticMeshes.end())
    return &it->second;

  return nullptr;
}

const std::vector<Room>& World::getRooms() const
{
  return m_rooms;
}

std::vector<Room>& World::getRooms()
{
  return m_rooms;
}

const std::vector<Box>& World::getBoxes() const
{
  return m_boxes;
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
  const auto d = object.m_state.location.position.toRenderSystem() - m_cameraController->getPosition();
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
  m_objectManager.getLara().getSkeleton()->setAnim(gsl::not_null{&getAnimation(loader::file::AnimationId::STAY_SOLID)});
  m_cameraController->setMode(CameraMode::Chase);
  getPresenter().getRenderer().getCamera()->setFieldOfView(Presenter::DefaultFov);
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

  const auto boneSpheres = modelNode->getSkeleton()->getBoneCollisionSpheres();

  const auto position = core::TRVec{boneSpheres.at(14).relative(core::TRVec{0_len, 0_len, 50_len}.toRenderSystem())};

  while(bubbleCount-- > 0)
  {
    auto particle = gslu::make_nn_shared<BubbleParticle>(Location{object.m_state.location.room, position}, *this);
    setParent(particle, object.m_state.location.room->node);
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
  case 3:
    m_audioEngine->playSoundEffect(TR1SoundEffect::RollingBall, nullptr);
    break;
  case 35:
    m_audioEngine->playSoundEffect(TR1SoundEffect::Explosion1, nullptr);
    break;
  case 20:
  case 50:
  case 70:
    m_audioEngine->playSoundEffect(TR1SoundEffect::TRexFootstep, nullptr);
    break;
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
    auto pos = m_objectManager.getLara().m_state.location.position;
    core::Frame mul = 0_frame;
    if(m_effectTimer >= core::FrameRate * 1_sec)
    {
      mul = m_effectTimer - core::FrameRate * 1_sec;
    }
    else
    {
      mul = core::FrameRate * 1_sec - m_effectTimer;
    }
    pos.Y = 100_len * mul / 1_frame + m_cameraController->getLookAt().position.Y;
    if(m_globalSoundEffect == nullptr)
      m_globalSoundEffect = m_audioEngine->playSoundEffect(TR1SoundEffect::WaterFlow3, pos.toRenderSystem());
    else
      m_globalSoundEffect->setPosition(pos.toRenderSystem());
  }
  else
  {
    m_activeEffect.reset();
    m_globalSoundEffect->stop();
    m_globalSoundEffect.reset();
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
  if(m_effectTimer > core::FrameRate * 4_sec)
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
  if(m_effectTimer <= core::FrameRate * 4_sec)
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

void World::swapWithAlternate(Room& orig, Room& alternate)
{
  // find any blocks in the original room and un-patch the floor heights

  for(const auto& object : m_objectManager.getObjects() | boost::adaptors::map_values)
  {
    if(object->m_state.location.room != &orig)
      continue;

    if(const auto tmp = std::dynamic_pointer_cast<objects::Block>(object.get()))
    {
      patchHeightsForBlock(*tmp, core::SectorSize);
    }
    else if(const auto tmp2 = std::dynamic_pointer_cast<objects::TallBlock>(object.get()))
    {
      patchHeightsForBlock(*tmp2, core::SectorSize * 2);
    }
  }

  // now swap the rooms and patch the alternate room ids
  std::swap(orig, alternate);
  orig.alternateRoom = std::exchange(alternate.alternateRoom, nullptr);

  // patch heights in the new room, and swap object ownerships.
  // note that this is exactly the same code as above,
  // except for the heights.
  for(const auto& object : m_objectManager.getObjects() | boost::adaptors::map_values)
  {
    if(object->m_state.location.room == &orig)
    {
      // although this seems contradictory, remember the nodes have been swapped above
      setParent(gsl::not_null{object->getNode()}, orig.node);
    }
    else if(object->m_state.location.room == &alternate)
    {
      setParent(gsl::not_null{object->getNode()}, alternate.node);
      continue;
    }
    else
    {
      continue;
    }

    if(const auto tmp = std::dynamic_pointer_cast<objects::Block>(object.get()))
    {
      patchHeightsForBlock(*tmp, -core::SectorSize);
    }
    else if(const auto tmp2 = std::dynamic_pointer_cast<objects::TallBlock>(object.get()))
    {
      patchHeightsForBlock(*tmp2, -core::SectorSize * 2);
    }
  }

  for(const auto& object : m_objectManager.getDynamicObjects())
  {
    if(object->m_state.location.room == &orig)
    {
      setParent(gsl::not_null{object->getNode()}, orig.node);
    }
    else if(object->m_state.location.room == &alternate)
    {
      setParent(gsl::not_null{object->getNode()}, alternate.node);
    }
  }
}

gsl::not_null<std::shared_ptr<objects::PickupObject>>
  World::createPickup(const core::TypeId& type, const gsl::not_null<const Room*>& room, const core::TRVec& position)
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
  const Sprite& sprite = spriteSequence->sprites[0];

  auto object = gslu::make_nn_shared<objects::PickupObject>(
    objects::makeObjectName(item.type.get_as<TR1ItemId>(), m_objectManager.getDynamicObjectCount()),
    gsl::not_null{this},
    room,
    item,
    gsl::not_null{&sprite});

  m_objectManager.registerDynamicObject(object);
  addChild(gsl::not_null{room->node}, gsl::not_null{object->getNode()});

  return object;
}

void World::doGlobalEffect()
{
  if(m_activeEffect.has_value())
    runEffect(*m_activeEffect, nullptr);

  m_audioEngine->setUnderwater(m_cameraController->getCurrentRoom()->isWaterRoom);
}

const Animation& World::getAnimation(loader::file::AnimationId id) const
{
  return m_animations.at(static_cast<int>(id));
}

const std::vector<CinematicFrame>& World::getCinematicFrames() const
{
  return m_cinematicFrames;
}

const std::vector<int16_t>& World::getAnimCommands() const
{
  return m_animCommands;
}

void World::update(const bool godMode)
{
  m_objectManager.update(*this, godMode);

  static constexpr auto UVAnimTime = core::FrameRate * 1_sec / 3;

  m_uvAnimTime += 1_frame;
  if(m_uvAnimTime >= UVAnimTime)
  {
    m_textureAnimator->updateCoordinates(m_atlasTiles);
    m_uvAnimTime -= UVAnimTime;
  }

  m_pickupWidgets.erase(std::remove_if(m_pickupWidgets.begin(),
                                       m_pickupWidgets.end(),
                                       [](const ui::PickupWidget& w)
                                       {
                                         return w.expired();
                                       }),
                        m_pickupWidgets.end());
  for(auto& w : m_pickupWidgets)
    w.nextFrame();

  m_audioEngine->cleanup();
}

void World::runEffect(const size_t id, objects::Object* object)
{
  BOOST_LOG_TRIVIAL(trace) << "Global effect " << id;
  switch(id)
  {
  case 0:
    Expects(object != nullptr);
    return turn180Effect(*object);
  case 1:
    Expects(object != nullptr);
    return dinoStompEffect(*object);
  case 2:
    return laraNormalEffect();
  case 3:
    Expects(object != nullptr);
    return laraBubblesEffect(*object);
  case 4:
    return finishLevelEffect();
  case 5:
    return earthquakeEffect();
  case 6:
    return floodEffect();
  case 7:
    return chandelierEffect();
  case 8:
    return raisingBlockEffect();
  case 9:
    return stairsToSlopeEffect();
  case 10:
    return sandEffect();
  case 11:
    return explosionEffect();
  case 12:
    return laraHandsFreeEffect();
  case 13:
    return flipMapEffect();
  case 14:
    Expects(object != nullptr);
    if(const auto m = dynamic_cast<objects::ModelObject*>(object))
      return drawRightWeaponEffect(*m);
    break;
  case 15:
    return chainBlockEffect();
  case 16:
    return flickerEffect();
  default:
    BOOST_LOG_TRIVIAL(warning) << "Unhandled effect: " << id;
  }
}

const std::vector<int16_t>& World::getPoseFrames() const
{
  return m_poseFrames;
}

const std::vector<Animation>& World::getAnimations() const
{
  return m_animations;
}

const std::unique_ptr<SkeletalModelType>& World::findAnimatedModelForType(const core::TypeId& type) const
{
  const auto it = m_animatedModels.find(type);
  if(it != m_animatedModels.end())
    return it->second;

  static const std::unique_ptr<SkeletalModelType> none;
  return none;
}

gsl::not_null<std::shared_ptr<RenderMeshData>> World::getRenderMesh(const size_t idx) const
{
  return m_meshes.at(idx).meshData;
}

const std::vector<Mesh>& World::getMeshes() const
{
  return m_meshes;
}

const std::array<gl::SRGBA8, 256>& World::getPalette() const
{
  return m_palette;
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void World::turn180Effect(objects::Object& object)
{
  object.m_state.rotation.Y += 180_deg;
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void World::drawRightWeaponEffect(const objects::ModelObject& object)
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
      if(m_objectManager.getLara().m_state.location.position.Y == m_objectManager.getLara().m_state.floor)
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

  bool switchIsOn = false;
  if(fromHeavy)
  {
    if(chunkHeader.sequenceCondition != floordata::SequenceCondition::ItemIsHere)
      return;
  }
  else
  {
    if(!evaluateCondition(chunkHeader.sequenceCondition, activationRequest, m_objectManager, floorData, switchIsOn))
      return;
  }

  bool swapRooms = false;
  std::optional<size_t> flipEffect;
  while(true)
  {
    const floordata::Command command{*floorData++};
    switch(command.opcode)
    {
    case floordata::CommandOpcode::Activate:
      if(auto object = m_objectManager.getObject(command.parameter))
        activateCommand(*object, activationRequest, chunkHeader.sequenceCondition);
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
      m_cameraController->setLookAtObject(gsl::not_null{m_objectManager.getObject(command.parameter)});
      break;
    case floordata::CommandOpcode::UnderwaterCurrent:
    {
      const auto& sink = m_cameraSinks.at(command.parameter);
      {
        m_objectManager.getLara().m_underwaterRoute.setTargetBox(gsl::not_null{&m_boxes.at(sink.box_index)});
        auto newTarget = sink.position;
        newTarget.X = m_boxes[sink.box_index].xInterval.clamp(newTarget.X);
        newTarget.Z = m_boxes[sink.box_index].zInterval.clamp(newTarget.Z);
        m_objectManager.getLara().m_underwaterRoute.target = newTarget;
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
    case floordata::CommandOpcode::FlipEffect:
      flipEffect = command.parameter;
      break;
    case floordata::CommandOpcode::EndLevel:
      finishLevel();
      break;
    case floordata::CommandOpcode::PlayTrack:
      m_audioEngine->triggerCdTrack(m_engine.getScriptEngine(),
                                    static_cast<TR1TrackId>(command.parameter),
                                    activationRequest,
                                    chunkHeader.sequenceCondition);
      break;
    case floordata::CommandOpcode::Secret:
      BOOST_ASSERT(command.parameter < 16);
      if(!m_secretsFoundBitmask.test(command.parameter))
      {
        m_secretsFoundBitmask.set(command.parameter);
        m_audioEngine->playStopCdTrack(m_engine.getScriptEngine(), TR1TrackId::Secret, false);
        ++m_player->secrets;
      }
      break;
    default:
      break;
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

core::TypeId World::find(const SkeletalModelType* model) const
{
  auto it = std::find_if(m_animatedModels.begin(),
                         m_animatedModels.end(),
                         [&model](const auto& item)
                         {
                           return item.second.get() == model;
                         });
  if(it != m_animatedModels.end())
    return it->first;

  BOOST_THROW_EXCEPTION(std::runtime_error("Cannot find model"));
}

core::TypeId World::find(const Sprite* sprite) const
{
  auto it = std::find_if(m_spriteSequences.begin(),
                         m_spriteSequences.end(),
                         [&sprite](const auto& sequence)
                         {
                           return !sequence.second->sprites.empty() && &sequence.second->sprites[0] == sprite;
                         });
  if(it != m_spriteSequences.end())
    return it->first;

  BOOST_THROW_EXCEPTION(std::runtime_error("Cannot find sprite"));
}

void World::serialize(const serialization::Serializer<World>& ser)
{
  std::vector<size_t> physicalIds;
  std::transform(m_rooms.begin(),
                 m_rooms.end(),
                 std::back_inserter(physicalIds),
                 [](const Room& room)
                 {
                   return room.physicalId;
                 });

  if(ser.loading)
  {
    getPresenter().getRenderer().getRootNode()->clear();
    for(auto& room : m_rooms)
    {
      room.resetScenery();
      setParent(gsl::not_null{room.node}, getPresenter().getRenderer().getRootNode());
    }

    ser(S_NV("roomPhysicalIds", serialization::FrozenVector{physicalIds}));
    for(size_t i = 0; i < m_rooms.size(); ++i)
    {
      if(m_rooms[i].physicalId == physicalIds[i])
        continue;

      // do not use "swapWithAlternate", as that may break the "alternateRoom" member
      std::swap(m_rooms[i], m_rooms[physicalIds[i]]);
    }
    for(size_t i = 0; i < m_rooms.size(); ++i)
      Ensures(physicalIds[i] == m_rooms[i].physicalId);
  }

  ser(S_NV("objectManager", m_objectManager),
      S_NV("player", *m_player),
      S_NV("mapFlipActivationStates", m_mapFlipActivationStates),
      S_NV("cameras", serialization::FrozenVector{m_cameraSinks}),
      S_NV("activeEffect", m_activeEffect),
      S_NV("effectTimer", m_effectTimer),
      S_NV("cameraController", *m_cameraController),
      S_NV("secretsFound", m_secretsFoundBitmask),
      S_NV("roomsAreSwapped", m_roomsAreSwapped),
      S_NV("roomPhysicalIds", physicalIds),
      S_NV("rooms", serialization::FrozenVector{m_rooms}),
      S_NV("boxes", serialization::FrozenVector{m_boxes}),
      S_NV("audioEngine", *m_audioEngine));

  if(ser.loading)
  {
    updateStaticSoundEffects();
  }
}

void World::gameLoop(bool godMode, float waitRatio, float blackAlpha, ui::Ui& ui)
{
  update(godMode);
  m_player->laraHealth = m_objectManager.getLara().m_state.health;

  const auto waterEntryPortals = m_cameraController->update();
  doGlobalEffect();
  getPresenter().drawBars(ui, m_palette, getObjectManager());

  drawPickupWidgets(ui);
  getPresenter().renderWorld(getObjectManager(), getRooms(), getCameraController(), waterEntryPortals, waitRatio);
  getPresenter().renderScreenOverlay();
  if(blackAlpha > 0)
  {
    ui.drawBox({0, 0}, ui.getSize(), gl::SRGBA8{0, 0, 0, gsl::narrow_cast<uint8_t>(255 * blackAlpha)});
  }

  drawPerformanceBar(ui, waitRatio);

  getPresenter().renderUi(ui, 1);
  getPresenter().updateSoundEngine();
  getPresenter().swapBuffers();
}

bool World::cinematicLoop()
{
  m_cameraController->m_cinematicFrame += 1_frame;
  if(m_cameraController->m_cinematicFrame.get() >= m_cinematicFrames.size())
    return false;

  update(false);

  const auto waterEntryPortals
    = m_cameraController->updateCinematic(m_cinematicFrames.at(m_cameraController->m_cinematicFrame.get()), false);
  doGlobalEffect();

  ui::Ui ui{getPresenter().getMaterialManager()->getUi(), getPalette(), getPresenter().getRenderViewport()};
  getPresenter().renderWorld(getObjectManager(), getRooms(), getCameraController(), waterEntryPortals, 0);
  getPresenter().renderScreenOverlay();
  getPresenter().renderUi(ui, 1);
  getPresenter().updateSoundEngine();
  getPresenter().swapBuffers();
  return true;
}

void World::load(const std::optional<size_t>& slot)
{
  getPresenter().drawLoadingScreen(_("Loading..."));
  const auto filename = m_engine.getSavegamePath(slot);
  BOOST_LOG_TRIVIAL(info) << "Load " << filename;
  serialization::YAMLDocument<true> doc{filename};
  SavegameMeta meta{};
  doc.load("meta", meta, meta);
  if(!util::preferredEqual(meta.filename, std::filesystem::relative(m_levelFilename, m_engine.getUserDataPath())))
  {
    BOOST_LOG_TRIVIAL(error) << "Savegame mismatch. File is for " << meta.filename << ", but current level is "
                             << m_levelFilename;
    return;
  }
  doc.load("data", *this, *this);
  m_objectManager.getLara().m_state.health = m_player->laraHealth;
  m_objectManager.getLara().initWeaponAnimData();
  connectSectors();
  getPresenter().disableScreenOverlay();
}

void World::save(const std::optional<size_t>& slot)
{
  getPresenter().drawLoadingScreen(_("Saving..."));
  const auto filename = m_engine.getSavegamePath(slot);
  BOOST_LOG_TRIVIAL(info) << "Save " << filename;
  serialization::YAMLDocument<false> doc{filename};
  SavegameMeta meta{std::filesystem::relative(m_levelFilename, m_engine.getUserDataPath()).string(), m_title};
  doc.save("meta", meta, meta);
  doc.save("data", *this, *this);
  doc.write();
  getPresenter().disableScreenOverlay();
}

std::map<size_t, SavegameInfo> World::getSavedGames() const
{
  std::map<size_t, SavegameInfo> result;
  for(size_t i = 0; i < core::SavegameSlots; ++i)
  {
    const auto path = m_engine.getSavegamePath(i);
    if(!std::filesystem::is_regular_file(path))
      continue;

    serialization::YAMLDocument<true> doc{path};
    SavegameMeta meta{};
    doc.load("meta", meta, meta);
    result.emplace(i, SavegameInfo{std::move(meta), std::filesystem::last_write_time(path)});
  }
  return result;
}

bool World::hasSavedGames() const
{
  for(size_t i = 0; i < core::SavegameSlots; ++i)
  {
    const auto path = m_engine.getSavegamePath(i);
    if(!std::filesystem::is_regular_file(path))
      continue;

    return true;
  }
  return false;
}

World::World(Engine& engine,
             std::unique_ptr<loader::file::level::Level>&& level,
             std::string title,
             size_t totalSecrets,
             const std::optional<TR1TrackId>& track,
             bool useAlternativeLara,
             std::unordered_map<std::string, std::unordered_map<TR1ItemId, std::string>> itemTitles,
             std::shared_ptr<Player> player)
    : m_engine{engine}
    , m_levelFilename{level->getFilename()}
    , m_audioEngine{std::make_unique<AudioEngine>(
        *this, engine.getUserDataPath() / "data" / "tr1" / "AUDIO", engine.getPresenter().getSoundEngine())}
    , m_title{std::move(title)}
    , m_totalSecrets{totalSecrets}
    , m_itemTitles{std::move(itemTitles)}
    , m_textureAnimator{std::make_unique<render::TextureAnimator>(level->m_animatedTextures)}
    , m_player{std::move(player)}
    , m_samplesData{std::move(level->m_samplesData)}
{
  m_engine.registerWorld(this);
  m_audioEngine->setMusicGain(m_engine.getEngineConfig()->audioSettings.musicVolume);
  m_audioEngine->setSfxGain(m_engine.getEngineConfig()->audioSettings.sfxVolume);

  initTextureDependentDataFromLevel(*level);

  render::MultiTextureAtlas atlases{2048};
  m_controllerLayouts
    = loadControllerButtonIcons(atlases,
                                util::ensureFileExists(m_engine.getEngineDataPath() / "button-icons" / "buttons.yaml"),
                                getPresenter().getMaterialManager()->getSprite(true));
  m_allTextures = buildTextures(*level,
                                m_engine.getGlidos(),
                                atlases,
                                m_atlasTiles,
                                m_sprites,
                                [this](const std::string& s)
                                {
                                  getPresenter().drawLoadingScreen(s);
                                });

  auto sampler = gslu::make_nn_unique<gl::Sampler>("all-textures-sampler")
                 | set(gl::api::TextureMinFilter::NearestMipmapLinear) | set(gl::api::TextureMagFilter::Nearest)
                 | set(gl::api::SamplerParameterI::TextureWrapS, gl::api::TextureWrapMode::ClampToEdge)
                 | set(gl::api::SamplerParameterI::TextureWrapT, gl::api::TextureWrapMode::ClampToEdge);
  if(const auto anisotropyLevel = getEngine().getEngineConfig()->renderSettings.anisotropyLevel;
     anisotropyLevel != 0 && gl::hasAnisotropicFilteringExtension())
    sampler->set(gl::api::SamplerParameterF::TextureMaxAnisotropy, gsl::narrow<float>(anisotropyLevel));
  m_allTexturesHandle = std::make_shared<gl::TextureHandle<gl::Texture2DArray<gl::SRGBA8>>>(
    gsl::not_null{m_allTextures}, std::move(sampler));
  getPresenter().getMaterialManager()->setGeometryTextures(m_allTexturesHandle);

  for(size_t i = 0; i < m_sprites.size(); ++i)
  {
    auto& sprite = m_sprites[i];
    sprite.yBoundMesh = render::scene::createSpriteMesh(static_cast<float>(sprite.render0.x),
                                                        static_cast<float>(-sprite.render0.y),
                                                        static_cast<float>(sprite.render1.x),
                                                        static_cast<float>(-sprite.render1.y),
                                                        sprite.uv0,
                                                        sprite.uv1,
                                                        getPresenter().getMaterialManager()->getSprite(false),
                                                        sprite.textureId.get_as<int32_t>(),
                                                        "sprite-" + std::to_string(i));
    sprite.billboardMesh = render::scene::createSpriteMesh(static_cast<float>(sprite.render0.x),
                                                           static_cast<float>(-sprite.render0.y),
                                                           static_cast<float>(sprite.render1.x),
                                                           static_cast<float>(-sprite.render1.y),
                                                           sprite.uv0,
                                                           sprite.uv1,
                                                           getPresenter().getMaterialManager()->getSprite(true),
                                                           sprite.textureId.get_as<int32_t>(),
                                                           "sprite-" + std::to_string(i));
  }

  m_audioEngine->init(level->m_soundEffectProperties, level->m_soundEffects);

  BOOST_LOG_TRIVIAL(info) << "Loading samples...";

  for(const auto offset : level->m_sampleIndices)
  {
    m_audioEngine->addWav(gsl::not_null{&m_samplesData.at(offset)});
  }

  getPresenter().drawLoadingScreen(util::unescape(m_title));

  initFromLevel(*level);

  if(useAlternativeLara)
  {
    useAlternativeLaraAppearance();
  }

  getPresenter().getSoundEngine()->setListener(m_cameraController.get());
  getPresenter().setTrFont(std::make_unique<ui::TRFont>(*m_spriteSequences.at(TR1ItemId::FontGraphics)));
  if(track.has_value())
    m_audioEngine->playStopCdTrack(m_engine.getScriptEngine(), *track, false);
  getPresenter().disableScreenOverlay();
}

World::~World()
{
  m_engine.unregisterWorld(gsl::not_null{this});
}

void World::drawPickupWidgets(ui::Ui& ui)
{
  auto x = ui.getSize().x * 9 / 10;
  auto y = ui.getSize().y * 9 / 10;
  auto widthPerWidget = ui.getSize().x / 10 * 4 / 3;
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
  if(auto langIt = m_itemTitles.find(m_engine.getLocaleWithoutEncoding()); langIt != m_itemTitles.end())
  {
    if(auto itemIt = langIt->second.find(id); itemIt != langIt->second.end())
    {
      return itemIt->second;
    }
  }
  if(auto langIt = m_itemTitles.find("en_GB"); langIt != m_itemTitles.end())
  {
    if(auto itemIt = langIt->second.find(id); itemIt != langIt->second.end())
    {
      return itemIt->second;
    }
  }

  return std::nullopt;
}

void World::initFromLevel(loader::file::level::Level& level)
{
  BOOST_LOG_TRIVIAL(info) << "Post-processing data structures";

  m_poseFrames = std::move(level.m_poseFrames);
  m_animCommands = std::move(level.m_animCommands);
  m_boneTrees = std::move(level.m_boneTrees);
  m_floorData = std::move(level.m_floorData);
  std::transform(level.m_palette->colors.begin(),
                 level.m_palette->colors.end(),
                 m_palette.begin(),
                 [](const loader::file::ByteColor& color)
                 {
                   return color.toTextureColor();
                 });

  m_animations.resize(level.m_animations.size());
  m_transitions.resize(level.m_transitions.size());
  for(size_t i = 0; i < m_animations.size(); ++i)
  {
    const auto& anim = level.m_animations[i];
    const loader::file::AnimFrame* frames = nullptr;

    if(anim.poseDataOffset.index<decltype(m_poseFrames[0])>() >= m_poseFrames.size())
    {
      BOOST_LOG_TRIVIAL(warning) << "Pose frame data index " << anim.poseDataOffset.index<decltype(m_poseFrames[0])>()
                                 << " out of range 0.." << m_poseFrames.size() - 1;
    }
    else
    {
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
      frames = reinterpret_cast<const loader::file::AnimFrame*>(&anim.poseDataOffset.from(m_poseFrames));
    }

    Expects(anim.nextAnimationIndex < m_animations.size());
    auto nextAnimation = &m_animations[anim.nextAnimationIndex];

    Expects((anim.animCommandIndex + anim.animCommandCount).exclusiveIn(m_animCommands));
    Expects((anim.transitionsIndex + anim.transitionsCount).exclusiveIn(m_transitions));
    gsl::span<const Transitions> transitions;
    if(anim.transitionsCount > 0)
      transitions = gsl::span{&anim.transitionsIndex.from(m_transitions), anim.transitionsCount};

    Expects(anim.segmentLength > 0_frame);
    Expects(anim.firstFrame <= anim.lastFrame);
    m_animations[i] = Animation{frames,
                                anim.segmentLength,
                                anim.state_id,
                                anim.speed,
                                anim.acceleration,
                                anim.firstFrame,
                                anim.lastFrame,
                                anim.nextFrame,
                                anim.animCommandCount,
                                anim.animCommandCount == 0 ? nullptr : &anim.animCommandIndex.from(m_animCommands),
                                nextAnimation,
                                transitions};
  }

  std::transform(level.m_meshes.begin(),
                 level.m_meshes.end(),
                 std::back_inserter(m_meshes),
                 [this](const loader::file::Mesh& mesh)
                 {
                   return Mesh{mesh.collision_center,
                               mesh.collision_radius,
                               gslu::make_nn_shared<RenderMeshData>(mesh, m_atlasTiles, m_palette)};
                 });

  std::vector<gsl::not_null<const Mesh*>> meshesDirect;
  for(auto idx : level.m_meshIndices)
  {
    meshesDirect.emplace_back(&m_meshes.at(idx));
  }

  for(const auto& [modelId, model] : level.m_animatedModels)
  {
    if(model->pose_data_offset.index<decltype(m_poseFrames[0])>() >= m_poseFrames.size())
    {
      BOOST_LOG_TRIVIAL(warning) << "Pose frame data index "
                                 << model->pose_data_offset.index<decltype(m_poseFrames[0])>() << " out of range 0.."
                                 << m_poseFrames.size() - 1;
      continue;
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    const auto frames = reinterpret_cast<const loader::file::AnimFrame*>(&model->pose_data_offset.from(m_poseFrames));
    if(model->nMeshes > 1)
    {
      model->boneTree = gsl::make_span(
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        reinterpret_cast<const loader::file::BoneTreeEntry*>(&model->bone_index.from(m_boneTrees)),
        model->nMeshes - 1);
    }

    Animation* animations = nullptr;
    if(model->animation_index.index != 0xffff)
      animations = &model->animation_index.from(m_animations);

    std::vector<SkeletalModelType::Bone> bones;
    if(model->nMeshes > 0)
    {
      BOOST_ASSERT(model->boneTree.empty() || static_cast<size_t>(model->nMeshes) == model->boneTree.size() + 1);
      for(size_t i = 0; i < gsl::narrow_cast<size_t>(model->nMeshes); ++i)
      {
        const auto& mesh = (model->mesh_base_index + i).from(meshesDirect);
        bones.emplace_back(mesh->meshData,
                           mesh->collisionCenter,
                           mesh->collisionRadius,
                           i == 0 || model->boneTree.empty() ? std::nullopt
                                                             : std::make_optional(model->boneTree[i - 1]));
      }
    }

    m_animatedModels.emplace(modelId,
                             std::make_unique<SkeletalModelType>(SkeletalModelType{
                               model->type, model->mesh_base_index, std::move(bones), frames, animations}));
  }

  for(const auto& transitionCase : level.m_transitionCases)
  {
    const Animation* anim = nullptr;
    if(transitionCase.targetAnimationIndex.index < m_animations.size())
      anim = &transitionCase.targetAnimationIndex.from(m_animations);
    else
      BOOST_LOG_TRIVIAL(warning) << "Animation index " << transitionCase.targetAnimationIndex.index << " not less than "
                                 << m_animations.size();

    m_transitionCases.emplace_back(
      TransitionCase{transitionCase.firstFrame, transitionCase.lastFrame, transitionCase.targetFrame, anim});
  }

  Expects(m_transitions.size() == level.m_transitions.size());
  std::transform(
    level.m_transitions.begin(),
    level.m_transitions.end(),
    m_transitions.begin(),
    [this](const loader::file::Transitions& transitions)
    {
      Expects((transitions.firstTransitionCase + transitions.transitionCaseCount).exclusiveIn(m_transitionCases));
      if(transitions.transitionCaseCount > 0)
        return Transitions{
          transitions.stateId,
          gsl::span{&transitions.firstTransitionCase.from(m_transitionCases), transitions.transitionCaseCount}};
      return Transitions{};
    });

  m_boxes.resize(level.m_boxes.size());
  auto getOverlaps = [this, &level](const uint16_t idx) -> std::vector<gsl::not_null<Box*>>
  {
    if(idx >= level.m_overlaps.size())
      return {};

    std::vector<gsl::not_null<Box*>> result;
    const auto first = &level.m_overlaps.at(idx);
    auto current = first;
    const auto endOfUniverse = &level.m_overlaps.back() + 1;

    while(current < endOfUniverse && (*current & 0x8000u) == 0)
    {
      result.emplace_back(&m_boxes.at(*current));
      ++current;
    }
    result.emplace_back(&m_boxes.at(*current & 0x7FFFu));

    return result;
  };

  std::transform(level.m_boxes.begin(),
                 level.m_boxes.end(),
                 m_boxes.begin(),
                 [&getOverlaps](const loader::file::Box& box)
                 {
                   return Box{{box.xmin, box.xmax},
                              {box.zmin, box.zmax},
                              box.floor,
                              box.blocked,
                              box.blockable,
                              getOverlaps(box.overlap_index)};
                 });

  Expects(level.m_baseZones.flyZone.size() == m_boxes.size());
  Expects(level.m_baseZones.groundZone1.size() == m_boxes.size());
  Expects(level.m_baseZones.groundZone2.size() == m_boxes.size());
  Expects(level.m_alternateZones.flyZone.size() == m_boxes.size());
  Expects(level.m_alternateZones.groundZone1.size() == m_boxes.size());
  Expects(level.m_alternateZones.groundZone2.size() == m_boxes.size());
  for(size_t i = 0; i < m_boxes.size(); ++i)
  {
    m_boxes[i].zoneFly = level.m_baseZones.flyZone[i];
    m_boxes[i].zoneGround1 = level.m_baseZones.groundZone1[i];
    m_boxes[i].zoneGround2 = level.m_baseZones.groundZone2[i];
    m_boxes[i].zoneFlySwapped = level.m_alternateZones.flyZone[i];
    m_boxes[i].zoneGround1Swapped = level.m_alternateZones.groundZone1[i];
    m_boxes[i].zoneGround2Swapped = level.m_alternateZones.groundZone2[i];
  }

  for(const auto& staticMesh : level.m_staticMeshes)
  {
    RenderMeshDataCompositor compositor;
    if(staticMesh.isVisible())
      compositor.append(*meshesDirect.at(staticMesh.mesh)->meshData);
    auto mesh = compositor.toMesh(*getPresenter().getMaterialManager(), false, {});
    mesh->getRenderState().setScissorTest(false);
    const bool distinct
      = m_staticMeshes.emplace(staticMesh.id, StaticMesh{staticMesh.collision_box, staticMesh.doNotCollide(), mesh})
          .second;

    Expects(distinct);
  }

  for(size_t i = 0; i < level.m_rooms.size(); ++i)
  {
    auto& srcRoom = level.m_rooms[i];
    Room room{
      i, srcRoom.isWaterRoom(), srcRoom.position, srcRoom.sectorCountZ, srcRoom.sectorCountX, srcRoom.ambientShade};
    m_rooms.emplace_back(std::move(room));
  }
  for(size_t i = 0; i < m_rooms.size(); ++i)
  {
    const auto& srcRoom = level.m_rooms.at(i);
    std::transform(srcRoom.sectors.begin(),
                   srcRoom.sectors.end(),
                   std::back_inserter(m_rooms[i].sectors),
                   [this](const loader::file::Sector& sector)
                   {
                     return Sector{sector, m_rooms, m_boxes, m_floorData};
                   });
    std::transform(srcRoom.lights.begin(),
                   srcRoom.lights.end(),
                   std::back_inserter(m_rooms[i].lights),
                   [](const loader::file::Light& light)
                   {
                     return Light{light.position, light.intensity, light.fadeDistance};
                   });
    std::transform(
      srcRoom.staticMeshes.begin(),
      srcRoom.staticMeshes.end(),
      std::back_inserter(m_rooms[i].staticMeshes),
      [this](const loader::file::RoomStaticMesh& rsm)
      {
        return RoomStaticMesh{rsm.position, rsm.rotation, rsm.shade, gsl::not_null{findStaticMeshById(rsm.meshId)}};
      });
    m_rooms[i].alternateRoom = srcRoom.alternateRoom.get() >= 0 ? &m_rooms.at(srcRoom.alternateRoom.get()) : nullptr;
  }

  Ensures(m_animations.size() == level.m_animations.size());
  Ensures(m_transitionCases.size() == level.m_transitionCases.size());
  Ensures(m_transitions.size() == level.m_transitions.size());
  Ensures(m_boxes.size() == level.m_boxes.size());

  connectSectors();

  std::transform(level.m_cinematicFrames.begin(),
                 level.m_cinematicFrames.end(),
                 std::back_inserter(m_cinematicFrames),
                 [](const loader::file::CinematicFrame& frame)
                 {
                   return CinematicFrame{frame.lookAt, frame.position, toRad(frame.fov), toRad(frame.rotZ)};
                 });

  for(size_t i = 0; i < m_rooms.size(); ++i)
  {
    m_rooms[i].createSceneNode(level.m_rooms.at(i), i, *this, *m_textureAnimator, *getPresenter().getMaterialManager());
    setParent(gsl::not_null{m_rooms[i].node}, getPresenter().getRenderer().getRootNode());
  }

  std::transform(level.m_cameras.begin(),
                 level.m_cameras.end(),
                 std::back_inserter(m_cameraSinks),
                 [](const loader::file::Camera& camera)
                 {
                   return CameraSink{camera.position, {camera.room}, {camera.flags}};
                 });

  m_objectManager.createObjects(*this, level.m_items);
  if(m_objectManager.getLaraPtr() == nullptr)
  {
    m_cameraController
      = std::make_unique<CameraController>(gsl::not_null{this}, getPresenter().getRenderer().getCamera(), true);

    for(const auto& item : level.m_items)
    {
      if(item.type == TR1ItemId::CutsceneActor1)
      {
        m_cameraController->setPosition(item.position);
      }
    }
  }
  else
  {
    m_cameraController
      = std::make_unique<CameraController>(gsl::not_null{this}, getPresenter().getRenderer().getCamera());
  }

  m_positionalEmitters.clear();
  m_positionalEmitters.reserve(level.m_soundSources.size());
  for(loader::file::SoundSource& src : level.m_soundSources)
  {
    m_positionalEmitters.emplace_back(src.position.toRenderSystem(),
                                      gsl::not_null{getPresenter().getSoundEngine().get()});
    auto voice = m_audioEngine->playSoundEffect(src.sound_effect_id, &m_positionalEmitters.back());
    Expects(voice != nullptr);
    voice->pause();
    m_staticSoundEffects.emplace_back(
      StaticSoundEffect{voice,
                        (src.flags & loader::file::SoundSource::PlayIfRoomsSwapped) != 0,
                        (src.flags & loader::file::SoundSource::PlayIfRoomsNotSwapped) != 0});
  }
  m_audioEngine->getSoundEngine().setListenerGain(1.0f);
  updateStaticSoundEffects();
}

void World::connectSectors()
{
  for(auto& room : m_rooms)
  {
    room.collectShaderLights(m_engine.getEngineConfig()->renderSettings.getLightCollectionDepth());
    for(auto& sector : room.sectors)
      sector.connect(m_rooms);
  }
}

void World::initTextureDependentDataFromLevel(const loader::file::level::Level& level)
{
  std::transform(level.m_textureTiles.begin(),
                 level.m_textureTiles.end(),
                 std::back_inserter(m_atlasTiles),
                 [](const loader::file::TextureTile& tile)
                 {
                   return AtlasTile{tile.textureKey,
                                    {tile.uvCoordinates[0].toGl(),
                                     tile.uvCoordinates[1].toGl(),
                                     tile.uvCoordinates[2].toGl(),
                                     tile.uvCoordinates[3].toGl()}};
                 });

  std::transform(
    level.m_sprites.begin(),
    level.m_sprites.end(),
    std::back_inserter(m_sprites),
    [](const loader::file::Sprite& sprite)
    {
      return Sprite{
        sprite.texture_id, sprite.uv0.toGl(), sprite.uv1.toGl(), sprite.render0, sprite.render1, nullptr, nullptr};
    });

  for(const auto& [sequenceId, sequence] : level.m_spriteSequences)
  {
    Expects(sequence != nullptr);
    Expects(sequence->length <= 0);
    Expects(gsl::narrow<size_t>(sequence->offset - sequence->length) <= m_sprites.size());

    auto seq = std::make_unique<SpriteSequence>();
    *seq = SpriteSequence{sequence->type, gsl::make_span(&m_sprites.at(sequence->offset), -sequence->length)};
    const bool distinct = m_spriteSequences.emplace(sequenceId, std::move(seq)).second;
    Expects(distinct);
  }
}

void World::drawPerformanceBar(ui::Ui& ui, float waitRatio) const
{
  if(!getEngine().getEngineConfig()->displaySettings.performanceMeter)
    return;

  ui.drawBox({0, ui.getSize().y}, {ui.getSize().x, -20}, gl::SRGBA8{0, 0, 0, 224});
  const auto w = gsl::narrow_cast<int>(waitRatio * gsl::narrow_cast<float>(ui.getSize().x));
  if(w > 0)
  {
    ui.drawBox({0, ui.getSize().y}, {w, -20}, gl::SRGBA8{0, 255, 0, 128});
  }
  else
  {
    ui.drawBox({ui.getSize().x, ui.getSize().y}, {w, -20}, gl::SRGBA8{255, 0, 0, 128});
  }
}

void World::updateStaticSoundEffects()
{
  for(const auto& soundEffect : m_staticSoundEffects)
  {
    const auto voice = soundEffect.voice.lock();
    if(voice == nullptr)
      continue;

    if(m_roomsAreSwapped && soundEffect.playIfSwapped)
    {
      voice->play();
      voice->setLooping(true);
    }
    else if(!m_roomsAreSwapped && soundEffect.playIfNotSwapped)
    {
      voice->play();
      voice->setLooping(true);
    }
    else
    {
      voice->setLooping(false);
    }
  }
}
} // namespace engine::world
