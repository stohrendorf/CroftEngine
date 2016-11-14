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


    void MeshAnimationController::play(size_t animId)
    {
        if( animId >= m_level->m_animations.size() )
        BOOST_THROW_EXCEPTION(std::runtime_error("Invalid animation id"));

        m_animId = animId;
        m_time = core::toTime(getFirstFrame());

        BOOST_LOG_TRIVIAL(debug) << "Playing animation " << animId << ", state " << getCurrentState();
    }


    void MeshAnimationController::playLocal(size_t animId, const core::Frame& frame)
    {
        play(animId);
        m_time = core::toTime(getFirstFrame() + frame);
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
        const auto localFrame = core::toFrame(m_time) - core::Frame(anim.firstFrame);
        BOOST_ASSERT(localFrame.count() >= 0);
        const auto realFrame = localFrame.count() / anim.stretchFactor;
        result.stretchOffset = localFrame.count() % anim.stretchFactor;
        result.firstFrame = reinterpret_cast<const AnimFrame*>(firstFrameData + frameDataOfs * realFrame);
        result.secondFrame = reinterpret_cast<const AnimFrame*>(firstFrameData + frameDataOfs * (realFrame + 1));

        result.stretchFactor = anim.stretchFactor;
        if( result.stretchOffset > 0 )
        {
            auto lastFrame = anim.lastFrame;
            if( result.stretchFactor + result.stretchFactor * realFrame > lastFrame )
                result.stretchFactor = lastFrame - result.stretchFactor * realFrame;
        }

        return result;
    }


    void MeshAnimationController::updatePose()
    {
        BOOST_ASSERT(getChildCount() > 0);
        BOOST_ASSERT(getChildCount() == m_model.boneCount);

        auto framePair = getInterpolationInfo();
        std::stack<gameplay::Transform> transformsFirst;
        transformsFirst.push(gameplay::Transform());
        transformsFirst.top().translate(framePair.firstFrame->pos.toGl());

        std::stack<gameplay::Transform> transformsSecond;
        transformsSecond.push(gameplay::Transform());
        transformsSecond.top().translate(framePair.secondFrame->pos.toGl());

        auto payloadFirst = framePair.firstFrame->getPayload();
        transformsFirst.top().rotate(util::xyzToYpr(payloadFirst));
        payloadFirst += 2;

        auto payloadSecond = framePair.secondFrame->getPayload();
        transformsSecond.top().rotate(util::xyzToYpr(payloadSecond));
        payloadSecond += 2;

        const auto bias = gsl::narrow<float>(framePair.stretchOffset) / framePair.stretchFactor;
        BOOST_ASSERT(bias >= 0 && bias <= 1);

        if(m_rotationPatches.empty())
            resetPose();

        BOOST_ASSERT(m_rotationPatches.size() == getChildCount());

        getChildren()[0]->set(transformsFirst.top().mix(transformsSecond.top(), bias));
        getChildren()[0]->rotate(m_rotationPatches[0]);

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
                transformsSecond.push(transformsFirst.top());
            }

            transformsFirst.top().translate(boneTreeData->toGl());
            transformsSecond.top().translate(boneTreeData->toGl());

            transformsFirst.top().rotate(util::xyzToYpr(payloadFirst));
            payloadFirst += 2;
            transformsSecond.top().rotate(util::xyzToYpr(payloadSecond));
            payloadSecond += 2;
#if 0
            if(boneTreeData->flags & 0x08)
                matrices.top() = glm::rotate(matrices.top(), util::auToRad(...));
#endif
            getChildren()[i]->set(transformsFirst.top().mix(transformsSecond.top(), bias));
            getChildren()[i]->rotate(m_rotationPatches[i]);

            ++boneTreeData;
        }
    }


    void MeshAnimationController::advanceFrame()
    {
        BOOST_LOG_TRIVIAL(debug) << "Advance frame: current=" << core::toFrame(m_time).count() << ", end=" << getLastFrame().count();

        m_time += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::seconds(1)) / core::FrameRate;

        if( getCurrentFrame() > getLastFrame() )
        {
            handleAnimationEnd();
        }

        handleTRTransitions();
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
                    playLocal(trc.targetAnimation, core::Frame(trc.targetFrame));
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

        playLocal(currentAnim.nextAnimation, core::Frame(currentAnim.nextFrame));

        setTargetState(getCurrentState());
    }
}
