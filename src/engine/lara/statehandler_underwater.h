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
            explicit StateHandler_Underwater(LaraNode& lara, LaraStateId id)
                : AbstractStateHandler(lara, id)
            {
            }


            void postprocessFrame(CollisionInfo& collisionInfo) override
            {
                collisionInfo.facingAngle = getRotation().Y;
                if( abs(getRotation().X) > 90_deg )
                    collisionInfo.facingAngle += 180_deg;
                setMovementAngle(collisionInfo.facingAngle);
                collisionInfo.initHeightInfo(getPosition() + core::TRCoordinates{0, 200, 0}, getLevel(), core::LaraHeightUnderwater);

                applyShift(collisionInfo);

                switch( collisionInfo.collisionType )
                {
                    case CollisionInfo::AxisColl_Left:
                        getLara().addYRotation(5_deg);
                        break;
                    case CollisionInfo::AxisColl_Right:
                        getLara().addYRotation(-5_deg);
                        break;
                    case CollisionInfo::AxisColl_TopFront:
                        setFallSpeed(0);
                        return;
                    case CollisionInfo::AxisColl_TopBottom:
                        setFallSpeed(0);
                        break;
                    case CollisionInfo::AxisColl_Top:
                        if( getRotation().X > -45_deg )
                            getLara().addXRotation(-2_deg);
                        break;
                    case CollisionInfo::AxisColl_Front:
                        if( getRotation().X > 35_deg )
                            getLara().addXRotation(2_deg);
                        else if( getRotation().X < -35_deg )
                            getLara().addXRotation(-2_deg);
                        else
                            setFallSpeed(0);
                        break;
                    default:
                        break;
                }

                if( collisionInfo.mid.floor.distance >= 0 )
                    return;

                placeOnFloor(collisionInfo);
                getLara().addXRotation(2_deg);
            }


        protected:
            void handleDiveRotationInput()
            {
                if( getLevel().m_inputHandler->getInputState().zMovement == AxisMovement::Forward )
                    getLara().addXRotation(-2_deg);
                else if( getLevel().m_inputHandler->getInputState().zMovement == AxisMovement::Backward )
                    getLara().addXRotation(2_deg);

                if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Left )
                {
                    getLara().addYRotation(-6_deg);
                    getLara().addZRotation(-3_deg);
                }
                else if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Right )
                {
                    getLara().addYRotation(6_deg);
                    getLara().addZRotation(3_deg);
                }
            }
        };
    }
}
