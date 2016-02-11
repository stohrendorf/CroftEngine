#pragma once

/*
 * File:   obb.h
 * Author: nTesla
 *
 * Created on January 21, 2013, 7:11 PM
 */

#include "polygon.h"

#include <array>

namespace world
{
class Camera;
class Entity;
class Room;

namespace core
{
struct BoundingBox;

struct OrientedBoundingBox
{
    std::array<Polygon, 6> base_polygons;           // bv base surface
    std::array<Polygon, 6> polygons;                // bv world coordinate surface
    const irr::core::matrix4* transform = nullptr;          // Object transform matrix
    irr::f32 radius;

    irr::core::vector3df base_centre;
    irr::core::vector3df center;
    irr::core::vector3df extent;

    void doTransform();
    void rebuild(const BoundingBox &boundingBox);
};

constexpr irr::f32 DefaultTestOverlap = 1.2f;

bool testOverlap(const Entity &e1, const Entity &e2, irr::f32 overlap = DefaultTestOverlap);
} // namespace core
} // namespace world
