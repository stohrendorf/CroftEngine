#pragma once

#include "modelobject.h"

namespace engine::objects
{
class Dart final : public ModelObject
{
public:
  Dart(const gsl::not_null<World*>& world, const core::RoomBoundPosition& position)
      : ModelObject{world, position}
  {
  }

  Dart(const gsl::not_null<World*>& world,
       const gsl::not_null<const loader::file::Room*>& room,
       const loader::file::Item& item,
       const gsl::not_null<const loader::file::SkeletalModelType*>& animatedModel)
      : ModelObject{world, room, item, true, animatedModel}
  {
    m_state.collidable = true;
  }

  void collide(CollisionInfo& info) override;

  void update() override;
};
} // namespace engine::objects
