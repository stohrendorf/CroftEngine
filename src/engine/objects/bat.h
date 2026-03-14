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
class Bat final : public AIAgent
{
public:
  AIAGENT_DEFAULT_CONSTRUCTORS(Bat)

  void updateLogic() override;
};
} // namespace engine::objects
