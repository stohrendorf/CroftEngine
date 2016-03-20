#pragma once

#include "level.h"

namespace loader
{
class DefaultAnimDispatcher final : public irr::scene::IAnimationEndCallBack, public irr::scene::ISceneNodeAnimator
{
private:
    const Level* const m_level;
    const AnimatedModel& m_model;
    uint16_t m_currentAnimationId;
    uint16_t m_targetState = 0;
    const std::string m_name;
    irr::u32 m_lastActiveFrame = std::numeric_limits<irr::u32>::max();

public:
    DefaultAnimDispatcher(const Level* level, const AnimatedModel& model, irr::scene::IAnimatedMeshSceneNode* node, const std::string& name);

    virtual void OnAnimationEnd(irr::scene::IAnimatedMeshSceneNode* node) override;

    uint16_t getCurrentState() const;
    void setTargetState(uint16_t state) noexcept
    {
        BOOST_LOG_TRIVIAL(debug) << "Set target state=" << state << " (" << m_name << ") current=" << getCurrentState();
        m_targetState = state;
    }
    uint16_t getCurrentAnimationId() const noexcept
    {
        return m_currentAnimationId;
    }

    const std::string& getName() const
    {
        return m_name;
    }
    
    virtual void animateNode(irr::scene::ISceneNode* node, irr::u32 /*timeMs*/) override
    {
        BOOST_ASSERT(node->getType() == irr::scene::ESNT_ANIMATED_MESH);
        irr::scene::IAnimatedMeshSceneNode* animNode = static_cast<irr::scene::IAnimatedMeshSceneNode*>(node);
        
        BOOST_ASSERT(animNode->getFrameNr() >= 0);
        const auto currentFrame = static_cast<irr::u32>(animNode->getFrameNr());
        if(currentFrame == m_lastActiveFrame)
            return;
        
        m_lastActiveFrame = currentFrame;
        handleTransitions(animNode, false);
    }

    virtual ISceneNodeAnimator* createClone(irr::scene::ISceneNode* /*node*/, irr::scene::ISceneManager* /*newManager*/ = nullptr) override
    {
        BOOST_ASSERT(false);
        return nullptr;
    }
private:
    void startAnimLoop(irr::scene::IAnimatedMeshSceneNode* node, irr::u32 frame);
    irr::u32 getCurrentFrame(irr::scene::IAnimatedMeshSceneNode* node) const;
    void handleTransitions(irr::scene::IAnimatedMeshSceneNode* node, bool isLoopEnd);
};
}
