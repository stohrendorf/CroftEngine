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
                    subYRotationSpeed(2.25_deg, -4_deg);
                else if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Right )
                    addYRotationSpeed(2.25_deg, 4_deg);
            }


            void postprocessFrame(CollisionInfo& collisionInfo) override
            {
                setFallSpeed(0);
                setFalling(false);
                collisionInfo.badPositiveDistance = 128;
                collisionInfo.badNegativeDistance = -128;
                collisionInfo.badCeilingDistance = 0;
                collisionInfo.facingAngle = getRotation().Y - 90_deg;
                setMovementAngle(collisionInfo.facingAngle);
                collisionInfo.policyFlags |= CollisionInfo::SlopesAreWalls | CollisionInfo::SlopesArePits;
                collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);

                if( stopIfCeilingBlocked(collisionInfo) )
                    return;

                if( checkWallCollision(collisionInfo) )
                {
                    setAnimIdGlobal(loader::AnimationId::STAY_SOLID, 185);
                    setTargetState(LaraStateId::Stop);
                }

                if( !tryStartSlide(collisionInfo) )
                    placeOnFloor(collisionInfo);
            }
        };
    }
}
