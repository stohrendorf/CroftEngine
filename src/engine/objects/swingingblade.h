#pragma once

#include "modelobject.h"

namespace engine::objects
{
class SwingingBlade final : public ModelObject
{
public:
  SwingingBlade(const gsl::not_null<Engine*>& engine, const core::RoomBoundPosition& position)
      : ModelObject{engine, position}
  {
  }

  SwingingBlade(const gsl::not_null<Engine*>& engine,
                const gsl::not_null<const loader::file::Room*>& room,
                const loader::file::Item& item,
                const gsl::not_null<const loader::file::SkeletalModelType*>& animatedModel)
      : ModelObject{engine, room, item, true, animatedModel}
  {
  }

  void update() override;

  void collide(CollisionInfo& collisionInfo) override;
};
} // namespace engine
