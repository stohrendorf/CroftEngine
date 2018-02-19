#include "laranode.h"

#include "cameracontroller.h"
#include "level/level.h"
#include "render/textureanimator.h"

#include "items/block.h"
#include "items/tallblock.h"

#include <boost/range/adaptors.hpp>
#include <stack>

namespace
{
std::array<engine::floordata::ActivationState, 10> mapFlipActivationStates;

inline core::TRRotationXY anglesFromPosition(const core::TRCoordinates& co)
{
    return core::anglesFromPosition( co.X, co.Y, co.Z );
}

void swapWithAlternate(loader::Room& orig, loader::Room& alternate)
{
    // find any blocks in the original room and un-patch the floor heights
    for( const auto& child : orig.node->getChildren() )
    {
        if( auto tmp = std::dynamic_pointer_cast<engine::items::Block>( child ) )
        {
            loader::Room::patchHeightsForBlock( *tmp, loader::SectorSize );
        }
        else if( auto tmp = std::dynamic_pointer_cast<engine::items::TallBlock>( child ) )
        {
            loader::Room::patchHeightsForBlock( *tmp, loader::SectorSize * 2 );
        }
    }

    // now swap the rooms and patch the alternate room ids
    std::swap( orig, alternate );
    orig.alternateRoom = alternate.alternateRoom;
    alternate.alternateRoom = -1;

    // move all items over
    orig.node->swapChildren( alternate.node );

    // patch heights in the new room.
    // note that this is exactly the same code as above,
    // except for the heights.
    for( const auto& child : orig.node->getChildren() )
    {
        if( auto tmp = std::dynamic_pointer_cast<engine::items::Block>( child ) )
        {
            loader::Room::patchHeightsForBlock( *tmp, -loader::SectorSize );
        }
        else if( auto tmp = std::dynamic_pointer_cast<engine::items::TallBlock>( child ) )
        {
            loader::Room::patchHeightsForBlock( *tmp, -loader::SectorSize * 2 );
        }
    }
}

void swapAllRooms(level::Level& level)
{
    for( auto& room : level.m_rooms )
    {
        if( room.alternateRoom < 0 )
            continue;

        BOOST_ASSERT( static_cast<size_t>(room.alternateRoom) < level.m_rooms.size() );
        swapWithAlternate( room, level.m_rooms[room.alternateRoom] );
    }

    level.roomsAreSwapped = !level.roomsAreSwapped;
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
    getSkeleton()->setAnimIdGlobal( m_state, static_cast<uint16_t>(anim), firstFrame.get_value_or( 0 ) );
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
        m_state.rotation.Z += +1_deg;
        if( m_state.rotation.Z >= 0_deg )
        {
            m_state.rotation.Z = 0_deg;
        }
    }
    else
    {
        if( m_state.rotation.Z > 1_deg )
        {
            m_state.rotation.Z += -1_deg;
            if( m_state.rotation.Z <= 0_deg )
            {
                m_state.rotation.Z = 0_deg;
            }
        }
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

    //! @todo updateWeaponState()

    updateWeaponState();
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
    //! @todo update weapon state
    updateWeaponState();
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
    //! @todo Update weapon state
    updateWeaponState();
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
            const int16_t spd = m_state.fallspeed * 2;
            m_state.fallspeed = spd;
        }
        else if( getCurrentAnimState() == LaraStateId::SwandiveEnd )
        {
            m_state.rotation.X = -85_deg;
            setTargetState( LaraStateId::UnderwaterDiving );
            updateImpl();
            const int16_t spd = m_state.fallspeed * 2;
            m_state.fallspeed = spd;
        }
        else
        {
            m_state.rotation.X = -45_deg;
            setAnimIdGlobal( loader::AnimationId::FREE_FALL_TO_UNDERWATER, 1895 );
            setTargetState( LaraStateId::UnderwaterForward );
            //m_currentStateHandler = lara::AbstractStateHandler::create(LaraStateId::UnderwaterDiving, *this);
            const int16_t spd = m_state.fallspeed * 3 / 2;
            m_state.fallspeed = spd;
        }

        resetHeadTorsoRotation();

        //! @todo Show water splash effect
    }
    else
    {
        if( m_underwaterState == UnderwaterState::Diving && !m_state.position.room->isWaterRoom() )
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
                //m_currentStateHandler = lara::AbstractStateHandler::create(LaraStateId::JumpForward, *this);
                //! @todo Check formula
                const int16_t speed = m_state.speed / 4;
                m_state.speed = speed;
                m_state.falling = true;
            }
            else
            {
                m_underwaterState = UnderwaterState::Swimming;
                setAnimIdGlobal( loader::AnimationId::UNDERWATER_TO_ONWATER, 1937 );
                setTargetState( LaraStateId::OnWaterStop );
                //m_currentStateHandler = lara::AbstractStateHandler::create(LaraStateId::OnWaterStop, *this);
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
            //m_currentStateHandler = lara::AbstractStateHandler::create(LaraStateId::JumpForward, *this);
            m_state.fallspeed = 0;
            //! @todo Check formula
            const int16_t speed = m_state.speed / 5;
            m_state.speed = speed;
            m_state.falling = true;
            m_handStatus = HandStatus::None;
            m_state.rotation.X = 0_deg;
            m_state.rotation.Z = 0_deg;
            resetHeadTorsoRotation();
        }
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

    if( endOfAnim )
    {
        const loader::Animation& animation = getLevel().m_animations[m_state.anim_number];
        if( animation.animCommandCount > 0 )
        {
            BOOST_ASSERT( animation.animCommandIndex < getLevel().m_animCommands.size() );
            const auto* cmd = &getLevel().m_animCommands[animation.animCommandIndex];
            for( uint16_t i = 0; i < animation.animCommandCount; ++i )
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
                        BOOST_LOG_TRIVIAL( debug ) << getNode()->getId() << " -- end of animation velocity: override "
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

        const loader::Animation& currentAnim = getSkeleton()->getCurrentAnimData( m_state );
        getSkeleton()->setAnimIdGlobal( m_state, currentAnim.nextAnimation, currentAnim.nextFrame );
    }

    const loader::Animation& animation = getLevel().m_animations[m_state.anim_number];
    if( animation.animCommandCount > 0 )
    {
        BOOST_ASSERT( animation.animCommandIndex < getLevel().m_animCommands.size() );
        const auto* cmd = &getLevel().m_animCommands[animation.animCommandIndex];
        for( uint16_t i = 0; i < animation.animCommandCount; ++i )
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
                        if( cmd[1] == 0 )
                        {
                            m_state.rotation.Y += 180_deg;
                        }
                        else if( cmd[1] == 12 )
                            setHandStatus( HandStatus::None );
                        //! @todo Execute anim effect cmd[1]
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
    const loader::Room* room = m_state.position.room;
    auto sector = getLevel().findRealFloorSector( pos, &room );
    setCurrentRoom( room );
    HeightInfo hi = HeightInfo::fromFloor( sector, pos, getLevel().m_itemNodes, getLevel().m_floorData );
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
                items::ItemNode& swtch = *getLevel().m_itemNodes[command.parameter];
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
                items::ItemNode& key = *getLevel().m_itemNodes[command.parameter];
                if( key.triggerKey() )
                    conditionFulfilled = true;
            }
                return;
            case floordata::SequenceCondition::ItemPickedUp:
            {
                const floordata::Command command{*floorData++};
                Expects( getLevel().m_itemNodes.find( command.parameter ) != getLevel().m_itemNodes.end() );
                items::ItemNode& pickup = *getLevel().m_itemNodes[command.parameter];
                if( pickup.triggerPickUp() )
                    conditionFulfilled = true;
            }
                return;
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
    while( true )
    {
        const floordata::Command command{*floorData++};
        switch( command.opcode )
        {
            case floordata::CommandOpcode::Activate:
            {
                Expects( getLevel().m_itemNodes.find( command.parameter ) != getLevel().m_itemNodes.end() );
                engine::items::ItemNode& item = *getLevel().m_itemNodes[command.parameter];
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

                if( (item.m_characteristics & Intelligent) == 0 )
                {
                    item.m_state.triggerState = items::TriggerState::Enabled;
                    item.activate();
                    break;
                }

                if( item.m_state.triggerState == items::TriggerState::Disabled )
                {
                    //! @todo Implement baddie
                    item.m_state.triggerState = items::TriggerState::Enabled;
                    item.activate();
                    break;
                }

                if( item.m_state.triggerState != items::TriggerState::Locked )
                    break;

                item.m_state.triggerState = items::TriggerState::Enabled;
                item.activate();
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
                //! @todo handle flip effect
                break;
            case floordata::CommandOpcode::EndLevel:
                //! @todo handle level end
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

    if( swapRooms )
        swapAllRooms( getLevel() );
}

boost::optional<int> LaraNode::getWaterSurfaceHeight() const
{
    gsl::not_null<const loader::Sector*> sector = m_state.position.room
                                                         ->getSectorByAbsolutePosition( m_state.position.position );

    if( m_state.position.room->isWaterRoom() )
    {
        while( sector->roomAbove != nullptr )
        {
            if( !sector->roomAbove->isWaterRoom() )
                break;

            sector = sector->roomAbove->getSectorByAbsolutePosition( m_state.position.position );
        }

        return sector->ceilingHeight * loader::QuarterSectorSize;
    }

    while( sector->roomBelow != nullptr )
    {
        if( sector->roomBelow->isWaterRoom() )
        {
            return sector->floorHeight * loader::QuarterSectorSize;
        }

        sector = sector->roomBelow->getSectorByAbsolutePosition( m_state.position.position );
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

    std::set<const loader::Room*> rooms;
    rooms.insert( m_state.position.room );
    for( const loader::Portal& p : m_state.position.room->portals )
        rooms.insert( &getLevel().m_rooms[p.adjoining_room] );

    for( const std::shared_ptr<items::ItemNode>& item : getLevel().m_itemNodes | boost::adaptors::map_values )
    {
        if( rooms.find( item->m_state.position.room ) == rooms.end() )
            continue;

        if( !item->m_state.collidable )
            continue;

        if( item->m_state.triggerState == items::TriggerState::Locked )
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

void LaraNode::updateWeaponState()
{
    if( leftArm.shootTimeout > 0 )
    {
        --leftArm.shootTimeout;
    }
    if( rightArm.shootTimeout > 0 )
    {
        --rightArm.shootTimeout;
    }
    bool doHolster = false;
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
                doHolster = true;
            }
        }
        else if( requestedGunType == gunType )
        {
            if( getLevel().m_inputHandler->getInputState().holster )
            {
                doHolster = true;
            }
        }
        else if( m_handStatus == HandStatus::Combat )
        {
            doHolster = true;
        }
        else if( m_handStatus == HandStatus::None )
        {
            gunType = requestedGunType;
            unholsterReplaceMeshes();
            doHolster = true;
        }
    }

    if( doHolster && gunType != WeaponId::None )
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
                unholsterDoubleWeapon( gunType );
            }
            else if( gunType == WeaponId::Shotgun )
            {
                if( getLevel().m_cameraController->getMode() != CameraMode::Cinematic
                    && getLevel().m_cameraController->getMode() != CameraMode::FreeLook )
                {
                    getLevel().m_cameraController->setMode( CameraMode::Combat );
                }
                unholsterShotgunAnimUpdate();
            }
        }
    }
    else if( m_handStatus == HandStatus::Holster )
    {
        {
            const auto& normalLara = *getLevel().m_animatedModels[0];
            BOOST_ASSERT( normalLara.nmeshes == getNode()->getChildCount() );
            getNode()->getChild( 14 )
                     ->setDrawable( getLevel().getModel( getLevel().m_meshIndices[normalLara.frame_number + 14] ) );
        }

        if( gunType >= WeaponId::Pistols )
        {
            if( gunType <= WeaponId::Uzi )
            {
                playSingleShot( gunType );
            }
            else if( gunType == WeaponId::Shotgun )
            {
                playSingleShotShotgun();
            }
        }
    }
    else if( m_handStatus == HandStatus::Combat )
    {
        {
            const auto& normalLara = *getLevel().m_animatedModels[0];
            BOOST_ASSERT( normalLara.nmeshes == getNode()->getChildCount() );
            getNode()->getChild( 14 )
                     ->setDrawable( getLevel().getModel( getLevel().m_meshIndices[normalLara.frame_number + 14] ) );
        }

        switch( gunType )
        {
            case WeaponId::Pistols:
                if( pistolsAmmo.ammo != 0 )
                {
                    if( getLevel().m_inputHandler->getInputState().action )
                    {
                        const auto& uziLara = *getLevel().m_animatedModels[4];
                        BOOST_ASSERT( uziLara.nmeshes == getNode()->getChildCount() );
                        getNode()->getChild( 14 )
                                 ->setDrawable(
                                         getLevel().getModel( getLevel().m_meshIndices[uziLara.frame_number + 14] ) );
                    }
                }
                if( getLevel().m_cameraController->getMode() != CameraMode::Cinematic
                    && getLevel().m_cameraController->getMode() != CameraMode::FreeLook )
                {
                    getLevel().m_cameraController->setMode( CameraMode::Combat );
                }
                updateNotShotgun( gunType );
                break;
            case WeaponId::AutoPistols:
                if( revolverAmmo.ammo != 0 )
                {
                    if( getLevel().m_inputHandler->getInputState().action )
                    {
                        const auto& uziLara = *getLevel().m_animatedModels[4];
                        BOOST_ASSERT( uziLara.nmeshes == getNode()->getChildCount() );
                        getNode()->getChild( 14 )
                                 ->setDrawable(
                                         getLevel().getModel( getLevel().m_meshIndices[uziLara.frame_number + 14] ) );
                    }
                }
                if( getLevel().m_cameraController->getMode() != CameraMode::Cinematic
                    && getLevel().m_cameraController->getMode() != CameraMode::FreeLook )
                {
                    getLevel().m_cameraController->setMode( CameraMode::Combat );
                }
                updateNotShotgun( gunType );
                break;
            case WeaponId::Uzi:
                if( uziAmmo.ammo != 0 )
                {
                    if( getLevel().m_inputHandler->getInputState().action )
                    {
                        const auto& uziLara = *getLevel().m_animatedModels[4];
                        BOOST_ASSERT( uziLara.nmeshes == getNode()->getChildCount() );
                        getNode()->getChild( 14 )
                                 ->setDrawable(
                                         getLevel().getModel( getLevel().m_meshIndices[uziLara.frame_number + 14] ) );
                    }
                }
                if( getLevel().m_cameraController->getMode() != CameraMode::Cinematic
                    && getLevel().m_cameraController->getMode() != CameraMode::FreeLook )
                {
                    getLevel().m_cameraController->setMode( CameraMode::Combat );
                }
                updateNotShotgun( gunType );
                break;
            case WeaponId::Shotgun:
                if( shotgunAmmo.ammo != 0 )
                {
                    if( getLevel().m_inputHandler->getInputState().action )
                    {
                        const auto& uziLara = *getLevel().m_animatedModels[4];
                        BOOST_ASSERT( uziLara.nmeshes == getNode()->getChildCount() );
                        getNode()->getChild( 14 )
                                 ->setDrawable(
                                         getLevel().getModel( getLevel().m_meshIndices[uziLara.frame_number + 14] ) );
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

void LaraNode::updateNotShotgun(WeaponId weaponId)
{
    BOOST_ASSERT( weapons.find( weaponId ) != weapons.end() );
    auto weapon = &weapons[weaponId];
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

void LaraNode::updateAimingState(const engine::LaraNode::Weapon& weapon)
{
    if( target == nullptr )
    {
        rightArm.aiming = false;
        leftArm.aiming = false;
        m_enemyLookRot.X = 0_deg;
        m_enemyLookRot.Y = 0_deg;
        return;
    }

    core::RoomBoundPosition laraHead;
    laraHead.position.X = m_state.position.position.X;
    laraHead.position.Y = m_state.position.position.Y - 650;
    laraHead.position.Z = m_state.position.position.Z;
    laraHead.room = m_state.position.room;
    auto enemyChestPos = getUpperThirdBBoxCtr( *target );
    auto sightAngle = anglesFromPosition( enemyChestPos.position - laraHead.position );
    sightAngle.Y -= m_state.rotation.Y;
    sightAngle.X -= m_state.rotation.X;
    if( !CameraController::clampPosition( laraHead, enemyChestPos, getLevel() ) )
    {
        rightArm.aiming = false;
        leftArm.aiming = false;
    }
    else if( sightAngle.Y < weapon.sightAngleRange.y.min
            || sightAngle.Y > weapon.sightAngleRange.y.max
            || sightAngle.X < weapon.sightAngleRange.x.min
            || sightAngle.X > weapon.sightAngleRange.x.max )
    {
        if( leftArm.aiming )
        {
            if( sightAngle.Y < weapon.targetingAngleRangeLeft.y.min
                || sightAngle.Y > weapon.targetingAngleRangeLeft.y.max
                || sightAngle.X < weapon.targetingAngleRangeLeft.x.min
                || sightAngle.X > weapon.targetingAngleRangeLeft.x.max )
            {
                leftArm.aiming = false;
            }
        }
        if( rightArm.aiming )
        {
            if( sightAngle.Y < weapon.targetingAngleRangeRight.y.min
                || sightAngle.Y > weapon.targetingAngleRangeRight.y.max
                || sightAngle.X < weapon.targetingAngleRangeRight.x.min
                || sightAngle.X > weapon.targetingAngleRangeRight.x.max )
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
    m_enemyLookRot = sightAngle;
}

void LaraNode::unholsterReplaceMeshes()
{
    rightArm.frame = 0;
    leftArm.frame = 0;
    leftArm.aimRotation.Z = 0_deg;
    leftArm.aimRotation.Y = 0_deg;
    leftArm.aimRotation.X = 0_deg;
    rightArm.aimRotation.Z = 0_deg;
    rightArm.aimRotation.Y = 0_deg;
    rightArm.aimRotation.X = 0_deg;
    rightArm.aiming = false;
    leftArm.aiming = false;
    rightArm.shootTimeout = 0;
    leftArm.shootTimeout = 0;
    target = nullptr;
    if( gunType == WeaponId::None )
    {
        const auto* positionData = getLevel().m_animatedModels[0]->frame_base;

        rightArm.weaponAnimData = positionData;
        leftArm.weaponAnimData = positionData;
    }
    else if( gunType == WeaponId::Pistols || gunType == WeaponId::AutoPistols || gunType == WeaponId::Uzi )
    {
        const auto* positionData = getLevel().m_animatedModels[1]->frame_base;

        rightArm.weaponAnimData = positionData;
        leftArm.weaponAnimData = positionData;

        if( m_handStatus != HandStatus::None )
        {
            overrideLaraMeshesUnholsterBothLegs( gunType );
        }
    }
    else if( gunType == WeaponId::Shotgun )
    {
        const auto* positionData = getLevel().m_animatedModels[2]->frame_base;

        rightArm.weaponAnimData = positionData;
        leftArm.weaponAnimData = positionData;

        if( m_handStatus != HandStatus::None )
        {
            overrideLaraMeshesUnholsterShotgun();
        }
    }
    else
    {
        const auto* positionData = getLevel().m_animatedModels[0]->frame_base;

        rightArm.weaponAnimData = positionData;
        leftArm.weaponAnimData = positionData;
    }
}

core::RoomBoundPosition LaraNode::getUpperThirdBBoxCtr(const ModelItemNode& item)
{
    auto kf = item.getSkeleton()->getInterpolationInfo( item.m_state ).getNearestFrame();

    auto ctrX = (kf->bbox.minX + kf->bbox.maxX) / 2;
    auto ctrZ = (kf->bbox.minZ + kf->bbox.maxZ) / 2;
    auto ctrY3 = (kf->bbox.minY + kf->bbox.maxY) / 3 + kf->bbox.minY;

    auto cos = item.m_state.rotation.Y.cos();
    auto sin = item.m_state.rotation.Y.sin();

    core::RoomBoundPosition result{item.m_state.position.room};
    result.position.X = ctrZ * sin + ctrX * cos + item.m_state.position.position.X;
    result.position.Y = ctrY3 + item.m_state.position.position.Y;
    result.position.Z = ctrZ * cos - ctrX * sin + item.m_state.position.position.Z;
    return result;
}

void LaraNode::unholsterDoubleWeapon(engine::LaraNode::WeaponId weaponId)
{
    auto nextFrame = leftArm.frame + 1;
    if( nextFrame < 5 || nextFrame > 23 )
    {
        nextFrame = 5;
    }
    else if( nextFrame == 13 )
    {
        overrideLaraMeshesUnholsterBothLegs( weaponId );
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

void LaraNode::findTarget(const engine::LaraNode::Weapon& weapon)
{
    core::RoomBoundPosition origin{m_state.position};
    origin.position.Y -= 650;
    std::shared_ptr<ModelItemNode> bestEnemy = nullptr;
    core::Angle bestXAngle{std::numeric_limits<int16_t>::max()};
    const auto maxDistanceSq = weapon.maxDistance * weapon.maxDistance;
    for( const std::shared_ptr<engine::items::ItemNode>& currentEnemy : getLevel().m_itemNodes
                                                                        | boost::adaptors::map_values )
    {
        if( currentEnemy->m_state.health <= 0 )
            continue;

        const auto modelEnemy = std::dynamic_pointer_cast<ModelItemNode>(currentEnemy);
        if(modelEnemy == nullptr)
        {
            BOOST_LOG_TRIVIAL(warning) << "Ignoring non-model item " << currentEnemy->getNode()->getId();
            continue;
        }

        auto dx = currentEnemy->m_state.position.position.X - origin.position.X;
        auto dy = currentEnemy->m_state.position.position.Y - origin.position.Y;
        auto dz = currentEnemy->m_state.position.position.Z - origin.position.Z;
        if( std::abs( dx ) > weapon.maxDistance )
            continue;

        if( std::abs( dy ) > weapon.maxDistance )
            continue;

        if( std::abs( dz ) > weapon.maxDistance )
            continue;

        if( dx * dx + dy * dy + dz * dz >= maxDistanceSq )
            continue;

        auto target = getUpperThirdBBoxCtr( *std::dynamic_pointer_cast<const ModelItemNode>( currentEnemy ) );
        if( !CameraController::clampPosition( origin, target, getLevel() ) )
            continue;

        auto aimAngle = anglesFromPosition( target.position - origin.position );
        aimAngle.X -= m_torsoRotation.Y + m_state.rotation.Y;
        aimAngle.Y -= m_torsoRotation.X + m_state.rotation.X;
        if( aimAngle.Y < weapon.sightAngleRange.y.min || aimAngle.Y > weapon.sightAngleRange.y.max
            || aimAngle.X < weapon.sightAngleRange.x.min || aimAngle.X > weapon.sightAngleRange.x.max )
            continue;

        auto absX = abs( aimAngle.X );
        if( absX >= bestXAngle )
            continue;

        bestXAngle = absX;
        bestEnemy = modelEnemy;
    }
    target = bestEnemy;
    updateAimingState( weapon );
}

void LaraNode::initAimInfoPistol()
{
    m_handStatus = HandStatus::Combat;
    leftArm.aimRotation.Z = 0_deg;
    leftArm.aimRotation.Y = 0_deg;
    leftArm.aimRotation.X = 0_deg;
    rightArm.aimRotation.Z = 0_deg;
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

    rightArm.weaponAnimData = getLevel().m_animatedModels[1]->frame_base;
    leftArm.weaponAnimData = rightArm.weaponAnimData;
}

void LaraNode::initAimInfoShotgun()
{
    m_handStatus = HandStatus::Combat;
    leftArm.aimRotation.Z = 0_deg;
    leftArm.aimRotation.Y = 0_deg;
    leftArm.aimRotation.X = 0_deg;
    rightArm.aimRotation.Z = 0_deg;
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

    rightArm.weaponAnimData = getLevel().m_animatedModels[2]->frame_base;
    leftArm.weaponAnimData = rightArm.weaponAnimData;
}

void LaraNode::overrideLaraMeshesUnholsterBothLegs(engine::LaraNode::WeaponId weaponId)
{
    int id;
    if( weaponId == WeaponId::AutoPistols )
    {
        id = 3;
    }
    else if( weaponId == WeaponId::Uzi )
    {
        id = 4;
    }
    else
    {
        id = 1;
    }

    const auto& src = *getLevel().m_animatedModels[id];
    BOOST_ASSERT( src.nmeshes == getNode()->getChildCount() );
    getNode()->getChild( 1 )
             ->setDrawable( getLevel().getModel( getLevel().m_meshIndices[src.frame_number + 1] ) );
    getNode()->getChild( 4 )
             ->setDrawable( getLevel().getModel( getLevel().m_meshIndices[src.frame_number + 4] ) );
    getNode()->getChild( 10 )
             ->setDrawable( getLevel().getModel( getLevel().m_meshIndices[src.frame_number + 10] ) );
    getNode()->getChild( 13 )
             ->setDrawable( getLevel().getModel( getLevel().m_meshIndices[src.frame_number + 13] ) );
}

void LaraNode::overrideLaraMeshesUnholsterShotgun()
{
    const auto& src = *getLevel().m_animatedModels[2];
    BOOST_ASSERT( src.nmeshes == getNode()->getChildCount() );
    getNode()->getChild( 7 )
             ->setDrawable( getLevel().getModel( getLevel().m_meshIndices[src.frame_number + 7] ) );
    getNode()->getChild( 10 )
             ->setDrawable( getLevel().getModel( getLevel().m_meshIndices[src.frame_number + 10] ) );
    getNode()->getChild( 13 )
             ->setDrawable( getLevel().getModel( getLevel().m_meshIndices[src.frame_number + 13] ) );
}

void LaraNode::unholsterShotgunAnimUpdate()
{
    auto nextFrame = leftArm.frame + 1;
    if( nextFrame < 5 || nextFrame > 47 )
    {
        nextFrame = 13;
    }
    else if( nextFrame == 23 )
    {
        const auto& src = *getLevel().m_animatedModels[2];
        BOOST_ASSERT( src.nmeshes == getNode()->getChildCount() );

        getNode()->getChild( 7 )
                 ->setDrawable( getLevel().getModel( getLevel().m_meshIndices[src.frame_number + 7] ) );
        getNode()->getChild( 10 )
                 ->setDrawable( getLevel().getModel( getLevel().m_meshIndices[src.frame_number + 10] ) );
        getNode()->getChild( 13 )
                 ->setDrawable( getLevel().getModel( getLevel().m_meshIndices[src.frame_number + 13] ) );

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

void LaraNode::updateAimAngles(engine::LaraNode::Weapon& weapon, engine::LaraNode::AimInfo& aimInfo)
{
    core::Angle targetX = 0_deg;
    core::Angle targetY = 0_deg;
    if( aimInfo.aiming )
    {
        targetX = m_enemyLookRot.X;
        targetY = m_enemyLookRot.Y;
    }

    if( aimInfo.aimRotation.X >= targetX - weapon.rotationCone && aimInfo.aimRotation.X <= targetX + weapon.rotationCone )
    {
        aimInfo.aimRotation.X = targetX;
    }
    else if( aimInfo.aimRotation.X >= targetX )
    {
        aimInfo.aimRotation.X = aimInfo.aimRotation.X - weapon.rotationCone;
    }
    else
    {
        aimInfo.aimRotation.X = aimInfo.aimRotation.X + weapon.rotationCone;
    }

    if( aimInfo.aimRotation.Y >= targetY - weapon.rotationCone && aimInfo.aimRotation.Y <= weapon.rotationCone + targetY )
    {
        aimInfo.aimRotation.Y = targetY;
    }
    else if( aimInfo.aimRotation.Y >= targetY )
    {
        aimInfo.aimRotation.Y = aimInfo.aimRotation.Y - weapon.rotationCone;
    }
    else
    {
        aimInfo.aimRotation.Y = aimInfo.aimRotation.Y + weapon.rotationCone;
    }

    aimInfo.aimRotation.Z = 0_deg;
}

void LaraNode::updateAnimShotgun()
{
    auto aimingFrame = leftArm.frame;
    if( leftArm.aiming )
    {
        if( leftArm.frame < 0 || leftArm.frame >= 13 )
        {
            auto nextFrame = leftArm.frame + 1;
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
                else if( nextFrame == 57 )
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
        auto nextFrame = leftArm.frame + 1;
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
        else if( leftArm.frame <= 47 || leftArm.frame >= 80 )
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
            else if( nextFrame == 80 )
            {
                rightArm.frame = 114;
                leftArm.frame = 114;
                return;
            }
            else if( nextFrame == 57 )
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
        auto rand1 = (std::rand() & 0x7fff) - 0x4000;
        core::TRRotationXY aimAngle;
        aimAngle.X = (+20_deg * rand1 / 65536) + m_state.rotation.Y + leftArm.aimRotation.Y;
        auto rand2 = (std::rand() & 0x7fff) - 0x4000;
        aimAngle.Y = (+20_deg * rand2 / 65536) + leftArm.aimRotation.X;
        if( tryShoot( WeaponId::Shotgun, target, *this, aimAngle ) )
        {
            fireShotgun = true;
        }
    }
    if( fireShotgun )
    {
        getLevel().playSound( weapons[WeaponId::Shotgun].fireSoundId, getNode()->getTranslationWorld() );
    }
}

void LaraNode::playSingleShotShotgun()
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
            const auto& src = *getLevel().m_animatedModels[0];
            BOOST_ASSERT( src.nmeshes == getNode()->getChildCount() );
            getNode()->getChild( 7 )
                     ->setDrawable(
                             getLevel().getModel( getLevel().m_meshIndices[src.frame_number + 7] ) );
            getNode()->getChild( 10 )
                     ->setDrawable(
                             getLevel().getModel( getLevel().m_meshIndices[src.frame_number + 10] ) );
            getNode()->getChild( 13 )
                     ->setDrawable(
                             getLevel().getModel( getLevel().m_meshIndices[src.frame_number + 13] ) );

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

void LaraNode::playSingleShot(engine::LaraNode::WeaponId weaponId)
{
    if( leftArm.frame >= 24 )
    {
        leftArm.frame = 4;
    }
    else if( leftArm.frame > 0 && leftArm.frame < 5 )
    {
        leftArm.aimRotation.X -= leftArm.aimRotation.Y / leftArm.frame;
        leftArm.aimRotation.Y -= leftArm.aimRotation.Y / leftArm.frame;
        --leftArm.frame;
    }
    else if( leftArm.frame == 0 )
    {
        leftArm.aimRotation.Z = 0_deg;
        leftArm.aimRotation.Y = 0_deg;
        leftArm.aimRotation.X = 0_deg;
        leftArm.frame = 23;
    }
    else if( leftArm.frame > 5 && leftArm.frame < 24 )
    {
        --leftArm.frame;
        if( leftArm.frame == 12 )
        {
            int srcId = 1;
            if( weaponId == WeaponId::AutoPistols )
            {
                srcId = 3;
            }
            else if( weaponId == WeaponId::Uzi )
            {
                srcId = 4;
            }

            const auto& src = *getLevel().m_animatedModels[srcId];
            BOOST_ASSERT( src.nmeshes == getNode()->getChildCount() );
            getNode()->getChild( 1 )
                     ->setDrawable(
                             getLevel().getModel( getLevel().m_meshIndices[src.frame_number + 1] ) );
            getNode()->getChild( 13 )
                     ->setDrawable(
                             getLevel().getModel( getLevel().m_meshIndices[src.frame_number + 13] ) );

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
        rightArm.aimRotation.Z = 0_deg;
        rightArm.aimRotation.Y = 0_deg;
        rightArm.aimRotation.X = 0_deg;
        rightArm.frame = 23;
    }
    else if( rightArm.frame > 5 && rightArm.frame < 24 )
    {
        --rightArm.frame;
        if( rightArm.frame == 12 )
        {
            int srcId = 1;
            if( weaponId == WeaponId::AutoPistols )
            {
                srcId = 3;
            }
            else if( weaponId == WeaponId::Uzi )
            {
                srcId = 4;
            }

            const auto& src = *getLevel().m_animatedModels[srcId];
            BOOST_ASSERT( src.nmeshes == getNode()->getChildCount() );
            getNode()->getChild( 4 )
                     ->setDrawable(
                             getLevel().getModel( getLevel().m_meshIndices[src.frame_number + 4] ) );
            getNode()->getChild( 10 )
                     ->setDrawable(
                             getLevel().getModel( getLevel().m_meshIndices[src.frame_number + 10] ) );

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

    m_headRotation.Y = rightArm.aimRotation.X / 4;
    m_torsoRotation.Y = rightArm.aimRotation.X / 4;
    m_headRotation.X = (rightArm.aimRotation.X + leftArm.aimRotation.X) / 4;
    m_torsoRotation.X = (rightArm.aimRotation.X + leftArm.aimRotation.X) / 4;
}

void LaraNode::updateAnimNotShotgun(engine::LaraNode::WeaponId weaponId)
{
    const auto& weapon = weapons[weaponId];

    if( !rightArm.aiming && (!getLevel().m_inputHandler->getInputState().action || target!=nullptr) )
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
        aimAngle.Y = m_state.rotation.Y + rightArm.aimRotation.Y;
        aimAngle.X = rightArm.aimRotation.X;
        if( tryShoot( weaponId, target, *this, aimAngle ) )
        {
            rightArm.shootTimeout = weapon.shootTimeout;
            getLevel().playSound( weapon.fireSoundId, getNode()->getTranslationWorld() );
        }
        rightArm.frame = 24;
    }
    else if( rightArm.frame >= 24 )
    {
        ++rightArm.frame;
        if( rightArm.frame == weapon.field_28 + 24 )
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
        if( tryShoot( weaponId, target, *this, aimAngle ) )
        {
            leftArm.shootTimeout = weapon.shootTimeout;
            getLevel().playSound( weapon.fireSoundId, getNode()->getTranslationWorld() );
        }
        leftArm.frame = 24;
    }
    else if( leftArm.frame >= 24 )
    {
        ++leftArm.frame;
        if( leftArm.frame == weapon.field_28 + 24 )
        {
            leftArm.frame = 4;
        }
    }
}

bool LaraNode::tryShoot(engine::LaraNode::WeaponId weaponId,
                        const std::shared_ptr<engine::items::ModelItemNode>& target,
                        const engine::items::ModelItemNode& gunHolder,
                        const core::TRRotationXY& aimAngle)
{
    Expects( weaponId != WeaponId::None );

    Ammo* ammoPtr = nullptr;
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
    auto weapon = &weapons[weaponId];
    core::TRCoordinates gunPosition = gunHolder.m_state.position.position;
    gunPosition.Y -= weapon->height;
    core::TRRotation shootVector{
            weapon->anglePrecision * ((std::rand() & 0x7fff) - 0x4000) / 65536 + aimAngle.X,
            weapon->anglePrecision * ((std::rand() & 0x7fff) - 0x4000) / 65536 + aimAngle.Y,
            +0_deg
    };
    auto m = glm::translate( shootVector.toMatrix(), gunPosition.toRenderSystem() );
    // render::initViewMatrix(&v_sourcePosAngle);
    std::vector<SkeletalModelNode::Cylinder> cylinders;
    if(target != nullptr)
    {
        cylinders = target->getSkeleton()->getBoneCollisionCylinders( target->m_state, *target->getSkeleton()
                                                                                              ->getInterpolationInfo(
                                                                                                      target->m_state )
                                                                                              .getNearestFrame(),
                                                                      nullptr );
    }
    bool hasHit = false;
    int minD = std::numeric_limits<int>::max();
    if( !cylinders.empty() )
    {
        for( const auto& cylinder : cylinders )
        {
            if( std::abs( cylinder.position.X ) >= cylinder.radius )
                continue;

            if( std::abs( cylinder.position.Y ) >= cylinder.radius )
                continue;

            if( cylinder.radius >= cylinder.position.Z )
                continue;

            if( cylinder.position.Z - cylinder.radius >= minD )
                continue;

            const auto d = cylinder.position.Z - cylinder.radius;
            if( util::square( cylinder.position.Y ) + util::square( cylinder.position.X )
                > cylinder.radius * cylinder.radius )
                continue;

            minD = d;
            hasHit = true;
        }
    }
    core::RoomBoundPosition bulletPos{gunHolder.m_state.position.room};
    bulletPos.position = gunPosition;

    if( !hasHit )
    {
        ++ammoPtr->misses;
        core::RoomBoundPosition hitPos{gunHolder.m_state.position.room};
        static constexpr float VeryLargeDistanceProbablyClipping = 1 << 14;
        // FIXME this causes an assert to fail
        hitPos.position = bulletPos.position + core::TRCoordinates{glm::vec3{m[3]} * VeryLargeDistanceProbablyClipping};
        CameraController::clampPosition( bulletPos, hitPos, getLevel() );
        playShotMissed( hitPos );
    }
    else
    {
        ++ammoPtr->hits;
        core::TRCoordinates hitPos{float( minD ) * glm::vec3{m[3]} + bulletPos.position.toRenderSystem()};
        if(target != nullptr)
            enemyHit( *target, hitPos, weapon->healthDamage );
    }

    return true;
}

void LaraNode::playShotMissed(const core::RoomBoundPosition& pos)
{
    // TODO emit particles
    getLevel().playSound(10, pos.position.toRenderSystem());
}

void LaraNode::enemyHit(engine::items::ModelItemNode& item, const core::TRCoordinates& pos, int healthDamage)
{
    if ( item.m_state.health > 0 && item.m_state.health <= healthDamage )
    {
        // TODO ++g_numKills;
    }
    item.m_state.is_hit = true;
    item.m_state.health -= healthDamage;
    // TODO emitBloodSplatterFx(pos.X, pos.Y, pos.Z, item.m_state.speed, item.m_state.rotation.Y, item.m_state.position.room);
    if( item.m_state.health <= 0 )
        return;

    int soundId;
    switch(item.m_state.object_number)
    {
        case 7:
            soundId = 20;
            break;
        case 8:
            soundId = 16;
            break;
        case 12:
        case 13:
            soundId = 0x55;
            break;
        case 16:
            soundId = 0x5f;
            break;
        case 30:
            soundId = 0x84;
            break;
        case 34:
            soundId = 0x8e;
            break;
        default:
            return;
    }

    getLevel().playSound( soundId, item.m_state.position.position.toRenderSystem() );
}

void LaraNode::drawRoutine()
{
    auto interpolationInfo = getSkeleton()->getInterpolationInfo(m_state);
    if ( false && !hit_direction.is_initialized() && interpolationInfo.secondFrame != nullptr )
    {
        drawRoutineInterpolated(interpolationInfo);
    }
    else
    {
        const auto& objInfo = *getLevel().m_animatedModels[m_state.object_number];
        const loader::AnimFrame* frame = nullptr;
        if ( !hit_direction.is_initialized() )
        {
            frame = interpolationInfo.firstFrame;
        }
        else
        {
            switch ( *hit_direction )
            {
                case core::Axis::PosX:
                    frame = getLevel().m_animations[127].poseData;
                    break;
                case core::Axis::NegZ:
                    frame = getLevel().m_animations[126].poseData;
                    break;
                case core::Axis::NegX:
                    frame = getLevel().m_animations[128].poseData;
                    break;
                default:
                    frame = getLevel().m_animations[125].poseData;
                    break;
            }
            frame = frame->next(hit_frame);
        }
        std::stack<glm::mat4> matrixStack;

#define mPushMatrix() matrixStack.push(matrixStack.top())
#define mPopMatrix() matrixStack.pop()
#define mTranslateAbsXYZ(POS) matrixStack.top()[3] = glm::vec4{POS, 1.0f}
#define mRotYXZ(ROT) matrixStack.top() *= (ROT).toMatrix()
#define mTranslateXYZ(POS) matrixStack.top() = glm::translate(matrixStack.top(), POS)
#define mRotPackedYXZ(ROT) matrixStack.top() *= core::fromPackedAngles(ROT)
#define renderMesh(IDX) getSkeleton()->getChild(IDX)->setLocalMatrix(matrixStack.top())

        matrixStack.push(glm::mat4{1.0f});

        //mPushMatrix();
        //mTranslateAbsXYZ(m_state.position.position.toRenderSystem());
        //mRotYXZ(m_state.rotation);

        BOOST_ASSERT(matrixStack.size() == 1);
        const auto rootMatrix = matrixStack.top();

        mPushMatrix();
        // render::calcMeshLighting(aLara, &v_frame->bbox);
        const auto* boneTree = reinterpret_cast<const BoneTreeEntry*>(&getLevel().m_boneTrees[objInfo.bone_index]);
        mTranslateXYZ(frame->pos.toGl());
        auto angleData = frame->getAngleData();
        mRotPackedYXZ(angleData[0]);
        renderMesh(0);

        mPushMatrix();
        mTranslateXYZ(boneTree[0].toGl());
        mRotPackedYXZ(angleData[1]);
        renderMesh(1);

        mTranslateXYZ(boneTree[1].toGl());
        mRotPackedYXZ(angleData[2]);
        renderMesh(2);

        mTranslateXYZ(boneTree[2].toGl());
        mRotPackedYXZ(angleData[3]);
        renderMesh(3);

        mPopMatrix();
        mPushMatrix();
        mTranslateXYZ(boneTree[3].toGl());
        mRotPackedYXZ(angleData[4]);
        renderMesh(4);

        mTranslateXYZ(boneTree[4].toGl());
        mRotPackedYXZ(angleData[5]);
        renderMesh(5);

        mTranslateXYZ(boneTree[5].toGl());
        mRotPackedYXZ(angleData[6]);
        renderMesh(6);

        mPopMatrix();
        mTranslateXYZ(boneTree[6].toGl());
        mRotPackedYXZ(angleData[7]);
        mRotYXZ(m_torsoRotation);
        renderMesh(7);

        mPushMatrix();
        mTranslateXYZ(boneTree[13].toGl());
        mRotPackedYXZ(angleData[14]);
        mRotYXZ(m_headRotation);
        renderMesh(14);

        WeaponId activeGunType = WeaponId::None;
        if ( m_handStatus == HandStatus::Combat || m_handStatus == HandStatus::Unholster || m_handStatus == HandStatus::Holster )
        {
            activeGunType = gunType;
        }

        mPopMatrix();
        gsl::span<const uint32_t> armAngleData;
        switch ( activeGunType )
        {
            case WeaponId::None:
                mPushMatrix();
                mTranslateXYZ(boneTree[7].toGl());
                mRotPackedYXZ(angleData[8]);
                renderMesh(8);

                mTranslateXYZ(boneTree[8].toGl());
                mRotPackedYXZ(angleData[9]);
                renderMesh(9);

                mTranslateXYZ(boneTree[9].toGl());
                mRotPackedYXZ(angleData[10]);
                renderMesh(10);

                mPopMatrix();
                mPushMatrix();
                mTranslateXYZ(boneTree[10].toGl());
                mRotPackedYXZ(angleData[11]);
                renderMesh(11);

                mTranslateXYZ(boneTree[11].toGl());
                mRotPackedYXZ(angleData[12]);
                renderMesh(12);

                mTranslateXYZ(boneTree[12].toGl());
                mRotPackedYXZ(angleData[13]);
                renderMesh(13);
                break;
            case WeaponId::Pistols:
            case WeaponId::AutoPistols:
            case WeaponId::Uzi:
                mPushMatrix();
                mTranslateXYZ(boneTree[7].toGl());
                matrixStack.top()[0] = rootMatrix[0];
                matrixStack.top()[1] = rootMatrix[1];
                matrixStack.top()[2] = rootMatrix[2];
                mRotYXZ(rightArm.aimRotation);

                armAngleData = rightArm.weaponAnimData->next(rightArm.frame)->getAngleData();
                mRotPackedYXZ(armAngleData[8]);
                renderMesh(8);

                mTranslateXYZ(boneTree[8].toGl());
                mRotPackedYXZ(armAngleData[9]);
                renderMesh(9);

                mTranslateXYZ(boneTree[9].toGl());
                mRotPackedYXZ(armAngleData[10]);
                renderMesh(10);

                if ( rightArm.shootTimeout != 0 )
                {
                    // TODO qmemcpy(&v24, MATRIX::stackPtr, sizeof(v24));
                }
                mPopMatrix();
                mPushMatrix();
                mTranslateXYZ(boneTree[10].toGl());
                matrixStack.top()[0] = rootMatrix[0];
                matrixStack.top()[1] = rootMatrix[1];
                matrixStack.top()[2] = rootMatrix[2];
                mRotYXZ(leftArm.aimRotation);
                armAngleData = leftArm.weaponAnimData->next(leftArm.frame)->getAngleData();
                mRotPackedYXZ(armAngleData[11]);
                renderMesh(11);

                mTranslateXYZ(boneTree[11].toGl());
                mRotPackedYXZ(armAngleData[12]);
                renderMesh(12);

                mTranslateXYZ(boneTree[12].toGl());
                mRotPackedYXZ(armAngleData[13]);
                renderMesh(13);

                if ( leftArm.shootTimeout != 0 )
                {
                    // TODO renderGunFlare(activeGunType);
                }
                if ( rightArm.shootTimeout != 0 )
                {
                    // TODO qmemcpy(MATRIX::stackPtr, &v24, sizeof(MATRIX));
                    // TODO renderGunFlare(activeGunType);
                }
                break;
            case WeaponId::Shotgun:
                mPushMatrix();
                armAngleData = rightArm.weaponAnimData->next(rightArm.frame)->getAngleData();
                mTranslateXYZ(boneTree[7].toGl());
                mRotPackedYXZ(armAngleData[8]);
                renderMesh(8);

                mTranslateXYZ(boneTree[8].toGl());
                mRotPackedYXZ(armAngleData[9]);
                renderMesh(9);

                mTranslateXYZ(boneTree[9].toGl());
                mRotPackedYXZ(armAngleData[10]);
                renderMesh(10);

                mPopMatrix();
                mPushMatrix();
                armAngleData = leftArm.weaponAnimData->next(leftArm.frame)->getAngleData();
                mTranslateXYZ(boneTree[10].toGl());
                mRotPackedYXZ(armAngleData[11]);
                renderMesh(11);

                mTranslateXYZ(boneTree[11].toGl());
                mRotPackedYXZ(armAngleData[12]);
                renderMesh(12);

                mTranslateXYZ(boneTree[12].toGl());
                mRotPackedYXZ(armAngleData[13]);
                renderMesh(13);
                break;
            default:
                break;
        }
    }
}

void LaraNode::drawRoutineInterpolated(const engine::SkeletalModelNode::InterpolationInfo& interpolationInfo)
{

}
}
                                 