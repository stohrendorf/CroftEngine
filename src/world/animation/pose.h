#pragma once

#include "world/core/boundingbox.h"

#include <vector>

namespace loader
{
class Level;
}

namespace world
{
namespace animation
{
/**
* Defines position and rotation in the parent's coordinate system
*
* @remark A parent is either another BonePose, or a SkeletonPose.
*/
struct BonePose
{
    irr::core::vector3df position;
    irr::core::quaternion rotation;
};

/**
* Defines a full pose of a @c Skeleton.
*/
struct SkeletonPose
{
    std::vector<BonePose> bonePoses;
    irr::core::vector3df position = { 0,0,0 };
    core::BoundingBox boundingBox;

    void load(const loader::Level& level, size_t poseDataOffset);
};
}
}
