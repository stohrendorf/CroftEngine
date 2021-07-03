#pragma once

#include "engine/ai/ai.h"
#include "modelobject.h"

namespace engine::objects
{
class CentaurStatue final : public ModelObject
{
public:
  CentaurStatue(const gsl::not_null<world::World*>& world, const core::RoomBoundPosition& position)
      : ModelObject{world, position}
  {
  }

  CentaurStatue(const gsl::not_null<world::World*>& world,
                const gsl::not_null<const world::Room*>& room,
                loader::file::Item item,
                const gsl::not_null<const world::SkeletalModelType*>& animatedModel);

  void update() override;

  void serialize(const serialization::Serializer<world::World>& ser) override;

private:
  std::shared_ptr<AIAgent> m_childObject{nullptr};
};
} // namespace engine::objects
