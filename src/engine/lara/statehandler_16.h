#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "engine/inputstate.h"
#include "level/level.h"

namespace engine
{
    namespace lara
    {
        class StateHandler_16 final : public AbstractStateHandler
        {
        public:
            explicit StateHandler_16(LaraNode& lara)
                    : AbstractStateHandler(lara, LaraStateId::WalkBackward)
            {
            }


            void handleInputImpl(CollisionInfo& /*collisionInfo*/, const std::chrono::microseconds& deltaTime) override
            {
                if( getHealth() <= 0 )
                {
                    setTargetState(LaraStateId::Stop);
                    return;
                }

                if( getLevel().m_inputHandler->getInputState().zMovement == AxisMovement::Backward && getLevel().m_inputHandler->getInputState().moveSlow )
                    setTargetState(LaraStateId::WalkBackward);
                else
                    setTargetState(LaraStateId::Stop);
            }

            void animateImpl(CollisionInfo& /*collisionInfo*/, const std::chrono::microseconds& deltaTime) override
            {
                if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Left )
                    subYRotationSpeed(deltaTime, 2.25_deg, -4_deg);
                else if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Right )
                    addYRotationSpeed(deltaTime, 2.25_deg, 4_deg);
            }


            void postprocessFrame(CollisionInfo& collisionInfo) override
            {
                setFallSpeed(core::makeInterpolatedValue(0.0f));
                setFalling(false);
                collisionInfo.passableFloorDistanceBottom = core::ClimbLimit2ClickMin;
                collisionInfo.passableFloorDistanceTop = -core::ClimbLimit2ClickMin;
                collisionInfo.neededCeilingDistance = 0;
                collisionInfo.yAngle = getRotation().Y + 180_deg;
                setMovementAngle(collisionInfo.yAngle);
                collisionInfo.policyFlags |= CollisionInfo::SlopesAreWalls | CollisionInfo::SlopesArePits;
                collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);

                if( stopIfCeilingBlocked(collisionInfo) )
                    return;

                if(checkWallCollision(collisionInfo))
                {
                    setAnimIdGlobal(loader::AnimationId::STAY_SOLID, 185);
                }

                if( collisionInfo.current.floor.distance > loader::QuarterSectorSize && collisionInfo.current.floor.distance < core::ClimbLimit2ClickMin )
                {
                    if(getCurrentTime() < 964_frame || getCurrentTime() >= 994_frame)
                    {
                        setAnimIdGlobal(loader::AnimationId::WALK_DOWN_BACK_LEFT, 899);
                    }
                    else
                    {
                        setAnimIdGlobal(loader::AnimationId::WALK_DOWN_BACK_RIGHT, 930);
                    }
                }

                if( !tryStartSlide(collisionInfo) )
                {
                    placeOnFloor(collisionInfo);
                }
            }
        };
    }
}
