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
                        const bool startFromWater)
  {
    if(std::find(seenRooms.rbegin(), seenRooms.rend(), &room) != seenRooms.rend())
      return false;
    seenRooms.emplace_back(&room);

    room.node->setVisible(true);
    for(const auto& portal : room.portals)
    {
      if(const auto narrowedCullBox = narrowCullBox(roomCullBox, portal, world.getCameraController()))
      {
        const auto& childRoom = world.getRooms().at(portal.adjoining_room.get());
        const bool waterChanged = inWater == startFromWater && childRoom.isWaterRoom() != startFromWater;
        if(traceRoom(childRoom,
                     *narrowedCullBox,
                     world,
                     seenRooms,
                     inWater || childRoom.isWaterRoom(),
                     waterSurfacePortals,
                     startFromWater)
           && waterChanged)
        {
          waterSurfacePortals.emplace(&portal);
        }
      }
    }
    seenRooms.pop_back();
    return true;
  }

  static std::optional<CullBox> narrowCullBox(const CullBox& parentCullBox,
                                              const loader::file::Portal& portal,
                                              const engine::CameraController& camera)
  {
    static constexpr auto Eps = 1.0f / (1 << 14);

    if(dot(portal.normal.toRenderSystem(), portal.vertices[0].toRenderSystem() - camera.getPosition()) >= 0)
    {
      return std::nullopt; // wrong orientation (normals must face the camera)
    }

    const auto toView = [&camera](const core::TRVec& v) {
      const auto tmp = camera.getCamera()->getViewMatrix() * glm::vec4{v.toRenderSystem(), 1.0f};
      BOOST_ASSERT(tmp.w > std::numeric_limits<float>::epsilon());
      return glm::vec3{tmp} / tmp.w;
    };

    const auto toScreen = [&camera](const glm::vec3& v) {
      const auto tmp = camera.getCamera()->getProjectionMatrix() * glm::vec4{v, 1.0f};
      BOOST_ASSERT(tmp.w > std::numeric_limits<float>::epsilon());
      return glm::vec2{tmp} / tmp.w;
    };

    // 1. determine the screen cull box of the current portal
    // 2. intersect it with the parent's bbox
    CullBox portalCullBox{1, 1, -1, -1};
    size_t behindCamera = 0, tooFar = 0;
    for(const auto& camSpace : portal.vertices | boost::adaptors::transformed(toView))
    {
      if(-camSpace.z <= camera.getCamera()->getNearPlane())
      {
        ++behindCamera;
        continue;
      }
      else if(-camSpace.z >= camera.getCamera()->getFarPlane())
      {
        ++tooFar;
        continue;
      }

      const auto screen = toScreen(camSpace);

      portalCullBox.min.x = std::min(portalCullBox.min.x, screen.x);
      portalCullBox.min.y = std::min(portalCullBox.min.y, screen.y);
      portalCullBox.max.x = std::max(portalCullBox.max.x, screen.x);
      portalCullBox.max.y = std::max(portalCullBox.max.y, screen.y);

      // the first vertex must set the cull box to a valid state
      BOOST_ASSERT(portalCullBox.min.x <= portalCullBox.max.x);
      BOOST_ASSERT(portalCullBox.min.y <= portalCullBox.max.y);
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
          portalCullBox.min.x = -1;
        }
        else if((prev.x > 0) && (current.x > 0))
        {
          portalCullBox.max.x = 1;
        }
        else
        {
          portalCullBox.min.x = -1;
          portalCullBox.max.x = 1;
        }

        if((prev.y < 0) && (current.y < 0))
        {
          portalCullBox.min.y = -1;
        }
        else if((prev.y > 0) && (current.y > 0))
        {
          portalCullBox.max.y = 1;
        }
        else
        {
          portalCullBox.min.y = -1;
          portalCullBox.max.y = 1;
        }

        prev = current;
      }
    }

    portalCullBox.min.x = std::max(parentCullBox.min.x, portalCullBox.min.x);
    portalCullBox.min.y = std::max(parentCullBox.min.y, portalCullBox.min.y);
    portalCullBox.max.x = std::min(parentCullBox.max.x, portalCullBox.max.x);
    portalCullBox.max.y = std::min(parentCullBox.max.y, portalCullBox.max.y);

    if(portalCullBox.min.x + Eps >= portalCullBox.max.x || portalCullBox.min.y + Eps >= portalCullBox.max.y)
    {
      return std::nullopt;
    }

    return portalCullBox;
  }
};
} // namespace render
