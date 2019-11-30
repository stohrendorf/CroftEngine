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
  AIAgent(const gsl::not_null<Engine*>& engine,
          const gsl::not_null<const loader::file::Room*>& room,
          const loader::file::Item& item,
          const gsl::not_null<const loader::file::SkeletalModelType*>& animatedModel);

  void collide(CollisionInfo& collisionInfo) override;

  void loadObjectInfo(bool withoutGameState = false);

protected:
  AIAgent(const gsl::not_null<Engine*>& engine, const core::RoomBoundPosition& position)
      : ModelObject{engine, position}
  {
  }

  void rotateCreatureTilt(const core::Angle& angle)
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

private:
  bool anyMovingEnabledObjectInReach() const;

  bool isPositionOutOfReach(const core::TRVec& testPosition,
                            const core::Length& currentBoxFloor,
                            const core::Length& nextBoxFloor,
                            const ai::PathFinder& lotInfo) const;

  core::Length m_collisionRadius = 0_len;
};
} // namespace objects
} // namespace engine
