#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "engine/inputstate.h"
#include "level/level.h"


namespace engine
{
    namespace lara
    {
        class StateHandler_38 final : public AbstractStateHandler
        {
        public:
            explicit StateHandler_38(LaraNode& lara)
                : AbstractStateHandler(lara, LaraStateId::PushableGrab)
            {
            }


            void handleInputImpl(CollisionInfo& collisionInfo, const std::chrono::microseconds& deltaTime) override
            {
                collisionInfo.policyFlags &= ~(CollisionInfo::EnableBaddiePush | CollisionInfo::EnableSpaz);
                setCameraRotationY(75_deg);
                if( !getLevel().m_inputHandler->getInputState().action )
                    setTargetState(LaraStateId::Stop);
                else
                    setTargetState(LaraStateId::PushableGrab);
            }


            void animateImpl(CollisionInfo& /*collisionInfo*/, const std::chrono::microseconds& /*deltaTimeMs*/) override
            {
            }


            void postprocessFrame(CollisionInfo& collisionInfo, const std::chrono::microseconds& deltaTime) override
            {
                setMovementAngle(getRotation().Y);
                collisionInfo.passableFloorDistanceBottom = core::ClimbLimit2ClickMin;
                collisionInfo.passableFloorDistanceTop = -core::ClimbLimit2ClickMin;
                collisionInfo.neededCeilingDistance = 0;
                collisionInfo.policyFlags |= CollisionInfo::SlopesArePits | CollisionInfo::SlopesAreWalls;

                collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
            }
        };
    }
}
