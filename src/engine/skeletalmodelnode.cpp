#include "skeletalmodelnode.h"

#include "core/angle.h"
#include "core/boundingbox.h"
#include "core/id.h"
#include "core/units.h"
#include "engine/engine.h"
#include "engine/engineconfig.h"
#include "loader/file/animation.h"
#include "objects/objectstate.h"
#include "presenter.h"
#include "qs/qs.h"
#include "render/material/materialmanager.h"
#include "render/scene/mesh.h" // IWYU pragma: keep
#include "serialization/glm.h"
#include "serialization/not_null.h"
#include "serialization/quantity.h"
#include "serialization/rendermeshdata_ptr.h"
#include "serialization/serialization.h"
#include "serialization/skeletalmodeltype_ptr.h"
#include "serialization/vector.h"
#include "serialization/vector_element.h"
#include "util/helpers.h"
#include "world/animation.h"
#include "world/rendermeshdata.h"
#include "world/skeletalmodeltype.h"
#include "world/transition.h"
#include "world/world.h"

#include <algorithm>
#include <boost/assert.hpp>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <gl/buffer.h>
#include <gl/pixel.h>
#include <glm/common.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <gsl-lite/gsl-lite.hpp>
#include <iterator>
#include <memory>
#include <optional>
#include <stack>
#include <string>
#include <utility>
#include <vector>

namespace engine
{
namespace
{
std::optional<const world::TransitionCase>
  findTransition(const world::Animation& anim, const core::Frame frame, const core::AnimStateId goal)
{
  if(anim.state_id == goal)
    return std::nullopt;

  for(const world::Transitions& tr : anim.transitions)
  {
    if(tr.stateId != goal)
      continue;

    const auto it = std::ranges::find_if(tr.transitionCases,
                                         [frame](const world::TransitionCase& trc)
                                         {
                                           return frame >= trc.firstFrame && frame <= trc.lastFrame;
                                         });

    if(it != tr.transitionCases.cend())
      return *it;
  }

  return std::nullopt;
}
} // namespace

SkeletalModelNode::SkeletalModelNode(const std::string& id,
                                     gsl_lite::not_null<world::World*> world,
                                     gsl_lite::not_null<const world::SkeletalModelType*> model,
                                     const bool shadowCaster)
    : Node{id}
    , m_world{std::move(world)}
    , m_model{std::move(model)}
    , m_shadowCaster{shadowCaster}
{
}

core::Speed SkeletalModelNode::calculateFloorSpeed() const
{
  const auto scaled = m_anim->speed + m_anim->acceleration * getLocalFrame();
  // NOLINTNEXTLINE(hicpp-signed-bitwise)
  return scaled / gsl_lite::narrow_cast<core::Speed::type>(1 << 16);
}

core::Acceleration SkeletalModelNode::getAcceleration() const noexcept
{
  const auto scaled = m_anim->acceleration;
  // NOLINTNEXTLINE(hicpp-signed-bitwise)
  return scaled / gsl_lite::narrow_cast<core::Acceleration::type>(1 << 16);
}

AnimSegmentInterpolationInfo SkeletalModelNode::getInterpolationInfo(const world::Animation& anim,
                                                                     core::Frame frame) const
{
  gsl_Expects(frame >= anim.firstFrame && frame <= anim.lastFrame);

  const auto localFrame = frame - anim.firstFrame;
  const auto firstLocalKeyframeIndex = localFrame / anim.segmentLength;

  const auto firstKeyframe = anim.frames->next(firstLocalKeyframeIndex);
  gsl_Assert(m_world->getWorldGeometry().isValid(firstKeyframe));

  const auto segmentFrame = localFrame % anim.segmentLength;
  if(segmentFrame == 0_frame)
  {
    return AnimSegmentInterpolationInfo{firstKeyframe, firstKeyframe, 0.0f};
  }

  const auto interKeyframeFactor = segmentFrame.cast<float>() / anim.segmentLength.cast<float>();
  BOOST_ASSERT(interKeyframeFactor >= 0 && interKeyframeFactor <= 1);

  const auto secondKeyframe = firstKeyframe->next();
  gsl_Assert(m_world->getWorldGeometry().isValid(secondKeyframe));
  return AnimSegmentInterpolationInfo{firstKeyframe, secondKeyframe, interKeyframeFactor};
}

AnimSegmentInterpolationInfo SkeletalModelNode::getInterpolationInfo() const
{
  return getInterpolationInfo(*m_anim, m_frame);
}

AnimSegmentInterpolationInfo SkeletalModelNode::getNextInterpolationInfo() const
{
  // TODO this is non-contiguous, check if it can be made smoother by using goal anim states and state transition checks
  return getInterpolationInfo(*m_anim, std::min(m_frame + 1_frame, m_anim->lastFrame));
}

void SkeletalModelNode::calculatePoseMatrices(const bool predictive)
{
  if(m_meshParts.empty())
    return;

  BOOST_ASSERT(m_meshParts.size() >= m_model->bones.size());

  calculatePoseMatrices(getInterpolationInfo(), &MeshPart::poseMatrix);
  calculatePoseMatrices(predictive ? getNextInterpolationInfo() : getInterpolationInfo(), &MeshPart::nextPoseMatrix);
}

void SkeletalModelNode::calculatePoseMatrices(const AnimSegmentInterpolationInfo& framePair,
                                              glm::mat4 MeshPart::* targetMatrix)
{
  BOOST_ASSERT(!m_model->bones.empty());
  BOOST_ASSERT(framePair.firstFrame->numValues > 0);
  BOOST_ASSERT(framePair.secondFrame->numValues > 0);

  const auto angleDataFirst = framePair.firstFrame->getAngleData();
  std::stack<glm::mat4> transformsFirst;
  transformsFirst.push(glm::translate(glm::mat4{1.0f}, framePair.firstFrame->pos.toGl())
                       * core::fromPackedAngles(angleDataFirst.data()) * m_meshParts[0].patch);

  const auto angleDataSecond = framePair.secondFrame->getAngleData();
  std::stack<glm::mat4> transformsSecond;
  transformsSecond.push(translate(glm::mat4{1.0f}, framePair.secondFrame->pos.toGl())
                        * core::fromPackedAngles(angleDataSecond.data()) * m_meshParts[0].patch);

  m_meshParts[0].*targetMatrix
    = util::lerp(transformsFirst.top(), transformsSecond.top(), framePair.interKeyframeFactor);

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
      transformsFirst.push({transformsFirst.top()});
      transformsSecond.push({transformsSecond.top()});
    }

    if(framePair.firstFrame->numValues < i)
      transformsFirst.top() *= translate(glm::mat4{1.0f}, m_model->bones[i].position) * m_meshParts[i].patch;
    else
      transformsFirst.top() *= translate(glm::mat4{1.0f}, m_model->bones[i].position)
                               * core::fromPackedAngles(&angleDataFirst[sizeof(uint32_t) * i]) * m_meshParts[i].patch;

    if(framePair.firstFrame->numValues < i)
      transformsSecond.top() *= translate(glm::mat4{1.0f}, m_model->bones[i].position) * m_meshParts[i].patch;
    else
      transformsSecond.top() *= translate(glm::mat4{1.0f}, m_model->bones[i].position)
                                * core::fromPackedAngles(&angleDataSecond[sizeof(uint32_t) * i]) * m_meshParts[i].patch;

    m_meshParts[i].*targetMatrix
      = util::lerp(transformsFirst.top(), transformsSecond.top(), framePair.interKeyframeFactor);
  }
}

core::BoundingBox SkeletalModelNode::getBoundingBox() const
{
  const auto framePair = getInterpolationInfo();
  BOOST_ASSERT(framePair.interKeyframeFactor >= 0 && framePair.interKeyframeFactor <= 1);

  return core::BoundingBox{
    framePair.firstFrame->bbox.toBBox(), framePair.secondFrame->bbox.toBBox(), framePair.interKeyframeFactor};
}

bool SkeletalModelNode::handleStateTransitions(core::AnimStateId& animState, const core::AnimStateId& goal)
{
  gsl_Expects(m_anim != nullptr);

  if(const auto transition = findTransition(*m_anim, m_frame, goal))
  {
    setAnimation(animState, gsl_lite::not_null{transition->targetAnimation}, transition->targetFrame);
    return true;
  }

  return false;
}

void SkeletalModelNode::setAnimation(core::AnimStateId& animState,
                                     const gsl_lite::not_null<const world::Animation*>& animation,
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

std::vector<SkeletalModelNode::Sphere> SkeletalModelNode::getBoneCollisionSpheres()
{
  calculatePoseMatrices(true);
  gsl_Expects(m_meshParts.size() == m_model->bones.size());
  std::vector<Sphere> result;
  result.reserve(m_meshParts.size());
  for(size_t i = 0; i < m_meshParts.size(); ++i)
  {
    const auto& bone = m_model->bones[i];
    result.emplace_back(getModelMatrix() * m_meshParts[i].poseMatrix, bone.collisionCenter, bone.collisionSize);
  }
  return result;
}

void SkeletalModelNode::serialize(const serialization::Serializer<world::World>& ser) const
{
  auto id = getName();
  ser(S_NV("id", id),
      S_NV("model", m_model),
      S_NV("parts", m_meshParts),
      S_NV_VECTOR_ELEMENT("anim", std::cref(ser.context->getWorldGeometry().getAnimations()), std::cref(m_anim)),
      S_NV("frame", m_frame),
      S_NV("shadowCaster", m_shadowCaster));
}

void SkeletalModelNode::deserialize(const serialization::Deserializer<world::World>& ser)
{
  auto id = getName();
  ser(S_NV("id", id),
      S_NV("model", m_model),
      S_NV("parts", m_meshParts),
      S_NV_VECTOR_ELEMENT("anim", std::cref(ser.context->getWorldGeometry().getAnimations()), std::ref(m_anim)),
      S_NV("frame", m_frame),
      S_NV("shadowCaster", m_shadowCaster));

  ser << [this](const serialization::Deserializer<world::World>&)
  {
    m_forceMeshRebuild = true;
    rebuildMesh();
    calculatePoseMatrices(true);
  };
}

void serialize(const std::shared_ptr<SkeletalModelNode>& data, const serialization::Serializer<world::World>& ser)
{
  gsl_Expects(data != nullptr);
  data->serialize(ser);
}

void deserialize(std::shared_ptr<SkeletalModelNode>& data, const serialization::Deserializer<world::World>& ser)
{
  const world::SkeletalModelType* model = nullptr;
  bool shadowCaster;
  ser(S_NV("model", model), S_NV("shadowCaster", shadowCaster));
  data = std::make_shared<SkeletalModelNode>(
    create(serialization::TypeId<std::string>{}, ser["id"]), ser.context, gsl_lite::not_null{model}, shadowCaster);
  data->deserialize(ser);
}

void SkeletalModelNode::buildMesh(const std::shared_ptr<SkeletalModelNode>& skeleton, core::AnimStateId& animState)
{
  if(!skeleton->m_meshParts.empty())
    return;

  skeleton->setAnimation(
    animState, gsl_lite::not_null{&skeleton->m_model->animations[0]}, skeleton->m_model->animations->firstFrame);
  for(const auto& bone : skeleton->m_model->bones)
  {
    skeleton->m_meshParts.emplace_back(bone.mesh.get());
  }

  skeleton->m_forceMeshRebuild = true;
  skeleton->rebuildMesh();
  skeleton->calculatePoseMatrices(true);
}

void SkeletalModelNode::rebuildMesh()
{
  if(!m_forceMeshRebuild
     && !std::ranges::any_of(m_meshParts,
                             [](const auto& part)
                             {
                               return part.meshChanged();
                             }))
  {
    return;
  }

  m_forceMeshRebuild = false;

  world::RenderMeshDataCompositor compositor;
  for(auto& part : m_meshParts)
  {
    if(part.mesh == nullptr || !part.visible)
      compositor.appendEmpty();
    else
      compositor.append(*part.mesh, part.reflective);

    part.currentVisible = part.visible;
    part.currentMesh = part.mesh;
    part.currentReflective = part.reflective;
  }

  if(compositor.empty())
    setRenderable(nullptr);
  else
    setRenderable(compositor.toMesh(
      m_world->getEngine().getPresenter().getRenderSystem().getMaterialManager(),
      true,
      m_shadowCaster,
      [&engine = m_world->getEngine()]
      {
        return engine.getEngineConfig()->animSmoothing;
      },
      [&engine = m_world->getEngine()]
      {
        const auto& settings = engine.getEngineConfig()->renderSettings;
        return !settings.lightingModeActive ? 0 : settings.lightingMode;
      },
      getName()));
}

bool SkeletalModelNode::canBeCulled(const glm::mat4& viewProjection) const
{
  auto bbox = getInterpolationInfo().firstFrame->bbox.toBBox();
  bbox.x = bbox.x.broadened(bbox.x.size() / 2);
  bbox.y = bbox.x.broadened(bbox.y.size() / 2);
  bbox.z = bbox.x.broadened(bbox.z.size() / 2);

  glm::vec2 min{1000.0f}, max{-1000.0f};
  for(const auto& x : {bbox.x.min, bbox.x.max})
    for(const auto& y : {bbox.y.min, bbox.y.max})
      for(const auto& z : {bbox.z.min, bbox.z.max})
      {
        const auto v = core::TRVec{x, y, z}.toRenderSystem();
        auto proj = viewProjection * getModelMatrix() * glm::vec4{v, 1.0f};
        proj /= proj.w;
        min = glm::min(min, glm::vec2{proj});
        max = glm::max(max, glm::vec2{proj});
      }

  return min.x > 1 || min.y > 1 || max.x < -1 || max.y < -1;
}

void SkeletalModelNode::setAnim(const gsl_lite::not_null<const world::Animation*>& anim,
                                const std::optional<core::Frame>& frame)
{
  m_anim = anim;
  m_frame = frame.value_or(anim->firstFrame);
}

core::Frame SkeletalModelNode::getLocalFrame() const noexcept
{
  return m_frame - m_anim->firstFrame;
}

void SkeletalModelNode::replaceAnim(const gsl_lite::not_null<const world::Animation*>& anim,
                                    const core::Frame& localFrame)
{
  setAnim(anim, anim->firstFrame + localFrame);
}

const gl::ShaderStorageBuffer<glm::mat4>&
  SkeletalModelNode::getMeshMatricesBuffer(const std::function<bool()>& smooth) const
{
  std::vector<glm::mat4> matrices;
  std::ranges::transform(m_meshParts,
                         std::back_inserter(matrices),
                         [smooth](const auto& part)
                         {
                           return smooth() ? part.poseMatrixSmooth.value_or(part.poseMatrix) : part.poseMatrix;
                         });

  if(m_meshMatricesBuffer == nullptr || m_meshMatricesBuffer->size() != matrices.size())
  {
    m_meshMatricesBuffer = std::make_unique<gl::ShaderStorageBuffer<glm::mat4>>(
      "mesh-matrices-ssb", gl::api::BufferUsage::DynamicDraw, matrices);
  }
  else
  {
    m_meshMatricesBuffer->setSubData(matrices, 0);
  }

  return *m_meshMatricesBuffer;
}

const gl::ShaderStorageBuffer<glm::mat4>&
  SkeletalModelNode::getNextMeshMatricesBuffer(const std::function<bool()>& smooth) const
{
  std::vector<glm::mat4> matrices;
  std::ranges::transform(m_meshParts,
                         std::back_inserter(matrices),
                         [smooth](const auto& part)
                         {
                           return smooth() ? part.nextPoseMatrixSmooth.value_or(part.nextPoseMatrix)
                                           : part.nextPoseMatrix;
                         });

  if(m_nextMeshMatricesBuffer == nullptr || m_nextMeshMatricesBuffer->size() != matrices.size())
  {
    m_nextMeshMatricesBuffer = std::make_unique<gl::ShaderStorageBuffer<glm::mat4>>(
      "next-mesh-matrices-ssb", gl::api::BufferUsage::DynamicDraw, matrices);
  }
  else
  {
    m_nextMeshMatricesBuffer->setSubData(matrices, 0);
  }

  return *m_nextMeshMatricesBuffer;
}

void SkeletalModelNode::updateSmoothMatrices()
{
  for(auto& part : m_meshParts)
  {
    part.updateSmoothMatrices();
  }
}

void SkeletalModelNode::MeshPart::updateSmoothMatrices()
{
  static constexpr float Smoothing = 0.2f;
  if(!poseMatrixSmooth.has_value())
    poseMatrixSmooth = poseMatrix;
  else
    poseMatrixSmooth = Smoothing * *poseMatrixSmooth + (1 - Smoothing) * poseMatrix;
  if(!nextPoseMatrixSmooth.has_value())
    nextPoseMatrixSmooth = nextPoseMatrix;
  else
    nextPoseMatrixSmooth = Smoothing * *nextPoseMatrixSmooth + (1 - Smoothing) * nextPoseMatrix;
}

void SkeletalModelNode::MeshPart::serialize(const serialization::Serializer<world::World>& ser) const
{
  ser(S_NV("patch", patch), S_NV("poseMatrix", poseMatrix), S_NV("mesh", mesh), S_NV("visible", visible));
}

void SkeletalModelNode::MeshPart::deserialize(const serialization::Deserializer<world::World>& ser)
{
  ser(S_NV("patch", patch), S_NV("poseMatrix", poseMatrix), S_NV("mesh", mesh), S_NV("visible", visible));
}

SkeletalModelNode::MeshPart SkeletalModelNode::MeshPart::create(const serialization::Deserializer<world::World>& ser)
{
  MeshPart tmp{};
  tmp.deserialize(ser);
  return tmp;
}
} // namespace engine