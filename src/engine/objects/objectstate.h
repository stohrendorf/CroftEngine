#pragma once

#include "audio/emitter.h"
#include "core/angle.h"
#include "core/id.h"
#include "core/units.h"
#include "core/verlet.h"
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
  core::Verlet speed{};
  core::Verlet fallspeed{};
  core::AnimStateId current_anim_state = 0_as;
  core::AnimStateId goal_anim_state = 0_as;
  core::AnimStateId required_anim_state = 0_as;
  core::Health health = 0_hp;
  TriggerState triggerState = TriggerState::Inactive;
  core::RenderFrame timer = 0_rframe;
  floordata::ActivationState activationState;
  core::Length floor = 0_len;
  std::bitset<32> touch_bits;

  bool falling = false;
  bool is_hit = false;
  bool collidable = true;
  bool already_looked_at = false;

  void serialize(const serialization::Serializer<world::World>& ser);

  bool updateActivationTimeout()
  {
    if(!activationState.isFullyActivated())
    {
      return activationState.isInverted();
    }

    if(timer == 0_rframe)
      return !activationState.isInverted();

    if(timer < 0_rframe)
      return activationState.isInverted();

    timer -= 1_rframe;
    if(timer == 0_rframe)
      timer = -1_rframe;

    return !activationState.isInverted();
  }

  bool isStalkBox(const world::World& world, const world::Box& targetBox) const;

  bool isEscapeBox(const world::World& world, const world::Box& targetBox) const;

  const world::Sector* getCurrentSector() const;

  void loadObjectInfo(const script::Gameflow& gameflow);

  bool isDead() const
  {
    return health <= 0_hp;
  }

  gsl::not_null<const world::Box*> getCurrentBox() const;
};
} // namespace engine::objects
