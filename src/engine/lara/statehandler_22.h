#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "hid/inputstate.h"

namespace engine::lara
{
class StateHandler_22 final : public AbstractStateHandler
{
public:
  explicit StateHandler_22(objects::LaraObject& lara)
      : AbstractStateHandler{lara, LaraStateId::StepLeft}
  {
  }

  void handleInput(CollisionInfo& /*collisionInfo*/) override
  {
    if(getLara().isDead())
    {
      setGoalAnimState(LaraStateId::Stop);
      return;
    }

    if(getWorld().getPresenter().getInputHandler().getInputState().stepMovement != hid::AxisMovement::Left)
    {
      setGoalAnimState(LaraStateId::Stop);
    }

    if(getWorld().getPresenter().getInputHandler().getInputState().xMovement == hid::AxisMovement::Left)
    {
      subYRotationSpeed(core::SlowTurnSpeedAcceleration, -core::SlowTurnSpeed);
    }
    else if(getWorld().getPresenter().getInputHandler().getInputState().xMovement == hid::AxisMovement::Right)
    {
      addYRotationSpeed(core::SlowTurnSpeedAcceleration, core::SlowTurnSpeed);
    }
  }

  void postprocessFrame(CollisionInfo& collisionInfo) override
  {
    auto& laraState = getLara().m_state;
    laraState.fallspeed = 0_spd;
    laraState.falling = false;
    collisionInfo.validFloorHeight = {-core::SteppableHeight, core::SteppableHeight};
    collisionInfo.validCeilingHeightMin = 0_len;
    collisionInfo.facingAngle = laraState.rotation.Y - 90_deg;
    setMovementAngle(collisionInfo.facingAngle);
    collisionInfo.policies |= CollisionInfo::SlopeBlockingPolicy;
    collisionInfo.initHeightInfo(laraState.location.position, getWorld(), core::LaraWalkHeight);

    if(stopIfCeilingBlocked(collisionInfo))
    {
      return;
    }

    if(checkWallCollision(collisionInfo))
    {
      setAnimation(AnimationId::STAY_SOLID);
      setGoalAnimState(LaraStateId::Stop);
      setCurrentAnimState(LaraStateId::Stop);
    }

    if(!tryStartSlide(collisionInfo))
    {
      placeOnFloor(collisionInfo);
    }
  }
};
} // namespace engine::lara
