#pragma once

#include "modelobject.h"

namespace engine::objects
{
class WaterfallMist final : public ModelObject
{
public:
  WaterfallMist(const gsl::not_null<Engine*>& engine, const core::RoomBoundPosition& position)
      : ModelObject{engine, position}
  {
  }

  WaterfallMist(const gsl::not_null<Engine*>& engine,
                const gsl::not_null<const loader::file::Room*>& room,
                const loader::file::Item& item,
                const gsl::not_null<const loader::file::SkeletalModelType*>& animatedModel)
      : ModelObject{engine, room, item, true, animatedModel}
  {
    getSkeleton()->setDrawable(nullptr);
    getSkeleton()->removeAllChildren();
    getSkeleton()->resetPose();
  }

  void update() override;
};
} // namespace engine
