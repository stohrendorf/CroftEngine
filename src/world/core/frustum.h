#pragma once

#include "util/vmath.h"

#include <array>
#include <vector>

namespace world
{
class Camera;
struct Portal;

namespace core
{
struct OrientedBoundingBox;
struct Polygon;
struct BoundingBox;

class Frustum
{
private:
    irr::scene::SViewFrustum m_frustum;

public:
    void setFromMatrix(const irr::core::matrix4& mv)
    {
        m_frustum.setFrom(mv);
    }

    bool isVisible(const Polygon &polygon, const Camera& cam) const;
    bool isVisible(const std::vector<irr::core::vector3df>& vertices) const;
    bool isVisible(const BoundingBox& bb) const;
    bool isVisible(const OrientedBoundingBox &obb, const Camera& cam) const;
    bool isVisible(const Portal &portal) const;

    //! Check if a line intersects with the frustum
    bool intersects(const irr::core::vector3df& a, const irr::core::vector3df& b) const;
};
} // namespace core
} // namespace world
