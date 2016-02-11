#include "orientedboundingbox.h"

#include "engine/engine.h"
#include "world/core/polygon.h"
#include "world/entity.h"

namespace world
{
namespace core
{
void OrientedBoundingBox::rebuild(const BoundingBox& boundingBox)
{
    extent = boundingBox.getExtent() * 0.5f;
    base_centre = boundingBox.getCenter();
    radius = extent.getLength();

    Polygon *p = base_polygons.data();
    // UP
    Polygon *p_up = p;
    auto v = &p->vertices.front();
    // 0 1
    // 0 0
    v->Pos.X = boundingBox.max.X;
    v->Pos.Y = boundingBox.max.Y;
    v->Pos.Z = boundingBox.max.Z;
    v++;

    // 1 0
    // 0 0
    v->Pos.X = boundingBox.min.X;
    v->Pos.Y = boundingBox.max.Y;
    v->Pos.Z = boundingBox.max.Z;
    v++;

    // 0 0
    // 1 0
    v->Pos.X = boundingBox.min.X;
    v->Pos.Y = boundingBox.min.Y;
    v->Pos.Z = boundingBox.max.Z;
    v++;

    // 0 0
    // 0 1
    v->Pos.X = boundingBox.max.X;
    v->Pos.Y = boundingBox.min.Y;
    v->Pos.Z = boundingBox.max.Z;

    //p->plane[0] = 0.0;
    //p->plane[1] = 0.0;
    //p->plane[2] = 1.0;
    //p->plane[3] = -vec3_dot(p->plane, v);
    p->updateNormal();
    p++;

    // DOWN
    Polygon *p_down = p;
    v = &p->vertices.front();
    // 0 1
    // 0 0
    v->Pos.X = boundingBox.max.X;
    v->Pos.Y = boundingBox.max.Y;
    v->Pos.Z = boundingBox.min.Z;
    v++;

    // 0 0
    // 0 1
    v->Pos.X = boundingBox.max.X;
    v->Pos.Y = boundingBox.min.Y;
    v->Pos.Z = boundingBox.min.Z;
    v++;

    // 0 0
    // 1 0
    v->Pos.X = boundingBox.min.X;
    v->Pos.Y = boundingBox.min.Y;
    v->Pos.Z = boundingBox.min.Z;
    v++;

    // 1 0
    // 0 0
    v->Pos.X = boundingBox.min.X;
    v->Pos.Y = boundingBox.max.Y;
    v->Pos.Z = boundingBox.min.Z;

    p->updateNormal();
    p++;

    // RIGHT: OX+
    v = &p->vertices.front();
    v[0].Pos = p_up->vertices[0].Pos;                       // 0 1  up
    v[1].Pos = p_up->vertices[3].Pos;                       // 1 0  up
    v[2].Pos = p_down->vertices[1].Pos;                     // 1 0  down
    v[3].Pos = p_down->vertices[0].Pos;                     // 0 1  down

    p->updateNormal();
    p++;

    // LEFT: OX-
    v = &p->vertices.front();
    v[0].Pos = p_up->vertices[1].Pos;                       // 0 1  up
    v[3].Pos = p_up->vertices[2].Pos;                       // 1 0  up
    v[2].Pos = p_down->vertices[2].Pos;                     // 1 0  down
    v[1].Pos = p_down->vertices[3].Pos;                     // 0 1  down

    p->updateNormal();
    p++;

    // FORWARD: OY+
    v = &p->vertices.front();
    v[0].Pos = p_up->vertices[0].Pos;                       // 0 1  up
    v[3].Pos = p_up->vertices[1].Pos;                       // 1 0  up
    v[2].Pos = p_down->vertices[3].Pos;                     // 1 0  down
    v[1].Pos = p_down->vertices[0].Pos;                     // 0 1  down

    p->updateNormal();
    p++;

    // BACKWARD: OY-
    v = &p->vertices.front();
    v[0].Pos = p_up->vertices[3].Pos;                       // 0 1  up
    v[1].Pos = p_up->vertices[2].Pos;                       // 1 0  up
    v[2].Pos = p_down->vertices[2].Pos;                     // 1 0  down
    v[3].Pos = p_down->vertices[1].Pos;                     // 0 1  down

    //p->plane[0] = 0.0;
    //p->plane[1] = 1.0;
    //p->plane[2] = 0.0;
    //p->plane[3] = -vec3_dot(p->plane, v);
    p->updateNormal();
}

void OrientedBoundingBox::doTransform()
{
    if(transform != nullptr)
    {
        for(size_t i = 0; i < polygons.size(); i++)
        {
            polygons[i].copyTransformed(base_polygons[i], *transform);
        }
        center = *transform * base_centre;
    }
    else
    {
        for(size_t i = 0; i < polygons.size(); i++)
        {
            polygons[i] = base_polygons[i];
        }
        center = base_centre;
    }
}

/*
 * http://www.gamasutra.com/view/feature/131790/simple_intersection_tests_for_games.php?print=1
 */
bool testOverlap(const Entity& e1, const Entity& e2, irr::f32 overlap)
{
    //translation, in parent frame
    auto v = e2.m_obb.center - e1.m_obb.center;
    //translation, in A's frame
    auto T = e1.m_transform * v;

    auto a = e1.m_obb.extent * overlap;
    auto b = e2.m_obb.extent * overlap;

    //B's basis with respect to A's local frame
    irr::f32 ra, rb, t;
    
    irr::core::matrix4 R = e1.m_transform.getTransposed() * e2.m_transform;

    /*ALGORITHM: Use the separating axis test for all 15 potential
    separating axes. If a separating axis could not be found, the two
    boxes overlap. */

    //A's basis vectors
    for(int i = 0; i < 3; i++)
    {
        ra = a[i];
        rb = b.X * std::abs(R(0,i)) + b.Y * std::abs(R(1,i)) + b.Z * std::abs(R(2,i));
        t = std::abs(T[i]);

        if(t > ra + rb)
        {
            return false;
        }
    }

    //B's basis vectors
    for(int k = 0; k < 3; k++)
    {
        ra = a.X * std::abs(R(k,0)) + a.Y * std::abs(R(k,1)) + a.Z * std::abs(R(k,2));
        rb = b[k];
        t = std::abs(T.X * R(k,0)+ T.Y * R(k,1) + T.Z * R(k,2));
        if(t > ra + rb)
        {
            return false;
        }
    }

    //9 cross products
    //L = A0 x B0
    ra = a.Y * std::abs(R(0,2)) + a.Z * std::abs(R(0,1));
    rb = b.Y * std::abs(R(2,0)) + b.Z * std::abs(R(1,0));
    t = std::abs(T.Z * R(0,1) - T.Y * R(0,2));

    if(t > ra + rb)
    {
        return false;
    }

    //L = A0 x B1
    ra = a.Y * std::abs(R(1,2)) + a.Z * std::abs(R(1,1));
    rb = b.X * std::abs(R(2,0)) + b.Z * std::abs(R(0,0));
    t = std::abs(T.Z * R(1,1) - T.Y * R(1,2));

    if(t > ra + rb)
    {
        return false;
    }

    //L = A0 x B2
    ra = a.Y * std::abs(R(2,2)) + a.Z * std::abs(R(2,1));
    rb = b.X * std::abs(R(1,0)) + b.Y * std::abs(R(0,0));
    t = std::abs(T.Z * R(2,1) - T.Y * R(2,2));

    if(t > ra + rb)
    {
        return false;
    }

    //L = A1 x B0
    ra = a.X * std::abs(R(0,2)) + a.Z * std::abs(R(0,0));
    rb = b.Y * std::abs(R(2,1)) + b.Z * std::abs(R(1,1));
    t = std::abs(T.X * R(0,2) - T.Z * R(0,0));

    if(t > ra + rb)
    {
        return false;
    }

    //L = A1 x B1
    ra = a.X * std::abs(R(1,2)) + a.Z * std::abs(R(1,0));
    rb = b.X * std::abs(R(2,1)) + b.Z * std::abs(R(0,1));
    t = std::abs(T.X * R(1,2) - T.Z * R(1,0));

    if(t > ra + rb)
    {
        return false;
    }

    //L = A1 x B2
    ra = a.X * std::abs(R(2,2)) + a.Z * std::abs(R(2,0));
    rb = b.X * std::abs(R(1,1)) + b.Y * std::abs(R(0,1));
    t = std::abs(T.X * R(2,2) - T.Z * R(2,0));

    if(t > ra + rb)
    {
        return false;
    }

    //L = A2 x B0
    ra = a.X * std::abs(R(0,1)) + a.Y * std::abs(R(0,0));
    rb = b.Y * std::abs(R(2,2)) + b.Z * std::abs(R(1,2));
    t = std::abs(T.Y * R(0,0) - T.X * R(0,1));

    if(t > ra + rb)
    {
        return false;
    }

    //L = A2 x B1
    ra = a.X * std::abs(R(1,1)) + a.Y * std::abs(R(1,0));
    rb = b.X * std::abs(R(2,2)) + b.Z * std::abs(R(0,2));
    t = std::abs(T.Y * R(1,0) - T.X * R(1,1));

    if(t > ra + rb)
    {
        return false;
    }

    //L = A2 x B2
    ra = a.X * std::abs(R(2,1)) + a.Y * std::abs(R(2,0));
    rb = b.X * std::abs(R(1,2)) + b.Y * std::abs(R(0,2));
    t = std::abs(T.Y * R(2,0) - T.X * R(2,1));

    if(t > ra + rb)
    {
        return false;
    }

    /*no separating axis found, the two boxes overlap */
    return true;
}
} // namespace core
} // namespace world
