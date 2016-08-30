#include "Base.h"
#include "AnimationTarget.h"
#include "Animation.h"
#include "Game.h"
#include "Node.h"
#include "MaterialParameter.h"

#define ANIMATION_TARGET_INDEFINITE_STR "INDEFINITE"

namespace gameplay
{

AnimationTarget::AnimationTarget()
    : _targetType(SCALAR), _animationChannels(NULL)
{
}

AnimationTarget::~AnimationTarget()
{
    if (_animationChannels)
    {
        std::vector<Animation::Channel*>::iterator itr = _animationChannels->begin();
        while (itr != _animationChannels->end())
        {
            Animation::Channel* channel = (*itr);
            GP_ASSERT(channel);
            GP_ASSERT(channel->_animation);
            channel->_animation->removeChannel(channel);
            SAFE_DELETE(channel);
            itr++;
        }
        _animationChannels->clear();
        SAFE_DELETE(_animationChannels);
    }
}

Animation* AnimationTarget::createAnimation(const char* id, int propertyId, unsigned int keyCount, const unsigned* keyTimes, const float* keyValues, Curve::InterpolationType type)
{
    GP_ASSERT(type != Curve::BEZIER && type != Curve::HERMITE);
    GP_ASSERT(keyCount >= 1 && keyTimes && keyValues);

    Animation* animation = new Animation(id, this, propertyId, keyCount, keyTimes, keyValues, type);

    return animation;
}

Animation* AnimationTarget::createAnimation(const char* id, int propertyId, unsigned int keyCount, const unsigned* keyTimes, const float* keyValues, const float* keyInValue, const float* keyOutValue, Curve::InterpolationType type)
{
    GP_ASSERT(keyCount >= 1 && keyTimes && keyValues && keyInValue && keyOutValue);
    Animation* animation = new Animation(id, this, propertyId, keyCount, keyTimes, keyValues, keyInValue, keyOutValue, type);

    return animation;
}

Animation* AnimationTarget::createAnimationFromTo(const char* id, int propertyId, float* from, float* to, Curve::InterpolationType type, unsigned long duration)
{
    GP_ASSERT(from);
    GP_ASSERT(to);

    const unsigned int propertyComponentCount = getAnimationPropertyComponentCount(propertyId);
    GP_ASSERT(propertyComponentCount > 0);
    float* keyValues = new float[2 * propertyComponentCount];

    memcpy(keyValues, from, sizeof(float) * propertyComponentCount);
    memcpy(keyValues + propertyComponentCount, to, sizeof(float) * propertyComponentCount);

    unsigned int* keyTimes = new unsigned int[2];
    keyTimes[0] = 0;
    keyTimes[1] = (unsigned int)duration;

    Animation* animation = createAnimation(id, propertyId, 2, keyTimes, keyValues, type);

    SAFE_DELETE_ARRAY(keyValues);
    SAFE_DELETE_ARRAY(keyTimes);

    return animation;
}

Animation* AnimationTarget::createAnimationFromBy(const char* id, int propertyId, float* from, float* by, Curve::InterpolationType type, unsigned long duration)
{
    GP_ASSERT(from);
    GP_ASSERT(by);

    const unsigned int propertyComponentCount = getAnimationPropertyComponentCount(propertyId);
    GP_ASSERT(propertyComponentCount > 0);
    float* keyValues = new float[2 * propertyComponentCount];

    memcpy(keyValues, from, sizeof(float) * propertyComponentCount);

    convertByValues(propertyId, propertyComponentCount, from, by);
    memcpy(keyValues + propertyComponentCount, by, sizeof(float) * propertyComponentCount);

    unsigned int* keyTimes = new unsigned int[2];
    keyTimes[0] = 0;
    keyTimes[1] = (unsigned int)duration;

    Animation* animation = createAnimation(id, propertyId, 2, keyTimes, keyValues, type);

    SAFE_DELETE_ARRAY(keyValues);
    SAFE_DELETE_ARRAY(keyTimes);

    return animation;
}

void AnimationTarget::destroyAnimation(const char* id)
{
    // Find the animation with the specified ID.
    Animation::Channel* channel = getChannel(id);
    if (channel == NULL)
        return;

    // Remove this target's channel from animation, and from the target's list of channels.
    GP_ASSERT(channel->_animation);
    channel->_animation->removeChannel(channel);
    removeChannel(channel);

    SAFE_DELETE(channel);
}

Animation* AnimationTarget::getAnimation(const char* id) const
{
    if (_animationChannels)
    {
        std::vector<Animation::Channel*>::iterator itr = _animationChannels->begin();
        GP_ASSERT(*itr);

        if (id == NULL)
            return (*itr)->_animation;

        Animation::Channel* channel = NULL;
        for (; itr != _animationChannels->end(); itr++)
        {
            channel = (Animation::Channel*)(*itr);
            GP_ASSERT(channel);
            GP_ASSERT(channel->_animation);
            if (channel->_animation->_id.compare(id) == 0)
            {
                return channel->_animation;
            }
        }
    }

    return NULL;
}

int AnimationTarget::getPropertyId(TargetType type, const char* propertyIdStr)
{
    GP_ASSERT(propertyIdStr);

    if (type == AnimationTarget::TRANSFORM)
    {
        if (strcmp(propertyIdStr, "ANIMATE_SCALE") == 0)
        {
            return Transform::ANIMATE_SCALE;
        }
        else if (strcmp(propertyIdStr, "ANIMATE_SCALE_X") == 0)
        {
            return Transform::ANIMATE_SCALE_X;
        }
        else if (strcmp(propertyIdStr, "ANIMATE_SCALE_Y") == 0)
        {
            return Transform::ANIMATE_SCALE_Y;
        }
        else if (strcmp(propertyIdStr, "ANIMATE_SCALE_Z") == 0)
        {
            return Transform::ANIMATE_SCALE_Z;
        }
        else if (strcmp(propertyIdStr, "ANIMATE_ROTATE") == 0)
        {
            return Transform::ANIMATE_ROTATE;
        }
        else if (strcmp(propertyIdStr, "ANIMATE_TRANSLATE") == 0)
        {
            return Transform::ANIMATE_TRANSLATE;
        }
        else if (strcmp(propertyIdStr, "ANIMATE_TRANSLATE_X") == 0)
        {
            return Transform::ANIMATE_TRANSLATE_X;
        }
        else if (strcmp(propertyIdStr, "ANIMATE_TRANSLATE_Y") == 0)
        {
            return Transform::ANIMATE_TRANSLATE_Y;
        }
        else if (strcmp(propertyIdStr, "ANIMATE_TRANSLATE_Z") == 0)
        {
            return Transform::ANIMATE_TRANSLATE_Z;
        }
        else if (strcmp(propertyIdStr, "ANIMATE_ROTATE_TRANSLATE") == 0)
        {
            return Transform::ANIMATE_ROTATE_TRANSLATE;
        }
        else if (strcmp(propertyIdStr, "ANIMATE_SCALE_ROTATE_TRANSLATE") == 0)
        {
            return Transform::ANIMATE_SCALE_ROTATE_TRANSLATE;
        }
    }
    else
    {
        if (strcmp(propertyIdStr, "ANIMATE_UNIFORM") == 0)
        {
            return MaterialParameter::ANIMATE_UNIFORM;
        }
    }

    return -1;
}

void AnimationTarget::addChannel(Animation::Channel* channel)
{
    if (_animationChannels == NULL)
        _animationChannels = new std::vector<Animation::Channel*>;

    GP_ASSERT(channel);
    _animationChannels->push_back(channel);
}

void AnimationTarget::removeChannel(Animation::Channel* channel)
{
    if (_animationChannels)
    {
        std::vector<Animation::Channel*>::iterator itr = _animationChannels->begin();
        for ( ; itr != _animationChannels->end(); itr++)
        {
            Animation::Channel* temp = *itr;
            if (channel == temp)
            {
                _animationChannels->erase(itr);

                if (_animationChannels->empty())
                    SAFE_DELETE(_animationChannels);

                return;
            }
        }
    }
}

Animation::Channel* AnimationTarget::getChannel(const char* id) const
{
    if (_animationChannels)
    {
        std::vector<Animation::Channel*>::iterator itr = _animationChannels->begin();

        if (id == NULL)
            return (*itr);

        Animation::Channel* channel = NULL;
        for (; itr != _animationChannels->end(); itr++)
        {
            channel = (Animation::Channel*)(*itr);
            GP_ASSERT(channel);
            if (channel->_animation->_id.compare(id) == 0)
            {
                return channel;
            }
        }
    }

    return NULL;
}

void AnimationTarget::convertByValues(unsigned int propertyId, unsigned int componentCount, float* from, float* by)
{
    if (_targetType == AnimationTarget::TRANSFORM)
    {
        switch(propertyId)
        {
            case Transform::ANIMATE_SCALE:
            case Transform::ANIMATE_SCALE_UNIT:
            case Transform::ANIMATE_SCALE_X:
            case Transform::ANIMATE_SCALE_Y:
            case Transform::ANIMATE_SCALE_Z:
            {
                convertScaleByValues(from, by, componentCount);
                break;
            }
            case Transform::ANIMATE_TRANSLATE:
            case Transform::ANIMATE_TRANSLATE_X:
            case Transform::ANIMATE_TRANSLATE_Y:
            case Transform::ANIMATE_TRANSLATE_Z:
            {
                convertByValues(from, by, componentCount);
                break;
            }
            case Transform::ANIMATE_ROTATE:
            {
                convertQuaternionByValues(from, by);
                break;
            }
            case Transform::ANIMATE_ROTATE_TRANSLATE:
            {
                convertQuaternionByValues(from, by);
                convertByValues(from + 4, by + 4, 3);
                break;
            }
            case Transform::ANIMATE_SCALE_ROTATE_TRANSLATE:
            {
                convertScaleByValues(from, by, 3);
                convertQuaternionByValues(from + 3, by + 3);
                convertByValues(from + 7, by + 7, 3);
                break;
            }
        }
    }
    else
    {
        convertByValues(from, by, componentCount);
    }
}

void AnimationTarget::convertQuaternionByValues(float* from, float* by)
{
    Quaternion qFrom(from);
    Quaternion qBy(by);
    qBy.multiply(qFrom);
    memcpy(by, (float*)&qBy, sizeof(float) * 4);
}

void AnimationTarget::convertScaleByValues(float* from, float* by, unsigned int componentCount)
{
    for (unsigned int i = 0; i < componentCount; i++)
    {
        by[i] *= from[i];
    }
}

void AnimationTarget::convertByValues(float* from, float* by, unsigned int componentCount)
{
    for (unsigned int i = 0; i < componentCount; i++)
    {
        by[i] += from[i];
    }
}

}



