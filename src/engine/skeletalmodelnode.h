#pragma once

#include "core/id.h"
#include "core/units.h"
#include "core/vec.h"
#include "render/scene/node.h"
#include "serialization/serialization_fwd.h"

#include <cstddef>
#include <functional>
#include <gl/buffer.h>
#include <gl/pixel.h>
#include <glm/fwd.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <gsl-lite/gsl-lite.hpp>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace core
{
struct BoundingBox;
}

namespace loader::file
{
struct AnimFrame;
}

namespace engine::world
{
struct Animation;
struct SkeletalModelType;
class World;
class RenderMeshData;
} // namespace engine::world

namespace engine::objects
{
struct ObjectState;
}

namespace engine
{
struct AnimSegmentInterpolationInfo
{
  // Animations can span multiple logic ticks (not strictly 30fps - keyframes may be several ticks apart).
  gsl_lite::not_null<const loader::file::AnimFrame*> firstFrame;
  gsl_lite::not_null<const loader::file::AnimFrame*> secondFrame;
  float interKeyframeFactor;

  [[nodiscard]] const auto& getNearestFrame() const noexcept
  {
    if(interKeyframeFactor <= 0.5f)
    {
      return firstFrame;
    }
    else
    {
      return secondFrame;
    }
  }
};

class SkeletalModelNode final : public render::scene::Node
{
public:
  explicit SkeletalModelNode(const std::string& id,
                             gsl_lite::not_null<world::World*> world,
                             gsl_lite::not_null<const world::SkeletalModelType*> model,
                             bool shadowCaster);

  void calculatePoseMatrices(bool predictive);

  void setAnimation(core::AnimStateId& animState,
                    const gsl_lite::not_null<const world::Animation*>& animation,
                    core::Frame frame);

  [[nodiscard]] core::Speed calculateFloorSpeed() const;
  [[nodiscard]] core::Acceleration getAcceleration() const noexcept;

  [[nodiscard]] core::BoundingBox getBoundingBox() const;

  void patchBone(const size_t idx, const glm::mat4& m)
  {
    m_meshParts.at(idx).patch = m;
  }

  [[nodiscard]] bool advanceFrame(objects::ObjectState& state);

  [[nodiscard]] AnimSegmentInterpolationInfo getInterpolationInfo() const;
  [[nodiscard]] AnimSegmentInterpolationInfo getNextInterpolationInfo() const;

  struct Sphere
  {
    glm::mat4 m;
    core::TRVec offset;
    core::Length radius;

    Sphere(const glm::mat4& m, core::TRVec offset, const core::Length& radius) noexcept
        : m{m}
        , offset{std::move(offset)}
        , radius{radius}
    {
    }

    [[nodiscard]] glm::vec3 getPosition() const
    {
      return {m[3]};
    }

    [[nodiscard]] glm::vec3 getCollisionPosition() const
    {
      return relative(offset.toRenderSystem());
    }

    [[nodiscard]] glm::vec3 relative(const glm::vec3& pos) const
    {
      return glm::vec3{m * glm::vec4{pos, 1.0f}};
    }
  };

  /**
* Bone collision spheres in world space.
*/
  [[nodiscard]] std::vector<Sphere> getBoneCollisionSpheres();

  void serialize(const serialization::Serializer<world::World>& ser) const;
  void deserialize(const serialization::Deserializer<world::World>& ser);

  static void buildMesh(const std::shared_ptr<SkeletalModelNode>& skeleton, core::AnimStateId& animState);

  void rebuildMesh();

  [[nodiscard]] bool canBeCulled(const glm::mat4& viewProjection) const override;

  void setMesh(const size_t idx, const std::shared_ptr<world::RenderMeshData>& mesh)
  {
    m_meshParts.at(idx).mesh = mesh;
  }

  [[nodiscard]] const auto& getMesh(const size_t idx) const
  {
    return m_meshParts.at(idx).mesh;
  }

  [[nodiscard]] const auto& getCurrentMesh(const size_t idx) const
  {
    return m_meshParts.at(idx).currentMesh;
  }

  [[nodiscard]] glm::vec3 getMeshPartTranslationWorld(const size_t idx) const
  {
    auto m = getModelMatrix() * m_meshParts.at(idx).poseMatrix;
    return glm::vec3{m[3]};
  }

  [[nodiscard]] size_t getBoneCount() const noexcept
  {
    return m_meshParts.size();
  }

  void setPoseMatrix(const size_t idx, const glm::mat4& m, const glm::mat4& next)
  {
    m_meshParts.at(idx).poseMatrix = m;
    m_meshParts.at(idx).nextPoseMatrix = next;
  }

  [[nodiscard]] const auto& getPoseMatrix(const size_t idx) const
  {
    return m_meshParts.at(idx).poseMatrix;
  }

  [[nodiscard]] const auto& getNextPoseMatrix(const size_t idx) const
  {
    return m_meshParts.at(idx).nextPoseMatrix;
  }

  void setReflective(const size_t idx, const gl::SRGBA8& reflective)
  {
    m_meshParts.at(idx).reflective = reflective;
  }

  void setVisible(const size_t idx, const bool visible)
  {
    m_meshParts.at(idx).visible = visible;
  }

  bool isVisible(const size_t idx) const
  {
    return m_meshParts.at(idx).visible;
  }

  [[nodiscard]] const gl::ShaderStorageBuffer<glm::mat4>&
    getMeshMatricesBuffer(const std::function<bool()>& smooth) const;

  [[nodiscard]] const gl::ShaderStorageBuffer<glm::mat4>&
    getNextMeshMatricesBuffer(const std::function<bool()>& smooth) const;

  void updateSmoothMatrices();

  void clearParts()
  {
    m_meshParts.clear();
    m_forceMeshRebuild = true;
    rebuildMesh();
  }

  void setAnim(const gsl_lite::not_null<const world::Animation*>& anim,
               const std::optional<core::Frame>& frame = std::nullopt);

  void replaceAnim(const gsl_lite::not_null<const world::Animation*>& anim, const core::Frame& localFrame);

  [[nodiscard]] const auto& getFrame() const noexcept
  {
    return m_frame;
  }

  [[nodiscard]] core::Frame getLocalFrame() const noexcept;

  [[nodiscard]] const auto& getAnim() const noexcept
  {
    return m_anim;
  }

  void resetSmoothing() noexcept
  {
    for(auto& part : m_meshParts)
    {
      part.poseMatrixSmooth.reset();
      part.nextPoseMatrixSmooth.reset();
    }
  }

protected:
  bool handleStateTransitions(core::AnimStateId& animState, const core::AnimStateId& goal);

private:
  struct MeshPart
  {
    explicit MeshPart(std::shared_ptr<world::RenderMeshData> mesh = nullptr)
        : mesh{std::move(mesh)}
    {
    }

    glm::mat4 patch{1.0f};
    glm::mat4 poseMatrix{1.0f};
    std::optional<glm::mat4> poseMatrixSmooth;
    glm::mat4 nextPoseMatrix{1.0f};
    std::optional<glm::mat4> nextPoseMatrixSmooth;
    std::shared_ptr<world::RenderMeshData> mesh{nullptr};
    std::shared_ptr<world::RenderMeshData> currentMesh{nullptr};
    gl::SRGBA8 reflective{0, 0, 0, 0};
    gl::SRGBA8 currentReflective{0, 0, 0, 0};
    bool visible = true;
    bool currentVisible = true;

    [[nodiscard]] bool meshChanged() const
    {
      return visible != currentVisible || mesh != currentMesh || reflective != currentReflective;
    }

    void updateSmoothMatrices();

    void serialize(const serialization::Serializer<world::World>& ser) const;
    void deserialize(const serialization::Deserializer<world::World>& ser);
    [[nodiscard]] static MeshPart create(const serialization::Deserializer<world::World>& ser);
  };

  gsl_lite::not_null<world::World*> m_world;
  gsl_lite::not_null<const world::SkeletalModelType*> m_model;
  std::vector<MeshPart> m_meshParts;
  mutable std::unique_ptr<gl::ShaderStorageBuffer<glm::mat4>> m_meshMatricesBuffer;
  mutable std::unique_ptr<gl::ShaderStorageBuffer<glm::mat4>> m_nextMeshMatricesBuffer;
  bool m_forceMeshRebuild = false;

  const world::Animation* m_anim = nullptr;
  core::Frame m_frame = 0_frame;

  void calculatePoseMatrices(const AnimSegmentInterpolationInfo& framePair, glm::mat4 MeshPart::* targetMatrix);
  AnimSegmentInterpolationInfo getInterpolationInfo(const world::Animation& anim, core::Frame frame) const;

  bool m_shadowCaster;
};

void serialize(const std::shared_ptr<SkeletalModelNode>& data, const serialization::Serializer<world::World>& ser);
void deserialize(std::shared_ptr<SkeletalModelNode>& data, const serialization::Deserializer<world::World>& ser);
} // namespace engine