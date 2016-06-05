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

    std::unique_ptr<AbstractStateHandler> handleInput(LaraState& /*state*/) override
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
            nextHandler = create(LaraStateId::JumpForward);
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

    std::unique_ptr<AbstractStateHandler> handleInput(LaraState& /*state*/) override
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
            return create(LaraStateId::RollForward);
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
                nextHandler = create(LaraStateId::Unknown12);
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
            return create(LaraStateId::JumpForward);
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

    std::unique_ptr<AbstractStateHandler> handleInput(LaraState& state) override
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
            return create(LaraStateId::RollForward);
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
                create(LaraStateId::WalkForward)->handleInput(state);
            else
                create(LaraStateId::RunForward)->handleInput(state);
        }
        else if( getInputState().zMovement == AxisMovement::Backward )
        {
            if( getInputState().moveSlow )
                AbstractStateHandler::create(LaraStateId::WalkBackward, getStateHandler())->handleInput(state);
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

    std::unique_ptr<AbstractStateHandler> handleInput(LaraState& /*state*/) override
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

    std::unique_ptr<AbstractStateHandler> handleInput(LaraState& /*state*/) override
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

    std::unique_ptr<AbstractStateHandler> handleInput(LaraState& /*state*/) override
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
            return create(LaraStateId::FallBackward);
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
        return create(LaraStateId::JumpForward);
    }
};

class StateHandler_6 final : public StateHandler_TurnSlow
{
public:
    explicit StateHandler_6(LaraStateHandler& lara)
        : StateHandler_TurnSlow(lara)
    {
    }

    std::unique_ptr<AbstractStateHandler> handleInput(LaraState& /*state*/) override
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

    std::unique_ptr<AbstractStateHandler> handleInput(LaraState& /*state*/) override
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

    std::unique_ptr<AbstractStateHandler> handleInput(LaraState& state) override
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

    std::unique_ptr<AbstractStateHandler> handleInput(LaraState& /*state*/) override
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
            nextHandler = create(LaraStateId::Stop);
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

    std::unique_ptr<AbstractStateHandler> handleInput(LaraState& state) override
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

    void animateImpl(LaraState& state, int deltaTimeMs) override
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

    std::unique_ptr<AbstractStateHandler> handleInput(LaraState& /*state*/) override
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

    std::unique_ptr<AbstractStateHandler> handleInput(LaraState& /*state*/) override
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

class StateHandler_15 final : public AbstractStateHandler
{
public:
    explicit StateHandler_15(LaraStateHandler& lara)
        : AbstractStateHandler(lara)
    {
    }

    std::unique_ptr<AbstractStateHandler> handleInput(LaraState& /*state*/) override
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

        return create(LaraStateId::Stop);
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

    std::unique_ptr<AbstractStateHandler> handleInput(LaraState& /*state*/) override
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

class StateHandler_19 final : public AbstractStateHandler
{
public:
    explicit StateHandler_19(LaraStateHandler& lara)
        : AbstractStateHandler(lara)
    {
    }

    std::unique_ptr<AbstractStateHandler> handleInput(LaraState& state) override
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

    std::unique_ptr<AbstractStateHandler> handleInput(LaraState& /*state*/) override
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

class StateHandler_23 final : public AbstractStateHandler
{
public:
    explicit StateHandler_23(LaraStateHandler& lara)
        : AbstractStateHandler(lara)
    {
    }

    std::unique_ptr<AbstractStateHandler> handleInput(LaraState& /*state*/) override
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

        return create(LaraStateId::FallBackward);
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

    std::unique_ptr<AbstractStateHandler> handleInput(LaraState& /*state*/) override
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

    std::unique_ptr<AbstractStateHandler> handleInput(LaraState& /*state*/) override
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

    std::unique_ptr<AbstractStateHandler> handleInput(LaraState& /*state*/) override
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

    std::unique_ptr<AbstractStateHandler> handleInput(LaraState& /*state*/) override
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

    std::unique_ptr<AbstractStateHandler> handleInput(LaraState& /*state*/) override
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

    std::unique_ptr<AbstractStateHandler> handleInput(LaraState& /*state*/) override
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

    std::unique_ptr<AbstractStateHandler> handleInput(LaraState& state) override
    {
        state.frobbelFlags &= ~(LaraState::FrobbelFlag08 | LaraState::FrobbelFlag10);
        //! @todo set camera rotation (x,y)=(-10920,0)
        if(getInputState().xMovement != AxisMovement::Left && getInputState().stepMovement != AxisMovement::Left)
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

    void animateImpl(LaraState& state, int deltaTimeMs) override
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

    std::unique_ptr<AbstractStateHandler> handleInput(LaraState& state) override
    {
        state.frobbelFlags &= ~(LaraState::FrobbelFlag08 | LaraState::FrobbelFlag10);
        //! @todo set camera rotation (x,y)=(-10920,0)
        if(getInputState().xMovement != AxisMovement::Right && getInputState().stepMovement != AxisMovement::Right)
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

    void animateImpl(LaraState& state, int deltaTimeMs) override
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

    std::unique_ptr<AbstractStateHandler> handleInput(LaraState& /*state*/) override
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

class StateHandler_45 final : public AbstractStateHandler
{
public:
    explicit StateHandler_45(LaraStateHandler& lara)
        : AbstractStateHandler(lara)
    {
    }

    std::unique_ptr<AbstractStateHandler> handleInput(LaraState& /*state*/) override
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

        return create(LaraStateId::JumpForward);
    }

    loader::LaraStateId getId() const noexcept override
    {
        return LaraStateId::RollForward;
    }
};

class StateHandler_52 final : public AbstractStateHandler
{
public:
    explicit StateHandler_52(LaraStateHandler& lara)
        : AbstractStateHandler(lara)
    {
    }

    std::unique_ptr<AbstractStateHandler> handleInput(LaraState& state) override
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

    std::unique_ptr<AbstractStateHandler> handleInput(LaraState& state) override
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

void AbstractStateHandler::animate(LaraState& state, int deltaTimeMs)
{
    animateImpl(state, deltaTimeMs);

    m_stateHandler.addYRotation(m_yRotationSpeed.getScaledExact(deltaTimeMs));
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
    case LaraStateId::JumpPrepare:
        return std::make_unique<StateHandler_15>(lara);
    case LaraStateId::WalkBackward:
        return std::make_unique<StateHandler_16>(lara);
    case LaraStateId::Climbing:
        return std::make_unique<StateHandler_19>(lara);
    case LaraStateId::TurnFast:
        return std::make_unique<StateHandler_20>(lara);
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
    case LaraStateId::RollForward:
        return std::make_unique<StateHandler_45>(lara);
    case LaraStateId::SwandiveBegin:
        return std::make_unique<StateHandler_52>(lara);
    case LaraStateId::SwandiveEnd:
        return std::make_unique<StateHandler_53>(lara);
    default:
        BOOST_LOG_TRIVIAL(error) << "No state handler for state " << loader::toString(id);
        throw std::runtime_error("Unhandled state");
    }

    return nullptr;
}

std::unique_ptr<AbstractStateHandler> AbstractStateHandler::create(loader::LaraStateId id) const
{
    auto handler = create(id, m_stateHandler);
    handler->m_yRotationSpeed = m_yRotationSpeed;
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

bool AbstractStateHandler::canClimbOnto(int16_t angle) const
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

std::unique_ptr<AbstractStateHandler> AbstractStateHandler::tryReach(LaraState& state)
{
    if( state.axisCollisions != LaraState::AxisColl_InsufficientFrontSpace || !getInputState().action || getHandStatus() != 0 )
        return nullptr;

    if( std::abs(state.frontLeft.floor.distance - state.frontRight.floor.distance) >= core::MaxGrabableGradient )
        return nullptr;

    if( state.front.ceiling.distance > 0 || state.current.ceiling.distance > -core::ClimbLimit2ClickMin || state.current.floor.distance < 200 )
        return nullptr;

    getLara()->updateAbsolutePosition();

    const auto spaceToReach = state.front.floor.distance - (loader::TRCoordinates(getLara()->getTransformedBoundingBox().MaxEdge).Y - getPosition().Y);
    BOOST_LOG_TRIVIAL(debug) << "spaceToReach=" << spaceToReach << ", fallSpeed=" << getFallSpeed().getExact();
    if( spaceToReach < 0 && spaceToReach + getFallSpeed().getExact() < 0 )
        return nullptr;
    if( spaceToReach > 0 && spaceToReach + getFallSpeed().getExact() > 0 )
        return nullptr;

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
        return nullptr;

    if( canClimbOnto(rot) )
        playAnimation(loader::AnimationId::OSCILLATE_HANG_ON, 3974);
    else
        playAnimation(loader::AnimationId::HANG_IDLE, 1493);

    setTargetState(LaraStateId::Hang);
    m_yMovement = spaceToReach;
    setHorizontalSpeed(0);
    applyCollisionFeedback(state);
    setYRotation(rot);
    setFalling(false);
    setFallSpeed(0);
    setHandStatus(1);
    return create(LaraStateId::Hang);
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
    return create(LaraStateId::Stop);
}

std::unique_ptr<AbstractStateHandler> AbstractStateHandler::tryClimb(LaraState& state)
{
    if( state.axisCollisions != LaraState::AxisColl_InsufficientFrontSpace || !getInputState().jump || getHandStatus() != 0 )
        return nullptr;

    const auto floorGradient = std::abs(state.frontLeft.floor.distance - state.frontRight.floor.distance);
    if( floorGradient >= core::MaxGrabableGradient )
        return nullptr;

    int16_t alignedRotation = static_cast<int16_t>(getRotation().Y);
    //! @todo MAGICK +/- 30 degrees
    if(alignedRotation >= util::degToAu(0-30) && alignedRotation <= util::degToAu(0+30) )
        alignedRotation = util::degToAu(0);
    else if(alignedRotation >= util::degToAu(90-30) && alignedRotation <= util::degToAu(90+30) )
        alignedRotation = util::degToAu(90);
    else if(alignedRotation >= util::degToAu(180-30) && alignedRotation <= util::degToAu(-180-30) )
        alignedRotation = util::degToAu(180);
    else if(alignedRotation >= util::degToAu(-90-30) && alignedRotation <= util::degToAu(-90+30) )
        alignedRotation = util::degToAu(-90);
    else
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
        nextHandler = create(LaraStateId::Climbing);
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
        nextHandler = create(LaraStateId::Climbing);
        playAnimation(loader::AnimationId::CLIMB_3CLICK, 614);
        m_yMovement = 3 * loader::QuarterSectorSize + climbHeight;
        setHandStatus(1);
    }
    else
    {
        if( climbHeight < -core::JumpReachableHeight || climbHeight > -core::ClimbLimit3ClickMax )
            return nullptr;

        setTargetState(LaraStateId::JumpUp);
        nextHandler = create(LaraStateId::Stop);
        playAnimation(loader::AnimationId::STAY_SOLID, 185);
        setFallSpeedOverride(-static_cast<int>(std::sqrt(-12 * (climbHeight + 800) + 3)));
        auto tmp = getStateHandler().processAnimCommands();
        if( tmp )
            nextHandler = std::move(tmp);
    }

    setYRotation(alignedRotation);
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
        return create(LaraStateId::Stop);
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
            nextHandler = create(LaraStateId::SlideBackward);
            setMovementAngle(targetAngle);
            setCurrentSlideAngle(targetAngle);
            setYRotation(targetAngle + util::degToAu(180));
        }
    }
    else if( getCurrentAnimState() != LaraStateId::SlideForward || targetAngle != getCurrentSlideAngle() )
    {
        playAnimation(loader::AnimationId::SLIDE_FORWARD, 1133);
        setTargetState(LaraStateId::SlideForward);
        nextHandler = create(LaraStateId::SlideForward);
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

    const auto spaceToReach = state.front.ceiling.distance - (loader::TRCoordinates(getLara()->getTransformedBoundingBox().MaxEdge).Y - getPosition().Y);

    if( spaceToReach < 0 && spaceToReach + getFallSpeed().getExact() < 0 )
        return nullptr;
    if( spaceToReach > 0 && spaceToReach + getFallSpeed().getExact() > 0 )
        return nullptr;

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
        return nullptr;

    setTargetState(LaraStateId::Hang);
    playAnimation(loader::AnimationId::HANG_IDLE, 1505);

    m_yMovement = spaceToReach;
    applyCollisionFeedback(state);
    setHorizontalSpeed(0);
    setFallSpeed(0);
    setFalling(false);
    setHandStatus(1);
    setYRotation(rot);

    return create(LaraStateId::Hang);
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
        nextHandler = create(LaraStateId::JumpForward);
    }
    else
    {
        playAnimation(loader::AnimationId::FREE_FALL_BACK, 1473);
        setTargetState(LaraStateId::FallBackward);
        nextHandler = create(LaraStateId::FallBackward);
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
    const auto axis = static_cast<Axis>(static_cast<irr::u16>(getMovementAngle() + util::degToAu(45)) / util::degToAu(90));
    switch( axis )
    {
    case Axis::PosZ:
        setPosition(getExactPosition() + loader::ExactTRCoordinates(0, 0, 2));
        break;
    case Axis::PosX:
        setPosition(getExactPosition() + loader::ExactTRCoordinates(2, 0, 0));
        break;
    case Axis::NegZ:
        setPosition(getExactPosition() - loader::ExactTRCoordinates(0, 0, 2));
        break;
    case Axis::NegX:
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
        const auto hangDistance = state.front.floor.distance - (loader::TRCoordinates(getLara()->getTransformedBoundingBox().MaxEdge).Y - getPosition().Y) + 2;
        setPosition(getExactPosition() + loader::ExactTRCoordinates(state.collisionFeedback.X, hangDistance, state.collisionFeedback.Z));
        setHorizontalSpeed(2);
        setFallSpeed(1);
        setFalling(true);
        return create(LaraStateId::JumpUp);
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
        return create(LaraStateId::Hang);
    }

    switch( axis )
    {
    case Axis::PosZ:
    case Axis::NegZ:
        setPosition(getExactPosition() + loader::ExactTRCoordinates(0, 0, state.collisionFeedback.Z));
        break;
    case Axis::PosX:
    case Axis::NegX:
        setPosition(getExactPosition() + loader::ExactTRCoordinates(state.collisionFeedback.X, 0, 0));
        break;
    }
    const auto spaceToReach = state.front.floor.distance - (loader::TRCoordinates(getLara()->getTransformedBoundingBox().MaxEdge).Y - getPosition().Y);
    if( spaceToReach >= -loader::QuarterSectorSize && spaceToReach <= loader::QuarterSectorSize )
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
        return create(LaraStateId::FreeFall);
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
