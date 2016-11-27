#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "engine/inputstate.h"
#include "level/level.h"

namespace engine
{
    namespace lara
    {
        class StateHandler_Underwater : public AbstractStateHandler
        {
        public:
            explicit StateHandler_Underwater(LaraNode& lara)
                    : AbstractStateHandler(lara)
            {
            }

            boost::optional<LaraStateId> postprocessFrame(CollisionInfo& collisionInfo) override
            {
                collisionInfo.yAngle = getRotation().Y;
                if( abs(getRotation().X) > 90_deg )
                    collisionInfo.yAngle += 180_deg;
                setMovementAngle(collisionInfo.yAngle);
                collisionInfo.initHeightInfo(getPosition() + core::ExactTRCoordinates{0, 200, 0}, getLevel(), 400);

                applyCollisionFeedback(collisionInfo);

                m_xRotationSpeed = 0_deg;
                m_yRotationSpeed = 0_deg;

                switch( collisionInfo.axisCollisions )
                {
                    case CollisionInfo::AxisColl_FrontLeftBlocked:
                        m_yRotationSpeed = 5_deg;
                        break;
                    case CollisionInfo::AxisColl_FrontRightBlocked:
                        m_yRotationSpeed = -5_deg;
                        break;
                    case CollisionInfo::AxisColl_InvalidPosition:
                        setFallSpeed(core::makeInterpolatedValue(0.0f));
                        return {};
                    case CollisionInfo::AxisColl_InsufficientFrontCeilingSpace:
                        setFallSpeed(core::makeInterpolatedValue(0.0f));
                        break;
                    case CollisionInfo::AxisColl_ScalpCollision:
                        if( getRotation().X > -45_deg )
                            m_xRotationSpeed = -2_deg; // setXRotation(getRotation().X - 364);
                        break;
                    case CollisionInfo::AxisColl_FrontForwardBlocked:
                        if( getRotation().X > 35_deg )
                            m_xRotationSpeed = 2_deg; // setXRotation(getRotation().X + 364);
                        else if( getRotation().X < -35_deg )
                            m_xRotationSpeed = -2_deg; // setXRotation(getRotation().X - 364);
                        else
                            setFallSpeed(core::makeInterpolatedValue(0.0f));
                        break;
                    default:
                        break;
                }

                if( collisionInfo.current.floor.distance >= 0 )
                    return {};

                setPosition(getPosition() + core::ExactTRCoordinates(0, gsl::narrow_cast<float>(collisionInfo.current.floor.distance), 0));
                m_xRotationSpeed = m_xRotationSpeed + 2_deg;

                return {};
            }

        protected:
            void handleDiveInput()
            {
                if( getLevel().m_inputHandler->getInputState().zMovement == AxisMovement::Forward )
                    m_xRotationSpeed = -2_deg;
                else if( getLevel().m_inputHandler->getInputState().zMovement == AxisMovement::Backward )
                    m_xRotationSpeed = 2_deg;
                else
                    m_xRotationSpeed = 0_deg;
                if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Left )
                {
                    m_yRotationSpeed = -6_deg;
                    m_zRotationSpeed = -3_deg;
                }
                else if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Right )
                {
                    m_yRotationSpeed = 6_deg;
                    m_zRotationSpeed = 3_deg;
                }
                else
                {
                    m_yRotationSpeed = 0_deg;
                    m_zRotationSpeed = 0_deg;
                }
            }
        };
    }
}
