#pragma once

#include "modelobject.h"
#include "spriteobject.h"

namespace engine::objects
{
class ScionPiece final : public SpriteObject
{
public:
  ScionPiece(const gsl::not_null<world::World*>& world, const Location& location, std::string name)
      : SpriteObject{world, location, std::move(name)}
  {
  }

  ScionPiece(const gsl::not_null<world::World*>& world,
             const std::string& name,
             const gsl::not_null<const world::Room*>& room,
             const loader::file::Item& item,
             const gsl::not_null<const world::Sprite*>& sprite)
      : SpriteObject{world, name, room, item, true, sprite}
  {
  }

  void collide(CollisionInfo& collisionInfo) override;
};

class ScionPiece3 final : public ModelObject
{
public:
  ScionPiece3(const gsl::not_null<world::World*>& world, const Location& location)
      : ModelObject{world, location}
  {
  }

  ScionPiece3(const gsl::not_null<world::World*>& world,
              const gsl::not_null<const world::Room*>& room,
              const loader::file::Item& item,
              const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
      : ModelObject{world, room, item, true, animatedModel}
  {
  }

  void update() override;

private:
  core::Frame m_deadTime = 0_frame;
};

class ScionPiece4 final : public ModelObject
{
public:
  ScionPiece4(const gsl::not_null<world::World*>& world, const Location& location)
      : ModelObject{world, location}
  {
  }

  ScionPiece4(const gsl::not_null<world::World*>& world,
              const gsl::not_null<const world::Room*>& room,
              const loader::file::Item& item,
              const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
      : ModelObject{world, room, item, true, animatedModel}
  {
  }

  void collide(CollisionInfo& info) override;
};

class ScionHolder final : public ModelObject
{
public:
  ScionHolder(const gsl::not_null<world::World*>& world, const Location& location)
      : ModelObject{world, location}
  {
  }

  ScionHolder(const gsl::not_null<world::World*>& world,
              const gsl::not_null<const world::Room*>& room,
              const loader::file::Item& item,
              const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
      : ModelObject{world, room, item, true, animatedModel}
  {
  }

  void collide(CollisionInfo& info) override;
};
} // namespace engine::objects
