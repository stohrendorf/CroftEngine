#pragma once

#include "core/boundingbox.h"
#include "core/containeroffset.h"
#include "core/id.h"
#include "core/units.h"
#include "core/vec.h"
#include "qs/quantity.h"

#include <boost/assert.hpp>
#include <cstddef>
#include <cstdint>
#include <glm/vec3.hpp>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <memory>

namespace render::scene
{
class Mesh;
}

namespace engine::world
{
struct Animation;
struct TransitionCase;
struct Transitions;
struct Mesh;
} // namespace engine::world

namespace loader::file
{
namespace io
{
class SDLReader;
}

#pragma pack(push, 1)

struct BoundingBoxIO
{
  int16_t minX{0}, maxX{0};
  int16_t minY{0}, maxY{0};
  int16_t minZ{0}, maxZ{0};

  [[nodiscard]] core::BoundingBox toBBox() const
  {
    return core::BoundingBox{core::Length{static_cast<core::Length::type>(minX)},
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

  [[nodiscard]] gsl::span<const uint8_t> getAngleData() const noexcept
  {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    const auto begin = reinterpret_cast<const uint8_t*>(this + 1);
    return {begin, gsl::narrow_cast<std::size_t>(numValues) * sizeof(uint32_t)};
  }

  [[nodiscard]] gsl::not_null<const AnimFrame*> next() const
  {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    const auto end = reinterpret_cast<const AnimFrame*>(&*getAngleData().end());
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    auto next = gsl::not_null{end};
    gsl_Assert(next->numValues == numValues);
    return next;
  }

  [[nodiscard]] gsl::not_null<const AnimFrame*> next(size_t n) const
  {
    auto result = gsl::not_null{this};
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

struct Transitions
{
  core::AnimStateId stateId{uint16_t(0)};
  uint16_t transitionCaseCount{}; // number of ranges (seems to always be 1..5)
  core::ContainerIndex<uint16_t, engine::world::TransitionCase> firstTransitionCase{}; // Offset into AnimDispatches[]

  /// \brief reads an animation state change.
  static std::unique_ptr<Transitions> read(io::SDLReader& reader);
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
  core::ContainerIndex<uint16_t, engine::world::Transitions> transitionsIndex{}; // offset into StateChanges[]
  uint16_t animCommandCount{};                                                   // How many of them to use.
  core::ContainerIndex<uint16_t, int16_t> animCommandIndex{};                    // offset into AnimCommand[]

  static std::unique_ptr<Animation> readTr1(io::SDLReader& reader);
  static std::unique_ptr<Animation> readTr4(io::SDLReader& reader);

private:
  static std::unique_ptr<Animation> read(io::SDLReader& reader, bool withLateral);
};

#pragma pack(push, 1)

struct BoneTreeEntry
{
  uint32_t flags;
  int32_t x, y, z;

  [[nodiscard]] glm::vec3 toGl() const noexcept
  {
    return core::TRVec(core::Length{gsl::narrow_cast<core::Length::type>(x)},
                       core::Length{gsl::narrow_cast<core::Length::type>(y)},
                       core::Length{gsl::narrow_cast<core::Length::type>(z)})
      .toRenderSystem();
  }
};

#pragma pack(pop)

static_assert(sizeof(BoneTreeEntry) == 16, "BoneTreeEntry must be of size 16");

struct SkeletalModelType
{
  core::TypeId type{uint16_t(0)};
  int16_t nMeshes
    = 0; // number of meshes in this object, or (in case of sprite sequences) the negative number of sprites in the sequence
  core::ContainerIndex<uint16_t,
                       gsl::not_null<const engine::world::Mesh*>,
                       gslu::nn_shared<render::scene::Mesh>>
    mesh_base_index;                                         // starting mesh (offset into MeshPointers[])
  core::ContainerIndex<uint32_t, int32_t> bone_index;        // offset into MeshTree[]
  core::ContainerOffset<uint32_t, int16_t> pose_data_offset; // byte offset into Frames[] (divide by 2 for Frames[i])
  core::ContainerIndex<uint16_t, engine::world::Animation> animation_index; // offset into Animations[]

  gsl::span<const BoneTreeEntry> boneTree{};

  static std::unique_ptr<SkeletalModelType> readTr1(io::SDLReader& reader);
  static std::unique_ptr<SkeletalModelType> readTr5(io::SDLReader& reader);
};
} // namespace loader::file
