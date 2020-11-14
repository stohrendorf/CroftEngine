#include "object.h"

#include "engine/audioengine.h"
#include "engine/particle.h"
#include "engine/presenter.h"
#include "engine/script/reflection.h"
#include "laraobject.h"
#include "render/scene/sprite.h"
#include "serialization/vector.h"

namespace engine::objects
{
// NOLINTNEXTLINE(readability-make-member-function-const)
void Object::applyTransform()
{
  const glm::vec3 tr = m_state.position.position.toRenderSystem() - m_state.position.room->position.toRenderSystem();
  getNode()->setLocalMatrix(translate(glm::mat4{1.0f}, tr) * m_state.rotation.toMatrix());
}

Object::Object(const gsl::not_null<Engine*>& engine, const core::RoomBoundPosition& position)
    : m_engine{engine}
    , m_state{&engine->getPresenter().getSoundEngine(), position}
    , m_hasUpdateFunction{false}
{
}

Object::Object(const gsl::not_null<Engine*>& engine,
               const gsl::not_null<const loader::file::Room*>& room,
               const loader::file::Item& item,
               const bool hasUpdateFunction)
    : Object{engine, core::RoomBoundPosition{room, item.position}}
{
  m_hasUpdateFunction = hasUpdateFunction;
  m_state.type = item.type;

  BOOST_ASSERT(room->isInnerPositionXZ(item.position));

  m_state.loadObjectInfo();

  m_state.rotation.Y = item.rotation;
  m_state.shade = item.shade;
  m_state.activationState = floordata::ActivationState(item.activationState);
  m_state.timer = m_state.activationState.getTimeout();

  m_state.floor = room->getSectorByAbsolutePosition(item.position)->floorHeight;

  if(m_state.activationState.isOneshot())
  {
    m_state.activationState.setOneshot(false);
    m_state.triggerState = TriggerState::Invisible;
  }

  if(m_state.activationState.isFullyActivated())
  {
    m_state.activationState.fullyDeactivate();
    m_state.activationState.setInverted(true);
    activate();
    m_state.triggerState = TriggerState::Active;
  }
}

void Object::setCurrentRoom(const gsl::not_null<const loader::file::Room*>& newRoom)
{
  if(newRoom == m_state.position.room)
  {
    return;
  }

  setParent(getNode(), newRoom->node);

  m_state.position.room = newRoom;
  applyTransform();
}

void Object::activate()
{
  if(!m_hasUpdateFunction)
  {
    m_state.triggerState = TriggerState::Inactive;
    return;
  }

  m_isActive = true;
}

void Object::deactivate()
{
  m_isActive = false;
}

std::shared_ptr<audio::SourceHandle> Object::playSoundEffect(const core::SoundEffectId id)
{
  return getEngine().getPresenter().getAudioEngine().playSoundEffect(id, &m_state);
}

bool Object::triggerKey()
{
  if(getEngine().getObjectManager().getLara().getHandStatus() != HandStatus::None)
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
  if(this == getEngine().getObjectManager().getLara().target.get())
  {
    getEngine().getObjectManager().getLara().target.reset();
  }
  getEngine().getObjectManager().scheduleDeletion(this);
  m_state.activationState.setLocked(true);
}

bool Object::triggerPickUp()
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
  const auto angle = laraState.rotation - objectState.rotation;
  if(angle.X < minAngle.X || angle.X > maxAngle.X || angle.Y < minAngle.Y || angle.Y > maxAngle.Y
     || angle.Z < minAngle.Z || angle.Z > maxAngle.Z)
  {
    return false;
  }

  const auto offs = laraState.position.position - objectState.position.position;
  const auto dist = glm::vec4{offs.toRenderSystem(), 0.0f} * objectState.rotation.toMatrix();
  return distance.contains(core::TRVec{glm::vec3{dist}});
}

void Object::playShotMissed(const core::RoomBoundPosition& pos)
{
  const auto particle = std::make_shared<RicochetParticle>(pos, getEngine());
  setParent(particle, m_state.position.room->node);
  getEngine().getObjectManager().registerParticle(particle);
  getEngine().getPresenter().getAudioEngine().playSoundEffect(TR1SoundEffect::Ricochet, particle.get());
}

std::optional<core::Length> Object::getWaterSurfaceHeight() const
{
  return loader::file::Room::getWaterSurfaceHeight(m_state.position);
}

void Object::updateLighting()
{
  auto tmp = m_state.position;
  tmp.position += getBoundingBox().getCenter();
  m_lighting.updateDynamic(m_state.shade, tmp, m_engine->getRooms());
}

bool Object::alignTransformClamped(const core::TRVec& targetPos,
                                   const core::TRRotation& targetRot,
                                   const core::Length& maxDistance,
                                   const core::Angle& maxAngle)
{
  auto d = targetPos - m_state.position.position;
  const auto dist = d.length();
  if(maxDistance < dist)
  {
    move(maxDistance.cast<float>().get() * normalize(d.toRenderSystem()));
  }
  else
  {
    m_state.position.position = targetPos;
  }

  core::TRRotation phi = targetRot - m_state.rotation;
  m_state.rotation.X += util::clamp(phi.X, -maxAngle, maxAngle);
  m_state.rotation.Y += util::clamp(phi.Y, -maxAngle, maxAngle);
  m_state.rotation.Z += util::clamp(phi.Z, -maxAngle, maxAngle);

  phi = targetRot - m_state.rotation;
  d = targetPos - m_state.position.position;

  return abs(phi.X) < 1_au && abs(phi.Y) < 1_au && abs(phi.Z) < 1_au && d.X == 0_len && d.Y == 0_len && d.Z == 0_len;
}

void Object::serialize(const serialization::Serializer& ser)
{
  ser(S_NV("@type", m_state.type),
      S_NV("@position", m_state.position),
      S_NV("state", m_state),
      S_NV("hasUpdateFunction", m_hasUpdateFunction),
      S_NV("isActive", m_isActive));

  ser.lazy([this](const serialization::Serializer& ser) {
    // FIXME ser(S_NV("renderables", serialization::FrozenVector{getNode()->getChildren()}));

    if(ser.loading)
    {
      setParent(getNode(), m_state.position.room->node);

      applyTransform();
      updateLighting();
    }
  });
}
} // namespace engine::objects
