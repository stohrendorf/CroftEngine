#pragma once

#include "itemnode.h"

namespace engine
{
namespace items
{
class ThorHammerBlock final : public ModelItemNode
{
  public:
  ThorHammerBlock(const gsl::not_null<Engine*>& engine,
                  const gsl::not_null<const loader::file::Room*>& room,
                  const loader::file::Item& item,
                  const loader::file::SkeletalModelType& animatedModel)
      : ModelItemNode{engine, room, item, true, animatedModel}
  {
  }

  void collide(LaraNode& node, CollisionInfo& info) override;
};

class ThorHammerHandle final : public ModelItemNode
{
  public:
  ThorHammerHandle(const gsl::not_null<Engine*>& engine,
                   const gsl::not_null<const loader::file::Room*>& room,
                   const loader::file::Item& item,
                   const loader::file::SkeletalModelType& animatedModel);

  void update() override;

  void collide(LaraNode& node, CollisionInfo& info) override;

  private:
  std::shared_ptr<ThorHammerBlock> m_block;
};
} // namespace items
} // namespace engine
