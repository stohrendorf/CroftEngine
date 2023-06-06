#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "engine/objects/laraobject.h"
#include "engine/presenter.h"
#include "engine/skeletalmodelnode.h"
#include "engine/world/world.h"
#include "hid/inputhandler.h"

namespace engine::lara
{
class StateHandler_0 final : public AbstractStateHandler
{
public:
  explicit StateHandler_0(const gsl::not_null<objects::LaraObject*>& lara)
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
      subYRotationSpeed(core::SlowTurnSpeedAcceleration, -core::SlowTurnSpeed);
    }
    else if(getWorld().getPresenter().getInputHandler().getInputState().xMovement == hid::AxisMovement::Right)
    {
      addYRotationSpeed(core::SlowTurnSpeedAcceleration, core::SlowTurnSpeed);
    }

    if(getWorld().getPresenter().getInputHandler().getInputState().zMovement == hid::AxisMovement::Forward)
    {
      if(getWorld().getPresenter().getInputHandler().hasAction(hid::Action::Walk))
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
    auto& laraState = getLara().m_state;
    collisionInfo.facingAngle = laraState.rotation.Y;
    collisionInfo.validFloorHeight = {-core::ClimbLimit2ClickMin, core::ClimbLimit2ClickMin};
    collisionInfo.validCeilingHeightMin = 0_len;
    collisionInfo.policies |= CollisionInfo::SlopeBlockingPolicy;
    collisionInfo.policies.set(CollisionInfo::PolicyFlags::LavaIsPit);
    collisionInfo.initHeightInfo(laraState.location.position, getWorld(), core::LaraWalkHeight);

    laraState.fallspeed = 0_spd;
    laraState.falling = false;
    setMovementAngle(collisionInfo.facingAngle);

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
        setAnimation(AnimationId::END_WALK_LEFT);
      }
      else if((fr >= 22_frame && fr <= 28_frame) || (fr >= 48_frame && fr <= 57_frame))
      {
        setAnimation(AnimationId::END_WALK_RIGHT);
      }
      else
      {
        setAnimation(AnimationId::STAY_SOLID);
      }
    }

    if(collisionInfo.mid.floor.dy > core::ClimbLimit2ClickMin)
    {
      setAnimation(AnimationId::FREE_FALL_FORWARD);
      setGoalAnimState(LaraStateId::JumpForward);
      setCurrentAnimState(LaraStateId::JumpForward);
      getLara().m_state.fallspeed = 0_spd;
      getLara().m_state.falling = true;
    }

    if(collisionInfo.mid.floor.dy > core::SteppableHeight)
    {
      const auto fr = getLara().getSkeleton()->getFrame();
      if(fr < 28_frame || fr > 45_frame)
      {
        setAnimation(AnimationId::WALK_DOWN_RIGHT);
      }
      else
      {
        setAnimation(AnimationId::WALK_DOWN_LEFT);
      }
    }

    if(collisionInfo.mid.floor.dy >= -core::ClimbLimit2ClickMin && collisionInfo.mid.floor.dy < -core::SteppableHeight)
    {
      const auto fr = getLara().getSkeleton()->getFrame();
      if(fr < 27_frame || fr > 44_frame)
      {
        setAnimation(AnimationId::WALK_UP_STEP_RIGHT);
      }
      else
      {
        setAnimation(AnimationId::WALK_UP_STEP_LEFT);
      }
    }

    if(!tryStartSlide(collisionInfo))
    {
      placeOnFloor(collisionInfo);
    }
  }
};
} // namespace engine::lara
