#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "hid/inputstate.h"

namespace engine
{
namespace lara
{
class StateHandler_15 final : public AbstractStateHandler
{
  public:
  explicit StateHandler_15(LaraNode& lara)
      : AbstractStateHandler{lara, LaraStateId::JumpPrepare}
  {
  }

  void handleInput(CollisionInfo& /*collisionInfo*/) override
  {
    if(getEngine().getInputHandler().getInputState().zMovement == hid::AxisMovement::Forward
       && getRelativeHeightAtDirection(getLara().m_state.rotation.Y, 256_len) >= -core::ClimbLimit2ClickMin)
    {
      setMovementAngle(getLara().m_state.rotation.Y);
      setGoalAnimState(LaraStateId::JumpForward);
    }
    else
    {
      if(getEngine().getInputHandler().getInputState().xMovement == hid::AxisMovement::Left
         && getRelativeHeightAtDirection(getLara().m_state.rotation.Y - 90_deg, 256_len) >= -core::ClimbLimit2ClickMin)
      {
        setMovementAngle(getLara().m_state.rotation.Y - 90_deg);
        setGoalAnimState(LaraStateId::JumpRight);
      }
      else if(getEngine().getInputHandler().getInputState().xMovement == hid::AxisMovement::Right
              && getRelativeHeightAtDirection(getLara().m_state.rotation.Y + 90_deg, 256_len)
                   >= -core::ClimbLimit2ClickMin)
      {
        setMovementAngle(getLara().m_state.rotation.Y + 90_deg);
        setGoalAnimState(LaraStateId::JumpLeft);
      }
      else if(getEngine().getInputHandler().getInputState().zMovement == hid::AxisMovement::Backward
              && getRelativeHeightAtDirection(getLara().m_state.rotation.Y + 180_deg, 256_len)
                   >= -core::ClimbLimit2ClickMin)
      {
        setMovementAngle(getLara().m_state.rotation.Y + 180_deg);
        setGoalAnimState(LaraStateId::JumpBack);
      }
    }

    if(getLara().m_state.fallspeed > core::FreeFallSpeedThreshold)
    {
      setGoalAnimState(LaraStateId::FreeFall);
    }
  }

  void postprocessFrame(CollisionInfo& collisionInfo) override
  {
    getLara().m_state.fallspeed = 0_spd;
    getLara().m_state.falling = false;
    collisionInfo.badPositiveDistance = core::HeightLimit;
    collisionInfo.badNegativeDistance = -core::HeightLimit;
    collisionInfo.badCeilingDistance = 0_len;
    collisionInfo.facingAngle = getMovementAngle();
    collisionInfo.initHeightInfo(getLara().m_state.position.position, getEngine(), core::LaraWalkHeight);

    if(collisionInfo.mid.ceilingSpace.y <= -core::DefaultCollisionRadius)
    {
      return;
    }

    setAnimation(AnimationId::STAY_SOLID, 185_frame);
    setGoalAnimState(LaraStateId::Stop);
    getLara().m_state.speed = 0_spd;
    getLara().m_state.position.position = collisionInfo.oldPosition;
  }
};
} // namespace lara
} // namespace engine
