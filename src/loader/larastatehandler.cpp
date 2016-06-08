#include "larastatehandler.h"

#include "defaultanimdispatcher.h"
#include "trcamerascenenodeanimator.h"
#include "heightinfo.h"
#include "larastate.h"
#include "abstractstatehandler.h"

void LaraStateHandler::setTargetState(LaraStateId st)
{
    m_dispatcher->setTargetState(static_cast<uint16_t>(st));
}

loader::LaraStateId LaraStateHandler::getTargetState() const
{
    return static_cast<LaraStateId>(m_dispatcher->getTargetState());
}

void LaraStateHandler::playAnimation(loader::AnimationId anim, const boost::optional<irr::u32>& firstFrame)
{
    m_dispatcher->playLocalAnimation(static_cast<uint16_t>(anim), firstFrame);
}

void LaraStateHandler::handleLaraStateOnLand(bool newFrame)
{
    //! @todo Only when on solid ground
    m_air = 1800;

    LaraState laraState;
    laraState.position = loader::ExactTRCoordinates(getPosition());
    laraState.collisionRadius = 100; //!< @todo MAGICK 100
    laraState.frobbelFlags = LaraState::FrobbelFlag10 | LaraState::FrobbelFlag08;

    std::unique_ptr<AbstractStateHandler> nextHandler = nullptr;
    if( m_currentStateHandler == nullptr )
    {
        m_currentStateHandler = AbstractStateHandler::create(getCurrentAnimState(), *this);
    }
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
    if( getRotation().Z < 0 )
    {
        m_rotation.Z += makeSpeedValue(182).getScaledExact(getCurrentDeltaTime());
        if( getRotation().Z >= 0 )
            m_rotation.Z = 0;
    }
    else if( getRotation().Z > 0 )
    {
        m_rotation.Z -= makeSpeedValue(182).getScaledExact(getCurrentDeltaTime());
        if( getRotation().Z <= 0 )
            m_rotation.Z = 0;
    }

    if( getYRotationSpeed() < 0 )
    {
        m_yRotationSpeed.addExact(364, getCurrentDeltaTime()).limitMax(0);
    }
    else if( getYRotationSpeed() > 0 )
    {
        m_yRotationSpeed.subExact(364, getCurrentDeltaTime()).limitMin(0);
    }
    else
    {
        setYRotationSpeed(0);
    }

    m_rotation.Y += m_yRotationSpeed.getScaledExact(getCurrentDeltaTime());

    {
        //! @todo This is horribly inefficient code, but it properly converts ZXY angles to XYZ angles.
        irr::core::quaternion q;
        q.makeIdentity();
        q *= irr::core::quaternion().fromAngleAxis(util::auToRad(getRotation().Y), {0,1,0});
        q *= irr::core::quaternion().fromAngleAxis(util::auToRad(getRotation().X), {1,0,0});
        q *= irr::core::quaternion().fromAngleAxis(util::auToRad(getRotation().Z), {0,0,-1});

        irr::core::vector3df euler;
        q.toEuler(euler);
        m_lara->setRotation(euler * 180 / irr::core::PI);
    }

    if( !newFrame )
        return;

    //BOOST_LOG_TRIVIAL(debug) << "Post-processing state: " << loader::toString(m_currentStateHandler->getId());

    auto animCommandOverride = processAnimCommands();
    if( animCommandOverride )
    {
        m_currentStateHandler = std::move(animCommandOverride);
        BOOST_LOG_TRIVIAL(debug) << "New anim command state override: " << loader::toString(m_currentStateHandler->getId());
    }

    // @todo test interactions?

    nextHandler = m_currentStateHandler->postprocessFrame(laraState);
    if( nextHandler != nullptr )
    {
        m_currentStateHandler = std::move(nextHandler);
        BOOST_LOG_TRIVIAL(debug) << "New post-processing state override: " << loader::toString(m_currentStateHandler->getId());
    }

    updateFloorHeight(-381);
    handleTriggers(laraState.current.floor.lastTriggerOrKill, false);
}

irr::u32 LaraStateHandler::getCurrentFrame() const
{
    return m_dispatcher->getCurrentFrame();
}

irr::u32 LaraStateHandler::getAnimEndFrame() const
{
    return m_dispatcher->getAnimEndFrame();
}

void LaraStateHandler::placeOnFloor(const LaraState& state)
{
    m_position.Y += state.current.floor.distance;
}

loader::LaraStateId LaraStateHandler::getCurrentState() const
{
    return m_currentStateHandler->getId();
}

loader::LaraStateId LaraStateHandler::getCurrentAnimState() const
{
    return static_cast<loader::LaraStateId>(m_dispatcher->getCurrentAnimState());
}

LaraStateHandler::~LaraStateHandler() = default;

void LaraStateHandler::animateNode(irr::scene::ISceneNode* node, irr::u32 timeMs)
{
    BOOST_ASSERT(m_lara == node);

    if( m_lastFrameTime < 0 )
        m_lastFrameTime = m_lastEngineFrameTime = m_currentFrameTime = timeMs;

    if( m_lastFrameTime == timeMs )
        return;

    m_currentFrameTime = timeMs;

    static constexpr int FrameTime = 1000 / 30;

    bool isNewFrame = m_lastAnimFrame != getCurrentFrame();

    if( timeMs - m_lastEngineFrameTime >= FrameTime )
    {
        isNewFrame = true;
        m_lastEngineFrameTime -= (timeMs - m_lastEngineFrameTime) / FrameTime * FrameTime;
    }

    handleLaraStateOnLand(isNewFrame);

    m_lastFrameTime = m_currentFrameTime;
}

std::unique_ptr<AbstractStateHandler> LaraStateHandler::processAnimCommands()
{
    std::unique_ptr<AbstractStateHandler> nextHandler = nullptr;
    bool newFrame = false;
    if( m_dispatcher->handleTRTransitions() || m_lastAnimFrame != getCurrentFrame() )
    {
        nextHandler = AbstractStateHandler::create(getCurrentAnimState(), *this);
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
                    setFallSpeed(m_fallSpeedOverride == 0 ? cmd[0] : m_fallSpeedOverride);
                    m_fallSpeedOverride = 0;
                    setHorizontalSpeed(cmd[1]);
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
                        m_rotation.Y += util::degToAu(180);
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
        m_horizontalSpeed.addExact(m_dispatcher->getAccelleration(), getCurrentDeltaTime());
        if( getFallSpeed().get() >= 128 )
            m_fallSpeed.addExact(1, getCurrentDeltaTime());
        else
            m_fallSpeed.addExact(6, getCurrentDeltaTime());
    }
    else
    {
        m_horizontalSpeed = m_dispatcher->calculateFloorSpeed();
    }

    move(
         std::sin(util::auToRad(getMovementAngle())) * m_horizontalSpeed.getScaledExact(getCurrentDeltaTime()),
         getFallSpeed().getScaledExact(getCurrentDeltaTime()),
         std::cos(util::auToRad(getMovementAngle())) * m_horizontalSpeed.getScaledExact(getCurrentDeltaTime())
        );

    m_lara->setPosition(m_position.toIrrlicht());
    m_lara->updateAbsolutePosition();

    return nextHandler;
}

void LaraStateHandler::updateFloorHeight(int dy)
{
    auto pos = getPosition();
    pos.Y += dy;
    auto sector = getLevel().findSectorForPosition(pos, getLevel().m_camera->getCurrentRoom());
    HeightInfo hi = HeightInfo::fromFloor(sector, pos, getLevel().m_camera);
    setFloorHeight(hi.distance);

    //! @todo Check room ownership change
}

void LaraStateHandler::handleTriggers(const uint16_t* floorData, bool isDoppelganger)
{
    if( floorData == nullptr )
        return;

    if( loader::extractFDFunction(*floorData) == loader::FDFunction::Death )
    {
        if( !isDoppelganger )
        {
            if( irr::core::equals(getPosition().Y, getFloorHeight(), 1) )
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
        switch(loader::extractTriggerType(*nextFloorData))
        {
        case loader::TriggerType::Trigger:
            ++nextFloorData;
            //! @todo handle trigger
            break;
        case loader::TriggerType::Pad:
            ++nextFloorData;
            ++nextFloorData;
            //! @todo handle pad
            break;
        case loader::TriggerType::AntiPad:
            ++nextFloorData;
            //! @todo handle antipad
            break;
        case loader::TriggerType::Switch:
            ++nextFloorData;
            //! @todo handle switch
            break;
        case loader::TriggerType::Key:
            ++nextFloorData;
            //! @todo handle key
            break;
        case loader::TriggerType::Pickup:
            ++nextFloorData;
            //! @todo handle pickup
            break;
        case loader::TriggerType::Heavy:
            ++nextFloorData;
            //! @todo handle heavy
            break;
        case loader::TriggerType::Combat:
            ++nextFloorData;
            //! @todo handle combat
            break;
        case loader::TriggerType::Dummy:
            ++nextFloorData;
            //! @todo handle dummy
            break;
        case loader::TriggerType::HeavySwitch:
            ++nextFloorData;
            //! @todo handle heavy switch
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

irr::core::aabbox3di LaraStateHandler::getBoundingBox() const
{
    return m_dispatcher->getBoundingBox();
}
