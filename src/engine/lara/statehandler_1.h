#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "engine/inputstate.h"
#include "level/level.h"

namespace engine
{
    namespace lara
    {
        class StateHandler_1 final : public AbstractStateHandler
        {
        public:
            explicit StateHandler_1(LaraNode& lara)
                    : AbstractStateHandler( lara, LaraStateId::RunForward )
            {
            }


            void handleInputImpl(CollisionInfo& /*collisionInfo*/, const std::chrono::microseconds& deltaTime) override
            {
                if( getHealth() <= 0 )
                {
                    setTargetState( LaraStateId::Death );
                    return;
                }

                if( getLevel().m_inputHandler->getInputState().roll )
                {
                    setAnimIdGlobal( loader::AnimationId::ROLL_BEGIN, 3857 );
                    setTargetState( LaraStateId::Stop );
                    return;
                }

                if( getLevel().m_inputHandler->getInputState().jump && !isFalling() )
                {
                    setTargetState( LaraStateId::JumpForward );
                    return;
                }

                if( getLevel().m_inputHandler->getInputState().zMovement != AxisMovement::Forward )
                {
                    setTargetState( LaraStateId::Stop );
                    return;
                }

                if( getLevel().m_inputHandler->getInputState().moveSlow )
                    setTargetState( LaraStateId::WalkForward );
                else
                    setTargetState( LaraStateId::RunForward );
            }


            void animateImpl(CollisionInfo& /*collisionInfo*/, const std::chrono::microseconds& deltaTime) override
            {
                if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Left )
                {
                    subYRotationSpeed( deltaTime, 2.25_deg, -8_deg );
                    setZRotation( std::max( -11_deg, getRotation().Z - core::makeInterpolatedValue( +1.5_deg )
                            .getScaled( deltaTime ) ) );
                }
                else if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Right )
                {
                    addYRotationSpeed( deltaTime, 2.25_deg, 8_deg );
                    setZRotation( std::min( +11_deg, getRotation().Z + core::makeInterpolatedValue( +1.5_deg )
                            .getScaled( deltaTime ) ) );
                }
            }


            void postprocessFrame(CollisionInfo& collisionInfo) override
            {
                collisionInfo.yAngle = getRotation().Y;
                setMovementAngle( collisionInfo.yAngle );
                collisionInfo.passableFloorDistanceBottom = loader::HeightLimit;
                collisionInfo.passableFloorDistanceTop = -core::ClimbLimit2ClickMin;
                collisionInfo.neededCeilingDistance = 0;
                collisionInfo.policyFlags |= CollisionInfo::SlopesAreWalls;
                collisionInfo.initHeightInfo( getPosition(), getLevel(), core::ScalpHeight );

                if(stopIfCeilingBlocked(collisionInfo))
                    return;

                if(tryClimb(collisionInfo))
                    return;

                if(checkWallCollision(collisionInfo))
                {
                    setZRotation( 0_deg );
                    if( collisionInfo.front.floor.slantClass == SlantClass::None
                        && collisionInfo.front.floor.distance < -core::ClimbLimit2ClickMax )
                    {
                        if( getCurrentTime() < 10_frame )
                        {
                            setAnimIdGlobal( loader::AnimationId::WALL_SMASH_LEFT, 800 );
                            return;
                        }
                        if( getCurrentTime() >= 10_frame && getCurrentTime() < 22_frame )
                        {
                            setAnimIdGlobal( loader::AnimationId::WALL_SMASH_RIGHT, 815 );
                            return;
                        }
                    }

                    setAnimIdGlobal( loader::AnimationId::STAY_SOLID, 185 );
                }

                if( collisionInfo.current.floor.distance > core::ClimbLimit2ClickMin )
                {
                    setAnimIdGlobal( loader::AnimationId::FREE_FALL_FORWARD, 492 );
                    setTargetState( LaraStateId::JumpForward );
                    setFalling( true );
                    setFallSpeed( core::makeInterpolatedValue( 0.0f ) );
                    return;
                }

                if( collisionInfo.current.floor.distance >= -core::ClimbLimit2ClickMin
                    && collisionInfo.current.floor.distance < -core::SteppableHeight )
                {
                    if( getCurrentTime() >= 3_frame && getCurrentTime() < 15_frame )
                    {
                        setAnimIdGlobal( loader::AnimationId::RUN_UP_STEP_LEFT, 837 );
                    }
                    else
                    {
                        setAnimIdGlobal( loader::AnimationId::RUN_UP_STEP_RIGHT, 830 );
                    }
                }

                if( !tryStartSlide( collisionInfo) )
                {
                    if( collisionInfo.current.floor.distance > 50 )
                        collisionInfo.current.floor.distance = 50;
                    placeOnFloor( collisionInfo );
                }
            }
        };
    }
}