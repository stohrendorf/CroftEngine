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
struct SkeletalModelType;
} // namespace engine::world

namespace loader::file
{
struct Item;
}

namespace engine::objects
{
class DartGun final : public ModelObject
{
public:
  MODELOBJECT_DEFAULT_CONSTRUCTORS(DartGun, true, false)

  void update() override;
};
} // namespace engine::objects
