#include "polygon.h"

#include <cmath>

#include "util/vmath.h"
#include "boundingbox.h"

namespace world
{
namespace core
{
/*
 * POLYGONS
 */

bool Polygon::isBroken() const
{
    if(vertices.size() < 3)
    {
        return true;
    }

    irr::f32 dif0 = plane.normal.dotProduct(plane.normal);
    if(dif0 < 0.999 || dif0 > 1.001)
    {
        return true;
    }

    auto curr_v = &vertices.back();
    for(const auto& v : vertices)
    {
        auto dif = v.Pos - curr_v->Pos;
        if(dif.dotProduct(dif) < 0.0001)
        {
            return true;
        }

        curr_v = &v;
    }

    return false;
}

void Polygon::updateNormal()
{
    auto v1 = vertices[0].Pos - vertices[1].Pos;
    auto v2 = vertices[2].Pos - vertices[1].Pos;
    plane.assign(v1, v2, { 0,0,0 });
}

void Polygon::move(const irr::core::vector3df& move)
{
    for(auto& v : vertices)
    {
        v.Pos += move;
    }
    plane.moveTo(vertices[0].Pos);
}

void Polygon::copyMoved(const Polygon& src, const irr::core::vector3df& move)
{
    for(size_t i = 0; i < src.vertices.size(); i++)
    {
        vertices[i].Pos = src.vertices[i].Pos + move;
    }

    plane = src.plane;
    plane.moveTo(vertices[0].Pos);
}

void Polygon::transform(const irr::core::matrix4& tr)
{
    tr.rotateVect(plane.normal);
    for(irr::video::S3DVertex& vp : vertices)
    {
        tr.transformVect(vp.Pos);
        tr.rotateVect(vp.Normal);
    }

    plane.moveTo(vertices[0].Pos);
}

void Polygon::copyTransformed(const Polygon& src, const irr::core::matrix4& tr, bool copyNormals)
{
    tr.rotateVect(plane.normal, src.plane.normal);
    for(size_t i = 0; i < src.vertices.size(); i++)
    {
        tr.transformVect(vertices[i].Pos, src.vertices[i].Pos);
        if(copyNormals)
            tr.rotateVect(vertices[i].Normal, src.vertices[i].Normal);
    }

    plane.moveTo(vertices[0].Pos);
}

bool Polygon::rayIntersect(const irr::core::vector3df& rayDir, const irr::core::vector3df& dot, irr::f32& lambda) const
{
    irr::f32 u = plane.normal.dotProduct(rayDir);
    if(std::abs(u) < 0.001 /*|| vec3_plane_dist(plane, dot) < -0.001*/)          // FIXME: magick
    {
        return false;    // plane is parallel to the ray - no intersection
    }
    lambda = -plane.distance(dot) / u;

    const irr::video::S3DVertex* vp = &vertices.front();           // current polygon pointer
    irr::core::vector3df T = dot - vp[0].Pos;

    irr::core::vector3df E2 = vp[1].Pos - vp[0].Pos;
    for(size_t i = 0; i < vertices.size() - 2; i++, vp++)
    {
        irr::core::vector3df E1 = E2;                           // PREV
        E2 = vp[2].Pos - vertices[0].Pos;  // NEXT

        irr::core::vector3df P = rayDir.crossProduct(E2);
        irr::core::vector3df Q = T.crossProduct(E1);

        irr::f32 tt = P.dotProduct(E1);
        u = P.dotProduct(T);
        u /= tt;
        irr::f32 v = Q.dotProduct(rayDir);
        v /= tt;
        tt = 1.0f - u - v;
        if(u <= 1.0 && u >= 0.0 && v <= 1.0 && v >= 0.0 && tt <= 1.0 && tt >= 0.0)
        {
            return true;
        }
    }
    return false;
}

bool Polygon::intersectPolygon(const Polygon& p2)
{
    if(SplitType::InBoth != splitClassify(p2.plane) || SplitType::InBoth != p2.splitClassify(plane))
    {
        return false;  // quick check
    }

    std::vector<irr::core::vector3df> result_buf;

    /*
     * intersection of polygon p1 and plane p2
     */
    const irr::video::S3DVertex* prev_v = &vertices.back();
    const irr::video::S3DVertex* curr_v = &vertices.front();
    irr::f32 dist0 = p2.plane.distance(prev_v->Pos);
    for(size_t i = 0; i < vertices.size(); i++)
    {
        irr::f32 dist1 = p2.plane.distance(curr_v->Pos);
        if(dist1 > SplitEpsilon)
        {
            if(dist0 < -SplitEpsilon)
            {
                result_buf.emplace_back(p2.plane.rayIntersect(prev_v->Pos,
                                                              curr_v->Pos - prev_v->Pos));
            }
        }
        else if(dist1 < -SplitEpsilon)
        {
            if(dist0 > SplitEpsilon)
            {
                result_buf.emplace_back(p2.plane.rayIntersect(prev_v->Pos,
                                                              curr_v->Pos - prev_v->Pos));
            }
        }
        else
        {
            result_buf.emplace_back(curr_v->Pos);
        }

        if(result_buf.size() >= 2)
        {
            break;
        }
        dist0 = dist1;
        prev_v = curr_v;
        curr_v++;
    }

    /*
     * splitting p2 by p1 split plane
     */
    prev_v = &p2.vertices.back();
    curr_v = &p2.vertices.front();
    dist0 = plane.distance(prev_v->Pos);
    for(size_t i = 0; i < p2.vertices.size(); i++)
    {
        irr::f32 dist1 = plane.distance(curr_v->Pos);
        if(dist1 > SplitEpsilon)
        {
            if(dist0 < -SplitEpsilon)
            {
                result_buf.emplace_back(plane.rayIntersect(prev_v->Pos,
                                                           curr_v->Pos - prev_v->Pos));
            }
        }
        else if(dist1 < -SplitEpsilon)
        {
            if(dist0 > SplitEpsilon)
            {
                result_buf.emplace_back(plane.rayIntersect(prev_v->Pos,
                                                           curr_v->Pos - prev_v->Pos));
            }
        }
        else
        {
            result_buf.emplace_back(curr_v->Pos);
        }

        if(result_buf.size() >= 4)
        {
            break;
        }
        dist0 = dist1;
        prev_v = curr_v;
        curr_v++;
    }

    auto dir = plane.normal.crossProduct(p2.plane.normal);  // vector of two planes intersection line
    irr::f32 t = std::abs(dir.X);
    dist0 = std::abs(dir.Y);
    irr::f32 dist1 = std::abs(dir.Z);
    irr::f32 dist2 = 0;
    int pn = PLANE_X;
    if(t < dist0)
    {
        t = dist0;
        pn = PLANE_Y;
    }
    if(t < dist1)
    {
        pn = PLANE_Z;
    }

    //! @todo Use vector operations here.
    switch(pn)
    {
        case PLANE_X:
            dist0 = (result_buf[1].X - result_buf[0].X) / dir.X;
            dist1 = (result_buf[2].Y - result_buf[0].X) / dir.Y;
            dist2 = (result_buf[3].Z - result_buf[0].X) / dir.Z;
            break;

        case PLANE_Y:
            dist0 = (result_buf[1].Y - result_buf[0].Y) / dir.Y;
            dist1 = (result_buf[2].Y - result_buf[0].Y) / dir.Y;
            dist2 = (result_buf[3].Y - result_buf[0].Y) / dir.Y;
            break;

        case PLANE_Z:
            dist0 = (result_buf[1].Z - result_buf[0].Z) / dir.Z;
            dist1 = (result_buf[2].Z - result_buf[0].Z) / dir.Z;
            dist2 = (result_buf[3].Z - result_buf[0].Z) / dir.Z;
            break;
    };

    if(dist0 > 0)
    {
        return !((dist1 < 0.0 && dist2 < 0.0) || (dist1 > dist0 && dist2 > dist0));
    }
    return !((dist1 < dist0 && dist2 < dist0) || (dist1 > 0.0 && dist2 > 0.0));
}

SplitType Polygon::splitClassify(const util::Plane& plane) const
{
    size_t positive = 0, negative = 0;
    for(const auto& v : vertices)
    {
        auto dist = plane.distance(v.Pos);
        if(dist > SplitEpsilon)
        {
            positive++;
        }
        else if(dist < -SplitEpsilon)
        {
            negative++;
        }
    }

    if(positive > 0 && negative == 0)
    {
        return SplitType::Front;
    }
    else if(positive == 0 && negative > 0)
    {
        return SplitType::Back;
    }
    else if(positive < 1 && negative < 1)
    {
        return SplitType::InPlane;
    }

    return SplitType::InBoth;
}

/*
 * animated textures coordinates splits too!
 */
void Polygon::split(const util::Plane& n, Polygon& front, Polygon& back)
{
    front.plane = plane;
    front.textureAnimationId = textureAnimationId;
    front.startFrame = startFrame;
    front.isDoubleSided = isDoubleSided;
    front.textureIndex = textureIndex;
    front.blendMode = blendMode;

    back.plane = plane;
    back.textureAnimationId = textureAnimationId;
    back.startFrame = startFrame;
    back.isDoubleSided = isDoubleSided;
    back.textureIndex = textureIndex;
    back.blendMode = blendMode;

    auto curr_v = &vertices.front();
    auto prev_v = &vertices.back();

    auto dist0 = n.distance(prev_v->Pos);
    for(size_t i = 0; i < vertices.size(); ++i)
    {
        auto dist1 = n.distance(curr_v->Pos);

        if(dist1 > SplitEpsilon)
        {
            if(dist0 < -SplitEpsilon)
            {
                auto dir = curr_v->Pos - prev_v->Pos;
                irr::f32 t;
                irr::video::S3DVertex tv;
                tv.Pos = n.rayIntersect(prev_v->Pos, dir, t);
                tv.Normal.interpolate(prev_v->Normal, curr_v->Normal, t).normalize();

                //! @todo Interpolate using member function
                tv.Color.set(
                            prev_v->Color.getAlpha() + t * (curr_v->Color.getAlpha() - prev_v->Color.getAlpha()),
                            prev_v->Color.getRed() + t * (curr_v->Color.getRed() - prev_v->Color.getRed()),
                            prev_v->Color.getGreen() + t * (curr_v->Color.getGreen() - prev_v->Color.getGreen()),
                            prev_v->Color.getBlue() + t * (curr_v->Color.getBlue() - prev_v->Color.getBlue())
                            );

                tv.TCoords.X = prev_v->TCoords.X + t * (curr_v->TCoords.X - prev_v->TCoords.X);
                tv.TCoords.Y = prev_v->TCoords.Y + t * (curr_v->TCoords.Y - prev_v->TCoords.Y);

                front.vertices.emplace_back(tv);
                back.vertices.emplace_back(tv);
            }
            front.vertices.emplace_back(*curr_v);
        }
        else if(dist1 < -SplitEpsilon)
        {
            if(dist0 > SplitEpsilon)
            {
                auto dir = curr_v->Pos - prev_v->Pos;
                irr::f32 t;
                irr::video::S3DVertex tv;
                tv.Pos = n.rayIntersect(prev_v->Pos, dir, t);
                tv.Normal.interpolate(prev_v->Normal, curr_v->Normal, t).normalize();

                //! @todo Interpolate using member function
                tv.Color.set(
                            prev_v->Color.getAlpha() + t * (curr_v->Color.getAlpha() - prev_v->Color.getAlpha()),
                            prev_v->Color.getRed() + t * (curr_v->Color.getRed() - prev_v->Color.getRed()),
                            prev_v->Color.getGreen() + t * (curr_v->Color.getGreen() - prev_v->Color.getGreen()),
                            prev_v->Color.getBlue() + t * (curr_v->Color.getBlue() - prev_v->Color.getBlue())
                            );

                tv.TCoords.X = prev_v->TCoords.X + t * (curr_v->TCoords.X - prev_v->TCoords.X);
                tv.TCoords.Y = prev_v->TCoords.Y + t * (curr_v->TCoords.Y - prev_v->TCoords.Y);

                front.vertices.emplace_back(tv);
                back.vertices.emplace_back(tv);
            }
            back.vertices.emplace_back(*curr_v);
        }
        else
        {
            front.vertices.emplace_back(*curr_v);
            back.vertices.emplace_back(*curr_v);
        }

        prev_v = curr_v;
        curr_v++;
        dist0 = dist1;
    }
}

bool Polygon::isInsideBBox(const BoundingBox& bb) const
{
    for(const auto& v : vertices)
    {
        if(!bb.contains(v.Pos))
            return false;
    }

    return true;
}

bool Polygon::isInsideBQuad(const BoundingBox& bb) const
{
    for(const auto& v : vertices)
    {
        if(v.Pos.X < bb.min.X || v.Pos.X > bb.max.X ||
           v.Pos.Y < bb.min.Y || v.Pos.Y > bb.max.Y)
        {
            return false;
        }
    }

    return true;
}
} // namespace core
} // namespace world
