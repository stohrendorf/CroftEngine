#pragma once

#include "modelobject.h"

namespace engine
{
struct Location;
}

namespace engine::world
{
class World;
struct Room;
} // namespace engine::world

namespace loader::file
{
struct Item;
}

namespace engine::world
{
struct SkeletalModelType;
}

namespace engine::objects
{
class Animating final : public ModelObject
{
public:
  MODELOBJECT_DEFAULT_CONSTRUCTORS(Animating, true)

  void update() override;
};
} // namespace engine::objects
