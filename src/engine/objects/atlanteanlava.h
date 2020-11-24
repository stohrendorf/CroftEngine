#pragma once

#include "modelobject.h"

namespace engine::objects
{
class AtlanteanLava final : public ModelObject
{
public:
  AtlanteanLava(const gsl::not_null<World*>& world, const core::RoomBoundPosition& position)
      : ModelObject{world, position}
  {
  }

  AtlanteanLava(const gsl::not_null<World*>& world,
                const gsl::not_null<const loader::file::Room*>& room,
                const loader::file::Item& item,
                const gsl::not_null<const loader::file::SkeletalModelType*>& animatedModel)
      : ModelObject{world, room, item, true, animatedModel}
  {
  }

  void update() override;
};
} // namespace engine::objects
