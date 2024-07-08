#pragma once

#include "core/containeroffset.h"
#include "core/id.h"
#include "core/units.h"
#include "core/vec.h"
#include "loader/file/animation.h"
#include "rendermeshdata.h"

#include <boost/assert.hpp>
#include <cstdint>
#include <glm/vec3.hpp>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <optional>
#include <utility>
#include <vector>

namespace engine::world
{
struct SkeletalModelType final
{
  core::TypeId type{uint16_t(0)};
  core::ContainerIndex<uint16_t, gsl::not_null<const Mesh*>, gslu::nn_shared<render::scene::Mesh>> meshBaseIndex;

  struct Bone final
  {
    gslu::nn_shared<RenderMeshData> mesh;
    core::TRVec collisionCenter;
    core::Length collisionSize;
    glm::vec3 position;
    bool pushMatrix;
    bool popMatrix;

    explicit Bone(gslu::nn_shared<RenderMeshData> mesh,
                  core::TRVec collisionCenter,
                  const core::Length& collision_size,
                  const std::optional<loader::file::BoneTreeEntry>& boneTreeEntry)
        : mesh{std::move(mesh)}
        , collisionCenter{std::move(collisionCenter)}
        , collisionSize{collision_size}
        , position{boneTreeEntry.has_value() ? boneTreeEntry->toGl() : glm::vec3{0}}
        , pushMatrix{(boneTreeEntry.has_value() && (boneTreeEntry->flags & 0x02u) != 0)}
        , popMatrix{(boneTreeEntry.has_value() && (boneTreeEntry->flags & 0x01u) != 0)}
    {
      BOOST_ASSERT(!boneTreeEntry.has_value() || (boneTreeEntry->flags & 0x1cu) == 0);
    }
  };

  std::vector<Bone> bones;

  const loader::file::AnimFrame* frames = nullptr;
  const Animation* animations = nullptr;
};
} // namespace engine::world
