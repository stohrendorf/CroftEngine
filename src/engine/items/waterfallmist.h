#pragma once

#include "itemnode.h"

namespace engine
{
namespace items
{
class WaterfallMist final : public ModelItemNode
{
public:
    WaterfallMist(const gsl::not_null<loader::file::level::Level*>& level,
                  const gsl::not_null<const loader::file::Room*>& room,
                  const loader::file::Item& item,
                  const loader::file::SkeletalModelType& animatedModel)
            : ModelItemNode{level, room, item, true, animatedModel}
    {
        getSkeleton()->setDrawable( nullptr );
        getSkeleton()->removeAllChildren();
        getSkeleton()->resetPose();
    }

    void update() override;
};
}
}
