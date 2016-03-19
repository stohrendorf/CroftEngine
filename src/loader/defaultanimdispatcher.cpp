#include "defaultanimdispatcher.h"

namespace loader
{

DefaultAnimDispatcher::DefaultAnimDispatcher(const Level* level, const AnimatedModel& model, irr::scene::IAnimatedMeshSceneNode* node)
    : m_level(level), m_model(model), m_currentAnimation(model.animationIndex)
{
    auto it = model.frameMapping.find(m_currentAnimation);
    if(it == model.frameMapping.end())
        return;
    
    node->setFrameLoop(it->second.firstFrame, it->second.lastFrame);
}

void DefaultAnimDispatcher::OnAnimationEnd(irr::scene::IAnimatedMeshSceneNode* node)
{
    BOOST_ASSERT(m_currentAnimation < m_level->m_animations.size());
    const Animation& currentAnim = m_level->m_animations[m_currentAnimation];
    const auto currentFrame = getCurrentFrame(node);
    for(size_t i = 0; i < currentAnim.transitionsCount; ++i)
    {
        auto tIdx = currentAnim.transitionsIndex + i;
        BOOST_ASSERT(tIdx < m_level->m_transitions.size());
        const Transitions& tr = m_level->m_transitions[tIdx];
        if(tr.stateId != getCurrentState())
            continue;
        
        for(auto j = tr.firstTransitionCase; j < tr.firstTransitionCase + tr.transitionCaseCount; ++j)
        {
            BOOST_ASSERT(j < m_level->m_transitionCases.size());
            const TransitionCase& trc = m_level->m_transitionCases[j];
            if(currentFrame >= trc.firstFrame && currentFrame <= trc.lastFrame)
            {
                m_currentAnimation = trc.targetAnimation;
                startAnimLoop(node, trc.targetFrame);
                return;
            }
        }
    }
    
    m_currentAnimation = currentAnim.nextAnimation;
    startAnimLoop(node, currentAnim.nextFrame);
}

void DefaultAnimDispatcher::startAnimLoop(irr::scene::IAnimatedMeshSceneNode* node, irr::u32 frame)
{
    auto it = m_model.frameMapping.find(m_currentAnimation);
    BOOST_ASSERT(it != m_model.frameMapping.end());
    
    node->setFrameLoop(it->second.firstFrame, it->second.lastFrame);
    node->setCurrentFrame(static_cast<irr::f32>(it->second.offset + frame));
}

irr::u32 DefaultAnimDispatcher::getCurrentFrame(irr::scene::IAnimatedMeshSceneNode* node) const
{
    auto it = m_model.frameMapping.find(m_currentAnimation);
    BOOST_ASSERT(it != m_model.frameMapping.end());
    
    return static_cast<irr::u32>(node->getFrameNr() - it->second.offset);
}

uint16_t DefaultAnimDispatcher::getCurrentState() const
{
    BOOST_ASSERT(m_currentAnimation < m_level->m_animations.size());
    const Animation& currentAnim = m_level->m_animations[m_currentAnimation];
    return currentAnim.state_id;
}

}
