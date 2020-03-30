#pragma once

#include "modelobject.h"

namespace engine::objects
{
class Earthquake final : public ModelObject
{
public:
  Earthquake(const gsl::not_null<Engine*>& engine, const core::RoomBoundPosition& position)
      : ModelObject{engine, position}
  {
  }

  Earthquake(const gsl::not_null<Engine*>& engine,
             const gsl::not_null<const loader::file::Room*>& room,
             const loader::file::Item& item,
             const gsl::not_null<const loader::file::SkeletalModelType*>& animatedModel)
      : ModelObject{engine, room, item, true, animatedModel}
  {
    getSkeleton()->setRenderable(nullptr);
    getSkeleton()->clearParts();
  }

  void update() override;
};
} // namespace engine::objects
