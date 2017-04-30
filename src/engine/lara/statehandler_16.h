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


            void handleInput(CollisionInfo& /*collisionInfo*/) override
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

                if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Left )
                    subYRotationSpeed(2.25_deg, -4_deg);
                else if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Right )
                    addYRotationSpeed(2.25_deg, 4_deg);
            }


            void postprocessFrame(CollisionInfo& collisionInfo) override
            {
                setFallSpeed(0);
                setFalling(false);
                collisionInfo.passableFloorDistanceBottom = core::ClimbLimit2ClickMin;
                collisionInfo.passableFloorDistanceTop = -core::ClimbLimit2ClickMin;
                collisionInfo.neededCeilingDistance = 0;
                collisionInfo.facingAngle = getRotation().Y + 180_deg;
                setMovementAngle(collisionInfo.facingAngle);
                collisionInfo.policyFlags |= CollisionInfo::SlopesAreWalls | CollisionInfo::SlopesArePits;
                collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);

                if( stopIfCeilingBlocked(collisionInfo) )
                    return;

                if( checkWallCollision(collisionInfo) )
                {
                    setAnimIdGlobal(loader::AnimationId::STAY_SOLID, 185);
                }

                if( collisionInfo.mid.floor.distance > loader::QuarterSectorSize && collisionInfo.mid.floor.distance < core::ClimbLimit2ClickMin )
                {
                    if( getCurrentFrame() < 964 || getCurrentFrame() >= 994 )
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
