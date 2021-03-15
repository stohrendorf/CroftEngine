#pragma once

#include "modelobject.h"

namespace engine::objects
{
class CutsceneActor : public ModelObject
{
public:
  CutsceneActor(const gsl::not_null<world::World*>& world, const core::RoomBoundPosition& position)
      : ModelObject{world, position}
  {
  }

  CutsceneActor(const gsl::not_null<world::World*>& world,
                const gsl::not_null<const world::Room*>& room,
                const loader::file::Item& item,
                const gsl::not_null<const world::SkeletalModelType*>& animatedModel);

  void update() override;
};

class CutsceneActor1 final : public CutsceneActor
{
public:
  CutsceneActor1(const gsl::not_null<world::World*>& world, const core::RoomBoundPosition& position)
      : CutsceneActor{world, position}
  {
  }

  CutsceneActor1(const gsl::not_null<world::World*>& world,
                 const gsl::not_null<const world::Room*>& room,
                 const loader::file::Item& item,
                 const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
      : CutsceneActor(world, room, item, animatedModel)
  {
  }
};

class CutsceneActor2 final : public CutsceneActor
{
public:
  CutsceneActor2(const gsl::not_null<world::World*>& world, const core::RoomBoundPosition& position)
      : CutsceneActor{world, position}
  {
  }

  CutsceneActor2(const gsl::not_null<world::World*>& world,
                 const gsl::not_null<const world::Room*>& room,
                 const loader::file::Item& item,
                 const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
      : CutsceneActor(world, room, item, animatedModel)
  {
  }
};

class CutsceneActor3 final : public CutsceneActor
{
public:
  CutsceneActor3(const gsl::not_null<world::World*>& world, const core::RoomBoundPosition& position)
      : CutsceneActor{world, position}
  {
  }

  CutsceneActor3(const gsl::not_null<world::World*>& world,
                 const gsl::not_null<const world::Room*>& room,
                 const loader::file::Item& item,
                 const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
      : CutsceneActor(world, room, item, animatedModel)
  {
  }
};

class CutsceneActor4 final : public CutsceneActor
{
public:
  CutsceneActor4(const gsl::not_null<world::World*>& world, const core::RoomBoundPosition& position)
      : CutsceneActor{world, position}
  {
  }

  CutsceneActor4(const gsl::not_null<world::World*>& world,
                 const gsl::not_null<const world::Room*>& room,
                 const loader::file::Item& item,
                 const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
      : CutsceneActor(world, room, item, animatedModel)
  {
  }

  void update() override;
};
} // namespace engine::objects
