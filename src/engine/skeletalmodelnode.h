#pragma once

#include "loader/file/animation.h"
#include "render/scene/node.h"

#include <gsl-lite.hpp>

namespace loader::file
{
struct SkeletalModelType;
struct Animation;
} // namespace loader::file

namespace engine
{
class Engine;

namespace objects
{
struct ObjectState;
}
class SkeletalModelNode : public render::scene::Node
{
public:
  explicit SkeletalModelNode(const std::string& id,
                             gsl::not_null<const Engine*> engine,
                             gsl::not_null<const loader::file::SkeletalModelType*> model);

  void updatePose();

  void setAnimation(objects::ObjectState& state,
                    const gsl::not_null<const loader::file::Animation*>& animation,
                    core::Frame frame);

  core::Speed calculateFloorSpeed(const core::Frame& frameOffset = 0_frame) const;

  loader::file::BoundingBox getBoundingBox() const;

  void resetPose()
  {
    m_bonePatches.clear();
    m_bonePatches.resize(getChildren().size(), glm::mat4{1.0f});
  }

  void patchBone(const size_t idx, const glm::mat4& m)
  {
    if(m_bonePatches.empty())
    {
      resetPose();
    }

    BOOST_ASSERT(m_bonePatches.size() == getChildren().size());
    BOOST_ASSERT(idx < m_bonePatches.size());

    m_bonePatches[idx] = m;
  }

  bool advanceFrame(objects::ObjectState& state);

  struct InterpolationInfo
  {
    const loader::file::AnimFrame* firstFrame = nullptr;
    const loader::file::AnimFrame* secondFrame = nullptr;
    float bias = 0;

    [[nodiscard]] const loader::file::AnimFrame* getNearestFrame() const
    {
      if(bias <= 0.5f)
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

  InterpolationInfo getInterpolationInfo() const;

  void updatePose(const InterpolationInfo& interpolationInfo)
  {
    if(interpolationInfo.bias == 0 || interpolationInfo.secondFrame == nullptr)
      updatePoseKeyframe(interpolationInfo);
    else
      updatePoseInterpolated(interpolationInfo);
  }

  struct Sphere
  {
    const glm::mat4 m;
    const core::Length radius;

    Sphere(const glm::mat4& m, const core::Length& radius)
        : m{m}
        , radius{radius}
    {
    }

    [[nodiscard]] glm::vec3 getPosition() const
    {
      return glm::vec3(m[3]);
    }
  };

  std::vector<Sphere> getBoneCollisionSpheres(const objects::ObjectState& state,
                                              const loader::file::AnimFrame& frame,
                                              const glm::mat4* baseTransform);

  void serialize(const serialization::Serializer& ser);

  static void initNodes(const std::shared_ptr<SkeletalModelNode>& skeleton, objects::ObjectState& state);

  bool canBeCulled(const glm::mat4& viewProjection) const override;

  const loader::file::Animation* anim = nullptr;
  core::Frame frame_number = 0_frame;

protected:
  bool handleStateTransitions(objects::ObjectState& state);

private:
  const gsl::not_null<const Engine*> m_engine;
  gsl::not_null<const loader::file::SkeletalModelType*> m_model;
  std::vector<glm::mat4> m_bonePatches;

  void updatePoseKeyframe(const InterpolationInfo& framePair);

  void updatePoseInterpolated(const InterpolationInfo& framePair);
};

void serialize(std::shared_ptr<SkeletalModelNode>& data, const serialization::Serializer& ser);

} // namespace engine
