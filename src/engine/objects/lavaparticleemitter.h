#pragma once

#include "modelobject.h"

namespace engine::objects
{
class LavaParticleEmitter final : public NullRenderModelObject
{
public:
  LavaParticleEmitter(const gsl::not_null<world::World*>& world, const Location& location)
      : NullRenderModelObject{world, location}
  {
  }

  LavaParticleEmitter(const std::string& name,
                      const gsl::not_null<world::World*>& world,
                      const gsl::not_null<const world::Room*>& room,
                      const loader::file::Item& item,
                      const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
      : NullRenderModelObject{name, world, room, item, true, animatedModel}
  {
  }

  void update() override;
};
} // namespace engine::objects
