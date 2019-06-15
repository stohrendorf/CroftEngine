#pragma once

#include "loader/file/datatypes.h"
#include "engine/engine.h"

namespace render
{
struct PortalTracer
{
    struct BoundingBox
    {
        glm::vec2 min;
        glm::vec2 max;

        BoundingBox(const float minX, const float minY,
                    const float maxX, const float maxY)
            : min{ minX, minY }
              , max{ maxX, maxY }
        {
        }
    };

    static std::unordered_set<const loader::file::Portal*>
    trace(const loader::file::Room& startRoom, const engine::Engine& engine)
    {
        std::vector<const loader::file::Room*> seenRooms;
        seenRooms.reserve( 32 );
        std::unordered_set<const loader::file::Portal*> waterEntryPortals;
        traceRoom( startRoom, { -1, -1, 1, 1 }, engine, seenRooms, startRoom.isWaterRoom(), waterEntryPortals,
                   startRoom.isWaterRoom() );
        Expects( seenRooms.empty() );
        return waterEntryPortals;
    }

    static bool traceRoom(const loader::file::Room& room, const BoundingBox& roomBBox, const engine::Engine& engine,
                          std::vector<const loader::file::Room*>& seenRooms, const bool inWater,
                          std::unordered_set<const loader::file::Portal*>& waterEntryPortals, const bool startFromWater)
    {
        if( std::find( seenRooms.rbegin(), seenRooms.rend(), &room ) != seenRooms.rend() )
            return false;
        seenRooms.emplace_back( &room );

        room.node->setVisible( true );
        for( const auto& portal : room.portals )
        {
            if( const auto narrowedBounds = narrowPortal( room, roomBBox, portal, engine.getCameraController() ) )
            {
                const auto& childRoom = engine.getRooms().at( portal.adjoining_room.get() );
                const bool waterChanged = inWater == startFromWater && childRoom.isWaterRoom() != startFromWater;
                if( traceRoom( childRoom, *narrowedBounds, engine, seenRooms, inWater || childRoom.isWaterRoom(),
                               waterEntryPortals, startFromWater ) && waterChanged )
                {
                    waterEntryPortals.emplace( &portal );
                }
            }
        }
        seenRooms.pop_back();
        return true;
    }

    static boost::optional<BoundingBox> narrowPortal(
        const loader::file::Room& parentRoom,
        const BoundingBox& parentBBox,
        const loader::file::Portal& portal,
        const engine::CameraController& camera)
    {
        static const constexpr auto Eps = 1.0f / (1 << 14);

        const auto portalToCam = glm::vec3{ camera.getPosition() - portal.vertices[0].toRenderSystem() };
        if( dot( portal.normal.toRenderSystem(), portalToCam ) <= Eps )
        {
            return boost::none; // wrong orientation (normals must face the camera)
        }

        // 1. determine the screen bbox of the current portal
        // 2. intersect it with the parent's bbox
        BoundingBox portalBB{ 1, 1, -1, -1 };
        size_t behindCamera = 0, tooFar = 0;
        for( const auto& vertex : portal.vertices )
        {
            glm::vec3 camSpace = glm::vec3{
                camera.getCamera()->getViewMatrix() * glm::vec4{ vertex.toRenderSystem(), 1.0f }
            };
            if( -camSpace.z <= camera.getCamera()->getNearPlane() )
            {
                ++behindCamera;
                continue;
            }
            else if( -camSpace.z >= camera.getCamera()->getFarPlane() )
            {
                ++tooFar;
                continue;
            }

            auto screen = camera.getCamera()->getProjectionMatrix() * glm::vec4{ camSpace, 1.0f };
            screen /= screen.w;

            portalBB.min.x = std::min( portalBB.min.x, screen.x );
            portalBB.min.y = std::min( portalBB.min.y, screen.y );
            portalBB.max.x = std::max( portalBB.max.x, screen.x );
            portalBB.max.y = std::max( portalBB.max.y, screen.y );

            // the first vertex must set the boundingbox to a valid state
            BOOST_ASSERT( portalBB.min.x <= portalBB.max.x );
            BOOST_ASSERT( portalBB.min.y <= portalBB.max.y );
        }

        if( behindCamera == portal.vertices.size() || tooFar == portal.vertices.size() )
        {
            return boost::none;
        }

        if( behindCamera > 0 )
        {
            glm::vec3 prev{ camera.getCamera()->getViewMatrix()
                                * glm::vec4{ portal.vertices.back().toRenderSystem(), 1.0f }
            };
            for( const auto& currentPV : portal.vertices )
            {
                const glm::vec3 current{
                    camera.getCamera()->getViewMatrix() * glm::vec4{ currentPV.toRenderSystem(), 1.0f }
                };
                const auto crossing = (-prev.z <= camera.getCamera()->getNearPlane())
                    != (-current.z <= camera.getCamera()->getNearPlane());
                prev = current;

                if( !crossing )
                {
                    continue;
                }

                // edge crosses the camera plane, max out the bounds
                if( (prev.x < 0) && (current.x < 0) )
                {
                    portalBB.min.x = -1;
                }
                else if( (prev.x > 0) && (current.x > 0) )
                {
                    portalBB.max.x = 1;
                }
                else
                {
                    portalBB.min.x = -1;
                    portalBB.max.x = 1;
                }

                if( (prev.y < 0) && (current.y < 0) )
                {
                    portalBB.min.y = -1;
                }
                else if( (prev.y > 0) && (current.y > 0) )
                {
                    portalBB.max.y = 1;
                }
                else
                {
                    portalBB.min.y = -1;
                    portalBB.max.y = 1;
                }
            }
        }

        portalBB.min.x = std::max( parentBBox.min.x, portalBB.min.x );
        portalBB.min.y = std::max( parentBBox.min.y, portalBB.min.y );
        portalBB.max.x = std::min( parentBBox.max.x, portalBB.max.x );
        portalBB.max.y = std::min( parentBBox.max.y, portalBB.max.y );

        if( portalBB.min.x + Eps >= portalBB.max.x || portalBB.min.y + Eps >= portalBB.max.y )
        {
            return boost::none;
        }

        return portalBB;
    }
};
}
