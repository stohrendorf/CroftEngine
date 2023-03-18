#include "portaltracer.h"

#include "engine/cameracontroller.h"
#include "engine/world/room.h"
#include "engine/world/world.h"
#include "scene/camera.h"
#include "scene/node.h"

#include <algorithm>
#include <array>
#include <boost/assert.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/range/adaptor/argument_fwd.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <cstddef>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <gsl/gsl-lite.hpp>
#include <limits>
#include <memory>

namespace render
{
std::optional<PortalTracer::CullBox> PortalTracer::narrowCullBox(const PortalTracer::CullBox& parentCullBox,
                                                                 const engine::world::Portal& portal,
                                                                 const engine::CameraController& camera)
{
  // NOLINTNEXTLINE(hicpp-signed-bitwise)
  static constexpr auto Eps = 1.0f / (1 << 14);

  if(dot(portal.normal, portal.vertices[0] - camera.getPosition()) >= 0)
  {
    return std::nullopt; // wrong orientation (normals must face the camera)
  }

  const auto toView = [&camera](const glm::vec3& v)
  {
    const auto tmp = camera.getCamera()->getViewMatrix() * glm::vec4{v, 1.0f};
    BOOST_ASSERT(tmp.w > std::numeric_limits<float>::epsilon());
    return glm::vec3{tmp} / tmp.w;
  };

  const auto toScreen = [&camera](const glm::vec3& v) -> std::optional<glm::vec2>
  {
    const auto tmp = camera.getCamera()->getProjectionMatrix() * glm::vec4{v, 1.0f};
    if(tmp.w <= std::numeric_limits<float>::epsilon())
      return std::nullopt;

    const auto projected = glm::vec2{tmp} / tmp.w;

    static constexpr auto Margin = 50.0f;
    if(std::abs(v.z) >= Margin)
      return projected;

    return glm::vec2{
      projected.x > 0 ? 1 : -1,
      projected.y > 0 ? 1 : -1,
    };
  };

  // 1. determine the screen cull box of the current portal
  // 2. intersect it with the parent's bbox
  CullBox portalCullBox{{1, -1}, {1, -1}};
  size_t behindCamera = 0, tooFar = 0;
  for(const auto& camSpace : portal.vertices | boost::adaptors::transformed(toView))
  {
    if(-camSpace.z < 0)
    {
      ++behindCamera;
      continue;
    }
    else if(-camSpace.z >= camera.getCamera()->getFarPlane())
    {
      ++tooFar;
    }

    const auto screen = toScreen(camSpace);
    if(!screen.has_value())
    {
      // if calculation fails because of numerical issues, just don't do anything
      return parentCullBox;
    }

    portalCullBox.x = portalCullBox.x.union_(screen->x);
    portalCullBox.y = portalCullBox.y.union_(screen->y);

    // the first vertex must set the cull box to a valid state
    BOOST_ASSERT(portalCullBox.x.isValid());
    BOOST_ASSERT(portalCullBox.y.isValid());
  }

  if(behindCamera == portal.vertices.size() || tooFar == portal.vertices.size())
  {
    return std::nullopt;
  }

  if(behindCamera > 0)
  {
    glm::vec3 prev = toView(portal.vertices.back());
    for(const auto& current : portal.vertices | boost::adaptors::transformed(toView))
    {
      const auto crossing
        = (-prev.z <= camera.getCamera()->getNearPlane()) != (-current.z <= camera.getCamera()->getNearPlane());

      if(!crossing)
      {
        prev = current;
        continue;
      }

      // edge crosses the camera plane, max out the bounds
      if((prev.x < 0) && (current.x < 0))
      {
        portalCullBox.x.min = -1;
      }
      else if((prev.x > 0) && (current.x > 0))
      {
        portalCullBox.x.max = 1;
      }
      else
      {
        portalCullBox.x.min = -1;
        portalCullBox.x.max = 1;
      }

      if((prev.y < 0) && (current.y < 0))
      {
        portalCullBox.y.min = -1;
      }
      else if((prev.y > 0) && (current.y > 0))
      {
        portalCullBox.y.max = 1;
      }
      else
      {
        portalCullBox.y.min = -1;
        portalCullBox.y.max = 1;
      }

      prev = current;
    }
  }

  portalCullBox.x = parentCullBox.x.intersect(portalCullBox.x);
  portalCullBox.y = parentCullBox.y.intersect(portalCullBox.y);

  if(portalCullBox.x.size() <= Eps || portalCullBox.y.size() <= Eps)
  {
    return std::nullopt;
  }

  return portalCullBox;
}

// NOLINTNEXTLINE(misc-no-recursion)
bool PortalTracer::traceRoom(const engine::world::Room& room,
                             const PortalTracer::CullBox& roomCullBox,
                             const engine::world::World& world,
                             std::vector<const engine::world::Room*>& seenRooms,
                             const bool inWater,
                             std::unordered_set<const engine::world::Portal*>& waterSurfacePortals,
                             const bool startFromWater,
                             int depth)
{
  if(std::find(seenRooms.rbegin(), seenRooms.rend(), &room) != seenRooms.rend())
    return false;
  seenRooms.emplace_back(&room);

  room.node->setVisible(true);
  room.node->setRenderOrder(-depth);
  for(const auto& portal : room.portals)
  {
    if(const auto narrowedCullBox = narrowCullBox(roomCullBox, portal, world.getCameraController()))
    {
      const auto& childRoom = portal.adjoiningRoom;
      const bool waterChanged = inWater == startFromWater && childRoom->isWaterRoom != startFromWater;
      childRoom->node->addScissor(narrowedCullBox->x, narrowedCullBox->y);
      if(traceRoom(*childRoom,
                   *narrowedCullBox,
                   world,
                   seenRooms,
                   inWater || childRoom->isWaterRoom,
                   waterSurfacePortals,
                   startFromWater,
                   depth + 1)
         && waterChanged)
      {
        waterSurfacePortals.emplace(&portal);
      }
    }
  }
  seenRooms.pop_back();
  return true;
}

std::unordered_set<const engine::world::Portal*> PortalTracer::trace(const engine::world::Room& startRoom,
                                                                     const engine::world::World& world)
{
  std::vector<const engine::world::Room*> seenRooms;
  seenRooms.reserve(32);
  std::unordered_set<const engine::world::Portal*> waterSurfacePortals;
  traceRoom(startRoom,
            {{-1, 1}, {-1, 1}},
            world,
            seenRooms,
            startRoom.isWaterRoom,
            waterSurfacePortals,
            startRoom.isWaterRoom,
            1);
  gsl_Assert(seenRooms.empty());
  return waterSurfacePortals;
}
} // namespace render
