#pragma once

#include "engine/ai/ai.h"
#include "modelobject.h"

namespace engine::objects
{
class Doppelganger final : public ModelObject
{
public:
  Doppelganger(const gsl::not_null<world::World*>& world, const core::RoomBoundPosition& position)
      : ModelObject{world, position}
  {
  }

  Doppelganger(const gsl::not_null<world::World*>& world,
               const gsl::not_null<const loader::file::Room*>& room,
               const loader::file::Item& item,
               const gsl::not_null<const loader::file::SkeletalModelType*>& animatedModel);

  void update() override;

private:
  bool m_flag = false;
};
} // namespace engine::objects
