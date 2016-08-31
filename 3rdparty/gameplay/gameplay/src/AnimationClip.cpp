#include "Base.h"
#include "AnimationClip.h"
#include "Animation.h"
#include "Game.h"
#include "Quaternion.h"


namespace gameplay
{
    AnimationClip::AnimationClip(const std::string& id, Animation* animation, const std::chrono::microseconds& startTime, const std::chrono::microseconds& endTime)
        : _id(id)
        , _animation{animation}
        , _startTime{startTime}
        , _endTime{endTime}
        , _stateBits{0x00}
        , _timeStarted{std::chrono::microseconds::zero()}
        , _elapsedTime{std::chrono::microseconds::zero()}
        , _beginListeners{}
        , _endListeners{}
        , _listeners{}
        , _listenerItr{}
    {
        GP_ASSERT(_animation);
        GP_ASSERT(std::chrono::microseconds::zero() <= startTime && startTime <= _animation->_duration && std::chrono::microseconds::zero() <= endTime && endTime <= _animation->_duration);
    }


    AnimationClip::~AnimationClip()
    {
        if( !_listeners.empty() )
        {
            *_listenerItr = _listeners.begin();
            while( *_listenerItr != _listeners.end() )
            {
                ListenerEvent* lEvt = **_listenerItr;
                SAFE_DELETE(lEvt);
                ++(*_listenerItr);
            }
            _listeners.clear();
        }
        SAFE_DELETE(_listenerItr);
    }


    AnimationClip::ListenerEvent::ListenerEvent(Listener* listener, const std::chrono::microseconds& eventTime)
    {
        _listener = listener;
        _eventTime = eventTime;
    }


    AnimationClip::ListenerEvent::~ListenerEvent()
    {
    }


    const std::string& AnimationClip::getId() const
    {
        return _id;
    }


    Animation* AnimationClip::getAnimation() const
    {
        return _animation;
    }


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


    void AnimationClip::play(const std::chrono::microseconds& timeOffset)
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
            GP_ASSERT(_animation);
            GP_ASSERT(_animation->_controller);
            _animation->_controller->schedule(this);
        }

        _timeStarted = Game::getGameTime() - timeOffset;
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
        GP_ASSERT(listener);
        GP_ASSERT(eventTime < getDuration());

        ListenerEvent* listenerEvent = new ListenerEvent(listener, eventTime);

        if( _listeners.empty() )
        {
            _listeners.push_front(listenerEvent);

            _listenerItr = new std::list<ListenerEvent*>::iterator;
            if( isClipStateBitSet(CLIP_IS_PLAYING_BIT) )
                *_listenerItr = _listeners.begin();
        }
        else
        {
            for( std::list<ListenerEvent*>::iterator itr = _listeners.begin(); itr != _listeners.end(); ++itr )
            {
                GP_ASSERT(*itr);
                if( eventTime < (*itr)->_eventTime )
                {
                    itr = _listeners.insert(itr, listenerEvent);

                    // If playing, update the iterator if we need to.
                    // otherwise, it will just be set the next time the clip gets played.
                    if( isClipStateBitSet(CLIP_IS_PLAYING_BIT) )
                    {
                        std::chrono::microseconds currentTime = _elapsedTime % getDuration();
                        GP_ASSERT(**_listenerItr || *_listenerItr == _listeners.end());
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
            GP_ASSERT(listener);
            std::list<ListenerEvent*>::iterator iter = std::find_if(_listeners.begin(), _listeners.end(), [&](ListenerEvent* lst)
                                                                    {
                                                                        return lst->_eventTime == eventTime && lst->_listener == listener;
                                                                    });
            if( iter != _listeners.end() )
            {
                if( isClipStateBitSet(CLIP_IS_PLAYING_BIT) )
                {
                    std::chrono::microseconds currentTime = _elapsedTime % getDuration();
                    GP_ASSERT(**_listenerItr || *_listenerItr == _listeners.end());

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
        GP_ASSERT(listener);
        _beginListeners.push_back(listener);
    }


    void AnimationClip::removeBeginListener(AnimationClip::Listener* listener)
    {
        if( !_beginListeners.empty() )
        {
            GP_ASSERT(listener);
            auto iter = std::find(_beginListeners.begin(), _beginListeners.end(), listener);
            if( iter != _beginListeners.end() )
            {
                _beginListeners.erase(iter);
            }
        }
    }


    void AnimationClip::addEndListener(AnimationClip::Listener* listener)
    {
        GP_ASSERT(listener);
        _endListeners.push_back(listener);
    }


    void AnimationClip::removeEndListener(AnimationClip::Listener* listener)
    {
        if( !_endListeners.empty() )
        {
            GP_ASSERT(listener);
            auto iter = std::find(_endListeners.begin(), _endListeners.end(), listener);
            if( iter != _endListeners.end() )
            {
                _endListeners.erase(iter);
            }
        }
    }


    bool AnimationClip::update(const std::chrono::microseconds& elapsedTime)
    {
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
            _elapsedTime += elapsedTime;
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
            GP_ASSERT(_listenerItr);

            while( *_listenerItr != _listeners.end() && _elapsedTime >= (**_listenerItr)->_eventTime )
            {
                GP_ASSERT(_listenerItr);
                GP_ASSERT(**_listenerItr);
                GP_ASSERT((**_listenerItr)->_listener);

                (**_listenerItr)->_listener->animationEvent(this, Listener::TIME);
                ++(*_listenerItr);
            }
        }

        // Add back in start time, and divide by the total animation's duration to get the actual percentage complete
        GP_ASSERT(_animation);

        // Compute percentage complete for the current loop (prevent a divide by zero if _duration==0).
        // Note that we don't use (currentTime/(_duration+_loopBlendTime)). That's because we want a
        // % value that is outside the 0-1 range for loop smoothing/blending purposes.
        float percentComplete = getDuration() == std::chrono::microseconds::zero()
                                    ? 1
                                    : static_cast<float>(currentTime.count()) / getDuration().count();

        percentComplete = MATH_CLAMP(percentComplete, 0.0f, 1.0f);

        // Evaluate this clip.
        //! @todo Implement me
#if 0
        Animation::Channel* channel = nullptr;
        AnimationValue* value = nullptr;
        AnimationTarget* target = nullptr;
        size_t channelCount = _animation->_channels.size();
        float percentageStart = static_cast<float>(_startTime.count()) / _animation->_duration.count();
        float percentageEnd = static_cast<float>(_endTime.count()) / _animation->_duration.count();
        float percentageBlend = static_cast<float>(_loopBlendTime.count()) / _animation->_duration.count();
        for( size_t i = 0; i < channelCount; i++ )
        {
            channel = _animation->_channels[i];
            GP_ASSERT(channel);
            target = channel->_target;
            GP_ASSERT(target);
            value = _values[i];
            GP_ASSERT(value);

        // Evaluate the point on Curve
            GP_ASSERT(channel->getCurve());
            channel->getCurve()->evaluate(percentComplete, percentageStart, percentageEnd, percentageBlend, value->_value);

        // Set the animation value on the target property.
            target->setAnimationPropertyValue(channel->_propertyId, value, _blendWeight);
        }
#endif

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
        addRef();

        // Initialize animation to play.
        setClipStateBit(CLIP_IS_STARTED_BIT);

        _elapsedTime = Game::getGameTime() - _timeStarted;

        if( !_listeners.empty() )
            *_listenerItr = _listeners.begin();

        // Notify begin listeners if any.
        if( !_beginListeners.empty() )
        {
            auto listener = _beginListeners.begin();
            while( listener != _beginListeners.end() )
            {
                GP_ASSERT(*listener);
                (*listener)->animationEvent(this, Listener::BEGIN);
                ++listener;
            }
        }

        release();
    }


    void AnimationClip::onEnd()
    {
        addRef();

        resetClipStateBit(CLIP_ALL_BITS);

        // Notify end listeners if any.
        if( !_endListeners.empty() )
        {
            auto listener = _endListeners.begin();
            while( listener != _endListeners.end() )
            {
                GP_ASSERT(*listener);
                (*listener)->animationEvent(this, Listener::END);
                ++listener;
            }
        }

        release();
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
}
