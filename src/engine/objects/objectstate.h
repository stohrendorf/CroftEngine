#pragma once

#include "audio/emitter.h"
#include "core/angle.h"
#include "core/id.h"
#include "core/units.h"
#include "engine/floordata/floordata.h"
#include "engine/location.h"
#include "qs/quantity.h"
#include "serialization/serialization_fwd.h"

#include <bitset>
#include <cstdint>
#include <glm/fwd.hpp>
#include <gsl/gsl-lite.hpp>
#include <utility>

namespace audio
{
class SoundEngine;
}

namespace engine::world
{
class World;
struct Sector;
struct Box;
struct Room;
} // namespace engine::world

namespace engine::script
{
class Gameflow;
}

namespace engine::objects
{
enum class TriggerState : uint8_t
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

  explicit ObjectState(const gsl::not_null<audio::SoundEngine*>& engine, Location location)
      : Emitter{engine}
      , location{std::move(location)}
  {
  }

public:
  explicit ObjectState(const gsl::not_null<audio::SoundEngine*>& engine,
                       const gsl::not_null<const world::Room*>& room,
                       const core::TypeId& type)
      : Emitter{engine}
      , type{type}
      , location{room}
  {
  }

  ObjectState(const ObjectState&) = delete;
  ObjectState(ObjectState&&) = default;
  ObjectState& operator=(const ObjectState&) = delete;
  ObjectState& operator=(ObjectState&&) = default;

  ~ObjectState() override;

  glm::vec3 getPosition() const override;

  core::TypeId type = core::TypeId{uint16_t(-1)};
  Location location;
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

  void serialize(const serialization::Serializer<world::World>& ser) const;
  void deserialize(const serialization::Deserializer<world::World>& ser);

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

  bool isEscapeBox(const world::World& world, const world::Box& targetBox) const;

  const world::Sector* getCurrentSector() const;

  void loadObjectInfo(const script::Gameflow& gameflow);

  bool isDead() const noexcept
  {
    return health <= 0_hp;
  }

  gsl::not_null<const world::Box*> getCurrentBox() const;
  const world::Box* tryGetCurrentBox() const;

  void hitLara(const core::Health& damage, bool noHits);
};
} // namespace engine::objects
