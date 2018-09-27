#pragma once

#include "core/magic.h"
#include "loader/animation.h"

#include <gsl/gsl>

namespace level
{
class Level;
}

namespace loader
{
struct SkeletalModelType;
struct Animation;
}

namespace engine
{
namespace items
{
struct ItemState;
}
class SkeletalModelNode
        : public gameplay::Node
{
public:
    explicit SkeletalModelNode(const std::string& id,
                               const gsl::not_null<const level::Level*>& lvl,
                               const loader::SkeletalModelType& mdl);

    void updatePose(items::ItemState& state);

    void setAnimation(items::ItemState& state,
                      const gsl::not_null<const loader::Animation*>& animation,
                      uint16_t frame);

    static int calculateFloorSpeed(const items::ItemState& state, int frameOffset = 0);

    static int getAcceleration(const items::ItemState& state);

    loader::BoundingBox getBoundingBox(const items::ItemState& state) const;

    void resetPose()
    {
        m_bonePatches.clear();
        m_bonePatches.resize( getChildren().size(), glm::mat4{1.0f} );
    }

    void patchBone(const size_t idx, const glm::mat4& m)
    {
        if( m_bonePatches.empty() )
        {
            resetPose();
        }

        BOOST_ASSERT( m_bonePatches.size() == getChildren().size() );
        BOOST_ASSERT( idx < m_bonePatches.size() );

        m_bonePatches[idx] = m;
    }

    bool advanceFrame(items::ItemState& state);


    struct InterpolationInfo
    {
        const loader::AnimFrame* firstFrame = nullptr;
        const loader::AnimFrame* secondFrame = nullptr;
        float bias = 0;

        const loader::AnimFrame* getNearestFrame() const
        {
            if( bias <= 0.5f )
            {
                return firstFrame;
            }
            else
            {
                BOOST_ASSERT( secondFrame != nullptr );
                return secondFrame;
            }
        }
    };


    InterpolationInfo getInterpolationInfo(const items::ItemState& state) const;

    void updatePose(const InterpolationInfo& interpolationInfo)
    {
        if( interpolationInfo.bias == 0 || interpolationInfo.secondFrame == nullptr )
            updatePoseKeyframe( interpolationInfo );
        else
            updatePoseInterpolated( interpolationInfo );
    }


    struct Sphere
    {
        const glm::mat4 m;
        const int radius;

        Sphere(const glm::mat4& m, const int radius)
                : m{m}
                , radius{radius}
        {
        }

        glm::vec3 getPosition() const
        {
            return glm::vec3( m[3] );
        }
    };


    std::vector<Sphere> getBoneCollisionSpheres(const items::ItemState& state,
                                                const loader::AnimFrame& frame,
                                                const glm::mat4* baseTransform);

    void load(const YAML::Node& n);

    YAML::Node save() const;

protected:
    bool handleStateTransitions(items::ItemState& state);

private:
    const gsl::not_null<const level::Level*> m_level;
    const loader::SkeletalModelType& m_model;
    std::vector<glm::mat4> m_bonePatches;

    void updatePoseKeyframe(const InterpolationInfo& framePair);

    void updatePoseInterpolated(const InterpolationInfo& framePair);
};
}
