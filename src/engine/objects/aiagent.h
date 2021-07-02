#pragma once

#include "engine/ai/ai.h"
#include "modelobject.h"

namespace engine
{
namespace ai
{
struct AiInfo;
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

protected:
  AIAgent(const gsl::not_null<world::World*>& world, const core::RoomBoundPosition& position)
      : ModelObject{world, position}
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
    m_state.creatureInfo->rotateHead(angle);
  }

  bool animateCreature(const core::Angle& angle, const core::Angle& tilt);

  core::Angle rotateTowardsTarget(core::Angle maxRotationSpeed);

  core::Health getHealth() const
  {
    return m_state.health;
  }

  bool canShootAtLara(const ai::AiInfo& aiInfo) const;

  bool tryShootAtLara(engine::objects::ModelObject& object,
                      const core::Area& distance,
                      const core::TRVec& bonePos,
                      size_t boneIndex,
                      const core::Angle& angle);

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
      require(required.value());
  }

  bool isBored() const
  {
    return m_state.creatureInfo->mood == ai::Mood::Bored;
  }

  void bored()
  {
    m_state.creatureInfo->mood = ai::Mood::Bored;
  }

  bool isAttacking() const
  {
    return m_state.creatureInfo->mood == ai::Mood::Attack;
  }

  void attacking()
  {
    m_state.creatureInfo->mood = ai::Mood::Attack;
  }

  bool isStalking() const
  {
    return m_state.creatureInfo->mood == ai::Mood::Stalk;
  }

  void stalking()
  {
    m_state.creatureInfo->mood = ai::Mood::Stalk;
  }

  bool isEscaping() const
  {
    return m_state.creatureInfo->mood == ai::Mood::Escape;
  }

  void escaping()
  {
    m_state.creatureInfo->mood = ai::Mood::Escape;
  }

  void settle()
  {
    m_state.position.position.Y = m_state.floor;
    m_state.falling = false;
  }

  void activateAi()
  {
    if(m_state.triggerState == TriggerState::Invisible)
    {
      m_state.triggerState = TriggerState::Active;
    }

    m_state.initCreatureInfo(getWorld());
  }

private:
  bool anyMovingEnabledObjectInReach() const;

  bool isPositionOutOfReach(const core::TRVec& testPosition,
                            const core::Length& currentBoxFloor,
                            const core::Length& nextBoxFloor,
                            const ai::PathFinder& pathFinder) const;

  core::Length m_collisionRadius = 0_len;
};
} // namespace objects
} // namespace engine
