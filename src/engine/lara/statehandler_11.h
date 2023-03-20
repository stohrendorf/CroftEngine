#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"

namespace engine::lara
{
class StateHandler_11 final : public AbstractStateHandler
{
public:
  explicit StateHandler_11(const gsl::not_null<objects::LaraObject*>& lara)
      : AbstractStateHandler{lara, LaraStateId::Reach}
  {
  }

  void handleInput(CollisionInfo& /*collisionInfo*/) override
  {
    getWorld().getCameraController().setRotationAroundLaraY(85_deg);
    if(getLara().m_state.fallspeed > core::FreeFallSpeedThreshold)
    {
      setGoalAnimState(LaraStateId::FreeFall);
    }
  }

  [[nodiscard]] bool canSwing(const core::Axis axis) const
  {
    auto location = getLara().m_state.location;
    switch(axis)
    {
    case core::Axis::PosZ:
      location.position.Z += core::QuarterSectorSize;
      break;
    case core::Axis::PosX:
      location.position.X += core::QuarterSectorSize;
      break;
    case core::Axis::NegZ:
      location.position.Z -= core::QuarterSectorSize;
      break;
    case core::Axis::NegX:
      location.position.X -= core::QuarterSectorSize;
      break;
    }

    const auto sector = location.updateRoom();
    VerticalDistances distances;
    distances.init(sector, location.position, getWorld().getObjectManager().getObjects(), location.position.Y, 400_len);
    return distances.floor.y != core::InvalidHeight && distances.floor.y > 0_len && distances.ceiling.y < 0_len;
  }

  [[nodiscard]] bool tryHang(CollisionInfo& collisionInfo)
  {
    if(collisionInfo.collisionType != CollisionInfo::AxisColl::Front
       || !getWorld().getPresenter().getInputHandler().hasAction(hid::Action::Action)
       || getHandStatus() != objects::HandStatus::None)
    {
      return false;
    }

    if(abs(collisionInfo.frontLeft.floor.y - collisionInfo.frontRight.floor.y) >= core::MaxGrabbableGradient)
    {
      return false;
    }

    if(collisionInfo.front.ceiling.y > 0_len || collisionInfo.mid.ceiling.y > -core::ClimbLimit2ClickMin
       || collisionInfo.mid.floor.y < 200_len)
    {
      return false;
    }

    const auto spaceToReach = collisionInfo.front.floor.y - getLara().getBoundingBox().y.min;
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

    if(canSwing(*axisFromAngle(getLara().m_state.rotation.Y, 35_deg)))
    {
      setAnimation(AnimationId::OSCILLATE_HANG_ON);
    }
    else
    {
      setAnimation(AnimationId::HANG_IDLE);
    }

    setGoalAnimState(LaraStateId::Hang);
    setCurrentAnimState(LaraStateId::Hang);
    getLara().m_state.location.position += core::TRVec(collisionInfo.shift.X, spaceToReach, collisionInfo.shift.Z);
    getLara().m_state.speed = 0_spd;
    getLara().m_state.rotation.Y = *alignedRotation;
    getLara().m_state.falling = false;
    getLara().m_state.fallspeed = 0_spd;
    setHandStatus(objects::HandStatus::Grabbing);
    return true;
  }

  void postprocessFrame(CollisionInfo& collisionInfo) override
  {
    collisionInfo.facingAngle = getLara().m_state.rotation.Y;
    collisionInfo.validFloorHeight = {0_len, core::HeightLimit};
    collisionInfo.validCeilingHeightMin = 192_len;
    collisionInfo.initHeightInfo(getLara().m_state.location.position, getWorld(), core::LaraWalkHeight);

    setMovementAngle(collisionInfo.facingAngle);
    getLara().m_state.falling = true;

    if(tryHang(collisionInfo))
    {
      return;
    }

    jumpAgainstWall(collisionInfo);
    if(getLara().m_state.fallspeed <= 0_spd || collisionInfo.mid.floor.y > 0_len)
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
};
} // namespace engine::lara
