#pragma once

#include "engine/ai/ai.h"
#include "modelobject.h"

namespace engine::objects
{
class Doppelganger final : public ModelObject
{
public:
  Doppelganger(const gsl::not_null<world::World*>& world, const Location& location)
      : ModelObject{world, location}
  {
  }

  Doppelganger(const std::string& name,
               const gsl::not_null<world::World*>& world,
               const gsl::not_null<const world::Room*>& room,
               const loader::file::Item& item,
               const gsl::not_null<const world::SkeletalModelType*>& animatedModel);

  void update() override;

private:
  bool m_flag = false;
};
} // namespace engine::objects
