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
class AtlanteanLava final : public ModelObject
{
public:
  MODELOBJECT_DEFAULT_CONSTRUCTORS(AtlanteanLava, true, false)

  void update() override;
};
} // namespace engine::objects
