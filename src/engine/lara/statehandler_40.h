#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"

namespace engine
{
    namespace lara
    {
        class StateHandler_40 final : public AbstractStateHandler
        {
        public:
            explicit StateHandler_40(LaraNode& lara)
                    : AbstractStateHandler(lara, LaraStateId::SwitchDown)
            {
            }

            boost::optional<LaraStateId> handleInputImpl(CollisionInfo& collisionInfo) override
            {
                collisionInfo.frobbelFlags &= ~(CollisionInfo::FrobbelFlag10 | CollisionInfo::FrobbelFlag08);
                setCameraRotation(-25_deg, 80_deg);
                setCameraDistance(1024);

                return {};
            }

            void animateImpl(CollisionInfo& /*collisionInfo*/, const std::chrono::microseconds& /*deltaTimeMs*/) override
            {
            }

            boost::optional<LaraStateId> postprocessFrame(CollisionInfo& collisionInfo) override
            {
                setMovementAngle(getRotation().Y);
                collisionInfo.yAngle = getRotation().Y;
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
