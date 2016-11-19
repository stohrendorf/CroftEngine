#include "animationcontroller.h"

#include "level/level.h"

#include <chrono>

#include <glm/gtc/matrix_transform.hpp>


namespace
{
#pragma pack(push, 1)
    struct BoneTreeEntry
    {
        uint32_t flags;
        int32_t x, y, z;


        glm::vec3 toGl() const noexcept
        {
            return glm::vec3(x, y, z);
        }
    };
#pragma pack(pop)
}


namespace engine
{
    MeshAnimationController::MeshAnimationController(const std::string& id, const gsl::not_null<const level::Level*>& lvl, const loader::AnimatedModel& mdl)
        : Node{id}
        , m_level{lvl}
        , m_model{mdl}
        , m_targetState{lvl->m_animations[mdl.animationIndex].state_id}
    {
        setAnimId(mdl.animationIndex);
    }


    core::Frame MeshAnimationController::getFirstFrame() const
    {
        return core::Frame(m_level->m_animations[m_animId].firstFrame);
    }


    core::Frame MeshAnimationController::getLastFrame() const
    {
        return core::Frame(m_level->m_animations[m_animId].lastFrame);
    }


    uint16_t MeshAnimationController::getCurrentState() const
    {
        return m_level->m_animations[m_animId].state_id;
    }


    const loader::Animation& MeshAnimationController::getCurrentAnimData() const
    {
        if( m_animId >= m_level->m_animations.size() )
            BOOST_THROW_EXCEPTION(std::runtime_error("Invalid animation id"));

        return m_level->m_animations[m_animId];
    }


    float MeshAnimationController::calculateFloorSpeed() const
    {
        const loader::Animation& currentAnim = getCurrentAnimData();
        return float(currentAnim.speed + currentAnim.accelleration * getCurrentLocalFrame().count()) / (1 << 16);
    }


    int MeshAnimationController::getAccelleration() const
    {
        const loader::Animation& currentAnim = getCurrentAnimData();
        return currentAnim.accelleration / (1 << 16);
    }


    MeshAnimationController::InterpolationInfo MeshAnimationController::getInterpolationInfo() const
    {
        InterpolationInfo result;

        BOOST_ASSERT(m_animId < m_level->m_animations.size());
        const auto& anim = m_level->m_animations[m_animId];
        BOOST_ASSERT(anim.stretchFactor > 0);
        const int16_t* firstFrameData = &m_level->m_poseData[anim.poseDataOffset / 2];
        const auto frameDataOfs = m_model.boneCount * 2 + 10;
        const auto globalFrame = core::toFrame(m_time);
        BOOST_ASSERT(globalFrame.count() >= anim.firstFrame && globalFrame.count() <= anim.lastFrame);
        const auto localFrame = core::toFrame(m_time) - core::Frame(anim.firstFrame);
        const auto keyFrame = localFrame.count() / anim.stretchFactor;
        result.stretchOffset = localFrame.count() % anim.stretchFactor;
        result.firstFrame = reinterpret_cast<const AnimFrame*>(firstFrameData + frameDataOfs * keyFrame);
        result.secondFrame = reinterpret_cast<const AnimFrame*>(firstFrameData + frameDataOfs * (keyFrame + 1));

        result.stretchFactor = anim.stretchFactor;
        if( result.stretchOffset > 0 )
        {
            // avoid the interpolation range to reach beyond the last frame
            if( result.stretchFactor * (keyFrame + 1) > anim.lastFrame)
                result.stretchFactor = anim.lastFrame - result.stretchFactor * keyFrame;

            BOOST_ASSERT(result.stretchFactor > 0);
        }

        return result;
    }


    void MeshAnimationController::updatePose()
    {
        BOOST_ASSERT(getChildCount() > 0);
        BOOST_ASSERT(getChildCount() == m_model.boneCount);

        auto framePair = getInterpolationInfo();
        std::stack<glm::mat4> transformsFirst;
        transformsFirst.push(glm::translate(glm::mat4{ 1.0f }, framePair.firstFrame->pos.toGl()));

        std::stack<glm::mat4> transformsSecond;
        transformsSecond.push(glm::translate(glm::mat4{ 1.0f }, framePair.secondFrame->pos.toGl()));

        auto payloadFirst = framePair.firstFrame->getPayload();
        transformsFirst.top() *= glm::mat4_cast(util::xyzToYpr(payloadFirst));
        payloadFirst += 2;

        auto payloadSecond = framePair.secondFrame->getPayload();
        transformsSecond.top() *= glm::mat4_cast(util::xyzToYpr(payloadSecond));
        payloadSecond += 2;

        const auto bias = gsl::narrow<float>(framePair.stretchOffset) / framePair.stretchFactor;
        BOOST_ASSERT(bias >= 0 && bias <= 1);

        if(m_rotationPatches.empty())
            resetPose();

        BOOST_ASSERT(m_rotationPatches.size() == getChildCount());

        getChildren()[0]->set(glm::mix(transformsFirst.top(), transformsSecond.top(), bias) * glm::mat4_cast(m_rotationPatches[0]));

        if(m_model.boneCount <= 1)
            return;

        const auto* boneTreeData = reinterpret_cast<const BoneTreeEntry*>(&m_level->m_boneTrees[m_model.boneTreeIndex]);

        for( uint16_t i = 1; i < m_model.boneCount; ++i )
        {
            if( boneTreeData->flags & 0x01 )
            {
                transformsFirst.pop();
                transformsSecond.pop();
            }
            if( boneTreeData->flags & 0x02 )
            {
                transformsFirst.push(transformsFirst.top());
                transformsSecond.push(transformsSecond.top());
            }

            transformsFirst.top() *= glm::translate(glm::mat4{ 1.0f }, boneTreeData->toGl()) * glm::mat4_cast(util::xyzToYpr(payloadFirst));
            payloadFirst += 2;
            transformsSecond.top() *= glm::translate(glm::mat4{ 1.0f }, boneTreeData->toGl()) * glm::mat4_cast(util::xyzToYpr(payloadSecond));
            payloadSecond += 2;
#if 0
            if(boneTreeData->flags & 0x08)
                matrices.top() = glm::rotate(matrices.top(), util::auToRad(...));
#endif
            getChildren()[i]->set(glm::mix(transformsFirst.top(), transformsSecond.top(), bias) * glm::mat4_cast(m_rotationPatches[i]));

            ++boneTreeData;
        }
    }


    void MeshAnimationController::advanceFrame()
    {
        BOOST_LOG_TRIVIAL(debug) << "Advance frame: current=" << core::toFrame(m_time).count() << ", end=" << getLastFrame().count();

        addTime(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::seconds(1)) / core::FrameRate);
    }


    gameplay::BoundingBox MeshAnimationController::getBoundingBox() const
    {
        auto framePair = getInterpolationInfo();
        const auto bias = gsl::narrow<float>(framePair.stretchOffset) / framePair.stretchFactor;
        BOOST_ASSERT(bias >= 0 && bias <= 1);

        gameplay::BoundingBox result;

        result.min = glm::mix(framePair.firstFrame->bbox.getMinGl(), framePair.firstFrame->bbox.getMinGl(), bias);
        result.max = glm::mix(framePair.firstFrame->bbox.getMaxGl(), framePair.firstFrame->bbox.getMaxGl(), bias);

        return result;
    }


    bool MeshAnimationController::handleTRTransitions()
    {
        if( getCurrentState() == m_targetState )
            return false;

        const loader::Animation& currentAnim = getCurrentAnimData();
        const auto currentFrame = getCurrentFrame();

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

                if( currentFrame >= core::Frame(trc.firstFrame) && currentFrame <= core::Frame(trc.lastFrame) )
                {
                    setAnimIdGlobal(trc.targetAnimation, trc.targetFrame);
                    BOOST_LOG_TRIVIAL(debug) << getId() << " -- found transition to state " << m_targetState << ", new animation " << m_animId << "/frame " << trc.targetFrame;
                    return true;
                }
            }
        }

        return false;
    }


    void MeshAnimationController::handleAnimationEnd()
    {
        const loader::Animation& currentAnim = getCurrentAnimData();

        setAnimIdGlobal(currentAnim.nextAnimation, currentAnim.nextFrame);

        setTargetState(getCurrentState());
    }


    void MeshAnimationController::setAnimId(size_t animId, size_t frameOfs)
    {
        BOOST_ASSERT(animId < m_level->m_animations.size());

        const auto& anim = m_level->m_animations[animId];
        BOOST_ASSERT(anim.firstFrame + frameOfs <= anim.lastFrame);

        m_animId = animId;
        m_time = core::toTime(core::Frame(anim.firstFrame + frameOfs));
    }


    void MeshAnimationController::setAnimIdGlobal(size_t animId, size_t frame)
    {
        BOOST_ASSERT(animId < m_level->m_animations.size());

        const auto& anim = m_level->m_animations[animId];

        if(frame < anim.firstFrame || frame > anim.lastFrame)
            frame = anim.firstFrame;

        m_animId = animId;
        m_time = core::toTime(core::Frame(frame));
    }
}
