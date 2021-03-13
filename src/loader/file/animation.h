#pragma once

#include "core/containeroffset.h"
#include "core/id.h"
#include "core/units.h"
#include "core/vec.h"

#include <boost/assert.hpp>
#include <gsl-lite.hpp>
#include <optional>
#include <utility>

namespace render::scene
{
class Mesh;
}

namespace engine::world
{
struct Animation;
}

namespace loader::file
{
namespace io
{
class SDLReader;
}

#pragma pack(push, 1)

struct BoundingBox
{
  core::Length minX{0_len}, maxX{0_len};
  core::Length minY{0_len}, maxY{0_len};
  core::Length minZ{0_len}, maxZ{0_len};

  explicit BoundingBox() = default;

  BoundingBox(const core::Length& minX,
              const core::Length& maxX,
              const core::Length& minY,
              const core::Length& maxY,
              const core::Length& minZ,
              const core::Length& maxZ)
      : minX{minX}
      , maxX{maxX}
      , minY{minY}
      , maxY{maxY}
      , minZ{minZ}
      , maxZ{maxZ}
  {
  }

  BoundingBox(const BoundingBox& a, const BoundingBox& b, const float bias)
      : minX{lerp(a.minX, b.minX, bias)}
      , maxX{lerp(a.maxX, b.maxX, bias)}
      , minY{lerp(a.minY, b.minY, bias)}
      , maxY{lerp(a.maxY, b.maxY, bias)}
      , minZ{lerp(a.minZ, b.minZ, bias)}
      , maxZ{lerp(a.maxZ, b.maxZ, bias)}
  {
  }

  [[nodiscard]] core::TRVec getCenter() const
  {
    return {(minX + maxX) / 2, (minY + maxY) / 2, (minZ + maxZ) / 2};
  }
};

struct BoundingBoxIO
{
  int16_t minX{0}, maxX{0};
  int16_t minY{0}, maxY{0};
  int16_t minZ{0}, maxZ{0};

  [[nodiscard]] BoundingBox toBBox() const
  {
    return BoundingBox{core::Length{static_cast<core::Length::type>(minX)},
                       core::Length{static_cast<core::Length::type>(maxX)},
                       core::Length{static_cast<core::Length::type>(minY)},
                       core::Length{static_cast<core::Length::type>(maxY)},
                       core::Length{static_cast<core::Length::type>(minZ)},
                       core::Length{static_cast<core::Length::type>(maxZ)}};
  }
};

struct AnimFrame
{
  struct Vec
  {
    int16_t x = 0, y = 0, z = 0;

    [[nodiscard]] glm::vec3 toGl() const noexcept
    {
      return toTr().toRenderSystem();
    }

    [[nodiscard]] core::TRVec toTr() const noexcept
    {
      return core::TRVec{core::Length{static_cast<core::Length::type>(x)},
                         core::Length{static_cast<core::Length::type>(y)},
                         core::Length{static_cast<core::Length::type>(z)}};
    }
  };

  BoundingBoxIO bbox;
  Vec pos{};
  uint16_t numValues = 0;

  [[nodiscard]] gsl::span<const uint32_t> getAngleData() const noexcept
  {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    const auto begin = reinterpret_cast<const uint32_t*>(this + 1);
    return gsl::span(begin, numValues);
  }

  [[nodiscard]] const AnimFrame* next() const
  {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    const auto begin = reinterpret_cast<const uint32_t*>(this + 1);
    const auto end = begin + numValues;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    const auto next = reinterpret_cast<const AnimFrame*>(end);
    Expects(next->numValues == numValues);
    return next;
  }

  [[nodiscard]] const AnimFrame* next(size_t n) const
  {
    auto result = this;
    while(n--)
    {
      result = result->next();
      BOOST_ASSERT(result->numValues == numValues);
    }
    return result;
  }
};

static_assert(sizeof(AnimFrame) == 20, "AnimFrame has wrong size");

#pragma pack(pop)

struct TransitionCase
{
  core::Frame firstFrame = 0_frame; // Lowest frame that uses this range
  core::Frame lastFrame = 0_frame;  // Highest frame (+1?) that uses this range
  core::ContainerIndex<uint16_t, engine::world::Animation> targetAnimationIndex; // Animation to dispatch to
  core::Frame targetFrame = 0_frame;                                             // Frame offset to dispatch to

  static std::unique_ptr<TransitionCase> read(io::SDLReader& reader);
};

struct TypedTransitionCase
{
  const core::Frame firstFrame;
  const core::Frame lastFrame;
  const core::Frame targetFrame;

  const engine::world::Animation* targetAnimation = nullptr;
};

struct Transitions
{
  core::AnimStateId stateId{uint16_t(0)};
  uint16_t transitionCaseCount{}; // number of ranges (seems to always be 1..5)
  core::ContainerIndex<uint16_t, TypedTransitionCase> firstTransitionCase{}; // Offset into AnimDispatches[]

  /// \brief reads an animation state change.
  static std::unique_ptr<Transitions> read(io::SDLReader& reader);
};

struct TypedTransitions
{
  core::AnimStateId stateId{uint16_t(0)};
  gsl::span<const TypedTransitionCase> transitionCases{};
};

struct Animation
{
  core::ContainerOffset<uint32_t, int16_t> poseDataOffset; // byte offset into Frames[] (divide by 2 for Frames[i])

  core::Frame segmentLength = 0_frame; // Slowdown factor of this animation
  uint8_t poseDataSize{};              // number of bit16's in Frames[] used by this animation
  core::AnimStateId state_id = 0_as;

  core::Speed speed{};
  core::Acceleration acceleration{};

  core::Speed lateralSpeed{};               // new in TR4 -->
  core::Acceleration lateralAcceleration{}; // lateral speed and acceleration.

  core::Frame firstFrame = 0_frame; // first frame in this animation
  core::Frame lastFrame = 0_frame;  // last frame in this animation (numframes = (End - Start) + 1)
  uint16_t nextAnimationIndex{};
  core::Frame nextFrame = 0_frame;

  uint16_t transitionsCount{};
  core::ContainerIndex<uint16_t, TypedTransitions> transitionsIndex{}; // offset into StateChanges[]
  uint16_t animCommandCount{};                                         // How many of them to use.
  core::ContainerIndex<uint16_t, int16_t> animCommandIndex{};          // offset into AnimCommand[]

  static std::unique_ptr<Animation> readTr1(io::SDLReader& reader);
  static std::unique_ptr<Animation> readTr4(io::SDLReader& reader);

private:
  static std::unique_ptr<Animation> read(io::SDLReader& reader, bool withLateral);
};

struct Mesh;
class RenderMeshData;

#pragma pack(push, 1)

struct BoneTreeEntry
{
  uint32_t flags;

  int32_t x, y, z;

  [[nodiscard]] glm::vec3 toGl() const noexcept
  {
    return core::TRVec(core::Length{x}, core::Length{y}, core::Length{z}).toRenderSystem();
  }
};

#pragma pack(pop)

static_assert(sizeof(BoneTreeEntry) == 16, "BoneTreeEntry must be of size 16");

struct SkeletalModelType
{
  core::TypeId type{uint16_t(0)};
  int16_t nMeshes
    = 0; // number of meshes in this object, or (in case of sprite sequences) the negative number of sprites in the sequence
  core::ContainerIndex<uint16_t, gsl::not_null<const Mesh*>, gsl::not_null<std::shared_ptr<render::scene::Mesh>>>
    mesh_base_index;                                         // starting mesh (offset into MeshPointers[])
  core::ContainerIndex<uint32_t, int32_t> bone_index;        // offset into MeshTree[]
  core::ContainerOffset<uint32_t, int16_t> pose_data_offset; // byte offset into Frames[] (divide by 2 for Frames[i])
  core::ContainerIndex<uint16_t, engine::world::Animation> animation_index; // offset into Animations[]

  struct Bone
  {
    const gsl::not_null<std::shared_ptr<RenderMeshData>> mesh;
    const core::TRVec center;
    const core::Length collision_size;
    const glm::vec3 position;
    const bool pushMatrix;
    const bool popMatrix;

    explicit Bone(gsl::not_null<std::shared_ptr<RenderMeshData>> mesh,
                  core::TRVec center,
                  const core::Length& collision_size,
                  const std::optional<BoneTreeEntry>& boneTreeEntry)
        : mesh{std::move(mesh)}
        , center{std::move(center)}
        , collision_size{collision_size}
        , position{boneTreeEntry.has_value() ? boneTreeEntry->toGl() : glm::vec3{0}}
        , pushMatrix{(boneTreeEntry.has_value() && (boneTreeEntry->flags & 0x02u) != 0)}
        , popMatrix{(boneTreeEntry.has_value() && (boneTreeEntry->flags & 0x01u) != 0)}
    {
      BOOST_ASSERT(!boneTreeEntry.has_value() || (boneTreeEntry->flags & 0x1cu) == 0);
    }
  };

  std::vector<Bone> bones;
  gsl::span<const BoneTreeEntry> boneTree{};

  const AnimFrame* frames = nullptr;

  const engine::world::Animation* animations = nullptr;

  static std::unique_ptr<SkeletalModelType> readTr1(io::SDLReader& reader);

  static std::unique_ptr<SkeletalModelType> readTr5(io::SDLReader& reader);
};
} // namespace loader::file
