#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "hid/inputstate.h"

namespace engine::lara
{
class StateHandler_5 final : public AbstractStateHandler
{
public:
  explicit StateHandler_5(const gsl::not_null<objects::LaraObject*>& lara)
      : AbstractStateHandler{lara, LaraStateId::RunBack}
  {
  }

  void handleInput(CollisionInfo& /*collisionInfo*/) override
  {
    setGoalAnimState(LaraStateId::Stop);

    const auto& inputHandler = getWorld().getPresenter().getInputHandler();
    switch(inputHandler.getInputState().xMovement)
    {
    case hid::AxisMovement::Right:
      addYRotationSpeed(core::SlowTurnSpeedAcceleration, core::RunBackTurnSpeed);
      break;
    case hid::AxisMovement::Left:
      subYRotationSpeed(core::SlowTurnSpeedAcceleration, -core::RunBackTurnSpeed);
      break;
    case hid::AxisMovement::Null:
      break;
    }
  }

  void postprocessFrame(CollisionInfo& collisionInfo) override
  {
    auto& laraState = getLara().m_state;
    collisionInfo.validFloorHeight = {-core::ClimbLimit2ClickMin, core::HeightLimit};
    collisionInfo.validCeilingHeightMin = 0_len;
    collisionInfo.policies |= CollisionInfo::SlopeBlockingPolicy;
    collisionInfo.facingAngle = laraState.rotation.Y + 180_deg;
    collisionInfo.initHeightInfo(laraState.location.position, getWorld(), core::LaraWalkHeight);

    setMovementAngle(collisionInfo.facingAngle);
    laraState.fallspeed = 0_spd;
    laraState.falling = false;

    if(stopIfCeilingBlocked(collisionInfo))
    {
      return;
    }

    if(collisionInfo.mid.floor.y > 200_len)
    {
      setAnimation(AnimationId::FREE_FALL_BACK);
      setGoalAnimState(LaraStateId::FallBackward);
      setCurrentAnimState(LaraStateId::FallBackward);
      getLara().m_state.fallspeed = 0_spd;
      getLara().m_state.falling = true;
      return;
    }

    if(checkWallCollision(collisionInfo))
    {
      setAnimation(AnimationId::STAY_SOLID);
    }
    placeOnFloor(collisionInfo);
  }
};
} // namespace engine::lara
