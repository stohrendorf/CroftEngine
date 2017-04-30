#include "laranode.h"

#include "cameracontroller.h"
#include "level/level.h"
#include "render/textureanimator.h"

#include "items/block.h"
#include "items/tallblock.h"

#include <boost/range/adaptors.hpp>


namespace
{
    std::array<engine::floordata::ActivationState, 10> mapFlipActivationStates;


    void swapWithAlternate(loader::Room& orig, loader::Room& alternate)
    {
        // find any blocks in the original room and un-patch the floor heights
        for( const auto& child : orig.node->getChildren() )
        {
            if( auto tmp = std::dynamic_pointer_cast<engine::items::Block>(child) )
            {
                loader::Room::patchHeightsForBlock(*tmp, loader::SectorSize);
            }
            else if( auto tmp = std::dynamic_pointer_cast<engine::items::TallBlock>(child) )
            {
                loader::Room::patchHeightsForBlock(*tmp, loader::SectorSize * 2);
            }
        }

        // now swap the rooms and patch the alternate room ids
        std::swap(orig, alternate);
        orig.alternateRoom = alternate.alternateRoom;
        alternate.alternateRoom = -1;

        // move all items over
        orig.node->swapChildren(alternate.node);

        // patch heights in the new room.
        // note that this is exactly the same code as above,
        // except for the heights.
        for( const auto& child : orig.node->getChildren() )
        {
            if( auto tmp = std::dynamic_pointer_cast<engine::items::Block>(child) )
            {
                loader::Room::patchHeightsForBlock(*tmp, -loader::SectorSize);
            }
            else if( auto tmp = std::dynamic_pointer_cast<engine::items::TallBlock>(child) )
            {
                loader::Room::patchHeightsForBlock(*tmp, -loader::SectorSize * 2);
            }
        }
    }


    void swapAllRooms(level::Level& level)
    {
        for( auto& room : level.m_rooms )
        {
            if( room.alternateRoom < 0 )
                continue;

            BOOST_ASSERT(static_cast<size_t>(room.alternateRoom) < level.m_rooms.size());
            swapWithAlternate(room, level.m_rooms[room.alternateRoom]);
        }

        level.roomsAreSwapped = !level.roomsAreSwapped;
    }
}


namespace engine
{
    void LaraNode::setTargetState(LaraStateId st)
    {
        ItemNode::setTargetState(static_cast<uint16_t>(st));
    }


    loader::LaraStateId LaraNode::getTargetState() const
    {
        return static_cast<LaraStateId>(ItemNode::getTargetState());
    }


    void LaraNode::setAnimIdGlobal(loader::AnimationId anim, const boost::optional<uint16_t>& firstFrame)
    {
        ItemNode::setAnimIdGlobal(static_cast<uint16_t>(anim), firstFrame.get_value_or(0));
    }


    void LaraNode::handleLaraStateOnLand()
    {
        CollisionInfo collisionInfo;
        collisionInfo.oldPosition = getPosition();
        collisionInfo.collisionRadius = 100; //!< @todo MAGICK 100
        collisionInfo.policyFlags = CollisionInfo::EnableSpaz | CollisionInfo::EnableBaddiePush;

        lara::AbstractStateHandler::create(getCurrentAnimState(), *this)->handleInput(collisionInfo);

        if(getLevel().m_cameraController->getCamOverrideType() != CamOverrideType::FreeLook)
        {
            const auto headX = getLevel().m_cameraController->getHeadRotation().X;
            if (headX <= -2_deg || headX >= 2_deg)
            {
                getLevel().m_cameraController->setHeadRotationX(headX - headX / 8);
            }
            else
            {
                getLevel().m_cameraController->setHeadRotationX(0_deg);
            }
            const auto headY = getLevel().m_cameraController->getHeadRotation().Y;
            if (headY <= -2_deg || headY >= 2_deg)
            {
                getLevel().m_cameraController->setHeadRotationY(headY - headY / 8);
            }
            else
            {
                getLevel().m_cameraController->setHeadRotationY(0_deg);
            }
            getLevel().m_cameraController->setTorsoRotation(getLevel().m_cameraController->getHeadRotation());
        }

        // "slowly" revert rotations to zero
        if(getRotation().Z < -1_deg)
        {
            addZRotation(+1_deg);
            if(getRotation().Z >= 0_deg)
                setZRotation(0_deg);
        }
        else if(getRotation().Z > 1_deg)
        {
            addZRotation(-1_deg);
            if(getRotation().Z <= 0_deg)
                setZRotation(0_deg);
        }

        if(m_yRotationSpeed > 2_deg)
        {
            m_yRotationSpeed -= 2_deg;
        }
        else if(m_yRotationSpeed < -2_deg)
        {
            m_yRotationSpeed += 2_deg;
        }
        else
        {
            m_yRotationSpeed = 0_deg;
        }

        addYRotation(m_yRotationSpeed);

        updateImpl();

        testInteractions();

        lara::AbstractStateHandler::create(getCurrentAnimState(), *this)->postprocessFrame(collisionInfo);

        updateFloorHeight(-381);

        //! @todo updateWeaponState()

        handleCommandSequence(collisionInfo.mid.floor.lastCommandSequenceOrDeath, false);

        applyTransform();

#ifndef NDEBUG
        lastUsedCollisionInfo = collisionInfo;
#endif
    }


    void LaraNode::handleLaraStateDiving()
    {
#if 0
        CollisionInfo collisionInfo;
        collisionInfo.position = getPosition();
        collisionInfo.collisionRadius = 300; //!< @todo MAGICK 300
        collisionInfo.policyFlags &= ~(CollisionInfo::EnableSpaz | CollisionInfo::EnableBaddiePush
                                        | CollisionInfo::LavaIsPit
                                        | CollisionInfo::SlopesArePits
                                        | CollisionInfo::SlopesAreWalls);
        collisionInfo.neededCeilingDistance = 400;
        collisionInfo.passableFloorDistanceBottom = loader::HeightLimit;
        collisionInfo.passableFloorDistanceTop = -400;

        BOOST_ASSERT( m_currentStateHandler != nullptr );

        auto nextHandler = m_currentStateHandler->handleInput(collisionInfo);

        m_currentStateHandler->animate(collisionInfo, deltaTime);

        if( nextHandler.is_initialized() && *nextHandler != m_currentStateHandler->getId() )
        {
            m_currentStateHandler = lara::AbstractStateHandler::create(*nextHandler, *this);
            BOOST_LOG_TRIVIAL( debug ) << "New input state override: "
                                      << loader::toString(m_currentStateHandler->getId());
        }

        // "slowly" revert rotations to zero
        if( getRotation().Z < 0_deg )
        {
            addZRotation(core::makeInterpolatedValue(+2_deg).getScaled(deltaTime));
            if( getRotation().Z >= 0_deg )
                setZRotation(0_deg);
        }
        else if( getRotation().Z > 0_deg )
        {
            addZRotation(-core::makeInterpolatedValue(+2_deg).getScaled(deltaTime));
            if( getRotation().Z <= 0_deg )
                setZRotation(0_deg);
        }
        setXRotation(util::clamp(getRotation().X, -100_deg, +100_deg));
        setZRotation(util::clamp(getRotation().Z, -22_deg, +22_deg));
        {
            auto pos = getPosition();
            pos.X += getRotation().Y.sin() * getRotation().X.cos() * getFallSpeed().getScaled(deltaTime) / 4;
            pos.Y -= getRotation().X.sin() * getFallSpeed().getScaled(deltaTime) / 4;
            pos.Z += getRotation().Y.cos() * getRotation().X.cos() * getFallSpeed().getScaled(deltaTime) / 4;
            setPosition(pos);
        }

        /*
                if( getCurrentFrameChangeType() == FrameChangeType::SameFrame )
                {
        #ifndef NDEBUG
                    lastUsedCollisionInfo = collisionInfo;
        #endif
                    return;
                }
                */

        testInteractions();

        if( frameChangeType.is_initialized() )
        {
            nextHandler = m_currentStateHandler->postprocessFrame(collisionInfo);
            if( nextHandler.is_initialized() && *nextHandler != m_currentStateHandler->getId() )
            {
                m_currentStateHandler = lara::AbstractStateHandler::create(*nextHandler, *this);
                BOOST_LOG_TRIVIAL(debug) << "New post-processing state override: "
                                        << loader::toString(m_currentStateHandler->getId());
            }
        }

        updateFloorHeight(0);
        handleCommandSequence(collisionInfo.mid.floor.lastCommandSequenceOrDeath, false);
#ifndef NDEBUG
        lastUsedCollisionInfo = collisionInfo;
#endif
#endif
    }


    void LaraNode::handleLaraStateSwimming()
    {
#if 0
        CollisionInfo collisionInfo;
        collisionInfo.position = getPosition();
        collisionInfo.collisionRadius = 100; //!< @todo MAGICK 100
        collisionInfo.policyFlags &= ~(CollisionInfo::EnableSpaz | CollisionInfo::EnableBaddiePush
                                        | CollisionInfo::LavaIsPit
                                        | CollisionInfo::SlopesArePits
                                        | CollisionInfo::SlopesAreWalls);
        collisionInfo.neededCeilingDistance = 100;
        collisionInfo.passableFloorDistanceBottom = loader::HeightLimit;
        collisionInfo.passableFloorDistanceTop = -100;

        setCameraRotationX(-22_deg);

        BOOST_ASSERT( m_currentStateHandler != nullptr );

        auto nextHandler = m_currentStateHandler->handleInput(collisionInfo);

        m_currentStateHandler->animate(collisionInfo, deltaTime);

        if( nextHandler.is_initialized() && *nextHandler != m_currentStateHandler->getId() )
        {
            m_currentStateHandler = lara::AbstractStateHandler::create(*nextHandler, *this);
            BOOST_LOG_TRIVIAL( debug ) << "New input state override: "
                                      << loader::toString(m_currentStateHandler->getId());
        }

        // "slowly" revert rotations to zero
        if( getRotation().Z < 0_deg )
        {
            addZRotation(core::makeInterpolatedValue(+2_deg).getScaled(deltaTime));
            if( getRotation().Z >= 0_deg )
                setZRotation(0_deg);
        }
        else if( getRotation().Z > 0_deg )
        {
            addZRotation(-core::makeInterpolatedValue(+2_deg).getScaled(deltaTime));
            if( getRotation().Z <= 0_deg )
                setZRotation(0_deg);
        }

        setPosition(getPosition() + core::ExactTRCoordinates(
                        getMovementAngle().sin() * getFallSpeed().getScaled(deltaTime) / 4,
                        0,
                        getMovementAngle().cos() * getFallSpeed().getScaled(deltaTime) / 4
                    ));

        if( getLevel().m_cameraController->getCamOverrideType() != CamOverrideType::FreeLook )
        {
            auto x = makeInterpolatedValue(getLevel().m_cameraController->getHeadRotation().X * 0.125f)
                .getScaled(deltaTime);
            auto y = makeInterpolatedValue(getLevel().m_cameraController->getHeadRotation().Y * 0.125f)
                .getScaled(deltaTime);
            getLevel().m_cameraController->addHeadRotationXY(-x, -y);
            auto r = getLevel().m_cameraController->getHeadRotation();
            r.X = 0_deg;
            r.Y *= 0.5f;
            getLevel().m_cameraController->setTorsoRotation(r);
        }

        /*
                if( getCurrentFrameChangeType() == FrameChangeType::SameFrame )
                {
                    updateFloorHeight(100);
        #ifndef NDEBUG
                    lastUsedCollisionInfo = collisionInfo;
        #endif
                    return;
                }
                */

        testInteractions();

        if( frameChangeType.is_initialized() )
        {
            nextHandler = m_currentStateHandler->postprocessFrame(collisionInfo);
            if( nextHandler.is_initialized() && *nextHandler != m_currentStateHandler->getId() )
            {
                m_currentStateHandler = lara::AbstractStateHandler::create(*nextHandler, *this);
                BOOST_LOG_TRIVIAL(debug) << "New post-processing state override: "
                                        << loader::toString(m_currentStateHandler->getId());
            }
        }

        updateFloorHeight(100);
        handleCommandSequence(collisionInfo.mid.floor.lastCommandSequenceOrDeath, false);
#ifndef NDEBUG
        lastUsedCollisionInfo = collisionInfo;
#endif
#endif
    }


    void LaraNode::placeOnFloor(const CollisionInfo& collisionInfo)
    {
        moveY(collisionInfo.mid.floor.distance);
    }


    loader::LaraStateId LaraNode::getCurrentAnimState() const
    {
        return static_cast<loader::LaraStateId>(items::ItemNode::getCurrentState());
    }


    LaraNode::~LaraNode() = default;


    void LaraNode::update()
    {
        if(m_underwaterState == UnderwaterState::OnLand && getCurrentRoom()->isWaterRoom())
        {
            m_air = core::LaraAir;
            m_underwaterState = UnderwaterState::Diving;
            setFalling(false);
            setPosition(getPosition() + core::TRCoordinates(0, 100, 0));
            updateFloorHeight(0);
            getLevel().stopSoundEffect(30);
            if(getCurrentAnimState() == LaraStateId::SwandiveBegin)
            {
                setXRotation(-45_deg);
                setTargetState(LaraStateId::UnderwaterDiving);
                updateImpl();
                setFallSpeed(getFallSpeed() * 2);
            }
            else if(getCurrentAnimState() == LaraStateId::SwandiveEnd)
            {
                setXRotation(-85_deg);
                setTargetState(LaraStateId::UnderwaterDiving);
                updateImpl();
                setFallSpeed(getFallSpeed() * 2);
            }
            else
            {
                setXRotation(-45_deg);
                setAnimIdGlobal(loader::AnimationId::FREE_FALL_TO_UNDERWATER, 1895);
                setTargetState(LaraStateId::UnderwaterForward);
                //m_currentStateHandler = lara::AbstractStateHandler::create(LaraStateId::UnderwaterDiving, *this);
                setFallSpeed(getFallSpeed() * 3 / 2);
            }

            getLevel().m_cameraController->resetHeadTorsoRotation();

            //! @todo Show water splash effect
        }
        else if(m_underwaterState == UnderwaterState::Diving && !getCurrentRoom()->isWaterRoom())
        {
            auto waterSurfaceHeight = getWaterSurfaceHeight();
            setFallSpeed(0);
            setXRotation(0_deg);
            setZRotation(0_deg);
            getLevel().m_cameraController->resetHeadTorsoRotation();
            m_handStatus = 0;

            if(!waterSurfaceHeight || std::abs(*waterSurfaceHeight - getPosition().Y) >= loader::QuarterSectorSize)
            {
                m_underwaterState = UnderwaterState::OnLand;
                setAnimIdGlobal(loader::AnimationId::FREE_FALL_FORWARD, 492);
                setTargetState(LaraStateId::JumpForward);
                //m_currentStateHandler = lara::AbstractStateHandler::create(LaraStateId::JumpForward, *this);
                //! @todo Check formula
                setHorizontalSpeed(getHorizontalSpeed() / 4);
                setFalling(true);
            }
            else
            {
                m_underwaterState = UnderwaterState::Swimming;
                setAnimIdGlobal(loader::AnimationId::UNDERWATER_TO_ONWATER, 1937);
                setTargetState(LaraStateId::OnWaterStop);
                //m_currentStateHandler = lara::AbstractStateHandler::create(LaraStateId::OnWaterStop, *this);
                {
                    auto pos = getPosition();
                    pos.Y = *waterSurfaceHeight + 1;
                    setPosition(pos);
                }
                m_swimToDiveKeypressDuration = 11;
                updateFloorHeight(-381);
                playSoundEffect(36);
            }
        }
        else if(m_underwaterState == UnderwaterState::Swimming && !getCurrentRoom()->isWaterRoom())
        {
            m_underwaterState = UnderwaterState::OnLand;
            setAnimIdGlobal(loader::AnimationId::FREE_FALL_FORWARD, 492);
            setTargetState(LaraStateId::JumpForward);
            //m_currentStateHandler = lara::AbstractStateHandler::create(LaraStateId::JumpForward, *this);
            setFallSpeed(0);
            //! @todo Check formula
            setHorizontalSpeed(getHorizontalSpeed() * 0.2f);
            setFalling(true);
            m_handStatus = 0;
            setXRotation(0_deg);
            setZRotation(0_deg);
            getLevel().m_cameraController->resetHeadTorsoRotation();
        }

        if(m_underwaterState == UnderwaterState::OnLand)
        {
            m_air = core::LaraAir;
            handleLaraStateOnLand();
        }
        else if(m_underwaterState == UnderwaterState::Diving)
        {
            if(m_health >= 0)
            {
                --m_air;
                if(m_air < 0)
                {
                    m_air = -1;
                    m_health -= 5;
                }
            }
            handleLaraStateDiving();
        }
        else if(m_underwaterState == UnderwaterState::Swimming)
        {
            if(m_health >= 0)
            {
                m_air = std::min(m_air + 10, core::LaraAir);
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
            getLevel().m_textureAnimator->updateCoordinates(getLevel().m_textureProxies);
            m_uvAnimTime -= UVAnimTime;
        }
        // <<<<<<<<<<<<<<<<<

        const auto endOfAnim = SkeletalModelNode::advanceFrame();

        if(endOfAnim)
        {
            const loader::Animation& animation = getLevel().m_animations[getAnimId()];
            if(animation.animCommandCount > 0)
            {
                BOOST_ASSERT(animation.animCommandIndex < getLevel().m_animCommands.size());
                const auto* cmd = &getLevel().m_animCommands[animation.animCommandIndex];
                for(uint16_t i = 0; i < animation.animCommandCount; ++i)
                {
                    BOOST_ASSERT(cmd < &getLevel().m_animCommands.back());
                    const auto opcode = static_cast<AnimCommandOpcode>(*cmd);
                    ++cmd;
                    switch(opcode)
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
                            BOOST_LOG_TRIVIAL(debug) << getId() << " -- end of animation velocity: override " << m_fallSpeedOverride
                                << ", anim fall speed " << cmd[0] << ", anim horizontal speed " << cmd[1];
                            if(m_fallSpeedOverride != 0)
                            {
                                setFallSpeed(m_fallSpeedOverride);
                                m_fallSpeedOverride = 0;
                            }
                            else
                            {
                                setFallSpeed(cmd[0]);
                            }
                            setHorizontalSpeed(cmd[1]);
                            setFalling(true);
                            cmd += 2;
                            break;
                        case AnimCommandOpcode::EmptyHands:
                            setHandStatus(0);
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

            const loader::Animation& currentAnim = getCurrentAnimData();
            ItemNode::setAnimIdGlobal(currentAnim.nextAnimation, currentAnim.nextFrame);
        }

        const loader::Animation& animation = getLevel().m_animations[getAnimId()];
        if(animation.animCommandCount > 0)
        {
            BOOST_ASSERT(animation.animCommandIndex < getLevel().m_animCommands.size());
            const auto* cmd = &getLevel().m_animCommands[animation.animCommandIndex];
            for(uint16_t i = 0; i < animation.animCommandCount; ++i)
            {
                BOOST_ASSERT(cmd < &getLevel().m_animCommands.back());
                const auto opcode = static_cast<AnimCommandOpcode>(*cmd);
                ++cmd;
                switch(opcode)
                {
                    case AnimCommandOpcode::SetPosition:
                        cmd += 3;
                        break;
                    case AnimCommandOpcode::StartFalling:
                        cmd += 2;
                        break;
                    case AnimCommandOpcode::PlaySound:
                        if(getCurrentFrame() == cmd[0])
                        {
                            playSoundEffect(cmd[1]);
                        }
                        cmd += 2;
                        break;
                    case AnimCommandOpcode::PlayEffect:
                        if(getCurrentFrame() == cmd[0])
                        {
                            BOOST_LOG_TRIVIAL(debug) << "Anim effect: " << int(cmd[1]);
                            if(cmd[1] == 0)
                                addYRotation(180_deg);
                            else if(cmd[1] == 12)
                                setHandStatus(0);
                            //! @todo Execute anim effect cmd[1]
                        }
                        cmd += 2;
                        break;
                    default:
                        break;
                }
            }
        }

        applyMovement();

        //! @todo Check if there is a better place for this.
        resetPose();
        patchBone(7, getLevel().m_cameraController->getTorsoRotation().toMatrix());
        patchBone(14, getLevel().m_cameraController->getHeadRotation().toMatrix());
    }


    void LaraNode::updateFloorHeight(int dy)
    {
        auto pos = getPosition();
        pos.Y += dy;
        gsl::not_null<const loader::Room*> room = getCurrentRoom();
        auto sector = getLevel().findRealFloorSector(pos, &room);
        setCurrentRoom(room);
        HeightInfo hi = HeightInfo::fromFloor(sector, pos, getLevel().m_cameraController);
        setFloorHeight(hi.distance);
    }


    void LaraNode::handleCommandSequence(const uint16_t* floorData, bool isNotLara)
    {
        if( floorData == nullptr )
            return;

        floordata::FloorDataChunk chunkHeader{*floorData};

        if( chunkHeader.type == floordata::FloorDataChunkType::Death )
        {
            if( !isNotLara )
            {
                if( getPosition().Y == getFloorHeight() )
                {
                    //! @todo kill Lara
                }
            }

            if( chunkHeader.isLast )
                return;

            ++floorData;
        }

        chunkHeader = floordata::FloorDataChunk{*floorData++};
        BOOST_ASSERT(chunkHeader.type == floordata::FloorDataChunkType::CommandSequence);
        const floordata::ActivationState activationRequest{*floorData++};

        getLevel().m_cameraController->findCameraTarget(floorData);

        bool conditionFulfilled = false, switchIsOn = false;
        if( !isNotLara )
        {
            switch( chunkHeader.sequenceCondition )
            {
                case floordata::SequenceCondition::LaraIsHere:
                    conditionFulfilled = true;
                    break;
                case floordata::SequenceCondition::LaraOnGround:
                case floordata::SequenceCondition::LaraOnGroundInverted:
                    conditionFulfilled = getPosition().Y == getFloorHeight();
                    break;
                case floordata::SequenceCondition::ItemActivated:
                {
                    const floordata::Command command{*floorData++};
                    Expects( getLevel().m_itemNodes.find(command.parameter) != getLevel().m_itemNodes.end() );
                    items::ItemNode& swtch = *getLevel().m_itemNodes[command.parameter];
                    if( !swtch.triggerSwitch(activationRequest) )
                        return;

                    switchIsOn = (swtch.getCurrentState() == 1);
                    conditionFulfilled = true;
                }
                    break;
                case floordata::SequenceCondition::KeyUsed:
                {
                    const floordata::Command command{*floorData++};
                    Expects( getLevel().m_itemNodes.find(command.parameter) != getLevel().m_itemNodes.end() );
                    items::ItemNode& key = *getLevel().m_itemNodes[command.parameter];
                    if( key.triggerKey() )
                        conditionFulfilled = true;
                }
                    return;
                case floordata::SequenceCondition::ItemPickedUp:
                {
                    const floordata::Command command{*floorData++};
                    Expects( getLevel().m_itemNodes.find(command.parameter) != getLevel().m_itemNodes.end() );
                    items::ItemNode& pickup = *getLevel().m_itemNodes[command.parameter];
                    if( pickup.triggerPickUp() )
                        conditionFulfilled = true;
                }
                    return;
                case floordata::SequenceCondition::LaraInCombatMode:
                    conditionFulfilled = getHandStatus() == 4;
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
                    Expects( getLevel().m_itemNodes.find(command.parameter) != getLevel().m_itemNodes.end() );
                    items::ItemNode& item = *getLevel().m_itemNodes[command.parameter];
                    if( item.m_activationState.isOneshot() )
                        break;

                    item.m_activationState.setTimeout(activationRequest.getTimeout());

                    //BOOST_LOG_TRIVIAL(trace) << "Setting trigger timeout of " << item.getName() << " to " << item.m_triggerTimeout << "ms";

                    if( chunkHeader.sequenceCondition == floordata::SequenceCondition::ItemActivated )
                        item.m_activationState ^= activationRequest.getActivationSet();
                    else if( chunkHeader.sequenceCondition == floordata::SequenceCondition::LaraOnGroundInverted )
                        item.m_activationState &= ~activationRequest.getActivationSet();
                    else
                        item.m_activationState |= activationRequest.getActivationSet();

                    if( !item.m_activationState.isFullyActivated() )
                        break;

                    if( activationRequest.isOneshot() )
                        item.m_activationState.setOneshot(true);

                    if( item.m_isActive )
                        break;

                    if( (item.m_characteristics & ItemNode::Intelligent) == 0 )
                    {
                        item.m_triggerState = engine::items::TriggerState::Enabled;
                        item.activate();
                        break;
                    }

                    if( item.m_triggerState == engine::items::TriggerState::Disabled )
                    {
                        //! @todo Implement baddie
                        item.m_triggerState = engine::items::TriggerState::Enabled;
                        item.activate();
                        break;
                    }

                    if( item.m_triggerState != engine::items::TriggerState::Locked )
                        break;

                    item.m_triggerState = engine::items::TriggerState::Enabled;
                    item.activate();
                }
                    break;
                case floordata::CommandOpcode::SwitchCamera:
                {
                    const floordata::CameraParameters camParams{*floorData++};
                    getLevel().m_cameraController->setCamOverride(camParams, command.parameter, chunkHeader.sequenceCondition,
                                                                  isNotLara, activationRequest, switchIsOn);
                    command.isLast = camParams.isLast;
                }
                    break;
                case floordata::CommandOpcode::LookAt:
                    getLevel().m_cameraController->setLookAtItem(getLevel().getItemController(command.parameter));
                    break;
                case floordata::CommandOpcode::UnderwaterCurrent:
                    //! @todo handle underwater current
                    break;
                case floordata::CommandOpcode::FlipMap:
                    BOOST_ASSERT(command.parameter < mapFlipActivationStates.size());
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
                                mapFlipActivationStates[command.parameter].setOneshot(true);

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
                    BOOST_ASSERT(command.parameter < mapFlipActivationStates.size());
                    if( !getLevel().roomsAreSwapped && mapFlipActivationStates[command.parameter].isFullyActivated() )
                        swapRooms = true;
                    break;
                case floordata::CommandOpcode::FlipOff:
                    BOOST_ASSERT(command.parameter < mapFlipActivationStates.size());
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
                    getLevel().triggerCdTrack(command.parameter, activationRequest, chunkHeader.sequenceCondition);
                    break;
                case floordata::CommandOpcode::Secret:
                {
                    BOOST_ASSERT(command.parameter < 16 );
                    const uint16_t mask = 1u << command.parameter;
                    if( (m_secretsFoundBitmask & mask) == 0 )
                    {
                        m_secretsFoundBitmask |= mask;
                        getLevel().playCdTrack(13);
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
            swapAllRooms(getLevel());
    }


    boost::optional<int> LaraNode::getWaterSurfaceHeight() const
    {
        gsl::not_null<const loader::Sector*> sector = getCurrentRoom()
            ->getSectorByAbsolutePosition(getPosition());

        if( getCurrentRoom()->isWaterRoom() )
        {
            while( true )
            {
                if( sector->roomAbove == 0xff )
                    break;

                BOOST_ASSERT( sector->roomAbove < getLevel().m_rooms.size() );
                const auto& room = getLevel().m_rooms[sector->roomAbove];
                if( !room.isWaterRoom() )
                    break;

                sector = room.getSectorByAbsolutePosition(getPosition());
            }

            return sector->ceilingHeight * loader::QuarterSectorSize;
        }

        while( true )
        {
            if( sector->roomBelow == 0xff )
                break;

            BOOST_ASSERT( sector->roomBelow < getLevel().m_rooms.size() );
            const auto& room = getLevel().m_rooms[sector->roomBelow];
            if( room.isWaterRoom() )
            {
                return sector->floorHeight * loader::QuarterSectorSize;
            }

            sector = room.getSectorByAbsolutePosition(getPosition());
        }

        return sector->ceilingHeight * loader::QuarterSectorSize;
    }


    void LaraNode::setCameraRotation(core::Angle x, core::Angle y)
    {
        getLevel().m_cameraController->setLocalRotation(x, y);
    }


    void LaraNode::setCameraRotationY(core::Angle y)
    {
        getLevel().m_cameraController->setLocalRotationY(y);
    }


    void LaraNode::setCameraRotationX(core::Angle x)
    {
        getLevel().m_cameraController->setLocalRotationX(x);
    }


    void LaraNode::setCameraDistance(int d)
    {
        getLevel().m_cameraController->setLocalDistance(d);
    }


    void LaraNode::setCameraUnknown1(CamOverrideType k)
    {
        getLevel().m_cameraController->setUnknown1(k);
    }


    void LaraNode::testInteractions()
    {
        m_flags2_10_isHit = false;

        if( m_health < 0 )
            return;

        std::set<const loader::Room*> rooms;
        rooms.insert(getCurrentRoom());
        for( const loader::Portal& p : getCurrentRoom()->portals )
            rooms.insert(&getLevel().m_rooms[p.adjoining_room]);

        for( const std::shared_ptr<engine::items::ItemNode>& item : getLevel().m_itemNodes | boost::adaptors::map_values )
        {
            if( rooms.find(item->getCurrentRoom()) == rooms.end() )
                continue;

            if( !item->m_flags2_20_collidable )
                continue;

            if( item->m_triggerState == items::TriggerState::Locked )
                continue;

            const auto d = getPosition() - item->getPosition();
            if( std::abs(d.X) >= 4096 || std::abs(d.Y) >= 4096 || std::abs(d.Z) >= 4096 )
                continue;

            item->onInteract(*this);
        }
    }
}
