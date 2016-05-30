#include "larastatehandler.h"

#include "defaultanimdispatcher.h"
#include "trcamerascenenodeanimator.h"
#include "heightinfo.h"
#include "core/magic.h"
#include "larastate.h"
#include <larastate.h>

void LaraStateHandler::setTargetState(LaraStateId st)
{
    m_dispatcher->setTargetState(static_cast<uint16_t>(st));
}

void LaraStateHandler::setStateOverride(LaraStateId st)
{
    m_dispatcher->setStateOverride(static_cast<uint16_t>(st));
}

void LaraStateHandler::clearStateOverride()
{
    m_dispatcher->clearStateOverride();
}

loader::LaraStateId LaraStateHandler::getTargetState() const
{
    return static_cast<LaraStateId>(m_dispatcher->getTargetState());
}

void LaraStateHandler::playAnimation(loader::AnimationId anim, const boost::optional<irr::u32>& firstFrame)
{
    m_dispatcher->playLocalAnimation(static_cast<uint16_t>(anim), firstFrame);
}

void LaraStateHandler::onInput0WalkForward(LaraState& /*state*/)
{
    if( getHealth() <= 0 )
    {
        setTargetState(LaraStateId::Stop);
        return;
    }

    if( getInputState().xMovement == AxisMovement::Left )
        subYRotationSpeed(409, -728);
    else if( getInputState().xMovement == AxisMovement::Right )
        addYRotationSpeed(409, 728);
    if( getInputState().zMovement == AxisMovement::Forward )
    {
        if( getInputState().moveSlow )
            setTargetState(LaraStateId::WalkForward);
        else
            setTargetState(LaraStateId::RunForward);
    }
    else
    {
        setTargetState(LaraStateId::Stop);
    }
}

void LaraStateHandler::onBehave0WalkForward(LaraState& state)
{
    setFallSpeed(0);
    setFalling(false);
    state.yAngle = getRotation().Y;
    setMovementAngle(state.yAngle);
    state.neededFloorDistanceBottom = core::ClimbLimit2ClickMin;
    state.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
    state.neededCeilingDistance = 0;
    state.frobbelFlags |= LaraState::FrobbelFlag_UnpassableSteepUpslant | LaraState::FrobbelFlag_UnwalkableSteepFloor | LaraState::FrobbelFlag_UnwalkableDeadlyFloor;
    state.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);

    if( tryStopOnFloor(state) || tryClimb(state) )
        return;

    if( checkWallCollision(state) )
    {
        const auto fr = getCurrentFrame();
        if( fr >= 29 && fr <= 47 )
        {
            playAnimation(loader::AnimationId::END_WALK_LEFT, 74);
        }
        else if( (fr >= 22 && fr <= 28) || (fr >= 48 && fr <= 57) )
        {
            playAnimation(loader::AnimationId::END_WALK_RIGHT, 58);
        }
        else
        {
            playAnimation(loader::AnimationId::STAY_SOLID, 185);
        }
    }

    if( state.current.floor.distance > core::ClimbLimit2ClickMin )
    {
        playAnimation(loader::AnimationId::FREE_FALL_FORWARD, 492);
        setStateOverride(LaraStateId::JumpForward);
        setTargetState(LaraStateId::JumpForward);
        setFallSpeed(0);
        setFalling(true);
    }

    if( state.current.floor.distance > core::SteppableHeight )
    {
        const auto fr = getCurrentFrame();
        if( fr < 28 || fr > 45 )
        {
            playAnimation(loader::AnimationId::WALK_DOWN_RIGHT, 887);
        }
        else
        {
            playAnimation(loader::AnimationId::WALK_DOWN_LEFT, 874);
        }
    }

    if( state.current.floor.distance >= -core::ClimbLimit2ClickMin && state.current.floor.distance < -core::SteppableHeight )
    {
        const auto fr = getCurrentFrame();
        if( fr < 27 || fr > 44 )
        {
            playAnimation(loader::AnimationId::WALK_UP_STEP_RIGHT, 844);
        }
        else
        {
            playAnimation(loader::AnimationId::WALK_UP_STEP_LEFT, 858);
        }
    }

    if( !tryStartSlide(state) )
    {
        placeOnFloor(state);
    }
}

void LaraStateHandler::onInput1RunForward(LaraState& /*state*/)
{
    if( getHealth() <= 0 )
    {
        setTargetState(LaraStateId::Death);
        return;
    }

    if( getInputState().roll )
    {
        playAnimation(loader::AnimationId::ROLL_BEGIN, 3857);
        setTargetState(LaraStateId::Stop);
        setStateOverride(LaraStateId::RollForward);
        return;
    }

    if( getInputState().xMovement == AxisMovement::Left )
    {
        subYRotationSpeed(409, -1456);
        setZRotationExact( std::max(-2002.f, getRotation().Z - makeSpeedValue(273).getScaledExact(getCurrentDeltaTime())) );
    }
    else if( getInputState().xMovement == AxisMovement::Right )
    {
        addYRotationSpeed(409, 1456);
        setZRotationExact( std::min(2002.f, getRotation().Z + makeSpeedValue(273).getScaledExact(getCurrentDeltaTime())) );
    }

    if( getInputState().jump && !isFalling() )
    {
        setTargetState(LaraStateId::JumpForward);
        return;
    }

    if( getInputState().zMovement != AxisMovement::Forward )
    {
        setTargetState(LaraStateId::Stop);
        return;
    }

    if( getInputState().moveSlow )
        setTargetState(LaraStateId::WalkForward);
    else
        setTargetState(LaraStateId::RunForward);
}

void LaraStateHandler::onInput2Stop(LaraState& state)
{
    if( getHealth() <= 0 )
    {
        setTargetState(LaraStateId::Death);
        return;
    }

    if( getInputState().roll )
    {
        playAnimation(loader::AnimationId::ROLL_BEGIN);
        setTargetState(LaraStateId::Stop);
        setStateOverride(LaraStateId::RollForward);
        return;
    }

    setTargetState(LaraStateId::Stop);

    if( getInputState().stepMovement == AxisMovement::Left )
    {
        setTargetState(LaraStateId::StepLeft);
    }
    else if( getInputState().stepMovement == AxisMovement::Right )
    {
        setTargetState(LaraStateId::StepRight);
    }

    if( getInputState().xMovement == AxisMovement::Left )
    {
        setTargetState(LaraStateId::TurnLeftSlow);
    }
    else if( getInputState().xMovement == AxisMovement::Right )
    {
        setTargetState(LaraStateId::TurnRightSlow);
    }

    if( getInputState().jump )
    {
        setTargetState(LaraStateId::JumpPrepare);
    }
    else if( getInputState().zMovement == AxisMovement::Forward )
    {
        if( getInputState().moveSlow )
            onInput0WalkForward(state);
        else
            onInput1RunForward(state);
    }
    else if( getInputState().zMovement == AxisMovement::Backward )
    {
        if( getInputState().moveSlow )
            onInput16WalkBackward(state);
        else
            setTargetState(LaraStateId::RunBack);
    }
}

void LaraStateHandler::onInput3JumpForward(LaraState& /*state*/)
{
    if( getTargetState() == LaraStateId::SwandiveBegin || getTargetState() == LaraStateId::Reach )
        setTargetState(LaraStateId::JumpForward);

    if( getTargetState() != LaraStateId::Death && getTargetState() != LaraStateId::Stop )
    {
        if( getInputState().action && getHandStatus() == 0 )
            setTargetState(LaraStateId::Reach);
        if( getInputState().moveSlow && getHandStatus() == 0 )
            setTargetState(LaraStateId::SwandiveBegin);
        if( getFallSpeed().get() > core::FreeFallSpeedThreshold )
            setTargetState(LaraStateId::FreeFall);
    }

    if( getInputState().xMovement == AxisMovement::Left )
    {
        subYRotationSpeed(409, -546);
    }
    else if( getInputState().xMovement == AxisMovement::Right )
    {
        addYRotationSpeed(409, 546);
    }
}

void LaraStateHandler::onInput5RunBackward(LaraState& /*state*/)
{
    setTargetState(LaraStateId::Stop);

    if( getInputState().xMovement == AxisMovement::Left )
        subYRotationSpeed(409, -1092);
    else if( getInputState().xMovement == AxisMovement::Right )
        addYRotationSpeed(409, 1092);
}

void LaraStateHandler::onInput6TurnRightSlow(LaraState& /*state*/)
{
    if( getHealth() <= 0 )
    {
        setTargetState(LaraStateId::Stop);
        return;
    }

    addYRotationSpeed(409);

    if( getHandStatus() == 4 )
    {
        setTargetState(LaraStateId::TurnFast);
        return;
    }

    if( getYRotationSpeed() > 728 )
    {
        if( getInputState().moveSlow )
            setYRotationSpeed( 728 );
        else
            setTargetState(LaraStateId::TurnFast);
    }

    if( getInputState().zMovement != AxisMovement::Forward )
    {
        if( getInputState().xMovement != AxisMovement::Right )
            setTargetState(LaraStateId::Stop);
        return;
    }

    if( getInputState().moveSlow )
        setTargetState(LaraStateId::WalkForward);
    else
        setTargetState(LaraStateId::RunForward);
}

void LaraStateHandler::onInput7TurnLeftSlow(LaraState& /*state*/)
{
    if( getHealth() <= 0 )
    {
        setTargetState(LaraStateId::Stop);
        return;
    }

    subYRotationSpeed(409);

    if( getHandStatus() == 4 )
    {
        setTargetState(LaraStateId::TurnFast);
        return;
    }

    if( getYRotationSpeed() < -728 )
    {
        if( getInputState().moveSlow )
            setYRotationSpeed(-728);
        else
            setTargetState(LaraStateId::TurnFast);
    }

    if( getInputState().zMovement != AxisMovement::Forward )
    {
        if( getInputState().xMovement != AxisMovement::Left )
            setTargetState(LaraStateId::Stop);
        return;
    }

    if( getInputState().moveSlow )
        setTargetState(LaraStateId::WalkForward);
    else
        setTargetState(LaraStateId::RunForward);
}

void LaraStateHandler::onBehaveTurnSlow(LaraState& state)
{
    setFallSpeed(0);
    setFalling(false);
    state.yAngle = getRotation().Y;
    setMovementAngle(state.yAngle);
    state.neededFloorDistanceBottom = core::ClimbLimit2ClickMin;
    state.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
    state.neededCeilingDistance = 0;
    state.frobbelFlags |= LaraState::FrobbelFlag_UnpassableSteepUpslant | LaraState::FrobbelFlag_UnwalkableSteepFloor;
    state.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);

    if( state.current.floor.distance <= 100 )
    {
        if( !tryStartSlide(state) )
            placeOnFloor(state);

        return;
    }

    playAnimation(loader::AnimationId::FREE_FALL_FORWARD, 492);
    setTargetState(LaraStateId::JumpForward);
    setStateOverride(LaraStateId::JumpForward);
    setFallSpeed(0);
    setFalling(true);
}

void LaraStateHandler::onInput9FreeFall(LaraState& /*state*/)
{
    dampenHorizontalSpeed(5, 100);
    if( getFallSpeed().get() > 154 )
    {
        //! @todo playSound(30)
    }
}

void LaraStateHandler::onInput15JumpPrepare(LaraState& /*state*/)
{
    if( getInputState().zMovement == AxisMovement::Forward && getRelativeHeightAtDirection(getRotation().Y, 256) >= -core::ClimbLimit2ClickMin )
    {
        setMovementAngle(getRotation().Y);
        setTargetState(LaraStateId::JumpForward);
    }
    else if( getInputState().xMovement == AxisMovement::Left && getRelativeHeightAtDirection(getRotation().Y - util::degToAu(90), 256) >= -core::ClimbLimit2ClickMin )
    {
        setMovementAngle(getRotation().Y - util::degToAu(90));
        setTargetState(LaraStateId::JumpRight);
    }
    else if( getInputState().xMovement == AxisMovement::Right && getRelativeHeightAtDirection(getRotation().Y + util::degToAu(90), 256) >= -core::ClimbLimit2ClickMin )
    {
        setMovementAngle(getRotation().Y + util::degToAu(90));
        setTargetState(LaraStateId::JumpLeft);
    }
    else if( getInputState().zMovement == AxisMovement::Backward && getRelativeHeightAtDirection(getRotation().Y + util::degToAu(180), 256) >= -core::ClimbLimit2ClickMin )
    {
        setMovementAngle(getRotation().Y + util::degToAu(180));
        setTargetState(LaraStateId::JumpBack);
    }

    if( getFallSpeed().get() > core::FreeFallSpeedThreshold )
    {
        setTargetState(LaraStateId::FreeFall);
    }
}

void LaraStateHandler::onInput16WalkBackward(LaraState& /*state*/)
{
    if( getHealth() <= 0 )
    {
        setTargetState(LaraStateId::Stop);
        return;
    }

    if( getInputState().zMovement == AxisMovement::Backward && getInputState().moveSlow )
        setTargetState(LaraStateId::WalkBackward);
    else
        setTargetState(LaraStateId::Stop);

    if( getInputState().xMovement == AxisMovement::Left )
        subYRotationSpeed(409, -728);
    else if( getInputState().xMovement == AxisMovement::Right )
        addYRotationSpeed(409, 728);
}

void LaraStateHandler::onInput19Climbing(LaraState& state)
{
    state.frobbelFlags &= ~(LaraState::FrobbelFlag08 | LaraState::FrobbelFlag10);
}

void LaraStateHandler::onInput20TurnFast(LaraState& /*state*/)
{
    if( getHealth() <= 0 )
    {
        setTargetState(LaraStateId::Stop);
        return;
    }

    if( getYRotationSpeed() >= 0 )
    {
        setYRotationSpeed( 1456 );
        if( getInputState().xMovement == AxisMovement::Right )
            return;
    }
    else
    {
        setYRotationSpeed( -1456 );
        if( getInputState().xMovement == AxisMovement::Left )
            return;
    }
    setTargetState(LaraStateId::Stop);
}

void LaraStateHandler::onInput24SlideForward(LaraState& /*state*/)
{
    if( getInputState().jump )
        setTargetState(LaraStateId::JumpForward);
}

void LaraStateHandler::onBehave24SlideForward(LaraState& state)
{
    setMovementAngle(getRotation().Y);
    commonSlideHandling(state);
}

void LaraStateHandler::onInput25JumpBackward(LaraState& /*state*/)
{
    //! @todo Set local camera Y rotation to 24570 AU
    if( getFallSpeed().get() > core::FreeFallSpeedThreshold )
        setTargetState(LaraStateId::FreeFall);
}

void LaraStateHandler::onBehave25JumpBackward(LaraState& state)
{
    setMovementAngle(getRotation().Y + util::degToAu(180));
    commonJumpHandling(state);
}

void LaraStateHandler::onInput28JumpUp(LaraState& /*state*/)
{
    if( getFallSpeed().get() > core::FreeFallSpeedThreshold )
        setTargetState(LaraStateId::FreeFall);
}

void LaraStateHandler::onInput29FallBackward(LaraState& /*state*/)
{
    if( getFallSpeed().get() > core::FreeFallSpeedThreshold )
        setTargetState(LaraStateId::FreeFall);

    if( getInputState().action && getHandStatus() == 0 )
        setTargetState(LaraStateId::Reach);
}

void LaraStateHandler::onBehave29FallBackward(LaraState& state)
{
    state.neededFloorDistanceBottom = loader::HeightLimit;
    state.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
    state.neededCeilingDistance = 192;
    state.yAngle = getRotation().Y + util::degToAu(180);
    state.initHeightInfo(getPosition(), getLevel(), 870); //! @todo MAGICK 870
    checkJumpWallSmash(state);
    if( state.current.floor.distance > 0 || getFallSpeed().get() <= 0 )
        return;

    if( applyLandingDamage(state) )
        setTargetState(LaraStateId::Death);
    else
        setTargetState(LaraStateId::Stop);

    setFallSpeed(0);
    placeOnFloor(state);
    setFalling(false);
}

void LaraStateHandler::onInput32SlideBackward(LaraState& /*state*/)
{
    if( getInputState().jump )
        setTargetState(LaraStateId::JumpBack);
}

void LaraStateHandler::handleLaraStateOnLand()
{
    //! @todo Only when on solid ground
    m_air = 1800;

    LaraState laraState;
    laraState.position = loader::ExactTRCoordinates(getPosition());
    laraState.collisionRadius = 100; //!< @todo MAGICK 100
    laraState.frobbelFlags = LaraState::FrobbelFlag10 | LaraState::FrobbelFlag08;

    static HandlersArray inputHandlers{{
        &LaraStateHandler::onInput0WalkForward,
        &LaraStateHandler::onInput1RunForward,
        &LaraStateHandler::onInput2Stop,
        &LaraStateHandler::onInput3JumpForward,
        nullptr,
        &LaraStateHandler::onInput5RunBackward,
        &LaraStateHandler::onInput6TurnRightSlow,
        &LaraStateHandler::onInput7TurnLeftSlow,
        nullptr,
        &LaraStateHandler::onInput9FreeFall,
        // 10
        nullptr,
        &LaraStateHandler::onInput11Reach,
        &LaraStateHandler::nopHandler,
        nullptr,
        &LaraStateHandler::nopHandler,
        &LaraStateHandler::onInput15JumpPrepare,
        &LaraStateHandler::onInput16WalkBackward,
        nullptr,
        nullptr,
        &LaraStateHandler::onInput19Climbing,
        &LaraStateHandler::onInput20TurnFast,
        nullptr,
        nullptr,
        &LaraStateHandler::nopHandler,
        &LaraStateHandler::onInput24SlideForward,
        &LaraStateHandler::onInput25JumpBackward,
        &LaraStateHandler::onInput26JumpLeft,
        &LaraStateHandler::onInput27JumpRight,
        &LaraStateHandler::onInput28JumpUp,
        &LaraStateHandler::onInput29FallBackward,
        // 30
        nullptr,
        nullptr,
        &LaraStateHandler::onInput32SlideBackward,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        // 40
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        &LaraStateHandler::nopHandler,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        // 50
        nullptr,
        nullptr,
        &LaraStateHandler::onInput52SwandiveBegin,
        &LaraStateHandler::onInput53SwandiveEnd,
        nullptr,
        nullptr
    }};

    callHandler(inputHandlers, m_dispatcher->getCurrentState(), laraState, "input");

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

    processAnimCommands();

    // @todo test interactions?

    // BOOST_LOG_TRIVIAL(debug) << "BEHAVE State=" << m_dispatcher->getCurrentState() << ", pos = (" << m_position.X << "/" << m_position.Y << "/" << m_position.Z << ")";

    static HandlersArray behaviourHandlers{{
        &LaraStateHandler::onBehave0WalkForward,
        &LaraStateHandler::onBehave1RunForward,
        &LaraStateHandler::onBehaveStanding,
        &LaraStateHandler::onBehave3JumpForward,
        &LaraStateHandler::onBehaveStanding,
        &LaraStateHandler::onBehave5RunBackward,
        &LaraStateHandler::onBehaveTurnSlow,
        &LaraStateHandler::onBehaveTurnSlow,
        nullptr,
        &LaraStateHandler::onBehave9FreeFall,
        // 10
        nullptr,
        &LaraStateHandler::onBehave11Reach,
        &LaraStateHandler::onBehave12Unknown,
        nullptr,
        &LaraStateHandler::onBehaveStanding,
        &LaraStateHandler::onBehave15JumpPrepare,
        &LaraStateHandler::onBehave16WalkBackward,
        nullptr,
        nullptr,
        &LaraStateHandler::onBehave19Climbing,
        &LaraStateHandler::onBehaveStanding,
        nullptr,
        nullptr,
        &LaraStateHandler::onBehave23RollBackward,
        &LaraStateHandler::onBehave24SlideForward,
        &LaraStateHandler::onBehave25JumpBackward,
        &LaraStateHandler::onBehave26JumpLeft,
        &LaraStateHandler::onBehave27JumpRight,
        &LaraStateHandler::onBehave28JumpUp,
        &LaraStateHandler::onBehave29FallBackward,
        // 30
        nullptr,
        nullptr,
        &LaraStateHandler::onBehave32SlideBackward,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        // 40
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        &LaraStateHandler::onBehave45RollForward,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        // 50
        nullptr,
        nullptr,
        &LaraStateHandler::onBehave52SwandiveBegin,
        &LaraStateHandler::onBehave53SwandiveEnd,
        nullptr,
        nullptr
    }};

    callHandler(behaviourHandlers, m_dispatcher->getCurrentState(), laraState, "behaviour");

    updateFloorHeight(-381);
    handleTriggers(laraState.current.floor.lastTriggerOrKill, false);
}

irr::u32 LaraStateHandler::getCurrentFrame() const
{
    return m_dispatcher->getCurrentFrame();
}

void LaraStateHandler::placeOnFloor(const LaraState & state)
{
    m_position.Y += state.current.floor.distance;
}

loader::LaraStateId LaraStateHandler::getCurrentState() const
{
    return static_cast<LaraStateId>(m_dispatcher->getCurrentState());
}

void LaraStateHandler::onBehave19Climbing(LaraState& state)
{
    state.neededFloorDistanceBottom = core::ClimbLimit2ClickMin;
    state.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
    state.neededCeilingDistance = 0;
    state.yAngle = getRotation().Y;
    setMovementAngle(state.yAngle);
    state.frobbelFlags |= LaraState::FrobbelFlag_UnpassableSteepUpslant | LaraState::FrobbelFlag_UnwalkableSteepFloor;
    state.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
}

void LaraStateHandler::onBehave15JumpPrepare(LaraState& state)
{
    setFallSpeed(0);
    setFalling(false);
    state.neededFloorDistanceBottom = loader::HeightLimit;
    state.neededFloorDistanceTop = -loader::HeightLimit;
    state.neededCeilingDistance = 0;
    state.yAngle = getRotation().Y;
    state.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);

    if( state.current.ceiling.distance <= -100 )
        return;

    setTargetState(LaraStateId::Stop);
    setStateOverride(LaraStateId::Stop);
    playAnimation(loader::AnimationId::STAY_SOLID, 185);
    setHorizontalSpeed(0);
    setPosition(state.position);
}

void LaraStateHandler::onBehave1RunForward(LaraState& state)
{
    state.yAngle = getRotation().Y;
    setMovementAngle(state.yAngle);
    state.neededFloorDistanceBottom = loader::HeightLimit;
    state.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
    state.neededCeilingDistance = 0;
    state.frobbelFlags |= LaraState::FrobbelFlag_UnpassableSteepUpslant;
    state.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
    if( tryStopOnFloor(state) || tryClimb(state) )
        return;

    if( checkWallCollision(state) )
    {
        setZRotation(0);
        if( state.front.floor.slantClass == SlantClass::None && state.front.floor.distance < -core::ClimbLimit2ClickMax )
        {
            setStateOverride(LaraStateId::Unknown12);
            if( getCurrentFrame() >= 0 && getCurrentFrame() <= 9 )
            {
                playAnimation(loader::AnimationId::WALL_SMASH_LEFT, 800);
                return;
            }
            if( getCurrentFrame() >= 10 && getCurrentFrame() <= 21 )
            {
                playAnimation(loader::AnimationId::WALL_SMASH_RIGHT, 815);
                return;
            }

            playAnimation(loader::AnimationId::STAY_SOLID, 185);
        }
    }

    if( state.current.floor.distance > core::ClimbLimit2ClickMin )
    {
        playAnimation(loader::AnimationId::FREE_FALL_FORWARD, 492);
        setStateOverride(LaraStateId::JumpForward);
        setTargetState(LaraStateId::JumpForward);
        setFalling(true);
        setFallSpeed(0);
        return;
    }

    if( state.current.floor.distance >= -core::ClimbLimit2ClickMin && state.current.floor.distance < -core::SteppableHeight )
    {
        if( getCurrentFrame() >= 3 && getCurrentFrame() <= 14 )
        {
            playAnimation(loader::AnimationId::RUN_UP_STEP_LEFT, 837);
        }
        else
        {
            playAnimation(loader::AnimationId::RUN_UP_STEP_RIGHT, 830);
        }
    }

    if( !tryStartSlide(state) )
    {
        if(state.current.floor.distance > 50)
            state.current.floor.distance = 50;
        placeOnFloor(state);
    }
}

void LaraStateHandler::onBehave5RunBackward(LaraState& state)
{
    setFallSpeed(0);
    setFalling(false);
    state.neededFloorDistanceBottom = loader::HeightLimit;
    state.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
    state.neededCeilingDistance = 0;
    state.frobbelFlags |= LaraState::FrobbelFlag_UnpassableSteepUpslant | LaraState::FrobbelFlag_UnwalkableSteepFloor;
    state.yAngle = getRotation().Y + util::degToAu(180);
    setMovementAngle(state.yAngle);
    state.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
    if( tryStopOnFloor(state) )
        return;

    if( state.current.floor.distance > 200 )
    {
        playAnimation(loader::AnimationId::FREE_FALL_BACK, 1473);
        setTargetState(LaraStateId::FallBackward);
        setStateOverride(LaraStateId::FallBackward);
        setFallSpeed(0);
        setFalling(true);
        return;
    }

    if( checkWallCollision(state) )
    {
        playAnimation(loader::AnimationId::STAY_SOLID, 185);
    }
    placeOnFloor(state);
}

void LaraStateHandler::onBehave16WalkBackward(LaraState& state)
{
    setFallSpeed(0);
    setFalling(false);
    state.neededFloorDistanceBottom = core::ClimbLimit2ClickMin;
    state.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
    state.neededCeilingDistance = 0;
    state.yAngle = getRotation().Y + util::degToAu(180);
    setMovementAngle(state.yAngle);
    state.frobbelFlags |= LaraState::FrobbelFlag_UnpassableSteepUpslant | LaraState::FrobbelFlag_UnwalkableSteepFloor;
    state.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
    if( tryStopOnFloor(state) )
        return;

    if( checkWallCollision(state) )
    {
        playAnimation(loader::AnimationId::STAY_SOLID, 185);
    }

    if( state.current.floor.distance > loader::QuarterSectorSize && state.current.floor.distance < core::ClimbLimit2ClickMin )
    {
        if( getCurrentFrame() < 964 || getCurrentFrame() > 993 )
        {
            playAnimation(loader::AnimationId::WALK_DOWN_BACK_LEFT, 899);
        }
        else
        {
            playAnimation(loader::AnimationId::WALK_DOWN_BACK_RIGHT, 930);
        }
    }

    if( !tryStartSlide(state) )
    {
        placeOnFloor(state);
    }
}

void LaraStateHandler::onBehave9FreeFall(LaraState& state)
{
    state.neededFloorDistanceBottom = loader::HeightLimit;
    state.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
    state.neededCeilingDistance = 192;
    state.yAngle = getMovementAngle();
    setFalling(true);
    state.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
    jumpAgainstWall(state);
    if( state.current.floor.distance > 0 )
        return;

    if( applyLandingDamage(state) )
    {
        setTargetState(LaraStateId::Death);
    }
    else
    {
        setTargetState(LaraStateId::Stop);
        setStateOverride(LaraStateId::Stop);
        playAnimation(loader::AnimationId::LANDING_HARD, 358);
    }
    setFallSpeed(0);
    placeOnFloor(state);
    setFalling(false);
}

void LaraStateHandler::onBehave12Unknown(LaraState& state)
{
    state.neededFloorDistanceBottom = core::ClimbLimit2ClickMin;
    state.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
    state.neededCeilingDistance = 0;
    state.yAngle = getMovementAngle();
    state.frobbelFlags |= LaraState::FrobbelFlag_UnpassableSteepUpslant | LaraState::FrobbelFlag_UnwalkableSteepFloor;
    state.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
    applyCollisionFeedback(state);
}

void LaraStateHandler::onBehave3JumpForward(LaraState& state)
{
    state.neededFloorDistanceBottom = loader::HeightLimit;
    state.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
    state.neededCeilingDistance = 192;
    state.yAngle = getRotation().Y;
    setMovementAngle(state.yAngle);
    state.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
    checkJumpWallSmash(state);

    if( state.current.floor.distance > 0 || getFallSpeed().get() <= 0 )
        return;

    if( applyLandingDamage(state) )
    {
        setTargetState(LaraStateId::Death);
    }
    else if( getInputState().zMovement != AxisMovement::Forward || getInputState().moveSlow )
    {
        setTargetState(LaraStateId::Stop);
    }
    else
    {
        setTargetState(LaraStateId::RunForward);
    }

    setFallSpeed(0);
    setFalling(false);
    setHorizontalSpeed(0);
    placeOnFloor(state);
    processAnimCommands();
}

void LaraStateHandler::onBehave28JumpUp(LaraState& state)
{
    state.neededFloorDistanceBottom = loader::HeightLimit;
    state.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
    state.neededCeilingDistance = 192;
    state.yAngle = getRotation().Y;
    state.initHeightInfo(getPosition(), getLevel(), 870); //! @todo MAGICK 870
    if( tryGrabEdge(state) )
        return;

    jumpAgainstWall(state);
    if( getFallSpeed().get() <= 0 || state.current.floor.distance > 0 )
        return;

    if( applyLandingDamage(state) )
        setTargetState(LaraStateId::Death);
    else
        setTargetState(LaraStateId::Stop);
    setFallSpeed(0);
    placeOnFloor(state);
    setFalling(false);
}

void LaraStateHandler::onBehave32SlideBackward(LaraState& state)
{
    setMovementAngle(getRotation().Y + util::degToAu(180));
    commonSlideHandling(state);
}

void LaraStateHandler::jumpAgainstWall(LaraState& state)
{
    applyCollisionFeedback(state);
    if( state.axisCollisions == LaraState::AxisColl_FrontLeftBump )
        addYRotation( makeSpeedValue(910).getScaledExact(getCurrentDeltaTime()) );
    else if( state.axisCollisions == LaraState::AxisColl_FrontRightBump )
        addYRotation( -makeSpeedValue(940).getScaledExact(getCurrentDeltaTime()) );
    else if( state.axisCollisions == LaraState::AxisColl_HeadInCeiling )
    {
        if( getFallSpeed().get() <= 0 )
            setFallSpeed(1);
    }
    else if( state.axisCollisions == LaraState::AxisColl_CeilingTooLow )
    {
        moveXZ(
            -makeSpeedValue(100).getScaledExact(getCurrentDeltaTime()) * std::sin(util::auToRad(getRotation().Y)),
            -makeSpeedValue(100).getScaledExact(getCurrentDeltaTime()) * std::cos(util::auToRad(getRotation().Y))
        );
        setHorizontalSpeed(0);
        state.current.floor.distance = 0;
        if( getFallSpeed().get() < 0 )
            setFallSpeed(16);
    }
}

void LaraStateHandler::checkJumpWallSmash(LaraState& state)
{
    applyCollisionFeedback(state);

    if( state.axisCollisions == LaraState::AxisColl_None )
        return;

    if( state.axisCollisions == LaraState::AxisColl_InsufficientFrontSpace || state.axisCollisions == LaraState::AxisColl_BumpHead )
    {
        setTargetState(LaraStateId::FreeFall);
        //! @todo Check values
        //! @bug Time is too short to properly apply collision momentum!
        dampenHorizontalSpeed(4, 5);
        setMovementAngle(getMovementAngle() - util::degToAu(180));
        playAnimation(loader::AnimationId::SMASH_JUMP, 481);
        setStateOverride(LaraStateId::FreeFall);
        if( getFallSpeed().get() <= 0 )
            setFallSpeed(1);
        return;
    }

    if( state.axisCollisions == LaraState::AxisColl_FrontLeftBump )
    {
        addYRotation( makeSpeedValue(910).getScaledExact(getCurrentDeltaTime()) );
        return;
    }
    else if( state.axisCollisions == LaraState::AxisColl_FrontRightBump )
    {
        addYRotation( -makeSpeedValue(910).getScaledExact(getCurrentDeltaTime()) );
        return;
    }

    if( state.axisCollisions == LaraState::AxisColl_CeilingTooLow )
    {
        moveXZ(
            -makeSpeedValue(100).getScaledExact(getCurrentDeltaTime()) * std::sin(util::auToRad(state.yAngle)),
            -makeSpeedValue(100).getScaledExact(getCurrentDeltaTime()) * std::cos(util::auToRad(state.yAngle))
        );
        setHorizontalSpeed(0);
        state.current.floor.distance = 0;
        if( getFallSpeed().get() <= 0 )
            setFallSpeed(16);
    }

    if( state.axisCollisions == LaraState::AxisColl_HeadInCeiling && getFallSpeed().get() <= 0 )
        setFallSpeed(1);
}

void LaraStateHandler::animateNode(irr::scene::ISceneNode* node, irr::u32 timeMs)
{
    BOOST_ASSERT(m_lara == node);

    if( m_lastFrameTime < 0 )
        m_lastFrameTime = m_currentFrameTime = timeMs;

    if( m_lastFrameTime == timeMs )
        return;

    m_currentFrameTime = timeMs;

    handleLaraStateOnLand();

    m_lastFrameTime = m_currentFrameTime;
}

void LaraStateHandler::processAnimCommands()
{
    if( m_dispatcher->handleTRTransitions() || m_lastAnimFrame != getCurrentFrame() )
    {
        clearStateOverride();
        m_lastAnimFrame = getCurrentFrame();
    }

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
                m_position += {makeSpeedValue(cmd[0]).getScaledExact(getCurrentDeltaTime()), makeSpeedValue(cmd[1]).getScaledExact(getCurrentDeltaTime()), makeSpeedValue(cmd[2]).getScaledExact(getCurrentDeltaTime())};
                cmd += 3;
                break;
            case AnimCommandOpcode::SetVelocity:
                setFallSpeed(m_fallSpeedOverride == 0 ? cmd[0] : m_fallSpeedOverride);
                m_fallSpeedOverride = 0;
                setHorizontalSpeed(cmd[1]);
                setFalling(true);
                cmd += 2;
                break;
            case AnimCommandOpcode::EmptyHands:
                setHandStatus(0);
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
                    if( cmd[1] == 0 )
                        m_rotation.Y += util::degToAu(180);
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

    m_position.X += std::sin(util::auToRad(getMovementAngle())) * m_horizontalSpeed.getScaledExact(getCurrentDeltaTime());
    m_position.Y += getFallSpeed().getScaledExact(getCurrentDeltaTime());
    m_position.Z += std::cos(util::auToRad(getMovementAngle())) * m_horizontalSpeed.getScaledExact(getCurrentDeltaTime());
    m_lara->setPosition(m_position.toIrrlicht());

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

    m_lara->updateAbsolutePosition();
}

bool LaraStateHandler::tryStopOnFloor(LaraState& state)
{
    if( state.axisCollisions != LaraState::AxisColl_HeadInCeiling && state.axisCollisions != LaraState::AxisColl_CeilingTooLow )
        return false;

    setPosition(state.position);

    setTargetState(LaraStateId::Stop);
    setStateOverride(LaraStateId::Stop);
    playAnimation(loader::AnimationId::STAY_SOLID, 185);
    setHorizontalSpeed(0);
    setFallSpeed(0);
    setFalling(false);
    return true;
}

bool LaraStateHandler::tryClimb(LaraState& state)
{
    if( state.axisCollisions != LaraState::AxisColl_InsufficientFrontSpace || !getInputState().jump || getHandStatus() != 0 )
        return false;

    const auto floorGradient = std::abs(state.frontLeft.floor.distance - state.frontRight.floor.distance);
    if( floorGradient >= core::MaxGrabableGradient )
        return false;

    int alignedRotation;
    //! @todo MAGICK +/- 30 degrees
    if( getRotation().Y >= util::degToAu(-30) && getRotation().Y <= util::degToAu(30) )
        alignedRotation = util::degToAu(0);
    else if( getRotation().Y >= util::degToAu(60) && getRotation().Y <= util::degToAu(120) )
        alignedRotation = util::degToAu(90);
    else if( getRotation().Y >= util::degToAu(150) && getRotation().Y <= util::degToAu(210) )
        alignedRotation = util::degToAu(180);
    else if( getRotation().Y >= util::degToAu(240) && getRotation().Y <= util::degToAu(300) )
        alignedRotation = util::degToAu(270);
    else
        return false;

    const auto climbHeight = state.front.floor.distance;
    if( climbHeight >= -core::ClimbLimit2ClickMax && climbHeight <= -core::ClimbLimit2ClickMin )
    {
        if( climbHeight < state.front.ceiling.distance
            || state.frontLeft.floor.distance < state.frontLeft.ceiling.distance
            || state.frontRight.floor.distance < state.frontRight.ceiling.distance )
            return false;

        setTargetState(LaraStateId::Stop);
        setStateOverride(LaraStateId::Climbing);
        playAnimation(loader::AnimationId::CLIMB_2CLICK, 759);
        moveY(2 * loader::QuarterSectorSize + climbHeight);
        setHandStatus(1);
    }
    else if( climbHeight >= -core::ClimbLimit3ClickMax && climbHeight <= -core::ClimbLimit2ClickMax )
    {
        if( state.front.floor.distance < state.front.ceiling.distance
            || state.frontLeft.floor.distance < state.frontLeft.ceiling.distance
            || state.frontRight.floor.distance < state.frontRight.ceiling.distance )
            return false;

        setTargetState(LaraStateId::Stop);
        setStateOverride(LaraStateId::Climbing);
        playAnimation(loader::AnimationId::CLIMB_3CLICK, 614);
        moveY(3 * loader::QuarterSectorSize + climbHeight);
        setHandStatus(1);
    }
    else
    {
        if( climbHeight < -core::JumpReachableHeight || climbHeight > -core::ClimbLimit3ClickMax )
            return false;

        setTargetState(LaraStateId::JumpUp);
        setStateOverride(LaraStateId::Stop);
        playAnimation(loader::AnimationId::STAY_SOLID, 185);
        setFallSpeedOverride( -static_cast<int>(std::sqrt(-12 * (climbHeight + 800) + 3)) );
        processAnimCommands();
    }

    setYRotation(alignedRotation);
    applyCollisionFeedback(state);

    return true;
}

void LaraStateHandler::applyCollisionFeedback(LaraState& state)
{
    setPosition(loader::ExactTRCoordinates(getPosition() + state.collisionFeedback));
    state.collisionFeedback = {0,0,0};
}

bool LaraStateHandler::checkWallCollision(LaraState& state)
{
    if( state.axisCollisions == LaraState::AxisColl_InsufficientFrontSpace || state.axisCollisions == LaraState::AxisColl_BumpHead )
    {
        applyCollisionFeedback(state);
        setTargetState(LaraStateId::Stop);
        setStateOverride(LaraStateId::Stop);
        setFalling(false);
        setHorizontalSpeed(0);
        return true;
    }

    if( state.axisCollisions == LaraState::AxisColl_FrontLeftBump )
    {
        applyCollisionFeedback(state);
        addYRotation( makeSpeedValue(910).getScaledExact(getCurrentDeltaTime()) );
    }
    else if( state.axisCollisions == LaraState::AxisColl_FrontRightBump )
    {
        applyCollisionFeedback(state);
        addYRotation( -makeSpeedValue(910).getScaledExact(getCurrentDeltaTime()) );
    }

    return false;
}

bool LaraStateHandler::tryStartSlide(LaraState& state)
{
    auto slantX = std::abs(state.floorSlantX);
    auto slantZ = std::abs(state.floorSlantZ);
    if( slantX <= 2 && slantZ <= 2 )
        return false;

    auto targetAngle = util::degToAu(0);
    if( state.floorSlantX < -2 )
        targetAngle = util::degToAu(90);
    else if( state.floorSlantX > 2 )
        targetAngle = util::degToAu(-90);

    if( state.floorSlantZ > std::max(2, slantX) )
        targetAngle = util::degToAu(180);
    else if( state.floorSlantZ < std::min(-2, -slantX) )
        targetAngle = util::degToAu(0);

    int16_t dy = std::abs(targetAngle - getRotation().Y);
    applyCollisionFeedback(state);
    if( dy > util::degToAu(90) || dy < util::degToAu(-90) )
    {
        if( m_dispatcher->getCurrentState() != static_cast<uint16_t>(LaraStateId::SlideBackward) || targetAngle != getCurrentSlideAngle() )
        {
            playAnimation(loader::AnimationId::START_SLIDE_BACKWARD, 1677);
            setTargetState(LaraStateId::SlideBackward);
            setStateOverride(LaraStateId::SlideBackward);
            setMovementAngle(targetAngle);
            setCurrentSlideAngle( targetAngle );
            setYRotation(targetAngle + util::degToAu(180));
        }
    }
    else if( m_dispatcher->getCurrentState() != static_cast<uint16_t>(LaraStateId::SlideForward) || targetAngle != getCurrentSlideAngle() )
    {
        playAnimation(loader::AnimationId::SLIDE_FORWARD, 1133);
        setTargetState(LaraStateId::SlideForward);
        setStateOverride(LaraStateId::SlideForward);
        setMovementAngle(targetAngle);
        setCurrentSlideAngle( targetAngle );
        setYRotation(targetAngle);
    }
    return true;
}

bool LaraStateHandler::tryGrabEdge(LaraState& state)
{
    if( state.axisCollisions != LaraState::AxisColl_InsufficientFrontSpace || !getInputState().action || getHandStatus() != 0 )
        return false;

    const auto floorGradient = std::abs(state.frontLeft.floor.distance - state.frontRight.floor.distance);
    if( floorGradient >= core::MaxGrabableGradient )
        return false;

    if( state.front.ceiling.distance > 0 || state.current.ceiling.distance > -core::ClimbLimit2ClickMin )
        return false;

    getLara()->updateAbsolutePosition();

    const auto spaceToReach = state.front.ceiling.distance - (loader::TRCoordinates(getLara()->getTransformedBoundingBox().MaxEdge).Y - getPosition().Y);

    if( spaceToReach < 0 && spaceToReach + getFallSpeed().getExact() < 0 )
        return false;
    if( spaceToReach > 0 && spaceToReach + getFallSpeed().getExact() > 0 )
        return false;

    int16_t rot = static_cast<int16_t>(getRotation().Y);
    if( std::abs(rot) <= util::degToAu(35) )
        rot = 0;
    else if( rot >= util::degToAu(90 - 35) && rot <= util::degToAu(90 + 35) )
        rot = util::degToAu(90);
    else if( std::abs(rot) >= util::degToAu(180 - 35) )
        rot = util::degToAu(180);
    else if( -rot >= util::degToAu(90 - 35) && -rot <= util::degToAu(90 + 35) )
        rot = util::degToAu(-90);
    else
        return false;

    setTargetState(LaraStateId::Hang);
    setStateOverride(LaraStateId::Hang);
    playAnimation(loader::AnimationId::HANG_IDLE, 1505);

    moveY(spaceToReach);
    applyCollisionFeedback(state);
    setHorizontalSpeed(0);
    setFallSpeed(0);
    setFalling(false);
    setHandStatus(1);
    setYRotation(rot);

    return true;
}

void LaraStateHandler::handleTriggers(const uint16_t* floorData, bool isDoppelganger)
{
    if( floorData == nullptr )
        return;

    if( loader::extractFDFunction(*floorData) == loader::FDFunction::Death )
    {
        if( !isDoppelganger )
        {
            if( irr::core::equals(getPosition().Y, m_floorHeight, 1) )
            {
                //! @todo kill Lara
            }
        }

        if( *floorData & 0x8000 )
            return;

        ++floorData;
    }

    //! @todo Implement the rest
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

int LaraStateHandler::getRelativeHeightAtDirection(int16_t angle, int dist) const
{
    auto pos = getPosition();
    pos.X += std::sin(util::auToRad(angle)) * dist;
    pos.Y -= core::ScalpHeight;
    pos.Z += std::cos(util::auToRad(angle)) * dist;

    auto sector = getLevel().findSectorForPosition(pos, getLevel().m_camera->getCurrentRoom());
    BOOST_ASSERT(sector != nullptr);

    HeightInfo h = HeightInfo::fromFloor(sector, pos, getLevel().m_camera);

    if( h.distance != -loader::HeightLimit )
        h.distance -= getPosition().Y;

    return h.distance;
}

void LaraStateHandler::commonJumpHandling(LaraState& state)
{
    state.neededFloorDistanceBottom = loader::HeightLimit;
    state.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
    state.neededCeilingDistance = 192;
    state.yAngle = getMovementAngle();
    state.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
    checkJumpWallSmash(state);
    if( getFallSpeed().get() <= 0 || state.current.floor.distance > 0 )
        return;

    if( applyLandingDamage(state) )
        setTargetState(LaraStateId::Death);
    else
        setTargetState(LaraStateId::Stop);
    setFallSpeed(0);
    placeOnFloor(state);
    setFalling(false);
}

void LaraStateHandler::commonSlideHandling(LaraState& state)
{
    state.neededFloorDistanceBottom = loader::HeightLimit;
    state.neededFloorDistanceTop = -loader::QuarterSectorSize * 2;
    state.neededCeilingDistance = 0;
    state.yAngle = getMovementAngle();
    state.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);

    if( tryStopOnFloor(state) )
        return;

    checkWallCollision(state);
    if( state.current.floor.distance <= 200 )
    {
        tryStartSlide(state);
        placeOnFloor(state);
        const auto absSlantX = std::abs(state.floorSlantX);
        const auto absSlantZ = std::abs(state.floorSlantZ);
        if( absSlantX <= 2 && absSlantZ <= 2 )
        {
            setTargetState(LaraStateId::Stop);
        }
    }
    else
    {
        if( getCurrentState() == LaraStateId::SlideForward )
        {
            playAnimation(loader::AnimationId::FREE_FALL_FORWARD, 492);
            setTargetState(LaraStateId::JumpForward);
            setStateOverride(LaraStateId::JumpForward);
        }
        else
        {
            playAnimation(loader::AnimationId::FREE_FALL_BACK, 1473);
            setTargetState(LaraStateId::FallBackward);
            setStateOverride(LaraStateId::FallBackward);
        }

        setFallSpeed(0);
        setFalling(true);
    }
}

bool LaraStateHandler::applyLandingDamage(LaraState& /*state*/)
{
    auto sector = getLevel().findSectorForPosition(getPosition(), getLevel().m_camera->getCurrentRoom());
    HeightInfo h = HeightInfo::fromFloor(sector, getPosition() - loader::TRCoordinates{0, core::ScalpHeight, 0}, getLevel().m_camera);
    setFloorHeight(h.distance);
    handleTriggers(h.lastTriggerOrKill, false);
    auto damageSpeed = getFallSpeed().get() - 140;
    if( damageSpeed <= 0 )
        return false;

    static constexpr int DeathSpeedLimit = 14;

    if( damageSpeed <= DeathSpeedLimit )
        setHealth(getHealth() - 1000 * damageSpeed * damageSpeed / (DeathSpeedLimit * DeathSpeedLimit));
    else
        setHealth(-1);
    return getHealth() <= 0;
}

void LaraStateHandler::onInput52SwandiveBegin(LaraState& state)
{
    state.frobbelFlags &= ~LaraState::FrobbelFlag10;
    state.frobbelFlags |= LaraState::FrobbelFlag08;
    if( getFallSpeed().get() > core::FreeFallSpeedThreshold )
        setTargetState(LaraStateId::SwandiveEnd);
}

void LaraStateHandler::onBehave52SwandiveBegin(LaraState& state)
{
    state.neededFloorDistanceBottom = loader::HeightLimit;
    state.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
    state.neededCeilingDistance = 192;
    state.yAngle = getRotation().Y;
    setMovementAngle(state.yAngle);
    state.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
    checkJumpWallSmash(state);
    if( state.current.floor.distance > 0 || getFallSpeed().get() <= 0 )
        return;

    setTargetState(LaraStateId::Stop);
    setFallSpeed(0);
    setFalling(false);
    placeOnFloor(state);
}

void LaraStateHandler::onInput53SwandiveEnd(LaraState& state)
{
    state.frobbelFlags &= ~LaraState::FrobbelFlag10;
    state.frobbelFlags |= LaraState::FrobbelFlag08;
    dampenHorizontalSpeed(5, 100);
}

void LaraStateHandler::onBehave53SwandiveEnd(LaraState& state)
{
    state.neededFloorDistanceBottom = loader::HeightLimit;
    state.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
    state.neededCeilingDistance = 192;
    state.yAngle = getRotation().Y;
    setMovementAngle(state.yAngle);
    state.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
    checkJumpWallSmash(state);
    if( state.current.floor.distance > 0 || getFallSpeed().get() <= 0 )
        return;

    if( getFallSpeed().get() <= 133 )
        setTargetState(LaraStateId::Stop);
    else
        setTargetState(LaraStateId::Death);

    setFallSpeed(0);
    setFalling(false);
    placeOnFloor(state);
}

void LaraStateHandler::onBehave23RollBackward(LaraState& state)
{
    setFalling(false);
    setFallSpeed(0);
    state.yAngle = getRotation().Y + util::degToAu(180);
    setMovementAngle(state.yAngle);
    state.frobbelFlags |= LaraState::FrobbelFlag_UnpassableSteepUpslant;
    state.neededFloorDistanceBottom = loader::HeightLimit;
    state.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
    state.neededCeilingDistance = 0;
    state.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);

    if( tryStopOnFloor(state) || tryStartSlide(state) )
        return;

    if( state.current.floor.distance <= 200 )
    {
        applyCollisionFeedback(state);
        placeOnFloor(state);
        return;
    }

    playAnimation(loader::AnimationId::FREE_FALL_BACK, 1473);
    setTargetState(LaraStateId::FallBackward);
    setStateOverride(LaraStateId::FallBackward);
    setFallSpeed(0);
    setFalling(true);
}

void LaraStateHandler::onBehave45RollForward(LaraState& state)
{
    setFalling(false);
    setFallSpeed(0);
    state.yAngle = getRotation().Y;
    setMovementAngle(state.yAngle);
    state.frobbelFlags |= LaraState::FrobbelFlag_UnpassableSteepUpslant;
    state.neededFloorDistanceBottom = loader::HeightLimit;
    state.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
    state.neededCeilingDistance = 0;
    state.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);

    if( tryStopOnFloor(state) || tryStartSlide(state) )
        return;

    if( state.current.floor.distance <= 200 )
    {
        applyCollisionFeedback(state);
        placeOnFloor(state);
        return;
    }

    playAnimation(loader::AnimationId::FREE_FALL_FORWARD, 492);
    setTargetState(LaraStateId::JumpForward);
    setStateOverride(LaraStateId::JumpForward);
    setFallSpeed(0);
    setFalling(true);
}

void LaraStateHandler::onInput26JumpLeft(LaraState& /*state*/)
{
    if( getFallSpeed().get() > core::FreeFallSpeedThreshold )
        setTargetState(LaraStateId::FreeFall);
}

void LaraStateHandler::onBehave26JumpLeft(LaraState& state)
{
    setMovementAngle(getRotation().Y + util::degToAu(90));
    commonJumpHandling(state);
}

void LaraStateHandler::onInput27JumpRight(LaraState& /*state*/)
{
    if( getFallSpeed().get() > core::FreeFallSpeedThreshold )
        setTargetState(LaraStateId::FreeFall);
}

void LaraStateHandler::onBehave27JumpRight(LaraState& state)
{
    setMovementAngle(getRotation().Y - util::degToAu(90));
    commonJumpHandling(state);
}

void LaraStateHandler::onInput11Reach(LaraState& /*state*/)
{
    if( getFallSpeed().get() > core::FreeFallSpeedThreshold )
        setTargetState(LaraStateId::FreeFall);
}

void LaraStateHandler::onBehave11Reach(LaraState& state)
{
    setFalling(true);
    state.yAngle = getRotation().Y;
    setMovementAngle(state.yAngle);
    state.neededFloorDistanceBottom = loader::HeightLimit;
    state.neededFloorDistanceTop = 0;
    state.neededCeilingDistance = 192;
    state.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);

    if( tryReach(state) )
        return;

    jumpAgainstWall(state);
    if( getFallSpeed().get() <= 0 || state.current.floor.distance > 0 )
        return;

    if( applyLandingDamage(state) )
        setTargetState(LaraStateId::Death);
    else
        setTargetState(LaraStateId::Stop);

    setFallSpeed(0);
    setFalling(false);
    placeOnFloor(state);
}

bool LaraStateHandler::tryReach(LaraState& state)
{
    if( state.axisCollisions != LaraState::AxisColl_InsufficientFrontSpace || !getInputState().action || getHandStatus() != 0 )
        return false;

    if( std::abs(state.frontLeft.floor.distance - state.frontRight.floor.distance) >= core::MaxGrabableGradient )
        return false;

    if( state.front.ceiling.distance > 0 || state.current.ceiling.distance > -core::ClimbLimit2ClickMin || state.current.floor.distance < 200 )
        return false;

    getLara()->updateAbsolutePosition();

    const auto spaceToReach = state.front.floor.distance - (loader::TRCoordinates(getLara()->getTransformedBoundingBox().MaxEdge).Y - getPosition().Y);
    BOOST_LOG_TRIVIAL(debug) << "spaceToReach=" << spaceToReach << ", fallSpeed=" << getFallSpeed().getExact();
    if( spaceToReach < 0 && spaceToReach + getFallSpeed().getExact() < 0 )
        return false;
    if( spaceToReach > 0 && spaceToReach + getFallSpeed().getExact() > 0 )
        return false;

    int16_t rot = static_cast<int16_t>(getRotation().Y);
    if( std::abs(rot) <= util::degToAu(35) )
        rot = 0;
    else if( rot >= util::degToAu(90 - 35) && rot <= util::degToAu(90 + 35) )
        rot = util::degToAu(90);
    else if( std::abs(rot) >= util::degToAu(180 - 35) )
        rot = util::degToAu(180);
    else if( -rot >= util::degToAu(90 - 35) && -rot <= util::degToAu(90 + 35) )
        rot = util::degToAu(-90);
    else
        return false;

    if( canClimbOnto(rot) )
        playAnimation(loader::AnimationId::OSCILLATE_HANG_ON, 3974);
    else
        playAnimation(loader::AnimationId::HANG_IDLE, 1493);

    setStateOverride(LaraStateId::Hang);
    setTargetState(LaraStateId::Hang);
    moveY(spaceToReach);
    setHorizontalSpeed(0);
    applyCollisionFeedback(state);
    setYRotation(rot);
    setFalling(false);
    setFallSpeed(0);
    setHandStatus(1);
    return true;
}

bool LaraStateHandler::canClimbOnto(int16_t angle) const
{
    auto pos = getPosition();
    if( angle == 0 )
        pos.Z += 256;
    else if( angle == util::degToAu(90) )
        pos.X += 256;
    else if( angle == util::degToAu(180) )
        pos.Z -= 256;
    else if( angle == util::degToAu(-90) )
        pos.X -= 256;

    auto sector = getLevel().findSectorForPosition(pos, getLevel().m_camera->getCurrentRoom());
    HeightInfo floor = HeightInfo::fromFloor(sector, pos, getLevel().m_camera);
    HeightInfo ceil = HeightInfo::fromCeiling(sector, pos, getLevel().m_camera);
    return floor.distance != -loader::HeightLimit && floor.distance - pos.Y > 0 && ceil.distance - pos.Y < -400;
}

void LaraStateHandler::onBehaveStanding(LaraState& state)
{
    setFallSpeed(0);
    setFalling(false);
    state.yAngle = getRotation().Y;
    setMovementAngle(state.yAngle);
    state.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
    state.neededFloorDistanceBottom = core::ClimbLimit2ClickMin;
    state.neededCeilingDistance = 0;
    state.frobbelFlags |= LaraState::FrobbelFlag_UnpassableSteepUpslant | LaraState::FrobbelFlag_UnwalkableSteepFloor;
    state.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
    if( tryStopOnFloor(state) )
        return;

    if( state.current.floor.distance <= 100 )
    {
        if( !tryStartSlide(state) )
        {
            applyCollisionFeedback(state);
            placeOnFloor(state);
        }
    }
    else
    {
        playAnimation(loader::AnimationId::FREE_FALL_FORWARD, 492);
        setTargetState(LaraStateId::JumpForward);
        setFallSpeed(0);
        setFalling(true);
    }
}
