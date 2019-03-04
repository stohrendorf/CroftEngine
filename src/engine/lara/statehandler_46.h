#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"

namespace engine
{
namespace lara
{
class StateHandler_46 final
        : public AbstractStateHandler
{
public:
    explicit StateHandler_46(LaraNode& lara)
            : AbstractStateHandler{lara, LaraStateId::BoulderDeath}
    {
    }

    void handleInput(CollisionInfo& /*collisionInfo*/) override
    {
        const auto& camera = getEngine().m_cameraController;
        camera->setModifier( CameraModifier::FollowCenter );
        camera->setEyeRotation( -25_deg, 170_deg );
    }

    void postprocessFrame(CollisionInfo& /*collisionInfo*/) override
    {
    }
};
}
}
