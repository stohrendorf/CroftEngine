#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"

namespace engine
{
    namespace lara
    {
        class StateHandler_Pushable : public AbstractStateHandler
        {
        public:
            explicit StateHandler_Pushable(LaraNode& lara, LaraStateId id)
                    : AbstractStateHandler(lara, id)
            {
            }

            boost::optional<LaraStateId> handleInputImpl(CollisionInfo& collisionInfo) override final
            {
                collisionInfo.frobbelFlags &= ~(CollisionInfo::FrobbelFlag08 | CollisionInfo::FrobbelFlag10);
                setCameraUnknown1(1);
                setCameraRotation(-25_deg, 35_deg);
                return {};
            }

            void animateImpl(CollisionInfo& /*collisionInfo*/, const std::chrono::microseconds& /*deltaTimeMs*/) override final
            {
            }

            boost::optional<LaraStateId> postprocessFrame(CollisionInfo& collisionInfo) override final
            {
                setMovementAngle(getRotation().Y);
                collisionInfo.passableFloorDistanceBottom = core::ClimbLimit2ClickMin;
                collisionInfo.passableFloorDistanceTop = -core::ClimbLimit2ClickMin;
                collisionInfo.neededCeilingDistance = 0;
                collisionInfo.frobbelFlags |= CollisionInfo::FrobbelFlag_UnwalkableSteepFloor | CollisionInfo::FrobbelFlag_UnpassableSteepUpslant;

                collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);

                return {};
            }
        };
    }
}
