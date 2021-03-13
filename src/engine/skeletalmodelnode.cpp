#include "skeletalmodelnode.h"

#include "engine/objects/object.h"
#include "engine/presenter.h"
#include "loader/file/animation.h"
#include "loader/file/mesh.h"
#include "loader/file/rendermeshdata.h"
#include "render/scene/mesh.h"
#include "serialization/glm.h"
#include "serialization/not_null.h"
#include "serialization/quantity.h"
#include "serialization/rendermeshdata_ptr.h"
#include "serialization/serialization.h"
#include "serialization/skeletalmodeltype_ptr.h"
#include "serialization/vector.h"
#include "serialization/vector_element.h"

#include <stack>
#include <utility>

namespace engine
{
SkeletalModelNode::SkeletalModelNode(const std::string& id,
                                     gsl::not_null<const World*> world,
                                     gsl::not_null<const loader::file::SkeletalModelType*> model)
    : Node{id}
    , m_world{std::move(world)}
    , m_model{std::move(model)}
{
}

core::Speed SkeletalModelNode::calculateFloorSpeed(const core::Frame& frameOffset) const
{
  const auto scaled = m_anim->speed + m_anim->acceleration * (getLocalFrame() + frameOffset);
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
  Expects(m_anim != nullptr);
  Expects(m_anim->segmentLength > 0_frame);

  Expects(m_frame >= m_anim->firstFrame && m_frame <= m_anim->lastFrame);
  const auto firstKeyframeIndex = (m_frame - m_anim->firstFrame) / m_anim->segmentLength;

  auto firstFrame = m_anim->frames->next(firstKeyframeIndex);
  Expects(m_world->isValid(firstFrame));

  if(m_frame >= m_anim->lastFrame)
  {
    return InterpolationInfo{firstFrame, firstFrame, 0.0f};
  }

  const auto secondFrame = firstFrame->next();
  Expects(m_world->isValid(secondFrame));

  auto segmentDuration = m_anim->segmentLength;
  const auto segmentFrame = (m_frame - m_anim->firstFrame) % m_anim->segmentLength;

  if((firstKeyframeIndex + 1) * m_anim->segmentLength >= m_anim->getFrameCount())
  {
    // second keyframe beyond end
    if(const auto tmp = m_anim->getFrameCount() % m_anim->segmentLength; tmp != 0_frame)
      segmentDuration = tmp + 1_frame;
  }

  const auto bias = segmentFrame.cast<float>() / segmentDuration.cast<float>();
  BOOST_ASSERT(bias >= 0 && bias <= 1);

  return InterpolationInfo{firstFrame, secondFrame, bias};
}

void SkeletalModelNode::updatePose()
{
  if(m_meshParts.empty())
    return;

  BOOST_ASSERT(m_meshParts.size() >= m_model->bones.size());

  updatePose(getInterpolationInfo());
}

void SkeletalModelNode::updatePose(const InterpolationInfo& framePair)
{
  BOOST_ASSERT(!m_model->bones.empty());

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

loader::file::BoundingBox SkeletalModelNode::getBoundingBox() const
{
  const auto framePair = getInterpolationInfo();
  BOOST_ASSERT(framePair.bias >= 0 && framePair.bias <= 1);

  return {framePair.firstFrame->bbox.toBBox(), framePair.secondFrame->bbox.toBBox(), framePair.bias};
}

bool SkeletalModelNode::handleStateTransitions(core::AnimStateId& animState, const core::AnimStateId& goal)
{
  Expects(m_anim != nullptr);
  if(m_anim->state_id == goal)
    return false;

  for(const loader::file::TypedTransitions& tr : m_anim->transitions)
  {
    if(tr.stateId != goal)
      continue;

    const auto it = std::find_if(
      tr.transitionCases.cbegin(), tr.transitionCases.cend(), [this](const loader::file::TypedTransitionCase& trc) {
        return m_frame >= trc.firstFrame && m_frame <= trc.lastFrame;
      });

    if(it != tr.transitionCases.cend())
    {
      setAnimation(animState, it->targetAnimation, it->targetFrame);
      return true;
    }
  }

  return false;
}

void SkeletalModelNode::setAnimation(core::AnimStateId& animState,
                                     const gsl::not_null<const loader::file::TypedAnimation*>& animation,
                                     core::Frame frame)
{
  BOOST_ASSERT(m_model->bones.empty() || animation->frames->numValues == m_model->bones.size());

  if(frame < animation->firstFrame || frame > animation->lastFrame)
    frame = animation->firstFrame;

  m_anim = animation;
  m_frame = frame;
  animState = m_anim->state_id;
}

bool SkeletalModelNode::advanceFrame(objects::ObjectState& state)
{
  m_frame += 1_frame;
  if(handleStateTransitions(state.current_anim_state, state.goal_anim_state))
  {
    state.current_anim_state = m_anim->state_id;
    if(state.current_anim_state == state.required_anim_state)
      state.required_anim_state = 0_as;
  }

  return m_frame > m_anim->lastFrame;
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

void SkeletalModelNode::serialize(const serialization::Serializer<World>& ser)
{
  auto id = getName();
  ser(S_NV("id", id),
      S_NV("model", m_model),
      S_NV("parts", m_meshParts),
      S_NVVE("anim", ser.context.getAnimations(), m_anim),
      S_NV("frame", m_frame));

  if(ser.loading)
    ser.lazy([this](const serialization::Serializer<World>&) {
      m_needsMeshRebuild = true;
      rebuildMesh();
      updatePose();
    });
}

void serialize(std::shared_ptr<SkeletalModelNode>& data, const serialization::Serializer<World>& ser)
{
  if(ser.loading)
  {
    const loader::file::SkeletalModelType* model = nullptr;
    ser(S_NV("model", model));
    data = std::make_shared<SkeletalModelNode>(
      create(serialization::TypeId<std::string>{}, ser["id"]), &ser.context, model);
  }
  else
  {
    Expects(data != nullptr);
  }
  data->serialize(ser);
}

void SkeletalModelNode::buildMesh(const std::shared_ptr<SkeletalModelNode>& skeleton, core::AnimStateId& animState)
{
  if(!skeleton->m_meshParts.empty())
    return;

  skeleton->setAnimation(animState, skeleton->m_model->animations, skeleton->m_model->animations->firstFrame);
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

  if(compositor.empty())
    setRenderable(nullptr);
  else
    setRenderable(compositor.toMesh(*m_world->getPresenter().getMaterialManager(), true, getName()));
}

bool SkeletalModelNode::canBeCulled(const glm::mat4& viewProjection) const
{
  const auto bbox = getInterpolationInfo().firstFrame->bbox.toBBox();
  const std::array<glm::vec3, 8> corners{
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

void SkeletalModelNode::setAnim(const gsl::not_null<const loader::file::TypedAnimation*>& anim,
                                const std::optional<core::Frame>& frame)
{
  m_anim = anim;
  m_frame = frame.value_or(anim->firstFrame);
}

core::Frame SkeletalModelNode::getLocalFrame() const
{
  return m_frame - m_anim->firstFrame;
}

void SkeletalModelNode::replaceAnim(const gsl::not_null<const loader::file::TypedAnimation*>& anim,
                                    const core::Frame& localFrame)
{
  setAnim(anim, anim->firstFrame + localFrame);
}

void SkeletalModelNode::MeshPart::serialize(const serialization::Serializer<World>& ser)
{
  ser(S_NV("patch", patch), S_NV("matrix", matrix), S_NV("mesh", mesh), S_NV("visible", visible));
}

SkeletalModelNode::MeshPart SkeletalModelNode::MeshPart::create(const serialization::Serializer<World>& ser)
{
  Expects(ser.loading);
  MeshPart tmp{};
  tmp.serialize(ser);
  return tmp;
}
} // namespace engine
