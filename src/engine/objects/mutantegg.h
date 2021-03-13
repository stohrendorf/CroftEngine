#pragma once

#include "engine/ai/ai.h"
#include "modelobject.h"

namespace engine::objects
{
extern bool shatterModel(ModelObject& object, const std::bitset<32>& meshMask, const core::Length& damageRadius);

class MutantEgg final : public ModelObject
{
public:
  MutantEgg(const gsl::not_null<world::World*>& world, const core::RoomBoundPosition& position)
      : ModelObject{world, position}
  {
  }

  MutantEgg(const gsl::not_null<world::World*>& world,
            const gsl::not_null<const loader::file::Room*>& room,
            loader::file::Item item,
            const gsl::not_null<const loader::file::SkeletalModelType*>& animatedModel);

  void update() override;

  void collide(CollisionInfo& info) override;

  void serialize(const serialization::Serializer<world::World>& ser) override;

private:
  std::shared_ptr<Object> m_childObject{nullptr};
};
} // namespace engine::objects
