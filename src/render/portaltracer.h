#pragma once

#include "loader/datatypes.h"

namespace render
{
struct PortalTracer
{
    std::vector<const loader::Portal*> portals;

    bool checkVisibility(const loader::Portal* portal, const irr::core::vector3df& cameraPosition, const irr::scene::SViewFrustum& frustum)
    {
        if(portal->normal.dotProduct(portal->vertices[0] - cameraPosition) >= 0)
        {
            return false; // wrong orientation (normals point usually into the source room)
        }

        if(!isVisible(frustum, *portal))
        {
           return false; // not in frustum
        }

        if(portals.empty())
        {
            // no need to check the path
            portals.emplace_back(portal);
            return true;
        }

        // Now the heavy work: trace the portal path and test if we can see the target portal through any other portal.
        for(const loader::Portal* testPortal : portals)
        {
            if(!testIntersection(cameraPosition, *portal, *testPortal))
            {
                return false;
            }
        }

        portals.emplace_back(portal);
        return true;
    }

    uint16_t getLastDestinationRoom() const
    {
        return getLastPortal()->adjoining_room;
    }

    const loader::Portal* getLastPortal() const
    {
        BOOST_ASSERT(!portals.empty());
        return portals.back();
    }

private:
    static bool isVisible(const irr::scene::SViewFrustum& frustum, const loader::Portal& portal)
    {
        if(intersects(frustum, portal.vertices[0], portal.vertices[1])) return true;
        if(intersects(frustum, portal.vertices[1], portal.vertices[2])) return true;
        if(intersects(frustum, portal.vertices[2], portal.vertices[3])) return true;
        if(intersects(frustum, portal.vertices[3], portal.vertices[0])) return true;
        if(intersects(frustum, portal.vertices[0], portal.vertices[2])) return true;
        if(intersects(frustum, portal.vertices[1], portal.vertices[3])) return true;
    
        return false;
    }
    
    static bool intersects(const irr::scene::SViewFrustum& frustum, const irr::core::vector3df& a, const irr::core::vector3df& b)
    {
        uint32_t aOutside = 0, bOutside = 0;
        for(size_t i = 0; i < 6; ++i)
        {
            const auto aDist = frustum.planes[i].classifyPointRelation(a);
            if(aDist != irr::core::ISREL3D_BACK)
                aOutside |= 1 << i;
    
            const auto bDist = frustum.planes[i].classifyPointRelation(b);
            if(bDist != irr::core::ISREL3D_BACK)
                bOutside |= 1 << i;
        }
    
        if(aOutside == 0 || bOutside == 0)
            return true; // a or b or both are inside the frustum
    
        // if both are outside different planes, chances are high that they cross the frustum;
        // chances are low for false positives unless there are very very long edges compared to the frustum
        return aOutside != bOutside;
    }
    
    static bool testIntersectionFwd(const irr::core::vector3df& camPos, const loader::Portal& a, const loader::Portal& b)
    {
        irr::core::triangle3df tri1{b.vertices[0], b.vertices[1], b.vertices[2]};
        irr::core::triangle3df tri2{b.vertices[0], b.vertices[2], b.vertices[3]};
        irr::core::vector3df dummy;
        // test if the ray from the camera to a's vertices crosses b's triangles
        for(const irr::core::vector3df& v : a.vertices)
        {
            if(tri1.getIntersectionWithLine(camPos, v-camPos, dummy))
                return true;
            if(tri2.getIntersectionWithLine(camPos, v-camPos, dummy))
                return true;
        }

        return false;
    }

    static bool testIntersection(const irr::core::vector3df& camPos, const loader::Portal& a, const loader::Portal& b)
    {
        if(testIntersectionFwd(camPos, a, b))
            return true;
        if(testIntersectionFwd(camPos, b, a))
            return true;
        return false;
    }
};
}
