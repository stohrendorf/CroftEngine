#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "engine/inputstate.h"
#include "level/level.h"

namespace engine
{
    namespace lara
    {
        class StateHandler_5 final : public AbstractStateHandler
        {
        public:
            explicit StateHandler_5(LaraNode& lara)
                    : AbstractStateHandler(lara, LaraStateId::RunBack)
            {
            }


            void handleInputImpl(CollisionInfo& /*collisionInfo*/, const std::chrono::microseconds& deltaTime) override
            {
                setTargetState(LaraStateId::Stop);
            }

            void animateImpl(CollisionInfo& /*collisionInfo*/, const std::chrono::microseconds& deltaTime) override
            {
                if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Left )
                    subYRotationSpeed(deltaTime, 2.25_deg, -6_deg);
                else if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Right )
                    addYRotationSpeed(deltaTime, 2.25_deg, 6_deg);
            }


            void postprocessFrame(CollisionInfo& collisionInfo) override
            {
                setFallSpeed(core::makeInterpolatedValue(0.0f));
                setFalling(false);
                collisionInfo.passableFloorDistanceBottom = loader::HeightLimit;
                collisionInfo.passableFloorDistanceTop = -core::ClimbLimit2ClickMin;
                collisionInfo.neededCeilingDistance = 0;
                collisionInfo.policyFlags |= CollisionInfo::SlopesAreWalls | CollisionInfo::SlopesArePits;
                collisionInfo.yAngle = getRotation().Y + 180_deg;
                setMovementAngle(collisionInfo.yAngle);
                collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
                if( stopIfCeilingBlocked(collisionInfo) )
                    return;

                if( collisionInfo.current.floor.distance > 200 )
                {
                    setAnimIdGlobal(loader::AnimationId::FREE_FALL_BACK, 1473);
                    setTargetState(LaraStateId::FallBackward);
                    setFallSpeed(core::makeInterpolatedValue(0.0f));
                    setFalling(true);
                    return;
                }

                if(checkWallCollision(collisionInfo))
                {
                    setAnimIdGlobal(loader::AnimationId::STAY_SOLID, 185);
                }
                placeOnFloor(collisionInfo);
            }
        };
    }
}
