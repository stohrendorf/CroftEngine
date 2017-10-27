#pragma once

#include "core/magic.h"

#include <gsl/gsl>


namespace level
{
    class Level;
}


namespace loader
{
    struct AnimatedModel;
    struct Animation;
}


namespace engine
{
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


    class SkeletalModelNode : public gameplay::Node
    {
    public:
        explicit SkeletalModelNode(const std::string& id,
                                   const gsl::not_null<const level::Level*>& lvl,
                                   const loader::AnimatedModel& mdl);

        void updatePose();


        void setAnimIdGlobal(size_t animId, size_t frame);


        size_t getAnimId() const noexcept
        {
            return m_animId;
        }


        int getCurrentFrame() const
        {
            return m_frame;
        }


        uint16_t getCurrentState() const;


        void setTargetState(uint16_t state) noexcept
        {
            m_targetState = state;
        }


        uint16_t getTargetState() const noexcept
        {
            return m_targetState;
        }


        int calculateFloorSpeed(int frameOffset = 0) const;
        
        int getAccelleration() const;

        BoundingBox getBoundingBox() const;


        void resetPose()
        {
            m_bonePatches.clear();
            m_bonePatches.resize(getChildCount(), glm::mat4{1.0f});
        }


        void patchBone(size_t idx, const glm::mat4& m)
        {
            if( m_bonePatches.empty() )
                resetPose();

            BOOST_ASSERT( m_bonePatches.size() == getChildCount() );
            BOOST_ASSERT( idx < m_bonePatches.size() );

            m_bonePatches[idx] = m;
        }


        int getCurrentLocalFrame() const
        {
            return m_frame - getStartFrame();
        }


        const loader::Animation& getCurrentAnimData() const;

        bool advanceFrame();

    protected:
        bool handleStateTransitions();

    private:
        const gsl::not_null<const level::Level*> m_level;
        size_t m_animId = 0;
        uint16_t m_frame = 0;
        const loader::AnimatedModel& m_model;
        uint16_t m_targetState = 0;
        std::vector<glm::mat4> m_bonePatches;

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
        };


        InterpolationInfo getInterpolationInfo() const;

        void updatePoseKeyframe(const InterpolationInfo& framepair);

        void updatePoseInterpolated(const InterpolationInfo& framepair);

        int getStartFrame() const;

        int getEndFrame() const;
    };
}
