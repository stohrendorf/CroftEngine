#pragma once

#include "level/level.h"
#include "loader/animation.h"

#include <boost/optional.hpp>

namespace engine
{
/**
 * @brief Handles state transitions and animation playback.
 */
class AnimationController final
{
private:
    const level::Level* const m_level;
    const loader::AnimatedModel& m_model;
    uint16_t m_currentAnimationId;
    uint16_t m_targetState = 0;
    const std::string m_name;
    irr::scene::IAnimatedMeshSceneNode* const m_node;

    AnimationController(gsl::not_null<const level::Level*> level, const loader::AnimatedModel& model, gsl::not_null<irr::scene::IAnimatedMeshSceneNode*> node, const std::string& name);
public:

    static std::shared_ptr<AnimationController> create(irr::scene::IAnimatedMeshSceneNode* node, const level::Level* level, const loader::AnimatedModel& model, const std::string& name);

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
        const loader::Animation& currentAnim = m_level->m_animations[m_currentAnimationId];
        return float(currentAnim.speed + currentAnim.accelleration * getCurrentRelativeFrame()) / (1 << 16);
    }

    int getAccelleration() const
    {
        BOOST_ASSERT(m_currentAnimationId < m_level->m_animations.size());
        const loader::Animation& currentAnim = m_level->m_animations[m_currentAnimationId];
        return currentAnim.accelleration / (1 << 16);
    }

    void advanceFrame();
    irr::u32 getCurrentFrame() const;
    irr::u32 getAnimEndFrame() const;
    irr::core::aabbox3di getBoundingBox() const;

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
class AnimationEndControllerHelper final : public irr::scene::IAnimationEndCallBack
{
    std::shared_ptr<AnimationController> m_animationController;

public:
    explicit AnimationEndControllerHelper(const std::shared_ptr<AnimationController>& animationController)
        : m_animationController(animationController)
    {
        Expects(animationController != nullptr);
    }

    void OnAnimationEnd(irr::scene::IAnimatedMeshSceneNode* /*node*/) override
    {
        m_animationController->handleAnimationEnd();
    }
};

/**
 * @brief Handles transitions while playing animations.
 *
 * @note Transitions are only checked once per frame.
 */
class IntermediateTransitionControllerHelper final : public irr::scene::ISceneNodeAnimator
{
    std::shared_ptr<AnimationController> m_animationController;
    irr::u32 m_lastCheckedFrame = std::numeric_limits<irr::u32>::max();

public:
    explicit IntermediateTransitionControllerHelper(const std::shared_ptr<AnimationController>& animationController)
        : m_animationController(animationController)
    {
        Expects(animationController != nullptr);
    }

    void animateNode(irr::scene::ISceneNode* node, irr::u32 /*timeMs*/) override
    {
        BOOST_ASSERT(node->getType() == irr::scene::ESNT_ANIMATED_MESH);
        irr::scene::IAnimatedMeshSceneNode* animNode = static_cast<irr::scene::IAnimatedMeshSceneNode*>(node);

        BOOST_ASSERT(animNode->getFrameNr() >= 0);
        const auto currentFrame = static_cast<irr::u32>(animNode->getFrameNr());
        if(currentFrame == m_lastCheckedFrame)
            return;

        m_animationController->handleTRTransitions();

        m_lastCheckedFrame = currentFrame;
    }

    ISceneNodeAnimator* createClone(irr::scene::ISceneNode* /*node*/, irr::scene::ISceneManager* /*newManager*/ = nullptr) override
    {
        BOOST_ASSERT(false);
        return nullptr;
    }
};

}
