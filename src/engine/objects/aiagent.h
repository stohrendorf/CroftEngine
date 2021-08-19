#pragma once

#include "engine/ai/ai.h"
#include "modelobject.h"

namespace engine
{
namespace ai
{
struct EnemyLocation;
}

namespace objects
{
class AIAgent : public ModelObject
{
public:
  AIAgent(const gsl::not_null<world::World*>& world,
          const gsl::not_null<const world::Room*>& room,
          const loader::file::Item& item,
          const gsl::not_null<const world::SkeletalModelType*>& animatedModel);

  void collide(CollisionInfo& collisionInfo) override;

  void loadObjectInfo(bool withoutGameState = false);

  void serialize(const serialization::Serializer<world::World>& ser) override;

  void initCreatureInfo();

  [[nodiscard]] const auto& getCreatureInfo() const
  {
    return m_creatureInfo;
  }

  [[nodiscard]] bool isInsideZoneButNotInBox(world::ZoneId zoneId, const world::Box& targetBox) const;

protected:
  AIAgent(const gsl::not_null<world::World*>& world, const Location& location)
      : ModelObject{world, location}
  {
  }

  void rotateCreatureTilt(const core::Angle& angle)
  {
    const auto dz = 4 * angle - m_state.rotation.Z;
    const core::Angle z = std::clamp(dz, -3_deg, +3_deg);
    m_state.rotation.Z += z;
  }

  // ReSharper disable once CppMemberFunctionMayBeConst
  void rotateCreatureHead(const core::Angle& angle)
  {
    m_creatureInfo->rotateHead(angle);
  }

  bool animateCreature(const core::Angle& angle, const core::Angle& tilt);

  core::Angle rotateTowardsTarget(core::RotationSpeed maxRotationSpeed);

  core::Health getHealth() const
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

  bool touched() const
  {
    return m_state.touch_bits.any();
  }

  bool touched(unsigned long bits) const
  {
    return (m_state.touch_bits.to_ulong() & bits) != 0;
  }

  void hitLara(const core::Health& strength);

  void require(const core::AnimStateId& require)
  {
    m_state.required_anim_state = require;
  }

  void goal(const core::AnimStateId& goal, const std::optional<core::AnimStateId>& required = std::nullopt)
  {
    m_state.goal_anim_state = goal;
    if(required.has_value())
      require(*required);
  }

  bool isBored() const
  {
    return m_creatureInfo->mood == ai::Mood::Bored;
  }

  void bored()
  {
    m_creatureInfo->mood = ai::Mood::Bored;
  }

  bool isAttacking() const
  {
    return m_creatureInfo->mood == ai::Mood::Attack;
  }

  void attacking()
  {
    m_creatureInfo->mood = ai::Mood::Attack;
  }

  bool isStalking() const
  {
    return m_creatureInfo->mood == ai::Mood::Stalk;
  }

  void stalking()
  {
    m_creatureInfo->mood = ai::Mood::Stalk;
  }

  bool isEscaping() const
  {
    return m_creatureInfo->mood == ai::Mood::Escape;
  }

  void escaping()
  {
    m_creatureInfo->mood = ai::Mood::Escape;
  }

  void settle()
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

  void freeCreatureInfo()
  {
    m_creatureInfo.reset();
  }

private:
  bool anyMovingEnabledObjectInReach() const;

  bool isPositionOutOfReach(const core::TRVec& testPosition,
                            const core::Length& currentBoxFloor,
                            const core::Length& nextBoxFloor,
                            const ai::PathFinder& pathFinder) const;

  core::Length m_collisionRadius = 0_len;

  std::unique_ptr<ai::CreatureInfo> m_creatureInfo;
};

#define AIAGENT_DEFAULT_CONSTRUCTORS(CLASS)                                  \
  CLASS(const gsl::not_null<world::World*>& world, const Location& location) \
      : AIAgent{world, location}                                             \
  {                                                                          \
  }                                                                          \
                                                                             \
  CLASS(const gsl::not_null<world::World*>& world,                           \
        const gsl::not_null<const world::Room*>& room,                       \
        const loader::file::Item& item,                                      \
        const gsl::not_null<const world::SkeletalModelType*>& animatedModel) \
      : AIAgent{world, room, item, animatedModel}                            \
  {                                                                          \
  }

} // namespace objects
} // namespace engine
