#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "hid/inputstate.h"

namespace engine::lara
{
class StateHandler_SideStep : public AbstractStateHandler
{
public:
  explicit StateHandler_SideStep(objects::LaraObject& lara,
                                 const LaraStateId& stateId,
                                 const hid::AxisMovement& inputAxis,
                                 const core::Angle& rotation)
      : AbstractStateHandler{lara, stateId}
      , m_inputAxis{inputAxis}
      , m_rotation{rotation}
  {
  }

  void handleInput(CollisionInfo& /*collisionInfo*/, bool /*doPhysics*/) final
  {
    if(getLara().isDead())
    {
      setGoalAnimState(LaraStateId::Stop);
      return;
    }

    const auto& inputHandler = getWorld().getPresenter().getInputHandler();
    if(inputHandler.getInputState().stepMovement != m_inputAxis)
    {
      setGoalAnimState(LaraStateId::Stop);
    }

    if(inputHandler.getInputState().xMovement == hid::AxisMovement::Left)
    {
      subYRotationSpeed(core::SlowTurnSpeedAcceleration, -core::SlowTurnSpeed);
    }
    else if(inputHandler.getInputState().xMovement == hid::AxisMovement::Right)
    {
      addYRotationSpeed(core::SlowTurnSpeedAcceleration, core::SlowTurnSpeed);
    }
  }

  void postprocessFrame(CollisionInfo& collisionInfo, bool /*doPhysics*/) final
  {
    auto& laraState = getLara().m_state;
    collisionInfo.validFloorHeight = {-core::SteppableHeight, core::SteppableHeight};
    collisionInfo.validCeilingHeightMin = 0_len;
    collisionInfo.facingAngle = laraState.rotation.Y + m_rotation;
    collisionInfo.policies |= CollisionInfo::SlopeBlockingPolicy;
    collisionInfo.initHeightInfo(laraState.location.position, getWorld(), core::LaraWalkHeight);

    setMovementAngle(collisionInfo.facingAngle);
    laraState.fallspeed = 0_spd;
    laraState.falling = false;

    if(stopIfCeilingBlocked(collisionInfo))
    {
      return;
    }

    if(checkWallCollision(collisionInfo))
    {
      setAnimation(AnimationId::STAY_SOLID);
      setGoalAnimState(LaraStateId::Stop);
      setCurrentAnimState(LaraStateId::Stop);
    }

    if(!tryStartSlide(collisionInfo))
    {
      placeOnFloor(collisionInfo);
    }
  }

private:
  const hid::AxisMovement m_inputAxis;
  const core::Angle m_rotation;
};
} // namespace engine::lara
