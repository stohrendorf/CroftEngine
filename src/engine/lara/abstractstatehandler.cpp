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
            animateImpl( collisionInfo, deltaTimeMs );

            m_controller.rotate(
                    m_xRotationSpeed.getScaled( deltaTimeMs ),
                    m_yRotationSpeed.getScaled( deltaTimeMs ),
                    m_zRotationSpeed.getScaled( deltaTimeMs )
            );
            m_controller.move(
                    m_xMovement.getScaled( deltaTimeMs ),
                    m_yMovement.getScaled( deltaTimeMs ),
                    m_zMovement.getScaled( deltaTimeMs )
            );
        }


        std::unique_ptr<AbstractStateHandler> AbstractStateHandler::create(loader::LaraStateId id, LaraNode& controller)
        {
            switch( id )
            {
                case LaraStateId::WalkForward:return std::make_unique<StateHandler_0>( controller );
                case LaraStateId::RunForward:return std::make_unique<StateHandler_1>( controller );
                case LaraStateId::Stop:return std::make_unique<StateHandler_2>( controller );
                case LaraStateId::JumpForward:return std::make_unique<StateHandler_3>( controller );
                case LaraStateId::Pose:return std::make_unique<StateHandler_4>( controller );
                case LaraStateId::RunBack:return std::make_unique<StateHandler_5>( controller );
                case LaraStateId::TurnRightSlow:return std::make_unique<StateHandler_6>( controller );
                case LaraStateId::TurnLeftSlow:return std::make_unique<StateHandler_7>( controller );
                case LaraStateId::Death:return std::make_unique<StateHandler_8>( controller );
                case LaraStateId::FreeFall:return std::make_unique<StateHandler_9>( controller );
                case LaraStateId::Hang:return std::make_unique<StateHandler_10>( controller );
                case LaraStateId::Reach:return std::make_unique<StateHandler_11>( controller );
                case LaraStateId::Unknown12:return std::make_unique<StateHandler_12>( controller );
                case LaraStateId::UnderwaterStop:return std::make_unique<StateHandler_13>( controller );
                case LaraStateId::JumpPrepare:return std::make_unique<StateHandler_15>( controller );
                case LaraStateId::WalkBackward:return std::make_unique<StateHandler_16>( controller );
                case LaraStateId::UnderwaterForward:return std::make_unique<StateHandler_17>( controller );
                case LaraStateId::UnderwaterInertia:return std::make_unique<StateHandler_18>( controller );
                case LaraStateId::Climbing:return std::make_unique<StateHandler_19>( controller );
                case LaraStateId::TurnFast:return std::make_unique<StateHandler_20>( controller );
                case LaraStateId::StepRight:return std::make_unique<StateHandler_21>( controller );
                case LaraStateId::StepLeft:return std::make_unique<StateHandler_22>( controller );
                case LaraStateId::RollBackward:return std::make_unique<StateHandler_23>( controller );
                case LaraStateId::SlideForward:return std::make_unique<StateHandler_24>( controller );
                case LaraStateId::JumpBack:return std::make_unique<StateHandler_25>( controller );
                case LaraStateId::JumpLeft:return std::make_unique<StateHandler_26>( controller );
                case LaraStateId::JumpRight:return std::make_unique<StateHandler_27>( controller );
                case LaraStateId::JumpUp:return std::make_unique<StateHandler_28>( controller );
                case LaraStateId::FallBackward:return std::make_unique<StateHandler_29>( controller );
                case LaraStateId::ShimmyLeft:return std::make_unique<StateHandler_30>( controller );
                case LaraStateId::ShimmyRight:return std::make_unique<StateHandler_31>( controller );
                case LaraStateId::SlideBackward:return std::make_unique<StateHandler_32>( controller );
                case LaraStateId::OnWaterStop:return std::make_unique<StateHandler_33>( controller );
                case LaraStateId::OnWaterForward:return std::make_unique<StateHandler_34>( controller );
                case LaraStateId::UnderwaterDiving:return std::make_unique<StateHandler_35>( controller );
                case LaraStateId::PushablePush:return std::make_unique<StateHandler_36>( controller );
                case LaraStateId::PushablePull:return std::make_unique<StateHandler_37>( controller );
                case LaraStateId::PushableGrab:return std::make_unique<StateHandler_38>( controller );
                case LaraStateId::SwitchDown:return std::make_unique<StateHandler_40>( controller );
                case LaraStateId::SwitchUp:return std::make_unique<StateHandler_41>( controller );
                case LaraStateId::WaterDeath:return std::make_unique<StateHandler_44>( controller );
                case LaraStateId::RollForward:return std::make_unique<StateHandler_45>( controller );
                case LaraStateId::OnWaterBackward:return std::make_unique<StateHandler_47>( controller );
                case LaraStateId::OnWaterLeft:return std::make_unique<StateHandler_48>( controller );
                case LaraStateId::OnWaterRight:return std::make_unique<StateHandler_49>( controller );
                case LaraStateId::SwandiveBegin:return std::make_unique<StateHandler_52>( controller );
                case LaraStateId::SwandiveEnd:return std::make_unique<StateHandler_53>( controller );
                case LaraStateId::Handstand:return std::make_unique<StateHandler_54>( controller );
                case LaraStateId::OnWaterExit:return std::make_unique<StateHandler_55>( controller );
                default:BOOST_LOG_TRIVIAL( error ) << "No state handler for state " << loader::toString( id );
                    throw std::runtime_error( "Unhandled state" );
            }

            return nullptr;
        }


        std::unique_ptr<AbstractStateHandler>
        AbstractStateHandler::createWithRetainedAnimation(loader::LaraStateId id) const
        {
            auto handler = create( id, m_controller );
            handler->m_xRotationSpeed = m_xRotationSpeed;
            handler->m_yRotationSpeed = m_yRotationSpeed;
            handler->m_zRotationSpeed = m_zRotationSpeed;
            handler->m_xMovement = m_xMovement;
            handler->m_yMovement = m_yMovement;
            handler->m_zMovement = m_zMovement;
            return handler;
        }


        const core::InterpolatedValue<float>& AbstractStateHandler::getHealth() const noexcept
        {
            return m_controller.getHealth();
        }


        void AbstractStateHandler::setHealth(const core::InterpolatedValue<float>& h) noexcept
        {
            m_controller.setHealth( h );
        }


        void AbstractStateHandler::setAir(const core::InterpolatedValue<float>& a) noexcept
        {
            m_controller.setAir( a );
        }


        void AbstractStateHandler::setMovementAngle(core::Angle angle) noexcept
        {
            m_controller.setMovementAngle( angle );
        }


        core::Angle AbstractStateHandler::getMovementAngle() const noexcept
        {
            return m_controller.getMovementAngle();
        }


        void AbstractStateHandler::setFallSpeed(const core::InterpolatedValue<float>& spd)
        {
            m_controller.setFallSpeed( spd );
        }


        const core::InterpolatedValue<float>& AbstractStateHandler::getFallSpeed() const noexcept
        {
            return m_controller.getFallSpeed();
        }


        bool AbstractStateHandler::isFalling() const noexcept
        {
            return m_controller.isFalling();
        }


        void AbstractStateHandler::setFalling(bool falling) noexcept
        {
            m_controller.setFalling( falling );
        }


        int AbstractStateHandler::getHandStatus() const noexcept
        {
            return m_controller.getHandStatus();
        }


        void AbstractStateHandler::setHandStatus(int status) noexcept
        {
            m_controller.setHandStatus( status );
        }


        std::chrono::microseconds AbstractStateHandler::getCurrentTime() const
        {
            return m_controller.getCurrentTime();
        }


        loader::LaraStateId AbstractStateHandler::getCurrentAnimState() const
        {
            return m_controller.getCurrentAnimState();
        }


        void AbstractStateHandler::playAnimation(loader::AnimationId anim, const boost::optional<uint16_t>& firstFrame)
        {
            m_controller.playAnimation( anim, firstFrame );
        }


        const core::TRRotation& AbstractStateHandler::getRotation() const noexcept
        {
            return m_controller.getRotation();
        }


        void AbstractStateHandler::setHorizontalSpeed(const core::InterpolatedValue<float>& speed)
        {
            m_controller.setHorizontalSpeed( speed );
        }


        const core::InterpolatedValue<float>& AbstractStateHandler::getHorizontalSpeed() const
        {
            return m_controller.getHorizontalSpeed();
        }


        const level::Level& AbstractStateHandler::getLevel() const
        {
            return m_controller.getLevel();
        }


        void AbstractStateHandler::placeOnFloor(const CollisionInfo& collisionInfo)
        {
            m_controller.placeOnFloor( collisionInfo );
        }


        const core::ExactTRCoordinates& AbstractStateHandler::getPosition() const
        {
            return m_controller.getPosition();
        }


        void AbstractStateHandler::setPosition(const core::ExactTRCoordinates& pos)
        {
            m_controller.setPosition( pos );
        }


        long AbstractStateHandler::getFloorHeight() const
        {
            return m_controller.getFloorHeight();
        }


        void AbstractStateHandler::setFloorHeight(long h)
        {
            m_controller.setFloorHeight( h );
        }


        void AbstractStateHandler::setYRotationSpeed(core::Angle spd)
        {
            m_controller.setYRotationSpeed( spd );
        }


        core::Angle AbstractStateHandler::getYRotationSpeed() const
        {
            return m_controller.getYRotationSpeed();
        }


        void AbstractStateHandler::subYRotationSpeed(const std::chrono::microseconds& deltaTime, core::Angle val,
                                                     core::Angle limit)
        {
            m_controller.subYRotationSpeed( deltaTime, val, limit );
        }


        void AbstractStateHandler::addYRotationSpeed(const std::chrono::microseconds& deltaTime, core::Angle val,
                                                     core::Angle limit)
        {
            m_controller.addYRotationSpeed( deltaTime, val, limit );
        }


        void AbstractStateHandler::setXRotation(core::Angle x)
        {
            m_controller.setXRotation( x );
        }


        void AbstractStateHandler::setYRotation(core::Angle y)
        {
            m_controller.setYRotation( y );
        }


        void AbstractStateHandler::setZRotation(core::Angle z)
        {
            m_controller.setZRotation( z );
        }


        void AbstractStateHandler::setFallSpeedOverride(int v)
        {
            m_controller.setFallSpeedOverride( v );
        }


        void AbstractStateHandler::dampenHorizontalSpeed(const std::chrono::microseconds& deltaTime, float f)
        {
            m_controller.dampenHorizontalSpeed( deltaTime, f );
        }


        core::Angle AbstractStateHandler::getCurrentSlideAngle() const noexcept
        {
            return m_controller.getCurrentSlideAngle();
        }


        void AbstractStateHandler::setCurrentSlideAngle(core::Angle a) noexcept
        {
            m_controller.setCurrentSlideAngle( a );
        }


        void AbstractStateHandler::setTargetState(loader::LaraStateId state)
        {
            m_controller.setTargetState( state );
        }


        loader::LaraStateId AbstractStateHandler::getTargetState() const
        {
            return m_controller.getTargetState();
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
                    .findFloorSectorWithClampedPosition( pos.toInexact(), m_controller.getCurrentRoom() );
            HeightInfo floor = HeightInfo::fromFloor( sector, pos.toInexact(), getLevel().m_cameraController );
            HeightInfo ceil = HeightInfo::fromCeiling( sector, pos.toInexact(), getLevel().m_cameraController );
            return floor.distance != -loader::HeightLimit && floor.distance - pos.Y > 0 && ceil.distance - pos.Y < -400;
        }


        std::unique_ptr<AbstractStateHandler> AbstractStateHandler::tryReach(CollisionInfo& collisionInfo)
        {
            if( collisionInfo.axisCollisions != CollisionInfo::AxisColl_FrontForwardBlocked
                || !getLevel().m_inputHandler->getInputState().action || getHandStatus() != 0 )
                return nullptr;

            if( std::abs( collisionInfo.frontLeft.floor.distance - collisionInfo.frontRight.floor.distance )
                >= core::MaxGrabbableGradient )
                return nullptr;

            if( collisionInfo.front.ceiling.distance > 0
                || collisionInfo.current.ceiling.distance > -core::ClimbLimit2ClickMin
                || collisionInfo.current.floor.distance < 200 )
                return nullptr;

            const auto bbox = getBoundingBox();
            long spaceToReach = collisionInfo.front.floor.distance - bbox.min.y;

            if( spaceToReach < 0 && getFallSpeed() + spaceToReach < 0 )
                return nullptr;
            if( spaceToReach > 0 && getFallSpeed() + spaceToReach > 0 )
                return nullptr;

            auto alignedRotation = core::alignRotation( getRotation().Y, 35_deg );
            if( !alignedRotation )
                return nullptr;

            if( canClimbOnto( *core::axisFromAngle( getRotation().Y, 35_deg ) ) )
                playAnimation( loader::AnimationId::OSCILLATE_HANG_ON, 3974 );
            else
                playAnimation( loader::AnimationId::HANG_IDLE, 1493 );

            setTargetState( LaraStateId::Hang );
            setPosition( getPosition() + core::ExactTRCoordinates( collisionInfo.collisionFeedback.X, spaceToReach,
                                                                   collisionInfo.collisionFeedback.Z ) );
            setHorizontalSpeed( core::makeInterpolatedValue( 0.0f ) );
            setYRotation( *alignedRotation );
            setFalling( false );
            setFallSpeed( core::makeInterpolatedValue( 0.0f ) );
            setHandStatus( 1 );
            return createWithRetainedAnimation( LaraStateId::Hang );
        }


        std::unique_ptr<AbstractStateHandler>
        AbstractStateHandler::stopIfCeilingBlocked(const CollisionInfo& collisionInfo)
        {
            if( collisionInfo.axisCollisions != CollisionInfo::AxisColl_ScalpCollision
                && collisionInfo.axisCollisions != CollisionInfo::AxisColl_InvalidPosition )
                return nullptr;

            setPosition( collisionInfo.position );

            setTargetState( LaraStateId::Stop );
            playAnimation( loader::AnimationId::STAY_SOLID, 185 );
            setHorizontalSpeed( core::makeInterpolatedValue( 0.0f ) );
            setFallSpeed( core::makeInterpolatedValue( 0.0f ) );
            setFalling( false );
            return createWithRetainedAnimation( LaraStateId::Stop );
        }


        std::unique_ptr<AbstractStateHandler> AbstractStateHandler::tryClimb(CollisionInfo& collisionInfo)
        {
            if( collisionInfo.axisCollisions != CollisionInfo::AxisColl_FrontForwardBlocked
                || !getLevel().m_inputHandler->getInputState().action || getHandStatus() != 0 )
                return nullptr;

            const auto floorGradient = std::abs(
                    collisionInfo.frontLeft.floor.distance - collisionInfo.frontRight.floor.distance );
            if( floorGradient >= core::MaxGrabbableGradient )
                return nullptr;

            //! @todo MAGICK +/- 30 degrees
            auto alignedRotation = core::alignRotation( getRotation().Y, 30_deg );
            if( !alignedRotation )
                return nullptr;

            const auto climbHeight = collisionInfo.front.floor.distance;
            std::unique_ptr<AbstractStateHandler> nextHandler = nullptr;
            if( climbHeight >= -core::ClimbLimit2ClickMax && climbHeight <= -core::ClimbLimit2ClickMin )
            {
                if( climbHeight < collisionInfo.front.ceiling.distance
                    || collisionInfo.frontLeft.floor.distance < collisionInfo.frontLeft.ceiling.distance
                    || collisionInfo.frontRight.floor.distance < collisionInfo.frontRight.ceiling.distance )
                    return nullptr;

                setTargetState( LaraStateId::Stop );
                nextHandler = createWithRetainedAnimation( LaraStateId::Climbing );
                playAnimation( loader::AnimationId::CLIMB_2CLICK, 759 );
                m_yMovement = 2.0f * loader::QuarterSectorSize + climbHeight;
                setHandStatus( 1 );
            }
            else if( climbHeight >= -core::ClimbLimit3ClickMax && climbHeight <= -core::ClimbLimit2ClickMax )
            {
                if( collisionInfo.front.floor.distance < collisionInfo.front.ceiling.distance
                    || collisionInfo.frontLeft.floor.distance < collisionInfo.frontLeft.ceiling.distance
                    || collisionInfo.frontRight.floor.distance < collisionInfo.frontRight.ceiling.distance )
                    return nullptr;

                setTargetState( LaraStateId::Stop );
                nextHandler = createWithRetainedAnimation( LaraStateId::Climbing );
                playAnimation( loader::AnimationId::CLIMB_3CLICK, 614 );
                m_yMovement = 3.0f * loader::QuarterSectorSize + climbHeight;
                setHandStatus( 1 );
            }
            else if( climbHeight >= -core::JumpReachableHeight && climbHeight <= -core::ClimbLimit3ClickMax )
            {
                setTargetState( LaraStateId::JumpUp );
                nextHandler = createWithRetainedAnimation( LaraStateId::Stop );
                playAnimation( loader::AnimationId::STAY_SOLID, 185 );
                setFallSpeedOverride( -static_cast<int>(std::sqrt( -12 * (climbHeight + 800) + 3 )) );
            }
            else
            {
                return nullptr;
            }

            setYRotation( *alignedRotation );
            applyCollisionFeedback( collisionInfo );

            BOOST_ASSERT( nextHandler != nullptr );
            return nextHandler;
        }


        void AbstractStateHandler::applyCollisionFeedback(const CollisionInfo& collisionInfo)
        {
            setPosition( getPosition() + collisionInfo.collisionFeedback );
            collisionInfo.collisionFeedback = {0, 0, 0};
        }


        std::unique_ptr<AbstractStateHandler> AbstractStateHandler::checkWallCollision(CollisionInfo& collisionInfo)
        {
            if( collisionInfo.axisCollisions == CollisionInfo::AxisColl_FrontForwardBlocked
                || collisionInfo.axisCollisions == CollisionInfo::AxisColl_InsufficientFrontCeilingSpace )
            {
                applyCollisionFeedback( collisionInfo );
                setTargetState( LaraStateId::Stop );
                setFalling( false );
                setHorizontalSpeed( core::makeInterpolatedValue( 0.0f ) );
                return createWithRetainedAnimation( LaraStateId::Stop );
            }

            if( collisionInfo.axisCollisions == CollisionInfo::AxisColl_FrontLeftBlocked )
            {
                applyCollisionFeedback( collisionInfo );
                m_yRotationSpeed = 5_deg;
            }
            else if( collisionInfo.axisCollisions == CollisionInfo::AxisColl_FrontRightBlocked )
            {
                applyCollisionFeedback( collisionInfo );
                m_yRotationSpeed = -5_deg;
            }

            return nullptr;
        }


        bool AbstractStateHandler::tryStartSlide(const CollisionInfo& collisionInfo,
                                                 std::unique_ptr<AbstractStateHandler>& nextHandler)
        {
            int slantX = std::abs( collisionInfo.floorSlantX );
            int slantZ = std::abs( collisionInfo.floorSlantZ );
            if( slantX <= 2 && slantZ <= 2 )
                return false;

            core::Angle targetAngle{0_deg};
            if( collisionInfo.floorSlantX < -2 )
                targetAngle = 90_deg;
            else if( collisionInfo.floorSlantX > 2 )
                targetAngle = -90_deg;

            if( collisionInfo.floorSlantZ > std::max( 2, slantX ) )
                targetAngle = 180_deg;
            else if( collisionInfo.floorSlantZ < std::min( -2, -slantX ) )
                targetAngle = 0_deg;

            core::Angle dy = abs( targetAngle - getRotation().Y );
            applyCollisionFeedback( collisionInfo );
            if( dy > 90_deg || dy < -90_deg )
            {
                if( getCurrentAnimState() != LaraStateId::SlideBackward || targetAngle != getCurrentSlideAngle() )
                {
                    playAnimation( loader::AnimationId::START_SLIDE_BACKWARD, 1677 );
                    setTargetState( LaraStateId::SlideBackward );
                    nextHandler = createWithRetainedAnimation( LaraStateId::SlideBackward );
                    setMovementAngle( targetAngle );
                    setCurrentSlideAngle( targetAngle );
                    setYRotation( targetAngle + 180_deg );
                }
            }
            else if( getCurrentAnimState() != LaraStateId::SlideForward || targetAngle != getCurrentSlideAngle() )
            {
                playAnimation( loader::AnimationId::SLIDE_FORWARD, 1133 );
                setTargetState( LaraStateId::SlideForward );
                nextHandler = createWithRetainedAnimation( LaraStateId::SlideForward );
                setMovementAngle( targetAngle );
                setCurrentSlideAngle( targetAngle );
                setYRotation( targetAngle );
            }
            return true;
        }


        std::unique_ptr<AbstractStateHandler> AbstractStateHandler::tryGrabEdge(CollisionInfo& collisionInfo)
        {
            if( collisionInfo.axisCollisions != CollisionInfo::AxisColl_FrontForwardBlocked
                || !getLevel().m_inputHandler->getInputState().action || getHandStatus() != 0 )
                return nullptr;

            const auto floorGradient = std::abs(
                    collisionInfo.frontLeft.floor.distance - collisionInfo.frontRight.floor.distance );
            if( floorGradient >= core::MaxGrabbableGradient )
                return nullptr;

            if( collisionInfo.front.ceiling.distance > 0
                || collisionInfo.current.ceiling.distance > -core::ClimbLimit2ClickMin )
                return nullptr;

            auto bbox = getBoundingBox();
            long spaceToReach = collisionInfo.front.floor.distance - bbox.min.y;

            if( spaceToReach < 0 && getFallSpeed() + spaceToReach < 0 )
                return nullptr;
            if( spaceToReach > 0 && getFallSpeed() + spaceToReach > 0 )
                return nullptr;

            auto alignedRotation = core::alignRotation( getRotation().Y, 35_deg );
            if( !alignedRotation )
                return nullptr;

            setTargetState( LaraStateId::Hang );
            playAnimation( loader::AnimationId::HANG_IDLE, 1505 );
            bbox = getBoundingBox();
            spaceToReach = collisionInfo.front.floor.distance - bbox.min.y;

            setPosition( getPosition() + core::ExactTRCoordinates( 0, spaceToReach, 0 ) );
            applyCollisionFeedback( collisionInfo );
            setHorizontalSpeed( core::makeInterpolatedValue( 0.0f ) );
            setFallSpeed( core::makeInterpolatedValue( 0.0f ) );
            setFalling( false );
            setHandStatus( 1 );
            setYRotation( *alignedRotation );

            return createWithRetainedAnimation( LaraStateId::Hang );
        }


        int AbstractStateHandler::getRelativeHeightAtDirection(core::Angle angle, int dist) const
        {
            auto pos = getPosition();
            pos.X += angle.sin() * dist;
            pos.Y -= core::ScalpHeight;
            pos.Z += angle.cos() * dist;

            gsl::not_null<const loader::Sector*> sector = getLevel()
                    .findFloorSectorWithClampedPosition( pos.toInexact(), m_controller.getCurrentRoom() );

            HeightInfo h = HeightInfo::fromFloor( sector, pos.toInexact(), getLevel().m_cameraController );

            if( h.distance != -loader::HeightLimit )
                h.distance -= std::lround( getPosition().Y );

            return h.distance;
        }


        std::unique_ptr<AbstractStateHandler> AbstractStateHandler::commonJumpHandling(CollisionInfo& collisionInfo)
        {
            collisionInfo.neededFloorDistanceBottom = loader::HeightLimit;
            collisionInfo.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
            collisionInfo.neededCeilingDistance = 192;
            collisionInfo.yAngle = getMovementAngle();
            collisionInfo.initHeightInfo( getPosition(), getLevel(), core::ScalpHeight );
            auto nextHandler = checkJumpWallSmash( collisionInfo );
            if( getFallSpeed() <= 0 || collisionInfo.current.floor.distance > 0 )
                return nextHandler;

            if( applyLandingDamage() )
                setTargetState( LaraStateId::Death );
            else
                setTargetState( LaraStateId::Stop );
            setFallSpeed( core::makeInterpolatedValue( 0.0f ) );
            placeOnFloor( collisionInfo );
            setFalling( false );

            return nextHandler;
        }


        std::unique_ptr<AbstractStateHandler> AbstractStateHandler::commonSlideHandling(CollisionInfo& collisionInfo)
        {
            collisionInfo.neededFloorDistanceBottom = loader::HeightLimit;
            collisionInfo.neededFloorDistanceTop = -loader::QuarterSectorSize * 2;
            collisionInfo.neededCeilingDistance = 0;
            collisionInfo.yAngle = getMovementAngle();
            collisionInfo.initHeightInfo( getPosition(), getLevel(), core::ScalpHeight );

            if( auto nextHandler = stopIfCeilingBlocked( collisionInfo ) )
                return nextHandler;

            auto nextHandler = checkWallCollision( collisionInfo );
            if( collisionInfo.current.floor.distance <= 200 )
            {
                tryStartSlide( collisionInfo, nextHandler );
                placeOnFloor( collisionInfo );
                const auto absSlantX = std::abs( collisionInfo.floorSlantX );
                const auto absSlantZ = std::abs( collisionInfo.floorSlantZ );
                if( absSlantX <= 2 && absSlantZ <= 2 )
                {
                    setTargetState( LaraStateId::Stop );
                }
                return nextHandler;
            }

            if( getCurrentAnimState() == LaraStateId::SlideForward )
            {
                playAnimation( loader::AnimationId::FREE_FALL_FORWARD, 492 );
                setTargetState( LaraStateId::JumpForward );
                nextHandler = createWithRetainedAnimation( LaraStateId::JumpForward );
            }
            else
            {
                playAnimation( loader::AnimationId::FREE_FALL_BACK, 1473 );
                setTargetState( LaraStateId::FallBackward );
                nextHandler = createWithRetainedAnimation( LaraStateId::FallBackward );
            }

            setFallSpeed( core::makeInterpolatedValue( 0.0f ) );
            setFalling( true );

            return nextHandler;
        }


        std::unique_ptr<AbstractStateHandler> AbstractStateHandler::commonEdgeHangHandling(CollisionInfo& collisionInfo)
        {
            collisionInfo.neededFloorDistanceBottom = loader::HeightLimit;
            collisionInfo.neededFloorDistanceTop = -loader::HeightLimit;
            collisionInfo.neededCeilingDistance = 0;
            collisionInfo.yAngle = getMovementAngle();
            collisionInfo.initHeightInfo( getPosition(), getLevel(), core::ScalpHeight );
            const bool tooSteepToGrab = collisionInfo.front.floor.distance < 200;
            setFallSpeed( core::makeInterpolatedValue( 0.0f ) );
            setFalling( false );
            setMovementAngle( getRotation().Y );
            const auto axis = *core::axisFromAngle( getMovementAngle(), 45_deg );
            switch( axis )
            {
                case core::Axis::PosZ:setPosition( getPosition() + core::ExactTRCoordinates( 0, 0, 2 ) );
                    break;
                case core::Axis::PosX:setPosition( getPosition() + core::ExactTRCoordinates( 2, 0, 0 ) );
                    break;
                case core::Axis::NegZ:setPosition( getPosition() - core::ExactTRCoordinates( 0, 0, 2 ) );
                    break;
                case core::Axis::NegX:setPosition( getPosition() - core::ExactTRCoordinates( 2, 0, 0 ) );
                    break;
            }

            collisionInfo.neededFloorDistanceBottom = loader::HeightLimit;
            collisionInfo.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
            collisionInfo.neededCeilingDistance = 0;
            collisionInfo.yAngle = getMovementAngle();
            collisionInfo.initHeightInfo( getPosition(), getLevel(), core::ScalpHeight );
            if( !getLevel().m_inputHandler->getInputState().action || getHealth() <= 0 )
            {
                setTargetState( LaraStateId::JumpUp );
                playAnimation( loader::AnimationId::TRY_HANG_VERTICAL, 448 );
                setHandStatus( 0 );
                const auto bbox = getBoundingBox();
                const long hangDistance = collisionInfo.front.floor.distance - bbox.min.y + 2;
                setPosition( getPosition() + core::ExactTRCoordinates( collisionInfo.collisionFeedback.X, hangDistance,
                                                                       collisionInfo.collisionFeedback.Z ) );
                setHorizontalSpeed( core::makeInterpolatedValue( 2.0f ) );
                setFallSpeed( core::makeInterpolatedValue( 1.0f ) );
                setFalling( true );
                return createWithRetainedAnimation( LaraStateId::JumpUp );
            }

            auto gradient = std::abs(
                    collisionInfo.frontLeft.floor.distance - collisionInfo.frontRight.floor.distance );
            if( gradient >= core::MaxGrabbableGradient || collisionInfo.current.ceiling.distance >= 0
                || collisionInfo.axisCollisions != CollisionInfo::AxisColl_FrontForwardBlocked || tooSteepToGrab )
            {
                setPosition( collisionInfo.position );
                if( getCurrentAnimState() != LaraStateId::ShimmyLeft
                    && getCurrentAnimState() != LaraStateId::ShimmyRight )
                {
                    return nullptr;
                }

                setTargetState( LaraStateId::Hang );
                playAnimation( loader::AnimationId::HANG_IDLE, 1514 );
                return createWithRetainedAnimation( LaraStateId::Hang );
            }

            switch( axis )
            {
                case core::Axis::PosZ:
                case core::Axis::NegZ:
                    setPosition( getPosition() + core::ExactTRCoordinates( 0, 0, collisionInfo.collisionFeedback.Z ) );
                    break;
                case core::Axis::PosX:
                case core::Axis::NegX:
                    setPosition( getPosition() + core::ExactTRCoordinates( collisionInfo.collisionFeedback.X, 0, 0 ) );
                    break;
            }

            const auto bbox = getBoundingBox();
            const long spaceToReach = collisionInfo.front.floor.distance - bbox.min.y;

            if( spaceToReach >= -loader::QuarterSectorSize && spaceToReach <= loader::QuarterSectorSize )
                setPosition( getPosition() + core::ExactTRCoordinates( 0, spaceToReach, 0 ) );
            return nullptr;
        }


        bool AbstractStateHandler::applyLandingDamage()
        {
            auto sector = getLevel()
                    .findFloorSectorWithClampedPosition( getPosition().toInexact(), m_controller.getCurrentRoom() );
            HeightInfo h = HeightInfo::fromFloor( sector, getPosition().toInexact()
                                                          - core::TRCoordinates{0, core::ScalpHeight, 0},
                                                  getLevel().m_cameraController );
            setFloorHeight( h.distance );
            getController().handleTriggers( h.lastTriggerOrKill, false );
            auto damageSpeed = static_cast<float>(getFallSpeed()) - 140;
            if( damageSpeed <= 0 )
                return false;

            static constexpr int DeathSpeedLimit = 14;

            if( damageSpeed <= DeathSpeedLimit )
                setHealth( getHealth() - 1000 * damageSpeed * damageSpeed / (DeathSpeedLimit * DeathSpeedLimit) );
            else
                setHealth( core::makeInterpolatedValue( -1.0f ) );
            return getHealth() <= 0;
        }


        gameplay::BoundingBox AbstractStateHandler::getBoundingBox() const
        {
            return m_controller.getBoundingBox();
        }


        void AbstractStateHandler::addSwimToDiveKeypressDuration(const std::chrono::microseconds& ms) noexcept
        {
            m_controller.addSwimToDiveKeypressDuration( ms );
        }


        void AbstractStateHandler::setSwimToDiveKeypressDuration(const std::chrono::microseconds& ms) noexcept
        {
            m_controller.setSwimToDiveKeypressDuration( ms );
        }


        const boost::optional<std::chrono::microseconds>& AbstractStateHandler::getSwimToDiveKeypressDuration() const
        {
            return m_controller.getSwimToDiveKeypressDuration();
        }


        void AbstractStateHandler::setUnderwaterState(UnderwaterState u) noexcept
        {
            m_controller.setUnderwaterState( u );
        }


        void AbstractStateHandler::setCameraRotation(core::Angle x, core::Angle y)
        {
            m_controller.setCameraRotation( x, y );
        }


        void AbstractStateHandler::setCameraRotationX(core::Angle x)
        {
            m_controller.setCameraRotationX( x );
        }


        void AbstractStateHandler::setCameraRotationY(core::Angle y)
        {
            m_controller.setCameraRotationY( y );
        }


        void AbstractStateHandler::setCameraDistance(int d)
        {
            m_controller.setCameraDistance( d );
        }


        void AbstractStateHandler::setCameraUnknown1(int k)
        {
            m_controller.setCameraUnknown1( k );
        }


        void AbstractStateHandler::jumpAgainstWall(CollisionInfo& collisionInfo)
        {
            applyCollisionFeedback( collisionInfo );
            if( collisionInfo.axisCollisions == CollisionInfo::AxisColl_FrontLeftBlocked )
                m_yRotationSpeed = 5_deg;
            else if( collisionInfo.axisCollisions == CollisionInfo::AxisColl_FrontRightBlocked )
                m_yRotationSpeed = -5_deg;
            else if( collisionInfo.axisCollisions == CollisionInfo::AxisColl_ScalpCollision )
            {
                if( getFallSpeed() <= 0 )
                    setFallSpeed( core::makeInterpolatedValue( 1.0f ) );
            }
            else if( collisionInfo.axisCollisions == CollisionInfo::AxisColl_InvalidPosition )
            {
                m_xMovement = 100 * getRotation().Y.sin();
                m_zMovement = 100 * getRotation().Y.cos();
                setHorizontalSpeed( core::makeInterpolatedValue( 0.0f ) );
                collisionInfo.current.floor.distance = 0;
                if( getFallSpeed() < 0 )
                    setFallSpeed( core::makeInterpolatedValue( 16.0f ) );
            }
        }


        std::unique_ptr<AbstractStateHandler> AbstractStateHandler::checkJumpWallSmash(CollisionInfo& collisionInfo)
        {
            applyCollisionFeedback( collisionInfo );

            if( collisionInfo.axisCollisions == CollisionInfo::AxisColl_None )
                return nullptr;

            if( collisionInfo.axisCollisions == CollisionInfo::AxisColl_FrontForwardBlocked
                || collisionInfo.axisCollisions == CollisionInfo::AxisColl_InsufficientFrontCeilingSpace )
            {
                setTargetState( LaraStateId::FreeFall );
                //! @todo Check formula
                setHorizontalSpeed( getHorizontalSpeed() * 0.2f );
                setMovementAngle( getMovementAngle() - 180_deg );
                playAnimation( loader::AnimationId::SMASH_JUMP, 481 );
                if( getFallSpeed() <= 0 )
                    setFallSpeed( core::makeInterpolatedValue( 1.0f ) );
                return createWithRetainedAnimation( LaraStateId::FreeFall );
            }

            if( collisionInfo.axisCollisions == CollisionInfo::AxisColl_FrontLeftBlocked )
            {
                m_yRotationSpeed = 5_deg;
                return nullptr;
            }
            else if( collisionInfo.axisCollisions == CollisionInfo::AxisColl_FrontRightBlocked )
            {
                m_yRotationSpeed = -5_deg;
                return nullptr;
            }

            if( collisionInfo.axisCollisions == CollisionInfo::AxisColl_InvalidPosition )
            {
                m_xMovement = 100 * collisionInfo.yAngle.sin();
                m_zMovement = 100 * collisionInfo.yAngle.cos();
                setHorizontalSpeed( core::makeInterpolatedValue( 0.0f ) );
                collisionInfo.current.floor.distance = 0;
                if( getFallSpeed() <= 0 )
                    setFallSpeed( core::makeInterpolatedValue( 16.0f ) );
            }

            if( collisionInfo.axisCollisions == CollisionInfo::AxisColl_ScalpCollision && getFallSpeed() <= 0 )
                setFallSpeed( core::makeInterpolatedValue( 1.0f ) );

            return nullptr;
        }
    }
}
