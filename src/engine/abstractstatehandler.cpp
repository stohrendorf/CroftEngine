#include "abstractstatehandler.h"

#include "core/magic.h"
#include "inputstate.h"
#include "laracontroller.h"
#include "collisioninfo.h"

namespace engine
{
    namespace
    {
        const core::Angle FreeLookMouseMovementScale{ 50_deg };
    }

    using LaraStateId = loader::LaraStateId;

    class StateHandler_Standing : public AbstractStateHandler
    {
    protected:
        explicit StateHandler_Standing(LaraController& lara)
            : AbstractStateHandler(lara)
        {
        }

    public:
        std::unique_ptr<AbstractStateHandler> postprocessFrame(CollisionInfo& collisionInfo) override final
        {
            setFallSpeed(core::makeInterpolatedValue(0.0f));
            setFalling(false);
            collisionInfo.yAngle = getRotation().Y;
            setMovementAngle(collisionInfo.yAngle);
            collisionInfo.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
            collisionInfo.neededFloorDistanceBottom = core::ClimbLimit2ClickMin;
            collisionInfo.neededCeilingDistance = 0;
            collisionInfo.frobbelFlags |= CollisionInfo::FrobbelFlag_UnpassableSteepUpslant | CollisionInfo::FrobbelFlag_UnwalkableSteepFloor;
            collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);

            auto nextHandler = stopIfCeilingBlocked(collisionInfo);
            if( nextHandler )
                return nextHandler;

            if( collisionInfo.current.floor.distance <= 100 )
            {
                if( !tryStartSlide(collisionInfo, nextHandler) )
                {
                    applyCollisionFeedback(collisionInfo);
                    placeOnFloor(collisionInfo);
                }
                return nextHandler;
            }

            playAnimation(loader::AnimationId::FREE_FALL_FORWARD, 492);
            setTargetState(LaraStateId::JumpForward);
            setFallSpeed(core::makeInterpolatedValue(0.0f));
            setFalling(true);
            return createWithRetainedAnimation(LaraStateId::JumpForward);
        }
    };

    class StateHandler_0 final : public AbstractStateHandler
    {
    public:

        explicit StateHandler_0(LaraController& lara)
            : AbstractStateHandler(lara)
        {
        }

        std::unique_ptr<AbstractStateHandler> handleInputImpl(CollisionInfo& /*collisionInfo*/) override
        {
            if( getHealth() <= 0 )
            {
                setTargetState(LaraStateId::Stop);
                return nullptr;
            }

            if( getLevel().m_inputHandler->getInputState().zMovement == AxisMovement::Forward )
            {
                if( getLevel().m_inputHandler->getInputState().moveSlow )
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

        void animateImpl(CollisionInfo& /*collisionInfo*/, int /*deltaTimeMs*/) override
        {
            if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Left )
                subYRotationSpeed(2.25_deg, -4_deg);
            else if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Right )
                addYRotationSpeed(2.25_deg, 4_deg);
        }

        std::unique_ptr<AbstractStateHandler> postprocessFrame(CollisionInfo& collisionInfo) override
        {
            setFallSpeed(core::makeInterpolatedValue(0.0f));
            setFalling(false);
            collisionInfo.yAngle = getRotation().Y;
            setMovementAngle(collisionInfo.yAngle);
            collisionInfo.neededFloorDistanceBottom = core::ClimbLimit2ClickMin;
            collisionInfo.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
            collisionInfo.neededCeilingDistance = 0;
            collisionInfo.frobbelFlags |= CollisionInfo::FrobbelFlag_UnpassableSteepUpslant | CollisionInfo::FrobbelFlag_UnwalkableSteepFloor | CollisionInfo::FrobbelFlag_UnwalkableDeadlyFloor;
            collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);

            auto nextHandler = stopIfCeilingBlocked(collisionInfo);
            if( nextHandler )
                return nextHandler;
            nextHandler = tryClimb(collisionInfo);
            if( nextHandler )
                return nextHandler;

            nextHandler = checkWallCollision(collisionInfo);
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

            if( collisionInfo.current.floor.distance > core::ClimbLimit2ClickMin )
            {
                playAnimation(loader::AnimationId::FREE_FALL_FORWARD, 492);
                nextHandler = createWithRetainedAnimation(LaraStateId::JumpForward);
                setTargetState(LaraStateId::JumpForward);
                setFallSpeed(core::makeInterpolatedValue(0.0f));
                setFalling(true);
            }

            if( collisionInfo.current.floor.distance > core::SteppableHeight )
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

            if( collisionInfo.current.floor.distance >= -core::ClimbLimit2ClickMin && collisionInfo.current.floor.distance < -core::SteppableHeight )
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

            if( !tryStartSlide(collisionInfo, nextHandler) )
            {
                placeOnFloor(collisionInfo);
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
        explicit StateHandler_1(LaraController& lara)
            : AbstractStateHandler(lara)
        {
        }

        std::unique_ptr<AbstractStateHandler> handleInputImpl(CollisionInfo& /*collisionInfo*/) override
        {
            if( getHealth() <= 0 )
            {
                setTargetState(LaraStateId::Death);
                return nullptr;
            }

            if( getLevel().m_inputHandler->getInputState().roll )
            {
                playAnimation(loader::AnimationId::ROLL_BEGIN, 3857);
                setTargetState(LaraStateId::Stop);
                return createWithRetainedAnimation(LaraStateId::RollForward);
            }

            if( getLevel().m_inputHandler->getInputState().jump && !isFalling() )
            {
                setTargetState(LaraStateId::JumpForward);
                return nullptr;
            }

            if( getLevel().m_inputHandler->getInputState().zMovement != AxisMovement::Forward )
            {
                setTargetState(LaraStateId::Stop);
                return nullptr;
            }

            if( getLevel().m_inputHandler->getInputState().moveSlow )
                setTargetState(LaraStateId::WalkForward);
            else
                setTargetState(LaraStateId::RunForward);

            return nullptr;
        }

        void animateImpl(CollisionInfo& /*collisionInfo*/, int deltaTimeMs) override
        {
            if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Left )
            {
                subYRotationSpeed(2.25_deg, -8_deg);
                setZRotation(std::max(-11_deg, getRotation().Z - core::makeInterpolatedValue(+1.5_deg).getScaled(deltaTimeMs)));
            }
            else if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Right )
            {
                addYRotationSpeed(2.25_deg, 8_deg);
                setZRotation(std::min(+11_deg, getRotation().Z + core::makeInterpolatedValue(+1.5_deg).getScaled(deltaTimeMs)));
            }
        }

        std::unique_ptr<AbstractStateHandler> postprocessFrame(CollisionInfo& collisionInfo) override
        {
            collisionInfo.yAngle = getRotation().Y;
            setMovementAngle(collisionInfo.yAngle);
            collisionInfo.neededFloorDistanceBottom = loader::HeightLimit;
            collisionInfo.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
            collisionInfo.neededCeilingDistance = 0;
            collisionInfo.frobbelFlags |= CollisionInfo::FrobbelFlag_UnpassableSteepUpslant;
            collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);

            auto nextHandler = stopIfCeilingBlocked(collisionInfo);
            if( nextHandler )
                return nextHandler;
            nextHandler = tryClimb(collisionInfo);
            if( nextHandler )
                return nextHandler;

            nextHandler = checkWallCollision(collisionInfo);
            if( nextHandler != nullptr )
            {
                setZRotation(0_deg);
                if( collisionInfo.front.floor.slantClass == SlantClass::None && collisionInfo.front.floor.distance < -core::ClimbLimit2ClickMax )
                {
                    nextHandler = createWithRetainedAnimation(LaraStateId::Unknown12);
                    if( getCurrentFrame() <= 9 )
                    {
                        playAnimation(loader::AnimationId::WALL_SMASH_LEFT, 800);
                        return nextHandler;
                    }
                    if( getCurrentFrame() >= 10 && getCurrentFrame() <= 21 )
                    {
                        playAnimation(loader::AnimationId::WALL_SMASH_RIGHT, 815);
                        return nextHandler;
                    }
                }

                playAnimation(loader::AnimationId::STAY_SOLID, 185);
            }

            if( collisionInfo.current.floor.distance > core::ClimbLimit2ClickMin )
            {
                playAnimation(loader::AnimationId::FREE_FALL_FORWARD, 492);
                setTargetState(LaraStateId::JumpForward);
                setFalling(true);
                setFallSpeed(core::makeInterpolatedValue(0.0f));
                return createWithRetainedAnimation(LaraStateId::JumpForward);
            }

            if( collisionInfo.current.floor.distance >= -core::ClimbLimit2ClickMin && collisionInfo.current.floor.distance < -core::SteppableHeight )
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

            if( !tryStartSlide(collisionInfo, nextHandler) )
            {
                if( collisionInfo.current.floor.distance > 50 )
                    collisionInfo.current.floor.distance = 50;
                placeOnFloor(collisionInfo);
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
        explicit StateHandler_2(LaraController& lara)
            : StateHandler_Standing(lara)
        {
        }

        std::unique_ptr<AbstractStateHandler> handleInputImpl(CollisionInfo& collisionInfo) override
        {
            if( getHealth() <= 0 )
            {
                setTargetState(LaraStateId::Death);
                return nullptr;
            }

            if( getLevel().m_inputHandler->getInputState().roll )
            {
                playAnimation(loader::AnimationId::ROLL_BEGIN);
                setTargetState(LaraStateId::Stop);
                return createWithRetainedAnimation(LaraStateId::RollForward);
            }

            setTargetState(LaraStateId::Stop);

            if( getLevel().m_inputHandler->getInputState().stepMovement == AxisMovement::Left )
            {
                setTargetState(LaraStateId::StepLeft);
            }
            else if( getLevel().m_inputHandler->getInputState().stepMovement == AxisMovement::Right )
            {
                setTargetState(LaraStateId::StepRight);
            }

            if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Left )
            {
                setTargetState(LaraStateId::TurnLeftSlow);
            }
            else if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Right )
            {
                setTargetState(LaraStateId::TurnRightSlow);
            }

            if( getLevel().m_inputHandler->getInputState().jump )
            {
                setTargetState(LaraStateId::JumpPrepare);
            }
            else if( getLevel().m_inputHandler->getInputState().zMovement == AxisMovement::Forward )
            {
                if( getLevel().m_inputHandler->getInputState().moveSlow )
                    createWithRetainedAnimation(LaraStateId::WalkForward)->handleInputImpl(collisionInfo);
                else
                    createWithRetainedAnimation(LaraStateId::RunForward)->handleInputImpl(collisionInfo);
            }
            else if( getLevel().m_inputHandler->getInputState().zMovement == AxisMovement::Backward )
            {
                if( getLevel().m_inputHandler->getInputState().moveSlow )
                    createWithRetainedAnimation(LaraStateId::WalkBackward)->handleInputImpl(collisionInfo);
                else
                    setTargetState(LaraStateId::RunBack);
            }

            return nullptr;
        }

        void animateImpl(CollisionInfo& /*collisionInfo*/, int /*deltaTimeMs*/) override
        {
            if(getLevel().m_inputHandler->getInputState().freeLook)
            {
                getLevel().m_cameraController->setCamOverrideType(2);
                getLevel().m_cameraController->addHeadRotationXY(
                    -FreeLookMouseMovementScale * getLevel().m_inputHandler->getInputState().mouseMovement.y,
                    FreeLookMouseMovementScale * getLevel().m_inputHandler->getInputState().mouseMovement.x
                );
                auto r = getLevel().m_cameraController->getHeadRotation();
                if(r.Y < -44_deg)
                    r.Y = -44_deg;
                else if(r.Y > 44_deg)
                    r.Y = 44_deg;

                if(r.X < -42_deg)
                    r.X = -42_deg;
                else if(r.X > 22_deg)
                    r.X = 22_deg;

                getLevel().m_cameraController->setHeadRotation(r);
                getLevel().m_cameraController->setTorsoRotation(r);
            }
            else if(getLevel().m_cameraController->getCamOverrideType() == 2)
            {
                getLevel().m_cameraController->setCamOverrideType(0);
            }
        }

        loader::LaraStateId getId() const noexcept override
        {
            return LaraStateId::Stop;
        }
    };

    class StateHandler_3 final : public AbstractStateHandler
    {
    public:
        explicit StateHandler_3(LaraController& lara)
            : AbstractStateHandler(lara)
        {
        }

        std::unique_ptr<AbstractStateHandler> handleInputImpl(CollisionInfo& /*collisionInfo*/) override
        {
            if( getTargetState() == LaraStateId::SwandiveBegin || getTargetState() == LaraStateId::Reach )
                setTargetState(LaraStateId::JumpForward);

            if( getTargetState() == LaraStateId::Death || getTargetState() == LaraStateId::Stop )
                return nullptr;

            if( getLevel().m_inputHandler->getInputState().action && getHandStatus() == 0 )
                setTargetState(LaraStateId::Reach);

            if( getLevel().m_inputHandler->getInputState().moveSlow && getHandStatus() == 0 )
                setTargetState(LaraStateId::SwandiveBegin);

            if( getFallSpeed() > core::FreeFallSpeedThreshold )
                setTargetState(LaraStateId::FreeFall);

            return nullptr;
        }

        void animateImpl(CollisionInfo& /*collisionInfo*/, int /*deltaTimeMs*/) override
        {
            if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Left )
            {
                subYRotationSpeed(2.25_deg, -3_deg);
            }
            else if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Right )
            {
                addYRotationSpeed(2.25_deg, 3_deg);
            }
        }

        std::unique_ptr<AbstractStateHandler> postprocessFrame(CollisionInfo& collisionInfo) override
        {
            collisionInfo.neededFloorDistanceBottom = loader::HeightLimit;
            collisionInfo.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
            collisionInfo.neededCeilingDistance = 192;
            collisionInfo.yAngle = getRotation().Y;
            setMovementAngle(collisionInfo.yAngle);
            collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
            auto nextHandler = checkJumpWallSmash(collisionInfo);

            if( collisionInfo.current.floor.distance > 0 || getFallSpeed() <= 0 )
                return nextHandler;

            if( applyLandingDamage() )
            {
                setTargetState(LaraStateId::Death);
            }
            else if( getLevel().m_inputHandler->getInputState().zMovement != AxisMovement::Forward || getLevel().m_inputHandler->getInputState().moveSlow )
            {
                setTargetState(LaraStateId::Stop);
            }
            else
            {
                setTargetState(LaraStateId::RunForward);
            }

            setFallSpeed(core::makeInterpolatedValue(0.0f));
            setFalling(false);
            setHorizontalSpeed(core::makeInterpolatedValue(0.0f));
            placeOnFloor(collisionInfo);
            return getController().processLaraAnimCommands();
        }

        loader::LaraStateId getId() const noexcept override
        {
            return LaraStateId::JumpForward;
        }
    };

    class StateHandler_4 final : public StateHandler_Standing
    {
    public:
        explicit StateHandler_4(LaraController& lara)
            : StateHandler_Standing(lara)
        {
        }

        std::unique_ptr<AbstractStateHandler> handleInputImpl(CollisionInfo& /*collisionInfo*/) override
        {
            return nullptr;
        }

        void animateImpl(CollisionInfo& /*collisionInfo*/, int /*deltaTimeMs*/) override
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
        explicit StateHandler_5(LaraController& lara)
            : AbstractStateHandler(lara)
        {
        }

        std::unique_ptr<AbstractStateHandler> handleInputImpl(CollisionInfo& /*collisionInfo*/) override
        {
            setTargetState(LaraStateId::Stop);
            return nullptr;
        }

        void animateImpl(CollisionInfo& /*collisionInfo*/, int /*deltaTimeMs*/) override
        {
            if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Left )
                subYRotationSpeed(2.25_deg, -6_deg);
            else if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Right )
                addYRotationSpeed(2.25_deg, 6_deg);
        }

        std::unique_ptr<AbstractStateHandler> postprocessFrame(CollisionInfo& collisionInfo) override
        {
            setFallSpeed(core::makeInterpolatedValue(0.0f));
            setFalling(false);
            collisionInfo.neededFloorDistanceBottom = loader::HeightLimit;
            collisionInfo.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
            collisionInfo.neededCeilingDistance = 0;
            collisionInfo.frobbelFlags |= CollisionInfo::FrobbelFlag_UnpassableSteepUpslant | CollisionInfo::FrobbelFlag_UnwalkableSteepFloor;
            collisionInfo.yAngle = getRotation().Y + 180_deg;
            setMovementAngle(collisionInfo.yAngle);
            collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
            if( auto nextHandler = stopIfCeilingBlocked(collisionInfo) )
                return nextHandler;

            if( collisionInfo.current.floor.distance > 200 )
            {
                playAnimation(loader::AnimationId::FREE_FALL_BACK, 1473);
                setTargetState(LaraStateId::FallBackward);
                setFallSpeed(core::makeInterpolatedValue(0.0f));
                setFalling(true);
                return createWithRetainedAnimation(LaraStateId::FallBackward);
            }

            auto nextHandler = checkWallCollision(collisionInfo);
            if( nextHandler )
            {
                playAnimation(loader::AnimationId::STAY_SOLID, 185);
            }
            placeOnFloor(collisionInfo);

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
        explicit StateHandler_TurnSlow(LaraController& lara)
            : AbstractStateHandler(lara)
        {
        }

    public:
        std::unique_ptr<AbstractStateHandler> postprocessFrame(CollisionInfo& collisionInfo) override final
        {
            setFallSpeed(core::makeInterpolatedValue(0.0f));
            setFalling(false);
            collisionInfo.yAngle = getRotation().Y;
            setMovementAngle(collisionInfo.yAngle);
            collisionInfo.neededFloorDistanceBottom = core::ClimbLimit2ClickMin;
            collisionInfo.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
            collisionInfo.neededCeilingDistance = 0;
            collisionInfo.frobbelFlags |= CollisionInfo::FrobbelFlag_UnpassableSteepUpslant | CollisionInfo::FrobbelFlag_UnwalkableSteepFloor;
            collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);

            if( collisionInfo.current.floor.distance <= 100 )
            {
                std::unique_ptr<AbstractStateHandler> nextHandler = nullptr;
                if( !tryStartSlide(collisionInfo, nextHandler) )
                    placeOnFloor(collisionInfo);

                return nextHandler;
            }

            playAnimation(loader::AnimationId::FREE_FALL_FORWARD, 492);
            setTargetState(LaraStateId::JumpForward);
            setFallSpeed(core::makeInterpolatedValue(0.0f));
            setFalling(true);
            return createWithRetainedAnimation(LaraStateId::JumpForward);
        }
    };

    class StateHandler_6 final : public StateHandler_TurnSlow
    {
    public:
        explicit StateHandler_6(LaraController& lara)
            : StateHandler_TurnSlow(lara)
        {
        }

        std::unique_ptr<AbstractStateHandler> handleInputImpl(CollisionInfo& /*collisionInfo*/) override
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

            if( getLevel().m_inputHandler->getInputState().zMovement != AxisMovement::Forward )
            {
                if( getLevel().m_inputHandler->getInputState().xMovement != AxisMovement::Right )
                    setTargetState(LaraStateId::Stop);
                return nullptr;
            }

            if( getLevel().m_inputHandler->getInputState().moveSlow )
                setTargetState(LaraStateId::WalkForward);
            else
                setTargetState(LaraStateId::RunForward);

            return nullptr;
        }

        void animateImpl(CollisionInfo& /*collisionInfo*/, int /*deltaTimeMs*/) override
        {
            addYRotationSpeed(2.25_deg);
            if( getYRotationSpeed() <= 4_deg )
                return;

            if( getLevel().m_inputHandler->getInputState().moveSlow )
                setYRotationSpeed(4_deg);
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
        explicit StateHandler_7(LaraController& lara)
            : StateHandler_TurnSlow(lara)
        {
        }

        std::unique_ptr<AbstractStateHandler> handleInputImpl(CollisionInfo& /*collisionInfo*/) override
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

            if( getLevel().m_inputHandler->getInputState().zMovement != AxisMovement::Forward )
            {
                if( getLevel().m_inputHandler->getInputState().xMovement != AxisMovement::Left )
                    setTargetState(LaraStateId::Stop);
                return nullptr;
            }

            if( getLevel().m_inputHandler->getInputState().moveSlow )
                setTargetState(LaraStateId::WalkForward);
            else
                setTargetState(LaraStateId::RunForward);

            return nullptr;
        }

        void animateImpl(CollisionInfo& /*collisionInfo*/, int /*deltaTimeMs*/) override
        {
            subYRotationSpeed(2.25_deg);
            if( getYRotationSpeed() >= -4_deg )
                return;

            if( getLevel().m_inputHandler->getInputState().moveSlow )
                setYRotationSpeed(-4_deg);
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
        explicit StateHandler_8(LaraController& lara)
            : AbstractStateHandler(lara)
        {
        }

        std::unique_ptr<AbstractStateHandler> handleInputImpl(CollisionInfo& collisionInfo) override
        {
            collisionInfo.frobbelFlags &= ~(CollisionInfo::FrobbelFlag08 | CollisionInfo::FrobbelFlag10);
            return nullptr;
        }

        std::unique_ptr<AbstractStateHandler> postprocessFrame(CollisionInfo& collisionInfo) override
        {
            collisionInfo.neededFloorDistanceBottom = core::ClimbLimit2ClickMin;
            collisionInfo.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
            collisionInfo.neededCeilingDistance = 0;
            collisionInfo.collisionRadius = 400;
            collisionInfo.yAngle = getRotation().Y;
            setMovementAngle(collisionInfo.yAngle);
            applyCollisionFeedback(collisionInfo);
            placeOnFloor(collisionInfo);
            collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
            setHealth(core::makeInterpolatedValue(-1.0f));
            //! @todo set air=-1
            return nullptr;
        }

        loader::LaraStateId getId() const noexcept override
        {
            return LaraStateId::Death;
        }

        void animateImpl(CollisionInfo& /*collisionInfo*/, int /*deltaTimeMs*/) override
        {
        }
    };

    class StateHandler_9 final : public AbstractStateHandler
    {
    public:
        explicit StateHandler_9(LaraController& lara)
            : AbstractStateHandler(lara)
        {
        }

        std::unique_ptr<AbstractStateHandler> handleInputImpl(CollisionInfo& /*collisionInfo*/) override
        {
            return nullptr;
        }

        void animateImpl(CollisionInfo& /*collisionInfo*/, int /*deltaTimeMs*/) override
        {
            dampenHorizontalSpeed(0.05f);
            if( getFallSpeed() > 154 )
            {
                getController().playSoundEffect(30);
            }
        }

        std::unique_ptr<AbstractStateHandler> postprocessFrame(CollisionInfo& collisionInfo) override
        {
            collisionInfo.neededFloorDistanceBottom = loader::HeightLimit;
            collisionInfo.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
            collisionInfo.neededCeilingDistance = 192;
            collisionInfo.yAngle = getMovementAngle();
            setFalling(true);
            collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
            jumpAgainstWall(collisionInfo);
            if( collisionInfo.current.floor.distance > 0 )
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
            getLevel().stopSoundEffect(30);
            setFallSpeed(core::makeInterpolatedValue(0.0f));
            placeOnFloor(collisionInfo);
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
        explicit StateHandler_10(LaraController& lara)
            : AbstractStateHandler(lara)
        {
        }

        std::unique_ptr<AbstractStateHandler> handleInputImpl(CollisionInfo& collisionInfo) override
        {
            setCameraRotation(-60_deg, 0_deg);
            collisionInfo.frobbelFlags &= ~(CollisionInfo::FrobbelFlag08 | CollisionInfo::FrobbelFlag10);
            if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Left || getLevel().m_inputHandler->getInputState().stepMovement == AxisMovement::Left )
                setTargetState(LaraStateId::ShimmyLeft);
            else if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Right || getLevel().m_inputHandler->getInputState().stepMovement == AxisMovement::Right )
                setTargetState(LaraStateId::ShimmyRight);

            return nullptr;
        }

        std::unique_ptr<AbstractStateHandler> postprocessFrame(CollisionInfo& collisionInfo) override
        {
            auto nextHandler = commonEdgeHangHandling(collisionInfo);

            if( getTargetState() != LaraStateId::Hang )
                return nextHandler;

            if( getLevel().m_inputHandler->getInputState().zMovement != AxisMovement::Forward )
                return nextHandler;

            const auto frontHeight = collisionInfo.front.floor.distance;
            const auto frontSpace = frontHeight - collisionInfo.front.ceiling.distance;
            const auto frontLeftSpace = collisionInfo.frontLeft.floor.distance - collisionInfo.frontLeft.ceiling.distance;
            const auto frontRightSpace = collisionInfo.frontRight.floor.distance - collisionInfo.frontRight.ceiling.distance;
            if( frontHeight <= -850 || frontHeight >= -650 || frontSpace < 0 || frontLeftSpace < 0 || frontRightSpace < 0 || collisionInfo.hasStaticMeshCollision )
            {
                return nextHandler;
            }

            if( getLevel().m_inputHandler->getInputState().moveSlow )
                setTargetState(LaraStateId::Handstand);
            else
                setTargetState(LaraStateId::Climbing);

            return nextHandler;
        }

        loader::LaraStateId getId() const noexcept override
        {
            return LaraStateId::Hang;
        }

        void animateImpl(CollisionInfo& /*collisionInfo*/, int /*deltaTimeMs*/) override
        {
        }
    };

    class StateHandler_11 final : public AbstractStateHandler
    {
    public:
        explicit StateHandler_11(LaraController& lara)
            : AbstractStateHandler(lara)
        {
        }

        std::unique_ptr<AbstractStateHandler> handleInputImpl(CollisionInfo& /*collisionInfo*/) override
        {
            setCameraRotationY(85_deg);
            if( getFallSpeed() > core::FreeFallSpeedThreshold )
                setTargetState(LaraStateId::FreeFall);
            return nullptr;
        }

        void animateImpl(CollisionInfo& /*collisionInfo*/, int /*deltaTimeMs*/) override
        {
        }

        std::unique_ptr<AbstractStateHandler> postprocessFrame(CollisionInfo& collisionInfo) override
        {
            setFalling(true);
            collisionInfo.yAngle = getRotation().Y;
            setMovementAngle(collisionInfo.yAngle);
            collisionInfo.neededFloorDistanceBottom = loader::HeightLimit;
            collisionInfo.neededFloorDistanceTop = 0;
            collisionInfo.neededCeilingDistance = 192;
            collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);

            auto nextHandler = tryReach(collisionInfo);
            if( nextHandler )
                return nextHandler;

            jumpAgainstWall(collisionInfo);
            if( getFallSpeed() <= 0 || collisionInfo.current.floor.distance > 0 )
                return nextHandler;

            if( applyLandingDamage() )
                setTargetState(LaraStateId::Death);
            else
                setTargetState(LaraStateId::Stop);

            setFallSpeed(core::makeInterpolatedValue(0.0f));
            setFalling(false);
            placeOnFloor(collisionInfo);

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
        explicit StateHandler_12(LaraController& lara)
            : AbstractStateHandler(lara)
        {
        }

        std::unique_ptr<AbstractStateHandler> handleInputImpl(CollisionInfo& /*collisionInfo*/) override
        {
            return nullptr;
        }

        void animateImpl(CollisionInfo& /*collisionInfo*/, int /*deltaTimeMs*/) override
        {
        }

        std::unique_ptr<AbstractStateHandler> postprocessFrame(CollisionInfo& collisionInfo) override
        {
            collisionInfo.neededFloorDistanceBottom = core::ClimbLimit2ClickMin;
            collisionInfo.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
            collisionInfo.neededCeilingDistance = 0;
            collisionInfo.yAngle = getMovementAngle();
            collisionInfo.frobbelFlags |= CollisionInfo::FrobbelFlag_UnpassableSteepUpslant | CollisionInfo::FrobbelFlag_UnwalkableSteepFloor;
            collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
            applyCollisionFeedback(collisionInfo);
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
        explicit StateHandler_Underwater(LaraController& lara)
            : AbstractStateHandler(lara)
        {
        }

        std::unique_ptr<AbstractStateHandler> postprocessFrame(CollisionInfo& collisionInfo) override
        {
            collisionInfo.yAngle = getRotation().Y;
            if( std::abs(getRotation().X) > 90_deg )
                collisionInfo.yAngle += 180_deg;
            setMovementAngle(collisionInfo.yAngle);
            collisionInfo.initHeightInfo(getPosition() + core::ExactTRCoordinates{0, 200, 0}, getLevel(), 400);

            applyCollisionFeedback(collisionInfo);

            m_xRotationSpeed = 0_deg;
            m_yRotationSpeed = 0_deg;

            switch( collisionInfo.axisCollisions )
            {
            case CollisionInfo::AxisColl_FrontLeftBlocked:
                m_yRotationSpeed = 5_deg;
                break;
            case CollisionInfo::AxisColl_FrontRightBlocked:
                m_yRotationSpeed = -5_deg;
                break;
            case CollisionInfo::AxisColl_InvalidPosition:
                setFallSpeed(core::makeInterpolatedValue(0.0f));
                return nullptr;
            case CollisionInfo::AxisColl_InsufficientFrontCeilingSpace:
                setFallSpeed(core::makeInterpolatedValue(0.0f));
                break;
            case CollisionInfo::AxisColl_ScalpCollision:
                if( getRotation().X > -45_deg )
                    m_xRotationSpeed = -2_deg; // setXRotation(getRotation().X - 364);
                break;
            case CollisionInfo::AxisColl_FrontForwardBlocked:
                if( getRotation().X > 35_deg )
                    m_xRotationSpeed = 2_deg; // setXRotation(getRotation().X + 364);
                else if( getRotation().X < -35_deg )
                    m_xRotationSpeed = -2_deg; // setXRotation(getRotation().X - 364);
                else
                    setFallSpeed(core::makeInterpolatedValue(0.0f));
                break;
            default:
                break;
            }

            if( collisionInfo.current.floor.distance >= 0 )
                return nullptr;

            setPosition(getPosition() + core::ExactTRCoordinates(0, gsl::narrow_cast<float>(collisionInfo.current.floor.distance), 0));
            m_xRotationSpeed = m_xRotationSpeed + 2_deg;

            return nullptr;
        }

    protected:
        void handleDiveInput()
        {
            if( getLevel().m_inputHandler->getInputState().zMovement == AxisMovement::Forward )
                m_xRotationSpeed = -2_deg;
            else if( getLevel().m_inputHandler->getInputState().zMovement == AxisMovement::Backward )
                m_xRotationSpeed = 2_deg;
            else
                m_xRotationSpeed = 0_deg;
            if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Left )
            {
                m_yRotationSpeed = -6_deg;
                m_zRotationSpeed = -3_deg;
            }
            else if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Right )
            {
                m_yRotationSpeed = 6_deg;
                m_zRotationSpeed = 3_deg;
            }
            else
            {
                m_yRotationSpeed = 0_deg;
                m_zRotationSpeed = 0_deg;
            }
        }
    };

    class StateHandler_13 final : public StateHandler_Underwater
    {
    public:
        explicit StateHandler_13(LaraController& lara)
            : StateHandler_Underwater(lara)
        {
        }

        std::unique_ptr<AbstractStateHandler> handleInputImpl(CollisionInfo& /*collisionInfo*/) override
        {
            if( getHealth() < 0 )
            {
                setTargetState(LaraStateId::WaterDeath);
                return nullptr;
            }

            handleDiveInput();

            if( getLevel().m_inputHandler->getInputState().jump )
                setTargetState(LaraStateId::UnderwaterForward);

            return nullptr;
        }

        void animateImpl(CollisionInfo& /*collisionInfo*/, int deltaTimeMs) override
        {
            setFallSpeed(std::max(core::makeInterpolatedValue(0.0f), getFallSpeed() - core::makeInterpolatedValue(6.0f).getScaled(deltaTimeMs)));
        }

        loader::LaraStateId getId() const noexcept override
        {
            return LaraStateId::UnderwaterStop;
        }
    };

    class StateHandler_15 final : public AbstractStateHandler
    {
    public:
        explicit StateHandler_15(LaraController& lara)
            : AbstractStateHandler(lara)
        {
        }

        std::unique_ptr<AbstractStateHandler> handleInputImpl(CollisionInfo& /*collisionInfo*/) override
        {
            if( getLevel().m_inputHandler->getInputState().zMovement == AxisMovement::Forward && getRelativeHeightAtDirection(getRotation().Y, 256) >= -core::ClimbLimit2ClickMin )
            {
                setMovementAngle(getRotation().Y);
                setTargetState(LaraStateId::JumpForward);
            }
            else if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Left && getRelativeHeightAtDirection(getRotation().Y - 90_deg, 256) >= -core::ClimbLimit2ClickMin )
            {
                setMovementAngle(getRotation().Y - 90_deg);
                setTargetState(LaraStateId::JumpRight);
            }
            else if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Right && getRelativeHeightAtDirection(getRotation().Y + 90_deg, 256) >= -core::ClimbLimit2ClickMin )
            {
                setMovementAngle(getRotation().Y + 90_deg);
                setTargetState(LaraStateId::JumpLeft);
            }
            else if( getLevel().m_inputHandler->getInputState().zMovement == AxisMovement::Backward && getRelativeHeightAtDirection(getRotation().Y + 180_deg, 256) >= -core::ClimbLimit2ClickMin )
            {
                setMovementAngle(getRotation().Y + 180_deg);
                setTargetState(LaraStateId::JumpBack);
            }

            if( getFallSpeed() > core::FreeFallSpeedThreshold )
            {
                setTargetState(LaraStateId::FreeFall);
            }

            return nullptr;
        }

        void animateImpl(CollisionInfo& /*collisionInfo*/, int /*deltaTimeMs*/) override
        {
        }

        std::unique_ptr<AbstractStateHandler> postprocessFrame(CollisionInfo& collisionInfo) override
        {
            setFallSpeed(core::makeInterpolatedValue(0.0f));
            setFalling(false);
            collisionInfo.neededFloorDistanceBottom = loader::HeightLimit;
            collisionInfo.neededFloorDistanceTop = -loader::HeightLimit;
            collisionInfo.neededCeilingDistance = 0;
            collisionInfo.yAngle = getRotation().Y;
            collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);

            if( collisionInfo.current.ceiling.distance <= -100 )
                return nullptr;

            setTargetState(LaraStateId::Stop);
            playAnimation(loader::AnimationId::STAY_SOLID, 185);
            setHorizontalSpeed(core::makeInterpolatedValue(0.0f));
            setPosition(collisionInfo.position);

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
        explicit StateHandler_16(LaraController& lara)
            : AbstractStateHandler(lara)
        {
        }

        std::unique_ptr<AbstractStateHandler> handleInputImpl(CollisionInfo& /*collisionInfo*/) override
        {
            if( getHealth() <= 0 )
            {
                setTargetState(LaraStateId::Stop);
                return nullptr;
            }

            if( getLevel().m_inputHandler->getInputState().zMovement == AxisMovement::Backward && getLevel().m_inputHandler->getInputState().moveSlow )
                setTargetState(LaraStateId::WalkBackward);
            else
                setTargetState(LaraStateId::Stop);

            return nullptr;
        }

        void animateImpl(CollisionInfo& /*collisionInfo*/, int /*deltaTimeMs*/) override
        {
            if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Left )
                subYRotationSpeed(2.25_deg, -4_deg);
            else if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Right )
                addYRotationSpeed(2.25_deg, 4_deg);
        }

        std::unique_ptr<AbstractStateHandler> postprocessFrame(CollisionInfo& collisionInfo) override
        {
            setFallSpeed(core::makeInterpolatedValue(0.0f));
            setFalling(false);
            collisionInfo.neededFloorDistanceBottom = core::ClimbLimit2ClickMin;
            collisionInfo.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
            collisionInfo.neededCeilingDistance = 0;
            collisionInfo.yAngle = getRotation().Y + 180_deg;
            setMovementAngle(collisionInfo.yAngle);
            collisionInfo.frobbelFlags |= CollisionInfo::FrobbelFlag_UnpassableSteepUpslant | CollisionInfo::FrobbelFlag_UnwalkableSteepFloor;
            collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);

            if( auto nextHandler = stopIfCeilingBlocked(collisionInfo) )
                return nextHandler;

            auto nextHandler = checkWallCollision(collisionInfo);
            if( nextHandler )
            {
                playAnimation(loader::AnimationId::STAY_SOLID, 185);
            }

            if( collisionInfo.current.floor.distance > loader::QuarterSectorSize && collisionInfo.current.floor.distance < core::ClimbLimit2ClickMin )
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

            if( !tryStartSlide(collisionInfo, nextHandler) )
            {
                placeOnFloor(collisionInfo);
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
        explicit StateHandler_17(LaraController& lara)
            : StateHandler_Underwater(lara)
        {
        }

        std::unique_ptr<AbstractStateHandler> handleInputImpl(CollisionInfo& /*collisionInfo*/) override
        {
            if( getHealth() < 0 )
            {
                setTargetState(LaraStateId::WaterDeath);
                return nullptr;
            }

            handleDiveInput();

            if( !getLevel().m_inputHandler->getInputState().jump )
                setTargetState(LaraStateId::UnderwaterInertia);

            return nullptr;
        }

        void animateImpl(CollisionInfo& /*collisionInfo*/, int deltaTimeMs) override
        {
            setFallSpeed((getFallSpeed() + core::makeInterpolatedValue(8.0f).getScaled(deltaTimeMs)).limitMax(200.0f));
        }

        loader::LaraStateId getId() const noexcept override
        {
            return LaraStateId::UnderwaterForward;
        }
    };

    class StateHandler_18 final : public StateHandler_Underwater
    {
    public:
        explicit StateHandler_18(LaraController& lara)
            : StateHandler_Underwater(lara)
        {
        }

        std::unique_ptr<AbstractStateHandler> handleInputImpl(CollisionInfo& /*collisionInfo*/) override
        {
            if( getHealth() < 0 )
            {
                setTargetState(LaraStateId::WaterDeath);
                return nullptr;
            }

            handleDiveInput();

            if( getLevel().m_inputHandler->getInputState().jump )
                setTargetState(LaraStateId::UnderwaterForward);

            if( getFallSpeed() <= 133 )
                setTargetState(LaraStateId::UnderwaterStop);

            return nullptr;
        }

        void animateImpl(CollisionInfo& /*collisionInfo*/, int deltaTimeMs) override
        {
            setFallSpeed(std::max(core::makeInterpolatedValue(0.0f), getFallSpeed() - core::makeInterpolatedValue(6.0f).getScaled(deltaTimeMs)));
        }

        loader::LaraStateId getId() const noexcept override
        {
            return LaraStateId::UnderwaterInertia;
        }
    };

    class StateHandler_19 final : public AbstractStateHandler
    {
    public:
        explicit StateHandler_19(LaraController& lara)
            : AbstractStateHandler(lara)
        {
        }

        std::unique_ptr<AbstractStateHandler> handleInputImpl(CollisionInfo& collisionInfo) override
        {
            collisionInfo.frobbelFlags &= ~(CollisionInfo::FrobbelFlag08 | CollisionInfo::FrobbelFlag10);
            return nullptr;
        }

        void animateImpl(CollisionInfo& /*collisionInfo*/, int /*deltaTimeMs*/) override
        {
        }

        std::unique_ptr<AbstractStateHandler> postprocessFrame(CollisionInfo& collisionInfo) override
        {
            collisionInfo.neededFloorDistanceBottom = core::ClimbLimit2ClickMin;
            collisionInfo.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
            collisionInfo.neededCeilingDistance = 0;
            collisionInfo.yAngle = getRotation().Y;
            setMovementAngle(collisionInfo.yAngle);
            collisionInfo.frobbelFlags |= CollisionInfo::FrobbelFlag_UnpassableSteepUpslant | CollisionInfo::FrobbelFlag_UnwalkableSteepFloor;
            collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
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
        explicit StateHandler_20(LaraController& lara)
            : StateHandler_Standing(lara)
        {
        }

        std::unique_ptr<AbstractStateHandler> handleInputImpl(CollisionInfo& /*collisionInfo*/) override
        {
            if( getHealth() <= 0 )
            {
                setTargetState(LaraStateId::Stop);
                return nullptr;
            }

            if( getYRotationSpeed() >= 0_deg )
            {
                setYRotationSpeed(8_deg);
                if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Right )
                    return nullptr;
            }
            else
            {
                setYRotationSpeed(-8_deg);
                if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Left )
                    return nullptr;
            }

            setTargetState(LaraStateId::Stop);
            return nullptr;
        }

        void animateImpl(CollisionInfo& /*collisionInfo*/, int /*deltaTimeMs*/) override
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
        explicit StateHandler_21(LaraController& lara)
            : AbstractStateHandler(lara)
        {
        }

        std::unique_ptr<AbstractStateHandler> handleInputImpl(CollisionInfo& /*collisionInfo*/) override
        {
            if( getHealth() <= 0 )
            {
                setTargetState(LaraStateId::Stop);
                return nullptr;
            }

            if( getLevel().m_inputHandler->getInputState().stepMovement != AxisMovement::Right )
                setTargetState(LaraStateId::Stop);

            if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Left )
                setYRotationSpeed(std::max(-4_deg, getYRotationSpeed() - 2.25_deg));
            else if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Right )
                setYRotationSpeed(std::min(+4_deg, getYRotationSpeed() + 2.25_deg));

            return nullptr;
        }

        void animateImpl(CollisionInfo& /*collisionInfo*/, int /*deltaTimeMs*/) override
        {
        }

        loader::LaraStateId getId() const noexcept override
        {
            return LaraStateId::StepRight;
        }

        std::unique_ptr<AbstractStateHandler> postprocessFrame(CollisionInfo& collisionInfo) override
        {
            setFallSpeed(core::makeInterpolatedValue(0.0f));
            setFalling(false);
            collisionInfo.neededFloorDistanceBottom = 128;
            collisionInfo.neededFloorDistanceTop = -128;
            collisionInfo.neededCeilingDistance = 0;
            collisionInfo.yAngle = getRotation().Y + 90_deg;
            setMovementAngle(collisionInfo.yAngle);
            collisionInfo.frobbelFlags |= CollisionInfo::FrobbelFlag_UnpassableSteepUpslant | CollisionInfo::FrobbelFlag_UnwalkableSteepFloor;
            collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);

            auto nextHandler = stopIfCeilingBlocked(collisionInfo);
            if( nextHandler != nullptr )
                return nextHandler;

            nextHandler = checkWallCollision(collisionInfo);
            if( nextHandler != nullptr )
            {
                playAnimation(loader::AnimationId::STAY_SOLID, 185);
                setTargetState(LaraStateId::Stop);
                return createWithRetainedAnimation(LaraStateId::Stop);
            }

            if( !tryStartSlide(collisionInfo, nextHandler) )
                setPosition(getPosition() + core::ExactTRCoordinates(0, gsl::narrow_cast<float>(collisionInfo.current.floor.distance), 0));

            return nextHandler;
        }
    };

    class StateHandler_22 final : public AbstractStateHandler
    {
    public:
        explicit StateHandler_22(LaraController& lara)
            : AbstractStateHandler(lara)
        {
        }

        std::unique_ptr<AbstractStateHandler> handleInputImpl(CollisionInfo& /*collisionInfo*/) override
        {
            if( getHealth() <= 0 )
            {
                setTargetState(LaraStateId::Stop);
                return nullptr;
            }

            if( getLevel().m_inputHandler->getInputState().stepMovement != AxisMovement::Left )
                setTargetState(LaraStateId::Stop);

            if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Left )
                setYRotationSpeed(std::max(-4_deg, getYRotationSpeed() - 2.25_deg));
            else if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Right )
                setYRotationSpeed(std::min(+4_deg, getYRotationSpeed() + 2.25_deg));

            return nullptr;
        }

        void animateImpl(CollisionInfo& /*collisionInfo*/, int /*deltaTimeMs*/) override
        {
        }

        loader::LaraStateId getId() const noexcept override
        {
            return LaraStateId::StepLeft;
        }

        std::unique_ptr<AbstractStateHandler> postprocessFrame(CollisionInfo& collisionInfo) override
        {
            setFallSpeed(core::makeInterpolatedValue(0.0f));
            setFalling(false);
            collisionInfo.neededFloorDistanceBottom = 128;
            collisionInfo.neededFloorDistanceTop = -128;
            collisionInfo.neededCeilingDistance = 0;
            collisionInfo.yAngle = getRotation().Y - 90_deg;
            setMovementAngle(collisionInfo.yAngle);
            collisionInfo.frobbelFlags |= CollisionInfo::FrobbelFlag_UnpassableSteepUpslant | CollisionInfo::FrobbelFlag_UnwalkableSteepFloor;
            collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);

            auto nextHandler = stopIfCeilingBlocked(collisionInfo);
            if( nextHandler != nullptr )
                return nextHandler;

            nextHandler = checkWallCollision(collisionInfo);
            if( nextHandler != nullptr )
            {
                playAnimation(loader::AnimationId::STAY_SOLID, 185);
                setTargetState(LaraStateId::Stop);
                return createWithRetainedAnimation(LaraStateId::Stop);
            }

            if( !tryStartSlide(collisionInfo, nextHandler) )
                setPosition(getPosition() + core::ExactTRCoordinates(0, gsl::narrow_cast<float>(collisionInfo.current.floor.distance), 0));

            return nextHandler;
        }
    };

    class StateHandler_23 final : public AbstractStateHandler
    {
    public:
        explicit StateHandler_23(LaraController& lara)
            : AbstractStateHandler(lara)
        {
        }

        std::unique_ptr<AbstractStateHandler> handleInputImpl(CollisionInfo& /*collisionInfo*/) override
        {
            return nullptr;
        }

        void animateImpl(CollisionInfo& /*collisionInfo*/, int /*deltaTimeMs*/) override
        {
        }

        std::unique_ptr<AbstractStateHandler> postprocessFrame(CollisionInfo& collisionInfo) override
        {
            setFalling(false);
            setFallSpeed(core::makeInterpolatedValue(0.0f));
            collisionInfo.yAngle = getRotation().Y + 180_deg;
            setMovementAngle(collisionInfo.yAngle);
            collisionInfo.frobbelFlags |= CollisionInfo::FrobbelFlag_UnpassableSteepUpslant;
            collisionInfo.neededFloorDistanceBottom = loader::HeightLimit;
            collisionInfo.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
            collisionInfo.neededCeilingDistance = 0;
            collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);

            auto nextHandler = stopIfCeilingBlocked(collisionInfo);
            if( nextHandler )
                return nextHandler;
            if( tryStartSlide(collisionInfo, nextHandler) )
                return nextHandler;

            if( collisionInfo.current.floor.distance <= 200 )
            {
                applyCollisionFeedback(collisionInfo);
                placeOnFloor(collisionInfo);
                return nextHandler;
            }

            playAnimation(loader::AnimationId::FREE_FALL_BACK, 1473);
            setTargetState(LaraStateId::FallBackward);
            setFallSpeed(core::makeInterpolatedValue(0.0f));
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
        explicit StateHandler_24(LaraController& lara)
            : AbstractStateHandler(lara)
        {
        }

        std::unique_ptr<AbstractStateHandler> handleInputImpl(CollisionInfo& /*collisionInfo*/) override
        {
            setCameraRotationX(-45_deg);
            if( getLevel().m_inputHandler->getInputState().jump )
                setTargetState(LaraStateId::JumpForward);

            return nullptr;
        }

        void animateImpl(CollisionInfo& /*collisionInfo*/, int /*deltaTimeMs*/) override
        {
        }

        std::unique_ptr<AbstractStateHandler> postprocessFrame(CollisionInfo& collisionInfo) override
        {
            setMovementAngle(getRotation().Y);
            return commonSlideHandling(collisionInfo);
        }

        loader::LaraStateId getId() const noexcept override
        {
            return LaraStateId::SlideForward;
        }
    };

    class StateHandler_25 final : public AbstractStateHandler
    {
    public:
        explicit StateHandler_25(LaraController& lara)
            : AbstractStateHandler(lara)
        {
        }

        std::unique_ptr<AbstractStateHandler> handleInputImpl(CollisionInfo& /*collisionInfo*/) override
        {
            setCameraRotationY(135_deg);
            if( getFallSpeed() > core::FreeFallSpeedThreshold )
                setTargetState(LaraStateId::FreeFall);

            return nullptr;
        }

        void animateImpl(CollisionInfo& /*collisionInfo*/, int /*deltaTimeMs*/) override
        {
        }

        std::unique_ptr<AbstractStateHandler> postprocessFrame(CollisionInfo& collisionInfo) override
        {
            setMovementAngle(getRotation().Y + 180_deg);
            return commonJumpHandling(collisionInfo);
        }

        loader::LaraStateId getId() const noexcept override
        {
            return LaraStateId::JumpBack;
        }
    };

    class StateHandler_26 final : public AbstractStateHandler
    {
    public:
        explicit StateHandler_26(LaraController& lara)
            : AbstractStateHandler(lara)
        {
        }

        std::unique_ptr<AbstractStateHandler> handleInputImpl(CollisionInfo& /*collisionInfo*/) override
        {
            if( getFallSpeed() > core::FreeFallSpeedThreshold )
                setTargetState(LaraStateId::FreeFall);

            return nullptr;
        }

        void animateImpl(CollisionInfo& /*collisionInfo*/, int /*deltaTimeMs*/) override
        {
        }

        std::unique_ptr<AbstractStateHandler> postprocessFrame(CollisionInfo& collisionInfo) override
        {
            setMovementAngle(getRotation().Y + 90_deg);
            return commonJumpHandling(collisionInfo);
        }

        loader::LaraStateId getId() const noexcept override
        {
            return LaraStateId::JumpLeft;
        }
    };

    class StateHandler_27 final : public AbstractStateHandler
    {
    public:
        explicit StateHandler_27(LaraController& lara)
            : AbstractStateHandler(lara)
        {
        }

        std::unique_ptr<AbstractStateHandler> handleInputImpl(CollisionInfo& /*collisionInfo*/) override
        {
            if( getFallSpeed() > core::FreeFallSpeedThreshold )
                setTargetState(LaraStateId::FreeFall);

            return nullptr;
        }

        void animateImpl(CollisionInfo& /*collisionInfo*/, int /*deltaTimeMs*/) override
        {
        }

        std::unique_ptr<AbstractStateHandler> postprocessFrame(CollisionInfo& collisionInfo) override
        {
            setMovementAngle(getRotation().Y - 90_deg);
            return commonJumpHandling(collisionInfo);
        }

        loader::LaraStateId getId() const noexcept override
        {
            return LaraStateId::JumpRight;
        }
    };

    class StateHandler_28 final : public AbstractStateHandler
    {
    public:
        explicit StateHandler_28(LaraController& lara)
            : AbstractStateHandler(lara)
        {
        }

        std::unique_ptr<AbstractStateHandler> handleInputImpl(CollisionInfo& /*collisionInfo*/) override
        {
            if( getFallSpeed() > core::FreeFallSpeedThreshold )
                setTargetState(LaraStateId::FreeFall);

            return nullptr;
        }

        void animateImpl(CollisionInfo& /*collisionInfo*/, int /*deltaTimeMs*/) override
        {
        }

        std::unique_ptr<AbstractStateHandler> postprocessFrame(CollisionInfo& collisionInfo) override
        {
            collisionInfo.neededFloorDistanceBottom = loader::HeightLimit;
            collisionInfo.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
            collisionInfo.neededCeilingDistance = 192;
            collisionInfo.yAngle = getRotation().Y;
            collisionInfo.initHeightInfo(getPosition(), getLevel(), 870); //! @todo MAGICK 870

            if( auto nextHandler = tryGrabEdge(collisionInfo) )
                return nextHandler;

            jumpAgainstWall(collisionInfo);
            if( getFallSpeed() <= 0 || collisionInfo.current.floor.distance > 0 )
                return nullptr;

            if( applyLandingDamage() )
                setTargetState(LaraStateId::Death);
            else
                setTargetState(LaraStateId::Stop);
            setFallSpeed(core::makeInterpolatedValue(0.0f));
            placeOnFloor(collisionInfo);
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
        explicit StateHandler_29(LaraController& lara)
            : AbstractStateHandler(lara)
        {
        }

        std::unique_ptr<AbstractStateHandler> handleInputImpl(CollisionInfo& /*collisionInfo*/) override
        {
            if( getFallSpeed() > core::FreeFallSpeedThreshold )
                setTargetState(LaraStateId::FreeFall);

            if( getLevel().m_inputHandler->getInputState().action && getHandStatus() == 0 )
                setTargetState(LaraStateId::Reach);

            return nullptr;
        }

        void animateImpl(CollisionInfo& /*collisionInfo*/, int /*deltaTimeMs*/) override
        {
        }

        std::unique_ptr<AbstractStateHandler> postprocessFrame(CollisionInfo& collisionInfo) override
        {
            collisionInfo.neededFloorDistanceBottom = loader::HeightLimit;
            collisionInfo.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
            collisionInfo.neededCeilingDistance = 192;
            collisionInfo.yAngle = getRotation().Y + 180_deg;
            collisionInfo.initHeightInfo(getPosition(), getLevel(), 870); //! @todo MAGICK 870
            auto nextHandler = checkJumpWallSmash(collisionInfo);
            if( collisionInfo.current.floor.distance > 0 || getFallSpeed() <= 0 )
                return nextHandler;

            if( applyLandingDamage() )
                setTargetState(LaraStateId::Death);
            else
                setTargetState(LaraStateId::Stop);

            setFallSpeed(core::makeInterpolatedValue(0.0f));
            placeOnFloor(collisionInfo);
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
        explicit StateHandler_30(LaraController& lara)
            : AbstractStateHandler(lara)
        {
        }

        std::unique_ptr<AbstractStateHandler> handleInputImpl(CollisionInfo& collisionInfo) override
        {
            setCameraRotation(-60_deg, 0_deg);
            collisionInfo.frobbelFlags &= ~(CollisionInfo::FrobbelFlag08 | CollisionInfo::FrobbelFlag10);
            if( getLevel().m_inputHandler->getInputState().xMovement != AxisMovement::Left && getLevel().m_inputHandler->getInputState().stepMovement != AxisMovement::Left )
                setTargetState(LaraStateId::Hang);

            return nullptr;
        }

        std::unique_ptr<AbstractStateHandler> postprocessFrame(CollisionInfo& collisionInfo) override
        {
            setMovementAngle(getRotation().Y - 90_deg);
            auto nextHandler = commonEdgeHangHandling(collisionInfo);
            setMovementAngle(getRotation().Y - 90_deg);
            return nextHandler;
        }

        loader::LaraStateId getId() const noexcept override
        {
            return LaraStateId::ShimmyLeft;
        }

        void animateImpl(CollisionInfo& /*collisionInfo*/, int /*deltaTimeMs*/) override
        {
        }
    };

    class StateHandler_31 final : public AbstractStateHandler
    {
    public:
        explicit StateHandler_31(LaraController& lara)
            : AbstractStateHandler(lara)
        {
        }

        std::unique_ptr<AbstractStateHandler> handleInputImpl(CollisionInfo& collisionInfo) override
        {
            setCameraRotation(-60_deg, 0_deg);
            collisionInfo.frobbelFlags &= ~(CollisionInfo::FrobbelFlag08 | CollisionInfo::FrobbelFlag10);
            if( getLevel().m_inputHandler->getInputState().xMovement != AxisMovement::Right && getLevel().m_inputHandler->getInputState().stepMovement != AxisMovement::Right )
                setTargetState(LaraStateId::Hang);

            return nullptr;
        }

        std::unique_ptr<AbstractStateHandler> postprocessFrame(CollisionInfo& collisionInfo) override
        {
            setMovementAngle(getRotation().Y + 90_deg);
            auto nextHandler = commonEdgeHangHandling(collisionInfo);
            setMovementAngle(getRotation().Y + 90_deg);
            return nextHandler;
        }

        loader::LaraStateId getId() const noexcept override
        {
            return LaraStateId::ShimmyRight;
        }

        void animateImpl(CollisionInfo& /*collisionInfo*/, int /*deltaTimeMs*/) override
        {
        }
    };

    class StateHandler_32 final : public AbstractStateHandler
    {
    public:
        explicit StateHandler_32(LaraController& lara)
            : AbstractStateHandler(lara)
        {
        }

        std::unique_ptr<AbstractStateHandler> handleInputImpl(CollisionInfo& /*collisionInfo*/) override
        {
            if( getLevel().m_inputHandler->getInputState().jump )
                setTargetState(LaraStateId::JumpBack);

            return nullptr;
        }

        void animateImpl(CollisionInfo& /*collisionInfo*/, int /*deltaTimeMs*/) override
        {
        }

        std::unique_ptr<AbstractStateHandler> postprocessFrame(CollisionInfo& collisionInfo) override
        {
            setMovementAngle(getRotation().Y + 180_deg);
            return commonSlideHandling(collisionInfo);
        }

        loader::LaraStateId getId() const noexcept override
        {
            return LaraStateId::SlideBackward;
        }
    };

    class StateHandler_OnWater : public AbstractStateHandler
    {
    public:
        explicit StateHandler_OnWater(LaraController& lara)
            : AbstractStateHandler(lara)
        {
        }

    protected:
        std::unique_ptr<AbstractStateHandler> commonOnWaterHandling(CollisionInfo& collisionInfo)
        {
            collisionInfo.yAngle = getMovementAngle();
            collisionInfo.initHeightInfo(getPosition() + core::ExactTRCoordinates(0, 700, 0), getLevel(), 700);
            applyCollisionFeedback(collisionInfo);
            if( collisionInfo.current.floor.distance < 0
                || collisionInfo.axisCollisions == CollisionInfo::AxisColl_InvalidPosition
                || collisionInfo.axisCollisions == CollisionInfo::AxisColl_InsufficientFrontCeilingSpace
                || collisionInfo.axisCollisions == CollisionInfo::AxisColl_ScalpCollision
                || collisionInfo.axisCollisions == CollisionInfo::AxisColl_FrontForwardBlocked
            )
            {
                setFallSpeed(core::makeInterpolatedValue(0.0f));
                setPosition(collisionInfo.position);
            }
            else
            {
                if( collisionInfo.axisCollisions == CollisionInfo::AxisColl_FrontLeftBlocked )
                    m_yRotationSpeed = 5_deg;
                else if( collisionInfo.axisCollisions == CollisionInfo::AxisColl_FrontRightBlocked )
                    m_yRotationSpeed = -5_deg;
                else
                    m_yRotationSpeed = 0_deg;
            }

            auto wsh = getController().getWaterSurfaceHeight();
            if( wsh && *wsh > getPosition().Y - 100 )
            {
                return tryClimbOutOfWater(collisionInfo);
            }

            setTargetState(LaraStateId::UnderwaterForward);
            playAnimation(loader::AnimationId::FREE_FALL_TO_UNDERWATER_ALTERNATE, 2041);
            setXRotation(-45_deg);
            setFallSpeed(core::makeInterpolatedValue(80.0f));
            setUnderwaterState(UnderwaterState::Diving);
            return createWithRetainedAnimation(LaraStateId::UnderwaterDiving);
        }

    private:
        std::unique_ptr<AbstractStateHandler> tryClimbOutOfWater(CollisionInfo& collisionInfo)
        {
            if( getMovementAngle() != getRotation().Y )
                return nullptr;

            if( collisionInfo.axisCollisions != CollisionInfo::AxisColl_FrontForwardBlocked )
                return nullptr;

            if( !getLevel().m_inputHandler->getInputState().action )
                return nullptr;

            const auto gradient = std::abs(collisionInfo.frontLeft.floor.distance - collisionInfo.frontRight.floor.distance);
            if( gradient >= core::MaxGrabbableGradient )
                return nullptr;

            if( collisionInfo.front.ceiling.distance > 0 )
                return nullptr;

            if( collisionInfo.current.ceiling.distance > -core::ClimbLimit2ClickMin )
                return nullptr;

            if( collisionInfo.front.floor.distance + 700 <= -2 * loader::QuarterSectorSize )
                return nullptr;

            if( collisionInfo.front.floor.distance + 700 > 100 )
                return nullptr;

            const auto yRot = core::alignRotation(getRotation().Y, 35_deg);
            if( !yRot )
                return nullptr;

            setPosition(getPosition() + core::ExactTRCoordinates(0, 695 + gsl::narrow_cast<float>(collisionInfo.front.floor.distance), 0));
            getController().updateFloorHeight(-381);
            core::ExactTRCoordinates d = getPosition();
            if( *yRot == 0_deg )
                d.Z = (std::floor(getPosition().Z / loader::SectorSize) + 1) * loader::SectorSize + 100;
            else if( *yRot == 180_deg )
                d.Z = (std::floor(getPosition().Z / loader::SectorSize) + 0) * loader::SectorSize - 100;
            else if( *yRot == -90_deg )
                d.X = (std::floor(getPosition().X / loader::SectorSize) + 0) * loader::SectorSize - 100;
            else if( *yRot == 90_deg )
                d.X = (std::floor(getPosition().X / loader::SectorSize) + 1) * loader::SectorSize + 100;
            else
                throw std::runtime_error("Unexpected angle value");

            setPosition(d);

            setTargetState(LaraStateId::Stop);
            playAnimation(loader::AnimationId::CLIMB_OUT_OF_WATER, 1849);
            setHorizontalSpeed(core::makeInterpolatedValue(0.0f));
            setFallSpeed(core::makeInterpolatedValue(0.0f));
            setFalling(false);
            setXRotation(0_deg);
            setYRotation(*yRot);
            setZRotation(0_deg);
            setHandStatus(1);
            setUnderwaterState(UnderwaterState::OnLand);
            return createWithRetainedAnimation(LaraStateId::OnWaterExit);
        }
    };

    class StateHandler_33 final : public StateHandler_OnWater
    {
    public:
        explicit StateHandler_33(LaraController& lara)
            : StateHandler_OnWater(lara)
        {
        }

        std::unique_ptr<AbstractStateHandler> handleInputImpl(CollisionInfo& /*collisionInfo*/) override
        {
            if( getHealth() <= 0 )
            {
                setTargetState(LaraStateId::WaterDeath);
                return nullptr;
            }

            if( getLevel().m_inputHandler->getInputState().zMovement == AxisMovement::Forward )
                setTargetState(LaraStateId::OnWaterForward);
            else if( getLevel().m_inputHandler->getInputState().zMovement == AxisMovement::Backward )
                setTargetState(LaraStateId::OnWaterBackward);

            if( getLevel().m_inputHandler->getInputState().stepMovement == AxisMovement::Left )
                setTargetState(LaraStateId::OnWaterLeft);
            else if( getLevel().m_inputHandler->getInputState().stepMovement == AxisMovement::Right )
                setTargetState(LaraStateId::OnWaterRight);

            if( !getLevel().m_inputHandler->getInputState().jump )
            {
                setSwimToDiveKeypressDuration(0);
                return nullptr;
            }

            if(!getSwimToDiveKeypressDuration())
                return nullptr; // not allowed to dive at all

            if(*getSwimToDiveKeypressDuration() * 30 / 1000 < 10)
                return nullptr; // not yet allowed to dive

            setTargetState(LaraStateId::UnderwaterForward);
            playAnimation(loader::AnimationId::FREE_FALL_TO_UNDERWATER_ALTERNATE, 2041);
            setXRotation(-45_deg);
            setFallSpeed(core::makeInterpolatedValue(80.0f));
            setUnderwaterState(UnderwaterState::Diving);
            return createWithRetainedAnimation(LaraStateId::UnderwaterDiving);
        }

        void animateImpl(CollisionInfo& /*collisionInfo*/, int deltaTimeMs) override
        {
            if(getLevel().m_inputHandler->getInputState().freeLook)
            {
                getLevel().m_cameraController->setCamOverrideType(2);
                getLevel().m_cameraController->addHeadRotationXY(
                    -FreeLookMouseMovementScale * getLevel().m_inputHandler->getInputState().mouseMovement.y,
                    FreeLookMouseMovementScale * getLevel().m_inputHandler->getInputState().mouseMovement.x
                );

                getLevel().m_cameraController->setTorsoRotation(getLevel().m_cameraController->getHeadRotation());
            }
            else if(getLevel().m_cameraController->getCamOverrideType() == 2)
            {
                getLevel().m_cameraController->setCamOverrideType(0);
            }

            setFallSpeed(std::max(core::makeInterpolatedValue(0.0f), getFallSpeed() - core::makeInterpolatedValue(4.0f).getScaled(deltaTimeMs)));

            if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Left )
                m_yRotationSpeed = -4_deg;
            else if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Right )
                m_yRotationSpeed = 4_deg;
            else
                m_yRotationSpeed = 0_deg;

            addSwimToDiveKeypressDuration(deltaTimeMs);
        }

        std::unique_ptr<AbstractStateHandler> postprocessFrame(CollisionInfo& collisionInfo) override
        {
            setMovementAngle(getRotation().Y);
            return commonOnWaterHandling(collisionInfo);
        }

        loader::LaraStateId getId() const noexcept override
        {
            return LaraStateId::OnWaterStop;
        }
    };

    class StateHandler_34 final : public StateHandler_OnWater
    {
    public:
        explicit StateHandler_34(LaraController& lara)
            : StateHandler_OnWater(lara)
        {
        }

        std::unique_ptr<AbstractStateHandler> handleInputImpl(CollisionInfo& /*collisionInfo*/) override
        {
            if( getHealth() <= 0 )
            {
                setTargetState(LaraStateId::WaterDeath);
                return nullptr;
            }

            setSwimToDiveKeypressDuration(0);

            if( getLevel().m_inputHandler->getInputState().zMovement != AxisMovement::Forward )
                setTargetState(LaraStateId::OnWaterStop);

            if( getLevel().m_inputHandler->getInputState().jump )
                setTargetState(LaraStateId::OnWaterStop);

            return nullptr;
        }

        void animateImpl(CollisionInfo& /*collisionInfo*/, int deltaTimeMs) override
        {
            setFallSpeed(std::min(core::makeInterpolatedValue(60.0f), getFallSpeed() + core::makeInterpolatedValue(8.0f).getScaled(deltaTimeMs)));

            if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Left )
                m_yRotationSpeed = -4_deg;
            else if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Right )
                m_yRotationSpeed = 4_deg;
            else
                m_yRotationSpeed = 0_deg;
        }

        std::unique_ptr<AbstractStateHandler> postprocessFrame(CollisionInfo& collisionInfo) override
        {
            setMovementAngle(getRotation().Y);
            return commonOnWaterHandling(collisionInfo);
        }

        loader::LaraStateId getId() const noexcept override
        {
            return LaraStateId::OnWaterForward;
        }
    };

    class StateHandler_35 final : public StateHandler_Underwater
    {
    public:
        explicit StateHandler_35(LaraController& lara)
            : StateHandler_Underwater(lara)
        {
        }

        std::unique_ptr<AbstractStateHandler> handleInputImpl(CollisionInfo& /*collisionInfo*/) override
        {
            if( getLevel().m_inputHandler->getInputState().zMovement == AxisMovement::Forward )
                m_yRotationSpeed = -1_deg;
            else
                m_yRotationSpeed = 0_deg;

            return nullptr;
        }

        void animateImpl(CollisionInfo& /*collisionInfo*/, int /*deltaTimeMs*/) override
        {
        }

        loader::LaraStateId getId() const noexcept override
        {
            return LaraStateId::UnderwaterDiving;
        }
    };

    class StateHandler_Pushable : public AbstractStateHandler
    {
    public:
        explicit StateHandler_Pushable(LaraController& lara)
            : AbstractStateHandler(lara)
        {
        }

        std::unique_ptr<AbstractStateHandler> handleInputImpl(CollisionInfo& collisionInfo) override final
        {
            collisionInfo.frobbelFlags &= ~(CollisionInfo::FrobbelFlag08 | CollisionInfo::FrobbelFlag10);
            setCameraUnknown1(1);
            setCameraRotation(-25_deg, 35_deg);
            return nullptr;
        }

        void animateImpl(CollisionInfo& /*collisionInfo*/, int /*deltaTimeMs*/) override final
        {
        }

        std::unique_ptr<AbstractStateHandler> postprocessFrame(CollisionInfo& collisionInfo) override final
        {
            setMovementAngle(getRotation().Y);
            collisionInfo.neededFloorDistanceBottom = core::ClimbLimit2ClickMin;
            collisionInfo.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
            collisionInfo.neededCeilingDistance = 0;
            collisionInfo.frobbelFlags |= CollisionInfo::FrobbelFlag_UnwalkableSteepFloor | CollisionInfo::FrobbelFlag_UnpassableSteepUpslant;

            collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);

            return nullptr;
        }
    };

    class StateHandler_36 final : public StateHandler_Pushable
    {
    public:
        explicit StateHandler_36(LaraController& lara)
            : StateHandler_Pushable(lara)
        {
        }

        loader::LaraStateId getId() const noexcept override
        {
            return LaraStateId::PushablePush;
        }
    };

    class StateHandler_37 final : public StateHandler_Pushable
    {
    public:
        explicit StateHandler_37(LaraController& lara)
            : StateHandler_Pushable(lara)
        {
        }

        loader::LaraStateId getId() const noexcept override
        {
            return LaraStateId::PushablePull;
        }
    };

    class StateHandler_38 final : public AbstractStateHandler
    {
    public:
        explicit StateHandler_38(LaraController& lara)
            : AbstractStateHandler(lara)
        {
        }

        std::unique_ptr<AbstractStateHandler> handleInputImpl(CollisionInfo& collisionInfo) override
        {
            collisionInfo.frobbelFlags &= ~(CollisionInfo::FrobbelFlag08 | CollisionInfo::FrobbelFlag10);
            setCameraRotationY(75_deg);
            if(!getLevel().m_inputHandler->getInputState().action)
                setTargetState(LaraStateId::Stop);
            else
                setTargetState(LaraStateId::PushableGrab);
            return nullptr;
        }

        void animateImpl(CollisionInfo& /*collisionInfo*/, int /*deltaTimeMs*/) override
        {
        }

        loader::LaraStateId getId() const noexcept override
        {
            return LaraStateId::PushableGrab;
        }

        std::unique_ptr<AbstractStateHandler> postprocessFrame(CollisionInfo& collisionInfo) override
        {
            setMovementAngle(getRotation().Y);
            collisionInfo.neededFloorDistanceBottom = core::ClimbLimit2ClickMin;
            collisionInfo.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
            collisionInfo.neededCeilingDistance = 0;
            collisionInfo.frobbelFlags |= CollisionInfo::FrobbelFlag_UnwalkableSteepFloor | CollisionInfo::FrobbelFlag_UnpassableSteepUpslant;

            collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);

            return nullptr;
        }
    };

    class StateHandler_40 final : public AbstractStateHandler
    {
    public:
        explicit StateHandler_40(LaraController& lara)
            : AbstractStateHandler(lara)
        {
        }

        std::unique_ptr<AbstractStateHandler> handleInputImpl(CollisionInfo& collisionInfo) override
        {
            collisionInfo.frobbelFlags &= ~(CollisionInfo::FrobbelFlag10 | CollisionInfo::FrobbelFlag08);
            setCameraRotation(-25_deg, 80_deg);
            setCameraDistance(1024);

            return nullptr;
        }

        void animateImpl(CollisionInfo& /*collisionInfo*/, int /*deltaTimeMs*/) override
        {
        }

        loader::LaraStateId getId() const noexcept override
        {
            return LaraStateId::SwitchDown;
        }

        std::unique_ptr<AbstractStateHandler> postprocessFrame(CollisionInfo& collisionInfo) override
        {
            setMovementAngle(getRotation().Y);
            collisionInfo.yAngle = getRotation().Y;
            collisionInfo.neededFloorDistanceBottom = core::ClimbLimit2ClickMin;
            collisionInfo.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
            collisionInfo.neededCeilingDistance = 0;
            collisionInfo.frobbelFlags |= CollisionInfo::FrobbelFlag_UnwalkableSteepFloor | CollisionInfo::FrobbelFlag_UnpassableSteepUpslant;

            collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);

            return nullptr;
        }
     };

    class StateHandler_41 final : public AbstractStateHandler
    {
    public:
        explicit StateHandler_41(LaraController& lara)
            : AbstractStateHandler(lara)
        {
        }

        std::unique_ptr<AbstractStateHandler> handleInputImpl(CollisionInfo& collisionInfo) override
        {
            collisionInfo.frobbelFlags &= ~(CollisionInfo::FrobbelFlag10 | CollisionInfo::FrobbelFlag08);
            setCameraRotation(-25_deg, 80_deg);
            setCameraDistance(1024);

            return nullptr;
        }

        void animateImpl(CollisionInfo& /*collisionInfo*/, int /*deltaTimeMs*/) override
        {
        }

        loader::LaraStateId getId() const noexcept override
        {
            return LaraStateId::SwitchUp;
        }

        std::unique_ptr<AbstractStateHandler> postprocessFrame(CollisionInfo& collisionInfo) override
        {
            setMovementAngle(getRotation().Y);
            collisionInfo.yAngle = getRotation().Y;
            collisionInfo.neededFloorDistanceBottom = core::ClimbLimit2ClickMin;
            collisionInfo.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
            collisionInfo.neededCeilingDistance = 0;
            collisionInfo.frobbelFlags |= CollisionInfo::FrobbelFlag_UnwalkableSteepFloor | CollisionInfo::FrobbelFlag_UnpassableSteepUpslant;

            collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);

            return nullptr;
        }
     };

    class StateHandler_44 final : public StateHandler_Underwater
    {
    public:
        explicit StateHandler_44(LaraController& lara)
            : StateHandler_Underwater(lara)
        {
        }

        std::unique_ptr<AbstractStateHandler> handleInputImpl(CollisionInfo& /*collisionInfo*/) override
        {
            if( getRotation().X < 0_deg )
                m_xRotationSpeed = -2_deg;
            else if( getRotation().X > 0_deg )
                m_xRotationSpeed = 2_deg;
            else
                m_xRotationSpeed = 0_deg;

            return nullptr;
        }

        void animateImpl(CollisionInfo& /*collisionInfo*/, int deltaTimeMs) override
        {
            setFallSpeed(std::max(core::makeInterpolatedValue(0.0f), getFallSpeed() - core::makeInterpolatedValue(8.0f).getScaled(deltaTimeMs)));
        }

        loader::LaraStateId getId() const noexcept override
        {
            return LaraStateId::WaterDeath;
        }

        std::unique_ptr<AbstractStateHandler> postprocessFrame(CollisionInfo& collisionInfo) override
        {
            setHealth(core::makeInterpolatedValue(-1.0f));
            setAir(core::makeInterpolatedValue(-1.0f));
            setHandStatus(1);
            auto h = getController().getWaterSurfaceHeight();
            if( h && *h < getPosition().Y - 100 )
                setPosition(getPosition() - core::ExactTRCoordinates(0, 5, 0));

            return StateHandler_Underwater::postprocessFrame(collisionInfo);
        }
    };

    class StateHandler_45 final : public AbstractStateHandler
    {
    public:
        explicit StateHandler_45(LaraController& lara)
            : AbstractStateHandler(lara)
        {
        }

        std::unique_ptr<AbstractStateHandler> handleInputImpl(CollisionInfo& /*collisionInfo*/) override
        {
            return nullptr;
        }

        void animateImpl(CollisionInfo& /*collisionInfo*/, int /*deltaTimeMs*/) override
        {
        }

        std::unique_ptr<AbstractStateHandler> postprocessFrame(CollisionInfo& collisionInfo) override
        {
            setFalling(false);
            setFallSpeed(core::makeInterpolatedValue(0.0f));
            collisionInfo.yAngle = getRotation().Y;
            setMovementAngle(collisionInfo.yAngle);
            collisionInfo.frobbelFlags |= CollisionInfo::FrobbelFlag_UnpassableSteepUpslant;
            collisionInfo.neededFloorDistanceBottom = loader::HeightLimit;
            collisionInfo.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
            collisionInfo.neededCeilingDistance = 0;
            collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);

            auto nextHandler = stopIfCeilingBlocked(collisionInfo);
            if( nextHandler )
                return nextHandler;
            if( tryStartSlide(collisionInfo, nextHandler) )
                return nextHandler;

            if( collisionInfo.current.floor.distance <= 200 )
            {
                applyCollisionFeedback(collisionInfo);
                placeOnFloor(collisionInfo);
                return nextHandler;
            }

            playAnimation(loader::AnimationId::FREE_FALL_FORWARD, 492);
            setTargetState(LaraStateId::JumpForward);
            setFallSpeed(core::makeInterpolatedValue(0.0f));
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
        explicit StateHandler_47(LaraController& lara)
            : StateHandler_OnWater(lara)
        {
        }

        std::unique_ptr<AbstractStateHandler> handleInputImpl(CollisionInfo& /*collisionInfo*/) override
        {
            if( getHealth() <= 0 )
            {
                setTargetState(LaraStateId::WaterDeath);
                return nullptr;
            }

            setSwimToDiveKeypressDuration(0);

            if( getLevel().m_inputHandler->getInputState().zMovement != AxisMovement::Backward )
                setTargetState(LaraStateId::OnWaterStop);

            return nullptr;
        }

        void animateImpl(CollisionInfo& /*collisionInfo*/, int deltaTimeMs) override
        {
            setFallSpeed(std::min(core::makeInterpolatedValue(60.0f), getFallSpeed() + core::makeInterpolatedValue(8.0f).getScaled(deltaTimeMs)));

            if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Left )
                m_yRotationSpeed = -2_deg;
            else if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Right )
                m_yRotationSpeed = 2_deg;
            else
                m_yRotationSpeed = 0_deg;
        }

        std::unique_ptr<AbstractStateHandler> postprocessFrame(CollisionInfo& collisionInfo) override
        {
            setMovementAngle(getRotation().Y + 180_deg);
            return commonOnWaterHandling(collisionInfo);
        }

        loader::LaraStateId getId() const noexcept override
        {
            return LaraStateId::OnWaterBackward;
        }
    };

    class StateHandler_48 final : public StateHandler_OnWater
    {
    public:
        explicit StateHandler_48(LaraController& lara)
            : StateHandler_OnWater(lara)
        {
        }

        std::unique_ptr<AbstractStateHandler> handleInputImpl(CollisionInfo& /*collisionInfo*/) override
        {
            if( getHealth() <= 0 )
            {
                setTargetState(LaraStateId::WaterDeath);
                return nullptr;
            }

            setSwimToDiveKeypressDuration(0);

            if( getLevel().m_inputHandler->getInputState().stepMovement != AxisMovement::Left )
                setTargetState(LaraStateId::OnWaterStop);

            return nullptr;
        }

        void animateImpl(CollisionInfo& /*collisionInfo*/, int deltaTimeMs) override
        {
            setFallSpeed(std::min(core::makeInterpolatedValue(60.0f), getFallSpeed() + core::makeInterpolatedValue(8.0f).getScaled(deltaTimeMs)));

            if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Left )
                m_yRotationSpeed = -2_deg;
            else if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Right )
                m_yRotationSpeed = 2_deg;
            else
                m_yRotationSpeed = 0_deg;
        }

        std::unique_ptr<AbstractStateHandler> postprocessFrame(CollisionInfo& collisionInfo) override
        {
            setMovementAngle(getRotation().Y - 90_deg);
            return commonOnWaterHandling(collisionInfo);
        }

        loader::LaraStateId getId() const noexcept override
        {
            return LaraStateId::OnWaterLeft;
        }
    };

    class StateHandler_49 final : public StateHandler_OnWater
    {
    public:
        explicit StateHandler_49(LaraController& lara)
            : StateHandler_OnWater(lara)
        {
        }

        std::unique_ptr<AbstractStateHandler> handleInputImpl(CollisionInfo& /*collisionInfo*/) override
        {
            if( getHealth() <= 0 )
            {
                setTargetState(LaraStateId::WaterDeath);
                return nullptr;
            }

            setSwimToDiveKeypressDuration(0);

            if( getLevel().m_inputHandler->getInputState().stepMovement != AxisMovement::Right )
                setTargetState(LaraStateId::OnWaterStop);

            return nullptr;
        }

        void animateImpl(CollisionInfo& /*collisionInfo*/, int deltaTimeMs) override
        {
            setFallSpeed(std::min(core::makeInterpolatedValue(60.0f), getFallSpeed() + core::makeInterpolatedValue(8.0f).getScaled(deltaTimeMs)));

            if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Left )
                m_yRotationSpeed = -2_deg;
            else if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Right )
                m_yRotationSpeed = 2_deg;
            else
                m_yRotationSpeed = 0_deg;
        }

        std::unique_ptr<AbstractStateHandler> postprocessFrame(CollisionInfo& collisionInfo) override
        {
            setMovementAngle(getRotation().Y + 90_deg);
            return commonOnWaterHandling(collisionInfo);
        }

        loader::LaraStateId getId() const noexcept override
        {
            return LaraStateId::OnWaterRight;
        }
    };

    class StateHandler_52 final : public AbstractStateHandler
    {
    public:
        explicit StateHandler_52(LaraController& lara)
            : AbstractStateHandler(lara)
        {
        }

        std::unique_ptr<AbstractStateHandler> handleInputImpl(CollisionInfo& collisionInfo) override
        {
            collisionInfo.frobbelFlags &= ~CollisionInfo::FrobbelFlag10;
            collisionInfo.frobbelFlags |= CollisionInfo::FrobbelFlag08;
            if( getFallSpeed() > core::FreeFallSpeedThreshold )
                setTargetState(LaraStateId::SwandiveEnd);

            return nullptr;
        }

        void animateImpl(CollisionInfo& /*collisionInfo*/, int /*deltaTimeMs*/) override
        {
        }

        std::unique_ptr<AbstractStateHandler> postprocessFrame(CollisionInfo& collisionInfo) override
        {
            collisionInfo.neededFloorDistanceBottom = loader::HeightLimit;
            collisionInfo.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
            collisionInfo.neededCeilingDistance = 192;
            collisionInfo.yAngle = getRotation().Y;
            setMovementAngle(collisionInfo.yAngle);
            collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
            auto nextHandler = checkJumpWallSmash(collisionInfo);
            if( collisionInfo.current.floor.distance > 0 || getFallSpeed() <= 0 )
                return nextHandler;

            setTargetState(LaraStateId::Stop);
            setFallSpeed(core::makeInterpolatedValue(0.0f));
            setFalling(false);
            placeOnFloor(collisionInfo);

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
        explicit StateHandler_53(LaraController& lara)
            : AbstractStateHandler(lara)
        {
        }

        std::unique_ptr<AbstractStateHandler> handleInputImpl(CollisionInfo& collisionInfo) override
        {
            collisionInfo.frobbelFlags &= ~CollisionInfo::FrobbelFlag10;
            collisionInfo.frobbelFlags |= CollisionInfo::FrobbelFlag08;
            return nullptr;
        }

        void animateImpl(CollisionInfo& /*collisionInfo*/, int /*deltaTimeMs*/) override
        {
            dampenHorizontalSpeed(0.05f);
        }

        std::unique_ptr<AbstractStateHandler> postprocessFrame(CollisionInfo& collisionInfo) override
        {
            collisionInfo.neededFloorDistanceBottom = loader::HeightLimit;
            collisionInfo.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
            collisionInfo.neededCeilingDistance = 192;
            collisionInfo.yAngle = getRotation().Y;
            setMovementAngle(collisionInfo.yAngle);
            collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
            auto nextHandler = checkJumpWallSmash(collisionInfo);
            if( collisionInfo.current.floor.distance > 0 || getFallSpeed() <= 0 )
                return nextHandler;

            if( getFallSpeed() <= 133 )
                setTargetState(LaraStateId::Stop);
            else
                setTargetState(LaraStateId::Death);

            setFallSpeed(core::makeInterpolatedValue(0.0f));
            setFalling(false);
            placeOnFloor(collisionInfo);

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
        explicit StateHandler_54(LaraController& lara)
            : AbstractStateHandler(lara)
        {
        }

        std::unique_ptr<AbstractStateHandler> handleInputImpl(CollisionInfo& collisionInfo) override
        {
            collisionInfo.frobbelFlags &= ~(CollisionInfo::FrobbelFlag08 | CollisionInfo::FrobbelFlag10);
            return nullptr;
        }

        std::unique_ptr<AbstractStateHandler> postprocessFrame(CollisionInfo& collisionInfo) override
        {
            collisionInfo.neededFloorDistanceBottom = core::ClimbLimit2ClickMin;
            collisionInfo.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
            collisionInfo.neededCeilingDistance = 0;
            collisionInfo.frobbelFlags |= CollisionInfo::FrobbelFlag_UnwalkableSteepFloor | CollisionInfo::FrobbelFlag_UnpassableSteepUpslant;
            collisionInfo.yAngle = getRotation().Y;
            setMovementAngle(collisionInfo.yAngle);
            collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
            return nullptr;
        }

        loader::LaraStateId getId() const noexcept override
        {
            return LaraStateId::Handstand;
        }

        void animateImpl(CollisionInfo& /*collisionInfo*/, int /*deltaTimeMs*/) override
        {
        }
    };

    class StateHandler_55 final : public AbstractStateHandler
    {
    public:
        explicit StateHandler_55(LaraController& lara)
            : AbstractStateHandler(lara)
        {
        }

        std::unique_ptr<AbstractStateHandler> handleInputImpl(CollisionInfo& collisionInfo) override
        {
            collisionInfo.frobbelFlags &= ~(CollisionInfo::FrobbelFlag08 | CollisionInfo::FrobbelFlag10);
            return nullptr;
        }

        std::unique_ptr<AbstractStateHandler> postprocessFrame(CollisionInfo& collisionInfo) override
        {
            collisionInfo.neededFloorDistanceBottom = core::ClimbLimit2ClickMin;
            collisionInfo.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
            collisionInfo.neededCeilingDistance = 0;
            collisionInfo.frobbelFlags |= CollisionInfo::FrobbelFlag_UnwalkableSteepFloor | CollisionInfo::FrobbelFlag_UnpassableSteepUpslant;
            collisionInfo.yAngle = getRotation().Y;
            setMovementAngle(collisionInfo.yAngle);
            collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
            return nullptr;
        }

        loader::LaraStateId getId() const noexcept override
        {
            return LaraStateId::OnWaterExit;
        }

        void animateImpl(CollisionInfo& /*collisionInfo*/, int /*deltaTimeMs*/) override
        {
        }
    };

    void AbstractStateHandler::animate(CollisionInfo& collisionInfo, uint32_t deltaTimeMs)
    {
        animateImpl(collisionInfo, deltaTimeMs);

        m_controller.rotate(
                            m_xRotationSpeed.getScaled(deltaTimeMs),
                            m_yRotationSpeed.getScaled(deltaTimeMs),
                            m_zRotationSpeed.getScaled(deltaTimeMs)
                           );
        m_controller.move(
                          m_xMovement.getScaled(deltaTimeMs),
                          m_yMovement.getScaled(deltaTimeMs),
                          m_zMovement.getScaled(deltaTimeMs)
                         );
    }

    std::unique_ptr<AbstractStateHandler> AbstractStateHandler::create(loader::LaraStateId id, LaraController& controller)
    {
        switch( id )
        {
        case LaraStateId::WalkForward:
            return std::make_unique<StateHandler_0>(controller);
        case LaraStateId::RunForward:
            return std::make_unique<StateHandler_1>(controller);
        case LaraStateId::Stop:
            return std::make_unique<StateHandler_2>(controller);
        case LaraStateId::JumpForward:
            return std::make_unique<StateHandler_3>(controller);
        case LaraStateId::Pose:
            return std::make_unique<StateHandler_4>(controller);
        case LaraStateId::RunBack:
            return std::make_unique<StateHandler_5>(controller);
        case LaraStateId::TurnRightSlow:
            return std::make_unique<StateHandler_6>(controller);
        case LaraStateId::TurnLeftSlow:
            return std::make_unique<StateHandler_7>(controller);
        case LaraStateId::Death:
            return std::make_unique<StateHandler_8>(controller);
        case LaraStateId::FreeFall:
            return std::make_unique<StateHandler_9>(controller);
        case LaraStateId::Hang:
            return std::make_unique<StateHandler_10>(controller);
        case LaraStateId::Reach:
            return std::make_unique<StateHandler_11>(controller);
        case LaraStateId::Unknown12:
            return std::make_unique<StateHandler_12>(controller);
        case LaraStateId::UnderwaterStop:
            return std::make_unique<StateHandler_13>(controller);
        case LaraStateId::JumpPrepare:
            return std::make_unique<StateHandler_15>(controller);
        case LaraStateId::WalkBackward:
            return std::make_unique<StateHandler_16>(controller);
        case LaraStateId::UnderwaterForward:
            return std::make_unique<StateHandler_17>(controller);
        case LaraStateId::UnderwaterInertia:
            return std::make_unique<StateHandler_18>(controller);
        case LaraStateId::Climbing:
            return std::make_unique<StateHandler_19>(controller);
        case LaraStateId::TurnFast:
            return std::make_unique<StateHandler_20>(controller);
        case LaraStateId::StepRight:
            return std::make_unique<StateHandler_21>(controller);
        case LaraStateId::StepLeft:
            return std::make_unique<StateHandler_22>(controller);
        case LaraStateId::RollBackward:
            return std::make_unique<StateHandler_23>(controller);
        case LaraStateId::SlideForward:
            return std::make_unique<StateHandler_24>(controller);
        case LaraStateId::JumpBack:
            return std::make_unique<StateHandler_25>(controller);
        case LaraStateId::JumpLeft:
            return std::make_unique<StateHandler_26>(controller);
        case LaraStateId::JumpRight:
            return std::make_unique<StateHandler_27>(controller);
        case LaraStateId::JumpUp:
            return std::make_unique<StateHandler_28>(controller);
        case LaraStateId::FallBackward:
            return std::make_unique<StateHandler_29>(controller);
        case LaraStateId::ShimmyLeft:
            return std::make_unique<StateHandler_30>(controller);
        case LaraStateId::ShimmyRight:
            return std::make_unique<StateHandler_31>(controller);
        case LaraStateId::SlideBackward:
            return std::make_unique<StateHandler_32>(controller);
        case LaraStateId::OnWaterStop:
            return std::make_unique<StateHandler_33>(controller);
        case LaraStateId::OnWaterForward:
            return std::make_unique<StateHandler_34>(controller);
        case LaraStateId::UnderwaterDiving:
            return std::make_unique<StateHandler_35>(controller);
        case LaraStateId::PushablePush:
            return std::make_unique<StateHandler_36>(controller);
        case LaraStateId::PushablePull:
            return std::make_unique<StateHandler_37>(controller);
        case LaraStateId::PushableGrab:
            return std::make_unique<StateHandler_38>(controller);
        case LaraStateId::SwitchDown:
            return std::make_unique<StateHandler_40>(controller);
        case LaraStateId::SwitchUp:
            return std::make_unique<StateHandler_41>(controller);
        case LaraStateId::WaterDeath:
            return std::make_unique<StateHandler_44>(controller);
        case LaraStateId::RollForward:
            return std::make_unique<StateHandler_45>(controller);
        case LaraStateId::OnWaterBackward:
            return std::make_unique<StateHandler_47>(controller);
        case LaraStateId::OnWaterLeft:
            return std::make_unique<StateHandler_48>(controller);
        case LaraStateId::OnWaterRight:
            return std::make_unique<StateHandler_49>(controller);
        case LaraStateId::SwandiveBegin:
            return std::make_unique<StateHandler_52>(controller);
        case LaraStateId::SwandiveEnd:
            return std::make_unique<StateHandler_53>(controller);
        case LaraStateId::Handstand:
            return std::make_unique<StateHandler_54>(controller);
        case LaraStateId::OnWaterExit:
            return std::make_unique<StateHandler_55>(controller);
        default:
            BOOST_LOG_TRIVIAL(error) << "No state handler for state " << loader::toString(id);
            throw std::runtime_error("Unhandled state");
        }

        return nullptr;
    }

    std::unique_ptr<AbstractStateHandler> AbstractStateHandler::createWithRetainedAnimation(loader::LaraStateId id) const
    {
        auto handler = create(id, m_controller);
        handler->m_xRotationSpeed = m_xRotationSpeed;
        handler->m_yRotationSpeed = m_yRotationSpeed;
        handler->m_zRotationSpeed = m_zRotationSpeed;
        handler->m_xMovement = m_xMovement;
        handler->m_yMovement = m_yMovement;
        handler->m_zMovement = m_zMovement;
        return handler;
    }

    const core::InterpolatedValue<float>& AbstractStateHandler::getHealth() const noexcept
    {
        return m_controller.getHealth();
    }

    void AbstractStateHandler::setHealth(const core::InterpolatedValue<float>& h) noexcept
    {
        m_controller.setHealth(h);
    }

    void AbstractStateHandler::setAir(const core::InterpolatedValue<float>& a) noexcept
    {
        m_controller.setAir(a);
    }

    void AbstractStateHandler::setMovementAngle(core::Angle angle) noexcept
    {
        m_controller.setMovementAngle(angle);
    }

    core::Angle AbstractStateHandler::getMovementAngle() const noexcept
    {
        return m_controller.getMovementAngle();
    }

    void AbstractStateHandler::setFallSpeed(const core::InterpolatedValue<float>& spd)
    {
        m_controller.setFallSpeed(spd);
    }

    const core::InterpolatedValue<float>& AbstractStateHandler::getFallSpeed() const noexcept
    {
        return m_controller.getFallSpeed();
    }

    bool AbstractStateHandler::isFalling() const noexcept
    {
        return m_controller.isFalling();
    }

    void AbstractStateHandler::setFalling(bool falling) noexcept
    {
        m_controller.setFalling(falling);
    }

    int AbstractStateHandler::getHandStatus() const noexcept
    {
        return m_controller.getHandStatus();
    }

    void AbstractStateHandler::setHandStatus(int status) noexcept
    {
        m_controller.setHandStatus(status);
    }

    uint32_t AbstractStateHandler::getCurrentFrame() const
    {
        return m_controller.getCurrentFrame();
    }

    loader::LaraStateId AbstractStateHandler::getCurrentAnimState() const
    {
        return m_controller.getCurrentAnimState();
    }

    void AbstractStateHandler::playAnimation(loader::AnimationId anim, const boost::optional<uint32_t>& firstFrame)
    {
        m_controller.playAnimation(anim, firstFrame);
    }

    const core::TRRotation& AbstractStateHandler::getRotation() const noexcept
    {
        return m_controller.getRotation();
    }

    void AbstractStateHandler::setHorizontalSpeed(const core::InterpolatedValue<float>& speed)
    {
        m_controller.setHorizontalSpeed(speed);
    }

    const core::InterpolatedValue<float>& AbstractStateHandler::getHorizontalSpeed() const
    {
        return m_controller.getHorizontalSpeed();
    }

    const level::Level& AbstractStateHandler::getLevel() const
    {
        return m_controller.getLevel();
    }

    void AbstractStateHandler::placeOnFloor(const CollisionInfo& collisionInfo)
    {
        m_controller.placeOnFloor(collisionInfo);
    }

    const core::ExactTRCoordinates& AbstractStateHandler::getPosition() const
    {
        return m_controller.getPosition();
    }

    void AbstractStateHandler::setPosition(const core::ExactTRCoordinates& pos)
    {
        m_controller.setPosition(pos);
    }

    long AbstractStateHandler::getFloorHeight() const
    {
        return m_controller.getFloorHeight();
    }

    void AbstractStateHandler::setFloorHeight(long h)
    {
        m_controller.setFloorHeight(h);
    }

    void AbstractStateHandler::setYRotationSpeed(core::Angle spd)
    {
        m_controller.setYRotationSpeed(spd);
    }

    core::Angle AbstractStateHandler::getYRotationSpeed() const
    {
        return m_controller.getYRotationSpeed();
    }

    void AbstractStateHandler::subYRotationSpeed(core::Angle val, core::Angle limit)
    {
        m_controller.subYRotationSpeed(val, limit);
    }

    void AbstractStateHandler::addYRotationSpeed(core::Angle val, core::Angle limit)
    {
        m_controller.addYRotationSpeed(val, limit);
    }

    void AbstractStateHandler::setXRotation(core::Angle x)
    {
        m_controller.setXRotation(x);
    }

    void AbstractStateHandler::setYRotation(core::Angle y)
    {
        m_controller.setYRotation(y);
    }

    void AbstractStateHandler::setZRotation(core::Angle z)
    {
        m_controller.setZRotation(z);
    }

    void AbstractStateHandler::setFallSpeedOverride(int v)
    {
        m_controller.setFallSpeedOverride(v);
    }

    void AbstractStateHandler::dampenHorizontalSpeed(float f)
    {
        m_controller.dampenHorizontalSpeed(f);
    }

    core::Angle AbstractStateHandler::getCurrentSlideAngle() const noexcept
    {
        return m_controller.getCurrentSlideAngle();
    }

    void AbstractStateHandler::setCurrentSlideAngle(core::Angle a) noexcept
    {
        m_controller.setCurrentSlideAngle(a);
    }

    void AbstractStateHandler::setTargetState(loader::LaraStateId state)
    {
        m_controller.setTargetState(state);
    }

    loader::LaraStateId AbstractStateHandler::getTargetState() const
    {
        return m_controller.getTargetState();
    }

    bool AbstractStateHandler::canClimbOnto(core::Axis axis) const
    {
        auto pos = getPosition();
        switch( axis )
        {
        case core::Axis::PosZ: pos.Z += 256;
            break;
        case core::Axis::PosX: pos.X += 256;
            break;
        case core::Axis::NegZ: pos.Z -= 256;
            break;
        case core::Axis::NegX: pos.X -= 256;
            break;
        }

        auto sector = getLevel().findFloorSectorWithClampedPosition(pos.toInexact(), m_controller.getCurrentRoom());
        HeightInfo floor = HeightInfo::fromFloor(sector, pos.toInexact(), getLevel().m_cameraController);
        HeightInfo ceil = HeightInfo::fromCeiling(sector, pos.toInexact(), getLevel().m_cameraController);
        return floor.distance != -loader::HeightLimit && floor.distance - pos.Y > 0 && ceil.distance - pos.Y < -400;
    }

    std::unique_ptr<AbstractStateHandler> AbstractStateHandler::tryReach(CollisionInfo& collisionInfo)
    {
        if( collisionInfo.axisCollisions != CollisionInfo::AxisColl_FrontForwardBlocked || !getLevel().m_inputHandler->getInputState().action || getHandStatus() != 0 )
            return nullptr;

        if( std::abs(collisionInfo.frontLeft.floor.distance - collisionInfo.frontRight.floor.distance) >= core::MaxGrabbableGradient )
            return nullptr;

        if( collisionInfo.front.ceiling.distance > 0 || collisionInfo.current.ceiling.distance > -core::ClimbLimit2ClickMin || collisionInfo.current.floor.distance < 200 )
            return nullptr;

        const auto bbox = getBoundingBox();
        long spaceToReach = collisionInfo.front.floor.distance - bbox.min.y;

        if( spaceToReach < 0 && getFallSpeed() + spaceToReach < 0 )
            return nullptr;
        if( spaceToReach > 0 && getFallSpeed() + spaceToReach > 0 )
            return nullptr;

        auto alignedRotation = core::alignRotation(getRotation().Y, 35_deg);
        if( !alignedRotation )
            return nullptr;

        if( canClimbOnto(*core::axisFromAngle(getRotation().Y, 35_deg)) )
            playAnimation(loader::AnimationId::OSCILLATE_HANG_ON, 3974);
        else
            playAnimation(loader::AnimationId::HANG_IDLE, 1493);

        setTargetState(LaraStateId::Hang);
        setPosition(getPosition() + core::ExactTRCoordinates(collisionInfo.collisionFeedback.X, spaceToReach, collisionInfo.collisionFeedback.Z));
        setHorizontalSpeed(core::makeInterpolatedValue(0.0f));
        setYRotation(*alignedRotation);
        setFalling(false);
        setFallSpeed(core::makeInterpolatedValue(0.0f));
        setHandStatus(1);
        return createWithRetainedAnimation(LaraStateId::Hang);
    }

    std::unique_ptr<AbstractStateHandler> AbstractStateHandler::stopIfCeilingBlocked(const CollisionInfo& collisionInfo)
    {
        if( collisionInfo.axisCollisions != CollisionInfo::AxisColl_ScalpCollision && collisionInfo.axisCollisions != CollisionInfo::AxisColl_InvalidPosition )
            return nullptr;

        setPosition(collisionInfo.position);

        setTargetState(LaraStateId::Stop);
        playAnimation(loader::AnimationId::STAY_SOLID, 185);
        setHorizontalSpeed(core::makeInterpolatedValue(0.0f));
        setFallSpeed(core::makeInterpolatedValue(0.0f));
        setFalling(false);
        return createWithRetainedAnimation(LaraStateId::Stop);
    }

    std::unique_ptr<AbstractStateHandler> AbstractStateHandler::tryClimb(CollisionInfo& collisionInfo)
    {
        if( collisionInfo.axisCollisions != CollisionInfo::AxisColl_FrontForwardBlocked || !getLevel().m_inputHandler->getInputState().action || getHandStatus() != 0 )
            return nullptr;

        const auto floorGradient = std::abs(collisionInfo.frontLeft.floor.distance - collisionInfo.frontRight.floor.distance);
        if( floorGradient >= core::MaxGrabbableGradient )
            return nullptr;

        //! @todo MAGICK +/- 30 degrees
        auto alignedRotation = core::alignRotation(getRotation().Y, 30_deg);
        if( !alignedRotation )
            return nullptr;

        const auto climbHeight = collisionInfo.front.floor.distance;
        std::unique_ptr<AbstractStateHandler> nextHandler = nullptr;
        if( climbHeight >= -core::ClimbLimit2ClickMax && climbHeight <= -core::ClimbLimit2ClickMin )
        {
            if( climbHeight < collisionInfo.front.ceiling.distance
                || collisionInfo.frontLeft.floor.distance < collisionInfo.frontLeft.ceiling.distance
                || collisionInfo.frontRight.floor.distance < collisionInfo.frontRight.ceiling.distance )
                return nullptr;

            setTargetState(LaraStateId::Stop);
            nextHandler = createWithRetainedAnimation(LaraStateId::Climbing);
            playAnimation(loader::AnimationId::CLIMB_2CLICK, 759);
            m_yMovement = 2.0f * loader::QuarterSectorSize + climbHeight;
            setHandStatus(1);
        }
        else if( climbHeight >= -core::ClimbLimit3ClickMax && climbHeight <= -core::ClimbLimit2ClickMax )
        {
            if( collisionInfo.front.floor.distance < collisionInfo.front.ceiling.distance
                || collisionInfo.frontLeft.floor.distance < collisionInfo.frontLeft.ceiling.distance
                || collisionInfo.frontRight.floor.distance < collisionInfo.frontRight.ceiling.distance )
                return nullptr;

            setTargetState(LaraStateId::Stop);
            nextHandler = createWithRetainedAnimation(LaraStateId::Climbing);
            playAnimation(loader::AnimationId::CLIMB_3CLICK, 614);
            m_yMovement = 3.0f * loader::QuarterSectorSize + climbHeight;
            setHandStatus(1);
        }
        else if(climbHeight >= -core::JumpReachableHeight && climbHeight <= -core::ClimbLimit3ClickMax)
        {
            setTargetState(LaraStateId::JumpUp);
            nextHandler = createWithRetainedAnimation(LaraStateId::Stop);
            playAnimation(loader::AnimationId::STAY_SOLID, 185);
            setFallSpeedOverride(-static_cast<int>(std::sqrt(-12 * (climbHeight + 800) + 3)));
            auto tmp = getController().processLaraAnimCommands();
            if( tmp )
                nextHandler = std::move(tmp);
        }
        else
        {
            return nullptr;
        }

        setYRotation(*alignedRotation);
        applyCollisionFeedback(collisionInfo);

        BOOST_ASSERT(nextHandler != nullptr);
        return nextHandler;
    }

    void AbstractStateHandler::applyCollisionFeedback(const CollisionInfo& collisionInfo)
    {
        setPosition(getPosition() + collisionInfo.collisionFeedback);
        collisionInfo.collisionFeedback = {0,0,0};
    }

    std::unique_ptr<AbstractStateHandler> AbstractStateHandler::checkWallCollision(CollisionInfo& collisionInfo)
    {
        if( collisionInfo.axisCollisions == CollisionInfo::AxisColl_FrontForwardBlocked || collisionInfo.axisCollisions == CollisionInfo::AxisColl_InsufficientFrontCeilingSpace )
        {
            applyCollisionFeedback(collisionInfo);
            setTargetState(LaraStateId::Stop);
            setFalling(false);
            setHorizontalSpeed(core::makeInterpolatedValue(0.0f));
            return createWithRetainedAnimation(LaraStateId::Stop);
        }

        if( collisionInfo.axisCollisions == CollisionInfo::AxisColl_FrontLeftBlocked )
        {
            applyCollisionFeedback(collisionInfo);
            m_yRotationSpeed = 5_deg;
        }
        else if( collisionInfo.axisCollisions == CollisionInfo::AxisColl_FrontRightBlocked )
        {
            applyCollisionFeedback(collisionInfo);
            m_yRotationSpeed = -5_deg;
        }

        return nullptr;
    }

    bool AbstractStateHandler::tryStartSlide(const CollisionInfo& collisionInfo, std::unique_ptr<AbstractStateHandler>& nextHandler)
    {
        auto slantX = std::abs(collisionInfo.floorSlantX);
        auto slantZ = std::abs(collisionInfo.floorSlantZ);
        if( slantX <= 2 && slantZ <= 2 )
            return false;

        core::Angle targetAngle{0_deg};
        if( collisionInfo.floorSlantX < -2 )
            targetAngle = 90_deg;
        else if( collisionInfo.floorSlantX > 2 )
            targetAngle = -90_deg;

        if( collisionInfo.floorSlantZ > std::max(2, slantX) )
            targetAngle = 180_deg;
        else if( collisionInfo.floorSlantZ < std::min(-2, -slantX) )
            targetAngle = 0_deg;

        core::Angle dy = std::abs(targetAngle - getRotation().Y);
        applyCollisionFeedback(collisionInfo);
        if( dy > 90_deg || dy < -90_deg )
        {
            if( getCurrentAnimState() != LaraStateId::SlideBackward || targetAngle != getCurrentSlideAngle() )
            {
                playAnimation(loader::AnimationId::START_SLIDE_BACKWARD, 1677);
                setTargetState(LaraStateId::SlideBackward);
                nextHandler = createWithRetainedAnimation(LaraStateId::SlideBackward);
                setMovementAngle(targetAngle);
                setCurrentSlideAngle(targetAngle);
                setYRotation(targetAngle + 180_deg);
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

    std::unique_ptr<AbstractStateHandler> AbstractStateHandler::tryGrabEdge(CollisionInfo& collisionInfo)
    {
        if( collisionInfo.axisCollisions != CollisionInfo::AxisColl_FrontForwardBlocked || !getLevel().m_inputHandler->getInputState().action || getHandStatus() != 0 )
            return nullptr;

        const auto floorGradient = std::abs(collisionInfo.frontLeft.floor.distance - collisionInfo.frontRight.floor.distance);
        if( floorGradient >= core::MaxGrabbableGradient )
            return nullptr;

        if( collisionInfo.front.ceiling.distance > 0 || collisionInfo.current.ceiling.distance > -core::ClimbLimit2ClickMin )
            return nullptr;

        auto bbox = getBoundingBox();
        long spaceToReach = collisionInfo.front.floor.distance - bbox.min.y;

        if( spaceToReach < 0 && getFallSpeed() + spaceToReach < 0 )
            return nullptr;
        if( spaceToReach > 0 && getFallSpeed() + spaceToReach > 0 )
            return nullptr;

        auto alignedRotation = core::alignRotation(getRotation().Y, 35_deg);
        if( !alignedRotation )
            return nullptr;

        setTargetState(LaraStateId::Hang);
        playAnimation(loader::AnimationId::HANG_IDLE, 1505);
        bbox = getBoundingBox();
        spaceToReach = collisionInfo.front.floor.distance - bbox.min.y;

        setPosition(getPosition() + core::ExactTRCoordinates(0, spaceToReach, 0));
        applyCollisionFeedback(collisionInfo);
        setHorizontalSpeed(core::makeInterpolatedValue(0.0f));
        setFallSpeed(core::makeInterpolatedValue(0.0f));
        setFalling(false);
        setHandStatus(1);
        setYRotation(*alignedRotation);

        return createWithRetainedAnimation(LaraStateId::Hang);
    }

    int AbstractStateHandler::getRelativeHeightAtDirection(core::Angle angle, int dist) const
    {
        auto pos = getPosition();
        pos.X += angle.sin() * dist;
        pos.Y -= core::ScalpHeight;
        pos.Z += angle.cos() * dist;

        gsl::not_null<const loader::Sector*> sector = getLevel().findFloorSectorWithClampedPosition(pos.toInexact(), m_controller.getCurrentRoom());

        HeightInfo h = HeightInfo::fromFloor(sector, pos.toInexact(), getLevel().m_cameraController);

        if( h.distance != -loader::HeightLimit )
            h.distance -= std::lround(getPosition().Y);

        return h.distance;
    }

    std::unique_ptr<AbstractStateHandler> AbstractStateHandler::commonJumpHandling(CollisionInfo& collisionInfo)
    {
        collisionInfo.neededFloorDistanceBottom = loader::HeightLimit;
        collisionInfo.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
        collisionInfo.neededCeilingDistance = 192;
        collisionInfo.yAngle = getMovementAngle();
        collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
        auto nextHandler = checkJumpWallSmash(collisionInfo);
        if( getFallSpeed() <= 0 || collisionInfo.current.floor.distance > 0 )
            return nextHandler;

        if( applyLandingDamage() )
            setTargetState(LaraStateId::Death);
        else
            setTargetState(LaraStateId::Stop);
        setFallSpeed(core::makeInterpolatedValue(0.0f));
        placeOnFloor(collisionInfo);
        setFalling(false);

        return nextHandler;
    }

    std::unique_ptr<AbstractStateHandler> AbstractStateHandler::commonSlideHandling(CollisionInfo& collisionInfo)
    {
        collisionInfo.neededFloorDistanceBottom = loader::HeightLimit;
        collisionInfo.neededFloorDistanceTop = -loader::QuarterSectorSize * 2;
        collisionInfo.neededCeilingDistance = 0;
        collisionInfo.yAngle = getMovementAngle();
        collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);

        if( auto nextHandler = stopIfCeilingBlocked(collisionInfo) )
            return nextHandler;

        auto nextHandler = checkWallCollision(collisionInfo);
        if( collisionInfo.current.floor.distance <= 200 )
        {
            tryStartSlide(collisionInfo, nextHandler);
            placeOnFloor(collisionInfo);
            const auto absSlantX = std::abs(collisionInfo.floorSlantX);
            const auto absSlantZ = std::abs(collisionInfo.floorSlantZ);
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

        setFallSpeed(core::makeInterpolatedValue(0.0f));
        setFalling(true);

        return nextHandler;
    }

    std::unique_ptr<AbstractStateHandler> AbstractStateHandler::commonEdgeHangHandling(CollisionInfo& collisionInfo)
    {
        collisionInfo.neededFloorDistanceBottom = loader::HeightLimit;
        collisionInfo.neededFloorDistanceTop = -loader::HeightLimit;
        collisionInfo.neededCeilingDistance = 0;
        collisionInfo.yAngle = getMovementAngle();
        collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
        const bool tooSteepToGrab = collisionInfo.front.floor.distance < 200;
        setFallSpeed(core::makeInterpolatedValue(0.0f));
        setFalling(false);
        setMovementAngle(getRotation().Y);
        const auto axis = *core::axisFromAngle(getMovementAngle(), 45_deg);
        switch( axis )
        {
        case core::Axis::PosZ:
            setPosition(getPosition() + core::ExactTRCoordinates(0, 0, 2));
            break;
        case core::Axis::PosX:
            setPosition(getPosition() + core::ExactTRCoordinates(2, 0, 0));
            break;
        case core::Axis::NegZ:
            setPosition(getPosition() - core::ExactTRCoordinates(0, 0, 2));
            break;
        case core::Axis::NegX:
            setPosition(getPosition() - core::ExactTRCoordinates(2, 0, 0));
            break;
        }

        collisionInfo.neededFloorDistanceBottom = loader::HeightLimit;
        collisionInfo.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
        collisionInfo.neededCeilingDistance = 0;
        collisionInfo.yAngle = getMovementAngle();
        collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
        if( !getLevel().m_inputHandler->getInputState().action || getHealth() <= 0 )
        {
            setTargetState(LaraStateId::JumpUp);
            playAnimation(loader::AnimationId::TRY_HANG_VERTICAL, 448);
            setHandStatus(0);
            const auto bbox = getBoundingBox();
            const long hangDistance = collisionInfo.front.floor.distance - bbox.min.y + 2;
            setPosition(getPosition() + core::ExactTRCoordinates(collisionInfo.collisionFeedback.X, hangDistance, collisionInfo.collisionFeedback.Z));
            setHorizontalSpeed(core::makeInterpolatedValue(2.0f));
            setFallSpeed(core::makeInterpolatedValue(1.0f));
            setFalling(true);
            return createWithRetainedAnimation(LaraStateId::JumpUp);
        }

        auto gradient = std::abs(collisionInfo.frontLeft.floor.distance - collisionInfo.frontRight.floor.distance);
        if( gradient >= core::MaxGrabbableGradient || collisionInfo.current.ceiling.distance >= 0 || collisionInfo.axisCollisions != CollisionInfo::AxisColl_FrontForwardBlocked || tooSteepToGrab )
        {
            setPosition(collisionInfo.position);
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
        case core::Axis::PosZ:
        case core::Axis::NegZ:
            setPosition(getPosition() + core::ExactTRCoordinates(0, 0, collisionInfo.collisionFeedback.Z));
            break;
        case core::Axis::PosX:
        case core::Axis::NegX:
            setPosition(getPosition() + core::ExactTRCoordinates(collisionInfo.collisionFeedback.X, 0, 0));
            break;
        }

        const auto bbox = getBoundingBox();
        const long spaceToReach = collisionInfo.front.floor.distance - bbox.min.y;

        if( spaceToReach >= -loader::QuarterSectorSize && spaceToReach <= loader::QuarterSectorSize )
            setPosition(getPosition() + core::ExactTRCoordinates(0, spaceToReach, 0));
        return nullptr;
    }

    bool AbstractStateHandler::applyLandingDamage()
    {
        auto sector = getLevel().findFloorSectorWithClampedPosition(getPosition().toInexact(), m_controller.getCurrentRoom());
        HeightInfo h = HeightInfo::fromFloor(sector, getPosition().toInexact() - core::TRCoordinates{0, core::ScalpHeight, 0}, getLevel().m_cameraController);
        setFloorHeight(h.distance);
        getController().handleTriggers(h.lastTriggerOrKill, false);
        auto damageSpeed = static_cast<float>(getFallSpeed()) - 140;
        if( damageSpeed <= 0 )
            return false;

        static constexpr int DeathSpeedLimit = 14;

        if( damageSpeed <= DeathSpeedLimit )
            setHealth(getHealth() - 1000 * damageSpeed * damageSpeed / (DeathSpeedLimit * DeathSpeedLimit));
        else
            setHealth(core::makeInterpolatedValue(-1.0f));
        return getHealth() <= 0;
    }

    gameplay::Node* AbstractStateHandler::getLara()
    {
        return m_controller.getSceneNode();
    }

    gameplay::BoundingBox AbstractStateHandler::getBoundingBox() const
    {
        return m_controller.getBoundingBox();
    }

    void AbstractStateHandler::addSwimToDiveKeypressDuration(int ms) noexcept
    {
        m_controller.addSwimToDiveKeypressDuration(ms);
    }

    void AbstractStateHandler::setSwimToDiveKeypressDuration(int ms) noexcept
    {
        m_controller.setSwimToDiveKeypressDuration(ms);
    }

    const boost::optional<int>& AbstractStateHandler::getSwimToDiveKeypressDuration() const
    {
        return m_controller.getSwimToDiveKeypressDuration();
    }

    void AbstractStateHandler::setUnderwaterState(UnderwaterState u) noexcept
    {
        m_controller.setUnderwaterState(u);
    }

    void AbstractStateHandler::setCameraRotation(core::Angle x, core::Angle y)
    {
        m_controller.setCameraRotation(x, y);
    }

    void AbstractStateHandler::setCameraRotationX(core::Angle x)
    {
        m_controller.setCameraRotationX(x);
    }

    void AbstractStateHandler::setCameraRotationY(core::Angle y)
    {
        m_controller.setCameraRotationY(y);
    }

    void AbstractStateHandler::setCameraDistance(int d)
    {
        m_controller.setCameraDistance(d);
    }

    void AbstractStateHandler::setCameraUnknown1(int k)
    {
        m_controller.setCameraUnknown1(k);
    }

    void AbstractStateHandler::jumpAgainstWall(CollisionInfo& collisionInfo)
    {
        applyCollisionFeedback(collisionInfo);
        if( collisionInfo.axisCollisions == CollisionInfo::AxisColl_FrontLeftBlocked )
            m_yRotationSpeed = 5_deg;
        else if( collisionInfo.axisCollisions == CollisionInfo::AxisColl_FrontRightBlocked )
            m_yRotationSpeed = -5_deg;
        else if( collisionInfo.axisCollisions == CollisionInfo::AxisColl_ScalpCollision )
        {
            if( getFallSpeed() <= 0 )
                setFallSpeed(core::makeInterpolatedValue(1.0f));
        }
        else if( collisionInfo.axisCollisions == CollisionInfo::AxisColl_InvalidPosition )
        {
            m_xMovement = 100 * getRotation().Y.sin();
            m_zMovement = 100 * getRotation().Y.cos();
            setHorizontalSpeed(core::makeInterpolatedValue(0.0f));
            collisionInfo.current.floor.distance = 0;
            if( getFallSpeed() < 0 )
                setFallSpeed(core::makeInterpolatedValue(16.0f));
        }
    }

    std::unique_ptr<AbstractStateHandler> AbstractStateHandler::checkJumpWallSmash(CollisionInfo& collisionInfo)
    {
        applyCollisionFeedback(collisionInfo);

        if( collisionInfo.axisCollisions == CollisionInfo::AxisColl_None )
            return nullptr;

        if( collisionInfo.axisCollisions == CollisionInfo::AxisColl_FrontForwardBlocked || collisionInfo.axisCollisions == CollisionInfo::AxisColl_InsufficientFrontCeilingSpace )
        {
            setTargetState(LaraStateId::FreeFall);
            //! @todo Check formula
            setHorizontalSpeed(getHorizontalSpeed() * 0.2f);
            setMovementAngle(getMovementAngle() - 180_deg);
            playAnimation(loader::AnimationId::SMASH_JUMP, 481);
            if( getFallSpeed() <= 0 )
                setFallSpeed(core::makeInterpolatedValue(1.0f));
            return createWithRetainedAnimation(LaraStateId::FreeFall);
        }

        if( collisionInfo.axisCollisions == CollisionInfo::AxisColl_FrontLeftBlocked )
        {
            m_yRotationSpeed = 5_deg;
            return nullptr;
        }
        else if( collisionInfo.axisCollisions == CollisionInfo::AxisColl_FrontRightBlocked )
        {
            m_yRotationSpeed = -5_deg;
            return nullptr;
        }

        if( collisionInfo.axisCollisions == CollisionInfo::AxisColl_InvalidPosition )
        {
            m_xMovement = 100 * collisionInfo.yAngle.sin();
            m_zMovement = 100 * collisionInfo.yAngle.cos();
            setHorizontalSpeed(core::makeInterpolatedValue(0.0f));
            collisionInfo.current.floor.distance = 0;
            if( getFallSpeed() <= 0 )
                setFallSpeed(core::makeInterpolatedValue(16.0f));
        }

        if( collisionInfo.axisCollisions == CollisionInfo::AxisColl_ScalpCollision && getFallSpeed() <= 0 )
            setFallSpeed(core::makeInterpolatedValue(1.0f));

        return nullptr;
    }

}
