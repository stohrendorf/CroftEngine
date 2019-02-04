#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "engine/inputstate.h"
#include "level/level.h"

namespace engine
{
namespace lara
{
class StateHandler_10 final
        : public AbstractStateHandler
{
public:
    explicit StateHandler_10(LaraNode& lara)
            : AbstractStateHandler{lara, LaraStateId::Hang}
    {
    }

    void handleInput(CollisionInfo& collisionInfo) override
    {
        setCameraRotationAroundCenter( -60_deg, 0_deg );
        collisionInfo.policyFlags &= ~CollisionInfo::SpazPushPolicy;
        if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Left ||
            getLevel().m_inputHandler->getInputState().stepMovement == AxisMovement::Left )
        {
            setGoalAnimState( LaraStateId::ShimmyLeft );
        }
        else if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Right ||
                 getLevel().m_inputHandler->getInputState().stepMovement == AxisMovement::Right )
        {
            setGoalAnimState( LaraStateId::ShimmyRight );
        }
    }

    void postprocessFrame(CollisionInfo& collisionInfo) override
    {
        commonEdgeHangHandling( collisionInfo );

        if( getGoalAnimState() != LaraStateId::Hang )
        {
            return;
        }

        if( getLevel().m_inputHandler->getInputState().zMovement != AxisMovement::Forward )
        {
            return;
        }

        const auto frontHeight = collisionInfo.front.floor.y;
        const auto frontSpace = frontHeight - collisionInfo.front.ceiling.y;
        const auto frontLeftSpace = collisionInfo.frontLeft.floor.y - collisionInfo.frontLeft.ceiling.y;
        const auto frontRightSpace = collisionInfo.frontRight.floor.y - collisionInfo.frontRight.ceiling.y;
        if( frontHeight <= -850_len
            || frontHeight >= -650_len
            || frontSpace < 0_len
            || frontLeftSpace < 0_len
            || frontRightSpace < 0_len
            || collisionInfo.hasStaticMeshCollision )
        {
            return;
        }

        if( getLevel().m_inputHandler->getInputState().moveSlow )
        {
            setGoalAnimState( LaraStateId::Handstand );
        }
        else
        {
            setGoalAnimState( LaraStateId::Climbing );
        }
    }
};
}
}
