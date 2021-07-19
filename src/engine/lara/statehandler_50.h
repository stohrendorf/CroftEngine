#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "engine/particle.h"

namespace engine::lara
{
class StateHandler_50 final : public AbstractStateHandler
{
public:
  explicit StateHandler_50(objects::LaraObject& lara)
      : AbstractStateHandler{lara, LaraStateId::UseMidas}
  {
  }

  void handleInput(CollisionInfo& collisionInfo) override
  {
    collisionInfo.policies &= ~CollisionInfo::SpazPushPolicy;
    emitSparkles(getWorld());
  }

  void postprocessFrame(CollisionInfo& collisionInfo) override
  {
    collisionInfo.badPositiveDistance = core::ClimbLimit2ClickMin;
    collisionInfo.badNegativeDistance = -core::ClimbLimit2ClickMin;
    collisionInfo.badCeilingDistance = 0_len;
    setMovementAngle(getLara().m_state.rotation.Y);
    collisionInfo.policies |= CollisionInfo::SlopeBlockingPolicy;
    collisionInfo.facingAngle = getLara().m_state.rotation.Y;
    collisionInfo.initHeightInfo(getLara().m_state.location.position, getWorld(), core::LaraWalkHeight);
  }

  static void emitSparkles(world::World& world)
  {
    const auto spheres = world.getObjectManager().getLara().getSkeleton()->getBoneCollisionSpheres(
      world.getObjectManager().getLara().m_state,
      *world.getObjectManager().getLara().getSkeleton()->getInterpolationInfo().getNearestFrame(),
      nullptr);

    const auto& normalLara = world.findAnimatedModelForType(TR1ItemId::Lara);
    Expects(normalLara != nullptr);
    for(size_t i = 0; i < spheres.size(); ++i)
    {
      if(world.getObjectManager().getLara().getSkeleton()->getMeshPart(i) == normalLara->bones[i].mesh)
        continue;

      const auto r = spheres[i].radius;
      auto p = core::TRVec{spheres[i].getPosition()};
      p.X += util::rand15s(r);
      p.Y += util::rand15s(r);
      p.Z += util::rand15s(r);
      auto fx = std::make_shared<SparkleParticle>(
        RoomBoundPosition{world.getObjectManager().getLara().m_state.location.room, p}, world);
      world.getObjectManager().registerParticle(fx);
    }
  }
};
} // namespace engine::lara
