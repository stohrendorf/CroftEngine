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
                  const gsl::not_null<const loader::Room*>& room,
                  const loader::Item& item,
                  const loader::SkeletalModelType& animatedModel)
            : ModelItemNode( level, room, item, false, animatedModel )
    {
        getNode()->setDrawable( nullptr );
        getNode()->removeAllChildren();
        getSkeleton()->resetPose();
    }

    void update() override;
};
}
}
