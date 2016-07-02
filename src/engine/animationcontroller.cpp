#include "animationcontroller.h"

#include "laracontroller.h"

#include <set>

namespace engine
{
    AnimationController::AnimationController(gsl::not_null<const level::Level*> level, const loader::AnimatedModel& model, gsl::not_null<irr::scene::IAnimatedMeshSceneNode*> node, const std::string& name)
        : m_level(level), m_model(model), m_currentAnimationId(model.animationIndex), m_name(name), m_node(node)
    {
        auto it = model.frameMapping.find(m_currentAnimationId);
        if( it == model.frameMapping.end() )
        {
            BOOST_LOG_TRIVIAL(error) << "No initial animation for " << name;
            return;
        }

        startAnimLoop(it->second.firstFrame);
        m_targetState = getCurrentAnimState();
    }

    std::shared_ptr<AnimationController> AnimationController::create(irr::scene::IAnimatedMeshSceneNode* node, const level::Level* level, const loader::AnimatedModel& model, const std::string& name)
    {
        std::shared_ptr<AnimationController> dispatcher{new AnimationController(level, model, node, name)};
        auto animEnd = new AnimationEndControllerHelper(dispatcher);
        node->setAnimationEndCallback(animEnd);
        animEnd->drop();
        BOOST_ASSERT(animEnd->getReferenceCount() > 0);
        auto animator = new IntermediateTransitionControllerHelper(dispatcher);
        node->addAnimator(animator);
        animator->drop();
        BOOST_ASSERT(animator->getReferenceCount() > 0);
        return dispatcher;
    }

    void AnimationController::startAnimLoop(irr::u32 localFrame)
    {
        auto it = m_model.frameMapping.find(m_currentAnimationId);
        BOOST_ASSERT(it != m_model.frameMapping.end());
        it->second.apply(m_node, localFrame);
    }

    void AnimationController::advanceFrame()
    {
        BOOST_LOG_TRIVIAL(debug) << "Advance frame: current=" << m_node->getFrameNr() << ", end=" << m_node->getEndFrame();
        if(m_node->getFrameNr() >= m_node->getEndFrame())
        {
            handleAnimationEnd();
        }
        else
        {
            m_node->setCurrentFrame(m_node->getFrameNr() + 1);
        }

        handleTRTransitions();

        m_node->animateJoints();
    }


    irr::u32 AnimationController::getCurrentFrame() const
    {
        auto it = m_model.frameMapping.find(m_currentAnimationId);
        BOOST_ASSERT(it != m_model.frameMapping.end());

        return std::lround(m_node->getFrameNr() - it->second.offset + it->second.firstFrame);
    }

    irr::u32 AnimationController::getAnimEndFrame() const
    {
        auto it = m_model.frameMapping.find(m_currentAnimationId);
        BOOST_ASSERT(it != m_model.frameMapping.end());

        return it->second.lastFrame;
    }

    irr::core::aabbox3di AnimationController::getBoundingBox() const
    {
        auto it = m_model.frameMapping.find(m_currentAnimationId);
        BOOST_ASSERT(it != m_model.frameMapping.end());

        return it->second.getBoundingBox(getCurrentFrame());
    }

    irr::u32 AnimationController::getCurrentRelativeFrame() const
    {
        auto it = m_model.frameMapping.find(m_currentAnimationId);
        BOOST_ASSERT(it != m_model.frameMapping.end());

        return std::lround(m_node->getFrameNr() - it->second.offset);
    }

    uint16_t AnimationController::getCurrentAnimState() const
    {
        BOOST_ASSERT(m_currentAnimationId < m_level->m_animations.size());
        const loader::Animation& currentAnim = m_level->m_animations[m_currentAnimationId];
        return currentAnim.state_id;
    }

    void AnimationController::playGlobalAnimation(uint16_t anim, const boost::optional<irr::u32>& firstFrame)
    {
        auto it = m_model.frameMapping.find(anim);
        if( it == m_model.frameMapping.end() )
        {
            BOOST_LOG_TRIVIAL(error) << "No animation " << anim << " for " << m_name;
            return;
        }

        m_currentAnimationId = anim;
        it->second.apply(m_node, firstFrame.get_value_or(it->second.firstFrame));
        //m_targetState = getCurrentState();

        BOOST_LOG_TRIVIAL(debug) << "Playing animation " << anim << ", state " << getCurrentAnimState();
    }

    bool AnimationController::handleTRTransitions()
    {
        if( getCurrentAnimState() == m_targetState )
            return false;

        BOOST_ASSERT(m_currentAnimationId < m_level->m_animations.size());
        const loader::Animation& currentAnim = m_level->m_animations[m_currentAnimationId];
        const auto currentFrame = getCurrentFrame();

        for( size_t i = 0; i < currentAnim.transitionsCount; ++i )
        {
            auto tIdx = currentAnim.transitionsIndex + i;
            BOOST_ASSERT(tIdx < m_level->m_transitions.size());
            const loader::Transitions& tr = m_level->m_transitions[tIdx];
            if( tr.stateId != m_targetState )
                continue;

            for( auto j = tr.firstTransitionCase; j < tr.firstTransitionCase + tr.transitionCaseCount; ++j )
            {
                BOOST_ASSERT(j < m_level->m_transitionCases.size());
                const loader::TransitionCase& trc = m_level->m_transitionCases[j];

                if( currentFrame >= trc.firstFrame && currentFrame <= trc.lastFrame )
                {
                    m_currentAnimationId = trc.targetAnimation;
                    startAnimLoop(trc.targetFrame);
                    BOOST_LOG_TRIVIAL(debug) << m_name << " -- found transition to state " << m_targetState << ", new animation " << m_currentAnimationId << "/frame " << trc.targetFrame;
                    return true;
                }
            }
        }

        return false;
    }

    void AnimationController::handleAnimationEnd()
    {
        BOOST_ASSERT(m_currentAnimationId < m_level->m_animations.size());
        const loader::Animation& currentAnim = m_level->m_animations[m_currentAnimationId];

        m_currentAnimationId = currentAnim.nextAnimation;
        startAnimLoop(currentAnim.nextFrame);

        setTargetState(getCurrentAnimState());
    }
}
