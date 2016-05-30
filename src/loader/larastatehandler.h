#pragma once

#include "animationids.h"
#include "larastate.h"
#include "util/vmath.h"
#include "inputstate.h"

#include <irrlicht.h>

struct LaraState;

class LaraStateHandler final : public irr::scene::ISceneNodeAnimator
{
    using LaraStateId = loader::LaraStateId;

private:
    const loader::Level* const m_level;
    std::shared_ptr<loader::DefaultAnimDispatcher> m_dispatcher;
    const std::string m_name;

    irr::scene::IAnimatedMeshSceneNode* const m_lara;

    // Lara's vars
    int m_health = 1000;
    //! @brief Additional rotation in AU per TR Engine Frame
    SpeedValue<int> m_yRotationSpeed = 0;
    bool m_falling = false;
    SpeedValue<int> m_fallSpeed = 0;
    SpeedValue<int> m_horizontalSpeed = 0;
    int m_fallSpeedOverride = 0;
    int16_t m_movementAngle = 0;
    int m_air = 1800;
    int16_t m_currentSlideAngle = 0;

    InputState m_inputState;

    int m_handStatus = 0;
    int m_floorHeight = 0;
    int m_lastFrameTime = -1;
    int m_currentFrameTime = 0;
    int m_lastAnimFrame = -1;

    int getCurrentDeltaTime() const
    {
        BOOST_ASSERT(m_lastFrameTime < m_currentFrameTime);
        return m_currentFrameTime - m_lastFrameTime;
    }

    // needed for YPR rotation, because the scene node uses XYZ rotation
    irr::core::vector3df m_rotation;

    loader::ExactTRCoordinates m_position;

    using Handler = void (LaraStateHandler::*)(LaraState&);
    using HandlersArray = std::array<Handler, 56>;

    void callHandler(const HandlersArray& handlers, uint16_t state, LaraState& laraState, const char* semantic)
    {
        if( state >= handlers.size() )
        {
            BOOST_LOG_TRIVIAL(error) << "Unexpected state " << state;
            return;
        }

        if( !handlers[state] )
        BOOST_LOG_TRIVIAL(warning) << "No " << semantic << " handler for state " << state;
        else
            (this ->* handlers[state])(laraState);
    }

    void onBehaveStanding(LaraState& state);

    void onInput0WalkForward(LaraState& state);
    void onBehave0WalkForward(LaraState& state);

    void onInput1RunForward(LaraState& state);
    void onBehave1RunForward(LaraState& state);

    void onInput2Stop(LaraState& state);

    void onInput3JumpForward(LaraState& state);
    void onBehave3JumpForward(LaraState& state);

    void onInput5RunBackward(LaraState& state);
    void onBehave5RunBackward(LaraState& state);

    void onInput6TurnRightSlow(LaraState& state);
    void onInput7TurnLeftSlow(LaraState& state);
    void onBehaveTurnSlow(LaraState& state);

    void onInput9FreeFall(LaraState& state);
    void onBehave9FreeFall(LaraState& state);

    void onInput11Reach(LaraState& state);
    void onBehave11Reach(LaraState& state);

    void onBehave12Unknown(LaraState& state);

    void onInput15JumpPrepare(LaraState& state);
    void onBehave15JumpPrepare(LaraState& state);

    void onInput16WalkBackward(LaraState& state);
    void onBehave16WalkBackward(LaraState& state);

    void onInput19Climbing(LaraState& state);
    void onBehave19Climbing(LaraState& state);

    void onInput20TurnFast(LaraState& state);

    void onBehave23RollBackward(LaraState& state);

    void onInput24SlideForward(LaraState& state);
    void onBehave24SlideForward(LaraState& state);

    void onInput25JumpBackward(LaraState& state);
    void onBehave25JumpBackward(LaraState& state);

    void onInput26JumpLeft(LaraState& state);
    void onBehave26JumpLeft(LaraState& state);

    void onInput27JumpRight(LaraState& state);
    void onBehave27JumpRight(LaraState& state);

    void onInput28JumpUp(LaraState& state);
    void onBehave28JumpUp(LaraState& state);

    void onInput29FallBackward(LaraState& state);
    void onBehave29FallBackward(LaraState& state);

    void onInput32SlideBackward(LaraState& state);
    void onBehave32SlideBackward(LaraState& state);

    void onBehave45RollForward(LaraState& state);

    void onInput52SwandiveBegin(LaraState& state);
    void onBehave52SwandiveBegin(LaraState& state);

    void onInput53SwandiveEnd(LaraState& state);
    void onBehave53SwandiveEnd(LaraState& state);

    void nopHandler(LaraState&)
    {
    }

public:
    LaraStateHandler(const loader::Level* level, const std::shared_ptr<loader::DefaultAnimDispatcher>& dispatcher, irr::scene::IAnimatedMeshSceneNode* lara, const std::string& name)
        : m_level(level), m_dispatcher(dispatcher), m_name(name), m_lara(lara)
    {
        BOOST_ASSERT(level != nullptr);
        BOOST_ASSERT(dispatcher != nullptr);
        BOOST_ASSERT(lara != nullptr);
        playAnimation(loader::AnimationId::STAY_IDLE);

        auto laraRot = lara->getRotation();
        m_rotation.X = util::degToAu(laraRot.X);
        m_rotation.Y = util::degToAu(laraRot.Y);
        m_rotation.Z = util::degToAu(laraRot.Z);

        setMovementAngle(getRotation().Y);

        m_lara->updateAbsolutePosition();
        m_position = loader::ExactTRCoordinates(m_lara->getAbsolutePosition());
    }

    ~LaraStateHandler() = default;

    virtual void animateNode(irr::scene::ISceneNode* node, irr::u32 timeMs) override;

    enum class AnimCommandOpcode : uint16_t
    {
        SetPosition = 1,
        SetVelocity = 2,
        EmptyHands = 3,
        Kill = 4,
        PlaySound = 5,
        PlayEffect = 6,
        Interact = 7
    };

    void processAnimCommands();

    virtual ISceneNodeAnimator* createClone(irr::scene::ISceneNode* /*node*/, irr::scene::ISceneManager* /*newManager*/ = nullptr) override
    {
        BOOST_ASSERT(false);
        return nullptr;
    }

    void setInputState(const InputState& state)
    {
        m_inputState = state;
    }

    irr::scene::IAnimatedMeshSceneNode* getLara() const noexcept
    {
        return m_lara;
    }

    loader::TRCoordinates getPosition() const noexcept
    {
        return m_position.toInexact();
    }

private:
    void setTargetState(loader::LaraStateId st);
    void setStateOverride(loader::LaraStateId st);
    void clearStateOverride();

    LaraStateId getTargetState() const;

    void playAnimation(loader::AnimationId anim, const boost::optional<irr::u32>& firstFrame = boost::none);

    bool tryStopOnFloor(LaraState& state);
    bool tryClimb(LaraState& state);
    bool checkWallCollision(LaraState& state);
    bool tryStartSlide(LaraState& state);
    bool tryGrabEdge(LaraState& state);
    void jumpAgainstWall(LaraState& state);
    void checkJumpWallSmash(LaraState& state);

    void applyCollisionFeedback(LaraState& state);
    void handleTriggers(const uint16_t* floorData, bool skipFirstTriggers);
    void updateFloorHeight(int dy);
    int getRelativeHeightAtDirection(int16_t angle, int dist) const;
    void commonJumpHandling(LaraState& state);
    void commonSlideHandling(LaraState& state);
    bool tryReach(LaraState& state);
    bool canClimbOnto(int16_t angle) const;

    bool applyLandingDamage(LaraState& state);

    void handleLaraStateOnLand();

    ///////////////////////////////////////

    int getHealth() const noexcept
    {
        return m_health;
    }

    void setHealth(int h) noexcept
    {
        m_health = h;
    }

    const InputState& getInputState() const noexcept
    {
        return m_inputState;
    }

    void setMovementAngle(int16_t angle) noexcept
    {
        m_movementAngle = angle;
    }

    int16_t getMovementAngle() const noexcept
    {
        return m_movementAngle;
    }

    void setFallSpeed(int spd)
    {
        m_fallSpeed = spd;
    }

    const SpeedValue<int>& getFallSpeed() const noexcept
    {
        return m_fallSpeed;
    }

    bool isFalling() const noexcept
    {
        return m_falling;
    }

    void setFalling(bool falling) noexcept
    {
        m_falling = falling;
    }

    int getHandStatus() const noexcept
    {
        return m_handStatus;
    }

    void setHandStatus(int status) noexcept
    {
        m_handStatus = status;
    }

    irr::u32 getCurrentFrame() const;

    const irr::core::vector3df& getRotation() const noexcept
    {
        return m_rotation;
    }

    void setHorizontalSpeed(int speed)
    {
        m_horizontalSpeed = speed;
    }

    const loader::Level& getLevel() const
    {
        BOOST_ASSERT(m_level != nullptr);
        return *m_level;
    }

    void placeOnFloor(const LaraState& state);

    void moveY(int distance)
    {
        m_position.Y += distance;
    }

    void moveXZ(float dx, float dz)
    {
        m_position.X += dx;
        m_position.Z += dz;
    }

    void setPosition(const loader::ExactTRCoordinates& pos)
    {
        m_position = pos;
    }

    void setFloorHeight(int h) noexcept
    {
        m_floorHeight = h;
    }

    void setYRotationSpeed(int spd)
    {
        m_yRotationSpeed = spd;
    }

    int getYRotationSpeed() const
    {
        return m_yRotationSpeed.get();
    }

    void subYRotationSpeed(int val, int limit = std::numeric_limits<int>::min())
    {
        m_yRotationSpeed.subExact(val, getCurrentDeltaTime()).limitMin(limit);
    }

    void addYRotationSpeed(int val, int limit = std::numeric_limits<int>::max())
    {
        m_yRotationSpeed.addExact(val, getCurrentDeltaTime()).limitMax(limit);
    }

    void setYRotation(int16_t y)
    {
        m_rotation.Y = y;
    }

    void addYRotation(float v)
    {
        m_rotation.Y = v;
    }

    void setZRotation(int16_t z)
    {
        m_rotation.Z = z;
    }

    void setZRotationExact(float z)
    {
        m_rotation.Z = z;
    }

    void setFallSpeedOverride(int v)
    {
        m_fallSpeedOverride = v;
    }

    void dampenHorizontalSpeed(int nom, int den)
    {
        m_horizontalSpeed.subExact(m_horizontalSpeed.getExact() * nom / den, getCurrentDeltaTime());
    }

    int16_t getCurrentSlideAngle() const noexcept
    {
        return m_currentSlideAngle;
    }

    void setCurrentSlideAngle(int16_t a) noexcept
    {
        m_currentSlideAngle = a;
    }

    loader::LaraStateId getCurrentState() const;
};
