#pragma once

#include "engine/world/room.h"
#include "modelobject.h"

namespace engine::objects
{
class Block final : public ModelObject
{
public:
  Block(const gsl::not_null<world::World*>& world, const Location& location)
      : ModelObject{world, location}
  {
  }

  Block(const std::string& name,
        const gsl::not_null<world::World*>& world,
        const gsl::not_null<const world::Room*>& room,
        const loader::file::Item& item,
        const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
      : ModelObject{name, world, room, item, true, animatedModel}
  {
    if(m_state.triggerState != TriggerState::Invisible)
    {
      world::patchHeightsForBlock(*this, -core::SectorSize);
    }
  }

  void collide(CollisionInfo& collisionInfo) override;

  void update() override;

private:
  bool isOnFloor(const core::Length& height) const;

  bool canPushBlock(const core::Length& height, core::Axis axis) const;

  bool canPullBlock(const core::Length& height, core::Axis axis) const;
};
} // namespace engine::objects
