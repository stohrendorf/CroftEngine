#pragma once

#include "animationids.h"
#include "larastate.h"

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

constexpr irr::f32 auToDeg(irr::s32 au)
{
    return au / 65536.0f * 360;
}

constexpr irr::f32 auToRad(irr::s32 au)
{
    return au / 65536.0f * 2 * irr::core::PI;
}

constexpr irr::s32 degToAu(irr::f32 deg)
{
    return static_cast<irr::s32>(deg * 65536 / 360);
}

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

    // needed for YPR rotation, because the scene node uses XYZ rotation
    irr::core::vector3di m_rotation;

    using InputHandler = void (LaraStateHandler::*)();
    
    void onInput0WalkForward()
    {
        if(m_health <= 0)
        {
            setTargetState(LaraState::Stop);
            return;
        }

        if(m_xMovement == AxisMovement::Left)
            m_yRotationSpeed = std::max(-728, m_yRotationSpeed - 409);
        else if(m_xMovement == AxisMovement::Right)
            m_yRotationSpeed = std::min(728, m_yRotationSpeed + 409);
        if(m_zMovement == AxisMovement::Forward)
        {
            if(m_moveSlow)
                setTargetState(LaraState::WalkForward);
            else
                setTargetState(LaraState::RunForward);
        }
        else
        {
            setTargetState(LaraState::Stop);
        }
    }

    void onInput1RunForward()
    {
        if(m_health <= 0)
        {
            setTargetState(LaraState::Death);
            return;
        }

        if(m_roll)
        {
            //! @todo Play animation from frame 3857
            playAnimation(loader::AnimationId::ROLL_BEGIN);
            setTargetState(LaraState::Stop);
            return;
        }
        if(m_xMovement == AxisMovement::Left)
        {
            m_yRotationSpeed = std::max(-1456, m_yRotationSpeed - 409);
            m_rotation.Z = std::max(-2002, m_rotation.Z - 273);
        }
        else if(m_xMovement == AxisMovement::Right)
        {
            m_yRotationSpeed = std::min(1456, m_yRotationSpeed + 409);
            m_rotation.Z = std::min(2002, m_rotation.Z + 273);
        }
        if(m_jump && !m_falling)
        {
            setTargetState(LaraState::JumpForward);
            return;
        }
        if(m_zMovement != AxisMovement::Forward)
        {
            setTargetState(LaraState::Stop);
            return;
        }
        if(m_moveSlow)
            setTargetState(LaraState::WalkForward);
        else
            setTargetState(LaraState::RunForward);
    }

    void onInput2Stop()
    {
        if(m_health <= 0)
        {
            setTargetState(LaraState::Death);
            return;
        }

        if(m_roll)
        {
            playAnimation(loader::AnimationId::ROLL_BEGIN);
            setTargetState(LaraState::Stop);
            return;
        }

        setTargetState(LaraState::Stop);
        if(m_stepMovement == AxisMovement::Left)
        {
            setTargetState(LaraState::StepLeft);
        }
        else if(m_stepMovement == AxisMovement::Right)
        {
            setTargetState(LaraState::StepRight);
        }
        if(m_xMovement == AxisMovement::Left)
        {
            setTargetState(LaraState::TurnLeftSlow);
        }
        else if(m_xMovement == AxisMovement::Right)
        {
            setTargetState(LaraState::TurnRightSlow);
        }
        if(m_jump)
        {
            setTargetState(LaraState::JumpPrepare);
        }
        if(m_zMovement == AxisMovement::Forward)
        {
            if(m_moveSlow)
                onInput0WalkForward();
            else
                onInput1RunForward();
        }
        else if(m_zMovement == AxisMovement::Backward)
        {
            if(m_moveSlow)
                onInput16WalkBackward();
            else
                setTargetState(LaraState::RunBack);
        }
    }

    void onInput3JumpForward()
    {
        if(getTargetState() == LaraState::SwandiveBegin || getTargetState() == LaraState::Reach)
            setTargetState(LaraState::JumpForward);

        if(getTargetState() != LaraState::Death && getTargetState() != LaraState::Stop)
        {
            //! @todo Not only m_action, but also free hands!
            if(m_action)
                setTargetState(LaraState::Reach);
            //! @todo Not only m_action, but also free hands!
            if(m_moveSlow)
                setTargetState(LaraState::SwandiveBegin);
            if(m_fallSpeed > 131)
                setTargetState(LaraState::FreeFall);
        }

        if(m_xMovement == AxisMovement::Left)
        {
            m_yRotationSpeed = std::max(-546, m_yRotationSpeed - 409);
        }
        else if(m_xMovement == AxisMovement::Right)
        {
            m_yRotationSpeed = std::min(546, m_yRotationSpeed + 409);
        }
    }

    void onInput5RunBackward()
    {
        setTargetState(LaraState::Stop);

        if(m_xMovement == AxisMovement::Left)
            m_yRotationSpeed = std::max(-1092, m_yRotationSpeed - 409);
        else if(m_xMovement == AxisMovement::Right)
            m_yRotationSpeed = std::min(1092, m_yRotationSpeed + 409);
    }

    void onInput6TurnRightSlow()
    {
        if(m_health <= 0)
        {
            setTargetState(LaraState::Stop);
            return;
        }

        m_yRotationSpeed += 409;

        //! @todo Hand status
        if(false /* hands are in combat? */)
        {
            setTargetState(LaraState::TurnFast);
            return;
        }
        
        if(m_yRotationSpeed > 728)
        {
            if(m_moveSlow)
                m_yRotationSpeed = 728;
            else
                setTargetState(LaraState::TurnFast);
        }

        if(m_zMovement != AxisMovement::Forward)
        {
            if(m_xMovement != AxisMovement::Right)
                setTargetState(LaraState::Stop);
            return;
        }

        if(m_moveSlow)
            setTargetState(LaraState::WalkForward);
        else
            setTargetState(LaraState::RunForward);
    }

    void onInput7TurnLeftSlow()
    {
        if(m_health <= 0)
        {
            setTargetState(LaraState::Stop);
            return;
        }

        m_yRotationSpeed -= 409;

        //! @todo Hand status
        if(false /* hands are in combat? */)
        {
            setTargetState(LaraState::TurnFast);
            return;
        }

        if(m_yRotationSpeed < -728)
        {
            if(m_moveSlow)
                m_yRotationSpeed = -728;
            else
                setTargetState(LaraState::TurnFast);
        }

        if(m_zMovement != AxisMovement::Forward)
        {
            if(m_xMovement != AxisMovement::Left)
                setTargetState(LaraState::Stop);
            return;
        }

        if(m_moveSlow)
            setTargetState(LaraState::WalkForward);
        else
            setTargetState(LaraState::RunForward);
    }

    void onInput16WalkBackward()
    {
        if(m_health <= 0)
        {
            setTargetState(LaraState::Stop);
            return;
        }

        if(m_zMovement == AxisMovement::Backward && m_moveSlow)
            setTargetState(LaraState::WalkBackward);
        else
            setTargetState(LaraState::Stop);

        if(m_xMovement == AxisMovement::Left)
            m_yRotationSpeed = std::max(-728, m_yRotationSpeed - 409);
        else if(m_xMovement == AxisMovement::Right)
            m_yRotationSpeed = std::min(728, m_yRotationSpeed + 409);
    }

    void onInput20TurnFast()
    {
        if(m_health <= 0)
        {
            setTargetState(LaraState::Stop);
            return;
        }

        if(m_yRotationSpeed >= 0)
        {
            m_yRotationSpeed = 1456;
            if(m_xMovement == AxisMovement::Right)
                return;
        }
        else
        {
            m_yRotationSpeed = -1456;
            if(m_xMovement == AxisMovement::Left)
                return;
        }
        setTargetState(LaraState::Stop);
    }

    void onInput25JumpBackward()
    {
        //! @todo Set local camera Y rotation to 24570 AU
        if(m_fallSpeed > 131)
            setTargetState(LaraState::FreeFall);
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
        m_rotation.X = degToAu(laraRot.X);
        m_rotation.Y = degToAu(laraRot.Y);
        m_rotation.Z = degToAu(laraRot.Z);
    }
    
    ~LaraStateHandler() = default;

    virtual void animateNode(irr::scene::ISceneNode* node, irr::u32 timeMs) override
    {
        BOOST_ASSERT(m_lara == node);
        
        const auto currentFrame = 30*timeMs/1000;
        if(currentFrame == m_lastActiveFrame)
            return;
        m_lastActiveFrame = currentFrame;

        {
            // "slowly" revert rotations to zero

            if(m_rotation.Z < -182)
                m_rotation.Z += 182;
            else if(m_rotation.Z > 182)
                m_rotation.Z -= 182;
            else
                m_rotation.Z = 0;

            if(m_yRotationSpeed < -364)
                m_yRotationSpeed += 364;
            else if(m_yRotationSpeed > 364)
                m_yRotationSpeed -= 364;
            else
                m_yRotationSpeed = 0;
            m_rotation.Y += m_yRotationSpeed;
        }


        static std::array<InputHandler, 55> inputHandlers{{
            &LaraStateHandler::onInput0WalkForward,
            &LaraStateHandler::onInput1RunForward,
            &LaraStateHandler::onInput2Stop,
            &LaraStateHandler::onInput3JumpForward,
            nullptr,
            &LaraStateHandler::onInput5RunBackward,
            &LaraStateHandler::onInput6TurnRightSlow,
            &LaraStateHandler::onInput7TurnLeftSlow,
            nullptr,
            nullptr,
            // 10
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            &LaraStateHandler::onInput16WalkBackward,
            nullptr,
            nullptr,
            nullptr,
            &LaraStateHandler::onInput20TurnFast,
            nullptr,nullptr,nullptr,nullptr,
            &LaraStateHandler::onInput25JumpBackward,
            nullptr,nullptr,nullptr,nullptr,
            // 30
            nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,
            // 40
            nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,
            // 50
            nullptr,nullptr,nullptr,nullptr,nullptr
        }};

        const auto currentState = m_dispatcher->getCurrentState();
        if(currentState >= inputHandlers.size())
        {
            BOOST_LOG_TRIVIAL(error) << "Unexpected state " << currentState;
            return;
        }

        if(!inputHandlers[currentState])
            BOOST_LOG_TRIVIAL(warning) << "No input handler for state " << currentState;
        else
            (this->*inputHandlers[currentState])();


        if(m_falling)
        {
            m_horizontalSpeed += m_dispatcher->getAccelleration();
            if(m_fallSpeed >= 128)
                m_fallSpeed += 1;
            else
                m_fallSpeed += 6;
            m_fallSpeed = 0; //!< @todo We're never falling (for now).
        }
        else
        {
            m_horizontalSpeed = m_dispatcher->calculateFloorSpeed();
        }

        auto movementAngle = m_rotation.Y;

        // behaviour handling depends on the current state *after* handling the input
        switch(static_cast<LaraState>(m_dispatcher->getCurrentState()))
        {
            case LaraState::WalkBackward:
            case LaraState::RunBack:
            case LaraState::JumpBack:
                movementAngle += 32768;
                break;
            default:
                break;
        }

        auto pos = m_lara->getPosition();
        pos.X += std::sin(auToRad(movementAngle)) * m_horizontalSpeed;
        pos.Z += std::cos(auToRad(movementAngle)) * m_horizontalSpeed;
        m_lara->setPosition(pos);

        {
            //! @todo This is horribly inefficient code, but it properly converts ZXY angles to XYZ angles.
            irr::core::quaternion q;
            q.makeIdentity();
            q *= irr::core::quaternion().fromAngleAxis(auToRad(m_rotation.Y), { 0,1,0 });
            q *= irr::core::quaternion().fromAngleAxis(auToRad(m_rotation.X), { 1,0,0 });
            q *= irr::core::quaternion().fromAngleAxis(auToRad(m_rotation.Z), { 0,0,-1 });

            irr::core::vector3df euler;
            q.toEuler(euler);
            m_lara->setRotation(euler * 180 / irr::core::PI);
        }

        m_lara->updateAbsolutePosition();
    }

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
    
private:
    void setTargetState(loader::LaraState st)
    {
        m_dispatcher->setTargetState(static_cast<uint16_t>(st));
    }

    LaraState getTargetState() const
    {
        return static_cast<LaraState>(m_dispatcher->getTargetState());
    }
    
    void playAnimation(loader::AnimationId anim)
    {
        m_dispatcher->playLocalAnimation(static_cast<uint16_t>(anim));
    }
};
