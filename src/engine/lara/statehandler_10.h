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
        : AbstractStateHandler(lara, LaraStateId::Hang)
    {
    }

    void handleInput(CollisionInfo& collisionInfo) override
    {
        setCameraCurrentRotation(-60_deg, 0_deg);
        collisionInfo.policyFlags &= ~(CollisionInfo::EnableBaddiePush | CollisionInfo::EnableSpaz);
        if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Left ||
            getLevel().m_inputHandler->getInputState().stepMovement == AxisMovement::Left )
        {
            setTargetState(LaraStateId::ShimmyLeft);
        }
        else if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Right ||
                 getLevel().m_inputHandler->getInputState().stepMovement == AxisMovement::Right )
        {
            setTargetState(LaraStateId::ShimmyRight);
        }
    }

    void postprocessFrame(CollisionInfo& collisionInfo) override
    {
        commonEdgeHangHandling(collisionInfo);

        if( getTargetState() != LaraStateId::Hang )
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
        if( frontHeight <= -850 || frontHeight >= -650 || frontSpace < 0 || frontLeftSpace < 0 || frontRightSpace < 0 || collisionInfo.hasStaticMeshCollision )
        {
            return;
        }

        if( getLevel().m_inputHandler->getInputState().moveSlow )
        {
            setTargetState(LaraStateId::Handstand);
        }
        else
        {
            setTargetState(LaraStateId::Climbing);
        }
    }
};
}
}
