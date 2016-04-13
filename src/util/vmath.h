#pragma once

#include "loader/datatypes.h"
#include "util/helpers.h"

#include <LinearMath/btVector3.h>

#include <irrlicht.h>

#define PLANE_X        1
#define PLANE_Y        2
#define PLANE_Z        3

namespace util
{
constexpr const float Rad90 = static_cast<float>(0.5*SIMD_PI);
constexpr const float Rad180 = static_cast<float>(SIMD_PI);
constexpr const float Rad360 = static_cast<float>(2 * SIMD_PI);

// A simple Hesse normal form plane

struct Plane
{
    irr::core::vector3df normal = { 0,0,0 };   // The plane's normal
    irr::f32 dot = 0;   // The plane's distance to the origin

    // Calculates the normalized distance of an arbitrary point in terms of the normal
    // @param pos The point
    // @return The distance in multiples of the normal (if >0, @a pos is in the direction of the normal)

    irr::f32 distance(const irr::core::vector3df& position) const
    {
        return normal.dotProduct(position) + dot;
    }

    irr::core::vector3df rayIntersect(const irr::core::vector3df& rayStart, const irr::core::vector3df& rayDir, irr::f32& lambda) const
    {
        lambda = dot + normal.dotProduct(rayStart);
        lambda /= normal.dotProduct(rayDir);
        return rayStart - lambda * rayDir;
    }

    irr::core::vector3df rayIntersect(const irr::core::vector3df& rayStart, const irr::core::vector3df& rayDir) const
    {
        irr::f32 t;
        return rayIntersect(rayStart, rayDir, t);
    }

    void assign(const irr::core::vector3df& v1, const irr::core::vector3df& v2, const irr::core::vector3df& position)
    {
        normal = v1.crossProduct(v2);
        normal.normalize();
        dot = normal.dotProduct(position);
    }

    void assign(const irr::core::vector3df& n, irr::f32 w)
    {
        const irr::f32 len = n.getLength();
        dot = w / len;
        normal = n/len;
    }

    void mirrorNormal()
    {
        normal = -normal;
        dot = -dot;
    }

    void moveTo(const irr::core::vector3df& where)
    {
        ///@TODO: Project the (where--0) onto the normal before calculating the dot
        dot = normal.dotProduct(where);
    }
};

inline irr::core::quaternion trRotationToQuat(const irr::core::vector3df& rotation)
{
    irr::core::quaternion v;
    v *= irr::core::quaternion().fromAngleAxis(irr::core::degToRad(rotation.Z), {0,0,1});
    v *= irr::core::quaternion().fromAngleAxis(irr::core::degToRad(rotation.X), {1,0,0});
    v *= irr::core::quaternion().fromAngleAxis(irr::core::degToRad(rotation.Y), {0,1,0});
    return v;
}

inline irr::core::vector3df convert(const btVector3& v)
{
    return {v[0], v[1], v[2]};
}

inline btVector3 convert(const irr::core::vector3df& v)
{
    return btVector3(v.X, v.Y, v.Z);
}

inline irr::video::SColorf convert(const loader::FloatColor& tr_c)
{
    return {tr_c.a*2, tr_c.r*2, tr_c.g*2, tr_c.b*2};
}

#if 0
inline bool intersectRayTriangle(const irr::core::vector3df& rayStart, const irr::core::vector3df& rayDir, const irr::core::vector3df& v0, const irr::core::vector3df& v1, const irr::core::vector3df& v2)
{
    BOOST_ASSERT(!fuzzyZero(rayDir.getLength()));
    // Check for intersection with each of the portal's 2 front triangles
    // Solve line-plane intersection using parametric form
    irr::core::vector3df tuv = glm::inverse(glm::mat3(rayDir, v1 - v0, v2 - v0)) * (rayStart - v0);
    if(tuv.y >= 0 && tuv.y <= 1 && tuv.z >= 0 && tuv.z <= 1 && tuv.y + tuv.z <= 1)
        return true;
    else
        return false;
}

inline bool intersectRayRectangle(const irr::core::vector3df& rayStart, const irr::core::vector3df& rayDir, const irr::core::vector3df& v0, const irr::core::vector3df& v1, const irr::core::vector3df& v2)
{
    BOOST_ASSERT(!fuzzyZero(rayDir.getLength()));
    BOOST_ASSERT(fuzzyZero((v1 - v0).dotProduct(v2 - v0))); // test if the vertices are perpendicular
    // Solve line-plane intersection using parametric form
    irr::core::vector3df tuv = glm::inverse(glm::mat3(rayDir, v1 - v0, v2 - v0)) * (rayStart - v0);
    if(tuv.y >= 0 && tuv.y <= 1 && tuv.z >= 0 && tuv.z <= 1)
        return true;
    else
        return false;
}
#endif

constexpr irr::f32 auToDeg(irr::s32 au)
{
    return au / 65536.0f * 360;
}

constexpr irr::f32 auToRad(irr::s32 au)
{
    return au / 65536.0f * 2 * irr::core::PI;
}

constexpr irr::s32 degToAu(irr::f32 deg)
{
    return static_cast<irr::s32>(deg * 65536 / 360);
}
} // namespace util
