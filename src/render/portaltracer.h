#pragma once

#include "engine/world.h"
#include "loader/file/datatypes.h"

#include <boost/range/adaptor/transformed.hpp>

namespace render
{
struct PortalTracer
{
  struct CullBox
  {
    glm::vec2 min;
    glm::vec2 max;

    CullBox(const float minX, const float minY, const float maxX, const float maxY)
        : min{minX, minY}
        , max{maxX, maxY}
    {
    }
  };

  static std::unordered_set<const loader::file::Portal*> trace(const loader::file::Room& startRoom,
                                                               const engine::World& world)
  {
    std::vector<const loader::file::Room*> seenRooms;
    seenRooms.reserve(32);
    std::unordered_set<const loader::file::Portal*> waterSurfacePortals;
    traceRoom(startRoom,
              {-1, -1, 1, 1},
              world,
              seenRooms,
              startRoom.isWaterRoom(),
              waterSurfacePortals,
              startRoom.isWaterRoom());
    Expects(seenRooms.empty());
    return waterSurfacePortals;
  }

  static bool traceRoom(const loader::file::Room& room,
                        const CullBox& roomCullBox,
                        const engine::World& world,
                        std::vector<const loader::file::Room*>& seenRooms,
                        const bool inWater,
                        std::unordered_set<const loader::file::Portal*>& waterSurfacePortals,
                        const bool startFromWater);

  static std::optional<CullBox> narrowCullBox(const CullBox& parentCullBox,
                                              const loader::file::Portal& portal,
                                              const engine::CameraController& camera);
};
} // namespace render
