#pragma once

#include "core/id.h"
#include "core/magic.h"
#include "core/units.h"
#include "engine/floordata/types.h"
#include "serialization/serialization_fwd.h"

#include <optional>

namespace loader::file
{
struct Room;
struct Sector;
} // namespace loader::file

namespace engine::world
{
class World;
struct Box;
struct Room;

struct Sector
{
  /**
     * @brief Index into FloorData[]
     *
     * @note If this is 0, no floor data is attached to this sector.
     */
  const engine::floordata::FloorDataValue* floorData = nullptr;
  Room* portalTarget = nullptr;

  const Box* box = nullptr;
  Room* roomBelow = nullptr;
  core::Length floorHeight = -core::HeightLimit;
  Room* roomAbove = nullptr;
  core::Length ceilingHeight = -core::HeightLimit;

  Sector() = default;
  Sector(const loader::file::Sector& src,
         std::vector<Room>& rooms,
         const std::vector<Box>& boxes,
         const engine::floordata::FloorData& newFloorData);

  void connect(std::vector<Room>& rooms);

  void serialize(const serialization::Serializer<engine::world::World>& ser);

private:
  std::optional<size_t> m_roomIndexBelow;
  std::optional<size_t> m_roomIndexAbove;
};
} // namespace engine::world
