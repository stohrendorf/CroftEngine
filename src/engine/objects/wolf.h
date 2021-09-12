#pragma once

#include "aiagent.h"

namespace engine::objects
{
class Wolf final : public AIAgent
{
public:
  Wolf(const gsl::not_null<world::World*>& world, const Location& location)
      : AIAgent{world, location}
  {
  }

  Wolf(const std::string& name,
       const gsl::not_null<world::World*>& world,
       const gsl::not_null<const world::Room*>& room,
       const loader::file::Item& item,
       const gsl::not_null<const world::SkeletalModelType*>& animatedModel);

  void update() override;
};
} // namespace engine::objects
