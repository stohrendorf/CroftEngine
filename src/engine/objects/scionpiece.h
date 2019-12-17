#pragma once

#include "modelobject.h"
#include "spriteobject.h"

namespace engine::objects
{
class ScionPiece final : public SpriteObject
{
public:
  ScionPiece(const gsl::not_null<Engine*>& engine,
             const core::RoomBoundPosition& position,
             std::string name,
             const gsl::not_null<std::shared_ptr<render::scene::Material>>& material)
      : SpriteObject{engine, position, std::move(name), material}
  {
  }

  ScionPiece(const gsl::not_null<Engine*>& engine,
             const std::string& name,
             const gsl::not_null<const loader::file::Room*>& room,
             const loader::file::Item& item,
             const loader::file::Sprite& sprite,
             const gsl::not_null<std::shared_ptr<render::scene::Material>>& material)
      : SpriteObject{engine, name, room, item, true, &sprite, material}
  {
  }

  void update() override
  {
    updateLighting();
  }

  void collide(CollisionInfo& collisionInfo) override;
};

class ScionPiece3 final : public ModelObject
{
public:
  ScionPiece3(const gsl::not_null<Engine*>& engine, const core::RoomBoundPosition& position)
      : ModelObject{engine, position}
  {
  }

  ScionPiece3(const gsl::not_null<Engine*>& engine,
              const gsl::not_null<const loader::file::Room*>& room,
              const loader::file::Item& item,
              const gsl::not_null<const loader::file::SkeletalModelType*>& animatedModel)
      : ModelObject{engine, room, item, true, animatedModel}
  {
  }

  void update() override;

private:
  core::Frame m_deadTime = 0_frame;
};

class ScionPiece4 final : public ModelObject
{
public:
  ScionPiece4(const gsl::not_null<Engine*>& engine, const core::RoomBoundPosition& position)
      : ModelObject{engine, position}
  {
  }

  ScionPiece4(const gsl::not_null<Engine*>& engine,
              const gsl::not_null<const loader::file::Room*>& room,
              const loader::file::Item& item,
              const gsl::not_null<const loader::file::SkeletalModelType*>& animatedModel)
      : ModelObject{engine, room, item, true, animatedModel}
  {
  }

  void collide(CollisionInfo& info) override;
};

class ScionHolder final : public ModelObject
{
public:
  ScionHolder(const gsl::not_null<Engine*>& engine, const core::RoomBoundPosition& position)
      : ModelObject{engine, position}
  {
  }

  ScionHolder(const gsl::not_null<Engine*>& engine,
              const gsl::not_null<const loader::file::Room*>& room,
              const loader::file::Item& item,
              const gsl::not_null<const loader::file::SkeletalModelType*>& animatedModel)
      : ModelObject{engine, room, item, true, animatedModel}
  {
  }

  void collide(CollisionInfo& info) override;
};
} // namespace engine::objects
