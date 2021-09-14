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
class Rat final : public AIAgent
{
public:
  AIAGENT_DEFAULT_CONSTRUCTORS(Rat)

  void update() override;
};
} // namespace engine::objects
