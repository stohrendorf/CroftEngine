#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "engine/particle.h"
#include "engine/world/skeletalmodeltype.h"

#include <gslu.h>

namespace engine::lara
{
class StateHandler_50 final : public AbstractStateHandler
{
public:
  explicit StateHandler_50(const gsl::not_null<objects::LaraObject*>& lara)
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
    collisionInfo.validFloorHeight = {-core::ClimbLimit2ClickMin, core::ClimbLimit2ClickMin};
    collisionInfo.validCeilingHeightMin = 0_len;
    collisionInfo.policies |= CollisionInfo::SlopeBlockingPolicy;
    collisionInfo.facingAngle = getLara().m_state.rotation.Y;
    collisionInfo.initHeightInfo(getLara().m_state.location.position, getWorld(), core::LaraWalkHeight);

    setMovementAngle(collisionInfo.facingAngle);
  }

  static void emitSparkles(world::World& world)
  {
    const auto spheres = world.getObjectManager().getLara().getSkeleton()->getBoneCollisionSpheres();

    const auto& normalLara = world.findAnimatedModelForType(TR1ItemId::Lara);
    gsl_Assert(normalLara != nullptr);
    for(size_t i = 0; i < spheres.size(); ++i)
    {
      if(world.getObjectManager().getLara().getSkeleton()->getMesh(i) == normalLara->bones[i].mesh)
        continue;

      const auto r = spheres[i].radius;
      auto p = core::TRVec{spheres[i].getCollisionPosition()};
      p.X += util::rand15s(r);
      p.Y += util::rand15s(r);
      p.Z += util::rand15s(r);
      auto fx = gsl::make_shared<SparkleParticle>(Location{world.getObjectManager().getLara().m_state.location.room, p},
                                                  world);
      world.getObjectManager().registerParticle(fx);
    }
  }
};
} // namespace engine::lara
