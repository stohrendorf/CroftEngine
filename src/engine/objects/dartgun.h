#pragma once

#include "dart.h"

namespace engine::objects
{
class DartGun final : public ModelObject
{
public:
  DartGun(const gsl::not_null<Engine*>& engine, const core::RoomBoundPosition& position)
      : ModelObject{engine, position}
  {
  }

  DartGun(const gsl::not_null<Engine*>& engine,
          const gsl::not_null<const loader::file::Room*>& room,
          const loader::file::Item& item,
          const gsl::not_null<const loader::file::SkeletalModelType*>& animatedModel)
      : ModelObject{engine, room, item, true, animatedModel}
  {
  }

  void update() override;
};
} // namespace engine
