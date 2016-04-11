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
    
    uint16_t getCurrentState() const;
    
    void setTargetState(uint16_t state) noexcept
    {
        BOOST_LOG_TRIVIAL(debug) << "Set target state=" << state << " (" << m_name << ") current=" << getCurrentState();
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
    void playGlobalAnimation(uint16_t anim);
    
    void playLocalAnimation(uint16_t anim)
    {
        playGlobalAnimation(m_model.animationIndex + anim);
    }
    
    uint16_t getCurrentAnimationId() const noexcept
    {
        return m_currentAnimationId;
    }

    const std::string& getName() const
    {
        return m_name;
    }

    /**
     * @brief Handles animation transitions
     * @param[in] useDefaultAnimationLoop Whether to apply the current animation's default follow-up animation
     * 
     * @note The target state is never changed.
     * 
     * @details
     * Checks the current animation's transitions for the target state and switches to the appropriate
     * animation if applicable.  If no such transition applies and @a useDefaultAnimationLoop is @c true,
     * the animation's default follow-up animation is played.
     */
    void handleTransitions(bool useDefaultAnimationLoop);

    int calculateFloorSpeed() const
    {
        BOOST_ASSERT(m_currentAnimationId < m_level->m_animations.size());
        const Animation& currentAnim = m_level->m_animations[m_currentAnimationId];
        return (currentAnim.speed + currentAnim.accelleration * getCurrentRelativeFrame()) / (1 << 16);
    }

    int getAccelleration() const
    {
        BOOST_ASSERT(m_currentAnimationId < m_level->m_animations.size());
        const Animation& currentAnim = m_level->m_animations[m_currentAnimationId];
        return currentAnim.accelleration / (1 << 16);
    }

private:
    /**
     * @brief Starts to play the current animation at the specified frame.
     * @param[in] localFrame The animation-local frame number.
     */
    void startAnimLoop(irr::u32 localFrame);
    irr::u32 getCurrentFrame() const;
    irr::u32 getCurrentRelativeFrame() const;
};

/**
 * @brief Handles looping and default transitions of animations.
 */
class AnimationEndStateHandler final : public irr::scene::IAnimationEndCallBack
{
private:
    std::shared_ptr<DefaultAnimDispatcher> m_dispatcher;

public:
    explicit AnimationEndStateHandler(std::shared_ptr<DefaultAnimDispatcher> dispatcher)
        : m_dispatcher(dispatcher)
    {
        BOOST_ASSERT(dispatcher != nullptr);
    }

    virtual void OnAnimationEnd(irr::scene::IAnimatedMeshSceneNode* /*node*/) override
    {
        m_dispatcher->handleTransitions(true);
    }
};

/**
 * @brief Handles transitions while playing animations.
 * 
 * @note Transitions are only checked once per frame.
 */
class IntermediateStateHandler final : public irr::scene::ISceneNodeAnimator
{
private:
    std::shared_ptr<DefaultAnimDispatcher> m_dispatcher;
    irr::u32 m_lastCheckedFrame = std::numeric_limits<irr::u32>::max();

public:
    explicit IntermediateStateHandler(std::shared_ptr<DefaultAnimDispatcher> dispatcher)
        : m_dispatcher(dispatcher)
    {
        BOOST_ASSERT(dispatcher != nullptr);
    }

    virtual void animateNode(irr::scene::ISceneNode* node, irr::u32 /*timeMs*/) override
    {
        BOOST_ASSERT(node->getType() == irr::scene::ESNT_ANIMATED_MESH);
        irr::scene::IAnimatedMeshSceneNode* animNode = static_cast<irr::scene::IAnimatedMeshSceneNode*>(node);
        
        BOOST_ASSERT(animNode->getFrameNr() >= 0);
        const auto currentFrame = static_cast<irr::u32>(animNode->getFrameNr());
        if(currentFrame == m_lastCheckedFrame)
            return;
        
        m_lastCheckedFrame = currentFrame;
        m_dispatcher->handleTransitions(false);
    }

    virtual ISceneNodeAnimator* createClone(irr::scene::ISceneNode* /*node*/, irr::scene::ISceneManager* /*newManager*/ = nullptr) override
    {
        BOOST_ASSERT(false);
        return nullptr;
    }
};

}
