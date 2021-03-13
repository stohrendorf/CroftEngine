#pragma once

#include "core/id.h"
#include "core/units.h"

namespace engine::world
{
struct Animation
{
  const loader::file::AnimFrame* frames = nullptr;

  core::Frame segmentLength = 0_frame;
  core::AnimStateId state_id = 0_as;

  core::Speed speed{};
  core::Acceleration acceleration{};

  core::Frame firstFrame = 0_frame;
  core::Frame lastFrame = 0_frame;
  core::Frame nextFrame = 0_frame;

  uint16_t animCommandCount{};
  const int16_t* animCommands = nullptr;

  const Animation* nextAnimation = nullptr;
  gsl::span<const loader::file::TypedTransitions> transitions{};

  [[nodiscard]] constexpr core::Frame getFrameCount() const
  {
    return lastFrame - firstFrame + 1_frame;
  }
};
} // namespace engine::world
