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

#pragma pack(push, 1)


struct BoneTreeEntry
{
    uint32_t flags;

    int32_t x, y, z;

    glm::vec3 toGl() const noexcept
    {
        return core::TRCoordinates( x, y, z ).toRenderSystem();
    }
};


#pragma pack(pop)

static_assert( sizeof( BoneTreeEntry ) == 16, "BoneTreeEntry must be of size 16" );


class SkeletalModelNode
        : public gameplay::Node
{
public:
    explicit SkeletalModelNode(const std::string& id,
                               const gsl::not_null<const level::Level*>& lvl,
                               const loader::SkeletalModelType& mdl);

    void updatePose(engine::items::ItemState& state);

    void setAnimIdGlobal(engine::items::ItemState& state, uint16_t animId, uint16_t frame);

    int calculateFloorSpeed(const engine::items::ItemState& state, int frameOffset = 0) const;

    int getAccelleration(const engine::items::ItemState& state) const;

    loader::BoundingBox getBoundingBox(const engine::items::ItemState& state) const;

    void resetPose()
    {
        m_bonePatches.clear();
        m_bonePatches.resize( getChildCount(), glm::mat4{1.0f} );
    }

    void patchBone(size_t idx, const glm::mat4& m)
    {
        if( m_bonePatches.empty() )
        {
            resetPose();
        }

        BOOST_ASSERT( m_bonePatches.size() == getChildCount() );
        BOOST_ASSERT( idx < m_bonePatches.size() );

        m_bonePatches[idx] = m;
    }

    const loader::Animation& getCurrentAnimData(const engine::items::ItemState& state) const;

    bool advanceFrame(engine::items::ItemState& state);

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


    InterpolationInfo getInterpolationInfo(const engine::items::ItemState& state) const;

    void updatePose(const InterpolationInfo& interpolationInfo)
    {
        if( interpolationInfo.bias == 0 || interpolationInfo.secondFrame == nullptr )
            updatePoseKeyframe( interpolationInfo );
        else
            updatePoseInterpolated( interpolationInfo );
    }


    struct Cylinder
    {
        const core::TRCoordinates position;
        const int radius;

        Cylinder(const core::TRCoordinates& position, int radius)
                : position{position}
                , radius{radius}
        {
        }
    };


    std::vector<Cylinder> getBoneCollisionCylinders(const engine::items::ItemState& state,
                                                    const loader::AnimFrame& frame,
                                                    const glm::mat4* baseTransform);

protected:
    bool handleStateTransitions(engine::items::ItemState& state);

private:
    const gsl::not_null<const level::Level*> m_level;
    const loader::SkeletalModelType& m_model;
    std::vector<glm::mat4> m_bonePatches;

    void updatePoseKeyframe(const InterpolationInfo& framepair);

    void updatePoseInterpolated(const InterpolationInfo& framepair);

    int getEndFrame(const engine::items::ItemState& state) const;
};
}
