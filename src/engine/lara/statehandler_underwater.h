#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "engine/inputstate.h"
#include "level/level.h"

namespace engine
{
namespace lara
{
class StateHandler_Underwater
        : public AbstractStateHandler
{
public:
    explicit StateHandler_Underwater(LaraNode& lara, const LaraStateId id)
            : AbstractStateHandler{lara, id}
    {
    }

    void postprocessFrame(CollisionInfo& collisionInfo) override
    {
        collisionInfo.facingAngle = getLara().m_state.rotation.Y;
        if( abs( getLara().m_state.rotation.X ) > 90_deg )
        {
            collisionInfo.facingAngle += 180_deg;
        }
        setMovementAngle( collisionInfo.facingAngle );
        collisionInfo.initHeightInfo( getLara().m_state.position.position + core::TRVec{0_len, core::LaraDiveGroundElevation, 0_len},
                                      getLevel(),
                                      core::LaraDiveHeight );

        applyShift( collisionInfo );

        switch( collisionInfo.collisionType )
        {
            case CollisionInfo::AxisColl::Left:
                getLara().m_state.rotation.Y += 5_deg;
                break;
            case CollisionInfo::AxisColl::Right:
                getLara().m_state.rotation.Y -= 5_deg;
                break;
            case CollisionInfo::AxisColl::TopFront:
            {
                getLara().m_state.fallspeed = 0_len;
            }
                return;
            case CollisionInfo::AxisColl::TopBottom:
            {
                getLara().m_state.fallspeed = 0_len;
            }
                break;
            case CollisionInfo::AxisColl::Top:
            {
                if( getLara().m_state.rotation.X > -45_deg )
                {
                    getLara().m_state.rotation.X -= 2_deg;
                }
            }
                break;
            case CollisionInfo::AxisColl::Front:
            {
                if( getLara().m_state.rotation.X > 35_deg )
                {
                    getLara().m_state.rotation.X += 2_deg;
                }
                else if( getLara().m_state.rotation.X < -35_deg )
                {
                    getLara().m_state.rotation.X -= 2_deg;
                }
                else
                {
                    getLara().m_state.fallspeed = 0_len;
                }
            }
                break;
            default:
                break;
        }

        if( collisionInfo.mid.floorSpace.y >= 0_len )
        {
            return;
        }

        placeOnFloor( collisionInfo );
        getLara().m_state.rotation.X += 2_deg;
    }

protected:
    void handleDiveRotationInput()
    {
        if( getLevel().m_inputHandler->getInputState().zMovement == AxisMovement::Forward )
        {
            getLara().m_state.rotation.X -= 2_deg;
        }
        else if( getLevel().m_inputHandler->getInputState().zMovement == AxisMovement::Backward )
        {
            getLara().m_state.rotation.X += 2_deg;
        }

        if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Left )
        {
            getLara().m_state.rotation.Y -= 6_deg;
            getLara().m_state.rotation.Z -= 3_deg;
        }
        else if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Right )
        {
            getLara().m_state.rotation.Y += 6_deg;
            getLara().m_state.rotation.Z += 3_deg;
        }
    }
};
}
}
