#pragma once

#include "level.h"

namespace loader
{
class DefaultAnimDispatcher final : public irr::scene::IAnimationEndCallBack
{
private:
    const Level* const m_level;
    const AnimatedModel& m_model;
    uint16_t m_currentAnimation;

public:
    DefaultAnimDispatcher(const Level* level, const AnimatedModel& model, irr::scene::IAnimatedMeshSceneNode* node);

    virtual void OnAnimationEnd(irr::scene::IAnimatedMeshSceneNode* node) override;

private:
    void startAnimLoop(irr::scene::IAnimatedMeshSceneNode* node, irr::u32 frame);

    irr::u32 getCurrentFrame(irr::scene::IAnimatedMeshSceneNode* node) const;

    uint16_t getCurrentState() const;
};
}
