#include "abstractstatehandler.h"

#include "loader/larastatehandler.h"
#include "larastate.h"
#include "core/magic.h"
#include "loader/level.h"
#include "loader/trcamerascenenodeanimator.h"

using LaraStateId = loader::LaraStateId;

class StateHandler_Standing : public AbstractStateHandler
{
protected:
    explicit StateHandler_Standing(LaraStateHandler& lara)
        : AbstractStateHandler(lara)
    {
    }

public:
    std::unique_ptr<AbstractStateHandler> postprocessFrame(LaraState& state) override final
    {
        setFallSpeed(0);
        setFalling(false);
        state.yAngle = static_cast<int16_t>(getRotation().Y);
        setMovementAngle(state.yAngle);
        state.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
        state.neededFloorDistanceBottom = core::ClimbLimit2ClickMin;
        state.neededCeilingDistance = 0;
        state.frobbelFlags |= LaraState::FrobbelFlag_UnpassableSteepUpslant | LaraState::FrobbelFlag_UnwalkableSteepFloor;
        state.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);

        auto nextHandler = tryStopOnFloor(state);
        if( nextHandler )
            return nextHandler;

        if( state.current.floor.distance <= 100 )
        {
            if( !tryStartSlide(state, nextHandler) )
            {
                applyCollisionFeedback(state);
                placeOnFloor(state);
            }
            return nextHandler;
        }

        playAnimation(loader::AnimationId::FREE_FALL_FORWARD, 492);
        setTargetState(LaraStateId::JumpForward);
        setFallSpeed(0);
        setFalling(true);
        return nullptr;
    }
};

class StateHandler_0 final : public AbstractStateHandler
{
public:

    explicit StateHandler_0(LaraStateHandler& lara)
        : AbstractStateHandler(lara)
    {
    }

    std::unique_ptr<AbstractStateHandler> handleInputImpl(LaraState& /*state*/) override
    {
        if( getHealth() <= 0 )
        {
            setTargetState(LaraStateId::Stop);
            return nullptr;
        }

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

        return nullptr;
    }

    void animateImpl(LaraState& /*state*/, int /*deltaTimeMs*/) override
    {
        if( getInputState().xMovement == AxisMovement::Left )
            subYRotationSpeed(409, -728);
        else if( getInputState().xMovement == AxisMovement::Right )
            addYRotationSpeed(409, 728);
    }

    std::unique_ptr<AbstractStateHandler> postprocessFrame(LaraState& state) override
    {
        setFallSpeed(0);
        setFalling(false);
        state.yAngle = static_cast<int16_t>(getRotation().Y);
        setMovementAngle(state.yAngle);
        state.neededFloorDistanceBottom = core::ClimbLimit2ClickMin;
        state.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
        state.neededCeilingDistance = 0;
        state.frobbelFlags |= LaraState::FrobbelFlag_UnpassableSteepUpslant | LaraState::FrobbelFlag_UnwalkableSteepFloor | LaraState::FrobbelFlag_UnwalkableDeadlyFloor;
        state.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);

        auto nextHandler = tryStopOnFloor(state);
        if( nextHandler )
            return nextHandler;
        nextHandler = tryClimb(state);
        if( nextHandler )
            return nextHandler;

        nextHandler = checkWallCollision(state);
        if( nextHandler != nullptr )
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
            nextHandler = createWithRetainedAnimation(LaraStateId::JumpForward);
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

        if( !tryStartSlide(state, nextHandler) )
        {
            placeOnFloor(state);
        }

        return nextHandler;
    }

    loader::LaraStateId getId() const noexcept override
    {
        return LaraStateId::WalkForward;
    }
};

class StateHandler_1 final : public AbstractStateHandler
{
public:
    explicit StateHandler_1(LaraStateHandler& lara)
        : AbstractStateHandler(lara)
    {
    }

    std::unique_ptr<AbstractStateHandler> handleInputImpl(LaraState& /*state*/) override
    {
        if( getHealth() <= 0 )
        {
            setTargetState(LaraStateId::Death);
            return nullptr;
        }

        if( getInputState().roll )
        {
            playAnimation(loader::AnimationId::ROLL_BEGIN, 3857);
            setTargetState(LaraStateId::Stop);
            return createWithRetainedAnimation(LaraStateId::RollForward);
        }

        if( getInputState().jump && !isFalling() )
        {
            setTargetState(LaraStateId::JumpForward);
            return nullptr;
        }

        if( getInputState().zMovement != AxisMovement::Forward )
        {
            setTargetState(LaraStateId::Stop);
            return nullptr;
        }

        if( getInputState().moveSlow )
            setTargetState(LaraStateId::WalkForward);
        else
            setTargetState(LaraStateId::RunForward);

        return nullptr;
    }

    void animateImpl(LaraState& /*state*/, int deltaTimeMs) override
    {
        if( getInputState().xMovement == AxisMovement::Left )
        {
            subYRotationSpeed(409, -1456);
            setZRotationExact(std::max(-2002.f, getRotation().Z - makeSpeedValue(273).getScaledExact(deltaTimeMs)));
        }
        else if( getInputState().xMovement == AxisMovement::Right )
        {
            addYRotationSpeed(409, 1456);
            setZRotationExact(std::min(2002.f, getRotation().Z + makeSpeedValue(273).getScaledExact(deltaTimeMs)));
        }
    }

    std::unique_ptr<AbstractStateHandler> postprocessFrame(LaraState& state) override
    {
        state.yAngle = static_cast<int16_t>(getRotation().Y);
        setMovementAngle(state.yAngle);
        state.neededFloorDistanceBottom = loader::HeightLimit;
        state.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
        state.neededCeilingDistance = 0;
        state.frobbelFlags |= LaraState::FrobbelFlag_UnpassableSteepUpslant;
        state.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);

        auto nextHandler = tryStopOnFloor(state);
        if( nextHandler )
            return nextHandler;
        nextHandler = tryClimb(state);
        if( nextHandler )
            return nextHandler;

        nextHandler = checkWallCollision(state);
        if( nextHandler != nullptr )
        {
            setZRotation(0);
            if( state.front.floor.slantClass == SlantClass::None && state.front.floor.distance < -core::ClimbLimit2ClickMax )
            {
                nextHandler = createWithRetainedAnimation(LaraStateId::Unknown12);
                if( getCurrentFrame() >= 0 && getCurrentFrame() <= 9 )
                {
                    playAnimation(loader::AnimationId::WALL_SMASH_LEFT, 800);
                    return nextHandler;
                }
                if( getCurrentFrame() >= 10 && getCurrentFrame() <= 21 )
                {
                    playAnimation(loader::AnimationId::WALL_SMASH_RIGHT, 815);
                    return nextHandler;
                }

                playAnimation(loader::AnimationId::STAY_SOLID, 185);
            }
        }

        if( state.current.floor.distance > core::ClimbLimit2ClickMin )
        {
            playAnimation(loader::AnimationId::FREE_FALL_FORWARD, 492);
            setTargetState(LaraStateId::JumpForward);
            setFalling(true);
            setFallSpeed(0);
            return createWithRetainedAnimation(LaraStateId::JumpForward);
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

        if( !tryStartSlide(state, nextHandler) )
        {
            if( state.current.floor.distance > 50 )
                state.current.floor.distance = 50;
            placeOnFloor(state);
        }

        return nextHandler;
    }

    loader::LaraStateId getId() const noexcept override
    {
        return LaraStateId::RunForward;
    }
};

class StateHandler_2 final : public StateHandler_Standing
{
public:
    explicit StateHandler_2(LaraStateHandler& lara)
        : StateHandler_Standing(lara)
    {
    }

    std::unique_ptr<AbstractStateHandler> handleInputImpl(LaraState& state) override
    {
        //! @todo HACK
        setHandStatus(0);

        if( getHealth() <= 0 )
        {
            setTargetState(LaraStateId::Death);
            return nullptr;
        }

        if( getInputState().roll )
        {
            playAnimation(loader::AnimationId::ROLL_BEGIN);
            setTargetState(LaraStateId::Stop);
            return createWithRetainedAnimation(LaraStateId::RollForward);
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
                createWithRetainedAnimation(LaraStateId::WalkForward)->handleInputImpl(state);
            else
                createWithRetainedAnimation(LaraStateId::RunForward)->handleInputImpl(state);
        }
        else if( getInputState().zMovement == AxisMovement::Backward )
        {
            if( getInputState().moveSlow )
                createWithRetainedAnimation(LaraStateId::WalkBackward)->handleInputImpl(state);
            else
                setTargetState(LaraStateId::RunBack);
        }

        return nullptr;
    }

    void animateImpl(LaraState& /*state*/, int /*deltaTimeMs*/) override
    {
    }

    loader::LaraStateId getId() const noexcept override
    {
        return LaraStateId::Stop;
    }
};

class StateHandler_3 final : public AbstractStateHandler
{
public:
    explicit StateHandler_3(LaraStateHandler& lara)
        : AbstractStateHandler(lara)
    {
    }

    std::unique_ptr<AbstractStateHandler> handleInputImpl(LaraState& /*state*/) override
    {
        if( getTargetState() == LaraStateId::SwandiveBegin || getTargetState() == LaraStateId::Reach )
            setTargetState(LaraStateId::JumpForward);

        if( getTargetState() == LaraStateId::Death || getTargetState() == LaraStateId::Stop )
            return nullptr;

        if( getInputState().action && getHandStatus() == 0 )
            setTargetState(LaraStateId::Reach);

        if( getInputState().moveSlow && getHandStatus() == 0 )
            setTargetState(LaraStateId::SwandiveBegin);

        if( getFallSpeed().get() > core::FreeFallSpeedThreshold )
            setTargetState(LaraStateId::FreeFall);

        return nullptr;
    }

    void animateImpl(LaraState& /*state*/, int /*deltaTimeMs*/) override
    {
        if( getInputState().xMovement == AxisMovement::Left )
        {
            subYRotationSpeed(409, -546);
        }
        else if( getInputState().xMovement == AxisMovement::Right )
        {
            addYRotationSpeed(409, 546);
        }
    }

    std::unique_ptr<AbstractStateHandler> postprocessFrame(LaraState& state) override
    {
        state.neededFloorDistanceBottom = loader::HeightLimit;
        state.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
        state.neededCeilingDistance = 192;
        state.yAngle = static_cast<int16_t>(getRotation().Y);
        setMovementAngle(state.yAngle);
        state.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
        auto nextHandler = checkJumpWallSmash(state);

        if( state.current.floor.distance > 0 || getFallSpeed().get() <= 0 )
            return nextHandler;

        if( applyLandingDamage() )
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
        auto tmp = getStateHandler().processAnimCommands();
        if( tmp )
            return tmp;

        return nextHandler;
    }

    loader::LaraStateId getId() const noexcept override
    {
        return LaraStateId::JumpForward;
    }
};

class StateHandler_4 final : public StateHandler_Standing
{
public:
    explicit StateHandler_4(LaraStateHandler& lara)
        : StateHandler_Standing(lara)
    {
    }

    std::unique_ptr<AbstractStateHandler> handleInputImpl(LaraState& /*state*/) override
    {
        return nullptr;
    }

    void animateImpl(LaraState& /*state*/, int /*deltaTimeMs*/) override
    {
    }

    loader::LaraStateId getId() const noexcept override
    {
        return LaraStateId::Pose;
    }
};

class StateHandler_5 final : public AbstractStateHandler
{
public:
    explicit StateHandler_5(LaraStateHandler& lara)
        : AbstractStateHandler(lara)
    {
    }

    std::unique_ptr<AbstractStateHandler> handleInputImpl(LaraState& /*state*/) override
    {
        setTargetState(LaraStateId::Stop);
        return nullptr;
    }

    void animateImpl(LaraState& /*state*/, int /*deltaTimeMs*/) override
    {
        if( getInputState().xMovement == AxisMovement::Left )
            subYRotationSpeed(409, -1092);
        else if( getInputState().xMovement == AxisMovement::Right )
            addYRotationSpeed(409, 1092);
    }

    std::unique_ptr<AbstractStateHandler> postprocessFrame(LaraState& state) override
    {
        setFallSpeed(0);
        setFalling(false);
        state.neededFloorDistanceBottom = loader::HeightLimit;
        state.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
        state.neededCeilingDistance = 0;
        state.frobbelFlags |= LaraState::FrobbelFlag_UnpassableSteepUpslant | LaraState::FrobbelFlag_UnwalkableSteepFloor;
        state.yAngle = static_cast<int16_t>(getRotation().Y + util::degToAu(180));
        setMovementAngle(state.yAngle);
        state.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
        if( auto nextHandler = tryStopOnFloor(state) )
            return nextHandler;

        if( state.current.floor.distance > 200 )
        {
            playAnimation(loader::AnimationId::FREE_FALL_BACK, 1473);
            setTargetState(LaraStateId::FallBackward);
            setFallSpeed(0);
            setFalling(true);
            return createWithRetainedAnimation(LaraStateId::FallBackward);
        }

        auto nextHandler = checkWallCollision(state);
        if( nextHandler )
        {
            playAnimation(loader::AnimationId::STAY_SOLID, 185);
        }
        placeOnFloor(state);

        return nextHandler;
    }

    loader::LaraStateId getId() const noexcept override
    {
        return LaraStateId::RunBack;
    }
};

class StateHandler_TurnSlow : public AbstractStateHandler
{
protected:
    explicit StateHandler_TurnSlow(LaraStateHandler& lara)
        : AbstractStateHandler(lara)
    {
    }

public:
    std::unique_ptr<AbstractStateHandler> postprocessFrame(LaraState& state) override final
    {
        setFallSpeed(0);
        setFalling(false);
        state.yAngle = static_cast<int16_t>(getRotation().Y);
        setMovementAngle(state.yAngle);
        state.neededFloorDistanceBottom = core::ClimbLimit2ClickMin;
        state.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
        state.neededCeilingDistance = 0;
        state.frobbelFlags |= LaraState::FrobbelFlag_UnpassableSteepUpslant | LaraState::FrobbelFlag_UnwalkableSteepFloor;
        state.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);

        if( state.current.floor.distance <= 100 )
        {
            std::unique_ptr<AbstractStateHandler> nextHandler = nullptr;
            if( !tryStartSlide(state, nextHandler) )
                placeOnFloor(state);

            return nextHandler;
        }

        playAnimation(loader::AnimationId::FREE_FALL_FORWARD, 492);
        setTargetState(LaraStateId::JumpForward);
        setFallSpeed(0);
        setFalling(true);
        return createWithRetainedAnimation(LaraStateId::JumpForward);
    }
};

class StateHandler_6 final : public StateHandler_TurnSlow
{
public:
    explicit StateHandler_6(LaraStateHandler& lara)
        : StateHandler_TurnSlow(lara)
    {
    }

    std::unique_ptr<AbstractStateHandler> handleInputImpl(LaraState& /*state*/) override
    {
        if( getHealth() <= 0 )
        {
            setTargetState(LaraStateId::Stop);
            return nullptr;
        }

        if( getHandStatus() == 4 )
        {
            setTargetState(LaraStateId::TurnFast);
            return nullptr;
        }

        if( getInputState().zMovement != AxisMovement::Forward )
        {
            if( getInputState().xMovement != AxisMovement::Right )
                setTargetState(LaraStateId::Stop);
            return nullptr;
        }

        if( getInputState().moveSlow )
            setTargetState(LaraStateId::WalkForward);
        else
            setTargetState(LaraStateId::RunForward);

        return nullptr;
    }

    void animateImpl(LaraState& /*state*/, int /*deltaTimeMs*/) override
    {
        addYRotationSpeed(409);
        if( getYRotationSpeed() <= 728 )
            return;

        if( getInputState().moveSlow )
            setYRotationSpeed(728);
        else
            setTargetState(LaraStateId::TurnFast);
    }

    loader::LaraStateId getId() const noexcept override
    {
        return LaraStateId::TurnRightSlow;
    }
};

class StateHandler_7 final : public StateHandler_TurnSlow
{
public:
    explicit StateHandler_7(LaraStateHandler& lara)
        : StateHandler_TurnSlow(lara)
    {
    }

    std::unique_ptr<AbstractStateHandler> handleInputImpl(LaraState& /*state*/) override
    {
        if( getHealth() <= 0 )
        {
            setTargetState(LaraStateId::Stop);
            return nullptr;
        }

        if( getHandStatus() == 4 )
        {
            setTargetState(LaraStateId::TurnFast);
            return nullptr;
        }

        if( getInputState().zMovement != AxisMovement::Forward )
        {
            if( getInputState().xMovement != AxisMovement::Left )
                setTargetState(LaraStateId::Stop);
            return nullptr;
        }

        if( getInputState().moveSlow )
            setTargetState(LaraStateId::WalkForward);
        else
            setTargetState(LaraStateId::RunForward);

        return nullptr;
    }

    void animateImpl(LaraState& /*state*/, int /*deltaTimeMs*/) override
    {
        subYRotationSpeed(409);
        if( getYRotationSpeed() >= -728 )
            return;

        if( getInputState().moveSlow )
            setYRotationSpeed(-728);
        else
            setTargetState(LaraStateId::TurnFast);
    }

    loader::LaraStateId getId() const noexcept override
    {
        return LaraStateId::TurnLeftSlow;
    }
};

class StateHandler_8 final : public AbstractStateHandler
{
public:
    explicit StateHandler_8(LaraStateHandler& lara)
        : AbstractStateHandler(lara)
    {
    }

    std::unique_ptr<AbstractStateHandler> handleInputImpl(LaraState& state) override
    {
        state.frobbelFlags &= ~(LaraState::FrobbelFlag08 | LaraState::FrobbelFlag10);
        return nullptr;
    }

    std::unique_ptr<AbstractStateHandler> postprocessFrame(LaraState& state) override
    {
        state.neededFloorDistanceBottom = core::ClimbLimit2ClickMin;
        state.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
        state.neededCeilingDistance = 0;
        state.collisionRadius = 400;
        state.yAngle = static_cast<int16_t>(getRotation().Y);
        setMovementAngle(state.yAngle);
        applyCollisionFeedback(state);
        placeOnFloor(state);
        state.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
        setHealth(-1);
        //! @todo set air=-1
        return nullptr;
    }

    loader::LaraStateId getId() const noexcept override
    {
        return LaraStateId::Death;
    }

    void animateImpl(LaraState& /*state*/, int /*deltaTimeMs*/) override
    {
    }
};

class StateHandler_9 final : public AbstractStateHandler
{
public:
    explicit StateHandler_9(LaraStateHandler& lara)
        : AbstractStateHandler(lara)
    {
    }

    std::unique_ptr<AbstractStateHandler> handleInputImpl(LaraState& /*state*/) override
    {
        return nullptr;
    }

    void animateImpl(LaraState& /*state*/, int /*deltaTimeMs*/) override
    {
        dampenHorizontalSpeed(5, 100);
        if( getFallSpeed().get() > 154 )
        {
            //! @todo playSound(30)
        }
    }

    std::unique_ptr<AbstractStateHandler> postprocessFrame(LaraState& state) override
    {
        state.neededFloorDistanceBottom = loader::HeightLimit;
        state.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
        state.neededCeilingDistance = 192;
        state.yAngle = getMovementAngle();
        setFalling(true);
        state.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
        jumpAgainstWall(state);
        if( state.current.floor.distance > 0 )
            return nullptr;

        std::unique_ptr<AbstractStateHandler> nextHandler = nullptr;
        if( applyLandingDamage() )
        {
            setTargetState(LaraStateId::Death);
        }
        else
        {
            setTargetState(LaraStateId::Stop);
            nextHandler = createWithRetainedAnimation(LaraStateId::Stop);
            playAnimation(loader::AnimationId::LANDING_HARD, 358);
        }
        setFallSpeed(0);
        placeOnFloor(state);
        setFalling(false);

        return nextHandler;
    }

    loader::LaraStateId getId() const noexcept override
    {
        return LaraStateId::FreeFall;
    }
};

class StateHandler_10 final : public AbstractStateHandler
{
public:
    explicit StateHandler_10(LaraStateHandler& lara)
        : AbstractStateHandler(lara)
    {
    }

    std::unique_ptr<AbstractStateHandler> handleInputImpl(LaraState& state) override
    {
        state.frobbelFlags &= ~(LaraState::FrobbelFlag08 | LaraState::FrobbelFlag10);
        //! @todo Set camera rotation (x,y) = (-10920,0)
        if( getInputState().xMovement == AxisMovement::Left || getInputState().stepMovement == AxisMovement::Left )
            setTargetState(LaraStateId::ShimmyLeft);
        else if( getInputState().xMovement == AxisMovement::Right || getInputState().stepMovement == AxisMovement::Right )
            setTargetState(LaraStateId::ShimmyRight);

        return nullptr;
    }

    std::unique_ptr<AbstractStateHandler> postprocessFrame(LaraState& state) override
    {
        auto nextHandler = commonEdgeHangHandling(state);

        if( getTargetState() != LaraStateId::Hang )
            return nextHandler;

        if( getInputState().zMovement != AxisMovement::Forward )
            return nextHandler;

        const auto frontHeight = state.front.floor.distance;
        const auto frontSpace = frontHeight - state.front.ceiling.distance;
        const auto frontLeftSpace = state.frontLeft.floor.distance - state.frontLeft.ceiling.distance;
        const auto frontRightSpace = state.frontRight.floor.distance - state.frontRight.ceiling.distance;
        if( frontHeight <= -850 || frontHeight >= -650 || frontSpace < 0 || frontLeftSpace < 0 || frontRightSpace < 0 || state.hasStaticMeshCollision )
        {
            return nextHandler;
        }

        if( getInputState().moveSlow )
            setTargetState(LaraStateId::Handstand);
        else
            setTargetState(LaraStateId::Climbing);

        return nextHandler;
    }

    loader::LaraStateId getId() const noexcept override
    {
        return LaraStateId::Hang;
    }

    void animateImpl(LaraState& /*state*/, int /*deltaTimeMs*/) override
    {
    }
};

class StateHandler_11 final : public AbstractStateHandler
{
public:
    explicit StateHandler_11(LaraStateHandler& lara)
        : AbstractStateHandler(lara)
    {
    }

    std::unique_ptr<AbstractStateHandler> handleInputImpl(LaraState& /*state*/) override
    {
        if( getFallSpeed().get() > core::FreeFallSpeedThreshold )
            setTargetState(LaraStateId::FreeFall);
        return nullptr;
    }

    void animateImpl(LaraState& /*state*/, int /*deltaTimeMs*/) override
    {
    }

    std::unique_ptr<AbstractStateHandler> postprocessFrame(LaraState& state) override
    {
        setFalling(true);
        state.yAngle = static_cast<int16_t>(getRotation().Y);
        setMovementAngle(state.yAngle);
        state.neededFloorDistanceBottom = loader::HeightLimit;
        state.neededFloorDistanceTop = 0;
        state.neededCeilingDistance = 192;
        state.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);

        auto nextHandler = tryReach(state);
        if( nextHandler )
            return nextHandler;

        jumpAgainstWall(state);
        if( getFallSpeed().get() <= 0 || state.current.floor.distance > 0 )
            return nextHandler;

        if( applyLandingDamage() )
            setTargetState(LaraStateId::Death);
        else
            setTargetState(LaraStateId::Stop);

        setFallSpeed(0);
        setFalling(false);
        placeOnFloor(state);

        return nextHandler;
    }

    loader::LaraStateId getId() const noexcept override
    {
        return LaraStateId::Reach;
    }
};

class StateHandler_12 final : public AbstractStateHandler
{
public:
    explicit StateHandler_12(LaraStateHandler& lara)
        : AbstractStateHandler(lara)
    {
    }

    std::unique_ptr<AbstractStateHandler> handleInputImpl(LaraState& /*state*/) override
    {
        return nullptr;
    }

    void animateImpl(LaraState& /*state*/, int /*deltaTimeMs*/) override
    {
    }

    std::unique_ptr<AbstractStateHandler> postprocessFrame(LaraState& state) override
    {
        state.neededFloorDistanceBottom = core::ClimbLimit2ClickMin;
        state.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
        state.neededCeilingDistance = 0;
        state.yAngle = getMovementAngle();
        state.frobbelFlags |= LaraState::FrobbelFlag_UnpassableSteepUpslant | LaraState::FrobbelFlag_UnwalkableSteepFloor;
        state.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
        applyCollisionFeedback(state);
        return nullptr;
    }

    loader::LaraStateId getId() const noexcept override
    {
        return LaraStateId::Unknown12;
    }
};

class StateHandler_Underwater : public AbstractStateHandler
{
public:
    explicit StateHandler_Underwater(LaraStateHandler& lara)
        : AbstractStateHandler(lara)
    {
    }

    std::unique_ptr<AbstractStateHandler> postprocessFrame(LaraState& state) override
    {
        state.yAngle = getRotation().Y;
        if(std::abs(getRotation().X) > util::degToAu(90))
            state.yAngle += util::degToAu(180);
        setMovementAngle(state.yAngle);
        state.initHeightInfo(getPosition() + loader::TRCoordinates{ 0, 200, 0 }, getLevel(), 400);

        applyCollisionFeedback(state);

        m_xRotationSpeed = 0;
        m_yRotationSpeed = 0;

        switch(state.axisCollisions)
        {
            case LaraState::AxisColl_FrontLeftBump:
                m_yRotationSpeed = 910;
                break;
            case LaraState::AxisColl_FrontRightBump:
                m_yRotationSpeed = -910;
                break;
            case LaraState::AxisColl_CeilingTooLow:
                setFallSpeed(0);
                return nullptr;
            case LaraState::AxisColl_BumpHead:
                setFallSpeed(0);
                break;
            case LaraState::AxisColl_HeadInCeiling:
                if(static_cast<int16_t>(getRotation().X) > -8190)
                    m_xRotationSpeed = -364; // setXRotation(getRotation().X - 364);
                break;
            case LaraState::AxisColl_InsufficientFrontSpace:
                if(static_cast<int16_t>(getRotation().X) > 6370)
                    m_xRotationSpeed = 364; // setXRotation(getRotation().X + 364);
                else if(static_cast<int>(getRotation().X) < -6370)
                    m_xRotationSpeed = -364; // setXRotation(getRotation().X - 364);
                else
                    setFallSpeed(0);
                break;
            default:
                break;
        }

        if(state.current.floor.distance >= 0)
            return nullptr;

        setPosition(loader::ExactTRCoordinates(getPosition() + loader::TRCoordinates(0, state.current.floor.distance, 0)));
        m_xRotationSpeed = m_xRotationSpeed.get() + 364;

        return nullptr;
    }

protected:
    void handleDiveInput()
    {
        if(getInputState().zMovement == AxisMovement::Forward)
            m_xRotationSpeed = -util::degToAu(2);
        else if(getInputState().zMovement == AxisMovement::Backward)
            m_xRotationSpeed = util::degToAu(2);
        else
            m_xRotationSpeed = 0;
        if(getInputState().xMovement == AxisMovement::Left)
        {
            m_yRotationSpeed = -util::degToAu(6);
            m_zRotationSpeed = -util::degToAu(3);
        }
        else if(getInputState().xMovement == AxisMovement::Right)
        {
            m_yRotationSpeed = util::degToAu(6);
            m_zRotationSpeed = util::degToAu(3);
        }
        else
        {
            m_yRotationSpeed = 0;
            m_zRotationSpeed = 0;
        }
    }
};

class StateHandler_13 final : public StateHandler_Underwater
{
public:
    explicit StateHandler_13(LaraStateHandler& lara)
        : StateHandler_Underwater(lara)
    {
    }

    std::unique_ptr<AbstractStateHandler> handleInputImpl(LaraState& /*state*/) override
    {
        if(getHealth() < 0)
        {
            setTargetState(LaraStateId::WaterDeath);
            return nullptr;
        }

        handleDiveInput();

        if(getInputState().jump)
            setTargetState(LaraStateId::UnderwaterForward);

        return nullptr;
    }

    void animateImpl(LaraState& /*state*/, int deltaTimeMs) override
    {
        setFallSpeedExact(std::max(0.0f, getFallSpeed().getExact() - makeSpeedValue(6).getScaledExact(deltaTimeMs)));
    }

    loader::LaraStateId getId() const noexcept override
    {
        return LaraStateId::UnderwaterStop;
    }
};

class StateHandler_15 final : public AbstractStateHandler
{
public:
    explicit StateHandler_15(LaraStateHandler& lara)
        : AbstractStateHandler(lara)
    {
    }

    std::unique_ptr<AbstractStateHandler> handleInputImpl(LaraState& /*state*/) override
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

        return nullptr;
    }

    void animateImpl(LaraState& /*state*/, int /*deltaTimeMs*/) override
    {
    }

    std::unique_ptr<AbstractStateHandler> postprocessFrame(LaraState& state) override
    {
        setFallSpeed(0);
        setFalling(false);
        state.neededFloorDistanceBottom = loader::HeightLimit;
        state.neededFloorDistanceTop = -loader::HeightLimit;
        state.neededCeilingDistance = 0;
        state.yAngle = static_cast<int16_t>(getRotation().Y);
        state.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);

        if( state.current.ceiling.distance <= -100 )
            return nullptr;

        setTargetState(LaraStateId::Stop);
        playAnimation(loader::AnimationId::STAY_SOLID, 185);
        setHorizontalSpeed(0);
        setPosition(state.position);

        return createWithRetainedAnimation(LaraStateId::Stop);
    }

    loader::LaraStateId getId() const noexcept override
    {
        return LaraStateId::JumpPrepare;
    }
};

class StateHandler_16 final : public AbstractStateHandler
{
public:
    explicit StateHandler_16(LaraStateHandler& lara)
        : AbstractStateHandler(lara)
    {
    }

    std::unique_ptr<AbstractStateHandler> handleInputImpl(LaraState& /*state*/) override
    {
        if( getHealth() <= 0 )
        {
            setTargetState(LaraStateId::Stop);
            return nullptr;
        }

        if( getInputState().zMovement == AxisMovement::Backward && getInputState().moveSlow )
            setTargetState(LaraStateId::WalkBackward);
        else
            setTargetState(LaraStateId::Stop);

        return nullptr;
    }

    void animateImpl(LaraState& /*state*/, int /*deltaTimeMs*/) override
    {
        if( getInputState().xMovement == AxisMovement::Left )
            subYRotationSpeed(409, -728);
        else if( getInputState().xMovement == AxisMovement::Right )
            addYRotationSpeed(409, 728);
    }

    std::unique_ptr<AbstractStateHandler> postprocessFrame(LaraState& state) override
    {
        setFallSpeed(0);
        setFalling(false);
        state.neededFloorDistanceBottom = core::ClimbLimit2ClickMin;
        state.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
        state.neededCeilingDistance = 0;
        state.yAngle = static_cast<int16_t>(getRotation().Y + util::degToAu(180));
        setMovementAngle(state.yAngle);
        state.frobbelFlags |= LaraState::FrobbelFlag_UnpassableSteepUpslant | LaraState::FrobbelFlag_UnwalkableSteepFloor;
        state.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);

        if( auto nextHandler = tryStopOnFloor(state) )
            return nextHandler;

        auto nextHandler = checkWallCollision(state);
        if( nextHandler )
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

        if( !tryStartSlide(state, nextHandler) )
        {
            placeOnFloor(state);
        }

        return nextHandler;
    }

    loader::LaraStateId getId() const noexcept override
    {
        return LaraStateId::WalkBackward;
    }
};

class StateHandler_17 final : public StateHandler_Underwater
{
public:
    explicit StateHandler_17(LaraStateHandler& lara)
        : StateHandler_Underwater(lara)
    {
    }

    std::unique_ptr<AbstractStateHandler> handleInputImpl(LaraState& /*state*/) override
    {
        if(getHealth() < 0)
        {
            setTargetState(LaraStateId::WaterDeath);
            return nullptr;
        }

        handleDiveInput();

        if(!getInputState().jump)
            setTargetState(LaraStateId::UnderwaterInertia);

        return nullptr;
    }

    void animateImpl(LaraState& /*state*/, int deltaTimeMs) override
    {
        setFallSpeedExact(std::min(200.0f, getFallSpeed().getExact() + makeSpeedValue(8).getScaledExact(deltaTimeMs)));
    }

    loader::LaraStateId getId() const noexcept override
    {
        return LaraStateId::UnderwaterForward;
    }
};

class StateHandler_18 final : public StateHandler_Underwater
{
public:
    explicit StateHandler_18(LaraStateHandler& lara)
        : StateHandler_Underwater(lara)
    {
    }

    std::unique_ptr<AbstractStateHandler> handleInputImpl(LaraState& /*state*/) override
    {
        if(getHealth() < 0)
        {
            setTargetState(LaraStateId::WaterDeath);
            return nullptr;
        }

        handleDiveInput();

        if(getInputState().jump)
            setTargetState(LaraStateId::UnderwaterForward);

        if(getFallSpeed().get() <= 133)
            setTargetState(LaraStateId::UnderwaterStop);

        return nullptr;
    }

    void animateImpl(LaraState& /*state*/, int deltaTimeMs) override
    {
        setFallSpeedExact(std::max(0.0f, getFallSpeed().getExact() - makeSpeedValue(6).getScaledExact(deltaTimeMs)));
    }

    loader::LaraStateId getId() const noexcept override
    {
        return LaraStateId::UnderwaterInertia;
    }
};

class StateHandler_19 final : public AbstractStateHandler
{
public:
    explicit StateHandler_19(LaraStateHandler& lara)
        : AbstractStateHandler(lara)
    {
    }

    std::unique_ptr<AbstractStateHandler> handleInputImpl(LaraState& state) override
    {
        state.frobbelFlags &= ~(LaraState::FrobbelFlag08 | LaraState::FrobbelFlag10);
        return nullptr;
    }

    void animateImpl(LaraState& /*state*/, int /*deltaTimeMs*/) override
    {
    }

    std::unique_ptr<AbstractStateHandler> postprocessFrame(LaraState& state) override
    {
        state.neededFloorDistanceBottom = core::ClimbLimit2ClickMin;
        state.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
        state.neededCeilingDistance = 0;
        state.yAngle = static_cast<int16_t>(getRotation().Y);
        setMovementAngle(state.yAngle);
        state.frobbelFlags |= LaraState::FrobbelFlag_UnpassableSteepUpslant | LaraState::FrobbelFlag_UnwalkableSteepFloor;
        state.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
        return nullptr;
    }

    loader::LaraStateId getId() const noexcept override
    {
        return LaraStateId::Climbing;
    }
};

class StateHandler_20 final : public StateHandler_Standing
{
public:
    explicit StateHandler_20(LaraStateHandler& lara)
        : StateHandler_Standing(lara)
    {
    }

    std::unique_ptr<AbstractStateHandler> handleInputImpl(LaraState& /*state*/) override
    {
        if( getHealth() <= 0 )
        {
            setTargetState(LaraStateId::Stop);
            return nullptr;
        }

        if( getYRotationSpeed() >= 0 )
        {
            setYRotationSpeed(1456);
            if( getInputState().xMovement == AxisMovement::Right )
                return nullptr;
        }
        else
        {
            setYRotationSpeed(-1456);
            if( getInputState().xMovement == AxisMovement::Left )
                return nullptr;
        }

        setTargetState(LaraStateId::Stop);
        return nullptr;
    }

    void animateImpl(LaraState& /*state*/, int /*deltaTimeMs*/) override
    {
    }

    loader::LaraStateId getId() const noexcept override
    {
        return LaraStateId::TurnFast;
    }
};

class StateHandler_21 final : public AbstractStateHandler
{
public:
    explicit StateHandler_21(LaraStateHandler& lara)
        : AbstractStateHandler(lara)
    {
    }

    std::unique_ptr<AbstractStateHandler> handleInputImpl(LaraState& /*state*/) override
    {
        if( getHealth() <= 0 )
        {
            setTargetState(LaraStateId::Stop);
            return nullptr;
        }

        if(getInputState().stepMovement != AxisMovement::Right)
            setTargetState(LaraStateId::Stop);

        if(getInputState().xMovement == AxisMovement::Left)
            setYRotationSpeed(std::max(-728, getYRotationSpeed() - 409));
        else if(getInputState().xMovement == AxisMovement::Right)
            setYRotationSpeed(std::min(728, getYRotationSpeed() + 409));

        return nullptr;
    }

    void animateImpl(LaraState& /*state*/, int /*deltaTimeMs*/) override
    {
    }

    loader::LaraStateId getId() const noexcept override
    {
        return LaraStateId::StepRight;
    }

    std::unique_ptr<AbstractStateHandler> postprocessFrame(LaraState& state) override
    {
        setFallSpeed(0);
        setFalling(false);
        state.neededFloorDistanceBottom = 128;
        state.neededFloorDistanceTop = -128;
        state.neededCeilingDistance = 0;
        state.yAngle = static_cast<int16_t>(getRotation().Y + util::degToAu(90));
        setMovementAngle(state.yAngle);
        state.frobbelFlags |= LaraState::FrobbelFlag_UnpassableSteepUpslant | LaraState::FrobbelFlag_UnwalkableSteepFloor;
        state.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);

        auto nextHandler = tryStopOnFloor(state);
        if(nextHandler != nullptr)
            return nextHandler;

        nextHandler = checkWallCollision(state);
        if(nextHandler != nullptr)
        {
            playAnimation(loader::AnimationId::STAY_SOLID, 185);
            setTargetState(LaraStateId::Stop);
            return createWithRetainedAnimation(LaraStateId::Stop);
        }

        if(!tryStartSlide(state, nextHandler))
            setPosition(getExactPosition() + loader::ExactTRCoordinates(0, state.current.floor.distance, 0));

        return nextHandler;
    }
};

class StateHandler_22 final : public AbstractStateHandler
{
public:
    explicit StateHandler_22(LaraStateHandler& lara)
        : AbstractStateHandler(lara)
    {
    }

    std::unique_ptr<AbstractStateHandler> handleInputImpl(LaraState& /*state*/) override
    {
        if( getHealth() <= 0 )
        {
            setTargetState(LaraStateId::Stop);
            return nullptr;
        }

        if(getInputState().stepMovement != AxisMovement::Left)
            setTargetState(LaraStateId::Stop);

        if(getInputState().xMovement == AxisMovement::Left)
            setYRotationSpeed(std::max(-728, getYRotationSpeed() - 409));
        else if(getInputState().xMovement == AxisMovement::Right)
            setYRotationSpeed(std::min(728, getYRotationSpeed() + 409));

        return nullptr;
    }

    void animateImpl(LaraState& /*state*/, int /*deltaTimeMs*/) override
    {
    }

    loader::LaraStateId getId() const noexcept override
    {
        return LaraStateId::StepLeft;
    }

    std::unique_ptr<AbstractStateHandler> postprocessFrame(LaraState& state) override
    {
        setFallSpeed(0);
        setFalling(false);
        state.neededFloorDistanceBottom = 128;
        state.neededFloorDistanceTop = -128;
        state.neededCeilingDistance = 0;
        state.yAngle = static_cast<int16_t>(getRotation().Y - util::degToAu(90));
        setMovementAngle(state.yAngle);
        state.frobbelFlags |= LaraState::FrobbelFlag_UnpassableSteepUpslant | LaraState::FrobbelFlag_UnwalkableSteepFloor;
        state.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);

        auto nextHandler = tryStopOnFloor(state);
        if(nextHandler != nullptr)
            return nextHandler;

        nextHandler = checkWallCollision(state);
        if(nextHandler != nullptr)
        {
            playAnimation(loader::AnimationId::STAY_SOLID, 185);
            setTargetState(LaraStateId::Stop);
            return createWithRetainedAnimation(LaraStateId::Stop);
        }

        if(!tryStartSlide(state, nextHandler))
            setPosition(getExactPosition() + loader::ExactTRCoordinates(0, state.current.floor.distance, 0));

        return nextHandler;
    }
};

class StateHandler_23 final : public AbstractStateHandler
{
public:
    explicit StateHandler_23(LaraStateHandler& lara)
        : AbstractStateHandler(lara)
    {
    }

    std::unique_ptr<AbstractStateHandler> handleInputImpl(LaraState& /*state*/) override
    {
        return nullptr;
    }

    void animateImpl(LaraState& /*state*/, int /*deltaTimeMs*/) override
    {
    }

    std::unique_ptr<AbstractStateHandler> postprocessFrame(LaraState& state) override
    {
        setFalling(false);
        setFallSpeed(0);
        state.yAngle = static_cast<int16_t>(getRotation().Y + util::degToAu(180));
        setMovementAngle(state.yAngle);
        state.frobbelFlags |= LaraState::FrobbelFlag_UnpassableSteepUpslant;
        state.neededFloorDistanceBottom = loader::HeightLimit;
        state.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
        state.neededCeilingDistance = 0;
        state.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);

        auto nextHandler = tryStopOnFloor(state);
        if( nextHandler )
            return nextHandler;
        if( tryStartSlide(state, nextHandler) )
            return nextHandler;

        if( state.current.floor.distance <= 200 )
        {
            applyCollisionFeedback(state);
            placeOnFloor(state);
            return nextHandler;
        }

        playAnimation(loader::AnimationId::FREE_FALL_BACK, 1473);
        setTargetState(LaraStateId::FallBackward);
        setFallSpeed(0);
        setFalling(true);

        return createWithRetainedAnimation(LaraStateId::FallBackward);
    }

    loader::LaraStateId getId() const noexcept override
    {
        return LaraStateId::RollBackward;
    }
};

class StateHandler_24 final : public AbstractStateHandler
{
public:
    explicit StateHandler_24(LaraStateHandler& lara)
        : AbstractStateHandler(lara)
    {
    }

    std::unique_ptr<AbstractStateHandler> handleInputImpl(LaraState& /*state*/) override
    {
        if( getInputState().jump )
            setTargetState(LaraStateId::JumpForward);

        return nullptr;
    }

    void animateImpl(LaraState& /*state*/, int /*deltaTimeMs*/) override
    {
    }

    std::unique_ptr<AbstractStateHandler> postprocessFrame(LaraState& state) override
    {
        setMovementAngle(static_cast<int16_t>(getRotation().Y));
        return commonSlideHandling(state);
    }

    loader::LaraStateId getId() const noexcept override
    {
        return LaraStateId::SlideForward;
    }
};

class StateHandler_25 final : public AbstractStateHandler
{
public:
    explicit StateHandler_25(LaraStateHandler& lara)
        : AbstractStateHandler(lara)
    {
    }

    std::unique_ptr<AbstractStateHandler> handleInputImpl(LaraState& /*state*/) override
    {
        //! @todo Set local camera Y rotation to 24570 AU
        if( getFallSpeed().get() > core::FreeFallSpeedThreshold )
            setTargetState(LaraStateId::FreeFall);

        return nullptr;
    }

    void animateImpl(LaraState& /*state*/, int /*deltaTimeMs*/) override
    {
    }

    std::unique_ptr<AbstractStateHandler> postprocessFrame(LaraState& state) override
    {
        setMovementAngle(getRotation().Y + util::degToAu(180));
        return commonJumpHandling(state);
    }

    loader::LaraStateId getId() const noexcept override
    {
        return LaraStateId::JumpBack;
    }
};

class StateHandler_26 final : public AbstractStateHandler
{
public:
    explicit StateHandler_26(LaraStateHandler& lara)
        : AbstractStateHandler(lara)
    {
    }

    std::unique_ptr<AbstractStateHandler> handleInputImpl(LaraState& /*state*/) override
    {
        if( getFallSpeed().get() > core::FreeFallSpeedThreshold )
            setTargetState(LaraStateId::FreeFall);

        return nullptr;
    }

    void animateImpl(LaraState& /*state*/, int /*deltaTimeMs*/) override
    {
    }

    std::unique_ptr<AbstractStateHandler> postprocessFrame(LaraState& state) override
    {
        setMovementAngle(getRotation().Y + util::degToAu(90));
        return commonJumpHandling(state);
    }

    loader::LaraStateId getId() const noexcept override
    {
        return LaraStateId::JumpLeft;
    }
};

class StateHandler_27 final : public AbstractStateHandler
{
public:
    explicit StateHandler_27(LaraStateHandler& lara)
        : AbstractStateHandler(lara)
    {
    }

    std::unique_ptr<AbstractStateHandler> handleInputImpl(LaraState& /*state*/) override
    {
        if( getFallSpeed().get() > core::FreeFallSpeedThreshold )
            setTargetState(LaraStateId::FreeFall);

        return nullptr;
    }

    void animateImpl(LaraState& /*state*/, int /*deltaTimeMs*/) override
    {
    }

    std::unique_ptr<AbstractStateHandler> postprocessFrame(LaraState& state) override
    {
        setMovementAngle(getRotation().Y - util::degToAu(90));
        return commonJumpHandling(state);
    }

    loader::LaraStateId getId() const noexcept override
    {
        return LaraStateId::JumpRight;
    }
};

class StateHandler_28 final : public AbstractStateHandler
{
public:
    explicit StateHandler_28(LaraStateHandler& lara)
        : AbstractStateHandler(lara)
    {
    }

    std::unique_ptr<AbstractStateHandler> handleInputImpl(LaraState& /*state*/) override
    {
        if( getFallSpeed().get() > core::FreeFallSpeedThreshold )
            setTargetState(LaraStateId::FreeFall);

        return nullptr;
    }

    void animateImpl(LaraState& /*state*/, int /*deltaTimeMs*/) override
    {
    }

    std::unique_ptr<AbstractStateHandler> postprocessFrame(LaraState& state) override
    {
        state.neededFloorDistanceBottom = loader::HeightLimit;
        state.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
        state.neededCeilingDistance = 192;
        state.yAngle = getRotation().Y;
        state.initHeightInfo(getPosition(), getLevel(), 870); //! @todo MAGICK 870

        if( auto nextHandler = tryGrabEdge(state) )
            return nextHandler;

        jumpAgainstWall(state);
        if( getFallSpeed().get() <= 0 || state.current.floor.distance > 0 )
            return nullptr;

        if( applyLandingDamage() )
            setTargetState(LaraStateId::Death);
        else
            setTargetState(LaraStateId::Stop);
        setFallSpeed(0);
        placeOnFloor(state);
        setFalling(false);

        return nullptr;
    }

    loader::LaraStateId getId() const noexcept override
    {
        return LaraStateId::JumpUp;
    }
};

class StateHandler_29 final : public AbstractStateHandler
{
public:
    explicit StateHandler_29(LaraStateHandler& lara)
        : AbstractStateHandler(lara)
    {
    }

    std::unique_ptr<AbstractStateHandler> handleInputImpl(LaraState& /*state*/) override
    {
        if( getFallSpeed().get() > core::FreeFallSpeedThreshold )
            setTargetState(LaraStateId::FreeFall);

        if( getInputState().action && getHandStatus() == 0 )
            setTargetState(LaraStateId::Reach);

        return nullptr;
    }

    void animateImpl(LaraState& /*state*/, int /*deltaTimeMs*/) override
    {
    }

    std::unique_ptr<AbstractStateHandler> postprocessFrame(LaraState& state) override
    {
        state.neededFloorDistanceBottom = loader::HeightLimit;
        state.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
        state.neededCeilingDistance = 192;
        state.yAngle = getRotation().Y + util::degToAu(180);
        state.initHeightInfo(getPosition(), getLevel(), 870); //! @todo MAGICK 870
        auto nextHandler = checkJumpWallSmash(state);
        if( state.current.floor.distance > 0 || getFallSpeed().get() <= 0 )
            return nextHandler;

        if( applyLandingDamage() )
            setTargetState(LaraStateId::Death);
        else
            setTargetState(LaraStateId::Stop);

        setFallSpeed(0);
        placeOnFloor(state);
        setFalling(false);

        return nextHandler;
    }

    loader::LaraStateId getId() const noexcept override
    {
        return LaraStateId::FallBackward;
    }
};

class StateHandler_30 final : public AbstractStateHandler
{
public:
    explicit StateHandler_30(LaraStateHandler& lara)
        : AbstractStateHandler(lara)
    {
    }

    std::unique_ptr<AbstractStateHandler> handleInputImpl(LaraState& state) override
    {
        state.frobbelFlags &= ~(LaraState::FrobbelFlag08 | LaraState::FrobbelFlag10);
        //! @todo set camera rotation (x,y)=(-10920,0)
        if( getInputState().xMovement != AxisMovement::Left && getInputState().stepMovement != AxisMovement::Left )
            setTargetState(LaraStateId::Hang);

        return nullptr;
    }

    std::unique_ptr<AbstractStateHandler> postprocessFrame(LaraState& state) override
    {
        setMovementAngle(static_cast<int16_t>(getRotation().Y - util::degToAu(90)));
        auto nextHandler = commonEdgeHangHandling(state);
        setMovementAngle(static_cast<int16_t>(getRotation().Y - util::degToAu(90)));
        return nextHandler;
    }

    loader::LaraStateId getId() const noexcept override
    {
        return LaraStateId::ShimmyLeft;
    }

    void animateImpl(LaraState& /*state*/, int /*deltaTimeMs*/) override
    {
    }
};

class StateHandler_31 final : public AbstractStateHandler
{
public:
    explicit StateHandler_31(LaraStateHandler& lara)
        : AbstractStateHandler(lara)
    {
    }

    std::unique_ptr<AbstractStateHandler> handleInputImpl(LaraState& state) override
    {
        state.frobbelFlags &= ~(LaraState::FrobbelFlag08 | LaraState::FrobbelFlag10);
        //! @todo set camera rotation (x,y)=(-10920,0)
        if( getInputState().xMovement != AxisMovement::Right && getInputState().stepMovement != AxisMovement::Right )
            setTargetState(LaraStateId::Hang);

        return nullptr;
    }

    std::unique_ptr<AbstractStateHandler> postprocessFrame(LaraState& state) override
    {
        setMovementAngle(static_cast<int16_t>(getRotation().Y + util::degToAu(90)));
        auto nextHandler = commonEdgeHangHandling(state);
        setMovementAngle(static_cast<int16_t>(getRotation().Y + util::degToAu(90)));
        return nextHandler;
    }

    loader::LaraStateId getId() const noexcept override
    {
        return LaraStateId::ShimmyRight;
    }

    void animateImpl(LaraState& /*state*/, int /*deltaTimeMs*/) override
    {
    }
};

class StateHandler_32 final : public AbstractStateHandler
{
public:
    explicit StateHandler_32(LaraStateHandler& lara)
        : AbstractStateHandler(lara)
    {
    }

    std::unique_ptr<AbstractStateHandler> handleInputImpl(LaraState& /*state*/) override
    {
        if( getInputState().jump )
            setTargetState(LaraStateId::JumpBack);

        return nullptr;
    }

    void animateImpl(LaraState& /*state*/, int /*deltaTimeMs*/) override
    {
    }

    std::unique_ptr<AbstractStateHandler> postprocessFrame(LaraState& state) override
    {
        setMovementAngle(getRotation().Y + util::degToAu(180));
        return commonSlideHandling(state);
    }

    loader::LaraStateId getId() const noexcept override
    {
        return LaraStateId::SlideBackward;
    }
};

class StateHandler_OnWater : public AbstractStateHandler
{
public:
    explicit StateHandler_OnWater(LaraStateHandler& lara)
        : AbstractStateHandler(lara)
    {
    }

protected:
    std::unique_ptr<AbstractStateHandler> commonOnWaterHandling(LaraState& state)
    {
        state.yAngle = getMovementAngle();
        state.initHeightInfo(getPosition() + loader::TRCoordinates(0, 700, 0), getLevel(), 700);
        applyCollisionFeedback(state);
        if(   state.current.floor.distance < 0
           || state.axisCollisions == LaraState::AxisColl_CeilingTooLow
           || state.axisCollisions == LaraState::AxisColl_BumpHead
           || state.axisCollisions == LaraState::AxisColl_HeadInCeiling
           || state.axisCollisions == LaraState::AxisColl_InsufficientFrontSpace
           )
        {
            setFallSpeed(0);
            setPosition(state.position);
        }
        else
        {
            if(state.axisCollisions == LaraState::AxisColl_FrontLeftBump)
                m_yRotationSpeed = 910;
            else if(state.axisCollisions == LaraState::AxisColl_FrontRightBump)
                m_yRotationSpeed = -910;
            else
                m_yRotationSpeed = 0;
        }

        auto wsh = getStateHandler().getWaterSurfaceHeight();
        if(wsh && *wsh > getPosition().Y - 100)
        {
            return tryClimbOutOfWater(state);
        }

        setTargetState(LaraStateId::UnderwaterForward);
        playAnimation(loader::AnimationId::FREE_FALL_TO_UNDERWATER_ALTERNATE, 2041);
        setXRotation(-8190);
        setFallSpeed(80);
        setUnderwaterState(UnderwaterState::Diving);
        return createWithRetainedAnimation(LaraStateId::UnderwaterDiving);
    }

private:
    std::unique_ptr<AbstractStateHandler> tryClimbOutOfWater(LaraState& state)
    {
        if(getMovementAngle() != gsl::narrow_cast<int16_t>(getRotation().Y))
            return nullptr;

        if(state.axisCollisions != LaraState::AxisColl_InsufficientFrontSpace)
            return nullptr;

        if(!getInputState().action)
            return nullptr;

        const auto gradient = std::abs(state.frontLeft.floor.distance - state.frontRight.floor.distance);
        if(gradient >= core::MaxGrabableGradient)
            return nullptr;

        if(state.front.ceiling.distance > 0)
            return nullptr;

        if(state.current.ceiling.distance > -core::ClimbLimit2ClickMin)
            return nullptr;

        if(state.front.floor.distance + 700 <= -2*loader::QuarterSectorSize)
            return nullptr;

        if(state.front.floor.distance + 700 > 100)
            return nullptr;

        const auto yRot = util::alignRotation(gsl::narrow_cast<int16_t>(getRotation().Y), util::degToAu(35));
        if(!yRot)
            return nullptr;

        setPosition(getExactPosition() + loader::ExactTRCoordinates(0, 695 + state.front.floor.distance, 0));
        getStateHandler().updateFloorHeight(-381);
        loader::ExactTRCoordinates d = getExactPosition();
        if(*yRot == util::degToAu(0))
            d.Z = (getPosition().Z / loader::SectorSize + 1) * loader::SectorSize + 100;
        else if(*yRot == util::degToAu(180))
            d.Z = (getPosition().Z / loader::SectorSize + 0) * loader::SectorSize - 100;
        else if(*yRot == util::degToAu(-90))
            d.X = (getPosition().X / loader::SectorSize + 0) * loader::SectorSize - 100;
        else if(*yRot == util::degToAu(90))
            d.X = (getPosition().X / loader::SectorSize + 1) * loader::SectorSize + 100;
        else
            throw std::runtime_error("Unexpected angle value");

        setPosition(d);

        setTargetState(LaraStateId::Stop);
        playAnimation(loader::AnimationId::CLIMB_OUT_OF_WATER, 1849);
        setHorizontalSpeed(0);
        setFallSpeed(0);
        setFalling(false);
        setXRotation(0);
        setYRotation(*yRot);
        setZRotation(0);
        setHandStatus(1);
        setUnderwaterState(UnderwaterState::OnLand);
        return createWithRetainedAnimation(LaraStateId::OnWaterExit);
    }
};

class StateHandler_33 final : public StateHandler_OnWater
{
public:
    explicit StateHandler_33(LaraStateHandler& lara)
        : StateHandler_OnWater(lara)
    {
    }

    std::unique_ptr<AbstractStateHandler> handleInputImpl(LaraState& /*state*/) override
    {
        if(getHealth() <= 0)
        {
            setTargetState(LaraStateId::WaterDeath);
            return nullptr;
        }

        if(getInputState().zMovement == AxisMovement::Forward)
            setTargetState(LaraStateId::OnWaterForward);
        else if(getInputState().zMovement == AxisMovement::Backward)
            setTargetState(LaraStateId::OnWaterBackward);

        if(getInputState().stepMovement == AxisMovement::Left)
            setTargetState(LaraStateId::OnWaterLeft);
        else if(getInputState().stepMovement == AxisMovement::Right)
            setTargetState(LaraStateId::OnWaterRight);

        if(!getInputState().jump)
        {
            setSwimToDiveKeypressDuration(0);
            return nullptr;
        }

        if(getSwimToDiveKeypressDuration() * 30 / 1000 < 10)
            return nullptr;

        setTargetState(LaraStateId::UnderwaterForward);
        playAnimation(loader::AnimationId::FREE_FALL_TO_UNDERWATER_ALTERNATE, 2041);
        setXRotation(-8190);
        setFallSpeed(80);
        setUnderwaterState(UnderwaterState::Diving);
        return createWithRetainedAnimation(LaraStateId::UnderwaterDiving);
    }

    void animateImpl(LaraState& /*state*/, int deltaTimeMs) override
    {
        setFallSpeedExact(std::max(0.0f, getFallSpeed().getExact() - makeSpeedValue(4).getScaledExact(deltaTimeMs)));

        if(getInputState().xMovement == AxisMovement::Left)
            m_yRotationSpeed = -728;
        else if(getInputState().xMovement == AxisMovement::Right)
            m_yRotationSpeed = 728;
        else
            m_yRotationSpeed = 0;

        addSwimToDiveKeypressDuration(deltaTimeMs);
    }

    std::unique_ptr<AbstractStateHandler> postprocessFrame(LaraState& state) override
    {
        setMovementAngle(getRotation().Y);
        return commonOnWaterHandling(state);
    }

    loader::LaraStateId getId() const noexcept override
    {
        return LaraStateId::OnWaterStop;
    }
};

class StateHandler_34 final : public StateHandler_OnWater
{
public:
    explicit StateHandler_34(LaraStateHandler& lara)
        : StateHandler_OnWater(lara)
    {
    }

    std::unique_ptr<AbstractStateHandler> handleInputImpl(LaraState& /*state*/) override
    {
        if(getHealth() <= 0)
        {
            setTargetState(LaraStateId::WaterDeath);
            return nullptr;
        }

        setSwimToDiveKeypressDuration(0);

        if(getInputState().zMovement != AxisMovement::Forward)
            setTargetState(LaraStateId::OnWaterStop);

        if(getInputState().jump)
            setTargetState(LaraStateId::OnWaterStop);

        return nullptr;
    }

    void animateImpl(LaraState& /*state*/, int deltaTimeMs) override
    {
        setFallSpeedExact(std::min(60.0f, getFallSpeed().getExact() + makeSpeedValue(8).getScaledExact(deltaTimeMs)));

        if(getInputState().xMovement == AxisMovement::Left)
            m_yRotationSpeed = -728;
        else if(getInputState().xMovement == AxisMovement::Right)
            m_yRotationSpeed = 728;
        else
            m_yRotationSpeed = 0;
    }

    std::unique_ptr<AbstractStateHandler> postprocessFrame(LaraState& state) override
    {
        setMovementAngle(getRotation().Y);
        return commonOnWaterHandling(state);
    }

    loader::LaraStateId getId() const noexcept override
    {
        return LaraStateId::OnWaterForward;
    }
};

class StateHandler_35 final : public StateHandler_Underwater
{
public:
    explicit StateHandler_35(LaraStateHandler& lara)
        : StateHandler_Underwater(lara)
    {
    }

    std::unique_ptr<AbstractStateHandler> handleInputImpl(LaraState& /*state*/) override
    {
        if(getInputState().zMovement == AxisMovement::Forward)
            m_yRotationSpeed = -182;
        else
            m_yRotationSpeed = 0;

        return nullptr;
    }

    void animateImpl(LaraState& /*state*/, int /*deltaTimeMs*/) override
    {
    }

    loader::LaraStateId getId() const noexcept override
    {
        return LaraStateId::UnderwaterDiving;
    }
};

class StateHandler_44 final : public StateHandler_Underwater
{
public:
    explicit StateHandler_44(LaraStateHandler& lara)
        : StateHandler_Underwater(lara)
    {
    }

    std::unique_ptr<AbstractStateHandler> handleInputImpl(LaraState& /*state*/) override
    {
        if(getRotation().X < 0)
            m_xRotationSpeed = -364;
        else if(getRotation().X > 0)
            m_xRotationSpeed = 364;
        else
            m_xRotationSpeed = 0;

        return nullptr;
    }

    void animateImpl(LaraState& /*state*/, int deltaTimeMs) override
    {
        setFallSpeedExact(std::max(0.0f, getFallSpeed().getExact() - makeSpeedValue(8).getScaledExact(deltaTimeMs)));
    }

    loader::LaraStateId getId() const noexcept override
    {
        return LaraStateId::WaterDeath;
    }

    std::unique_ptr<AbstractStateHandler> postprocessFrame(LaraState& state) override
    {
        setHealth(-1);
        setAir(-1);
        setHandStatus(1);
        auto h = getStateHandler().getWaterSurfaceHeight();
        if(h && *h < getPosition().Y - 100)
            setPosition(getExactPosition() - loader::ExactTRCoordinates(0,5,0));

        return StateHandler_Underwater::postprocessFrame(state);
    }
};

class StateHandler_45 final : public AbstractStateHandler
{
public:
    explicit StateHandler_45(LaraStateHandler& lara)
        : AbstractStateHandler(lara)
    {
    }

    std::unique_ptr<AbstractStateHandler> handleInputImpl(LaraState& /*state*/) override
    {
        return nullptr;
    }

    void animateImpl(LaraState& /*state*/, int /*deltaTimeMs*/) override
    {
    }

    std::unique_ptr<AbstractStateHandler> postprocessFrame(LaraState& state) override
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

        auto nextHandler = tryStopOnFloor(state);
        if( nextHandler )
            return nextHandler;
        if( tryStartSlide(state, nextHandler) )
            return nextHandler;

        if( state.current.floor.distance <= 200 )
        {
            applyCollisionFeedback(state);
            placeOnFloor(state);
            return nextHandler;
        }

        playAnimation(loader::AnimationId::FREE_FALL_FORWARD, 492);
        setTargetState(LaraStateId::JumpForward);
        setFallSpeed(0);
        setFalling(true);

        return createWithRetainedAnimation(LaraStateId::JumpForward);
    }

    loader::LaraStateId getId() const noexcept override
    {
        return LaraStateId::RollForward;
    }
};

class StateHandler_47 final : public StateHandler_OnWater
{
public:
    explicit StateHandler_47(LaraStateHandler& lara)
        : StateHandler_OnWater(lara)
    {
    }

    std::unique_ptr<AbstractStateHandler> handleInputImpl(LaraState& /*state*/) override
    {
        if(getHealth() <= 0)
        {
            setTargetState(LaraStateId::WaterDeath);
            return nullptr;
        }

        setSwimToDiveKeypressDuration(0);

        if(getInputState().zMovement != AxisMovement::Backward)
            setTargetState(LaraStateId::OnWaterStop);

        return nullptr;
    }

    void animateImpl(LaraState& /*state*/, int deltaTimeMs) override
    {
        setFallSpeedExact(std::min(60.0f, getFallSpeed().getExact() + makeSpeedValue(8).getScaledExact(deltaTimeMs)));

        if(getInputState().xMovement == AxisMovement::Left)
            m_yRotationSpeed = -364;
        else if(getInputState().xMovement == AxisMovement::Right)
            m_yRotationSpeed = 364;
        else
            m_yRotationSpeed = 0;
    }

    std::unique_ptr<AbstractStateHandler> postprocessFrame(LaraState& state) override
    {
        setMovementAngle(getRotation().Y + util::degToAu(180));
        return commonOnWaterHandling(state);
    }

    loader::LaraStateId getId() const noexcept override
    {
        return LaraStateId::OnWaterBackward;
    }
};

class StateHandler_48 final : public StateHandler_OnWater
{
public:
    explicit StateHandler_48(LaraStateHandler& lara)
        : StateHandler_OnWater(lara)
    {
    }

    std::unique_ptr<AbstractStateHandler> handleInputImpl(LaraState& /*state*/) override
    {
        if(getHealth() <= 0)
        {
            setTargetState(LaraStateId::WaterDeath);
            return nullptr;
        }

        setSwimToDiveKeypressDuration(0);

        if(getInputState().stepMovement != AxisMovement::Left)
            setTargetState(LaraStateId::OnWaterStop);

        return nullptr;
    }

    void animateImpl(LaraState& /*state*/, int deltaTimeMs) override
    {
        setFallSpeedExact(std::min(60.0f, getFallSpeed().getExact() + makeSpeedValue(8).getScaledExact(deltaTimeMs)));

        if(getInputState().xMovement == AxisMovement::Left)
            m_yRotationSpeed = -364;
        else if(getInputState().xMovement == AxisMovement::Right)
            m_yRotationSpeed = 364;
        else
            m_yRotationSpeed = 0;
    }

    std::unique_ptr<AbstractStateHandler> postprocessFrame(LaraState& state) override
    {
        setMovementAngle(getRotation().Y - util::degToAu(90));
        return commonOnWaterHandling(state);
    }

    loader::LaraStateId getId() const noexcept override
    {
        return LaraStateId::OnWaterLeft;
    }
};

class StateHandler_49 final : public StateHandler_OnWater
{
public:
    explicit StateHandler_49(LaraStateHandler& lara)
        : StateHandler_OnWater(lara)
    {
    }

    std::unique_ptr<AbstractStateHandler> handleInputImpl(LaraState& /*state*/) override
    {
        if(getHealth() <= 0)
        {
            setTargetState(LaraStateId::WaterDeath);
            return nullptr;
        }

        setSwimToDiveKeypressDuration(0);

        if(getInputState().stepMovement != AxisMovement::Right)
            setTargetState(LaraStateId::OnWaterStop);

        return nullptr;
    }

    void animateImpl(LaraState& /*state*/, int deltaTimeMs) override
    {
        setFallSpeedExact(std::min(60.0f, getFallSpeed().getExact() + makeSpeedValue(8).getScaledExact(deltaTimeMs)));

        if(getInputState().xMovement == AxisMovement::Left)
            m_yRotationSpeed = -364;
        else if(getInputState().xMovement == AxisMovement::Right)
            m_yRotationSpeed = 364;
        else
            m_yRotationSpeed = 0;
    }

    std::unique_ptr<AbstractStateHandler> postprocessFrame(LaraState& state) override
    {
        setMovementAngle(getRotation().Y + util::degToAu(90));
        return commonOnWaterHandling(state);
    }

    loader::LaraStateId getId() const noexcept override
    {
        return LaraStateId::OnWaterRight;
    }
};

class StateHandler_52 final : public AbstractStateHandler
{
public:
    explicit StateHandler_52(LaraStateHandler& lara)
        : AbstractStateHandler(lara)
    {
    }

    std::unique_ptr<AbstractStateHandler> handleInputImpl(LaraState& state) override
    {
        state.frobbelFlags &= ~LaraState::FrobbelFlag10;
        state.frobbelFlags |= LaraState::FrobbelFlag08;
        if( getFallSpeed().get() > core::FreeFallSpeedThreshold )
            setTargetState(LaraStateId::SwandiveEnd);

        return nullptr;
    }

    void animateImpl(LaraState& /*state*/, int /*deltaTimeMs*/) override
    {
    }

    std::unique_ptr<AbstractStateHandler> postprocessFrame(LaraState& state) override
    {
        state.neededFloorDistanceBottom = loader::HeightLimit;
        state.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
        state.neededCeilingDistance = 192;
        state.yAngle = getRotation().Y;
        setMovementAngle(state.yAngle);
        state.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
        auto nextHandler = checkJumpWallSmash(state);
        if( state.current.floor.distance > 0 || getFallSpeed().get() <= 0 )
            return nextHandler;

        setTargetState(LaraStateId::Stop);
        setFallSpeed(0);
        setFalling(false);
        placeOnFloor(state);

        return nextHandler;
    }

    loader::LaraStateId getId() const noexcept override
    {
        return LaraStateId::SwandiveBegin;
    }
};

class StateHandler_53 final : public AbstractStateHandler
{
public:
    explicit StateHandler_53(LaraStateHandler& lara)
        : AbstractStateHandler(lara)
    {
    }

    std::unique_ptr<AbstractStateHandler> handleInputImpl(LaraState& state) override
    {
        state.frobbelFlags &= ~LaraState::FrobbelFlag10;
        state.frobbelFlags |= LaraState::FrobbelFlag08;
        return nullptr;
    }

    void animateImpl(LaraState& /*state*/, int /*deltaTimeMs*/) override
    {
        dampenHorizontalSpeed(5, 100);
    }

    std::unique_ptr<AbstractStateHandler> postprocessFrame(LaraState& state) override
    {
        state.neededFloorDistanceBottom = loader::HeightLimit;
        state.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
        state.neededCeilingDistance = 192;
        state.yAngle = getRotation().Y;
        setMovementAngle(state.yAngle);
        state.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
        auto nextHandler = checkJumpWallSmash(state);
        if( state.current.floor.distance > 0 || getFallSpeed().get() <= 0 )
            return nextHandler;

        if( getFallSpeed().get() <= 133 )
            setTargetState(LaraStateId::Stop);
        else
            setTargetState(LaraStateId::Death);

        setFallSpeed(0);
        setFalling(false);
        placeOnFloor(state);

        return nextHandler;
    }

    loader::LaraStateId getId() const noexcept override
    {
        return LaraStateId::SwandiveEnd;
    }
};

class StateHandler_54 final : public AbstractStateHandler
{
public:
    explicit StateHandler_54(LaraStateHandler& lara)
        : AbstractStateHandler(lara)
    {
    }

    std::unique_ptr<AbstractStateHandler> handleInputImpl(LaraState& state) override
    {
        state.frobbelFlags &= ~(LaraState::FrobbelFlag08 | LaraState::FrobbelFlag10);
        return nullptr;
    }

    std::unique_ptr<AbstractStateHandler> postprocessFrame(LaraState& state) override
    {
        state.neededFloorDistanceBottom = core::ClimbLimit2ClickMin;
        state.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
        state.neededCeilingDistance = 0;
        state.frobbelFlags |= LaraState::FrobbelFlag_UnwalkableSteepFloor | LaraState::FrobbelFlag_UnpassableSteepUpslant;
        state.yAngle = getRotation().Y;
        setMovementAngle(state.yAngle);
        state.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
        return nullptr;
    }

    loader::LaraStateId getId() const noexcept override
    {
        return LaraStateId::Handstand;
    }

    void animateImpl(LaraState& /*state*/, int /*deltaTimeMs*/) override
    {

    }
};

class StateHandler_55 final : public AbstractStateHandler
{
public:
    explicit StateHandler_55(LaraStateHandler& lara)
        : AbstractStateHandler(lara)
    {
    }

    std::unique_ptr<AbstractStateHandler> handleInputImpl(LaraState& state) override
    {
        state.frobbelFlags &= ~(LaraState::FrobbelFlag08 | LaraState::FrobbelFlag10);
        return nullptr;
    }

    std::unique_ptr<AbstractStateHandler> postprocessFrame(LaraState& state) override
    {
        state.neededFloorDistanceBottom = core::ClimbLimit2ClickMin;
        state.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
        state.neededCeilingDistance = 0;
        state.frobbelFlags |= LaraState::FrobbelFlag_UnwalkableSteepFloor | LaraState::FrobbelFlag_UnpassableSteepUpslant;
        state.yAngle = getRotation().Y;
        setMovementAngle(state.yAngle);
        state.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
        return nullptr;
    }

    loader::LaraStateId getId() const noexcept override
    {
        return LaraStateId::OnWaterExit;
    }

    void animateImpl(LaraState& /*state*/, int /*deltaTimeMs*/) override
    {

    }
};

void AbstractStateHandler::animate(LaraState& state, int deltaTimeMs)
{
    animateImpl(state, deltaTimeMs);

    m_stateHandler.rotate(
                          m_xRotationSpeed.getScaledExact(deltaTimeMs),
                          m_yRotationSpeed.getScaledExact(deltaTimeMs),
                          m_zRotationSpeed.getScaledExact(deltaTimeMs)
                         );
    m_stateHandler.move(
                        m_xMovement.getScaledExact(deltaTimeMs),
                        m_yMovement.getScaledExact(deltaTimeMs),
                        m_zMovement.getScaledExact(deltaTimeMs)
                       );
}

std::unique_ptr<AbstractStateHandler> AbstractStateHandler::create(loader::LaraStateId id, LaraStateHandler& lara)
{
    switch( id )
    {
    case LaraStateId::WalkForward:
        return std::make_unique<StateHandler_0>(lara);
    case LaraStateId::RunForward:
        return std::make_unique<StateHandler_1>(lara);
    case LaraStateId::Stop:
        return std::make_unique<StateHandler_2>(lara);
    case LaraStateId::JumpForward:
        return std::make_unique<StateHandler_3>(lara);
    case LaraStateId::Pose:
        return std::make_unique<StateHandler_4>(lara);
    case LaraStateId::RunBack:
        return std::make_unique<StateHandler_5>(lara);
    case LaraStateId::TurnRightSlow:
        return std::make_unique<StateHandler_6>(lara);
    case LaraStateId::TurnLeftSlow:
        return std::make_unique<StateHandler_7>(lara);
    case LaraStateId::Death:
        return std::make_unique<StateHandler_8>(lara);
    case LaraStateId::FreeFall:
        return std::make_unique<StateHandler_9>(lara);
    case LaraStateId::Hang:
        return std::make_unique<StateHandler_10>(lara);
    case LaraStateId::Reach:
        return std::make_unique<StateHandler_11>(lara);
    case LaraStateId::Unknown12:
        return std::make_unique<StateHandler_12>(lara);
    case LaraStateId::UnderwaterStop:
        return std::make_unique<StateHandler_13>(lara);
    case LaraStateId::JumpPrepare:
        return std::make_unique<StateHandler_15>(lara);
    case LaraStateId::WalkBackward:
        return std::make_unique<StateHandler_16>(lara);
    case LaraStateId::UnderwaterForward:
        return std::make_unique<StateHandler_17>(lara);
    case LaraStateId::UnderwaterInertia:
        return std::make_unique<StateHandler_18>(lara);
    case LaraStateId::Climbing:
        return std::make_unique<StateHandler_19>(lara);
    case LaraStateId::TurnFast:
        return std::make_unique<StateHandler_20>(lara);
    case LaraStateId::StepRight:
        return std::make_unique<StateHandler_21>(lara);
    case LaraStateId::StepLeft:
        return std::make_unique<StateHandler_22>(lara);
    case LaraStateId::RollBackward:
        return std::make_unique<StateHandler_23>(lara);
    case LaraStateId::SlideForward:
        return std::make_unique<StateHandler_24>(lara);
    case LaraStateId::JumpBack:
        return std::make_unique<StateHandler_25>(lara);
    case LaraStateId::JumpLeft:
        return std::make_unique<StateHandler_26>(lara);
    case LaraStateId::JumpRight:
        return std::make_unique<StateHandler_27>(lara);
    case LaraStateId::JumpUp:
        return std::make_unique<StateHandler_28>(lara);
    case LaraStateId::FallBackward:
        return std::make_unique<StateHandler_29>(lara);
    case LaraStateId::ShimmyLeft:
        return std::make_unique<StateHandler_30>(lara);
    case LaraStateId::ShimmyRight:
        return std::make_unique<StateHandler_31>(lara);
    case LaraStateId::SlideBackward:
        return std::make_unique<StateHandler_32>(lara);
    case LaraStateId::OnWaterStop:
        return std::make_unique<StateHandler_33>(lara);
    case LaraStateId::OnWaterForward:
        return std::make_unique<StateHandler_34>(lara);
    case LaraStateId::UnderwaterDiving:
        return std::make_unique<StateHandler_35>(lara);
    case LaraStateId::WaterDeath:
        return std::make_unique<StateHandler_44>(lara);
    case LaraStateId::RollForward:
        return std::make_unique<StateHandler_45>(lara);
    case LaraStateId::OnWaterBackward:
        return std::make_unique<StateHandler_47>(lara);
    case LaraStateId::OnWaterLeft:
        return std::make_unique<StateHandler_48>(lara);
    case LaraStateId::OnWaterRight:
        return std::make_unique<StateHandler_49>(lara);
    case LaraStateId::SwandiveBegin:
        return std::make_unique<StateHandler_52>(lara);
    case LaraStateId::SwandiveEnd:
        return std::make_unique<StateHandler_53>(lara);
    case LaraStateId::Handstand:
        return std::make_unique<StateHandler_54>(lara);
    case LaraStateId::OnWaterExit:
        return std::make_unique<StateHandler_55>(lara);
    default:
        BOOST_LOG_TRIVIAL(error) << "No state handler for state " << loader::toString(id);
        throw std::runtime_error("Unhandled state");
    }

    return nullptr;
}

std::unique_ptr<AbstractStateHandler> AbstractStateHandler::createWithRetainedAnimation(loader::LaraStateId id) const
{
    auto handler = create(id, m_stateHandler);
    handler->m_xRotationSpeed = m_xRotationSpeed;
    handler->m_yRotationSpeed = m_yRotationSpeed;
    handler->m_zRotationSpeed = m_zRotationSpeed;
    handler->m_xMovement = m_xMovement;
    handler->m_yMovement = m_yMovement;
    handler->m_zMovement = m_zMovement;
    return handler;
}

int AbstractStateHandler::getHealth() const noexcept
{
    return m_stateHandler.getHealth();
}

void AbstractStateHandler::setHealth(int h) noexcept
{
    m_stateHandler.setHealth(h);
}

void AbstractStateHandler::setAir(int a) noexcept
{
    m_stateHandler.setAir(a);
}

const InputState& AbstractStateHandler::getInputState() const noexcept
{
    return m_stateHandler.getInputState();
}

void AbstractStateHandler::setMovementAngle(int16_t angle) noexcept
{
    m_stateHandler.setMovementAngle(angle);
}

int16_t AbstractStateHandler::getMovementAngle() const noexcept
{
    return m_stateHandler.getMovementAngle();
}

void AbstractStateHandler::setFallSpeed(int spd)
{
    m_stateHandler.setFallSpeed(spd);
}

void AbstractStateHandler::setFallSpeedExact(float spd)
{
    m_stateHandler.setFallSpeedExact(spd);
}

const SpeedValue<int>& AbstractStateHandler::getFallSpeed() const noexcept
{
    return m_stateHandler.getFallSpeed();
}

bool AbstractStateHandler::isFalling() const noexcept
{
    return m_stateHandler.isFalling();
}

void AbstractStateHandler::setFalling(bool falling) noexcept
{
    m_stateHandler.setFalling(falling);
}

int AbstractStateHandler::getHandStatus() const noexcept
{
    return m_stateHandler.getHandStatus();
}

void AbstractStateHandler::setHandStatus(int status) noexcept
{
    m_stateHandler.setHandStatus(status);
}

uint32_t AbstractStateHandler::getCurrentFrame() const
{
    return m_stateHandler.getCurrentFrame();
}

loader::LaraStateId AbstractStateHandler::getCurrentAnimState() const
{
    return m_stateHandler.getCurrentAnimState();
}

void AbstractStateHandler::playAnimation(loader::AnimationId anim, const boost::optional<irr::u32>& firstFrame)
{
    m_stateHandler.playAnimation(anim, firstFrame);
}

const irr::core::vector3df& AbstractStateHandler::getRotation() const noexcept
{
    return m_stateHandler.getRotation();
}

void AbstractStateHandler::setHorizontalSpeed(int speed)
{
    m_stateHandler.setHorizontalSpeed(speed);
}

int AbstractStateHandler::getHorizontalSpeed() const
{
    return m_stateHandler.getHorizontalSpeed();
}

const loader::Level& AbstractStateHandler::getLevel() const
{
    return m_stateHandler.getLevel();
}

void AbstractStateHandler::placeOnFloor(const LaraState& state)
{
    m_stateHandler.placeOnFloor(state);
}

loader::TRCoordinates AbstractStateHandler::getPosition() const
{
    return m_stateHandler.getPosition();
}

const loader::ExactTRCoordinates& AbstractStateHandler::getExactPosition() const
{
    return m_stateHandler.getExactPosition();
}

void AbstractStateHandler::setPosition(const loader::ExactTRCoordinates& pos)
{
    m_stateHandler.setPosition(pos);
}

int AbstractStateHandler::getFloorHeight() const noexcept
{
    return m_stateHandler.getFloorHeight();
}

void AbstractStateHandler::setFloorHeight(int h) noexcept
{
    m_stateHandler.setFloorHeight(h);
}

void AbstractStateHandler::setYRotationSpeed(int spd)
{
    m_stateHandler.setYRotationSpeed(spd);
}

int AbstractStateHandler::getYRotationSpeed() const
{
    return m_stateHandler.getYRotationSpeed();
}

void AbstractStateHandler::subYRotationSpeed(int val, int limit)
{
    m_stateHandler.subYRotationSpeed(val, limit);
}

void AbstractStateHandler::addYRotationSpeed(int val, int limit)
{
    m_stateHandler.addYRotationSpeed(val, limit);
}

void AbstractStateHandler::setXRotation(int16_t x)
{
    m_stateHandler.setXRotation(x);
}

void AbstractStateHandler::setXRotationExact(float x)
{
    m_stateHandler.setXRotationExact(x);
}

void AbstractStateHandler::setYRotation(int16_t y)
{
    m_stateHandler.setYRotation(y);
}

void AbstractStateHandler::setZRotation(int16_t z)
{
    m_stateHandler.setZRotation(z);
}

void AbstractStateHandler::setZRotationExact(float z)
{
    m_stateHandler.setZRotationExact(z);
}

void AbstractStateHandler::setFallSpeedOverride(int v)
{
    m_stateHandler.setFallSpeedOverride(v);
}

void AbstractStateHandler::dampenHorizontalSpeed(int nom, int den)
{
    m_stateHandler.dampenHorizontalSpeed(nom, den);
}

int16_t AbstractStateHandler::getCurrentSlideAngle() const noexcept
{
    return m_stateHandler.getCurrentSlideAngle();
}

void AbstractStateHandler::setCurrentSlideAngle(int16_t a) noexcept
{
    m_stateHandler.setCurrentSlideAngle(a);
}

void AbstractStateHandler::setTargetState(loader::LaraStateId state)
{
    m_stateHandler.setTargetState(state);
}

loader::LaraStateId AbstractStateHandler::getTargetState() const
{
    return m_stateHandler.getTargetState();
}

bool AbstractStateHandler::canClimbOnto(util::Axis axis) const
{
    auto pos = getPosition();
    switch( axis )
    {
    case util::Axis::PosZ: pos.Z += 256;
        break;
    case util::Axis::PosX: pos.X += 256;
        break;
    case util::Axis::NegZ: pos.Z -= 256;
        break;
    case util::Axis::NegX: pos.X -= 256;
        break;
    }

    auto sector = getLevel().findSectorForPosition(pos, getLevel().m_camera->getCurrentRoom());
    HeightInfo floor = HeightInfo::fromFloor(sector, pos, getLevel().m_camera);
    HeightInfo ceil = HeightInfo::fromCeiling(sector, pos, getLevel().m_camera);
    return floor.distance != -loader::HeightLimit && floor.distance - pos.Y > 0 && ceil.distance - pos.Y < -400;
}

std::unique_ptr<AbstractStateHandler> AbstractStateHandler::tryReach(LaraState& state)
{
    if( state.axisCollisions != LaraState::AxisColl_InsufficientFrontSpace || !getInputState().action || getHandStatus() != 0 )
        return nullptr;

    if( std::abs(state.frontLeft.floor.distance - state.frontRight.floor.distance) >= core::MaxGrabableGradient )
        return nullptr;

    if( state.front.ceiling.distance > 0 || state.current.ceiling.distance > -core::ClimbLimit2ClickMin || state.current.floor.distance < 200 )
        return nullptr;

    getLara()->updateAbsolutePosition();

    const auto bbox = getBoundingBox();
    auto spaceToReach = state.front.floor.distance - bbox.MinEdge.Y;

    if( spaceToReach < 0 && spaceToReach + getFallSpeed().getExact() < 0 )
        return nullptr;
    if( spaceToReach > 0 && spaceToReach + getFallSpeed().getExact() > 0 )
        return nullptr;

    auto alignedRotation = util::alignRotation(getRotation().Y, util::degToAu(35));
    if( !alignedRotation )
        return nullptr;

    if( canClimbOnto(*util::axisFromAngle(getRotation().Y, util::degToAu(35))) )
        playAnimation(loader::AnimationId::OSCILLATE_HANG_ON, 3974);
    else
        playAnimation(loader::AnimationId::HANG_IDLE, 1493);

    setTargetState(LaraStateId::Hang);
    setPosition(getExactPosition() + loader::ExactTRCoordinates(state.collisionFeedback.X,spaceToReach,state.collisionFeedback.Z));
    setHorizontalSpeed(0);
    setYRotation(*alignedRotation);
    setFalling(false);
    setFallSpeed(0);
    setHandStatus(1);
    return createWithRetainedAnimation(LaraStateId::Hang);
}

std::unique_ptr<AbstractStateHandler> AbstractStateHandler::tryStopOnFloor(LaraState& state)
{
    if( state.axisCollisions != LaraState::AxisColl_HeadInCeiling && state.axisCollisions != LaraState::AxisColl_CeilingTooLow )
        return nullptr;

    setPosition(state.position);

    setTargetState(LaraStateId::Stop);
    playAnimation(loader::AnimationId::STAY_SOLID, 185);
    setHorizontalSpeed(0);
    setFallSpeed(0);
    setFalling(false);
    return createWithRetainedAnimation(LaraStateId::Stop);
}

std::unique_ptr<AbstractStateHandler> AbstractStateHandler::tryClimb(LaraState& state)
{
    if( state.axisCollisions != LaraState::AxisColl_InsufficientFrontSpace || !getInputState().jump || getHandStatus() != 0 )
        return nullptr;

    const auto floorGradient = std::abs(state.frontLeft.floor.distance - state.frontRight.floor.distance);
    if( floorGradient >= core::MaxGrabableGradient )
        return nullptr;

    //! @todo MAGICK +/- 30 degrees
    auto alignedRotation = util::alignRotation(getRotation().Y, util::degToAu(30));
    if( !alignedRotation )
        return nullptr;

    const auto climbHeight = state.front.floor.distance;
    std::unique_ptr<AbstractStateHandler> nextHandler = nullptr;
    if( climbHeight >= -core::ClimbLimit2ClickMax && climbHeight <= -core::ClimbLimit2ClickMin )
    {
        if( climbHeight < state.front.ceiling.distance
            || state.frontLeft.floor.distance < state.frontLeft.ceiling.distance
            || state.frontRight.floor.distance < state.frontRight.ceiling.distance )
            return nullptr;

        setTargetState(LaraStateId::Stop);
        nextHandler = createWithRetainedAnimation(LaraStateId::Climbing);
        playAnimation(loader::AnimationId::CLIMB_2CLICK, 759);
        m_yMovement = 2 * loader::QuarterSectorSize + climbHeight;
        setHandStatus(1);
    }
    else if( climbHeight >= -core::ClimbLimit3ClickMax && climbHeight <= -core::ClimbLimit2ClickMax )
    {
        if( state.front.floor.distance < state.front.ceiling.distance
            || state.frontLeft.floor.distance < state.frontLeft.ceiling.distance
            || state.frontRight.floor.distance < state.frontRight.ceiling.distance )
            return nullptr;

        setTargetState(LaraStateId::Stop);
        nextHandler = createWithRetainedAnimation(LaraStateId::Climbing);
        playAnimation(loader::AnimationId::CLIMB_3CLICK, 614);
        m_yMovement = 3 * loader::QuarterSectorSize + climbHeight;
        setHandStatus(1);
    }
    else
    {
        if( climbHeight < -core::JumpReachableHeight || climbHeight > -core::ClimbLimit3ClickMax )
            return nullptr;

        setTargetState(LaraStateId::JumpUp);
        nextHandler = createWithRetainedAnimation(LaraStateId::Stop);
        playAnimation(loader::AnimationId::STAY_SOLID, 185);
        setFallSpeedOverride(-static_cast<int>(std::sqrt(-12 * (climbHeight + 800) + 3)));
        auto tmp = getStateHandler().processAnimCommands();
        if( tmp )
            nextHandler = std::move(tmp);
    }

    setYRotation(*alignedRotation);
    applyCollisionFeedback(state);

    BOOST_ASSERT(nextHandler != nullptr);
    return nextHandler;
}

void AbstractStateHandler::applyCollisionFeedback(LaraState& state)
{
    setPosition(getExactPosition() + state.collisionFeedback);
    state.collisionFeedback = {0,0,0};
}

std::unique_ptr<AbstractStateHandler> AbstractStateHandler::checkWallCollision(LaraState& state)
{
    if( state.axisCollisions == LaraState::AxisColl_InsufficientFrontSpace || state.axisCollisions == LaraState::AxisColl_BumpHead )
    {
        applyCollisionFeedback(state);
        setTargetState(LaraStateId::Stop);
        setFalling(false);
        setHorizontalSpeed(0);
        return createWithRetainedAnimation(LaraStateId::Stop);
    }

    if( state.axisCollisions == LaraState::AxisColl_FrontLeftBump )
    {
        applyCollisionFeedback(state);
        m_yRotationSpeed = 910;
    }
    else if( state.axisCollisions == LaraState::AxisColl_FrontRightBump )
    {
        applyCollisionFeedback(state);
        m_yRotationSpeed = -910;
    }

    return nullptr;
}

bool AbstractStateHandler::tryStartSlide(LaraState& state, std::unique_ptr<AbstractStateHandler>& nextHandler)
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
        if( getCurrentAnimState() != LaraStateId::SlideBackward || targetAngle != getCurrentSlideAngle() )
        {
            playAnimation(loader::AnimationId::START_SLIDE_BACKWARD, 1677);
            setTargetState(LaraStateId::SlideBackward);
            nextHandler = createWithRetainedAnimation(LaraStateId::SlideBackward);
            setMovementAngle(targetAngle);
            setCurrentSlideAngle(targetAngle);
            setYRotation(targetAngle + util::degToAu(180));
        }
    }
    else if( getCurrentAnimState() != LaraStateId::SlideForward || targetAngle != getCurrentSlideAngle() )
    {
        playAnimation(loader::AnimationId::SLIDE_FORWARD, 1133);
        setTargetState(LaraStateId::SlideForward);
        nextHandler = createWithRetainedAnimation(LaraStateId::SlideForward);
        setMovementAngle(targetAngle);
        setCurrentSlideAngle(targetAngle);
        setYRotation(targetAngle);
    }
    return true;
}

std::unique_ptr<AbstractStateHandler> AbstractStateHandler::tryGrabEdge(LaraState& state)
{
    if( state.axisCollisions != LaraState::AxisColl_InsufficientFrontSpace || !getInputState().action || getHandStatus() != 0 )
        return nullptr;

    const auto floorGradient = std::abs(state.frontLeft.floor.distance - state.frontRight.floor.distance);
    if( floorGradient >= core::MaxGrabableGradient )
        return nullptr;

    if( state.front.ceiling.distance > 0 || state.current.ceiling.distance > -core::ClimbLimit2ClickMin )
        return nullptr;

    getLara()->updateAbsolutePosition();

    auto bbox = getBoundingBox();
    auto spaceToReach = state.front.floor.distance - bbox.MinEdge.Y;

    if( spaceToReach < 0 && spaceToReach + getFallSpeed().getExact() < 0 )
        return nullptr;
    if( spaceToReach > 0 && spaceToReach + getFallSpeed().getExact() > 0 )
        return nullptr;

    auto alignedRotation = util::alignRotation(getRotation().Y, util::degToAu(35));
    if( !alignedRotation )
        return nullptr;

    setTargetState(LaraStateId::Hang);
    playAnimation(loader::AnimationId::HANG_IDLE, 1505);
    bbox = getBoundingBox();
    spaceToReach = state.front.floor.distance - bbox.MinEdge.Y;

    setPosition(getExactPosition() + loader::ExactTRCoordinates(0,spaceToReach,0));
    applyCollisionFeedback(state);
    setHorizontalSpeed(0);
    setFallSpeed(0);
    setFalling(false);
    setHandStatus(1);
    setYRotation(*alignedRotation);

    return createWithRetainedAnimation(LaraStateId::Hang);
}

int AbstractStateHandler::getRelativeHeightAtDirection(int16_t angle, int dist) const
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

std::unique_ptr<AbstractStateHandler> AbstractStateHandler::commonJumpHandling(LaraState& state)
{
    state.neededFloorDistanceBottom = loader::HeightLimit;
    state.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
    state.neededCeilingDistance = 192;
    state.yAngle = getMovementAngle();
    state.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
    auto nextHandler = checkJumpWallSmash(state);
    if( getFallSpeed().get() <= 0 || state.current.floor.distance > 0 )
        return nextHandler;

    if( applyLandingDamage() )
        setTargetState(LaraStateId::Death);
    else
        setTargetState(LaraStateId::Stop);
    setFallSpeed(0);
    placeOnFloor(state);
    setFalling(false);

    return nextHandler;
}

std::unique_ptr<AbstractStateHandler> AbstractStateHandler::commonSlideHandling(LaraState& state)
{
    state.neededFloorDistanceBottom = loader::HeightLimit;
    state.neededFloorDistanceTop = -loader::QuarterSectorSize * 2;
    state.neededCeilingDistance = 0;
    state.yAngle = getMovementAngle();
    state.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);

    if( auto nextHandler = tryStopOnFloor(state) )
        return nextHandler;

    auto nextHandler = checkWallCollision(state);
    if( state.current.floor.distance <= 200 )
    {
        tryStartSlide(state, nextHandler);
        placeOnFloor(state);
        const auto absSlantX = std::abs(state.floorSlantX);
        const auto absSlantZ = std::abs(state.floorSlantZ);
        if( absSlantX <= 2 && absSlantZ <= 2 )
        {
            setTargetState(LaraStateId::Stop);
        }
        return nextHandler;
    }

    if( getCurrentAnimState() == LaraStateId::SlideForward )
    {
        playAnimation(loader::AnimationId::FREE_FALL_FORWARD, 492);
        setTargetState(LaraStateId::JumpForward);
        nextHandler = createWithRetainedAnimation(LaraStateId::JumpForward);
    }
    else
    {
        playAnimation(loader::AnimationId::FREE_FALL_BACK, 1473);
        setTargetState(LaraStateId::FallBackward);
        nextHandler = createWithRetainedAnimation(LaraStateId::FallBackward);
    }

    setFallSpeed(0);
    setFalling(true);

    return nextHandler;
}

std::unique_ptr<AbstractStateHandler> AbstractStateHandler::commonEdgeHangHandling(LaraState& state)
{
    state.neededFloorDistanceBottom = loader::HeightLimit;
    state.neededFloorDistanceTop = -loader::HeightLimit;
    state.neededCeilingDistance = 0;
    state.yAngle = getMovementAngle();
    state.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
    const bool frobbel = state.front.floor.distance < 200;
    setFallSpeed(0);
    setFalling(false);
    setMovementAngle(static_cast<int16_t>(getRotation().Y));
    const auto axis = *util::axisFromAngle(getMovementAngle(), util::degToAu(45));
    switch( axis )
    {
    case util::Axis::PosZ:
        setPosition(getExactPosition() + loader::ExactTRCoordinates(0, 0, 2));
        break;
    case util::Axis::PosX:
        setPosition(getExactPosition() + loader::ExactTRCoordinates(2, 0, 0));
        break;
    case util::Axis::NegZ:
        setPosition(getExactPosition() - loader::ExactTRCoordinates(0, 0, 2));
        break;
    case util::Axis::NegX:
        setPosition(getExactPosition() - loader::ExactTRCoordinates(2, 0, 0));
        break;
    }

    state.neededFloorDistanceBottom = loader::HeightLimit;
    state.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
    state.neededCeilingDistance = 0;
    state.yAngle = getMovementAngle();
    state.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
    if( !getInputState().action || getHealth() <= 0 )
    {
        setTargetState(LaraStateId::JumpUp);
        playAnimation(loader::AnimationId::TRY_HANG_VERTICAL, 448);
        setHandStatus(0);
        const auto bbox = getBoundingBox();
        const auto hangDistance = state.front.floor.distance - bbox.MinEdge.Y + 2;
        setPosition(getExactPosition() + loader::ExactTRCoordinates(state.collisionFeedback.X, hangDistance, state.collisionFeedback.Z));
        setHorizontalSpeed(2);
        setFallSpeed(1);
        setFalling(true);
        return createWithRetainedAnimation(LaraStateId::JumpUp);
    }

    auto gradient = std::abs(state.frontLeft.floor.distance - state.frontRight.floor.distance);
    if( gradient >= core::MaxGrabableGradient || state.current.ceiling.distance >= 0 || state.axisCollisions != LaraState::AxisColl_InsufficientFrontSpace || frobbel )
    {
        setPosition(state.position);
        if( getCurrentAnimState() != LaraStateId::ShimmyLeft && getCurrentAnimState() != LaraStateId::ShimmyRight )
        {
            return nullptr;
        }

        setTargetState(LaraStateId::Hang);
        playAnimation(loader::AnimationId::HANG_IDLE, 1514);
        return createWithRetainedAnimation(LaraStateId::Hang);
    }

    switch( axis )
    {
    case util::Axis::PosZ:
    case util::Axis::NegZ:
        setPosition(getExactPosition() + loader::ExactTRCoordinates(0, 0, state.collisionFeedback.Z));
        break;
    case util::Axis::PosX:
    case util::Axis::NegX:
        setPosition(getExactPosition() + loader::ExactTRCoordinates(state.collisionFeedback.X, 0, 0));
        break;
    }

    const auto bbox = getBoundingBox();
    const auto spaceToReach = state.front.floor.distance - bbox.MinEdge.Y;

    if(spaceToReach >= -loader::QuarterSectorSize && spaceToReach <= loader::QuarterSectorSize )
        setPosition(getExactPosition() + loader::ExactTRCoordinates(0, spaceToReach, 0));
    return nullptr;
}

bool AbstractStateHandler::applyLandingDamage()
{
    auto sector = getLevel().findSectorForPosition(getPosition(), getLevel().m_camera->getCurrentRoom());
    HeightInfo h = HeightInfo::fromFloor(sector, getPosition() - loader::TRCoordinates{0, core::ScalpHeight, 0}, getLevel().m_camera);
    setFloorHeight(h.distance);
    getStateHandler().handleTriggers(h.lastTriggerOrKill, false);
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

irr::scene::ISceneNode* AbstractStateHandler::getLara()
{
    return m_stateHandler.getLara();
}

irr::core::aabbox3di AbstractStateHandler::getBoundingBox() const
{
    return m_stateHandler.getBoundingBox();
}

void AbstractStateHandler::addSwimToDiveKeypressDuration(int ms) noexcept
{
    m_stateHandler.addSwimToDiveKeypressDuration(ms);
}

void AbstractStateHandler::setSwimToDiveKeypressDuration(int ms) noexcept
{
    m_stateHandler.setSwimToDiveKeypressDuration(ms);
}

int AbstractStateHandler::getSwimToDiveKeypressDuration() const noexcept
{
    return m_stateHandler.getSwimToDiveKeypressDuration();
}

void AbstractStateHandler::setUnderwaterState(UnderwaterState u) noexcept
{
    m_stateHandler.setUnderwaterState(u);
}

void AbstractStateHandler::jumpAgainstWall(LaraState& state)
{
    applyCollisionFeedback(state);
    if( state.axisCollisions == LaraState::AxisColl_FrontLeftBump )
        m_yRotationSpeed = 910;
    else if( state.axisCollisions == LaraState::AxisColl_FrontRightBump )
        m_yRotationSpeed = -910;
    else if( state.axisCollisions == LaraState::AxisColl_HeadInCeiling )
    {
        if( getFallSpeed().get() <= 0 )
            setFallSpeed(1);
    }
    else if( state.axisCollisions == LaraState::AxisColl_CeilingTooLow )
    {
        m_xMovement = 100 * std::sin(util::auToRad(getRotation().Y));
        m_zMovement = 100 * std::cos(util::auToRad(getRotation().Y));
        setHorizontalSpeed(0);
        state.current.floor.distance = 0;
        if( getFallSpeed().get() < 0 )
            setFallSpeed(16);
    }
}

std::unique_ptr<AbstractStateHandler> AbstractStateHandler::checkJumpWallSmash(LaraState& state)
{
    applyCollisionFeedback(state);

    if( state.axisCollisions == LaraState::AxisColl_None )
        return nullptr;

    if( state.axisCollisions == LaraState::AxisColl_InsufficientFrontSpace || state.axisCollisions == LaraState::AxisColl_BumpHead )
    {
        setTargetState(LaraStateId::FreeFall);
        //! @todo Check formula
        setHorizontalSpeed(getHorizontalSpeed() / 5);
        setMovementAngle(getMovementAngle() - util::degToAu(180));
        playAnimation(loader::AnimationId::SMASH_JUMP, 481);
        if( getFallSpeed().get() <= 0 )
            setFallSpeed(1);
        return createWithRetainedAnimation(LaraStateId::FreeFall);
    }

    if( state.axisCollisions == LaraState::AxisColl_FrontLeftBump )
    {
        m_yRotationSpeed = 910;
        return nullptr;
    }
    else if( state.axisCollisions == LaraState::AxisColl_FrontRightBump )
    {
        m_yRotationSpeed = -910;
        return nullptr;
    }

    if( state.axisCollisions == LaraState::AxisColl_CeilingTooLow )
    {
        m_xMovement = 100 * std::sin(util::auToRad(state.yAngle));
        m_zMovement = 100 * std::cos(util::auToRad(state.yAngle));
        setHorizontalSpeed(0);
        state.current.floor.distance = 0;
        if( getFallSpeed().get() <= 0 )
            setFallSpeed(16);
    }

    if( state.axisCollisions == LaraState::AxisColl_HeadInCeiling && getFallSpeed().get() <= 0 )
        setFallSpeed(1);

    return nullptr;
}
