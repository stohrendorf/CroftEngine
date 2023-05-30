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
  explicit StateHandler_1(const gsl::not_null<objects::LaraObject*>& lara)
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

    const auto& inputHandler = getWorld().getPresenter().getInputHandler();
    if(inputHandler.hasAction(hid::Action::Roll))
    {
      setAnimation(AnimationId::ROLL_BEGIN,
                   getWorld().getWorldGeometry().getAnimation(AnimationId::ROLL_BEGIN).firstFrame + 2_frame);
      setGoalAnimState(LaraStateId::Stop);
      setCurrentAnimState(LaraStateId::RollForward);
      return;
    }

    if(inputHandler.getInputState().xMovement == hid::AxisMovement::Left)
    {
      subYRotationSpeed(core::SlowTurnSpeedAcceleration, -core::FastTurnSpeed);
      const core::Angle z
        = std::max(-core::MaxRunTilt, getLara().m_state.rotation.Z - core::RunTiltAcceleration * 1_frame);
      getLara().m_state.rotation.Z = z;
    }
    else if(inputHandler.getInputState().xMovement == hid::AxisMovement::Right)
    {
      addYRotationSpeed(core::SlowTurnSpeedAcceleration, core::FastTurnSpeed);
      const core::Angle z
        = std::min(core::MaxRunTilt, getLara().m_state.rotation.Z + core::RunTiltAcceleration * 1_frame);
      getLara().m_state.rotation.Z = z;
    }

    if(inputHandler.hasAction(hid::Action::Jump) && !getLara().m_state.falling)
    {
      setGoalAnimState(LaraStateId::JumpForward);
      return;
    }

    if(inputHandler.getInputState().zMovement != hid::AxisMovement::Forward)
    {
      setGoalAnimState(LaraStateId::Stop);
      return;
    }

    if(inputHandler.hasAction(hid::Action::Walk))
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
    collisionInfo.validFloorHeight = {-core::ClimbLimit2ClickMin, core::HeightLimit};
    collisionInfo.validCeilingHeightMin = 0_len;
    collisionInfo.policies.set(CollisionInfo::PolicyFlags::SlopesAreWalls);
    collisionInfo.initHeightInfo(getLara().m_state.location.position, getWorld(), core::LaraWalkHeight);

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
      getLara().m_state.rotation.Z = 0_deg;
      if(collisionInfo.front.floor.slantClass == SlantClass::None
         && collisionInfo.front.floor.y < -core::ClimbLimit2ClickMax)
      {
        if(getLara().getSkeleton()->getFrame() < 10_frame)
        {
          setAnimation(AnimationId::WALL_SMASH_LEFT);
          setCurrentAnimState(LaraStateId::Unknown12);
          return;
        }
        if(getLara().getSkeleton()->getFrame() >= 10_frame && getLara().getSkeleton()->getFrame() < 22_frame)
        {
          setAnimation(AnimationId::WALL_SMASH_RIGHT);
          setCurrentAnimState(LaraStateId::Unknown12);
          return;
        }
      }

      setAnimation(AnimationId::STAY_SOLID);
    }

    if(collisionInfo.mid.floor.y > core::ClimbLimit2ClickMin)
    {
      setAnimation(AnimationId::FREE_FALL_FORWARD);
      setGoalAnimState(LaraStateId::JumpForward);
      setCurrentAnimState(LaraStateId::JumpForward);
      getLara().m_state.fallspeed = 0_spd;
      getLara().m_state.falling = true;
      return;
    }

    if(collisionInfo.mid.floor.y >= -core::ClimbLimit2ClickMin && collisionInfo.mid.floor.y < -core::SteppableHeight)
    {
      if(getLara().getSkeleton()->getFrame() >= 3_frame && getLara().getSkeleton()->getFrame() <= 14_frame)
      {
        setAnimation(AnimationId::RUN_UP_STEP_LEFT);
      }
      else
      {
        setAnimation(AnimationId::RUN_UP_STEP_RIGHT);
      }
    }

    if(!tryStartSlide(collisionInfo))
    {
      getLara().m_state.location.position.Y += std::min(collisionInfo.mid.floor.y, 50_len);
    }
  }
};
} // namespace engine::lara
