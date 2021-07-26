#pragma once

#include "aiagent.h"
#include "engine/ai/ai.h"

namespace engine::objects
{
class Gorilla final : public AIAgent
{
public:
  AIAGENT_DEFAULT_CONSTRUCTORS(Gorilla)

  void update() override;
  void serialize(const serialization::Serializer<world::World>& ser) override;

private:
  bool m_wantAttack = false;
  bool m_turnedRight = false;
  bool m_turnedLeft = false;
};
} // namespace engine::objects
