#pragma once

#include "loader/datatypes.h"
#include "util/vmath.h"

#include <boost/optional.hpp>

#include <irrlicht.h>

#include <cstdint>
#include <vector>

namespace world
{
namespace core
{
enum class SplitType
{
    Front,
    Back,
    InPlane,
    InBoth
};

constexpr float SplitEpsilon = 0.02f;

struct BoundingBox;

struct Polygon
{
    std::vector<irr::video::S3DVertex> vertices{ 4 }; //!< vertices data
    size_t textureIndex = 0; //!< texture index
    boost::optional<size_t> textureAnimationId = boost::none; //!< anim texture ID
    size_t startFrame = 0; //!< anim texture frame offset
    loader::BlendingMode blendMode = loader::BlendingMode::Solid; //!< transparency information
    bool isDoubleSided = false;  //!< double side flag
    util::Plane plane; //!< polygon plane equation

    bool isBroken() const;

    void move(const irr::core::vector3df& copyMoved);
    void copyMoved(const Polygon &src, const irr::core::vector3df& copyMoved);
    void copyTransformed(const Polygon &src, const irr::core::matrix4& tr, bool copyNormals = false);
    void transform(const irr::core::matrix4& tr);

    void updateNormal();
    bool rayIntersect(const irr::core::vector3df& rayDir, const irr::core::vector3df& dot, irr::f32& lambda) const;
    bool intersectPolygon(const Polygon& p2);

    SplitType splitClassify(const util::Plane &plane) const;
    void split(const util::Plane &n, Polygon& front, Polygon& back);

    bool isInsideBBox(const BoundingBox& bb) const;
    bool isInsideBQuad(const BoundingBox& bb) const;
};
} // namespace core
} // namespace world
