#include "world.h"

#include "audio/device.h"
#include "audio/fadevolumecallback.h"
#include "audio/soundengine.h"
#include "audio/voice.h"
#include "box.h"
#include "camerasink.h"
#include "cinematicframe.h"
#include "core/angle.h"
#include "core/genericvec.h"
#include "core/i18n.h"
#include "core/interval.h"
#include "core/magic.h"
#include "engine/ai/pathfinder.h"
#include "engine/audioengine.h"
#include "engine/audiosettings.h"
#include "engine/cameracontroller.h"
#include "engine/engine.h"
#include "engine/engineconfig.h"
#include "engine/floordata/floordata.h"
#include "engine/floordata/secrets.h"
#include "engine/location.h"
#include "engine/objects/aiagent.h"
#include "engine/objects/block.h" // IWYU pragma: keep
#include "engine/objects/laraobject.h"
#include "engine/objects/modelobject.h"
#include "engine/objects/objectstate.h"
#include "engine/objects/pickupobject.h"
#include "engine/objects/tallblock.h" // IWYU pragma: keep
#include "engine/particle.h"
#include "engine/particlecollection.h"
#include "engine/player.h"
#include "engine/presenter.h"
#include "engine/script/scriptengine.h"
#include "engine/skeletalmodelnode.h"
#include "engine/soundeffects_tr1.h"
#include "engine/tracks_tr1.h"
#include "gsl/gsl-lite.hpp"
#include "loader/file/animationid.h"
#include "loader/file/audio.h"
#include "loader/file/color.h"
#include "loader/file/datatypes.h"
#include "loader/file/larastateid.h"
#include "loader/file/level/level.h"
#include "loader/file/meshes.h"
#include "qs/qs.h"
#include "render/material/materialmanager.h"
#include "render/rendersettings.h"
#include "render/scene/camera.h"
#include "render/scene/node.h"
#include "render/scene/renderer.h"
#include "render/textureanimator.h"
#include "room.h"
#include "sector.h"
#include "serialization/array.h"
#include "serialization/bitset.h"
#include "serialization/objectreference.h"
#include "serialization/optional.h"
#include "serialization/quantity.h"
#include "serialization/serialization.h"
#include "serialization/vector.h"
#include "serialization/yamldocument.h"
#include "skeletalmodeltype.h"
#include "sprite.h"
#include "staticsoundeffect.h"
#include "ui/text.h"
#include "ui/ui.h"
#include "util/fsutil.h"
#include "util/helpers.h"
#include "worldgeometry.h"

#include <algorithm>
#include <boost/assert.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/log/trivial.hpp>
#include <boost/range/adaptor/indexed.hpp>
#include <boost/range/adaptor/map.hpp>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <functional>
#include <gl/pixel.h>
#include <glm/gtx/norm.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <gslu.h>
#include <iterator>
#include <set>
#include <sstream>
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

  if(object.isActive())
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
    gsl_Assert(swtch != nullptr);
    if(!swtch->triggerSwitch(request.getTimeout()))
      return false;

    switchIsOn = (swtch->m_state.current_anim_state == 1_as);
    return true;
  }
  case floordata::SequenceCondition::KeyUsed:
  {
    auto key = objectManager.getObject(floordata::Command{*floorData++}.parameter);
    gsl_Assert(key != nullptr);
    return key->triggerKey();
  }
  case floordata::SequenceCondition::ItemPickedUp:
  {
    auto item = objectManager.getObject(floordata::Command{*floorData++}.parameter);
    gsl_Assert(item != nullptr);
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

void emitGroundBubbles(const gsl::not_null<Room*>& room, World& world)
{
  if(!room->isWaterRoom || !room->node->isVisible())
    return;

  static constexpr const size_t MaxParticlesPerSector = 3;
  static constexpr const float EmissionProbability = 0.01f;

  for(int x = 0; x < room->sectorCountX; ++x)
  {
    for(int z = 0; z < room->sectorCountZ; ++z)
    {
      const gsl::not_null s{room->getSectorByIndex(x, z)};
      if(s->roomBelow != nullptr)
        continue;

      for(size_t i = 0; i < MaxParticlesPerSector; ++i)
      {
        if(util::rand15(1.0f) > EmissionProbability)
          continue;

        const auto px = room->position.X + x * core::SectorSize + util::rand15(core::SectorSize);
        const auto pz = room->position.Z + z * core::SectorSize + util::rand15(core::SectorSize);
        const auto py = s->floorHeight;

        auto particle = gsl::make_shared<BubbleParticle>(Location{room, core::TRVec{px, py, pz}}, world, true, true);
        particle->scale = 0.5f;
        particle->circleRadius = 1_len;
        room->particles.registerParticle(std::move(particle));
      }
    }
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

const std::vector<Room>& World::getRooms() const noexcept
{
  return m_rooms;
}

std::vector<Room>& World::getRooms() noexcept
{
  return m_rooms;
}

const std::vector<Box>& World::getBoxes() const noexcept
{
  return m_boxes;
}

void World::useAlternativeLaraAppearance(const bool withHead)
{
  const auto& base = *m_worldGeometry->findAnimatedModelForType(TR1ItemId::Lara);
  auto& laraSkeleton = *m_objectManager.getLara().getSkeleton();
  BOOST_ASSERT(base.bones.size() == laraSkeleton.getBoneCount());

  const auto& alternate = *m_worldGeometry->findAnimatedModelForType(TR1ItemId::AlternativeLara);
  BOOST_ASSERT(alternate.bones.size() == laraSkeleton.getBoneCount());

  for(size_t i = 0; i < laraSkeleton.getBoneCount(); ++i)
    laraSkeleton.setMesh(i, alternate.bones[i].mesh);

  if(!withHead)
    laraSkeleton.setMesh(14, base.bones[14].mesh);

  laraSkeleton.rebuildMesh();
}

void World::dinoStompEffect(objects::Object& object)
{
  const auto d = object.m_state.location.position.toRenderSystem() - m_cameraController->getPosition();
  const auto absD = glm::abs(d);

  static constexpr auto MaxD = (16_sectors).get<float>();
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
    gsl::not_null{&m_worldGeometry->getAnimation(loader::file::AnimationId::STAY_SOLID)});
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
    auto particle = gsl::make_shared<BubbleParticle>(Location{object.m_state.location.room, position}, *this);
    setParent(particle, object.m_state.location.room->node);
    m_objectManager.registerParticle(particle);
  }
}

void World::finishLevelEffect()
{
  finishLevel();
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
  if(m_effectTimer <= core::FrameRate * 4_sec)
  {
    auto pos = m_objectManager.getLara().m_state.location.position;
    core::Frame mul = 0_frame;
    if(m_effectTimer >= core::FrameRate * 1_sec)
    {
      mul = m_effectTimer - (core::FrameRate * 1_sec).cast<core::Frame>();
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
  else if(m_effectTimer == 0_frame)
  {
    m_audioEngine->playSoundEffect(TR1SoundEffect::HeavyDoorSlam, nullptr);
    auto voice = m_audioEngine->playSoundEffect(TR1SoundEffect::FlowingAir,
                                                m_cameraController->getLookAt().position.toRenderSystem());
    m_audioEngine->getSoundEngine().getDevice().registerUpdateCallback(
      audio::FadeVolumeCallback{0.0f,
                                std::chrono::seconds{4},
                                gsl::not_null{voice},
                                [this]()
                                {
                                  m_audioEngine->stopSoundEffect(TR1SoundEffect::FlowingAir, nullptr);
                                }});
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

    if(const auto block = std::dynamic_pointer_cast<objects::Block>(object.get()); block != nullptr)
    {
      patchHeightsForBlock(*block, 1_sectors);
      block->getSkeleton()->resetInterpolation();
    }
    else if(const auto tallBlock = std::dynamic_pointer_cast<objects::TallBlock>(object.get()); tallBlock != nullptr)
    {
      patchHeightsForBlock(*tallBlock, 2_sectors);
      tallBlock->getSkeleton()->resetInterpolation();
    }
  }

  // now swap the rooms and patch the alternate room ids
  {
    std::swap(orig, alternate);
    const auto origVisible = orig.node->isVisible();
    const auto alternateVisible = alternate.node->isVisible();
    orig.node->setVisible(alternateVisible);
    alternate.node->setVisible(origVisible);
  }
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

    if(const auto block = std::dynamic_pointer_cast<objects::Block>(object.get()); block != nullptr)
    {
      patchHeightsForBlock(*block, -1_sectors);
      block->getSkeleton()->resetInterpolation();
    }
    else if(const auto tallBlock = std::dynamic_pointer_cast<objects::TallBlock>(object.get()))
    {
      patchHeightsForBlock(*tallBlock, -2_sectors);
      tallBlock->getSkeleton()->resetInterpolation();
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

gslu::nn_shared<objects::PickupObject>
  World::createPickup(const core::TypeId& type, const gsl::not_null<const Room*>& room, const core::TRVec& position)
{
  loader::file::Item item;
  item.type = type;
  item.room = uint16_t(-1);
  item.position = position;
  item.rotation = 0_deg;
  item.shade = core::Shade{core::Shade::type{0}};
  item.activationState = 0;

  const auto& spriteSequence = m_worldGeometry->findSpriteSequenceForType(type);
  gsl_Assert(spriteSequence != nullptr && !spriteSequence->sprites.empty());
  const Sprite& sprite = spriteSequence->sprites[0];

  auto object = gsl::make_shared<objects::PickupObject>(
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

const std::vector<CinematicFrame>& World::getCinematicFrames() const noexcept
{
  return m_cinematicFrames;
}

void World::update(const bool godMode)
{
  m_objectManager.update(*this, godMode);
  if(const auto lara = m_objectManager.getLaraPtr();
     getEngine().getEngineConfig()->lowHealthMonochrome && lara != nullptr)
  {
    const auto newStrength = 1 - lara->m_state.health.cast<float>() / core::LaraHealth * 5;
    if(newStrength < m_currentDeathStrength)
      m_currentDeathStrength = std::max(m_currentDeathStrength - DeathStrengthFadeDeltaPerFrame, newStrength);
    else if(newStrength > m_currentDeathStrength)
      m_currentDeathStrength = std::min(m_currentDeathStrength + DeathStrengthFadeDeltaPerFrame, newStrength);
    m_currentDeathStrength = std::clamp(m_currentDeathStrength, 0.0f, 1.0f);
  }
  else
  {
    m_currentDeathStrength = 0;
  }
  getPresenter().getMaterialManager()->setDeathStrength(m_currentDeathStrength);

  static constexpr auto UVAnimTime = core::FrameRate * 1_sec / 3;

  m_uvAnimTime += 1_frame;
  if(m_uvAnimTime >= UVAnimTime)
  {
    for(const auto& room : m_rooms)
    {
      room.roomGeometry->animateUv(m_worldGeometry->getAtlasTiles());
    }
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
    gsl_Expects(object != nullptr);
    return turn180Effect(*object);
  case 1:
    gsl_Expects(object != nullptr);
    return dinoStompEffect(*object);
  case 2:
    return laraNormalEffect();
  case 3:
    gsl_Expects(object != nullptr);
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
    gsl_Expects(object != nullptr);
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

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void World::turn180Effect(objects::Object& object) noexcept
{
  object.m_state.rotation.Y += 180_deg;
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void World::drawRightWeaponEffect(const objects::ModelObject& object)
{
  const auto& src = *m_worldGeometry->findAnimatedModelForType(TR1ItemId::LaraPistolsAnim);
  BOOST_ASSERT(src.bones.size() == object.getSkeleton()->getBoneCount());
  object.getSkeleton()->setMesh(10, src.bones[10].mesh);
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
  std::shared_ptr<objects::Object> lookAtObject{};
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
      lookAtObject = m_objectManager.getObject(command.parameter);
      break;
    case floordata::CommandOpcode::UnderwaterCurrent:
    {
      const auto& sink = m_cameraSinks.at(command.parameter);
      {
        m_objectManager.getLara().m_underwaterRoute.setTargetBox(gsl::not_null{&m_boxes.at(sink.box_index)});
        auto newTarget = sink.position;
        newTarget.X = m_boxes[sink.box_index].xInterval.clamp(newTarget.X);
        newTarget.Z = m_boxes[sink.box_index].zInterval.clamp(newTarget.Z);
        m_objectManager.getLara().m_underwaterRoute.setTarget(newTarget);
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
      m_audioEngine->triggerCdTrack(m_engine->getScriptEngine().getGameflow(),
                                    static_cast<TR1TrackId>(command.parameter),
                                    activationRequest,
                                    chunkHeader.sequenceCondition);
      break;
    case floordata::CommandOpcode::Secret:
      BOOST_ASSERT(command.parameter < 16);
      if(!m_secretsFoundBitmask.test(command.parameter))
      {
        m_secretsFoundBitmask.set(command.parameter);
        m_audioEngine->playStopCdTrack(m_engine->getScriptEngine().getGameflow(), TR1TrackId::Secret, false);
        ++m_player->secrets;
      }
      break;
    default:
      break;
    }

    if(command.isLast)
      break;
  }

  if(lookAtObject != nullptr)
  {
    if(m_cameraController->getMode() == CameraMode::FixedPosition
       || m_cameraController->getMode() == CameraMode::HeavyFixedPosition)
    {
      m_cameraController->setLookAtObject(gsl::not_null{lookAtObject});
    }
  }

  if(!swapRooms)
    return;

  swapAllRooms();

  if(flipEffect.has_value())
    setGlobalEffect(*flipEffect);
}

void World::serialize(const serialization::Serializer<World>& ser) const
{
  std::vector<size_t> physicalIds;
  std::transform(m_rooms.begin(),
                 m_rooms.end(),
                 std::back_inserter(physicalIds),
                 [](const Room& room)
                 {
                   return room.physicalId;
                 });

  ser(S_NV("objectManager", m_objectManager),
      S_NV("player", *m_player),
      S_NV("initialLevelStartPlayer", *m_levelStartPlayer),
      S_NV("mapFlipActivationStates", m_mapFlipActivationStates),
      S_NV("cameras", serialization::SerializingFrozenVector{std::cref(m_cameraSinks)}),
      S_NV("activeEffect", m_activeEffect),
      S_NV("effectTimer", m_effectTimer),
      S_NV("cameraController", *m_cameraController),
      S_NV("secretsFound", m_secretsFoundBitmask),
      S_NV("roomsAreSwapped", m_roomsAreSwapped),
      S_NV("roomPhysicalIds", physicalIds),
      S_NV("rooms", serialization::SerializingFrozenVector{std::cref(m_rooms)}),
      S_NV("boxes", serialization::SerializingFrozenVector{std::cref(m_boxes)}),
      S_NV("audioEngine", *m_audioEngine),
      S_NV("ghostFrame", m_ghostFrame),
      S_NV("pierre", serialization::ObjectReference{std::cref(m_pierre)}));
}

void World::deserialize(const serialization::Deserializer<World>& ser)
{
  std::vector<size_t> physicalIds;
  std::transform(m_rooms.begin(),
                 m_rooms.end(),
                 std::back_inserter(physicalIds),
                 [](const Room& room)
                 {
                   return room.physicalId;
                 });

  getPresenter().getRenderer().getRootNode()->clear();
  for(auto& room : m_rooms)
  {
    room.resetScenery();
    setParent(gsl::not_null{room.node}, getPresenter().getRenderer().getRootNode());
  }

  ser(S_NV("roomPhysicalIds", serialization::DeserializingFrozenVector{std::ref(physicalIds)}));
  for(size_t i = 0; i < m_rooms.size(); ++i)
  {
    if(m_rooms[i].physicalId == physicalIds[i])
      continue;

    // do not use "swapWithAlternate", as that may break the "alternateRoom" member
    std::swap(m_rooms[i], m_rooms[physicalIds[i]]);
  }
  for(size_t i = 0; i < m_rooms.size(); ++i)
    gsl_Assert(physicalIds[i] == m_rooms[i].physicalId);

  ser(S_NV("objectManager", m_objectManager),
      S_NV("player", *m_player),
      S_NV("initialLevelStartPlayer", *m_levelStartPlayer),
      S_NV("mapFlipActivationStates", m_mapFlipActivationStates),
      S_NV("cameras", serialization::DeserializingFrozenVector{std::ref(m_cameraSinks)}),
      S_NV("activeEffect", m_activeEffect),
      S_NV("effectTimer", m_effectTimer),
      S_NV("cameraController", *m_cameraController),
      S_NV("secretsFound", m_secretsFoundBitmask),
      S_NV("roomsAreSwapped", m_roomsAreSwapped),
      S_NV("roomPhysicalIds", physicalIds),
      S_NV("rooms", serialization::DeserializingFrozenVector{std::ref(m_rooms)}),
      S_NV("boxes", serialization::DeserializingFrozenVector{std::ref(m_boxes)}),
      S_NV("audioEngine", *m_audioEngine),
      S_NV("ghostFrame", m_ghostFrame));
  // TODO CE-625
  if(ser.node.has_child(c4::to_csubstr("pierre")))
    ser(S_NV("pierre", serialization::ObjectReference{std::cref(m_pierre)}));

  updateStaticSoundEffects();
}

void World::gameLoop(bool godMode, float blackAlpha, ui::Ui& ui)
{
  update(godMode);
  m_player->laraHealth = m_objectManager.getLara().m_state.health;

  const auto waterEntryPortals = m_cameraController->update();

  for(const auto& room : m_rooms)
  {
    std::vector<std::shared_ptr<Particle>> particlesToErase;
    for(const auto& particle : room.particles)
    {
      if(const auto parent = particle->location.room; parent != &room)
      {
        particlesToErase.emplace_back(particle);
        particle->location.room->particles.registerParticle(particle);
      }
    }
    for(const auto& particle : particlesToErase)
      room.particles.eraseParticle(particle);
  }

  doGlobalEffect();
  getPresenter().drawBars(
    ui, m_worldGeometry->getPalette(), getObjectManager(), getEngine().getEngineConfig()->pulseLowHealthHealthBar);

  drawPickupWidgets(ui);
  if(const auto lara = getObjectManager().getLaraPtr())
    lara->m_state.location.room->node->setVisible(true);
  getPresenter().renderWorld(getRooms(), getCameraController(), waterEntryPortals, *this);
  getPresenter().renderScreenOverlay();
  if(blackAlpha > 0)
  {
    ui.drawBox({0, 0}, ui.getSize(), gl::SRGBA8{0, 0, 0, gsl::narrow_cast<uint8_t>(255 * blackAlpha)});
  }

  getPresenter().renderUi(ui, 1);
  getPresenter().updateSoundEngine();
  getPresenter().swapBuffers();

  if(m_engine->getEngineConfig()->waterBedBubbles)
  {
    for(auto& room : m_rooms)
    {
      emitGroundBubbles(gsl::not_null{&room}, *this);
    }
  }
}

bool World::cinematicLoop()
{
  m_cameraController->m_cinematicFrame += 1_frame;
  if(gsl::narrow<size_t>(m_cameraController->m_cinematicFrame.get()) >= m_cinematicFrames.size())
    return false;

  update(false);
  getPresenter().getMaterialManager()->setDeathStrength(0);

  const auto waterEntryPortals
    = m_cameraController->updateCinematic(m_cinematicFrames.at(m_cameraController->m_cinematicFrame.get()), false);
  doGlobalEffect();

  ui::Ui ui{
    getPresenter().getMaterialManager()->getUi(), m_worldGeometry->getPalette(), getPresenter().getUiViewport()};
  getPresenter().renderWorld(getRooms(), getCameraController(), waterEntryPortals, *this);
  getPresenter().renderScreenOverlay();
  getPresenter().renderUi(ui, 1);
  getPresenter().updateSoundEngine();
  getPresenter().swapBuffers();
  return true;
}

void World::load(const std::optional<size_t>& slot)
{
  getPresenter().drawLoadingScreen(_("Loading..."));
  const auto filename = m_engine->getSavegamePath(slot);
  BOOST_LOG_TRIVIAL(info) << "Load " << filename;
  serialization::YAMLDocument<true> doc{filename};
  SavegameMeta meta{};
  doc.deserialize("meta", gsl::not_null{&meta}, meta);
  if(!util::preferredEqual(meta.filename, std::filesystem::relative(m_levelFilename, m_engine->getAssetDataPath())))
  {
    BOOST_LOG_TRIVIAL(error) << "Savegame mismatch. File is for " << meta.filename << ", but current level is "
                             << m_levelFilename;
    return;
  }
  doc.deserialize("data", gsl::not_null{this}, *this);
  m_objectManager.getLara().m_state.health = m_player->laraHealth;
  m_objectManager.getLara().initWeaponAnimData();
  connectSectors();
  getPresenter().disableScreenOverlay();

  m_engine->onGameSavedOrLoaded();
}

void World::save(const std::filesystem::path& filename)
{
  BOOST_LOG_TRIVIAL(info) << "Save " << filename;
  serialization::YAMLDocument<false> doc{filename};
  SavegameMeta meta{std::filesystem::relative(m_levelFilename, m_engine->getAssetDataPath()).string()};
  doc.serialize("meta", gsl::not_null{&meta}, meta);
  doc.serialize("data", gsl::not_null{this}, *this);
  doc.write();

  serialization::YAMLDocument<false> metaCacheDoc{makeMetaFilepath(filename)};
  metaCacheDoc.serialize("meta", gsl::not_null{&meta}, meta);
  metaCacheDoc.write();

  m_engine->onGameSavedOrLoaded();
}

void World::save(const std::optional<size_t>& slot)
{
  const auto filename = m_engine->getSavegamePath(slot);
  save(filename);
  getPresenter().disableScreenOverlay();
}

std::tuple<std::optional<SavegameInfo>, std::map<size_t, SavegameInfo>> World::getSavedGames() const
{
  auto getSavegameInfo = [](const std::filesystem::path& path) -> std::optional<SavegameInfo>
  {
    if(!std::filesystem::is_regular_file(path))
      return std::nullopt;

    auto metaPath = makeMetaFilepath(path);
    if(std::filesystem::is_regular_file(metaPath))
    {
      serialization::YAMLDocument<true> metaCacheDoc{metaPath};
      SavegameMeta meta{};
      metaCacheDoc.deserialize("meta", gsl::not_null{&meta}, meta);
      return SavegameInfo{std::move(meta), std::filesystem::last_write_time(path)};
    }

    serialization::YAMLDocument<true> doc{path};
    SavegameMeta meta{};
    doc.deserialize("meta", gsl::not_null{&meta}, meta);
    serialization::YAMLDocument<false> newMetaCacheDoc{metaPath};
    newMetaCacheDoc.serialize("meta", gsl::not_null{&meta}, meta);
    newMetaCacheDoc.write();
    return SavegameInfo{std::move(meta), std::filesystem::last_write_time(path)};
  };

  std::map<size_t, SavegameInfo> result;
  for(size_t i = 0; i < core::SavegameSlots; ++i)
  {
    const auto path = m_engine->getSavegamePath(i);
    if(auto info = getSavegameInfo(path); info.has_value())
      result.emplace(i, *info);
  }
  return {getSavegameInfo(m_engine->getSavegamePath(std::nullopt)), result};
}

bool World::hasSavedGames() const
{
  if(std::filesystem::is_regular_file(m_engine->getSavegamePath(std::nullopt)))
    return true;

  for(size_t i = 0; i < core::SavegameSlots; ++i)
  {
    const auto path = m_engine->getSavegamePath(i);
    if(!std::filesystem::is_regular_file(path))
      continue;

    return true;
  }
  return false;
}

World::World(const gsl::not_null<Engine*>& engine,
             std::unique_ptr<loader::file::level::Level>&& level,
             std::string title,
             const std::optional<TR1TrackId>& ambient,
             bool useAlternativeLara,
             std::unordered_map<std::string, std::unordered_map<TR1ItemId, std::string>> itemTitles,
             std::shared_ptr<Player> player,
             std::shared_ptr<Player> levelStartPlayer,
             bool fromSave,
             std::shared_ptr<WorldGeometry>&& worldGeometry,
             const std::filesystem::path& worldGeometryCacheKey)
    : m_engine{engine}
    , m_levelFilename{level->getFilename()}
    , m_audioEngine{std::make_unique<AudioEngine>(
        gsl::not_null{this}, engine->getAssetDataPath(), engine->getPresenter().getSoundEngine())}
    , m_title{std::move(title)}
    , m_itemTitles{std::move(itemTitles)}
    , m_player{std::move(player)}
    , m_levelStartPlayer{std::move(levelStartPlayer)}
    , m_samplesData{std::move(level->m_samplesData)}
    , m_worldGeometry{worldGeometry != nullptr ? std::move(worldGeometry)
                                               : gsl::make_shared<WorldGeometry>(*m_engine, *level)}
{
  m_engine->setWorldGeometryCache(worldGeometryCacheKey, m_worldGeometry);
  m_engine->registerWorld(this);
  m_audioEngine->setMusicGain(m_engine->getEngineConfig()->audioSettings.musicVolume);
  m_audioEngine->setSfxGain(m_engine->getEngineConfig()->audioSettings.sfxVolume);

  m_audioEngine->initForWorld(level->m_soundEffectProperties, level->m_soundEffects);

  BOOST_LOG_TRIVIAL(info) << "Loading samples...";

  for(const auto offset : level->m_sampleIndices)
  {
    m_audioEngine->addWav(gsl::not_null{&m_samplesData.at(offset)});
  }

  getPresenter().drawLoadingScreen(util::unescape(m_title));

  initFromLevel(*level, fromSave);

  if(useAlternativeLara)
  {
    useAlternativeLaraAppearance();
  }

  getPresenter().getSoundEngine()->setListener(m_cameraController.get());
  getPresenter().setTrFont(
    std::make_unique<ui::TRFont>(*m_worldGeometry->getSpriteSequences().at(TR1ItemId::FontGraphics)));
  if(ambient.has_value())
  {
    m_audioEngine->playStopCdTrack(m_engine->getScriptEngine().getGameflow(), *ambient, false);
  }
  getPresenter().disableScreenOverlay();

  m_engine->onGameSavedOrLoaded();
}

World::~World()
{
  m_engine->unregisterWorld(gsl::not_null{this});
}

void World::drawPickupWidgets(ui::Ui& ui)
{
  auto x = ui.getSize().x;
  const auto y = ui.getSize().y * 9 / 10;

  static constexpr float WidgetScale = 0.5f;

  for(const auto& widget : m_pickupWidgets)
  {
    if(widget.expired())
      continue;

    x -= std::lround(gsl::narrow_cast<float>(widget.getWidth()) * WidgetScale);
    widget.draw(ui, x, y, m_engine->getPresenter().getTrFont(), WidgetScale);
  }
}

const Presenter& World::getPresenter() const
{
  return m_engine->getPresenter();
}

Presenter& World::getPresenter()
{
  return m_engine->getPresenter();
}

std::optional<std::string> World::getItemTitle(TR1ItemId id) const
{
  if(auto langIt = m_itemTitles.find(m_engine->getLocaleWithoutEncoding()); langIt != m_itemTitles.end())
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

void World::initFromLevel(loader::file::level::Level& level, bool fromSave)
{
  BOOST_LOG_TRIVIAL(info) << "Pre-flight checks for " << m_levelFilename.stem();
  for(const auto& idItem : level.m_items | boost::adaptors::indexed())
  {
    const auto i = idItem.index();
    const auto& item = idItem.value();

    if(item.room.get() >= level.m_rooms.size())
    {
      BOOST_LOG_TRIVIAL(fatal) << "invalid item #" << i << "(" << toString(item.type.get_as<TR1ItemId>())
                               << "): invalid room " << item.room.get();
      continue;
    }
    const auto& room = level.m_rooms[item.room.get()];
    const auto inRoom = item.position - room.position;
    const auto sectorX = sectorOf(inRoom.X);
    const auto sectorZ = sectorOf(inRoom.Z);
    if(sectorX < 1 || sectorX >= room.sectorCountX - 1 || sectorZ < 1 || sectorZ >= room.sectorCountZ - 1)
    {
      BOOST_LOG_TRIVIAL(warning) << "invalid item #" << i << "(" << toString(item.type.get_as<TR1ItemId>())
                                 << "): horizontally out of bounds";
    }
    if(item.position.Y > room.lowestHeight)
    {
      BOOST_LOG_TRIVIAL(warning) << "invalid item #" << i << "(" << toString(item.type.get_as<TR1ItemId>())
                                 << "): vertically too low - " << item.position.Y << " > " << room.lowestHeight;
    }
    if(item.position.Y < room.greatestHeight)
    {
      BOOST_LOG_TRIVIAL(warning) << "invalid item #" << i << "(" << toString(item.type.get_as<TR1ItemId>())
                                 << "): vertically too high - " << item.position.Y << " < " << room.greatestHeight;
    }
  }

  BOOST_LOG_TRIVIAL(info) << "Post-processing data structures for " << m_levelFilename.stem();

  m_floorData = std::move(level.m_floorData);

  initBoxes(level);
  initRooms(level);
  initCinematicFrames(level);
  initCameras(level);

  connectSectors();

  if(!fromSave)
  {
    m_objectManager.createObjects(*this, level.m_items);
  }

  initCameraController();
  initStaticSoundEffects(level);
  m_audioEngine->getSoundEngine().setListenerGain(1.0f);
  updateStaticSoundEffects();

  countSecrets();
}

void World::initStaticSoundEffects(const loader::file::level::Level& level)
{
  m_positionalEmitters.clear();
  m_positionalEmitters.reserve(level.m_soundSources.size());
  for(const loader::file::SoundSource& src : level.m_soundSources)
  {
    m_positionalEmitters.emplace_back(src.position.toRenderSystem(),
                                      gsl::not_null{getPresenter().getSoundEngine().get()});
    auto voice = m_audioEngine->playSoundEffect(src.sound_effect_id, &m_positionalEmitters.back());
    gsl_Assert(voice != nullptr);
    voice->pause();
    m_staticSoundEffects.emplace_back(
      StaticSoundEffect{voice,
                        (src.flags & loader::file::SoundSource::PlayIfRoomsSwapped) != 0,
                        (src.flags & loader::file::SoundSource::PlayIfRoomsNotSwapped) != 0});
  }
}

void World::initCameraController()
{
  if(m_objectManager.getLaraPtr() == nullptr)
  {
    m_cameraController
      = std::make_unique<CameraController>(gsl::not_null{this}, getPresenter().getRenderer().getCamera(), true);

    for(const auto& item : m_objectManager.getObjects() | boost::adaptors::map_values)
    {
      if(item->m_state.type == TR1ItemId::CutsceneActor1)
      {
        m_cameraController->setPosition(item->m_state.location.position);
      }
    }
  }
  else
  {
    m_cameraController
      = std::make_unique<CameraController>(gsl::not_null{this}, getPresenter().getRenderer().getCamera());
  }
}

void World::countSecrets()
{
  BOOST_LOG_TRIVIAL(info) << "Counting secrets...";

  std::bitset<16> secretsMask;
  for(const auto& room : m_rooms)
  {
    for(const auto& sector : room.sectors)
    {
      secretsMask |= floordata::getSecretsMask(sector.floorData);
    }
  }
  m_totalSecrets = secretsMask.count();
  BOOST_LOG_TRIVIAL(info) << "Found " << m_totalSecrets << " secrets";
}

void World::initCameras(const loader::file::level::Level& level)
{
  std::transform(level.m_cameras.begin(),
                 level.m_cameras.end(),
                 std::back_inserter(m_cameraSinks),
                 [](const loader::file::Camera& camera)
                 {
                   return CameraSink{camera.position, {camera.room}, {camera.flags}};
                 });
}

void World::initCinematicFrames(const loader::file::level::Level& level)
{
  std::transform(level.m_cinematicFrames.begin(),
                 level.m_cinematicFrames.end(),
                 std::back_inserter(m_cinematicFrames),
                 [](const loader::file::CinematicFrame& frame) noexcept
                 {
                   return CinematicFrame{frame.lookAt, frame.position, toRad(frame.fov), toRad(frame.rotZ)};
                 });
}

void World::initRooms(const loader::file::level::Level& level)
{
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
    gsl_Assert(srcRoom.sectors.size() == m_rooms[i].sectors.size());
    std::transform(srcRoom.lights.begin(),
                   srcRoom.lights.end(),
                   std::back_inserter(m_rooms[i].lights),
                   [](const loader::file::Light& light)
                   {
                     return Light{light.position, light.intensity, light.fadeDistance};
                   });
    for(const auto& rsm : srcRoom.staticMeshes)
    {
      if(const auto mesh = m_worldGeometry->findStaticMeshById(rsm.meshId); mesh != nullptr)
      {
        m_rooms[i].staticMeshes.emplace_back(
          RoomStaticMesh{rsm.position, rsm.rotation, rsm.shade, gsl::not_null{mesh}});
      }
      else
      {
        BOOST_LOG_TRIVIAL(warning) << "No static mesh found for id " << rsm.meshId.get();
      }
    }
    m_rooms[i].alternateRoom = srcRoom.alternateRoom.get() >= 0 ? &m_rooms.at(srcRoom.alternateRoom.get()) : nullptr;

    m_rooms[i].createSceneNode(
      level.m_rooms.at(i), *this, level.m_animatedTextures, *getPresenter().getMaterialManager());
    setParent(gsl::not_null{m_rooms[i].node}, getPresenter().getRenderer().getRootNode());
  }
}

void World::initBoxes(const loader::file::level::Level& level)
{
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
  gsl_Ensures(m_boxes.size() == level.m_boxes.size());

  gsl_Assert(level.m_baseZones.flyZone.size() == m_boxes.size());
  gsl_Assert(level.m_baseZones.groundZone1.size() == m_boxes.size());
  gsl_Assert(level.m_baseZones.groundZone2.size() == m_boxes.size());
  gsl_Assert(level.m_alternateZones.flyZone.size() == m_boxes.size());
  gsl_Assert(level.m_alternateZones.groundZone1.size() == m_boxes.size());
  gsl_Assert(level.m_alternateZones.groundZone2.size() == m_boxes.size());
  for(size_t i = 0; i < m_boxes.size(); ++i)
  {
    m_boxes[i].zoneFly = level.m_baseZones.flyZone[i];
    m_boxes[i].zoneGround1 = level.m_baseZones.groundZone1[i];
    m_boxes[i].zoneGround2 = level.m_baseZones.groundZone2[i];
    m_boxes[i].zoneFlySwapped = level.m_alternateZones.flyZone[i];
    m_boxes[i].zoneGround1Swapped = level.m_alternateZones.groundZone1[i];
    m_boxes[i].zoneGround2Swapped = level.m_alternateZones.groundZone2[i];
  }
}

void World::connectSectors()
{
  for(auto& room : m_rooms)
  {
    room.collectShaderLights(m_engine->getEngineConfig()->renderSettings.getLightCollectionDepth());
    for(auto& sector : room.sectors)
      sector.connect(m_rooms);
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
      voice->pause();
      voice->setLooping(false);
    }
  }
}
} // namespace engine::world
