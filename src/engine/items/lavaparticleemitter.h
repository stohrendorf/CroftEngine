#pragma once

#include "itemnode.h"

namespace engine
{
namespace items
{
class LavaParticleEmitter final : public ModelItemNode
{
public:
  LavaParticleEmitter(const gsl::not_null<Engine*>& engine,
                      const gsl::not_null<const loader::file::Room*>& room,
                      const loader::file::Item& item,
                      const loader::file::SkeletalModelType& animatedModel)
      : ModelItemNode{engine, room, item, true, animatedModel}
  {
    getSkeleton()->setDrawable(nullptr);
    getSkeleton()->removeAllChildren();
    getSkeleton()->resetPose();
  }

  void update() override;
};
} // namespace items
} // namespace engine
