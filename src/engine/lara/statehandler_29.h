#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "engine/inputstate.h"
#include "level/level.h"

namespace engine
{
namespace lara
{
class StateHandler_29 final
    : public AbstractStateHandler
{
public:
    explicit StateHandler_29(LaraNode& lara)
        : AbstractStateHandler(lara, LaraStateId::FallBackward)
    {
    }

    void handleInput(CollisionInfo& /*collisionInfo*/) override
    {
        if( getLara().m_state.fallspeed > core::FreeFallSpeedThreshold )
        {
            setGoalAnimState( LaraStateId::FreeFall );
        }

        if( getLevel().m_inputHandler->getInputState().action && getHandStatus() == HandStatus::None )
        {
            setGoalAnimState( LaraStateId::Reach );
        }
    }

    void postprocessFrame(CollisionInfo& collisionInfo) override
    {
        collisionInfo.badPositiveDistance = loader::HeightLimit;
        collisionInfo.badNegativeDistance = -core::ClimbLimit2ClickMin;
        collisionInfo.badCeilingDistance = 192;
        collisionInfo.facingAngle = getLara().m_state.rotation.Y + 180_deg;
        setMovementAngle(collisionInfo.facingAngle);
        collisionInfo.initHeightInfo(getLara().m_state.position.position, getLevel(), 870); //! @todo MAGICK 870
        checkJumpWallSmash(collisionInfo);
        if( collisionInfo.mid.floor.y > 0 || getLara().m_state.fallspeed <= 0 )
        {
            return;
        }

        if( applyLandingDamage() )
        {
            setGoalAnimState( LaraStateId::Death );
        }
        else
        {
            setGoalAnimState( LaraStateId::Stop );
        }

        getLara().m_state.fallspeed = 0;
        placeOnFloor(collisionInfo);
        getLara().m_state.falling = false;
    }
};
}
}
