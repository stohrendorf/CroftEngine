#pragma once

#include "modelobject.h"

namespace engine::objects
{
class TallBlock final : public ModelObject
{
public:
  TallBlock(const gsl::not_null<world::World*>& world, const core::RoomBoundPosition& position)
      : ModelObject{world, position}
  {
  }

  TallBlock(const gsl::not_null<world::World*>& world,
            const gsl::not_null<const world::Room*>& room,
            const loader::file::Item& item,
            const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
      : ModelObject{world, room, item, true, animatedModel}
  {
    world::Room::patchHeightsForBlock(*this, -2 * core::SectorSize);
  }

  void update() override;

  void serialize(const serialization::Serializer<world::World>& ser) override;
};
} // namespace engine::objects
