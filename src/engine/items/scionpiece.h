#pragma once

#include "itemnode.h"

namespace engine
{
namespace items
{
class ScionPiece final : public SpriteItemNode
{
public:
  ScionPiece(const gsl::not_null<Engine*>& engine,
             const std::string& name,
             const gsl::not_null<const loader::file::Room*>& room,
             const loader::file::Item& item,
             const loader::file::Sprite& sprite,
             const gsl::not_null<std::shared_ptr<render::scene::Material>>& material)
      : SpriteItemNode{engine, name, room, item, true, sprite, material}
  {
  }

  void update() override
  {
    updateLighting();
  }

  void collide(LaraNode& lara, CollisionInfo& collisionInfo) override;
};

class ScionPiece3 final : public ModelItemNode
{
public:
  ScionPiece3(const gsl::not_null<Engine*>& engine,
              const gsl::not_null<const loader::file::Room*>& room,
              const loader::file::Item& item,
              const loader::file::SkeletalModelType& animatedModel)
      : ModelItemNode{engine, room, item, false, animatedModel}
  {
  }

  void update() override;

private:
  core::Frame m_deadTime = 0_frame;
};
} // namespace items
} // namespace engine
