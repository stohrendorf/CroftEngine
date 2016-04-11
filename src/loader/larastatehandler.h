#pragma once

#include "animationids.h"
#include "larastate.h"

#include <irrlicht.h>

#include <cstdint>

enum class AxisMovement
{
    Positive,
    Null,
    Negative
};

constexpr irr::f32 auToDeg(irr::s32 au)
{
    return au / 65536.0f * 360;
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

    // Lara's vars
    int m_health = 1000;
    //! @brief Additional rotation in AU per TR Engine Frame
    int m_yRotationSpeed = 0;
    bool m_falling = false;
    int m_fallSpeed = 0;
    int m_horizontalSpeed = 0;

    using InputHandler = void (LaraStateHandler::*)(irr::scene::IAnimatedMeshSceneNode*);
    
    void onInput0WalkForward(irr::scene::IAnimatedMeshSceneNode* node)
    {
        if(m_health <= 0)
        {
            setTargetState(LaraState::Stop);
            return;
        }

        if(m_xMovement == AxisMovement::Negative)
            m_yRotationSpeed = std::max(-728, m_yRotationSpeed - 409);
        else if(m_xMovement == AxisMovement::Positive)
            m_yRotationSpeed = std::min(728, m_yRotationSpeed + 409);
        if(m_zMovement == AxisMovement::Positive)
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

    void onInput1RunForward(irr::scene::IAnimatedMeshSceneNode* node)
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
        if(m_xMovement == AxisMovement::Negative)
        {
            m_yRotationSpeed = std::max(-1456, m_yRotationSpeed - 409);
            auto rot = node->getRotation();
            rot.Z = std::max(auToDeg(-2002), rot.Z - auToDeg(273));
            node->setRotation(rot);
        }
        else if(m_xMovement == AxisMovement::Positive)
        {
            m_yRotationSpeed = std::min(1456, m_yRotationSpeed + 409);
            auto rot = node->getRotation();
            rot.Z = std::min(auToDeg(2002), rot.Z + auToDeg(273));
            node->setRotation(rot);
        }
        if(m_jump && !m_falling)
        {
            setTargetState(LaraState::FreeFall);
            return;
        }
        if(m_zMovement != AxisMovement::Positive)
        {
            setTargetState(LaraState::Stop);
            return;
        }
        if(m_moveSlow)
            setTargetState(LaraState::WalkForward);
        else
            setTargetState(LaraState::RunForward);
    }

    void onInput2Stop(irr::scene::IAnimatedMeshSceneNode* node)
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
        if(m_stepMovement == AxisMovement::Negative)
        {
            setTargetState(LaraState::StepLeft);
        }
        else if(m_stepMovement == AxisMovement::Positive)
        {
            setTargetState(LaraState::StepRight);
        }
        if(m_xMovement == AxisMovement::Negative)
        {
            setTargetState(LaraState::TurnLeftSlow);
        }
        else if(m_xMovement == AxisMovement::Positive)
        {
            setTargetState(LaraState::TurnRightSlow);
        }
        if(m_jump)
        {
            setTargetState(LaraState::JumpPrepare);
        }
        if(m_zMovement == AxisMovement::Positive)
        {
            if(m_moveSlow)
                onInput0WalkForward(node);
            else
                onInput1RunForward(node);
        }
        else if(m_zMovement == AxisMovement::Negative)
        {
            if(m_moveSlow)
                onInput16WalkBackward(node);
            else
                setTargetState(LaraState::RunBack);
        }
    }

    void onInput3JumpForward(irr::scene::IAnimatedMeshSceneNode* node)
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
                setTargetState(LaraState::Reach);
            if(m_fallSpeed > 131)
                setTargetState(LaraState::FreeFall);
        }

        if(m_xMovement == AxisMovement::Negative)
        {
            m_yRotationSpeed = std::max(-546, m_yRotationSpeed - 409);
        }
        else if(m_xMovement == AxisMovement::Positive)
        {
            m_yRotationSpeed = std::min(546, m_yRotationSpeed + 409);
        }
    }

    void onInput6TurnRightSlow(irr::scene::IAnimatedMeshSceneNode* node)
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

        if(m_zMovement != AxisMovement::Positive)
        {
            if(m_xMovement != AxisMovement::Positive)
                setTargetState(LaraState::Stop);
            return;
        }

        if(m_moveSlow)
            setTargetState(LaraState::WalkForward);
        else
            setTargetState(LaraState::RunForward);
    }

    void onInput7TurnLeftSlow(irr::scene::IAnimatedMeshSceneNode* node)
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

        if(m_zMovement != AxisMovement::Positive)
        {
            if(m_xMovement != AxisMovement::Negative)
                setTargetState(LaraState::Stop);
            return;
        }

        if(m_moveSlow)
            setTargetState(LaraState::WalkForward);
        else
            setTargetState(LaraState::RunForward);
    }

    void onInput16WalkBackward(irr::scene::IAnimatedMeshSceneNode* node)
    {
        if(m_health <= 0)
        {
            setTargetState(LaraState::Stop);
            return;
        }

        if(m_zMovement == AxisMovement::Negative && m_moveSlow)
            setTargetState(LaraState::WalkBackward);
        else
            setTargetState(LaraState::Stop);

        if(m_xMovement == AxisMovement::Negative)
            m_yRotationSpeed = std::max(-728, m_yRotationSpeed - 409);
        else if(m_xMovement == AxisMovement::Positive)
            m_yRotationSpeed = std::min(728, m_yRotationSpeed + 409);
    }

    void onInput20TurnFast(irr::scene::IAnimatedMeshSceneNode* node)
    {
        if(m_health <= 0)
        {
            setTargetState(LaraState::Stop);
            return;
        }

        if(m_yRotationSpeed >= 0)
        {
            m_yRotationSpeed = 1456;
            if(m_xMovement == AxisMovement::Positive)
                return;
        }
        else
        {
            m_yRotationSpeed = -1456;
            if(m_xMovement == AxisMovement::Negative)
                return;
        }
        setTargetState(LaraState::Stop);
    }

public:
    LaraStateHandler(const loader::Level* level, const std::shared_ptr<loader::DefaultAnimDispatcher>& dispatcher, const std::string& name)
        : m_level(level), m_dispatcher(dispatcher), m_name(name)
    {
        BOOST_ASSERT(level != nullptr);
        BOOST_ASSERT(dispatcher != nullptr);
        playAnimation(loader::AnimationId::STAY_IDLE);
    }
    
    ~LaraStateHandler() = default;

    virtual void animateNode(irr::scene::ISceneNode* node, irr::u32 /*timeMs*/) override
    {
        BOOST_ASSERT(node->getType() == irr::scene::ESNT_ANIMATED_MESH);
        irr::scene::IAnimatedMeshSceneNode* animNode = static_cast<irr::scene::IAnimatedMeshSceneNode*>(node);
        
        BOOST_ASSERT(animNode->getFrameNr() >= 0);
        const auto currentFrame = static_cast<irr::u32>(animNode->getFrameNr());
        if(currentFrame == m_lastActiveFrame)
            return;
        
        m_lastActiveFrame = currentFrame;

        {
            // "slowly" revert rotations to zero

            auto rot = animNode->getRotation();
            
            if(rot.Z < auToDeg(-182))
                rot.Z += auToDeg(182);
            else if(rot.Z > auToDeg(182))
                rot.Z -= auToDeg(182);
            else
                rot.Z = 0;

            if(m_yRotationSpeed < -364)
                m_yRotationSpeed += 364;
            else if(m_yRotationSpeed > 364)
                m_yRotationSpeed -= 364;
            else
                m_yRotationSpeed = 0;
            rot.Y += auToDeg(m_yRotationSpeed);

            animNode->setRotation(rot);
        }

        switch(static_cast<LaraState>(m_dispatcher->getCurrentState()))
        {
            case LaraState::WalkForward:
                onInput0WalkForward(animNode);
                break;
            case LaraState::RunForward:
                onInput1RunForward(animNode);
                break;
            case LaraState::Stop:
                onInput2Stop(animNode);
                break;
            case LaraState::JumpForward:
                onInput3JumpForward(animNode);
                break;
            case LaraState::TurnRightSlow:
                onInput6TurnRightSlow(animNode);
                break;
            case LaraState::TurnLeftSlow:
                onInput7TurnLeftSlow(animNode);
                break;
            case LaraState::WalkBackward:
                onInput16WalkBackward(animNode);
                break;
            case LaraState::TurnFast:
                onInput20TurnFast(animNode);
                break;
            default:
                BOOST_LOG_TRIVIAL(debug) << "Unhandled state: " << m_dispatcher->getCurrentState();
        }

        if(m_falling)
        {
            m_horizontalSpeed += m_dispatcher->getAccelleration();
            if(m_fallSpeed >= 128)
                m_fallSpeed += 1;
            else
                m_fallSpeed += 6;
        }
        else
        {
            m_horizontalSpeed = m_dispatcher->calculateFloorSpeed();
        }

        auto pos = animNode->getPosition();
        pos.X += std::sin(irr::core::degToRad(animNode->getRotation().Y)) * m_horizontalSpeed;
        pos.Z += std::cos(irr::core::degToRad(animNode->getRotation().Y)) * m_horizontalSpeed;
        animNode->setPosition(pos);
        animNode->updateAbsolutePosition();
    }

    virtual ISceneNodeAnimator* createClone(irr::scene::ISceneNode* /*node*/, irr::scene::ISceneManager* /*newManager*/ = nullptr) override
    {
        BOOST_ASSERT(false);
        return nullptr;
    }
    
    void setXAxisMovement(bool left, bool right)
    {
        if(left < right)
            m_xMovement = AxisMovement::Positive;
        else if(left > right)
            m_xMovement = AxisMovement::Negative;
        else
            m_xMovement = AxisMovement::Null;
    }
    
    void setZAxisMovement(bool back, bool forward)
    {
        if(back < forward)
            m_zMovement = AxisMovement::Positive;
        else if(back > forward)
            m_zMovement = AxisMovement::Negative;
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
