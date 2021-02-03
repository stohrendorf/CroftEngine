#pragma once

#include "modelobject.h"

namespace engine::objects
{
class FlameEmitter final : public NullRenderModelObject
{
public:
  FlameEmitter(const gsl::not_null<World*>& world, const core::RoomBoundPosition& position)
      : NullRenderModelObject{world, position}
  {
  }

  FlameEmitter(const gsl::not_null<World*>& world,
               const gsl::not_null<const loader::file::Room*>& room,
               const loader::file::Item& item,
               const gsl::not_null<const loader::file::SkeletalModelType*>& animatedModel)
      : NullRenderModelObject{world, room, item, true, animatedModel}
  {
  }

  void update() override;

private:
  std::shared_ptr<Particle> m_flame;

  void removeParticle();
};
} // namespace engine::objects
