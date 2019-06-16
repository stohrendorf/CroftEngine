#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"

namespace engine
{
namespace lara
{
class StateHandler_46 final : public AbstractStateHandler
{
public:
    explicit StateHandler_46(LaraNode& lara)
        : AbstractStateHandler{lara, LaraStateId::BoulderDeath}
    {
    }

    void handleInput(CollisionInfo& /*collisionInfo*/) override
    {
        getEngine().getCameraController().setModifier(CameraModifier::FollowCenter);
        getEngine().getCameraController().setEyeRotation(-25_deg, 170_deg);
    }

    void postprocessFrame(CollisionInfo& /*collisionInfo*/) override
    {
    }
};
} // namespace lara
} // namespace engine
