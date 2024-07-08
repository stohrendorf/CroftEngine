#pragma once

#include "abstractstatehandler.h"
#include "core/magic.h"
#include "core/units.h"
#include "engine/collisioninfo.h"
#include "qs/quantity.h"

#include <gsl/gsl-lite.hpp>

namespace engine::lara
{
class StateHandler_28 final : public AbstractStateHandler
{
public:
  explicit StateHandler_28(const gsl::not_null<objects::LaraObject*>& lara)
      : AbstractStateHandler{lara, LaraStateId::JumpUp}
  {
  }

  void handleInput(CollisionInfo& /*collisionInfo*/) override
  {
    if(getLara().m_state.fallspeed > core::FreeFallSpeedThreshold)
    {
      setGoalAnimState(LaraStateId::FreeFall);
    }
  }

  void postprocessFrame(CollisionInfo& collisionInfo) override
  {
    collisionInfo.validFloorHeight = {-core::ClimbLimit2ClickMin, core::HeightLimit};
    collisionInfo.validCeilingHeightMin = 192_len;
    collisionInfo.facingAngle = getLara().m_state.rotation.Y;
    collisionInfo.initHeightInfo(getLara().m_state.location.position, getWorld(), core::LaraHangingHeight);

    setMovementAngle(collisionInfo.facingAngle);

    if(tryGrabEdge(collisionInfo))
    {
      return;
    }

    jumpAgainstWall(collisionInfo);
    if(getLara().m_state.fallspeed <= 0_spd || collisionInfo.mid.floor.dy > 0_len)
    {
      return;
    }

    if(applyLandingDamage())
    {
      setGoalAnimState(LaraStateId::Death);
    }
    else
    {
      setGoalAnimState(LaraStateId::Stop);
    }
    getLara().m_state.fallspeed = 0_spd;
    getLara().m_state.falling = false;
    placeOnFloor(collisionInfo);
  }

  [[nodiscard]] bool tryGrabEdge(const CollisionInfo& collisionInfo)
  {
    if(collisionInfo.collisionType != CollisionInfo::AxisColl::Front
       || !getWorld().getPresenter().getInputHandler().hasAction(hid::Action::Action)
       || getHandStatus() != objects::HandStatus::None)
    {
      return false;
    }

    if(abs(collisionInfo.frontLeft.floor.dy - collisionInfo.frontRight.floor.dy) >= core::MaxGrabbableGradient)
    {
      return false;
    }

    if(collisionInfo.front.ceiling.dy > 0_len || collisionInfo.mid.ceiling.dy > -core::ClimbLimit2ClickMin)
    {
      return false;
    }

    const auto spaceToReach = collisionInfo.front.floor.dy - getLara().getBoundingBox().y.min;
    if(spaceToReach < 0_len && spaceToReach + getLara().m_state.fallspeed * 1_frame < 0_len)
    {
      return false;
    }
    if(spaceToReach > 0_len && spaceToReach + getLara().m_state.fallspeed * 1_frame > 0_len)
    {
      return false;
    }

    auto alignedRotation = snapRotation(getLara().m_state.rotation.Y, 35_deg);
    if(!alignedRotation.has_value())
    {
      return false;
    }

    setAnimation(AnimationId::HANG_IDLE,
                 getWorld().getWorldGeometry().getAnimation(AnimationId::HANG_IDLE).firstFrame + 12_frame);
    setGoalAnimState(LaraStateId::Hang);
    setCurrentAnimState(LaraStateId::Hang);

    getLara().m_state.location.position.Y += collisionInfo.front.floor.dy - getLara().getBoundingBox().y.min;
    applyShift(collisionInfo);
    getLara().m_state.speed = 0_spd;
    getLara().m_state.rotation.Y = *alignedRotation;
    getLara().m_state.falling = false;
    getLara().m_state.fallspeed = 0_spd;
    setHandStatus(objects::HandStatus::Grabbing);

    return true;
  }
};
} // namespace engine::lara
