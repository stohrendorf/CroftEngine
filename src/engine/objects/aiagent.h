#pragma once

#include "core/angle.h"
#include "core/id.h"
#include "core/units.h"
#include "core/vec.h"
#include "engine/ai/ai.h"
#include "engine/location.h"
#include "modelobject.h"
#include "objectstate.h"
#include "qs/qs.h"
#include "serialization/serialization_fwd.h"

#include <algorithm>
#include <bitset>
#include <cstddef>
#include <cstdint>
#include <gsl/gsl-lite.hpp>
#include <memory>
#include <optional>
#include <string>

namespace engine
{
struct CollisionInfo;
}

namespace engine::world
{
class World;
struct Box;
struct Room;
struct SkeletalModelType;
} // namespace engine::world

namespace loader::file
{
struct Item;
}

namespace engine::objects
{
class AIAgent : public ModelObject
{
public:
  AIAgent(const std::string& name,
          const gsl::not_null<world::World*>& world,
          const gsl::not_null<const world::Room*>& room,
          const loader::file::Item& item,
          const gsl::not_null<const world::SkeletalModelType*>& animatedModel);

  void collide(CollisionInfo& collisionInfo) override;

  void loadObjectInfo(bool withoutGameState = false);

  void serialize(const serialization::Serializer<world::World>& ser) const override;
  void deserialize(const serialization::Deserializer<world::World>& ser) override;

  void initCreatureInfo();

  [[nodiscard]] const auto& getCreatureInfo() const noexcept
  {
    return m_creatureInfo;
  }

  [[nodiscard]] bool isInsideZoneButNotInBox(uint32_t zoneId, const world::Box& targetBox) const;

protected:
  AIAgent(const gsl::not_null<world::World*>& world, const Location& location)
      : ModelObject{world, location}
  {
  }

  void rotateCreatureTilt(const core::Angle& angle)
  {
    const auto dz = 4 * angle - m_state.rotation.Z;
    m_state.rotation.Z += std::clamp(dz, -3_deg, +3_deg);
  }

  void rotateCreatureHead(const core::Angle& angle)
  {
    m_creatureInfo->rotateHead(angle);
  }

  void animateCreature(const core::Angle& collisionRotationY, const core::Angle& tilt);

  core::Angle rotateTowardsTarget(core::RotationSpeed maxRotationSpeed);

  core::Health getHealth() const noexcept
  {
    return m_state.health;
  }

  bool canShootAtLara(const ai::EnemyLocation& enemyLocation) const;

  bool tryShootAtLara(engine::objects::ModelObject& object,
                      const core::Area& distance,
                      const core::TRVec& bonePos,
                      size_t boneIndex,
                      const core::Angle& muzzleFlashAngle);

  bool alive() const
  {
    return !m_state.isDead();
  }

  bool touched() const noexcept
  {
    return m_state.touch_bits.any();
  }

  bool touched(unsigned long bits) const noexcept
  {
    return (m_state.touch_bits.to_ulong() & bits) != 0;
  }

  void hitLara(const core::Health& strength);

  void require(const core::AnimStateId& require) noexcept
  {
    m_state.required_anim_state = require;
  }

  void goal(const core::AnimStateId& goal, const std::optional<core::AnimStateId>& required = std::nullopt) noexcept
  {
    m_state.goal_anim_state = goal;
    if(required.has_value())
      require(*required);
  }

  bool isBored() const noexcept
  {
    return m_creatureInfo->mood == ai::Mood::Bored;
  }

  void bored() noexcept
  {
    m_creatureInfo->mood = ai::Mood::Bored;
  }

  bool isAttacking() const noexcept
  {
    return m_creatureInfo->mood == ai::Mood::Attack;
  }

  void attacking() noexcept
  {
    m_creatureInfo->mood = ai::Mood::Attack;
  }

  bool isStalking() const noexcept
  {
    return m_creatureInfo->mood == ai::Mood::Stalk;
  }

  void stalking() noexcept
  {
    m_creatureInfo->mood = ai::Mood::Stalk;
  }

  bool isEscaping() const noexcept
  {
    return m_creatureInfo->mood == ai::Mood::Escape;
  }

  void escaping() noexcept
  {
    m_creatureInfo->mood = ai::Mood::Escape;
  }

  void settle() noexcept
  {
    m_state.location.position.Y = m_state.floor;
    m_state.falling = false;
  }

  void activateAi()
  {
    if(m_state.triggerState == TriggerState::Invisible)
    {
      m_state.triggerState = TriggerState::Active;
    }

    initCreatureInfo();
  }

  void freeCreatureInfo() noexcept
  {
    m_creatureInfo.reset();
  }

private:
  bool anyMovingEnabledObjectInReach() const;

  core::Length m_collisionRadius = 0_len;

  std::unique_ptr<ai::CreatureInfo> m_creatureInfo;
};

#define AIAGENT_DEFAULT_CONSTRUCTORS(CLASS)                                  \
  CLASS(const gsl::not_null<world::World*>& world, const Location& location) \
      : AIAgent{world, location}                                             \
  {                                                                          \
  }                                                                          \
                                                                             \
  CLASS(const std::string& name,                                             \
        const gsl::not_null<world::World*>& world,                           \
        const gsl::not_null<const world::Room*>& room,                       \
        const loader::file::Item& item,                                      \
        const gsl::not_null<const world::SkeletalModelType*>& animatedModel) \
      : AIAgent{name, world, room, item, animatedModel}                      \
  {                                                                          \
  }

} // namespace engine::objects
