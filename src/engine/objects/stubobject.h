#pragma once

#include "modelobject.h"

namespace engine::objects
{
class StubObject final : public ModelObject
{
public:
  StubObject(const gsl::not_null<world::World*>& world, const RoomBoundPosition& position)
      : ModelObject{world, position}
  {
  }

  StubObject(const gsl::not_null<world::World*>& world,
             const gsl::not_null<const world::Room*>& room,
             const loader::file::Item& item,
             const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
      : ModelObject{world, room, item, false, animatedModel}
  {
  }
};
} // namespace engine::objects
