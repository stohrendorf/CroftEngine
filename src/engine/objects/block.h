#pragma once

#include "modelobject.h"

namespace engine::objects
{
class Block final : public ModelObject
{
public:
  Block(const gsl::not_null<Engine*>& engine, const core::RoomBoundPosition& position)
      : ModelObject{engine, position}
  {
  }

  Block(const gsl::not_null<Engine*>& engine,
        const gsl::not_null<const loader::file::Room*>& room,
        const loader::file::Item& item,
        const gsl::not_null<const loader::file::SkeletalModelType*>& animatedModel)
      : ModelObject{engine, room, item, true, animatedModel}
  {
    if(m_state.triggerState != TriggerState::Invisible)
    {
      loader::file::Room::patchHeightsForBlock(*this, -core::SectorSize);
      m_patched = true;
    }
  }

  void collide(CollisionInfo& collisionInfo) override;

  void update() override;

  void serialize(const serialization::Serializer& ser) override;

private:
  bool isOnFloor(const core::Length& height) const;

  bool canPushBlock(const core::Length& height, core::Axis axis) const;

  bool canPullBlock(const core::Length& height, core::Axis axis) const;

  bool m_patched = false;
};
} // namespace engine
