#pragma once

#include "core/id.h"
#include "core/units.h"
#include "core/vec.h"
#include "render/scene/node.h"
#include "serialization/serialization_fwd.h"

#include <algorithm>
#include <cstddef>
#include <gl/buffer.h>
#include <gl/pixel.h>
#include <glm/fwd.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <gsl/gsl-lite.hpp>
#include <iterator>
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
struct InterpolationInfo
{
  const gsl::not_null<const loader::file::AnimFrame*> firstFrame;
  const gsl::not_null<const loader::file::AnimFrame*> secondFrame;
  const float bias;

  [[nodiscard]] const auto& getNearestFrame() const
  {
    if(bias <= 0.5f)
    {
      return firstFrame;
    }
    else
    {
      return secondFrame;
    }
  }
};

class SkeletalModelNode : public render::scene::Node
{
public:
  explicit SkeletalModelNode(const std::string& id,
                             gsl::not_null<const world::World*> world,
                             gsl::not_null<const world::SkeletalModelType*> model,
                             bool shadowCaster);

  void updatePose();

  void setAnimation(core::AnimStateId& animState,
                    const gsl::not_null<const world::Animation*>& animation,
                    core::Frame frame);

  [[nodiscard]] core::Speed calculateFloorSpeed() const;
  [[nodiscard]] core::Acceleration getAcceleration() const;

  [[nodiscard]] core::BoundingBox getBoundingBox() const;

  void patchBone(const size_t idx, const glm::mat4& m)
  {
    m_meshParts.at(idx).patch = m;
  }

  [[nodiscard]] bool advanceFrame(objects::ObjectState& state);

  InterpolationInfo getInterpolationInfo() const;

  struct Sphere
  {
    const glm::mat4 m;
    const core::TRVec offset;
    const core::Length radius;

    Sphere(const glm::mat4& m, core::TRVec offset, const core::Length& radius)
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

  void serialize(const serialization::Serializer<world::World>& ser);

  static void buildMesh(const std::shared_ptr<SkeletalModelNode>& skeleton, core::AnimStateId& animState);

  void rebuildMesh();

  [[nodiscard]] bool canBeCulled(const glm::mat4& viewProjection) const override;

  void setMeshPart(size_t idx, const std::shared_ptr<world::RenderMeshData>& mesh)
  {
    m_meshParts.at(idx).mesh = mesh;
  }

  [[nodiscard]] const auto& getMeshPart(size_t idx) const
  {
    return m_meshParts.at(idx).mesh;
  }

  [[nodiscard]] const auto& getCurrentMeshPart(size_t idx) const
  {
    return m_meshParts.at(idx).currentMesh;
  }

  [[nodiscard]] const auto& getPoseMatrix(size_t idx) const
  {
    return m_meshParts.at(idx).poseMatrix;
  }

  [[nodiscard]] glm::vec3 getMeshPartTranslationWorld(size_t idx) const
  {
    auto m = getModelMatrix() * m_meshParts.at(idx).poseMatrix;
    return glm::vec3{m[3]};
  }

  [[nodiscard]] size_t getBoneCount() const
  {
    return m_meshParts.size();
  }

  void setMeshMatrix(size_t idx, const glm::mat4& m)
  {
    m_meshParts.at(idx).poseMatrix = m;
  }

  void setMeshReflective(size_t idx, const gl::SRGBA8& reflective)
  {
    m_meshParts.at(idx).reflective = reflective;
  }

  void setVisible(size_t idx, bool visible)
  {
    m_meshParts.at(idx).visible = visible;
  }

  bool isVisible(size_t idx) const
  {
    return m_meshParts.at(idx).visible;
  }

  [[nodiscard]] const gl::ShaderStorageBuffer<glm::mat4>& getMeshMatricesBuffer(std::function<bool()> smooth) const;

  void clearParts()
  {
    m_meshParts.clear();
    m_forceMeshRebuild = true;
    rebuildMesh();
  }

  void setAnim(const gsl::not_null<const world::Animation*>& anim,
               const std::optional<core::Frame>& frame = std::nullopt);

  void replaceAnim(const gsl::not_null<const world::Animation*>& anim, const core::Frame& localFrame);

  [[nodiscard]] const auto& getFrame() const
  {
    return m_frame;
  }

  [[nodiscard]] core::Frame getLocalFrame() const;

  [[nodiscard]] const auto& getAnim() const
  {
    return m_anim;
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
    mutable std::optional<glm::mat4> poseMatrixSmooth;
    std::shared_ptr<world::RenderMeshData> mesh{nullptr};
    std::shared_ptr<world::RenderMeshData> currentMesh{nullptr};
    bool visible = true;
    bool currentVisible = true;
    gl::SRGBA8 reflective{0, 0, 0, 0};
    gl::SRGBA8 currentReflective{0, 0, 0, 0};

    [[nodiscard]] bool meshChanged() const
    {
      return visible != currentVisible || mesh != currentMesh || reflective != currentReflective;
    }

    void serialize(const serialization::Serializer<world::World>& ser);
    [[nodiscard]] static MeshPart create(const serialization::Serializer<world::World>& ser);
  };

  const gsl::not_null<const world::World*> m_world;
  gsl::not_null<const world::SkeletalModelType*> m_model;
  std::vector<MeshPart> m_meshParts{};
  mutable std::unique_ptr<gl::ShaderStorageBuffer<glm::mat4>> m_meshMatricesBuffer;
  bool m_forceMeshRebuild = false;

  const world::Animation* m_anim = nullptr;
  core::Frame m_frame = 0_frame;

  void updatePose(const InterpolationInfo& framePair);

  bool m_shadowCaster;
};

void serialize(std::shared_ptr<SkeletalModelNode>& data, const serialization::Serializer<world::World>& ser);

} // namespace engine
