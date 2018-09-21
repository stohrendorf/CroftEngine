#pragma once

#include "itemnode.h"

namespace engine
{
namespace items
{
class WaterfallMist final : public ModelItemNode
{
public:
    WaterfallMist(const gsl::not_null<level::Level*>& level,
                  const std::string& name,
                  const gsl::not_null<const loader::Room*>& room,
                  const loader::Item& item,
                  const loader::SkeletalModelType& animatedModel)
            : ModelItemNode( level, name, room, item, true, animatedModel )
    {
        getNode()->setDrawable( nullptr );
        getNode()->removeAllChildren();
    }

    void update() override;

    void collide(LaraNode& lara, CollisionInfo& collisionInfo) override
    {

    }
};
}
}
