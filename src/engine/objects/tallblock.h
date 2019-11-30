#pragma once

#include "modelobject.h"

namespace engine::objects
{
class TallBlock final : public ModelObject
{
public:
  TallBlock(const gsl::not_null<Engine*>& engine, const core::RoomBoundPosition& position)
      : ModelObject{engine, position}
  {
  }

  TallBlock(const gsl::not_null<Engine*>& engine,
            const gsl::not_null<const loader::file::Room*>& room,
            const loader::file::Item& item,
            const gsl::not_null<const loader::file::SkeletalModelType*>& animatedModel)
      : ModelObject{engine, room, item, true, animatedModel}
  {
    loader::file::Room::patchHeightsForBlock(*this, -2 * core::SectorSize);
  }

  void update() override;

  void serialize(const serialization::Serializer& ser) override;
};
} // namespace engine
