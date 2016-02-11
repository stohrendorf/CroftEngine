#pragma once

#include "core/basemesh.h"
#include "core/orientedboundingbox.h"
#include "object.h"

namespace world
{
struct StaticMesh : public Object
{
    explicit StaticMesh(ObjectId id, World* world, Room* room = nullptr)
        : Object(id, world, room)
    {
    }

    bool was_rendered = false;
    bool was_rendered_lines = false;
    bool hide = true;
    irr::video::SColorf tint{ 0,0,0,0 };

    core::BoundingBox visibleBoundingBox;
    core::BoundingBox collisionBoundingBox;

    irr::core::matrix4 transform;
    core::OrientedBoundingBox obb;

    std::shared_ptr<core::BaseMesh> mesh = nullptr;
    btRigidBody* bt_body = nullptr;
};
} // namespace world
