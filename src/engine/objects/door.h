#pragma once

#include "modelobject.h"

namespace engine::objects
{
class Door final : public ModelObject
{
public:
  Door(const gsl::not_null<Engine*>& engine, const core::RoomBoundPosition& position)
      : ModelObject{engine, position}
  {
  }

  Door(const gsl::not_null<Engine*>& engine,
       const gsl::not_null<const loader::file::Room*>& room,
       const loader::file::Item& item,
       const gsl::not_null<const loader::file::SkeletalModelType*>& animatedModel);

  void update() override;

  void collide(CollisionInfo& collisionInfo) override;

  void serialize(const serialization::Serializer& ser) override;

private:
  struct Info
  {
    loader::file::Sector* wingsSector{nullptr};
    loader::file::Sector originalSector;
    loader::file::Box* wingsBox{nullptr};

    // ReSharper disable once CppMemberFunctionMayBeConst
    void open();

    // ReSharper disable once CppMemberFunctionMayBeConst
    void close();

    void init(const loader::file::Room& room, const core::TRVec& wingsPosition);

    void serialize(const serialization::Serializer& ser);
  };

  Info m_info;
  Info m_alternateInfo;
  Info m_target;
  Info m_alternateTarget;
  core::TRVec m_wingsPosition;
};
} // namespace engine::objects
