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


namespace engine
{
    namespace lara
    {
        void AbstractStateHandler::animate(CollisionInfo& collisionInfo, const std::chrono::microseconds& deltaTimeMs)
        {
            animateImpl(collisionInfo, deltaTimeMs);

            m_lara.rotate(
                m_xRotationSpeed.getScaled(deltaTimeMs),
                m_yRotationSpeed.getScaled(deltaTimeMs),
                m_zRotationSpeed.getScaled(deltaTimeMs)
            );
            m_lara.move(
                m_xMovement.getScaled(deltaTimeMs),
                m_yMovement.getScaled(deltaTimeMs),
                m_zMovement.getScaled(deltaTimeMs)
            );
        }


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


        const core::InterpolatedValue<float>& AbstractStateHandler::getHealth() const noexcept
        {
            return m_lara.getHealth();
        }


        void AbstractStateHandler::setHealth(const core::InterpolatedValue<float>& h) noexcept
        {
            m_lara.setHealth(h);
        }


        void AbstractStateHandler::setAir(const core::InterpolatedValue<float>& a) noexcept
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


        void AbstractStateHandler::setFallSpeed(const core::InterpolatedValue<float>& spd)
        {
            m_lara.setFallSpeed(spd);
        }


        const core::InterpolatedValue<float>& AbstractStateHandler::getFallSpeed() const noexcept
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


        std::chrono::microseconds AbstractStateHandler::getCurrentTime() const
        {
            return m_lara.getCurrentTime();
        }


        loader::LaraStateId AbstractStateHandler::getCurrentAnimState() const
        {
            return m_lara.getCurrentAnimState();
        }


        void AbstractStateHandler::setAnimIdGlobal(loader::AnimationId anim, const boost::optional<uint16_t>& firstFrame)
        {
            m_lara.setAnimIdGlobal(anim, firstFrame);
        }


        const core::TRRotation& AbstractStateHandler::getRotation() const noexcept
        {
            return m_lara.getRotation();
        }


        void AbstractStateHandler::setHorizontalSpeed(const core::InterpolatedValue<float>& speed)
        {
            m_lara.setHorizontalSpeed(speed);
        }


        const core::InterpolatedValue<float>& AbstractStateHandler::getHorizontalSpeed() const
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


        const core::ExactTRCoordinates& AbstractStateHandler::getPosition() const
        {
            return m_lara.getPosition();
        }


        void AbstractStateHandler::setPosition(const core::ExactTRCoordinates& pos)
        {
            m_lara.setPosition(pos);
        }


        long AbstractStateHandler::getFloorHeight() const
        {
            return m_lara.getFloorHeight();
        }


        void AbstractStateHandler::setFloorHeight(long h)
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


        void AbstractStateHandler::subYRotationSpeed(const std::chrono::microseconds& deltaTime, core::Angle val,
                                                     core::Angle limit)
        {
            m_lara.subYRotationSpeed(deltaTime, val, limit);
        }


        void AbstractStateHandler::addYRotationSpeed(const std::chrono::microseconds& deltaTime, core::Angle val,
                                                     core::Angle limit)
        {
            m_lara.addYRotationSpeed(deltaTime, val, limit);
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


        void AbstractStateHandler::dampenHorizontalSpeed(const std::chrono::microseconds& deltaTime, float f)
        {
            m_lara.dampenHorizontalSpeed(deltaTime, f);
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
                .findRealFloorSector(pos.toInexact(), m_lara.getCurrentRoom());
            HeightInfo floor = HeightInfo::fromFloor(sector, pos.toInexact(), getLevel().m_cameraController);
            HeightInfo ceil = HeightInfo::fromCeiling(sector, pos.toInexact(), getLevel().m_cameraController);
            return floor.distance != -loader::HeightLimit && floor.distance - pos.Y > 0 && ceil.distance - pos.Y < -400;
        }


        bool AbstractStateHandler::tryReach(CollisionInfo& collisionInfo)
        {
            if( collisionInfo.axisCollisions != CollisionInfo::AxisColl_FrontForwardBlocked
                || !getLevel().m_inputHandler->getInputState().action || getHandStatus() != 0 )
                return false;

            if( std::abs(collisionInfo.frontLeft.floor.distance - collisionInfo.frontRight.floor.distance)
                >= core::MaxGrabbableGradient )
                return false;

            if( collisionInfo.front.ceiling.distance > 0
                || collisionInfo.current.ceiling.distance > -core::ClimbLimit2ClickMin
                || collisionInfo.current.floor.distance < 200 )
                return false;

            const auto bbox = getBoundingBox();
            long spaceToReach = collisionInfo.front.floor.distance - bbox.min.y;

            BOOST_LOG_TRIVIAL(debug) << "spaceToReach = " << spaceToReach << ", getFallSpeed() + spaceToReach = " << (getFallSpeed() + spaceToReach).getCurrentValue();

            if( spaceToReach < 0 && getFallSpeed() + spaceToReach < 0 )
                return false;
            if( spaceToReach > 0 && getFallSpeed() + spaceToReach > 0 )
                return false;

            auto alignedRotation = core::alignRotation(getRotation().Y, 35_deg);
            if( !alignedRotation )
                return false;

            if( canClimbOnto(*core::axisFromAngle(getRotation().Y, 35_deg)) )
                setAnimIdGlobal(loader::AnimationId::OSCILLATE_HANG_ON, 3974);
            else
                setAnimIdGlobal(loader::AnimationId::HANG_IDLE, 1493);

            setTargetState(LaraStateId::Hang);
            setPosition(getPosition() + core::ExactTRCoordinates(collisionInfo.collisionFeedback.X, spaceToReach,
                                                                 collisionInfo.collisionFeedback.Z));
            setHorizontalSpeed(core::makeInterpolatedValue(0.0f));
            setYRotation(*alignedRotation);
            setFalling(false);
            setFallSpeed(core::makeInterpolatedValue(0.0f));
            setHandStatus(1);
            return true;
        }


        bool AbstractStateHandler::stopIfCeilingBlocked(const CollisionInfo& collisionInfo)
        {
            if( collisionInfo.axisCollisions != CollisionInfo::AxisColl_ScalpCollision
                && collisionInfo.axisCollisions != CollisionInfo::AxisColl_InvalidPosition )
                return false;

            setPosition(collisionInfo.oldPosition);

            setTargetState(LaraStateId::Stop);
            setAnimIdGlobal(loader::AnimationId::STAY_SOLID, 185);
            setHorizontalSpeed(core::makeInterpolatedValue(0.0f));
            setFallSpeed(core::makeInterpolatedValue(0.0f));
            setFalling(false);
            return true;
        }


        bool AbstractStateHandler::tryClimb(CollisionInfo& collisionInfo)
        {
            if( collisionInfo.axisCollisions != CollisionInfo::AxisColl_FrontForwardBlocked
                || !getLevel().m_inputHandler->getInputState().action || getHandStatus() != 0 )
                return false;

            const auto floorGradient = std::abs(
                collisionInfo.frontLeft.floor.distance - collisionInfo.frontRight.floor.distance);
            if( floorGradient >= core::MaxGrabbableGradient )
                return false;

            //! @todo MAGICK +/- 30 degrees
            auto alignedRotation = core::alignRotation(getRotation().Y, 30_deg);
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
                m_yMovement = 2.0f * loader::QuarterSectorSize + climbHeight;
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
                m_yMovement = 3.0f * loader::QuarterSectorSize + climbHeight;
                setHandStatus(1);
            }
            else if( climbHeight >= -core::JumpReachableHeight && climbHeight <= -core::ClimbLimit3ClickMax )
            {
                setTargetState(LaraStateId::JumpUp);
                setAnimIdGlobal(loader::AnimationId::STAY_SOLID, 185);
                setFallSpeedOverride(-static_cast<int>(std::sqrt(-12 * (climbHeight + 800) + 3)));
            }
            else
            {
                return false;
            }

            setYRotation(*alignedRotation);
            applyCollisionFeedback(collisionInfo);

            return true;
        }


        void AbstractStateHandler::applyCollisionFeedback(const CollisionInfo& collisionInfo)
        {
            static constexpr float Margin = 1;

            auto shrinkMargin = [](float& x)
            {
                if(x < 0)
                    x += Margin;
                else if(x > 0)
                    x -= Margin;
            };

            shrinkMargin(collisionInfo.collisionFeedback.X);
            shrinkMargin(collisionInfo.collisionFeedback.Y);
            shrinkMargin(collisionInfo.collisionFeedback.Z);

            setPosition(getPosition() + collisionInfo.collisionFeedback);
            collisionInfo.collisionFeedback = {0, 0, 0};
        }


        bool AbstractStateHandler::checkWallCollision(CollisionInfo& collisionInfo)
        {
            if( collisionInfo.axisCollisions == CollisionInfo::AxisColl_FrontForwardBlocked
                || collisionInfo.axisCollisions == CollisionInfo::AxisColl_InsufficientFrontCeilingSpace )
            {
                applyCollisionFeedback(collisionInfo);
                setTargetState(LaraStateId::Stop);
                setFalling(false);
                setHorizontalSpeed(core::makeInterpolatedValue(0.0f));
                return true;
            }

            if( collisionInfo.axisCollisions == CollisionInfo::AxisColl_FrontLeftBlocked )
            {
                applyCollisionFeedback(collisionInfo);
                m_yRotationSpeed = 5_deg;
            }
            else if( collisionInfo.axisCollisions == CollisionInfo::AxisColl_FrontRightBlocked )
            {
                applyCollisionFeedback(collisionInfo);
                m_yRotationSpeed = -5_deg;
            }

            return false;
        }


        bool AbstractStateHandler::tryStartSlide(const CollisionInfo& collisionInfo)
        {
            int slantX = std::abs(collisionInfo.floorSlantX);
            int slantZ = std::abs(collisionInfo.floorSlantZ);
            if( slantX <= 2 && slantZ <= 2 )
                return false;

            core::Angle targetAngle{0_deg};
            if( collisionInfo.floorSlantX < -2 )
                targetAngle = 90_deg;
            else if( collisionInfo.floorSlantX > 2 )
                targetAngle = -90_deg;

            if( collisionInfo.floorSlantZ > std::max(2, slantX) )
                targetAngle = 180_deg;
            else if( collisionInfo.floorSlantZ < std::min(-2, -slantX) )
                targetAngle = 0_deg;

            core::Angle dy = abs(targetAngle - getRotation().Y);
            applyCollisionFeedback(collisionInfo);
            if( dy > 90_deg || dy < -90_deg )
            {
                if( getCurrentAnimState() != LaraStateId::SlideBackward || targetAngle != getCurrentSlideAngle() )
                {
                    setAnimIdGlobal(loader::AnimationId::START_SLIDE_BACKWARD, 1677);
                    setTargetState(LaraStateId::SlideBackward);
                    setMovementAngle(targetAngle);
                    setCurrentSlideAngle(targetAngle);
                    setYRotation(targetAngle + 180_deg);
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
            if( collisionInfo.axisCollisions != CollisionInfo::AxisColl_FrontForwardBlocked
                || !getLevel().m_inputHandler->getInputState().action || getHandStatus() != 0 )
                return false;

            const auto floorGradient = std::abs(
                collisionInfo.frontLeft.floor.distance - collisionInfo.frontRight.floor.distance);
            if( floorGradient >= core::MaxGrabbableGradient )
                return false;

            if( collisionInfo.front.ceiling.distance > 0
                || collisionInfo.current.ceiling.distance > -core::ClimbLimit2ClickMin )
                return false;

            auto bbox = getBoundingBox();
            long spaceToReach = collisionInfo.front.floor.distance - bbox.min.y;

            if( spaceToReach < 0 && getFallSpeed() + spaceToReach < 0 )
                return false;
            if( spaceToReach > 0 && getFallSpeed() + spaceToReach > 0 )
                return false;

            auto alignedRotation = core::alignRotation(getRotation().Y, 35_deg);
            if( !alignedRotation )
                return false;

            setTargetState(LaraStateId::Hang);
            setAnimIdGlobal(loader::AnimationId::HANG_IDLE, 1505);
            bbox = getBoundingBox();
            spaceToReach = collisionInfo.front.floor.distance - bbox.min.y;

            setPosition(getPosition() + core::ExactTRCoordinates(0, spaceToReach, 0));
            applyCollisionFeedback(collisionInfo);
            setHorizontalSpeed(core::makeInterpolatedValue(0.0f));
            setFallSpeed(core::makeInterpolatedValue(0.0f));
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

            gsl::not_null<const loader::Sector*> sector = getLevel()
                .findRealFloorSector(pos.toInexact(), m_lara.getCurrentRoom());

            HeightInfo h = HeightInfo::fromFloor(sector, pos.toInexact(), getLevel().m_cameraController);

            if( h.distance != -loader::HeightLimit )
                h.distance -= std::lround(getPosition().Y);

            return h.distance;
        }


        void AbstractStateHandler::commonJumpHandling(CollisionInfo& collisionInfo)
        {
            collisionInfo.passableFloorDistanceBottom = loader::HeightLimit;
            collisionInfo.passableFloorDistanceTop = -core::ClimbLimit2ClickMin;
            collisionInfo.neededCeilingDistance = 192;
            collisionInfo.yAngle = getMovementAngle();
            collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
            checkJumpWallSmash(collisionInfo);
            if( getFallSpeed() < 0 || collisionInfo.current.floor.distance > 0 )
                return;

            if( applyLandingDamage() )
                setTargetState(LaraStateId::Death);
            else
                setTargetState(LaraStateId::Stop);
            setFallSpeed(core::makeInterpolatedValue(0.0f));
            placeOnFloor(collisionInfo);
            setFalling(false);
        }


        void AbstractStateHandler::commonSlideHandling(CollisionInfo& collisionInfo)
        {
            collisionInfo.passableFloorDistanceBottom = loader::HeightLimit;
            collisionInfo.passableFloorDistanceTop = -loader::QuarterSectorSize * 2;
            collisionInfo.neededCeilingDistance = 0;
            collisionInfo.yAngle = getMovementAngle();
            collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);

            if( stopIfCeilingBlocked(collisionInfo) )
                return;

            checkWallCollision(collisionInfo);
            if( collisionInfo.current.floor.distance <= 200 )
            {
                tryStartSlide(collisionInfo);
                placeOnFloor(collisionInfo);
                const auto absSlantX = std::abs(collisionInfo.floorSlantX);
                const auto absSlantZ = std::abs(collisionInfo.floorSlantZ);
                if( absSlantX <= 2 && absSlantZ <= 2 )
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

            setFallSpeed(core::makeInterpolatedValue(0.0f));
            setFalling(true);
        }


        void AbstractStateHandler::commonEdgeHangHandling(CollisionInfo& collisionInfo)
        {
            collisionInfo.passableFloorDistanceBottom = loader::HeightLimit;
            collisionInfo.passableFloorDistanceTop = -loader::HeightLimit;
            collisionInfo.neededCeilingDistance = 0;
            collisionInfo.yAngle = getMovementAngle();
            collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
            const bool tooSteepToGrab = collisionInfo.front.floor.distance < 200;
            setFallSpeed(core::makeInterpolatedValue(0.0f));
            setFalling(false);
            setMovementAngle(getRotation().Y);
            const auto axis = *core::axisFromAngle(getMovementAngle(), 45_deg);
            switch( axis )
            {
                case core::Axis::PosZ: setPosition(getPosition() + core::ExactTRCoordinates(0, 0, 2));
                    break;
                case core::Axis::PosX: setPosition(getPosition() + core::ExactTRCoordinates(2, 0, 0));
                    break;
                case core::Axis::NegZ: setPosition(getPosition() - core::ExactTRCoordinates(0, 0, 2));
                    break;
                case core::Axis::NegX: setPosition(getPosition() - core::ExactTRCoordinates(2, 0, 0));
                    break;
            }

            collisionInfo.passableFloorDistanceBottom = loader::HeightLimit;
            collisionInfo.passableFloorDistanceTop = -core::ClimbLimit2ClickMin;
            collisionInfo.neededCeilingDistance = 0;
            collisionInfo.yAngle = getMovementAngle();
            collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
            if( !getLevel().m_inputHandler->getInputState().action || getHealth() <= 0 )
            {
                setTargetState(LaraStateId::JumpUp);
                setAnimIdGlobal(loader::AnimationId::TRY_HANG_VERTICAL, 448);
                setHandStatus(0);
                const auto bbox = getBoundingBox();
                const long hangDistance = collisionInfo.front.floor.distance - bbox.min.y + 2;
                setPosition(getPosition() + core::ExactTRCoordinates(collisionInfo.collisionFeedback.X, hangDistance,
                                                                     collisionInfo.collisionFeedback.Z));
                setHorizontalSpeed(core::makeInterpolatedValue(2.0f));
                setFallSpeed(core::makeInterpolatedValue(1.0f));
                setFalling(true);
                return;
            }

            auto gradient = std::abs(
                collisionInfo.frontLeft.floor.distance - collisionInfo.frontRight.floor.distance);
            if( gradient >= core::MaxGrabbableGradient || collisionInfo.current.ceiling.distance >= 0
                || collisionInfo.axisCollisions != CollisionInfo::AxisColl_FrontForwardBlocked || tooSteepToGrab )
            {
                setPosition(collisionInfo.oldPosition);
                if( getCurrentAnimState() != LaraStateId::ShimmyLeft
                    && getCurrentAnimState() != LaraStateId::ShimmyRight )
                {
                    return;
                }

                setTargetState(LaraStateId::Hang);
                setAnimIdGlobal(loader::AnimationId::HANG_IDLE, 1514);
                return;
            }

            switch( axis )
            {
                case core::Axis::PosZ:
                case core::Axis::NegZ:
                    setPosition(getPosition() + core::ExactTRCoordinates(0, 0, collisionInfo.collisionFeedback.Z));
                    break;
                case core::Axis::PosX:
                case core::Axis::NegX:
                    setPosition(getPosition() + core::ExactTRCoordinates(collisionInfo.collisionFeedback.X, 0, 0));
                    break;
            }

            const auto bbox = getBoundingBox();
            const long spaceToReach = collisionInfo.front.floor.distance - bbox.min.y;

            if( spaceToReach >= -loader::QuarterSectorSize && spaceToReach <= loader::QuarterSectorSize )
                setPosition(getPosition() + core::ExactTRCoordinates(0, spaceToReach, 0));
        }


        bool AbstractStateHandler::applyLandingDamage()
        {
            auto sector = getLevel()
                .findRealFloorSector(getPosition().toInexact(), m_lara.getCurrentRoom());
            HeightInfo h = HeightInfo::fromFloor(sector, getPosition().toInexact()
                                                 - core::TRCoordinates{0, core::ScalpHeight, 0},
                                                 getLevel().m_cameraController);
            setFloorHeight(h.distance);
            getLara().handleCommandSequence(h.lastCommandSequenceOrDeath, false);
            auto damageSpeed = static_cast<float>(getFallSpeed()) - 140;
            if( damageSpeed <= 0 )
                return false;

            static constexpr int DeathSpeedLimit = 14;

            if( damageSpeed <= DeathSpeedLimit )
                setHealth(getHealth() - 1000 * damageSpeed * damageSpeed / (DeathSpeedLimit * DeathSpeedLimit));
            else
                setHealth(core::makeInterpolatedValue(-1.0f));
            return getHealth() <= 0;
        }


        gameplay::BoundingBox AbstractStateHandler::getBoundingBox() const
        {
            return m_lara.getBoundingBox();
        }


        void AbstractStateHandler::addSwimToDiveKeypressDuration(const std::chrono::microseconds& ms) noexcept
        {
            m_lara.addSwimToDiveKeypressDuration(ms);
        }


        void AbstractStateHandler::setSwimToDiveKeypressDuration(const std::chrono::microseconds& ms) noexcept
        {
            m_lara.setSwimToDiveKeypressDuration(ms);
        }


        const boost::optional<std::chrono::microseconds>& AbstractStateHandler::getSwimToDiveKeypressDuration() const
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
            applyCollisionFeedback(collisionInfo);
            if( collisionInfo.axisCollisions == CollisionInfo::AxisColl_FrontLeftBlocked )
                m_yRotationSpeed = 5_deg;
            else if( collisionInfo.axisCollisions == CollisionInfo::AxisColl_FrontRightBlocked )
                m_yRotationSpeed = -5_deg;
            else if( collisionInfo.axisCollisions == CollisionInfo::AxisColl_ScalpCollision )
            {
                if( getFallSpeed() <= 0 )
                    setFallSpeed(core::makeInterpolatedValue(1.0f));
            }
            else if( collisionInfo.axisCollisions == CollisionInfo::AxisColl_InvalidPosition )
            {
                m_xMovement = 100 * getRotation().Y.sin();
                m_zMovement = 100 * getRotation().Y.cos();
                setHorizontalSpeed(core::makeInterpolatedValue(0.0f));
                collisionInfo.current.floor.distance = 0;
                if( getFallSpeed() < 0 )
                    setFallSpeed(core::makeInterpolatedValue(16.0f));
            }
        }


        void AbstractStateHandler::checkJumpWallSmash(CollisionInfo& collisionInfo)
        {
            applyCollisionFeedback(collisionInfo);

            if( collisionInfo.axisCollisions == CollisionInfo::AxisColl_None )
                return;

            if( collisionInfo.axisCollisions == CollisionInfo::AxisColl_FrontForwardBlocked
                || collisionInfo.axisCollisions == CollisionInfo::AxisColl_InsufficientFrontCeilingSpace )
            {
                setTargetState(LaraStateId::FreeFall);
                //! @todo Check formula
                setHorizontalSpeed(getHorizontalSpeed() * 0.2f);
                setMovementAngle(getMovementAngle() - 180_deg);
                setAnimIdGlobal(loader::AnimationId::SMASH_JUMP, 481);
                if( getFallSpeed() <= 0 )
                    setFallSpeed(core::makeInterpolatedValue(1.0f));
                return;
            }

            if( collisionInfo.axisCollisions == CollisionInfo::AxisColl_FrontLeftBlocked )
            {
                m_yRotationSpeed = 5_deg;
                return;
            }
            else if( collisionInfo.axisCollisions == CollisionInfo::AxisColl_FrontRightBlocked )
            {
                m_yRotationSpeed = -5_deg;
                return;
            }

            if( collisionInfo.axisCollisions == CollisionInfo::AxisColl_InvalidPosition )
            {
                m_xMovement = 100 * collisionInfo.yAngle.sin();
                m_zMovement = 100 * collisionInfo.yAngle.cos();
                setHorizontalSpeed(core::makeInterpolatedValue(0.0f));
                collisionInfo.current.floor.distance = 0;
                if( getFallSpeed() <= 0 )
                    setFallSpeed(core::makeInterpolatedValue(16.0f));
            }

            if( collisionInfo.axisCollisions == CollisionInfo::AxisColl_ScalpCollision && getFallSpeed() <= 0 )
                setFallSpeed(core::makeInterpolatedValue(1.0f));
        }


        void AbstractStateHandler::laraUpdateImpl(const std::chrono::microseconds& deltaTime)
        {
            m_lara.updateImpl(deltaTime, true);
        }
    }
}
