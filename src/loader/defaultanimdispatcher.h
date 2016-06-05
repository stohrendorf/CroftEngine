#pragma once

#include "level.h"

namespace loader
{
/**
 * @brief Handles state transitions and animation playback.
 */
class DefaultAnimDispatcher final
{
private:
    const Level* const m_level;
    const AnimatedModel& m_model;
    uint16_t m_currentAnimationId;
    uint16_t m_targetState = 0;
    const std::string m_name;
    irr::scene::IAnimatedMeshSceneNode* const m_node;

    DefaultAnimDispatcher(const Level* level, const AnimatedModel& model, irr::scene::IAnimatedMeshSceneNode* node, const std::string& name);
public:

    static std::shared_ptr<DefaultAnimDispatcher> create(irr::scene::IAnimatedMeshSceneNode* node, const Level* level, const AnimatedModel& model, const std::string& name);
    
    uint16_t getCurrentAnimState() const;

    void setTargetState(uint16_t state) noexcept
    {
        if(state == m_targetState)
            return;
        
        BOOST_LOG_TRIVIAL(debug) << m_name << " -- set target state=" << state << " (was " << m_targetState << "), current state=" << getCurrentAnimState();
        m_targetState = state;
    }

    uint16_t getTargetState() const noexcept
    {
        return m_targetState;
    }

    /**
     * @brief Play a specific animation.
     * @param anim Animation ID
     * 
     * @details
     * Plays the animation specified; if the animation does not exist, nothing happens;
     * if it exists, the target state is changed to the animation's state.
     */
    void playGlobalAnimation(uint16_t anim, const boost::optional<irr::u32>& firstFrame = boost::none);
    
    void playLocalAnimation(uint16_t anim, const boost::optional<irr::u32>& firstFrame = boost::none)
    {
        playGlobalAnimation(m_model.animationIndex + anim, firstFrame);
    }
    
    uint16_t getCurrentAnimationId() const noexcept
    {
        return m_currentAnimationId;
    }

    const std::string& getName() const
    {
        return m_name;
    }

    bool handleTRTransitions();
    void handleAnimationEnd();

    float calculateFloorSpeed() const
    {
        BOOST_ASSERT(m_currentAnimationId < m_level->m_animations.size());
        const Animation& currentAnim = m_level->m_animations[m_currentAnimationId];
        return float(currentAnim.speed + currentAnim.accelleration * getCurrentRelativeFrame()) / (1 << 16);
    }

    int getAccelleration() const
    {
        BOOST_ASSERT(m_currentAnimationId < m_level->m_animations.size());
        const Animation& currentAnim = m_level->m_animations[m_currentAnimationId];
        return currentAnim.accelleration / (1 << 16);
    }

    irr::u32 getCurrentFrame() const;
    irr::u32 getAnimEndFrame() const;

private:
    /**
     * @brief Starts to play the current animation at the specified frame.
     * @param[in] localFrame The animation-local frame number.
     */
    void startAnimLoop(irr::u32 localFrame);
    irr::u32 getCurrentRelativeFrame() const;
};

/**
 * @brief Handles looping and default transitions of animations.
 */
class AnimationEndStateHandler final : public irr::scene::IAnimationEndCallBack
{
    std::shared_ptr<DefaultAnimDispatcher> m_dispatcher;

public:
    explicit AnimationEndStateHandler(std::shared_ptr<DefaultAnimDispatcher> dispatcher)
        : m_dispatcher(dispatcher)
    {
        BOOST_ASSERT(dispatcher != nullptr);
    }

    void OnAnimationEnd(irr::scene::IAnimatedMeshSceneNode* /*node*/) override
    {
        m_dispatcher->handleAnimationEnd();
    }
};

/**
 * @brief Handles transitions while playing animations.
 * 
 * @note Transitions are only checked once per frame.
 */
class IntermediateStateHandler final : public irr::scene::ISceneNodeAnimator
{
    std::shared_ptr<DefaultAnimDispatcher> m_dispatcher;
    irr::u32 m_lastCheckedFrame = std::numeric_limits<irr::u32>::max();

public:
    explicit IntermediateStateHandler(std::shared_ptr<DefaultAnimDispatcher> dispatcher)
        : m_dispatcher(dispatcher)
    {
        BOOST_ASSERT(dispatcher != nullptr);
    }

    void animateNode(irr::scene::ISceneNode* node, irr::u32 /*timeMs*/) override
    {
        BOOST_ASSERT(node->getType() == irr::scene::ESNT_ANIMATED_MESH);
        irr::scene::IAnimatedMeshSceneNode* animNode = static_cast<irr::scene::IAnimatedMeshSceneNode*>(node);
        
        BOOST_ASSERT(animNode->getFrameNr() >= 0);
        const auto currentFrame = static_cast<irr::u32>(animNode->getFrameNr());
        if(currentFrame == m_lastCheckedFrame)
            return;
        
        m_lastCheckedFrame = currentFrame;
    }

    ISceneNodeAnimator* createClone(irr::scene::ISceneNode* /*node*/, irr::scene::ISceneManager* /*newManager*/ = nullptr) override
    {
        BOOST_ASSERT(false);
        return nullptr;
    }
};

}
