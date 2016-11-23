#include "skeletalmodelnode.h"

#include "level/level.h"

#include <stack>


namespace
{
#pragma pack(push, 1)
    struct BoneTreeEntry
    {
        uint32_t flags;
        int32_t x, y, z;


        glm::vec3 toGl() const noexcept
        {
            return core::TRCoordinates(x, y, z).toRenderSystem();
        }
    };
#pragma pack(pop)

    static_assert(sizeof(BoneTreeEntry) == 16, "BoneTreeEntry must be of size 16");
}


namespace engine
{
    SkeletalModelNode::SkeletalModelNode(const std::string& id, const gsl::not_null<const level::Level*>& lvl, const loader::AnimatedModel& mdl)
        : Node{id}
        , m_level{lvl}
        , m_model{mdl}
        , m_targetState{lvl->m_animations[mdl.animationIndex].state_id}
    {
        setAnimId(mdl.animationIndex);
    }


    core::Frame SkeletalModelNode::getFirstFrame() const
    {
        return core::Frame(m_level->m_animations[m_animId].firstFrame);
    }


    core::Frame SkeletalModelNode::getLastFrame() const
    {
        return core::Frame(m_level->m_animations[m_animId].lastFrame);
    }


    uint16_t SkeletalModelNode::getCurrentState() const
    {
        return m_level->m_animations[m_animId].state_id;
    }


    const loader::Animation& SkeletalModelNode::getCurrentAnimData() const
    {
        if( m_animId >= m_level->m_animations.size() )
        BOOST_THROW_EXCEPTION(std::runtime_error("Invalid animation id"));

        return m_level->m_animations[m_animId];
    }


    float SkeletalModelNode::calculateFloorSpeed() const
    {
        const loader::Animation& currentAnim = getCurrentAnimData();
        return float(currentAnim.speed + currentAnim.accelleration * getCurrentLocalFrame().count()) / (1 << 16);
    }


    int SkeletalModelNode::getAccelleration() const
    {
        const loader::Animation& currentAnim = getCurrentAnimData();
        return currentAnim.accelleration / (1 << 16);
    }


    SkeletalModelNode::InterpolationInfo SkeletalModelNode::getInterpolationInfo() const
    {
        /*
         * == Animation Layout ==
         *
         * Each character in the timeline depicts a single frame.
         *
         * First frame                Last frame/end of animation
         * v                          v
         * |-----|-----|-----|-----|--x..|
         *       ^           <----->     ^
         *       Keyframe    Segment     Last keyframe
         */
        InterpolationInfo result;

        BOOST_ASSERT(m_animId < m_level->m_animations.size());
        const auto& anim = m_level->m_animations[m_animId];
        BOOST_ASSERT(anim.segmentLength > 0);
        const int16_t* keyframes = &m_level->m_poseData[anim.poseDataOffset / 2];

        if(anim.firstFrame == anim.lastFrame)
        {
            result.firstFrame = reinterpret_cast<const AnimFrame*>(keyframes);
            return result;
        }

        const auto keyframeDataSize = m_model.boneCount * 2 + 10;

        const auto startTime = core::toTime(core::Frame(anim.firstFrame));
        const auto endTime = core::toTime(core::Frame(anim.lastFrame) + 1_frame);
        auto segmentDuration = core::toTime(core::Frame(anim.segmentLength));

        BOOST_ASSERT(m_time >= startTime && m_time < endTime);
        const auto animationTime = m_time - startTime;
        const auto keyframeIndex = animationTime.count() / segmentDuration.count();
        BOOST_ASSERT(keyframeIndex < anim.getKeyframeCount());

        result.firstFrame = reinterpret_cast<const AnimFrame*>(keyframes + keyframeDataSize * keyframeIndex);
        result.secondFrame = reinterpret_cast<const AnimFrame*>(keyframes + keyframeDataSize * (keyframeIndex + 1));

        auto segmentTime = animationTime % segmentDuration;

        // If we are interpolating the last two keyframes, the real animation may be shorter
        // than the position of the last keyframe.  E.g., with a stretch factor of 10 and a length of 12,
        // the last segment would only be 2 frames long.  Fame 1 is interpolated with a bias of 0.1, but
        // frame 11 must be interpolated with a bias of 0.5 to compensate the shorter segment length.
        if(segmentDuration * (keyframeIndex + 1) >= endTime)
            segmentDuration = endTime - segmentDuration * keyframeIndex;

        BOOST_ASSERT(segmentTime <= segmentDuration);

        result.bias = static_cast<float>(segmentTime.count()) / segmentDuration.count();
        BOOST_ASSERT(result.bias >= 0 && result.bias <= 1);

        return result;
    }


    void SkeletalModelNode::updatePose()
    {
        BOOST_ASSERT(getChildCount() > 0);
        BOOST_ASSERT(getChildCount() == m_model.boneCount);

        auto framePair = getInterpolationInfo();
        if( framePair.bias <= 1 / 1000.0f || framePair.secondFrame == nullptr )
            updatePoseKeyframe(framePair);
        else
            updatePoseInterpolated(framePair);
    }


    void SkeletalModelNode::updatePoseInterpolated(const InterpolationInfo& framePair)
    {
        BOOST_ASSERT(framePair.bias > 0);
        BOOST_ASSERT(framePair.secondFrame != nullptr);

        BOOST_ASSERT(framePair.firstFrame->numValues > 0);
        BOOST_ASSERT(framePair.secondFrame->numValues > 0);

        if( m_bonePatches.empty() )
            resetPose();
        BOOST_ASSERT(m_bonePatches.size() == getChildCount());

        auto angleDataFirst = framePair.firstFrame->getAngleData();
        std::stack<glm::mat4> transformsFirst;
        transformsFirst.push(glm::translate(glm::mat4{1.0f}, framePair.firstFrame->pos.toGl()) * core::xyzToYprMatrix(*angleDataFirst) * m_bonePatches[0]);
        ++angleDataFirst;

        auto angleDataSecond = framePair.secondFrame->getAngleData();
        std::stack<glm::mat4> transformsSecond;
        transformsSecond.push(glm::translate(glm::mat4{1.0f}, framePair.secondFrame->pos.toGl()) * core::xyzToYprMatrix(*angleDataSecond) * m_bonePatches[0]);
        ++angleDataSecond;

        BOOST_ASSERT(framePair.bias >= 0 && framePair.bias <= 1);

        getChildren()[0]->setLocalMatrix(glm::mix(transformsFirst.top(), transformsSecond.top(), framePair.bias));

        if( m_model.boneCount <= 1 )
            return;

        const auto* positionData = reinterpret_cast<const BoneTreeEntry*>(&m_level->m_boneTrees[m_model.boneTreeIndex]);

        for( uint16_t i = 1; i < m_model.boneCount; ++i , ++positionData , ++angleDataFirst , ++angleDataSecond )
        {
            if( positionData->flags & 0x01 )
            {
                transformsFirst.pop();
                transformsSecond.pop();
            }
            if( positionData->flags & 0x02 )
            {
                transformsFirst.push({transformsFirst.top()}); // make sure to have a copy, not a reference
                transformsSecond.push({transformsSecond.top()}); // make sure to have a copy, not a reference
            }

            BOOST_ASSERT((positionData->flags & 0x1c) == 0);

            if(framePair.firstFrame->numValues < i)
                transformsFirst.top() *= glm::translate(glm::mat4{ 1.0f }, positionData->toGl()) * m_bonePatches[i];
            else
                transformsFirst.top() *= glm::translate(glm::mat4{1.0f}, positionData->toGl()) * core::xyzToYprMatrix(*angleDataFirst) * m_bonePatches[i];

            if(framePair.firstFrame->numValues < i)
                transformsSecond.top() *= glm::translate(glm::mat4{ 1.0f }, positionData->toGl()) * m_bonePatches[i];
            else
                transformsSecond.top() *= glm::translate(glm::mat4{ 1.0f }, positionData->toGl()) * core::xyzToYprMatrix(*angleDataSecond) * m_bonePatches[i];

            getChildren()[i]->setLocalMatrix(glm::mix(transformsFirst.top(), transformsSecond.top(), framePair.bias));
        }
    }


    void SkeletalModelNode::updatePoseKeyframe(const InterpolationInfo& framePair)
    {
        BOOST_ASSERT(framePair.firstFrame->numValues > 0);

        if( m_bonePatches.empty() )
            resetPose();
        BOOST_ASSERT(m_bonePatches.size() == getChildCount());

        auto angleData = framePair.firstFrame->getAngleData();

        std::stack<glm::mat4> transforms;
        transforms.push(glm::translate(glm::mat4{1.0f}, framePair.firstFrame->pos.toGl()) * core::xyzToYprMatrix(*angleData) * m_bonePatches[0]);
        ++angleData;

        getChildren()[0]->setLocalMatrix(transforms.top());

        if( m_model.boneCount <= 1 )
            return;

        const auto* positionData = reinterpret_cast<const BoneTreeEntry*>(&m_level->m_boneTrees[m_model.boneTreeIndex]);

        for( uint16_t i = 1; i < m_model.boneCount; ++i , ++positionData , ++angleData )
        {
            BOOST_ASSERT((positionData->flags & 0x1c) == 0);

            if( positionData->flags & 0x01 )
            {
                transforms.pop();
            }
            if( positionData->flags & 0x02 )
            {
                transforms.push({transforms.top()}); // make sure to have a copy, not a reference
            }

            if(framePair.firstFrame->numValues < i)
                transforms.top() *= glm::translate(glm::mat4{ 1.0f }, positionData->toGl()) * m_bonePatches[i];
            else
                transforms.top() *= glm::translate(glm::mat4{ 1.0f }, positionData->toGl()) * core::xyzToYprMatrix(*angleData) * m_bonePatches[i];

            getChildren()[i]->setLocalMatrix(transforms.top());
        }
    }


    void SkeletalModelNode::advanceFrame()
    {
        BOOST_LOG_TRIVIAL(debug) << "Advance frame: current=" << core::toFrame(m_time).count() << ", end=" << getLastFrame().count();

        addTime(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::seconds(1)) / core::FrameRate);
    }


    gameplay::BoundingBox SkeletalModelNode::getBoundingBox() const
    {
        auto framePair = getInterpolationInfo();
        BOOST_ASSERT(framePair.bias >= 0 && framePair.bias <= 1);

        gameplay::BoundingBox result;

        if( framePair.secondFrame != nullptr )
        {
            result.min = glm::mix(framePair.firstFrame->bbox.getMinGl(), framePair.secondFrame->bbox.getMinGl(), framePair.bias);
            result.max = glm::mix(framePair.firstFrame->bbox.getMaxGl(), framePair.secondFrame->bbox.getMaxGl(), framePair.bias);
        }
        else
        {
            result.min = framePair.firstFrame->bbox.getMinGl();
            result.max = framePair.firstFrame->bbox.getMaxGl();
        }

        return result;
    }


    bool SkeletalModelNode::handleTRTransitions()
    {
        if( getCurrentState() == m_targetState )
            return false;

        const loader::Animation& currentAnim = getCurrentAnimData();

        for( size_t i = 0; i < currentAnim.transitionsCount; ++i )
        {
            auto tIdx = currentAnim.transitionsIndex + i;
            BOOST_ASSERT(tIdx < m_level->m_transitions.size());
            const loader::Transitions& tr = m_level->m_transitions[tIdx];
            if( tr.stateId != m_targetState )
                continue;

            for( auto j = tr.firstTransitionCase; j < tr.firstTransitionCase + tr.transitionCaseCount; ++j )
            {
                BOOST_ASSERT(j < m_level->m_transitionCases.size());
                const loader::TransitionCase& trc = m_level->m_transitionCases[j];

                if( m_time >= core::toTime(core::Frame(trc.firstFrame)) && m_time < core::toTime(core::Frame(trc.lastFrame) + 1_frame) )
                {
                    setAnimIdGlobal(trc.targetAnimation, trc.targetFrame);
                    BOOST_LOG_TRIVIAL(debug) << getId() << " -- found transition to state " << m_targetState << ", new animation " << m_animId << "/frame " << trc.targetFrame;
                    return true;
                }
            }
        }

        return false;
    }


    void SkeletalModelNode::loopAnimation()
    {
        const loader::Animation& currentAnim = getCurrentAnimData();
        setAnimIdGlobal(currentAnim.nextAnimation, currentAnim.nextFrame);
    }


    void SkeletalModelNode::setAnimId(size_t animId, size_t frameOfs)
    {
        BOOST_ASSERT(animId < m_level->m_animations.size());

        const auto& anim = m_level->m_animations[animId];
        BOOST_ASSERT(anim.firstFrame + frameOfs <= anim.lastFrame);

        m_animId = animId;
        m_time = core::toTime(core::Frame(anim.firstFrame + frameOfs));
    }


    void SkeletalModelNode::setAnimIdGlobal(size_t animId, size_t frame)
    {
        BOOST_ASSERT(animId < m_level->m_animations.size());

        const auto& anim = m_level->m_animations[animId];

        if( frame < anim.firstFrame || frame > anim.lastFrame )
            frame = anim.firstFrame;

        m_animId = animId;
        m_time = core::toTime(core::Frame(frame));
    }
}
