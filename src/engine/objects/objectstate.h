#pragma once

#include "audio/soundengine.h"
#include "core/id.h"
#include "engine/floordata/floordata.h"
#include "engine/roomboundposition.h"
#include "engine/world/box.h"
#include "loader/file/datatypes.h"

namespace pybind11
{
class dict;
}

namespace engine::world
{
class World;
struct Sector;
} // namespace engine::world

namespace engine
{
namespace ai
{
struct CreatureInfo;
}

namespace objects
{
enum class TriggerState
{
  Inactive,
  Active,
  Deactivated,
  Invisible
};

class Object;

struct ObjectState final : public audio::Emitter
{
private:
  friend Object;

  explicit ObjectState(const gsl::not_null<audio::SoundEngine*>& engine, RoomBoundPosition position)
      : Emitter{engine}
      , position{std::move(position)}
  {
  }

public:
  explicit ObjectState(const gsl::not_null<audio::SoundEngine*>& engine,
                       const gsl::not_null<const world::Room*>& room,
                       const core::TypeId type)
      : Emitter{engine}
      , type{type}
      , position{room}
  {
  }

  ObjectState(const ObjectState&) = delete;

  ObjectState(ObjectState&&) = default;

  ObjectState& operator=(const ObjectState&) = delete;

  ObjectState& operator=(ObjectState&&) = default;

  ~ObjectState() override;

  glm::vec3 getPosition() const override;

  core::TypeId type = core::TypeId{uint16_t(-1)};
  RoomBoundPosition position;
  core::TRRotation rotation;
  core::Speed speed = 0_spd;
  core::Speed fallspeed = 0_spd;
  core::AnimStateId current_anim_state = 0_as;
  core::AnimStateId goal_anim_state = 0_as;
  core::AnimStateId required_anim_state = 0_as;
  core::Health health = 0_hp;
  TriggerState triggerState = TriggerState::Inactive;
  core::Frame timer = 0_frame;
  floordata::ActivationState activationState;
  core::Length floor = 0_len;
  std::bitset<32> touch_bits;

  bool falling = false;
  bool is_hit = false;
  bool collidable = true;
  bool already_looked_at = false;

  std::shared_ptr<ai::CreatureInfo> creatureInfo;

  void serialize(const serialization::Serializer<world::World>& ser);

  bool updateActivationTimeout()
  {
    if(!activationState.isFullyActivated())
    {
      return activationState.isInverted();
    }

    if(timer == 0_frame)
      return !activationState.isInverted();

    if(timer < 0_frame)
      return activationState.isInverted();

    timer -= 1_frame;
    if(timer == 0_frame)
      timer = -1_frame;

    return !activationState.isInverted();
  }

  bool isStalkBox(const world::World& world, const world::Box& targetBox) const;

  bool isInsideZoneButNotInBox(const world::World& world, world::ZoneId zoneId, const world::Box& targetBox) const;

  bool isEscapeBox(const world::World& world, const world::Box& targetBox) const;

  void initCreatureInfo(const world::World& world);

  const world::Sector* getCurrentSector() const;

  void loadObjectInfo();

  bool isDead() const
  {
    return health <= 0_hp;
  }

  gsl::not_null<const world::Box*> getCurrentBox() const;
};

} // namespace objects
} // namespace engine
