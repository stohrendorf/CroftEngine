#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "engine/inputstate.h"
#include "level/level.h"

namespace engine
{
namespace lara
{
class StateHandler_46 final
    : public AbstractStateHandler
{
public:
    explicit StateHandler_46(LaraNode& lara)
        : AbstractStateHandler(lara, LaraStateId::BoulderDeath)
    {
    }

    void handleInput(CollisionInfo& /*collisionInfo*/) override
    {
    }

    void postprocessFrame(CollisionInfo& /*collisionInfo*/) override
    {
    }
};
}
}
