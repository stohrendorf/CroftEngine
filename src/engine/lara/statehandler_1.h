#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "engine/objects/laraobject.h"
#include "hid/inputstate.h"

namespace engine::lara
{
class StateHandler_1 final : public AbstractStateHandler
{
public:
  explicit StateHandler_1(objects::LaraObject& lara)
      : AbstractStateHandler{lara, LaraStateId::RunForward}
  {
  }

  void handleInput(CollisionInfo& /*collisionInfo*/) override
  {
    if(getLara().isDead())
    {
      setGoalAnimState(LaraStateId::Death);
      return;
    }

    if(getWorld().getPresenter().getInputHandler().hasAction(hid::Action::Roll))
    {
      setAnimation(AnimationId::ROLL_BEGIN, 3857_frame);
      setGoalAnimState(LaraStateId::Stop);
      setCurrentAnimState(LaraStateId::RollForward);
      return;
    }

    if(getWorld().getPresenter().getInputHandler().getInputState().xMovement == hid::AxisMovement::Left)
    {
      subYRotationSpeed(2.25_deg, -8_deg);
      const core::Angle z = std::max(-11_deg, getLara().m_state.rotation.Z - 1.5_deg);
      getLara().m_state.rotation.Z = z;
    }
    else if(getWorld().getPresenter().getInputHandler().getInputState().xMovement == hid::AxisMovement::Right)
    {
      addYRotationSpeed(2.25_deg, 8_deg);
      const core::Angle z = std::min(+11_deg, getLara().m_state.rotation.Z + 1.5_deg);
      getLara().m_state.rotation.Z = z;
    }

    if(getWorld().getPresenter().getInputHandler().hasAction(hid::Action::Jump) && !getLara().m_state.falling)
    {
      setGoalAnimState(LaraStateId::JumpForward);
      return;
    }

    if(getWorld().getPresenter().getInputHandler().getInputState().zMovement != hid::AxisMovement::Forward)
    {
      setGoalAnimState(LaraStateId::Stop);
      return;
    }

    if(getWorld().getPresenter().getInputHandler().hasAction(hid::Action::MoveSlow))
    {
      setGoalAnimState(LaraStateId::WalkForward);
    }
    else
    {
      setGoalAnimState(LaraStateId::RunForward);
    }
  }

  void postprocessFrame(CollisionInfo& collisionInfo) override
  {
    collisionInfo.facingAngle = getLara().m_state.rotation.Y;
    setMovementAngle(collisionInfo.facingAngle);
    collisionInfo.badPositiveDistance = core::HeightLimit;
    collisionInfo.badNegativeDistance = -core::ClimbLimit2ClickMin;
    collisionInfo.badCeilingDistance = 0_len;
    collisionInfo.policyFlags.set(CollisionInfo::PolicyFlags::SlopesAreWalls);
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
      getLara().m_state.rotation.Z = 0_deg;
      if(collisionInfo.front.floorSpace.slantClass == SlantClass::None
         && collisionInfo.front.floorSpace.y < -core::ClimbLimit2ClickMax)
      {
        if(getLara().getSkeleton()->getFrame() < 10_frame)
        {
          setAnimation(AnimationId::WALL_SMASH_LEFT, 800_frame);
          setCurrentAnimState(LaraStateId::Unknown12);
          return;
        }
        if(getLara().getSkeleton()->getFrame() >= 10_frame && getLara().getSkeleton()->getFrame() < 22_frame)
        {
          setAnimation(AnimationId::WALL_SMASH_RIGHT, 815_frame);
          setCurrentAnimState(LaraStateId::Unknown12);
          return;
        }
      }

      setAnimation(AnimationId::STAY_SOLID, 185_frame);
    }

    if(collisionInfo.mid.floorSpace.y > core::ClimbLimit2ClickMin)
    {
      setAnimation(AnimationId::FREE_FALL_FORWARD, 492_frame);
      setGoalAnimState(LaraStateId::JumpForward);
      setCurrentAnimState(LaraStateId::JumpForward);
      getLara().m_state.falling = true;
      getLara().m_state.fallspeed = 0_spd;
      return;
    }

    if(collisionInfo.mid.floorSpace.y >= -core::ClimbLimit2ClickMin
       && collisionInfo.mid.floorSpace.y < -core::SteppableHeight)
    {
      if(getLara().getSkeleton()->getFrame() >= 3_frame && getLara().getSkeleton()->getFrame() <= 14_frame)
      {
        setAnimation(AnimationId::RUN_UP_STEP_LEFT, 837_frame);
      }
      else
      {
        setAnimation(AnimationId::RUN_UP_STEP_RIGHT, 830_frame);
      }
    }

    if(!tryStartSlide(collisionInfo))
    {
      getLara().m_state.position.position.Y += std::min(collisionInfo.mid.floorSpace.y, 50_len);
    }
  }
};
} // namespace engine::lara
