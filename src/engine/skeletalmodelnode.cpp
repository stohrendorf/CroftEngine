#include "skeletalmodelnode.h"

#include "engine/engine.h"
#include "engine/objects/object.h"
#include "loader/file/mesh.h"
#include "loader/file/rendermeshdata.h"
#include "serialization/animation_ptr.h"
#include "serialization/glm.h"
#include "serialization/not_null.h"
#include "serialization/quantity.h"
#include "serialization/rendermeshdata_ptr.h"
#include "serialization/skeletalmodeltype_ptr.h"
#include "serialization/vector.h"

#include <stack>
#include <utility>

namespace engine
{
SkeletalModelNode::SkeletalModelNode(const std::string& id,
                                     gsl::not_null<const Engine*> engine,
                                     gsl::not_null<const loader::file::SkeletalModelType*> model)
    : Node{id}
    , m_engine{std::move(engine)}
    , m_model{std::move(model)}
{
}

core::Speed SkeletalModelNode::calculateFloorSpeed(const core::Frame& frameOffset) const
{
  const auto scaled = anim->speed + anim->acceleration * (frame_number - anim->firstFrame + frameOffset);
  return scaled / (1 << 16);
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

  Expects(anim != nullptr);
  Expects(anim->segmentLength > 0_frame);

  Expects(frame_number >= anim->firstFrame && frame_number <= anim->lastFrame);
  const auto firstKeyframeIndex = (frame_number - anim->firstFrame) / anim->segmentLength;

  result.firstFrame = anim->frames->next(firstKeyframeIndex);
  Expects(m_engine->isValid(result.firstFrame));

  if(frame_number >= anim->lastFrame)
  {
    result.secondFrame = result.firstFrame;
    return result;
  }

  result.secondFrame = result.firstFrame->next();
  Expects(m_engine->isValid(result.secondFrame));

  auto segmentDuration = anim->segmentLength;
  const auto segmentFrame = (frame_number - anim->firstFrame) % anim->segmentLength;

  if((firstKeyframeIndex + 1) * anim->segmentLength >= anim->getFrameCount())
  {
    // second keyframe beyond end
    const auto tmp = anim->getFrameCount() % anim->segmentLength;
    if(tmp != 0_frame)
      segmentDuration = tmp + 1_frame;
  }

  result.bias = segmentFrame.retype_as<float>() / segmentDuration.retype_as<float>();
  BOOST_ASSERT(result.bias >= 0 && result.bias <= 1);

  return result;
}

void SkeletalModelNode::updatePose()
{
  if(m_meshParts.empty())
    return;

  BOOST_ASSERT(m_meshParts.size() >= m_model->bones.size());

  updatePose(getInterpolationInfo());
}

void SkeletalModelNode::updatePoseInterpolated(const InterpolationInfo& framePair)
{
  BOOST_ASSERT(!m_model->bones.empty());

  BOOST_ASSERT(framePair.bias > 0);
  BOOST_ASSERT(framePair.secondFrame != nullptr);

  BOOST_ASSERT(framePair.firstFrame->numValues > 0);
  BOOST_ASSERT(framePair.secondFrame->numValues > 0);

  const auto angleDataFirst = framePair.firstFrame->getAngleData();
  std::stack<glm::mat4> transformsFirst;
  transformsFirst.push(translate(glm::mat4{1.0f}, framePair.firstFrame->pos.toGl())
                       * core::fromPackedAngles(angleDataFirst[0]) * m_meshParts[0].patch);

  const auto angleDataSecond = framePair.secondFrame->getAngleData();
  std::stack<glm::mat4> transformsSecond;
  transformsSecond.push(translate(glm::mat4{1.0f}, framePair.secondFrame->pos.toGl())
                        * core::fromPackedAngles(angleDataSecond[0]) * m_meshParts[0].patch);

  BOOST_ASSERT(framePair.bias >= 0 && framePair.bias <= 2);

  m_meshParts[0].matrix = util::mix(transformsFirst.top(), transformsSecond.top(), framePair.bias);

  if(m_model->bones.size() <= 1)
    return;

  for(size_t i = 1; i < m_model->bones.size(); ++i)
  {
    if(m_model->bones[i].popMatrix)
    {
      transformsFirst.pop();
      transformsSecond.pop();
    }
    if(m_model->bones[i].pushMatrix)
    {
      transformsFirst.push({transformsFirst.top()});   // make sure to have a copy, not a reference
      transformsSecond.push({transformsSecond.top()}); // make sure to have a copy, not a reference
    }

    if(framePair.firstFrame->numValues < i)
      transformsFirst.top() *= translate(glm::mat4{1.0f}, m_model->bones[i].position) * m_meshParts[i].patch;
    else
      transformsFirst.top() *= translate(glm::mat4{1.0f}, m_model->bones[i].position)
                               * core::fromPackedAngles(angleDataFirst[i]) * m_meshParts[i].patch;

    if(framePair.firstFrame->numValues < i)
      transformsSecond.top() *= translate(glm::mat4{1.0f}, m_model->bones[i].position) * m_meshParts[i].patch;
    else
      transformsSecond.top() *= translate(glm::mat4{1.0f}, m_model->bones[i].position)
                                * core::fromPackedAngles(angleDataSecond[i]) * m_meshParts[i].patch;

    m_meshParts[i].matrix = util::mix(transformsFirst.top(), transformsSecond.top(), framePair.bias);
  }
}

void SkeletalModelNode::updatePoseKeyframe(const InterpolationInfo& framePair)
{
  BOOST_ASSERT(!m_model->bones.empty());

  BOOST_ASSERT(framePair.firstFrame->numValues > 0);

  const auto angleData = framePair.firstFrame->getAngleData();

  std::stack<glm::mat4> transforms;
  transforms.push(translate(glm::mat4{1.0f}, framePair.firstFrame->pos.toGl()) * core::fromPackedAngles(angleData[0])
                  * m_meshParts[0].patch);

  m_meshParts[0].matrix = transforms.top();

  if(m_model->bones.size() <= 1)
    return;

  for(size_t i = 1; i < m_model->bones.size(); ++i)
  {
    if(m_model->bones[i].popMatrix)
    {
      transforms.pop();
    }
    if(m_model->bones[i].pushMatrix)
    {
      transforms.push({transforms.top()}); // make sure to have a copy, not a reference
    }

    if(framePair.firstFrame->numValues < i)
      transforms.top() *= translate(glm::mat4{1.0f}, m_model->bones[i].position) * m_meshParts[i].patch;
    else
      transforms.top() *= translate(glm::mat4{1.0f}, m_model->bones[i].position) * core::fromPackedAngles(angleData[i])
                          * m_meshParts[i].patch;

    m_meshParts[i].matrix = transforms.top();
  }
}

loader::file::BoundingBox SkeletalModelNode::getBoundingBox() const
{
  const auto framePair = getInterpolationInfo();
  BOOST_ASSERT(framePair.bias >= 0 && framePair.bias <= 1);

  if(framePair.secondFrame != nullptr)
  {
    return {framePair.firstFrame->bbox.toBBox(), framePair.secondFrame->bbox.toBBox(), framePair.bias};
  }
  return framePair.firstFrame->bbox.toBBox();
}

bool SkeletalModelNode::handleStateTransitions(objects::ObjectState& state)
{
  Expects(anim != nullptr);
  if(anim->state_id == state.goal_anim_state)
    return false;

  for(const loader::file::Transitions& tr : anim->transitions)
  {
    if(tr.stateId != state.goal_anim_state)
      continue;

    const auto it = std::find_if(
      tr.transitionCases.cbegin(), tr.transitionCases.cend(), [this](const loader::file::TransitionCase& trc) {
        return frame_number >= trc.firstFrame && frame_number <= trc.lastFrame;
      });

    if(it != tr.transitionCases.cend())
    {
      setAnimation(state, it->targetAnimation, it->targetFrame);
      return true;
    }
  }

  return false;
}

// ReSharper disable once CppMemberFunctionMayBeConst
void SkeletalModelNode::setAnimation(objects::ObjectState& state,
                                     const gsl::not_null<const loader::file::Animation*>& animation,
                                     core::Frame frame)
{
  BOOST_ASSERT(m_model->bones.empty() || animation->frames->numValues == m_model->bones.size());

  if(frame < animation->firstFrame || frame > animation->lastFrame)
    frame = animation->firstFrame;

  anim = animation;
  frame_number = frame;
  state.current_anim_state = anim->state_id;
}

bool SkeletalModelNode::advanceFrame(objects::ObjectState& state)
{
  frame_number += 1_frame;
  if(handleStateTransitions(state))
  {
    state.current_anim_state = anim->state_id;
    if(state.current_anim_state == state.required_anim_state)
      state.required_anim_state = 0_as;
  }

  return frame_number > anim->lastFrame;
}

std::vector<SkeletalModelNode::Sphere> SkeletalModelNode::getBoneCollisionSpheres(const objects::ObjectState& state,
                                                                                  const loader::file::AnimFrame& frame,
                                                                                  const glm::mat4* baseTransform)
{
  BOOST_ASSERT(frame.numValues > 0);
  BOOST_ASSERT(!m_model->bones.empty());

  const auto angleData = frame.getAngleData();

  std::stack<glm::mat4> transforms;

  core::TRVec pos;

  if(baseTransform == nullptr)
  {
    pos = state.position.position;
    transforms.push(state.rotation.toMatrix());
  }
  else
  {
    pos = core::TRVec{};
    transforms.push(*baseTransform * state.rotation.toMatrix());
  }

  transforms.top()
    = translate(transforms.top(), frame.pos.toGl()) * core::fromPackedAngles(angleData[0]) * m_meshParts[0].patch;

  std::vector<Sphere> result;
  result.emplace_back(translate(glm::mat4{1.0f}, pos.toRenderSystem())
                        + translate(transforms.top(), m_model->bones[0].center.toRenderSystem()),
                      m_model->bones[0].collision_size);

  for(gsl::index i = 1; i < m_model->bones.size(); ++i)
  {
    if(m_model->bones[i].popMatrix)
    {
      transforms.pop();
    }
    if(m_model->bones[i].pushMatrix)
    {
      transforms.push({transforms.top()}); // make sure to have a copy, not a reference
    }

    if(frame.numValues < i)
      transforms.top() *= translate(glm::mat4{1.0f}, m_model->bones[i].position) * m_meshParts[i].patch;
    else
      transforms.top() *= translate(glm::mat4{1.0f}, m_model->bones[i].position) * core::fromPackedAngles(angleData[i])
                          * m_meshParts[i].patch;

    auto m = translate(transforms.top(), m_model->bones[i].center.toRenderSystem());
    m[3] += glm::vec4(pos.toRenderSystem(), 0);
    result.emplace_back(m, m_model->bones[i].collision_size);
  }

  return result;
}

void SkeletalModelNode::serialize(const serialization::Serializer& ser)
{
  auto id = getId();
  ser(S_NV("id", id),
      S_NV("model", m_model),
      S_NV("parts", m_meshParts),
      S_NV("anim", anim),
      S_NV("frame", frame_number));

  if(ser.loading)
    ser.lazy([this](const serialization::Serializer&) {
      m_needsMeshRebuild = true;
      rebuildMesh();
    });
}

void serialize(std::shared_ptr<SkeletalModelNode>& data, const serialization::Serializer& ser)
{
  if(ser.loading)
  {
    const loader::file::SkeletalModelType* model = nullptr;
    ser(S_NV("model", model));
    data = std::make_shared<SkeletalModelNode>(
      create(serialization::TypeId<std::string>{}, ser["id"]), &ser.engine, model);
  }
  else
  {
    Expects(data != nullptr);
  }
  data->serialize(ser);
}

void SkeletalModelNode::buildMesh(const std::shared_ptr<SkeletalModelNode>& skeleton, objects::ObjectState& state)
{
  skeleton->setAnimation(state, skeleton->m_model->animations, skeleton->m_model->animations->firstFrame);
  skeleton->m_meshParts.clear();

  for(const auto& bone : skeleton->m_model->bones)
  {
    skeleton->m_meshParts.emplace_back(bone.mesh.get());
  }

  skeleton->m_needsMeshRebuild = true;
  skeleton->rebuildMesh();
  skeleton->updatePose();
}

void SkeletalModelNode::rebuildMesh()
{
  if(!m_needsMeshRebuild)
    return;
  m_needsMeshRebuild = false;

  loader::file::RenderMeshDataCompositor compositor;
  for(const auto& mesh : m_meshParts)
  {
    if(mesh.mesh == nullptr || !mesh.visible)
      compositor.appendEmpty();
    else
      compositor.append(*mesh.mesh);
  }

  setRenderable(compositor.toMesh(*m_engine->getMaterialManager(), true, getId()));
}

bool SkeletalModelNode::canBeCulled(const glm::mat4& viewProjection) const
{
  const auto bbox = getInterpolationInfo().firstFrame->bbox.toBBox();
  const glm::vec3 corners[]{
    core::TRVec{bbox.maxX, bbox.maxY, bbox.maxZ}.toRenderSystem(),
    core::TRVec{bbox.maxX, bbox.maxY, bbox.minZ}.toRenderSystem(),
    core::TRVec{bbox.maxX, bbox.minY, bbox.maxZ}.toRenderSystem(),
    core::TRVec{bbox.maxX, bbox.minY, bbox.minZ}.toRenderSystem(),
    core::TRVec{bbox.minX, bbox.maxY, bbox.maxZ}.toRenderSystem(),
    core::TRVec{bbox.minX, bbox.maxY, bbox.minZ}.toRenderSystem(),
    core::TRVec{bbox.minX, bbox.minY, bbox.maxZ}.toRenderSystem(),
    core::TRVec{bbox.minX, bbox.minY, bbox.minZ}.toRenderSystem(),
  };

  glm::vec2 min{1000.0f}, max{-1000.0f};
  for(const auto& v : corners)
  {
    auto proj = viewProjection * getModelMatrix() * glm::vec4{v, 1.0f};
    proj /= proj.w;
    min = glm::min(min, glm::vec2{proj});
    max = glm::max(max, glm::vec2{proj});
  }

  return min.x > 1 || min.y > 1 || max.x < -1 || max.y < -1;
}

void SkeletalModelNode::MeshPart::serialize(const serialization::Serializer& ser)
{
  ser(S_NV("patch", patch), S_NV("matrix", matrix), S_NV("mesh", mesh), S_NV("visible", visible));
}

SkeletalModelNode::MeshPart SkeletalModelNode::MeshPart::create(const serialization::Serializer& ser)
{
  Expects(ser.loading);
  MeshPart tmp{};
  tmp.serialize(ser);
  return tmp;
}
} // namespace engine
