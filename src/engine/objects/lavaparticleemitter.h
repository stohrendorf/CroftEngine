#pragma once

#include "modelobject.h"

namespace engine::objects
{
class LavaParticleEmitter final : public ModelObject
{
public:
  LavaParticleEmitter(const gsl::not_null<World*>& world, const core::RoomBoundPosition& position)
      : ModelObject{world, position}
  {
  }

  LavaParticleEmitter(const gsl::not_null<World*>& world,
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

  void serialize(const serialization::Serializer<World>& ser) override;
};
} // namespace engine::objects
