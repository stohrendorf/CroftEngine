#pragma once

#include "core/id.h"
#include "core/units.h"

namespace engine::world
{
struct Animation;

struct TransitionCase
{
  core::Frame firstFrame;
  core::Frame lastFrame;
  core::Frame targetFrame;

  const Animation* targetAnimation = nullptr;
};

struct Transitions
{
  core::AnimStateId stateId{uint16_t(0)};
  gsl::span<const TransitionCase> transitionCases{};
};
} // namespace engine::world
