#include "laracontroller.h"

#include "animationcontroller.h"
#include "cameracontroller.h"
#include "heightinfo.h"
#include "larastate.h"
#include "abstractstatehandler.h"
#include "textureanimator.h"

void LaraController::setTargetState(LaraStateId st)
{
    m_dispatcher->setTargetState(static_cast<uint16_t>(st));
}

loader::LaraStateId LaraController::getTargetState() const
{
    return static_cast<LaraStateId>(m_dispatcher->getTargetState());
}

void LaraController::playAnimation(loader::AnimationId anim, const boost::optional<irr::u32>& firstFrame)
{
    m_dispatcher->playLocalAnimation(static_cast<uint16_t>(anim), firstFrame);
}

void LaraController::applyRotation()
{
    //! @todo This is horribly inefficient code, but it properly converts ZXY angles to XYZ angles.
    irr::core::quaternion q;
    q.makeIdentity();
    q *= irr::core::quaternion().fromAngleAxis(getRotation().Y.toRad(), {0,1,0});
    q *= irr::core::quaternion().fromAngleAxis(getRotation().X.toRad(), {-1,0,0});
    q *= irr::core::quaternion().fromAngleAxis(getRotation().Z.toRad(), {0,0,-1});

    irr::core::vector3df euler;
    q.toEuler(euler);
    m_sceneNode->setRotation(euler * 180 / irr::core::PI);
}

void LaraController::handleLaraStateOnLand(bool newFrame)
{
    LaraState laraState;
    laraState.position = getPosition();
    laraState.collisionRadius = 100; //!< @todo MAGICK 100
    laraState.frobbelFlags = LaraState::FrobbelFlag10 | LaraState::FrobbelFlag08;

    std::unique_ptr<AbstractStateHandler> nextHandler = nullptr;
    if( newFrame )
    {
        //BOOST_LOG_TRIVIAL(debug) << "Input state: " << loader::toString(m_currentStateHandler->getId());
        nextHandler = m_currentStateHandler->handleInput(laraState);
    }

    m_currentStateHandler->animate(laraState, getCurrentDeltaTime());

    if( nextHandler != nullptr )
    {
        m_currentStateHandler = std::move(nextHandler);
        BOOST_LOG_TRIVIAL(debug) << "New input state override: " << loader::toString(m_currentStateHandler->getId());
    }

    // "slowly" revert rotations to zero
    if( getRotation().Z < 0_deg )
    {
        m_rotation.Z += core::makeInterpolatedValue(+1_deg).getScaled(getCurrentDeltaTime());
        if( getRotation().Z >= 0_deg )
            m_rotation.Z = 0_deg;
    }
    else if( getRotation().Z > 0_deg )
    {
        m_rotation.Z -= core::makeInterpolatedValue(+1_deg).getScaled(getCurrentDeltaTime());
        if( getRotation().Z <= 0_deg )
            m_rotation.Z = 0_deg;
    }

    if( getYRotationSpeed() < 0_deg )
    {
        m_yRotationSpeed.add(2_deg, getCurrentDeltaTime()).limitMax(0_deg);
    }
    else if( getYRotationSpeed() > 0_deg )
    {
        m_yRotationSpeed.sub(2_deg, getCurrentDeltaTime()).limitMin(0_deg);
    }
    else
    {
        setYRotationSpeed(0_deg);
    }

    m_rotation.Y += m_yRotationSpeed.getScaled(getCurrentDeltaTime());

    applyRotation();

    //BOOST_LOG_TRIVIAL(debug) << "Post-processing state: " << loader::toString(m_currentStateHandler->getId());

    auto animCommandOverride = processAnimCommands();
    if( animCommandOverride )
    {
        m_currentStateHandler = std::move(animCommandOverride);
        BOOST_LOG_TRIVIAL(debug) << "New anim command state override: " << loader::toString(m_currentStateHandler->getId());
    }

    if(!newFrame)
        return;

    // @todo test interactions

    nextHandler = m_currentStateHandler->postprocessFrame(laraState);
    if( nextHandler != nullptr )
    {
        m_currentStateHandler = std::move(nextHandler);
        BOOST_LOG_TRIVIAL(debug) << "New post-processing state override: " << loader::toString(m_currentStateHandler->getId());
    }

    updateFloorHeight(-381);
    handleTriggers(laraState.current.floor.lastTriggerOrKill, false);
}

void LaraController::handleLaraStateDiving(bool newFrame)
{
    LaraState laraState;
    laraState.position = getPosition();
    laraState.collisionRadius = 300; //!< @todo MAGICK 300
    laraState.frobbelFlags &= ~(LaraState::FrobbelFlag10 | LaraState::FrobbelFlag08 | LaraState::FrobbelFlag_UnwalkableDeadlyFloor | LaraState::FrobbelFlag_UnwalkableSteepFloor | LaraState::FrobbelFlag_UnpassableSteepUpslant);
    laraState.neededCeilingDistance = 400;
    laraState.neededFloorDistanceBottom = loader::HeightLimit;
    laraState.neededFloorDistanceTop = -400;

    std::unique_ptr<AbstractStateHandler> nextHandler = nullptr;
    if(newFrame)
    {
        //BOOST_LOG_TRIVIAL(debug) << "Input state: " << loader::toString(m_currentStateHandler->getId());
        nextHandler = m_currentStateHandler->handleInput(laraState);
    }

    m_currentStateHandler->animate(laraState, getCurrentDeltaTime());

    if(nextHandler != nullptr)
    {
        m_currentStateHandler = std::move(nextHandler);
        BOOST_LOG_TRIVIAL(debug) << "New input state override: " << loader::toString(m_currentStateHandler->getId());
    }

    // "slowly" revert rotations to zero
    if(m_rotation.Z < 0_deg)
    {
        m_rotation.Z += core::makeInterpolatedValue(+2_deg).getScaled(getCurrentDeltaTime());
        if(m_rotation.Z >= 0_deg)
            m_rotation.Z = 0_deg;
    }
    else if(m_rotation.Z > 0_deg)
    {
        m_rotation.Z -= core::makeInterpolatedValue(+2_deg).getScaled(getCurrentDeltaTime());
        if(m_rotation.Z <= 0_deg)
            m_rotation.Z = 0_deg;
    }
    m_rotation.X = irr::core::clamp(m_rotation.X, -100_deg, +100_deg);
    m_rotation.Z = irr::core::clamp(m_rotation.Z, -22_deg, +22_deg);

    m_position.X += m_rotation.Y.sin() * m_rotation.X.cos() * m_fallSpeed.getScaled(getCurrentDeltaTime()) / 4;
    m_position.Y -=                      m_rotation.X.sin() * m_fallSpeed.getScaled(getCurrentDeltaTime()) / 4;
    m_position.Z += m_rotation.Y.cos() * m_rotation.X.cos() * m_fallSpeed.getScaled(getCurrentDeltaTime()) / 4;

    applyRotation();
    m_sceneNode->setPosition(m_position.toIrrlicht());
    m_sceneNode->updateAbsolutePosition();

    auto animCommandOverride = processAnimCommands();
    if(animCommandOverride)
    {
        m_currentStateHandler = std::move(animCommandOverride);
        BOOST_LOG_TRIVIAL(debug) << "New anim command state override: " << loader::toString(m_currentStateHandler->getId());
    }

    if(!newFrame)
        return;

    // @todo test interactions

    nextHandler = m_currentStateHandler->postprocessFrame(laraState);
    if(nextHandler != nullptr)
    {
        m_currentStateHandler = std::move(nextHandler);
        BOOST_LOG_TRIVIAL(debug) << "New post-processing state override: " << loader::toString(m_currentStateHandler->getId());
    }

    updateFloorHeight(0);
    handleTriggers(laraState.current.floor.lastTriggerOrKill, false);
}

void LaraController::handleLaraStateSwimming(bool newFrame)
{
    LaraState laraState;
    laraState.position = getPosition();
    laraState.collisionRadius = 100; //!< @todo MAGICK 100
    laraState.frobbelFlags &= ~(LaraState::FrobbelFlag10 | LaraState::FrobbelFlag08 | LaraState::FrobbelFlag_UnwalkableDeadlyFloor | LaraState::FrobbelFlag_UnwalkableSteepFloor | LaraState::FrobbelFlag_UnpassableSteepUpslant);
    laraState.neededCeilingDistance = 100;
    laraState.neededFloorDistanceBottom = loader::HeightLimit;
    laraState.neededFloorDistanceTop = -100;

    setCameraRotationX(-22_deg);

    std::unique_ptr<AbstractStateHandler> nextHandler = nullptr;
    if(newFrame)
    {
        //BOOST_LOG_TRIVIAL(debug) << "Input state: " << loader::toString(m_currentStateHandler->getId());
        nextHandler = m_currentStateHandler->handleInput(laraState);
    }

    m_currentStateHandler->animate(laraState, getCurrentDeltaTime());

    if(nextHandler != nullptr)
    {
        m_currentStateHandler = std::move(nextHandler);
        BOOST_LOG_TRIVIAL(debug) << "New input state override: " << loader::toString(m_currentStateHandler->getId());
    }

    // "slowly" revert rotations to zero
    if(m_rotation.Z < 0_deg)
    {
        m_rotation.Z += core::makeInterpolatedValue(+2_deg).getScaled(getCurrentDeltaTime());
        if(m_rotation.Z >= 0_deg)
            m_rotation.Z = 0_deg;
    }
    else if(m_rotation.Z > 0_deg)
    {
        m_rotation.Z -= core::makeInterpolatedValue(+2_deg).getScaled(getCurrentDeltaTime());
        if(m_rotation.Z <= 0_deg)
            m_rotation.Z = 0_deg;
    }

    m_position.X += getMovementAngle().sin() * m_fallSpeed.getScaled(getCurrentDeltaTime()) / 4;
    m_position.Z += getMovementAngle().cos() * m_fallSpeed.getScaled(getCurrentDeltaTime()) / 4;

    applyRotation();
    m_sceneNode->setPosition(m_position.toIrrlicht());
    m_sceneNode->updateAbsolutePosition();

    auto animCommandOverride = processAnimCommands();
    if(animCommandOverride)
    {
        m_currentStateHandler = std::move(animCommandOverride);
        BOOST_LOG_TRIVIAL(debug) << "New anim command state override: " << loader::toString(m_currentStateHandler->getId());
    }

    if(!newFrame)
        return;

    // @todo test interactions

    nextHandler = m_currentStateHandler->postprocessFrame(laraState);
    if(nextHandler != nullptr)
    {
        m_currentStateHandler = std::move(nextHandler);
        BOOST_LOG_TRIVIAL(debug) << "New post-processing state override: " << loader::toString(m_currentStateHandler->getId());
    }

    updateFloorHeight(100);
    handleTriggers(laraState.current.floor.lastTriggerOrKill, false);
}

irr::u32 LaraController::getCurrentFrame() const
{
    return m_dispatcher->getCurrentFrame();
}

irr::u32 LaraController::getAnimEndFrame() const
{
    return m_dispatcher->getAnimEndFrame();
}

void LaraController::placeOnFloor(const LaraState& state)
{
    m_position.Y += state.current.floor.distance;
}

loader::LaraStateId LaraController::getCurrentState() const
{
    return m_currentStateHandler->getId();
}

loader::LaraStateId LaraController::getCurrentAnimState() const
{
    return static_cast<loader::LaraStateId>(m_dispatcher->getCurrentAnimState());
}

LaraController::~LaraController() = default;

void LaraController::animateNode(irr::scene::ISceneNode* node, irr::u32 timeMs)
{
    BOOST_ASSERT(m_sceneNode == node);

    if( m_lastFrameTime < 0 )
        m_lastFrameTime = m_lastEngineFrameTime = m_currentFrameTime = timeMs;

    if( m_lastFrameTime == timeMs )
        return;

    m_currentFrameTime = timeMs;

    static constexpr int FrameTime = 1000 / 30;
    static constexpr int UVAnimTime = 1000 / 3;

    bool isNewFrame = m_lastAnimFrame != getCurrentFrame();

    if( timeMs - m_lastEngineFrameTime >= FrameTime )
    {
        isNewFrame = true;
        m_lastEngineFrameTime -= (timeMs - m_lastEngineFrameTime) / FrameTime * FrameTime;
    }

    m_uvAnimTime += getCurrentDeltaTime();
    if(m_uvAnimTime >= UVAnimTime)
    {
        m_level->m_textureAnimator->updateCoordinates(m_level->m_textureProxies);
        m_uvAnimTime -= UVAnimTime;
    }

    if(m_currentStateHandler == nullptr)
    {
        m_currentStateHandler = AbstractStateHandler::create(getCurrentAnimState(), *this);
    }

    if(m_underwaterState == UnderwaterState::OnLand && m_currentRoom->isWaterRoom())
    {
        m_air = 1800;
        m_underwaterState = UnderwaterState::Diving;
        m_falling = false;
        m_position.Y += 100;
        updateFloorHeight(0);
        //! @todo stop sound 30
        if(getCurrentAnimState() == LaraStateId::SwandiveBegin)
        {
            m_rotation.X = -45_deg;
            setTargetState(LaraStateId::UnderwaterDiving);
            if(auto tmp = processAnimCommands())
                m_currentStateHandler = std::move(tmp);
            m_fallSpeed = m_fallSpeed * 2;
        }
        else if(getCurrentAnimState() == LaraStateId::SwandiveEnd)
        {
            m_rotation.X = -85_deg;
            setTargetState(LaraStateId::UnderwaterDiving);
            if(auto tmp = processAnimCommands())
                m_currentStateHandler = std::move(tmp);
            m_fallSpeed = m_fallSpeed * 2;
        }
        else
        {
            m_rotation.X = -45_deg;
            playAnimation(loader::AnimationId::FREE_FALL_TO_UNDERWATER, 1895);
            setTargetState(LaraStateId::UnderwaterForward);
            m_currentStateHandler = AbstractStateHandler::create(LaraStateId::UnderwaterDiving, *this);
            if(auto tmp = processAnimCommands())
                m_currentStateHandler = std::move(tmp);
            m_fallSpeed = m_fallSpeed * 1.5f;
        }

        //! @todo Show water splash effect
    }
    else if(m_underwaterState == UnderwaterState::Diving && !m_currentRoom->isWaterRoom())
    {
        auto waterSurfaceHeight = getWaterSurfaceHeight();
        if(!waterSurfaceHeight || std::abs(*waterSurfaceHeight - m_position.Y) >= 256)
        {
            m_underwaterState = UnderwaterState::OnLand;
            playAnimation(loader::AnimationId::FREE_FALL_FORWARD, 492);
            setTargetState(LaraStateId::JumpForward);
            m_currentStateHandler = AbstractStateHandler::create(LaraStateId::JumpForward, *this);
            m_fallSpeed = 0;
            //! @todo Check formula
            m_horizontalSpeed = m_horizontalSpeed * 0.2f;
            m_falling = true;
            m_handStatus = 0;
            m_rotation.X = m_rotation.Z = 0_deg;
        }
        else
        {
            m_underwaterState = UnderwaterState::Swimming;
            playAnimation(loader::AnimationId::UNDERWATER_TO_ONWATER, 1937);
            setTargetState(LaraStateId::OnWaterStop);
            m_currentStateHandler = AbstractStateHandler::create(LaraStateId::OnWaterStop, *this);
            m_handStatus = 0;
            m_rotation.X = m_rotation.Z = 0_deg;
            m_position.Y = *waterSurfaceHeight + 1;
            m_swimToDiveKeypressDuration = 11 * 1000 / 30;
            updateFloorHeight(-381);
            //! @todo play sound 36
        }
    }
    else if(m_underwaterState == UnderwaterState::Swimming && !m_currentRoom->isWaterRoom())
    {
        m_underwaterState = UnderwaterState::OnLand;
        playAnimation(loader::AnimationId::FREE_FALL_FORWARD, 492);
        setTargetState(LaraStateId::JumpForward);
        m_currentStateHandler = AbstractStateHandler::create(LaraStateId::JumpForward, *this);
        m_fallSpeed = 0;
        //! @todo Check formula
        m_horizontalSpeed = m_horizontalSpeed * 0.2f;
        m_falling = true;
        m_handStatus = 0;
        m_rotation.X = m_rotation.Z = 0_deg;
    }

    if(m_underwaterState == UnderwaterState::OnLand)
    {
        m_air = 1800;
        handleLaraStateOnLand(isNewFrame);
    }
    else if(m_underwaterState == UnderwaterState::Diving)
    {
        if(m_health >= 0)
        {
            m_air.sub(1, getCurrentDeltaTime());
            if(m_air < 0)
            {
                m_air = -1;
                m_health.sub(5, getCurrentDeltaTime());
            }
        }
        handleLaraStateDiving(isNewFrame);
    }
    else if(m_underwaterState == UnderwaterState::Swimming)
    {
        if(m_health >= 0)
        {
            m_air.add(10, getCurrentDeltaTime()).limitMax(1800);
        }
        handleLaraStateSwimming(isNewFrame);
    }

    m_lastFrameTime = m_currentFrameTime;
}

std::unique_ptr<AbstractStateHandler> LaraController::processAnimCommands()
{
    std::unique_ptr<AbstractStateHandler> nextHandler = nullptr;
    bool newFrame = false;
    if( m_dispatcher->handleTRTransitions() || m_lastAnimFrame != getCurrentFrame() )
    {
        nextHandler = m_currentStateHandler->createWithRetainedAnimation(getCurrentAnimState());
        m_lastAnimFrame = getCurrentFrame();
        newFrame = true;
    }

    const bool isAnimEnd = getCurrentFrame() >= getAnimEndFrame();

    const loader::Animation& animation = getLevel().m_animations[m_dispatcher->getCurrentAnimationId()];
    if( animation.animCommandCount > 0 )
    {
        BOOST_ASSERT(animation.animCommandIndex < getLevel().m_animCommands.size());
        const auto* cmd = &getLevel().m_animCommands[animation.animCommandIndex];
        for( uint16_t i = 0; i < animation.animCommandCount; ++i )
        {
            BOOST_ASSERT(cmd < &getLevel().m_animCommands.back());
            const auto opcode = static_cast<AnimCommandOpcode>(*cmd);
            ++cmd;
            switch( opcode )
            {
            case AnimCommandOpcode::SetPosition:
                if( isAnimEnd && newFrame )
                {
                    moveLocal(
                              cmd[0],
                              cmd[1],
                              cmd[2]
                             );
                }
                cmd += 3;
                break;
            case AnimCommandOpcode::SetVelocity:
                if( isAnimEnd && newFrame )
                {
                    setFallSpeed(core::makeInterpolatedValue<float>(m_fallSpeedOverride == 0 ? cmd[0] : m_fallSpeedOverride));
                    m_fallSpeedOverride = 0;
                    setHorizontalSpeed(core::makeInterpolatedValue<float>(cmd[1]));
                    setFalling(true);
                }
                cmd += 2;
                break;
            case AnimCommandOpcode::EmptyHands:
                if( isAnimEnd )
                {
                    setHandStatus(0);
                }
                break;
            case AnimCommandOpcode::PlaySound:
                if( getCurrentFrame() == cmd[0] )
                {
                    //! @todo playsound(cmd[1])
                }
                cmd += 2;
                break;
            case AnimCommandOpcode::PlayEffect:
                if( getCurrentFrame() == cmd[0] )
                {
                    BOOST_LOG_TRIVIAL(debug) << "Anim effect: " << int(cmd[1]);
                    if( cmd[1] == 0 && newFrame )
                        m_rotation.Y += 180_deg;
                    else if( cmd[1] == 12 )
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

    if( m_falling )
    {
        m_horizontalSpeed.add(m_dispatcher->getAccelleration(), getCurrentDeltaTime());
        if( getFallSpeed() >= 128 )
            m_fallSpeed.add(1, getCurrentDeltaTime());
        else
            m_fallSpeed.add(6, getCurrentDeltaTime());

        move(
            getMovementAngle().sin() * m_horizontalSpeed.getScaled(getCurrentDeltaTime()),
            m_fallSpeed.getScaled(getCurrentDeltaTime()),
            getMovementAngle().cos() * m_horizontalSpeed.getScaled(getCurrentDeltaTime())
        );
    }
    else
    {
        m_horizontalSpeed = m_dispatcher->calculateFloorSpeed();

        move(
            getMovementAngle().sin() * m_horizontalSpeed.getScaled(getCurrentDeltaTime()),
            0,
            getMovementAngle().cos() * m_horizontalSpeed.getScaled(getCurrentDeltaTime())
        );
    }

    m_sceneNode->setPosition(m_position.toIrrlicht());
    m_sceneNode->updateAbsolutePosition();

    return nextHandler;
}

void LaraController::updateFloorHeight(int dy)
{
    auto pos = getPosition();
    pos.Y += dy;
    auto room = m_currentRoom;
    auto sector = getLevel().findSectorForPosition(pos.toInexact(), &room);
    setCurrentRoom(room);
    HeightInfo hi = HeightInfo::fromFloor(sector, pos.toInexact(), getLevel().m_cameraController);
    setFloorHeight(hi.distance);
}

void LaraController::handleTriggers(const uint16_t* floorData, bool isDoppelganger)
{
    if( floorData == nullptr )
        return;

    if( loader::extractFDFunction(*floorData) == loader::FDFunction::Death )
    {
        if( !isDoppelganger )
        {
            if( irr::core::equals(gsl::narrow<int>(getPosition().Y), getFloorHeight(), 1) )
            {
                //! @todo kill Lara
            }
        }

        if( loader::isLastFloordataEntry(*floorData) )
            return;

        ++floorData;
    }

    const auto triggerType = loader::extractTriggerType(*floorData);
    const auto triggerArg = floorData[1];
    auto nextFloorData = floorData + 2;

    //! @todo Find camera target if necessary

    bool doTrigger = false;
    if( !isDoppelganger )
    {
        switch( triggerType )
        {
        case loader::TriggerType::Trigger:
            break;
        case loader::TriggerType::Pad:
        case loader::TriggerType::AntiPad:
            doTrigger = getPosition().Y == getFloorHeight();
            break;
        case loader::TriggerType::Switch:
            //! @todo Handle switch
            ++nextFloorData;
            doTrigger = true;
            return;
        case loader::TriggerType::Key:
            //! @todo Handle key
            ++nextFloorData;
            doTrigger = true;
            return;
        case loader::TriggerType::Pickup:
            //! @todo Handle pickup
            ++nextFloorData;
            doTrigger = true;
            return;
        case loader::TriggerType::Combat:
            doTrigger = getHandStatus() == 4;
            break;
        case loader::TriggerType::Heavy:
        case loader::TriggerType::Dummy:
            return;
        default:
            doTrigger = true;
            break;
        }
    }
    else
    {
        doTrigger = triggerType == loader::TriggerType::Heavy;
    }

    if(!doTrigger)
        return;

    while(true)
    {
        const bool isLast = loader::isLastFloordataEntry(*nextFloorData);
        switch(loader::extractTriggerFunction(*nextFloorData))
        {
        case loader::TriggerFunction::Object:
            ++nextFloorData;
            //! @todo handle object
            break;
        case loader::TriggerFunction::CameraTarget:
            ++nextFloorData;
            ++nextFloorData;
            //! @todo handle camera target
            break;
        case loader::TriggerFunction::LookAt:
            ++nextFloorData;
            //! @todo handle "look at"
            break;
        case loader::TriggerFunction::UnderwaterCurrent:
            ++nextFloorData;
            //! @todo handle underwater current
            break;
        case loader::TriggerFunction::FlipMap:
            ++nextFloorData;
            //! @todo handle flip map
            break;
        case loader::TriggerFunction::FlipOn:
            ++nextFloorData;
            //! @todo handle flip on
            break;
        case loader::TriggerFunction::FlipOff:
            ++nextFloorData;
            //! @todo handle flip off
            break;
        case loader::TriggerFunction::FlipEffect:
            ++nextFloorData;
            //! @todo handle flip effect
            break;
        case loader::TriggerFunction::EndLevel:
            ++nextFloorData;
            //! @todo handle level end
            break;
        case loader::TriggerFunction::PlayTrack:
            ++nextFloorData;
            //! @todo handle "play track"
            break;
        case loader::TriggerFunction::Secret:
            ++nextFloorData;
            //! @todo handle secrets
            break;
        default:
            ++nextFloorData;
            break;
        }

        if(isLast)
            break;
    }

    //! @todo Implement the rest
}

irr::core::aabbox3di LaraController::getBoundingBox() const
{
    return m_dispatcher->getBoundingBox();
}

boost::optional<int> LaraController::getWaterSurfaceHeight() const
{
    auto sector = m_currentRoom->getSectorByAbsolutePosition(m_position.toInexact());

    if(m_currentRoom->isWaterRoom())
    {
        while(true)
        {
            if(sector->roomAbove == 0xff)
                break;

            BOOST_ASSERT(sector->roomAbove < m_level->m_rooms.size());
            const auto& room = m_level->m_rooms[sector->roomAbove];
            if(!room.isWaterRoom())
                break;

            sector = room.getSectorByAbsolutePosition(m_position.toInexact());
        }

        return sector->ceilingHeight * loader::QuarterSectorSize;
    }
    else
    {
        while(true)
        {
            if(sector->roomBelow == 0xff)
                break;

            BOOST_ASSERT(sector->roomBelow < m_level->m_rooms.size());
            const auto& room = m_level->m_rooms[sector->roomBelow];
            if(room.isWaterRoom())
            {
                return sector->floorHeight * loader::QuarterSectorSize;
            }

            sector = room.getSectorByAbsolutePosition(m_position.toInexact());
        }

        return sector->ceilingHeight * loader::QuarterSectorSize;
    }

    return boost::none;
}

void LaraController::setCameraRotation(core::Angle x, core::Angle y)
{
    m_level->m_cameraController->setLocalRotation(x, y);
}

void LaraController::setCameraRotationY(core::Angle y)
{
    m_level->m_cameraController->setLocalRotationY(y);
}

void LaraController::setCameraRotationX(core::Angle x)
{
    m_level->m_cameraController->setLocalRotationX(x);
}

void LaraController::setCurrentRoom(const loader::Room* newRoom)
{
    if(newRoom == m_currentRoom)
        return;

    BOOST_LOG_TRIVIAL(debug) << "Room switch to " << newRoom->node->getName();
    if(newRoom == nullptr)
    {
        BOOST_LOG_TRIVIAL(fatal) << "No room to switch to. Matching rooms by position:";
        for(size_t i = 0; i < m_level->m_rooms.size(); ++i)
        {
            const loader::Room& room = m_level->m_rooms[i];
            if(room.node->getTransformedBoundingBox().isPointInside(m_sceneNode->getAbsolutePosition()))
            {
                BOOST_LOG_TRIVIAL(fatal) << "  - " << i;
            }
        }
        return;
    }

    m_currentRoom = newRoom;
    for(irr::u32 i = 0; i < m_sceneNode->getMaterialCount(); ++i)
    {
        irr::video::SMaterial& material = m_sceneNode->getMaterial(i);
        const auto col = m_currentRoom->lightColor.toSColor(m_currentRoom->darkness / 8191.0f / 4);
        material.EmissiveColor = col;
        material.AmbientColor = col;
    }
}
