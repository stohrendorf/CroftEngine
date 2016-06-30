#include "laracontroller.h"

#include "abstractstatehandler.h"
#include "animationcontroller.h"
#include "cameracontroller.h"
#include "heightinfo.h"
#include "larastate.h"
#include "render/textureanimator.h"

#include <boost/range/adaptors.hpp>

namespace engine
{
    void LaraController::setTargetState(LaraStateId st)
    {
        ItemController::setTargetState(static_cast<uint16_t>(st));
    }

    loader::LaraStateId LaraController::getTargetState() const
    {
        return static_cast<LaraStateId>(ItemController::getTargetState());
    }

    void LaraController::playAnimation(loader::AnimationId anim, const boost::optional<irr::u32>& firstFrame)
    {
        ItemController::playAnimation(static_cast<uint16_t>(anim), firstFrame);
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
            addZRotation(core::makeInterpolatedValue(+1_deg).getScaled(getCurrentDeltaTime()));
            if( getRotation().Z >= 0_deg )
                setZRotation(0_deg);
        }
        else if( getRotation().Z > 0_deg )
        {
            addZRotation(-core::makeInterpolatedValue(+1_deg).getScaled(getCurrentDeltaTime()));
            if( getRotation().Z <= 0_deg )
                setZRotation(0_deg);
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

        addYRotation(m_yRotationSpeed.getScaled(getCurrentDeltaTime()));

        applyRotation();

        //BOOST_LOG_TRIVIAL(debug) << "Post-processing state: " << loader::toString(m_currentStateHandler->getId());

        auto animCommandOverride = processLaraAnimCommands();
        if( animCommandOverride )
        {
            m_currentStateHandler = std::move(animCommandOverride);
            BOOST_LOG_TRIVIAL(debug) << "New anim command state override: " << loader::toString(m_currentStateHandler->getId());
        }

        if( !newFrame )
            return;

        testInteractions(laraState);

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
            addZRotation(core::makeInterpolatedValue(+2_deg).getScaled(getCurrentDeltaTime()));
            if( getRotation().Z >= 0_deg )
                setZRotation(0_deg);
        }
        else if( getRotation().Z > 0_deg )
        {
            addZRotation(-core::makeInterpolatedValue(+2_deg).getScaled(getCurrentDeltaTime()));
            if( getRotation().Z <= 0_deg )
                setZRotation(0_deg);
        }
        setXRotation(irr::core::clamp(getRotation().X, -100_deg, +100_deg));
        setZRotation(irr::core::clamp(getRotation().Z, -22_deg, +22_deg));
        {
            auto pos = getPosition();
            pos.X += getRotation().Y.sin() * getRotation().X.cos() * getFallSpeed().getScaled(getCurrentDeltaTime()) / 4;
            pos.Y -= getRotation().X.sin() * getFallSpeed().getScaled(getCurrentDeltaTime()) / 4;
            pos.Z += getRotation().Y.cos() * getRotation().X.cos() * getFallSpeed().getScaled(getCurrentDeltaTime()) / 4;
            setPosition(pos);
        }

        applyRotation();
        getSceneNode()->setPosition(getPosition().toIrrlicht());
        getSceneNode()->updateAbsolutePosition();

        auto animCommandOverride = processLaraAnimCommands();
        if( animCommandOverride )
        {
            m_currentStateHandler = std::move(animCommandOverride);
            BOOST_LOG_TRIVIAL(debug) << "New anim command state override: " << loader::toString(m_currentStateHandler->getId());
        }

        if( !newFrame )
            return;

        testInteractions(laraState);

        nextHandler = m_currentStateHandler->postprocessFrame(laraState);
        if( nextHandler != nullptr )
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
            addZRotation(core::makeInterpolatedValue(+2_deg).getScaled(getCurrentDeltaTime()));
            if( getRotation().Z >= 0_deg )
                setZRotation(0_deg);
        }
        else if( getRotation().Z > 0_deg )
        {
            addZRotation(-core::makeInterpolatedValue(+2_deg).getScaled(getCurrentDeltaTime()));
            if( getRotation().Z <= 0_deg )
                setZRotation(0_deg);
        }

        setPosition(getPosition() + core::ExactTRCoordinates(
                                                             getMovementAngle().sin() * getFallSpeed().getScaled(getCurrentDeltaTime()) / 4,
                                                             0,
                                                             getMovementAngle().cos() * getFallSpeed().getScaled(getCurrentDeltaTime()) / 4
                                                            ));

        applyRotation();
        getSceneNode()->setPosition(getPosition().toIrrlicht());
        getSceneNode()->updateAbsolutePosition();

        auto animCommandOverride = processLaraAnimCommands();
        if( animCommandOverride )
        {
            m_currentStateHandler = std::move(animCommandOverride);
            BOOST_LOG_TRIVIAL(debug) << "New anim command state override: " << loader::toString(m_currentStateHandler->getId());
        }

        if( !newFrame )
            return;

        testInteractions(laraState);

        nextHandler = m_currentStateHandler->postprocessFrame(laraState);
        if( nextHandler != nullptr )
        {
            m_currentStateHandler = std::move(nextHandler);
            BOOST_LOG_TRIVIAL(debug) << "New post-processing state override: " << loader::toString(m_currentStateHandler->getId());
        }

        updateFloorHeight(100);
        handleTriggers(laraState.current.floor.lastTriggerOrKill, false);
    }

    void LaraController::placeOnFloor(const LaraState& state)
    {
        auto pos = getPosition();
        pos.Y += state.current.floor.distance;
        setPosition(pos);
    }

    loader::LaraStateId LaraController::getCurrentState() const
    {
        return m_currentStateHandler->getId();
    }

    loader::LaraStateId LaraController::getCurrentAnimState() const
    {
        return static_cast<loader::LaraStateId>(ItemController::getCurrentAnimState());
    }

    LaraController::~LaraController() = default;

    void LaraController::animate(bool isNewFrame)
    {
        if(isNewFrame)
        {
            for(const std::unique_ptr<ItemController>& ctrl : getLevel().m_itemControllers | boost::adaptors::map_values)
            {
                if(ctrl->m_isActive && ctrl->m_hasProcessAnimCommandsOverride)
                    ctrl->processAnimCommands();
            }
        }

        static constexpr int UVAnimTime = 1000 / 10;

        m_uvAnimTime += getCurrentDeltaTime();
        if( m_uvAnimTime >= UVAnimTime )
        {
            getLevel().m_textureAnimator->updateCoordinates(getLevel().m_textureProxies);
            m_uvAnimTime -= UVAnimTime;
        }

        if( m_currentStateHandler == nullptr )
        {
            m_currentStateHandler = AbstractStateHandler::create(getCurrentAnimState(), *this);
        }

        if( m_underwaterState == UnderwaterState::OnLand && getCurrentRoom()->isWaterRoom() )
        {
            m_air = 1800;
            m_underwaterState = UnderwaterState::Diving;
            setFalling(false);
            setPosition(getPosition() + core::ExactTRCoordinates(0, 100, 0));
            updateFloorHeight(0);
            //! @todo stop sound 30
            if( getCurrentAnimState() == LaraStateId::SwandiveBegin )
            {
                setXRotation(-45_deg);
                setTargetState(LaraStateId::UnderwaterDiving);
                if( auto tmp = processLaraAnimCommands() )
                    m_currentStateHandler = std::move(tmp);
                setFallSpeed(getFallSpeed() * 2);
            }
            else if( getCurrentAnimState() == LaraStateId::SwandiveEnd )
            {
                setXRotation(-85_deg);
                setTargetState(LaraStateId::UnderwaterDiving);
                if( auto tmp = processLaraAnimCommands() )
                    m_currentStateHandler = std::move(tmp);
                setFallSpeed(getFallSpeed() * 2);
            }
            else
            {
                setXRotation(-45_deg);
                playAnimation(loader::AnimationId::FREE_FALL_TO_UNDERWATER, 1895);
                setTargetState(LaraStateId::UnderwaterForward);
                m_currentStateHandler = AbstractStateHandler::create(LaraStateId::UnderwaterDiving, *this);
                if( auto tmp = processLaraAnimCommands() )
                    m_currentStateHandler = std::move(tmp);
                setFallSpeed(getFallSpeed() * 1.5f);
            }

            //! @todo Show water splash effect
        }
        else if( m_underwaterState == UnderwaterState::Diving && !getCurrentRoom()->isWaterRoom() )
        {
            auto waterSurfaceHeight = getWaterSurfaceHeight();
            if( !waterSurfaceHeight || std::abs(*waterSurfaceHeight - getPosition().Y) >= 256 )
            {
                m_underwaterState = UnderwaterState::OnLand;
                playAnimation(loader::AnimationId::FREE_FALL_FORWARD, 492);
                setTargetState(LaraStateId::JumpForward);
                m_currentStateHandler = AbstractStateHandler::create(LaraStateId::JumpForward, *this);
                setFallSpeed(core::makeInterpolatedValue(0.0f));
                //! @todo Check formula
                setHorizontalSpeed(getHorizontalSpeed() * 0.2f);
                setFalling(true);
                m_handStatus = 0;
                setXRotation(0_deg);
                setZRotation(0_deg);
            }
            else
            {
                m_underwaterState = UnderwaterState::Swimming;
                playAnimation(loader::AnimationId::UNDERWATER_TO_ONWATER, 1937);
                setTargetState(LaraStateId::OnWaterStop);
                m_currentStateHandler = AbstractStateHandler::create(LaraStateId::OnWaterStop, *this);
                m_handStatus = 0;
                setXRotation(0_deg);
                setZRotation(0_deg);
                {
                    auto pos = getPosition();
                    pos.Y = *waterSurfaceHeight + 1;
                    setPosition(pos);
                }
                m_swimToDiveKeypressDuration = 11 * 1000 / 30;
                updateFloorHeight(-381);
                //! @todo play sound 36
            }
        }
        else if( m_underwaterState == UnderwaterState::Swimming && !getCurrentRoom()->isWaterRoom() )
        {
            m_underwaterState = UnderwaterState::OnLand;
            playAnimation(loader::AnimationId::FREE_FALL_FORWARD, 492);
            setTargetState(LaraStateId::JumpForward);
            m_currentStateHandler = AbstractStateHandler::create(LaraStateId::JumpForward, *this);
            setFallSpeed(core::makeInterpolatedValue(0.0f));
            //! @todo Check formula
            setHorizontalSpeed(getHorizontalSpeed() * 0.2f);
            setFalling(true);
            m_handStatus = 0;
            setXRotation(0_deg);
            setZRotation(0_deg);
        }

        if( m_underwaterState == UnderwaterState::OnLand )
        {
            m_air = 1800;
            handleLaraStateOnLand(isNewFrame);
        }
        else if( m_underwaterState == UnderwaterState::Diving )
        {
            if( m_health >= 0 )
            {
                m_air.sub(1, getCurrentDeltaTime());
                if( m_air < 0 )
                {
                    m_air = -1;
                    m_health.sub(5, getCurrentDeltaTime());
                }
            }
            handleLaraStateDiving(isNewFrame);
        }
        else if( m_underwaterState == UnderwaterState::Swimming )
        {
            if( m_health >= 0 )
            {
                m_air.add(10, getCurrentDeltaTime()).limitMax(1800);
            }
            handleLaraStateSwimming(isNewFrame);
        }
    }

    std::unique_ptr<AbstractStateHandler> LaraController::processLaraAnimCommands(bool advanceFrame)
    {
        std::unique_ptr<AbstractStateHandler> nextHandler = nullptr;
        bool newFrame = false;

        if( advanceFrame )
        {
            nextFrame();
        }

        if( handleTRTransitions() || getLastAnimFrame() != getCurrentFrame() )
        {
            nextHandler = m_currentStateHandler->createWithRetainedAnimation(getCurrentAnimState());
            setLastAnimFrame(getCurrentFrame());
            newFrame = true;
        }

        const bool isAnimEnd = getCurrentFrame() >= getAnimEndFrame();

        const loader::Animation& animation = getLevel().m_animations[getCurrentAnimationId()];
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
                            addYRotation(180_deg);
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

        if( isFalling() )
        {
            getHorizontalSpeed().add(getAnimAccelleration(), getCurrentDeltaTime());
            if( getFallSpeed() >= 128 )
                getFallSpeed().add(1, getCurrentDeltaTime());
            else
                getFallSpeed().add(6, getCurrentDeltaTime());
        }
        else
        {
            setHorizontalSpeed(core::makeInterpolatedValue(calculateAnimFloorSpeed()));
        }

        move(
             getMovementAngle().sin() * getHorizontalSpeed().getScaled(getCurrentDeltaTime()),
             isFalling() ? getFallSpeed().getScaled(getCurrentDeltaTime()) : 0,
             getMovementAngle().cos() * getHorizontalSpeed().getScaled(getCurrentDeltaTime())
            );

        applyPosition();

        return nextHandler;
    }

    void LaraController::updateFloorHeight(int dy)
    {
        auto pos = getPosition();
        pos.Y += dy;
        gsl::not_null<const loader::Room*> room = getCurrentRoom();
        auto sector = getLevel().findFloorSectorWithClampedPosition(pos.toInexact(), &room);
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

        const auto srcTriggerType = loader::extractTriggerType(*floorData);
        const auto srcTriggerArg = floorData[1];
        auto actionFloorData = floorData + 2;

        getLevel().m_cameraController->findCameraTarget(actionFloorData);

        bool runActions = false, switchIsOn = false;
        if( !isDoppelganger )
        {
            switch( srcTriggerType )
            {
            case loader::TriggerType::Trigger:
                runActions = true;
                break;
            case loader::TriggerType::Pad:
            case loader::TriggerType::AntiPad:
                runActions = getPosition().Y == getFloorHeight();
                break;
            case loader::TriggerType::Switch:
                {
                    Expects(getLevel().m_itemControllers.find(loader::extractTriggerFunctionParam(*actionFloorData)) != getLevel().m_itemControllers.end());
                    ItemController& swtch = *getLevel().m_itemControllers[loader::extractTriggerFunctionParam(*actionFloorData)];
                    if(!swtch.triggerSwitch(srcTriggerArg))
                        return;

                    switchIsOn = (swtch.getCurrentAnimState() == 1);
                }
                ++actionFloorData;
                runActions = true;
                break;
            case loader::TriggerType::Key:
                //! @todo Handle key
                ++actionFloorData;
                runActions = true;
                return;
            case loader::TriggerType::Pickup:
                //! @todo Handle pickup
                ++actionFloorData;
                runActions = true;
                return;
            case loader::TriggerType::Combat:
                runActions = getHandStatus() == 4;
                break;
            case loader::TriggerType::Heavy:
            case loader::TriggerType::Dummy:
                return;
            default:
                runActions = true;
                break;
            }
        }
        else
        {
            runActions = srcTriggerType == loader::TriggerType::Heavy;
        }

        if( !runActions )
            return;

        const ItemController* lookAtItem = nullptr;

        while( true )
        {
            const bool isLastAction = loader::isLastFloordataEntry(*actionFloorData);
            const auto actionParam = loader::extractTriggerFunctionParam(*actionFloorData);
            switch( loader::extractTriggerFunction(*actionFloorData++) )
            {
            case loader::TriggerFunction::Object:
                {
                    Expects(getLevel().m_itemControllers.find(actionParam) != getLevel().m_itemControllers.end());
                    ItemController& item = *getLevel().m_itemControllers[actionParam];
                    if( (item.m_itemFlags & Oneshot) != 0 )
                        break;

                    item.m_triggerTimeout = static_cast<uint8_t>(srcTriggerArg);
                    if( item.m_triggerTimeout != 1 )
                        item.m_triggerTimeout *= 1000;

                    if( srcTriggerType == loader::TriggerType::Switch )
                        item.m_itemFlags ^= srcTriggerArg & ActivationMask;
                    else if( srcTriggerType == loader::TriggerType::AntiPad )
                        item.m_itemFlags &= ~(srcTriggerArg & ActivationMask);
                    else
                        item.m_itemFlags |= srcTriggerArg & ActivationMask;

                    if( (item.m_itemFlags & ActivationMask) != ActivationMask)
                        break;

                    if( (srcTriggerArg & Oneshot) != 0 )
                        item.m_itemFlags |= Oneshot;

                    if(item.m_isActive)
                        break;

                    if( (item.m_characteristics & 0x02) == 0 )
                    {
                        item.m_flags2_02_toggledOn = true;
                        item.m_flags2_04_ready = false;
                        item.activate();
                        break;
                    }

                    if( !item.m_flags2_02_toggledOn && !item.m_flags2_04_ready )
                    {
                        //! @todo Implement baddie
                        item.m_flags2_02_toggledOn = true;
                        item.m_flags2_04_ready = false;
                        item.activate();
                        break;
                    }

                    if(!item.m_flags2_02_toggledOn || !item.m_flags2_04_ready)
                        break;

                    //! @todo Implement baddie
                    if( false ) //!< @todo unpauseBaddie
                    {
                        item.m_flags2_02_toggledOn = true;
                        item.m_flags2_04_ready = false;
                    }
                    else
                    {
                        item.m_flags2_02_toggledOn = true;
                        item.m_flags2_04_ready = true;
                    }
                    item.activate();
                }
                break;
            case loader::TriggerFunction::CameraTarget:
                getLevel().m_cameraController->setCamOverride(actionFloorData[0], actionParam, srcTriggerType, isDoppelganger, srcTriggerArg, switchIsOn);
                ++actionFloorData;
                break;
            case loader::TriggerFunction::LookAt:
                lookAtItem = getLevel().getItemController(actionParam);
                break;
            case loader::TriggerFunction::UnderwaterCurrent:
                //! @todo handle underwater current
                break;
            case loader::TriggerFunction::FlipMap:
                //! @todo handle flip map
                break;
            case loader::TriggerFunction::FlipOn:
                //! @todo handle flip on
                break;
            case loader::TriggerFunction::FlipOff:
                //! @todo handle flip off
                break;
            case loader::TriggerFunction::FlipEffect:
                //! @todo handle flip effect
                break;
            case loader::TriggerFunction::EndLevel:
                //! @todo handle level end
                break;
            case loader::TriggerFunction::PlayTrack:
                //! @todo handle "play track"
                break;
            case loader::TriggerFunction::Secret:
                //! @todo handle secrets
                break;
            default:
                break;
            }

            if( isLastAction )
                break;
        }

        getLevel().m_cameraController->setLookAtItem(lookAtItem);

        //! @todo Implement the rest
    }

    boost::optional<int> LaraController::getWaterSurfaceHeight() const
    {
        auto sector = getCurrentRoom()->getSectorByAbsolutePosition(getPosition().toInexact());

        if( getCurrentRoom()->isWaterRoom() )
        {
            while( true )
            {
                if( sector->roomAbove == 0xff )
                    break;

                BOOST_ASSERT(sector->roomAbove < getLevel().m_rooms.size());
                const auto& room = getLevel().m_rooms[sector->roomAbove];
                if( !room.isWaterRoom() )
                    break;

                sector = room.getSectorByAbsolutePosition(getPosition().toInexact());
            }

            return sector->ceilingHeight * loader::QuarterSectorSize;
        }

        while( true )
        {
            if( sector->roomBelow == 0xff )
                break;

            BOOST_ASSERT(sector->roomBelow < getLevel().m_rooms.size());
            const auto& room = getLevel().m_rooms[sector->roomBelow];
            if( room.isWaterRoom() )
            {
                return sector->floorHeight * loader::QuarterSectorSize;
            }

            sector = room.getSectorByAbsolutePosition(getPosition().toInexact());
        }

        return sector->ceilingHeight * loader::QuarterSectorSize;
    }

    void LaraController::setCameraRotation(core::Angle x, core::Angle y)
    {
        getLevel().m_cameraController->setLocalRotation(x, y);
    }

    void LaraController::setCameraRotationY(core::Angle y)
    {
        getLevel().m_cameraController->setLocalRotationY(y);
    }

    void LaraController::setCameraRotationX(core::Angle x)
    {
        getLevel().m_cameraController->setLocalRotationX(x);
    }

    void LaraController::setCameraDistance(int d)
    {
        getLevel().m_cameraController->setLocalDistance(d);
    }

    void LaraController::testInteractions(LaraState& state)
    {
        m_flags2_10 = false;

        if( m_health < 0 )
            return;

        std::set<const loader::Room*> rooms;
        rooms.insert(getCurrentRoom());
        for( const loader::Portal& p : getCurrentRoom()->portals )
            rooms.insert(&getLevel().m_rooms[p.adjoining_room]);

        for( const std::unique_ptr<ItemController>& ctrl : getLevel().m_itemControllers | boost::adaptors::map_values )
        {
            if( rooms.find(ctrl->getCurrentRoom()) == rooms.end() )
                continue;

            if( !ctrl->m_flags2_20 )
                continue;

            if( ctrl->m_flags2_04_ready && ctrl->m_flags2_02_toggledOn )
                continue;

            const auto d = getPosition() - ctrl->getPosition();
            if( std::abs(d.X) >= 4096 || std::abs(d.Y) >= 4096 || std::abs(d.Z) >= 4096 )
                continue;

            ctrl->onInteract(*this, state);
        }
    }
}
