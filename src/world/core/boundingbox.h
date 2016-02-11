#pragma once

#include <algorithm>

#include <irrlicht.h>

class btCollisionShape;

namespace world
{
namespace core
{
struct BoundingBox
{
    irr::core::vector3df min = { 0,0,0 };
    irr::core::vector3df max = { 0,0,0 };

    irr::core::vector3df getCenter() const noexcept
    {
        return (min + max) * 0.5f;
    }

    irr::core::vector3df getExtent() const noexcept
    {
        return max - min;
    }

    void adjust(const irr::core::vector3df& v, irr::f32 r = 0) noexcept
    {
        const irr::f32* f = &v.X;
        irr::f32* mi = &min.X;
        irr::f32* ma = &max.X;
        for(int i = 0; i < 3; ++i)
        {
            if(mi[i] > f[i] - r)
                mi[i] = f[i] - r;
            if(ma[i] < f[i] + r)
                ma[i] = f[i] + r;
        }
    }

    bool contains(const irr::core::vector3df& v) const
    {
        return
            v.X >= min.X && v.X <= max.X &&
            v.Y >= min.Y && v.Y <= max.Y &&
            v.Z >= min.Z && v.Z <= max.Z;
    }

    bool overlaps(const BoundingBox& b) const
    {
        if(min.X >= b.max.X || max.X <= b.min.X ||
           min.Y >= b.max.Y || max.Y <= b.min.Y ||
           min.Z >= b.max.Z || max.Z <= b.min.Z)
        {
            return false;
        }

        return true;
    }

    irr::f32 getMinimumExtent() const
    {
        auto d = getExtent();
        return std::min(d.Y, std::min(d.Y, d.Z));
    }

    irr::f32 getMaximumExtent() const
    {
        auto d = getExtent();
        return std::max(d.X, std::max(d.Y, d.Z));
    }
};

btCollisionShape* BT_CSfromBBox(const BoundingBox &boundingBox, bool useCompression, bool buildBvh);
} // namespace core
} // namespace world
