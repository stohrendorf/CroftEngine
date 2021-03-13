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
            const gsl::not_null<const loader::file::Room*>& room,
            const loader::file::Item& item,
            const gsl::not_null<const loader::file::SkeletalModelType*>& animatedModel)
      : ModelObject{world, room, item, true, animatedModel}
  {
    loader::file::Room::patchHeightsForBlock(*this, -2 * core::SectorSize);
  }

  void update() override;

  void serialize(const serialization::Serializer<world::World>& ser) override;
};
} // namespace engine::objects
