#include "defaultanimdispatcher.h"

namespace loader
{

DefaultAnimDispatcher::DefaultAnimDispatcher(const Level* level, const AnimatedModel& model, irr::scene::IAnimatedMeshSceneNode* node, const std::string& name)
    : m_level(level), m_model(model), m_currentAnimationId(model.animationIndex), m_name(name), m_node(node)
{
    BOOST_ASSERT(level != nullptr);
    BOOST_ASSERT(node != nullptr);
    
    auto it = model.frameMapping.find(m_currentAnimationId);
    if(it == model.frameMapping.end())
    {
        BOOST_LOG_TRIVIAL(error) << "No initial animation for " << name;
        return;
    }
    
    startAnimLoop(it->second.firstFrame);
    m_targetState = getCurrentState();
}

std::shared_ptr<DefaultAnimDispatcher> DefaultAnimDispatcher::create(irr::scene::IAnimatedMeshSceneNode* node, const Level* level, const AnimatedModel& model, const std::string& name)
{
    std::shared_ptr<DefaultAnimDispatcher> dispatcher{ new DefaultAnimDispatcher(level, model, node, name) };
    auto animEnd = new AnimationEndStateHandler(dispatcher);
    node->setAnimationEndCallback(animEnd);
    animEnd->drop();
    BOOST_ASSERT(animEnd->getReferenceCount() > 0);
    auto animator = new IntermediateStateHandler(dispatcher);
    node->addAnimator(animator);
    animator->drop();
    BOOST_ASSERT(animator->getReferenceCount() > 0);
    return dispatcher;
}

void DefaultAnimDispatcher::handleTransitions(bool useDefaultAnimationLoop)
{
    BOOST_ASSERT(m_currentAnimationId < m_level->m_animations.size());
    const Animation& currentAnim = m_level->m_animations[m_currentAnimationId];
    const auto currentFrame = getCurrentFrame();
    // const auto currentState = getCurrentState();
    
    // BOOST_LOG_TRIVIAL(debug) << "End of anim (" << node->getName() << "): animation=" << m_currentAnimationId << " currentState=" << currentState << " targetState=" << m_targetState;
    
    for(size_t i = 0; i < currentAnim.transitionsCount; ++i)
    {
        auto tIdx = currentAnim.transitionsIndex + i;
        BOOST_ASSERT(tIdx < m_level->m_transitions.size());
        const Transitions& tr = m_level->m_transitions[tIdx];
        if(tr.stateId != m_targetState)
            continue;
        
        for(auto j = tr.firstTransitionCase; j < tr.firstTransitionCase + tr.transitionCaseCount; ++j)
        {
            BOOST_ASSERT(j < m_level->m_transitionCases.size());
            const TransitionCase& trc = m_level->m_transitionCases[j];

            if(currentFrame >= trc.firstFrame && currentFrame <= trc.lastFrame)
            {
                //BOOST_LOG_TRIVIAL(debug) << "  - Starting target animation, targetAnimation=" << trc.targetAnimation << ", targetFrame=" << trc.targetFrame;
                m_currentAnimationId = trc.targetAnimation;
                startAnimLoop(trc.targetFrame);
                return;
            }
        }
    }
    
    if(useDefaultAnimationLoop)
    {
        m_currentAnimationId = currentAnim.nextAnimation;
        startAnimLoop(currentAnim.nextFrame);
        // m_targetState = getCurrentState();
        // BOOST_LOG_TRIVIAL(debug) << "  - Starting default animation, new targetState=" << m_targetState << ", nextAnimation=" << currentAnim.nextAnimation << ", nextFrame=" << currentAnim.nextFrame;
    }
}

void DefaultAnimDispatcher::startAnimLoop(irr::u32 localFrame)
{
    auto it = m_model.frameMapping.find(m_currentAnimationId);
    BOOST_ASSERT(it != m_model.frameMapping.end());
    it->second.apply(m_node, localFrame);
}

irr::u32 DefaultAnimDispatcher::getCurrentFrame() const
{
    auto it = m_model.frameMapping.find(m_currentAnimationId);
    BOOST_ASSERT(it != m_model.frameMapping.end());
    
    return static_cast<irr::u32>(m_node->getFrameNr() - it->second.offset + it->second.firstFrame);
}

uint16_t DefaultAnimDispatcher::getCurrentState() const
{
    BOOST_ASSERT(m_currentAnimationId < m_level->m_animations.size());
    const Animation& currentAnim = m_level->m_animations[m_currentAnimationId];
    return currentAnim.state_id;
}

void DefaultAnimDispatcher::playGlobalAnimation(uint16_t anim)
{
    auto it = m_model.frameMapping.find(anim);
    if(it == m_model.frameMapping.end())
    {
        BOOST_LOG_TRIVIAL(error) << "No animation " << anim << " for " << m_name;
        return;
    }
    
    m_currentAnimationId = anim;
    it->second.apply(m_node, it->second.firstFrame);
    m_targetState = getCurrentState();
    
    BOOST_LOG_TRIVIAL(debug) << "Playing animation " << anim << ", state " << m_targetState;
}

}
