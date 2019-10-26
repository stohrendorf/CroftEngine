#pragma once

#include "engine/ai/ai.h"
#include "itemnode.h"

namespace engine
{
namespace ai
{
struct AiInfo;
}

namespace items
{
class AIAgent : public ModelItemNode
{
  public:
  AIAgent(const gsl::not_null<Engine*>& engine,
          const gsl::not_null<const loader::file::Room*>& room,
          const loader::file::Item& item,
          const loader::file::SkeletalModelType& animatedModel);

  void collide(LaraNode& lara, CollisionInfo& collisionInfo) override;

  void loadObjectInfo(bool withoutGameState = false);

  protected:
  void rotateCreatureTilt(core::Angle angle)
  {
    const auto dz = 4 * angle - m_state.rotation.Z;
    const core::Angle z = util::clamp(dz, -3_deg, +3_deg);
    m_state.rotation.Z += z;
  }

  // ReSharper disable once CppMemberFunctionMayBeConst
  void rotateCreatureHead(const core::Angle& angle)
  {
    m_state.creatureInfo->rotateHead(angle);
  }

  bool animateCreature(core::Angle angle, core::Angle tilt);

  core::Angle rotateTowardsTarget(core::Angle maxRotationSpeed);

  core::Health getHealth() const
  {
    return m_state.health;
  }

  bool canShootAtLara(const ai::AiInfo& aiInfo) const;

  bool tryShootAtLara(engine::items::ModelItemNode& item,
                      core::Area distance,
                      const core::TRVec& bonePos,
                      size_t boneIndex,
                      core::Angle angle);

  private:
  bool anyMovingEnabledItemInReach() const;

  bool isPositionOutOfReach(const core::TRVec& testPosition,
                            core::Length currentBoxFloor,
                            core::Length nextBoxFloor,
                            const ai::PathFinder& lotInfo) const;

  core::Length m_collisionRadius = 0_len;
};
} // namespace items
} // namespace engine
