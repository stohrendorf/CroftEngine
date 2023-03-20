#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "engine/objects/laraobject.h"
#include "hid/inputstate.h"

namespace engine::lara
{
class StateHandler_OnWater : public AbstractStateHandler
{
public:
  explicit StateHandler_OnWater(const gsl::not_null<objects::LaraObject*>& lara, const LaraStateId id)
      : AbstractStateHandler{lara, id}
  {
  }

protected:
  void commonOnWaterHandling(CollisionInfo& collisionInfo)
  {
    collisionInfo.facingAngle = getMovementAngle();
    collisionInfo.initHeightInfo(getLara().m_state.location.position + core::TRVec(0_len, core::LaraSwimHeight, 0_len),
                                 getWorld(),
                                 core::LaraSwimHeight);
    applyShift(collisionInfo);
    if(collisionInfo.mid.floor.y < 0_len)
    {
      getLara().m_state.fallspeed = 0_spd;
      getLara().m_state.location.position = collisionInfo.initialPosition;
    }
    else
    {
      switch(collisionInfo.collisionType)
      {
      case CollisionInfo::AxisColl::Front:
      case CollisionInfo::AxisColl::Top:
      case CollisionInfo::AxisColl::FrontTop:
      case CollisionInfo::AxisColl::Jammed:
        getLara().m_state.fallspeed = 0_spd;
        getLara().m_state.location.position = collisionInfo.initialPosition;
        break;
      case CollisionInfo::AxisColl::FrontLeft:
        getLara().m_state.rotation.Y += core::WaterCollisionRotationSpeedY * 1_frame;
        break;
      case CollisionInfo::AxisColl::FrontRight:
        getLara().m_state.rotation.Y -= core::WaterCollisionRotationSpeedY * 1_frame;
        break;
      default:
        break;
      }
    }

    auto wsh = getLara().getWaterSurfaceHeight();
    if(wsh.has_value() && *wsh > getLara().m_state.location.position.Y - core::DefaultCollisionRadius)
    {
      tryClimbOutOfWater(collisionInfo);
      return;
    }

    setAnimation(AnimationId::FREE_FALL_TO_UNDERWATER_ALTERNATE);
    setGoalAnimState(LaraStateId::UnderwaterForward);
    setCurrentAnimState(LaraStateId::UnderwaterDiving);
    getLara().m_state.rotation.X = -45_deg;
    getLara().m_state.fallspeed = 80_spd;
    setUnderwaterState(objects::UnderwaterState::Diving);
  }

private:
  void tryClimbOutOfWater(const CollisionInfo& collisionInfo)
  {
    if(getMovementAngle() != getLara().m_state.rotation.Y)
    {
      return;
    }

    if(collisionInfo.collisionType != CollisionInfo::AxisColl::Front)
    {
      return;
    }

    if(!getWorld().getPresenter().getInputHandler().hasAction(hid::Action::Action))
    {
      return;
    }

    const auto gradient = abs(collisionInfo.frontLeft.floor.y - collisionInfo.frontRight.floor.y);
    if(gradient >= core::MaxGrabbableGradient)
    {
      return;
    }

    if(collisionInfo.front.ceiling.y > 0_len)
    {
      return;
    }

    if(collisionInfo.mid.ceiling.y > -core::ClimbLimit2ClickMin)
    {
      return;
    }

    if(collisionInfo.front.floor.y + core::LaraSwimHeight <= 2 * -core::QuarterSectorSize)
    {
      return;
    }

    if(collisionInfo.front.floor.y + core::LaraSwimHeight > core::DefaultCollisionRadius)
    {
      return;
    }

    const auto axis = axisFromAngle(getLara().m_state.rotation.Y, 35_deg);
    if(!axis.has_value())
    {
      return;
    }

    getLara().m_state.location.move(core::TRVec(0_len, 695_len + collisionInfo.front.floor.y, 0_len));
    getLara().updateFloorHeight(-381_len);
    core::TRVec d = getLara().m_state.location.position;
    switch(*axis)
    {
    case core::Axis::Deg0:
      d.Z = snappedSector(getLara().m_state.location.position.Z) + 1_sectors + core::DefaultCollisionRadius;
      break;
    case core::Axis::Deg180:
      d.Z = snappedSector(getLara().m_state.location.position.Z) - core::DefaultCollisionRadius;
      break;
    case core::Axis::Left90:
      d.X = snappedSector(getLara().m_state.location.position.X) - core::DefaultCollisionRadius;
      break;
    case core::Axis::Right90:
      d.X = snappedSector(getLara().m_state.location.position.X) + 1_sectors + core::DefaultCollisionRadius;
      break;
    default:
      BOOST_THROW_EXCEPTION(std::runtime_error("Unexpected angle value"));
    }

    getLara().m_state.location.position = d;

    setAnimation(AnimationId::CLIMB_OUT_OF_WATER);
    setGoalAnimState(LaraStateId::Stop);
    setCurrentAnimState(LaraStateId::OnWaterExit);
    getLara().m_state.speed = 0_spd;
    getLara().m_state.fallspeed = 0_spd;
    getLara().m_state.falling = false;
    getLara().m_state.rotation.X = 0_deg;
    getLara().m_state.rotation.Y = snapRotation(*axis);
    getLara().m_state.rotation.Z = 0_deg;
    setHandStatus(objects::HandStatus::Grabbing);
    setUnderwaterState(objects::UnderwaterState::OnLand);
  }
};
} // namespace engine::lara
