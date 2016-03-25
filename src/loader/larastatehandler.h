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
    bool m_jump = false;
    bool m_moveSlow = false;
    
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
        switch(static_cast<LaraState>(m_dispatcher->getCurrentState()))
        {
            case LaraState::Stop:
                if(m_jump)
                {
                    setTargetState(LaraState::JumpPrepare);
                }
                else if(m_zMovement == AxisMovement::Positive)
                {
                    if(m_moveSlow)
                        setTargetState(LaraState::WalkForward);
                    else
                        setTargetState(LaraState::RunForward);
                }
                else if(m_zMovement == AxisMovement::Negative)
                {
                    if(m_moveSlow)
                        setTargetState(LaraState::WalkBackward);
                    else
                        setTargetState(LaraState::RunBack);
                }
                else if(m_xMovement == AxisMovement::Positive)
                {
                    if(m_moveSlow)
                        setTargetState(LaraState::StepRight);
                    else
                        setTargetState(LaraState::TurnRightSlow);
                }
                else if(m_xMovement == AxisMovement::Negative)
                {
                    if(m_moveSlow)
                        setTargetState(LaraState::StepLeft);
                    else
                        setTargetState(LaraState::TurnLeftSlow);
                }
                break;
            case LaraState::RunForward:
                if(m_zMovement == AxisMovement::Positive) // continue running
                {
                    if(m_moveSlow)
                        setTargetState(LaraState::WalkForward);
                    else if(m_jump)
                        setTargetState(LaraState::JumpForward);
                    else
                        setTargetState(LaraState::RunForward);
                }
                else
                {
                    setTargetState(LaraState::Stop);
                }
                break;
            case LaraState::RunBack:
                if(m_zMovement == AxisMovement::Negative) // continue running
                {
                    if(m_moveSlow)
                        setTargetState(LaraState::WalkBackward);
                    else if(m_jump)
                        setTargetState(LaraState::JumpBack);
                    else
                        setTargetState(LaraState::RunBack);
                }
                else
                {
                    setTargetState(LaraState::Stop);
                }
                break;
            case LaraState::JumpForward:
                if(m_moveSlow)
                {
                    setTargetState(LaraState::SwandiveBegin);
                }
                else if(m_zMovement == AxisMovement::Positive)
                {
                    setTargetState(LaraState::RunForward);
                }
                else
                {
                    setTargetState(LaraState::Stop);
                }
                break;
            case LaraState::JumpPrepare:
                if(m_zMovement == AxisMovement::Positive)
                {
                    setTargetState(LaraState::JumpForward);
                }
                else if(m_zMovement == AxisMovement::Negative)
                {
                    setTargetState(LaraState::JumpBack);
                }
                else if(m_xMovement == AxisMovement::Positive)
                {
                    setTargetState(LaraState::JumpRight);
                }
                else if(m_xMovement == AxisMovement::Negative)
                {
                    setTargetState(LaraState::JumpLeft);
                }
                break;
            case LaraState::JumpUp:
                setTargetState(LaraState::FreeFall);
                break;
            case LaraState::FreeFall:
                playAnimation(loader::AnimationId::LANDING_HARD);
                break;
            default:
                BOOST_LOG_TRIVIAL(debug) << "Unhandled state: " << m_dispatcher->getCurrentState();
        }
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
    
    void playAnimation(loader::AnimationId anim)
    {
        m_dispatcher->playLocalAnimation(static_cast<uint16_t>(anim));
    }
};
