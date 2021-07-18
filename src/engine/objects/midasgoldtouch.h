#pragma once

#include "modelobject.h"

namespace engine::objects
{
class MidasGoldTouch final : public NullRenderModelObject
{
public:
  MidasGoldTouch(const gsl::not_null<world::World*>& world, const RoomBoundPosition& position)
      : NullRenderModelObject{world, position}
  {
  }

  MidasGoldTouch(const gsl::not_null<world::World*>& world,
                 const gsl::not_null<const world::Room*>& room,
                 const loader::file::Item& item,
                 const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
      : NullRenderModelObject{world, room, item, false, animatedModel}
  {
  }

  void collide(CollisionInfo& info) override;
};
} // namespace engine::objects
