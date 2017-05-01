#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "engine/inputstate.h"
#include "level/level.h"


namespace engine
{
    namespace lara
    {
        class StateHandler_29 final : public AbstractStateHandler
        {
        public:
            explicit StateHandler_29(LaraNode& lara)
                : AbstractStateHandler(lara, LaraStateId::FallBackward)
            {
            }


            void handleInput(CollisionInfo& /*collisionInfo*/) override
            {
                if( getFallSpeed() > core::FreeFallSpeedThreshold )
                    setTargetState(LaraStateId::FreeFall);

                if( getLevel().m_inputHandler->getInputState().action && getHandStatus() == 0 )
                    setTargetState(LaraStateId::Reach);
            }


            void postprocessFrame(CollisionInfo& collisionInfo) override
            {
                collisionInfo.passableFloorDistanceBottom = loader::HeightLimit;
                collisionInfo.passableFloorDistanceTop = -core::ClimbLimit2ClickMin;
                collisionInfo.neededCeilingDistance = 192;
                collisionInfo.facingAngle = getRotation().Y + 180_deg;
                setMovementAngle(collisionInfo.facingAngle);
                collisionInfo.initHeightInfo(getPosition(), getLevel(), 870); //! @todo MAGICK 870
                checkJumpWallSmash(collisionInfo);
                if( collisionInfo.mid.floor.distance > 0 || getFallSpeed() <= 0 )
                    return;

                if( applyLandingDamage() )
                    setTargetState(LaraStateId::Death);
                else
                    setTargetState(LaraStateId::Stop);

                setFallSpeed(0);
                placeOnFloor(collisionInfo);
                setFalling(false);
            }
        };
    }
}
