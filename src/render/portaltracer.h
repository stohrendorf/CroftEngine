#pragma once

#include "world/portal.h"
#include "world/room.h"
#include "world/core/frustum.h"

namespace render
{
struct PortalTracer
{
    std::vector<const world::Portal*> portals;

    bool checkVisibility(const world::Portal* portal, const irr::core::vector3df& cameraPosition, const world::core::Frustum& frustum)
    {
        if(!portal->destination || !portal->destination->isActive())
        {
            return false; // no relevant destination
        }

        if(portal->normal.dotProduct(portal->center - cameraPosition) >= 0)
        {
            return false; // wrong orientation (normals point usually into the source room)
        }

        if(!frustum.isVisible(*portal))
        {
            return false; // not in frustum
        }

        if(portals.empty())
        {
            // no need to check the path
            portals.emplace_back(portal);
            return true;
        }

        BOOST_ASSERT(portal->source == portals.back()->destination);

        // Now the heavy work: trace the portal path and test if we can see the target portal through any other portal.
        for(const world::Portal* testPortal : portals)
        {
            if(!testIntersection(cameraPosition, *portal, *testPortal))
            {
                return false;
            }
        }

        portals.emplace_back(portal);
        return true;
    }

    world::Room* getLastDestinationRoom() const
    {
        return getLastPortal()->destination;
    }

    const world::Portal* getLastPortal() const
    {
        BOOST_ASSERT(!portals.empty());
        return portals.back();
    }

private:
    static bool testIntersectionFwd(const irr::core::vector3df& camPos, const world::Portal& a, const world::Portal& b)
    {
        irr::core::triangle3df t{b.vertices[0], b.vertices[1], b.vertices[3]};
        irr::core::vector3df dummy;
        // test if the ray from the camera to a's vertices crosses b's triangles
        for(const irr::core::vector3df& v : a.vertices)
        {
            if(t.getIntersectionWithLine(camPos, v-camPos, dummy))
                return true;
        }

        return false;
    }

    static bool testIntersection(const irr::core::vector3df& camPos, const world::Portal& a, const world::Portal& b)
    {
        if(testIntersectionFwd(camPos, a, b))
            return true;
        if(testIntersectionFwd(camPos, b, a))
            return true;
        return false;
    }
};
}
