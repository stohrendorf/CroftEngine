#pragma once

#include "modelobject.h"

namespace engine::objects
{
class LavaParticleEmitter final : public ModelObject
{
public:
  LavaParticleEmitter(const gsl::not_null<Engine*>& engine, const core::RoomBoundPosition& position)
      : ModelObject{engine, position}
  {
  }

  LavaParticleEmitter(const gsl::not_null<Engine*>& engine,
                      const gsl::not_null<const loader::file::Room*>& room,
                      const loader::file::Item& item,
                      const gsl::not_null<const loader::file::SkeletalModelType*>& animatedModel)
      : ModelObject{engine, room, item, true, animatedModel}
  {
    getSkeleton()->setRenderable(nullptr);
    getSkeleton()->removeAllChildren();
    getSkeleton()->resetPose();
  }

  void update() override;

  void serialize(const serialization::Serializer& ser) override;
};
} // namespace engine::objects
