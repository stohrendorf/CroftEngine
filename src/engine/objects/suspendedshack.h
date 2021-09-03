#pragma once

#include "engine/world/sector.h"
#include "modelobject.h"

namespace engine::objects
{
class SuspendedShack final : public ModelObject
{
public:
  SuspendedShack(const gsl::not_null<world::World*>& world, const Location& location)
      : ModelObject{world, location}
  {
  }

  SuspendedShack(const std::string& name,
                 const gsl::not_null<world::World*>& world,
                 const gsl::not_null<const world::Room*>& room,
                 const loader::file::Item& item,
                 const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
      : ModelObject{name, world, room, item, true, animatedModel}
  {
    getSkeleton()->getRenderState().setScissorTest(false);
  }

  void update() override;
  void collide(CollisionInfo& collisionInfo) override;
  void serialize(const serialization::Serializer<world::World>& ser) override;
};
} // namespace engine::objects
