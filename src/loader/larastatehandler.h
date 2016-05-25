#pragma once

#include "animationids.h"
#include "larastate.h"
#include "util/vmath.h"

#include <irrlicht.h>

#include <cstdint>

enum class AxisMovement
{
    Positive,
    Right = Positive,
    Forward = Positive,
    Null,
    Negative,
    Left = Negative,
    Backward = Negative
};

struct LaraState;

class LaraStateHandler final : public irr::scene::ISceneNodeAnimator
{
    using LaraState = loader::LaraState;

private:
    const loader::Level* const m_level;
    std::shared_ptr<loader::DefaultAnimDispatcher> m_dispatcher;
    const std::string m_name;
    irr::u32 m_lastActiveFrame = std::numeric_limits<irr::u32>::max();

    AxisMovement m_xMovement = AxisMovement::Null;
    AxisMovement m_zMovement = AxisMovement::Null;
    AxisMovement m_stepMovement = AxisMovement::Null;
    bool m_jump = false;
    bool m_moveSlow = false;
    bool m_roll = false;
    bool m_action = false;

    irr::scene::IAnimatedMeshSceneNode* const m_lara;

    // Lara's vars
    int m_health = 1000;
    //! @brief Additional rotation in AU per TR Engine Frame
    int m_yRotationSpeed = 0;
    bool m_falling = false;
    int m_fallSpeed = 0;
    int m_horizontalSpeed = 0;
    int m_fallSpeedOverride = 0;
    int m_movementAngle = 0;
    int m_air = 1800;
    irr::s16 m_currentSlideAngle = 0;

    // needed for YPR rotation, because the scene node uses XYZ rotation
    irr::core::vector3di m_rotation;

    loader::TRCoordinates m_position;

    using InputHandler = void (LaraStateHandler::*)();
    using BehaviourHandler = void (LaraStateHandler::*)(::LaraState&);

    void onInput0WalkForward();

    void onInput1RunForward();
    void onBehave1RunForward(::LaraState& state);

    void onInput2Stop();

    void onInput3JumpForward();
    void onBehave3JumpForward(::LaraState& state);

    void onInput5RunBackward();
    void onBehave5RunBackward(::LaraState& state);

    void onInput6TurnRightSlow();

    void onInput7TurnLeftSlow();
    
    void onInput9FreeFall();
    void onBehave9FreeFall(::LaraState& state);

    void onInput15JumpPrepare();
    void onBehave15JumpPrepare(::LaraState& state);

    void onInput16WalkBackward();
    void onBehave16WalkBackward(::LaraState& state);

    void onInput20TurnFast();

    void onInput25JumpBackward();
    
    void onInput28JumpUp();
    void onBehave28JumpUp(::LaraState& state);

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

        m_movementAngle = m_rotation.Y;

        m_lara->updateAbsolutePosition();
        m_position = loader::TRCoordinates(m_lara->getAbsolutePosition());
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
    
    void setXAxisMovement(bool left, bool right)
    {
        if(left < right)
            m_xMovement = AxisMovement::Right;
        else if(left > right)
            m_xMovement = AxisMovement::Left;
        else
            m_xMovement = AxisMovement::Null;
    }
    
    void setZAxisMovement(bool back, bool forward)
    {
        if(back < forward)
            m_zMovement = AxisMovement::Forward;
        else if(back > forward)
            m_zMovement = AxisMovement::Backward;
        else
            m_zMovement = AxisMovement::Null;
    }
    
    void setJump(bool val)
    {
        m_jump = val;
    }
    
    void setMoveSlow(bool val)
    {
        m_moveSlow = val;
    }

    irr::scene::IAnimatedMeshSceneNode* getLara() const noexcept
    {
        return m_lara;
    }

    const loader::TRCoordinates& getPosition() const noexcept
    {
        return m_position;
    }

private:
    void setTargetState(loader::LaraState st);

    LaraState getTargetState() const;
    
    void playAnimation(loader::AnimationId anim, const boost::optional<irr::u32>& firstFrame = boost::none);

    bool tryStopOnFloor(::LaraState& state);
    bool tryClimb(::LaraState& state);
    bool checkWallCollision(::LaraState& state);
    bool tryStartSlide(::LaraState& state);
    bool tryGrabEdge(::LaraState& state)
    {
        //! @todo Implement me
        return false;
    }
    void jumpAgainstWall(::LaraState& state);
    void checkJumpWallSmash(::LaraState& state);

    void applyCollisionFeedback(::LaraState& state);
    void handleTriggers(const uint16_t* floorData, bool skipFirstTriggers);
    void updateFloorHeight(int dy);
    int getRelativeHeightAtDirection(int16_t angle, int dist) const;
    void commonJumpHandling(::LaraState& state);

    void handleLaraStateOnLand();

    int m_handStatus = 0;
    int m_floorHeight = 0;
};
