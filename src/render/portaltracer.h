#pragma once

#include <glm/vec2.hpp>
#include <optional>
#include <unordered_set>
#include <vector>

namespace engine
{
class CameraController;
}

namespace engine::world
{
class World;
struct Room;
struct Portal;
} // namespace engine::world

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

  static std::unordered_set<const engine::world::Portal*> trace(const engine::world::Room& startRoom,
                                                                const engine::world::World& world);

  static bool traceRoom(const engine::world::Room& room,
                        const CullBox& roomCullBox,
                        const engine::world::World& world,
                        std::vector<const engine::world::Room*>& seenRooms,
                        bool inWater,
                        std::unordered_set<const engine::world::Portal*>& waterSurfacePortals,
                        bool startFromWater,
                        int depth);

  static std::optional<CullBox> narrowCullBox(const CullBox& parentCullBox,
                                              const engine::world::Portal& portal,
                                              const engine::CameraController& camera);
};
} // namespace render
