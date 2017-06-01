#include "abstractstatehandler.h"

#include "statehandler_0.h"
#include "statehandler_1.h"
#include "statehandler_2.h"
#include "statehandler_3.h"
#include "statehandler_4.h"
#include "statehandler_5.h"
#include "statehandler_6.h"
#include "statehandler_7.h"
#include "statehandler_8.h"
#include "statehandler_9.h"
#include "statehandler_10.h"
#include "statehandler_11.h"
#include "statehandler_12.h"
#include "statehandler_13.h"
#include "statehandler_14.h"
#include "statehandler_15.h"
#include "statehandler_16.h"
#include "statehandler_17.h"
#include "statehandler_18.h"
#include "statehandler_19.h"
#include "statehandler_20.h"
#include "statehandler_21.h"
#include "statehandler_22.h"
#include "statehandler_23.h"
#include "statehandler_24.h"
#include "statehandler_25.h"
#include "statehandler_26.h"
#include "statehandler_27.h"
#include "statehandler_28.h"
#include "statehandler_29.h"
#include "statehandler_30.h"
#include "statehandler_31.h"
#include "statehandler_32.h"
#include "statehandler_33.h"
#include "statehandler_34.h"
#include "statehandler_35.h"
#include "statehandler_36.h"
#include "statehandler_37.h"
#include "statehandler_38.h"
#include "statehandler_39.h"
#include "statehandler_40.h"
#include "statehandler_41.h"
#include "statehandler_42.h"
#include "statehandler_43.h"
#include "statehandler_44.h"
#include "statehandler_45.h"
#include "statehandler_46.h"
#include "statehandler_47.h"
#include "statehandler_48.h"
#include "statehandler_49.h"
#include "statehandler_50.h"
#include "statehandler_51.h"
#include "statehandler_52.h"
#include "statehandler_53.h"
#include "statehandler_54.h"
#include "statehandler_55.h"

#include <cstdlib>

namespace engine
{
    namespace lara
    {
        std::unique_ptr<AbstractStateHandler> AbstractStateHandler::create(loader::LaraStateId id, LaraNode& lara)
        {
            switch( id )
            {
                case LaraStateId::WalkForward: return std::make_unique<StateHandler_0>(lara);
                case LaraStateId::RunForward: return std::make_unique<StateHandler_1>(lara);
                case LaraStateId::Stop: return std::make_unique<StateHandler_2>(lara);
                case LaraStateId::JumpForward: return std::make_unique<StateHandler_3>(lara);
                case LaraStateId::Pose: return std::make_unique<StateHandler_4>(lara);
                case LaraStateId::RunBack: return std::make_unique<StateHandler_5>(lara);
                case LaraStateId::TurnRightSlow: return std::make_unique<StateHandler_6>(lara);
                case LaraStateId::TurnLeftSlow: return std::make_unique<StateHandler_7>(lara);
                case LaraStateId::Death: return std::make_unique<StateHandler_8>(lara);
                case LaraStateId::FreeFall: return std::make_unique<StateHandler_9>(lara);
                case LaraStateId::Hang: return std::make_unique<StateHandler_10>(lara);
                case LaraStateId::Reach: return std::make_unique<StateHandler_11>(lara);
                case LaraStateId::Unknown12: return std::make_unique<StateHandler_12>(lara);
                case LaraStateId::UnderwaterStop: return std::make_unique<StateHandler_13>(lara);
                case LaraStateId::GrabToFall: return std::make_unique<StateHandler_14>(lara);
                case LaraStateId::JumpPrepare: return std::make_unique<StateHandler_15>(lara);
                case LaraStateId::WalkBackward: return std::make_unique<StateHandler_16>(lara);
                case LaraStateId::UnderwaterForward: return std::make_unique<StateHandler_17>(lara);
                case LaraStateId::UnderwaterInertia: return std::make_unique<StateHandler_18>(lara);
                case LaraStateId::Climbing: return std::make_unique<StateHandler_19>(lara);
                case LaraStateId::TurnFast: return std::make_unique<StateHandler_20>(lara);
                case LaraStateId::StepRight: return std::make_unique<StateHandler_21>(lara);
                case LaraStateId::StepLeft: return std::make_unique<StateHandler_22>(lara);
                case LaraStateId::RollBackward: return std::make_unique<StateHandler_23>(lara);
                case LaraStateId::SlideForward: return std::make_unique<StateHandler_24>(lara);
                case LaraStateId::JumpBack: return std::make_unique<StateHandler_25>(lara);
                case LaraStateId::JumpLeft: return std::make_unique<StateHandler_26>(lara);
                case LaraStateId::JumpRight: return std::make_unique<StateHandler_27>(lara);
                case LaraStateId::JumpUp: return std::make_unique<StateHandler_28>(lara);
                case LaraStateId::FallBackward: return std::make_unique<StateHandler_29>(lara);
                case LaraStateId::ShimmyLeft: return std::make_unique<StateHandler_30>(lara);
                case LaraStateId::ShimmyRight: return std::make_unique<StateHandler_31>(lara);
                case LaraStateId::SlideBackward: return std::make_unique<StateHandler_32>(lara);
                case LaraStateId::OnWaterStop: return std::make_unique<StateHandler_33>(lara);
                case LaraStateId::OnWaterForward: return std::make_unique<StateHandler_34>(lara);
                case LaraStateId::UnderwaterDiving: return std::make_unique<StateHandler_35>(lara);
                case LaraStateId::PushablePush: return std::make_unique<StateHandler_36>(lara);
                case LaraStateId::PushablePull: return std::make_unique<StateHandler_37>(lara);
                case LaraStateId::PushableGrab: return std::make_unique<StateHandler_38>(lara);
                case LaraStateId::PickUp: return std::make_unique<StateHandler_39>(lara);
                case LaraStateId::SwitchDown: return std::make_unique<StateHandler_40>(lara);
                case LaraStateId::SwitchUp: return std::make_unique<StateHandler_41>(lara);
                case LaraStateId::InsertKey: return std::make_unique<StateHandler_42>(lara);
                case LaraStateId::InsertPuzzle: return std::make_unique<StateHandler_43>(lara);
                case LaraStateId::WaterDeath: return std::make_unique<StateHandler_44>(lara);
                case LaraStateId::RollForward: return std::make_unique<StateHandler_45>(lara);
                case LaraStateId::OnWaterBackward: return std::make_unique<StateHandler_47>(lara);
                case LaraStateId::OnWaterLeft: return std::make_unique<StateHandler_48>(lara);
                case LaraStateId::OnWaterRight: return std::make_unique<StateHandler_49>(lara);
                case LaraStateId::SwandiveBegin: return std::make_unique<StateHandler_52>(lara);
                case LaraStateId::SwandiveEnd: return std::make_unique<StateHandler_53>(lara);
                case LaraStateId::Handstand: return std::make_unique<StateHandler_54>(lara);
                case LaraStateId::OnWaterExit: return std::make_unique<StateHandler_55>(lara);
                default:
                    BOOST_LOG_TRIVIAL( error ) << "No state handler for state " << loader::toString(id);
                    throw std::runtime_error("Unhandled state");
            }

            return nullptr;
        }


        int AbstractStateHandler::getHealth() const noexcept
        {
            return m_lara.getHealth();
        }


        void AbstractStateHandler::setHealth(int h) noexcept
        {
            m_lara.setHealth(h);
        }


        void AbstractStateHandler::setAir(int a) noexcept
        {
            m_lara.setAir(a);
        }


        void AbstractStateHandler::setMovementAngle(core::Angle angle) noexcept
        {
            m_lara.setMovementAngle(angle);
        }


        core::Angle AbstractStateHandler::getMovementAngle() const noexcept
        {
            return m_lara.getMovementAngle();
        }


        void AbstractStateHandler::setFallSpeed(int spd)
        {
            m_lara.setFallSpeed(spd);
        }


        int AbstractStateHandler::getFallSpeed() const noexcept
        {
            return m_lara.getFallSpeed();
        }


        bool AbstractStateHandler::isFalling() const noexcept
        {
            return m_lara.isFalling();
        }


        void AbstractStateHandler::setFalling(bool falling) noexcept
        {
            m_lara.setFalling(falling);
        }


        int AbstractStateHandler::getHandStatus() const noexcept
        {
            return m_lara.getHandStatus();
        }


        void AbstractStateHandler::setHandStatus(int status) noexcept
        {
            m_lara.setHandStatus(status);
        }


        int AbstractStateHandler::getCurrentFrame() const
        {
            return m_lara.getCurrentFrame();
        }


        loader::LaraStateId AbstractStateHandler::getCurrentAnimState() const
        {
            return m_lara.getCurrentAnimState();
        }


        void AbstractStateHandler::setAnimIdGlobal(loader::AnimationId anim, const boost::optional<uint16_t>& firstFrame)
        {
            m_lara.setAnimIdGlobal(anim, firstFrame);
            m_lara.updatePose();
        }


        const core::TRRotation& AbstractStateHandler::getRotation() const noexcept
        {
            return m_lara.getRotation();
        }


        void AbstractStateHandler::setHorizontalSpeed(int speed)
        {
            m_lara.setHorizontalSpeed(speed);
        }


        int AbstractStateHandler::getHorizontalSpeed() const
        {
            return m_lara.getHorizontalSpeed();
        }


        const level::Level& AbstractStateHandler::getLevel() const
        {
            return m_lara.getLevel();
        }


        void AbstractStateHandler::placeOnFloor(const CollisionInfo& collisionInfo)
        {
            m_lara.placeOnFloor(collisionInfo);
        }


        const core::TRCoordinates& AbstractStateHandler::getPosition() const
        {
            return m_lara.getPosition();
        }


        void AbstractStateHandler::setPosition(const core::TRCoordinates& pos)
        {
            m_lara.setPosition(pos);
        }


        int AbstractStateHandler::getFloorHeight() const
        {
            return m_lara.getFloorHeight();
        }


        void AbstractStateHandler::setFloorHeight(int h)
        {
            m_lara.setFloorHeight(h);
        }


        void AbstractStateHandler::setYRotationSpeed(core::Angle spd)
        {
            m_lara.setYRotationSpeed(spd);
        }


        core::Angle AbstractStateHandler::getYRotationSpeed() const
        {
            return m_lara.getYRotationSpeed();
        }


        void AbstractStateHandler::subYRotationSpeed(core::Angle val,
                                                     core::Angle limit)
        {
            m_lara.subYRotationSpeed(val, limit);
        }


        void AbstractStateHandler::addYRotationSpeed(core::Angle val,
                                                     core::Angle limit)
        {
            m_lara.addYRotationSpeed(val, limit);
        }


        void AbstractStateHandler::setXRotation(core::Angle x)
        {
            m_lara.setXRotation(x);
        }


        void AbstractStateHandler::setYRotation(core::Angle y)
        {
            m_lara.setYRotation(y);
        }


        void AbstractStateHandler::setZRotation(core::Angle z)
        {
            m_lara.setZRotation(z);
        }


        void AbstractStateHandler::setFallSpeedOverride(int v)
        {
            m_lara.setFallSpeedOverride(v);
        }


        void AbstractStateHandler::dampenHorizontalSpeed(float f)
        {
            m_lara.dampenHorizontalSpeed(f);
        }


        core::Angle AbstractStateHandler::getCurrentSlideAngle() const noexcept
        {
            return m_lara.getCurrentSlideAngle();
        }


        void AbstractStateHandler::setCurrentSlideAngle(core::Angle a) noexcept
        {
            m_lara.setCurrentSlideAngle(a);
        }


        void AbstractStateHandler::setTargetState(loader::LaraStateId state)
        {
            m_lara.setTargetState(state);
        }


        loader::LaraStateId AbstractStateHandler::getTargetState() const
        {
            return m_lara.getTargetState();
        }


        bool AbstractStateHandler::canClimbOnto(core::Axis axis) const
        {
            auto pos = getPosition();
            switch( axis )
            {
                case core::Axis::PosZ: pos.Z += 256;
                    break;
                case core::Axis::PosX: pos.X += 256;
                    break;
                case core::Axis::NegZ: pos.Z -= 256;
                    break;
                case core::Axis::NegX: pos.X -= 256;
                    break;
            }

            auto sector = getLevel()
                .findRealFloorSector(pos, m_lara.getCurrentRoom());
            HeightInfo floor = HeightInfo::fromFloor(sector, pos, getLevel().m_cameraController);
            HeightInfo ceil = HeightInfo::fromCeiling(sector, pos, getLevel().m_cameraController);
            return floor.distance != -loader::HeightLimit && floor.distance - pos.Y > 0 && ceil.distance - pos.Y < -400;
        }


        bool AbstractStateHandler::tryReach(CollisionInfo& collisionInfo)
        {
            if( collisionInfo.collisionType != CollisionInfo::AxisColl_FrontForwardBlocked
                || !getLevel().m_inputHandler->getInputState().action || getHandStatus() != 0 )
                return false;

            if( std::labs(collisionInfo.frontLeft.floor.distance - collisionInfo.frontRight.floor.distance)
                >= core::MaxGrabbableGradient )
                return false;

            if( collisionInfo.front.ceiling.distance > 0
                || collisionInfo.mid.ceiling.distance > -core::ClimbLimit2ClickMin
                || collisionInfo.mid.floor.distance < 200 ) //! @todo Magick 200
                return false;

            const auto bbox = getBoundingBox();
            int spaceToReach = collisionInfo.front.floor.distance - bbox.minY;

            BOOST_LOG_TRIVIAL(debug) << "spaceToReach = " << spaceToReach << ", getFallSpeed() + spaceToReach = " << (getFallSpeed() + spaceToReach);

            if( spaceToReach < 0 && getFallSpeed() + spaceToReach < 0 )
                return false;
            if( spaceToReach > 0 && getFallSpeed() + spaceToReach > 0 )
                return false;

            auto alignedRotation = alignRotation(getRotation().Y, 35_deg);
            if( !alignedRotation )
                return false;

            if( canClimbOnto(*axisFromAngle(getRotation().Y, 35_deg)) )
                setAnimIdGlobal(loader::AnimationId::OSCILLATE_HANG_ON, 3974);
            else
                setAnimIdGlobal(loader::AnimationId::HANG_IDLE, 1493);

            setTargetState(LaraStateId::Hang);
            setPosition(getPosition() + core::TRCoordinates(collisionInfo.shift.X, spaceToReach,
                                                            collisionInfo.shift.Z));
            setHorizontalSpeed(0);
            setYRotation(*alignedRotation);
            setFalling(false);
            setFallSpeed(0);
            setHandStatus(1);
            return true;
        }


        bool AbstractStateHandler::stopIfCeilingBlocked(const CollisionInfo& collisionInfo)
        {
            if( collisionInfo.collisionType != CollisionInfo::AxisColl_ScalpCollision
                && collisionInfo.collisionType != CollisionInfo::AxisColl_InvalidPosition )
                return false;

            setPosition(collisionInfo.oldPosition);

            setTargetState(LaraStateId::Stop);
            setAnimIdGlobal(loader::AnimationId::STAY_SOLID, 185);
            setHorizontalSpeed(0);
            setFallSpeed(0);
            setFalling(false);
            return true;
        }


        bool AbstractStateHandler::tryClimb(CollisionInfo& collisionInfo)
        {
            if( collisionInfo.collisionType != CollisionInfo::AxisColl_FrontForwardBlocked
                || !getLevel().m_inputHandler->getInputState().action || getHandStatus() != 0 )
                return false;

            const auto floorGradient = std::labs(
                collisionInfo.frontLeft.floor.distance - collisionInfo.frontRight.floor.distance);
            if( floorGradient >= core::MaxGrabbableGradient )
                return false;

            //! @todo MAGICK +/- 30 degrees
            auto alignedRotation = alignRotation(getRotation().Y, 30_deg);
            if( !alignedRotation )
                return false;

            const auto climbHeight = collisionInfo.front.floor.distance;
            if( climbHeight >= -core::ClimbLimit2ClickMax && climbHeight <= -core::ClimbLimit2ClickMin )
            {
                if( climbHeight < collisionInfo.front.ceiling.distance
                    || collisionInfo.frontLeft.floor.distance < collisionInfo.frontLeft.ceiling.distance
                    || collisionInfo.frontRight.floor.distance < collisionInfo.frontRight.ceiling.distance )
                    return false;

                setTargetState(LaraStateId::Stop);
                setAnimIdGlobal(loader::AnimationId::CLIMB_2CLICK, 759);
                m_lara.moveY(2 * loader::QuarterSectorSize + climbHeight);
                setHandStatus(1);
            }
            else if( climbHeight >= -core::ClimbLimit3ClickMax && climbHeight <= -core::ClimbLimit2ClickMax )
            {
                if( collisionInfo.front.floor.distance < collisionInfo.front.ceiling.distance
                    || collisionInfo.frontLeft.floor.distance < collisionInfo.frontLeft.ceiling.distance
                    || collisionInfo.frontRight.floor.distance < collisionInfo.frontRight.ceiling.distance )
                    return false;

                setTargetState(LaraStateId::Stop);
                setAnimIdGlobal(loader::AnimationId::CLIMB_3CLICK, 614);
                m_lara.moveY(3 * loader::QuarterSectorSize + climbHeight);
                setHandStatus(1);
            }
            else if( climbHeight >= -core::JumpReachableHeight && climbHeight <= -core::ClimbLimit3ClickMax )
            {
                setTargetState(LaraStateId::JumpUp);
                setAnimIdGlobal(loader::AnimationId::STAY_SOLID, 185);
                setFallSpeedOverride(-static_cast<int>(std::sqrt(-12 * (climbHeight + 800)) + 3));
                getLara().updateImpl();
            }
            else
            {
                return false;
            }

            setYRotation(*alignedRotation);
            applyShift(collisionInfo);

            return true;
        }


        void AbstractStateHandler::applyShift(const CollisionInfo& collisionInfo)
        {
            setPosition(getPosition() + collisionInfo.shift);
            collisionInfo.shift = {0, 0, 0};
        }


        bool AbstractStateHandler::checkWallCollision(CollisionInfo& collisionInfo)
        {
            if( collisionInfo.collisionType == CollisionInfo::AxisColl_FrontForwardBlocked
                || collisionInfo.collisionType == CollisionInfo::AxisColl_InsufficientFrontCeilingSpace )
            {
                applyShift(collisionInfo);
                setTargetState(LaraStateId::Stop);
                setFalling(false);
                setHorizontalSpeed(0);
                return true;
            }

            if( collisionInfo.collisionType == CollisionInfo::AxisColl_FrontLeftBlocked )
            {
                applyShift(collisionInfo);
                m_lara.addYRotation(5_deg);
            }
            else if( collisionInfo.collisionType == CollisionInfo::AxisColl_FrontRightBlocked )
            {
                applyShift(collisionInfo);
                m_lara.addYRotation(-5_deg);
            }

            return false;
        }


        bool AbstractStateHandler::tryStartSlide(const CollisionInfo& collisionInfo)
        {
            const auto slantX = std::labs(collisionInfo.floorSlantX);
            const auto slantZ = std::labs(collisionInfo.floorSlantZ);
            if( slantX <= 2 && slantZ <= 2 )
                return false;

            core::Angle targetAngle{0_deg};
            if( collisionInfo.floorSlantX < -2 )
                targetAngle = 90_deg;
            else if( collisionInfo.floorSlantX > 2 )
                targetAngle = -90_deg;

            if( collisionInfo.floorSlantZ > std::max(2L, slantX) )
                targetAngle = 180_deg;
            else if( collisionInfo.floorSlantZ < std::min(-2L, -slantX) )
                targetAngle = 0_deg;

            core::Angle dy = abs(targetAngle - getRotation().Y);
            applyShift(collisionInfo);
            if( dy > 90_deg || dy < -90_deg )
            {
                if( getCurrentAnimState() != LaraStateId::SlideBackward || targetAngle != getCurrentSlideAngle() )
                {
                    setAnimIdGlobal(loader::AnimationId::START_SLIDE_BACKWARD, 1677);
                    setTargetState(LaraStateId::SlideBackward);
                    setMovementAngle(targetAngle);
                    setCurrentSlideAngle(targetAngle);
                    setYRotation(targetAngle - 180_deg);
                }
            }
            else if( getCurrentAnimState() != LaraStateId::SlideForward || targetAngle != getCurrentSlideAngle() )
            {
                setAnimIdGlobal(loader::AnimationId::SLIDE_FORWARD, 1133);
                setTargetState(LaraStateId::SlideForward);
                setMovementAngle(targetAngle);
                setCurrentSlideAngle(targetAngle);
                setYRotation(targetAngle);
            }
            return true;
        }


        bool AbstractStateHandler::tryGrabEdge(CollisionInfo& collisionInfo)
        {
            if( collisionInfo.collisionType != CollisionInfo::AxisColl_FrontForwardBlocked
                || !getLevel().m_inputHandler->getInputState().action || getHandStatus() != 0 )
                return false;

            const auto floorGradient = std::labs(
                collisionInfo.frontLeft.floor.distance - collisionInfo.frontRight.floor.distance);
            if( floorGradient >= core::MaxGrabbableGradient )
                return false;

            if( collisionInfo.front.ceiling.distance > 0
                || collisionInfo.mid.ceiling.distance > -core::ClimbLimit2ClickMin )
                return false;

            auto bbox = getBoundingBox();
            int spaceToReach = collisionInfo.front.floor.distance - bbox.minY;

            if( spaceToReach < 0 && getFallSpeed() + spaceToReach < 0 )
                return false;
            if( spaceToReach > 0 && getFallSpeed() + spaceToReach > 0 )
                return false;

            auto alignedRotation = alignRotation(getRotation().Y, 35_deg);
            if( !alignedRotation )
                return false;

            setTargetState(LaraStateId::Hang);
            setAnimIdGlobal(loader::AnimationId::HANG_IDLE, 1505);
            bbox = getBoundingBox();
            spaceToReach = collisionInfo.front.floor.distance - bbox.minY;

            setPosition(getPosition() + core::TRCoordinates(0, spaceToReach, 0));
            applyShift(collisionInfo);
            setHorizontalSpeed(0);
            setFallSpeed(0);
            setFalling(false);
            setHandStatus(1);
            setYRotation(*alignedRotation);

            return true;
        }


        int AbstractStateHandler::getRelativeHeightAtDirection(core::Angle angle, int dist) const
        {
            auto pos = getPosition();
            pos.X += angle.sin() * dist;
            pos.Y -= core::ScalpHeight;
            pos.Z += angle.cos() * dist;

            gsl::not_null<const loader::Sector*> sector = getLevel().findRealFloorSector(pos, m_lara.getCurrentRoom());

            HeightInfo h = HeightInfo::fromFloor(sector, pos, getLevel().m_cameraController);

            if( h.distance != -loader::HeightLimit )
                h.distance -= getPosition().Y;

            return h.distance;
        }


        void AbstractStateHandler::commonJumpHandling(CollisionInfo& collisionInfo)
        {
            collisionInfo.passableFloorDistanceBottom = loader::HeightLimit;
            collisionInfo.passableFloorDistanceTop = -core::ClimbLimit2ClickMin;
            collisionInfo.neededCeilingDistance = 192;
            collisionInfo.facingAngle = getMovementAngle();
            collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
            checkJumpWallSmash(collisionInfo);
            if( getFallSpeed() <= 0 || collisionInfo.mid.floor.distance > 0 )
                return;

            if( applyLandingDamage() )
                setTargetState(LaraStateId::Death);
            else
                setTargetState(LaraStateId::Stop);
            setFallSpeed(0);
            placeOnFloor(collisionInfo);
            setFalling(false);
        }


        void AbstractStateHandler::commonSlideHandling(CollisionInfo& collisionInfo)
        {
            collisionInfo.passableFloorDistanceBottom = loader::HeightLimit;
            collisionInfo.passableFloorDistanceTop = -loader::QuarterSectorSize * 2;
            collisionInfo.neededCeilingDistance = 0;
            collisionInfo.facingAngle = getMovementAngle();
            collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);

            if( stopIfCeilingBlocked(collisionInfo) )
                return;

            checkWallCollision(collisionInfo);
            if( collisionInfo.mid.floor.distance <= 200 )
            {
                tryStartSlide(collisionInfo);
                placeOnFloor(collisionInfo);
                if(std::labs(collisionInfo.floorSlantX) <= 2 && std::labs(collisionInfo.floorSlantZ) <= 2 )
                {
                    setTargetState(LaraStateId::Stop);
                }
                return;
            }

            if( getCurrentAnimState() == LaraStateId::SlideForward )
            {
                setAnimIdGlobal(loader::AnimationId::FREE_FALL_FORWARD, 492);
                setTargetState(LaraStateId::JumpForward);
            }
            else
            {
                setAnimIdGlobal(loader::AnimationId::FREE_FALL_BACK, 1473);
                setTargetState(LaraStateId::FallBackward);
            }

            setFallSpeed(0);
            setFalling(true);
        }


        void AbstractStateHandler::commonEdgeHangHandling(CollisionInfo& collisionInfo)
        {
            collisionInfo.passableFloorDistanceBottom = loader::HeightLimit;
            collisionInfo.passableFloorDistanceTop = -loader::HeightLimit;
            collisionInfo.neededCeilingDistance = 0;
            collisionInfo.facingAngle = getMovementAngle();
            collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
            const bool tooSteepToGrab = collisionInfo.front.floor.distance < 200;
            setFallSpeed(0);
            setFalling(false);
            setMovementAngle(getRotation().Y);
            const auto axis = *axisFromAngle(getMovementAngle(), 45_deg);
            switch( axis )
            {
                case core::Axis::PosZ: setPosition(getPosition() + core::TRCoordinates(0, 0, 2));
                    break;
                case core::Axis::PosX: setPosition(getPosition() + core::TRCoordinates(2, 0, 0));
                    break;
                case core::Axis::NegZ: setPosition(getPosition() - core::TRCoordinates(0, 0, 2));
                    break;
                case core::Axis::NegX: setPosition(getPosition() - core::TRCoordinates(2, 0, 0));
                    break;
            }

            collisionInfo.passableFloorDistanceBottom = loader::HeightLimit;
            collisionInfo.passableFloorDistanceTop = -core::ClimbLimit2ClickMin;
            collisionInfo.neededCeilingDistance = 0;
            collisionInfo.facingAngle = getMovementAngle();
            collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
            if( !getLevel().m_inputHandler->getInputState().action || getHealth() <= 0 )
            {
                setAnimIdGlobal(loader::AnimationId::TRY_HANG_VERTICAL, 448);
                setTargetState(LaraStateId::JumpUp);
                setHandStatus(0);
                const auto bbox = getBoundingBox();
                const auto hangDistance = collisionInfo.front.floor.distance - bbox.minY + 2;
                setPosition(getPosition() + core::TRCoordinates(collisionInfo.shift.X, hangDistance,
                                                                collisionInfo.shift.Z));
                setHorizontalSpeed(2);
                setFallSpeed(1);
                setFalling(true);
                return;
            }

            auto gradient = std::labs(
                collisionInfo.frontLeft.floor.distance - collisionInfo.frontRight.floor.distance);
            if( gradient >= core::MaxGrabbableGradient || collisionInfo.mid.ceiling.distance >= 0
                || collisionInfo.collisionType != CollisionInfo::AxisColl_FrontForwardBlocked || tooSteepToGrab )
            {
                setPosition(collisionInfo.oldPosition);
                if( getCurrentAnimState() != LaraStateId::ShimmyLeft
                    && getCurrentAnimState() != LaraStateId::ShimmyRight )
                {
                    return;
                }

                setAnimIdGlobal(loader::AnimationId::HANG_IDLE, 1514);
                setTargetState(LaraStateId::Hang);
                return;
            }

            switch( axis )
            {
                case core::Axis::PosZ:
                case core::Axis::NegZ:
                    setPosition(getPosition() + core::TRCoordinates(0, 0, collisionInfo.shift.Z));
                    break;
                case core::Axis::PosX:
                case core::Axis::NegX:
                    setPosition(getPosition() + core::TRCoordinates(collisionInfo.shift.X, 0, 0));
                    break;
            }

            const auto bbox = getBoundingBox();
            const int spaceToReach = collisionInfo.front.floor.distance - bbox.minY;

            if( spaceToReach >= -loader::QuarterSectorSize && spaceToReach <= loader::QuarterSectorSize )
                setPosition(getPosition() + core::TRCoordinates(0, spaceToReach, 0));
        }


        bool AbstractStateHandler::applyLandingDamage()
        {
            auto sector = getLevel()
                .findRealFloorSector(getPosition(), m_lara.getCurrentRoom());
            HeightInfo h = HeightInfo::fromFloor(sector, getPosition()
                                                         - core::TRCoordinates{0, core::ScalpHeight, 0},
                                                 getLevel().m_cameraController);
            setFloorHeight(h.distance);
            getLara().handleCommandSequence(h.lastCommandSequenceOrDeath, false);
            auto damageSpeed = getFallSpeed() - core::DamageFallSpeedThreshold;
            if( damageSpeed <= 0 )
                return false;

            static constexpr int DeathSpeedLimit = core::DeadlyFallSpeedThreshold - core::DamageFallSpeedThreshold;

            if( damageSpeed <= DeathSpeedLimit )
                setHealth(getHealth() - core::LaraHealth * util::square(damageSpeed) / util::square(DeathSpeedLimit));
            else
                setHealth(-1);
            return getHealth() <= 0;
        }


        BoundingBox AbstractStateHandler::getBoundingBox() const
        {
            return m_lara.getBoundingBox();
        }


        void AbstractStateHandler::addSwimToDiveKeypressDuration(int n) noexcept
        {
            m_lara.addSwimToDiveKeypressDuration(n);
        }


        void AbstractStateHandler::setSwimToDiveKeypressDuration(int n) noexcept
        {
            m_lara.setSwimToDiveKeypressDuration(n);
        }


        int AbstractStateHandler::getSwimToDiveKeypressDuration() const
        {
            return m_lara.getSwimToDiveKeypressDuration();
        }


        void AbstractStateHandler::setUnderwaterState(UnderwaterState u) noexcept
        {
            m_lara.setUnderwaterState(u);
        }


        void AbstractStateHandler::setCameraRotation(core::Angle x, core::Angle y)
        {
            m_lara.setCameraRotation(x, y);
        }


        void AbstractStateHandler::setCameraRotationX(core::Angle x)
        {
            m_lara.setCameraRotationX(x);
        }


        void AbstractStateHandler::setCameraRotationY(core::Angle y)
        {
            m_lara.setCameraRotationY(y);
        }


        void AbstractStateHandler::setCameraDistance(int d)
        {
            m_lara.setCameraDistance(d);
        }


        void AbstractStateHandler::setCameraUnknown1(CamOverrideType k)
        {
            m_lara.setCameraUnknown1(k);
        }


        void AbstractStateHandler::jumpAgainstWall(CollisionInfo& collisionInfo)
        {
            applyShift(collisionInfo);
            if( collisionInfo.collisionType == CollisionInfo::AxisColl_FrontLeftBlocked )
                m_lara.addYRotation(5_deg);
            else if( collisionInfo.collisionType == CollisionInfo::AxisColl_FrontRightBlocked )
                m_lara.addYRotation(-5_deg);
            else if( collisionInfo.collisionType == CollisionInfo::AxisColl_ScalpCollision )
            {
                if( getFallSpeed() <= 0 )
                    setFallSpeed(1);
            }
            else if( collisionInfo.collisionType == CollisionInfo::AxisColl_InvalidPosition )
            {
                m_lara.moveX(100 * getRotation().Y.sin());
                m_lara.moveZ(100 * getRotation().Y.cos());
                setHorizontalSpeed(0);
                collisionInfo.mid.floor.distance = 0;
                if( getFallSpeed() < 0 )
                    setFallSpeed(16);
            }
        }


        void AbstractStateHandler::checkJumpWallSmash(CollisionInfo& collisionInfo)
        {
            applyShift(collisionInfo);

            if( collisionInfo.collisionType == CollisionInfo::AxisColl_None )
                return;

            if( collisionInfo.collisionType == CollisionInfo::AxisColl_FrontForwardBlocked
                || collisionInfo.collisionType == CollisionInfo::AxisColl_InsufficientFrontCeilingSpace )
            {
                setTargetState(LaraStateId::FreeFall);
                //! @todo Check formula
                setHorizontalSpeed(getHorizontalSpeed() * 0.2f);
                setMovementAngle(getMovementAngle() - 180_deg);
                setAnimIdGlobal(loader::AnimationId::SMASH_JUMP, 481);
                if( getFallSpeed() <= 0 )
                    setFallSpeed(1);
                return;
            }

            if( collisionInfo.collisionType == CollisionInfo::AxisColl_FrontLeftBlocked )
            {
                m_lara.addYRotation(5_deg);
                return;
            }
            if( collisionInfo.collisionType == CollisionInfo::AxisColl_FrontRightBlocked )
            {
                m_lara.addYRotation(-5_deg);
                return;
            }

            if( collisionInfo.collisionType == CollisionInfo::AxisColl_InvalidPosition )
            {
                m_lara.moveX(100 * collisionInfo.facingAngle.sin());
                m_lara.moveZ(100 * collisionInfo.facingAngle.cos());
                setHorizontalSpeed(0);
                collisionInfo.mid.floor.distance = 0;
                if( getFallSpeed() <= 0 )
                    setFallSpeed(16);
            }

            if( collisionInfo.collisionType == CollisionInfo::AxisColl_ScalpCollision && getFallSpeed() <= 0 )
                setFallSpeed(1);
        }


        void AbstractStateHandler::laraUpdateImpl()
        {
            m_lara.updateImpl();
        }
    }
}
