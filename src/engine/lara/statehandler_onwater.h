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
                collisionInfo.yAngle = getMovementAngle();
                collisionInfo.initHeightInfo(getPosition() + core::ExactTRCoordinates(0, 700, 0), getLevel(), 700);
                applyCollisionFeedback(collisionInfo);
                if( collisionInfo.current.floor.distance < 0
                    || collisionInfo.axisCollisions == CollisionInfo::AxisColl_InvalidPosition
                    || collisionInfo.axisCollisions == CollisionInfo::AxisColl_InsufficientFrontCeilingSpace
                    || collisionInfo.axisCollisions == CollisionInfo::AxisColl_ScalpCollision
                    || collisionInfo.axisCollisions == CollisionInfo::AxisColl_FrontForwardBlocked
                        )
                {
                    setFallSpeed(core::makeInterpolatedValue(0.0f));
                    setPosition(collisionInfo.oldPosition);
                }
                else
                {
                    if( collisionInfo.axisCollisions == CollisionInfo::AxisColl_FrontLeftBlocked )
                        m_yRotationSpeed = 5_deg;
                    else if( collisionInfo.axisCollisions == CollisionInfo::AxisColl_FrontRightBlocked )
                        m_yRotationSpeed = -5_deg;
                    else
                        m_yRotationSpeed = 0_deg;
                }

                auto wsh = getLara().getWaterSurfaceHeight();
                if( wsh && *wsh > getPosition().Y - 100 )
                {
                    return tryClimbOutOfWater(collisionInfo);
                }

                setTargetState(LaraStateId::UnderwaterForward);
                setAnimIdGlobal(loader::AnimationId::FREE_FALL_TO_UNDERWATER_ALTERNATE, 2041);
                setXRotation(-45_deg);
                setFallSpeed(core::makeInterpolatedValue(80.0f));
                setUnderwaterState(UnderwaterState::Diving);
                return true;
            }

        private:
            bool tryClimbOutOfWater(CollisionInfo& collisionInfo)
            {
                if( getMovementAngle() != getRotation().Y )
                    return false;

                if( collisionInfo.axisCollisions != CollisionInfo::AxisColl_FrontForwardBlocked )
                    return false;

                if( !getLevel().m_inputHandler->getInputState().action )
                    return false;

                const auto gradient = std::abs(collisionInfo.frontLeft.floor.distance - collisionInfo.frontRight.floor.distance);
                if( gradient >= core::MaxGrabbableGradient )
                    return false;

                if( collisionInfo.front.ceiling.distance > 0 )
                    return false;

                if( collisionInfo.current.ceiling.distance > -core::ClimbLimit2ClickMin )
                    return false;

                if( collisionInfo.front.floor.distance + 700 <= -2 * loader::QuarterSectorSize )
                    return false;

                if( collisionInfo.front.floor.distance + 700 > 100 )
                    return false;

                const auto yRot = core::alignRotation(getRotation().Y, 35_deg);
                if( !yRot )
                    return false;

                setPosition(getPosition() + core::ExactTRCoordinates(0, 695 + gsl::narrow_cast<float>(collisionInfo.front.floor.distance), 0));
                getLara().updateFloorHeight(-381);
                core::ExactTRCoordinates d = getPosition();
                if( *yRot == 0_deg )
                    d.Z = (std::floor(getPosition().Z / loader::SectorSize) + 1) * loader::SectorSize + 100;
                else if( *yRot == 180_deg )
                    d.Z = (std::floor(getPosition().Z / loader::SectorSize) + 0) * loader::SectorSize - 100;
                else if( *yRot == -90_deg )
                    d.X = (std::floor(getPosition().X / loader::SectorSize) + 0) * loader::SectorSize - 100;
                else if( *yRot == 90_deg )
                    d.X = (std::floor(getPosition().X / loader::SectorSize) + 1) * loader::SectorSize + 100;
                else
                    throw std::runtime_error("Unexpected angle value");

                setPosition(d);

                setTargetState(LaraStateId::Stop);
                setAnimIdGlobal(loader::AnimationId::CLIMB_OUT_OF_WATER, 1849);
                setHorizontalSpeed(core::makeInterpolatedValue(0.0f));
                setFallSpeed(core::makeInterpolatedValue(0.0f));
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
