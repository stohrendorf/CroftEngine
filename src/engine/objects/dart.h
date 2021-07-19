#pragma once

#include "modelobject.h"

namespace engine::objects
{
class Dart final : public ModelObject
{
public:
  Dart(const gsl::not_null<world::World*>& world, const RoomBoundPosition& location)
      : ModelObject{world, location}
  {
  }

  Dart(const gsl::not_null<world::World*>& world,
       const gsl::not_null<const world::Room*>& room,
       const loader::file::Item& item,
       const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
      : ModelObject{world, room, item, true, animatedModel}
  {
    m_state.collidable = true;
  }

  void collide(CollisionInfo& info) override;

  void update() override;
};
} // namespace engine::objects
