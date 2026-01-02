#pragma once

#include "core/id.h"
#include "core/units.h"

#include <cstdint>
#include <gsl-lite/gsl-lite.hpp>

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
  core::AnimStateId stateId{static_cast<uint16_t>(0)};
  gsl_lite::span<const TransitionCase> transitionCases;
};
} // namespace engine::world