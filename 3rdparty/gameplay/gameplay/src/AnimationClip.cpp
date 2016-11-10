#include "Base.h"
#include "AnimationClip.h"
#include "Game.h"
#include "MeshSkin.h"
#include "Joint.h"


namespace gameplay
{
    AnimationClip::AnimationClip(Game* game, MeshSkin* skin, const std::chrono::microseconds& startTime, const std::chrono::microseconds& endTime, const std::chrono::microseconds& step, const int16_t* poseData, size_t poseDataStride, const int32_t* boneTreeData)
        : _skin{skin}
        , _startTime{startTime}
        , _endTime{endTime}
        , _stateBits{0x00}
        , _timeStarted{std::chrono::microseconds::zero()}
        , _elapsedTime{std::chrono::microseconds::zero()}
        , _beginListeners{}
        , _endListeners{}
        , _listeners{}
        , _listenerItr{}
        , _game{game}
    {
        BOOST_ASSERT(_skin);
        BOOST_ASSERT(_game);
        BOOST_ASSERT(std::chrono::microseconds::zero() <= startTime && startTime <= endTime);

        for( auto i = startTime; i < endTime; i += step )
        {
            _poses.emplace(std::make_pair(i, Pose{_skin->getJointCount(), poseData, boneTreeData}));
            poseData += poseDataStride;
        }
    }


    AnimationClip::~AnimationClip()
    {
        _listeners.clear();
        _listenerItr.reset();
    }


    AnimationClip::ListenerEvent::ListenerEvent(Listener* listener, const std::chrono::microseconds& eventTime)
    {
        _listener = listener;
        _eventTime = eventTime;
    }


    AnimationClip::ListenerEvent::~ListenerEvent() = default;


    std::chrono::microseconds AnimationClip::getStartTime() const
    {
        return _startTime;
    }


    std::chrono::microseconds AnimationClip::getEndTime() const
    {
        return _endTime;
    }


    std::chrono::microseconds AnimationClip::getElapsedTime() const
    {
        return _elapsedTime;
    }


    std::chrono::microseconds AnimationClip::getDuration() const
    {
        return _endTime - _startTime;
    }


    bool AnimationClip::isPlaying() const
    {
        return (isClipStateBitSet(CLIP_IS_PLAYING_BIT) && !isClipStateBitSet(CLIP_IS_PAUSED_BIT));
    }


    void AnimationClip::play(const std::chrono::microseconds& time)
    {
        if( isClipStateBitSet(CLIP_IS_PLAYING_BIT) )
        {
            // If paused, reset the bit and return.
            if( isClipStateBitSet(CLIP_IS_PAUSED_BIT) )
            {
                resetClipStateBit(CLIP_IS_PAUSED_BIT);
                return;
            }

            // If the clip is set to be removed, reset the flag.
            if( isClipStateBitSet(CLIP_IS_MARKED_FOR_REMOVAL_BIT) )
                resetClipStateBit(CLIP_IS_MARKED_FOR_REMOVAL_BIT);

            // Set the state bit to restart.
            setClipStateBit(CLIP_IS_RESTARTED_BIT);
        }
        else
        {
            setClipStateBit(CLIP_IS_PLAYING_BIT);
        }

        const auto timeOffset = time - _startTime;
        _timeStarted = _game->getGameTime() - timeOffset;
    }


    void AnimationClip::stop()
    {
        if( isClipStateBitSet(CLIP_IS_PLAYING_BIT) )
        {
            // Reset the restarted and paused bits.
            resetClipStateBit(CLIP_IS_RESTARTED_BIT);
            resetClipStateBit(CLIP_IS_PAUSED_BIT);

            // Mark the clip to removed from the AnimationController.
            setClipStateBit(CLIP_IS_MARKED_FOR_REMOVAL_BIT);
        }
    }


    void AnimationClip::pause()
    {
        if( isClipStateBitSet(CLIP_IS_PLAYING_BIT) && !isClipStateBitSet(CLIP_IS_MARKED_FOR_REMOVAL_BIT) )
        {
            setClipStateBit(CLIP_IS_PAUSED_BIT);
        }
    }


    void AnimationClip::addListener(AnimationClip::Listener* listener, const std::chrono::microseconds& eventTime)
    {
        BOOST_ASSERT(listener);
        BOOST_ASSERT(eventTime < getDuration());

        auto listenerEvent = std::make_shared<ListenerEvent>(listener, eventTime);

        if( _listeners.empty() )
        {
            _listeners.push_front(listenerEvent);

            _listenerItr = std::make_unique<std::list<std::shared_ptr<ListenerEvent>>::iterator>();
            if( isClipStateBitSet(CLIP_IS_PLAYING_BIT) )
                *_listenerItr = _listeners.begin();
        }
        else
        {
            for( auto itr = _listeners.begin(); itr != _listeners.end(); ++itr )
            {
                BOOST_ASSERT(*itr);
                if( eventTime < (*itr)->_eventTime )
                {
                    itr = _listeners.insert(itr, listenerEvent);

                    // If playing, update the iterator if we need to.
                    // otherwise, it will just be set the next time the clip gets played.
                    if( isClipStateBitSet(CLIP_IS_PLAYING_BIT) )
                    {
                        std::chrono::microseconds currentTime = _elapsedTime % getDuration();
                        BOOST_ASSERT(**_listenerItr || *_listenerItr == _listeners.end());
                        if( currentTime < eventTime && (*_listenerItr == _listeners.end() || eventTime < (**_listenerItr)->_eventTime) )
                        {
                            *_listenerItr = itr;
                        }
                    }
                    return;
                }
            }
            _listeners.push_back(listenerEvent);
        }
    }


    void AnimationClip::removeListener(AnimationClip::Listener* listener, const std::chrono::microseconds& eventTime)
    {
        if( !_listeners.empty() )
        {
            BOOST_ASSERT(listener);
            auto iter = std::find_if(_listeners.begin(), _listeners.end(), [&](const std::shared_ptr<ListenerEvent>& lst)
                                     {
                                         return lst->_eventTime == eventTime && lst->_listener == listener;
                                     });
            if( iter != _listeners.end() )
            {
                if( isClipStateBitSet(CLIP_IS_PLAYING_BIT) )
                {
                    std::chrono::microseconds currentTime = _elapsedTime % getDuration();
                    BOOST_ASSERT(**_listenerItr || *_listenerItr == _listeners.end());

                    // We the listener has not been triggered yet, then check if it is next to be triggered, remove it, and update the iterator
                    if( currentTime < eventTime && *iter == **_listenerItr )
                    {
                        *_listenerItr = _listeners.erase(iter);
                        return;
                    }
                }
                _listeners.erase(iter);
            }
        }
    }


    void AnimationClip::addBeginListener(AnimationClip::Listener* listener)
    {
        BOOST_ASSERT(listener);
        _beginListeners.push_back(listener);
    }


    void AnimationClip::removeBeginListener(AnimationClip::Listener* listener)
    {
        if( !_beginListeners.empty() )
        {
            BOOST_ASSERT(listener);
            auto iter = std::find(_beginListeners.begin(), _beginListeners.end(), listener);
            if( iter != _beginListeners.end() )
            {
                _beginListeners.erase(iter);
            }
        }
    }


    void AnimationClip::addEndListener(AnimationClip::Listener* listener)
    {
        BOOST_ASSERT(listener);
        _endListeners.push_back(listener);
    }


    void AnimationClip::removeEndListener(AnimationClip::Listener* listener)
    {
        if( !_endListeners.empty() )
        {
            BOOST_ASSERT(listener);
            auto iter = std::find(_endListeners.begin(), _endListeners.end(), listener);
            if( iter != _endListeners.end() )
            {
                _endListeners.erase(iter);
            }
        }
    }


    bool AnimationClip::update(const std::chrono::microseconds& deltaTime)
    {
        BOOST_ASSERT(!_poses.empty());

        if( isClipStateBitSet(CLIP_IS_PAUSED_BIT) )
        {
            return false;
        }

        if( isClipStateBitSet(CLIP_IS_MARKED_FOR_REMOVAL_BIT) )
        {
            // If the marked for removal bit is set, it means stop() was called on the AnimationClip at some point
            // after the last update call. Reset the flag, and return true so the AnimationClip is removed from the
            // running clips on the AnimationController.
            onEnd();
            return true;
        }

        if( !isClipStateBitSet(CLIP_IS_STARTED_BIT) )
        {
            // Clip is just starting
            onBegin();
        }
        else
        {
            // Clip was already running
            _elapsedTime += deltaTime;
        }

        // Current time within a loop of the clip
        std::chrono::microseconds currentTime = std::chrono::microseconds::zero();

        if( getDuration() != std::chrono::microseconds::zero() )
        {
            // Animation is running normally.
            currentTime = _elapsedTime % getDuration();
        }

        // Notify any listeners of Animation events.
        if( !_listeners.empty() )
        {
            BOOST_ASSERT(_listenerItr);

            while( *_listenerItr != _listeners.end() && _elapsedTime >= (**_listenerItr)->_eventTime )
            {
                BOOST_ASSERT(_listenerItr);
                BOOST_ASSERT(**_listenerItr);
                BOOST_ASSERT((**_listenerItr)->_listener);

                (**_listenerItr)->_listener->animationEvent(this, Listener::TIME);
                ++(*_listenerItr);
            }
        }

        // Add back in start time, and divide by the total animation's duration to get the actual percentage complete
        BOOST_ASSERT(_skin);

        // Evaluate this clip.

        auto next = _poses.upper_bound(currentTime);
        if( next == _poses.begin() )
        {
            setPose(next->second);
        }
        else if( next == _poses.end() )
        {
            setPose(std::prev(next)->second);
        }
        else
        {
            auto prev = std::prev(next);
            const auto dist = next->first - prev->first;
            const auto lambdaDist = currentTime - prev->first;
            const auto lambda = static_cast<float>(lambdaDist.count()) / dist.count();
            BOOST_ASSERT(lambda >= 0 && lambda <= 1);

            const auto interpolated = prev->second.mix(next->second, lambda);
            setPose(interpolated);
        }

        // When ended. Probably should move to it's own method so we can call it when the clip is ended early.
        if( isClipStateBitSet(CLIP_IS_MARKED_FOR_REMOVAL_BIT) || !isClipStateBitSet(CLIP_IS_STARTED_BIT) )
        {
            onEnd();
            return true;
        }

        return false;
    }


    void AnimationClip::onBegin()
    {
        // Initialize animation to play.
        setClipStateBit(CLIP_IS_STARTED_BIT);

        _elapsedTime = _game->getGameTime() - _timeStarted;

        if( !_listeners.empty() )
            *_listenerItr = _listeners.begin();

        // Notify begin listeners if any.
        if( !_beginListeners.empty() )
        {
            auto listener = _beginListeners.begin();
            while( listener != _beginListeners.end() )
            {
                BOOST_ASSERT(*listener);
                (*listener)->animationEvent(this, Listener::BEGIN);
                ++listener;
            }
        }
    }


    void AnimationClip::onEnd()
    {
        resetClipStateBit(CLIP_ALL_BITS);

        // Notify end listeners if any.
        if( !_endListeners.empty() )
        {
            auto listener = _endListeners.begin();
            while( listener != _endListeners.end() )
            {
                BOOST_ASSERT(*listener);
                (*listener)->animationEvent(this, Listener::END);
                ++listener;
            }
        }
    }


    bool AnimationClip::isClipStateBitSet(unsigned char bit) const
    {
        return (_stateBits & bit) == bit;
    }


    void AnimationClip::setClipStateBit(unsigned char bit)
    {
        _stateBits |= bit;
    }


    void AnimationClip::resetClipStateBit(unsigned char bit)
    {
        _stateBits &= ~bit;
    }


    void AnimationClip::setPose(const Pose& pose)
    {
        BOOST_ASSERT(_skin);
        BOOST_ASSERT(_skin->getJointCount() == pose.bones.size());

        _bbox = pose.bbox;

        for( size_t i = 0; i < pose.bones.size(); ++i )
        {
            auto joint = _skin->getJoint(i);
            joint->setRotation(pose.bones[i].rotation);
            joint->setTranslation(pose.bones[i].translation);
        }
    }
}
