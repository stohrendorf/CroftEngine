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
class Crocodile final : public AIAgent
{
public:
  AIAGENT_DEFAULT_CONSTRUCTORS(Crocodile)

  void update() override;

private:
  void updateInWater();
  void updateOnLand();
};
} // namespace engine::objects
