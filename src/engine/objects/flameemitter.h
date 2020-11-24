#pragma once

#include "modelobject.h"

namespace engine::objects
{
class FlameEmitter final : public ModelObject
{
public:
  FlameEmitter(const gsl::not_null<World*>& world, const core::RoomBoundPosition& position)
      : ModelObject{world, position}
  {
  }

  FlameEmitter(const gsl::not_null<World*>& world,
               const gsl::not_null<const loader::file::Room*>& room,
               const loader::file::Item& item,
               const gsl::not_null<const loader::file::SkeletalModelType*>& animatedModel)
      : ModelObject{world, room, item, true, animatedModel}
  {
    getSkeleton()->setRenderable(nullptr);
    getSkeleton()->removeAllChildren();
    getSkeleton()->clearParts();
  }

  void update() override;

private:
  std::shared_ptr<Particle> m_flame;

  void removeParticle();
};
} // namespace engine::objects
