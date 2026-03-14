#include "object.h"

#include "core/angle.h"
#include "core/id.h"
#include "core/units.h"
#include "core/vec.h"
#include "engine/audioengine.h"
#include "engine/engine.h"
#include "engine/floordata/floordata.h"
#include "engine/items_tr1.h"
#include "engine/objectmanager.h"
#include "engine/particle.h"
#include "engine/presenter.h"
#include "engine/script/scriptengine.h"
#include "engine/soundeffects_tr1.h"
#include "engine/world/room.h"
#include "engine/world/sector.h"
#include "engine/world/world.h"
#include "laraobject.h"
#include "loader/file/item.h"
#include "objectstate.h"
#include "render/scene/node.h"
#include "serialization/serialization.h"
#include "util/helpers.h"

#include <algorithm>
#include <boost/log/trivial.hpp>
#include <cstddef>
#include <glm/geometric.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec3.hpp>
#include <gsl-lite/gsl-lite.hpp>
#include <memory>
#include <optional>
#include <string>

namespace engine::objects
{
// NOLINTNEXTLINE(readability-make-member-function-const)
void Object::applyLogicTransform()
{
  updatePrediction();
  interpolateTransform(0);
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void Object::interpolateTransform(const float interTickFactor)
{
  const auto pos = core::lerp(m_state.location.position, m_state.predictedPosition, interTickFactor);
  const auto rot = core::lerp(m_state.rotation, m_state.predictedRotation, interTickFactor);
  const auto tr = pos.toRenderSystem() - m_state.location.room->position.toRenderSystem();
  getNode()->setLocalMatrix(translate(glm::mat4{1.0f}, tr) * rot.toMatrix());
}

void Object::updatePrediction()
{
  m_state.predictedPosition = m_state.location.position;
  m_state.predictedRotation = m_state.rotation;

  if(m_state.speed != 0_spd)
  {
    m_state.predictedPosition += util::pitch(m_state.speed * 1_frame, getMovementAngle());
  }

  if(m_state.falling && m_state.fallspeed != 0_spd)
  {
    m_state.predictedPosition.Y += m_state.fallspeed * 1_frame;
  }
}

Object::Object(const gsl_lite::not_null<world::World*>& world, const Location& location)
    : m_world{world}
    , m_state{gsl_lite::not_null{world->getEngine().getPresenter().getSoundEngine().get().get()}, location}
    , m_hasUpdateFunction{false}
{
}

Object::Object(const gsl_lite::not_null<world::World*>& world,
               const gsl_lite::not_null<const world::Room*>& room,
               const loader::file::Item& item,
               const bool hasUpdateFunction)
    : Object{world, Location{room, item.position}}
{
  // NOLINTNEXTLINE(cppcoreguidelines-prefer-member-initializer)
  m_hasUpdateFunction = hasUpdateFunction;
  m_state.type = item.type;

  if(!m_state.location.room->isInnerPositionXZ(item.position))
  {
    BOOST_LOG_TRIVIAL(warning) << "patching location for " << toString(item.type.get_as<TR1ItemId>());
    m_state.location.updateRoom();
  }

  m_state.loadObjectInfo(world->getEngine().getScriptEngine().getGameflow());

  m_state.rotation.Y = item.rotation;
  m_state.activationState = floordata::ActivationState(item.activationState);
  m_state.timer = m_state.activationState.getTimeout();

  m_state.floor = m_state.location.room->getSectorByAbsolutePosition(item.position)->floorHeight;

  if(m_state.activationState.isOneshot())
  {
    m_state.activationState.setOneshot(false);
    m_state.triggerState = TriggerState::Invisible;
  }

  if(m_state.activationState.isFullyActivated())
  {
    m_state.activationState.fullyDeactivate();
    m_state.activationState.setInverted(true);
    m_isActive = true;
    m_state.triggerState = TriggerState::Active;
  }
}

void Object::setCurrentRoom(const gsl_lite::not_null<const world::Room*>& newRoom)
{
  setParent(gsl_lite::not_null{getNode()}, newRoom->node);

  m_state.location.room = newRoom;
  applyLogicTransform();
}

void Object::activate()
{
  if(!m_hasUpdateFunction)
  {
    m_state.triggerState = TriggerState::Inactive;
    return;
  }

  m_isActive = true;
  m_world->getObjectManager().activate(this);
}

void Object::deactivate()
{
  m_isActive = false;
  m_world->getObjectManager().deactivate(this);
}

std::shared_ptr<audio::Voice> Object::playSoundEffect(const core::SoundEffectId& id)
{
  return getWorld().getAudioEngine().playSoundEffect(id, &m_state);
}

bool Object::triggerKey()
{
  if(getWorld().getObjectManager().getLara().getHandStatus() != HandStatus::None)
  {
    return false;
  }

  if(m_state.triggerState != TriggerState::Active)
  {
    return false;
  }

  m_state.triggerState = TriggerState::Deactivated;
  return true;
}

void Object::kill()
{
  if(this == getWorld().getObjectManager().getLara().aimAt.get())
  {
    getWorld().getObjectManager().getLara().aimAt.reset();
  }
  getWorld().getObjectManager().scheduleDeletion(this);
  m_state.activationState.setLocked(true);
  deactivate();
}

bool Object::triggerPickUp() noexcept
{
  if(m_state.triggerState != TriggerState::Invisible)
  {
    return false;
  }

  m_state.triggerState = TriggerState::Deactivated;
  return true;
}

bool InteractionLimits::canInteract(const ObjectState& objectState, const ObjectState& laraState) const
{
  if(const auto angle = laraState.rotation - objectState.rotation; angle.X < minAngle.X || angle.X > maxAngle.X
                                                                   || angle.Y < minAngle.Y || angle.Y > maxAngle.Y
                                                                   || angle.Z < minAngle.Z || angle.Z > maxAngle.Z)
  {
    return false;
  }

  const auto offs = laraState.location.position - objectState.location.position;
  const auto dist = glm::vec4{offs.toRenderSystem(), 0.0f} * objectState.rotation.toMatrix();
  return distance.contains(core::TRVec{glm::vec3{dist}});
}

void Object::emitRicochet(const Location& location)
{
  const auto particle = gsl_lite::make_shared<RicochetParticle>(location, getWorld());
  setParent(particle, m_state.location.room->node);
  getWorld().getObjectManager().registerParticle(particle);
  getWorld().getAudioEngine().playSoundEffect(TR1SoundEffect::Ricochet, gsl_lite::not_null{particle.get().get()});
}

std::optional<core::Length> Object::getWaterSurfaceHeight() const
{
  return world::getWaterSurfaceHeight(m_state.location);
}

bool Object::alignTransformClamped(const core::TRVec& targetPos,
                                   const core::TRRotation& targetRot,
                                   const core::Length& maxDistance,
                                   const core::Angle& maxAngle)
{
  auto d = targetPos - m_state.location.position;
  if(const auto dist = length(d); maxDistance < dist)
  {
    m_state.location.move(maxDistance.cast<float>().get() * normalize(d.toRenderSystem()));
  }
  else
  {
    m_state.location.position = targetPos;
  }

  core::TRRotation phi = targetRot - m_state.rotation;
  m_state.rotation.X += std::clamp(phi.X, -maxAngle, maxAngle);
  m_state.rotation.Y += std::clamp(phi.Y, -maxAngle, maxAngle);
  m_state.rotation.Z += std::clamp(phi.Z, -maxAngle, maxAngle);

  phi = targetRot - m_state.rotation;
  d = targetPos - m_state.location.position;

  return abs(phi.X) < 1_au && abs(phi.Y) < 1_au && abs(phi.Z) < 1_au && d.X == 0_len && d.Y == 0_len && d.Z == 0_len;
}

void Object::serialize(const serialization::Serializer<world::World>& ser) const
{
  ser(S_NV("@type", m_state.type),
      S_NV("@location", m_state.location),
      S_NV("state", m_state),
      S_NV("hasUpdateFunction", m_hasUpdateFunction),
      S_NV("isActive", m_isActive));
}

void Object::deserialize(const serialization::Deserializer<world::World>& ser)
{
  ser(S_NV("@type", m_state.type),
      S_NV("@location", m_state.location),
      S_NV("state", m_state),
      S_NV("hasUpdateFunction", m_hasUpdateFunction),
      S_NV("isActive", m_isActive));

  ser << [this](const serialization::Deserializer<world::World>& /*ser*/)
  {
    setParent(gsl_lite::not_null{getNode()}, m_state.location.room->node);

    applyLogicTransform();
    updatePrediction();
  };
}

void Object::moveLocal(const core::TRVec& d)
{
  m_state.location.position += util::pitch(d, m_state.rotation.Y);
}

std::string makeObjectName(const TR1ItemId type, const size_t id)
{
  return "#" + std::to_string(id) + " " + toString(type);
}
} // namespace engine::objects