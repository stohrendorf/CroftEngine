#pragma once

#include "abstractstatehandler.h"
#include "core/magic.h"
#include "core/units.h"
#include "engine/collisioninfo.h"
#include "engine/heightinfo.h"
#include "hid/inputstate.h"
#include "util/helpers.h"

#include <gsl/gsl-lite.hpp>

namespace engine::lara
{
class StateHandler_15 final : public AbstractStateHandler
{
public:
  explicit StateHandler_15(const gsl::not_null<objects::LaraObject*>& lara)
      : AbstractStateHandler{lara, LaraStateId::JumpPrepare}
  {
  }

  [[nodiscard]] core::Length getRelativeHeightAtDirection(const core::Angle& angle, const core::Length& dist) const
  {
    auto location = getLara().m_state.location.moved(util::pitch(dist, angle));
    location.position.Y -= core::LaraWalkHeight;
    const auto sector = location.updateRoom();

    HeightInfo h = HeightInfo::fromFloor(sector, location.position, getWorld().getObjectManager().getObjects());
    if(h.y != core::InvalidHeight)
    {
      h.y -= getLara().m_state.location.position.Y;
    }

    return h.y;
  }

  void handleInput(CollisionInfo& /*collisionInfo*/) override
  {
    if(getWorld().getPresenter().getInputHandler().getInputState().zMovement == hid::AxisMovement::Forward
       && getRelativeHeightAtDirection(getLara().m_state.rotation.Y, 256_len) >= -core::ClimbLimit2ClickMin)
    {
      setMovementAngle(getLara().m_state.rotation.Y);
      setGoalAnimState(LaraStateId::JumpForward);
    }
    else if(getWorld().getPresenter().getInputHandler().getInputState().xMovement == hid::AxisMovement::Left
            && getRelativeHeightAtDirection(getLara().m_state.rotation.Y - 90_deg, 256_len)
                 >= -core::ClimbLimit2ClickMin)
    {
      setMovementAngle(getLara().m_state.rotation.Y - 90_deg);
      setGoalAnimState(LaraStateId::JumpRight);
    }
    else if(getWorld().getPresenter().getInputHandler().getInputState().xMovement == hid::AxisMovement::Right
            && getRelativeHeightAtDirection(getLara().m_state.rotation.Y + 90_deg, 256_len)
                 >= -core::ClimbLimit2ClickMin)
    {
      setMovementAngle(getLara().m_state.rotation.Y + 90_deg);
      setGoalAnimState(LaraStateId::JumpLeft);
    }
    else if(getWorld().getPresenter().getInputHandler().getInputState().zMovement == hid::AxisMovement::Backward
            && getRelativeHeightAtDirection(getLara().m_state.rotation.Y + 180_deg, 256_len)
                 >= -core::ClimbLimit2ClickMin)
    {
      setMovementAngle(getLara().m_state.rotation.Y + 180_deg);
      setGoalAnimState(LaraStateId::JumpBack);
    }

    if(getLara().m_state.fallspeed > core::FreeFallSpeedThreshold)
    {
      setGoalAnimState(LaraStateId::FreeFall);
    }
  }

  void postprocessFrame(CollisionInfo& collisionInfo) override
  {
    collisionInfo.validFloorHeight = {-core::HeightLimit, core::HeightLimit};
    collisionInfo.validCeilingHeightMin = 0_len;
    collisionInfo.facingAngle = getMovementAngle();
    collisionInfo.initHeightInfo(getLara().m_state.location.position, getWorld(), core::LaraWalkHeight);

    getLara().m_state.fallspeed = 0_spd;
    getLara().m_state.falling = false;

    if(collisionInfo.mid.ceiling.dy <= -core::DefaultCollisionRadius)
    {
      return;
    }

    setAnimation(AnimationId::STAY_SOLID);
    setGoalAnimState(LaraStateId::Stop);
    setCurrentAnimState(LaraStateId::Stop);
    getLara().m_state.speed = 0_spd;
    getLara().m_state.location.position = collisionInfo.initialPosition;
  }
};
} // namespace engine::lara
