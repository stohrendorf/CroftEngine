#pragma once

#include "itemnode.h"

namespace engine
{
namespace items
{
class FlameEmitter final : public ModelItemNode
{
public:
    FlameEmitter(const gsl::not_null<level::Level*>& level,
                  const gsl::not_null<const loader::Room*>& room,
                  const loader::Item& item,
                  const loader::SkeletalModelType& animatedModel)
            : ModelItemNode{level, room, item, true, animatedModel}
    {
        getSkeleton()->setDrawable( nullptr );
        getSkeleton()->removeAllChildren();
        getSkeleton()->resetPose();
    }

    void update() override;

private:
    std::shared_ptr<Particle> m_flame;

    void removeParticle();
};
}
}
