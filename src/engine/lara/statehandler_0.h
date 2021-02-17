#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "engine/objects/laraobject.h"
#include "engine/presenter.h"
#include "engine/world.h"
#include "hid/inputhandler.h"

namespace engine::lara
{
class StateHandler_0 final : public AbstractStateHandler
{
public:
  explicit StateHandler_0(objects::LaraObject& lara)
      : AbstractStateHandler{lara, LaraStateId::WalkForward}
  {
  }

  void handleInput(CollisionInfo& /*collisionInfo*/) override
  {
    if(getLara().isDead())
    {
      setGoalAnimState(LaraStateId::Stop);
      return;
    }

    if(getWorld().getPresenter().getInputHandler().getInputState().xMovement == hid::AxisMovement::Left)
    {
      subYRotationSpeed(2.25_deg, -4_deg);
    }
    else if(getWorld().getPresenter().getInputHandler().getInputState().xMovement == hid::AxisMovement::Right)
    {
      addYRotationSpeed(2.25_deg, 4_deg);
    }

    if(getWorld().getPresenter().getInputHandler().getInputState().zMovement == hid::AxisMovement::Forward)
    {
      if(getWorld().getPresenter().getInputHandler().hasAction(hid::Action::MoveSlow))
      {
        setGoalAnimState(LaraStateId::WalkForward);
      }
      else
      {
        setGoalAnimState(LaraStateId::RunForward);
      }
    }
    else
    {
      setGoalAnimState(LaraStateId::Stop);
    }
  }

  void postprocessFrame(CollisionInfo& collisionInfo) override
  {
    getLara().m_state.fallspeed = 0_spd;
    getLara().m_state.falling = false;
    collisionInfo.facingAngle = getLara().m_state.rotation.Y;
    setMovementAngle(collisionInfo.facingAngle);
    collisionInfo.badPositiveDistance = core::ClimbLimit2ClickMin;
    collisionInfo.badNegativeDistance = -core::ClimbLimit2ClickMin;
    collisionInfo.badCeilingDistance = 0_len;
    collisionInfo.policyFlags |= CollisionInfo::SlopeBlockingPolicy;
    collisionInfo.policyFlags.set(CollisionInfo::PolicyFlags::LavaIsPit);
    collisionInfo.initHeightInfo(getLara().m_state.position.position, getWorld(), core::LaraWalkHeight);

    if(stopIfCeilingBlocked(collisionInfo))
    {
      return;
    }

    if(tryClimb(collisionInfo))
    {
      return;
    }

    if(checkWallCollision(collisionInfo))
    {
      const auto fr = getLara().getSkeleton()->getFrame();
      if(fr >= 29_frame && fr <= 47_frame)
      {
        setAnimation(AnimationId::END_WALK_LEFT, 74_frame);
      }
      else if((fr >= 22_frame && fr <= 28_frame) || (fr >= 48_frame && fr <= 57_frame))
      {
        setAnimation(AnimationId::END_WALK_RIGHT, 58_frame);
      }
      else
      {
        setAnimation(AnimationId::STAY_SOLID, 185_frame);
      }
    }

    if(collisionInfo.mid.floorSpace.y > core::ClimbLimit2ClickMin)
    {
      setAnimation(AnimationId::FREE_FALL_FORWARD, 492_frame);
      setGoalAnimState(LaraStateId::JumpForward);
      setCurrentAnimState(LaraStateId::JumpForward);
      getLara().m_state.fallspeed = 0_spd;
      getLara().m_state.falling = true;
    }

    if(collisionInfo.mid.floorSpace.y > core::SteppableHeight)
    {
      const auto fr = getLara().getSkeleton()->getFrame();
      if(fr < 28_frame || fr > 45_frame)
      {
        setAnimation(AnimationId::WALK_DOWN_RIGHT, 887_frame);
      }
      else
      {
        setAnimation(AnimationId::WALK_DOWN_LEFT, 874_frame);
      }
    }

    if(collisionInfo.mid.floorSpace.y >= -core::ClimbLimit2ClickMin
       && collisionInfo.mid.floorSpace.y < -core::SteppableHeight)
    {
      const auto fr = getLara().getSkeleton()->getFrame();
      if(fr < 27_frame || fr > 44_frame)
      {
        setAnimation(AnimationId::WALK_UP_STEP_RIGHT, 844_frame);
      }
      else
      {
        setAnimation(AnimationId::WALK_UP_STEP_LEFT, 858_frame);
      }
    }

    if(!tryStartSlide(collisionInfo))
    {
      placeOnFloor(collisionInfo);
    }
  }
};
} // namespace engine::lara
