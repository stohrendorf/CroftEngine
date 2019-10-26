#pragma once

#include "itemnode.h"

namespace engine
{
namespace items
{
class FlameEmitter final : public ModelItemNode
{
  public:
  FlameEmitter(const gsl::not_null<Engine*>& engine,
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

  private:
  std::shared_ptr<Particle> m_flame;

  void removeParticle();
};
} // namespace items
} // namespace engine
