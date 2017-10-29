#pragma once

#include "core/magic.h"

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
class ItemState;
}

struct BoundingBox
{
    int16_t minX{0}, maxX{0};
    int16_t minY{0}, maxY{0};
    int16_t minZ{0}, maxZ{0};

    explicit BoundingBox() = default;

    BoundingBox(const BoundingBox& a, const BoundingBox& b, float bias)
        : minX{static_cast<int16_t>(a.minX * (1 - bias) + b.minX * bias)}
        , maxX{static_cast<int16_t>(a.maxX * (1 - bias) + b.maxX * bias)}
        , minY{static_cast<int16_t>(a.minY * (1 - bias) + b.minY * bias)}
        , maxY{static_cast<int16_t>(a.maxY * (1 - bias) + b.maxY * bias)}
        , minZ{static_cast<int16_t>(a.minZ * (1 - bias) + b.minZ * bias)}
        , maxZ{static_cast<int16_t>(a.maxZ * (1 - bias) + b.maxZ * bias)}
    {
    }

    core::TRCoordinates getCenter() const
    {
        return {(minX + maxX) / 2, (minY + maxY) / 2, (minZ + maxZ) / 2};
    }
};

class SkeletalModelNode
    : public gameplay::Node
{
public:
    explicit SkeletalModelNode(const std::string& id,
                               const gsl::not_null<const level::Level*>& lvl,
                               const loader::SkeletalModelType& mdl);

    void updatePose(engine::items::ItemState& state);

    void setAnimIdGlobal(engine::items::ItemState& state, int16_t animId, int16_t frame);

    int calculateFloorSpeed(const engine::items::ItemState& state, int frameOffset = 0) const;

    int getAccelleration(const engine::items::ItemState& state) const;

    BoundingBox getBoundingBox(const engine::items::ItemState& state) const;

    void resetPose()
    {
        m_bonePatches.clear();
        m_bonePatches.resize(getChildCount(), glm::mat4{1.0f});
    }

    void patchBone(size_t idx, const glm::mat4& m)
    {
        if( m_bonePatches.empty() )
        {
            resetPose();
        }

        BOOST_ASSERT(m_bonePatches.size() == getChildCount());
        BOOST_ASSERT(idx < m_bonePatches.size());

        m_bonePatches[idx] = m;
    }

    const loader::Animation& getCurrentAnimData(const engine::items::ItemState& state) const;

    bool advanceFrame(engine::items::ItemState& state);

#pragma pack(push, 1)

    struct AnimFrame
    {
        struct Vec
        {
            int16_t x, y, z;

            glm::vec3 toGl() const noexcept
            {
                return glm::vec3(x, -y, -z);
            }
        };

        BoundingBox bbox;
        Vec pos;
        uint16_t numValues;

        gsl::span<const uint32_t> getAngleData() const noexcept
        {
            const auto begin = reinterpret_cast<const uint32_t*>(this + 1);
            return gsl::make_span(begin, numValues);
        }
    };

#pragma pack(pop)

    struct InterpolationInfo
    {
        const AnimFrame* firstFrame = nullptr;
        const AnimFrame* secondFrame = nullptr;
        float bias = 0;

        const AnimFrame* getNearestFrame() const
        {
            if (bias <= 0.5f)
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

    InterpolationInfo getInterpolationInfo(const engine::items::ItemState& state) const;

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
