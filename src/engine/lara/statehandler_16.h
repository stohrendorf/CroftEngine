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
       && getWorld().getPresenter().getInputHandler().hasAction(hid::Action::MoveSlow))
    {
      setGoalAnimState(LaraStateId::WalkBackward);
    }
    else
    {
      setGoalAnimState(LaraStateId::Stop);
    }

    if(getWorld().getPresenter().getInputHandler().getInputState().xMovement == hid::AxisMovement::Left)
    {
      subYRotationSpeed(2.25_deg, -4_deg);
    }
    else if(getWorld().getPresenter().getInputHandler().getInputState().xMovement == hid::AxisMovement::Right)
    {
      addYRotationSpeed(2.25_deg, 4_deg);
    }
  }

  void postprocessFrame(CollisionInfo& collisionInfo) override
  {
    getLara().m_state.fallspeed = 0_spd;
    getLara().m_state.falling = false;
    collisionInfo.badPositiveDistance = core::ClimbLimit2ClickMin;
    collisionInfo.badNegativeDistance = -core::ClimbLimit2ClickMin;
    collisionInfo.badCeilingDistance = 0_len;
    collisionInfo.facingAngle = getLara().m_state.rotation.Y + 180_deg;
    setMovementAngle(collisionInfo.facingAngle);
    collisionInfo.policies |= CollisionInfo::SlopeBlockingPolicy;
    collisionInfo.initHeightInfo(getLara().m_state.position.position, getWorld(), core::LaraWalkHeight);

    if(stopIfCeilingBlocked(collisionInfo))
    {
      return;
    }

    if(checkWallCollision(collisionInfo))
    {
      setAnimation(AnimationId::STAY_SOLID, 185_frame);
    }

    if(collisionInfo.mid.floorSpace.y > core::QuarterSectorSize
       && collisionInfo.mid.floorSpace.y < core::ClimbLimit2ClickMin)
    {
      if(getLara().getSkeleton()->getFrame() < 964_frame || getLara().getSkeleton()->getFrame() > 993_frame)
      {
        setAnimation(AnimationId::WALK_DOWN_BACK_LEFT, 899_frame);
      }
      else
      {
        setAnimation(AnimationId::WALK_DOWN_BACK_RIGHT, 930_frame);
      }
    }

    if(!tryStartSlide(collisionInfo))
    {
      placeOnFloor(collisionInfo);
    }
  }
};
} // namespace engine::lara
