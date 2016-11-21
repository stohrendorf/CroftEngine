#pragma once

#include "core/magic.h"
#include "util/vmath.h"

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
    class MeshAnimationController : public gameplay::Node
    {
    public:
        explicit MeshAnimationController(const std::string& id, const gsl::not_null<const level::Level*>& lvl, const loader::AnimatedModel& mdl);

        void updatePose();

        void setAnimId(size_t animId, size_t frameOfs = 0);
        void setAnimIdGlobal(size_t animId, size_t frame);


        size_t getAnimId() const noexcept
        {
            return m_animId;
        }


        void checkTransitions()
        {
            if( m_time > getEndTime() )
            {
                handleAnimationEnd();
            }

            handleTRTransitions();
        }


        void addTime(const std::chrono::microseconds& time)
        {
            m_time += time;

            checkTransitions();
        }


        core::Frame getCurrentFrame() const
        {
            return core::toFrame(m_time);
        }


        core::Frame getCurrentLocalFrame() const
        {
            //BOOST_ASSERT(core::toFrame(m_time) >= getFirstFrame());
            return core::toFrame(m_time) - getFirstFrame();
        }


        core::Frame getFirstFrame() const;

        core::Frame getLastFrame() const;

        std::chrono::microseconds getEndTime() const
        {
            return core::toTime(getLastFrame());
        }

        uint16_t getCurrentState() const;


        void playLocal(size_t animId, const core::Frame& frame = core::Frame::zero())
        {
            setAnimId(animId, frame.count());
        }


        const loader::Animation& getCurrentAnimData() const;


        void setTargetState(uint16_t state) noexcept
        {
            if( state == m_targetState )
                return;

            BOOST_LOG_TRIVIAL(debug) << getId() << " -- set target state=" << state << " (was " << m_targetState << "), current state=" << getCurrentState();
            m_targetState = state;
        }


        uint16_t getTargetState() const noexcept
        {
            return m_targetState;
        }


        bool handleTRTransitions();
        void handleAnimationEnd();

        float calculateFloorSpeed() const;

        int getAccelleration() const;

        void advanceFrame();

        gameplay::BoundingBox getBoundingBox() const;


        void resetPose()
        {
            m_bonePatches.clear();
            m_bonePatches.resize(getChildCount(), glm::mat4{1.0f});
        }


        void patchBone(size_t idx, const glm::mat4& m)
        {
            if( m_bonePatches.empty() )
                resetPose();

            BOOST_ASSERT(m_bonePatches.size() == getChildCount());
            BOOST_ASSERT(idx < m_bonePatches.size());

            m_bonePatches[idx] = m;
        }


    private:
        const gsl::not_null<const level::Level*> m_level;
        size_t m_animId = 0;
        std::chrono::microseconds m_time = std::chrono::microseconds::zero();
        const loader::AnimatedModel& m_model;
        uint16_t m_targetState = 0;
        std::vector<glm::mat4> m_bonePatches;

#pragma pack(push, 1)
        struct AnimFrame
        {
            struct BBox
            {
                int16_t minX, maxX;
                int16_t minY, maxY;
                int16_t minZ, maxZ;


                glm::vec3 getMinGl() const noexcept
                {
                    return glm::vec3(minX, minY, minZ);
                }


                glm::vec3 getMaxGl() const noexcept
                {
                    return glm::vec3(maxX, maxY, maxZ);
                }
            };


            struct Vec
            {
                int16_t x, y, z;


                glm::vec3 toGl() const noexcept
                {
                    return glm::vec3(x, -y, -z);
                }
            };


            BBox bbox;
            Vec pos;
            uint16_t numValues;


            const uint32_t* getAngleData() const noexcept
            {
                return reinterpret_cast<const uint32_t*>(this + 1);
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
    };
}
