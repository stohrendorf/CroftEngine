#pragma once

#include "aiagent.h"

namespace engine::objects
{
class Bear final : public AIAgent
{
public:
  AIAGENT_DEFAULT_CONSTRUCTORS(Bear)

  void update() override;

  void serialize(const serialization::Serializer<world::World>& ser) override;

private:
  bool m_hurt = false;
};
} // namespace engine::objects
