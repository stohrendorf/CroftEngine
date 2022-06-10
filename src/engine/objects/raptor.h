#pragma once

#include "aiagent.h"

namespace engine::world
{
class World;
struct Room;
struct SkeletalModelType;
} // namespace engine::world

namespace loader::file
{
struct Item;
}

namespace engine::objects
{
class Raptor final : public AIAgent
{
public:
  AIAGENT_DEFAULT_CONSTRUCTORS(Raptor)

  void update() override;
};
} // namespace engine::objects
