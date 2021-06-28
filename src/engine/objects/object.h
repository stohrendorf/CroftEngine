#pragma once

#include "audio/soundengine.h"
#include "core/boundingbox.h"
#include "core/units.h"
#include "engine/collisioninfo.h"
#include "engine/floordata/floordata.h"
#include "engine/skeletalmodelnode.h"
#include "objectstate.h"

#include <utility>

namespace loader::file
{
struct Item;
} // namespace loader::file

namespace engine::world
{
class World;
}

namespace engine
{
class Particle;

namespace ai
{
struct CreatureInfo;
}

struct CollisionInfo;

namespace objects
{
struct InteractionLimits
{
  core::BoundingBox distance;
  core::TRRotation minAngle;
  core::TRRotation maxAngle;

  InteractionLimits(core::BoundingBox bbox, core::TRRotation min, core::TRRotation max)
      : distance{std::move(bbox)}
      , minAngle{std::move(min)}
      , maxAngle{std::move(max)}
  {
    distance.makeValid();
  }

  [[nodiscard]] bool canInteract(const ObjectState& objectState, const ObjectState& laraState) const;
};

class Object
{
  const gsl::not_null<world::World*> m_world;

protected:
  Object(const gsl::not_null<world::World*>& world, const core::RoomBoundPosition& position);

public:
  ObjectState m_state;
  bool m_isActive = false;
  bool m_hasUpdateFunction;

  enum class AnimCommandOpcode : uint16_t
  {
    SetPosition = 1,
    StartFalling = 2,
    EmptyHands = 3,
    Kill = 4,
    PlaySound = 5,
    PlayEffect = 6,
    Interact = 7
  };

  Object(const gsl::not_null<world::World*>& world,
         const gsl::not_null<const world::Room*>& room,
         const loader::file::Item& item,
         bool hasUpdateFunction);

  Object(const Object&) = delete;

  Object& operator=(const Object&) = delete;

  Object& operator=(Object&&) = delete;

  virtual ~Object() = default;

  virtual void update() = 0;

  virtual std::shared_ptr<render::scene::Node> getNode() const = 0;

  void setCurrentRoom(const gsl::not_null<const world::Room*>& newRoom);

  void applyTransform();

  void rotate(const core::Angle& dx, const core::Angle& dy, const core::Angle& dz)
  {
    m_state.rotation.X += dx;
    m_state.rotation.Y += dy;
    m_state.rotation.Z += dz;
  }

  void move(const core::Length& dx, const core::Length& dy, const core::Length& dz)
  {
    m_state.position.position.X += dx;
    m_state.position.position.Y += dy;
    m_state.position.position.Z += dz;
  }

  void move(const glm::vec3& d)
  {
    move(core::TRVec{d});
  }

  void move(const core::TRVec& d)
  {
    m_state.position.position += d;
  }

  void moveLocal(const core::TRVec& d)
  {
    m_state.position.position += util::pitch(d, m_state.rotation.Y);
  }

  const world::World& getWorld() const
  {
    return *m_world;
  }

  world::World& getWorld()
  {
    return *m_world;
  }

  void dampenHorizontalSpeed(const float f)
  {
    m_state.speed -= (m_state.speed.cast<float>() * f).cast<core::Speed>();
  }

  virtual void patchFloor(const core::TRVec& /*pos*/, core::Length& /*y*/)
  {
  }

  virtual void patchCeiling(const core::TRVec& /*pos*/, core::Length& /*y*/)
  {
  }

  void activate();

  void deactivate();

  virtual bool triggerSwitch(core::Frame timeout) = 0;

  std::shared_ptr<audio::Voice> playSoundEffect(core::SoundEffectId id);

  bool triggerPickUp();

  bool triggerKey();

  virtual core::Angle getMovementAngle() const
  {
    return m_state.rotation.Y;
  }

  bool alignTransform(const core::TRVec& speed, const Object& target)
  {
    auto targetPos = target.m_state.position.position.toRenderSystem();
    targetPos += glm::vec3{target.m_state.rotation.toMatrix() * glm::vec4{speed.toRenderSystem(), 1.0f}};

    return alignTransformClamped(core::TRVec{targetPos}, target.m_state.rotation, 16_len, 364_au);
  }

  virtual void updateLighting() = 0;

  virtual loader::file::BoundingBox getBoundingBox() const = 0;

  virtual void collide(CollisionInfo& /*collisionInfo*/)
  {
  }

  void kill();

  virtual void serialize(const serialization::Serializer<world::World>& ser);

  void emitRicochet(const core::RoomBoundPosition& pos);

  std::optional<core::Length> getWaterSurfaceHeight() const;

protected:
  bool alignTransformClamped(const core::TRVec& targetPos,
                             const core::TRRotation& targetRot,
                             const core::Length& maxDistance,
                             const core::Angle& maxAngle);
};
} // namespace objects
} // namespace engine
