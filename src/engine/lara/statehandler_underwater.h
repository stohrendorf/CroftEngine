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
    explicit StateHandler_Underwater(LaraNode& lara, LaraStateId id)
        : AbstractStateHandler(lara, id)
    {
    }

    void postprocessFrame(CollisionInfo& collisionInfo) override
    {
        collisionInfo.facingAngle = getLara().m_state.rotation.Y;
        if( abs(getLara().m_state.rotation.X) > 90_deg )
        {
            collisionInfo.facingAngle += 180_deg;
        }
        setMovementAngle(collisionInfo.facingAngle);
        collisionInfo.initHeightInfo(getLara().m_state.position.position + core::TRVec{0, 200, 0}, getLevel(), core::LaraHeightUnderwater);

        applyShift(collisionInfo);

        switch( collisionInfo.collisionType )
        {
            case CollisionInfo::AxisColl_Left:
                getLara().m_state.rotation.Y += 5_deg;
                break;
            case CollisionInfo::AxisColl_Right:
                getLara().m_state.rotation.Y -= 5_deg;
                break;
            case CollisionInfo::AxisColl_TopFront:
            {
                getLara().m_state.fallspeed = 0;
            }
                return;
            case CollisionInfo::AxisColl_TopBottom:
            {
                getLara().m_state.fallspeed = 0;
            }
                break;
            case CollisionInfo::AxisColl_Top:
            {
                if( getLara().m_state.rotation.X > -45_deg )
                {
                    getLara().m_state.rotation.X -= 2_deg;
                }
            }
                break;
            case CollisionInfo::AxisColl_Front:
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
                    getLara().m_state.fallspeed = 0;
                }
            }
                break;
            default:
                break;
        }

        if( collisionInfo.mid.floor.y >= 0 )
        {
            return;
        }

        placeOnFloor(collisionInfo);
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
