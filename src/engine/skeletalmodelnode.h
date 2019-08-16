#pragma once

#include "core/magic.h"
#include "gsl-lite.hpp"
#include "loader/file/animation.h"
#include "render/scene/node.h"

namespace loader
{
namespace file
{
struct SkeletalModelType;
struct Animation;
} // namespace file
} // namespace loader

namespace engine
{
class Engine;

namespace items
{
struct ItemState;
}
class SkeletalModelNode : public render::scene::Node
{
public:
    explicit SkeletalModelNode(const std::string& id,
                               const gsl::not_null<const Engine*>& engine,
                               const loader::file::SkeletalModelType& mdl);

    void updatePose(items::ItemState& state);

    void setAnimation(items::ItemState& state,
                      const gsl::not_null<const loader::file::Animation*>& animation,
                      core::Frame frame);

    static core::Speed calculateFloorSpeed(const items::ItemState& state, core::Frame frameOffset = 0_frame);

    loader::file::BoundingBox getBoundingBox(const items::ItemState& state) const;

    void resetPose()
    {
        m_bonePatches.clear();
        m_bonePatches.resize(getChildren().size(), glm::mat4{1.0f});
    }

    void patchBone(const size_t idx, const glm::mat4& m)
    {
        if(m_bonePatches.empty())
        {
            resetPose();
        }

        BOOST_ASSERT(m_bonePatches.size() == getChildren().size());
        BOOST_ASSERT(idx < m_bonePatches.size());

        m_bonePatches[idx] = m;
    }

    bool advanceFrame(items::ItemState& state);

    struct InterpolationInfo
    {
        const loader::file::AnimFrame* firstFrame = nullptr;
        const loader::file::AnimFrame* secondFrame = nullptr;
        float bias = 0;

        const loader::file::AnimFrame* getNearestFrame() const
        {
            if(bias <= 0.5f)
            {
                return firstFrame;
            }
            else
            {
                BOOST_ASSERT(secondFrame != nullptr);
                return secondFrame;
            }
        }
    };

    InterpolationInfo getInterpolationInfo(const items::ItemState& state) const;

    void updatePose(const InterpolationInfo& interpolationInfo)
    {
        if(interpolationInfo.bias == 0 || interpolationInfo.secondFrame == nullptr)
            updatePoseKeyframe(interpolationInfo);
        else
            updatePoseInterpolated(interpolationInfo);
    }

    struct Sphere
    {
        const glm::mat4 m;
        const core::Length radius;

        Sphere(const glm::mat4& m, const core::Length& radius)
            : m{m}
            , radius{radius}
        {
        }

        glm::vec3 getPosition() const
        {
            return glm::vec3(m[3]);
        }
    };

    std::vector<Sphere> getBoneCollisionSpheres(const items::ItemState& state,
                                                const loader::file::AnimFrame& frame,
                                                const glm::mat4* baseTransform);

    void load(const YAML::Node& n);

    YAML::Node save() const;

protected:
    bool handleStateTransitions(items::ItemState& state);

private:
    const gsl::not_null<const Engine*> m_engine;
    const loader::file::SkeletalModelType& m_model;
    std::vector<glm::mat4> m_bonePatches;

    void updatePoseKeyframe(const InterpolationInfo& framePair);

    void updatePoseInterpolated(const InterpolationInfo& framePair);
};
} // namespace engine
