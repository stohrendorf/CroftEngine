#pragma once

#include "modelobject.h"

namespace engine::objects
{
class ThorHammerBlock final : public ModelObject
{
public:
  MODELOBJECT_DEFAULT_CONSTRUCTORS(ThorHammerBlock, true)

  void collide(CollisionInfo& info) override;
};

class ThorHammerHandle final : public ModelObject
{
public:
  ThorHammerHandle(const gsl::not_null<world::World*>& world, const Location& location);

  ThorHammerHandle(const std::string& name,
                   const gsl::not_null<world::World*>& world,
                   const gsl::not_null<const world::Room*>& room,
                   const loader::file::Item& item,
                   const gsl::not_null<const world::SkeletalModelType*>& animatedModel);

  void update() override;

  void collide(CollisionInfo& info) override;

  void serialize(const serialization::Serializer<world::World>& ser) override;

private:
  const std::shared_ptr<ThorHammerBlock> m_block;
};
} // namespace engine::objects
