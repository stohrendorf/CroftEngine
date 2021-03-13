#pragma once

#include "core/id.h"
#include "core/units.h"

namespace engine::world
{
struct TransitionCase
{
  const core::Frame firstFrame;
  const core::Frame lastFrame;
  const core::Frame targetFrame;

  const engine::world::Animation* targetAnimation = nullptr;
};

struct Transitions
{
  core::AnimStateId stateId{uint16_t(0)};
  gsl::span<const TransitionCase> transitionCases{};
};
} // namespace engine::world
