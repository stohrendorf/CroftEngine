#include "laranode.h"

#include "cameracontroller.h"
#include "level/level.h"
#include "render/textureanimator.h"

#include "items/block.h"
#include "items/tallblock.h"
#include "items/aiagent.h"

#include <glm/gtx/norm.hpp>

#include <boost/range/adaptors.hpp>
#include <stack>

namespace
{
std::array<engine::floordata::ActivationState, 10> mapFlipActivationStates;

core::TRRotationXY getVectorAngles(const core::TRCoordinates& co)
{
    return core::getVectorAngles( co.X, co.Y, co.Z );
}
}

namespace engine
{
void LaraNode::setTargetState(LaraStateId st)
{
    m_state.goal_anim_state = static_cast<uint16_t>(st);
}

loader::LaraStateId LaraNode::getTargetState() const
{
    return static_cast<LaraStateId>(m_state.goal_anim_state);
}

void LaraNode::setAnimIdGlobal(loader::AnimationId anim, const boost::optional<uint16_t>& firstFrame)
{
    getSkeleton()->setAnimIdGlobal( m_state,
                                    to_not_null( &getLevel().m_animations.at( static_cast<uint16_t>(anim) ) ),
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

    setCameraCurrentRotationX( -22_deg );

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
    m_state.position.position.Y += collisionInfo.mid.floor.distance;
}

loader::LaraStateId LaraNode::getCurrentAnimState() const
{
    return static_cast<loader::LaraStateId>(m_state.current_anim_state);
}

LaraNode::~LaraNode() = default;

void LaraNode::update()
{
    if( m_underwaterState == UnderwaterState::OnLand && m_state.position.room->isWaterRoom() )
    {
        m_air = core::LaraAir;
        m_underwaterState = UnderwaterState::Diving;
        m_state.falling = false;
        const core::TRCoordinates& pos = m_state.position.position + core::TRCoordinates( 0, 100, 0 );
        m_state.position.position = pos;
        updateFloorHeight( 0 );
        getLevel().stopSoundEffect( 30 );
        if( getCurrentAnimState() == LaraStateId::SwandiveBegin )
        {
            m_state.rotation.X = -45_deg;
            setTargetState( LaraStateId::UnderwaterDiving );
            updateImpl();
            m_state.fallspeed *= 2;
        }
        else if( getCurrentAnimState() == LaraStateId::SwandiveEnd )
        {
            m_state.rotation.X = -85_deg;
            setTargetState( LaraStateId::UnderwaterDiving );
            updateImpl();
            m_state.fallspeed *= 2;
        }
        else
        {
            m_state.rotation.X = -45_deg;
            setAnimIdGlobal( loader::AnimationId::FREE_FALL_TO_UNDERWATER, 1895 );
            setTargetState( LaraStateId::UnderwaterForward );
            m_state.fallspeed += m_state.fallspeed / 2;
        }

        resetHeadTorsoRotation();

        auto waterSurfaceHeight = getWaterSurfaceHeight();
        BOOST_ASSERT( waterSurfaceHeight.is_initialized() );
        auto room = m_state.position.room;
        getLevel().findRealFloorSector( m_state.position.position, to_not_null( &room ) );
        getLevel().playSound( 33, m_state.position.position.toRenderSystem() );
        for( int i = 0; i < 10; ++i )
        {
            core::RoomBoundPosition pos{room};
            pos.position.X = m_state.position.position.X;
            pos.position.Y = *waterSurfaceHeight;
            pos.position.Z = m_state.position.position.Z;

            auto particle = make_not_null_shared<engine::SplashParticle>( pos, getLevel() );
            gameplay::setParent( particle, pos.room->node );
            getLevel().m_particles.emplace_back( particle );
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
            setAnimIdGlobal( loader::AnimationId::FREE_FALL_FORWARD, 492 );
            setTargetState( LaraStateId::JumpForward );
            m_state.speed = std::exchange( m_state.fallspeed, 0 ) / 4;
            m_state.falling = true;
        }
        else
        {
            m_underwaterState = UnderwaterState::Swimming;
            setAnimIdGlobal( loader::AnimationId::UNDERWATER_TO_ONWATER, 1937 );
            setTargetState( LaraStateId::OnWaterStop );
            {
                auto pos = m_state.position.position;
                pos.Y = *waterSurfaceHeight + 1;
                m_state.position.position = pos;
            }
            m_swimToDiveKeypressDuration = 11;
            updateFloorHeight( -381 );
            playSoundEffect( 36 );
        }
    }
    else if( m_underwaterState == UnderwaterState::Swimming && !m_state.position.room->isWaterRoom() )
    {
        m_underwaterState = UnderwaterState::OnLand;
        setAnimIdGlobal( loader::AnimationId::FREE_FALL_FORWARD, 492 );
        setTargetState( LaraStateId::JumpForward );
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
    // >>>>>>>>>>>>>>>>>
    //! @todo Move UV anim code to the level.
    static constexpr auto UVAnimTime = 10;

    ++m_uvAnimTime;
    if( m_uvAnimTime >= UVAnimTime )
    {
        getLevel().m_textureAnimator->updateCoordinates( getLevel().m_textureProxies );
        m_uvAnimTime -= UVAnimTime;
    }
    // <<<<<<<<<<<<<<<<<

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
                        BOOST_LOG_TRIVIAL( debug ) << getNode()->getId() <<
                                                   " -- end of animation velocity: override "
                                                   << m_fallSpeedOverride
                                                   << ", anim fall speed " << cmd[0] << ", anim horizontal speed "
                                                   << cmd[1];
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

        getSkeleton()->setAnimIdGlobal( m_state, to_not_null( m_state.anim->nextAnimation ), m_state.anim->nextFrame );
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
                        playSoundEffect( cmd[1] );
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

void LaraNode::updateFloorHeight(int dy)
{
    auto pos = m_state.position.position;
    pos.Y += dy;
    auto room = m_state.position.room;
    const auto sector = to_not_null( getLevel().findRealFloorSector( pos, to_not_null( &room ) ) );
    setCurrentRoom( room );
    const HeightInfo hi = HeightInfo::fromFloor( sector, pos, getLevel().m_itemNodes );
    m_state.floor = hi.distance;
}

void LaraNode::handleCommandSequence(const uint16_t* floorData, bool fromHeavy)
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

    getLevel().m_cameraController->findItem( floorData );

    bool conditionFulfilled = false, switchIsOn = false;
    if( !fromHeavy )
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
                Expects( getLevel().m_itemNodes.find( command.parameter ) != getLevel().m_itemNodes.end() );
                ItemNode& swtch = *getLevel().m_itemNodes[command.parameter];
                if( !swtch.triggerSwitch( activationRequestRaw ) )
                    return;

                switchIsOn = (swtch.m_state.current_anim_state == 1);
                conditionFulfilled = true;
            }
                break;
            case floordata::SequenceCondition::KeyUsed:
            {
                const floordata::Command command{*floorData++};
                Expects( getLevel().m_itemNodes.find( command.parameter ) != getLevel().m_itemNodes.end() );
                ItemNode& key = *getLevel().m_itemNodes[command.parameter];
                if( key.triggerKey() )
                    conditionFulfilled = true;
                else
                    return;
            }
                break;
            case floordata::SequenceCondition::ItemPickedUp:
            {
                const floordata::Command command{*floorData++};
                Expects( getLevel().m_itemNodes.find( command.parameter ) != getLevel().m_itemNodes.end() );
                ItemNode& pickup = *getLevel().m_itemNodes[command.parameter];
                if( pickup.triggerPickUp() )
                    conditionFulfilled = true;
                else
                    return;
            }
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
    else
    {
        conditionFulfilled = chunkHeader.sequenceCondition == floordata::SequenceCondition::ItemIsHere;
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
                ItemNode& item = *getLevel().m_itemNodes[command.parameter];
                if( item.m_state.activationState.isOneshot() )
                    break;

                item.m_state.timer = floordata::ActivationState::extractTimeout( activationRequestRaw );

                //BOOST_LOG_TRIVIAL(trace) << "Setting trigger timeout of " << item.getName() << " to " << item.m_triggerTimeout << "ms";

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
                getLevel().m_cameraController->setItem( getLevel().getItemController( command.parameter ) );
                break;
            case floordata::CommandOpcode::UnderwaterCurrent:
            {
                BOOST_ASSERT( command.parameter < getLevel().m_cameras.size() );
                const auto& sink = getLevel().m_cameras[command.parameter];
                if( m_underwaterRoute.required_box != &getLevel().m_boxes[sink.box_index] )
                {
                    m_underwaterRoute.required_box = &getLevel().m_boxes[sink.box_index];
                    m_underwaterRoute.target = sink.position;
                }
                m_underwaterCurrentStrength = 6 * sink.underwaterCurrentStrength;
            }
                break;
            case floordata::CommandOpcode::FlipMap:
                BOOST_ASSERT( command.parameter < mapFlipActivationStates.size() );
                if( !mapFlipActivationStates[command.parameter].isOneshot() )
                {
                    if( chunkHeader.sequenceCondition == floordata::SequenceCondition::ItemActivated )
                    {
                        mapFlipActivationStates[command.parameter] ^= activationRequest.getActivationSet();
                    }
                    else
                    {
                        mapFlipActivationStates[command.parameter] |= activationRequest.getActivationSet();
                    }

                    if( mapFlipActivationStates[command.parameter].isFullyActivated() )
                    {
                        if( activationRequest.isOneshot() )
                            mapFlipActivationStates[command.parameter].setOneshot( true );

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
                BOOST_ASSERT( command.parameter < mapFlipActivationStates.size() );
                if( !getLevel().roomsAreSwapped && mapFlipActivationStates[command.parameter].isFullyActivated() )
                    swapRooms = true;
                break;
            case floordata::CommandOpcode::FlipOff:
                BOOST_ASSERT( command.parameter < mapFlipActivationStates.size() );
                if( getLevel().roomsAreSwapped && mapFlipActivationStates[command.parameter].isFullyActivated() )
                    swapRooms = true;
                break;
            case floordata::CommandOpcode::FlipEffect:
                flipEffect = command.parameter;
                break;
            case floordata::CommandOpcode::EndLevel:
                getLevel().m_levelFinished = true;
                break;
            case floordata::CommandOpcode::PlayTrack:
                getLevel().triggerCdTrack( command.parameter, activationRequest, chunkHeader.sequenceCondition );
                break;
            case floordata::CommandOpcode::Secret:
            {
                BOOST_ASSERT( command.parameter < 16 );
                const uint16_t mask = 1u << command.parameter;
                if( (m_secretsFoundBitmask & mask) == 0 )
                {
                    m_secretsFoundBitmask |= mask;
                    getLevel().playCdTrack( 13 );
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

boost::optional<int> LaraNode::getWaterSurfaceHeight() const
{
    auto sector = to_not_null( m_state.position.room->getSectorByAbsolutePosition( m_state.position.position ) );

    if( m_state.position.room->isWaterRoom() )
    {
        while( sector->roomAbove != nullptr )
        {
            if( !sector->roomAbove->isWaterRoom() )
                break;

            sector = to_not_null( sector->roomAbove->getSectorByAbsolutePosition( m_state.position.position ) );
        }

        return sector->ceilingHeight * loader::QuarterSectorSize;
    }

    while( sector->roomBelow != nullptr )
    {
        if( sector->roomBelow->isWaterRoom() )
        {
            return sector->floorHeight * loader::QuarterSectorSize;
        }

        sector = to_not_null( sector->roomBelow->getSectorByAbsolutePosition( m_state.position.position ) );
    }

    return sector->ceilingHeight * loader::QuarterSectorSize;
}

void LaraNode::setCameraCurrentRotation(core::Angle x, core::Angle y)
{
    getLevel().m_cameraController->setCurrentRotation( x, y );
}

void LaraNode::setCameraCurrentRotationY(core::Angle y)
{
    getLevel().m_cameraController->setCurrentRotationY( y );
}

void LaraNode::setCameraCurrentRotationX(core::Angle x)
{
    getLevel().m_cameraController->setCurrentRotationX( x );
}

void LaraNode::setCameraTargetDistance(int d)
{
    getLevel().m_cameraController->setTargetDistance( d );
}

void LaraNode::setCameraOldMode(CameraMode k)
{
    getLevel().m_cameraController->setOldMode( k );
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
        rooms.insert( to_not_null( &getLevel().m_rooms[p.adjoining_room] ) );

    for( const std::shared_ptr<ItemNode>& item : getLevel().m_itemNodes | boost::adaptors::map_values )
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

    for( const std::shared_ptr<ItemNode>& item : getLevel().m_dynamicItems )
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
    m_state.box_number = m_state.getCurrentSector()->box;
    core::TRCoordinates targetPos;
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

    collisionInfo.initHeightInfo( m_state.position.position + core::TRCoordinates{0, 200, 0}, getLevel(), 400 );
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

    if( collisionInfo.mid.floor.distance < 0 )
    {
        m_state.position.position.Y += collisionInfo.mid.floor.distance;
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
            const auto& normalLara = *getLevel().m_animatedModels[engine::TR1ItemId::Lara];
            BOOST_ASSERT( normalLara.nmeshes == getNode()->getChildCount() );
            getNode()->getChild( 14 )
                     ->setDrawable( getLevel().m_models2[normalLara.model_base_index + 14].get() );
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
            const auto& normalLara = *getLevel().m_animatedModels[engine::TR1ItemId::Lara];
            BOOST_ASSERT( normalLara.nmeshes == getNode()->getChildCount() );
            getNode()->getChild( 14 )
                     ->setDrawable( getLevel().m_models2[normalLara.model_base_index + 14].get() );
        }

        switch( gunType )
        {
            case WeaponId::Pistols:
                if( pistolsAmmo.ammo != 0 )
                {
                    if( getLevel().m_inputHandler->getInputState().action )
                    {
                        const auto& uziLara = *getLevel().m_animatedModels[engine::TR1ItemId::LaraUzisAnim];
                        BOOST_ASSERT( uziLara.nmeshes == getNode()->getChildCount() );
                        getNode()->getChild( 14 )
                                 ->setDrawable( getLevel().m_models2[uziLara.model_base_index + 14].get() );
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
                        const auto& uziLara = *getLevel().m_animatedModels[engine::TR1ItemId::LaraUzisAnim];
                        BOOST_ASSERT( uziLara.nmeshes == getNode()->getChildCount() );
                        getNode()->getChild( 14 )
                                 ->setDrawable( getLevel().m_models2[uziLara.model_base_index + 14].get() );
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
                        const auto& uziLara = *getLevel().m_animatedModels[engine::TR1ItemId::LaraUzisAnim];
                        BOOST_ASSERT( uziLara.nmeshes == getNode()->getChildCount() );
                        getNode()->getChild( 14 )
                                 ->setDrawable( getLevel().m_models2[uziLara.model_base_index + 14].get() );
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
                        const auto& uziLara = *getLevel().m_animatedModels[engine::TR1ItemId::LaraUzisAnim];
                        BOOST_ASSERT( uziLara.nmeshes == getNode()->getChildCount() );
                        getNode()->getChild( 14 )
                                 ->setDrawable( getLevel().m_models2[uziLara.model_base_index + 14].get() );
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

void LaraNode::updateGuns(WeaponId weaponId)
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
        const auto* positionData = getLevel().m_animatedModels[engine::TR1ItemId::Lara]->frame_base;

        rightArm.weaponAnimData = positionData;
        leftArm.weaponAnimData = positionData;
    }
    else if( gunType == WeaponId::Pistols || gunType == WeaponId::AutoPistols || gunType == WeaponId::Uzi )
    {
        const auto* positionData = getLevel().m_animatedModels[engine::TR1ItemId::LaraPistolsAnim]->frame_base;

        rightArm.weaponAnimData = positionData;
        leftArm.weaponAnimData = positionData;

        if( m_handStatus != HandStatus::None )
        {
            overrideLaraMeshesUnholsterGuns( gunType );
        }
    }
    else if( gunType == WeaponId::Shotgun )
    {
        const auto* positionData = getLevel().m_animatedModels[engine::TR1ItemId::LaraShotgunAnim]->frame_base;

        rightArm.weaponAnimData = positionData;
        leftArm.weaponAnimData = positionData;

        if( m_handStatus != HandStatus::None )
        {
            overrideLaraMeshesUnholsterShotgun();
        }
    }
    else
    {
        const auto* positionData = getLevel().m_animatedModels[engine::TR1ItemId::Lara]->frame_base;

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

void LaraNode::unholsterGuns(WeaponId weaponId)
{
    auto nextFrame = leftArm.frame + 1;
    if( nextFrame < 5 || nextFrame > 23 )
    {
        nextFrame = 5;
    }
    else if( nextFrame == 13 )
    {
        overrideLaraMeshesUnholsterGuns( weaponId );
        getLevel().playSound( 6, getNode()->getTranslationWorld() );
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
    for( const std::shared_ptr<ItemNode>& currentEnemy
            : getLevel().m_itemNodes | boost::adaptors::map_values )
    {
        if( currentEnemy->m_state.health <= 0 || currentEnemy.get() == getLevel().m_lara )
            continue;

        const auto modelEnemy = std::dynamic_pointer_cast<ModelItemNode>( currentEnemy );
        if( modelEnemy == nullptr )
        {
            BOOST_LOG_TRIVIAL( warning ) << "Ignoring non-model item " << currentEnemy->getNode()->getId();
            continue;
        }

        const auto d = currentEnemy->m_state.position.position - gunPosition.position;
        if( std::abs( d.X ) > weapon.targetDist )
            continue;

        if( std::abs( d.Y ) > weapon.targetDist )
            continue;

        if( std::abs( d.Z ) > weapon.targetDist )
            continue;

        if( util::square( d.X ) + util::square( d.Y ) + util::square( d.Z ) >= util::square( weapon.targetDist ) )
            continue;

        auto target = getUpperThirdBBoxCtr( *std::dynamic_pointer_cast<const ModelItemNode>( currentEnemy ) );
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

    rightArm.weaponAnimData = getLevel().m_animatedModels[engine::TR1ItemId::LaraPistolsAnim]->frame_base;
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

    rightArm.weaponAnimData = getLevel().m_animatedModels[engine::TR1ItemId::LaraShotgunAnim]->frame_base;
    leftArm.weaponAnimData = rightArm.weaponAnimData;
}

void LaraNode::overrideLaraMeshesUnholsterGuns(WeaponId weaponId)
{
    engine::TR1ItemId id;
    if( weaponId == WeaponId::AutoPistols )
    {
        id = engine::TR1ItemId::LaraMagnumsAnim;
    }
    else if( weaponId == WeaponId::Uzi )
    {
        id = engine::TR1ItemId::LaraUzisAnim;
    }
    else
    {
        id = engine::TR1ItemId::LaraPistolsAnim;
    }

    const auto& src = *getLevel().m_animatedModels[id];
    BOOST_ASSERT( src.nmeshes == getNode()->getChildCount() );
    const auto& normalLara = *getLevel().m_animatedModels[engine::TR1ItemId::Lara];
    BOOST_ASSERT( normalLara.nmeshes == getNode()->getChildCount() );
    getNode()->getChild( 1 )->setDrawable( getLevel().m_models2[normalLara.model_base_index + 1].get() );
    getNode()->getChild( 4 )->setDrawable( getLevel().m_models2[normalLara.model_base_index + 4].get() );
    getNode()->getChild( 10 )->setDrawable( getLevel().m_models2[src.model_base_index + 10].get() );
    getNode()->getChild( 13 )->setDrawable( getLevel().m_models2[src.model_base_index + 13].get() );
}

void LaraNode::overrideLaraMeshesUnholsterShotgun()
{
    const auto& src = *getLevel().m_animatedModels[engine::TR1ItemId::LaraShotgunAnim];
    BOOST_ASSERT( src.nmeshes == getNode()->getChildCount() );
    const auto& normalLara = *getLevel().m_animatedModels[engine::TR1ItemId::Lara];
    BOOST_ASSERT( normalLara.nmeshes == getNode()->getChildCount() );
    getNode()->getChild( 7 )->setDrawable( getLevel().m_models2[normalLara.model_base_index + 7].get() );
    getNode()->getChild( 10 )->setDrawable( getLevel().m_models2[src.model_base_index + 10].get() );
    getNode()->getChild( 13 )->setDrawable( getLevel().m_models2[src.model_base_index + 13].get() );
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

        getLevel().playSound( 6, getNode()->getTranslationWorld() );
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
                    getLevel().playSound( 9, getNode()->getTranslationWorld() );
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
                getLevel().playSound( 9, getNode()->getTranslationWorld() );
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
        getLevel().playSound( weapons[WeaponId::Shotgun].sampleNum, getNode()->getTranslationWorld() );
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
            const auto& src = *getLevel().m_animatedModels[engine::TR1ItemId::Lara];
            BOOST_ASSERT( src.nmeshes == getNode()->getChildCount() );
            const auto& normalLara = *getLevel().m_animatedModels[engine::TR1ItemId::Lara];
            BOOST_ASSERT( normalLara.nmeshes == getNode()->getChildCount() );
            getNode()->getChild( 7 )->setDrawable( getLevel().m_models2[src.model_base_index + 7].get() );
            getNode()->getChild( 10 )->setDrawable( getLevel().m_models2[normalLara.model_base_index + 10].get() );
            getNode()->getChild( 13 )->setDrawable( getLevel().m_models2[normalLara.model_base_index + 13].get() );

            getLevel().playSound( 6, getNode()->getTranslationWorld() );
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

void LaraNode::holsterGuns(WeaponId weaponId)
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
            engine::TR1ItemId srcId = engine::TR1ItemId::LaraPistolsAnim;
            if( weaponId == WeaponId::AutoPistols )
            {
                srcId = engine::TR1ItemId::LaraMagnumsAnim;
            }
            else if( weaponId == WeaponId::Uzi )
            {
                srcId = engine::TR1ItemId::LaraUzisAnim;
            }

            const auto& src = *getLevel().m_animatedModels[srcId];
            BOOST_ASSERT( src.nmeshes == getNode()->getChildCount() );
            const auto& normalLara = *getLevel().m_animatedModels[engine::TR1ItemId::Lara];
            BOOST_ASSERT( normalLara.nmeshes == getNode()->getChildCount() );
            getNode()->getChild( 1 )->setDrawable( getLevel().m_models2[src.model_base_index + 1].get() );
            getNode()->getChild( 13 )->setDrawable( getLevel().m_models2[normalLara.model_base_index + 13].get() );

            getLevel().playSound( 7, getNode()->getTranslationWorld() );
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
            engine::TR1ItemId srcId = engine::TR1ItemId::LaraPistolsAnim;
            if( weaponId == WeaponId::AutoPistols )
            {
                srcId = engine::TR1ItemId::LaraMagnumsAnim;
            }
            else if( weaponId == WeaponId::Uzi )
            {
                srcId = engine::TR1ItemId::LaraUzisAnim;
            }

            const auto& src = *getLevel().m_animatedModels[srcId];
            BOOST_ASSERT( src.nmeshes == getNode()->getChildCount() );
            const auto& normalLara = *getLevel().m_animatedModels[engine::TR1ItemId::Lara];
            BOOST_ASSERT( normalLara.nmeshes == getNode()->getChildCount() );
            getNode()->getChild( 4 )->setDrawable( getLevel().m_models2[src.model_base_index + 4].get() );
            getNode()->getChild( 10 )->setDrawable( getLevel().m_models2[normalLara.model_base_index + 10].get() );

            getLevel().playSound( 7, getNode()->getTranslationWorld() );
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

void LaraNode::updateAnimNotShotgun(WeaponId weaponId)
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
            getLevel().playSound( weapon.sampleNum, getNode()->getTranslationWorld() );
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
            getLevel().playSound( weapon.sampleNum, getNode()->getTranslationWorld() );
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

bool LaraNode::fireWeapon(WeaponId weaponId,
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
        getLevel().playSound( 48, gunHolder.getNode()->getTranslationWorld() );
        requestedGunType = WeaponId::Pistols;
        return false;
    }

    --ammoPtr->ammo;
    const auto weapon = &weapons[weaponId];
    core::TRCoordinates gunPosition = gunHolder.m_state.position.position;
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
    const auto bulletDir = glm::normalize( glm::vec3( shootVector.toMatrix()[2] ) ); // +Z is our shooting direction
    if( !spheres.empty() )
    {
        float minD = std::numeric_limits<float>::max();
        for( const auto& sphere : spheres )
        {
            hitPos = gunPosition.toRenderSystem()
                     + bulletDir * glm::dot( sphere.getPosition() - gunPosition.toRenderSystem(), bulletDir );

            const auto d = glm::length( hitPos - sphere.getPosition() );
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
                gunPosition + core::TRCoordinates{-bulletDir * VeryLargeDistanceProbablyClipping}
        };

        core::RoomBoundPosition bulletPos{gunHolder.m_state.position.room, gunPosition};
        CameraController::clampPosition( bulletPos, aimHitPos, getLevel() );
        playShotMissed( aimHitPos );
    }
    else
    {
        BOOST_ASSERT( target != nullptr );
        ++ammoPtr->hits;
        hitTarget( *target, core::TRCoordinates{hitPos}, weapon->damage );
    }

    return true;
}

void LaraNode::playShotMissed(const core::RoomBoundPosition& pos)
{
    const auto particle = make_not_null_shared<RicochetParticle>( pos, getLevel() );
    gameplay::setParent( particle, m_state.position.room->node );
    getLevel().m_particles.emplace_back( particle );
    getLevel().playSound( 10, pos.position.toRenderSystem() );
}

void LaraNode::hitTarget(ModelItemNode& item, const core::TRCoordinates& hitPos, int damage)
{
    BOOST_LOG_TRIVIAL( debug ) << "Target " << item.getNode()->getId() << " is hit, health=" << item.m_state.health <<
                               ", damage=" << damage;

    if( item.m_state.health > 0 && item.m_state.health <= damage )
    {
        // TODO ++g_numKills;
    }
    item.m_state.is_hit = true;
    item.m_state.health -= damage;
    auto fx = engine::createBloodSplat( getLevel(),
                                        core::RoomBoundPosition{item.m_state.position.room, hitPos},
                                        item.m_state.speed,
                                        item.m_state.rotation.Y );
    getLevel().m_particles.emplace_back( fx );
    if( item.m_state.health <= 0 )
        return;

    int soundId;
    switch( item.m_state.object_number )
    {
        case engine::TR1ItemId::Wolf:
            soundId = 20;
            break;
        case engine::TR1ItemId::Bear:
            soundId = 16;
            break;
        case engine::TR1ItemId::LionMale:
        case engine::TR1ItemId::LionFemale:
            soundId = 0x55;
            break;
        case engine::TR1ItemId::RatOnLand:
            soundId = 0x5f;
            break;
        case engine::TR1ItemId::SkateboardKid:
            soundId = 0x84;
            break;
        case engine::TR1ItemId::TorsoBoss:
            soundId = 0x8e;
            break;
        default:
            return;
    }

    getLevel().playSound( soundId, item.m_state.position.position.toRenderSystem() );
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

    void rotate(uint32_t packed)
    {
        m_stack.top() *= core::fromPackedAngles( packed );
    }

    void translate(const glm::vec3& c)
    {
        m_stack.top() = glm::translate( m_stack.top(), c );
    }

    void translate(const BoneTreeEntry& bte)
    {
        translate( bte.toGl() );
    }

    void transform(const std::initializer_list<size_t>& indices,
                   const BoneTreeEntry* boneTree,
                   const gsl::span<const uint32_t>& angleData,
                   const std::shared_ptr<SkeletalModelNode>& skeleton)
    {
        for( auto idx : indices )
            transform( idx, boneTree, angleData, skeleton );
    }

    void transform(size_t idx,
                   const BoneTreeEntry* boneTree,
                   const gsl::span<const uint32_t>& angleData,
                   const std::shared_ptr<SkeletalModelNode>& skeleton)
    {
        BOOST_ASSERT( idx > 0 );
        translate( boneTree[idx - 1] );
        rotate( angleData[idx] );
        apply( skeleton, idx );
    }

    void apply(const std::shared_ptr<SkeletalModelNode>& skeleton, size_t idx)
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
    explicit DualMatrixStack(float bias) : m_bias{bias}
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

    void rotate(uint32_t packed1, uint32_t packed2)
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

    void translate(const BoneTreeEntry& bte)
    {
        translate( bte.toGl(), bte.toGl() );
    }

    void transform(const std::initializer_list<size_t>& indices,
                   const BoneTreeEntry* boneTree,
                   const gsl::span<const uint32_t>& angleData1,
                   const gsl::span<const uint32_t>& angleData2,
                   const std::shared_ptr<SkeletalModelNode>& skeleton)
    {
        for( auto idx : indices )
            transform( idx, boneTree, angleData1, angleData2, skeleton );
    }

    void transform(size_t idx,
                   const BoneTreeEntry* boneTree,
                   const gsl::span<const uint32_t>& angleData1,
                   const gsl::span<const uint32_t>& angleData2,
                   const std::shared_ptr<SkeletalModelNode>& skeleton)
    {
        BOOST_ASSERT( idx > 0 );
        translate( boneTree[idx - 1] );
        rotate( angleData1[idx], angleData2[idx] );
        apply( skeleton, idx );
    }

    void apply(const std::shared_ptr<SkeletalModelNode>& skeleton, size_t idx)
    {
        BOOST_ASSERT( skeleton != nullptr );
        skeleton->getChild( idx )->setLocalMatrix( itop() );
    }
};
}

void LaraNode::drawRoutine()
{
    auto interpolationInfo = getSkeleton()->getInterpolationInfo( m_state );
    if( !hit_direction.is_initialized() && interpolationInfo.firstFrame != interpolationInfo.secondFrame )
    {
        drawRoutineInterpolated( interpolationInfo );
        return;
    }

    const auto& objInfo = *getLevel().m_animatedModels[m_state.object_number];
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
                frame = getLevel().m_animations[static_cast<int>(loader::AnimationId::AH_LEFT)].poseData;
                break;
            case core::Axis::NegZ:
                frame = getLevel().m_animations[static_cast<int>(loader::AnimationId::AH_BACKWARD)].poseData;
                break;
            case core::Axis::NegX:
                frame = getLevel().m_animations[static_cast<int>(loader::AnimationId::AH_RIGHT)].poseData;
                break;
            default:
                frame = getLevel().m_animations[static_cast<int>(loader::AnimationId::AH_FORWARD)].poseData;
                break;
        }
        frame = frame->next( hit_frame );
    }

    updateLighting();

    MatrixStack matrixStack;

    matrixStack.push();
    const auto* boneTree = reinterpret_cast<const BoneTreeEntry*>(&getLevel().m_boneTrees[objInfo.bone_index]);
    matrixStack.translate( frame->pos.toGl() );
    const auto angleData = frame->getAngleData();
    matrixStack.rotate( angleData[0] );
    matrixStack.apply( getSkeleton(), 0 );

    matrixStack.push();
    matrixStack.transform( {1, 2, 3}, boneTree, angleData, getSkeleton() );

    matrixStack.pop();
    matrixStack.push();
    matrixStack.transform( {4, 5, 6}, boneTree, angleData, getSkeleton() );

    matrixStack.pop();
    matrixStack.translate( boneTree[6] );
    matrixStack.rotate( angleData[7] );
    matrixStack.rotate( m_torsoRotation );
    matrixStack.apply( getSkeleton(), 7 );

    matrixStack.push();
    matrixStack.translate( boneTree[13] );
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
            matrixStack.transform( {8, 9, 10}, boneTree, angleData, getSkeleton() );

            matrixStack.pop();
            matrixStack.push();
            matrixStack.transform( {11, 12, 13}, boneTree, angleData, getSkeleton() );
            break;
        case WeaponId::Pistols:
        case WeaponId::AutoPistols:
        case WeaponId::Uzi:
            matrixStack.push();
            matrixStack.translate( boneTree[7] );
            matrixStack.resetRotation();
            matrixStack.rotate( rightArm.aimRotation );

            armAngleData = rightArm.weaponAnimData->next( rightArm.frame )->getAngleData();
            matrixStack.rotate( armAngleData[8] );
            matrixStack.apply( getSkeleton(), 8 );

            matrixStack.transform( 9, boneTree, armAngleData, getSkeleton() );
            matrixStack.transform( 10, boneTree, armAngleData, getSkeleton() );

            renderGunFlare( activeGunType, matrixStack.top(), m_gunFlareRight, rightArm.flashTimeout != 0 );
            matrixStack.pop();
            matrixStack.push();
            matrixStack.translate( boneTree[10] );
            matrixStack.resetRotation();
            matrixStack.rotate( leftArm.aimRotation );
            armAngleData = leftArm.weaponAnimData->next( leftArm.frame )->getAngleData();
            matrixStack.rotate( armAngleData[11] );
            matrixStack.apply( getSkeleton(), 11 );

            matrixStack.transform( {12, 13}, boneTree, armAngleData, getSkeleton() );

            renderGunFlare( activeGunType, matrixStack.top(), m_gunFlareLeft, leftArm.flashTimeout != 0 );
            break;
        case WeaponId::Shotgun:
            matrixStack.push();
            armAngleData = rightArm.weaponAnimData->next( rightArm.frame )->getAngleData();
            matrixStack.transform( {8, 9, 10}, boneTree, armAngleData, getSkeleton() );

            matrixStack.pop();
            matrixStack.push();
            armAngleData = leftArm.weaponAnimData->next( leftArm.frame )->getAngleData();
            matrixStack.transform( {11, 12, 13}, boneTree, armAngleData, getSkeleton() );
            break;
        default:
            break;
    }
}

void LaraNode::drawRoutineInterpolated(const SkeletalModelNode::InterpolationInfo& interpolationInfo)
{
    updateLighting();

    const auto& objInfo = *getLevel().m_animatedModels[m_state.object_number];

    DualMatrixStack matrixStack{interpolationInfo.bias};

    matrixStack.push();
    const auto* boneTree = reinterpret_cast<const BoneTreeEntry*>(&getLevel().m_boneTrees[objInfo.bone_index]);
    matrixStack.translate( interpolationInfo.firstFrame->pos.toGl(), interpolationInfo.secondFrame->pos.toGl() );
    const auto angleDataA = interpolationInfo.firstFrame->getAngleData();
    const auto angleDataB = interpolationInfo.secondFrame->getAngleData();
    matrixStack.rotate( angleDataA[0], angleDataB[0] );
    matrixStack.apply( getSkeleton(), 0 );

    matrixStack.push();
    matrixStack.transform( {1, 2, 3}, boneTree, angleDataA, angleDataB, getSkeleton() );

    matrixStack.pop();
    matrixStack.push();
    matrixStack.transform( {4, 5, 6}, boneTree, angleDataA, angleDataB, getSkeleton() );

    matrixStack.pop();
    matrixStack.translate( boneTree[6] );
    matrixStack.rotate( angleDataA[7], angleDataB[7] );
    matrixStack.rotate( m_torsoRotation );
    matrixStack.apply( getSkeleton(), 7 );

    matrixStack.push();
    matrixStack.translate( boneTree[13] );
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
            matrixStack.transform( {8, 9, 10}, boneTree, angleDataA, angleDataB, getSkeleton() );

            matrixStack.pop();
            matrixStack.push();
            matrixStack.transform( {11, 12, 13}, boneTree, angleDataA, angleDataB, getSkeleton() );
            break;
        case WeaponId::Pistols:
        case WeaponId::AutoPistols:
        case WeaponId::Uzi:
            matrixStack.push();
            matrixStack.translate( boneTree[7] );
            matrixStack.resetRotation();
            matrixStack.rotate( rightArm.aimRotation );

            armAngleData = rightArm.weaponAnimData->next( rightArm.frame )->getAngleData();
            matrixStack.rotate( armAngleData[8], armAngleData[8] );
            matrixStack.apply( getSkeleton(), 8 );

            matrixStack.transform( 9, boneTree, armAngleData, armAngleData, getSkeleton() );
            matrixStack.transform( 10, boneTree, armAngleData, armAngleData, getSkeleton() );

            renderGunFlare( activeGunType, matrixStack.itop(), m_gunFlareRight, rightArm.flashTimeout != 0 );
            matrixStack.pop();
            matrixStack.push();
            matrixStack.translate( boneTree[10] );
            matrixStack.resetRotation();
            matrixStack.rotate( leftArm.aimRotation );
            armAngleData = leftArm.weaponAnimData->next( leftArm.frame )->getAngleData();
            matrixStack.rotate( armAngleData[11], armAngleData[11] );
            matrixStack.apply( getSkeleton(), 11 );

            matrixStack.transform( {12, 13}, boneTree, armAngleData, armAngleData, getSkeleton() );

            renderGunFlare( activeGunType, matrixStack.itop(), m_gunFlareLeft, leftArm.flashTimeout != 0 );
            break;
        case WeaponId::Shotgun:
            matrixStack.push();
            armAngleData = rightArm.weaponAnimData->next( rightArm.frame )->getAngleData();
            matrixStack.transform( {8, 9, 10}, boneTree, armAngleData, armAngleData, getSkeleton() );

            matrixStack.pop();
            matrixStack.push();
            armAngleData = leftArm.weaponAnimData->next( leftArm.frame )->getAngleData();
            matrixStack.transform( {11, 12, 13}, boneTree, armAngleData, armAngleData, getSkeleton() );
            break;
        default:
            break;
    }
}

void
LaraNode::renderGunFlare(LaraNode::WeaponId weaponId,
                         glm::mat4 m,
                         const gsl::not_null<std::shared_ptr<gameplay::Node>>& flareNode,
                         bool visible)
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
    }

    m = glm::translate( m, core::TRCoordinates{0, dy, 55}.toRenderSystem() );
    m *= core::TRRotation( -90_deg, 0_deg, core::Angle( 2 * util::rand15() ) ).toMatrix();

    flareNode->setVisible( true );
    setParent( flareNode, getNode()->getParent().lock() );
    flareNode->setLocalMatrix( getNode()->getLocalMatrix() * m );

    const auto brightness = util::clamp( 2.0f - shade / 8191.0f, 0.0f, 1.0f );
    flareNode->addMaterialParameterSetter( "u_baseLight", [brightness](const gameplay::Node& /*node*/,
                                                                       gameplay::gl::Program::ActiveUniform& uniform) {
        uniform.set( brightness );
    } );
    flareNode->addMaterialParameterSetter( "u_baseLightDiff", [this](const gameplay::Node& /*node*/,
                                                                     gameplay::gl::Program::ActiveUniform& uniform) {
        uniform.set( 0.0f );
    } );
}
}
