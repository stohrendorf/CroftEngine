#pragma once

#include "modelobject.h"

namespace engine::objects
{
class ThorHammerBlock final : public ModelObject
{
public:
  ThorHammerBlock(const gsl::not_null<Engine*>& engine, const core::RoomBoundPosition& position)
      : ModelObject{engine, position}
  {
  }

  ThorHammerBlock(const gsl::not_null<Engine*>& engine,
                  const gsl::not_null<const loader::file::Room*>& room,
                  const loader::file::Item& item,
                  const gsl::not_null<const loader::file::SkeletalModelType*>& animatedModel)
      : ModelObject{engine, room, item, true, animatedModel}
  {
  }

  void collide(CollisionInfo& info) override;
};

class ThorHammerHandle final : public ModelObject
{
public:
  ThorHammerHandle(const gsl::not_null<Engine*>& engine, const core::RoomBoundPosition& position)
      : ModelObject{engine, position}
  {
  }

  ThorHammerHandle(const gsl::not_null<Engine*>& engine,
                   const gsl::not_null<const loader::file::Room*>& room,
                   const loader::file::Item& item,
                   const gsl::not_null<const loader::file::SkeletalModelType*>& animatedModel);

  void update() override;

  void collide(CollisionInfo& info) override;

private:
  const std::shared_ptr<ThorHammerBlock> m_block;
};
} // namespace engine
