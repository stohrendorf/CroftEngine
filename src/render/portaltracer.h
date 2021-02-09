#pragma once

#include <glm/glm.hpp>
#include <gsl-lite.hpp>
#include <unordered_set>
#include <vector>

namespace engine
{
class CameraController;
class World;
} // namespace engine

namespace loader::file
{
struct Portal;
struct Room;
} // namespace loader::file

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
                                                               const engine::World& world);

  static bool traceRoom(const loader::file::Room& room,
                        const CullBox& roomCullBox,
                        const engine::World& world,
                        std::vector<const loader::file::Room*>& seenRooms,
                        bool inWater,
                        std::unordered_set<const loader::file::Portal*>& waterSurfacePortals,
                        bool startFromWater);

  static std::optional<CullBox> narrowCullBox(const CullBox& parentCullBox,
                                              const loader::file::Portal& portal,
                                              const engine::CameraController& camera);
};
} // namespace render
