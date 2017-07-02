#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "engine/inputstate.h"
#include "level/level.h"


namespace engine
{
    namespace lara
    {
        class StateHandler_0 final : public AbstractStateHandler
        {
        public:

            explicit StateHandler_0(LaraNode& lara)
                : AbstractStateHandler(lara, LaraStateId::WalkForward)
            {
            }


            void handleInput(CollisionInfo& /*collisionInfo*/) override
            {
                if( getHealth() <= 0 )
                {
                    setTargetState(LaraStateId::Stop);
                    return;
                }

                if (getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Left)
                    subYRotationSpeed(2.25_deg, -4_deg);
                else if (getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Right)
                    addYRotationSpeed(2.25_deg, 4_deg);

                if( getLevel().m_inputHandler->getInputState().zMovement == AxisMovement::Forward )
                {
                    if( getLevel().m_inputHandler->getInputState().moveSlow )
                        setTargetState(LaraStateId::WalkForward);
                    else
                        setTargetState(LaraStateId::RunForward);
                }
                else
                {
                    setTargetState(LaraStateId::Stop);
                }
            }


            void postprocessFrame(CollisionInfo& collisionInfo) override
            {
                setFallSpeed(0);
                setFalling(false);
                collisionInfo.facingAngle = getRotation().Y;
                setMovementAngle(collisionInfo.facingAngle);
                collisionInfo.badPositiveDistance = core::ClimbLimit2ClickMin;
                collisionInfo.badNegativeDistance = -core::ClimbLimit2ClickMin;
                collisionInfo.badCeilingDistance = 0;
                collisionInfo.policyFlags |= CollisionInfo::SlopesAreWalls | CollisionInfo::SlopesArePits | CollisionInfo::LavaIsPit;
                collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);

                if( stopIfCeilingBlocked(collisionInfo) )
                    return;

                if( tryClimb(collisionInfo) )
                    return;

                if( checkWallCollision(collisionInfo) )
                {
                    const auto fr = getCurrentFrame();
                    if( fr >= 29 && fr <= 47 )
                    {
                        setAnimIdGlobal(loader::AnimationId::END_WALK_LEFT, 74);
                    }
                    else if( (fr >= 22 && fr <= 28) || (fr >= 48 && fr <= 57) )
                    {
                        setAnimIdGlobal(loader::AnimationId::END_WALK_RIGHT, 58);
                    }
                    else
                    {
                        setAnimIdGlobal(loader::AnimationId::STAY_SOLID, 185);
                    }
                }

                if( collisionInfo.mid.floor.distance > core::ClimbLimit2ClickMin )
                {
                    setAnimIdGlobal(loader::AnimationId::FREE_FALL_FORWARD, 492);
                    setTargetState(LaraStateId::JumpForward);
                    setFallSpeed(0);
                    setFalling(true);
                }

                if( collisionInfo.mid.floor.distance > core::SteppableHeight )
                {
                    const auto fr = getCurrentFrame();
                    if( fr < 28 || fr > 45 )
                    {
                        setAnimIdGlobal(loader::AnimationId::WALK_DOWN_RIGHT, 887);
                    }
                    else
                    {
                        setAnimIdGlobal(loader::AnimationId::WALK_DOWN_LEFT, 874);
                    }
                }

                if( collisionInfo.mid.floor.distance >= -core::ClimbLimit2ClickMin && collisionInfo.mid.floor.distance < -core::SteppableHeight )
                {
                    const auto fr = getCurrentFrame();
                    if( fr < 27 || fr > 44 )
                    {
                        setAnimIdGlobal(loader::AnimationId::WALK_UP_STEP_RIGHT, 844);
                    }
                    else
                    {
                        setAnimIdGlobal(loader::AnimationId::WALK_UP_STEP_LEFT, 858);
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
