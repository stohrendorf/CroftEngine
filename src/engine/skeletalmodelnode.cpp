#include "skeletalmodelnode.h"

#include "level/level.h"

#include <stack>

namespace engine
{
SkeletalModelNode::SkeletalModelNode(const std::string& id,
                                     const gsl::not_null<const level::Level*>& lvl,
                                     const loader::SkeletalModelType& mdl)
        : Node{id}
        , m_level{lvl}
        , m_model{mdl}
{
    //setAnimId(mdl.animationIndex);
}

int SkeletalModelNode::getEndFrame(const engine::items::ItemState& state) const
{
    Expects( state.anim != nullptr );
    return state.anim->lastFrame + 1;
}

int SkeletalModelNode::calculateFloorSpeed(const engine::items::ItemState& state, int frameOffset) const
{
    const auto scaled = state.anim->speed
                        + state.anim->accelleration * (state.frame_number - state.anim->firstFrame + frameOffset);
    return scaled / (1 << 16);
}

int SkeletalModelNode::getAccelleration(const engine::items::ItemState& state) const
{
    return state.anim->accelleration / (1 << 16);
}

SkeletalModelNode::InterpolationInfo
SkeletalModelNode::getInterpolationInfo(const engine::items::ItemState& state) const
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

    Expects( state.anim != nullptr );
    BOOST_ASSERT( state.anim->segmentLength > 0 );

    if( state.anim->firstFrame == state.anim->lastFrame )
    {
        result.firstFrame = state.anim->poseData;
        result.secondFrame = state.anim->poseData;
        return result;
    }

    const uint16_t startFrame = state.anim->firstFrame;
    const uint16_t endFrame = state.anim->lastFrame + 1;

    //BOOST_ASSERT( m_time >= startTime && m_time < endTime );
    const auto animationFrame = util::clamp( state.frame_number, startFrame, endFrame ) - startFrame;
    int firstKeyframeIndex = animationFrame / state.anim->segmentLength;
    BOOST_ASSERT( firstKeyframeIndex >= 0 );
    BOOST_ASSERT( static_cast<size_t>(firstKeyframeIndex) < state.anim->getKeyframeCount() );

    if( static_cast<size_t>(firstKeyframeIndex) == state.anim->getKeyframeCount() - 1u )
    {
        result.firstFrame = state.anim->poseData->next( firstKeyframeIndex );
        result.secondFrame = result.firstFrame;
        result.bias = 0;
        return result;
    }
    result.firstFrame = state.anim->poseData->next( firstKeyframeIndex );
    result.secondFrame = result.firstFrame->next();

    auto segmentDuration = state.anim->segmentLength;
    auto segmentFrame = animationFrame % segmentDuration;

    // If we are interpolating the last two keyframes, the real animation may be shorter
    // than the position of the last keyframe.  E.g., with a stretch factor of 10 and a length of 12,
    // the last segment would only be 2 frames long.  Fame 1 is interpolated with a bias of 0.1, but
    // frame 11 must be interpolated with a bias of 0.5 to compensate the shorter segment length.
    if( segmentDuration * (firstKeyframeIndex + 1) >= endFrame )
        segmentDuration = endFrame - segmentDuration * firstKeyframeIndex;

    BOOST_ASSERT( segmentFrame <= segmentDuration );

    result.bias += static_cast<float>(segmentFrame) / segmentDuration;
    BOOST_ASSERT( result.bias >= 0 && result.bias <= 2 );

    return result;
}

void SkeletalModelNode::updatePose(engine::items::ItemState& state)
{
    BOOST_ASSERT( getChildCount() > 0 );
    BOOST_ASSERT( getChildCount() == m_model.nmeshes );

    auto interpolationInfo = getInterpolationInfo( state );
    updatePose( interpolationInfo );
}

void SkeletalModelNode::updatePoseInterpolated(const InterpolationInfo& framePair)
{
    BOOST_ASSERT( m_model.nmeshes > 0 );

    BOOST_ASSERT( framePair.bias > 0 );
    BOOST_ASSERT( framePair.secondFrame != nullptr );

    BOOST_ASSERT( framePair.firstFrame->numValues > 0 );
    BOOST_ASSERT( framePair.secondFrame->numValues > 0 );

    if( m_bonePatches.empty() )
        resetPose();
    BOOST_ASSERT( m_bonePatches.size() == getChildCount() );

    const auto angleDataFirst = framePair.firstFrame->getAngleData();
    std::stack<glm::mat4> transformsFirst;
    transformsFirst.push( glm::translate( glm::mat4{1.0f}, framePair.firstFrame->pos.toGl() )
                          * core::fromPackedAngles( angleDataFirst[0] ) * m_bonePatches[0] );

    const auto angleDataSecond = framePair.secondFrame->getAngleData();
    std::stack<glm::mat4> transformsSecond;
    transformsSecond.push( glm::translate( glm::mat4{1.0f}, framePair.secondFrame->pos.toGl() )
                           * core::fromPackedAngles( angleDataSecond[0] ) * m_bonePatches[0] );

    BOOST_ASSERT( framePair.bias >= 0 && framePair.bias <= 2 );

    getChildren()[0]->setLocalMatrix( util::mix( transformsFirst.top(), transformsSecond.top(), framePair.bias ) );

    if( m_model.nmeshes <= 1 )
        return;

    const auto* positionData = reinterpret_cast<const BoneTreeEntry*>(&m_level->m_boneTrees[m_model.bone_index]);

    for( int i = 1; i < m_model.nmeshes; ++i, ++positionData )
    {
        if( positionData->flags & 0x01 )
        {
            transformsFirst.pop();
            transformsSecond.pop();
        }
        if( positionData->flags & 0x02 )
        {
            transformsFirst.push( {transformsFirst.top()} ); // make sure to have a copy, not a reference
            transformsSecond.push( {transformsSecond.top()} ); // make sure to have a copy, not a reference
        }

        BOOST_ASSERT( (positionData->flags & 0x1c) == 0 );

        if( framePair.firstFrame->numValues < i )
            transformsFirst.top() *= glm::translate( glm::mat4{1.0f}, positionData->toGl() )
                                     * m_bonePatches[i];
        else
            transformsFirst.top() *= glm::translate( glm::mat4{1.0f}, positionData->toGl() )
                                     * core::fromPackedAngles( angleDataFirst[i] ) * m_bonePatches[i];

        if( framePair.firstFrame->numValues < i )
            transformsSecond.top() *= glm::translate( glm::mat4{1.0f}, positionData->toGl() )
                                      * m_bonePatches[i];
        else
            transformsSecond.top() *= glm::translate( glm::mat4{1.0f}, positionData->toGl() )
                                      * core::fromPackedAngles( angleDataSecond[i] ) * m_bonePatches[i];

        getChildren()[i]
                ->setLocalMatrix( util::mix( transformsFirst.top(), transformsSecond.top(), framePair.bias ) );
    }
}

void SkeletalModelNode::updatePoseKeyframe(const InterpolationInfo& framePair)
{
    BOOST_ASSERT( m_model.nmeshes > 0 );

    BOOST_ASSERT( framePair.firstFrame->numValues > 0 );

    if( m_bonePatches.empty() )
        resetPose();
    BOOST_ASSERT( m_bonePatches.size() == getChildCount() );

    const auto angleData = framePair.firstFrame->getAngleData();

    std::stack<glm::mat4> transforms;
    transforms.push( glm::translate( glm::mat4{1.0f}, framePair.firstFrame->pos.toGl() )
                     * core::fromPackedAngles( angleData[0] ) * m_bonePatches[0] );

    getChildren()[0]->setLocalMatrix( transforms.top() );

    if( m_model.nmeshes <= 1 )
        return;

    const auto* positionData = reinterpret_cast<const BoneTreeEntry*>(&m_level->m_boneTrees[m_model.bone_index]);

    for( uint16_t i = 1; i < m_model.nmeshes; ++i, ++positionData )
    {
        BOOST_ASSERT( (positionData->flags & 0x1c) == 0 );

        if( positionData->flags & 0x01 )
        {
            transforms.pop();
        }
        if( positionData->flags & 0x02 )
        {
            transforms.push( {transforms.top()} ); // make sure to have a copy, not a reference
        }

        if( framePair.firstFrame->numValues < i )
            transforms.top() *= glm::translate( glm::mat4{1.0f}, positionData->toGl() ) * m_bonePatches[i];
        else
            transforms.top() *= glm::translate( glm::mat4{1.0f}, positionData->toGl() )
                                * core::fromPackedAngles( angleData[i] ) * m_bonePatches[i];

        getChildren()[i]->setLocalMatrix( transforms.top() );
    }
}

loader::BoundingBox SkeletalModelNode::getBoundingBox(const engine::items::ItemState& state) const
{
    auto framePair = getInterpolationInfo( state );
    BOOST_ASSERT( framePair.bias >= 0 && framePair.bias <= 1 );

    if( framePair.secondFrame != nullptr )
    {
        return loader::BoundingBox( framePair.firstFrame->bbox, framePair.secondFrame->bbox, framePair.bias );
    }
    return framePair.firstFrame->bbox;
}

bool SkeletalModelNode::handleStateTransitions(engine::items::ItemState& state)
{
    Expects( state.anim != nullptr );
    if( state.anim->state_id == state.goal_anim_state )
        return false;

    for( const loader::Transitions& tr : state.anim->transitions )
    {
        if( tr.stateId != state.goal_anim_state )
            continue;

        for( const loader::TransitionCase& trc : tr.transitionCases )
        {
            if( state.frame_number >= trc.firstFrame && state.frame_number <= trc.lastFrame )
            {
                setAnimIdGlobal( state, to_not_null( trc.targetAnimation ), trc.targetFrame );
                return true;
            }
        }
    }

    return false;
}

void
SkeletalModelNode::setAnimIdGlobal(engine::items::ItemState& state, gsl::not_null<const loader::Animation*> animation,
                                   uint16_t frame)
{
    BOOST_ASSERT( getChildCount() == 0 || animation->poseData->numValues == getChildCount() );

    if( frame < animation->firstFrame || frame > animation->lastFrame )
        frame = animation->firstFrame;

    state.anim = animation;
    state.frame_number = frame;
    state.current_anim_state = state.anim->state_id;
}

bool SkeletalModelNode::advanceFrame(engine::items::ItemState& state)
{
    ++state.frame_number;
    handleStateTransitions( state );
    return state.frame_number >= getEndFrame( state );
}

std::vector<SkeletalModelNode::Sphere>
SkeletalModelNode::getBoneCollisionSpheres(const engine::items::ItemState& state,
                                           const loader::AnimFrame& frame,
                                           const glm::mat4* baseTransform)
{
    BOOST_ASSERT( frame.numValues > 0 );
    BOOST_ASSERT( m_model.nmeshes > 0 );

    if( m_bonePatches.empty() )
        resetPose();
    BOOST_ASSERT( m_bonePatches.size() == getChildCount() );

    const auto angleData = frame.getAngleData();

    std::stack<glm::mat4> transforms;

    core::TRVec pos;

    if( baseTransform == nullptr )
    {
        pos = state.position.position;
        transforms.push( glm::mat4{1.0f} );
    }
    else
    {
        pos = core::TRVec( 0, 0, 0 );
        transforms.push( *baseTransform * state.rotation.toMatrix() );
    }

    transforms.top() = glm::translate( transforms.top(), frame.pos.toGl() )
                       * core::fromPackedAngles( angleData[0] ) * m_bonePatches[0];

    const auto* mesh = m_model.meshes;

    std::vector<Sphere> result;
    result.emplace_back(
            glm::translate( glm::mat4{1.0f}, pos.toRenderSystem() )
            + glm::translate( transforms.top(), mesh->center.toRenderSystem() ),
            mesh->collision_size );
    ++mesh;

    const auto* positionData = reinterpret_cast<const BoneTreeEntry*>(&m_level->m_boneTrees[m_model.bone_index]);
    for( int i = 1; i < m_model.nmeshes; ++i, ++positionData, ++mesh )
    {
        BOOST_ASSERT( (positionData->flags & 0x1c) == 0 );

        if( positionData->flags & 0x01 )
        {
            transforms.pop();
        }
        if( positionData->flags & 0x02 )
        {
            transforms.push( {transforms.top()} ); // make sure to have a copy, not a reference
        }

        if( frame.numValues < i )
            transforms.top() *= glm::translate( glm::mat4{1.0f}, positionData->toGl() ) * m_bonePatches[i];
        else
            transforms.top() *= glm::translate( glm::mat4{1.0f}, positionData->toGl() )
                                * core::fromPackedAngles( angleData[i] ) * m_bonePatches[i];

        auto m = glm::translate( transforms.top(), mesh->center.toRenderSystem() );
        m[3] += glm::vec4( pos.toRenderSystem(), 0 );
        result.emplace_back( m, mesh->collision_size );
    }

    return result;
}
}
