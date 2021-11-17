#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "hid/inputstate.h"

namespace engine::lara
{
class StateHandler_Underwater : public AbstractStateHandler
{
public:
  explicit StateHandler_Underwater(objects::LaraObject& lara, const LaraStateId id)
      : AbstractStateHandler{lara, id}
  {
  }

  void postprocessFrame(CollisionInfo& collisionInfo) override
  {
    collisionInfo.facingAngle = getLara().m_state.rotation.Y;
    if(abs(getLara().m_state.rotation.X) > 90_deg)
    {
      collisionInfo.facingAngle += 180_deg;
    }
    setMovementAngle(collisionInfo.facingAngle);
    collisionInfo.initHeightInfo(getLara().m_state.location.position
                                   + core::TRVec{0_len, core::LaraDiveGroundElevation, 0_len},
                                 getWorld(),
                                 core::LaraDiveHeight);

    applyShift(collisionInfo);

    switch(collisionInfo.collisionType)
    {
    case CollisionInfo::AxisColl::FrontLeft:
      getLara().m_state.rotation.Y += core::WaterCollisionRotationSpeedY * 1_frame;
      break;
    case CollisionInfo::AxisColl::FrontRight:
      getLara().m_state.rotation.Y -= core::WaterCollisionRotationSpeedY * 1_frame;
      break;
    case CollisionInfo::AxisColl::Jammed:
      getLara().m_state.fallspeed = 0_spd;
      return;
    case CollisionInfo::AxisColl::FrontTop:
      getLara().m_state.fallspeed = 0_spd;
      break;
    case CollisionInfo::AxisColl::Top:
      if(getLara().m_state.rotation.X > -45_deg)
      {
        getLara().m_state.rotation.X -= core::WaterCollisionRotationSpeedX * 1_frame;
      }
      break;
    case CollisionInfo::AxisColl::Front:
      if(getLara().m_state.rotation.X > 35_deg)
      {
        getLara().m_state.rotation.X += core::WaterCollisionRotationSpeedX * 1_frame;
      }
      else if(getLara().m_state.rotation.X < -35_deg)
      {
        getLara().m_state.rotation.X -= core::WaterCollisionRotationSpeedX * 1_frame;
      }
      else
      {
        getLara().m_state.fallspeed = 0_spd;
      }
      break;
    default:
      break;
    }

    if(collisionInfo.mid.floor.y >= 0_len)
    {
      return;
    }

    placeOnFloor(collisionInfo);
    getLara().m_state.rotation.X += core::WaterCollisionRotationSpeedX * 1_frame;
  }

protected:
  void handleDiveRotationInput()
  {
    if(getWorld().getPresenter().getInputHandler().getInputState().zMovement == hid::AxisMovement::Forward)
    {
      getLara().m_state.rotation.X -= core::DiveRotationSpeedX * 1_frame;
    }
    else if(getWorld().getPresenter().getInputHandler().getInputState().zMovement == hid::AxisMovement::Backward)
    {
      getLara().m_state.rotation.X += core::DiveRotationSpeedX * 1_frame;
    }

    if(getWorld().getPresenter().getInputHandler().getInputState().xMovement == hid::AxisMovement::Left)
    {
      getLara().m_state.rotation.Y -= core::DiveRotationSpeedY * 1_frame;
      getLara().m_state.rotation.Z -= core::DiveRotationSpeedZ * 1_frame;
    }
    else if(getWorld().getPresenter().getInputHandler().getInputState().xMovement == hid::AxisMovement::Right)
    {
      getLara().m_state.rotation.Y += core::DiveRotationSpeedY * 1_frame;
      getLara().m_state.rotation.Z += core::DiveRotationSpeedZ * 1_frame;
    }
  }
};
} // namespace engine::lara
