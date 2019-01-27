#include "laranode.h"

#include "cameracontroller.h"
#include "level/level.h"
#include "render/textureanimator.h"

#include "items/block.h"
#include "items/aiagent.h"

#include <glm/gtx/norm.hpp>

#include <boost/range/adaptors.hpp>
#include <stack>

namespace
{
core::TRRotationXY getVectorAngles(const core::TRVec& co)
{
    return core::getVectorAngles( co.X, co.Y, co.Z );
}
}

namespace engine
{
namespace
{
const char* toString(const HandStatus s)
{
    switch( s )
    {
        case HandStatus::None:
            return "None";
        case HandStatus::Grabbing:
            return "Grabbing";
        case HandStatus::Unholster:
            return "Unholster";
        case HandStatus::Holster:
            return "Holster";
        case HandStatus::Combat:
            return "Combat";
        default:
            BOOST_THROW_EXCEPTION( std::domain_error( "Invalid HandStatus" ) );
    }
}

HandStatus parseHandStatus(const std::string& s)
{
    if( s == "None" )
        return HandStatus::None;
    if( s == "Grabbing" )
        return HandStatus::Grabbing;
    if( s == "Unholster" )
        return HandStatus::Unholster;
    if( s == "Holster" )
        return HandStatus::Holster;
    if( s == "Combat" )
        return HandStatus::Combat;
    BOOST_THROW_EXCEPTION( std::domain_error( "Invalid HandStatus" ) );
}

const char* toString(const UnderwaterState s)
{
    switch( s )
    {
        case UnderwaterState::OnLand:
            return "OnLand";
        case UnderwaterState::Diving:
            return "Diving";
        case UnderwaterState::Swimming:
            return "Swimming";
        default:
            BOOST_THROW_EXCEPTION( std::domain_error( "Invalid UnderwaterState" ) );
    }
}

UnderwaterState parseUnderwaterState(const std::string& s)
{
    if( s == "OnLand" )
        return UnderwaterState::OnLand;
    if( s == "Diving" )
        return UnderwaterState::Diving;
    if( s == "Swimming" )
        return UnderwaterState::Swimming;
    BOOST_THROW_EXCEPTION( std::domain_error( "Invalid UnderwaterState" ) );
}

const char* toString(const core::Axis a)
{
    switch( a )
    {
        case core::Axis::PosZ:
            return "+Z";
        case core::Axis::PosX:
            return "+X";
        case core::Axis::NegZ:
            return "-Z";
        case core::Axis::NegX:
            return "-X";
        default:
            BOOST_THROW_EXCEPTION( std::domain_error( "Invalid Axis" ) );
    }
}

core::Axis parseAxis(const std::string& s)
{
    if( s == "+Z" )
        return core::Axis::PosZ;
    if( s == "+X" )
        return core::Axis::PosX;
    if( s == "-Z" )
        return core::Axis::NegZ;
    if( s == "-X" )
        return core::Axis::NegX;
    BOOST_THROW_EXCEPTION( std::domain_error( "Invalid Axis" ) );
}

const char* toString(const LaraNode::WeaponId id)
{
    switch( id )
    {

        case LaraNode::WeaponId::None:
            return "None";
        case LaraNode::WeaponId::Pistols:
            return "Pistols";
        case LaraNode::WeaponId::AutoPistols:
            return "AutoPistols";
        case LaraNode::WeaponId::Uzi:
            return "Uzi";
        case LaraNode::WeaponId::Shotgun:
            return "Shotgun";
        default:
            BOOST_THROW_EXCEPTION( std::domain_error( "Invalid WeaponId" ) );
    }
}

LaraNode::WeaponId parseWeaponId(const std::string& s)
{
    if( s == "None" )
        return LaraNode::WeaponId::None;
    if( s == "Pistols" )
        return LaraNode::WeaponId::Pistols;
    if( s == "AutoPistols" )
        return LaraNode::WeaponId::AutoPistols;
    if( s == "Uzi" )
        return LaraNode::WeaponId::Uzi;
    if( s == "Shotgun" )
        return LaraNode::WeaponId::Shotgun;
    BOOST_THROW_EXCEPTION( std::domain_error( "Invalid WeaponId" ) );
}

}

void LaraNode::setAnimation(loader::AnimationId anim, const boost::optional<uint16_t>& firstFrame)
{
    getSkeleton()->setAnimation( m_state,
                                 &getLevel().m_animations.at( static_cast<uint16_t>(anim) ),
                                 firstFrame.get_value_or( 0 ) );
}

void LaraNode::handleLaraStateOnLand()
{
    CollisionInfo collisionInfo;
    collisionInfo.oldPosition = m_state.position.position;
    collisionInfo.collisionRadius = core::DefaultCollisionRadius;
    collisionInfo.policyFlags = CollisionInfo::EnableSpaz | CollisionInfo::EnableBaddiePush;

    lara::AbstractStateHandler::create( getCurrentAnimState(), *this )->handleInput( collisionInfo );

    if( getLevel().m_cameraController->getMode() != CameraMode::FreeLook )
    {
        const auto headX = m_headRotation.X;
        if( headX <= -2_deg || headX >= 2_deg )
        {
            m_headRotation.X = headX - headX / 8;
        }
        else
        {
            m_headRotation.X = 0_deg;
        }
        const auto headY = m_headRotation.Y;
        if( headY <= -2_deg || headY >= 2_deg )
        {
            m_headRotation.Y = headY - headY / 8;
        }
        else
        {
            m_headRotation.Y = 0_deg;
        }
        m_torsoRotation = m_headRotation;
    }

    // "slowly" revert rotations to zero
    if( m_state.rotation.Z < -1_deg )
    {
        m_state.rotation.Z += 1_deg;
        if( m_state.rotation.Z >= 0_deg )
        {
            m_state.rotation.Z = 0_deg;
        }
    }
    else if( m_state.rotation.Z > 1_deg )
    {
        m_state.rotation.Z -= 1_deg;
        if( m_state.rotation.Z <= 0_deg )
        {
            m_state.rotation.Z = 0_deg;
        }
    }
    else
    {
        m_state.rotation.Z = 0_deg;
    }

    if( m_yRotationSpeed > 2_deg )
    {
        m_yRotationSpeed -= 2_deg;
    }
    else if( m_yRotationSpeed < -2_deg )
    {
        m_yRotationSpeed += 2_deg;
    }
    else
    {
        m_yRotationSpeed = 0_deg;
    }

    m_state.rotation.Y += m_yRotationSpeed;

    updateImpl();

    testInteractions( collisionInfo );

    lara::AbstractStateHandler::create( getCurrentAnimState(), *this )->postprocessFrame( collisionInfo );

    updateFloorHeight( -381 );

    updateLarasWeaponsStatus();
    handleCommandSequence( collisionInfo.mid.floor.lastCommandSequenceOrDeath, false );

    applyTransform();

#ifndef NDEBUG
    lastUsedCollisionInfo = collisionInfo;
#endif
}

void LaraNode::handleLaraStateDiving()
{
    CollisionInfo collisionInfo;
    collisionInfo.oldPosition = m_state.position.position;
    collisionInfo.collisionRadius = core::DefaultCollisionRadiusUnderwater;
    collisionInfo.policyFlags &= ~(CollisionInfo::EnableSpaz
                                   | CollisionInfo::EnableBaddiePush
                                   | CollisionInfo::LavaIsPit
                                   | CollisionInfo::SlopesArePits
                                   | CollisionInfo::SlopesAreWalls);
    collisionInfo.badCeilingDistance = core::LaraHeightUnderwater;
    collisionInfo.badPositiveDistance = loader::HeightLimit;
    collisionInfo.badNegativeDistance = -core::LaraHeightUnderwater;

    lara::AbstractStateHandler::create( getCurrentAnimState(), *this )->handleInput( collisionInfo );

    // "slowly" revert rotations to zero
    if( m_state.rotation.Z < -2_deg )
    {
        m_state.rotation.Z += +2_deg;
    }
    else
    {
        if( m_state.rotation.Z > 2_deg )
        {
            m_state.rotation.Z += -2_deg;
        }
        else
        {
            m_state.rotation.Z = 0_deg;
        }
    }
    const core::Angle x = util::clamp( m_state.rotation.X, -100_deg, +100_deg );
    m_state.rotation.X = x;
    const core::Angle z = util::clamp( m_state.rotation.Z, -22_deg, +22_deg );
    m_state.rotation.Z = z;

    if( m_underwaterCurrentStrength != 0 )
    {
        handleUnderwaterCurrent( collisionInfo );
    }

    updateImpl();

    {
        auto pos = m_state.position.position;
        pos.X += m_state.rotation.Y.sin() * m_state.rotation.X.cos() * m_state.fallspeed / 4;
        pos.Y -= m_state.rotation.X.sin() * m_state.fallspeed / 4;
        pos.Z += m_state.rotation.Y.cos() * m_state.rotation.X.cos() * m_state.fallspeed / 4;
        m_state.position.position = pos;
    }

    testInteractions( collisionInfo );

    lara::AbstractStateHandler::create( getCurrentAnimState(), *this )->postprocessFrame( collisionInfo );

    updateFloorHeight( 0 );
    updateLarasWeaponsStatus();
    handleCommandSequence( collisionInfo.mid.floor.lastCommandSequenceOrDeath, false );
#ifndef NDEBUG
    lastUsedCollisionInfo = collisionInfo;
#endif
}

void LaraNode::handleLaraStateSwimming()
{
    CollisionInfo collisionInfo;
    collisionInfo.oldPosition = m_state.position.position;
    collisionInfo.collisionRadius = core::DefaultCollisionRadius;
    collisionInfo.policyFlags &= ~(CollisionInfo::EnableSpaz | CollisionInfo::EnableBaddiePush
                                   | CollisionInfo::LavaIsPit
                                   | CollisionInfo::SlopesArePits
                                   | CollisionInfo::SlopesAreWalls);
    collisionInfo.badCeilingDistance = core::DefaultCollisionRadius;
    collisionInfo.badPositiveDistance = loader::HeightLimit;
    collisionInfo.badNegativeDistance = -core::DefaultCollisionRadius;

    setCameraRotationAroundCenterX( -22_deg );

    lara::AbstractStateHandler::create( getCurrentAnimState(), *this )->handleInput( collisionInfo );

    // "slowly" revert rotations to zero
    if( m_state.rotation.Z < 0_deg )
    {
        m_state.rotation.Z += +2_deg;
    }
    else if( m_state.rotation.Z > 2_deg )
    {
        m_state.rotation.Z += -2_deg;
    }
    else
    {
        m_state.rotation.Z = 0_deg;
    }

    if( getLevel().m_cameraController->getMode() != CameraMode::FreeLook )
    {
        m_headRotation.X -= m_headRotation.X / 8;
        m_headRotation.Y -= m_headRotation.Y / 8;
        m_torsoRotation.X = 0_deg;
        m_torsoRotation.Y /= 2;
    }

    if( m_underwaterCurrentStrength != 0 )
    {
        handleUnderwaterCurrent( collisionInfo );
    }

    updateImpl();

    move(
            getMovementAngle().sin() * m_state.fallspeed / 4,
            0,
            getMovementAngle().cos() * m_state.fallspeed / 4
    );

    testInteractions( collisionInfo );

    lara::AbstractStateHandler::create( getCurrentAnimState(), *this )->postprocessFrame( collisionInfo );

    updateFloorHeight( core::DefaultCollisionRadius );
    updateLarasWeaponsStatus();
    handleCommandSequence( collisionInfo.mid.floor.lastCommandSequenceOrDeath, false );
#ifndef NDEBUG
    lastUsedCollisionInfo = collisionInfo;
#endif
}

void LaraNode::placeOnFloor(const CollisionInfo& collisionInfo)
{
    m_state.position.position.Y += collisionInfo.mid.floor.y;
}

LaraNode::~LaraNode() = default;

void LaraNode::update()
{
    if( getLevel().m_inputHandler->getInputState()._1 )
        getLevel().tryUseInventoryItem( TR1ItemId::Pistols );
    else if( getLevel().m_inputHandler->getInputState()._2 )
        getLevel().tryUseInventoryItem( TR1ItemId::Shotgun );
    else if( getLevel().m_inputHandler->getInputState()._3 )
        getLevel().tryUseInventoryItem( TR1ItemId::Uzis );
    else if( getLevel().m_inputHandler->getInputState()._4 )
        getLevel().tryUseInventoryItem( TR1ItemId::Magnums );
    else if( getLevel().m_inputHandler->getInputState()._5 )
        getLevel().tryUseInventoryItem( TR1ItemId::SmallMedipack );
    else if( getLevel().m_inputHandler->getInputState()._6 )
        getLevel().tryUseInventoryItem( TR1ItemId::LargeMedipack );

    if( m_underwaterState == UnderwaterState::OnLand && m_state.position.room->isWaterRoom() )
    {
        m_air = core::LaraAir;
        m_underwaterState = UnderwaterState::Diving;
        m_state.falling = false;
        const core::TRVec& pos = m_state.position.position + core::TRVec( 0, 100, 0 );
        m_state.position.position = pos;
        updateFloorHeight( 0 );
        getLevel().stopSound( TR1SoundId::LaraScream );
        if( getCurrentAnimState() == LaraStateId::SwandiveBegin )
        {
            m_state.rotation.X = -45_deg;
            setGoalAnimState( LaraStateId::UnderwaterDiving );
            updateImpl();
            m_state.fallspeed *= 2;
        }
        else if( getCurrentAnimState() == LaraStateId::SwandiveEnd )
        {
            m_state.rotation.X = -85_deg;
            setGoalAnimState( LaraStateId::UnderwaterDiving );
            updateImpl();
            m_state.fallspeed *= 2;
        }
        else
        {
            m_state.rotation.X = -45_deg;
            setAnimation( loader::AnimationId::FREE_FALL_TO_UNDERWATER, 1895 );
            setGoalAnimState( LaraStateId::UnderwaterForward );
            m_state.fallspeed += m_state.fallspeed / 2;
        }

        resetHeadTorsoRotation();

        if( const auto waterSurfaceHeight = getWaterSurfaceHeight() )
        {
            playSoundEffect( TR1SoundId::LaraFallIntoWater );

            auto room = m_state.position.room;
            level::Level::findRealFloorSector( m_state.position.position, &room );
            for( int i = 0; i < 10; ++i )
            {
                core::RoomBoundPosition surfacePos{room};
                surfacePos.position.X = m_state.position.position.X;
                surfacePos.position.Y = *waterSurfaceHeight;
                surfacePos.position.Z = m_state.position.position.Z;

                auto particle = std::make_shared<SplashParticle>( surfacePos, getLevel(), false );
                setParent( particle, surfacePos.room->node );
                getLevel().m_particles.emplace_back( particle );
            }
        }
    }
    else if( m_underwaterState == UnderwaterState::Diving && !m_state.position.room->isWaterRoom() )
    {
        auto waterSurfaceHeight = getWaterSurfaceHeight();
        m_state.fallspeed = 0;
        m_state.rotation.X = 0_deg;
        m_state.rotation.Z = 0_deg;
        resetHeadTorsoRotation();
        m_handStatus = HandStatus::None;

        if( !waterSurfaceHeight
            || std::abs( *waterSurfaceHeight - m_state.position.position.Y ) >= loader::QuarterSectorSize )
        {
            m_underwaterState = UnderwaterState::OnLand;
            setAnimation( loader::AnimationId::FREE_FALL_FORWARD, 492 );
            setGoalAnimState( LaraStateId::JumpForward );
            m_state.speed = std::exchange( m_state.fallspeed, 0 ) / 4;
            m_state.falling = true;
        }
        else
        {
            m_underwaterState = UnderwaterState::Swimming;
            setAnimation( loader::AnimationId::UNDERWATER_TO_ONWATER, 1937 );
            setGoalAnimState( LaraStateId::OnWaterStop );
            m_state.position.position.Y = *waterSurfaceHeight + 1;
            m_swimToDiveKeypressDuration = 11;
            updateFloorHeight( -381 );
            playSoundEffect( TR1SoundId::LaraCatchingAir );
        }
    }
    else if( m_underwaterState == UnderwaterState::Swimming && !m_state.position.room->isWaterRoom() )
    {
        m_underwaterState = UnderwaterState::OnLand;
        setAnimation( loader::AnimationId::FREE_FALL_FORWARD, 492 );
        setGoalAnimState( LaraStateId::JumpForward );
        m_state.speed = std::exchange( m_state.fallspeed, 0 ) / 4;
        m_state.falling = true;
        m_handStatus = HandStatus::None;
        m_state.rotation.X = 0_deg;
        m_state.rotation.Z = 0_deg;
        resetHeadTorsoRotation();
    }

    if( m_underwaterState == UnderwaterState::OnLand )
    {
        m_air = core::LaraAir;
        handleLaraStateOnLand();
    }
    else if( m_underwaterState == UnderwaterState::Diving )
    {
        if( m_state.health >= 0 )
        {
            --m_air;
            if( m_air < 0 )
            {
                m_air = -1;
                m_state.health -= 5;
            }
        }
        handleLaraStateDiving();
    }
    else if( m_underwaterState == UnderwaterState::Swimming )
    {
        if( m_state.health >= 0 )
        {
            m_air = std::min( m_air + 10, core::LaraAir );
        }
        handleLaraStateSwimming();
    }
}

void LaraNode::updateImpl()
{
    const auto endOfAnim = getSkeleton()->advanceFrame( m_state );

    Expects( m_state.anim != nullptr );
    if( endOfAnim )
    {
        if( m_state.anim->animCommandCount > 0 )
        {
            BOOST_ASSERT( m_state.anim->animCommandIndex < getLevel().m_animCommands.size() );
            const auto* cmd = &getLevel().m_animCommands[m_state.anim->animCommandIndex];
            for( uint16_t i = 0; i < m_state.anim->animCommandCount; ++i )
            {
                BOOST_ASSERT( cmd < &getLevel().m_animCommands.back() );
                const auto opcode = static_cast<AnimCommandOpcode>(*cmd);
                ++cmd;
                switch( opcode )
                {
                    case AnimCommandOpcode::SetPosition:
                        moveLocal(
                                cmd[0],
                                cmd[1],
                                cmd[2]
                        );
                        cmd += 3;
                        break;
                    case AnimCommandOpcode::StartFalling:
                        if( m_fallSpeedOverride != 0 )
                        {
                            m_state.fallspeed = m_fallSpeedOverride;
                            m_fallSpeedOverride = 0;
                        }
                        else
                        {
                            m_state.fallspeed = cmd[0];
                        }
                        m_state.speed = cmd[1];
                        m_state.falling = true;
                        cmd += 2;
                        break;
                    case AnimCommandOpcode::EmptyHands:
                        setHandStatus( HandStatus::None );
                        break;
                    case AnimCommandOpcode::PlaySound:
                        cmd += 2;
                        break;
                    case AnimCommandOpcode::PlayEffect:
                        cmd += 2;
                        break;
                    default:
                        break;
                }
            }
        }

        getSkeleton()
                ->setAnimation( m_state, m_state.anim->nextAnimation,
                                m_state.anim->nextFrame );
    }

    if( m_state.anim->animCommandCount > 0 )
    {
        BOOST_ASSERT( m_state.anim->animCommandIndex < getLevel().m_animCommands.size() );
        const auto* cmd = &getLevel().m_animCommands[m_state.anim->animCommandIndex];
        for( uint16_t i = 0; i < m_state.anim->animCommandCount; ++i )
        {
            BOOST_ASSERT( cmd < &getLevel().m_animCommands.back() );
            const auto opcode = static_cast<AnimCommandOpcode>(*cmd);
            ++cmd;
            switch( opcode )
            {
                case AnimCommandOpcode::SetPosition:
                    cmd += 3;
                    break;
                case AnimCommandOpcode::StartFalling:
                    cmd += 2;
                    break;
                case AnimCommandOpcode::PlaySound:
                    if( m_state.frame_number == cmd[0] )
                    {
                        playSoundEffect( static_cast<TR1SoundId>(cmd[1]) );
                    }
                    cmd += 2;
                    break;
                case AnimCommandOpcode::PlayEffect:
                    if( m_state.frame_number == cmd[0] )
                    {
                        BOOST_LOG_TRIVIAL( debug ) << "Anim effect: " << int( cmd[1] );
                        getLevel().runEffect( cmd[1], this );
                    }
                    cmd += 2;
                    break;
                default:
                    break;
            }
        }
    }

    applyMovement( true );

    drawRoutine();
}

void LaraNode::updateFloorHeight(const int dy)
{
    auto pos = m_state.position.position;
    pos.Y += dy;
    auto room = m_state.position.room;
    const auto sector = level::Level::findRealFloorSector( pos, &room );
    setCurrentRoom( room );
    const HeightInfo hi = HeightInfo::fromFloor( sector, pos, getLevel().m_itemNodes );
    m_state.floor = hi.y;
}

void LaraNode::handleCommandSequence(const uint16_t* floorData, const bool fromHeavy)
{
    if( floorData == nullptr )
        return;

    floordata::FloorDataChunk chunkHeader{*floorData};

    if( chunkHeader.type == floordata::FloorDataChunkType::Death )
    {
        if( !fromHeavy )
        {
            if( m_state.position.position.Y == m_state.floor )
            {
                //! @todo kill Lara
            }
        }

        if( chunkHeader.isLast )
            return;

        ++floorData;
    }

    chunkHeader = floordata::FloorDataChunk{*floorData++};
    BOOST_ASSERT( chunkHeader.type == floordata::FloorDataChunkType::CommandSequence );
    const uint16_t activationRequestRaw = *floorData++;
    const floordata::ActivationState activationRequest{activationRequestRaw};

    getLevel().m_cameraController->handleCommandSequence( floorData );

    bool conditionFulfilled, switchIsOn = false;
    if( fromHeavy )
    {
        conditionFulfilled = chunkHeader.sequenceCondition == floordata::SequenceCondition::ItemIsHere;
    }
    else
    {
        switch( chunkHeader.sequenceCondition )
        {
            case floordata::SequenceCondition::LaraIsHere:
                conditionFulfilled = true;
                break;
            case floordata::SequenceCondition::LaraOnGround:
            case floordata::SequenceCondition::LaraOnGroundInverted:
            {
                conditionFulfilled = m_state.position.position.Y == m_state.floor;
            }
                break;
            case floordata::SequenceCondition::ItemActivated:
            {
                const floordata::Command command{*floorData++};
                auto swtch = getLevel().m_itemNodes.find( command.parameter );
                Expects( swtch != getLevel().m_itemNodes.end() );
                if( !swtch->second->triggerSwitch( activationRequestRaw ) )
                    return;

                switchIsOn = (swtch->second->m_state.current_anim_state == 1);
                conditionFulfilled = true;
            }
                break;
            case floordata::SequenceCondition::KeyUsed:
            {
                const floordata::Command command{*floorData++};
                auto key = getLevel().m_itemNodes.find( command.parameter );
                Expects( key != getLevel().m_itemNodes.end() );
                if( key->second->triggerKey() )
                    conditionFulfilled = true;
                else
                    return;
            }
                break;
            case floordata::SequenceCondition::ItemPickedUp:
                if( getLevel().m_itemNodes.at( floordata::Command{*floorData++}.parameter )->triggerPickUp() )
                    conditionFulfilled = true;
                else
                    return;
                break;
            case floordata::SequenceCondition::LaraInCombatMode:
                conditionFulfilled = getHandStatus() == HandStatus::Combat;
                break;
            case floordata::SequenceCondition::ItemIsHere:
            case floordata::SequenceCondition::Dummy:
                return;
            default:
                conditionFulfilled = true;
                break;
        }
    }

    if( !conditionFulfilled )
        return;

    bool swapRooms = false;
    boost::optional<size_t> flipEffect;
    while( true )
    {
        const floordata::Command command{*floorData++};
        switch( command.opcode )
        {
            case floordata::CommandOpcode::Activate:
            {
                Expects( getLevel().m_itemNodes.find( command.parameter ) != getLevel().m_itemNodes.end() );
                auto itemIt = getLevel().m_itemNodes.find( command.parameter );
                Expects( itemIt != getLevel().m_itemNodes.end() );
                auto& item = *itemIt->second;
                if( item.m_state.activationState.isOneshot() )
                    break;

                item.m_state.timer = floordata::ActivationState::extractTimeout( activationRequestRaw );

                if( chunkHeader.sequenceCondition == floordata::SequenceCondition::ItemActivated )
                    item.m_state.activationState ^= activationRequest.getActivationSet();
                else if( chunkHeader.sequenceCondition == floordata::SequenceCondition::LaraOnGroundInverted )
                    item.m_state.activationState &= ~activationRequest.getActivationSet();
                else
                    item.m_state.activationState |= activationRequest.getActivationSet();

                if( !item.m_state.activationState.isFullyActivated() )
                    break;

                if( activationRequest.isOneshot() )
                    item.m_state.activationState.setOneshot( true );

                if( item.m_isActive )
                    break;

                if( item.m_state.triggerState == items::TriggerState::Inactive
                    || item.m_state.triggerState == items::TriggerState::Invisible
                    || dynamic_cast<items::AIAgent*>(&item) == nullptr )
                {
                    item.m_state.triggerState = items::TriggerState::Active;
                    item.m_state.touch_bits = 0;
                    item.activate();
                    break;
                }
            }
                break;
            case floordata::CommandOpcode::SwitchCamera:
            {
                const floordata::CameraParameters camParams{*floorData++};
                getLevel().m_cameraController
                          ->setCamOverride( camParams, command.parameter, chunkHeader.sequenceCondition,
                                            fromHeavy, activationRequestRaw, switchIsOn );
                command.isLast = camParams.isLast;
            }
                break;
            case floordata::CommandOpcode::LookAt:
                getLevel().m_cameraController->setLookAtItem( getLevel().getItem( command.parameter ) );
                break;
            case floordata::CommandOpcode::UnderwaterCurrent:
            {
                BOOST_ASSERT( command.parameter < getLevel().m_cameras.size() );
                const auto& sink = getLevel().m_cameras.at(command.parameter);
                if( m_underwaterRoute.required_box != &getLevel().m_boxes[sink.box_index] )
                {
                    m_underwaterRoute.required_box = &getLevel().m_boxes[sink.box_index];
                    m_underwaterRoute.target = sink.position;
                }
                m_underwaterCurrentStrength = 6 * sink.underwaterCurrentStrength;
            }
                break;
            case floordata::CommandOpcode::FlipMap:
                BOOST_ASSERT( command.parameter < getLevel().mapFlipActivationStates.size() );
                if( !getLevel().mapFlipActivationStates[command.parameter].isOneshot() )
                {
                    if( chunkHeader.sequenceCondition == floordata::SequenceCondition::ItemActivated )
                    {
                        getLevel().mapFlipActivationStates[command.parameter] ^= activationRequest.getActivationSet();
                    }
                    else
                    {
                        getLevel().mapFlipActivationStates[command.parameter] |= activationRequest.getActivationSet();
                    }

                    if( getLevel().mapFlipActivationStates[command.parameter].isFullyActivated() )
                    {
                        if( activationRequest.isOneshot() )
                            getLevel().mapFlipActivationStates[command.parameter].setOneshot( true );

                        if( !getLevel().roomsAreSwapped )
                            swapRooms = true;
                    }
                    else if( getLevel().roomsAreSwapped )
                    {
                        swapRooms = true;
                    }
                }
                break;
            case floordata::CommandOpcode::FlipOn:
                BOOST_ASSERT( command.parameter < getLevel().mapFlipActivationStates.size() );
                if( !getLevel().roomsAreSwapped
                    && getLevel().mapFlipActivationStates[command.parameter].isFullyActivated() )
                    swapRooms = true;
                break;
            case floordata::CommandOpcode::FlipOff:
                BOOST_ASSERT( command.parameter < getLevel().mapFlipActivationStates.size() );
                if( getLevel().roomsAreSwapped
                    && getLevel().mapFlipActivationStates[command.parameter].isFullyActivated() )
                    swapRooms = true;
                break;
            case floordata::CommandOpcode::FlipEffect:
                flipEffect = command.parameter;
                break;
            case floordata::CommandOpcode::EndLevel:
                getLevel().m_levelFinished = true;
                break;
            case floordata::CommandOpcode::PlayTrack:
                getLevel().triggerCdTrack( static_cast<TR1TrackId>(command.parameter),
                                           activationRequest, chunkHeader.sequenceCondition );
                break;
            case floordata::CommandOpcode::Secret:
            {
                BOOST_ASSERT( command.parameter < 16 );
                const uint16_t mask = 1u << command.parameter;
                if( (m_secretsFoundBitmask & mask) == 0 )
                {
                    m_secretsFoundBitmask |= mask;
                    getLevel().playStopCdTrack( TR1TrackId::Secret, false );
                }
            }
                break;
            default:
                break;
        }

        if( command.isLast )
            break;
    }

    if( !swapRooms )
        return;

    getLevel().swapAllRooms();

    if( flipEffect.is_initialized() )
        getLevel().setGlobalEffect( *flipEffect );
}

void LaraNode::setCameraRotationAroundCenter(const core::Angle x, const core::Angle y)
{
    getLevel().m_cameraController->setRotationAroundCenter( x, y );
}

void LaraNode::setCameraRotationAroundCenterY(const core::Angle y)
{
    getLevel().m_cameraController->setRotationAroundCenterY( y );
}

void LaraNode::setCameraRotationAroundCenterX(const core::Angle x)
{
    getLevel().m_cameraController->setRotationAroundCenterX( x );
}

void LaraNode::setCameraEyeCenterDistance(const int d)
{
    getLevel().m_cameraController->setEyeCenterDistance( d );
}

void LaraNode::setCameraModifier(const CameraModifier k)
{
    getLevel().m_cameraController->setModifier( k );
}

void LaraNode::testInteractions(CollisionInfo& collisionInfo)
{
    m_state.is_hit = false;
    hit_direction.reset();

    if( m_state.health < 0 )
        return;

    std::set<gsl::not_null<const loader::Room*>> rooms;
    rooms.insert( m_state.position.room );
    for( const loader::Portal& p : m_state.position.room->portals )
        rooms.insert( &getLevel().m_rooms[p.adjoining_room] );

    for( const auto& item : getLevel().m_itemNodes | boost::adaptors::map_values )
    {
        if( rooms.find( item->m_state.position.room ) == rooms.end() )
            continue;

        if( !item->m_state.collidable )
            continue;

        if( item->m_state.triggerState == items::TriggerState::Invisible )
            continue;

        const auto d = m_state.position.position - item->m_state.position.position;
        if( std::abs( d.X ) >= 4096 || std::abs( d.Y ) >= 4096 || std::abs( d.Z ) >= 4096 )
            continue;

        item->collide( *this, collisionInfo );
    }

    for( const auto& item : getLevel().m_dynamicItems )
    {
        if( rooms.find( item->m_state.position.room ) == rooms.end() )
            continue;

        if( !item->m_state.collidable )
            continue;

        if( item->m_state.triggerState == items::TriggerState::Invisible )
            continue;

        const auto d = m_state.position.position - item->m_state.position.position;
        if( std::abs( d.X ) >= 4096 || std::abs( d.Y ) >= 4096 || std::abs( d.Z ) >= 4096 )
            continue;

        item->collide( *this, collisionInfo );
    }

    if( getLevel().m_lara->explosionStumblingDuration != 0 )
    {
        getLevel().m_lara->updateExplosionStumbling();
    }
    if( !getLevel().m_lara->hit_direction.is_initialized() )
    {
        getLevel().m_lara->hit_frame = 0;
    }
    // TODO selectedPuzzleKey = -1;
}

void LaraNode::handleUnderwaterCurrent(CollisionInfo& collisionInfo)
{
    m_state.box = m_state.getCurrentSector()->box;
    core::TRVec targetPos;
    if( !m_underwaterRoute.calculateTarget( getLevel(), targetPos, m_state ) )
        return;

    targetPos -= m_state.position.position;
    m_state.position.position.X += util::clamp( targetPos.X, -m_underwaterCurrentStrength,
                                                m_underwaterCurrentStrength );
    m_state.position.position.Y += util::clamp( targetPos.Y, -m_underwaterCurrentStrength,
                                                m_underwaterCurrentStrength );
    m_state.position.position.Z += util::clamp( targetPos.Z, -m_underwaterCurrentStrength,
                                                m_underwaterCurrentStrength );

    m_underwaterCurrentStrength = 0;
    collisionInfo.facingAngle = core::Angle::fromAtan(
            m_state.position.position.X - collisionInfo.oldPosition.X,
            m_state.position.position.Z - collisionInfo.oldPosition.Z
    );

    collisionInfo.initHeightInfo( m_state.position.position + core::TRVec{0, 200, 0}, getLevel(), 400 );
    if( collisionInfo.collisionType == CollisionInfo::AxisColl_Front )
    {
        if( m_state.rotation.X > 35_deg )
            m_state.rotation.X += 2_deg;
        else if( m_state.rotation.X < -35_deg )
            m_state.rotation.X -= 2_deg;
    }
    else if( collisionInfo.collisionType == CollisionInfo::AxisColl_Top )
        m_state.rotation.X -= 2_deg;
    else if( collisionInfo.collisionType == CollisionInfo::AxisColl_TopBottom )
        m_state.fallspeed = 0;
    else if( collisionInfo.collisionType == CollisionInfo::AxisColl_Left )
        m_state.rotation.Y += 5_deg;
    else if( collisionInfo.collisionType == CollisionInfo::AxisColl_Right )
        m_state.rotation.Y -= 5_deg;

    if( collisionInfo.mid.floor.y < 0 )
    {
        m_state.position.position.Y += collisionInfo.mid.floor.y;
        m_state.rotation.X += 2_deg;
    }
    applyShift( collisionInfo );
    collisionInfo.oldPosition = m_state.position.position;
}

void LaraNode::updateLarasWeaponsStatus()
{
    if( leftArm.flashTimeout > 0 )
    {
        --leftArm.flashTimeout;
    }
    if( rightArm.flashTimeout > 0 )
    {
        --rightArm.flashTimeout;
    }
    bool doHolsterUpdate = false;
    if( m_state.health <= 0 )
    {
        m_handStatus = HandStatus::None;
    }
    else
    {
        if( m_underwaterState != UnderwaterState::OnLand )
        {
            if( m_handStatus == HandStatus::Combat )
            {
                doHolsterUpdate = true;
            }
        }
        else if( requestedGunType == gunType )
        {
            if( getLevel().m_inputHandler->getInputState().holster )
            {
                doHolsterUpdate = true;
            }
        }
        else if( m_handStatus == HandStatus::Combat )
        {
            doHolsterUpdate = true;
        }
        else if( m_handStatus == HandStatus::None )
        {
            gunType = requestedGunType;
            unholster();
            doHolsterUpdate = true;
        }
    }

    if( doHolsterUpdate && gunType != WeaponId::None )
    {
        if( m_handStatus == HandStatus::None )
        {
            rightArm.frame = 0;
            leftArm.frame = 0;
            m_handStatus = HandStatus::Unholster;
        }
        else if( m_handStatus == HandStatus::Combat )
        {
            m_handStatus = HandStatus::Holster;
        }
    }

    if( m_handStatus == HandStatus::Unholster )
    {
        if( gunType >= WeaponId::Pistols )
        {
            if( gunType <= WeaponId::Uzi )
            {
                if( getLevel().m_cameraController->getMode() != CameraMode::Cinematic
                    && getLevel().m_cameraController->getMode() != CameraMode::FreeLook )
                {
                    getLevel().m_cameraController->setMode( CameraMode::Combat );
                }
                unholsterGuns( gunType );
            }
            else if( gunType == WeaponId::Shotgun )
            {
                if( getLevel().m_cameraController->getMode() != CameraMode::Cinematic
                    && getLevel().m_cameraController->getMode() != CameraMode::FreeLook )
                {
                    getLevel().m_cameraController->setMode( CameraMode::Combat );
                }
                unholsterShotgun();
            }
        }
    }
    else if( m_handStatus == HandStatus::Holster )
    {
        {
            const auto& normalLara = *getLevel().m_animatedModels[TR1ItemId::Lara];
            BOOST_ASSERT( normalLara.meshes.size() == getNode()->getChildren().size() );
            getNode()->getChild( 14 )->setDrawable( normalLara.models[14].get() );
        }

        if( gunType >= WeaponId::Pistols )
        {
            if( gunType <= WeaponId::Uzi )
            {
                holsterGuns( gunType );
            }
            else if( gunType == WeaponId::Shotgun )
            {
                holsterShotgun();
            }
        }
    }
    else if( m_handStatus == HandStatus::Combat )
    {
        {
            const auto& normalLara = *getLevel().m_animatedModels[TR1ItemId::Lara];
            BOOST_ASSERT( normalLara.meshes.size() == getNode()->getChildren().size() );
            getNode()->getChild( 14 )->setDrawable( normalLara.models[14].get() );
        }

        switch( gunType )
        {
            case WeaponId::Pistols:
                if( pistolsAmmo.ammo != 0 )
                {
                    if( getLevel().m_inputHandler->getInputState().action )
                    {
                        const auto& uziLara = *getLevel().m_animatedModels[TR1ItemId::LaraUzisAnim];
                        BOOST_ASSERT( uziLara.meshes.size() == getNode()->getChildren().size() );
                        getNode()->getChild( 14 )->setDrawable( uziLara.models[14].get() );
                    }
                }
                if( getLevel().m_cameraController->getMode() != CameraMode::Cinematic
                    && getLevel().m_cameraController->getMode() != CameraMode::FreeLook )
                {
                    getLevel().m_cameraController->setMode( CameraMode::Combat );
                }
                updateGuns( gunType );
                break;
            case WeaponId::AutoPistols:
                if( revolverAmmo.ammo != 0 )
                {
                    if( getLevel().m_inputHandler->getInputState().action )
                    {
                        const auto& uziLara = *getLevel().m_animatedModels[TR1ItemId::LaraUzisAnim];
                        BOOST_ASSERT( uziLara.meshes.size() == getNode()->getChildren().size() );
                        getNode()->getChild( 14 )->setDrawable( uziLara.models[14].get() );
                    }
                }
                if( getLevel().m_cameraController->getMode() != CameraMode::Cinematic
                    && getLevel().m_cameraController->getMode() != CameraMode::FreeLook )
                {
                    getLevel().m_cameraController->setMode( CameraMode::Combat );
                }
                updateGuns( gunType );
                break;
            case WeaponId::Uzi:
                if( uziAmmo.ammo != 0 )
                {
                    if( getLevel().m_inputHandler->getInputState().action )
                    {
                        const auto& uziLara = *getLevel().m_animatedModels[TR1ItemId::LaraUzisAnim];
                        BOOST_ASSERT( uziLara.meshes.size() == getNode()->getChildren().size() );
                        getNode()->getChild( 14 )->setDrawable( uziLara.models[14].get() );
                    }
                }
                if( getLevel().m_cameraController->getMode() != CameraMode::Cinematic
                    && getLevel().m_cameraController->getMode() != CameraMode::FreeLook )
                {
                    getLevel().m_cameraController->setMode( CameraMode::Combat );
                }
                updateGuns( gunType );
                break;
            case WeaponId::Shotgun:
                if( shotgunAmmo.ammo != 0 )
                {
                    if( getLevel().m_inputHandler->getInputState().action )
                    {
                        const auto& uziLara = *getLevel().m_animatedModels[TR1ItemId::LaraUzisAnim];
                        BOOST_ASSERT( uziLara.meshes.size() == getNode()->getChildren().size() );
                        getNode()->getChild( 14 )->setDrawable( uziLara.models[14].get() );
                    }
                }
                if( getLevel().m_cameraController->getMode() != CameraMode::Cinematic
                    && getLevel().m_cameraController->getMode() != CameraMode::FreeLook )
                {
                    getLevel().m_cameraController->setMode( CameraMode::Combat );
                }
                updateShotgun();
                break;
            default:
                return;
        }
    }
}

void LaraNode::updateShotgun()
{
    if( getLevel().m_inputHandler->getInputState().action )
    {
        updateAimingState( weapons[WeaponId::Shotgun] );
    }
    else
    {
        target = nullptr;
    }
    if( target == nullptr )
    {
        findTarget( weapons[WeaponId::Shotgun] );
    }
    updateAimAngles( weapons[WeaponId::Shotgun], leftArm );
    if( leftArm.aiming )
    {
        m_torsoRotation.X = leftArm.aimRotation.X / 2;
        m_torsoRotation.Y = leftArm.aimRotation.Y / 2;
        m_headRotation.X = 0_deg;
        m_headRotation.Y = 0_deg;
    }
    updateAnimShotgun();
}

void LaraNode::updateGuns(const WeaponId weaponId)
{
    BOOST_ASSERT( weapons.find( weaponId ) != weapons.end() );
    const auto weapon = &weapons[weaponId];
    if( getLevel().m_inputHandler->getInputState().action )
    {
        updateAimingState( *weapon );
    }
    else
    {
        target = nullptr;
    }
    if( target == nullptr )
    {
        findTarget( *weapon );
    }
    updateAimAngles( *weapon, leftArm );
    updateAimAngles( *weapon, rightArm );
    if( leftArm.aiming && !rightArm.aiming )
    {
        m_headRotation.Y = m_torsoRotation.Y = leftArm.aimRotation.Y / 2;
        m_headRotation.X = m_torsoRotation.X = leftArm.aimRotation.X / 2;
    }
    else if( rightArm.aiming && !leftArm.aiming )
    {
        m_headRotation.Y = m_torsoRotation.Y = rightArm.aimRotation.Y / 2;
        m_headRotation.X = m_torsoRotation.X = rightArm.aimRotation.X / 2;
    }
    else if( leftArm.aiming && rightArm.aiming )
    {
        m_headRotation.Y = m_torsoRotation.Y = (leftArm.aimRotation.Y + rightArm.aimRotation.Y) / 4;
        m_headRotation.X = m_torsoRotation.X = (leftArm.aimRotation.X + rightArm.aimRotation.X) / 4;
    }

    updateAnimNotShotgun( weaponId );
}

void LaraNode::updateAimingState(const Weapon& weapon)
{
    if( target == nullptr )
    {
        rightArm.aiming = false;
        leftArm.aiming = false;
        m_weaponTargetVector.X = 0_deg;
        m_weaponTargetVector.Y = 0_deg;
        return;
    }

    core::RoomBoundPosition gunPosition{m_state.position};
    gunPosition.position.Y -= weapon.gunHeight;
    auto enemyChestPos = getUpperThirdBBoxCtr( *target );
    auto targetVector = getVectorAngles( enemyChestPos.position - gunPosition.position );
    targetVector.X -= m_state.rotation.X;
    targetVector.Y -= m_state.rotation.Y;
    if( !CameraController::clampPosition( gunPosition, enemyChestPos, getLevel() ) )
    {
        rightArm.aiming = false;
        leftArm.aiming = false;
    }
    else if( targetVector.Y < weapon.lockAngles.y.min
             || targetVector.Y > weapon.lockAngles.y.max
             || targetVector.X < weapon.lockAngles.x.min
             || targetVector.X > weapon.lockAngles.x.max )
    {
        if( leftArm.aiming )
        {
            if( targetVector.Y < weapon.leftAngles.y.min
                || targetVector.Y > weapon.leftAngles.y.max
                || targetVector.X < weapon.leftAngles.x.min
                || targetVector.X > weapon.leftAngles.x.max )
            {
                leftArm.aiming = false;
            }
        }
        if( rightArm.aiming )
        {
            if( targetVector.Y < weapon.rightAngles.y.min
                || targetVector.Y > weapon.rightAngles.y.max
                || targetVector.X < weapon.rightAngles.x.min
                || targetVector.X > weapon.rightAngles.x.max )
            {
                rightArm.aiming = false;
            }
        }
    }
    else
    {
        rightArm.aiming = true;
        leftArm.aiming = true;
    }
    m_weaponTargetVector = targetVector;
}

void LaraNode::unholster()
{
    rightArm.frame = 0;
    leftArm.frame = 0;
    leftArm.aimRotation.Y = 0_deg;
    leftArm.aimRotation.X = 0_deg;
    rightArm.aimRotation.Y = 0_deg;
    rightArm.aimRotation.X = 0_deg;
    rightArm.aiming = false;
    leftArm.aiming = false;
    rightArm.flashTimeout = 0;
    leftArm.flashTimeout = 0;
    target = nullptr;
    if( gunType == WeaponId::None )
    {
        const auto* positionData = getLevel().m_animatedModels[TR1ItemId::Lara]->frames;

        rightArm.weaponAnimData = positionData;
        leftArm.weaponAnimData = positionData;
    }
    else if( gunType == WeaponId::Pistols || gunType == WeaponId::AutoPistols || gunType == WeaponId::Uzi )
    {
        const auto* positionData = getLevel().m_animatedModels[TR1ItemId::LaraPistolsAnim]->frames;

        rightArm.weaponAnimData = positionData;
        leftArm.weaponAnimData = positionData;

        if( m_handStatus != HandStatus::None )
        {
            overrideLaraMeshesUnholsterGuns( gunType );
        }
    }
    else if( gunType == WeaponId::Shotgun )
    {
        const auto* positionData = getLevel().m_animatedModels[TR1ItemId::LaraShotgunAnim]->frames;

        rightArm.weaponAnimData = positionData;
        leftArm.weaponAnimData = positionData;

        if( m_handStatus != HandStatus::None )
        {
            overrideLaraMeshesUnholsterShotgun();
        }
    }
    else
    {
        const auto* positionData = getLevel().m_animatedModels[TR1ItemId::Lara]->frames;

        rightArm.weaponAnimData = positionData;
        leftArm.weaponAnimData = positionData;
    }
}

core::RoomBoundPosition LaraNode::getUpperThirdBBoxCtr(const ModelItemNode& item)
{
    const auto kf = item.getSkeleton()->getInterpolationInfo( item.m_state ).getNearestFrame();

    const auto ctrX = (kf->bbox.minX + kf->bbox.maxX) / 2;
    const auto ctrZ = (kf->bbox.minZ + kf->bbox.maxZ) / 2;
    const auto ctrY3 = (kf->bbox.maxY - kf->bbox.minY) / 3 + kf->bbox.minY;

    const auto cos = item.m_state.rotation.Y.cos();
    const auto sin = item.m_state.rotation.Y.sin();

    core::RoomBoundPosition result{item.m_state.position};
    result.position.X += ctrZ * sin + ctrX * cos;
    result.position.Y += ctrY3;
    result.position.Z += ctrZ * cos - ctrX * sin;
    return result;
}

void LaraNode::unholsterGuns(const WeaponId weaponId)
{
    auto nextFrame = leftArm.frame + 1;
    if( nextFrame < 5 || nextFrame > 23 )
    {
        nextFrame = 5;
    }
    else if( nextFrame == 13 )
    {
        overrideLaraMeshesUnholsterGuns( weaponId );
        playSoundEffect( TR1SoundId::LaraUnholster );
    }
    else if( nextFrame == 23 )
    {
        initAimInfoPistol();
        nextFrame = 0;
    }

    leftArm.frame = nextFrame;
    rightArm.frame = nextFrame;
}

void LaraNode::findTarget(const Weapon& weapon)
{
    core::RoomBoundPosition gunPosition{m_state.position};
    gunPosition.position.Y -= weapons[WeaponId::Shotgun].gunHeight;
    std::shared_ptr<ModelItemNode> bestEnemy = nullptr;
    core::Angle bestYAngle{std::numeric_limits<int16_t>::max()};
    for( const auto& currentEnemy : getLevel().m_itemNodes | boost::adaptors::map_values )
    {
        if( currentEnemy->m_state.health <= 0 || currentEnemy.get() == getLevel().m_lara )
            continue;

        const auto modelEnemy = std::dynamic_pointer_cast<ModelItemNode>( currentEnemy.get() );
        if( modelEnemy == nullptr )
        {
            BOOST_LOG_TRIVIAL( warning ) << "Ignoring non-model item " << currentEnemy->getNode()->getId();
            continue;
        }

        if( !modelEnemy->getNode()->isVisible() )
            continue;

        const auto d = currentEnemy->m_state.position.position - gunPosition.position;
        if( std::abs( d.X ) > weapon.targetDist )
            continue;

        if( std::abs( d.Y ) > weapon.targetDist )
            continue;

        if( std::abs( d.Z ) > weapon.targetDist )
            continue;

        if( util::square( d.X ) + util::square( d.Y ) + util::square( d.Z ) >= util::square( weapon.targetDist ) )
            continue;

        auto target = getUpperThirdBBoxCtr( *std::dynamic_pointer_cast<const ModelItemNode>( currentEnemy.get() ) );
        if( !CameraController::clampPosition( gunPosition, target, getLevel() ) )
            continue;

        auto aimAngle = getVectorAngles( target.position - gunPosition.position );
        aimAngle.X -= m_torsoRotation.X + m_state.rotation.X;
        aimAngle.Y -= m_torsoRotation.Y + m_state.rotation.Y;
        if( aimAngle.Y < weapon.lockAngles.y.min || aimAngle.Y > weapon.lockAngles.y.max
            || aimAngle.X < weapon.lockAngles.x.min || aimAngle.X > weapon.lockAngles.x.max )
            continue;

        const auto absY = abs( aimAngle.Y );
        if( absY >= bestYAngle )
            continue;

        bestYAngle = absY;
        bestEnemy = modelEnemy;
    }
    target = bestEnemy;
    updateAimingState( weapon );
}

void LaraNode::initAimInfoPistol()
{
    m_handStatus = HandStatus::Combat;
    leftArm.aimRotation.Y = 0_deg;
    leftArm.aimRotation.X = 0_deg;
    rightArm.aimRotation.Y = 0_deg;
    rightArm.aimRotation.X = 0_deg;
    rightArm.frame = 0;
    leftArm.frame = 0;
    rightArm.aiming = false;
    leftArm.aiming = false;
    m_torsoRotation.Y = 0_deg;
    m_torsoRotation.X = 0_deg;
    m_headRotation.Y = 0_deg;
    m_headRotation.X = 0_deg;
    target = nullptr;

    rightArm.weaponAnimData = getLevel().m_animatedModels[TR1ItemId::LaraPistolsAnim]->frames;
    leftArm.weaponAnimData = rightArm.weaponAnimData;
}

void LaraNode::initAimInfoShotgun()
{
    m_handStatus = HandStatus::Combat;
    leftArm.aimRotation.Y = 0_deg;
    leftArm.aimRotation.X = 0_deg;
    rightArm.aimRotation.Y = 0_deg;
    rightArm.aimRotation.X = 0_deg;
    rightArm.frame = 0;
    leftArm.frame = 0;
    rightArm.aiming = false;
    leftArm.aiming = false;
    m_torsoRotation.Y = 0_deg;
    m_torsoRotation.X = 0_deg;
    m_headRotation.Y = 0_deg;
    m_headRotation.X = 0_deg;
    target = nullptr;

    rightArm.weaponAnimData = getLevel().m_animatedModels[TR1ItemId::LaraShotgunAnim]->frames;
    leftArm.weaponAnimData = rightArm.weaponAnimData;
}

void LaraNode::overrideLaraMeshesUnholsterGuns(const WeaponId weaponId)
{
    TR1ItemId id;
    if( weaponId == WeaponId::AutoPistols )
    {
        id = TR1ItemId::LaraMagnumsAnim;
    }
    else if( weaponId == WeaponId::Uzi )
    {
        id = TR1ItemId::LaraUzisAnim;
    }
    else
    {
        id = TR1ItemId::LaraPistolsAnim;
    }

    const auto& src = *getLevel().m_animatedModels[id];
    BOOST_ASSERT( src.meshes.size() == getNode()->getChildren().size() );
    const auto& normalLara = *getLevel().m_animatedModels[TR1ItemId::Lara];
    BOOST_ASSERT( normalLara.meshes.size() == getNode()->getChildren().size() );
    getNode()->getChild( 1 )->setDrawable( normalLara.models[1].get() );
    getNode()->getChild( 4 )->setDrawable( normalLara.models[4].get() );
    getNode()->getChild( 10 )->setDrawable( src.models[10].get() );
    getNode()->getChild( 13 )->setDrawable( src.models[13].get() );
}

void LaraNode::overrideLaraMeshesUnholsterShotgun()
{
    const auto& src = *getLevel().m_animatedModels[TR1ItemId::LaraShotgunAnim];
    BOOST_ASSERT( src.meshes.size() == getNode()->getChildren().size() );
    const auto& normalLara = *getLevel().m_animatedModels[TR1ItemId::Lara];
    BOOST_ASSERT( normalLara.meshes.size() == getNode()->getChildren().size() );
    getNode()->getChild( 7 )->setDrawable( normalLara.models[7].get() );
    getNode()->getChild( 10 )->setDrawable( src.models[10].get() );
    getNode()->getChild( 13 )->setDrawable( src.models[13].get() );
}

void LaraNode::unholsterShotgun()
{
    auto nextFrame = leftArm.frame + 1;
    if( nextFrame < 5 || nextFrame > 47 )
    {
        nextFrame = 13;
    }
    else if( nextFrame == 23 )
    {
        overrideLaraMeshesUnholsterShotgun();

        playSoundEffect( TR1SoundId::LaraUnholster );
    }
    else if( nextFrame == 47 )
    {
        initAimInfoShotgun();
        nextFrame = 0;
    }

    leftArm.frame = nextFrame;
    rightArm.frame = nextFrame;
}

void LaraNode::updateAimAngles(const Weapon& weapon, AimInfo& aimInfo) const
{
    core::TRRotationXY target{0_deg, 0_deg};
    if( aimInfo.aiming )
    {
        target = m_weaponTargetVector;
    }

    if( aimInfo.aimRotation.X >= target.X - weapon.aimSpeed && aimInfo.aimRotation.X <= target.X + weapon.aimSpeed )
    {
        aimInfo.aimRotation.X = target.X;
    }
    else if( aimInfo.aimRotation.X >= target.X )
    {
        aimInfo.aimRotation.X -= weapon.aimSpeed;
    }
    else
    {
        aimInfo.aimRotation.X += weapon.aimSpeed;
    }

    if( aimInfo.aimRotation.Y >= target.Y - weapon.aimSpeed && aimInfo.aimRotation.Y <= weapon.aimSpeed + target.Y )
    {
        aimInfo.aimRotation.Y = target.Y;
    }
    else if( aimInfo.aimRotation.Y >= target.Y )
    {
        aimInfo.aimRotation.Y -= weapon.aimSpeed;
    }
    else
    {
        aimInfo.aimRotation.Y += weapon.aimSpeed;
    }
}

void LaraNode::updateAnimShotgun()
{
    auto aimingFrame = leftArm.frame;
    if( leftArm.aiming )
    {
        if( leftArm.frame < 0 || leftArm.frame >= 13 )
        {
            const auto nextFrame = leftArm.frame + 1;
            if( leftArm.frame == 47 )
            {
                if( getLevel().m_inputHandler->getInputState().action )
                {
                    tryShootShotgun();
                    rightArm.frame = nextFrame;
                    leftArm.frame = nextFrame;
                    return;
                }
            }
            else if( leftArm.frame <= 47 || leftArm.frame >= 80 )
            {
                if( leftArm.frame >= 114 && leftArm.frame < 127 )
                {
                    aimingFrame = leftArm.frame + 1;
                    if( leftArm.frame == 126 )
                    {
                        rightArm.frame = 0;
                        leftArm.frame = 0;
                        return;
                    }
                }
            }
            else
            {
                aimingFrame = leftArm.frame + 1;
                if( nextFrame == 80 )
                {
                    rightArm.frame = 47;
                    leftArm.frame = 47;
                    return;
                }
                if( nextFrame == 57 )
                {
                    playSoundEffect( TR1SoundId::LaraPistolsCocking );
                    rightArm.frame = aimingFrame;
                    leftArm.frame = aimingFrame;
                    return;
                }
            }
        }
        else
        {
            aimingFrame = leftArm.frame + 1;
            if( leftArm.frame == 12 )
            {
                aimingFrame = 47;
            }
        }

        rightArm.frame = aimingFrame;
        leftArm.frame = aimingFrame;
        return;
    }

    if( leftArm.frame == 0 && getLevel().m_inputHandler->getInputState().action )
    {
        rightArm.frame = leftArm.frame++ + 1;
        return;
    }

    if( leftArm.frame <= 0 || leftArm.frame >= 13 )
    {
        const auto nextFrame = leftArm.frame + 1;
        if( leftArm.frame == 47 )
        {
            if( getLevel().m_inputHandler->getInputState().action )
            {
                tryShootShotgun();
                rightArm.frame = aimingFrame + 1;
                leftArm.frame = aimingFrame + 1;
                return;
            }

            rightArm.frame = 114;
            leftArm.frame = 114;
            return;
        }
        if( leftArm.frame <= 47 || leftArm.frame >= 80 )
        {
            if( leftArm.frame >= 114 && leftArm.frame < 127 )
            {
                aimingFrame = leftArm.frame + 1;
                if( leftArm.frame == 126 )
                {
                    aimingFrame = 0;
                }
            }
        }
        else
        {
            aimingFrame = leftArm.frame + 1;
            if( nextFrame == 60 )
            {
                rightArm.frame = 0;
                leftArm.frame = 0;
                return;
            }
            if( nextFrame == 80 )
            {
                rightArm.frame = 114;
                leftArm.frame = 114;
                return;
            }
            if( nextFrame == 57 )
            {
                playSoundEffect( TR1SoundId::LaraPistolsCocking );
                rightArm.frame = aimingFrame;
                leftArm.frame = aimingFrame;
                return;
            }
        }
    }
    else
    {
        aimingFrame = leftArm.frame + 1;
        if( leftArm.frame == 12 )
        {
            if( getLevel().m_inputHandler->getInputState().action )
            {
                rightArm.frame = 47;
                leftArm.frame = 47;
                return;
            }

            rightArm.frame = 114;
            leftArm.frame = 114;
            return;
        }
    }

    rightArm.frame = aimingFrame;
    leftArm.frame = aimingFrame;
}

void LaraNode::tryShootShotgun()
{
    bool fireShotgun = false;
    for( int i = 0; i < 5; ++i )
    {
        const auto rand1 = util::rand15s();
        core::TRRotationXY aimAngle;
        aimAngle.Y = (+20_deg * rand1 / 65536) + m_state.rotation.Y + leftArm.aimRotation.Y;
        const auto rand2 = util::rand15s();
        aimAngle.X = (+20_deg * rand2 / 65536) + leftArm.aimRotation.X;
        if( fireWeapon( WeaponId::Shotgun, target, *this, aimAngle ) )
        {
            fireShotgun = true;
        }
    }
    if( fireShotgun )
    {
        playSoundEffect( weapons[WeaponId::Shotgun].sampleNum );
    }
}

void LaraNode::holsterShotgun()
{
    auto aimFrame = leftArm.frame;
    if( leftArm.frame == 0 )
    {
        aimFrame = 80;
    }
    else if( leftArm.frame >= 0 && leftArm.frame < 13 )
    {
        aimFrame = leftArm.frame + 1;
        if( leftArm.frame == 12 )
        {
            aimFrame = 114;
        }
    }
    else if( leftArm.frame == 47 )
    {
        aimFrame = 114;
    }
    else if( leftArm.frame >= 47 && leftArm.frame < 80 )
    {
        aimFrame = leftArm.frame + 1;
        if( leftArm.frame == 59 )
        {
            aimFrame = 0;
        }
        else if( aimFrame == 80 )
        {
            aimFrame = 114;
        }
    }
    else if( leftArm.frame >= 114 && leftArm.frame < 127 )
    {
        aimFrame = leftArm.frame + 1;
        if( leftArm.frame == 126 )
        {
            aimFrame = 80;
        }
    }
    else if( leftArm.frame >= 80 && leftArm.frame < 114 )
    {
        aimFrame = leftArm.frame + 1;
        if( leftArm.frame == 100 )
        {
            const auto& src = *getLevel().m_animatedModels[TR1ItemId::Lara];
            BOOST_ASSERT( src.meshes.size() == getNode()->getChildren().size() );
            const auto& normalLara = *getLevel().m_animatedModels[TR1ItemId::Lara];
            BOOST_ASSERT( normalLara.meshes.size() == getNode()->getChildren().size() );
            getNode()->getChild( 7 )->setDrawable( src.models[7].get() );
            getNode()->getChild( 10 )->setDrawable( normalLara.models[10].get() );
            getNode()->getChild( 13 )->setDrawable( normalLara.models[13].get() );

            playSoundEffect( TR1SoundId::LaraUnholster );
        }
        else if( leftArm.frame == 113 )
        {
            aimFrame = 0;
            m_handStatus = HandStatus::None;
            target = nullptr;
            rightArm.aiming = false;
            leftArm.aiming = false;
        }
    }

    rightArm.frame = aimFrame;
    leftArm.frame = aimFrame;

    m_torsoRotation.X /= 2;
    m_torsoRotation.Y /= 2;
    m_headRotation.X = 0_deg;
    m_headRotation.Y = 0_deg;
}

void LaraNode::holsterGuns(const WeaponId weaponId)
{
    if( leftArm.frame >= 24 )
    {
        leftArm.frame = 4;
    }
    else if( leftArm.frame > 0 && leftArm.frame < 5 )
    {
        leftArm.aimRotation.X -= leftArm.aimRotation.X / leftArm.frame;
        leftArm.aimRotation.Y -= leftArm.aimRotation.Y / leftArm.frame;
        --leftArm.frame;
    }
    else if( leftArm.frame == 0 )
    {
        leftArm.aimRotation.X = 0_deg;
        leftArm.aimRotation.Y = 0_deg;
        leftArm.frame = 23;
    }
    else if( leftArm.frame > 5 && leftArm.frame < 24 )
    {
        --leftArm.frame;
        if( leftArm.frame == 12 )
        {
            TR1ItemId srcId = TR1ItemId::LaraPistolsAnim;
            if( weaponId == WeaponId::AutoPistols )
            {
                srcId = TR1ItemId::LaraMagnumsAnim;
            }
            else if( weaponId == WeaponId::Uzi )
            {
                srcId = TR1ItemId::LaraUzisAnim;
            }

            const auto& src = *getLevel().m_animatedModels[srcId];
            BOOST_ASSERT( src.meshes.size() == getNode()->getChildren().size() );
            const auto& normalLara = *getLevel().m_animatedModels[TR1ItemId::Lara];
            BOOST_ASSERT( normalLara.meshes.size() == getNode()->getChildren().size() );
            getNode()->getChild( 1 )->setDrawable( src.models[1].get() );
            getNode()->getChild( 13 )->setDrawable( normalLara.models[13].get() );

            playSoundEffect( TR1SoundId::LaraHolster );
        }
    }

    if( rightArm.frame >= 24 )
    {
        rightArm.frame = 4;
    }
    else if( rightArm.frame > 0 && rightArm.frame < 5 )
    {
        rightArm.aimRotation.X -= rightArm.aimRotation.X / rightArm.frame;
        rightArm.aimRotation.Y -= rightArm.aimRotation.Y / rightArm.frame;
        --rightArm.frame;
    }
    else if( rightArm.frame == 0 )
    {
        rightArm.aimRotation.Y = 0_deg;
        rightArm.aimRotation.X = 0_deg;
        rightArm.frame = 23;
    }
    else if( rightArm.frame > 5 && rightArm.frame < 24 )
    {
        --rightArm.frame;
        if( rightArm.frame == 12 )
        {
            TR1ItemId srcId = TR1ItemId::LaraPistolsAnim;
            if( weaponId == WeaponId::AutoPistols )
            {
                srcId = TR1ItemId::LaraMagnumsAnim;
            }
            else if( weaponId == WeaponId::Uzi )
            {
                srcId = TR1ItemId::LaraUzisAnim;
            }

            const auto& src = *getLevel().m_animatedModels[srcId];
            BOOST_ASSERT( src.meshes.size() == getNode()->getChildren().size() );
            const auto& normalLara = *getLevel().m_animatedModels[TR1ItemId::Lara];
            BOOST_ASSERT( normalLara.meshes.size() == getNode()->getChildren().size() );
            getNode()->getChild( 4 )->setDrawable( src.models[4].get() );
            getNode()->getChild( 10 )->setDrawable( normalLara.models[10].get() );

            playSoundEffect( TR1SoundId::LaraHolster );
        }
    }

    if( leftArm.frame == 5 && rightArm.frame == 5 )
    {
        m_handStatus = HandStatus::None;
        leftArm.frame = 0;
        rightArm.frame = 0;
        target = nullptr;
        rightArm.aiming = false;
        leftArm.aiming = false;
    }

    m_headRotation.X = (rightArm.aimRotation.X + leftArm.aimRotation.X) / 4;
    m_headRotation.Y = rightArm.aimRotation.Y / 4;
    m_torsoRotation.X = (rightArm.aimRotation.X + leftArm.aimRotation.X) / 4;
    m_torsoRotation.Y = rightArm.aimRotation.Y / 4;
}

void LaraNode::updateAnimNotShotgun(const WeaponId weaponId)
{
    const auto& weapon = weapons[weaponId];

    if( !rightArm.aiming && (!getLevel().m_inputHandler->getInputState().action || target != nullptr) )
    {
        if( rightArm.frame >= 24 )
        {
            rightArm.frame = 4;
        }
        else if( rightArm.frame > 0 && rightArm.frame <= 4 )
        {
            --rightArm.frame;
        }
    }
    else if( rightArm.frame >= 0 && rightArm.frame < 4 )
    {
        ++rightArm.frame;
    }
    else if( getLevel().m_inputHandler->getInputState().action && rightArm.frame == 4 )
    {
        core::TRRotationXY aimAngle;
        aimAngle.X = rightArm.aimRotation.X;
        aimAngle.Y = m_state.rotation.Y + rightArm.aimRotation.Y;
        if( fireWeapon( weaponId, target, *this, aimAngle ) )
        {
            rightArm.flashTimeout = weapon.flashTime;
            playSoundEffect( weapon.sampleNum );
        }
        rightArm.frame = 24;
    }
    else if( rightArm.frame >= 24 )
    {
        ++rightArm.frame;
        if( rightArm.frame == weapon.recoilFrame + 24 )
        {
            rightArm.frame = 4;
        }
    }

    if( !leftArm.aiming && (!getLevel().m_inputHandler->getInputState().action || target != nullptr) )
    {
        if( leftArm.frame >= 24 )
        {
            leftArm.frame = 4;
        }
        else if( leftArm.frame > 0 && leftArm.frame <= 4 )
        {
            --leftArm.frame;
        }
    }
    else if( leftArm.frame >= 0 && leftArm.frame < 4 )
    {
        ++leftArm.frame;
    }
    else if( getLevel().m_inputHandler->getInputState().action && leftArm.frame == 4 )
    {
        core::TRRotationXY aimAngle;
        aimAngle.Y = m_state.rotation.Y + leftArm.aimRotation.Y;
        aimAngle.X = leftArm.aimRotation.X;
        if( fireWeapon( weaponId, target, *this, aimAngle ) )
        {
            leftArm.flashTimeout = weapon.flashTime;
            playSoundEffect( weapon.sampleNum );
        }
        leftArm.frame = 24;
    }
    else if( leftArm.frame >= 24 )
    {
        ++leftArm.frame;
        if( leftArm.frame == weapon.recoilFrame + 24 )
        {
            leftArm.frame = 4;
        }
    }
}

bool LaraNode::fireWeapon(const WeaponId weaponId,
                          const std::shared_ptr<ModelItemNode>& target,
                          const ModelItemNode& gunHolder,
                          const core::TRRotationXY& aimAngle)
{
    Expects( weaponId != WeaponId::None );

    Ammo* ammoPtr;
    if( weaponId == WeaponId::Pistols )
    {
        ammoPtr = &pistolsAmmo;
        pistolsAmmo.ammo = 1000;
    }
    else if( weaponId == WeaponId::AutoPistols )
    {
        ammoPtr = &revolverAmmo;
        if( true /* FIXME engine::allAmmoCheat */ )
        {
            revolverAmmo.ammo = 1000;
        }
    }
    else if( weaponId == WeaponId::Uzi )
    {
        ammoPtr = &uziAmmo;
        if( true /* FIXME engine::allAmmoCheat */ )
        {
            uziAmmo.ammo = 1000;
        }
    }
    else if( weaponId == WeaponId::Shotgun )
    {
        ammoPtr = &shotgunAmmo;
        if( true /* FIXME engine::allAmmoCheat */ )
        {
            shotgunAmmo.ammo = 1000;
        }
    }
    else
    {
        BOOST_THROW_EXCEPTION( std::out_of_range( "weaponId" ) );
    }

    if( ammoPtr->ammo <= 0 )
    {
        ammoPtr->ammo = 0;
        playSoundEffect( TR1SoundId::EmptyAmmo );
        requestedGunType = WeaponId::Pistols;
        return false;
    }

    --ammoPtr->ammo;
    const auto weapon = &weapons[weaponId];
    core::TRVec gunPosition = gunHolder.m_state.position.position;
    gunPosition.Y -= weapon->gunHeight;
    core::TRRotation shootVector{
            util::rand15s( weapon->shotAccuracy / 2 ) + aimAngle.X,
            util::rand15s( weapon->shotAccuracy / 2 ) + aimAngle.Y,
            +0_deg
    };

    std::vector<SkeletalModelNode::Sphere> spheres;
    if( target != nullptr )
    {
        spheres = target->getSkeleton()->getBoneCollisionSpheres( target->m_state,
                                                                  *target->getSkeleton()
                                                                         ->getInterpolationInfo( target->m_state )
                                                                         .getNearestFrame(),
                                                                  nullptr );
    }
    bool hasHit = false;
    glm::vec3 hitPos;
    const auto bulletDir = normalize( glm::vec3( shootVector.toMatrix()[2] ) ); // +Z is our shooting direction
    if( !spheres.empty() )
    {
        float minD = std::numeric_limits<float>::max();
        for( const auto& sphere : spheres )
        {
            hitPos = gunPosition.toRenderSystem()
                     + bulletDir * dot( sphere.getPosition() - gunPosition.toRenderSystem(), bulletDir );

            const auto d = length( hitPos - sphere.getPosition() );
            if( d > sphere.radius || d >= minD )
                continue;

            minD = d;
            hasHit = true;
        }
    }

    if( !hasHit )
    {
        ++ammoPtr->misses;

        static constexpr float VeryLargeDistanceProbablyClipping = 1 << 14;

        core::RoomBoundPosition aimHitPos{
                gunHolder.m_state.position.room,
                gunPosition + core::TRVec{-bulletDir * VeryLargeDistanceProbablyClipping}
        };

        const core::RoomBoundPosition bulletPos{gunHolder.m_state.position.room, gunPosition};
        CameraController::clampPosition( bulletPos, aimHitPos, getLevel() );
        playShotMissed( aimHitPos );
    }
    else
    {
        BOOST_ASSERT( target != nullptr );
        ++ammoPtr->hits;
        hitTarget( *target, core::TRVec{hitPos}, weapon->damage );
    }

    return true;
}

void LaraNode::hitTarget(ModelItemNode& item, const core::TRVec& hitPos, const int damage)
{
    if( item.m_state.health > 0 && item.m_state.health <= damage )
    {
        // TODO ++g_numKills;
    }
    item.m_state.is_hit = true;
    item.m_state.health -= damage;
    auto fx = createBloodSplat( getLevel(),
                                core::RoomBoundPosition{item.m_state.position.room, hitPos},
                                item.m_state.speed,
                                item.m_state.rotation.Y );
    getLevel().m_particles.emplace_back( fx );
    if( item.m_state.health <= 0 )
        return;

    TR1SoundId soundId;
    switch( item.m_state.type )
    {
        case TR1ItemId::Wolf:
            soundId = TR1SoundId::WolfHurt;
            break;
        case TR1ItemId::Bear:
            soundId = TR1SoundId::BearHurt;
            break;
        case TR1ItemId::LionMale:
        case TR1ItemId::LionFemale:
            soundId = TR1SoundId::LionHurt;
            break;
        case TR1ItemId::RatOnLand:
            soundId = TR1SoundId::RatHurt;
            break;
        case TR1ItemId::SkateboardKid:
            soundId = TR1SoundId::SkateboardKidHurt;
            break;
        case TR1ItemId::TorsoBoss:
            soundId = TR1SoundId::TorsoBossHurt;
            break;
        default:
            return;
    }

    playSoundEffect( soundId );
}

namespace
{
class MatrixStack
{
private:
    std::stack<glm::mat4> m_stack;

public:
    explicit MatrixStack()
    {
        m_stack.push( glm::mat4{1.0f} );
    }

    void push()
    {
        m_stack.push( m_stack.top() );
    }

    void pop()
    {
        m_stack.pop();
    }

    const glm::mat4& top() const
    {
        return m_stack.top();
    }

    glm::mat4& top()
    {
        return m_stack.top();
    }

    void rotate(const glm::mat4& m)
    {
        m_stack.top() *= m;
    }

    void rotate(const core::TRRotation& r)
    {
        rotate( r.toMatrix() );
    }

    void rotate(const core::TRRotationXY& r)
    {
        rotate( r.toMatrix() );
    }

    void resetRotation()
    {
        top()[0] = glm::vec4{1, 0, 0, 0};
        top()[1] = glm::vec4{0, 1, 0, 0};
        top()[2] = glm::vec4{0, 0, 1, 0};
    }

    void rotate(const uint32_t packed)
    {
        m_stack.top() *= core::fromPackedAngles( packed );
    }

    void translate(const glm::vec3& c)
    {
        m_stack.top() = glm::translate( m_stack.top(), c );
    }

    void translate(const loader::BoneTreeEntry& bte)
    {
        translate( bte.toGl() );
    }

    void transform(const std::initializer_list<size_t>& indices,
                   const gsl::span<const loader::BoneTreeEntry>& boneTree,
                   const gsl::span<const uint32_t>& angleData,
                   const std::shared_ptr<SkeletalModelNode>& skeleton)
    {
        for( auto idx : indices )
            transform( idx, boneTree, angleData, skeleton );
    }

    void transform(const size_t idx,
                   const gsl::span<const loader::BoneTreeEntry>& boneTree,
                   const gsl::span<const uint32_t>& angleData,
                   const std::shared_ptr<SkeletalModelNode>& skeleton)
    {
        BOOST_ASSERT( idx > 0 );
        translate( boneTree[idx - 1] );
        rotate( angleData[idx] );
        apply( skeleton, idx );
    }

    void apply(const std::shared_ptr<SkeletalModelNode>& skeleton, const size_t idx)
    {
        BOOST_ASSERT( skeleton != nullptr );
        skeleton->getChild( idx )->setLocalMatrix( m_stack.top() );
    }
};


class DualMatrixStack
{
private:
    MatrixStack m_stack1{};
    MatrixStack m_stack2{};
    const float m_bias;

public:
    explicit DualMatrixStack(const float bias) : m_bias{bias}
    {
    }

    void push()
    {
        m_stack1.push();
        m_stack2.push();
    }

    void pop()
    {
        m_stack1.pop();
        m_stack2.pop();
    }

    glm::mat4 itop() const
    {
        return util::mix( m_stack1.top(), m_stack2.top(), m_bias );
    }

    void rotate(const glm::mat4& m)
    {
        m_stack1.top() *= m;
        m_stack2.top() *= m;
    }

    void rotate(const core::TRRotation& r)
    {
        rotate( r.toMatrix() );
    }

    void rotate(const core::TRRotationXY& r)
    {
        rotate( r.toMatrix() );
    }

    void rotate(const uint32_t packed1, const uint32_t packed2)
    {
        m_stack1.top() *= core::fromPackedAngles( packed1 );
        m_stack2.top() *= core::fromPackedAngles( packed2 );
    }

    void resetRotation()
    {
        m_stack1.resetRotation();
        m_stack2.resetRotation();
    }

    void translate(const glm::vec3& v1, const glm::vec3& v2)
    {
        m_stack1.top() = glm::translate( m_stack1.top(), v1 );
        m_stack2.top() = glm::translate( m_stack2.top(), v2 );
    }

    void translate(const loader::BoneTreeEntry& bte)
    {
        translate( bte.toGl(), bte.toGl() );
    }

    void transform(const std::initializer_list<size_t>& indices,
                   const gsl::span<const loader::BoneTreeEntry>& boneTree,
                   const gsl::span<const uint32_t>& angleData1,
                   const gsl::span<const uint32_t>& angleData2,
                   const std::shared_ptr<SkeletalModelNode>& skeleton)
    {
        for( auto idx : indices )
            transform( idx, boneTree, angleData1, angleData2, skeleton );
    }

    void transform(const size_t idx,
                   const gsl::span<const loader::BoneTreeEntry>& boneTree,
                   const gsl::span<const uint32_t>& angleData1,
                   const gsl::span<const uint32_t>& angleData2,
                   const std::shared_ptr<SkeletalModelNode>& skeleton)
    {
        BOOST_ASSERT( idx > 0 );
        translate( boneTree[idx - 1] );
        rotate( angleData1[idx], angleData2[idx] );
        apply( skeleton, idx );
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    void apply(const std::shared_ptr<SkeletalModelNode>& skeleton, const size_t idx)
    {
        BOOST_ASSERT( skeleton != nullptr );
        skeleton->getChild( idx )->setLocalMatrix( itop() );
    }
};
}

void LaraNode::drawRoutine()
{
    const auto interpolationInfo = getSkeleton()->getInterpolationInfo( m_state );
    if( !hit_direction.is_initialized() && interpolationInfo.firstFrame != interpolationInfo.secondFrame )
    {
        drawRoutineInterpolated( interpolationInfo );
        return;
    }

    const auto& objInfo = *getLevel().m_animatedModels[m_state.type];
    const loader::AnimFrame* frame;
    if( !hit_direction.is_initialized() )
    {
        frame = interpolationInfo.firstFrame;
    }
    else
    {
        switch( *hit_direction )
        {
            case core::Axis::PosX:
                frame = getLevel().m_animations[static_cast<int>(loader::AnimationId::AH_LEFT)].frames;
                break;
            case core::Axis::NegZ:
                frame = getLevel().m_animations[static_cast<int>(loader::AnimationId::AH_BACKWARD)].frames;
                break;
            case core::Axis::NegX:
                frame = getLevel().m_animations[static_cast<int>(loader::AnimationId::AH_RIGHT)].frames;
                break;
            default:
                frame = getLevel().m_animations[static_cast<int>(loader::AnimationId::AH_FORWARD)].frames;
                break;
        }
        frame = frame->next( hit_frame );
    }

    updateLighting();

    MatrixStack matrixStack;

    matrixStack.push();
    matrixStack.translate( frame->pos.toGl() );
    const auto angleData = frame->getAngleData();
    matrixStack.rotate( angleData[0] );
    matrixStack.apply( getSkeleton(), 0 );

    matrixStack.push();
    matrixStack.transform( {1, 2, 3}, objInfo.boneTree, angleData, getSkeleton() );

    matrixStack.pop();
    matrixStack.push();
    matrixStack.transform( {4, 5, 6}, objInfo.boneTree, angleData, getSkeleton() );

    matrixStack.pop();
    matrixStack.translate( objInfo.boneTree[6] );
    matrixStack.rotate( angleData[7] );
    matrixStack.rotate( m_torsoRotation );
    matrixStack.apply( getSkeleton(), 7 );

    matrixStack.push();
    matrixStack.translate( objInfo.boneTree[13] );
    matrixStack.rotate( angleData[14] );
    matrixStack.rotate( m_headRotation );
    matrixStack.apply( getSkeleton(), 14 );

    WeaponId activeGunType = WeaponId::None;
    if( m_handStatus == HandStatus::Combat
        || m_handStatus == HandStatus::Unholster
        || m_handStatus == HandStatus::Holster )
    {
        activeGunType = gunType;
    }

    matrixStack.pop();
    gsl::span<const uint32_t> armAngleData;
    switch( activeGunType )
    {
        case WeaponId::None:
            matrixStack.push();
            matrixStack.transform( {8, 9, 10}, objInfo.boneTree, angleData, getSkeleton() );

            matrixStack.pop();
            matrixStack.push();
            matrixStack.transform( {11, 12, 13}, objInfo.boneTree, angleData, getSkeleton() );
            break;
        case WeaponId::Pistols:
        case WeaponId::AutoPistols:
        case WeaponId::Uzi:
            matrixStack.push();
            matrixStack.translate( objInfo.boneTree[7] );
            matrixStack.resetRotation();
            matrixStack.rotate( rightArm.aimRotation );

            armAngleData = rightArm.weaponAnimData->next( rightArm.frame )->getAngleData();
            matrixStack.rotate( armAngleData[8] );
            matrixStack.apply( getSkeleton(), 8 );

            matrixStack.transform( 9, objInfo.boneTree, armAngleData, getSkeleton() );
            matrixStack.transform( 10, objInfo.boneTree, armAngleData, getSkeleton() );

            renderGunFlare( activeGunType, matrixStack.top(), m_gunFlareRight, rightArm.flashTimeout != 0 );
            matrixStack.pop();
            matrixStack.push();
            matrixStack.translate( objInfo.boneTree[10] );
            matrixStack.resetRotation();
            matrixStack.rotate( leftArm.aimRotation );
            armAngleData = leftArm.weaponAnimData->next( leftArm.frame )->getAngleData();
            matrixStack.rotate( armAngleData[11] );
            matrixStack.apply( getSkeleton(), 11 );

            matrixStack.transform( {12, 13}, objInfo.boneTree, armAngleData, getSkeleton() );

            renderGunFlare( activeGunType, matrixStack.top(), m_gunFlareLeft, leftArm.flashTimeout != 0 );
            break;
        case WeaponId::Shotgun:
            matrixStack.push();
            armAngleData = rightArm.weaponAnimData->next( rightArm.frame )->getAngleData();
            matrixStack.transform( {8, 9, 10}, objInfo.boneTree, armAngleData, getSkeleton() );

            matrixStack.pop();
            matrixStack.push();
            armAngleData = leftArm.weaponAnimData->next( leftArm.frame )->getAngleData();
            matrixStack.transform( {11, 12, 13}, objInfo.boneTree, armAngleData, getSkeleton() );
            break;
        default:
            break;
    }
}

void LaraNode::drawRoutineInterpolated(const SkeletalModelNode::InterpolationInfo& interpolationInfo)
{
    updateLighting();

    const auto& objInfo = *getLevel().m_animatedModels[m_state.type];

    DualMatrixStack matrixStack{interpolationInfo.bias};

    matrixStack.push();
    matrixStack.translate( interpolationInfo.firstFrame->pos.toGl(), interpolationInfo.secondFrame->pos.toGl() );
    const auto angleDataA = interpolationInfo.firstFrame->getAngleData();
    const auto angleDataB = interpolationInfo.secondFrame->getAngleData();
    matrixStack.rotate( angleDataA[0], angleDataB[0] );
    matrixStack.apply( getSkeleton(), 0 );

    matrixStack.push();
    matrixStack.transform( {1, 2, 3}, objInfo.boneTree, angleDataA, angleDataB, getSkeleton() );

    matrixStack.pop();
    matrixStack.push();
    matrixStack.transform( {4, 5, 6}, objInfo.boneTree, angleDataA, angleDataB, getSkeleton() );

    matrixStack.pop();
    matrixStack.translate( objInfo.boneTree[6] );
    matrixStack.rotate( angleDataA[7], angleDataB[7] );
    matrixStack.rotate( m_torsoRotation );
    matrixStack.apply( getSkeleton(), 7 );

    matrixStack.push();
    matrixStack.translate( objInfo.boneTree[13] );
    matrixStack.rotate( angleDataA[14], angleDataB[14] );
    matrixStack.rotate( m_headRotation );
    matrixStack.apply( getSkeleton(), 14 );

    WeaponId activeGunType = WeaponId::None;
    if( m_handStatus == HandStatus::Combat
        || m_handStatus == HandStatus::Unholster
        || m_handStatus == HandStatus::Holster )
    {
        activeGunType = gunType;
    }

    matrixStack.pop();
    gsl::span<const uint32_t> armAngleData;
    switch( activeGunType )
    {
        case WeaponId::None:
            matrixStack.push();
            matrixStack.transform( {8, 9, 10}, objInfo.boneTree, angleDataA, angleDataB, getSkeleton() );

            matrixStack.pop();
            matrixStack.push();
            matrixStack.transform( {11, 12, 13}, objInfo.boneTree, angleDataA, angleDataB, getSkeleton() );
            break;
        case WeaponId::Pistols:
        case WeaponId::AutoPistols:
        case WeaponId::Uzi:
            matrixStack.push();
            matrixStack.translate( objInfo.boneTree[7] );
            matrixStack.resetRotation();
            matrixStack.rotate( rightArm.aimRotation );

            armAngleData = rightArm.weaponAnimData->next( rightArm.frame )->getAngleData();
            matrixStack.rotate( armAngleData[8], armAngleData[8] );
            matrixStack.apply( getSkeleton(), 8 );

            matrixStack.transform( 9, objInfo.boneTree, armAngleData, armAngleData, getSkeleton() );
            matrixStack.transform( 10, objInfo.boneTree, armAngleData, armAngleData, getSkeleton() );

            renderGunFlare( activeGunType, matrixStack.itop(), m_gunFlareRight, rightArm.flashTimeout != 0 );
            matrixStack.pop();
            matrixStack.push();
            matrixStack.translate( objInfo.boneTree[10] );
            matrixStack.resetRotation();
            matrixStack.rotate( leftArm.aimRotation );
            armAngleData = leftArm.weaponAnimData->next( leftArm.frame )->getAngleData();
            matrixStack.rotate( armAngleData[11], armAngleData[11] );
            matrixStack.apply( getSkeleton(), 11 );

            matrixStack.transform( {12, 13}, objInfo.boneTree, armAngleData, armAngleData, getSkeleton() );

            renderGunFlare( activeGunType, matrixStack.itop(), m_gunFlareLeft, leftArm.flashTimeout != 0 );
            break;
        case WeaponId::Shotgun:
            matrixStack.push();
            armAngleData = rightArm.weaponAnimData->next( rightArm.frame )->getAngleData();
            matrixStack.transform( {8, 9, 10}, objInfo.boneTree, armAngleData, armAngleData, getSkeleton() );

            matrixStack.pop();
            matrixStack.push();
            armAngleData = leftArm.weaponAnimData->next( leftArm.frame )->getAngleData();
            matrixStack.transform( {11, 12, 13}, objInfo.boneTree, armAngleData, armAngleData, getSkeleton() );
            break;
        default:
            break;
    }
}

void LaraNode::renderGunFlare(const WeaponId weaponId,
                              glm::mat4 m,
                              const gsl::not_null<std::shared_ptr<gameplay::Node>>& flareNode,
                              const bool visible) const
{
    if( !visible )
    {
        flareNode->setVisible( false );
        return;
    }

    uint16_t shade;
    int dy;
    switch( weaponId )
    {
        case WeaponId::None:
        case WeaponId::Pistols:
            shade = 5120;
            dy = 155;
            break;
        case WeaponId::AutoPistols:
            shade = 4096;
            dy = 155;
            break;
        case WeaponId::Uzi:
            shade = 2560;
            dy = 180;
            break;
        case WeaponId::Shotgun:
            shade = 5120;
            dy = 155;
            break;
        default:
            BOOST_THROW_EXCEPTION( std::domain_error( "WeaponId" ) );
    }

    m = translate( m, core::TRVec{0, dy, 55}.toRenderSystem() );
    m *= core::TRRotation( -90_deg, 0_deg, core::Angle( 2 * util::rand15() ) ).toMatrix();

    flareNode->setVisible( true );
    setParent( flareNode, getNode()->getParent().lock() );
    flareNode->setLocalMatrix( getNode()->getLocalMatrix() * m );

    const auto brightness = util::clamp( 2.0f - shade / 8191.0f, 0.0f, 1.0f );
    flareNode->addMaterialParameterSetter( "u_baseLight", [brightness](const gameplay::Node& /*node*/,
                                                                       gameplay::gl::Program::ActiveUniform& uniform) {
        uniform.set( brightness );
    } );
    flareNode->addMaterialParameterSetter( "u_baseLightDiff", [](const gameplay::Node& /*node*/,
                                                                 gameplay::gl::Program::ActiveUniform& uniform) {
        uniform.set( 0.0f );
    } );
}

YAML::Node LaraNode::save() const
{
    YAML::Node node = ItemNode::save();
    node["gun"] = toString( gunType );
    node["requestedGun"] = toString( requestedGunType );
    node["handStatus"] = toString( m_handStatus );
    node["underwater"] = toString( m_underwaterState );
    node["hitFrame"] = hit_frame;
    if( hit_direction.is_initialized() )
    {
        node["hitDir"] = toString( *hit_direction );
    }

    node["air"] = m_air;
    node["swimToDiveKeypressDuration"] = m_swimToDiveKeypressDuration;
    node["explosionStumblingDuration"] = explosionStumblingDuration;
    if( forceSourcePosition != nullptr )
    {
        node["forceSource"] = forceSourcePosition->save();
    }

    node["yRotationSpeed"] = m_yRotationSpeed.toDegrees();
    node["movementAngle"] = m_movementAngle.toDegrees();
    node["headRot"] = m_headRotation.save();
    node["torsoRot"] = m_torsoRotation.save();

    node["ammo"]["pistols"] = pistolsAmmo.save();
    node["ammo"]["magnums"] = revolverAmmo.save();
    node["ammo"]["uzis"] = uziAmmo.save();
    node["ammo"]["shotgun"] = shotgunAmmo.save();

    node["underwaterCurrentStrength"] = m_underwaterCurrentStrength;
    node["underwaterRoute"] = m_underwaterRoute.save( getLevel() );

    node["leftArm"] = leftArm.save( getLevel() );
    node["rightArm"] = rightArm.save( getLevel() );
    node["weaponTargetVector"] = m_weaponTargetVector.save();

    return node;
}

void LaraNode::load(const YAML::Node& n)
{
    ItemNode::load( n );

    gunType = parseWeaponId( n["gun"].as<std::string>() );
    requestedGunType = parseWeaponId( n["requestedGun"].as<std::string>() );
    m_handStatus = parseHandStatus( n["handStatus"].as<std::string>() );
    m_underwaterState = parseUnderwaterState( n["underwater"].as<std::string>() );
    hit_frame = n["hitFrame"].as<int>();
    if( !n["hitDir"].IsDefined() )
        hit_direction.reset();
    else
        hit_direction = parseAxis( n["hitDir"].as<std::string>() );

    m_air = n["air"].as<int>();
    m_swimToDiveKeypressDuration = n["swimToDiveKeypressDuration"].as<int>();
    explosionStumblingDuration = n["explosionStumblingDuration"].as<int>();
    if( !n["forceSource"].IsDefined() )
        forceSourcePosition = nullptr;
    else
        BOOST_THROW_EXCEPTION( std::runtime_error( "Cannot load forceSourcePosition yet" ) ); // FIXME

    m_yRotationSpeed = core::Angle::fromDegrees( n["yRotationSpeed"].as<float>() );
    m_movementAngle = core::Angle::fromDegrees( n["movementAngle"].as<float>() );
    m_headRotation.load( n["headRot"] );
    m_torsoRotation.load( n["torsoRot"] );

    pistolsAmmo.load( n["ammo"]["pistols"] );
    revolverAmmo.load( n["ammo"]["magnums"] );
    uziAmmo.load( n["ammo"]["uzis"] );
    shotgunAmmo.load( n["ammo"]["shotgun"] );

    m_underwaterCurrentStrength = n["underwaterCurrentStrength"].as<int>();
    m_underwaterRoute.load( n["underwaterRoute"], getLevel() );

    leftArm.load( n["leftArm"], getLevel() );
    rightArm.load( n["rightArm"], getLevel() );
    m_weaponTargetVector.load( n["weaponTargetVector"] );
}

YAML::Node LaraNode::AimInfo::save(const level::Level& lvl) const
{
    YAML::Node node;
    if( weaponAnimData != nullptr )
        node["animData"] = std::distance( &lvl.m_poseFrames[0], reinterpret_cast<const int16_t*>(weaponAnimData) );
    node["frame"] = frame;
    node["aiming"] = aiming;
    node["aimRotation"] = aimRotation.save();
    node["flashTimeout"] = flashTimeout;
    return node;
}

void LaraNode::AimInfo::load(const YAML::Node& n, const level::Level& lvl)
{
    if( !n["animData"].IsDefined() )
        weaponAnimData = nullptr;
    else
        weaponAnimData = reinterpret_cast<const loader::AnimFrame*>(&lvl.m_poseFrames.at( n["animData"].as<size_t>() ));
    frame = n["frame"].as<int16_t>();
    aiming = n["aiming"].as<bool>();
    aimRotation.load( n["aimRotation"] );
    flashTimeout = n["flashTimeout"].as<int16_t>();
}
}
