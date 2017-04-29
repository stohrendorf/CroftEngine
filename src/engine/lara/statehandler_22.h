#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "engine/inputstate.h"
#include "level/level.h"


namespace engine
{
    namespace lara
    {
        class StateHandler_22 final : public AbstractStateHandler
        {
        public:
            explicit StateHandler_22(LaraNode& lara)
                : AbstractStateHandler(lara, LaraStateId::StepLeft)
            {
            }


            void handleInput(CollisionInfo& /*collisionInfo*/) override
            {
                if( getHealth() <= 0 )
                {
                    setTargetState(LaraStateId::Stop);
                    return;
                }

                if( getLevel().m_inputHandler->getInputState().stepMovement != AxisMovement::Left )
                    setTargetState(LaraStateId::Stop);

                if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Left )
                    setYRotationSpeed(std::max(-4_deg, getYRotationSpeed() - 2.25_deg));
                else if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Right )
                    setYRotationSpeed(std::min(+4_deg, getYRotationSpeed() + 2.25_deg));
            }


            void postprocessFrame(CollisionInfo& collisionInfo) override
            {
                setFallSpeed(0);
                setFalling(false);
                collisionInfo.passableFloorDistanceBottom = 128;
                collisionInfo.passableFloorDistanceTop = -128;
                collisionInfo.neededCeilingDistance = 0;
                collisionInfo.yAngle = getRotation().Y - 90_deg;
                setMovementAngle(collisionInfo.yAngle);
                collisionInfo.policyFlags |= CollisionInfo::SlopesAreWalls | CollisionInfo::SlopesArePits;
                collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);

                if( stopIfCeilingBlocked(collisionInfo) )
                    return;

                if( checkWallCollision(collisionInfo) )
                {
                    setAnimIdGlobal(loader::AnimationId::STAY_SOLID, 185);
                    setTargetState(LaraStateId::Stop);
                    return;
                }

                if( !tryStartSlide(collisionInfo) )
                    setPosition(getPosition() + core::TRCoordinates(0, collisionInfo.current.floor.distance, 0));
            }
        };
    }
}
