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
    friend class Behavior;
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

    using InputHandler = void (LaraStateHandler::*)();
    
    void onInput0WalkForward();

    void onInput1RunForward();

    void onInput2Stop();

    void onInput3JumpForward();

    void onInput5RunBackward();

    void onInput6TurnRightSlow();

    void onInput7TurnLeftSlow();
    
    void onInput9FreeFall();

    void onInput16WalkBackward();

    void onInput20TurnFast();

    void onInput25JumpBackward();
    
    void onInput28JumpUp();

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

private:
    void setTargetState(loader::LaraState st);

    LaraState getTargetState() const;
    
    void playAnimation(loader::AnimationId anim);

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

    void applyCollisionFeedback(::LaraState& state);

    void handleLaraStateOnLand();

    int m_unknown10CD54 = 0;
    int m_handStatus = 0;
};
