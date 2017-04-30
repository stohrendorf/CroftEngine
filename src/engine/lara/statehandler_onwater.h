#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "engine/inputstate.h"
#include "level/level.h"

#include "engine/laranode.h"


namespace engine
{
    namespace lara
    {
        class StateHandler_OnWater : public AbstractStateHandler
        {
        public:
            explicit StateHandler_OnWater(LaraNode& lara, LaraStateId id)
                : AbstractStateHandler(lara, id)
            {
            }


        protected:
            bool commonOnWaterHandling(CollisionInfo& collisionInfo)
            {
                collisionInfo.facingAngle = getMovementAngle();
                collisionInfo.initHeightInfo(getPosition() + core::TRCoordinates(0, 700, 0), getLevel(), 700);
                applyCollisionFeedback(collisionInfo);
                if( collisionInfo.mid.floor.distance < 0
                    || collisionInfo.collisionType == CollisionInfo::AxisColl_InvalidPosition
                    || collisionInfo.collisionType == CollisionInfo::AxisColl_InsufficientFrontCeilingSpace
                    || collisionInfo.collisionType == CollisionInfo::AxisColl_ScalpCollision
                    || collisionInfo.collisionType == CollisionInfo::AxisColl_FrontForwardBlocked
                )
                {
                    setFallSpeed(0);
                    setPosition(collisionInfo.oldPosition);
                }
                else
                {
                    if( collisionInfo.collisionType == CollisionInfo::AxisColl_FrontLeftBlocked )
                        getLara().addYRotation(5_deg);
                    else if( collisionInfo.collisionType == CollisionInfo::AxisColl_FrontRightBlocked )
                        getLara().addYRotation(-5_deg);
                }

                auto wsh = getLara().getWaterSurfaceHeight();
                if( wsh && *wsh > getPosition().Y - 100 )
                {
                    return tryClimbOutOfWater(collisionInfo);
                }

                setTargetState(LaraStateId::UnderwaterForward);
                setAnimIdGlobal(loader::AnimationId::FREE_FALL_TO_UNDERWATER_ALTERNATE, 2041);
                setXRotation(-45_deg);
                setFallSpeed(80);
                setUnderwaterState(UnderwaterState::Diving);
                return true;
            }


        private:
            bool tryClimbOutOfWater(CollisionInfo& collisionInfo)
            {
                if( getMovementAngle() != getRotation().Y )
                    return false;

                if( collisionInfo.collisionType != CollisionInfo::AxisColl_FrontForwardBlocked )
                    return false;

                if( !getLevel().m_inputHandler->getInputState().action )
                    return false;

                const auto gradient = std::abs(collisionInfo.frontLeft.floor.distance - collisionInfo.frontRight.floor.distance);
                if( gradient >= core::MaxGrabbableGradient )
                    return false;

                if( collisionInfo.front.ceiling.distance > 0 )
                    return false;

                if( collisionInfo.mid.ceiling.distance > -core::ClimbLimit2ClickMin )
                    return false;

                if( collisionInfo.front.floor.distance + 700 <= -2 * loader::QuarterSectorSize )
                    return false;

                if( collisionInfo.front.floor.distance + 700 > 100 )
                    return false;

                const auto yRot = alignRotation(getRotation().Y, 35_deg);
                if( !yRot )
                    return false;

                setPosition(getPosition() + core::TRCoordinates(0, 695 + collisionInfo.front.floor.distance, 0));
                getLara().updateFloorHeight(-381);
                core::TRCoordinates d = getPosition();
                if( *yRot == 0_deg )
                    d.Z = (floor(getPosition().Z / loader::SectorSize) + 1) * loader::SectorSize + 100;
                else if( *yRot == 180_deg )
                    d.Z = (floor(getPosition().Z / loader::SectorSize) + 0) * loader::SectorSize - 100;
                else if( *yRot == -90_deg )
                    d.X = (floor(getPosition().X / loader::SectorSize) + 0) * loader::SectorSize - 100;
                else if( *yRot == 90_deg )
                    d.X = (floor(getPosition().X / loader::SectorSize) + 1) * loader::SectorSize + 100;
                else
                    throw std::runtime_error("Unexpected angle value");

                setPosition(d);

                setTargetState(LaraStateId::Stop);
                setAnimIdGlobal(loader::AnimationId::CLIMB_OUT_OF_WATER, 1849);
                setHorizontalSpeed(0);
                setFallSpeed(0);
                setFalling(false);
                setXRotation(0_deg);
                setYRotation(*yRot);
                setZRotation(0_deg);
                setHandStatus(1);
                setUnderwaterState(UnderwaterState::OnLand);
                return true;
            }
        };
    }
}
