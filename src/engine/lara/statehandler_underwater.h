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

  void postprocessFrame(CollisionInfo& collisionInfo, bool doPhysics) override
  {
    collisionInfo.facingAngle = getLara().m_state.rotation.Y;
    if(normalizeAngle(abs(getLara().m_state.rotation.X)) > 90_deg)
    {
      collisionInfo.facingAngle += 180_deg;
    }
    collisionInfo.initHeightInfo(getLara().m_state.location.position
                                   + core::TRVec{0_len, core::LaraDiveGroundElevation, 0_len},
                                 getWorld(),
                                 core::LaraDiveHeight);

    if(!doPhysics)
      return;

    setMovementAngle(collisionInfo.facingAngle);

    applyShift(collisionInfo);

    switch(collisionInfo.collisionType)
    {
    case CollisionInfo::AxisColl::FrontLeft:
      getLara().m_state.rotation.Y += core::WaterCollisionRotationSpeedY * 1_rframe;
      break;
    case CollisionInfo::AxisColl::FrontRight:
      getLara().m_state.rotation.Y -= core::WaterCollisionRotationSpeedY * 1_rframe;
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
        getLara().m_state.rotation.X -= core::WaterCollisionRotationSpeedX * 1_rframe;
      }
      break;
    case CollisionInfo::AxisColl::Front:
      if(getLara().m_state.rotation.X > 35_deg)
      {
        getLara().m_state.rotation.X += core::WaterCollisionRotationSpeedX * 1_rframe;
      }
      else if(getLara().m_state.rotation.X < -35_deg)
      {
        getLara().m_state.rotation.X -= core::WaterCollisionRotationSpeedX * 1_rframe;
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
    getLara().m_state.rotation.X += core::WaterCollisionRotationSpeedX * 1_rframe;
  }

protected:
  void handleDiveRotationInput()
  {
    const auto& inputHandler = getWorld().getPresenter().getInputHandler();
    if(inputHandler.getInputState().zMovement == hid::AxisMovement::Forward)
    {
      getLara().m_state.rotation.X -= core::DiveRotationSpeedX * 1_rframe;
    }
    else if(inputHandler.getInputState().zMovement == hid::AxisMovement::Backward)
    {
      getLara().m_state.rotation.X += core::DiveRotationSpeedX * 1_rframe;
    }

    if(inputHandler.getInputState().xMovement == hid::AxisMovement::Left)
    {
      getLara().m_state.rotation.Y -= core::DiveRotationSpeedY * 1_rframe;
      getLara().m_state.rotation.Z -= core::DiveRotationSpeedZ * 1_rframe;
    }
    else if(inputHandler.getInputState().xMovement == hid::AxisMovement::Right)
    {
      getLara().m_state.rotation.Y += core::DiveRotationSpeedY * 1_rframe;
      getLara().m_state.rotation.Z += core::DiveRotationSpeedZ * 1_rframe;
    }
  }
};
} // namespace engine::lara
