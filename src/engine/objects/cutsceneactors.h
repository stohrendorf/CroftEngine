#pragma once

#include "modelobject.h"

#include <gsl/gsl-lite.hpp>
#include <string>

namespace engine
{
struct Location;
}

namespace engine::world
{
class World;
struct Room;
struct SkeletalModelType;
} // namespace engine::world

namespace loader::file
{
struct Item;
}

namespace engine::objects
{
class CutsceneActor : public ModelObject
{
public:
  CutsceneActor(const gsl::not_null<world::World*>& world, const Location& location)
      : ModelObject{world, location}
  {
  }

  CutsceneActor(const std::string& name,
                const gsl::not_null<world::World*>& world,
                const gsl::not_null<const world::Room*>& room,
                const loader::file::Item& item,
                const gsl::not_null<const world::SkeletalModelType*>& animatedModel);

  void update() override;
};

class CutsceneActor1 final : public CutsceneActor
{
public:
  CutsceneActor1(const gsl::not_null<world::World*>& world, const Location& location)
      : CutsceneActor{world, location}
  {
  }

  CutsceneActor1(const std::string& name,
                 const gsl::not_null<world::World*>& world,
                 const gsl::not_null<const world::Room*>& room,
                 const loader::file::Item& item,
                 const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
      : CutsceneActor(name, world, room, item, animatedModel)
  {
  }
};

class CutsceneActor2 final : public CutsceneActor
{
public:
  CutsceneActor2(const gsl::not_null<world::World*>& world, const Location& location)
      : CutsceneActor{world, location}
  {
  }

  CutsceneActor2(const std::string& name,
                 const gsl::not_null<world::World*>& world,
                 const gsl::not_null<const world::Room*>& room,
                 const loader::file::Item& item,
                 const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
      : CutsceneActor(name, world, room, item, animatedModel)
  {
  }
};

class CutsceneActor3 final : public CutsceneActor
{
public:
  CutsceneActor3(const gsl::not_null<world::World*>& world, const Location& location)
      : CutsceneActor{world, location}
  {
  }

  CutsceneActor3(const std::string& name,
                 const gsl::not_null<world::World*>& world,
                 const gsl::not_null<const world::Room*>& room,
                 const loader::file::Item& item,
                 const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
      : CutsceneActor(name, world, room, item, animatedModel)
  {
  }
};

class CutsceneActor4 final : public CutsceneActor
{
public:
  CutsceneActor4(const gsl::not_null<world::World*>& world, const Location& location)
      : CutsceneActor{world, location}
  {
  }

  CutsceneActor4(const std::string& name,
                 const gsl::not_null<world::World*>& world,
                 const gsl::not_null<const world::Room*>& room,
                 const loader::file::Item& item,
                 const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
      : CutsceneActor(name, world, room, item, animatedModel)
  {
  }

  void update() override;
};
} // namespace engine::objects
