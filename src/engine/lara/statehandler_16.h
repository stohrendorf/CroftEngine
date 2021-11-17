#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "hid/inputstate.h"

namespace engine::lara
{
class StateHandler_16 final : public AbstractStateHandler
{
public:
  explicit StateHandler_16(objects::LaraObject& lara)
      : AbstractStateHandler{lara, LaraStateId::WalkBackward}
  {
  }

  void handleInput(CollisionInfo& /*collisionInfo*/) override
  {
    if(getLara().isDead())
    {
      setGoalAnimState(LaraStateId::Stop);
      return;
    }

    if(getWorld().getPresenter().getInputHandler().getInputState().zMovement == hid::AxisMovement::Backward
       && getWorld().getPresenter().getInputHandler().hasAction(hid::Action::Walk))
    {
      setGoalAnimState(LaraStateId::WalkBackward);
    }
    else
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
    collisionInfo.validFloorHeight = {-core::ClimbLimit2ClickMin, core::ClimbLimit2ClickMin};
    collisionInfo.validCeilingHeightMin = 0_len;
    collisionInfo.facingAngle = laraState.rotation.Y + 180_deg;
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
    }

    if(collisionInfo.mid.floor.y > core::QuarterSectorSize && collisionInfo.mid.floor.y < core::ClimbLimit2ClickMin)
    {
      if(getLara().getSkeleton()->getFrame() < 964_frame || getLara().getSkeleton()->getFrame() > 993_frame)
      {
        setAnimation(AnimationId::WALK_DOWN_BACK_LEFT);
      }
      else
      {
        setAnimation(AnimationId::WALK_DOWN_BACK_RIGHT);
      }
    }

    if(!tryStartSlide(collisionInfo))
    {
      placeOnFloor(collisionInfo);
    }
  }
};
} // namespace engine::lara
