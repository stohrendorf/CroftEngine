#include "Base.h"
#include "AnimationController.h"
#include "Transform.h"


namespace gameplay
{
    AnimationController::AnimationController()
        : m_state(STOPPED)
    {
    }


    AnimationController::~AnimationController()
    {
    }


    void AnimationController::stopAllAnimations()
    {
        std::list<AnimationClip*>::iterator clipIter = m_runningClips.begin();
        while( clipIter != m_runningClips.end() )
        {
            AnimationClip* clip = *clipIter;
            BOOST_ASSERT(clip);
            clip->stop();
            ++clipIter;
        }
    }


    AnimationController::State AnimationController::getState() const
    {
        return m_state;
    }


    void AnimationController::initialize()
    {
        m_state = IDLE;
    }


    void AnimationController::finalize()
    {
        m_runningClips.clear();
        m_state = STOPPED;
    }


    void AnimationController::resume()
    {
        if( m_runningClips.empty() )
            m_state = IDLE;
        else
            m_state = RUNNING;
    }


    void AnimationController::pause()
    {
        m_state = PAUSED;
    }


    void AnimationController::schedule(AnimationClip* clip)
    {
        if( m_runningClips.empty() )
        {
            m_state = RUNNING;
        }

        BOOST_ASSERT(clip);
        m_runningClips.push_back(clip);
    }


    void AnimationController::unschedule(AnimationClip* clip)
    {
        std::list<AnimationClip*>::iterator clipItr = m_runningClips.begin();
        while( clipItr != m_runningClips.end() )
        {
            AnimationClip* rClip = (*clipItr);
            if( rClip == clip )
            {
                m_runningClips.erase(clipItr);
                break;
            }
            ++clipItr;
        }

        if( m_runningClips.empty() )
            m_state = IDLE;
    }


    void AnimationController::update(const std::chrono::microseconds& elapsedTime)
    {
        if( m_state != RUNNING )
            return;

        Transform::suspendTransformChanged();

        // Loop through running clips and call update() on them.
        std::list<AnimationClip*>::iterator clipIter = m_runningClips.begin();
        while( clipIter != m_runningClips.end() )
        {
            AnimationClip* clip = (*clipIter);
            BOOST_ASSERT(clip);
            if( clip->isClipStateBitSet(AnimationClip::CLIP_IS_RESTARTED_BIT) )
            { // If the CLIP_IS_RESTARTED_BIT is set, we should end the clip and
                // move it from where it is in the running clips list to the back.
                clip->onEnd();
                clip->setClipStateBit(AnimationClip::CLIP_IS_PLAYING_BIT);
                m_runningClips.push_back(clip);
                clipIter = m_runningClips.erase(clipIter);
            }
            else if( clip->update(elapsedTime) )
            {
                clipIter = m_runningClips.erase(clipIter);
            }
            else
            {
                ++clipIter;
            }
        }

        Transform::resumeTransformChanged();

        if( m_runningClips.empty() )
            m_state = IDLE;
    }
}
