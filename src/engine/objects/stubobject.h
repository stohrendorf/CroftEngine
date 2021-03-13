#pragma once

#include "modelobject.h"

namespace engine::objects
{
class StubObject final : public ModelObject
{
public:
  StubObject(const gsl::not_null<world::World*>& world, const core::RoomBoundPosition& position)
      : ModelObject{world, position}
  {
  }

  StubObject(const gsl::not_null<world::World*>& world,
             const gsl::not_null<const loader::file::Room*>& room,
             const loader::file::Item& item,
             const gsl::not_null<const loader::file::SkeletalModelType*>& animatedModel)
      : ModelObject{world, room, item, false, animatedModel}
  {
  }
};
} // namespace engine::objects
