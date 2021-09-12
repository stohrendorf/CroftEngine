#pragma once

#include "aiagent.h"

namespace engine::objects
{
class Cowboy final : public AIAgent
{
public:
  AIAGENT_DEFAULT_CONSTRUCTORS(Cowboy)

  void update() override;

  void serialize(const serialization::Serializer<world::World>& ser) override;

private:
  core::Frame m_aimTime = 0_frame;
};
} // namespace engine::objects
