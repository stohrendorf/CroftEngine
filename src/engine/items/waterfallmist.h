#pragma once

#include "itemnode.h"

namespace engine
{
namespace items
{
class WaterfallMist final : public ModelItemNode
{
public:
    WaterfallMist(const gsl::not_null<Engine*>& engine,
                  const gsl::not_null<const loader::file::Room*>& room,
                  const loader::file::Item& item,
                  const loader::file::SkeletalModelType& animatedModel)
        : ModelItemNode{engine, room, item, true, animatedModel}
    {
        getSkeleton()->setDrawable(nullptr);
        getSkeleton()->removeAllChildren();
        getSkeleton()->resetPose();
    }

    void update() override;
};
} // namespace items
} // namespace engine
