#pragma once

#include "core/magic.h"
#include "core/units.h"
#include "engine/floordata/types.h"
#include "serialization/serialization_fwd.h"

#include <optional>
#include <vector>

namespace loader::file
{
struct Sector;
} // namespace loader::file

namespace engine::world
{
class World;
struct Box;
struct Room;

struct Sector
{
  const engine::floordata::FloorDataValue* floorData = nullptr;
  Room* boundaryRoom = nullptr;

  const Box* box = nullptr;
  Room* roomBelow = nullptr;
  core::Length floorHeight = core::InvalidHeight; // value is sometimes considered exclusive, sometimes not
  Room* roomAbove = nullptr;
  core::Length ceilingHeight = core::InvalidHeight; // value is sometimes considered exclusive, sometimes not

  Sector() = default;
  Sector(const loader::file::Sector& src,
         std::vector<Room>& rooms,
         const std::vector<Box>& boxes,
         const engine::floordata::FloorData& newFloorData);

  void connect(std::vector<Room>& rooms);

  void serialize(const serialization::Serializer<World>& ser);

private:
  std::optional<size_t> m_roomIndexBelow;
  std::optional<size_t> m_roomIndexAbove;
};
} // namespace engine::world
