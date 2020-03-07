#pragma once

#include "audio/soundengine.h"
#include "core/id.h"
#include "engine/floordata/floordata.h"
#include "loader/file/datatypes.h"

namespace loader::file
{
struct Item;
struct Animation;
struct Box;
} // namespace loader::file

namespace engine
{
class Engine;

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

  explicit ObjectState(const gsl::not_null<audio::SoundEngine*>& engine, core::RoomBoundPosition position)
      : Emitter{engine}
      , position{std::move(position)}
  {
  }

public:
  explicit ObjectState(const gsl::not_null<audio::SoundEngine*>& engine,
                       const gsl::not_null<const loader::file::Room*>& room,
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
  core::RoomBoundPosition position;
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
  const loader::file::Box* box = nullptr;
  core::Shade shade{core::Shade::type{-1}};

  bool falling = false;
  bool is_hit = false;
  bool collidable = true;
  bool already_looked_at = false;

  std::shared_ptr<ai::CreatureInfo> creatureInfo;

  void serialize(const serialization::Serializer& ser);

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

  bool stalkBox(const Engine& engine, const loader::file::Box& targetBox) const;

  bool isInsideZoneButNotInBox(const Engine& engine, int16_t zoneId, const loader::file::Box& targetBox) const;

  bool inSameQuadrantAsBoxRelativeToLara(const Engine& engine, const loader::file::Box& targetBox) const;

  void initCreatureInfo(const Engine& engine);

  void collectZoneBoxes(const Engine& engine);

  const loader::file::Sector* getCurrentSector() const
  {
    return position.room->getSectorByAbsolutePosition(position.position);
  }

  void loadObjectInfo(const sol::state& engine);
};

} // namespace objects
} // namespace engine
