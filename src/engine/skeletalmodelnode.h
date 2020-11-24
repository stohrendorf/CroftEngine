#pragma once

#include "loader/file/animation.h"
#include "render/scene/node.h"

#include <gsl-lite.hpp>
#include <utility>

namespace loader::file
{
struct SkeletalModelType;
struct Animation;
} // namespace loader::file

namespace engine
{
class World;

namespace objects
{
struct ObjectState;
}
class SkeletalModelNode : public render::scene::Node
{
public:
  explicit SkeletalModelNode(const std::string& id,
                             gsl::not_null<const World*> world,
                             gsl::not_null<const loader::file::SkeletalModelType*> model);

  void updatePose();

  void setAnimation(core::AnimStateId& animState,
                    const gsl::not_null<const loader::file::Animation*>& animation,
                    core::Frame frame);

  core::Speed calculateFloorSpeed(const core::Frame& frameOffset = 0_frame) const;

  loader::file::BoundingBox getBoundingBox() const;

  void patchBone(const size_t idx, const glm::mat4& m)
  {
    m_meshParts.at(idx).patch = m;
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

  static void buildMesh(const std::shared_ptr<SkeletalModelNode>& skeleton, core::AnimStateId& animState);

  void rebuildMesh();

  bool canBeCulled(const glm::mat4& viewProjection) const override;

  const loader::file::Animation* anim = nullptr;
  core::Frame frame_number = 0_frame;

  void setMeshPart(size_t idx, const std::shared_ptr<loader::file::RenderMeshData>& mesh)
  {
    m_needsMeshRebuild |= std::exchange(m_meshParts.at(idx).mesh, mesh) != mesh;
  }

  const auto& getMeshPart(size_t idx) const
  {
    return m_meshParts.at(idx).mesh;
  }

  size_t getBoneCount() const
  {
    return m_meshParts.size();
  }

  void setMeshMatrix(size_t idx, const glm::mat4& m)
  {
    m_meshParts.at(idx).matrix = m;
  }

  void setVisible(size_t idx, bool visible)
  {
    m_needsMeshRebuild |= std::exchange(m_meshParts.at(idx).visible, visible) != visible;
  }

  bool isVisible(size_t idx) const
  {
    return m_meshParts.at(idx).visible;
  }

  const auto& getMeshMatricesBuffer() const
  {
    std::vector<glm::mat4> matrices;
    std::transform(m_meshParts.begin(), m_meshParts.end(), std::back_inserter(matrices), [](const auto& part) {
      return part.matrix;
    });
    m_meshMatricesBuffer.setData(matrices, gl::api::BufferUsageARB::DynamicDraw);
    return m_meshMatricesBuffer;
  }

  void clearParts()
  {
    m_meshParts.clear();
    m_needsMeshRebuild = true;
    rebuildMesh();
  }

protected:
  bool handleStateTransitions(core::AnimStateId& animState, const core::AnimStateId& goal);

private:
  struct MeshPart
  {
    explicit MeshPart(std::shared_ptr<loader::file::RenderMeshData> mesh = nullptr)
        : mesh{std::move(mesh)}
    {
    }

    glm::mat4 patch{1.0f};
    glm::mat4 matrix{1.0f};
    std::shared_ptr<loader::file::RenderMeshData> mesh{nullptr};
    bool visible = true;

    void serialize(const serialization::Serializer& ser);
    static MeshPart create(const serialization::Serializer& ser);
  };

  const gsl::not_null<const World*> m_world;
  gsl::not_null<const loader::file::SkeletalModelType*> m_model;
  std::vector<MeshPart> m_meshParts{};
  mutable gl::ShaderStorageBuffer<glm::mat4> m_meshMatricesBuffer;
  bool m_needsMeshRebuild = false;

  void updatePoseKeyframe(const InterpolationInfo& framePair);
  void updatePoseInterpolated(const InterpolationInfo& framePair);
};

void serialize(std::shared_ptr<SkeletalModelNode>& data, const serialization::Serializer& ser);

} // namespace engine
