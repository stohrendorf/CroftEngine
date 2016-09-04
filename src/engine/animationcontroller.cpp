#include "animationcontroller.h"

#include "laracontroller.h"
#include <chrono>


namespace engine
{
    MeshAnimationController::MeshAnimationController(gsl::not_null<const level::Level*> level, const loader::AnimatedModel& model, gsl::not_null<gameplay::MeshSkin*> node, const std::string& name)
        : AnimationController(level, name)
        , m_model(model)
        , m_currentAnimationId(model.animationIndex)
        , m_node(node)
    {
        auto it = model.animationClips.find(m_currentAnimationId);
        if( it == model.animationClips.end() )
        {
            BOOST_LOG_TRIVIAL(error) << "No initial animation for " << name;
            return;
        }

        startAnimLoop(it->second->getStartTime());
        m_targetState = getCurrentAnimState();
    }


    void MeshAnimationController::startAnimLoop(const core::Frame& localFrame)
    {
        startAnimLoop(core::toTime(localFrame));
    }


    void MeshAnimationController::startAnimLoop(const std::chrono::microseconds& time)
    {
        auto it = m_model.animationClips.find(m_currentAnimationId);
        BOOST_ASSERT(it != m_model.animationClips.end());
        it->second->play(time);
    }


    void MeshAnimationController::advanceFrame()
    {
        auto it = m_model.animationClips.find(m_currentAnimationId);
        BOOST_ASSERT(it != m_model.animationClips.end());
        const auto& clip = it->second;

        BOOST_LOG_TRIVIAL(debug) << "Advance frame: current=" << clip->getElapsedTime().count() << ", end=" << clip->getEndTime().count();
        if( clip->getElapsedTime() + std::chrono::seconds(1) / core::FrameRate >= clip->getEndTime() )
        {
            handleAnimationEnd();
        }
        else
        {
            clip->setElapsedTime(clip->getElapsedTime() + std::chrono::seconds(1) / core::FrameRate);
        }

        handleTRTransitions();
    }


    core::Frame MeshAnimationController::getCurrentFrame() const
    {
        auto it = m_model.animationClips.find(m_currentAnimationId);
        BOOST_ASSERT(it != m_model.animationClips.end());

        return core::toFrame(it->second->getElapsedTime());
    }


    core::Frame MeshAnimationController::getAnimEndFrame() const
    {
        auto it = m_model.animationClips.find(m_currentAnimationId);
        BOOST_ASSERT(it != m_model.animationClips.end());

        return core::toFrame(it->second->getEndTime());
    }


    gameplay::BoundingBox MeshAnimationController::getBoundingBox() const
    {
        auto it = m_model.animationClips.find(m_currentAnimationId);
        BOOST_ASSERT(it != m_model.animationClips.end());

        return it->second->getBoundingBox();
    }


    core::Frame MeshAnimationController::getCurrentRelativeFrame() const
    {
        auto it = m_model.animationClips.find(m_currentAnimationId);
        BOOST_ASSERT(it != m_model.animationClips.end());

        return core::toFrame(it->second->getElapsedTime() - it->second->getStartTime());
    }


    uint16_t MeshAnimationController::getCurrentAnimState() const
    {
        BOOST_ASSERT(m_currentAnimationId < getLevel()->m_animations.size());
        const loader::Animation& currentAnim = getLevel()->m_animations[m_currentAnimationId];
        return currentAnim.state_id;
    }


    void MeshAnimationController::playGlobalAnimation(uint16_t anim, const boost::optional<core::Frame>& firstFrame)
    {
        auto it = m_model.animationClips.find(anim);
        if( it == m_model.animationClips.end() )
        {
            BOOST_LOG_TRIVIAL(error) << "No animation " << anim << " for " << getName();
            return;
        }

        m_currentAnimationId = anim;
        if(firstFrame.is_initialized())
            it->second->play(core::toTime(*firstFrame));
        else
            it->second->play();
        //m_targetState = getCurrentState();

        BOOST_LOG_TRIVIAL(debug) << "Playing animation " << anim << ", state " << getCurrentAnimState();
    }


    bool MeshAnimationController::handleTRTransitions()
    {
        if( getCurrentAnimState() == m_targetState )
            return false;

        BOOST_ASSERT(m_currentAnimationId < getLevel()->m_animations.size());
        const loader::Animation& currentAnim = getLevel()->m_animations[m_currentAnimationId];
        const auto currentFrame = getCurrentFrame();

        for( size_t i = 0; i < currentAnim.transitionsCount; ++i )
        {
            auto tIdx = currentAnim.transitionsIndex + i;
            BOOST_ASSERT(tIdx < getLevel()->m_transitions.size());
            const loader::Transitions& tr = getLevel()->m_transitions[tIdx];
            if( tr.stateId != m_targetState )
                continue;

            for( auto j = tr.firstTransitionCase; j < tr.firstTransitionCase + tr.transitionCaseCount; ++j )
            {
                BOOST_ASSERT(j < getLevel()->m_transitionCases.size());
                const loader::TransitionCase& trc = getLevel()->m_transitionCases[j];

                if( currentFrame >= core::Frame(trc.firstFrame) && currentFrame <= core::Frame(trc.lastFrame) )
                {
                    m_currentAnimationId = trc.targetAnimation;
                    startAnimLoop(core::Frame(trc.targetFrame));
                    BOOST_LOG_TRIVIAL(debug) << getName() << " -- found transition to state " << m_targetState << ", new animation " << m_currentAnimationId << "/frame " << trc.targetFrame;
                    return true;
                }
            }
        }

        return false;
    }


    void MeshAnimationController::handleAnimationEnd()
    {
        BOOST_ASSERT(m_currentAnimationId < getLevel()->m_animations.size());
        const loader::Animation& currentAnim = getLevel()->m_animations[m_currentAnimationId];

        m_currentAnimationId = currentAnim.nextAnimation;
        startAnimLoop(core::Frame(currentAnim.nextFrame));

        setTargetState(getCurrentAnimState());
    }
}
