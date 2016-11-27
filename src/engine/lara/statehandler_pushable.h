#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "engine/inputstate.h"
#include "level/level.h"

namespace engine
{
    namespace lara
    {
        class StateHandler_Pushable : public AbstractStateHandler
        {
        public:
            explicit StateHandler_Pushable(LaraNode& lara)
                    : AbstractStateHandler(lara)
            {
            }

            std::unique_ptr<AbstractStateHandler> handleInputImpl(CollisionInfo& collisionInfo) override final
            {
                collisionInfo.frobbelFlags &= ~(CollisionInfo::FrobbelFlag08 | CollisionInfo::FrobbelFlag10);
                setCameraUnknown1(1);
                setCameraRotation(-25_deg, 35_deg);
                return nullptr;
            }

            void animateImpl(CollisionInfo& /*collisionInfo*/, const std::chrono::microseconds& /*deltaTimeMs*/) override final
            {
            }

            std::unique_ptr<AbstractStateHandler> postprocessFrame(CollisionInfo& collisionInfo) override final
            {
                setMovementAngle(getRotation().Y);
                collisionInfo.neededFloorDistanceBottom = core::ClimbLimit2ClickMin;
                collisionInfo.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
                collisionInfo.neededCeilingDistance = 0;
                collisionInfo.frobbelFlags |= CollisionInfo::FrobbelFlag_UnwalkableSteepFloor | CollisionInfo::FrobbelFlag_UnpassableSteepUpslant;

                collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);

                return nullptr;
            }
        };
    }
}
