#pragma once

#include "engine/ai/ai.h"
#include "itemnode.h"

namespace engine
{
namespace items
{
extern bool shatterModel(ModelItemNode& item, const std::bitset<32>& meshMask, const core::Length& damageRadius);

class MutantEgg final : public ModelItemNode
{
public:
  MutantEgg(const gsl::not_null<Engine*>& engine,
            const gsl::not_null<const loader::file::Room*>& room,
            loader::file::Item item,
            const loader::file::SkeletalModelType& animatedModel);

  void update() override;

  void collide(LaraNode& lara, CollisionInfo& info) override;

private:
  core::Angle m_headRotation{0_deg};
  std::shared_ptr<ModelItemNode> m_childItem{nullptr};
};
} // namespace items
} // namespace engine
