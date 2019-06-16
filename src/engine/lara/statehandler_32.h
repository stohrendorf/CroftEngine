#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "hid/inputstate.h"

namespace engine
{
namespace lara
{
class StateHandler_32 final : public AbstractStateHandler
{
public:
    explicit StateHandler_32(LaraNode& lara)
        : AbstractStateHandler{lara, LaraStateId::SlideBackward}
    {
    }

    void handleInput(CollisionInfo& /*collisionInfo*/) override
    {
        if(getEngine().getInputHandler().getInputState().jump)
        {
            setGoalAnimState(LaraStateId::JumpBack);
        }
    }

    void postprocessFrame(CollisionInfo& collisionInfo) override
    {
        setMovementAngle(getLara().m_state.rotation.Y + 180_deg);
        commonSlideHandling(collisionInfo);
    }
};
} // namespace lara
} // namespace engine
