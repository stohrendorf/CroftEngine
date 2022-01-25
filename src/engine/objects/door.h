#pragma once

#include "core/vec.h"
#include "engine/world/sector.h"
#include "modelobject.h"
#include "serialization/serialization_fwd.h"

#include <gsl/gsl-lite.hpp>
#include <string>

namespace engine
{
struct CollisionInfo;
struct Location;
} // namespace engine

namespace engine::world
{
class World;
struct Box;
struct Room;
struct SkeletalModelType;
} // namespace engine::world

namespace loader::file
{
struct Item;
}

namespace engine::objects
{
class Door final : public ModelObject
{
public:
  Door(const gsl::not_null<world::World*>& world, const Location& location)
      : ModelObject{world, location, false}
  {
  }

  Door(const std::string& name,
       const gsl::not_null<world::World*>& world,
       const gsl::not_null<const world::Room*>& room,
       const loader::file::Item& item,
       const gsl::not_null<const world::SkeletalModelType*>& animatedModel);

  void update() override;

  void collide(CollisionInfo& collisionInfo) override;

  void serialize(const serialization::Serializer<world::World>& ser) override;

private:
  struct Info
  {
    world::Sector* wingsSector{nullptr};
    world::Sector originalSector;
    world::Box* wingsBox{nullptr};

    void open();
    void close();
    void init(const world::Room& room, const core::TRVec& position);
    void serialize(const serialization::Serializer<world::World>& ser);
  };

  Info m_info;
  Info m_alternateInfo;
  Info m_target;
  Info m_alternateTarget;
  core::TRVec m_wingsPosition;
};
} // namespace engine::objects
