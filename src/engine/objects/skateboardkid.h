#pragma once

#include "aiagent.h"
#include "engine/ai/ai.h"

namespace engine::objects
{
class SkateboardKid final : public AIAgent
{
public:
  SkateboardKid(const gsl::not_null<world::World*>& world, const Location& location);

  SkateboardKid(const std::string& name,
                const gsl::not_null<world::World*>& world,
                const gsl::not_null<const world::Room*>& room,
                const loader::file::Item& item,
                const gsl::not_null<const world::SkeletalModelType*>& animatedModel);

  void update() override;

  void serialize(const serialization::Serializer<world::World>& ser) override;

private:
  bool m_triedShoot = false;
  std::shared_ptr<SkeletalModelNode> m_skateboard;
};
} // namespace engine::objects
