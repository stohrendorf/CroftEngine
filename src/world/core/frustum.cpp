#include "frustum.h"

#include "engine/engine.h"
#include "orientedboundingbox.h"
#include "polygon.h"
#include "util/vmath.h"
#include "world/camera.h"
#include "world/portal.h"

namespace world
{
namespace core
{
bool Frustum::intersects(const irr::core::vector3df& a, const irr::core::vector3df& b) const
{
    uint32_t aOutside = 0, bOutside = 0;
    for(size_t i = 0; i < m_planes.size(); ++i)
    {
        const auto aDist = m_planes[i].distance(a);
        if(aDist < 0)
            aOutside |= 1 << i;

        const auto bDist = m_planes[i].distance(b);
        if(bDist < 0)
            bOutside |= 1 << i;
    }

    if(aOutside == 0 || bOutside == 0)
        return true; // a or b or both are inside the frustum

    // if both are outside different planes, chances are high that they cross the frustum;
    // chances are low for false positives unless there are very very long edges compared to the frustum
    return aOutside != bOutside;
}

bool Frustum::isVisible(const Portal& portal) const
{
    if(!portal.destination)
        return false;

    if(intersects(portal.vertices[0], portal.vertices[1])) return true;
    if(intersects(portal.vertices[1], portal.vertices[2])) return true;
    if(intersects(portal.vertices[2], portal.vertices[3])) return true;
    if(intersects(portal.vertices[3], portal.vertices[0])) return true;
    if(intersects(portal.vertices[0], portal.vertices[2])) return true;
    if(intersects(portal.vertices[1], portal.vertices[3])) return true;

    return false;
}

bool Frustum::isVisible(const Polygon& polygon, const Camera& cam) const
{
    if(!polygon.isDoubleSided && polygon.plane.normal.dotProduct(cam.getPosition()) < 0.0)
    {
        return false;
    }

    // iterate through all the planes of this frustum
    for(const util::Plane& plane : m_planes)
    {
        for(const irr::video::S3DVertex& vec : polygon.vertices)
        {
            if(plane.distance(vec.Pos) < 0)
                return false;
        }
    }

    return true;
}

bool Frustum::isVisible(const std::vector<irr::core::vector3df>& vertices) const
{
    // iterate through all the planes of this frustum
    for(const util::Plane& plane : m_planes)
    {
        for(const irr::core::vector3df& vec : vertices)
        {
            if(plane.distance(vec) > 0)
                return true;
        }
    }

    return false;
}

bool Frustum::isVisible(const BoundingBox& bb) const
{
    // see https://fgiesen.wordpress.com/2010/10/17/view-frustum-culling/, method 5
    const irr::core::vector3df center = bb.getCenter();
    const irr::core::vector3df extent = bb.getExtent();
    for(const util::Plane& plane : m_planes)
    {
        irr::core::vector3df signFlipped = center;
        for(int i = 0; i < 3; ++i)
            signFlipped[i] += std::sign(plane.normal[i]) * extent[i];
        if(plane.distance(signFlipped) >= 0)
            return true;
    }
    return false;
}

bool Frustum::isVisible(const OrientedBoundingBox& obb, const Camera& cam) const
{
    bool inside = true;
    for(const Polygon& p : obb.polygons)
    {
        auto t = p.plane.distance(cam.getPosition());
        if(t <= 0)
            continue;

        if(isVisible(p, cam))
            return true;

        inside = false;
    }

    return inside;
}
} // namespace core
} // namespace world
