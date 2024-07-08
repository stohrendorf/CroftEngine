#pragma once

#include "abstractstatehandler.h"
#include "core/magic.h"
#include "core/units.h"
#include "engine/collisioninfo.h"
#include "hid/actions.h"
#include "hid/inputstate.h"

#include <gsl/gsl-lite.hpp>

namespace engine::lara
{
class StateHandler_16 final : public AbstractStateHandler
{
public:
  explicit StateHandler_16(const gsl::not_null<objects::LaraObject*>& lara)
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

    const auto& inputHandler = getWorld().getPresenter().getInputHandler();
    if(inputHandler.getInputState().zMovement == hid::AxisMovement::Backward
       && inputHandler.hasAction(hid::Action::Walk))
    {
      setGoalAnimState(LaraStateId::WalkBackward);
    }
    else
    {
      setGoalAnimState(LaraStateId::Stop);
    }

    if(inputHandler.getInputState().xMovement == hid::AxisMovement::Left)
    {
      subYRotationSpeed(core::SlowTurnSpeedAcceleration, -core::SlowTurnSpeed);
    }
    else if(inputHandler.getInputState().xMovement == hid::AxisMovement::Right)
    {
      addYRotationSpeed(core::SlowTurnSpeedAcceleration, core::SlowTurnSpeed);
    }
  }

  void postprocessFrame(CollisionInfo& collisionInfo) override
  {
    auto& laraState = getLara().m_state;
    collisionInfo.validFloorHeight = {-core::ClimbLimit2ClickMin, core::ClimbLimit2ClickMin};
    collisionInfo.validCeilingHeightMin = 0_len;
    collisionInfo.facingAngle = laraState.rotation.Y + 180_deg;
    collisionInfo.policies |= CollisionInfo::SlopeBlockingPolicy;
    collisionInfo.initHeightInfo(laraState.location.position, getWorld(), core::LaraWalkHeight);

    setMovementAngle(collisionInfo.facingAngle);
    laraState.fallspeed = 0_spd;
    laraState.falling = false;

    if(stopIfCeilingBlocked(collisionInfo))
    {
      return;
    }

    if(checkWallCollision(collisionInfo))
    {
      setAnimation(AnimationId::STAY_SOLID);
    }

    if(collisionInfo.mid.floor.dy > core::QuarterSectorSize && collisionInfo.mid.floor.dy < core::ClimbLimit2ClickMin)
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
