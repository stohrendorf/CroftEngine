#pragma once

#include "modelobject.h"

namespace engine::objects
{
class Door final : public ModelObject
{
public:
  Door(const gsl::not_null<world::World*>& world, const core::RoomBoundPosition& position)
      : ModelObject{world, position}
  {
  }

  Door(const gsl::not_null<world::World*>& world,
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
    void init(const world::Room& room, const core::TRVec& wingsPosition);
    void serialize(const serialization::Serializer<world::World>& ser);
  };

  Info m_info;
  Info m_alternateInfo;
  Info m_target;
  Info m_alternateTarget;
  core::TRVec m_wingsPosition;
};
} // namespace engine::objects
