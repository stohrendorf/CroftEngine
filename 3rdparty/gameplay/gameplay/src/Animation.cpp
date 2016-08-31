#include "Base.h"
#include "Animation.h"
#include "AnimationController.h"
#include "AnimationClip.h"
#include "Game.h"

namespace gameplay
{

Animation::Animation(const char* id)
    : _controller(Game::getInstance()->getAnimationController()), _id(id), _duration(0L), _clips(nullptr)
{
}

Animation::~Animation()
{
    if (_clips)
    {
        std::vector<AnimationClip*>::iterator clipIter = _clips->begin();

        while (clipIter != _clips->end())
        {
            AnimationClip* clip = *clipIter;
            GP_ASSERT(clip);
            if (clip->isClipStateBitSet(AnimationClip::CLIP_IS_PLAYING_BIT))
            {
                GP_ASSERT(_controller);
                _controller->unschedule(clip);
            }
            SAFE_RELEASE(clip);
            ++clipIter;
        }
        _clips->clear();
    }
    SAFE_DELETE(_clips);
}

const std::string& Animation::getId() const
{
    return _id;
}

std::chrono::microseconds Animation::getDuration() const
{
    return _duration;
}

AnimationClip* Animation::createClip(const std::string& id, const std::chrono::microseconds& begin, const std::chrono::microseconds& end)
{
    AnimationClip* clip = new AnimationClip(id, this, begin, end);
    addClip(clip);
    return clip;
}

AnimationClip* Animation::getClip(const std::string& id)
{
    return findClip(id);
}

AnimationClip* Animation::getClip(unsigned int index) const
{
    if (_clips)
        return _clips->at(index);

    return nullptr;
}

size_t Animation::getClipCount() const
{
    return _clips ? _clips->size() : 0;
}

void Animation::play(const std::string& clipId)
{
    // Find animation clip and play.
    AnimationClip* clip = findClip(clipId);
    if (clip != nullptr)
        clip->play();
}

void Animation::stop(const std::string& clipId)
{
    // Find animation clip and play.
    AnimationClip* clip = findClip(clipId);
    if (clip != nullptr)
        clip->stop();
}

void Animation::pause(const std::string& clipId)
{
    AnimationClip* clip = findClip(clipId);
    if (clip != nullptr)
        clip->pause();
}

void Animation::addClip(AnimationClip* clip)
{
    if (_clips == nullptr)
        _clips = new std::vector<AnimationClip*>;

    GP_ASSERT(clip);
    _clips->push_back(clip);
}

AnimationClip* Animation::findClip(const std::string& id) const
{
    if (_clips)
    {
        size_t clipCount = _clips->size();
        for (size_t i = 0; i < clipCount; i++)
        {
            AnimationClip* clip = _clips->at(i);
            GP_ASSERT(clip);
            if (clip->_id == id)
            {
                return clip;
            }
        }
    }
    return nullptr;
}

}
