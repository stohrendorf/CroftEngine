#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "hid/inputstate.h"

namespace engine
{
namespace lara
{
class StateHandler_10 final : public AbstractStateHandler
{
public:
    explicit StateHandler_10(LaraNode& lara)
        : AbstractStateHandler{lara, LaraStateId::Hang}
    {
    }

    void handleInput(CollisionInfo& collisionInfo) override
    {
        setCameraRotationAroundCenter(-60_deg, 0_deg);
        collisionInfo.policyFlags &= ~CollisionInfo::SpazPushPolicy;
        if(getEngine().getInputHandler().getInputState().xMovement == hid::AxisMovement::Left
           || getEngine().getInputHandler().getInputState().stepMovement == hid::AxisMovement::Left)
        {
            setGoalAnimState(LaraStateId::ShimmyLeft);
        }
        else if(getEngine().getInputHandler().getInputState().xMovement == hid::AxisMovement::Right
                || getEngine().getInputHandler().getInputState().stepMovement == hid::AxisMovement::Right)
        {
            setGoalAnimState(LaraStateId::ShimmyRight);
        }
    }

    void postprocessFrame(CollisionInfo& collisionInfo) override
    {
        commonEdgeHangHandling(collisionInfo);

        if(getGoalAnimState() != LaraStateId::Hang)
        {
            return;
        }

        if(getEngine().getInputHandler().getInputState().zMovement != hid::AxisMovement::Forward)
        {
            return;
        }

        const auto frontHeight = collisionInfo.front.floorSpace.y;
        const auto frontSpace = frontHeight - collisionInfo.front.ceilingSpace.y;
        const auto frontLeftSpace = collisionInfo.frontLeft.floorSpace.y - collisionInfo.frontLeft.ceilingSpace.y;
        const auto frontRightSpace = collisionInfo.frontRight.floorSpace.y - collisionInfo.frontRight.ceilingSpace.y;
        if(frontHeight <= -850_len || frontHeight >= -650_len || frontSpace < 0_len || frontLeftSpace < 0_len
           || frontRightSpace < 0_len || collisionInfo.hasStaticMeshCollision)
        {
            return;
        }

        if(getEngine().getInputHandler().getInputState().moveSlow)
        {
            setGoalAnimState(LaraStateId::Handstand);
        }
        else
        {
            setGoalAnimState(LaraStateId::Climbing);
        }
    }
};
} // namespace lara
} // namespace engine
